/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to Network Share
 *  This is the handler of AT commands relating to network share,
 *  such as starting usb share,PPP dial etc.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x058e5b
#define LOG_MODULE    AMOD_AT

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_init.h>
#include <aic_osa.h>
#ifdef WIN32
#include <aic_osa_ex.h>
#endif
#include <aic_errno.h>
#include <aic_size.h>
#include <aic_boot_mode.h>
#include <connmgr_api.h>
#include <aic_version.h>
#ifndef WIN32
#include <gpio_inc.h>
#endif
#include <SN.h>
#include <sysoff.h>
#include "aic_at_cmd_handle.h"
#include "aic_general_at_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_at_parse_utils.h"
#include "aic_at_interact_cp.h"
#include "aic_manufacturer_info.h"
#include "aic_model_info.h"
#include "aic_net_cfg.h"
#if CONFIG_GX_AT_V1 || CONFIG_GX_AT_V2 || CONFIG_CUSTOMER_AT
#include "aic_at_tcpip_common.h"
#endif
#ifdef CONFIG_USB_NET_SHARE
#include "aic_usb_net_share_mgr.h"
#endif

#ifdef CONFIG_USB_SERVICE
#include <usb_service.h>
#endif

#ifdef CONFIG_MISC_SUPPORT
#include <aic_md_id.h>
#include <aic_md_api.h>
#endif

#ifdef CONFIG_NET_PCAP_DUMP
#include <aic_network_pcap.h>
#endif

#ifdef CONFIG_PDP_MGR
#include "aic_net_pdp_mgr.h"
#endif

#ifdef CONFIG_NET_TRAFFIC_CONTROL
#include <aic_net_tc.h>
#endif

#ifdef CONFIG_NET_EXTERNEL_API
#include "aic_net_api.h"
#endif

#ifdef CONFIG_CMUX
#include <cmux.h>
#endif

/* Define global vars. */
DATA_SP_CTRL data_sp_ctrl[AT_DATA_SPEED_CTRL_NUM];

static URC_CFG urc_cfg;
#ifndef CONFIG_AT_PROC_MINI
static MSG_TEXT msg_text;
static FASTSLP_CFG fastslp_cfg;
static u8 at_echo_enabled = AT_CMD_ECHO_DISABLED;
#endif
s32 cclk_time_zone = AIC_DEFAULT_TIME_ZONE_QUARTERS;
s32 current_time_zone = AIC_DEFAULT_TIME_ZONE;
s32 time_zone_in_quarters = AIC_DEFAULT_TIME_ZONE_QUARTERS;

#ifdef CONFIG_USB_SERVICE
u8 current_usb_mode = LOG_AT_DIAG;
u8 old_usb_mode = LOG_AT_DIAG;
#endif
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
MIPCALL_CFG mipcall_cfg[ANET_PDP_MAX_NUM];
#endif
#ifdef CONFIG_D2D
D2D_MODE_CFG d2d_mode_cfg;
#endif

extern u8 time_zone_set_flag;
extern aos_mutex_ptr pdp_info_mutex;
extern REG_SERVICE_INFO reg_service_info;
#ifdef CONFIG_PDP_MGR
extern NET_PDP_INFO net_pdp_info_list[ANET_PDP_MAX_NUM];
#endif
extern INIT_NET_CFG init_net_cfg;

#ifdef CONFIG_CUSTOMER_AT
static u8 get_pdp_type_from_cgdcont(u8 *resp_ptr, u8 line_cnt,
                                    u8 pdp_cid, u8 *apn_str);
static void handle_qicsgp_query_cmd(u8 cid, AT_REQ_RESP *at_req_ptr);
static u8 get_auth_config_from_cgauth(u8 *resp_ptr, u8 line_cnt, u8 pdp_cid,
                                      u8 *user_name, u8 *pwd_ptr);
#endif
extern u32 aic_handle_send_msg_cmd(u32 at_port, s8 *message, u32 msg_size);

#ifdef CONFIG_USB_SERVICE
/** Set usb work mode,the default mode is LOG_AT_DIAG.
 * 0: LOG_AT_DIAG,
 * 1: RNDIS+LOG_AT_DIAG,
 * 2: ECM+LOG_AT_DIAG,
 * 3: MSC+LOG_AT_DIAG,
*/
u32 aic_handle_usbcfg_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    s32 ret;
    u8 usb_mode;
    u32 error_code;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT^USBCFG=", &paras_count);
    if (!paras_arr || paras_count != 1 || strlen((u8 *)paras_arr[0]) != 1){
        error_code = AT_ERROR_INVALID_PARAS;
        goto PARA_ERROR;
    }

    /* The parameter must be 0,1,2 or 3. */
    if (!(*(u8 *)paras_arr[0] >= 0x30 && *(u8 *)paras_arr[0] <= 0x33)) {
        error_code = AT_ERROR_INVALID_PARAS;
        goto PARA_ERROR;
    }

    /* basic mode can't support usb. */
    if (aic_boot_mode_is_basic()) {
        error_code = AT_ERROR_INVALID_PARAS;
        goto PARA_ERROR;
    }

    usb_mode = atoi((u8 *)paras_arr[0]);
    ret = usb_service_enum_manage(usb_mode);
    if (!ret) {
        if (usb_mode != current_usb_mode) {
            old_usb_mode = current_usb_mode;
            current_usb_mode = usb_mode;
        }
    } else {
        aicx_error(0x14a464, "x", "usbcfg set error 0x%x\n", ret);
        if (usb_mode == MSC)
            error_code = AT_ERROR_PARA_NOT_SUPPORT;
        else
            error_code = AT_ERROR_HANDLE_FAILURE;
        goto PARA_ERROR;
    }

    aic_free_paras_mem(paras_arr, paras_count);
    /* Wait a while for USB restarting. */
    aos_sleep(2000);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, error_code);

    return AIC_SUCCESS;
}

u32 aic_handle_usbcfg_query_cmd(AT_REQ_RESP *at_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "^USBCFG: %d", current_usb_mode);
    aic_transmit_at_result(at_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_usbcfg_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    strncpy(result_line0, "^USBCFG: (0~3)", (AT_MAX_AP_RESP_LENGTH - 1));
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}
#endif

boolean aic_is_usb_in_net_mode(void)
{
#ifdef CONFIG_USB_SERVICE
    return (current_usb_mode == RNDIS || current_usb_mode == ECM);
#else
    return AIC_FALSE;
#endif
}

#ifndef CONFIG_AT_PROC_MINI
/** Enable or disable the flow control of UART port.
 * AT+IFC=[<by_te>[,<by_ta>]]
 * The set and query cmd can only be applied to UART port.
 * If it is an USB serial port,then return error.
 */
u32 aic_handle_ifc_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr = AIC_NULL;
    u8 by_te = 0, by_ta = 0;
    boolean enabled = AIC_FALSE;
    s32 ret = AIC_SUCCESS;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+IFC=", &paras_count);
    if (paras_count != 2)
        goto PARA_ERROR;

    by_te = atoi((u8 *)paras_arr[0]);
    by_ta = atoi((u8 *)paras_arr[1]);
    /* by_te should be equal to by_ta, and it should be enabled via uart at port. */
    if (by_te != by_ta || (by_te != 0 && by_te != 2) || (by_te == 2 &&
        at_req_res_ptr->serial_port_id != conn_get_uart_at_conid))
        goto PARA_ERROR;

    /* Enable or disable the serial port flow control. */
    enabled = by_te == 0 ? AIC_FALSE : AIC_TRUE;
    ret = conn_cmd_fctrl_set(CONN_CMD_AT, conn_get_uart_at_conid, enabled);
    /* Maybe not UART port. */
    if (ret < 0)
        goto CMD_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_res_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);
    return AIC_SUCCESS;

CMD_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_HANDLE_FAILURE);
    return AIC_SUCCESS;
}

u32 aic_handle_ifc_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 control_by_te = 0, control_by_ta = 0;
    u32 enabled;
    s32 ret;

    /* Get if flow control is enabled. */
    ret = conn_cmd_fctrl_get(CONN_CMD_AT, conn_get_uart_at_conid, &enabled);
    if (ret < 0) {
        goto CMD_ERROR;
    } else {
        control_by_te = enabled ? 2 : 0;
        control_by_ta = control_by_te;
        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
                 "+IFC: %d,%d", control_by_te, control_by_ta);
    }

    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

CMD_ERROR:
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

u32 aic_handle_ifc_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    strncpy(result_line0, "+IFC: (0,2),(0,2)", (AT_MAX_AP_RESP_LENGTH - 1));
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

/** Set the baud rate of the UART. AT+IPR=<rate>
 * The set and query cmd can only be applied to UART port.
 * If it is an USB serial port,then return error.
 */
u32 aic_handle_ipr_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u32 baud_rate;
    u8 ok_str[8] = {0};
#ifdef CONFIG_D2D
    u8 *resave_ptr;
    u32 len;
#endif

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+IPR=", &paras_count);
    if (paras_count != 1)
        goto PARA_ERROR;

    baud_rate = atoi((u8 *)paras_arr[0]);
#ifdef CONFIG_CUSTOMER_CM_AT
    if (!(baud_rate == 300 || baud_rate == 1200 ||baud_rate == 4800 || baud_rate == 9600
        || baud_rate == 14400 || baud_rate == 19200 || baud_rate == 28800 || baud_rate == 38400
        || baud_rate == 57600 || baud_rate == 115200 || baud_rate == 230400 || baud_rate == 460800
        || baud_rate == 921600)) {
        goto PARA_ERROR;
    }
#else
    if (!(baud_rate == 300 || baud_rate == 600 || baud_rate == 1200
        || baud_rate == 2400 || baud_rate == 4800 || baud_rate == 9600
        || baud_rate == 14400 || baud_rate == 19200 || baud_rate == 28800
        || baud_rate == 38400 || baud_rate == 57600
        || baud_rate == 115200 || baud_rate == 128000 || baud_rate == 230400
        || baud_rate == 256000 || baud_rate == 460800 || baud_rate == 500000
        || baud_rate == 512000 || baud_rate == 600000 || baud_rate == 750000
        || baud_rate == 921600 || baud_rate == 1000000 || baud_rate == 2000000
        || baud_rate == 3686400 || baud_rate == 3250000)) {
        goto PARA_ERROR;
    }
#endif

    aic_free_paras_mem(paras_arr, paras_count);
#ifdef CONFIG_CUSTOMER_AT
    strcpy(ok_str, AT_RESULT_OK);
    ok_str[2] = aic_get_cr_sign();
    ok_str[3] = aic_get_lf_sign();
#else
    fill_head_and_end_signs(ok_str, AT_RESULT_OK, 2);
#endif

#ifdef CONFIG_D2D
    if (aic_at_with_msg_head(at_req_res_ptr)) {
        len = strlen(ok_str);
        resave_ptr = aos_zalloc(len + AT_MSG_HEAD_LEN);
        aic_at_prepend_msg_head(resave_ptr, at_req_res_ptr, len);
        aos_memcpy((resave_ptr + AT_MSG_HEAD_LEN), ok_str, len);
        aic_send_to_at_port(at_req_res_ptr->serial_port_id, resave_ptr, (len + AT_MSG_HEAD_LEN));
        aos_free(resave_ptr);
    } else
#endif

    if (at_req_res_ptr->serial_port_id != AT_DIAG_SERIAL_PORT_ID)
        aic_send_to_at_port(at_req_res_ptr->serial_port_id, ok_str, strlen(ok_str));
    else
        aic_reply_to_certain_com(at_req_res_ptr->serial_port_id, ok_str, strlen(ok_str));

    aos_sleep(5);
#ifdef CONFIG_CUSTOMER_CM_AT
    conn_cmd_set_baud_rate(CONN_CMD_AT, conn_get_uart_at_conid, baud_rate, AIC_TRUE);
#else
#ifndef CONFIG_CMUX
    conn_cmd_set_baud_rate(CONN_CMD_AT, conn_get_uart_at_conid, baud_rate, AIC_FALSE);
#else
    aic_cmux_set_realdev_baudrate(baud_rate);
#endif
#endif

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);
    return AIC_SUCCESS;
}

u32 aic_handle_ipr_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    s32 baud_rate;

    /* get the baud rate. */
#ifndef CONFIG_CMUX
    baud_rate = conn_cmd_get_baud_rate(CONN_CMD_AT, conn_get_uart_at_conid);
#else
    baud_rate = aic_cmux_get_realdev_baudrate();
#endif
    if (baud_rate < 0)
        goto CMD_ERROR;

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+IPR: %d", baud_rate);
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

CMD_ERROR:
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

u32 aic_handle_ipr_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH + 40] = {'\0'};

#ifdef CONFIG_CUSTOMER_CM_AT
    strncpy(result_line0, "+IPR: (0),(300,1200,4800,9600,14400,19200,28800,38400,",
            (AT_MAX_AP_RESP_LENGTH - 1));
    strncat(result_line0, "57600,115200,230400,460800,921600)",
            (AT_MAX_AP_RESP_LENGTH - 1 - strlen(result_line0)));
#else
    strncpy(result_line0, "+IPR: (300,600,1200,2400,4800,9600,14400,19200,28800,",
            (AT_MAX_AP_RESP_LENGTH + 40 - 1));
    strncat(result_line0, "38400,57600,115200,128000,230400,256000,460800,500000,",
            (AT_MAX_AP_RESP_LENGTH + 40  - 1 - strlen(result_line0)));
    strncat(result_line0, "512000,600000,750000,921600,1000000,2000000,3686400)",
            (AT_MAX_AP_RESP_LENGTH + 40  - 1 - strlen(result_line0)));
#endif

    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

/* AT+CGMI: get manufacturer identification.No parameter. */
u32 aic_handle_cgmi_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
#ifdef CONFIG_MISC_SUPPORT
    u8 len;
    s32 ret = AIC_SUCCESS;
#endif

    /* The cmd has no parameters. */
    if (strchr(at_req_res_ptr->at_req, '='))
        goto PARA_ERROR;

    /** Get manufacturer identification from json file in misc data.
     * Return manufacturer identification without any prefix.
     */
#ifdef CONFIG_MISC_SUPPORT
    len = aic_md_get_length(MD_ID_MANUFACTURER_INFO);
    len = len >= AT_MAX_AP_RESP_LENGTH ? (AT_MAX_AP_RESP_LENGTH - 1) : len;
    ret = aic_md_read(MD_ID_MANUFACTURER_INFO, len, result_line0);
    if (ret)
        aicx_error(0x111eb6, "", "read manufacturer json file error\n");
#endif

    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

u32 aic_handle_cgmi_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

/* AT+CGMM: get model identification.No parameter. */
u32 aic_handle_cgmm_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
#ifdef CONFIG_MISC_SUPPORT
    u8 len;
    s32 ret = AIC_SUCCESS;
#endif

    /* The cmd has no parameters. */
    if (strchr(at_req_res_ptr->at_req, '='))
        goto PARA_ERROR;

    /** Get model identification from json file in misc data.
     * Return model identification without any prefix.
     */
#ifdef CONFIG_MISC_SUPPORT
    len = aic_md_get_length(MD_ID_MODEL_INFO);
    len = len >= AT_MAX_AP_RESP_LENGTH ? (AT_MAX_AP_RESP_LENGTH - 1) : len;
    ret = aic_md_read(MD_ID_MODEL_INFO, len, result_line0);
#endif

    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

u32 aic_handle_cgmm_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

#ifdef CONFIG_D2D
static u32 aic_at_transmit_msg_format_reply(AT_REQ_RESP *at_req_ptr, u8 *src_ptr, u32 len)
{
    u8 *resave_ptr;

    resave_ptr = aos_zalloc_relaxed(len + AT_MSG_HEAD_LEN);
    if (!resave_ptr)
        return AT_ERROR_MALLOC_FAIL;

    aic_at_prepend_msg_head(resave_ptr, at_req_ptr, len);
    aos_memcpy((resave_ptr + AT_MSG_HEAD_LEN), src_ptr, len);
    aic_reply_to_certain_com(at_req_ptr->serial_port_id, resave_ptr,
                             (len + AT_MSG_HEAD_LEN));
    aos_free(resave_ptr);

    return AIC_SUCCESS;
}
#endif

/* AT+CGMR: get revision info.No parameter. */
u32 aic_handle_cgmr_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AIC_VERSION_LEN + 1] = {'\0'};
    AT_RESP_FROM_CP resp_to_save;
    ulong *line_arr = AIC_NULL;
    u32 len;

    /* The cmd has no parameters. */
    if (strchr(at_req_res_ptr->at_req, '='))
        goto PARA_ERROR;

    /* Get CP version info. */
    resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    aic_send_cmd_and_wait(at_req_res_ptr->at_req, &resp_to_save);

    if (strcasestr(resp_to_save.resp_buf, AT_RESULT_ERROR)) {
        len = strlen(resp_to_save.resp_buf);
#ifdef CONFIG_D2D
        if (aic_at_with_msg_head(at_req_res_ptr)) {
            if (aic_at_transmit_msg_format_reply(at_req_res_ptr, resp_to_save.resp_buf, len))
                aic_reply_to_certain_com(at_req_res_ptr->serial_port_id, resp_to_save.resp_buf, len);
        } else
#endif
        aic_reply_to_certain_com(at_req_res_ptr->serial_port_id, resp_to_save.resp_buf, len);

        aos_free(resp_to_save.resp_buf);
        return AIC_SUCCESS;
    }
