/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Serial Input AT Cmds Parsing Component.
 *  This is the component of assembling,sending and
 *  invoking handler of AT commands.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1cb6f5
#define LOG_MODULE    AMOD_AT

#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_size.h>
#include <aic_errno.h>
#include <aic_version.h>
#include <connmgr_api.h>
#include <AIC_DEBUG.h>
#include "slp_inc.h"
#include "aic_at_cmd_handle.h"
#include "aic_general_at_handle.h"
#include "aic_at_interact_cp.h"
#include "aic_at_resp_handle.h"
#include "aic_serial_recv_send.h"
#include "aic_at_urc_handle.h"
#include "aic_at_parse_utils.h"
#if CONFIG_GX_AT_V1 || CONFIG_GX_AT_V2 || CONFIG_CUSTOMER_AT
#include "aic_at_tcpip_common.h"
#endif
#ifdef CONFIG_PDP_MGR
#include "aic_net_pdp_mgr.h"
#endif

#ifdef CONFIG_PPP_DIAL
#include "aic_ppp_dial_at_handle.h"
#endif

/* Define global vars. */
aos_event_ptr event_dispatch_at;
aos_timer_ptr at_data_mode_timer;
boolean multi_at_com_inited = AIC_FALSE;
struct list_head multi_at_com_list;

/* Define internal vars. */
#ifndef CONFIG_AT_PROC_MINI
/* Save the AT structs in AT data mode for each serial port. */
static SERIAL_DATA_MODE_AT data_mode_at_list[AT_MAX_SERIAL_PORT_NUM];
/* The timeout value can be set. */
static DATA_MODE_EXPIRE_TIME data_mode_timeouts[AT_DATA_MODE_TIMEOUT_CNT];
#endif
static u8 old_cr_lf_signs[2] = {0x0D, 0x0A};
static u8 current_cr_lf_signs[2] = {0x0D, 0x0A};
/* Save all lists of at cmd hanle sorted by length of the at cmd head. */
static struct list_head at_cmd_handle_lists[AT_MAX_HEAD_LEN];
static boolean at_list_head_inited = AIC_FALSE;
static AT_HANDLE_ITEM *at_handle_items_ptr = AIC_NULL;
#ifdef CONFIG_CUSTOMER_AT
static aos_timer_ptr tmp_cmd_mode_at_timer;
#endif
#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
static ASYNC_DATA_MODE_AT at_aync_data_mode;
#endif

static AT_CMD_HANDLE general_at_handles[] =
{
#ifdef CONFIG_USB_SERVICE
    {"AT^USBCFG", aic_handle_usbcfg_test_cmd, aic_handle_usbcfg_query_cmd,
     aic_handle_usbcfg_set_cmd, AIC_NULL},
#endif

#ifndef CONFIG_AT_PROC_MINI
    {"AT+CGMI", aic_handle_cgmi_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_cgmi_exe_cmd},

    {"AT+GMI", aic_handle_cgmi_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_cgmi_exe_cmd},

    {"AT+CGMM", aic_handle_cgmm_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_cgmm_exe_cmd},

    {"AT+GMM", aic_handle_cgmm_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_cgmm_exe_cmd},

    {"AT+CGMR", aic_handle_cgmr_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_cgmr_exe_cmd},

    {"AT+GMR", aic_handle_cgmr_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_cgmr_exe_cmd},

    {"ATI", aic_handle_ati_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_ati_exe_cmd},

    {"AT+IFC", aic_handle_ifc_test_cmd, aic_handle_ifc_query_cmd,
     aic_handle_ifc_set_cmd, AIC_NULL},

    {"AT+IPR", aic_handle_ipr_test_cmd, aic_handle_ipr_query_cmd,
     aic_handle_ipr_set_cmd, AIC_NULL},
    {"AT^FASTSLP", AIC_NULL, aic_handle_fastslp_query_cmd,
     aic_handle_fastslp_set_cmd, AIC_NULL},

    {"AT+CTZU", aic_handle_ctzu_test_cmd, aic_handle_ctzu_query_cmd,
     aic_handle_ctzu_set_cmd, AIC_NULL},
#endif

#ifdef CONFIG_NET_TRAFFIC_CONTROL
    {"AT^SPCTRL", AIC_NULL, aic_handle_spctrl_query_cmd,
     aic_handle_spctrl_set_cmd, AIC_NULL},
#endif

#if defined CONFIG_CUSTOMER_AT || defined CONFIG_CUSTOMER_JQ_AT
    {"AT+QURCCFG", aic_handle_qurccfg_test_cmd, aic_handle_qurccfg_query_cmd,
     aic_handle_qurccfg_set_cmd, AIC_NULL},
#endif

#ifdef CONFIG_CUSTOMER_JQ_AT
    {"AT+QCFG", AIC_NULL, AIC_NULL, aic_handle_qcfg_ri_set_cmd, AIC_NULL},
#endif

#ifdef CONFIG_CUSTOMER_AT
    {"AT+QICSGP", aic_handle_qicsgp_test_cmd, AIC_NULL,
     aic_handle_qicsgp_set_cmd, AIC_NULL},

    {"AT+QIACT", aic_handle_qiact_test_cmd, aic_handle_qiact_query_cmd,
     AIC_NULL, aic_handle_qiact_exe_cmd},

    {"AT+QCFG", aic_handle_qcfg_test_cmd, AIC_NULL,
     aic_handle_qcfg_set_cmd, AIC_NULL},

    {"AT+QIDEACT", aic_handle_qideact_test_cmd, AIC_NULL,
     AIC_NULL, aic_handle_qideact_exe_cmd},

    {"ATO", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_ato_exe_cmd},

    {"AT+IPREX", aic_handle_iprex_test_cmd, aic_handle_iprex_query_cmd,
     aic_handle_iprex_set_cmd, AIC_NULL},
#endif

#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    {"AT+MIPCALL", aic_handle_mipcall_test_cmd, aic_handle_mipcall_query_cmd,
     AIC_NULL, aic_handle_mipcall_exe_cmd},
#endif
#ifdef CONFIG_CUSTOMER_CM_AT
    {"AT+MUECONFIG", aic_handle_mueconfig_test_cmd, AIC_NULL,
     aic_handle_mueconfig_set_cmd, AIC_NULL},
    {"AT+MTSETID", aic_handle_mtsetid_test_cmd, AIC_NULL,
     aic_handle_mtsetid_set_cmd, AIC_NULL},
#endif

#ifdef CONFIG_GX_AT_V2
    {"AT+GNETMODE", AIC_NULL, AIC_NULL, aic_handle_gnetmode_set_cmd, AIC_NULL},
#endif
#ifndef CONFIG_AT_PROC_MINI
    {"AT+QCELLEX", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_qcellex_exe_cmd},
    {"AT+GNETNCOPY", AIC_NULL, AIC_NULL, aic_handle_disable_net_copy_set_cmd, AIC_NULL},
    {"AT+CTCCAUTHMODE", AIC_NULL, AIC_NULL, aic_handle_ctcc_mode_set_cmd, AIC_NULL},
#ifdef CONFIG_NX_NAT
    {"AT+TCPWIN", AIC_NULL, AIC_NULL, aic_handle_tcpwin_set_cmd, AIC_NULL},
#endif
    {"AT+MEMINFO", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_meminfo_exe_cmd},
    {"AT^CARDSTATUS", aic_handle_cardstatus_test_cmd, aic_handle_cardstatus_query_cmd,
     aic_handle_cardstatus_set_cmd, AIC_NULL},
    {"AT+CGSN", aic_commom_handle_cp_cmd, aic_commom_handle_cp_cmd,
     aic_handle_cgsn_exe_cmd, AIC_NULL},
    {"AT+GSN", aic_commom_handle_cp_cmd, aic_commom_handle_cp_cmd,
     aic_handle_cgsn_exe_cmd, AIC_NULL},
    {"AT+CMGS", aic_commom_handle_cp_cmd, aic_commom_handle_cp_cmd,
     aic_handle_cmgs_exe_cmd, AIC_NULL},
    {"AT+CMGW", aic_commom_handle_cp_cmd, aic_commom_handle_cp_cmd,
     aic_handle_cmgs_exe_cmd, AIC_NULL},
    {"AT+CMGC", aic_commom_handle_cp_cmd, aic_commom_handle_cp_cmd,
     aic_handle_cmgs_exe_cmd, AIC_NULL},

    {"AT+CFUN", aic_commom_handle_cp_cmd, aic_commom_handle_cp_cmd,
     aic_handle_cfun_exe_cmd, AIC_NULL},
#endif

    {"ATS3", aic_send_at_to_cp, aic_send_at_to_cp, aic_handle_s3_set_cmd,
     AIC_NULL},

    {"ATS4", aic_send_at_to_cp, aic_send_at_to_cp, aic_handle_s4_set_cmd,
     AIC_NULL},

#ifndef CONFIG_AT_PROC_MINI
    {"AT^REGMODE", aic_handle_regmode_test_cmd, aic_handle_regmode_query_cmd,
     aic_handle_regmode_set_cmd, AIC_NULL},
    {"AT+CSUB", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_csub_exe_cmd},
    {"ATV0", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_v0_exe_cmd},
    {"ATQ1", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_q1_exe_cmd},
    {"ATE0", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_ate_set_cmd},
    {"ATE1", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_ate_set_cmd},

    {"AT^AUTOSTARTPS", aic_handle_autostartps_test_cmd,
     aic_handle_autostartps_query_cmd, aic_handle_autostartps_set_cmd, AIC_NULL},

    {"AT^AUTOUSBNET", aic_handle_autousbnet_test_cmd,
     aic_handle_autousbnet_query_cmd, aic_handle_autousbnet_set_cmd, AIC_NULL},

    {"AT+CCLK", aic_handle_cclk_test_cmd, aic_handle_cclk_query_cmd,
     aic_handle_cclk_set_cmd, AIC_NULL},

    {"AT^CSTM", aic_commom_handle_cp_cmd, aic_commom_handle_cp_cmd,
     AIC_NULL, aic_handle_cstm_set_cmd},
#endif

#ifdef CONFIG_NET_PCAP_DUMP
    {"AT^CAPAPDATA", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_capapdata_exe_cmd},
#endif

