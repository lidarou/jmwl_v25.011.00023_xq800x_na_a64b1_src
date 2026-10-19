/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handle Response of AT Cmd About Network
 *  This file handles the AT response of cmds relating to network,
 *  such as AT+CGACT,AT+CGDATA,AT+CGDCONT etc.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1a1193
#define LOG_MODULE    AMOD_AT

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include "aic_at_resp_handle.h"
#include "aic_net_at_resp_handle.h"
#include "aic_at_cmd_handle.h"
#include "aic_at_parse_utils.h"
#include "aic_at_interact_cp.h"
#include "aic_boot_mode.h"
#ifdef CONFIG_PDP_MGR
#include "aic_net_pdp_mgr.h"
#endif

#ifdef CONFIG_NET_SHARE_COMMON
#include "aic_net_share_common.h"
#endif

#ifdef CONFIG_USB_NET_SHARE
#include "aic_usb_net_share_mgr.h"
#endif

#ifdef CONFIG_PPP_DIAL
#include "aic_ppp_dial_mgr.h"
#endif

/* Define external vars. */
extern REG_SERVICE_INFO reg_service_info;
extern aic_flight_mode_change_cb flight_mode_cbs[AT_MAX_FLIGHT_MODE_CBS];


/* In order to get the plmn via cmd AT+COPS?. */
s32 aic_handle_cops_query_resp(AT_REQ_RESP *at_to_ap)
{
    u16  paras_count;
    u8 line_cnt;
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;

    line_cnt = at_to_ap->resp_line_count;
    if (at_to_ap->is_external_cmd == AT_FROM_EXTERNAL ||
        strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR) || line_cnt < 2) {
        return AIC_SUCCESS;
    }

    line_arr = aic_get_each_line_from_resp(at_to_ap->at_resp, line_cnt);
    if (!line_arr)
        return AIC_SUCCESS;

    para_arr = aic_split_paras_to_str_array((u8 *)line_arr[0],
                                            "+COPS: ",
                                            &paras_count);

    if (paras_count >= 3) {
        /* The third para is plmn. */
        if (strlen((u8 *)para_arr[2]) > 0) {
            if (strncasecmp(reg_service_info.operator_plmn, (u8 *)para_arr[2],
                            ANET_PLMN_MAX_LEN) != 0) {
                strncpy(reg_service_info.operator_plmn, (u8 *)para_arr[2],
                        (ANET_PLMN_MAX_LEN - 1));
            }
        }
    }

    aic_free_paras_mem(line_arr, line_cnt);
    aic_free_paras_mem(para_arr, paras_count);

    return AIC_SUCCESS;
}

static void notify_flight_mode_change(u8 flight_mode)
{
    u8 i;

    for (i = 0; i < AT_MAX_FLIGHT_MODE_CBS; i++) {
        if (flight_mode_cbs[i])
            flight_mode_cbs[i](flight_mode);
    }
}

static void handle_cfun_cmd(u8 *at_cmd_str)
{
    if (!at_cmd_str)
        return;

    if (aic_is_cfun_cmd(at_cmd_str, AT_STOP_CP) ||
        aic_is_cfun_cmd(at_cmd_str, AT_SWITCH_OFF_PS)) {
        aic_enter_flight_mode();
        aic_send_cmd_with_callback("AT+MIMS?", aic_at_mims_query_callback);
        notify_flight_mode_change(AT_STOP_CP);
    } else if (aic_is_cfun_cmd(at_cmd_str, AT_START_CP)) {
        aic_set_flight_mode(ANET_OUT_OF_FLIGHT_MODE);
        notify_flight_mode_change(AT_START_CP);
    }
}

s32 aic_handle_cfun_exe_resp(AT_REQ_RESP *at_to_cp)
{
    if (!at_to_cp || !at_to_cp->at_req || !at_to_cp->at_resp ||
        strcasestr(at_to_cp->at_resp, AT_RESULT_ERROR))
        return AIC_SUCCESS;

    handle_cfun_cmd(at_to_cp->at_req);

    return AIC_SUCCESS;
}

#ifdef CONFIG_CUSTOMER_CM_AT
s32 aic_handle_ctzr_set_resp(AT_REQ_RESP *at_to_ap)
{
    u16 paras_count = 0;
    ulong *paras_arr = AIC_NULL;

    if (at_to_ap->is_external_cmd == AT_FROM_INTERNAL)
        return AIC_SUCCESS;

    if (at_to_ap->at_req && !strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR)) {
        paras_arr = aic_split_paras_to_str_array(at_to_ap->at_req,
                                                 "AT+CTZR=",
                                                 &paras_count);
        if (paras_count >= 1)
            reg_service_info.ctzr_mode = atoi((u8 *)paras_arr[0]);

        aic_free_paras_mem(paras_arr, paras_count);
    }

    return AIC_SUCCESS;
}

