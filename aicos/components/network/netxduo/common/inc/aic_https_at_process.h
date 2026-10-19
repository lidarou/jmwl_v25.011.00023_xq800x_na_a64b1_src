/******************************************************************************/
/*                                                                            */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_HTTPS_AT_PROCESS_H__
#define __AIC_HTTPS_AT_PROCESS_H__

#include <nx_web_http_common.h>
#include <nx_web_http_client.h>
#include <aic_http_comm.h>

/* Define HTTP Client info. */
#define HTTPS_AT_CLIENT_NAME "AIC AT HTTPS Client"
#define HTTPS_NO_REDIR 0
#define HTTPS_NEED_REDIR 1
#define HTTPS_CA_CRT_ID 1
#define HTTPS_CLIENT_CRT_ID 2
#define HTTPS_MAX_REDIR_COUNT 3

/* define CME ERROR NUM */
#define AIC_HTTPS_AT_PARAS_ERROR 1030
#define AIC_HTTPS_THREAD_CREATE_ERROR 1031
#define AIC_HTTPS_PACKET_ALLOCATE_ERROR 1032
#define AIC_HTTPS_DATA_APPEND_ERROR 1033
#define AIC_HTTPS_METHOD_EXECUTE_FAIL 1034
#define AIC_HTTPS_SEND_PACKET_FAIL 1035
#define AIC_HTTPS_EVENT_CREATE_FAIL 1036
#define AIC_HTTPS_TERM_ERROR 1037
#define AIC_HTTPS_CLIENT_CREATE_ERROR 1038
#define AIC_HTTPS_POST_DATA_EMPTY 1039
#define AIC_HTTPS_NO_CREATED_CLIENT 1040
#define AIC_HTTPS_CLIENT_REPEATED_REQUEST 1041
#define AIC_HTTPS_CRT_SET_ERROR 1042
#define AIC_HTTPS_CRT_INFO_EMPTY 1043
#define AIC_HTTPS_PARA_OUT_OF_RANGE 1044
#define AIC_HTTPS_BUF_ALLOCATE_FAIL 1045
#define AIC_HTTPS_GET_RESPONSE_FAIL 1046
#define AIC_HTTPS_REDIR_EXECUTE_FAIL 1047

typedef struct aic_https_at_paras {
    u8 url[HTTPS_MAX_INPUT_PARA_LEN];
    u8 user_data[HTTPS_MAX_INPUT_PARA_LEN];
    u8 ua[HTTPS_MAX_INPUT_PARA_LEN];
    u8 content_type[HTTPS_CONTENT_TYPE_LEN];
    u8 cid;
    u8 redir;
    slong range_start;
    slong range_end;
    s32 timeout;
    ulong post_data_size;
    ulong data_copied;
    ulong read_data_len;
    ulong crt_len;
    u32 handle;
    u8 crt_flag;
    u8 redir_count;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    u8 *post_buf;
} AIC_HTTPS_AT_PARAS;

typedef struct aic_https_at_crt_info {
    u8 *ca_crt;
    u32 ca_crt_len;
    u8 *client_crt;
    u32 client_crt_len;
    u8 *priv_key;
    u32 priv_key_len;
} AIC_HTTPS_AT_CRT_INFO;

/* Define global functions. */
u32 aic_handle_https_cfg_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_cfg_query_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_cfg_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_get_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_get_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_post_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_post_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_range_get_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_range_get_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_data_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_data_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_read_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_read_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_setcrt_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_setcrt_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_term_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_term_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_init_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_init_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_setcrt_add_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_https_setcrt_add_test_cmd(AT_REQ_RESP *at_ptr);

#endif
