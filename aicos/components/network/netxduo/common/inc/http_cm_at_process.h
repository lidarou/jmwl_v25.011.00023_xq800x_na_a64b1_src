/******************************************************************************/
/*                                                                            */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __HTTP_CM_AT_PROCESS_H__
#define __HTTP_CM_AT_PROCESS_H__

#include <aic_http_client.h>
#include <aic_http_comm.h>
#include <aic_https_client.h>

#define MOBILE_HTTP_INVALID_ID 0xF
#define MOBILE_HTTP_MAX_CLIENT_NUM 4
#define MOBILE_HTTP_MAX_SSL_ID 5
#define MOBILE_HTTP_URL_HEADER_LEN 7 /* http:// */
#define MOBILE_HTTPS_URL_HEADER_LEN 8 /* https:// */
#define MOBILE_HTTP_URC_HEADER_LEN 48 /* +MHTTPURC:.... */
#define MOBILE_HTTP_MAX_PARA_ID 9

#define MOBILE_HTTP_QUEUE_NUM 12
#define MOBILE_HTTP_MAX_HEADER_NUM 10
#define MOBILE_HTTP_MAX_COMMON_HEADER_LEN 1024
#define MOBILE_HTTP_MAX_HEADER_LEN 4096
#define MOBILE_HTTP_MAX_CONTENT_DATA_LEN 4096
#define MOBILE_HTTP_MAX_RES_URC_DATA_LEN 2048
#define MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN 1600
#define MOBILE_HTTP_ONE_RESPONSE_HEX_DATA_BUF_LEN 4096
#define MOBILE_HTTP_MAX_CACHE_LEN 8192
#define MOBILE_HTTP_MAX_URL_LEN 256
#define MOBILE_HTTP_MAX_RESOURCE_LEN 4096
#define MOBILE_HTTP_MAX_HOST_LEN 128
#define MOBILE_HTTP_MAX_LOCAL_PATH_LEN 256
#define MOBILE_HTTP_URC_FLOWCTRL_VALUE  (180 * 1024)

#define MOBILE_HTTP_MAX_FRAG_SIZE 1024
#define MOBILE_HTTP_MAX_INTERVAL_TIME 2000 //ms
#define MOBILE_HTTP_CONNECT_TIMEOUT 180 //180 s
#define MOBILE_HTTP_RESPONSE_TIMEOUT 60 //60 s
#define MOBILE_HTTP_INPUT_TIMEOUT 120 //120 s
#define MOBILE_HTTP_URC_DELAY_VALUE  5 //ms

#define MOBILE_HTTP_DEFAULT_CONNECT_TIMEOUT (60 *1000) //60 s
#define MOBILE_HTTP_DEFAULT_INPUT_TIMEOUT (10 * 1000)  //10 s
#define MOBILE_HTTP_DEFAULT_CACHE_LEN 4096
#define MOBILE_HTTP_DL_TIMEOUT (10 *1000) //10 s
#define MOBILE_HTTP_DEFAULT_WAIT_TIME (3 *1000) //3s

#define PRINT_ALL 1
#define PRINT_ONE 0
#define MOBILE_HTTP_ESC 0x1B
#define MOBILE_HTTP_CTRL_Z 0x1A

#define MOBILE_HTTP_HEADER_DATA 0
#define MOBILE_HTTP_CONTENT_DATA 1

#define MOBILE_HTTP_SSL_ENABLE 1
#define MOBILE_HTTP_SSL_DISABLE 0

#define MOBILE_HTTP_ASCII 0
#define MOBILE_HTTP_HEX 1
#define MOBILE_HTTP_ESCAPE 2

#define MOBILE_HTTP_END_DATA_EOF 0
#define MOBILE_HTTP_MORE_DATA_EOF 1
#define MOBILE_HTTP_CLEAR_DATA_EOF 2

#define MOBILE_HTTP_SP_HEADER_ID 1
#define MOBILE_HTTP_COMMON_HEADER_ID 2
#define MOBILE_HTTP_HEADER_SET 0
#define MOBILE_HTTP_CONTENT_SET 1