#ifndef WIN32
    /* Get AP version info. */
    aic_get_version(result_line0, sizeof(result_line0));
#endif
#ifdef CONFIG_CUSTOMER_AT
    /* Don't add \r\n,for there are \r\n at the end of each line. */
    aic_reply_to_certain_com(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    at_req_res_ptr->resp_line_count++;
#else
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
#endif

    if (resp_to_save.resp_line_cnt > 1) {
        line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (line_arr) {
#ifdef CONFIG_CUSTOMER_AT
            /* Don't add \r\n,for there are \r\n at the end of each line. */
            aic_reply_to_certain_com(at_req_res_ptr->serial_port_id, (u8 *)line_arr[0],
                                     strlen((u8 *)line_arr[0]));
            at_req_res_ptr->resp_line_count++;
#else
            /* Ignore the last OK. */
#ifdef CONFIG_D2D
            /* Ignore the beginning and ending \r\n. */
            aic_transmit_at_result(at_req_res_ptr->serial_port_id, (resp_to_save.resp_buf + 2),
                                   (strlen(resp_to_save.resp_buf) - 10));
#else
            aic_reply_to_certain_com(at_req_res_ptr->serial_port_id, resp_to_save.resp_buf,
                                     (strlen(resp_to_save.resp_buf) - 6));
#endif
#endif
        }
        aic_free_paras_mem(line_arr, resp_to_save.resp_line_cnt);
    }

    aos_free(resp_to_save.resp_buf);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

u32 aic_handle_cgmr_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * ATI: get product info from misc data,including the manufacturer id, model id
 * and revision info. The cmd has no parameters.
 */
u32 aic_handle_ati_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
#ifdef CONFIG_MISC_SUPPORT
    u8 len;
    s32 ret = AIC_SUCCESS;
#endif

    /* The cmd has no parameters. */
    if (strchr(at_req_res_ptr->at_req, '='))
        goto PARA_ERROR;

#ifdef CONFIG_MISC_SUPPORT
    len = aic_md_get_length(MD_ID_MANUFACTURER_INFO);
    len = len >= AT_MAX_AP_RESP_LENGTH ? (AT_MAX_AP_RESP_LENGTH - 1) : len;
    ret = aic_md_read(MD_ID_MANUFACTURER_INFO, len, result_line0);
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aos_memset(result_line0, 0, sizeof(result_line0));
    len = aic_md_get_length(MD_ID_MODEL_INFO);
    len = len >= AT_MAX_AP_RESP_LENGTH ? (AT_MAX_AP_RESP_LENGTH - 1) : len;
    ret = aic_md_read(MD_ID_MODEL_INFO, len, result_line0);
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
#endif

    aos_memset(result_line0, 0, sizeof(result_line0));
#ifndef WIN32
    aic_get_version(result_line0, sizeof(result_line0));
#endif
#ifdef CONFIG_CUSTOMER_AT
    /* Don't add \r\n,for there are \r\n at the end of each line. */
    aic_reply_to_certain_com(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    at_req_res_ptr->resp_line_count++;
#else
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
#endif

    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

u32 aic_handle_ati_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_cclk_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    aic_reply_ok_resp(at_req_res_ptr);
    return AIC_SUCCESS;
}

static boolean is_leap_year(u32 year)
{
    return (year % 4) || ((year % 100 == 0) && (year % 400)) ? AIC_FALSE : AIC_TRUE;
}

static boolean aic_is_valid_time(aos_systime *date_time)
{

    if (!date_time)
        return AIC_FALSE;

    /* The year must be during 1970 and 2037. */
    if ((date_time->year > 37 && date_time->year < 70) || date_time->month == 0 ||
        date_time->month > 12 || date_time->day == 0 || date_time->day > 31 ||
        date_time->hour > 24 || date_time->minute > 60 || date_time->second > 60)
        return AIC_FALSE;

    if (date_time->month == 2) {
        if ((is_leap_year(date_time->year) && date_time->day > 29) ||
            (!is_leap_year(date_time->year) && date_time->day > 28))
            return AIC_FALSE;
    } else if (date_time->month == 4 || date_time->month == 6 ||
        date_time->month == 9 || date_time->month == 11) {
        if (date_time->day > 30)
            return AIC_FALSE;
    }

    return AIC_TRUE;
}

s32 aic_get_cclk_time_zone(void)
{
    return cclk_time_zone;
}

/**
 * Set time:AT+CCLK=<time>
 * Parameter <time> is of the format:yy/mm/dd,hh:mm:ss+08.
 * yy is the last two digits of the year.
*/
u32 aic_handle_cclk_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 time_without_tz[30] = {'\0'};
    u8 tz_str[8] = {'\0'};
    u8 *tmp_ptr;
    aos_systime date_time;
    u32 ret = AIC_SUCCESS;
    s32 tmp_zone;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+CCLK=", &paras_count);
    if (!paras_arr || paras_count != 1)
        goto PARA_ERROR;

    tmp_ptr = strchr((u8 *)paras_arr[0], '/');
    if (!tmp_ptr || (tmp_ptr - (u8 *)paras_arr[0]) != 2)
        goto PARA_ERROR;

    tmp_ptr = (u8 *)paras_arr[0];
    while(*tmp_ptr) {
        if (*tmp_ptr == '+' || *tmp_ptr == '-') {
            strncpy(time_without_tz, (u8 *)paras_arr[0], (tmp_ptr - (u8 *)paras_arr[0]));
            strncpy(tz_str, tmp_ptr, (sizeof(tz_str) - 1));
            break;
        }
        tmp_ptr++;
    }

    if (strlen(tz_str) > 0) {
        tmp_zone = atoi(tz_str);
        if (tmp_zone < -96 || tmp_zone > 96)
            goto PARA_ERROR;
    } else
        goto PARA_ERROR;

    aos_memset((u8 *)&date_time, 0, sizeof(aos_systime));
    ret = aic_parse_civil_time(time_without_tz, &date_time);
    if (ret)
        goto PARA_ERROR;

    if (!aic_is_valid_time(&date_time))
        goto PARA_ERROR;

    if (date_time.year <= 37)
        date_time.year += 2000;
    else
        date_time.year += 1900;

    if (tmp_zone >= 48)
        cclk_time_zone = 48;
    if (tmp_zone <= -48)
        cclk_time_zone = -48;
    else
        cclk_time_zone = tmp_zone;

    current_time_zone = cclk_time_zone / 4;
    time_zone_in_quarters = cclk_time_zone;

    aic_free_paras_mem(paras_arr, paras_count);
    aos_set_utctime(&date_time);

    aic_reply_ok_resp(at_req_res_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);
    return AIC_SUCCESS;
}

u32 aic_handle_cclk_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    s32 time_zone;
    u8 time_str[100] = {'\0'};
    u8 len;

    strcpy(result_line0, "+CCLK: \"");
    time_zone = aic_get_time_zone_in_quarters();

#ifdef CONFIG_CUSTOMER_CM_AT
    aic_sntp_client_get_time(time_str, 100, 0);
    len = strlen(time_str);
    if (cclk_time_zone >= 0) {
        snprintf((time_str + len - 3), 4, "+%2d", cclk_time_zone);
        if (cclk_time_zone < 10)
            *(time_str + len - 2) = '0';
    } else {
        snprintf((time_str + len - 3), 4, "-%2d", (-cclk_time_zone));
        if (-cclk_time_zone < 10)
            *(time_str + len - 2) = '0';
    }
#else
    aic_sntp_client_get_time(time_str, 100, time_zone);
    len = strlen(time_str);
    if (time_zone >= 0) {
        snprintf((time_str + len - 3), 4, "+%2d", time_zone);
        if (time_zone < 10)
            *(time_str + len - 2) = '0';
    } else {
        snprintf((time_str + len - 3), 4, "-%2d", -time_zone);
        if (-time_zone < 10)
            *(time_str + len - 2) = '0';
    }
#endif

    /* The year should be yy,not yyyy. */
    if (len > 2)
        strncat(result_line0, (time_str + 2), (strlen(time_str) - 2));

    strncat(result_line0, "\"", 1);
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}
#endif

#ifdef CONFIG_CUSTOMER_AT
/* Only support context_id 1-6. */
u32 aic_handle_qideact_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+QIDEACT: (1-%d)", ANET_MAX_PDP_CID);
    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

/* AT+QIDEACT=<context_id> -> AT+CGACT=0,<context_id> */
u32 aic_handle_qideact_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 cid;
    u8 cmd_str[20] = {'\0'};
    AT_RESP_FROM_CP resp_to_save;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+QIDEACT=", &paras_count);
    if (!paras_arr || paras_count != 1)
        goto PARA_ERROR;

    cid = atoi((u8 *)paras_arr[0]);
    if (!(cid >= 1 && cid <= ANET_MAX_PDP_CID))
        goto PARA_ERROR;
    aic_free_paras_mem(paras_arr, paras_count);

    resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    snprintf(cmd_str, 20, "AT+CGACT=0,%d", cid);
    aic_send_cmd_and_wait(cmd_str, &resp_to_save);

    aic_reply_to_certain_com(at_req_ptr->serial_port_id, (resp_to_save.resp_buf + 2),
                             (strlen(resp_to_save.resp_buf) - 2));
    aos_free(resp_to_save.resp_buf);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

/* Only support context_id 1-6. */
u32 aic_handle_qiact_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+QIACT: (1-%d)", ANET_MAX_PDP_CID);
    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

/* List all activated pdp. Context_type only supports IPV4.
 * +QIACT: cid,<state>,<context_type>[,<ip_address>]
 */
u32 aic_handle_qiact_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH];
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    NET_PDP_INFO *pdp_info;
    u8 i, cid;
    u8 global_ipv6_first[ANET_MAX_LEN_HEX_IPV6_ADDR] = {'\0'};
    WAN_NET *wan_net;

#ifdef CONFIG_PDP_MGR
    aos_get_mutex(pdp_info_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        aos_memset(result_line0, '\0', AT_MAX_AP_RESP_LENGTH);
        pdp_info = &net_pdp_info_list[i];
        cid = pdp_info->pdp_cid;
        if (cid >= 1 && cid <= ANET_MAX_PDP_CID) {
            if (pdp_info->pdp_activated == ANET_PDP_ACTIVATED) {
                snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
                         "+QIACT: %d,1,%d", cid, aic_get_pdp_ip_type(cid));

                if (strlen(pdp_info->pdp_ipv4_address) > 0) {
                    strcat(result_line0, ",\"");
                    strncat(result_line0, pdp_info->pdp_ipv4_address,
                            (AT_MAX_AP_RESP_LENGTH - strlen(result_line0) - 1));
                    strcat(result_line0, "\"");
                }

                wan_net = aic_get_wan_net_per_pdp(cid);
                if (wan_net && aic_is_valid_ipv6_addr(wan_net->wan_ipv6_address1)) {
                    aic_convert_numeric_ipv6_to_str(wan_net->wan_ipv6_address1,
                            global_ipv6_first, ANET_MAX_LEN_HEX_IPV6_ADDR);
                    strcat(result_line0, ",\"");
                    strncat(result_line0, global_ipv6_first,
                            (AT_MAX_AP_RESP_LENGTH - strlen(result_line0) - 1));
                    strcat(result_line0, "\"");
                }

                line_pointers[0] = (ulong)&result_line0[0];
                aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
            }
        }
    }

    aos_put_mutex(pdp_info_mutex);
#endif

    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

/* AT+QIACT=<context_id> -> AT+CGACT=1,<context_id> */
u32 aic_handle_qiact_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 cid;
    u8 cmd_str[20] = {'\0'};
    AT_RESP_FROM_CP resp_to_save;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+QIACT=", &paras_count);
    if (!paras_arr || paras_count != 1)
        goto PARA_ERROR;

    cid = atoi((u8 *)paras_arr[0]);
    if (!(cid >= 1 && cid <= ANET_MAX_PDP_CID))
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);

    resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    snprintf(cmd_str, 20, "AT+CGACT=1,%d", cid);
    aic_send_cmd_and_wait(cmd_str, &resp_to_save);

    aic_reply_to_certain_com(at_req_ptr->serial_port_id, (resp_to_save.resp_buf + 2),
                             (strlen(resp_to_save.resp_buf) - 2));
    aos_free(resp_to_save.resp_buf);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

/* Only support context_id 1-6.
 * +QICSGP: (1-6),(1-3),<APN>,<username>,<password>,(0-3)
 */
u32 aic_handle_qicsgp_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+QICSGP: (1-%d),(1-3),<APN>,<username>,<password>,(0-2)", ANET_MAX_PDP_CID);
    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

/* AT+QICSGP=<context_id>.Query the configration of the context id.
 * +QICSGP: <context_type>,<APN>,<username>,<password>,<authentication>
 */
void handle_qicsgp_query_cmd(u8 cid, AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u8 pdp_type_num = ANET_DEFAULT_IP_TYPE_V4V6;
    u8 auth_protocol = ANET_APN_NO_AUTH_TYPE;
    AT_RESP_FROM_CP resp_to_save;
    u8 auth_proto_str[4] = {'\0'};
    u8 apn_name[ANET_APN_NAME_MAX_LEN] = {'\0'};
    u8 user_name[ANET_AUTH_USERID_LEN] = {'\0'};
    u8 password[ANET_AUTH_PWD_LEN] = {'\0'};

    resp_to_save.resp_buf = aos_zalloc(AT_MAX_ONE_RESP_LENGTH);
    resp_to_save.buf_len = AT_MAX_ONE_RESP_LENGTH;
    aic_send_cmd_and_wait("AT+CGDCONT?", &resp_to_save);

    if (resp_to_save.resp_line_cnt)
        pdp_type_num = get_pdp_type_from_cgdcont(resp_to_save.resp_buf,
                                                 resp_to_save.resp_line_cnt,
                                                 cid, apn_name);

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+QICSGP: %d,\"%s\"",
             pdp_type_num, apn_name);

    aos_memset(resp_to_save.resp_buf, 0, resp_to_save.buf_len);
    resp_to_save.resp_line_cnt = 0;
    aic_send_cmd_and_wait("AT+CGAUTH?", &resp_to_save);

    if (resp_to_save.resp_line_cnt)
        auth_protocol = get_auth_config_from_cgauth(resp_to_save.resp_buf,
                                                    resp_to_save.resp_line_cnt,
                                                    cid, user_name, password);

    strcat(result_line0, ",\"");
    strncat(result_line0, user_name, strlen(user_name));
    strcat(result_line0, "\",\"");
    strncat(result_line0, password, strlen(password));
    strcat(result_line0, "\",");
    snprintf(auth_proto_str, sizeof(auth_proto_str), "%d", auth_protocol);
    strcat(result_line0, auth_proto_str);

    aos_free(resp_to_save.resp_buf);

    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);
}

u8 get_auth_config_from_cgauth(u8 *resp_ptr, u8 line_cnt, u8 pdp_cid,
                               u8 *user_name, u8 *pwd_ptr)
{
    ulong *line_arr;
    u8 i, cid = 0, auth_proto = ANET_APN_NO_AUTH_TYPE;
    u16 paras_count = 0;
    ulong *para_arr;

    line_arr = aic_get_each_line_from_resp(resp_ptr, line_cnt);
    if (!line_arr)
        return auth_proto;

    for (i = 0; i < line_cnt; i++) {
        if (strcasestr((u8 *)line_arr[i], "+CGAUTH:")) {
            para_arr = aic_split_paras_to_str_array((u8 *)line_arr[i],
                                                    "+CGAUTH: ",
                                                    &paras_count);

            /* +CGAUTH: <cid>,<auth_proto>,<user_id>,<passwork> */
            if (para_arr && paras_count >= 1) {
                cid = atoi((u8 *)para_arr[0]);
                if (cid == pdp_cid) {
                    if (paras_count >= 2)
                        auth_proto = atoi((u8 *)para_arr[1]);

                    if (paras_count >= 3)
                        strncpy(user_name, (u8 *)para_arr[2],
                                (ANET_AUTH_USERID_LEN - 1));

                    if (paras_count >= 4)
                        strncpy(pwd_ptr, (u8 *)para_arr[3],
                                (ANET_AUTH_PWD_LEN - 1));

                    aic_free_paras_mem(para_arr, paras_count);
                    break;
                }
            }
            aic_free_paras_mem(para_arr, paras_count);
        }
    }
    aic_free_paras_mem(line_arr, line_cnt);

    return auth_proto;
}