#ifdef CONFIG_D2D
    {"AT+DMODE", AIC_NULL, aic_handle_dmode_query_cmd, aic_handle_dmode_set_cmd, AIC_NULL},
    {"AT+D2DBW", AIC_NULL, aic_handle_d2dbw_query_cmd, aic_handle_d2dbw_set_cmd, AIC_NULL},
    {"AT+DPKTSIZE", AIC_NULL, aic_handle_dpktsize_query_cmd, aic_handle_dpktsize_set_cmd, AIC_NULL},
    {"AT+DSRAWDATA", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_dsrawdata_exe_cmd},
    {"AT+DSTOPSEND", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_dstopsend_exe_cmd},
    {"AT+DSENDMSG", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_dsendmsg_exe_cmd},
    {"AT+DPKTDUMP", AIC_NULL, aic_handle_dpktdump_query_cmd, aic_handle_dpktdump_set_cmd, AIC_NULL},
    {"AT+DREPDR", AIC_NULL, aic_handle_drepdr_query_cmd, aic_handle_drepdr_set_cmd, AIC_NULL},
    {"AT+DREPFC", AIC_NULL, AIC_NULL, aic_handle_drepfc_set_cmd, AIC_NULL},
    {"AT+DSVOIDATA", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_dsvoidata_exe_cmd},
    {"AT+DVOISTUB", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_dvoistub_exe_cmd},
    {"AT+DROLE", AIC_NULL, aic_handle_drole_query_cmd, AIC_NULL, aic_handle_drole_set_cmd},
    {"AT+DDBGCFG", AIC_NULL, aic_handle_ddbgcfg_query_cmd, AIC_NULL, aic_handle_ddbgcfg_set_cmd},
#endif

    {AIC_NULL},
};

/* Define internal functions. */
static u32 handle_invalid_at_cmd(AT_REQ_RESP *at_req_resp_ptr);
static void serial_at_dispatch_entry(void *thread_input);
static void dispatch_serial_at(AT_REQ_RESP *at_req_resp);
static void invoke_at_cmd_handler(AT_REQ_RESP *at_req_resp);
#ifndef CONFIG_AT_PROC_MINI
static void set_serial_data_mode_at(AT_REQ_RESP *at_req_resp_ptr);
static void clear_serial_data_mode_at(AT_REQ_RESP *at_req_resp_ptr);
static void at_data_mode_timeout(void *input);
#endif
static u32 append_to_pending_at(AT_REQ_RESP *at_req_resp,
                                AT_REQ_RESP *head_ptr);
static AT_CMD_HANDLE *get_at_cmd_handle(u8 *at_cmd_str);
static boolean is_time_consuming_cmd(u8 *at_ptr);
static boolean is_ppp_dial_cmd(u8 *cmd_str);
static boolean is_pre_handled_by_ap(u8 *at_ptr, u8 cmd_type);
static void init_at_handle_list(void);
static AT_HANDLE_ITEM * get_at_handle_item_ptr(void);
static u32 init_sending_cmd_to_cp(AT_REQ_RESP *at_req_resp_ptr, u8 *at_str);
static boolean is_ten_seconds_cmd(u8 *at_ptr);
#ifdef CONFIG_CUSTOMER_AT
static void check_tmp_mode_at_by_port(u32 at_port_id);
static void start_tmp_cmd_mode_at_timer(void);
static void stop_tmp_cmd_mode_at_timer(void);
static void tmp_cmd_mode_at_timeout(void *input);
#endif

/* Define external vars. */
extern AT_REQ_RESP *at_pending_to_send;
extern aos_event_ptr event_send_at;
extern AT_ABUF_LINK_CFG_SET abuf_link_cfg_set;
extern aos_mutex_ptr mutex_at_pending;
extern DATA_SP_CTRL data_sp_ctrl[AT_DATA_SPEED_CTRL_NUM];
extern AT_PORT_INFO at_ports_info[AT_MAX_SERIAL_PORT_NUM];
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
extern MIPCALL_CFG mipcall_cfg[ANET_PDP_MAX_NUM];
extern boolean aic_is_transparent_mode(void);
#endif
#ifdef CONFIG_CUSTOMER_CM_AT
extern AIC_DEBUG s_debug_cfg;
#endif
#ifdef CONFIG_D2D
extern D2D_MODE_CFG d2d_mode_cfg;
#endif

/**
 * aic_at_init - Init AT framework, including receiving AT cmds from
 * the serial port, adding general at cmd handles,interacting with CP,
 * handling response received from CP and trransmitting URC to the
 * upper device etc..
 */
s32 aic_at_init(void)
{
    aic_serial_recv_init();
    aic_serial_at_dispatch_init();
    aic_at_interact_cp_init();
    aic_at_cmd_ap_init();
    aic_net_at_resp_init();
    aic_at_urc_handle_init();

    return AIC_SUCCESS;
}
AIC_APP_P2_INIT_EXPORT(aic_at_init);

void aic_at_cmd_ap_init(void)
{
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    u8 i;
#endif

    aic_add_at_cmd_handle(&general_at_handles[0]);
#if CONFIG_CUSTOMER_AT || CONFIG_CUSTOMER_JQ_AT
    aic_init_urc_cfg();
#endif
    aos_memset(data_sp_ctrl, 0, (sizeof(DATA_SP_CTRL) * AT_DATA_SPEED_CTRL_NUM));
#ifndef CONFIG_AT_PROC_MINI
    aic_init_fastslp_cfg();
#endif

#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
    at_aync_data_mode.pre_data_mutex = aos_create_mutex("pre_data");
    AIC_ASSERT(at_aync_data_mode.pre_data_mutex);
#endif
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    for (i = 0; i < ANET_PDP_MAX_NUM; i++)
        mipcall_cfg[i].urc_via_cgev = AIC_TRUE;
#endif
#ifdef CONFIG_CUSTOMER_CM_AT
    if (s_debug_cfg.aic_at.baudrate > 0)
        conn_cmd_set_baud_rate(CONN_CMD_AT, conn_get_uart_at_conid,
                               s_debug_cfg.aic_at.baudrate, AIC_FALSE);
#endif
}

/**
 * aic_add_at_cmd_handle - Add AT_CMD_HANDLE for at cmds handled by AP.
 *
 * @head_handle_ptr: The pointer to the first cmd handle.
 *
 * If the at cmds are handled by AP,then their cmd handles should be added
 * to at_cmd_handle_list by the function.
 *
 * @Return SUCCESS if the cmd handles are successfully added to
 *  at_cmd_handle_list. Return ERROR if at_cmd_handle_list is full.
 */
u32 aic_add_at_cmd_handle(AT_CMD_HANDLE *head_handle_ptr)
{
    AT_CMD_HANDLE *tmp_handle_ptr;
    u8 at_head_len = 0;
    AT_HANDLE_ITEM *handle_item;
    u32 total_size = 0;
    AOS_INTERRUPT_DECLARE

    if (!head_handle_ptr)
        return AT_ERROR_INVALID_AT_HANDLE;

    /* Malloc a big piece of memory,or many small pieces will be malloced. */
    if (!at_handle_items_ptr) {
        total_size = sizeof(AT_HANDLE_ITEM) * AT_MAX_CMDS_AP_HANDLED;
        at_handle_items_ptr = (AT_HANDLE_ITEM *)aos_zalloc_relaxed(total_size);
        AIC_ASSERT(at_handle_items_ptr);
    }

    init_at_handle_list();

    tmp_handle_ptr = head_handle_ptr;
    while (tmp_handle_ptr->at_cmd != AIC_NULL) {
        at_head_len = strlen(tmp_handle_ptr->at_cmd);
        if (at_head_len > AT_MAX_HEAD_LEN || at_head_len == 0) {
            aicx_error(0x1a9588, "s", "AT head %s too long\n", tmp_handle_ptr->at_cmd);
            continue;
        }

        handle_item = get_at_handle_item_ptr();
        handle_item->at_handle = tmp_handle_ptr;
        AOS_INTERRUPT_DISABLE
        list_add_tail(&handle_item->list_key,
                      &(at_cmd_handle_lists[at_head_len - 1]));
        AOS_INTERRUPT_RESTORE

        tmp_handle_ptr++;
    }

    return AIC_SUCCESS;
}

AT_HANDLE_ITEM * get_at_handle_item_ptr(void)
{
    AT_HANDLE_ITEM *handle_item_ptr = AIC_NULL, *tmp_item_ptr = AIC_NULL;
    u16 i;

    tmp_item_ptr = at_handle_items_ptr;
    for (i = 0; i < AT_MAX_CMDS_AP_HANDLED; i++) {
        if (tmp_item_ptr->at_handle == 0) {
            handle_item_ptr = tmp_item_ptr;
            break;
        }
        i++;
        tmp_item_ptr++;
    }

    if (!handle_item_ptr)
        handle_item_ptr = (AT_HANDLE_ITEM *)aos_zalloc_relaxed(sizeof(AT_HANDLE_ITEM));

    return handle_item_ptr;
}

u32 aic_serial_at_dispatch_init(void)
{
#ifndef CONFIG_AT_PROC_MINI
    u8 i;

    for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
        data_mode_at_list[i].serial_conn_id = AT_NO_SERIAL_PORT_ID;
        data_mode_at_list[i].data_mode_at = AIC_NULL;
    }
#endif

    event_dispatch_at = aos_create_event("dispatch_at");
    AIC_ASSERT(event_dispatch_at);

    return AIC_SUCCESS;
}

/* The task to dispatch AT received from serial ports. */
void serial_at_dispatch_entry(void *thread_input)
{
    u32 at_events = 0;
    AT_REQ_RESP *at_req_resp, *serial_dispatch_at;
    u32 at_com_id;
    AT_RECV_COM_ITEM *at_com_item = AIC_NULL;
    AOS_INTERRUPT_DECLARE

    at_com_item = (AT_RECV_COM_ITEM *)thread_input;
    if (!at_com_item)
        return;

    at_com_id = at_com_item->at_com_id;
    while (1) {
        aos_get_event(event_dispatch_at, at_com_item->dispatch_event_id, AOS_OR_CLEAR,
                      &at_events, AOS_WAIT_FOREVER);

        if (at_events & at_com_item->dispatch_event_id) {
            serial_dispatch_at = at_com_item->serial_dispatch_at;
            if (!serial_dispatch_at)
                continue;

            if (serial_dispatch_at->at_handle_state == AT_EXT_CMD_HANDLED_OVER) {
                AOS_INTERRUPT_DISABLE
                at_req_resp = serial_dispatch_at->next_ptr;
                at_com_item->serial_dispatch_at = at_req_resp;
                AOS_INTERRUPT_RESTORE

                if (at_com_item->serial_dispatch_at)
                    dispatch_serial_at(at_com_item->serial_dispatch_at);

            } else if (serial_dispatch_at->at_handle_state == AT_EXT_CMD_NOT_HANDLED) {
                dispatch_serial_at(serial_dispatch_at);
            } else if (serial_dispatch_at->at_handle_state == AT_EXT_CMD_HANDLING) {
                if (serial_dispatch_at->at_mode == AT_DATA_MODE) {
                    dispatch_serial_at(serial_dispatch_at);
                } else if (serial_dispatch_at->at_mode == AT_CMD_MODE) {
                    aicx_info(0x06e092, "", "some at is in handle\n");
                }
            }
        }
    }
}

