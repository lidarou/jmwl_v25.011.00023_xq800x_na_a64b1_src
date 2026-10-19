/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the structs and apis for common AT process.
 */

#ifndef AIC_AT_PROCESS_COMMON_H
#define AIC_AT_PROCESS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include "aic_manufacturer_info.h"
#include "aic_model_info.h"
#include "aic_net_cfg.h"
#include <aic_osa.h>

#define AT_RESULT_OK "OK"
#define AT_RESULT_ERROR "ERROR"
#define AT_RESULT_CME_ERROR "+CME ERROR: "
#define AT_RESULT_NO_CARRIER "NO CARRIER"
#define AT_RESULT_CONNECT "CONNECT"

#define AT_CMD_S3 "ATS3"
#define AT_CMD_S4 "ATS4"
#define AT_CSCON_RESP_PREFIX "+CSCON: "
#define AT_CPIN_RESP_PREFIX "+CPIN: "

#define AT_FROM_EXTERNAL 1
#define AT_FROM_INTERNAL 0

#define AT_MAX_AP_RESP_LENGTH 160

#define AT_REPLY_LINE_COUNT_ONCE 5

#define AT_NO_SERIAL_PORT_ID 0xffff

#define AT_CMD_MODE 1
#define AT_DATA_MODE 2
/* Temporary cmd mode,maybe switch to data mode later. */
#define AT_TMP_CMD_MODE 3
#define AT_PRE_DATA_MODE 4

#define AT_RETRY_COUNT0 0
#define AT_RETRY_COUNT1 1

#define ANET_IN_FLIGHT_MODE 1
#define ANET_OUT_OF_FLIGHT_MODE 0

#define ANET_PLMN_MAX_LEN 8

#define ANET_PDP_MAX_NUM 5
#define ANET_DEFAULT_PDP_CID 1
#define ANET_IMS_EMERGENCY_PDP_CID 3
#define ANET_IMS_VOICE_PDP_CID 4
#define ANET_DEFAULT_IMS_PDP_CID 5
#define ANET_MIN_PDP_CID 1
#define ANET_MAX_PDP_CID 5

#define ANET_APN_NAME_MAX_LEN 60
/* Pdp type is IP,IPV6 or IPV4V6. */
#define ANET_PDP_TYPE_MAX_LEN 8
/* Apn type is default,supl,mms etc. */
#define ANET_APN_TYPE_MAX_LEN 20
#define ANET_AUTH_TYPE_LEN 2
#define ANET_AUTH_USERID_LEN 30
#define ANET_AUTH_PWD_LEN 30

#define ANET_APN_INVALID_AUTH_TYPE 0xff
#define ANET_APN_NO_AUTH_TYPE 0
#define ANET_APN_AUTH_TYPE_PAP 1
#define ANET_APN_AUTH_TYPE_CHAP 2
#define ANET_APN_AUTH_TYPE_PAP_OR_CHAP 3

#define ANET_MAX_LEN_IPV4_ADDR 20
#define ANET_MAX_LEN_DEC_IPV6_ADDR 70
#define ANET_MAX_LEN_HEX_IPV6_ADDR 48

/* The port id is virtual,just to seperate if the AT cmd is from DIAG or not.
 * CMUX port id is 0~60. */
#define AT_DIAG_SERIAL_PORT_ID 100
#define AT_USB_AT_PORT_ID 200

