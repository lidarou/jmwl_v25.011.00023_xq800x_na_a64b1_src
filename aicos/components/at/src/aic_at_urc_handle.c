/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT URC
 *  This is the handler of AT URC received from CP.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f7777
#define LOG_MODULE    AMOD_AT

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <sysoff.h>
#include "aic_at_process_common.h"
#include "aic_general_at_handle.h"
#include "aic_at_urc_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_at_parse_utils.h"
#include "aic_at_interact_cp.h"
#include "aic_net_pdp_mgr.h"
#ifdef CONFIG_NET_SHARE_COMMON
#include "aic_net_share_common.h"
#endif

/* Define internal functions. */
static u32 handle_at_urc_ctze(u8 *urc_str, u8 *start_str);
static u32 handle_at_urc_cereg(u8 *urc_str, u8 *start_str);
static s32 adjust_time_zone(u8 *time_zone);
static u32 handle_at_urc_cardstatus(u8 *urc_str, u8 *start_str);
static u32 handle_at_urc_act(u8 *urc_str, u8 *start_str);
static u32 handle_at_urc_ephystat(u8 *urc_str, u8 *start_str);
static u32 handle_at_urc_cperr(u8 *urc_str, u8 *start_str);
#ifdef CONFIG_CUSTOMER_CM_AT
static u32 handle_at_urc_cpin(u8 *urc_str, u8 *start_str);
static u32 handle_at_urc_cscon(u8 *urc_str, u8 *start_str);
#endif

u8 time_zone_set_flag = 0;
REG_SERVICE_INFO reg_service_info;

/* The URC to handle,received from CP. */
static AT_URC_FROM_CP_HANDLE *at_urc_handle_list[AT_MAX_URC_HANDLES] = {0};

static AT_URC_FROM_CP_HANDLE general_urc_handles[] =
{
    {"+CTZE: ", handle_at_urc_ctze},
    {"+CTZEU: ", handle_at_urc_ctze},
    {"+CEREG: ", handle_at_urc_cereg},
    {"^CARDSTATUS: ", handle_at_urc_cardstatus},
    {"^ACT: ", handle_at_urc_act},
#ifdef CONFIG_CUSTOMER_CM_AT
    {AT_CPIN_RESP_PREFIX, handle_at_urc_cpin},
    {AT_CSCON_RESP_PREFIX, handle_at_urc_cscon},
#endif
    {"^EPHYSTAT: ", handle_at_urc_ephystat},
    {"^CPERR: ", handle_at_urc_cperr},
    {AIC_NULL},
};

static cereg_update_cb cereg_state_cbs[AT_MAX_CP_URC_CBS];
static ephystat_update_cb ephy_state_cbs[AT_MAX_CP_URC_CBS];
#ifdef CONFIG_CUSTOMER_CM_AT
static cscon_update_cb cscon_mode_cbs[AT_MAX_CP_URC_CBS];
#endif

extern aos_event_ptr event_pdp_mgr;

// Add by Nestor. 2024-05-21
typedef void (*CUSTOM_NW_EVT_CB)(u32 eventId, void* pData, u32 len);
CUSTOM_NW_EVT_CB custom_nw_evt_cb = NULL;

// reg_type: 1 register, 0 unregister
void aic_custom_nw_reg_callback(u8 reg_type, CUSTOM_NW_EVT_CB callback)
{
    if (reg_type)
    {
        custom_nw_evt_cb = callback;
    }
    else
    {
        custom_nw_evt_cb = NULL;
    }
}
// Add by Nestor. 2024-05-21


