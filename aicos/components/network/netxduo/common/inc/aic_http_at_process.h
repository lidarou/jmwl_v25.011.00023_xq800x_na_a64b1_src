/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_HTTP_AT_PROCESS_H__
#define __AIC_HTTP_AT_PROCESS_H__

#include <nx_web_http_common.h>
#include <nx_web_http_client.h>
#include <aic_http_comm.h>

#define HTTP_AT_CLIENT_NAME "AIC AT HTTP Client"
#define HTTP_MAX_REDIR_COUNT 3
#define HTTP_MAX_POST_DATA_LEN (12 * 1024)
/* define CME ERROR NUM */
#define AIC_HTTP_AT_PARAS_ERROR 1000
#define AIC_HTTP_THREAD_CREATE_ERROR 1001
#define AIC_HTTP_IP_ADDR_ERROR 1002
#define AIC_HTTP_INIT_ERROR 1003
#define AIC_HTTP_CLIENT_DELETE_ERROR 1004
#define AIC_HTTP_PACKET_ALLOCATE_ERROR 1005
#define AIC_HTTP_DATA_APPEND_ERROR 1006
#define AIC_HTTP_METHOD_EXECUTE_FAIL 1007
#define AIC_HTTP_EVENT_CREATE_FAIL 1008
#define AIC_HTTP_TERM_ERROR 1009
//#define AIC_HTTP_CLIENT_CREATE_ERROR 1010
#define AIC_HTTP_BUFFER_ALLOCATE_FAIL 1011
#define AIC_HTTP_NO_CREATED_CLIENT 1012
#define AIC_HTTP_SEND_PACKET_FAIL 1013
#define AIC_HTTP_INVALID_SEND_PACKET 1014
#define AIC_HTTP_PARA_OUT_OF_RANGE 1015
#define AIC_HTTP_INVALID_METHOD 1016
#define AIC_HTTP_GET_RESPONSE_FAIL 1017
#define AIC_HTTP_REDIR_EXECUTE_FAIL 1018

/* Define HTTP input paras. */
typedef struct aic_http_at_paras {
    u8 url[HTTPS_MAX_INPUT_PARA_LEN];
    u8 content_type[HTTPS_CONTENT_TYPE_LEN];
    u8 ua[HTTPS_MAX_INPUT_PARA_LEN];
    u8 user_data[HTTPS_MAX_INPUT_PARA_LEN];
    u8 cid;
    u8 redir;
    slong range_start;
    slong range_end;
    s32 timeout;
    ulong post_data_size;
    ulong read_data_len;
    u32 handle;
    u8 redir_count;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
} AIC_HTTP_AT_PARAS;

typedef struct aic_http_status_map {
    char *aic_http_status_string;
    u32 aic_http_status_code;
} AIC_HTTP_STATUS_MAP;

/* Define global functions. */
u32 aic_handle_http_init_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_init_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_para_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_para_query_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_para_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_data_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_data_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_action_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_action_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_read_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_read_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_term_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_http_term_test_cmd(AT_REQ_RESP *at_ptr);

#endif