#define AT_ERROR_BASE 48
/* Errors replied to the upper device. */
#ifdef CONFIG_CUSTOMER_CM_AT
#define AT_ERROR_INVALID_PARAS 50
#else
#define AT_ERROR_INVALID_PARAS AT_ERROR_BASE + 1
#endif
#define AT_ERROR_CMD_TOO_LONG AT_ERROR_BASE + 2
#define AT_ERROR_INVALID_CMD AT_ERROR_BASE + 3
#define AT_ERROR_HANDLE_TIMEOUT AT_ERROR_BASE + 4
#define AT_ERROR_PARA_NOT_SUPPORT AT_ERROR_BASE + 5
#define AT_ERROR_HANDLE_FAILURE AT_ERROR_BASE + 6
#define AT_ERROR_CP_NOT_ALIVE AT_ERROR_BASE + 7
#define AT_ERROR_NO_IDLE_LINK AT_ERROR_BASE + 8
#define AT_ERROR_WRITE_LINK_FAIL AT_ERROR_BASE + 9
/* Errors that API returns. */
#define AT_ERROR_NO_SERIAL_CHANNEL AT_ERROR_BASE + 10
#define AT_ERROR_CMD_NOT_FOUND AT_ERROR_BASE + 11
#define AT_ERROR_INCOMPLETE_CMD AT_ERROR_BASE + 12
#define AT_ERROR_ENDDING_SIGN_ONLY AT_ERROR_BASE + 13
#define AT_ERROR_SERIAL_BUFFER_FULL AT_ERROR_BASE + 14
#define AT_ERROR_INVALID_RESP AT_ERROR_BASE + 15
#define AT_ERROR_INVALID_AT_PTR AT_ERROR_BASE + 16
#define AT_ERROR_PENDING_CMD_OVERFLOW AT_ERROR_BASE + 17
#define AT_ERROR_INVALID_AT_HANDLE AT_ERROR_BASE + 18
#define AT_ERROR_AT_HANDLES_OVERFLOW AT_ERROR_BASE + 19
#define AT_ERROR_AT_RESP_HANDLES_OVERFLOW AT_ERROR_BASE + 20
#define AT_ERROR_URC_HANDLES_OVERFLOW AT_ERROR_BASE + 21
#define AT_ERROR_NO_ABUF_HANDLE AT_ERROR_BASE + 22
#define AT_ERROR_URC_CBS_OVERFLOW AT_ERROR_BASE + 23
#define AT_ERROR_UNCOMPLETE_MSG_TEXT AT_ERROR_BASE + 24
#define AT_ERROR_NO_LINK_SEND_MSG_TEXT AT_ERROR_BASE + 25
#define AT_ERROR_MSG_TEXT_TOO_LONG AT_ERROR_BASE + 26
#define AT_ERROR_MSG_MEDIUM_RESULT AT_ERROR_BASE + 27
#define AT_ERROR_REENTER_DATA_MODE AT_ERROR_BASE + 28
#define AT_ERROR_FLIGHT_MODE_CBS_OVERFLOW AT_ERROR_BASE + 29
#define AT_ERROR_DATA_MODE_TIMEOUT AT_ERROR_BASE + 30
#define AT_ERROR_TIMEOUTS_OVERFLOW AT_ERROR_BASE + 31
#define AT_ERROR_PRE_DATA_AT_ENGAGED AT_ERROR_BASE + 32
#define AT_ERROR_NO_PRE_DATA_MODE_AT AT_ERROR_BASE + 33
#define AT_ERROR_MALLOC_FAIL AT_ERROR_BASE + 34
#define AT_ERROR_NO_COM_FOUND AT_ERROR_BASE + 35

typedef void (*aic_get_cp_resp_callback)(u8 *cmd_resp, u8 line_count);
typedef void (*aic_get_cp_urc_callback)(u8 *urc_str);

#ifdef CONFIG_D2D
#define AT_MSG_HEAD_ENABLED 1
#define AT_MSG_HEAD_DISABLED 0

#define AT_MSG_HEAD_LEN 8
/* The msg id for URC is not cared about. */
#define AT_URC_MSG_ID 0x01020304
/* The cmux com id for urc. */
#define AT_D2D_URC_CMUX_COM_ID 5

#define AT_D2D_COM_TYPE_CMUX "cmux"
#define AT_D2D_COM_TYPE_UART "uart"
#define AT_D2D_COM_TYPE_USB "usb"

typedef struct AT_D2D_URC_MSG_INFO_STRUCT
{
    u8 *urc_str;
    u16 urc_msg_type;
} AT_D2D_URC_MSG_INFO;
#endif