/**
 * aic_assemble_and_handle_at_cmd - Assemble and process the at command sent
 * by the upper device or the internal module.
 *
 * @at_str: The string of at command to handle.
 * @at_req_resp: The pointer to save the pointer of struct AT_REQ_RESP.
 * @is_external:Indicate if the command is received from the serial port,
 *              or from internal modules.
 * @serial_port_id:The serial port id which the cmd is received from.
                   If the command is sent from AP,then just set @serial_port_id
                   to an invalid value 0xffff.
 * @retry_cnt: If the command is sent to CP and timeout to receive response,
 *             or receive ERROR,then it could be retried for @retry_cnt times.
 *
 * Assemble the at command to struct AT_REQ_RESP,whether the command is
 * received from serial port or sent to CP by AP modules. Also,it will
 * dispatch it to invoke the handler or send it to CP.
 *
 * @Return SUCCESS if the command is successfully handled,or return ERROR.
 */
s32 aic_assemble_and_handle_at_cmd(u8 *at_str, ulong *at_req_resp,
                                   u8 is_external, u32 serial_port_id,
                                   u8 retry_cnt)
{
    AT_MSG_HEAD msg_head;

    aos_memset(&msg_head, 0, sizeof(AT_MSG_HEAD));
    return aic_assemble_at_cmd_with_head(at_str, &msg_head, at_req_resp, is_external,
                                        serial_port_id, retry_cnt);
}

s32 aic_assemble_at_cmd_with_head(u8 *at_str, AT_MSG_HEAD *msg_head_ptr, ulong *at_req_resp,
                                 u8 is_external, u32 serial_port_id, u8 retry_cnt)
{
    AT_REQ_RESP *at_req_res_ptr = AIC_NULL;
    AT_CMD_HANDLE *cmd_handle_ptr = AIC_NULL;
    u32 status = AIC_SUCCESS, at_len = msg_head_ptr->cmd_len;
    s32 error_number = 0;
    AT_RECV_COM_ITEM *at_com_item;
    AOS_INTERRUPT_DECLARE

    at_req_res_ptr = (AT_REQ_RESP *)aos_zalloc_relaxed(sizeof(AT_REQ_RESP));
    if (!at_req_res_ptr) {
        error_number = AT_ERROR_HANDLE_FAILURE;
        goto HANDLE_ERROR;
    }

    *at_req_resp = (ulong)at_req_res_ptr;
    at_req_res_ptr->is_external_cmd = is_external;
    if (is_external == AT_FROM_EXTERNAL) {
        at_req_res_ptr->serial_port_id = serial_port_id;
    } else {
        at_req_res_ptr->serial_port_id = AT_NO_SERIAL_PORT_ID;
    }

    if (at_len == 0)
        at_len = strlen(at_str);

    /* Append the cr char if there is no cr char, it is 0x0D for default. */
    at_req_res_ptr->at_req = (u8 *)aos_zalloc_relaxed(at_len + 2);
    if (!at_req_res_ptr->at_req) {
        error_number = AT_ERROR_HANDLE_FAILURE;
        goto HANDLE_ERROR;
    }

    aos_memcpy(at_req_res_ptr->at_req, at_str, at_len);
    /* Append the ending sign for AT cmds sent from AP,or append ending sign
     * modified by ATS3 for cmds received from AT port. */
    if (*(at_str + at_len - 2) != aic_get_cr_sign() &&
        *(at_str + at_len - 1) != aic_get_cr_sign())
        *(at_req_res_ptr->at_req + at_len) = aic_get_cr_sign();

    if (!aic_is_valid_at_cmd(at_req_res_ptr->at_req)) {
        if (is_external == AT_FROM_EXTERNAL) {
            goto PUT_TO_LIST;
        } else {
            error_number = AT_ERROR_INVALID_CMD;
            goto HANDLE_ERROR;
        }
    }

    aos_memcpy(&(at_req_res_ptr->msg_head), msg_head_ptr, sizeof(AT_MSG_HEAD));
    /* not specify a certain link. */
    at_req_res_ptr->assigned_link_id = 0xff;
    at_req_res_ptr->retry_count = retry_cnt;
    at_req_res_ptr->at_mode = AT_CMD_MODE;
    if (aic_is_valid_at_cmd(at_req_res_ptr->at_req))
        at_req_res_ptr->at_cmd_type = aic_get_at_cmd_type(at_req_res_ptr->at_req);
    else
        at_req_res_ptr->at_cmd_type = AT_CMD_INVALID_TYPE;

    if (is_external == AT_FROM_EXTERNAL) {
        cmd_handle_ptr = get_at_cmd_handle(at_str);
        if (!cmd_handle_ptr) {
            at_req_res_ptr->is_send_to_cp = AT_SEND_TO_CP;
        } else {
            if (is_pre_handled_by_ap(at_req_res_ptr->at_req,
                                     at_req_res_ptr->at_cmd_type)) {
                at_req_res_ptr->is_send_to_cp = AT_SEND_TO_CP;
            } else {
                at_req_res_ptr->is_send_to_cp = AT_NOT_SEND_TO_CP;
            }
        }
    } else if (is_external == AT_FROM_INTERNAL) {
        at_req_res_ptr->is_send_to_cp = AT_SEND_TO_CP;
    }

    at_req_res_ptr->at_cmd_handle = cmd_handle_ptr;
    /* In certain unexpected cases,cmd handle is not null but send_to_cp
     * is AT_SEND_TO_CP,which will result in exception. */
    if (at_req_res_ptr->at_cmd_handle &&
        at_req_res_ptr->at_cmd_type == AT_CMD_INVALID_TYPE)
        at_req_res_ptr->is_send_to_cp = AT_NOT_SEND_TO_CP;

    /** No need to creat timer for cmds handled by AP.For they will engage
     * the thread thread_dispatch_at.The reply to upper device would only
     * depend on the processing function.
     */
    at_req_res_ptr->timeout =
        aic_get_at_resp_timeout(at_str, at_req_res_ptr->is_send_to_cp);

    if (at_req_res_ptr->is_send_to_cp == AT_SEND_TO_CP) {
        aic_set_timeout_handler(at_req_res_ptr);
        aic_set_resp_handler(at_req_res_ptr);
    }

PUT_TO_LIST:
    at_com_item = aic_find_at_com_item(serial_port_id);

    if (is_external == AT_FROM_EXTERNAL) {
        AOS_INTERRUPT_DISABLE
        if (!at_com_item->serial_dispatch_at) {
            status = AIC_SUCCESS;
            at_com_item->serial_dispatch_at = at_req_res_ptr;
            at_com_item->serial_dispatch_at->next_ptr = AIC_NULL;
        } else {
            status = append_to_pending_at(at_req_res_ptr, at_com_item->serial_dispatch_at);
        }
        AOS_INTERRUPT_RESTORE
        if (!status)
            aos_set_event(event_dispatch_at, at_com_item->dispatch_event_id, AOS_OR);
    } else if (is_external == AT_FROM_INTERNAL) {
        status = aic_send_at_to_cp(at_req_res_ptr);
    }
    if (status == AT_ERROR_PENDING_CMD_OVERFLOW)
        aicx_error(0x11189c, "", "pending list full,AT released\n");

    return status;

HANDLE_ERROR:
    if (is_external == AT_FROM_EXTERNAL)
        aic_reply_error_resp(at_req_res_ptr, error_number);

    aic_free_at_req_resp(at_req_res_ptr);

    return error_number;
}

boolean aic_is_valid_at_cmd(u8 * at_str)
{
    u16 len;

    len = strlen(at_str);
    if (len >= AT_MAX_AP_CMD_LENGTH || len < AT_MIN_CMD_LENGTH)
        return AIC_FALSE;

    if (strncasecmp(at_str, "AT", 2) == 0 || strncasecmp(at_str, "A/", 2) == 0)
        return AIC_TRUE;

     return AIC_FALSE;
}

boolean is_pre_handled_by_ap(u8 *at_ptr, u8 cmd_type)
{
    if (strncasecmp(at_ptr, "AT+CGACT", strlen("AT+CGACT")) == 0 ||
        strncasecmp(at_ptr, "ATH", strlen("ATH")) == 0 ||
        strncasecmp(at_ptr, AT_CMD_S3, strlen(AT_CMD_S3)) == 0 ||
        strncasecmp(at_ptr, AT_CMD_S4, strlen(AT_CMD_S4)) == 0)
        return AIC_TRUE;

    return AIC_FALSE;
}

boolean is_ten_seconds_cmd(u8 *at_ptr)
{
    if (!at_ptr)
        return AIC_FALSE;

    /* AT+CMSS is to send a message saved before,it may cost a while. */
    if ((strcasestr(at_ptr, "AT+CMSS") && !strchr(at_ptr, '?')) ||
        (strcasestr(at_ptr, "AT+CFUN=") && !strchr(at_ptr, '?')) ||
        strcasestr(at_ptr, "AT^AQCI") || strcasestr(at_ptr, "AT+MUESTATS") ||
        strcasestr(at_ptr, "AT+MTSETID") || strcasestr(at_ptr, "AT+MBAND") ||
        strcasestr(at_ptr, "AT+MCSEARFCN"))
        return AIC_TRUE;

    return AIC_FALSE;
}

boolean is_time_consuming_cmd(u8 *at_ptr)
{
    if (!at_ptr)
        return AIC_FALSE;

    /**
     * AT+COPS=3 is to set format,not to search cells,so it is not a time
     * consuming command.
     */
    if ((strcasestr(at_ptr, "AT+CGACT=") && !strchr(at_ptr, '?')) ||
        (strcasestr(at_ptr, "AT+QIACT=") && !strchr(at_ptr, '?')) ||
        (strcasestr(at_ptr, "AT+QIDEACT=") && !strchr(at_ptr, '?')) ||
        (strcasestr(at_ptr, "AT+COPS=") && !strcasestr(at_ptr, "AT+COPS=3")) ||
        (strcasestr(at_ptr, "AT+CGDATA=") && !strchr(at_ptr, '?')) ||
        (strcasestr(at_ptr, "AT+CGATT=") && !strchr(at_ptr, '?')) ||
        (strcasestr(at_ptr, "AT+IMSREG=") && !strchr(at_ptr, '?')) ||
        strcasestr(at_ptr, "AT+BGLTEPLMN=") ||
        strcasestr(at_ptr, "AT+CMGD") || strcasestr(at_ptr, "AT+CMGL") ||
        strcasestr(at_ptr, "AT+CMGR") || strcasestr(at_ptr, "AT+CCFC=") ||
        strcasestr(at_ptr, "AT+CSIM") || strcasestr(at_ptr, "AT+CGLA")) {
        return AIC_TRUE;
    }

    return AIC_FALSE;
}