u8 get_pdp_type_from_cgdcont(u8 *resp_ptr, u8 line_cnt, u8 pdp_cid, u8 *apn_str)
{
    ulong *line_arr;
    u8 i, cid = 0, pdp_type_num = ANET_DEFAULT_IP_TYPE_V4V6;
    u16 paras_count = 0;
    ulong *para_arr;

    line_arr = aic_get_each_line_from_resp(resp_ptr, line_cnt);
    if (!line_arr)
        return pdp_type_num;

    for (i = 0; i < line_cnt; i++) {
        if (strcasestr((u8 *)line_arr[i], "+CGDCONT:")) {
            para_arr = aic_split_paras_to_str_array((u8 *)line_arr[i],
                                                    "+CGDCONT: ",
                                                    &paras_count);
            if (para_arr && paras_count >= 2) {
                cid = atoi((u8 *)para_arr[0]);
                if (cid == pdp_cid) {
                    if (strcasestr((u8 *)para_arr[1], "V4V6")) {
                        pdp_type_num = ANET_DEFAULT_IP_TYPE_V4V6;
                    } else if (strcasestr((u8 *)para_arr[1], "V6")) {
                        pdp_type_num = ANET_DEFAULT_IP_TYPE_V6;
                    } else if (strcasestr((u8 *)para_arr[1], "IP")) {
                        pdp_type_num = ANET_DEFAULT_IP_TYPE_V4;
                    }

                    if (paras_count >= 3 && apn_str)
                        snprintf(apn_str, ANET_APN_NAME_MAX_LEN,
                                 "%s", (u8 *)para_arr[2]);

                    aic_free_paras_mem(para_arr, paras_count);
                    break;
                }
            }
            aic_free_paras_mem(para_arr, paras_count);
        }
    }
    aic_free_paras_mem(line_arr, line_cnt);

    return pdp_type_num;
}

/* AT+QICSGP=<context_id>,<context_type>[,<APN>,<username>,
 * <password>,<authentication>]
 */
u32 aic_handle_qicsgp_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0, cmd_len;
    ulong *paras_arr;
    u8 cid, context_type, auth_protocol, resp_len = 50;
    u8 *cmd_str1 = AIC_NULL, *cmd_str2 = AIC_NULL;
    u8 context_type_str[ANET_PDP_TYPE_MAX_LEN] = {'\0'};
    AT_RESP_FROM_CP resp_to_save;
    u32 err_num = AT_ERROR_INVALID_PARAS;

    aos_memset(&resp_to_save, 0, sizeof(AT_RESP_FROM_CP));
    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+QICSGP=", &paras_count);
    if (!paras_arr || paras_count < 1 || paras_count > 6)
        goto PARA_ERROR;

    cid = atoi((u8 *)paras_arr[0]);
    if (!(cid >= 1 && cid <= ANET_MAX_PDP_CID))
        goto PARA_ERROR;

    /* AT+QICSGP=<context_id>.is to query the configration of the context id.
     * reply: +QICSGP: <context_type>,<APN>,<username>,<password>,<authentication>
     */
    if (paras_count == 1) {
        aic_free_paras_mem(paras_arr, paras_count);
        handle_qicsgp_query_cmd(cid, at_req_ptr);
        return AIC_SUCCESS;
    }

    context_type = atoi((u8 *)paras_arr[1]);
    if (!(context_type >= ANET_DEFAULT_IP_TYPE_V4 &&
        context_type <= ANET_DEFAULT_IP_TYPE_V4V6))
        goto PARA_ERROR;

    if (context_type == ANET_DEFAULT_IP_TYPE_V4)
        strcpy(context_type_str, "IP");
    else if (context_type == ANET_DEFAULT_IP_TYPE_V6)
        strcpy(context_type_str, "IPV6");
    else if (context_type == ANET_DEFAULT_IP_TYPE_V4V6)
        strcpy(context_type_str, "IPV4V6");

    cmd_len = 180;
    cmd_str1 = aos_zalloc_relaxed(cmd_len);
    if (!cmd_str1) {
        err_num = AT_ERROR_MALLOC_FAIL;
        goto PARA_ERROR;
    }

    if (paras_count >= 3) {
        if (strlen((u8 *)paras_arr[2]) > ANET_APN_NAME_MAX_LEN)
            goto PARA_ERROR;

        snprintf(cmd_str1, cmd_len, "AT+CGDCONT=%d,\"%s\",\"%s\"", cid,
                 context_type_str, (u8 *)paras_arr[2]);

        if (paras_count >= 4) {
            cmd_str2 = aos_zalloc_relaxed(cmd_len);
            if (!cmd_str2) {
                err_num = AT_ERROR_MALLOC_FAIL;
                goto PARA_ERROR;
            }

            snprintf(cmd_str2, cmd_len, "AT+CGAUTH=%d,", cid);
            if (paras_count == 6) {
                auth_protocol = atoi((u8 *)paras_arr[5]);
                if (auth_protocol > ANET_APN_AUTH_TYPE_CHAP)
                    goto PARA_ERROR;

                strcat(cmd_str2, (u8 *)paras_arr[5]);
            }

            strcat(cmd_str2, ",\"");
            if (strlen((u8 *)paras_arr[3]) > ANET_AUTH_USERID_LEN)
                goto PARA_ERROR;

            if (strlen((u8 *)paras_arr[3]) < (cmd_len - strlen(cmd_str2)))
                strcat(cmd_str2, (u8 *)paras_arr[3]);

            strcat(cmd_str2, "\"");
            if (paras_count >= 5) {
                strcat(cmd_str2, ",\"");
                if (strlen((u8 *)paras_arr[4]) > ANET_AUTH_PWD_LEN)
                    goto PARA_ERROR;

                if (strlen((u8 *)paras_arr[4]) < (cmd_len - strlen(cmd_str2)))
                    strcat(cmd_str2, (u8 *)paras_arr[4]);

                strcat(cmd_str2, "\"");
            }
        }
    } else {
        snprintf(cmd_str1, cmd_len, "AT+CGDCONT=%d,\"%s\"", cid, context_type_str);
    }

    resp_to_save.resp_buf = aos_zalloc_relaxed(resp_len);
    if (!resp_to_save.resp_buf) {
        err_num = AT_ERROR_MALLOC_FAIL;
        goto PARA_ERROR;
    }

    resp_to_save.buf_len = resp_len;
    aic_send_cmd_and_wait(cmd_str1, &resp_to_save);
    if (strcasestr(resp_to_save.resp_buf, "ERROR")) {
        /* No need to send AT+CGAUTH. */
        aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                 (resp_to_save.resp_buf + 2),
                                 (strlen(resp_to_save.resp_buf) - 2));
    } else {
        if (paras_count < 4) {
            aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                     (resp_to_save.resp_buf + 2),
                                     (strlen(resp_to_save.resp_buf) - 2));
        } else {
        /* send cmd AT+CGAUTH to cp */
        aos_memset(resp_to_save.resp_buf, 0, resp_to_save.buf_len);
        aic_send_cmd_and_wait(cmd_str2, &resp_to_save);
        aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                 (resp_to_save.resp_buf + 2),
                                 (strlen(resp_to_save.resp_buf) - 2));
        }
    }

    if (cmd_str1)
        aos_free(cmd_str1);

    if (cmd_str2)
        aos_free(cmd_str2);

    aic_free_paras_mem(paras_arr, paras_count);
    aos_free(resp_to_save.resp_buf);

    return AIC_SUCCESS;

PARA_ERROR:
    if (cmd_str1)
        aos_free(cmd_str1);

    if (cmd_str2)
        aos_free(cmd_str2);

    if (resp_to_save.resp_buf)
        aos_free(resp_to_save.resp_buf);

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, err_num);

    return AIC_SUCCESS;
}

/** Set the baud rate of the UART. AT+IPREX=<rate>
 * The set and query cmd can only be applied to UART port.
 * If it is an USB serial port,then return error.
 */
u32 aic_handle_iprex_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u32 baud_rate;
    u8 ok_str[8] = {0};

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+IPREX=", &paras_count);
    if (paras_count == 0) {
        baud_rate = 115200;
    } else {
        baud_rate = atoi((u8 *)paras_arr[0]);
        if (baud_rate == 0)
            goto PARA_ERROR;
    }

    if (!(baud_rate == 300 || baud_rate == 600 || baud_rate == 1200
        || baud_rate == 2400 || baud_rate == 4800 || baud_rate == 9600
        || baud_rate == 14400 || baud_rate == 19200 || baud_rate == 28800
        || baud_rate == 38400 || baud_rate == 57600
        || baud_rate == 115200 || baud_rate == 128000 || baud_rate == 230400
        || baud_rate == 256000 || baud_rate == 460800 || baud_rate == 500000
        || baud_rate == 512000 || baud_rate == 600000 || baud_rate == 750000
        || baud_rate == 921600 || baud_rate == 1000000 || baud_rate == 2000000
        || baud_rate == 3686400)) {
        goto PARA_ERROR;
    }

    aic_free_paras_mem(paras_arr, paras_count);
    strcpy(ok_str, "OK");
    ok_str[2] = aic_get_cr_sign();
    ok_str[3] = aic_get_lf_sign();
    aic_send_to_at_port(at_req_res_ptr->serial_port_id, ok_str, 4);
    aos_sleep(5);
    conn_cmd_set_baud_rate(CONN_CMD_AT, conn_get_uart_at_conid, baud_rate, AIC_TRUE);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);
    return AIC_SUCCESS;
}

u32 aic_handle_iprex_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    s32 baud_rate;

    /* get the baud rate. */
    baud_rate = conn_cmd_get_baud_rate(CONN_CMD_AT, conn_get_uart_at_conid);
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+IPREX: %d", baud_rate);
    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 aic_handle_iprex_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH + 40] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line0, "+IPREX: (300,600,1200,2400,4800,9600,14400,19200,28800,",
            (AT_MAX_AP_RESP_LENGTH + 40 - 1));
    strncat(result_line0, "38400,57600,115200,128000,230400,256000,460800,500000,",
            (AT_MAX_AP_RESP_LENGTH + 40  - 1 - strlen(result_line0)));
    strncat(result_line0, "512000,600000,750000,921600,1000000,2000000,3686400)",
            (AT_MAX_AP_RESP_LENGTH + 40  - 1 - strlen(result_line0)));
    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    /* Fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}
#endif

u32 aic_handle_s3_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             AT_CMD_S3"=", &paras_count);
    if (!paras_arr || (paras_count != 1) ||
        !aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])) ||
        !(atoi((u8 *)paras_arr[0]) > 0 && atoi((u8 *)paras_arr[0]) <= 127))
        goto TO_CP;

    aic_set_cr_sign(atoi((u8 *)paras_arr[0]));

TO_CP:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_send_at_to_cp(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_s4_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 para[4] = {'\0'};
    u8 *tmp_ptr1 = AIC_NULL, *tmp_ptr2 = AIC_NULL;

    if (!strcasestr(at_req_res_ptr->at_req, AT_CMD_S4"="))
        goto TO_CP;

    tmp_ptr1 = at_req_res_ptr->at_req + strlen(AT_CMD_S4"=");
    tmp_ptr2 = tmp_ptr1;
    while (*tmp_ptr2) {
        if (is_cr_lf_sign(*tmp_ptr2))
            break;

        tmp_ptr2++;
        continue;
    }

    if ((tmp_ptr2 - tmp_ptr1) == 0 || (tmp_ptr2 - tmp_ptr1) > 3)
        goto TO_CP;

    strncpy(para, tmp_ptr1, (tmp_ptr2 - tmp_ptr1));
    if (!aic_is_valid_integer_para(para, strlen(para)) ||
        !(atoi(para) > 0 && atoi(para) <= 127))
        goto TO_CP;

    aic_set_lf_sign(atoi(para));

TO_CP:
    aic_send_at_to_cp(at_req_res_ptr);

    return AIC_SUCCESS;
}

#ifndef CONFIG_AT_PROC_MINI
/* Set MT registers network via real net or intrumentation.  */
u32 aic_handle_regmode_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 reg_mode;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT^REGMODE=", &paras_count);
    if (!paras_arr || paras_count != 1)
        goto PARA_ERROR;

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto PARA_ERROR;

    reg_mode = atoi((u8 *)paras_arr[0]);
    aic_free_paras_mem(paras_arr, paras_count);

    if (reg_mode != aic_get_reg_mode()) {
        aic_set_reg_mode(reg_mode);
        aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
        aic_read_net_config(&net_cfg);
        net_cfg.reg_mode = reg_mode;
        aic_save_config_to_nv(&net_cfg);
#endif
    }

    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_regmode_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    strncpy(result_line0, "^REGMODE: (0-3)", (AT_MAX_AP_RESP_LENGTH - 1));
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_regmode_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "^REGMODE: %d", aic_get_reg_mode());
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line0, strlen(result_line0));

    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}
#endif

u8 aic_get_reg_mode(void)
{
    return init_net_cfg.net_cfg.reg_mode;
}

void aic_set_reg_mode(u8 reg_mode)
{
    init_net_cfg.net_cfg.reg_mode = reg_mode;
}

#ifdef CONFIG_MISC_SUPPORT
s32 aic_save_config_to_nv(NET_CFG *net_cfg)
{
    s32 ret;

    ret = aic_md_write(MD_ID_NET_CFG, sizeof(NET_CFG), (u8 *)net_cfg);
    if (ret)
        aicx_error(0x17728c, "x", "write net cfg to nv error 0x%x\n", ret);

    return ret;
}

s32 aic_read_net_config(NET_CFG *net_cfg)
{
    s32 ret;

    ret = aic_md_read(MD_ID_NET_CFG, sizeof(NET_CFG), net_cfg);
    if (ret) {
        aicx_error(0x0dd312, "x", "read net cfg error 0x%x\n", ret);
        net_cfg->auto_start_ps = 1;
    }

    return ret;
}
#endif

#ifndef CONFIG_AT_PROC_MINI
/* Set if send AT+CFUN=1 to start ps when power on. */
u32 aic_handle_autostartps_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 start_mode;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT^AUTOSTARTPS=",
                                             &paras_count);
    if (!paras_arr || paras_count != 1)
        goto PARA_ERROR;

    start_mode = atoi((u8 *)paras_arr[0]);
    if (start_mode != 0 && start_mode != 1)
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
    net_cfg.auto_start_ps = start_mode;
    /* Write it to NV. */
    aic_save_config_to_nv(&net_cfg);
#endif
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_autostartps_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[30] = {'\0'};

    strcpy(result_line0, "^AUTOSTARTPS: (0,1)");
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_autostartps_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    NET_CFG net_cfg;

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
#else
    net_cfg.auto_start_ps = 1;
#endif

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "^AUTOSTARTPS: %d", net_cfg.auto_start_ps);
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* Set if auto start usb tether if usb is plugged in. */
u32 aic_handle_autousbnet_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 start_mode;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT^AUTOUSBNET=",
                                             &paras_count);
    if (paras_count != 1)
        goto PARA_ERROR;

    start_mode = atoi((u8 *)paras_arr[0]);
    if (aic_boot_mode_is_basic() || (start_mode != 0 && start_mode != 1))
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
    net_cfg.auto_start_usb_net = start_mode;
    /* Write it to NV. */
    aic_save_config_to_nv(&net_cfg);
#endif

    aic_reply_ok_resp(at_req_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_autousbnet_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    NET_CFG net_cfg;

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
#else
    net_cfg.auto_start_usb_net = 0;
#endif

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "^AUTOUSBNET: %d", net_cfg.auto_start_usb_net);
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_autousbnet_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line0[30] = {'\0'};

    strcpy(result_line0, "^AUTOUSBNET: (0,1)");
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}
#endif

boolean is_valid_urc_port_name(u8 *port_name)
{
    u8 len1, len2;

    if (!port_name || strlen(port_name) == 0)
        return AIC_FALSE;

    len1 = strlen(port_name);
    len2 = strlen(urc_cfg.uart_port_name);
    if (len1 == len2 && strncasecmp(urc_cfg.uart_port_name, port_name, len1) == 0)
        return AIC_TRUE;

    len2 = strlen(urc_cfg.usb_port_name);
    if (len1 == len2 && strncasecmp(urc_cfg.usb_port_name, port_name, len1) == 0)
        return AIC_TRUE;

    return AIC_FALSE;
}

#if defined CONFIG_CUSTOMER_AT || defined CONFIG_CUSTOMER_JQ_AT
/** Set the output port of URC.
 * AT+QURCCFG="urcport"[,<urcportvalue>]
 */
