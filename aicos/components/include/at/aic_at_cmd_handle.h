/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  This file defines the structs and apis for at cmd handling
 *  and abuf link configuration.
 */

#ifndef AIC_AT_CMD_HANDLE_H
#define AIC_AT_CMD_HANDLE_H

#include <aic_size.h>
#include <aic_list.h>
#include <aipc_api.h>
#include "aic_at_process_common.h"

#define AT_CMD_INVALID_TYPE 0
#define AT_CMD_TYPE_TEST BIT(0)
#define AT_CMD_TYPE_QUERY BIT(1)
#define AT_CMD_TYPE_SET BIT(2)
#define AT_CMD_TYPE_EXE BIT(3)

#define AT_WAITING_CP_RESP 1
#define AT_RECEIVED_CP_RESP 0

#define AT_EXT_CMD_NOT_HANDLED 0
#define AT_EXT_CMD_HANDLING 1
#define AT_EXT_CMD_HANDLED_OVER 2

#define AT_SEND_TO_CP 1
#define AT_NOT_SEND_TO_CP 0

#define AT_NO_CMD_HANDLE_INDEX -1

#define AT_MAX_SERIAL_PORT_NUM 8

/* The shortest at cmd is: AT\r. */
#define AT_MIN_CMD_LENGTH 3
/** The standard max len of at cmd is 256,But the customers' AT may be
 * much longer,especially the cmds with data as parameter.
 */
#define AT_MAX_AP_CMD_LENGTH (SIZE_2K + SIZE_1K)
#define AT_MAX_CP_CMD_LENGTH SIZE_1K
#define AT_MAX_HEAD_LEN 25

#define AT_MAX_MANUFACTURER_ID_LENGTH 30
#define AT_MAX_PRODUCT_ID_LENGTH 50
#define AT_MAX_MODEL_ID_LENGTH 50

#define AT_MAX_CMDS_AP_HANDLED 150

#define AT_PENDING_MAX_CMD_NUM 30

#define AT_DATA_CACHE_COUNT 10

#define AT_DATA_MODE_TIMEOUT_CNT 10

#define AT_DISPATCH_TASK_PRIORITY 9

/* The default time is 5 minutes. */
#define AT_TMP_CMD_MODE_TIMEOUT (5 * 60000)
#define AT_DATA_MODE_TIMEOUT (5 * 60000)

/* Define structs. */
typedef struct AT_ABUF_LINK_CFG_STRUCT
{
    u8 link_id;
    u8 resp_complete;
    u8 link_state;
    u8 activated_pdp_cid; /* the PDP activated via CGDATA */
    u8 is_link_timeout;
    u8 *rev_buf;
    aos_mutex_ptr abuf_link_mutex;
    /* The struct whose AT cmd is sent by the abuf link. */
    AT_REQ_RESP *at_req_resp;
} AT_ABUF_LINK_CFG;

typedef struct AT_ABUF_LINK_CFG_SET_STRUCT
{
    aos_event_ptr abuf_link_recv_event;
    u32 link_set;
    AT_ABUF_LINK_CFG at_abuf_link_list[AT_LINK_CNT];
} AT_ABUF_LINK_CFG_SET;

typedef struct SERIAL_DATA_MODE_AT_STRUCT
{
    u32 serial_conn_id;
    AT_REQ_RESP *data_mode_at;
} SERIAL_DATA_MODE_AT;

typedef struct AT_HANDLE_ITEM_STRUCT
{
    struct list_head list_key;
    AT_CMD_HANDLE *at_handle;
} AT_HANDLE_ITEM;

typedef struct AT_DATA_ITEM_STRUCT
{
    u8 *data_buf;
    u32 data_len;
} AT_DATA_ITEM;

typedef struct DATA_MODE_EXPIRE_TIME_STRUCT
{
    u8 at_head[AT_MAX_HEAD_LEN];
    u32 expire_time;
} DATA_MODE_EXPIRE_TIME;

typedef struct AT_RECV_COM_ITEM_STRUCT
{
    struct list_head list_key;
    u32 at_com_id;
    u8 thread_name[24];
    aos_thread_id thread_dispatch_at;
    AT_REQ_RESP *serial_dispatch_at;
    u32 dispatch_event_id;
    u32 send_to_cp_event_id;
    u32 timeout_event_id;
    u8 *collected_at_ptr;
    u32 collected_len;
    aos_queue_ptr at_data_queue;
} AT_RECV_COM_ITEM;

/* Define global functions. */
u32 aic_serial_at_dispatch_init(void);
SERIAL_DATA_MODE_AT *aic_get_serial_data_mode_at(u32 serial_port_id);
u32 aic_send_at_to_cp(AT_REQ_RESP *at_req_res);
void aic_free_at_req_resp(AT_REQ_RESP *at_ptr);
void aic_invoke_next_external_at(u32 at_port_id);
void aic_at_cmd_ap_init(void);
u32 aic_get_at_resp_timeout(u8 *at_str, u8 is_to_cp);
boolean aic_is_cfun_cmd(u8 *cmd_str, u8 is_start_cp);
boolean aic_is_valid_at_cmd(u8 * at_str);
void aic_set_cr_sign(u8 cr_char);
void aic_set_lf_sign(u8 lf_char);
u8 aic_get_cr_sign(void);
u8 aic_get_lf_sign(void);
boolean is_cr_lf_sign(u8 end_char);
void get_at_head_str(u8 *at_str, u8 *at_head_str);
void aic_put_at_data_to_queue(AT_RECV_COM_ITEM *at_com_item, u8 *data_ptr, u16 len);
u8 aic_get_at_cmd_type(u8 *at_cmd);
#ifdef CONFIG_CUSTOMER_AT
boolean aic_allowed_to_enter_data_mode(u32 at_port_id);
void aic_release_tmp_cmd_mode_at(AT_REQ_RESP *at_ptr);
#endif
void aic_add_to_multi_at_com_list(u32 at_com_id);
AT_RECV_COM_ITEM *aic_find_at_com_item(u32 at_com_id);
#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
u32 aic_set_pre_data_mode(AT_REQ_RESP *pre_data_at_ptr);
u32 aic_enter_data_mode_after_connect(boolean connect_ok, u8 *urc_str, u8 urc_len);
#endif

#endif  /* AIC_AT_CMD_HANDLE_H */
