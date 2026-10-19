/******************************************************************************/
/*                                                                            */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_HTTP_COMM_H__
#define __AIC_HTTP_COMM_H__

#include <aic_net_api.h>
#include <nx_web_http_client.h>
#include <aic_size.h>
#include <aic_list.h>

#ifndef AIC_HTTPS_CLIENT_MIN_PACKET_SIZE
#define AIC_HTTPS_CLIENT_MIN_PACKET_SIZE 600
#endif

#define AIC_HTTP_HEAD 0
#define AIC_HTTPS_HEAD 1
#define AIC_HTTPS_CA_CRT 1
#define AIC_HTTPS_CLIENT_CRT 2
#define AIC_HTTPS_PRIV_KEY 3
#define AIC_HTTPS_MBED_SET_SUCCESS 1
#define AIC_HTTPS_MAX_RECEIVE_DATA_TIMES 5

#define AIC_HTTPS_INIT_STATE 0xFF
#define AIC_HTTPS_REDIR_STATE 0x30041
#define HTTPS_CLIENT_QUEUE_NUM 10
#define AIC_HTTPS_MAX_CRT_NUM 6
#define AIC_HTTPS_MAX_CRT_PATH_LEN 64
#define AIC_HTTP_MAX_CLIENT_NUM 8 //4 http and 4 https

#define HTTPS_SOCKET_WINDOW_SIZE SIZE_64K
#define HTTPS_MAX_URL_LEN 1024
#define HTTPS_MAX_RESOURCE_LEN 512
#define HTTPS_MAX_HOST_LEN 512
#define HTTPS_MAX_RANGE_LEN 32
#define HTTPS_CONTENT_TYPE_LEN 1024
#define HTTPS_SPECIFIC_HEADER_LEN 128
#define HTTP_ONE_RESPONSE_DATA_BUF_LEN 1600

/* for at*/
#define HTTPS_MAX_INPUT_PARA_LEN 256
#define HTTPS_MAX_PARA_RES_LEN 300
#define HTTPS_CME_STR_LEN 20
#define HTTPS_MIN_RESLEN 10
#define HTTPS_RESLEN 48

/* Define constants for the various HTTPS methods supported. */
#define AIC_HTTPS_METHOD_NONE             0x0
#define AIC_HTTPS_METHOD_GET              0x1
#define AIC_HTTPS_METHOD_POST             0x2
#define AIC_HTTPS_METHOD_PUT              0x3
#define AIC_HTTPS_METHOD_HEAD             0x4
#define AIC_HTTPS_METHOD_DELETE           0x5
#define AIC_HTTPS_METHOD_RANGE_GET        0x6
#define AIC_HTTPS_METHOD_POST_EXT         0x7

/* define error num */
#define AIC_HTTPS_ERROR NX_WEB_HTTP_ERROR
#define AIC_HTTPS_CLIENT_CREATE_FAIL 0
#define AIC_HTTPS_URL_ERROR 1
#define AIC_HTTPS_CRT_INFO_ERROR 2
#define AIC_HTTPS_REDIR_THREAD_CREATE_ERROR 3
#define AIC_HTTPS_BUFFER_ALLOCATE_ERROR 4
#define AIC_HTTPS_BIND_ADDR_FAIL 5
#define AIC_HTTPS_INVALID_INPUT_PARA 6
#define AIC_HTTPS_PTR_ERROR 7
#define AIC_HTTPS_HANDLE_INFO_GET_FAIL 8
#define AIC_HTTPS_OPEN_LOCAL_CRT_FAIL 9
#define AIC_SSL_SETUP_FAIL 10
#define AIC_SSL_HANDSHAKE_FAIL 11
#define AIC_SSL_SERVER_CERT_VERIFY_FAIL 12
#define AIC_NOT_HTTPS_CLIENT 13