#define MOBILE_HTTP_METHOD_GET              0x1
#define MOBILE_HTTP_METHOD_POST             0x2
#define MOBILE_HTTP_METHOD_PUT              0x3
#define MOBILE_HTTP_METHOD_DELETE           0x4
#define MOBILE_HTTP_METHOD_HEAD             0x5

#define MOBILE_HTTP_CLIENT_NAME     "M_HTTP_C"
#define MOBILE_HTTPS_CLIENT_NAME    "M_HTTPS_C"
#define MOBILE_HTTP_DL_CLIENT_NAME  "M_HTTP_C_DL"
#if defined(CONFIG_FS_FATFS) && defined(CONFIG_CUSTOMER_CM_AT)
#define MOBILE_HTTP_HOME_PATH       "C:/home"
#define MOBILE_HTTP_MKDIR_PATH      "C:/home/etc/http_file/"
#else
#define MOBILE_HTTP_HOME_PATH       "C:"
#define MOBILE_HTTP_MKDIR_PATH      "C:/etc/http_file/"
#endif
#define MOBILE_HTTP_DEFAULT_DL_PATH "/etc/http_file/"

#define MOBILE_HTTP_QUEUE_NAME      "m_http_q"
#define MOBILE_HTTP_DL_QUEUE_NAME   "m_http_q_dl"

#ifdef  CONFIG_GX_AT_V2
#define HTTP_CFG_RESP        "GHTTPCFG"
#define HTTP_CONTENT_RESP    "+GHTTPCONTENT: "
#define HTTP_HEADER_RESP     "+GHTTPHEADER: "
#define HTTP_URC_STR         "+GHTTPURC: "
#define HTTP_CREATE_RESP     "+GHTTPCREATE: "
#define HTTP_REQUEST_RESP    "+GHTTPREQUEST: "
#define HTTP_READ_RESP       "+GHTTPREAD: "
#define HTTP_DEL_RESP        "+GHTTPDEL: "
#define HTTP_DL_FILE_RESP    "+GHTTPDLFILE: "
#define HTTP_TERM_RESP       "+GHTTPTERM: "

#define HTTP_CREATE_CMD      "AT+GHTTPCREATE"
#define HTTP_CFG_CMD         "AT+GHTTPCFG"
#define HTTP_HEADER_CMD      "AT+GHTTPHEADER"
#define HTTP_CONTENT_CMD     "AT+GHTTPCONTENT"
#define HTTP_REQUEST_CMD     "AT+GHTTPREQUEST"
#define MHTTP_READ_CMD       "AT+GHTTPREAD"
#define HTTP_DLFILE_CMD      "AT+GHTTPDLFILE"
#define MHTTP_DEL_CMD        "AT+GHTTPDEL"
#define HTTP_TERM_CMD        "AT+GHTTPTERM"
#endif

#ifdef  CONFIG_CUSTOMER_CM_AT
#define HTTP_CFG_RESP        "MHTTPCFG"
#define HTTP_CONTENT_RESP    "+MHTTPCONTENT: "
#define HTTP_HEADER_RESP     "+MHTTPHEADER: "
#define HTTP_URC_STR         "+MHTTPURC: "
#define HTTP_CREATE_RESP     "+MHTTPCREATE: "
#define HTTP_REQUEST_RESP    "+MHTTPREQUEST: "
#define HTTP_READ_RESP       "+MHTTPREAD: "
#define HTTP_DEL_RESP        "+MHTTPDEL: "
#define HTTP_DL_FILE_RESP    "+MHTTPDLFILE: "
#define HTTP_TERM_RESP       "+MHTTPTERM: "

#define HTTP_CREATE_CMD      "AT+MHTTPCREATE"
#define HTTP_CFG_CMD         "AT+MHTTPCFG"
#define HTTP_HEADER_CMD      "AT+MHTTPHEADER"
#define HTTP_CONTENT_CMD     "AT+MHTTPCONTENT"
#define HTTP_REQUEST_CMD     "AT+MHTTPREQUEST"
#define MHTTP_READ_CMD       "AT+MHTTPREAD"
#define HTTP_DLFILE_CMD      "AT+MHTTPDLFILE"
#define MHTTP_DEL_CMD        "AT+MHTTPDEL"
#define HTTP_TERM_CMD        "AT+MHTTPTERM"
#endif

