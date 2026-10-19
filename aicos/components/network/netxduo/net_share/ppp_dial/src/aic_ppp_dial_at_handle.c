/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to PPP Dial
 *  This is the handler of AT commands relating to PPP dial.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1900f7
#define LOG_MODULE    AMOD_NET

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <nx_api.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <connmgr_api.h>
#include <aic_md_id.h>
#include <aic_net_api.h>
#include "aic_at_cmd_handle.h"
#include "aic_general_at_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_at_parse_utils.h"
#include "aic_at_interact_cp.h"
#include "aic_net_pdp_mgr.h"
#include "aic_ppp_dial_at_handle.h"
#include "aic_net_share_common.h"
#include "aic_ppp_dial_mgr.h"

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
#include "aic_net_share_port_mgr.h"
#endif

#ifdef CONFIG_NET_PPP_CODEC
#include <aic_ppp_codec_core.h>
#endif
#ifdef CONFIG_MISC_SUPPORT
#include <aic_md_api.h>
#endif

AT_CMD_HANDLE ppp_dial_at_handles[] =
{
    {"AT^PPPSHARE", aic_handle_pppshare_test_cmd, aic_handle_pppshare_query_cmd,
     aic_handle_pppshare_set_cmd, AIC_NULL},

    {"ATH", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_ath_exe_cmd},

    {AIC_NULL},
};

AT_CMD_HANDLE ppp_atd_handles[] =
{
    /** ATD cmd for voice dial will be sent to CP. ATD(T)*99***1#,ATD(T)*98*1#,
     * ATD(T)*99# and ATD(T)#777 are all used for PPP dial.
     */
    {"*99***", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_atd_exe_cmd},

    {AIC_NULL},
};

/* Define external vars. */
extern PPP_DIAL_NET_INFO ppp_dial_net_info;
extern aos_mutex_ptr ppp_dial_net_mutex;
extern REG_SERVICE_INFO reg_service_info;
extern aos_event_ptr event_net_share;

/** Handle cmd:AT^PPPSHARE=cid,share_mode,pdp_type,apn
 * For example: AT^PPPSHARE=cid,1,"IPV4V6","cmnet"
 */
u32 aic_handle_pppshare_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    //u8 result_str[1][ANET_REPLY_MSG_MAX_SIZE] = {'\0'};
    //u8 port_range[20] = {'\0'};
    ulong *paras_arr;
    u16 paras_count;
    u8 dial_mode = ANET_SHARE_MODE_FORWARDING;
    u8 cid = ANET_DEFAULT_PDP_CID, error_num = 0;
    u32 ret = AIC_SUCCESS;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT^PPPSHARE=",
                                             &paras_count);
    if (!paras_arr || paras_count < 1 || paras_count > 4) {
        aicx_error(0x1a5b36, "", "get pppshare paras error\n");
        error_num++;
        goto PARA_ERROR;
    }

    aic_reset_ppp_dial_net_info();

    if (paras_arr[0] && strlen((u8 *)paras_arr[0]) > 0) {
        cid = atoi((u8 *)paras_arr[0]);
    }
    if (!aic_is_valid_pdp_cid(cid)) {
        error_num++;
        goto PARA_ERROR;
    }

    if (paras_count >= 2) {
        dial_mode = atoi((u8 *)paras_arr[1]);
        if (dial_mode != ANET_SHARE_MODE_FORWARDING &&
            dial_mode != ANET_SHARE_MODE_ROUTING) {
            error_num++;
            goto PARA_ERROR;
        }
    }

    if (paras_count >= 3) {
        if (strlen((u8 *)paras_arr[2]) != 0 &&
            !aic_is_valid_pdp_type((u8 *)paras_arr[2])) {
            error_num++;
            goto PARA_ERROR;
        }

        strncpy(ppp_dial_net_info.pdp_type, (u8 *)paras_arr[2],
                (ANET_PDP_TYPE_MAX_LEN - 1));
        if (paras_count >= 4) {
            if (strlen((u8 *)paras_arr[3]) >= ANET_APN_NAME_MAX_LEN) {
                error_num++;
                goto PARA_ERROR;
            }
            strncpy(ppp_dial_net_info.apn, (u8 *)paras_arr[3],
                    (ANET_APN_NAME_MAX_LEN - 1));
        } else {
            strncpy(ppp_dial_net_info.apn, reg_service_info.default_apn_name,
                    (ANET_APN_NAME_MAX_LEN - 1));
        }
    } else {
        strncpy(ppp_dial_net_info.pdp_type, ANET_PDP_TYPE_IPV4V6,
                (ANET_PDP_TYPE_MAX_LEN - 1));
    }

    /* port range */
    /*if (paras_count == 3) {
        strcpy(port_range, (u8 *)paras_arr[2]);
        ret = check_port_range(port_range, &start_port, &end_port);
        if (ret) {
            error_num++;
            //goto PARA_ERROR;
        }
    }*/

    if (error_num == 0) {
        aos_get_mutex(ppp_dial_net_mutex, AOS_WAIT_FOREVER);
        ppp_dial_net_info.wan_net_id = cid;
        ppp_dial_net_info.ppp_dial_mode = dial_mode;
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
        ppp_dial_net_info.port_range_start = ANET_PPP_DIAL_PORT_START;
        ppp_dial_net_info.port_range_end = ANET_PPP_DIAL_PORT_END;
#endif
        aos_put_mutex(ppp_dial_net_mutex);

        if (!aic_is_pdp_activated(cid)) {
            aic_send_cmd_cgdcont(cid, ppp_dial_net_info.pdp_type,
                                 ppp_dial_net_info.apn);
        }
        strcpy(result_line, AT_RESULT_OK);
    }