typedef struct AT_MSG_HEAD_STRUCT
{
    u16 msg_type;
    u16 cmd_len;
    u32 msg_id;
} AT_MSG_HEAD;

/* Define structs. */
/**
 * struct AT_REQ_RESP - The struct contains the complete info about certain
 * at cmd,such as the at string,the source,the response received from CP etc..
 * @is_external_cmd: If the cmd is sent by external devices.
 * @at_cmd_type: The cmd type,such as set,exe,test and query.
 * @retry_count: The times to retry when timeout for receiving response from CP.
 * @resp_line_count: The line count of response received from CP.
 * @is_send_to_cp: The cmd would be sent to CP or not.
 * @at_mode: The cmd is in cmd mode or data mode.If in data mode,then it will
 *  wait for data payload transmitted via the serial port.
 * @serial_port_id: The serial port id via which the cmd is received.
 * @timeout: The time to expire.
 * @at_req: The complete at cmd string.
 * @at_upstream_data: The data payload received from the serial port.
 * @at_resp: The response received from CP.It may include several lines.
 * @at_urc: The URC received from CP,just one line.
 * @cp_urc_len: The length of URC received from CP.
 * @at_timeout_handler: The handler when time expires to receive resp from CP.
 */
typedef struct AT_REQ_RESP_STRUCT
{
    u8 is_external_cmd;
    u8 at_cmd_type;
    u8 is_waiting_resp;
    u8 assigned_link_id;
    u8 retry_count;
    u8 resp_line_count;
    u8 is_send_to_cp;
    u8 at_mode;
    u8 at_handle_state;  /* The member is only for AT cmds handled by AP. */
    u8 end_data_mode;
    void *at_cmd_handle;
    u32 serial_port_id;
    u32 diag_sn;
    u8 diag_sys;
    u32 timeout;
    u32 start_time;
    u8 *at_req;
    u8 *at_upstream_data;
    u16 upstream_data_len;
    u8 *at_resp;
    u32 resp_buf_len;
    u32 current_resp_len;
    u8 *at_urc;
    u16 cp_urc_len;
    aos_semaphore_ptr wait_resp_semaphore;
    struct AT_ABUF_LINK_CFG_STRUCT *abuf_link_ptr;
    /* For at cmd received with msg head. */
    AT_MSG_HEAD msg_head;

    aic_get_cp_resp_callback inform_resp_cb;
    s32 (*at_resp_handler)(struct AT_REQ_RESP_STRUCT *at_to_ap);
    s32 (*at_timeout_handler)(struct AT_REQ_RESP_STRUCT *at_to_ap);

    struct AT_REQ_RESP_STRUCT *next_ptr;
} AT_REQ_RESP;

/**
 * struct AT_CMD_HANDLE - The struct used to handle certain at cmd
 * received from the serial port.The comand is a set command or exe command,
 * so one of handle_at_set_cmd and handle_at_exe_cmd should be null.
 * @at_cmd: The starting keyword of the at cmd,such as AT+CGACT.
 * @handle_at_test_cmd: The handler for the at cmd of test type.
 * @handle_at_query_cmd: The handler for the at cmd of query type.
 * @handle_at_set_cmd: The handler for the at cmd of set type.
 * @handle_at_exe_cmd: The handler for the at cmd of execute type.
 */
typedef struct AT_CMD_HANDLE_STRUCT
{
    u8 *at_cmd;
    u32 (*handle_at_test_cmd)(AT_REQ_RESP *at_req_resp);
    u32 (*handle_at_query_cmd)(AT_REQ_RESP *at_req_resp);
    u32 (*handle_at_set_cmd)(AT_REQ_RESP *at_req_resp);
    u32 (*handle_at_exe_cmd)(AT_REQ_RESP *at_req_resp);
} AT_CMD_HANDLE;