u32 aic_handle_qurccfg_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0, cmp_len;
    ulong *paras_arr;
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    NET_CFG net_cfg;
    boolean is_valid;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+QURCCFG=",
                                             &paras_count);
    if (!paras_arr || paras_count == 0)
        goto PARA_ERROR;

    cmp_len = strlen((u8 *)paras_arr[0]) >= strlen("urcport") ?
              strlen((u8 *)paras_arr[0]) : strlen("urcport");
    /* The first parameter must be urcport. */
    if (strncasecmp((u8 *)paras_arr[0], "urcport", cmp_len) != 0)
        goto PARA_ERROR;

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
    /* Return current config. */
    if (paras_count == 1) {
#ifdef CONFIG_MISC_SUPPORT
        aic_read_net_config(&net_cfg);
#endif
        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
                 "+QURCCFG: \"urcport\",\"%s\"", net_cfg.urc_port_name);
        line_pointers[0] = (ulong)&result_line0[0];
        aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    }

    if (paras_count == 2) {
        is_valid = is_valid_urc_port_name((u8 *)paras_arr[1]);
#ifdef CONFIG_MISC_SUPPORT
        if (is_valid) {
            aic_read_net_config(&net_cfg);
            aos_memset(net_cfg.urc_port_name, 0, AT_URC_PORT_NAME_LEN);
            snprintf(net_cfg.urc_port_name, AT_URC_PORT_NAME_LEN,
                     "%s", (u8 *)paras_arr[1]);
            /* Write it to NV. */
            aic_save_config_to_nv(&net_cfg);
        }
#endif
        if (!is_valid_urc_port_name(net_cfg.urc_port_name)) {
            urc_cfg.urc_port_id = AT_NO_SERIAL_PORT_ID;
        } else {
            if (strcasestr(net_cfg.urc_port_name, "usbat"))
                urc_cfg.urc_port_id = AT_USB_AT_PORT_ID;

            if (strcasestr(net_cfg.urc_port_name, "uart"))
                urc_cfg.urc_port_id = conn_get_uart_at_conid;
        }

        if (!is_valid && strlen((u8 *)paras_arr[1]) > 0)
            goto PARA_ERROR;
    }

    aic_reply_ok_resp(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

/* response: +QURCCFG: "urcport",<urcportvalue> */
u32 aic_handle_qurccfg_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    NET_CFG net_cfg;

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
#endif
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+QURCCFG: \"urcport\",\"%s\"", net_cfg.urc_port_name);

    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_qurccfg_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u8 uart_port[8] = {'\0'};

    strcpy(result_line0, "+QURCCFG: \"urcport\",(");
    if (conn_get_usb_at_conid != AIC_ENODEV)
        strcat(result_line0, "\"usbat\",");

    if (conn_get_uart_at_conid != AIC_ENODEV) {
        snprintf(uart_port, 8, "\"uart%d\"",
                 conn_get_at_com_number(conn_get_uart_at_conid));
        strcat(result_line0, uart_port);
    }
    strcat(result_line0, ")");

    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}
#endif

/* Save the configed urc port id. */
u32 aic_get_configed_urc_port_id(void)
{
#if CONFIG_CUSTOMER_AT || CONFIG_CUSTOMER_JQ_AT
    return urc_cfg.urc_port_id;
#else
    return AT_NO_SERIAL_PORT_ID;
#endif
}

#if CONFIG_CUSTOMER_AT || CONFIG_CUSTOMER_JQ_AT
void aic_init_urc_cfg(void)
{
    urc_cfg.urc_port_id = AT_NO_SERIAL_PORT_ID;
    urc_cfg.ri_enabled = AT_URC_RING_IND_ON;
    urc_cfg.pulse_duration = AT_URC_DEFAULT_PULSE_DURATION;
    urc_cfg.pulse_cnt = AT_URC_DEFAULT_PULSE_CNT;
    urc_cfg.ri_delay_time = AT_URC_DEFAULT_DELAY_TIME;
    urc_cfg.cache_enabled = AT_URC_CACHE_DISABLED;
    urc_cfg.usb_urc_line_cnt = 0;
    urc_cfg.uart_urc_line_cnt = 0;
    snprintf(urc_cfg.ri_signal_type, sizeof(urc_cfg.ri_signal_type), "%s", "respective");

    if (conn_get_uart_at_conid != AIC_ENODEV) {
        snprintf(urc_cfg.uart_port_name, sizeof(urc_cfg.uart_port_name),
                 "uart%d", conn_get_at_com_number(conn_get_uart_at_conid));
        urc_cfg.uart_urc_cache_queue = aos_create_queue("uart_urc_cache_queue",
                                                        sizeof(void*),
                                                        AT_URC_CACHE_MAX_LINE_NUM);
        AIC_ASSERT(urc_cfg.uart_urc_cache_queue);
    }

    if (conn_get_usb_at_conid != AIC_ENODEV) {
        strcpy(urc_cfg.usb_port_name, "usbat");
        urc_cfg.usb_urc_cache_queue = aos_create_queue("usb_urc_cache_queue",
                                                       sizeof(void*),
                                                       AT_URC_CACHE_MAX_LINE_NUM);
        AIC_ASSERT(urc_cfg.usb_urc_cache_queue);
    }

    urc_cfg.urc_cache_mutex = aos_create_mutex("urc_cache_mutex");
    AIC_ASSERT(urc_cfg.urc_cache_mutex);

#ifdef CONFIG_MISC_SUPPORT
    if (!init_net_cfg.init_read_flag) {
        aos_memset(&init_net_cfg, 0, sizeof(INIT_NET_CFG));
        if (!aic_read_net_config(&init_net_cfg.net_cfg))
            init_net_cfg.init_read_flag = 1;
    }

    if (init_net_cfg.init_read_flag) {
        snprintf(urc_cfg.ri_signal_type, sizeof(urc_cfg.ri_signal_type),
                 "%s", init_net_cfg.net_cfg.ri_signal_type);
        if (is_valid_urc_port_name(init_net_cfg.net_cfg.urc_port_name)) {
            if (strcasestr(init_net_cfg.net_cfg.urc_port_name, "usbat"))
                urc_cfg.urc_port_id = AT_USB_AT_PORT_ID;

            if (strcasestr(init_net_cfg.net_cfg.urc_port_name, "uart"))
                urc_cfg.urc_port_id = conn_get_uart_at_conid;
        }
    }
#endif
}
#endif

#ifdef CONFIG_CUSTOMER_JQ_AT
static boolean is_valid_ri_signal_type(u8 *signal_type)
{
    u8 len;

    if (!signal_type)
        return AIC_FALSE;

    len = strlen("physical");
    if (strlen(signal_type) == len && strncasecmp(signal_type, "physical", len) == 0)
        return AIC_TRUE;

    len = strlen("respective");
    if (strlen(signal_type) == len && strncasecmp(signal_type, "respective", len) == 0)
        return AIC_TRUE;

    return AIC_FALSE;
}

/* AT+QCFG="risignaltype"[,<RI_signal_type>] */
u32 aic_handle_qcfg_ri_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+QCFG=",
                                             &paras_count);
    if (!paras_arr || paras_count == 0)
        goto PARA_ERROR;

    if (strncasecmp((u8 *)paras_arr[0], "risignaltype", strlen("risignaltype")) == 0) {
        if (strlen((u8 *)paras_arr[0]) != strlen("risignaltype") || paras_count > 2)
            goto PARA_ERROR;

        if (paras_count == 1) {
            snprintf(result_line0, sizeof(result_line0),
                     "+QCFG: \"risignaltype\",\"%s\"", urc_cfg.ri_signal_type);
            line_pointers[0] = (ulong)&result_line0[0];
            aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
        } else if (paras_count == 2) {
            if (!is_valid_ri_signal_type((u8 *)paras_arr[1]))
                goto PARA_ERROR;

#ifdef CONFIG_MISC_SUPPORT
            aic_read_net_config(&net_cfg);
            aos_memset(net_cfg.ri_signal_type, 0, AT_URC_RI_SIGNAL_TYPE_LEN);
            strcpy(net_cfg.ri_signal_type, (u8 *)paras_arr[1]);
            aic_save_config_to_nv(&net_cfg);
#endif
            aos_memset(urc_cfg.ri_signal_type, 0, sizeof(urc_cfg.ri_signal_type));
            strcpy(urc_cfg.ri_signal_type, (u8 *)paras_arr[1]);
        }
    }

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_res_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

/* Output a gpio signal before sending URC to AT port */
void aic_send_ri_signal_for_urc(u32 urc_com_id)
{
    if (strcasestr(urc_cfg.ri_signal_type, "respective") && urc_com_id == AT_USB_AT_PORT_ID)
        return;

    if ((strcasestr(urc_cfg.ri_signal_type, "respective") && urc_com_id == conn_get_uart_at_conid) ||
         strcasestr(urc_cfg.ri_signal_type, "physical")) {
        aicx_info(0x061f4e, "dd", "output ri signal,duration %d,cnt %d\n", urc_cfg.pulse_duration, urc_cfg.pulse_cnt);
        aic_gpio_def_pulse_generate(GPIO_OUTPUT_LVL_LOW,
                                    urc_cfg.pulse_duration,
                                    urc_cfg.pulse_cnt);
    }
}
#endif

#ifdef CONFIG_CUSTOMER_AT
void aic_get_at_port_name(u32 at_port_id, u8 *port_name_buf, u8 buf_len)
{
    aos_memset(port_name_buf, 0, buf_len);

    if (at_port_id == conn_get_uart_at_conid)
        snprintf(port_name_buf, buf_len, "uart%d",
                 conn_get_at_com_number(at_port_id));
    else if (at_port_id == AT_USB_AT_PORT_ID)
        snprintf(port_name_buf, buf_len, "usbat");
}

u32 aic_handle_qcfg_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 *tmp_ptr = AIC_NULL;
    u16 resp_len = 0, last_line_len = 0;
    AT_RESP_FROM_CP resp_to_save;
    u8 result_line0[40] = {'\0'};
    u8 result_line1[40] = {'\0'};
    u8 result_line2[80] = {'\0'};
    u8 result_line3[4] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    resp_to_save.resp_buf = aos_zalloc(SIZE_1K);
    resp_to_save.buf_len = SIZE_1K;
    aic_send_cmd_and_wait("AT+QCFG=?", &resp_to_save);

    /* If receive ERROR from CP,then just send to the at port. */
    tmp_ptr = strcasestr(resp_to_save.resp_buf, AT_RESULT_ERROR);
    if (!tmp_ptr) {
        /* The last line is /r/nOK/r/n. */
        last_line_len = 6;
        resp_len = strlen(resp_to_save.resp_buf);
        //alog_info("CP resp last line %s\n", (resp_to_save.resp_buf + resp_len - 4));
        if (resp_len > last_line_len) {
            aic_refit_to_special_format(resp_to_save.resp_buf, resp_len,
                                        resp_to_save.resp_line_cnt);
            resp_len = strlen(resp_to_save.resp_buf);
            aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                     resp_to_save.resp_buf,
                                     (resp_len - last_line_len));
        }

        /* Append the reply of urc cfg. */
        strncpy(result_line0, "+QCFG: \"urcdelay\",(0-120)",
                (sizeof(result_line0) - 1));
        strncpy(result_line1, "+QCFG: \"urc/cache\",(0,1)",
                (sizeof(result_line1) - 1));
        strncpy(result_line2,
                "+QCFG: \"urc/ri/other\",(\"off\",\"pulse\"),(1-2000)",
                (sizeof(result_line2) - 1));
        strncpy(result_line3, AT_RESULT_OK, (sizeof(result_line3) - 1));

        line_pointers[0] = (ulong)result_line0;
        line_pointers[1] = (ulong)result_line1;
        line_pointers[2] = (ulong)result_line2;
        line_pointers[3] = (ulong)result_line3;
        aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 4);
    } else {
        aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                 (resp_to_save.resp_buf + 2),
                                 (strlen(resp_to_save.resp_buf) - 2));
    }
    aos_free(resp_to_save.resp_buf);

    return AIC_SUCCESS;
}

u32 aic_handle_qcfg_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    AT_RESP_FROM_CP resp_saved;
    u16 resp_buf_len = SIZE_1K/2, pulse_duration = AT_URC_DEFAULT_PULSE_DURATION;
    u8 pulse_count = AT_URC_DEFAULT_PULSE_CNT, cache_enabled;
    u32 delay_time;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+QCFG=",
                                             &paras_count);
    if (!paras_arr || paras_count == 0)
        goto PARA_ERROR;

    /* These cmds are handled by CP. */
    if (!(strcasestr((u8 *)paras_arr[0], "urc/cache") ||
        strcasestr((u8 *)paras_arr[0], "urcdelay") ||
        strcasestr((u8 *)paras_arr[0], "urc/ri/other"))) {
        resp_saved.resp_buf = aos_zalloc(resp_buf_len);
        resp_saved.buf_len = resp_buf_len;
        aic_send_cmd_and_wait(at_req_res_ptr->at_req, &resp_saved);

        aic_refit_to_special_format(resp_saved.resp_buf, strlen(resp_saved.resp_buf),
                                    resp_saved.resp_line_cnt);
        aic_reply_to_certain_com(at_req_res_ptr->serial_port_id,
                                 resp_saved.resp_buf,
                                 strlen(resp_saved.resp_buf));
        aos_free(resp_saved.resp_buf);
    } else {
        if (strncasecmp((u8 *)paras_arr[0], "urc/cache", strlen("urc/cache")) == 0) {
            if (strlen((u8 *)paras_arr[0]) != strlen("urc/cache") || paras_count > 2)
                goto PARA_ERROR;

            /* AT+QCFG="urc/cache"[,<enabled>] */
            if (paras_count == 1) {
                snprintf(result_line0, sizeof(result_line0),
                         "+QCFG: \"urc/cache\",%d", urc_cfg.cache_enabled);
                line_pointers[0] = (ulong)&result_line0[0];
                aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
            } else if (paras_count == 2) {
                cache_enabled = atoi((u8 *)paras_arr[1]);
                if (!(cache_enabled == 0 || cache_enabled == 1))
                    goto PARA_ERROR;

                if (urc_cfg.cache_enabled != cache_enabled) {
                    urc_cfg.cache_enabled = cache_enabled;
                    if (cache_enabled == AT_URC_CACHE_ENABLED)
                        aic_reset_urc_cache_mem();
                    else {
                        aic_free_paras_mem(paras_arr, paras_count);
                        aic_reply_ok_resp(at_req_res_ptr);

                        aic_invoke_sending_urc_cached();
                        return AIC_SUCCESS;
                    }
                }
            }
        } else if (strncasecmp((u8 *)paras_arr[0], "urcdelay",
                   strlen("urcdelay")) == 0) {
            if (strlen((u8 *)paras_arr[0]) != strlen("urcdelay") || paras_count > 2)
                goto PARA_ERROR;

            /* AT+QCFG="urcdelay"[,<time>] How to use the time? */
            if (paras_count == 1) {
                snprintf(result_line0, sizeof(result_line0),
                         "+QCFG: \"urcdelay\",%d", urc_cfg.ri_delay_time);
                line_pointers[0] = (ulong)&result_line0[0];
                aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
            } else if (paras_count == 2) {
                delay_time = atoi((u8 *)paras_arr[1]);
                if (delay_time > AT_URC_MAX_DELAY_TIME)
                    goto PARA_ERROR;
                urc_cfg.ri_delay_time = delay_time;
            }
        } else if (strncasecmp((u8 *)paras_arr[0], "urc/ri/other",
                   strlen("urc/ri/other")) == 0) {
            if (strlen((u8 *)paras_arr[0]) != strlen("urc/ri/other") || paras_count > 4)
                goto PARA_ERROR;

            /* AT+QCFG="urc/ri/other"[,<typeri>,<pulse duration>,<pulse count>] */
            if (paras_count == 1) {
                if (urc_cfg.ri_enabled == AT_URC_RING_IND_OFF) {
                    strcpy(result_line0, "+QCFG: \"urc/ri/other\",\"off\"");
                } else if (urc_cfg.ri_enabled == AT_URC_RING_IND_ON) {
                    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
                             "+QCFG: \"urc/ri/other\",\"pulse\",%d,%d",
                             urc_cfg.pulse_duration, urc_cfg.pulse_cnt);
                }

                line_pointers[0] = (ulong)&result_line0[0];
                aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
            } else if (paras_count >= 2) {
                if ((strncasecmp((u8 *)paras_arr[1], "pulse", strlen("pulse")) != 0 &&
                    strncasecmp((u8 *)paras_arr[1], "off", strlen("off")) != 0))
                    goto PARA_ERROR;

                if (paras_count >= 3) {
                    pulse_duration = atoi((u8 *)paras_arr[2]);
                    if (pulse_duration < AT_URC_MIN_PULSE_DURATION ||
                        pulse_duration > AT_URC_MAX_PULSE_DURATION)
                        goto PARA_ERROR;
                }

                if (paras_count == 4) {
                    pulse_count = atoi((u8 *)paras_arr[3]);
                    if (pulse_count < AT_URC_MIN_PULSE_CNT ||
                        pulse_count > AT_URC_MAX_PULSE_CNT)
                        goto PARA_ERROR;
                }

                if (strncasecmp((u8 *)paras_arr[1], "pulse", strlen("pulse")) == 0)
                    urc_cfg.ri_enabled = AT_URC_RING_IND_ON;
                else if (strncasecmp((u8 *)paras_arr[1], "off", strlen("off")) == 0) {
                    if (paras_count > 2)
                        goto PARA_ERROR;
                    urc_cfg.ri_enabled = AT_URC_RING_IND_OFF;
                }

                urc_cfg.pulse_duration = pulse_duration;
                urc_cfg.pulse_cnt = pulse_count;
            }
        }

        aic_reply_ok_resp(at_req_res_ptr);
    }

    aic_free_paras_mem(paras_arr, paras_count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

boolean aic_is_urc_ri_enabled(void)
{
    return urc_cfg.ri_enabled == AT_URC_RING_IND_ON;
}

/* Output a gpio signal before sending URC to AT port */
void aic_send_urc_gpio_signal(void)
{
    if (aic_is_urc_ri_enabled()) {
        aicx_info(0x132c67, "d", "output gpio signal,duration %d\n", urc_cfg.pulse_duration);
        aic_gpio_def_pulse_generate(GPIO_OUTPUT_LVL_LOW,
                                    urc_cfg.pulse_duration,
                                    urc_cfg.pulse_cnt);
    }

    return;
}

/* ATO:used to re-enter data mode for the at temporarily exit data mode before. */
u32 aic_handle_ato_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 result_line[20] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u32 error_num;
    SERIAL_DATA_MODE_AT *data_mode_at_ptr;
    AT_REQ_RESP *at_ptr;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "ATO", &paras_count);
    if (paras_count > 0) {
        error_num = AT_ERROR_INVALID_PARAS;
        goto HANDLE_ERROR;
    }

    data_mode_at_ptr = aic_get_serial_data_mode_at(at_req_res_ptr->serial_port_id);
    if (data_mode_at_ptr && data_mode_at_ptr->data_mode_at) {
        at_ptr = data_mode_at_ptr->data_mode_at;
        if (!(at_ptr->at_mode == AT_TMP_CMD_MODE && aic_get_socket_is_valid())) {
            error_num = AT_ERROR_REENTER_DATA_MODE;
            aic_release_tmp_cmd_mode_at(at_ptr);
            goto HANDLE_ERROR;
        }
    } else {
        error_num = AT_ERROR_REENTER_DATA_MODE;
        goto HANDLE_ERROR;
    }

    strncpy(result_line, AT_RESULT_CONNECT, (sizeof(result_line) - 1));
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;