/* URC is Unsolicited Result Code from CP. */
u32 aic_invoke_urc_handler(u8 *urc_str)
{
    AT_URC_FROM_CP_HANDLE *urc_handle_ptr;
    u8 i;

    /* No URC string. */
    if (!urc_str) {
        return AIC_SUCCESS;
    }

    for (i = 0; i < AT_MAX_URC_HANDLES; i++) {
        urc_handle_ptr = at_urc_handle_list[i];

        if (urc_handle_ptr) {
            if (strncasecmp(urc_str, urc_handle_ptr->urc_keyword,
                strlen(urc_handle_ptr->urc_keyword)) == 0) {
                if (urc_handle_ptr->at_urc_handler) {
                    urc_handle_ptr->at_urc_handler(urc_str,
                                                   urc_handle_ptr->urc_keyword);
                }
                break;
            }
        } else {
            /* No handle remains. */
            break;
        }
    }

    return AIC_SUCCESS;
}

/**
 * aic_add_at_urc_handle - Add handles of URC rceived from CP.
 *
 * @head_handle_ptr: The pointer to the first URC handle.
 *
 * @Return SUCCESS if the URC handles are successfully added to
 *  at_urc_handle_list.Return ERROR if at_urc_handle_list is full.
 *
 * If certain URC rceived from CP needed to be processed,then its handle
 * should be added to at_urc_handle_list by the function.
 */
u32 aic_add_at_urc_handle(AT_URC_FROM_CP_HANDLE *head_handle_ptr)
{
    AT_URC_FROM_CP_HANDLE *tmp_handle_ptr;
    u16 i, idle_index = 0;

    if (!head_handle_ptr)
        return AT_ERROR_INVALID_AT_HANDLE;

    /* Find the idle index in the array. */
    for (i = 0; i < AT_MAX_URC_HANDLES; i++) {
        if (at_urc_handle_list[i] == 0) {
            idle_index = i;
            break;
        }
    }
    if (i == AT_MAX_URC_HANDLES)
        return AT_ERROR_URC_HANDLES_OVERFLOW;

    tmp_handle_ptr = head_handle_ptr;
    while (tmp_handle_ptr->urc_keyword != AIC_NULL &&
           idle_index < AT_MAX_URC_HANDLES) {
        at_urc_handle_list[idle_index] = tmp_handle_ptr;
        tmp_handle_ptr++;
        idle_index++;
    }
    if (idle_index == AT_MAX_URC_HANDLES &&
        tmp_handle_ptr->urc_keyword != AIC_NULL) {
        aicx_error(0x11da30, "", "error,too many AT URC handles\n");
        return AT_ERROR_URC_HANDLES_OVERFLOW;
    }

    return AIC_SUCCESS;
}

void aic_at_urc_handle_init(void)
{
    aic_add_at_urc_handle(&general_urc_handles[0]);

    reg_service_info.auth_type = ANET_APN_INVALID_AUTH_TYPE;
    reg_service_info.cs_reg_state = AT_INVALID_REG_STATE;
    reg_service_info.ps_reg_state = AT_INVALID_REG_STATE;
    reg_service_info.flight_mode = ANET_OUT_OF_FLIGHT_MODE;
    reg_service_info.sim_card_state = AT_SIM_CARD_PLUGGED;
#ifndef CONFIG_CUSTOMER_AT
    reg_service_info.ctzu_mode = AT_CTZU_DEF_MODE;
#endif
#ifndef CONFIG_CUSTOMER_CM_AT
    reg_service_info.cereg_mode = AT_CEREG_DEF_MODE;
    reg_service_info.cgerep_mode = AT_CGEREP_DEF_MODE;
    reg_service_info.ctzr_mode = AT_CTZR_DEF_MODE;
    reg_service_info.cardstatus_mode = AT_CARDSTATUS_DEF_MODE;
#endif
}

/**
 * Handle the ^CARDSTATUS URC,which tells the status that sim card is
 * plugged in or plugged out.
 * The format is as follows:
 * ^CARDSTATUS: <status>
 */