PARA_ERROR:
    if (error_num > 0) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", AT_ERROR_INVALID_PARAS);
        ret = AT_ERROR_INVALID_PARAS;
    }

    aic_free_paras_mem(paras_arr, paras_count);
    /* Fill response,one line. */
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    return ret;
}

/** The reply is as follows:
 * ^PPPSHARE=<cids>,<dial modes>,<pdp_types>
 * OK
 */
u32 aic_handle_pppshare_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
            "^PPPSHARE: (1-%d),(1-2),(\"IPV4V6\",\"IP\",\"IPV6\")",
            ANET_MAX_PDP_CID);

    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 aic_handle_pppshare_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    //u8 cid_str[4] = {'\0'};
    //u8 port_range[20] = {'\0'};
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, sizeof(result_line0),
             "^PPPSHARE: %d,%d,\"", ppp_dial_net_info.wan_net_id,
             ppp_dial_net_info.ppp_dial_mode);

    strcat(result_line0, ppp_dial_net_info.pdp_type);
    strcat(result_line0, "\",\"");
    strncat(result_line0, ppp_dial_net_info.apn,
            (AT_MAX_AP_RESP_LENGTH - strlen(result_line0) - 2));
    strcat(result_line0, "\"");

    strcpy(result_line1, AT_RESULT_OK);

    /*if (ppp_dial_net_info.port_range_start > 0) {
        sprintf(port_range, ",\"%d~%d\"",
                ppp_dial_net_info.port_range_start,
                ppp_dial_net_info.port_range_end);
        strcat(result_str[0], port_range);
    }*/

    /* Fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

void aic_ppp_dial_wait_pdp(u8 cid)
{
    WAN_NET *wan_net_ptr;
    u32 waiting_interval = 200, wait_total_time = 0;
    u16 wait_count = 0;
    boolean pdp_act_sent = AIC_FALSE;

    if ((aic_is_flight_mode() || !aic_is_sim_card_plugged()) &&
        !aic_is_ps_registered()) {
        aicx_info(0x09943d, "", "no need to wait pdp act for ppp dial\n");
        return;
    }

    wan_net_ptr = aic_get_wan_net_per_pdp(cid);
    while (!(wan_net_ptr && wan_net_ptr->wan_ip_instance)) {
        if (!wan_net_ptr)
            wan_net_ptr = aic_get_wan_net_per_pdp(cid);

        if (aic_is_ps_registered() && !pdp_act_sent) {
            aic_enable_data_connection(ppp_dial_net_info.wan_net_id,
                                       ppp_dial_net_info.pdp_type,
                                       ppp_dial_net_info.apn);

            pdp_act_sent = AIC_TRUE;
        }

        aos_sleep(waiting_interval);
        wait_count++;
        wait_total_time += waiting_interval;
        if (wait_count == 10)
            waiting_interval = 500;

        if (wait_count > 10 && (wait_count % 2 == 0))
            aicx_info(0x07e48b, "d", "ppp dial wait %d times for PDP activation\n", wait_count);

        /* If the activating or attach cmd has received error,then wait 15s. */
        if ((!ppp_dial_net_info.waiting_pdp_act &&
            wait_total_time >= AT_TIMEOUT_FIFTEEN_SECONDS) ||
            wait_total_time >= AT_TIMEOUT_TWENTY_SECONDS)
            return;
    }
}