HANDLE_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    strncpy(result_line, AT_RESULT_NO_CARRIER, (sizeof(result_line) - 1));
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return error_num;
}

boolean aic_need_flush_urc_cache(u32 com_id)
{
    u8 urc_line_cnt = 0;

    if (aic_is_uart_com(com_id))
        urc_line_cnt = urc_cfg.uart_urc_line_cnt;
    else if (com_id == AT_USB_AT_PORT_ID)
        urc_line_cnt = urc_cfg.usb_urc_line_cnt;

    return (urc_line_cnt > 0 && urc_cfg.cache_enabled == AT_URC_CACHE_DISABLED);
}

void aic_set_if_cache_urc(u8 enable_cache)
{
    if (urc_cfg.cache_enabled == enable_cache)
        return;

    urc_cfg.cache_enabled = enable_cache;
    if (enable_cache == AT_URC_CACHE_ENABLED)
        aic_reset_urc_cache_mem();
    else if (enable_cache == AT_URC_CACHE_DISABLED)
        aic_invoke_sending_urc_cached();
}

boolean aic_is_urc_cache_enabled(void)
{
    return urc_cfg.cache_enabled == AT_URC_CACHE_ENABLED;
}

void aic_add_urc_to_cache(u8 *urc_str)
{
    u32 ret = 0xff;
    u8 *old_item = AIC_NULL;
    u8 *start_ptr = AIC_NULL;
    u8 current_line_cnt = 0;

    if (!urc_str)
        return;

    aos_get_mutex(urc_cfg.urc_cache_mutex, AOS_WAIT_FOREVER);
    /* The line count of uart and usb should be equal if they both exist. */
    if (urc_cfg.uart_urc_cache_queue)
        current_line_cnt = urc_cfg.uart_urc_line_cnt;
    else if (urc_cfg.usb_urc_cache_queue)
        current_line_cnt = urc_cfg.usb_urc_line_cnt;

    if (current_line_cnt < AT_URC_CACHE_MAX_LINE_NUM) {
        start_ptr = urc_cfg.urc_cache_ptr +
                    (AT_URC_CACHE_ONE_LINE_SIZE * current_line_cnt);
    } else if (current_line_cnt == AT_URC_CACHE_MAX_LINE_NUM) {
        /* The queue is full. */
        if (urc_cfg.uart_urc_cache_queue)
            ret = aos_recv_from_queue(urc_cfg.uart_urc_cache_queue,
                                      (void*)&old_item, AOS_NO_WAIT);

        if (urc_cfg.usb_urc_cache_queue)
            ret = aos_recv_from_queue(urc_cfg.usb_urc_cache_queue,
                                      (void*)&old_item, AOS_NO_WAIT);

        if (ret == AIC_SUCCESS) {
            /* Reuse the memory of old_item. */
            start_ptr = old_item;
        }
    }

    if (!start_ptr)
        return;

    aos_memset(start_ptr, 0, AT_URC_CACHE_ONE_LINE_SIZE);
    strncpy(start_ptr, urc_str, (AT_URC_CACHE_ONE_LINE_SIZE - 1));
    if (urc_cfg.uart_urc_cache_queue) {
        aos_send_to_queue(urc_cfg.uart_urc_cache_queue,
                          (void *)&start_ptr, AOS_NO_WAIT);
        if (urc_cfg.uart_urc_line_cnt < AT_URC_CACHE_MAX_LINE_NUM)
            urc_cfg.uart_urc_line_cnt++;
    }

    if (urc_cfg.usb_urc_cache_queue) {
        aos_send_to_queue(urc_cfg.usb_urc_cache_queue,
                          (void *)&start_ptr, AOS_NO_WAIT);
        if (urc_cfg.usb_urc_line_cnt < AT_URC_CACHE_MAX_LINE_NUM)
            urc_cfg.usb_urc_line_cnt++;
    }

    aos_put_mutex(urc_cfg.urc_cache_mutex);
}

/* Reset urc cache when urc cache memory is enabled. */
void aic_reset_urc_cache_mem(void)
{
    u16 cache_size;

    cache_size = AT_URC_CACHE_ONE_LINE_SIZE * AT_URC_CACHE_MAX_LINE_NUM;
    aicx_info(0x0cd655, "", "reset urc cache\n");
    aos_get_mutex(urc_cfg.urc_cache_mutex, AOS_WAIT_FOREVER);
    urc_cfg.usb_urc_line_cnt = 0;
    urc_cfg.uart_urc_line_cnt = 0;

    if (!urc_cfg.urc_cache_ptr) {
        urc_cfg.urc_cache_ptr = aos_zalloc(cache_size);
        AIC_ASSERT(urc_cfg.urc_cache_ptr);
    } else {
        aos_memset(urc_cfg.urc_cache_ptr, 0, cache_size);
    }
    aos_put_mutex(urc_cfg.urc_cache_mutex);
}

void aic_send_urc_cached(u32 com_id)
{
    u8 *old_item = AIC_NULL;
    u32 ret;
    aos_queue_ptr queue_ptr = AIC_NULL;

    if (aic_is_uart_com(com_id))
        queue_ptr = urc_cfg.uart_urc_cache_queue;
    else if (com_id == AT_USB_AT_PORT_ID)
        queue_ptr = urc_cfg.usb_urc_cache_queue;

    if (!queue_ptr)
        return;

    ret = aos_recv_from_queue(queue_ptr, (void*)&old_item, AOS_NO_WAIT);
    while (ret == AIC_SUCCESS) {
#ifdef CONFIG_CUSTOMER_AT
        aic_send_urc_gpio_signal();
#endif
        aic_send_to_at_port(com_id, old_item, strlen(old_item));
        ret = aos_recv_from_queue(queue_ptr, (void*)&old_item, AOS_NO_WAIT);
    };

    if (aic_is_uart_com(com_id))
        urc_cfg.uart_urc_line_cnt = 0;
    else if (com_id == AT_USB_AT_PORT_ID)
        urc_cfg.usb_urc_line_cnt = 0;
}
#endif

boolean find_msg_ending_sign(u8 *buf_ptr, u16 len)
{
    u16 i;

    for (i = 0; i < len; i++){
        if (*(buf_ptr + i) == 0x1A || *(buf_ptr + i) == 0x1B)
            return AIC_TRUE;
    }

    return AIC_FALSE;
}

u32 aic_commom_handle_cp_cmd(AT_REQ_RESP *at_req_ptr)
{
    AT_RESP_FROM_CP resp_to_save;

    resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    aic_send_cmd_and_wait(at_req_ptr->at_req, &resp_to_save);

#ifdef CONFIG_D2D
    if (aic_at_with_msg_head(at_req_ptr)) {
        if (!aic_at_transmit_msg_format_reply(at_req_ptr, resp_to_save.resp_buf,
                                               strlen(resp_to_save.resp_buf))) {
            aos_free(resp_to_save.resp_buf);
            return AIC_SUCCESS;
        } else
            goto NO_MSG_FORMAT;
    }

NO_MSG_FORMAT:
#endif
    aic_reply_to_certain_com(at_req_ptr->serial_port_id, resp_to_save.resp_buf,
                             strlen(resp_to_save.resp_buf));
    aos_free(resp_to_save.resp_buf);

    return AIC_SUCCESS;
}

#ifndef CONFIG_AT_PROC_MINI
u32 aic_handle_cmgs_exe_cmd(AT_REQ_RESP *at_req_resp)
{
    AT_RESP_FROM_CP resp_to_save;
    u16 data_len;
    u32 error_num = AIC_SUCCESS;

    resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    AIC_ASSERT(resp_to_save.resp_buf);
    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;

    if (at_req_resp->at_mode == AT_CMD_MODE) {
        aic_send_cmd_and_wait(at_req_resp->at_req, &resp_to_save);
        if (aic_is_message_medium_result(resp_to_save.resp_buf,
                                         strlen(resp_to_save.resp_buf))) {
            //alog_info("cmgs enter data mode\n");
            at_req_resp->at_mode = AT_DATA_MODE;
        }

        aic_reply_to_certain_com(at_req_resp->serial_port_id,
                                 resp_to_save.resp_buf,
                                 strlen(resp_to_save.resp_buf));
        aos_free(resp_to_save.resp_buf);
    } else if (at_req_resp->at_mode == AT_DATA_MODE) {
        if (!msg_text.text_buf) {
            msg_text.text_buf = aos_zalloc(AT_MSG_CONTENT_LEN);
            AIC_ASSERT(msg_text.text_buf);
        }

        if (!at_req_resp->end_data_mode) {
            data_len = at_req_resp->upstream_data_len;
            if (data_len <= (AT_MSG_CONTENT_LEN - msg_text.text_len)) {
                aos_memcpy((msg_text.text_buf + msg_text.text_len),
                           at_req_resp->at_upstream_data, data_len);
                msg_text.text_len += data_len;
            } else {
                error_num = AT_ERROR_MSG_TEXT_TOO_LONG;
                aos_memcpy((msg_text.text_buf + msg_text.text_len),
                           at_req_resp->at_upstream_data,
                           (AT_MSG_CONTENT_LEN - msg_text.text_len));
                msg_text.text_len = AT_MSG_CONTENT_LEN;
                aicx_error(0x14ff6e, "", "error,short message content too long\n");
            }
        } else {
            /* Add for data mode expires. */
            error_num = AT_ERROR_DATA_MODE_TIMEOUT;
            if (msg_text.text_len == 0) {
                *(msg_text.text_buf) = 0x1B;
                msg_text.text_len++;
            } else {
                if (msg_text.text_len < AT_MSG_CONTENT_LEN) {
                    *(msg_text.text_buf + msg_text.text_len) = 0x1B;
                    msg_text.text_len++;
                } else {
                    *(msg_text.text_buf + AT_MSG_CONTENT_LEN - 1) = 0x1B;
                    msg_text.text_len = AT_MSG_CONTENT_LEN;
                }
            }
        }

        /* The last char should be ctrl+z. */
        if (find_msg_ending_sign(msg_text.text_buf, msg_text.text_len) || error_num) {
            /* Set to cmd mode ahead,for maybe receive other AT cmds before
              finishing sending msg content.And they can be pended as cmds. */
            at_req_resp->at_mode = AT_CMD_MODE;
            aic_send_msg_content_and_wait(msg_text.text_buf,
                                          msg_text.text_len,
                                          &resp_to_save);

            if (error_num != AT_ERROR_DATA_MODE_TIMEOUT)
                aic_reply_to_certain_com(at_req_resp->serial_port_id,
                                         resp_to_save.resp_buf,
                                         strlen(resp_to_save.resp_buf));
        }

        aos_free(resp_to_save.resp_buf);
        if (at_req_resp->at_mode == AT_CMD_MODE) {
            if (msg_text.text_buf)
                aos_free(msg_text.text_buf);
            aos_memset(&msg_text, 0, sizeof(MSG_TEXT));
        }
    }

    return error_num;
}
#endif

#ifdef CONFIG_MISC_SUPPORT
/**
 * aic_write_manufacturer_info - Write manufacturer name into NVRAM.
 *
 * @manufacturer_name: The manufacturer name to write.
 *
 * The manufacturer name would be returned when queried by some AT cmds.
 *
 * @Return AIC_SUCCESS if the info is written successfully,or return error.
 */
s32 aic_write_manufacturer_info(MANUFACTURER_INFO *manufacturer_name)
{
    s32 ret;

    ret = aic_md_write(MD_ID_MANUFACTURER_INFO,
                       sizeof(MANUFACTURER_INFO),
                       (u8 *)manufacturer_name);
    if (ret)
        aicx_error(0x1a4e9b, "", "write manufacturer info error\n");

    return ret;
}

/**
 * aic_write_model_info - Write model information into NVRAM.
 *
 * @model_info: The model info to write.
 *
 * The model information would be returned when queried by some AT cmds.
 *
 * @Return AIC_SUCCESS if the info is written successfully,or return error.
 */
s32 aic_write_model_info(MODEL_INFO *model_info)
{
    s32 ret;

    ret = aic_md_write(MD_ID_MODEL_INFO, sizeof(MODEL_INFO), (u8 *)model_info);
    if (ret)
        aicx_error(0x13cc13, "", "write model info error\n");

    return ret;
}
#endif

#ifdef CONFIG_NET_PCAP_DUMP
/*
 * AT^CAPAPDATA: Capture the data packets transmitting
 * via the interfaces in AP.
 */
u32 aic_handle_capapdata_exe_cmd(AT_REQ_RESP *at_req_resp)
{
    u16 paras_count = 0;
    u32 len = 0;
    ulong *paras_arr = AIC_NULL;

    paras_arr = aic_split_paras_to_str_array(at_req_resp->at_req,
                                             "AT^CAPAPDATA=",
                                             &paras_count);
    if (paras_arr) {
        if (paras_count >= 1 && strncasecmp((u8 *)paras_arr[0], "mobile",
            strlen("mobile")) == 0)
            aic_network_enable_pdp_nic_capdump();

        if (paras_count >= 1 && strlen((u8 *)paras_arr[0]) == 0)
            aic_network_disable_pdp_nic_capdump();

        if (paras_count >= 2 && strncasecmp((u8 *)paras_arr[1], "usb",
            strlen("usb")) == 0)
            aic_network_enable_usb_nic_capdump();

        if (paras_count >= 2 && strlen((u8 *)paras_arr[1]) == 0)
            aic_network_disable_usb_nic_capdump();

        if (paras_count >= 3 && strncasecmp((u8 *)paras_arr[2], "ppp",
            strlen("ppp")) == 0)
            aic_network_enable_ppp_nic_capdump();

        if (paras_count >= 3 && strlen((u8 *)paras_arr[2]) == 0)
            aic_network_disable_ppp_nic_capdump();

        if (paras_count == 4) {
            if (strlen((u8 *)paras_arr[3]))
                len = atoi((u8 *)paras_arr[3]);
        }
        aic_network_config_pcap_size(len);

        aic_free_paras_mem(paras_arr, paras_count);
    } else {
        goto PARA_ERROR;
    }

    //TO DO...to capture the data packets
    aic_reply_ok_resp(at_req_resp);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_resp, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}
#endif

/* AT+CSUB: get the chip name and revision info.No parameter. */
u32 aic_handle_csub_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AIC_VERSION_LEN + 1] = {'\0'};

    /* The cmd has no parameters. */
    if (strchr(at_req_res_ptr->at_req, '='))
        goto PARA_ERROR;
#ifndef WIN32
    //TO DO...get the chip name.
    aic_get_version(result_line0, sizeof(result_line0));
#endif
    aic_transmit_at_result(at_req_res_ptr->serial_port_id, result_line0, strlen(result_line0));
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

#ifdef CONFIG_NET_TRAFFIC_CONTROL
/* AT^SPCTRL=cid,dl_max_speed,ul_max_speed
 * Control ul and dl speed of data traffic.
 */
