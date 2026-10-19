/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_AT_QTCPIP_H__
#define __AIC_AT_QTCPIP_H__

#include "aic_general_at_handle.h"

#define CONFIG_NET_AT_QTCPIP

#define BUFFER_SIZE  128
#define AT_SOCK_LINKID_DEFAULT   0
#define AT_RECV_DATA_BUFFER 1500
#define AT_RESP_RECV_DATA_BUFFER 1536

#define AT_SOCKET_DATA_LEN_MAX 1460
#define AT_SOCKET_DATA_LEN_DEFAULT 1460
#define AT_SOCKET_DATA_LEN_MIN 1
#define AT_SOCKET_LISTEN_MAX  5

#define REPORT_SIZE_DEFAULT  1024
#define STRANS_SIZE_DEFAULT  1024
#define TCP_SERVER_TIMEOUT   7200
#define RECONN_INTV_DEFAULT  1
#define RECONN_INTV_MAX      36000
#define AT_AUTO_MSG_LEN_MAX  12
#define AT_PING_COUNT_MAX    10

#define AT_SOCKET_SEND_TIMEOUT 20  // seconds
#define AT_SCKET_CONN_COUNT_MAX 5

#define SOCKET_RECV_EVENT_ALL  0xFFFF
#define SOCKET_RECV_EVENT    0x1
#define SOCKET_CLOSE_EVENT      0x2


struct at_error_description{
    u32 num;
    u8 *str;
};

typedef struct aic_at_tcpip_cfg{
    int transpktsize;
    int transwaittm;
    u32 cycle_time;
    u8 send_data_format;
    u8 recv_data_format;
    u8 view_mode;
    u8 max_backoffs;
    u16 max_rto;
    u8 msg_auto[AT_AUTO_MSG_LEN_MAX];
    u8 sendecho;
    u8 com_id;
} AT_TCPIP_CFG;

typedef struct aic_at_event_flag {
    u8 recv_flag;
    u8 close_flag;
} AT_EVENT_FLAG;

enum TCPIP_SOCKET_STATE{
    SOCKET_STATE_INITIAL = 0,
    SOCKET_STATE_OPENING,
    SOCKET_STATE_CONNECTED,
    SOCKET_STATE_LISTENING,
    SOCKET_STATE_CLOSING,

    SOCKET_CONNECT_SUCCESS,
    SOCKET_CONNECT_FAIL,
};

#ifdef CONFIG_NET_AT_QTCPIP
u32 handle_qiopen_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qiopen_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qiopen_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qiclose_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qiclose_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qistate_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qistate_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qistate_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qisend_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qisend_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qird_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qird_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qisendex_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qisendex_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qiswtmd_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qistwmd_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qistwmd_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qicfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qicfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qisde_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qisde_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qisde_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qidnscfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qidnscfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qidnsgip_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qidnsgip_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qping_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qping_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qntp_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qntp_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qntp_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qigeterror_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qigeterror_exec_cmd(AT_REQ_RESP *at_req_resp_ptr);
#endif
#ifdef CONFIG_NET_AT_QTLS
u32 handle_qsslcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslopen_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslopen_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslclose_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslclose_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslsend_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslsend_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslrecv_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslrecv_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslstate_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslstate_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_qsslstate_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ssl_ca_cert_set_cmd(AT_REQ_RESP *at_req_resp_ptr);

#endif
#endif