u32 aic_get_at_resp_timeout(u8 *at_str, u8 is_to_cp)
{
    u32 timeout = 0;

    if (!at_str)
        return timeout;

    if (is_to_cp == AT_SEND_TO_CP) {
        timeout = AT_CP_CMD_DEFAULT_TIMEOUT;

        if (strncasecmp(at_str, "ATD", 3) == 0)
            timeout = AT_TIMEOUT_SIX_SECONDS;

        if (is_ten_seconds_cmd(at_str))
            timeout = AT_TIMEOUT_TEN_SECONDS;

        if (is_time_consuming_cmd(at_str))
            timeout = AT_TIMEOUT_HALF_MINUTE;
    }

    return timeout;
}

u32 init_sending_cmd_to_cp(AT_REQ_RESP *at_req_resp_ptr, u8 *at_str)
{
    u16 at_len;
    u8 *tmp_ptr;
    u32 status = AIC_SUCCESS;

    at_len = strlen(at_str);
    /* Append the cr char if there is no cr char, it is 0x0D for default. */
    at_req_resp_ptr->at_req = (u8 *)aos_zalloc_relaxed(at_len + 2);
    if (!at_req_resp_ptr->at_req) {
        status = AT_ERROR_HANDLE_FAILURE;
        return status;
    }

    strncpy(at_req_resp_ptr->at_req, at_str, at_len);
    tmp_ptr = strchr(at_str, aic_get_cr_sign());
    /* Append the ending sign for AT cmds sent from AP. */
    if (!tmp_ptr)
        *(at_req_resp_ptr->at_req + at_len) = aic_get_cr_sign();

    /* The shortest at cmd is: AT\r or A/\r. */
    if (!aic_is_valid_at_cmd(at_req_resp_ptr->at_req)) {
        status = AT_ERROR_INVALID_CMD;
        return status;
    }

    at_req_resp_ptr->is_external_cmd = AT_FROM_INTERNAL;
    at_req_resp_ptr->serial_port_id = AT_NO_SERIAL_PORT_ID;
    at_req_resp_ptr->assigned_link_id = 0xff;
    at_req_resp_ptr->at_mode = AT_CMD_MODE;
    at_req_resp_ptr->is_send_to_cp = AT_SEND_TO_CP;
    at_req_resp_ptr->timeout =
        aic_get_at_resp_timeout(at_str, at_req_resp_ptr->is_send_to_cp);

    if (aic_is_valid_at_cmd(at_req_resp_ptr->at_req))
        at_req_resp_ptr->at_cmd_type = aic_get_at_cmd_type(at_req_resp_ptr->at_req);
    else
        at_req_resp_ptr->at_cmd_type = AT_CMD_INVALID_TYPE;

    aic_set_resp_handler(at_req_resp_ptr);

    return status;
}

u32 aic_send_cmd_and_wait(u8 *at_str, AT_RESP_FROM_CP *resp_to_save)
{
    AT_REQ_RESP *at_req_resp_ptr;
    u32 ret = AIC_SUCCESS;

    at_req_resp_ptr = (AT_REQ_RESP *)aos_zalloc_relaxed(sizeof(AT_REQ_RESP));
    if (!at_req_resp_ptr)
        return AT_ERROR_HANDLE_FAILURE;

    ret = init_sending_cmd_to_cp(at_req_resp_ptr, at_str);
    if (ret)
        goto HANDLE_ERROR;

    at_req_resp_ptr->wait_resp_semaphore =
        aos_create_semaphore("wait_resp_semaphore", 0);
    if (!at_req_resp_ptr->wait_resp_semaphore) {
        ret = AT_ERROR_HANDLE_FAILURE;
        goto HANDLE_ERROR;
    }

    ret = aic_send_at_to_cp(at_req_resp_ptr);
    if (ret == AT_ERROR_PENDING_CMD_OVERFLOW) {
        /* Memory freeed in append_to_pending_at. */
        aos_delete_semaphore(at_req_resp_ptr->wait_resp_semaphore);
        return ret;
    }

    ret = aos_get_semaphore(at_req_resp_ptr->wait_resp_semaphore, AOS_WAIT_FOREVER);
    if (ret)
        aicx_error(0x03cafb, "x", "send cmd wait:get semaphore error 0x%x\n", ret);

    aos_delete_semaphore(at_req_resp_ptr->wait_resp_semaphore);
    at_req_resp_ptr->wait_resp_semaphore = AIC_NULL;

    if (resp_to_save && resp_to_save->resp_buf && at_req_resp_ptr->at_resp &&
        at_req_resp_ptr->resp_line_count) {
        aos_memset(resp_to_save->resp_buf, 0, resp_to_save->buf_len);
        strncpy(resp_to_save->resp_buf, at_req_resp_ptr->at_resp,
                (resp_to_save->buf_len - 1));
        resp_to_save->resp_line_cnt = at_req_resp_ptr->resp_line_count;
    }

    /* Not the final result code,still waiting final result. */
    if (aic_contain_final_result_code(at_req_resp_ptr->at_resp,
                                      at_req_resp_ptr->resp_line_count)) {
        aic_free_at_req_resp(at_req_resp_ptr);
    }

    return ret;

HANDLE_ERROR:
    aic_free_at_req_resp(at_req_resp_ptr);

    return ret;
}

u32 aic_send_md_cmd_with_callback(u8 *at_str, aic_get_cp_resp_callback cb)
{
    AT_REQ_RESP *at_req_resp_ptr;
    u32 ret = AIC_SUCCESS;
    u16 at_len = 0;
    u8 tmp_resp[30] = {'\0'};
    u8 resp_str[40] = {'\0'};
    u32 timeout = 0;
    u8 link_state;

    aicx_info(0x19302b, "s", "send md cmd with cb: %s\n", at_str);
    link_state = aic_get_abuf_link_state(AT_LINK5);
    if ((link_state != AT_ABUF_LINK_IDLE) && (link_state != AT_ABUF_LINK_WAIT_MESSAGE)) {
        ret = link_state;
        goto HANDLE_ERROR;
    }

    at_req_resp_ptr = (AT_REQ_RESP *)aos_zalloc_relaxed(sizeof(AT_REQ_RESP));
    if (!at_req_resp_ptr) {
        ret = AT_ERROR_HANDLE_FAILURE;
        goto HANDLE_ERROR;
    }

    /* same with init_sending_cmd_to_cp,delete check at invalid */
    at_len = strlen(at_str);
    at_req_resp_ptr->at_req = (u8 *)aos_zalloc_relaxed(at_len + 1);
    if (!at_req_resp_ptr->at_req) {
        aic_free_at_req_resp(at_req_resp_ptr);
        ret = AT_ERROR_HANDLE_FAILURE;
        goto HANDLE_ERROR;
    }

    strncpy(at_req_resp_ptr->at_req, at_str, at_len);
    at_req_resp_ptr->is_external_cmd = AT_FROM_INTERNAL;
    at_req_resp_ptr->serial_port_id = AT_NO_SERIAL_PORT_ID;
    at_req_resp_ptr->assigned_link_id = AT_LINK5;
    aic_set_abuf_link_reserved(AT_LINK5);
    at_req_resp_ptr->at_mode = AT_CMD_MODE;
    at_req_resp_ptr->is_send_to_cp = AT_SEND_TO_CP;

    timeout = aic_get_at_resp_timeout(at_str, at_req_resp_ptr->is_send_to_cp);
    /* check if this id sms pdu str */
    if (strcasestr(at_str, "\x1A")) {
        aicx_info(0x1c690f, "", "current str is sms pdu!!!!!\n");
        timeout = AT_TIMEOUT_HALF_MINUTE;
    }

    at_req_resp_ptr->timeout = timeout;
    /* modem driver cmd use abuf AT_LINK5 to send to cp */
    at_req_resp_ptr->inform_resp_cb = cb;
    ret = aic_send_at_to_cp(at_req_resp_ptr);
    if (ret == AT_ERROR_PENDING_CMD_OVERFLOW) {
        goto HANDLE_ERROR;
    }

    return ret;

HANDLE_ERROR:
    aicx_error(0x055da4, "d", "send md cmd with cb error %d\n", ret);
    if (cb) {
        snprintf(tmp_resp, sizeof(tmp_resp), AT_RESULT_CME_ERROR"%d", ret);
        fill_head_and_end_signs(resp_str, tmp_resp, strlen(tmp_resp));
        cb(resp_str, 1);
    }

    return ret;
}

u32 aic_send_cmd_with_callback(u8 *at_str, aic_get_cp_resp_callback cb)
{
    AT_REQ_RESP *at_req_resp_ptr = AIC_NULL;
    u32 ret = AIC_SUCCESS;
    u8 tmp_resp[24] = {'\0'};
    u8 resp_str[30] = {'\0'};

    aicx_info(0x0d3440, "s", "send cmd with cb: %s\n", at_str);
    at_req_resp_ptr = (AT_REQ_RESP *)aos_zalloc_relaxed(sizeof(AT_REQ_RESP));
    if (!at_req_resp_ptr)
        goto HANDLE_ERROR;

    ret = init_sending_cmd_to_cp(at_req_resp_ptr, at_str);
    if (ret) {
        aic_free_at_req_resp(at_req_resp_ptr);
        goto HANDLE_ERROR;
    }

    at_req_resp_ptr->inform_resp_cb = cb;
    ret = aic_send_at_to_cp(at_req_resp_ptr);
    if (ret == AT_ERROR_PENDING_CMD_OVERFLOW)
        goto HANDLE_ERROR;

    return ret;

HANDLE_ERROR:
    aicx_error(0x1d3ae1, "d", "send cmd with cb error %d\n", ret);
    if (cb) {
        snprintf(tmp_resp, sizeof(tmp_resp), AT_RESULT_CME_ERROR"%d", ret);
        fill_head_and_end_signs(resp_str, tmp_resp, strlen(tmp_resp));
        cb(resp_str, 1);
    }

    return ret;
}

boolean aic_is_cfun_cmd(u8 *cmd_str, u8 is_start_cp)
{
    u8 cmp_str[12] = {'\0'};
    u8 len;

    if (!cmd_str)
        return AIC_FALSE;

    snprintf(cmp_str, sizeof(cmp_str), "AT+CFUN=%d", is_start_cp);
    len = strlen("AT+CFUN=1");
    if (strncasecmp(cmd_str, cmp_str, len) == 0 && is_cr_lf_sign(*(cmd_str + len)))
        return AIC_TRUE;

    return AIC_FALSE;
}

void aic_set_at_handle_mode(AT_REQ_RESP *at_req_resp, u8 at_mode)
{
    if (!at_req_resp)
        return;

    at_req_resp->at_mode = at_mode;
}

