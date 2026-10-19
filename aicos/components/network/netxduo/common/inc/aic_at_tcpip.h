/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_AT_TCPIP_H__
#define __AIC_AT_TCPIP_H__

#include <aic_osa.h>
#ifdef CONFIG_NET_SECURE
#include <nx_secure_tls.h>
#endif
#include "aic_at_cmd_handle.h"
#include "aic_general_at_handle.h"

#ifdef CONFIG_CRYPTO_MBEDTLS
#include "mbedtls/net_sockets.h"
#endif

#define AT_SOCKET_MAX_CONN_NUM   5
#define BUFFER_SIZE  128
#define AT_SOCK_LINKID_DEFAULT   0
#define AT_RECV_DATA_BUFFER 1500
#define AT_RESP_RECV_DATA_BUFFER 1536

#define AT_SOCKET_DATA_LEN_MAX 1460
#define AT_SOCKET_DATA_LEN_MIN 1
#define AT_SOCKET_LISTEN_MAX  5

#define REPORT_SIZE_DEFAULT  1024
#define STRANS_SIZE_DEFAULT  1024
#define TCP_SERVER_TIMEOUT   7200
#define RECONN_INTV_DEFAULT  1
#define RECONN_INTV_MAX      36000
#define AT_AUTO_MSG_LEN_MAX  12

#define AT_SOCKET_SEND_TIMEOUT 20  // seconds
#define AT_SCKET_CONN_TIMEOUT  10  // seconds

typedef struct tcpip_at_cfg{
    int ciphead;
    int cipmux;
    int cipserver;
    int cipmode;
    int cipsendmode;
    int server_max_count;
    int report_size;
    int strans_size;
    int cipsto;
    int cipinfo;
    int cipreconnintv;
    int ciprecvmode;
    u8 com_id;

    int period;
    int period_sockid;
    int period_cnt;
    int period_send_size;
    u8  *period_send_buff;
    //void *period_pthread;
} AIC_TCPIP_AT_CFG;

u32 handle_cipv6_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipv6_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipstate_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipdomain_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipstart_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipstart_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipstartex_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipstartex_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsend_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsend_exec_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipqsend_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipqsend_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipqsend_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsendl_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsendlcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsendlcfg_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsendex_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciphead_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciphead_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciphead_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipclose_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipclose_exec_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cifsr_exec_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipmux_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipmux_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipserver_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipserver_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipservermaxconn_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipservermaxconn_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipmode_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipmode_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
//u32 handle_savetranslink_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsto_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsto_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
//u32 handle_ciupdate_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
//u32 handle_ciupdate_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipdinfo_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipdinfo_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
#ifdef CONFIG_NET_AT_TLS
u32 handle_cipsslca_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
#ifdef CONFIG_CRYPTO_MBEDTLS
u32 handle_cipsslcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
#endif
#ifdef CONFIG_NET_SECURE
u32 handle_cipsslcconf_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsslcconf_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsslccn_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsslccn_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsslcsni_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsslcsni_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
//u32 handle_cipsslcalpn_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
//u32 handle_cipsslcalpn_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsslcpsk_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsslcpsk_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
#endif
#endif
u32 handle_cipreconnintv_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipreconnintv_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciprecvmode_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciprecvmode_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciprecvdata_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciprecvlen_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
//u32 handle_ping_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipdns_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipdns_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciptcpopt_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_ciptcpopt_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipack_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_keepalive_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_send_keepalive_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_exit_exec_cmd(AT_REQ_RESP *at_req_resp_ptr);

#ifdef CONFIG_NET_SNTP
u32 handle_cipsntpcfg_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsntpcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsntptime_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsntpintv_query_cmd(AT_REQ_RESP *at_req_resp_ptr);
u32 handle_cipsntpintv_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
#endif

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
#endif

#endif