u32 aic_handle_spctrl_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    s32 ret;
    u32 dl_max_speed = 0, ul_max_speed = 0;
    u8 interface_index = 0xff, i;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;
    struct aic_network_ip_instance *aic_ip;
    u32 err_num = AT_ERROR_INVALID_CMD;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT^SPCTRL=",
                                             &paras_count);
    if (paras_count < 2)
        goto PARA_ERROR;

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_error(0x133c36, "", "[SPCTRL] ip instance not ready\n");
        err_num = AT_ERROR_HANDLE_FAILURE;
        goto PARA_ERROR;
    }

    for (i = 0; i < NX_MAX_IP_INTERFACES; i++) {
        if (aic_ip->ip_ptr->nx_ip_interface[i].nx_interface_link_up &&
            !strcmp(aic_ip->ip_ptr->nx_ip_interface[i].nx_interface_name,
                    (u8 *)paras_arr[0]))
            break;
    }

    if (i >= NX_MAX_IP_INTERFACES)
        goto PARA_ERROR;

    interface_index = i;

    dl_max_speed = atoi((u8 *)paras_arr[1]);
    if (paras_count == 3)
        ul_max_speed = atoi((u8 *)paras_arr[2]);

    aicx_info(0x021b47, "sddd", "[SPCTRL] interface %s index %d dl %d ul %d\n", paras_arr[0],
              interface_index, dl_max_speed, ul_max_speed);
    aic_free_paras_mem(paras_arr, paras_count);

    if (dl_max_speed || ul_max_speed) {
        for (i = 0; i < AT_DATA_SPEED_CTRL_NUM; i++) {
            if ((data_sp_ctrl[i].interface_index == interface_index &&
                 data_sp_ctrl[i].active) ||
                !data_sp_ctrl[i].active) {
                data_sp_ctrl[i].interface_index = interface_index;
                data_sp_ctrl[i].dl_max_speed = dl_max_speed;
                data_sp_ctrl[i].ul_max_speed = ul_max_speed;
                data_sp_ctrl[i].active = TRUE;
                break;
            }
        }

        if (i == AT_DATA_SPEED_CTRL_NUM) {
            data_sp_ctrl[i - 1].interface_index = interface_index;
            data_sp_ctrl[i - 1].dl_max_speed = dl_max_speed;
            data_sp_ctrl[i - 1].ul_max_speed = ul_max_speed;
        }

        ret = aic_network_tc_enable(interface_index, dl_max_speed,
                                    ul_max_speed);
    } else {
        ret = aic_network_tc_disable(interface_index);
        for (i = 0; i < AT_DATA_SPEED_CTRL_NUM; i++) {
            if (data_sp_ctrl[i].interface_index == interface_index &&
                data_sp_ctrl[i].active) {
                memset(&data_sp_ctrl[i], 0, sizeof(struct DATA_SP_CTRL_STRUCT));
                break;
             }
        }
    }

    if (ret)
        aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
    else
        aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, err_num);

    return AIC_SUCCESS;
}

u32 aic_handle_spctrl_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 i;
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    for (i = 0; i < AT_DATA_SPEED_CTRL_NUM; i++) {
        if (!data_sp_ctrl[i].active)
            continue;
        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "^SPCTRL: interface %d, DL %d, ul %d",
                 data_sp_ctrl[i].interface_index,
                 data_sp_ctrl[i].dl_max_speed,
                 data_sp_ctrl[i].ul_max_speed);
        line_pointers[0] = (ulong)&result_line0[0];
        aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 1);
    }

    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}
#endif

#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
static void transmit_pdp_ip_info(AT_REQ_RESP *at_req_ptr, boolean is_urc, u8 pdp_cid)
{
    u8 result_line[100] = {'\0'};
    u8 ipv4_addr[ANET_MAX_LEN_IPV4_ADDR] = {'\0'};
    u8 ipv6_addr[ANET_MAX_LEN_HEX_IPV6_ADDR] = {'\0'};
    ulong numeric_ipv4;
    ulong numeric_ipv6[4];
    WAN_NET *wan_net = AIC_NULL;

    snprintf(result_line, sizeof(result_line), "+MIPCALL: %d,1", pdp_cid);
    wan_net = aic_get_wan_net_per_pdp(pdp_cid);
    if (!wan_net)
        return;

    numeric_ipv4 = wan_net->wan_ipv4_address;
    if (numeric_ipv4) {
        NX_CHANGE_ULONG_ENDIAN(numeric_ipv4);
        inet_ntop(AF_INET, &numeric_ipv4, ipv4_addr, ANET_MAX_LEN_IPV4_ADDR);
        strcat(result_line, ",\"");
        strncat(result_line, ipv4_addr, strlen(ipv4_addr));
        strcat(result_line, "\"");
    }

    /*Assemble ipv6. */
    if (aic_is_valid_ipv6_addr(wan_net->wan_ipv6_address1)) {
        aos_memcpy((u8 *)numeric_ipv6, (u8 *)wan_net->wan_ipv6_address1, 16);
        aic_convert_numeric_ipv6_to_str(numeric_ipv6, ipv6_addr, ANET_MAX_LEN_HEX_IPV6_ADDR);
        strcat(result_line, ",\"");
        strncat(result_line, ipv6_addr, strlen(ipv6_addr));
        strcat(result_line, "\"");
    }

    if (is_urc) {
        aic_transmit_at_cmd_urc(at_req_ptr->serial_port_id,
                                result_line, strlen(result_line));
    } else {
        aic_transmit_at_result(at_req_ptr->serial_port_id,
                               result_line, strlen(result_line));
    }
}

void aic_set_ipcall_act_flag(u8 pdp_cid)
{
    if (aic_is_valid_pdp_cid(pdp_cid)) {
        mipcall_cfg[pdp_cid - 1].mipcall_operation = ANET_PDP_TO_ACTIVATE;
        mipcall_cfg[pdp_cid - 1].act_by_mipcall = AIC_TRUE;
    }
}

/** AT+MIPCALL=<operation>,<cid> To activate or deactivate PDP.
 * Parameter <cid> is necessary.
 * If pdp is activated,send URC +MIPCALL: <cid>,<status>,<ipv4_addr>,<ipv6_addr>
 */
u32 aic_handle_mipcall_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 operation = 0xff, pdp_cid = 0xff, slp_cnt = 0, buf_len;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;
    AT_RESP_FROM_CP resp;
    u8 cmd_str[20] = {'\0'};
    u8 resp_buf[40];
    WAN_NET *wan_net = AIC_NULL;
    NET_PDP_INFO *pdp_info = AIC_NULL;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+MIPCALL=",
                                             &paras_count);
    if (!paras_arr || paras_count != 2)
        goto PARA_ERROR;

    operation = atoi((u8 *)paras_arr[0]);
    if (operation != ANET_PDP_TO_ACTIVATE && operation != ANET_PDP_TO_DEACTIVATE)
        goto PARA_ERROR;

    pdp_cid = atoi((u8 *)paras_arr[1]);
    if (!aic_is_valid_pdp_cid(pdp_cid) || aic_is_ims_pdp_cid(pdp_cid))
        goto PARA_ERROR;

    mipcall_cfg[pdp_cid - 1].mipcall_operation = operation;

    buf_len = sizeof(resp_buf);
    if (operation == ANET_PDP_TO_ACTIVATE) {
        if (aic_is_pdp_activated(pdp_cid)) {
            mipcall_cfg[pdp_cid - 1].act_by_mipcall = AIC_TRUE;
            aic_reply_ok_resp(at_req_ptr);
            transmit_pdp_ip_info(at_req_ptr, AIC_TRUE, pdp_cid);
        } else {
            snprintf(cmd_str, sizeof(cmd_str), "AT+CGACT=1,%d", pdp_cid);
            aos_memset(&resp, 0, sizeof(AT_RESP_FROM_CP));
            resp.resp_buf = resp_buf;
            resp.buf_len = buf_len;
            aic_send_cmd_and_wait(cmd_str, &resp);
            aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                     resp.resp_buf, strlen(resp.resp_buf));

            if (!strcasestr(resp.resp_buf, AT_RESULT_ERROR)) {
                mipcall_cfg[pdp_cid - 1].act_by_mipcall = AIC_TRUE;
                /* Then wait 3s for IPv6 before transmit the URC. */
                aos_sleep(500);
                pdp_info = aic_get_pdp_info_by_cid(pdp_cid);
                wan_net = aic_get_wan_net_per_pdp(pdp_cid);
                while (!pdp_info || !wan_net || (strlen(pdp_info->pdp_ipv6_suffix_hex) > 0 &&
                       !aic_is_valid_ipv6_addr(wan_net->wan_ipv6_address1))) {
                    slp_cnt++;
                    if (slp_cnt == 6)
                        break;

                    aos_sleep(500);
                    if (!pdp_info)
                        pdp_info = aic_get_pdp_info_by_cid(pdp_cid);

                    if (!wan_net)
                        wan_net = aic_get_wan_net_per_pdp(pdp_cid);
                }

                transmit_pdp_ip_info(at_req_ptr, AIC_TRUE, pdp_cid);
            }
        }
    } else if (operation == ANET_PDP_TO_DEACTIVATE) {
        if (!aic_is_pdp_activated(pdp_cid)) {
            aic_reply_ok_resp(at_req_ptr);
        } else {
            /* In case transmit URC twice. */
            mipcall_cfg[pdp_cid - 1].urc_via_cgev = AIC_FALSE;
            snprintf(cmd_str, sizeof(cmd_str), "AT+CGACT=0,%d", pdp_cid);
            aos_memset(&resp, 0, sizeof(AT_RESP_FROM_CP));
            resp.resp_buf = resp_buf;
            resp.buf_len = buf_len;
            aic_send_cmd_and_wait(cmd_str, &resp);
            aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                     resp.resp_buf, strlen(resp.resp_buf));
            if (!strcasestr(resp.resp_buf, AT_RESULT_ERROR) && mipcall_cfg[pdp_cid - 1].act_by_mipcall) {
                mipcall_cfg[pdp_cid - 1].act_by_mipcall = AIC_FALSE;
                aos_memset(resp_buf, '\0', sizeof(resp_buf));
                snprintf(resp_buf, sizeof(resp_buf), "+MIPCALL: %d,0", pdp_cid);
                aic_transmit_at_cmd_urc(at_req_ptr->serial_port_id, resp_buf, strlen(resp_buf));
            }
        }
    }

    aic_free_paras_mem(paras_arr, paras_count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

u32 aic_handle_mipcall_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 i;
    WAN_NET *wan_net;

    wan_net = aic_get_all_wan_net();
    for (i = 0; i < aic_get_max_wan_net_num(); i++) {
        if (wan_net->pdp_activated == ANET_PDP_ACTIVATED)
            transmit_pdp_ip_info(at_req_ptr, AIC_FALSE, wan_net->pdp_cid);

        wan_net++;
    }

    aic_reply_ok_resp(at_req_ptr);
    return AIC_SUCCESS;
}

u32 aic_handle_mipcall_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[40] = {'\0'};

    if (!reg_service_info.ims_enabled)
        snprintf(result_line, sizeof(result_line), "+MIPCALL: (0-1),(1-%d)",
                 ANET_MAX_PDP_CID);
    else
        snprintf(result_line, sizeof(result_line), "+MIPCALL: (0-1),(1-%d)",
                 (ANET_MAX_PDP_CID - 2));

    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}
#endif

#ifdef CONFIG_CUSTOMER_CM_AT
static boolean is_valid_customer_pdp_type(u8 *pdp_type)
{
    u8 len;

    if (!pdp_type || strlen(pdp_type) > 6)
        return AIC_FALSE;

    len = strlen(pdp_type);
    /* IP IPV6 IPV4V6 PPP Non-IP */
    if (len == 2 && strcasestr(pdp_type, "IP"))
        return AIC_TRUE;

    if (len == 3 && strcasestr(pdp_type, "PPP"))
        return AIC_TRUE;

    if (len == 4 && strcasestr(pdp_type, "IPV6"))
        return AIC_TRUE;

    if (len == 6 && (strcasestr(pdp_type, "IPV4V6") || strcasestr(pdp_type, "Non-IP")))
        return AIC_TRUE;

    return AIC_FALSE;
}

/** AT+MUECONFIG="defapn"[,<pdp_type>,<apn>],save to NV.
 * AT+MUECONFIG="autoconn"[,<autoconn>]
 */
u32 aic_handle_mueconfig_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    AT_RESP_FROM_CP resp_saved;
    u16 resp_buf_len = SIZE_1K/2;
    u8 start_mode, len = 0;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+MUECONFIG=",
                                             &paras_count);
    if (!paras_arr || paras_count == 0)
        goto PARA_ERROR;

    /* These cmds are handled by CP. */
    if (!(strcasestr((u8 *)paras_arr[0], "defapn") ||
        strcasestr((u8 *)paras_arr[0], "autoconn"))) {
        resp_saved.resp_buf = aos_zalloc(resp_buf_len);
        resp_saved.buf_len = resp_buf_len;
        aic_send_cmd_and_wait(at_req_res_ptr->at_req, &resp_saved);

        aic_reply_to_certain_com(at_req_res_ptr->serial_port_id,
                                 resp_saved.resp_buf,
                                 strlen(resp_saved.resp_buf));
        aos_free(resp_saved.resp_buf);
    } else {
        if (strncasecmp((u8 *)paras_arr[0], "defapn", strlen("defapn")) == 0) {
            if (strlen((u8 *)paras_arr[0]) != strlen("defapn") || paras_count > 3)
                goto PARA_ERROR;

            if (paras_count == 1) {
                aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
                aic_read_net_config(&net_cfg);
#endif
                snprintf(result_line0, sizeof(result_line0),
                         "+MUECONFIG: \"defapn\",\"%s\",\"%s\"",
                         net_cfg.pdp_type, net_cfg.default_apn);
                line_pointers[0] = (ulong)&result_line0[0];
                aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
            } else if (paras_count >= 2) {
                len = strlen((u8 *)paras_arr[1]);
                if (len > 0 && !is_valid_customer_pdp_type((u8 *)paras_arr[1]))
                    goto PARA_ERROR;

                aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
                aic_read_net_config(&net_cfg);
                aos_memset(net_cfg.pdp_type, 0, sizeof(net_cfg.pdp_type));
                if (len > 0)
                    strncpy(net_cfg.pdp_type, (u8 *)paras_arr[1], len);

                if (paras_count == 3) {
                    len = sizeof(net_cfg.default_apn);
                    aos_memset(net_cfg.default_apn, 0, len);
                    if (strlen((u8 *)paras_arr[2]) > 0)
                        strncpy(net_cfg.default_apn, (u8 *)paras_arr[2], (len - 1));
                }

                aic_save_config_to_nv(&net_cfg);
#endif
            }
        } else if (strncasecmp((u8 *)paras_arr[0], "autoconn", strlen("autoconn")) == 0) {
            if (strlen((u8 *)paras_arr[0]) != strlen("autoconn") || paras_count > 2)
                goto PARA_ERROR;

            if (paras_count == 1) {
                aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
                aic_read_net_config(&net_cfg);
#else
                net_cfg.auto_cfg_pdp_intf = 1;
#endif
                snprintf(result_line0, sizeof(result_line0),
                         "+MUECONFIG: \"autoconn\",%d", net_cfg.auto_cfg_pdp_intf);
                line_pointers[0] = (ulong)&result_line0[0];
                aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
            } else if (paras_count == 2) {
                start_mode = atoi((u8 *)paras_arr[1]);
                if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])) ||
                    (start_mode != 0 && start_mode != 1))
                    goto PARA_ERROR;

                aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
                aic_read_net_config(&net_cfg);
                net_cfg.auto_cfg_pdp_intf = start_mode;
                /* If not start ps,then not auto start usb dial. */
                if (start_mode == 0)
                    net_cfg.m_auto_flag = 0;

                aic_save_config_to_nv(&net_cfg);
#endif
            }
        }

        aic_reply_ok_resp(at_req_res_ptr);
    }

    aic_free_paras_mem(paras_arr, paras_count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_mueconfig_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    AT_RESP_FROM_CP resp_to_save;

    resp_to_save.resp_buf = aos_zalloc_relaxed(300);
    AOS_ASSERT(resp_to_save.resp_buf);
    resp_to_save.buf_len = 300;
    aic_send_cmd_and_wait(at_req_ptr->at_req, &resp_to_save);

    aic_reply_to_certain_com(at_req_ptr->serial_port_id, resp_to_save.resp_buf,
                             strlen(resp_to_save.resp_buf));
    aos_free(resp_to_save.resp_buf);

    return AIC_SUCCESS;
}

/* AT+MTSETID=<snt>,<value>
 * <snt> 0: Write SN;  <snt> 1: Write IMEI;  <snt> 2: Write SVN;
 * <snt> 16: Backup IMEI.
 */
u32 aic_handle_mtsetid_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 snt;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;
    s32 ret;
    SN_T sn = {0};

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+MTSETID=",
                                             &paras_count);
    if (paras_count != 2 || !aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto PARA_ERROR;

    snt = atoi((u8 *)paras_arr[0]);
    if (snt == 0) {
        if (strlen((u8 *)paras_arr[1]) > sizeof(SN_T))
            goto PARA_ERROR;

        aos_memcpy((u8 *)&sn, (u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]));
        ret = aic_md_write(MD_ID_SN0, sizeof(SN_T), (SN_T *)&sn);
        if (ret)
            aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
        else
            aic_reply_ok_resp(at_req_ptr);
    } else if (snt == 1 || snt == 2 || snt == 16) {
        aic_commom_handle_cp_cmd(at_req_ptr);
    } else {
        goto PARA_ERROR;
    }

    aic_free_paras_mem(paras_arr, paras_count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_mtsetid_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line, sizeof(result_line), "+MTSETID: (0,1,2,16)");
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}
#endif

#ifndef CONFIG_AT_PROC_MINI
/* AT+MEMINFO="heap"
 * +MEMINFO: "heap",<heap_size>,<heap_allocated>,<free>,<max_chunk>,<low_watermark>
 */