typedef struct mobile_http_msg_t
{
    slong dl_start;
    slong dl_end;
    u32 urc_port;
    u32 progind;
    u8 *local_path;
    u8 msg_id;
    u8 httpid;
    u8 ssl_id;
    u8 ssl_enable;
    u8 eof;
} MOBILE_HTTP_MSG;

typedef enum {
    MOBILE_HTTP_DL_FILE = 0,
    MOBILE_HTTP_METHOD_PROCESS,
    MOBILE_HTTP_CHUNKED_REQUEST_SEND,
    MOBILE_HTTP_CHUNKED_DATA_SEND, /* chunked data send by content set cmd */
    MOBILE_HTTP_CLIENT_DELETE,
} MOBILE_HTTP_MGD_ID;

typedef struct MOBILE_HTTP_AT_PARAS_MAP_STRUCT
{
    u8 *mobile_http_at_paras_str;      /* String of paras    */
    u8  mobile_http_at_paras_code;     /* Code of paras      */
    u8 paras_len;
} MOBILE_HTTP_AT_PARAS_MAP;

typedef enum {
    MOBILE_HTTP_AT_SSL = 1,
    MOBILE_HTTP_AT_CID,
    MOBILE_HTTP_AT_HEADER,
    MOBILE_HTTP_AT_CACHED,
    MOBILE_HTTP_AT_TIMEOUT,
    MOBILE_HTTP_AT_CHUNKED,
    MOBILE_HTTP_AT_ENCODING,
    MOBILE_HTTP_AT_FRAGMENT,
    MOBILE_HTTP_AT_URLENCODE,
    MOBILE_HTTP_AT_MAX,
} MOBILE_HTTP_AT_PARAS_CODE;

typedef struct mobile_http_timer_input {
    AT_REQ_RESP *at_ptr;
    u8 httpid;
} MOBILE_HTTP_TIMER_INPUT;

typedef struct mobile_http_header_content_info {
    u8 has_data;
    u32 offset;
    u32 actual_len;
    u32 input_len;
    u8 *data;
    u8 eof;
    u8 end_mob; //0:end by len or time; 1: end by ctrl+z or esc
    u8 chunked_data_input; //0:can not input content data; 1: can input
} MOBILE_HTTP_HEADER_CONTENT_INFO;

typedef struct mobile_http_specific_header_key_value {
    u8 *key;
    u8 *value;
} MOBILE_HTTP_SPECIFIC_HEADER_KEY_VALUE;

typedef struct mobile_http_common_header_key_value {
    u8 *key;
    u8 *value;
    u8 is_same; //0:diff with s_header;1:same with s_header
} MOBILE_HTTP_COMMOM_HEADER_KEY_VALUE;

typedef struct mobile_http_connect_info {
    NXD_ADDRESS server_ip;
    u8 *url;
    u8 *host;
    u8 *resource;
    u32 port;
    u32 resource_len;
    u32 actual_len;
    u8 end_mob; //0:end by len or time; 1: end by ctrl+z or esc
} MOBILE_HTTP_CONNECT_INFO;

typedef struct mobile_http_response_urc_data_info {
    u8 *res_data_buf;
    u8 *res_hex_buf;
    u32 sum_len; //has_read_len;
    u32 cur_len; //current_read_len
    u32 content_len;
} MOBILE_HTTP_RESPONSE_URC_DATA_INFO;

typedef struct mobile_http_read_data_info {
    u8 *read_header_buf;
    u8 *read_data_buf;
    u32 to_read_len;
    u32 unread_content_len;
    u32 unread_header_len;
    u32 has_read_len;
    u32 cur_read_len;
    u8 data_type; //0 header; 1 content
} MOBILE_HTTP_READ_DATA_INFO;

typedef struct mobile_http_cached_data_info {
    u8 *cached_header;
    u8 *cached_data;
    u32 cache_len;
    u32 actual_content_len;
    u32 header_len;
    u32 content_len;
} MOBILE_HTTP_CACHED_DATA_INFO;

