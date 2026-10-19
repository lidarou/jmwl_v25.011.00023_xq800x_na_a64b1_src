/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_AT_MTCPIP_H__
#define __AIC_AT_MTCPIP_H__

#include "aic_general_at_handle.h"
#include "nxd_bsd.h"

#define NET_AT_MTCPIP

#define AT_SOCKET_MAX_CONN_NUM   6
// #define BUFFER_SIZE  128
#define AT_SOCK_LINKID_DEFAULT   0
#define AT_RECV_DATA_BUFFER 1500
#define AT_RESP_RECV_DATA_BUFFER 1536

#define AT_SOCKET_DATA_LEN_MAX 62780 //1460
#define AT_SOCKET_DATA_LEN_DEFAULT 1460
#define AT_SOCKET_DATA_LEN_MIN 1
#define AT_SOCKET_LISTEN_MAX  5

#define AT_SOCKET_CONN_TIMEOUT_MIN 1
#define AT_SOCKET_CONN_TIMEOUT_DEFAULT 60
#define AT_SOCKET_CONN_TIMEOUT_MAX 200

/* keepalive unit:seconds */
#define AT_KEEPIDLE_DEFAULT  90
#define AT_KEEPINTERVAL_DEFAULT  75
#define AT_KEEPCOUNT_DEFAULT  3 //times
#define AT_KEEPIDLE_MIN  30
#define AT_KEEPINTERVAL_MIN  30
#define AT_KEEPCOUNT_MIN  1 //times
#define AT_KEEPIDLE_MAX  7200
#define AT_KEEPINTERVAL_MAX  600
#define AT_KEEPCOUNT_MAX  9 //times

#define AT_SOCKET_RECV_BUFFER_DEF 4096

#define REPORT_SIZE_DEFAULT  1024
#define STRANS_SIZE_DEFAULT  1024
#define STRANS_SIZE_MAX      1460
#define STRANS_SIZE_MIN      512
#define STRANS_WAITTM_DERFAULT 200 //unit:ms
#define STRANS_WAITTM_MAX    2000
#define STRANS_WAITTM_MIN    100
#define TCP_SERVER_TIMEOUT   7200
#define RECONN_INTV_DEFAULT  1
#define RECONN_INTV_MAX      36000
#define AT_AUTO_MSG_LEN_MAX  12

#define DNS_TIMEOUT_MIN      1
#define DNS_TIMEOUT_MAX      60
#define DNS_TIMEOUT_DEFAULT  10
#define DNS_RETRIES_MIN      1
#define DNS_RETRIES_MAX      9
#define DNS_RETRIES_DEFAULT  3

#define PING_COUNT_SIZE_DEFAULT  4
#define PING_COUNT_SIZE_MAX      65535
#define PING_COUNT_SIZE_MIN      1
#define PING_PLYLOAD_SIZE_DEFAULT  16
#define PING_PLYLOAD_SIZE_MAX      1400
#define PING_PLYLOAD_SIZE_MIN      1

#define AT_SOCKET_SEND_TIMEOUT 10  // seconds
#define AT_SCKET_CONN_COUNT_MAX 5
#define AT_SOCKET_SEND_TIMEOUT_MAX 120 //seconds
#define AT_SOCKET_RCVBUF_MAX     65536
#define AT_SOCKET_RECV_WINDOWS  64240  /*default windows size*/
#define AT_SOCKET_RECV_BUF_OFFSET 48

#define AT_DNS1_SERVER_V4   "119.29.29.29"
#define AT_DNS2_SERVER_V4   "114.114.114.114"
#define AT_DNS1_SERVER_V6   "2400:3200::1"
#define AT_DNS2_SERVER_V6   "2001:4860:4860::8888"

#define AT_SNTP_DEFAULT_SERVER  "ntp1.aliyun.com"
#define AT_SOCKET_QUEUE_NUM    50
#define AT_HOST_NAME_LEN_MAX   255

#define DNS_PROCESS_EVENT    0x1
#define PING_PROCESS_EVENT    0x1

#define SOCKET_EVENT_ALL        0xFF
#define SOCKET_RECV_EVENT       0x1
#define SOCKET_CLOSE_EVENT      0x2
#define SOCKET_ESTABLISH_EVENT  0x4

#define SOCKET_ESTABLISH_EVENT_ALL  0xFF
#define SOCKET_ESTABLISH_EVENT_1    0x1
#define SOCKET_ESTABLISH_EVENT_2    0x2
#define SOCKET_ESTABLISH_EVENT_3    0x4
#define SOCKET_ESTABLISH_EVENT_4    0x8
#define SOCKET_ESTABLISH_EVENT_5    0x10
#define SOCKET_ESTABLISH_EVENT_6    0x20
#define SOCKET_ESTABLISH_SUCCESS_EVENT    0x40
#define SOCKET_ESTABLISH_FAIL_EVENT    0x80