#define AIC_HTTPS_RECEIVE_PACKET_ERROR -1
#define AIC_HTTPS_EXTRACT_OFFSET_ERROR -2
#define AIC_HTTPS_CHUNKED_MODE_DATA_GET_DONE -3

typedef struct aic_https_status_map {
    char *https_status_str;
    u32  https_status_code;
} AIC_HTTPS_STATUS_MAP;

typedef struct aic_https_login_info
{
    u32 server_port;
    NXD_ADDRESS server_ip;
    u8 *resource;
    u8 *host;
} AIC_HTTPS_LOGIN_INFO;

typedef struct aic_https_client_instance
{
    NX_WEB_HTTP_CLIENT *https_client;
    NX_PACKET *res_data_packet;
    ulong recv_offset;
    NX_PACKET *put_packet;
    NX_PACKET *chunked_data_packet;
    AIC_HTTPS_LOGIN_INFO login_info;
    long packet_offset;
    u8 cid;
    u8 *prev_host;
    u8 status_code[4];
} AIC_HTTPS_CLIENT_INSTANCE;

typedef enum {
    HTTPS_CLIENT_MSG_GET_REDIR         = BIT_0,
    HTTPS_CLIENT_MSG_RANGE_GET_REDIR   = BIT_1,
    HTTPS_CLIENT_MSG_POST_REDIR        = BIT_2,
    HTTPS_CLIENT_MSG_POST_EXT_REDIR    = BIT_3,
    HTTPS_CLIENT_MSG_PUT_REDIR         = BIT_4,
    HTTPS_CLIENT_MSG_DELETE_REDIR      = BIT_5,
    HTTPS_CLIENT_MSG_HEAD_REDIR        = BIT_6,
    HTTPS_CLIENT_MSG_MAX,
} HTTPS_CLIENT_MSG;

typedef struct https_client_queue_msg {
    HTTPS_CLIENT_MSG msg;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
} HTTPS_CLIENT_QUEUE_MSG;

/*HTTPS and HTTP comm API*/
u32 https_tls_setup(NX_WEB_HTTP_CLIENT *client_ptr);

u32 aic_http_client_common_create(u8 *client_name, boolean is_https);
u32 aic_https_client_common_delete(u32 handle, boolean is_https);
u32 aic_http_get_start(u32 handle, u8 *url, u8 *username, u8 *password,
                       u32 server_port, ulong wait_option, boolean is_https);

u32 aic_http_range_get_start(u32 handle, u8 *url, u8 *username,
                             u8 *password, u32 server_port, slong start,
                             slong end, ulong wait_option, boolean is_https);

u32 aic_http_post_start(u32 handle, u8 *url, u8* username, u8 *password,
                        u32 server_port, ulong total_bytes, ulong wait_option,
                        boolean is_https);

u32 aic_http_post_start_ext(u32 handle, u8 *url, u8 *username,
                                    u8 *password, u32 server_port,
                                    ulong total_bytes, u8 *content_type,
                                    ulong wait_option, boolean is_https);

u32 aic_http_put_start(u32 handle, u8 *url, u8 *username, u8 *password,
                               u32 server_port, ulong total_bytes,
                               ulong wait_option, boolean is_https);

u32 aic_http_head_start(u32 handle, u8 *url, u8 *username, u8 *password,
                        u32 server_port, ulong wait_option, boolean is_https);

u32 aic_http_delete_start(u32 handle, u8 *url, u8 *username,
                          u8 *password, u32 server_port, ulong wait_option,
                          boolean is_https);

u32 aic_process_https_url(u32 handle, u8 *url, u8 type);

u32 aic_https_client_content_type_header_add(NX_WEB_HTTP_CLIENT *client_ptr,
                                             u8 *resource, u8 *content_type,
                                             ulong wait_option);

void aic_https_store_redir_info(u32 handle, u8 *username, u8 *password,
                                u32 port, u32 total_size, slong start,
                                slong end, u8 *content_type, ulong wait_option);