/**
 * struct AT_RESP_FROM_CP_HANDLE - The struct used to handle the response
 * received from CP for certain at cmd.
 * @at_cmd: The starting keyword of the at cmd,such as AT+CGACT.
 * @at_test_resp_handler: The handler for the response of at test cmd.
 * @at_query_resp_handler: The handler for the response of at query cmd.
 * @at_set_resp_handler: The handler for the response of at set cmd.
 * @at_exe_resp_handler: The handler for the response of at exe cmd.
 */
typedef struct AT_RESP_FROM_CP_HANDLE_STRUCT
{
    u8 *at_cmd;
    s32 (*at_test_resp_handler)(AT_REQ_RESP *at_req_resp);
    s32 (*at_query_resp_handler)(AT_REQ_RESP *at_req_resp);
    s32 (*at_set_resp_handler)(AT_REQ_RESP *at_req_resp);
    s32 (*at_exe_resp_handler)(AT_REQ_RESP *at_req_resp);
    s32 (*at_resp_timeout_handler)(AT_REQ_RESP *at_req_resp);
} AT_RESP_FROM_CP_HANDLE;

/**
 * struct AT_URC_FROM_CP_HANDLE - The struct used to handle the URC
 * infomation received from CP.
 * @urc_keyword: The starting keyword of the URC line,such as +CGEV: .
 * @at_urc_handler: The handler of the URC string.The urc_str is one line.
 */
typedef struct AT_URC_FROM_CP_HANDLE_STRUCT
{
    u8 *urc_keyword;
    u32 (*at_urc_handler)(u8 *urc_str, u8 *start_keyword);
} AT_URC_FROM_CP_HANDLE;

typedef struct AT_RESP_FROM_CP_STRUCT
{
    u8 *resp_buf;
    u16 buf_len;
    u8 resp_line_cnt;
} AT_RESP_FROM_CP;

typedef struct ASYNC_DATA_MODE_AT_STRUCT
{
    AT_REQ_RESP *at_req_ptr;
    aos_mutex_ptr pre_data_mutex;
    u8 *at_urc_str;
} ASYNC_DATA_MODE_AT;

typedef struct REG_SERVICE_INFO_STRUCT
{
    u8 operator_plmn[ANET_PLMN_MAX_LEN];
    u8 default_apn_name[ANET_APN_NAME_MAX_LEN];
    u8 auth_type;
    u8 cs_reg_state;
    u8 ps_reg_state;
    u8 flight_mode;
    u8 sim_card_state;
    u8 cereg_mode;
    u8 cgerep_mode;
    u8 ctzr_mode;
    u8 cardstatus_mode;
    u8 ctzu_mode;
    u8 act_type;
    u8 ims_enabled;
    boolean is_powering_off;
} REG_SERVICE_INFO;

/* Define global functions. */
u32 aic_add_at_cmd_handle(AT_CMD_HANDLE *head_handle_ptr);
u32 aic_add_at_resp_handle(AT_RESP_FROM_CP_HANDLE *head_resp_handle);
s32 aic_transmit_at_data(u8 serial_port_id, u8 *data_buf, u16 data_len);
ulong *aic_get_each_line_from_resp(u8 *at_resp, u16 line_count);
ulong *aic_split_paras_to_str_array(u8 *raw_str, u8 *start_str, u16 *paras_cnt);
void aic_free_paras_mem(ulong *ptr, u16 paras_cnt);
u32 aic_add_at_urc_handle(AT_URC_FROM_CP_HANDLE *head_handle_ptr);
s32 aic_assemble_and_handle_at_cmd(u8 *at_str, ulong *at_req_resp,
                                   u8 is_external, u32 serial_port_id,
                                   u8 retry_cnt);
s32 aic_assemble_at_cmd_with_head(u8 *at_str, AT_MSG_HEAD *msg_head_ptr, ulong *at_req_resp,
                                  u8 is_external, u32 serial_port_id, u8 retry_cnt);
u32 aic_get_at_port_id(void);
s32 aic_reply_error_resp(AT_REQ_RESP *at_req_resp_ptr, s32 error_value);
s32 aic_fill_transmit_cmd_resp(AT_REQ_RESP *at_req_resp,
                               ulong *reply_line_ptr, u16 line_cnt);