struct at_error_description{
    u32 num;
    u8 *str;
};

struct aic_at_stream_recv_info{
    u8 *buf;
    u32 len;
    u32 offset;
};

typedef struct aic_at_tcpip_cfg{
    int transpktsize;
    int transwaittm;
    int send_data_buffer;
    int recv_data_buffer;
    u32 com_id;
    u8 autofree;
    u8 send_data_format;
    u8 recv_data_format;
    u8 send_timeout;
    u8 ack_mode;
    u8 ssl_enable;
    u8 ssl_ctx_id;
    u8 connect_timeout;
    u8 mode;
    u8 recv_flag;
    u8 close_flag;
    struct sock_keepalive keepalive;
} AT_TCPIP_CFG;

struct aic_at_dnsinfo{
    u8 *host;
    u8 comid;
    u8 cid;
};

typedef struct aic_at_ping_cfg{
    u8 *host;
    int cnt;
    int cid;
    int ping_interval;
    int ping_timeout;
    int ping_size;
    int ping_comid;
} AT_PING_CFG;

struct aic_ping_result{
    u32 send_cnt;
    u32 recv_cnt;
    u32 shortest_time;
    u32 longest_time;
    u32 average_time;
};

struct aic_at_listen_info {
    u8 type;
    u8 *host;
    u16 port;
    u16 src_port;
    int link_id;
    u32 timestamp;
    u8 timeout;
};

enum TCPIP_SOCKET_CONNECT_RESULT{
    SOCKET_CONNECT_SUCCESS = 1,
    SOCKET_CONNECT_FAIL,
};

enum TCPIP_AT_ERROR_CODE {
    TCPIP_OPERATION_NOT_SUPPORT = 4,
    TCPIP_INVAL_AT_PARAM = 50,
    TCPIP_UNKNOW_ERR = 550,
    TCPIP_NOT_USE,
    TCPIP_IS_USED,
    TCPIP_NOT_CONNECTED,
    TCPIP_SOCKET_CREATE_FAIL,
    TCPIP_SOCKET_BIND_FAIL,
    TCPIP_SOCKET_LISTEN_FAIL,
    TCPIP_SOCKET_CONNECT_REFUSE,
    TCPIP_SOCKET_CONNECT_TIMEOUT,
    TCPIP_SOCKET_CONNECT_FAIL,
    TCPIP_SOCKET_SEND_FAIL,
    TCPIP_SOCKET_RECV_FAIL,
    TCPIP_SOCKET_ACCEPT_FAIL,
    TCPIP_PDP_NOT_ACTIVE = 570,
    TCPIP_PDP_ACTIVE_FAIL,
    TCPIP_PDP_DEACTIVE_FAIL,
    TCPIP_APN_NOT_CONFIG = 575,
    TCPIP_PORT_BUSY,
    TCPIP_IPV4_IPV6_NOT_SUPPORT,
    TCPIP_DNS_PARSING_FAIL = 580,
    TCPIP_DNS_BUSY,
    TCPIP_PING_BUSY
};

enum TCPIP_AT_PING_ERROR_CODE {
    PING_SUCCESS = 0,
    PING_DNS_FAIL,
    PING_DNS_TIMEOUT,
    PING_RSP_ERROR,
    PING_RSP_TIMEOUT,
    PING_OTHER_ERROR
};

typedef struct aic_at_tcpip_msg{
    u32 len;
    u32 com_id;
    u32 recv_len;
    u32 recv_cache;
    u32 recv_count;
    u8 msgid;
    u8 index;
} AT_SOCKET_MSG;

enum TCPIP_AT_SOCKET_MSG_ID{
    MSG_TCPIP_SOCKET_RECV_URC_ID = 1,
    MSG_TCPIP_SOCKET_CLOSE_URC_ID,
    MSG_TCPIP_SOCKET_RECV_ID,
    MSG_TCPIP_SOCKET_CLOSE_ID,
    MSG_TCPIP_SOCKET_ID_MAX
};

#ifdef NET_AT_MTCPIP
u32 handle_mipcfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_miptka_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_miptka_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_miptka_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipopen_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipopen_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipclose_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipclose_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipsend_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipsend_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_miprd_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_miprd_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipmode_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipmode_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipstate_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipstate_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipsack_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mipsack_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mdnscfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mdnscfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mdnsgip_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mdnsgip_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mping_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mping_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_transmit_mode_exit_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mntp_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_mntp_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
#endif

#endif