ulong aic_https_client_total_transfer_bytes_get(u32 handle);
ulong aic_https_client_actual_bytes_transferred_get(u32 handle);
ulong aic_https_client_total_receive_bytes_get(u32 handle);
ulong aic_https_client_actual_bytes_received_get(u32 handle);
u32 aic_https_client_set_cid(u32 handle, u8 cid);

u32 aic_https_client_request_packet_allocate(u32 handle, NX_PACKET **packet_ptr,
                                             ulong wait_option);

u32 aic_https_client_put_packet(u32 handle, NX_PACKET *packet_ptr,
                                ulong wait_option);

u32 aic_https_client_request_chunked_set(u32 handle, u32 chunk_size,
                                         NX_PACKET *packet_ptr);

u32 aic_https_client_response_body_get(u32 handle, NX_PACKET **packet_pptr,
                                       u8 *status_str, ulong wait_option);

long aic_https_client_response_data_get(u32 handle, u8 *buf, u32 buf_bytes,
                                        ulong wait_option);
#ifdef AIC_HTTP_REDIR
long aic_https_client_redir_response_data_get(u32 handle, u8 *buf, u32 buf_bytes,
                                              ulong wait_option);
#endif

u32 aic_https_client_put_data(u32 handle, u8 *data, ulong data_len,
                              ulong wait_option);

u32 aic_https_client_stream_data_put(u32 handle, u8 *data, slong data_len,
                                     ulong wait_option);

u32 aic_https_client_request_header_add(u32 handle, u8 *field_name,
                                        u32 name_length, u8 *field_value,
                                        u32 value_length, ulong wait_option);

u32 aic_https_client_request_initialize(u32 handle, u32 method, u32 input_size,
                                        u32 encoding_chunked, u8 *username,
                                        u8 *password, ulong wait_option);

u32 aic_https_client_request_send(u32 handle, ulong wait_option);

u32 aic_https_client_response_header_callback_set(u32 handle,
    void (*callback_function)(char *field_name, u32 field_name_length,
                              char *field_value, u32 field_value_length));

u32 aic_http_client_specific_header_add(NX_WEB_HTTP_CLIENT *client_ptr,
                                        u8 *header_name, u8 *header_value,
                                        ulong wait_option);

/* just use for source code */
u32 aic_http_packet_release(NX_PACKET **packet_ptr_ptr);
u32 aic_http_input_paras_check(AIC_HTTPS_CLIENT_INSTANCE *client_ptr);
void aic_http_host_check(AIC_HTTPS_CLIENT_INSTANCE *client_ptr, u8 *host,
                         ulong wait_option);
u32 aic_http_free_instance_space(AIC_HTTPS_CLIENT_INSTANCE *client_ptr);
u32 aic_http_bind_source_ip(AIC_HTTPS_CLIENT_INSTANCE *client_ptr, u8 cid,
                            NXD_ADDRESS *server_ip, ulong wait_option);

void aic_http_redir_cb(NX_WEB_HTTP_CLIENT *client_ptr, char *field_name,
                          u32 field_name_len, char *field_value,
                          u32 field_value_len);
AIC_HTTPS_CLIENT_INSTANCE *aic_http_get_handle_by_client_ptr(
       struct list_head *list, NX_WEB_HTTP_CLIENT *http_client);

u32 aic_http_action_prepare(u32 handle, u8 *url, u8 *username, u8 *password,
                            u32 server_port, ulong total_size, slong start,
                            slong end, u8 *content_type, ulong wait_option);

u8 aic_http_client_chunked_mode_check(u32 handle);

u32 aic_http_client_chunked_response_remaining_size_get(u32 handle);

u32 aic_https_client_request_chunked_packet_set(u32 handle, u32 chunk_size,
                                                ulong wait_option);

void http_server_ip_set(NXD_ADDRESS *server_ip, struct addrinfo *res);

#endif