void aic_put_at_data_to_queue(AT_RECV_COM_ITEM *at_com_item, u8 *data_ptr, u16 len)
{
    u32 ret;
    AT_DATA_ITEM *at_data_item;

    at_data_item = aos_zalloc_relaxed(sizeof(AT_DATA_ITEM));
    if (!at_com_item)
        return;

    at_data_item->data_len = len;
    at_data_item->data_buf = aos_zalloc_relaxed(len);
    if(!at_data_item->data_buf)
        return;

    aos_memcpy(at_data_item->data_buf, data_ptr, len);
    ret = aos_send_to_queue(at_com_item->at_data_queue, (void *)&at_data_item, AOS_NO_WAIT);
    /* queue is full. */
    if (ret) {
        aicx_error(0x14c056, "", "send to at data queue fail\n");
        aos_free(at_data_item->data_buf);
        aos_free(at_data_item);
    }
}

/* Append the AT to the tail. */
u32 append_to_pending_at(AT_REQ_RESP *at_req_resp,
                         AT_REQ_RESP *head_ptr)
{
    AT_REQ_RESP *tmp_at_ptr1;
    AT_REQ_RESP *tmp_at_ptr2;
    u16 pending_count = 1;

    if (at_req_resp == head_ptr) {
        return AIC_SUCCESS;
    }

    tmp_at_ptr1 = head_ptr;
    while (tmp_at_ptr1->next_ptr) {
        tmp_at_ptr2 = tmp_at_ptr1->next_ptr;
        tmp_at_ptr1 = tmp_at_ptr2;
        pending_count++;
    }

    if (pending_count >= AT_PENDING_MAX_CMD_NUM) {
        aicx_info(0x0db54a, "", "pending too many AT,just free it\n");
        aic_free_at_req_resp(at_req_resp);
        return AT_ERROR_PENDING_CMD_OVERFLOW;
    } else {
        pending_count++;
        aicx_info(0x071d25, "d", "AT pending count = %d\n", pending_count);
        tmp_at_ptr1->next_ptr = at_req_resp;
        return AIC_SUCCESS;
    }
}

void invoke_at_cmd_handler(AT_REQ_RESP *at_req_resp)
{
    u8 cmd_type;
    AT_CMD_HANDLE *at_handle_ptr = AIC_NULL;
    AT_DATA_ITEM *data_item = AIC_NULL;
    u32 ret = AIC_SUCCESS, len;
    AT_RECV_COM_ITEM *at_com_item = AIC_NULL;

    if (!at_req_resp)
        return;

    at_handle_ptr = (AT_CMD_HANDLE *)at_req_resp->at_cmd_handle;
    /* Not handled by AP,just send it to CP. */
    if (!at_handle_ptr) {
#ifndef WIN32
        aic_send_at_to_cp(at_req_resp);
#else
        handle_invalid_at_cmd(at_req_resp);
        at_req_resp->is_send_to_cp = AT_NOT_SEND_TO_CP;
#endif
    } else {
        cmd_type = at_req_resp->at_cmd_type;
        /* If the handle function is null,then it is an cmd not supported. */
        if (cmd_type & AT_CMD_TYPE_TEST) {
            if (at_handle_ptr->handle_at_test_cmd) {
                at_handle_ptr->handle_at_test_cmd(at_req_resp);
            } else {
                handle_invalid_at_cmd(at_req_resp);
            }
        } else if (cmd_type & AT_CMD_TYPE_QUERY) {
            if (at_handle_ptr->handle_at_query_cmd) {
                at_handle_ptr->handle_at_query_cmd(at_req_resp);
            } else {
                handle_invalid_at_cmd(at_req_resp);
            }
        } else if ((cmd_type & AT_CMD_TYPE_SET) || (cmd_type & AT_CMD_TYPE_EXE)) {
            if (!at_handle_ptr->handle_at_set_cmd && !at_handle_ptr->handle_at_exe_cmd) {
                handle_invalid_at_cmd(at_req_resp);
                return;
            }

            if (at_req_resp->at_mode == AT_CMD_MODE || at_req_resp->at_mode == AT_TMP_CMD_MODE) {
                /* It is set cmd or exe cmd,must be one of them,not both exist. */
                if (at_handle_ptr->handle_at_set_cmd) {
                    at_handle_ptr->handle_at_set_cmd(at_req_resp);
                } else if (at_handle_ptr->handle_at_exe_cmd) {
                    at_handle_ptr->handle_at_exe_cmd(at_req_resp);
                }
            } else if (at_req_resp->at_mode == AT_DATA_MODE) {
                do {
                    at_com_item = aic_find_at_com_item(at_req_resp->serial_port_id);
                    if (!at_com_item)
                        return;

                    ret = aos_recv_from_queue(at_com_item->at_data_queue,
                                              (void*)&data_item, AOS_NO_WAIT);

                    if (!ret && data_item) {
                        if (at_req_resp->at_mode == AT_DATA_MODE) {
                            len = data_item->data_len;
                            at_req_resp->at_upstream_data = data_item->data_buf;
                            at_req_resp->upstream_data_len = data_item->data_len;
                            if (at_handle_ptr->handle_at_set_cmd) {
                                at_handle_ptr->handle_at_set_cmd(at_req_resp);
                            } else if (at_handle_ptr->handle_at_exe_cmd) {
                                at_handle_ptr->handle_at_exe_cmd(at_req_resp);
                            }
                        }

                        aos_free(data_item->data_buf);
                        aos_free(data_item);
                        data_item = 0;
                        at_req_resp->at_upstream_data = AIC_NULL;
                        at_req_resp->upstream_data_len = 0;
                    /* End data mode at for timeout. */
                    } else if (at_req_resp->end_data_mode) {
                        aicx_info(0x0e038b, "", "End data mode for timeout\n");
                        if (at_handle_ptr->handle_at_set_cmd) {
                            at_handle_ptr->handle_at_set_cmd(at_req_resp);
                        } else if (at_handle_ptr->handle_at_exe_cmd) {
                            at_handle_ptr->handle_at_exe_cmd(at_req_resp);
                        }
                    }
                } while (ret == AIC_SUCCESS);
            }
        } else {
            handle_invalid_at_cmd(at_req_resp);
        }
    }
}

#ifndef CONFIG_AT_PROC_MINI
/* Set the data mode timeout in ms. */
u32 aic_set_data_mode_expire_time(u8 *at_head, u32 expire_time_ms)
{
    u8 i;
    DATA_MODE_EXPIRE_TIME *expire_time_ptr;
    u8 len1, len2;

    len2 = strlen(at_head);
    for (i = 0; i < AT_DATA_MODE_TIMEOUT_CNT; i++) {
        expire_time_ptr = &data_mode_timeouts[i];
        len1 = strlen(expire_time_ptr->at_head);
        if (len1 == len2 && strncasecmp(expire_time_ptr->at_head, at_head, len2) == 0) {
            expire_time_ptr->expire_time = expire_time_ms;
            return AIC_SUCCESS;
        }
    }

    for (i = 0; i < AT_DATA_MODE_TIMEOUT_CNT; i++) {
        expire_time_ptr = &data_mode_timeouts[i];
        if (strlen(expire_time_ptr->at_head) == 0) {
            strncpy(expire_time_ptr->at_head, at_head, (AT_MAX_HEAD_LEN - 1));
            expire_time_ptr->expire_time = expire_time_ms;
            return AIC_SUCCESS;
        }
    }

    return AT_ERROR_TIMEOUTS_OVERFLOW;
}

static u32 get_data_mode_expire_time(u8 *at_head)
{
    u8 i, len1, len2, cmp_len;
    DATA_MODE_EXPIRE_TIME *expire_time_ptr;

    len1 = strlen(at_head);
    for (i = 0; i < AT_DATA_MODE_TIMEOUT_CNT; i++) {
        expire_time_ptr = &data_mode_timeouts[i];
        len2 = strlen(expire_time_ptr->at_head);
        cmp_len = len1 >= len2 ? len1 : len2;
        if (strncasecmp(at_head, expire_time_ptr->at_head, cmp_len) == 0)
            return expire_time_ptr->expire_time;
    }

    return AT_DATA_MODE_TIMEOUT;
}

void start_at_data_mode_timer(AT_REQ_RESP *at_req_ptr)
{
    u32 expire_time;
    u8 at_cmd_head[AT_MAX_HEAD_LEN] = {'\0'};

#ifdef CONFIG_D2D
    if (strcasestr(at_req_ptr->at_req, "AT+DSVOIDATA") &&
        d2d_mode_cfg.is_transparent_mode)
        return;
#endif

#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
    if (!aic_boot_mode_is_basic() && aic_is_transparent_mode())
        return;
#endif

    if (!aos_is_timer_created(at_data_mode_timer)) {
        get_at_head_str(at_req_ptr->at_req, at_cmd_head);
        expire_time = get_data_mode_expire_time(at_cmd_head);
        at_data_mode_timer = aos_create_timer("data_mode",
                                              at_data_mode_timeout,
                                              AIC_NULL,
                                              expire_time,
                                              AOS_AUTO_ACTIVATE,
                                              AOS_NO_PERIOD);

        AIC_ASSERT(at_data_mode_timer);
    }
}

void stop_at_data_mode_timer(void)
{
    if (aos_is_timer_created(at_data_mode_timer)) {
        aos_deactivate_timer(at_data_mode_timer);
        aos_delete_timer(at_data_mode_timer);
        at_data_mode_timer = AIC_NULL;
    }
}

void at_data_mode_timeout(void *input)
{
    u8 i;
    u32 at_com_id;

    for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
        at_com_id = at_ports_info[i].at_com_id;
        if (at_com_id == AT_NO_SERIAL_PORT_ID)
            break;

        if (at_com_id != AT_DIAG_SERIAL_PORT_ID)
            aic_check_data_mode_at_by_port(at_com_id);
    }
}

void aic_check_data_mode_at_by_port(u32 at_port_id)
{
    SERIAL_DATA_MODE_AT *data_mode_at_ptr;
    AT_REQ_RESP *at_req_ptr;
    AT_RECV_COM_ITEM *at_com_item = AIC_NULL;

    data_mode_at_ptr = aic_get_serial_data_mode_at(at_port_id);

    if (data_mode_at_ptr) {
        at_req_ptr = data_mode_at_ptr->data_mode_at;
        if (at_req_ptr && at_req_ptr->at_mode == AT_DATA_MODE) {
            aicx_info(0x051168, "s", "data mode AT timeout %s\n", at_req_ptr->at_req);
            at_req_ptr->end_data_mode = 1;
            at_com_item = aic_find_at_com_item(at_port_id);
            if (!at_com_item)
                return;

            if (at_com_item->serial_dispatch_at != at_req_ptr)
                aicx_error(0x028b5c, "", "error,serial_dispatch_at is not data mode at\n");

            aos_set_event(event_dispatch_at, at_com_item->dispatch_event_id, AOS_OR);
        }
    }
}
#endif

