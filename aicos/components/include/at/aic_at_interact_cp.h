/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the structs and apis for sending and receiving,
 *    AT cmds and responses with CP.
 */

#ifndef AIC_AT_INTERACT_CP_H
#define AIC_AT_INTERACT_CP_H

#include <aipc_api.h>
#include <subsys_api.h>
#include <aic_at_cmd_handle.h>

/* \r\nxxx\r\n,it must be more than 4 bytes. */
#define AT_MIN_RESP_LENGTH 4
/** The length is defined for response from CP.
 * If the response if from AP,it may be a data packet,
 * so the length would be longer than 400.
 */
#define AT_MAX_ONE_RESP_LENGTH 800
#define AT_MAX_RESP_TOTAL_LENGTH 1024
#define AT_MAX_CP_URC_LENGTH 400

#define AT_RESP_NOT_COMPLETED 0
#define AT_RESP_COMPLETED 1
#define AT_RESP_BLOCKED 2

#define AT_ABUF_LINK_IDLE 0
#define AT_ABUF_LINK_RESERVED 1
#define AT_ABUF_LINK_BUSY 2
#define AT_ABUF_LINK_WAIT_MESSAGE 3

#define AT_ABUF_TIMEOUT 1
#define AT_ABUF_NOT_TIMEOUT 0

/* Define event flags for serial at receive thread control.
 * send_to_cp_event_id uses values more than 0x0100.
 */
#define AT_EVENT_SEND_CMD_TO_CP 0x01
#define AT_EVENT_SEND_INIT_CMD 0x02
#define AT_EVENT_ABORT_LINK 0x04
#define AT_EVENT_REGULAR_TIMEOUT 0x08

#define AT_TIMER_INTERVAL 1000

#define AT_START_CP 1
#define AT_STOP_CP 0
#define AT_SWITCH_OFF_PS 4

#define AT_RESET_INVOKED 1
#define AT_RESET_NOT_INVOKED 0

#define AT_COM_PORT_NUM 3

#define AT_SEND_TO_USB_TASK_PRIO (AT_DISPATCH_TASK_PRIORITY - 1)
#define AT_SEND_TO_UART_TASK_PRIO (AT_DISPATCH_TASK_PRIORITY - 3)
#define AT_REV_FROM_CP_TASK_PRIO 15

#define AT_REPLY_USB_CACHE_COUNT 6
#define AT_REPLY_UART_CACHE_COUNT 50

#define AT_WRITE_ABUF_WAIT_TIME 500

#define AT_MAX_CP_URC_CBS 5

#define AT_MAX_FLIGHT_MODE_CBS 2

#define AT_CEREG_DEF_MODE 2
#define AT_CGEREP_DEF_MODE 1
#define AT_CTZR_DEF_MODE 3
#define AT_CARDSTATUS_DEF_MODE 1
#define AT_CTZU_DEF_MODE 1

#define AT_CEREG_DISABLE_MODE 0
#define AT_CGEREP_DISABLE_MODE 0
#define AT_CTZR_DISABLE_MODE 0
#define AT_CARDSTATUS_DISABLE_MODE 0
#define AT_CTZU_DISABLE_MODE 0

typedef struct AT_REPLY_TO_COM_STRUCT
{
    struct list_head list_key;
    u32 com_id;
    u32 diag_sn;
    u8 port_type;
    u8 diag_sys;
    u8 event_id;
    u8 thread_name[24];
    aos_mutex_ptr send_mutex;
    aos_thread_id reply_thread;
    aos_queue_ptr reply_queue;
} AT_REPLY_TO_COM;

typedef struct REPLY_TO_COM_ITEM_STRUCT
{
    u8 *reply_ptr;
    u32 reply_len;
    boolean need_wakeup;
    u32 at_com_id;
} REPLY_TO_COM_ITEM;

typedef struct CP_URC_ITEM_STRUCT
{
    u8 *urc_ptr;
    u32 urc_len;
} CP_URC_ITEM;

typedef struct INIT_NET_CFG_STRUCT
{
    NET_CFG net_cfg;
    u8 init_read_flag;
} INIT_NET_CFG;

typedef struct CMUX_PORT_CFG_STRUCT
{
    u8 port_cnt;
    u32 *port_ids;
} CMUX_PORT_CFG;

/* Define public functions. */
u32 aic_at_interact_cp_init();
void aic_set_abuf_link_idle(AT_ABUF_LINK_CFG *abuf_link_cfg);
subsys_state_t aic_get_cp_state(void);
void aic_init_send_reply_to_diag(u32 diag_sn, u8 diag_sys);
u32 aic_send_cfun_cmd(u8 start_cp);
void aic_send_to_at_port(u32 com_id, u8 *reply_str, u32 len);
u32 aic_send_msg_content_and_wait(u8 *msg_buf, u16 len, AT_RESP_FROM_CP *resp_saved);
void block_sending_msg_content(AT_REQ_RESP *at_req_resp, u32 error_num);
boolean aic_is_message_medium_result(u8 *medium_resp, u32 len);
boolean aic_contain_final_result_code(u8 *resp_str, u8 line_cnt);
AT_REPLY_TO_COM *aic_get_reply_to_com_ptr(u32 serial_port_id);
void aic_put_reply_to_queue(u8 *resp_str, u32 len, u32 com_id,
                            boolean to_copy, boolean need_wakeup);
boolean is_final_result_code(u8 *at_resp_str, u32 len, boolean has_leading_sign);
#ifdef CONFIG_CUSTOMER_AT
void aic_invoke_sending_urc_cached(void);
/* Not proper for +CMGR +CMT +CMGL,whose format is not regular. */
u16 aic_refit_to_special_format(u8 *resp_ptr, u16 resp_len, u8 line_cnt);
#endif
void aic_set_abuf_link_reserved(u8 link_id);
u8 aic_get_abuf_link_state(u8 link_id);
s32 aic_reply_to_uart_with_wakeup(u32 com_id, u8 *reply_str, u16 reply_size);
void aic_add_reply_to_com_list(u32 at_com_id, u32 diag_sn, u8 diag_sys);
boolean aic_is_uart_com(u32 com_id);
boolean aic_is_cmux_com(u32 com_id);
#ifdef CONFIG_CUSTOMER_CM_AT
void aic_refit_new_format_resp(AT_REQ_RESP *at_req_ptr);
#endif
#if CONFIG_CUSTOMER_CM_AT || CONFIG_CUSTOMER_AT
boolean aic_need_refit_resp_format(AT_REQ_RESP *at_req_ptr);
#endif
void aic_at_mims_query_callback(u8 *cmd_resp, u8 line_count);

#endif  /* AIC_AT_INTERACT_CP_H */