u32 aic_handle_meminfo_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;
    aos_heap_size heap_info;
    u8 result_line[80] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+MEMINFO=",
                                             &paras_count);
    if (paras_count != 1)
        goto PARA_ERROR;

    if (strlen((u8 *)paras_arr[0]) == 4 && strcasecmp((u8 *)paras_arr[0], "heap") == 0) {
        if (!aos_get_heap_size_ex(&heap_info)) {
            snprintf(result_line, sizeof(result_line),
                     "+MEMINFO: \"heap\",%d,%d,%d,%d,%d",
                     heap_info.total_heap, heap_info.total_used,
                     heap_info.total_free, heap_info.max_free_node,
                     heap_info.usage_water_line);

            aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
            aic_reply_ok_resp(at_req_ptr);
        } else {
            aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
        }
    } else {
        goto PARA_ERROR;
    }

    aic_free_paras_mem(paras_arr, paras_count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_cardstatus_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    strcpy(result_line, "^CARDSTATUS: (0,1)");
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_cardstatus_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line, sizeof(result_line), "^CARDSTATUS: %d",
             reg_service_info.cardstatus_mode);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_cardstatus_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 mode;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT^CARDSTATUS=",
                                             &paras_count);
    if (!paras_arr || paras_count != 1)
        goto PARA_ERROR;

    mode = atoi((u8 *)paras_arr[0]);
    if (mode != 0 && mode != 1)
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);
    reg_service_info.cardstatus_mode = mode;
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

void aic_init_fastslp_cfg(void)
{
    fastslp_cfg.slp_enabled = 0;
    fastslp_cfg.idle_time = 10;
    fastslp_cfg.retry_time = 120;
}

/* AT^FASTSLP=<enabled>,<idle_time>,<retry_time>
 * Enable/disable CP sleeping after sending TCP data pkts for idle_time,
 * or not recv ACK within retry_time. The time unit is second.
 */
u32 aic_handle_fastslp_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 slp_enabled;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT^FASTSLP=",
                                             &paras_count);
    if (paras_count < 1)
        goto PARA_ERROR;

    slp_enabled = atoi((u8 *)paras_arr[0]);
    if (slp_enabled != 0 && slp_enabled != 1)
        goto PARA_ERROR;

    fastslp_cfg.slp_enabled = slp_enabled;
    if (slp_enabled == 1) {
        if (paras_count >= 2)
            fastslp_cfg.idle_time = atoi((u8 *)paras_arr[1]);

        if (paras_count >= 3)
            fastslp_cfg.retry_time = atoi((u8 *)paras_arr[2]);
#ifdef CONFIG_NET_ALTE_DRIVER
        /* Enable fast sleeping. */
        aic_network_setup_enhance_low_power(fastslp_cfg.slp_enabled,
                                            fastslp_cfg.idle_time,
                                            fastslp_cfg.retry_time);
#endif
    } else {
#ifdef CONFIG_NET_ALTE_DRIVER
        /* Disable fast sleeping. */
        aic_network_setup_enhance_low_power(fastslp_cfg.slp_enabled, 0, 0);
#else
        ;
#endif
    }

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

u32 aic_handle_fastslp_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[40] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    if (fastslp_cfg.slp_enabled == 1)
        snprintf(result_line, sizeof(result_line), "^FASTSLP: %d,%d,%d",
                 fastslp_cfg.slp_enabled, fastslp_cfg.idle_time,
                 fastslp_cfg.retry_time);
    else if (fastslp_cfg.slp_enabled == 0)
        strcpy(result_line, "^FASTSLP: 0");

    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

#ifdef CONFIG_MISC_SUPPORT
static s32 read_sn_from_nv(SN_T *sn)
{
    aos_memset((u8 *)sn, 0, sizeof(SN_T));
    return aic_md_read(MD_ID_SN0, sizeof(SN_T), sn);
}
#endif

u32 aic_handle_cgsn_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr = AIC_NULL;
    u8 para;
    u8 at_cmd_head[AT_MAX_HEAD_LEN] = {'\0'};
#ifdef CONFIG_MISC_SUPPORT
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    SN_T sn;
#endif

    get_at_head_str(at_req_ptr->at_req, at_cmd_head);
    /* No parameter. */
    if (is_cr_lf_sign(*(at_req_ptr->at_req + strlen(at_cmd_head))))
        goto READ_SN;

    strcat(at_cmd_head, "=");
    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             at_cmd_head,
                                             &paras_count);
    if (!paras_arr || paras_count != 1 ||
        !aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto PARA_ERROR;

    para = atoi((u8 *)paras_arr[0]);
    /* These cmds are handled by CP. */
    if (para == 0) {
        goto READ_SN;
    } else {
        aic_commom_handle_cp_cmd(at_req_ptr);
    }

    aic_free_paras_mem(paras_arr, paras_count);
    return AIC_SUCCESS;

READ_SN:
    aic_free_paras_mem(paras_arr, paras_count);
#ifdef CONFIG_MISC_SUPPORT
    if (!read_sn_from_nv(&sn)) {
        strncpy(result_line, sn.SN, (sizeof(result_line) - 1));
        line_pointers[0] = (ulong)&result_line[0];
        aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 1);
        aic_reply_ok_resp(at_req_ptr);
    } else {
        aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
    }
#else
    aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
#endif
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

/* AT+CFUN=<fun>,<rst>  Do reset if <rst>=1,and send AT+CFUN=<fun> after reset. */
u32 aic_handle_cfun_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr = AIC_NULL;
    u8 cfun_mode, rst;
    u8 cmd_str[20] = {'\0'};
    AT_RESP_FROM_CP resp_to_save;
    NET_CFG net_cfg;
    u32 len;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+CFUN=",
                                             &paras_count);
    if (!paras_arr || (paras_count != 1 && paras_count != 2 ) ||
        !aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto PARA_ERROR;

    if (paras_count == 1) {
        aic_commom_handle_cp_cmd(at_req_ptr);
    } else if (paras_count == 2) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            goto PARA_ERROR;

        cfun_mode = atoi((u8 *)paras_arr[0]);
        rst = atoi((u8 *)paras_arr[1]);
        /* Not do reset,just send AT+CFUN=<fun> to CP. */
        if (rst == 0) {
            snprintf(cmd_str, sizeof(cmd_str), "AT+CFUN=%d", cfun_mode);
            resp_to_save.resp_buf = aos_zalloc(40);
            resp_to_save.buf_len = 40;
            aic_send_cmd_and_wait(cmd_str, &resp_to_save);
            len = strlen(resp_to_save.resp_buf);
#ifdef CONFIG_D2D
            if (aic_at_with_msg_head(at_req_ptr)) {
                if (aic_at_transmit_msg_format_reply(at_req_ptr, resp_to_save.resp_buf, len))
                    aic_reply_to_certain_com(at_req_ptr->serial_port_id, resp_to_save.resp_buf, len);
            } else
#endif
            aic_reply_to_certain_com(at_req_ptr->serial_port_id, resp_to_save.resp_buf, len);

            aos_free(resp_to_save.resp_buf);
        } else if (rst == 1) {
            /* Save it to NV. Send it to CP after reset. */
            aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
            aic_read_net_config(&net_cfg);
            net_cfg.cfun_pre_rst = cfun_mode;
            aic_save_config_to_nv(&net_cfg);
#endif
            /* Do reset. */
            aic_reply_ok_resp(at_req_ptr);
            sysoff_reset();
        } else
            goto PARA_ERROR;
    }

    aic_free_paras_mem(paras_arr, paras_count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_v0_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);
    return AIC_SUCCESS;
}

u32 aic_handle_q1_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_CMD);
    return AIC_SUCCESS;
}

/* The cmd is ATE0 or ATE1. */
u32 aic_handle_ate_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 *tmp_ptr;

    tmp_ptr = at_req_ptr->at_req + 3;
    if (*(tmp_ptr + 1) == '=')
        goto PARA_ERROR;

    if (*tmp_ptr == '0')
        at_echo_enabled = AT_CMD_ECHO_DISABLED;

    if (*tmp_ptr == '1')
        at_echo_enabled = AT_CMD_ECHO_ENABLED;

    aic_reply_ok_resp(at_req_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);
    return AIC_SUCCESS;
}

u8 aic_is_at_echo_enabled(void)
{
    return at_echo_enabled;
}

/* AT+CTZU=<onoff>
 * <onoff>--0:Disable setting system time via CTZE URC.
 * <onoff>--1:Enable setting system time via CTZE URC.
 */
u32 aic_handle_ctzu_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 onoff;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+CTZU=",
                                             &paras_count);
    if (!paras_arr || paras_count == 0) {
        aic_reply_ok_resp(at_req_ptr);
        return AIC_SUCCESS;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])) ||
        paras_count > 1)
        goto PARA_ERROR;

    onoff = atoi((u8 *)paras_arr[0]);
    if (onoff != 0 && onoff != 1)
        goto PARA_ERROR;

    reg_service_info.ctzu_mode = onoff;
    aic_free_paras_mem(paras_arr, paras_count);
    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
    net_cfg.ctzu_mode = reg_service_info.ctzu_mode;
    aic_save_config_to_nv(&net_cfg);
#endif

    aic_reply_ok_resp(at_req_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_ctzu_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[20] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line, sizeof(result_line), "+CTZU: %d", reg_service_info.ctzu_mode);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_ctzu_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[20] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    strcpy(result_line, "+CTZU: (0,1)");
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_ctcc_mode_set_cmd(AT_REQ_RESP *at_req_ptr)
{
#ifdef CONFIG_NET_EXTERNEL_API
    struct aic_network_ip_instance *aic_ip_ptr;

    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr) {
        aicx_info(0x08fedd, "", "[AT] ip instance not ready when set ctcc mode\n");
        return AIC_ECANCELED;
    }

    aicx_info(0x1d8df9, "", "[AT] set ctcc mode\n");
    aic_ip_ptr->ctcc_auth_mode = TRUE;
    aic_reply_ok_resp(at_req_ptr);
#endif
    return AIC_SUCCESS;

}

#ifdef CONFIG_NX_NAT
u32 aic_handle_tcpwin_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u32 dl_win, ul_win;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+TCPWIN=",
                                             &paras_count);
    if (paras_count < 2)
        goto PARA_ERROR;

    dl_win = atoi((u8 *)paras_arr[0]);
    ul_win = atoi((u8 *)paras_arr[1]);

    aic_nat_update_win(dl_win, ul_win);
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}
#endif

u32 aic_handle_disable_net_copy_set_cmd(AT_REQ_RESP *at_req_ptr)
{
#ifdef CONFIG_NET_ALTE_DRIVER
    extern void aic_net_set_disable_copy();
    aic_net_set_disable_copy();
#endif
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* AT^CSTM will be sent to CP,and it will change reg_mode used for test. */
u32 aic_handle_cstm_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    AT_RESP_FROM_CP resp_to_save;
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 cstm_mode, reg_mode;
    NET_CFG net_cfg;

    resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    aic_send_cmd_and_wait(at_req_ptr->at_req, &resp_to_save);

    if (!strcasestr(resp_to_save.resp_buf, AT_RESULT_ERROR)) {
        paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                                 "AT^CSTM=",
                                                 &paras_count);
        if (paras_count >= 1) {
            cstm_mode = atoi((u8 *)paras_arr[0]);
            if (cstm_mode == 2)
                reg_mode = AT_REG_INSTRU_NET;
            else
                reg_mode = AT_REG_REAL_NET;

            if (reg_mode != aic_get_reg_mode()) {
                aic_set_reg_mode(reg_mode);
                aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
                aic_read_net_config(&net_cfg);
                net_cfg.reg_mode = reg_mode;
                aic_save_config_to_nv(&net_cfg);
#endif
            }
        }

        aic_free_paras_mem(paras_arr, paras_count);
    }

    aic_reply_to_certain_com(at_req_ptr->serial_port_id, resp_to_save.resp_buf,
                             strlen(resp_to_save.resp_buf));
    aos_free(resp_to_save.resp_buf);

    return AIC_SUCCESS;
}

u32 aic_handle_qcellex_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 value;
    AT_RESP_FROM_CP resp_to_save;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+QCELLEX=", &paras_count);
    if (paras_count != 1)
        goto PARA_ERROR;

    value = atoi((u8 *)paras_arr[0]);
    if (value != 1)
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);
    resp_to_save.resp_buf = aos_zalloc(SIZE_1K);
    resp_to_save.buf_len = SIZE_1K;
    aic_send_cmd_and_wait("AT+QCELL?", &resp_to_save);
    aic_reply_to_certain_com(at_req_ptr->serial_port_id, resp_to_save.resp_buf,
                             strlen(resp_to_save.resp_buf));
    aos_free(resp_to_save.resp_buf);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}
#endif

#ifdef CONFIG_GX_AT_V2
/* AT+GNETMODE Set the way to at socket by which the MCU accesses network.  */
u32 aic_handle_gnetmode_set_cmd(AT_REQ_RESP *at_req_ptr)
{
#ifdef CONFIG_NET_ALTE_DRIVER
    aic_set_lte_nic_work_mode(AIC_LTE_AT_SOCKET_MODE);
#endif
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}
#endif

#ifdef CONFIG_D2D
D2D_MODE_CFG *aic_get_d2d_mode_cfg(void)
{
    return &d2d_mode_cfg;
}

static void save_d2d_at_com_id(u32 at_port_id)
{
    NET_CFG net_cfg;

    if (d2d_mode_cfg.at_com_id != at_port_id) {
        d2d_mode_cfg.at_com_id = at_port_id;
        aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
        aic_read_net_config(&net_cfg);
        net_cfg.d2d_at_com_id = d2d_mode_cfg.at_com_id;
        aic_save_config_to_nv(&net_cfg);
#endif
    }
}

/* AT+DMODE=<mode>
 * <mode> 0 -- LTE mode(default value), 1 -- D2D mode
 */
u32 aic_handle_dmode_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 mode;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DMODE=",
                                             &paras_count);
    if (paras_count != 1)
        goto PARA_ERROR;

    mode = atoi((u8 *)paras_arr[0]);
    if (mode != AT_UE_MODE_MOBILE && mode != AT_UE_MODE_D2D)
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);
    if (mode != d2d_mode_cfg.mode || d2d_mode_cfg.at_com_id != at_req_ptr->serial_port_id) {
        d2d_mode_cfg.at_com_id = at_req_ptr->serial_port_id;
        d2d_mode_cfg.mode = mode;
        aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
        aic_read_net_config(&net_cfg);
        net_cfg.d2d_lte_mode = mode;
        net_cfg.d2d_at_com_id = d2d_mode_cfg.at_com_id;
        aic_save_config_to_nv(&net_cfg);
#endif
    }

    if (mode == AT_UE_MODE_D2D)
        aic_set_lte_nic_work_mode(AIC_D2D_DATA_MODE);
    else
        aic_set_lte_nic_work_mode(0);

    aic_reply_ok_resp(at_req_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_dmode_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {0};

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    snprintf(result_line, sizeof(result_line), "+DMODE: %d", d2d_mode_cfg.mode);
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_d2dbw_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {0};

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    extern int aic_d2d_get_bw_mode();
    snprintf(result_line, sizeof(result_line), "+D2DBW: %d", aic_d2d_get_bw_mode());
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_d2dbw_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 mode;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+D2DBW=",
                                             &paras_count);
    if (paras_count != 1)
        goto PARA_ERROR;

    mode = atoi((u8 *)paras_arr[0]);
    if (mode != 0 && mode != 1)
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);

    extern void aic_d2d_set_bw_mode(u8 mode);
    aic_d2d_set_bw_mode(mode);
    aic_reply_ok_resp(at_req_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

/* AT+DPKTSIZE=<pkt_size> Set pkt size of bit for sending raw data */
u32 aic_handle_dpktsize_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u32 pkt_size;
    NET_CFG net_cfg;

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DPKTSIZE=",
                                             &paras_count);
    if (paras_count != 1)
        goto PARA_ERROR;

    pkt_size = atoi((u8 *)paras_arr[0]);
    if (pkt_size == 0)
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);
    if (pkt_size != d2d_mode_cfg.pkt_size) {
        d2d_mode_cfg.pkt_size = pkt_size;
        aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
        aic_read_net_config(&net_cfg);
        net_cfg.d2d_pkt_size = pkt_size;
        aic_save_config_to_nv(&net_cfg);
#endif
    }

    extern u32 aic_handle_send_packet_size_cmd(u32 at_port, u32 size);
    aic_handle_send_packet_size_cmd(at_req_ptr->serial_port_id, pkt_size);

    aic_reply_ok_resp(at_req_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_dpktsize_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {0};

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    snprintf(result_line, sizeof(result_line), "+DPKTSIZE: %d", d2d_mode_cfg.pkt_size);
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* AT+DSRAWDATA=<keep_sending>[,<pkt_num>]
 * <keep_sending> 1 -- keep sending, 0 -- non-keep sending
 * <pkt_num> if keep_sending is 0,then send packets of pkt_num
 */
u32 aic_handle_dsrawdata_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 keep_sending;
    u32 pkt_num = 0;

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DSRAWDATA=",
                                             &paras_count);
    if (paras_count == 0 || paras_count > 2)
        goto PARA_ERROR;

    keep_sending = atoi((u8 *)paras_arr[0]);
    if (keep_sending != 0 && keep_sending != 1)
        goto PARA_ERROR;

    extern u32 aic_handle_send_packet_cmd(u32 at_port, u32 keep_send, u32 send_pkt_num);
    d2d_mode_cfg.keep_sending = keep_sending;
    if (keep_sending) {
        aic_handle_send_packet_cmd(at_req_ptr->serial_port_id, TRUE, 0);
    } else {
        if (paras_count > 1)
            pkt_num = atoi((u8 *)paras_arr[1]);

        aic_handle_send_packet_cmd(at_req_ptr->serial_port_id, FALSE, pkt_num);

    }

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