/* ATD*99***cid# or ATDT*99***cid# */
u32 aic_handle_atd_exe_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 cid = 0xff;
    u8 at_cmd[20] = {'\0'};
    u8 *tmp_ptr;
    u32 status, serial_port_id;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    /* The cmd is special,get cid seperately. */
    strncpy(at_cmd, at_req_resp_ptr->at_req, (sizeof(at_cmd) - 1));
    if (strstr(at_cmd, "*99#") || strstr(at_cmd, "#777")) {
        cid = ANET_DEFAULT_PDP_CID;
    } else {
        tmp_ptr = strcasestr(at_cmd, "***");
        if (tmp_ptr) {
            tmp_ptr += 3;
            cid = atoi(tmp_ptr);
        } else {
            tmp_ptr = strstr(at_cmd, "*98*");
            if (tmp_ptr) {
                tmp_ptr += 4;
                cid = atoi(tmp_ptr);
            }
        }
    }

    serial_port_id = at_req_resp_ptr->serial_port_id;

    if (aic_is_valid_pdp_cid(cid)) {
        status = aic_ppp_dial_start(serial_port_id, cid);
        if (status) {
            if (ppp_dial_net_info.dial_started == ANET_PPP_DIAL_STARTED &&
                !aic_is_pdp_activated(cid)) {
                aic_ppp_dial_stop_half_way(cid);
            }
            strcpy(result_line, ANET_PPP_DIAL_ERROR_RESULT);
        } else {
            strncpy(result_line, AT_RESULT_CONNECT,
                    (AT_MAX_AP_RESP_LENGTH - 1));

            /* Wait for PDP activating if it is deactivated.*/
            if (!aic_is_pdp_activated(cid)) {
                ppp_dial_net_info.waiting_pdp_act = ANET_WAIT_PDP_ACTIVATE;
                aic_ppp_dial_wait_pdp(ppp_dial_net_info.wan_net_id);
            }

            /* PDP is still not activated after waiting. */
            if (!aic_is_pdp_activated(cid)) {
                /* Failed to activate the PDP. */
                aic_ppp_dial_stop_half_way(cid);
                strcpy(result_line, ANET_PPP_DIAL_ERROR_RESULT);
                status = ANET_ERROR_PDP_ACTIVATE_FAIL;
            } else {
                status = aic_ppp_dial_pdp_activated(cid);
                if (status) {
                    aos_set_event(event_net_share,
                                  NET_EVENT_STOP_PPP_DIAL,
                                  AOS_OR);
                    strcpy(result_line, ANET_PPP_DIAL_ERROR_RESULT);
                }
            }
        }
    } else {
        strcpy(result_line, ANET_PPP_DIAL_ERROR_RESULT);
        status = AT_ERROR_INVALID_PARAS;
    }

    /* Fill response,one line. */
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    /* Make serial port enter data mode. */
    if (!status) {
        /* Call PPP codec API in the same task. */
        aos_set_event(event_net_share, NET_EVENT_PPP_DATA_MODE, AOS_OR);
    }

    return status;
}

/* ATH is used to stop PPP disl. */
u32 aic_handle_ath_exe_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    /* ATH is also handled by CP. */
    aic_send_at_to_cp(at_req_resp_ptr);

    aos_set_event(event_net_share, NET_EVENT_STOP_PPP_DIAL, AOS_OR);

    return AIC_SUCCESS;
}

#if 0
u32 check_port_range(u8 *str_port_range, u32 *start_port, u32 *end_port)
{
    u32 status = AIC_SUCCESS;
    u8 port0[10] = {0};
    u8 port1[10] = {0};
    u16 i;

    if (str_port_range) {
        for (i = 0; i < strlen(str_port_range); i++) {
            if (*(str_port_range + i) == '~') {
                memcpy(port0, str_port_range, i);
                memcpy(port1, (str_port_range + i + 1),
                       (strlen(str_port_range) - i -1));
                *start_port = atoi(port0);
                *end_port = atoi(port1);
                break;
            } else {
                continue;
            }
        }
    } else {
        *start_port = 0;
        *end_port = 0;
    }

    if (*start_port == 0 || *end_port == 0) {
        status = AT_ERROR_INVALID_PARAS;
    }

    return status;
}
#endif

AT_CMD_HANDLE *aic_get_ppp_dial_cmd_handle(void)
{
    return &ppp_atd_handles[0];
}