u32 handle_at_urc_cardstatus(u8 *urc_str, u8 *start_str)
{
    u16 paras_cnt = 0;
    ulong *paras_arr;
    u8 card_status;

    paras_arr = aic_split_paras_to_str_array(urc_str, start_str, &paras_cnt);
    if (paras_cnt >= 1) {
        card_status = atoi((u8 *)paras_arr[0]);
        reg_service_info.sim_card_state = card_status;
    }

    aic_free_paras_mem(paras_arr, paras_cnt);

    if (AT_SIM_CARD_PLUGGED == reg_service_info.sim_card_state)
    {
        if (NULL != custom_nw_evt_cb)
        {
            custom_nw_evt_cb(NW_EVENT_SIM_PRESENT, NULL, 0);
        }
    } else {
        if (NULL != custom_nw_evt_cb)
        {
            custom_nw_evt_cb(NW_EVENT_SIM_ABSENT, NULL, 0);
        }
    }

    return AIC_SUCCESS;
}

boolean aic_is_sim_card_plugged(void)
{
    return (reg_service_info.sim_card_state == AT_SIM_CARD_PLUGGED);
}

u32 aic_register_cereg_state_cb(cereg_update_cb cb)
{
    u8 i;

    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (!cereg_state_cbs[i]) {
            cereg_state_cbs[i] = cb;
            return AIC_SUCCESS;
        }
    }

    return AT_ERROR_URC_CBS_OVERFLOW;
}

void aic_unregister_cereg_state_cb(cereg_update_cb cb)
{
    u8 i;

    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (cereg_state_cbs[i] == cb) {
            cereg_state_cbs[i] = AIC_NULL;
            return;
        }
    }
}

static void notify_cereg_state_update(u8 cereg_state)
{
    u8 i;

    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (cereg_state_cbs[i]) {
            cereg_state_cbs[i](cereg_state);
        }
    }
}

static void set_cops_at_format_cb(u8 *cmd_resp, u8 line_count)
{
    if (!cmd_resp || strcasestr(cmd_resp, AT_RESULT_ERROR))
        return;

    aic_send_cmd_with_callback("AT+COPS?", AIC_NULL);
}

/**
 * Handle the +CEREG URC,which tells the registration state in ps domain.
 * The format is as follows:
 * +CEREG: <state>[,<tac>][,<ci>][,<Act>][,<cause_type>][,<reject_cause>]
 *
 * It is considered to be registered when the state isn't 0,2,3 or 4.
 * Try to get the plmn when UE is registered.
 */
u32 handle_at_urc_cereg(u8 *urc_str, u8 *start_str)
{
    u16 paras_cnt = 0;
    ulong *paras_arr;
    u8 reg_state;

    /* Get all paras after +CEREG: . */
    paras_arr = aic_split_paras_to_str_array(urc_str, start_str, &paras_cnt);
    if (paras_arr && paras_cnt >= 1) {
        reg_state = atoi((u8 *)paras_arr[0]);
        if (!(reg_state == 0 || reg_state == 2 || reg_state == 3 || reg_state == 4)) {
            /* Registered,get out of flight mode. */
            aic_set_flight_mode(ANET_OUT_OF_FLIGHT_MODE);
            if (reg_state != reg_service_info.ps_reg_state)
                aic_send_cmd_with_callback("AT+COPS=3,2", set_cops_at_format_cb);
        } else if (reg_state == 3) {
            if (aic_need_try_saved_apns())
                aos_set_event(event_pdp_mgr, AT_EVENT_TRY_ALL_APN, AOS_OR);
        }

        reg_service_info.ps_reg_state = reg_state;
#ifdef CONFIG_NET_SHARE_COMMON
        if(!aic_boot_mode_is_basic() && aic_is_ps_registered())
            aic_act_net_share_waiting_pdp();
#endif

        notify_cereg_state_update(reg_state);
    }

    aic_free_paras_mem(paras_arr, paras_cnt);

    return AIC_SUCCESS;
}

boolean aic_is_ps_registered(void)
{
    return (reg_service_info.ps_reg_state == 1 ||
            reg_service_info.ps_reg_state == 5);
}