/* Define HTTP input paras. */
typedef struct mobile_http_paras {
    MOBILE_HTTP_HEADER_CONTENT_INFO header;
    MOBILE_HTTP_HEADER_CONTENT_INFO s_header;
    MOBILE_HTTP_SPECIFIC_HEADER_KEY_VALUE sp_k_v[MOBILE_HTTP_MAX_HEADER_NUM];
    MOBILE_HTTP_COMMOM_HEADER_KEY_VALUE comm_k_v[MOBILE_HTTP_MAX_HEADER_NUM];
    MOBILE_HTTP_HEADER_CONTENT_INFO s_content;
    MOBILE_HTTP_CONNECT_INFO connect_info;
    MOBILE_HTTP_RESPONSE_URC_DATA_INFO res_data;
    MOBILE_HTTP_CACHED_DATA_INFO cached_data_info;
    MOBILE_HTTP_READ_DATA_INFO read_data_info;
    u8 cid;
    u8 httpid;
    u8 downloading;
    u8 chunked_mode;
    u8 cached;
    u32 conn_timeout;
    u32 rsp_timeout;
    u32 input_timeout;
    u8 input_format;
    u8 out_format;
    u8 ssl_enable;
    u8 ssl_id;
    u16 frag_size;
    u16 interval;
    u8 urlencode_mode;
    u8 method;
    u32 handle;
    u8 ready; //request ready to send
    u32 urc_port;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    u8 *local_path;
    u8 dl_mod; //1:dl file mode; 0: normal http mode
    u8 is_term;
    slong content_range;
    u8 has_set_fragment;
    u16 urc_interval; //urc flowctrl interbal
} MOBILE_HTTP_PARAS;

/* Define HTTP input paras. */
typedef struct mobile_http_paras_temp {
    u8 cached;
    s32 conn_timeout;
    s32 rsp_timeout;
    s32 input_timeout;
    s8 input_format;
    s8 out_format;
    s8 ssl_enable;
    s8 ssl_id;
    s16 frag_size;
    s16 interval;
    s32 cache_len;
} MOBILE_HTTP_PARAS_TEMP;

/* define URC ERROR NUM */
#define MOBILE_HTTP_URC_URL_PARSING_FAILED 1
#define MOBILE_HTTP_URC_CONNECTING_FAIL 2
#define MOBILE_HTTP_URC_CONNECTING_TIMEOUT 3
#define MOBILE_HTTP_URC_SSL_HANDSHAKE_FAILED 4
#define MOBILE_HTTP_URC_DISCONNECTED_ABNORMAL 5
#define MOBILE_HTTP_URC_RESPONSE_TIMEOUT 6
#define MOBILE_HTTP_URC_DATA_PARSING_FAILED 7
#define MOBILE_HTTP_URC_NO_CACHE_SPACE 8
#define MOBILE_HTTP_URC_PACKET_DROPOUT 9
#define MOBILE_HTTP_URC_WRITE_FILE_FAILED 10
#define MOBILE_HTTP_URC_UNKNOW_ERROR 255

/* define CME ERROR NUM */
#define MOBILE_HTTP_OPERATION_NOT_ALLOWED 3
#define MOBILE_HTTP_MEMORY_ALLOCATION_FAILED 23
#define MOBILE_HTTP_PARAMETER_ERROR 50
#define MOBILE_HTTP_UNKNOW_ERROR 650
#define MOBILE_HTTP_NO_IDLE_CLIENT 651
#define MOBILE_HTTP_CLIENT_NOT_CREATED 652
#define MOBILE_HTTP_CLIENT_BUSY 653
#define MOBILE_HTTP_URL_PARSING_FAILED 654
#define MOBILE_HTTP_SSL_NOT_ENABLED 655
#define MOBILE_HTTP_CONNECT_FAILED 656
#define MOBILE_HTTP_DATA_SENDING_FAILED 657
#define MOBILE_HTTP_OPEN_FILE_FAILED 658

u32 mobile_http_para_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_para_at_query_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_para_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_client_create_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_client_create_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_header_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_header_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_content_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_content_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_request_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_request_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_read_data_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_read_data_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_delete_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_delete_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_term_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_term_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_download_file_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_http_download_file_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_http_content_or_header_set(AT_REQ_RESP *at_ptr, u8 set_type, u32 max_len);


#endif