#ifdef CONFIG_CUSTOMER_AT
void aic_release_tmp_cmd_mode_at(AT_REQ_RESP *at_ptr)
{
    stop_tmp_cmd_mode_at_timer();
    clear_serial_data_mode_at(at_ptr);
    aic_free_at_req_resp(at_ptr);
}

void check_tmp_mode_at_by_port(u32 at_port_id)
{
    SERIAL_DATA_MODE_AT *data_mode_at_ptr;
    AT_REQ_RESP *tmp_cmd_mode_at;

    data_mode_at_ptr = aic_get_serial_data_mode_at(at_port_id);

    if (data_mode_at_ptr && data_mode_at_ptr->data_mode_at) {
        tmp_cmd_mode_at = data_mode_at_ptr->data_mode_at;
        if (tmp_cmd_mode_at->at_mode == AT_TMP_CMD_MODE) {
            aicx_info(0x04b6d2, "s", "tmp cmd mode AT timeout %s\n", tmp_cmd_mode_at->at_req);
            invoke_at_cmd_handler(tmp_cmd_mode_at);
        }

        clear_serial_data_mode_at(tmp_cmd_mode_at);
        aic_free_at_req_resp(tmp_cmd_mode_at);
    }
}

void tmp_cmd_mode_at_timeout(void *input)
{
    u8 i;
    u32 at_port_id;

    for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
        at_port_id = at_ports_info[i].at_com_id;
        if (at_port_id == AT_NO_SERIAL_PORT_ID)
            break;

        if (at_port_id != AT_DIAG_SERIAL_PORT_ID)
            check_tmp_mode_at_by_port(at_port_id);
    }
}

void start_tmp_cmd_mode_at_timer(void)
{
    if (!tmp_cmd_mode_at_timer) {
        tmp_cmd_mode_at_timer = aos_create_timer("tmp_cmd_mode_at_timer",
                                                 tmp_cmd_mode_at_timeout,
                                                 AIC_NULL,
                                                 AT_TMP_CMD_MODE_TIMEOUT,
                                                 AOS_NO_ACTIVATE,
                                                 AOS_NO_PERIOD);

        AIC_ASSERT(tmp_cmd_mode_at_timer);
    }

    aos_deactivate_timer(tmp_cmd_mode_at_timer);
    aos_activate_timer(tmp_cmd_mode_at_timer);
}

void stop_tmp_cmd_mode_at_timer(void)
{
    if (tmp_cmd_mode_at_timer)
        aos_deactivate_timer(tmp_cmd_mode_at_timer);
}

void enter_data_mode_via_ato(u32 at_port_id)
{
    SERIAL_DATA_MODE_AT *data_mode_at_ptr;
    AT_REQ_RESP *tmp_cmd_mode_at, *tmp_at_req = AIC_NULL;;
    AT_RECV_COM_ITEM *at_com_item;
    AOS_INTERRUPT_DECLARE

    data_mode_at_ptr = aic_get_serial_data_mode_at(at_port_id);
    if (data_mode_at_ptr && data_mode_at_ptr->data_mode_at) {
        tmp_cmd_mode_at = data_mode_at_ptr->data_mode_at;
        if (tmp_cmd_mode_at->at_mode == AT_TMP_CMD_MODE && aic_get_socket_is_valid()) {
            /* Stop checking the time. */
            stop_tmp_cmd_mode_at_timer();
            /* Re-enter data_mode. */
            tmp_cmd_mode_at->at_mode = AT_DATA_MODE;
            tmp_cmd_mode_at->at_handle_state = AT_EXT_CMD_HANDLING;

            at_com_item = aic_find_at_com_item(at_port_id);
            AOS_INTERRUPT_DISABLE
            if (at_com_item && at_com_item->serial_dispatch_at) {
                tmp_at_req = at_com_item->serial_dispatch_at->next_ptr;
                at_com_item->serial_dispatch_at = tmp_cmd_mode_at;
                at_com_item->serial_dispatch_at->next_ptr = tmp_at_req;
            }
            AOS_INTERRUPT_RESTORE
        }
    }
}

boolean aic_allowed_to_enter_data_mode(u32 at_port_id)
{
    SERIAL_DATA_MODE_AT *data_mode_at_ptr;
    AT_REQ_RESP *at_ptr;

    data_mode_at_ptr = aic_get_serial_data_mode_at(at_port_id);
    if (data_mode_at_ptr && data_mode_at_ptr->data_mode_at) {
        at_ptr = data_mode_at_ptr->data_mode_at;

        if (at_ptr->at_mode == AT_TMP_CMD_MODE && aic_get_socket_is_valid()) {
            return AIC_FALSE;
        } else {
            aic_release_tmp_cmd_mode_at(at_ptr);
            return AIC_TRUE;
        }
    }

    return AIC_TRUE;
}
#endif

boolean is_ato_cmd(u8 *at_cmd_str)
{
    if (!at_cmd_str || strlen(at_cmd_str) < (strlen("ATO") + 1))
        return AIC_FALSE;

    if (strncasecmp(at_cmd_str, "ATO", 3) == 0 && is_cr_lf_sign(*(at_cmd_str + 3)))
        return AIC_TRUE;

    return AIC_FALSE;
}

#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
static void handle_pre_data_mode_at(AT_REQ_RESP *at_req_resp)
{
    if (!at_aync_data_mode.at_req_ptr) {
        aicx_info(0x0060f7, "", "at in pre_data_mode is null\n");
        return;
    }

    at_req_resp->at_mode = AT_DATA_MODE;
    at_req_resp->at_handle_state = AT_EXT_CMD_HANDLING;

    aos_get_mutex(at_aync_data_mode.pre_data_mutex, AOS_WAIT_FOREVER);
    if (at_aync_data_mode.at_urc_str) {
        aic_transmit_at_cmd_urc(at_aync_data_mode.at_req_ptr->serial_port_id,
                                at_aync_data_mode.at_urc_str,
                                strlen(at_aync_data_mode.at_urc_str));
        aos_free(at_aync_data_mode.at_urc_str);
        at_aync_data_mode.at_urc_str = AIC_NULL;
    }

    at_aync_data_mode.at_req_ptr = AIC_NULL;
    aos_put_mutex(at_aync_data_mode.pre_data_mutex);

    aic_get_wakelock();
    set_serial_data_mode_at(at_req_resp);
    /* Start timer for at in data mode,in case rececive no data or +++. */
    start_at_data_mode_timer(at_req_resp);
}
#endif

#ifndef CONFIG_AT_PROC_MINI
static u8 is_no_need_time_out_urc(AT_REQ_RESP *at_req_resp)
{
    u8 staus = 0;

#if CONFIG_GX_AT_V2 || CONFIG_CUSTOMER_CM_AT
    return 1;
#endif
#ifdef CONFIG_CUSTOMER_AT
    if (at_req_resp->at_req && strcasestr(at_req_resp->at_req, "AT+QSSLOPEN"))
        staus = 1;
#endif

    return staus;
}

/* Clear the at data queue. */
static void clear_at_data_queue(u32 at_com_id)
{
    AT_RECV_COM_ITEM *at_com_item;
    AT_DATA_ITEM *data_item = AIC_NULL;
    u32 ret;

    at_com_item = aic_find_at_com_item(at_com_id);
    if (!at_com_item)
        return;

    do {
        ret = aos_recv_from_queue(at_com_item->at_data_queue,
                                  (void*)&data_item, AOS_NO_WAIT);

        if (!ret && data_item) {
            aos_free(data_item->data_buf);
            aos_free(data_item);
            data_item = 0;
        }
    } while (ret == AIC_SUCCESS);
}
#endif

void dispatch_serial_at(AT_REQ_RESP *at_req_resp)
{
    u8 old_at_mode;

    /* The shortest at cmd is: AT\r or A/\r. */
    if (!aic_is_valid_at_cmd(at_req_resp->at_req)) {
        handle_invalid_at_cmd(at_req_resp);
        aic_invoke_next_external_at(at_req_resp->serial_port_id);
        aic_free_at_req_resp(at_req_resp);

        return;
    }

#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    if (at_aync_data_mode.at_req_ptr && at_aync_data_mode.at_urc_str &&
        at_aync_data_mode.at_req_ptr == at_req_resp) {
        handle_pre_data_mode_at(at_req_resp);
        return;
    }
#endif
    at_req_resp->at_handle_state = AT_EXT_CMD_HANDLING;
    old_at_mode = at_req_resp->at_mode;

    /* Invoke the handler of the at cmd. */
    invoke_at_cmd_handler(at_req_resp);

    /** If not send to cp,the AT cmd has been handled above.
      * If send to cp,the it will wait for response.
      */
    if (at_req_resp->is_send_to_cp == AT_NOT_SEND_TO_CP) {
        if (at_req_resp->at_mode == AT_CMD_MODE) {
            at_req_resp->at_handle_state = AT_EXT_CMD_HANDLED_OVER;
#ifndef CONFIG_AT_PROC_MINI
            if (old_at_mode != at_req_resp->at_mode) {
                aicx_info(0x058be1, "", "put sleep wakelock\n");
                aic_put_wakelock();
                stop_at_data_mode_timer();
#ifdef CONFIG_CUSTOMER_AT
                aic_set_if_cache_urc(AT_URC_CACHE_DISABLED);
#endif
                if (at_req_resp->end_data_mode && !is_no_need_time_out_urc(at_req_resp))
                    aic_reply_error_resp(at_req_resp, AT_ERROR_DATA_MODE_TIMEOUT);
            }
#endif
            if (!is_ato_cmd(at_req_resp->at_req)) {
                aic_invoke_next_external_at(at_req_resp->serial_port_id);
#ifndef CONFIG_AT_PROC_MINI
                if (old_at_mode != at_req_resp->at_mode) {
                    clear_at_data_queue(at_req_resp->serial_port_id);
                    clear_serial_data_mode_at(at_req_resp);
                }
#endif
            } else {
#ifdef CONFIG_CUSTOMER_AT
                if (strcasestr(at_req_resp->at_resp, AT_RESULT_CONNECT))
                    enter_data_mode_via_ato(at_req_resp->serial_port_id);
                else
                    aic_invoke_next_external_at(at_req_resp->serial_port_id);
#endif
            }

            aic_free_at_req_resp(at_req_resp);
        }
#ifndef CONFIG_AT_PROC_MINI
        else if (at_req_resp->at_mode == AT_DATA_MODE) {
            /* Still need to receive data from serila port. */
            aicx_info(0x1b2327, "", "AT waiting for data\n", at_req_resp->at_req);
            if (old_at_mode != at_req_resp->at_mode) {
#ifdef CONFIG_CUSTOMER_AT
                aic_set_if_cache_urc(AT_URC_CACHE_ENABLED);
#endif
                aic_get_wakelock();
                set_serial_data_mode_at(at_req_resp);
            }

            /* Start timer for at in data mode,in case rececive no data or +++. */
            start_at_data_mode_timer(at_req_resp);
        } else if (at_req_resp->at_mode == AT_TMP_CMD_MODE) {
            aicx_info(0x190344, "s", "enter tmp cmd mode--%s\n", at_req_resp->at_req);
            at_req_resp->at_handle_state = AT_EXT_CMD_HANDLED_OVER;
            if (old_at_mode != at_req_resp->at_mode)
                aic_put_wakelock();

            stop_at_data_mode_timer();
#ifdef CONFIG_CUSTOMER_AT
            /* Record when to clear it if it doesn't re-enter data mode.*/
            start_tmp_cmd_mode_at_timer();
#endif
            aic_invoke_next_external_at(at_req_resp->serial_port_id);
        }
#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
        else if (at_req_resp->at_mode == AT_PRE_DATA_MODE) {
            at_req_resp->at_handle_state = AT_EXT_CMD_HANDLED_OVER;
            aic_invoke_next_external_at(at_req_resp->serial_port_id);
        }
#endif
#endif
    }
}