extern s32 time_zone_in_quarters;
extern s32 current_time_zone;
/**
 * Handle the +CTZE URC,the format is +CTZE: <tz>,<dst>,[<time>]
 * tz is time zone,string type,its range is -48~+56,in quarters of an hour,
 * for example -09.
 * dst is daylight saving time,integer type,its range is 0~2.
 * time is string type,the format is "YYYY/MM/DD,hh:mm:ss".
 */
u32 handle_at_urc_ctze(u8 *urc_str, u8 *start_str)
{
    u16 paras_cnt = 0;
    ulong *paras_arr;
    u8 time_zone[8] = {'\0'};
    u8 ctze_time[60] = {'\0'};
    aos_systime civil_time;
    u32 status = AIC_SUCCESS;
    s32 ctze_time_zone = AIC_INVALID_TIME_ZONE;

    /* If CTZU mode is 0,then don't update time zone and time. */
    if (reg_service_info.ctzu_mode != AT_CTZU_DEF_MODE)
        return status;

    /* Get all paras after +CTZE: . */
    paras_arr = aic_split_paras_to_str_array(urc_str, start_str, &paras_cnt);
    if (paras_arr) {
        strcpy(time_zone, (u8 *)paras_arr[0]);
        /* Get the real time zone,the range is -12~12. */
        if (strlen(time_zone) > 0)
            ctze_time_zone = adjust_time_zone(time_zone);
        alog_warn("[CTZE] get time zone %d flag %d\n", ctze_time_zone, time_zone_set_flag);

        if (ctze_time_zone >= AIC_MIN_TIME_ZONE_QUARTERS && ctze_time_zone <= AIC_MAX_TIME_ZONE_QUARTERS) {
            if (!time_zone_set_flag) {
                time_zone_in_quarters = ctze_time_zone;
                current_time_zone = ctze_time_zone / 4;
            }
        } else {
            alog_warn("[CTZE] get time zone %d is wrong\n", ctze_time_zone);
        }

        /* Parameter <time> is optional. */
        if (paras_cnt == 3) {
            strncpy(ctze_time, (u8 *)paras_arr[2], (sizeof(ctze_time) - 1));
            aos_memset((u8 *)&civil_time, 0, sizeof(aos_systime));
            status = aic_parse_civil_time(ctze_time, &civil_time);
            if (!status) {
                if (strcasestr(urc_str, "+CTZE:")) {
                    alog_info("[CTZE] set sys time %s\n", ctze_time);
                    aos_set_systime(&civil_time);
                } else if (strcasestr(urc_str, "+CTZEU:")) {
                    /* The time in CTZEU URC is UTC time. */
                    aos_set_utctime(&civil_time);
                }
            }
        }
    }

    aic_free_paras_mem(paras_arr, paras_cnt);

    return status;
}

s32 adjust_time_zone(u8 *time_zone)
{
    u8 time_zone_modified[8] = {'\0'};
    s32 original_tz;

    strncpy(time_zone_modified, time_zone, (sizeof(time_zone_modified) - 1));
    /* time zone may be -08,+06,so delete the leading 0. */
    if (*(time_zone + 1) == '0') {
        time_zone_modified[1] = *(time_zone + 2);
        time_zone_modified[2] = '\0';
    }

    original_tz = atoi(time_zone_modified);

    return original_tz;
}

/* ^ACT: <act_type>
 * <act_type>: 0,LTE_FDD
 * <act_type>: 1,LTE_TDD
 */
u32 handle_at_urc_act(u8 *urc_str, u8 *start_str)
{
    u16 paras_cnt = 0;
    ulong *paras_arr;

#ifdef CONFIG_NX_NAT
    void aic_nat_update_win(u32 dl_win, u32 ul_win);
#endif
    paras_arr = aic_split_paras_to_str_array(urc_str, start_str, &paras_cnt);
    if (paras_cnt >= 1)
        reg_service_info.act_type = atoi((u8 *)paras_arr[0]);

#ifdef CONFIG_NX_NAT
    if (reg_service_info.act_type)
        aic_nat_update_win(256, 64);
    else
        aic_nat_update_win(256, 128);
#endif
    aic_free_paras_mem(paras_arr, paras_cnt);

    return AIC_SUCCESS;
}