s32 aic_reply_ok_resp(AT_REQ_RESP *at_req_resp_ptr);
#ifdef CONFIG_MISC_SUPPORT
s32 aic_write_manufacturer_info(MANUFACTURER_INFO *manufacturer_name);
s32 aic_write_model_info(MODEL_INFO *model_info);
s32 aic_save_config_to_nv(NET_CFG *net_cfg);
s32 aic_read_net_config(NET_CFG *net_cfg);
#endif
u32 aic_transmit_reply_to_external(AT_REQ_RESP *at_to_ap);
s32 aic_reply_to_certain_com(u32 com_id, u8 *reply_str, u32 reply_size);
void aic_set_at_handle_mode(AT_REQ_RESP *at_req_resp, u8 at_mode);
s32 aic_format_transmit_reply(u32 serial_port_id, u8 *reply_str,
                              u32 reply_size, boolean is_urc);

#define aic_transmit_at_result(at_port_id, result_str, result_size) \
aic_format_transmit_reply((at_port_id), (result_str), (result_size), AIC_FALSE);

#define aic_transmit_at_cmd_urc(at_port_id, urc_str, urc_size) \
aic_format_transmit_reply((at_port_id), (urc_str), (urc_size), AIC_TRUE);

//void aic_transmit_ap_urc_without_ports(u8 *urc_ptr, u16 urc_len);
u32 aic_send_cmd_with_callback(u8 *at_str, aic_get_cp_resp_callback cb);
u32 aic_send_cmd_and_wait(u8 *at_str, AT_RESP_FROM_CP *resp_to_save);
u32 aic_register_cp_urc_callback(aic_get_cp_urc_callback cb);
void aic_unregister_cp_urc_callback(aic_get_cp_urc_callback cb);
u8 *fill_head_and_end_signs(u8 *dst_ptr, u8 *src_str, u32 src_len);

typedef void (*aic_flight_mode_change_cb)(u8 flight_mode_enabled);
u32 aic_register_flight_mode_change_cb(aic_flight_mode_change_cb cb);
void aic_unregister_flight_mode_change_cb(aic_flight_mode_change_cb cb);
boolean aic_is_valid_integer_para(u8 *para_str, u8 para_len);
boolean aic_is_ps_registered(void);
boolean aic_is_sim_card_plugged(void);
boolean aic_is_valid_pdp_cid(u16 cid);
boolean aic_is_ims_pdp_cid(u8 pdp_cid);
void aic_enter_flight_mode(void);
void aic_set_flight_mode(u8 mode);
u8 aic_get_flight_mode(void);
boolean aic_is_flight_mode(void);
s32 aic_transimt_reply_no_copy(u32 at_com_id, u8 *buf_ptr, u32 buf_size);

typedef void (*cereg_update_cb)(u8 cereg_state);
u32 aic_register_cereg_state_cb(cereg_update_cb cb);
/* Only for modem driver to send AT. */
u32 aic_send_md_cmd_with_callback(u8 *at_str, aic_get_cp_resp_callback cb);
u32 aic_set_data_mode_expire_time(u8 *at_head, u32 expire_time_ms);
u32 aic_end_at_data_mode(u32 at_com_id);
u8 aic_get_reg_act_type(void);
u32 aic_escape_data_normal(u8 *dst, u8 *src, int datalen, int *actual_len);

typedef void (*ephystat_update_cb)(u8 ephy_state);
u32 aic_register_ephystat_state_cb(ephystat_update_cb cb);
void aic_check_data_mode_at_by_port(u32 at_port_id);
s32 aic_load_net_nv_cfg(void);
#ifdef CONFIG_CUSTOMER_CM_AT
typedef void (*cscon_update_cb)(u8 mode, u8 state, u8 access);
u32 aic_register_cscon_mode_cb(cscon_update_cb cb);
#endif

#endif /* AIC_AT_PROCESS_COMMON_H */