void aic_invoke_next_external_at(u32 at_port_id)
{
    AT_REQ_RESP *at_req_resp = AIC_NULL;
    AT_RECV_COM_ITEM *at_com_item;
    AOS_INTERRUPT_DECLARE

    at_com_item = aic_find_at_com_item(at_port_id);
    if (!at_com_item)
        return;

#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    /* Make the at in pre_data_mode enter data mode. */
    if (at_aync_data_mode.at_req_ptr && at_aync_data_mode.at_urc_str) {
        AOS_INTERRUPT_DISABLE
        if (at_com_item->serial_dispatch_at)
            at_req_resp = at_com_item->serial_dispatch_at->next_ptr;

        at_com_item->serial_dispatch_at = at_aync_data_mode.at_req_ptr;
        at_com_item->serial_dispatch_at->next_ptr = at_req_resp;
        at_com_item->serial_dispatch_at->at_handle_state = AT_EXT_CMD_NOT_HANDLED;
        AOS_INTERRUPT_RESTORE
        aos_set_event(event_dispatch_at, at_com_item->dispatch_event_id, AOS_OR);

        return;
    }
#endif

    AOS_INTERRUPT_DISABLE
    if (at_com_item->serial_dispatch_at) {
        at_com_item->serial_dispatch_at->at_handle_state = AT_EXT_CMD_HANDLED_OVER;
        at_req_resp = at_com_item->serial_dispatch_at->next_ptr;
        if (at_req_resp)
            aicx_info(0x17d22a, "s", "next dispatch at is %s\n", at_req_resp->at_req);

        at_com_item->serial_dispatch_at = at_req_resp;
    }
    AOS_INTERRUPT_RESTORE

    if (at_com_item->serial_dispatch_at)
        aos_set_event(event_dispatch_at, at_com_item->dispatch_event_id, AOS_OR);
}

#ifndef CONFIG_AT_PROC_MINI
u32 aic_end_at_data_mode(u32 at_com_id)
{
    SERIAL_DATA_MODE_AT *data_mode_at_ptr;
    AT_REQ_RESP *at_ptr;
    AT_RECV_COM_ITEM *at_com_item = AIC_NULL;
    AT_REQ_RESP *current_com_at = AIC_NULL;

    data_mode_at_ptr = aic_get_serial_data_mode_at(at_com_id);
    if (!(data_mode_at_ptr && data_mode_at_ptr->data_mode_at &&
        (data_mode_at_ptr->data_mode_at->at_mode == AT_DATA_MODE ||
        data_mode_at_ptr->data_mode_at->at_mode == AT_TMP_CMD_MODE)))
        return AT_ERROR_CMD_NOT_FOUND;

    at_ptr = data_mode_at_ptr->data_mode_at;
    if (at_ptr->at_mode == AT_DATA_MODE) {
        at_com_item = aic_find_at_com_item(at_com_id);
        if (!at_com_item)
            return AT_ERROR_CMD_NOT_FOUND;

        current_com_at = at_com_item->serial_dispatch_at;
        if (!current_com_at || current_com_at->at_mode != AT_DATA_MODE)
            return AT_ERROR_CMD_NOT_FOUND;

        current_com_at->end_data_mode = 1;
        aos_set_event(event_dispatch_at, at_com_item->dispatch_event_id, AOS_OR);
    }
#ifdef CONFIG_CUSTOMER_AT
    else if (at_ptr->at_mode == AT_TMP_CMD_MODE) {
        invoke_at_cmd_handler(at_ptr);
        aic_release_tmp_cmd_mode_at(at_ptr);
    }
#endif

    return AIC_SUCCESS;
}

/**
 * aic_get_serial_data_mode_at - Get the AT which is waiting for data
 * payload from upper devices.
 * The AT is received from serial ports.
 */
SERIAL_DATA_MODE_AT *aic_get_serial_data_mode_at(u32 serial_port_id)
{
    u8 i;

    for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
        if (data_mode_at_list[i].serial_conn_id == serial_port_id)
            return &data_mode_at_list[i];
    }

    return AIC_NULL;
}

void clear_serial_data_mode_at(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 i;

    if (at_req_resp_ptr) {
        for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
            if (data_mode_at_list[i].serial_conn_id == at_req_resp_ptr->serial_port_id) {
                data_mode_at_list[i].serial_conn_id = AT_NO_SERIAL_PORT_ID;
                data_mode_at_list[i].data_mode_at = AIC_NULL;
            }
        }
    }
}

void set_serial_data_mode_at(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 i;
    SERIAL_DATA_MODE_AT *data_at_ptr = AIC_NULL;

    data_at_ptr = aic_get_serial_data_mode_at(at_req_resp_ptr->serial_port_id);
    if (data_at_ptr) {
        data_at_ptr->data_mode_at = at_req_resp_ptr;
        return;
    }

    for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
        if (data_mode_at_list[i].serial_conn_id == AT_NO_SERIAL_PORT_ID) {
            data_mode_at_list[i].serial_conn_id = at_req_resp_ptr->serial_port_id;
            data_mode_at_list[i].data_mode_at = at_req_resp_ptr;
            break;
        }
    }

    if (i == AT_MAX_SERIAL_PORT_NUM)
        aicx_error(0x05f6c9, "", "error,data mode at list full\n");
}
#endif

s32 aic_reply_error_resp(AT_REQ_RESP *at_req_resp_ptr, s32 error_value)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
             AT_RESULT_CME_ERROR"%d", error_value);
    line_pointers[0] = (ulong)&result_line[0];

    return aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
}

s32 aic_reply_ok_resp(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 result_line[4] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line, AT_RESULT_OK, (sizeof(result_line) - 1));
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

u8 aic_get_at_cmd_type(u8 *at_cmd)
{
    u8 cmd_type = AT_CMD_INVALID_TYPE;
    u8 at_cmd_head[AT_MAX_HEAD_LEN];
    u8 head_len;
    u8 *tmp_ptr;
    u16 cmd_len;

    cmd_len = strlen(at_cmd);
    aos_memset(at_cmd_head, 0, AT_MAX_HEAD_LEN);
    get_at_head_str(at_cmd, at_cmd_head);
    head_len = strlen(at_cmd_head);
    tmp_ptr = at_cmd + head_len;

    if (is_cr_lf_sign(*tmp_ptr)) {
        cmd_type = AT_CMD_TYPE_SET | AT_CMD_TYPE_EXE;
    } else {
        if (*tmp_ptr == '?' && is_cr_lf_sign(*(tmp_ptr + 1))) {
            cmd_type = AT_CMD_TYPE_QUERY;
        } else if (*tmp_ptr == '=') {
            if (cmd_len >= (head_len + 3) && *(tmp_ptr + 1) == '?' &&
                is_cr_lf_sign(*(tmp_ptr + 2))) {
                cmd_type = AT_CMD_TYPE_TEST;
            } else if ((cmd_len >= (head_len + 3) && *(tmp_ptr + 1) != '?') ||
                     is_cr_lf_sign(*(tmp_ptr + 1))) {
                cmd_type = AT_CMD_TYPE_SET | AT_CMD_TYPE_EXE;
            }
        } else {
            cmd_type = AT_CMD_INVALID_TYPE;
        }
    }

    return cmd_type;
}

/* The AT cmd is invalid,just reply ERROR. */
u32 handle_invalid_at_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aic_reply_error_resp(at_req_resp_ptr, AT_ERROR_INVALID_CMD);

    return AIC_SUCCESS;
}

void init_at_handle_list(void)
{
    u8 i;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (!at_list_head_inited) {
        for (i = 0; i < AT_MAX_HEAD_LEN; i++) {
            list_head_init(&at_cmd_handle_lists[i]);
        }
        at_list_head_inited = AIC_TRUE;
    }
    AOS_INTERRUPT_RESTORE
}

AT_CMD_HANDLE *get_at_cmd_handle(u8 *at_cmd_str)
{
    u8 at_cmd_head[AT_MAX_HEAD_LEN] = {'\0'};
    u8 len1, len2, cmp_len;
    AT_HANDLE_ITEM *at_handle_item = AIC_NULL;

    get_at_head_str(at_cmd_str, at_cmd_head);
    len1 = strlen(at_cmd_head);
    if (len1 == 0)
        return AIC_NULL;

    if (!is_ppp_dial_cmd(at_cmd_head)) {
        list_for_each_entry(at_handle_item, &at_cmd_handle_lists[len1 - 1],
                            list_key, AT_HANDLE_ITEM) {
            if (at_handle_item) {
                len2 = strlen(at_handle_item->at_handle->at_cmd);
                cmp_len = len1 >= len2 ? len1 : len2;
                if (!strncasecmp(at_cmd_head, at_handle_item->at_handle->at_cmd, cmp_len)) {
                    return at_handle_item->at_handle;
                }
            }
        }
    } else {
#ifdef CONFIG_PPP_DIAL
        if(!aic_boot_mode_is_basic())
            return aic_get_ppp_dial_cmd_handle();
#endif
    }

    return AIC_NULL;
}

boolean is_ppp_dial_cmd(u8 *cmd_str)
{
    if (!cmd_str)
        return AIC_FALSE;

    if (strstr(cmd_str, "ATD") && (strstr(cmd_str, "*99***") ||
        strstr(cmd_str, "*98*") || strstr(cmd_str, "*99#") ||
        strstr(cmd_str, "#777")))
        return AIC_TRUE;

    return AIC_FALSE;
}