u8 aic_get_reg_act_type(void)
{
    return reg_service_info.act_type;
}

#ifdef CONFIG_CUSTOMER_CM_AT
/* +CPIN: READY,then send AT+CFUN=1
 * +CPIN: SIM REMOVED,then send AT+CFUN=5
 */
u32 handle_at_urc_cpin(u8 *urc_str, u8 *start_str)
{
    if (urc_str) {
        if (strcasestr(urc_str, "+CPIN: READY") && aic_is_flight_mode())
            aic_send_cmd_with_callback("AT+CFUN=1", AIC_NULL);
    }

    return AIC_SUCCESS;
}

/* +CSCON: mode[,state[,access]] */
u32 handle_at_urc_cscon(u8 *urc_str, u8 *start_str)
{
    u8 mode = 0xff, state = 0xff, access = 0xff, i;
    u16 paras_cnt = 0;
    ulong *paras_arr;

    if (!urc_str)
        return AIC_SUCCESS;

    paras_arr = aic_split_paras_to_str_array(urc_str, start_str, &paras_cnt);
    if (paras_arr && paras_cnt >= 1) {
        mode = atoi((u8 *)paras_arr[0]);
        if (paras_cnt >= 2) {
            state = atoi((u8 *)paras_arr[1]);
            if (paras_cnt >= 3)
                access = atoi((u8 *)paras_arr[2]);
        }
    }

    aic_free_paras_mem(paras_arr, paras_cnt);
    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (cscon_mode_cbs[i])
            cscon_mode_cbs[i](mode, state, access);
    }

    return AIC_SUCCESS;
}

u32 aic_register_cscon_mode_cb(cscon_update_cb cb)
{
    u8 i;

    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (!cscon_mode_cbs[i]) {
            cscon_mode_cbs[i] = cb;
            return AIC_SUCCESS;
        }
    }

    return AT_ERROR_URC_CBS_OVERFLOW;
}
#endif

u32 handle_at_urc_ephystat(u8 *urc_str, u8 *start_str)
{
    u16 paras_cnt = 0;
    ulong *paras_arr;
    u8 ephy_state, i;

    paras_arr = aic_split_paras_to_str_array(urc_str, start_str, &paras_cnt);
    if (paras_cnt >= 1) {
        ephy_state = atoi((u8 *)paras_arr[0]);
        for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
            if (ephy_state_cbs[i])
                ephy_state_cbs[i](ephy_state);
        }
    }

    aic_free_paras_mem(paras_arr, paras_cnt);

    return AIC_SUCCESS;
}

u32 aic_register_ephystat_state_cb(ephystat_update_cb cb)
{
    u8 i;

    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (!ephy_state_cbs[i]) {
            ephy_state_cbs[i] = cb;
            return AIC_SUCCESS;
        }
    }

    return AT_ERROR_URC_CBS_OVERFLOW;
}

u32 handle_at_urc_cperr(u8 *urc_str, u8 *start_str)
{
    u16 paras_cnt = 0;
    ulong *paras_arr;
    u8 reason;

    paras_arr = aic_split_paras_to_str_array(urc_str, start_str, &paras_cnt);
    if (paras_cnt >= 1) {
        reason = atoi((u8 *)paras_arr[0]);
        aic_free_paras_mem(paras_arr, paras_cnt);
#ifdef CONFIG_AIC_SYSOFF
        if (reason == 1)
            aos_create_thread_simply("reset_sys", sysoff_reset, AIC_NULL, SIZE_1K, 8);
#endif
    }

    return AIC_SUCCESS;
}