/* AT+DSTOPSEND Stop sending raw data */
u32 aic_handle_dstopsend_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    extern u32 aic_handle_send_stop_cmd(u32 at_port);
    aic_handle_send_stop_cmd(at_req_ptr->serial_port_id);

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* AT+DSENDMSG=<len>,<msg> Send message  */
u32 aic_handle_dsendmsg_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u32 msg_len = 0;
    u8 *tmp_ptr1 = AIC_NULL, *tmp_ptr2 = AIC_NULL;
    u8 msg_len_str[8] = {0};

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    tmp_ptr1 = at_req_ptr->at_req + strlen("AT+DSENDMSG");
    if (*tmp_ptr1 == '=') {
        tmp_ptr1++;
        tmp_ptr2 = strchr(tmp_ptr1, ',');
        if (tmp_ptr2 && tmp_ptr2 > tmp_ptr1) {
            if ((tmp_ptr2 - tmp_ptr1) <= 7) {
                aos_memcpy(msg_len_str, tmp_ptr1, (tmp_ptr2 - tmp_ptr1));
                if (strlen(msg_len_str))
                    msg_len = atoi(msg_len_str);
            }
        }
    } else
        goto PARA_ERROR;

    if (msg_len == 0)
        goto PARA_ERROR;

    if (tmp_ptr2) {
        tmp_ptr2++;
        aic_handle_send_msg_cmd(at_req_ptr->serial_port_id, tmp_ptr2, msg_len);
    }

    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

static void clear_d2d_data_mode(void)
{
    if (d2d_mode_cfg.total_data_ptr) {
        aos_free(d2d_mode_cfg.total_data_ptr);
        d2d_mode_cfg.total_data_ptr = AIC_NULL;
        d2d_mode_cfg.data_mode_total_len = 0;
        d2d_mode_cfg.recved_data_len = 0;
    }
}

/* AT+DSVOIDATA=data_len,Enter data mode to send data. */
u32 aic_handle_dsvoidata_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u32 current_data_len, cpy_len, ret = AIC_SUCCESS;
    u32 timeout = AT_D2D_DATA_MODE_TIMEOUT;
    u16 paras_count = 0;
    ulong *paras_arr;

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    if (at_req_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                                 "AT+DSVOIDATA=",
                                                 &paras_count);
        if (paras_count >= 1) {
            d2d_mode_cfg.data_mode_total_len = atoi((u8 *)paras_arr[0]);
            if (d2d_mode_cfg.data_mode_total_len) {
                d2d_mode_cfg.total_data_ptr = AIC_NULL;
                d2d_mode_cfg.recved_data_len = 0;
            }

            if (paras_count >= 2)
                timeout = atoi((u8 *)paras_arr[1]);

            if (timeout == 0)
                timeout = AT_D2D_DATA_MODE_TIMEOUT;

            aic_set_data_mode_expire_time("AT+DSVOIDATA", timeout);
        } else {
            d2d_mode_cfg.data_mode_total_len = 0;
            d2d_mode_cfg.is_transparent_mode = 1;
        }

        aic_free_paras_mem(paras_arr, paras_count);
        aic_transmit_at_result(at_req_ptr->serial_port_id, ">", 1)
        at_req_ptr->at_mode = AT_DATA_MODE;
    } else if (at_req_ptr->at_mode == AT_DATA_MODE) {
        /* Data mode expires,then discard the data. */
        if (at_req_ptr->end_data_mode) {
            clear_d2d_data_mode();
            at_req_ptr->at_mode = AT_CMD_MODE;
            aic_reply_error_resp(at_req_ptr, AT_ERROR_DATA_MODE_TIMEOUT);
            return AIC_SUCCESS;
        }

        current_data_len = at_req_ptr->upstream_data_len;
        if (at_req_ptr->at_upstream_data == AIC_NULL || current_data_len == 0)
            return AIC_SUCCESS;

        /* For transparent mode. */
        if (d2d_mode_cfg.data_mode_total_len == 0 &&
            strncmp(at_req_ptr->at_upstream_data, "+++", 3) == 0) {
            d2d_mode_cfg.is_transparent_mode = 0;
            at_req_ptr->at_mode = AT_CMD_MODE;
            aic_reply_ok_resp(at_req_ptr);
            return AIC_SUCCESS;
        }

        if (d2d_mode_cfg.data_mode_total_len == 0) {
            ret = aic_handle_send_msg_cmd(at_req_ptr->serial_port_id,
                                          at_req_ptr->at_upstream_data,
                                          current_data_len);
        } else {
            if (d2d_mode_cfg.recved_data_len == 0 &&
                current_data_len == d2d_mode_cfg.data_mode_total_len) {
                ret = aic_handle_send_msg_cmd(at_req_ptr->serial_port_id,
                                              at_req_ptr->at_upstream_data,
                                              current_data_len);
                d2d_mode_cfg.data_mode_total_len = 0;
                at_req_ptr->at_mode = AT_CMD_MODE;
                aic_reply_ok_resp(at_req_ptr);
            } else {
                if (!d2d_mode_cfg.total_data_ptr) {
                    d2d_mode_cfg.total_data_ptr = aos_zalloc_relaxed(d2d_mode_cfg.data_mode_total_len);
                    AOS_ASSERT(d2d_mode_cfg.total_data_ptr);
                }

                if ((d2d_mode_cfg.recved_data_len + current_data_len) <= d2d_mode_cfg.data_mode_total_len)
                    cpy_len = current_data_len;
                else
                    cpy_len = d2d_mode_cfg.data_mode_total_len - d2d_mode_cfg.recved_data_len;

                aos_memcpy((d2d_mode_cfg.total_data_ptr + d2d_mode_cfg.recved_data_len),
                           at_req_ptr->at_upstream_data, cpy_len);
                d2d_mode_cfg.recved_data_len += cpy_len;

                if (d2d_mode_cfg.recved_data_len == d2d_mode_cfg.data_mode_total_len) {
                    ret = aic_handle_send_msg_cmd(at_req_ptr->serial_port_id,
                                                  d2d_mode_cfg.total_data_ptr,
                                                  d2d_mode_cfg.recved_data_len);
                    clear_d2d_data_mode();
                    at_req_ptr->at_mode = AT_CMD_MODE;
                    aic_reply_ok_resp(at_req_ptr);
                }
            }
        }

        aicx_info(0x0d0dda, "x", "send voice data ret 0x%x\n", ret);
    }

    return AIC_SUCCESS;
}

/* Send data and msg received to at port. */
u32 aic_d2d_report_downlink_data(u8 *data_ptr, u32 data_len)
{
    u8 temp_buf[30] = {0};
    u8 str_len;
    u32 total_len, ret;
    u8 *total_buf_ptr;

    if (data_len > AT_D2D_MSG_MAX_LEN)
        snprintf(temp_buf, sizeof(temp_buf), "+DRVOIDATA: %d,", data_len);
    else
        snprintf(temp_buf, sizeof(temp_buf), "+DREVMSG: %d,", data_len);

    str_len = strlen(temp_buf);
    total_len = str_len + data_len;
    total_buf_ptr = aos_dma_malloc(total_len);
    if (!total_buf_ptr)
        return AT_ERROR_MALLOC_FAIL;

    aos_memcpy(total_buf_ptr, temp_buf, str_len);
    aos_memcpy((total_buf_ptr + str_len), data_ptr, data_len);
    ret = aic_transmit_at_cmd_urc(aic_get_d2d_urc_com_id(), total_buf_ptr, total_len);
    aos_free(total_buf_ptr);

    return ret;
}

/* AT+DVOISTUB="SEND/RECV"
 * If "SEND",just save the data to a file;
 * If "RECV",send data in a file to at port.
 */
u32 aic_handle_dvoistub_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u32 period, pkt_size, pkt_cnt;

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DVOISTUB=",
                                             &paras_count);

    if (paras_count < 1)
        goto PARA_ERROR;

    extern int aic_handle_voice_stub_cmd(u32 at_port, u8 *cmd, u32 packet_size,
                                         u32 packet_count, u32 period);
    if (strcasestr((u8 *)paras_arr[0], "SEND")) {
        aic_handle_voice_stub_cmd(at_req_ptr->serial_port_id, "SEND", 0, 0, 0);
    } else if (strcasestr((u8 *)paras_arr[0], "RECV")) {
        if (paras_count == 4) {
            period = atoi((u8 *)paras_arr[1]);
            pkt_cnt = atoi((u8 *)paras_arr[2]);
            pkt_size = atoi((u8 *)paras_arr[3]);
            aic_handle_voice_stub_cmd(at_req_ptr->serial_port_id, "RECV",
                                      pkt_size, pkt_cnt, period);
        } else
            goto PARA_ERROR;
    }

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);
    return AIC_SUCCESS;
}

/* AT+DROLE=0/1 0:send role, 1:recv role */
u32 aic_handle_drole_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 role;
    NET_CFG net_cfg;
    AT_RESP_FROM_CP resp_to_save;
    u32 len;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DROLE=",
                                             &paras_count);
    if (paras_count != 1)
        goto PARA_ERROR;

    role = atoi((u8 *)paras_arr[0]);
    aic_free_paras_mem(paras_arr, paras_count);
    resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    aic_send_cmd_and_wait(at_req_ptr->at_req, &resp_to_save);
    if (strcasestr(resp_to_save.resp_buf, AT_RESULT_ERROR)) {
        len = strlen(resp_to_save.resp_buf);
#ifdef CONFIG_D2D
        if (aic_at_with_msg_head(at_req_ptr)) {
            if (aic_at_transmit_msg_format_reply(at_req_ptr, resp_to_save.resp_buf, len))
                aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                         resp_to_save.resp_buf, len);
        } else
#endif
        aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                 resp_to_save.resp_buf, len);

        aos_free(resp_to_save.resp_buf);
        return AIC_SUCCESS;
    }

    aos_free(resp_to_save.resp_buf);
    if (role != d2d_mode_cfg.role || d2d_mode_cfg.at_com_id != at_req_ptr->serial_port_id) {
        d2d_mode_cfg.at_com_id = at_req_ptr->serial_port_id;
        d2d_mode_cfg.role = role;
        aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
        aic_read_net_config(&net_cfg);
        net_cfg.d2d_role = role;
        net_cfg.d2d_at_com_id = d2d_mode_cfg.at_com_id;
        aic_save_config_to_nv(&net_cfg);
#endif
    }

    extern int aic_handle_set_role_cmd(u32 at_port, u32 role);
    aic_handle_set_role_cmd(at_req_ptr->serial_port_id, role);

    aic_reply_ok_resp(at_req_ptr);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_drole_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {0};

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    snprintf(result_line, sizeof(result_line), "+DROLE: %d", d2d_mode_cfg.role);
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* AT+DPKTDUMP=<enabled>
 * <enabled> 0 -- disable pkt dump, 1 -- enable pkt dump
 */
u32 aic_handle_dpktdump_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 pkt_dump;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DPKTDUMP=",
                                             &paras_count);
    if (paras_count >= 1) {
        pkt_dump = atoi((u8 *)paras_arr[0]);
        if (pkt_dump != d2d_mode_cfg.enable_pkt_dump) {
            d2d_mode_cfg.enable_pkt_dump = pkt_dump;
            aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
            aic_read_net_config(&net_cfg);
            net_cfg.d2d_pkt_dump = pkt_dump;
            aic_save_config_to_nv(&net_cfg);
#endif
            extern int aic_handle_dump_data_cmd(u32 at_port, u32 cmd);
            aic_handle_dump_data_cmd(at_req_ptr->serial_port_id, pkt_dump);
        }
    }

    save_d2d_at_com_id(at_req_ptr->serial_port_id);

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_dpktdump_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {0};

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    snprintf(result_line, sizeof(result_line), "+DPKTDUMP: %d", d2d_mode_cfg.enable_pkt_dump);
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* AT+DREPDR=<enable_rep_dr>
 * <enable_rep_dr> 0 -- disable reporting data rate, 1 -- enable reporting data rate
 */
u32 aic_handle_drepdr_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 enable_rep_dr;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DREPDR=",
                                             &paras_count);
    if (paras_count >= 1) {
        enable_rep_dr = atoi((u8 *)paras_arr[0]);
        if (enable_rep_dr != d2d_mode_cfg.enable_rep_dr) {
            d2d_mode_cfg.enable_rep_dr = enable_rep_dr;
            extern u32 aic_handle_report_data_rate_cmd(u32 at_port, u32 enable);
            aic_handle_report_data_rate_cmd(at_req_ptr->serial_port_id, enable_rep_dr);
        }
    }

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* AT+DREFC=<enable_rep_fc>
 * <enable_rep_fc> 0 -- disable reporting flow ctrl, 1 -- enable reporting flow ctrl
 */
u32 aic_handle_drepfc_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 enable_rep_fc;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DREPFC=",
                                             &paras_count);
    if (paras_count >= 1) {
        enable_rep_fc = atoi((u8 *)paras_arr[0]);
        extern u32 aic_handle_enable_flow_control_cmd(u32 at_port, u32 enable);
        aic_handle_enable_flow_control_cmd(at_req_ptr->serial_port_id, enable_rep_fc);
    }

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_drepdr_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[30] = {0};

    save_d2d_at_com_id(at_req_ptr->serial_port_id);
    snprintf(result_line, sizeof(result_line), "+DREPDR: %d", d2d_mode_cfg.enable_rep_dr);
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* AT+DDBGCFG=<enable_urc_msg_head>[,urc_com_type]
 * <enable_urc_msg_head>:if the urc contains msg head;
 * <urc_com_type>:string type,"cmux" or "usb" or "uart"
 * Default config: AT+DDBGCFG=1,"cmux"
 * For debug: AT+DDBGCFG=0/1,"cmux"/"usb"/"uart"
 */
u32 aic_handle_ddbgcfg_set_cmd(AT_REQ_RESP *at_req_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 enabled;
    boolean cfg_changed = AIC_FALSE;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req,
                                             "AT+DDBGCFG=",
                                             &paras_count);
    if (paras_count >= 2) {
        if (strncasecmp(d2d_mode_cfg.at_com_type, (u8 *)paras_arr[1],
                        strlen(d2d_mode_cfg.at_com_type)) != 0) {
            if (strcasestr((u8 *)paras_arr[1], AT_D2D_COM_TYPE_CMUX)) {
#ifdef CONFIG_CMUX
                aic_set_d2d_urc_com_id(AT_D2D_URC_CMUX_COM_ID);
#else
                goto PARA_ERROR;
#endif
            } else if (strcasestr((u8 *)paras_arr[1], AT_D2D_COM_TYPE_USB))
                aic_set_d2d_urc_com_id(AT_USB_AT_PORT_ID);
#ifdef CONFIG_CMUX
            else if (strcasestr((u8 *)paras_arr[1], AT_D2D_COM_TYPE_UART) &&
                     conn_get_uart_at_conid == AIC_ENODEV)
                goto PARA_ERROR;
#else
            else if (strcasestr((u8 *)paras_arr[1], AT_D2D_COM_TYPE_UART))
                aic_set_d2d_urc_com_id(conn_get_uart_at_conid);
#endif

            cfg_changed = AIC_TRUE;
            strncpy(d2d_mode_cfg.at_com_type, (u8 *)paras_arr[1],
                    (sizeof(d2d_mode_cfg.at_com_type) - 1));
        }
    }

    if (paras_count >= 1) {
        enabled = atoi((u8 *)paras_arr[0]);
        if (enabled != aic_is_urc_msg_head_enabled()) {
            cfg_changed = AIC_TRUE;
            aic_set_urc_msg_head_enabled(enabled);
        }
    }

    if (cfg_changed) {
        aos_memset(&net_cfg, 0, sizeof(NET_CFG));
        aic_read_net_config(&net_cfg);
        net_cfg.d2d_msg_head_enabled = d2d_mode_cfg.msg_head_enabled;
        strncpy(net_cfg.d2d_com_type, d2d_mode_cfg.at_com_type,
                (sizeof(d2d_mode_cfg.at_com_type) - 1));
        aic_save_config_to_nv(&net_cfg);
    }

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u32 aic_handle_ddbgcfg_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    u8 result_line[40] = {0};

    snprintf(result_line, sizeof(result_line), "+DDBGCFG: %d,\"%s\"",
             aic_is_urc_msg_head_enabled(), d2d_mode_cfg.at_com_type);
    aic_transmit_at_result(at_req_ptr->serial_port_id, result_line, strlen(result_line));
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

u8 aic_is_urc_msg_head_enabled(void)
{
    return d2d_mode_cfg.msg_head_enabled;
}

void aic_set_urc_msg_head_enabled(u8 enabled)
{
    aicx_info(0x0d1b70, "d", "set urc msg head enabled: %d\n", enabled);
    d2d_mode_cfg.msg_head_enabled = enabled;
}

u32 aic_get_d2d_urc_com_id(void)
{
    return d2d_mode_cfg.urc_com_id;
}

void aic_set_d2d_urc_com_id(u32 port_id)
{
    aicx_info(0x1a9cfd, "d", "set d2d urc com id: %d\n", port_id);
    d2d_mode_cfg.urc_com_id = port_id;
}
#endif