void get_at_head_str(u8 *at_str, u8 *at_head_str)
{
    u8 *tmp_ptr_start, *tmp_ptr_end;
    u8 head_len, tmp_len;
    u8 *tmp_ptr;

    if (!at_str || !at_head_str)
        return;

    tmp_ptr_start = strcasestr(at_str, "AT");
    if (!tmp_ptr_start)
        return;

    tmp_ptr_end = strchr(at_str, '=');
    if (tmp_ptr_end) {
        tmp_len = tmp_ptr_end - tmp_ptr_start;
        head_len = tmp_len > (AT_MAX_HEAD_LEN - 1) ?
                   (AT_MAX_HEAD_LEN - 1) : tmp_len;
        strncpy(at_head_str, tmp_ptr_start, head_len);
    } else {
        tmp_ptr_end = strchr(at_str, '?');
        if (tmp_ptr_end) {
            tmp_len = tmp_ptr_end - tmp_ptr_start;
            head_len = tmp_len > (AT_MAX_HEAD_LEN - 1) ?
                       (AT_MAX_HEAD_LEN - 1) : tmp_len;
            strncpy(at_head_str, tmp_ptr_start, head_len);
        } else {
            /* The cmd has no parameter,the head is itself excluding \r\n. */
            strncpy(at_head_str, tmp_ptr_start, (AT_MAX_HEAD_LEN - 1));
            tmp_ptr = at_head_str;
            while (*tmp_ptr) {
                if (is_cr_lf_sign(*tmp_ptr)) {
                    *tmp_ptr = '\0';
                    break;
                }
                tmp_ptr++;
            }
        }
    }
}

/* Send the at cmd to CP,whether it is from external or internal module. */
u32 aic_send_at_to_cp(AT_REQ_RESP *at_req_resp)
{
    u32 ret = AIC_SUCCESS;
    AT_RECV_COM_ITEM *at_com_item;

    if (at_req_resp->is_send_to_cp == AT_SEND_TO_CP) {
        /* Maybe next at cmd is not sent to cp. */
        at_req_resp->is_waiting_resp = AT_WAITING_CP_RESP;

        if (at_req_resp->is_external_cmd == AT_FROM_INTERNAL) {
            aos_get_mutex(mutex_at_pending, AOS_WAIT_FOREVER);
            if (!at_pending_to_send) {
                at_pending_to_send = at_req_resp;
            } else {
                ret = append_to_pending_at(at_req_resp, at_pending_to_send);
            }
            aos_put_mutex(mutex_at_pending);
            aos_set_event(event_send_at, AT_EVENT_SEND_CMD_TO_CP, AOS_OR);

        } else if (at_req_resp->is_external_cmd == AT_FROM_EXTERNAL) {
            at_com_item = aic_find_at_com_item(at_req_resp->serial_port_id);
            aos_set_event(event_send_at, at_com_item->send_to_cp_event_id, AOS_OR);
        }
    }

    return ret;
}

void aic_free_at_req_resp(AT_REQ_RESP *at_ptr)
{
    if (at_ptr) {
        if (!at_ptr->at_req)
            return;

        aicx_info(0x0100bf, "", "free AT struct\n");
        aos_free(at_ptr->at_req);

        if (at_ptr->at_resp)
            aos_free(at_ptr->at_resp);

        aos_memset(at_ptr, 0, sizeof(AT_REQ_RESP));
        aos_free(at_ptr);
    }
}

void aic_set_cr_sign(u8 cr_char)
{
    old_cr_lf_signs[0] = current_cr_lf_signs[0];
    current_cr_lf_signs[0] = cr_char;
}

void aic_set_lf_sign(u8 lf_char)
{
    old_cr_lf_signs[1] = current_cr_lf_signs[1];
    current_cr_lf_signs[1] = lf_char;
}

u8 aic_get_cr_sign(void)
{
    return current_cr_lf_signs[0];
}

u8 aic_get_lf_sign(void)
{
    return current_cr_lf_signs[1];
}

boolean is_cr_lf_sign(u8 sign_char)
{
    if (sign_char == current_cr_lf_signs[0] || sign_char == old_cr_lf_signs[0] ||
        sign_char == current_cr_lf_signs[1] || sign_char == old_cr_lf_signs[1] ||
        sign_char == '\r' || sign_char == '\n')
        return AIC_TRUE;

    return AIC_FALSE;
}

/* Add list for processing at cmds received from several ports concurrently. */
void aic_add_to_multi_at_com_list(u32 at_com_id)
{
    AT_RECV_COM_ITEM *at_com_item;
    u8 at_port_index = 0;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (!multi_at_com_inited) {
        list_head_init(&multi_at_com_list);
        multi_at_com_inited = AIC_TRUE;
    }

    if (aic_find_at_com_item(at_com_id)) {
        AOS_INTERRUPT_RESTORE
        return;
    }

    at_com_item = aos_zalloc(sizeof(AT_RECV_COM_ITEM));
    AOS_ASSERT(at_com_item);
    at_com_item->at_com_id = at_com_id;
    at_port_index = aic_get_at_port_index(at_com_id);
    at_com_item->dispatch_event_id = BIT(at_port_index);
    at_com_item->send_to_cp_event_id = BIT(at_port_index + 8);
    at_com_item->timeout_event_id = BIT(at_port_index + 8);
    at_com_item->at_data_queue = aos_create_queue("at_data_queue", sizeof(void*),
                                                  AT_DATA_CACHE_COUNT);
    AIC_ASSERT(at_com_item->at_data_queue);

    /* Creat thread to dispatch AT cmds received from serial ports. */
    if (at_com_id == AT_DIAG_SERIAL_PORT_ID) {
        strcpy(at_com_item->thread_name, "dispatch_diag_at");
        at_com_item->thread_dispatch_at = aos_create_thread(at_com_item->thread_name,
                                               serial_at_dispatch_entry,
                                               at_com_item,
                                               SIZE_1K, 0, NULL,
                                               7, AOS_AUTO_START);
    } else {
        snprintf(at_com_item->thread_name, sizeof(at_com_item->thread_name),
                 "dispatch_at_com_%d", at_com_id);
        at_com_item->thread_dispatch_at = aos_create_thread(
                                          at_com_item->thread_name,
                                          serial_at_dispatch_entry,
                                          at_com_item,
                                          SIZE_4K, 0, NULL,
                                          AT_DISPATCH_TASK_PRIORITY,
                                          AOS_AUTO_START);
    }

    AIC_ASSERT(at_com_item->thread_dispatch_at);
    list_add_tail(&at_com_item->list_key, &multi_at_com_list);
    AOS_INTERRUPT_RESTORE
}

AT_RECV_COM_ITEM *aic_find_at_com_item(u32 at_com_id)
{
    AT_RECV_COM_ITEM *at_com_item = AIC_NULL;
    AT_RECV_COM_ITEM *tmp_at_com_item = AIC_NULL;

    if (!multi_at_com_inited)
        return AIC_NULL;

    list_for_each_entry_safe(at_com_item, tmp_at_com_item, &multi_at_com_list,
                             list_key, AT_RECV_COM_ITEM) {
        if (at_com_item->at_com_id == at_com_id)
            return at_com_item;
    }

    return AIC_NULL;
}

#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
u32 aic_set_pre_data_mode(AT_REQ_RESP *pre_data_at_ptr)
{
    aos_get_mutex(at_aync_data_mode.pre_data_mutex, AOS_WAIT_FOREVER);
    /* There is already AT cmd in pre_data_mode. */
    if (at_aync_data_mode.at_req_ptr)
        return AT_ERROR_PRE_DATA_AT_ENGAGED;

    if (pre_data_at_ptr) {
        pre_data_at_ptr->at_mode = AT_PRE_DATA_MODE;
        at_aync_data_mode.at_req_ptr = pre_data_at_ptr;
    }

    aos_put_mutex(at_aync_data_mode.pre_data_mutex);
    return AIC_SUCCESS;
}

u32 aic_enter_data_mode_after_connect(boolean connect_ok, u8 *urc_str, u8 urc_len)
{
    AT_RECV_COM_ITEM *at_com_item;
    u32 ret = AIC_SUCCESS;
    AOS_INTERRUPT_DECLARE

    if (!at_aync_data_mode.at_req_ptr)
        return AT_ERROR_NO_PRE_DATA_MODE_AT;

    aos_get_mutex(at_aync_data_mode.pre_data_mutex, AOS_WAIT_FOREVER);
    aicx_info(0x053eb5, "d", "after connect,connect_ok is %d\n", connect_ok);
    if (connect_ok) {
        at_aync_data_mode.at_urc_str = aos_dma_malloc(urc_len + 1);
        if (at_aync_data_mode.at_urc_str) {
            aos_memset(at_aync_data_mode.at_urc_str, 0, (urc_len + 1));
            aos_memcpy(at_aync_data_mode.at_urc_str, urc_str, urc_len);
            at_com_item = aic_find_at_com_item(at_aync_data_mode.at_req_ptr->serial_port_id);
            if (!at_com_item) {
                ret = AT_ERROR_NO_COM_FOUND;
                goto HANDLE_ERROR;
            }

            aos_put_mutex(at_aync_data_mode.pre_data_mutex);
            if (!at_com_item->serial_dispatch_at) {
                AOS_INTERRUPT_DISABLE
                at_com_item->serial_dispatch_at = at_aync_data_mode.at_req_ptr;
                at_com_item->serial_dispatch_at->next_ptr = AIC_NULL;
                at_com_item->serial_dispatch_at->at_handle_state = AT_EXT_CMD_NOT_HANDLED;
                AOS_INTERRUPT_RESTORE

                aos_set_event(event_dispatch_at, at_com_item->dispatch_event_id, AOS_OR);
            }
        } else {
            ret = AT_ERROR_MALLOC_FAIL;
            goto HANDLE_ERROR;
        }
    } else {
        aic_transmit_at_cmd_urc(at_aync_data_mode.at_req_ptr->serial_port_id,
                                urc_str, urc_len);
        aic_free_at_req_resp(at_aync_data_mode.at_req_ptr);
        at_aync_data_mode.at_req_ptr = AIC_NULL;
        aos_put_mutex(at_aync_data_mode.pre_data_mutex);
    }

    return AIC_SUCCESS;

HANDLE_ERROR:
    aos_put_mutex(at_aync_data_mode.pre_data_mutex);
    if (at_aync_data_mode.at_urc_str)
        aos_free(at_aync_data_mode.at_urc_str);

    aic_free_at_req_resp(at_aync_data_mode.at_req_ptr);
    at_aync_data_mode.at_req_ptr = AIC_NULL;

    return ret;
}
#endif