s32 aic_handle_ctzr_query_resp(AT_REQ_RESP *at_to_ap)
{
    u8 line_cnt;
    u8 *tmp_ptr;

    line_cnt = at_to_ap->resp_line_count;
    if (reg_service_info.ctzr_mode != 0 || line_cnt <= 1)
        return AIC_SUCCESS;

    if (at_to_ap->at_req && !strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR)) {
        tmp_ptr = strcasestr(at_to_ap->at_resp, "+CTZR: ");
        if (tmp_ptr) {
            tmp_ptr += strlen("+CTZR: ");
            *tmp_ptr = '0';
        }
    }

    return AIC_SUCCESS;
}

void aic_set_cereg_mode(u8 cereg_mode)
{
    reg_service_info.cereg_mode = cereg_mode;
}

s32 aic_handle_cereg_set_resp(AT_REQ_RESP *at_to_ap)
{
    u16 paras_count = 0;
    ulong *paras_arr = AIC_NULL;
    NET_CFG net_cfg;

    if (at_to_ap->is_external_cmd == AT_FROM_INTERNAL)
        return AIC_SUCCESS;

    if (at_to_ap->at_req && !strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR)) {
        paras_arr = aic_split_paras_to_str_array(at_to_ap->at_req,
                                                 "AT+CEREG=",
                                                 &paras_count);
        if (paras_count >= 1) {
            reg_service_info.cereg_mode = atoi((u8 *)paras_arr[0]);
            aos_memset(&net_cfg, 0, sizeof(NET_CFG));
            aic_read_net_config(&net_cfg);
            net_cfg.cereg_mode = reg_service_info.cereg_mode;
            aic_save_config_to_nv(&net_cfg);
        }

        aic_free_paras_mem(paras_arr, paras_count);
    }

    return AIC_SUCCESS;
}

s32 aic_handle_cereg_query_resp(AT_REQ_RESP *at_to_ap)
{
    u8 line_cnt, para_cnt = 0, i;
    u8 *tmp_ptr;

    line_cnt = at_to_ap->resp_line_count;
    if (reg_service_info.cereg_mode != 0 || line_cnt <= 1)
        return AIC_SUCCESS;

    if (at_to_ap->at_req && !strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR)) {
        tmp_ptr = strcasestr(at_to_ap->at_resp, "+CEREG: ");
        if (tmp_ptr) {
            tmp_ptr += strlen("+CEREG: ");
            *tmp_ptr = '0';
        }

        /* There are 7 paras at most if cereg_mode is 0. */
        tmp_ptr++;
        while (*tmp_ptr) {
            if (*tmp_ptr == ',') {
                para_cnt++;
                if (para_cnt == 7) {
                    if (*(tmp_ptr - 1) == ',' && *(tmp_ptr - 2) == ',')
                        tmp_ptr -= 2;

                    for (i = 0; i < 2; i++) {
                        *tmp_ptr = aic_get_cr_sign();
                        tmp_ptr++;
                        *tmp_ptr = aic_get_lf_sign();
                        tmp_ptr++;
                    }

                    *tmp_ptr = 'O';
                    tmp_ptr++;
                    *tmp_ptr = 'K';
                    tmp_ptr++;
                    *tmp_ptr = aic_get_cr_sign();
                    tmp_ptr++;
                    *tmp_ptr = aic_get_lf_sign();
                    tmp_ptr++;
                    *tmp_ptr = 0x00;
                    at_to_ap->current_resp_len = (tmp_ptr - at_to_ap->at_resp);
                    break;
                }
            }

            tmp_ptr++;
            continue;
        }
    }

    return AIC_SUCCESS;
}

s32 aic_handle_cgerep_set_resp(AT_REQ_RESP *at_to_ap)
{
    u16 paras_count = 0;
    ulong *paras_arr = AIC_NULL;

    if (at_to_ap->is_external_cmd == AT_FROM_INTERNAL)
        return AIC_SUCCESS;

    if (at_to_ap->at_req && !strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR)) {
        paras_arr = aic_split_paras_to_str_array(at_to_ap->at_req,
                                                 "AT+CGEREP=",
                                                 &paras_count);
        if (paras_count >= 1)
            reg_service_info.cgerep_mode = atoi((u8 *)paras_arr[0]);

        aic_free_paras_mem(paras_arr, paras_count);
    }

    return AIC_SUCCESS;
}

s32 aic_handle_cgerep_query_resp(AT_REQ_RESP *at_to_ap)
{
    u8 line_cnt;
    u8 *tmp_ptr;

    line_cnt = at_to_ap->resp_line_count;
    if (reg_service_info.cgerep_mode != 0 || line_cnt <= 1)
        return AIC_SUCCESS;

    if (at_to_ap->at_req && !strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR)) {
        tmp_ptr = strcasestr(at_to_ap->at_resp, "+CGEREP: ");
        if (tmp_ptr) {
            tmp_ptr += strlen("+CGEREP: ");
            *tmp_ptr = '0';
        }
    }

    return AIC_SUCCESS;
}
#endif

