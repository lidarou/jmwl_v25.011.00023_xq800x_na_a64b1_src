/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to TCPIP.
 *  This is the handler of AT commands relating to TCPIP,
 *  such as establish a tcp connection.etc.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x11f658
#define LOG_MODULE    AMOD_NET

#include <stdlib.h>
#include <string.h>
#include <aic_size.h>
#include <nx_api.h>
#include <nxd_bsd.h>
#include <nxd_dns.h>
#ifdef CONFIG_NET_SNTP
#include <nxd_sntp_client.h>
#endif
#include <nx_ip.h>
#ifdef CONFIG_NET_SECURE
#include <nx_secure_tls.h>
#include <nx_secure_tls_api.h>
#endif
#include <aic_errno.h>
#include <aic_init.h>
#include "aic_at_tcpip_common.h"
#include "aic_at_tcpip.h"
#include "aic_at_tls.h"
#include "aic_at_ping.h"
#include "aic_net_api.h"
#ifdef CONFIG_NET_TLS_API
#include "aic_tls_api.h"
#endif
#include "aic_at_parse_utils.h"
#include "aic_at_resp_handle.h"

//#include "pthread.h"

#define SOCKET_SEND_EVENT  0x1

AIC_SOCKET_LIST sock_list[AT_SOCKET_MAX_CONN_NUM] = {0};
static AIC_AT_TCP_SERVER aic_at_tcp_server;
AIC_TCPIP_AT_CFG aic_tcpip_at_cfg;
int error_code;

static int sockid;
static int length;
static aos_thread_id at_socket_listen_thread;
static aos_event_ptr at_socket_listen_event;
static aos_thread_id at_socket_recv_thread;
static aos_event_ptr at_socket_recv_event;
static aos_thread_id at_socket_send_thread;
static aos_event_ptr at_socket_send_event;

#ifdef AT_TLS_TEST
static NX_PACKET_POOL test_packet_pool;
NX_IP tls_test_ip;
static UCHAR tls_test_packet_pool[16384];
static UCHAR tls_test_ip_instance_pool[2048];
static UCHAR tls_bsd_task_pool[2048];
static u32 arp_cache_mem[256];
#endif

extern NX_DNS *_nx_dns_instance_ptr;
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];

#ifdef CONFIG_NET_SECURE
extern NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;
#endif

#ifdef CONFIG_NX_RAM_DRIVER
extern VOID  _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);
#endif

AT_CMD_HANDLE tcpip_at_cmd[] = {
#ifdef CONFIG_NET_AT_TCPIP
    {"AT+CIPV6", AIC_NULL, handle_cipv6_query_cmd, handle_cipv6_set_cmd,
     AIC_NULL},

    {"AT+CIPSTATE", AIC_NULL, handle_cipstate_query_cmd, AIC_NULL, AIC_NULL},

    {"AT+CIPDOMAIN", AIC_NULL, AIC_NULL, handle_cipdomain_set_cmd, AIC_NULL},

    {"AT+CIPSTART", AIC_NULL, AIC_NULL, handle_cipstart_set_cmd, AIC_NULL},

    {"AT+CIPSTARTEX", AIC_NULL, AIC_NULL, handle_cipstartex_set_cmd, AIC_NULL},

    {"AT+CIPSEND", AIC_NULL, AIC_NULL, handle_cipsend_set_cmd, AIC_NULL},

    {"AT+CIPQSEND", handle_cipqsend_test_cmd, handle_cipqsend_query_cmd,
     handle_cipqsend_set_cmd, AIC_NULL},

    {"AT+CIPSENDL", AIC_NULL, AIC_NULL, handle_cipsendl_set_cmd, AIC_NULL},

    {"AT+CIPSENDLCFG", AIC_NULL, handle_cipsendlcfg_query_cmd,
     handle_cipsendlcfg_set_cmd, AIC_NULL},

    {"AT+CIPSENDEX", AIC_NULL, AIC_NULL, handle_cipsendex_set_cmd, AIC_NULL},

    {"AT+CIPHEAD", handle_ciphead_test_cmd, handle_ciphead_query_cmd,
     handle_ciphead_set_cmd, AIC_NULL},

    {"AT+CIPCLOSE", AIC_NULL, AIC_NULL, handle_cipclose_set_cmd, AIC_NULL},

    {"AT+CIFSR", AIC_NULL, AIC_NULL, AIC_NULL, handle_cifsr_exec_cmd},

    {"AT+CIPMUX", AIC_NULL, handle_cipmux_query_cmd, handle_cipmux_set_cmd,
     AIC_NULL},

    {"AT+CIPSERVER", AIC_NULL, handle_cipserver_query_cmd,
     handle_cipserver_set_cmd, AIC_NULL},

    {"AT+CIPSERVERMAXCONN", AIC_NULL, handle_cipservermaxconn_query_cmd,
     handle_cipservermaxconn_set_cmd, AIC_NULL},

    {"AT+CIPMODE", AIC_NULL, handle_cipmode_query_cmd, handle_cipmode_set_cmd,
     AIC_NULL},

    //{"AT+SAVETRANSLINK", AIC_NULL, AIC_NULL, handle_savetranslink_set_cmd,
     //AIC_NULL},

    {"AT+CIPSTO", AIC_NULL, handle_cipsto_query_cmd, handle_cipsto_set_cmd,
     AIC_NULL},

    {"AT+CIPDINFO", AIC_NULL, handle_cipdinfo_query_cmd,
     handle_cipdinfo_set_cmd, AIC_NULL},

    {"AT+SENDDATA", AIC_NULL, AIC_NULL, handle_send_keepalive_set_cmd, AIC_NULL},

#ifdef CONFIG_NET_AT_TLS
    //{"AT+CIPSSLCA", AIC_NULL, AIC_NULL, handle_cipsslca_set_cmd, AIC_NULL},
#ifdef CONFIG_CRYPTO_MBEDTLS
    {"AT+CIPSSLCFG", AIC_NULL, AIC_NULL, handle_cipsslcfg_set_cmd, AIC_NULL},
#endif
#ifdef CONFIG_NET_SECURE
    {"AT+CIPSSLCCONF", AIC_NULL, handle_cipsslcconf_query_cmd,
     handle_cipsslcconf_set_cmd, AIC_NULL},

    {"AT+CIPSSLCCN", AIC_NULL, handle_cipsslccn_query_cmd,
     handle_cipsslccn_set_cmd, AIC_NULL},

    {"AT+CIPSSLCSNI", AIC_NULL, handle_cipsslcsni_query_cmd,
     handle_cipsslcsni_set_cmd, AIC_NULL},

    //{"AT+CIPSSLCALPN", AIC_NULL, handle_cipsslcalpn_query_cmd,
     //handle_cipsslcalpn_set_cmd, AIC_NULL},

    {"AT+CIPSSLCPSK", AIC_NULL, handle_cipsslcpsk_query_cmd,
     handle_cipsslcpsk_set_cmd, AIC_NULL},
#endif
#endif

    {"AT+CIPDNS", AIC_NULL, handle_cipdns_query_cmd, handle_cipdns_set_cmd,
     AIC_NULL},

#ifdef CONFIG_NET_AT_TCP
    {"AT+CIPTCPOPT", AIC_NULL, handle_ciptcpopt_query_cmd,
     handle_ciptcpopt_set_cmd, AIC_NULL},

    {"AT+CIPACK", AIC_NULL, handle_cipack_query_cmd, AIC_NULL, AIC_NULL},

    {"AT+KEEPALIVE", AIC_NULL, AIC_NULL, handle_keepalive_set_cmd, AIC_NULL},
#endif

    {"+++", AIC_NULL, AIC_NULL, AIC_NULL, handle_exit_exec_cmd},

    {"AT+CIPRECONNINTV", AIC_NULL, handle_cipreconnintv_query_cmd,
     handle_cipreconnintv_set_cmd, AIC_NULL},

    {"AT+CIPRECVMODE", AIC_NULL, handle_ciprecvmode_query_cmd,
     handle_ciprecvmode_set_cmd, AIC_NULL},

    {"AT+CIPRECVDATA", AIC_NULL, AIC_NULL, handle_ciprecvdata_set_cmd,
     AIC_NULL},

    {"AT+CIPRECVLEN", AIC_NULL, handle_ciprecvlen_query_cmd, AIC_NULL,
     AIC_NULL},
#endif

#ifdef CONFIG_NET_AT_SNTP
    {"AT+CIPSNTPCFG", AIC_NULL, handle_cipsntpcfg_query_cmd,
     handle_cipsntpcfg_set_cmd, AIC_NULL},

    {"AT+CIPSNTPTIME", AIC_NULL, handle_cipsntptime_query_cmd,
     AIC_NULL, AIC_NULL},

    {"AT+CIPSNTPINTV", AIC_NULL, handle_cipsntpintv_query_cmd,
     handle_cipsntpintv_set_cmd, AIC_NULL},
#endif

#ifdef CONFIG_NET_AT_PING
    {"AT+PING", AIC_NULL, AIC_NULL, aic_at_ping_set_cmd_handle, AIC_NULL},
#endif

    {AIC_NULL},
};

static void aic_tcpip_at_cfg_init(void);
static void aic_at_socket_listen_init(void);
static void aic_at_socket_listen_uninit(void);
static void aic_at_tcpip_process_period_data(void *pdata);
//void *aic_at_tcpip_task_create(char *name, void (*fn)(void *), void *arg);
//void aic_at_tcpip_task_join(void *task);

int aic_tcpip_at_cmd_init(void)
{
    aic_add_at_cmd_handle(tcpip_at_cmd);
    aic_tcpip_at_cfg_init();

    return AIC_TCPIP_AT_OK;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_tcpip_at_cmd_init);

static void aic_tcpip_at_cfg_init(void)
{
    memset(&aic_tcpip_at_cfg, 0, sizeof(AIC_TCPIP_AT_CFG));

    aic_tcpip_at_cfg.server_max_count = AT_SOCKET_MAX_CONN_NUM;
    aic_tcpip_at_cfg.report_size = REPORT_SIZE_DEFAULT;
    aic_tcpip_at_cfg.strans_size = STRANS_SIZE_DEFAULT;
    aic_tcpip_at_cfg.cipinfo = FALSE;
    aic_tcpip_at_cfg.cipreconnintv = 1;
}

#ifdef AT_TLS_TEST
extern void aic_tcpip_at_test_init(void);

int aic_at_test_client(void){
    int ret;
    if(tls_test_ip.nx_ip_id != NX_IP_ID) {
        ret = nx_packet_pool_create(&test_packet_pool, "TLS Packet Pool",
                                       1536, tls_test_packet_pool, 16384);
        if (ret) {
            aicx_info(0x053d5a, "", "[AT_TCPIP]: Error in creating packet pool!\n");
        }
        ret = nx_ip_create(&tls_test_ip, "TLS IP Instance",
                           IP_ADDRESS(1, 2, 3, 5), 0xFFFFFF00UL, &test_packet_pool,
                           _nx_ram_network_driver, tls_test_ip_instance_pool,
                           2048, 20);
        if (ret) {
            aicx_error(0x0f0808, "x", "[AT_TCPIP]: Error in creating tls IP Instance! \
                       status 0x%x\n", ret);
        }

        nx_tcp_enable(&tls_test_ip);
        nx_udp_enable(&tls_test_ip);
        nx_arp_enable(&tls_test_ip, arp_cache_mem, sizeof(arp_cache_mem));

        bsd_initialize(&tls_test_ip, &test_packet_pool, tls_bsd_task_pool, 2048, 2);

        aic_init_dns_client();
        aic_tcpip_at_test_init();
    }

    return AIC_TCPIP_AT_OK;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_at_test_client);
#endif

#ifdef CONFIG_NET_AT_TCPIP
#ifdef CONFIG_NET_AT_TCP
static int aic_create_tcp_socket(int family, struct sockaddr *addr, int linkid)
{
    int i, sock, addrlen, ret;
    struct timeval timeout;
    fd_set master_list, write_ready;

    sock = anet_socket(family, SOCK_STREAM, NX_PROTOCOL_TCP);
    if (sock == NX_SOC_ERROR) {
        aicx_error(0x1a4b87, "d", "[AT_TCP]: --create tls socket error, errno %d\n", errno);
        return AIC_EFAULT;
    }

    timeout.tv_sec = AT_SOCKET_SEND_TIMEOUT;
    timeout.tv_usec = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret)
        aicx_info(0x0ff24c, "x", "[AT_TCP] set tcp send timeout fail 0x%x\r\n", ret);

    if (aic_tcpip_at_cfg.cipserver == 1) {
        if (family == AF_INET)
            addrlen = sizeof(struct sockaddr_in);
        else
            addrlen = sizeof(struct sockaddr_in6);

        anet_bind(sock, addr, addrlen);
        aic_at_tcp_server.sockfd = sock;
        aic_at_tcp_server.family = family;
        return sock;
    } else {
        ret = anet_fcntl(sock, F_SETFL, O_NONBLOCK);
        if (ret == NX_SOC_ERROR) {
            soc_close(sock);
            return AIC_EFAULT;
        }

        if (family == AF_INET)
            addrlen = sizeof(struct sockaddr_in);
        else
            addrlen = sizeof(struct sockaddr_in6);

        ret = anet_connect(sock, addr, addrlen);
        if (ret == NX_SOC_ERROR) {
            if (errno != EINPROGRESS && errno != EALREADY) {
                aicx_error(0x079049, "d", "[AT_TCP]:--connect fail, errno %d\n", errno);
                soc_close(sock);
                return ret;
            }
        }

        FD_ZERO(&write_ready);
        FD_ZERO(&master_list);
        FD_SET(sock, &master_list);
        write_ready = master_list;
        timeout.tv_sec = AT_SCKET_CONN_TIMEOUT;
        timeout.tv_usec = 0;
        ret = anet_select(FD_SETSIZE, NULL, &write_ready, NULL, &timeout);
        if (ret == ERROR || ret == 0) {
            aicx_error(0x096344, "d", "[AT_TCP]:--connect timeout, errno %d\n", errno);
            soc_close(sock);
            return AIC_EFAULT;
        }
    }

    i = linkid;
    if (sock_list[i].socket_id == 0) {
        if(aic_tcpip_at_cfg.cipmux == 0 ){
            sock_list[i].link_id = 0;
        } else {
            sock_list[i].link_id = linkid;
        }
        sock_list[i].socket_id = sock;
        sock_list[i].family = family;
        if (family == AF_INET)
            sock_list[i].socket_type = SOCK_TCP;
        else
            sock_list[i].socket_type = SOCK_TCPv6;
    }

    return sock;
}
#endif

#ifdef CONFIG_NET_AT_UDP
static int aic_create_udp_socket(int family, struct sockaddr *remoteaddr, int linkid)
{
    int i, sock, ret;
    struct timeval timeout;

    sock = anet_socket(family, SOCK_DGRAM, NX_PROTOCOL_UDP);
    if (sock == NX_SOC_ERROR) {
        aicx_error(0x1c07a0, "d", "[AT_UDP]: --create udp socket error, errno %d\n", errno);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    timeout.tv_sec = AT_SOCKET_SEND_TIMEOUT;
    timeout.tv_usec = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret)
        aicx_info(0x09724e, "x", "[AT_UDP] set udp send timeout fail 0x%x\r\n", ret);

    i = linkid;
    if (sock_list[i].socket_id == 0) {
        if(aic_tcpip_at_cfg.cipmux == 0 ){
            sock_list[i].link_id = 0;
        } else {
            sock_list[i].link_id = linkid;
        }
        sock_list[i].socket_id = sock;
        sock_list[i].family = family;
        if (family == AF_INET) {
            sock_list[i].addr =
                ((struct sockaddr_in *)remoteaddr)->sin_addr.s_addr;
            sock_list[i].port =
                ((struct sockaddr_in *)remoteaddr)->sin_port;
            sock_list[i].socket_type = SOCK_UDP;
        } else {
            aos_memcpy(sock_list[i].addr6,
                       &((struct sockaddr_in6 *)remoteaddr)->sin6_addr,
                       16);
            sock_list[i].port =
                ((struct sockaddr_in6 *)remoteaddr)->sin6_port;
            sock_list[i].socket_type = SOCK_UDPv6;
        }
    }

    return sock;
}

static int aic_udp_socket_send(int socket_id, char *data, int datalen)
{
    struct sockaddr_in destaddr;
    struct sockaddr_in6 destaddr6;
    int index, ret;

    index = aic_at_find_index_by_socket(socket_id, AT_TCPIP);
    if (index < 0) {
        return AIC_EINVAL;
    }

    if (sock_list[index].family == AF_INET) {
        destaddr.sin_family = AF_INET;
        destaddr.sin_port = sock_list[index].port;
        destaddr.sin_addr.s_addr = sock_list[index].addr;
        ret = anet_sendto(socket_id, data, datalen, 0,
                          (struct sockaddr *)&destaddr,
                          sizeof(destaddr));
    } else {
        destaddr6.sin6_family = AF_INET6;
        destaddr6.sin6_port = sock_list[index].port;
        aos_memcpy(destaddr6.sin6_addr._S6_un._S6_u32,
                   sock_list[index].addr6, 16);
        ret = anet_sendto(socket_id, data, datalen, 0,
                          (struct sockaddr *)&destaddr6,
                          sizeof(destaddr6));
    }

    return ret;
}
#endif

/**
 * handle_cipv6_set_cmd() - Enable/Disable IPv6.
 * @para_num: parameter.
 *
 * CALLS:
 *    nxd_ipv6_disable, nxd_ipv6_enable
 *
 * CALLED BY:
 *    Application
 *
 * Enable/Disable IPv6.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipv6_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    NX_IP *ip_ptr= _nx_ip_created_ptr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPV6=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (!ip_ptr) {
        aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    if ((*(u8 *)paras_arr[0] != '0') && (*(u8 *)paras_arr[0] != '1')) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    switch(atoi((u8 *)paras_arr[0])) {
    case 0:
        nxd_ipv6_disable(ip_ptr);
        break;
    case 1:
        nxd_ipv6_enable(ip_ptr);
        nx_icmp_enable(ip_ptr);
        break;
    default:
        aicx_info(0x066a48, "", "[AT_TCPIP]: Not support\n");
        aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_INVAL_AT_CMD);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipv6_query_cmd() - Query whether IPv6 is support.
 *
 * CALLS:
 *    non
 *
 * CALLED BY:
 *    Application
 *
 * Query whether IPv6 is support.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipv6_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_IP *ip_ptr= _nx_ip_created_ptr;
    int count = 0;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if (!ip_ptr) {
        aicx_error(0x050140, "", "[AT_TCPIP]: No IP instance\n");
        //strcpy(str, "+CME ERROR: pdp not active");
        aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    while (ip_ptr) {
        if (ip_ptr->nx_ipv6_packet_receive) {
            aicx_info(0x01762e, "", "[AT_TCPIP]: ipv6 is enable\n");
            count++;
        } else {
            aicx_info(0x10a4d3, "", "[AT_TCPIP]: ipv6 is disable\n");
        }
        if (ip_ptr->nx_ip_created_next == _nx_ip_created_ptr)
            break;
        else
            ip_ptr = ip_ptr->nx_ip_created_next;
    }

    if (count > 0) {
        strcpy(str, "+CIPV6: 1");
    } else {
        strcpy(str, "+CIPV6: 0");
    }

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipstate_query_cmd() - Query connection information.
 *
 * CALLS:
 *    non
 *
 * CALLED BY:
 *    Application
 *
 * Query whether IPv6 is support.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipstate_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int i = 0;
    int sock, addrlen, link_id, state = 0;
    char *socket_type = AIC_NULL;
    struct sockaddr_in remoteaddr, localaddr;
    struct sockaddr_in6 remoteaddr6, localaddr6;
    u16 remote_port, local_port;
    u8   addr[ADDR_BUFFERSIZE_IPV6];
    bool isclient;
    char str[BUFFER_SIZE] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    while (i < AT_SOCKET_MAX_CONN_NUM) {
        sock = sock_list[i].socket_id;

        if (aic_tcpip_at_cfg.cipmux == 0 && sock == 0 &&
            aic_tcpip_at_cfg.cipserver == 0){
            aic_reply_ok_resp(at_req_resp_ptr);
            return AIC_TCPIP_AT_OK;
        }

        if (sock == 0) {
            i++;
            continue;
        }

        if (sock_list[i].family == AF_INET) {
            memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
            memset(&localaddr, 0, sizeof(struct sockaddr_in));
            addrlen = sizeof(struct sockaddr_in);
            anet_getpeername(sock, (struct sockaddr *)&remoteaddr, &addrlen);
            anet_getsockname(sock, (struct sockaddr *)&localaddr, &addrlen);
            inet_ntop(AF_INET, &remoteaddr.sin_addr.s_addr,
                      addr, ADDR_BUFFERSIZE_IPV4);
            remote_port = htons(remoteaddr.sin_port);
            local_port = htons(localaddr.sin_port);
        } else {
            memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
            memset(&localaddr6, 0, sizeof(struct sockaddr_in6));
            addrlen = sizeof(struct sockaddr_in6);
            anet_getpeername(sock, (struct sockaddr *)&remoteaddr6, &addrlen);
            anet_getsockname(sock, (struct sockaddr *)&localaddr6, &addrlen);
            inet_ntop(AF_INET6, &remoteaddr6.sin6_addr, addr,
                      ADDR_BUFFERSIZE_IPV6);
            remote_port = htons(remoteaddr6.sin6_port);
            local_port = htons(localaddr6.sin6_port);
        }

        link_id = sock_list[i].link_id;

        if (sock_list[i].socket_type == SOCK_TCP) {
            socket_type = "TCP";
            state = 1;
        } else if (sock_list[i].socket_type == SOCK_TCPv6) {
            socket_type = "TCPv6";
            state = 1;
        } else if (sock_list[i].socket_type == SOCK_UDP) {
            socket_type = "UDP";
        } else if (sock_list[i].socket_type == SOCK_UDPv6) {
            socket_type = "UDPv6";
        } else if (sock_list[i].socket_type == SOCK_TLS) {
            socket_type = "TLS";
            state = 1;
        } else if (sock_list[i].socket_type == SOCK_TLSv6) {
            socket_type = "TLSv6";
            state = 1;
        } else {
            socket_type = "NULL";
        }

        if (aic_tcpip_at_cfg.cipserver == AIC_TRUE)
            isclient = FALSE;
        else
            isclient = TRUE;

        aos_memset(str, '\0', AT_RESP_RESULT_MAX);
        if (aic_tcpip_at_cfg.cipmux == 1) {
            snprintf(str, sizeof(str), "+CIPSTATE: %d,%d,%s,%s,%d,%d,%d",
                     link_id, state, socket_type, addr, remote_port,
                     local_port, isclient);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else {
            snprintf(str, sizeof(str), "+CIPSTATE: %d,%s,%s,%d,%d,%d",
                     state, socket_type, addr, remote_port, local_port,
                     isclient);
            aic_transmit_resp_normal(at_req_resp_ptr, str);

            if (aic_tcpip_at_cfg.cipserver == 0) {
                aic_reply_ok_resp(at_req_resp_ptr);
                return AIC_TCPIP_AT_OK;
            }
        }
        i++;
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipdomain_set_cmd() - Do dns query.
 * @para_num: parameter.
 *
 * CALLS:
 *    getaddrinfo
 *
 * CALLED BY:
 *    Application
 *
 * Do dns query, and info dns server address.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipdomain_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;
    struct addrinfo *res;
    struct addrinfo *hints;
    char addr[ADDR_BUFFERSIZE_IPV6] = {0};
    char str[BUFFER_SIZE] = {'\0'};
    int ret;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPDOMAIN=",
                                             &cnt);
    if (!paras_arr || (cnt < 1 || cnt > 2)) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    hints = aos_zalloc(sizeof(struct addrinfo));
    if (cnt == 2) {
        if (atoi((u8 *)paras_arr[1]) == 1 || atoi((u8 *)paras_arr[1]) == 2) {
            hints->ai_family = AF_INET;
            aicx_info(0x08a49d, "", "[AT_TCPIP]: do dns in ipv4\n");
        } else if (atoi((u8 *)paras_arr[1]) == 3) {
            hints->ai_family = AF_INET6;
            aicx_info(0x0c7eeb, "", "[AT_TCPIP]: do dns in ipv6\n");
        } else {
            aos_free(hints);
            aicx_info(0x066a48, "", "[AT_TCPIP]: Not support\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_AT_CMD_NOT_SUPPORT);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_AT_ERR;
        }
    }else {
        hints->ai_family = AF_INET;
    }

    ret = getaddrinfo((u8 *)paras_arr[0], NULL, hints, &res);
    if (ret != NX_SUCCESS || !res) {
        aicx_error(0x0a036e, "sd", "[AT_TCPIP]: get addr info fail %s ret %d\r\n",
                   (s8 *)paras_arr[0], ret);
        aic_free_paras_mem(paras_arr, cnt);
        aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_DNS_FAIL);
        aos_free(hints);
        return ret;
    }

    if(res->ai_addr->sa_family == AF_INET) {
        inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr,
                  addr, ADDR_BUFFERSIZE_IPV4);
    } else {
        inet_ntop(AF_INET6, &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr,
                  addr, ADDR_BUFFERSIZE_IPV6);
    }

    snprintf(str, sizeof(str), "+CIPDOMAIN: %s", addr);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);

    aic_free_paras_mem(paras_arr, cnt);
    aos_free(hints);
    freeaddrinfo(res);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipstart_set_cmd() - Establish TCP/UDP/SSL connection.
 * @para_num: parameter.
 *
 * CALLS:
 *    socket
 *
 * CALLED BY:
 *    Application
 *
 * Establish TCP/UDP/SSL connection.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipstart_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
#ifdef AT_TLS_TEST
    ulong addr, addr6[4];
#endif
    u16 cnt;
    int linkid = 0, sock, index, ret, len;
#if defined(CONFIG_NET_AT_TCP) || defined(CONFIG_NET_AT_UDP) || defined(CONFIG_NET_AT_TLS)
    int protocolFamily, socket_type;
#endif
    struct sockaddr_in remoteaddr;
    struct sockaddr_in6 remoteaddr6;
    struct addrinfo *res;
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        if (aic_tcpip_at_cfg.cipmux == 1) {
            paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                     "AT+CIPSTART=",
                                                     &cnt);
            index = 1;
            if (cnt != 4) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
            if (strlen((u8 *)paras_arr[0]))
                linkid = atoi((u8 *)paras_arr[0]);
        } else {
            if (sock_list[0].socket_id != 0) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT);
                return AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT;
            }
            paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                     "AT+CIPSTART=",
                                                     &cnt);
            index = 0;
            linkid = AT_SOCK_LINKID_DEFAULT;
            if (cnt != 3) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
        }

        if (!paras_arr) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }

        if (aic_at_find_socket_by_linkid(linkid, AT_TCPIP)) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }

        if (linkid >= AT_SOCKET_MAX_CONN_NUM) {
            aicx_error(0x09acca, "", "[AT_TCPIP]: socket link id error\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }

#ifdef AT_TLS_TEST
        aos_memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
        aos_memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
        remoteaddr.sin_port = htons(atoi((u8 *)paras_arr[index + 2]));
        remoteaddr6.sin6_port = htons(atoi((u8 *)paras_arr[index + 2]));
#else
        ret = getaddrinfo((u8 *)paras_arr[index + 1], AIC_NULL, AIC_NULL, &res);
        if (ret != NX_SUCCESS || !res) {
            aicx_error(0x0a036e, "sd", "[AT_TCPIP]: get addr info fail %s ret %d\r\n",
                       (s8 *)paras_arr[index + 1], ret);
            aic_free_paras_mem(paras_arr, cnt);
            aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_DNS_FAIL);
            return ret;
        }

        if (res->ai_family == AF_INET) {
            aos_memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
            remoteaddr.sin_family = AF_INET;
            remoteaddr.sin_port = htons(atoi((u8 *)paras_arr[index + 2]));
            remoteaddr.sin_addr.s_addr =
                ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
        } else {
            aos_memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
            remoteaddr6.sin6_family = AF_INET6;
            remoteaddr6.sin6_port = htons(atoi((u8 *)paras_arr[index + 2]));
            aos_memcpy(&remoteaddr6.sin6_addr,
                       &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
        }
#endif

#ifdef CONFIG_NET_AT_TCP
        if (strcmp((u8 *)paras_arr[index], "TCP") == 0 ){
            protocolFamily = AF_INET;
            socket_type = SOCK_TCP;
            aicx_info(0x1770e8, "", "[AT_TCP]: --TCP\n");
#ifdef AT_TLS_TEST
            remoteaddr.sin_family = protocolFamily;
            inet_aton((u8 *)paras_arr[index + 1], (struct in_addr *)&addr);
            remoteaddr.sin_addr.s_addr = addr;
#endif
            sock = aic_create_tcp_socket(AF_INET,
                                         (struct sockaddr *)&remoteaddr,
                                         linkid);
        } else if (strcmp((u8 *)paras_arr[index], "TCPv6") == 0) {
            protocolFamily = AF_INET6;
            socket_type = SOCK_TCPv6;
            aicx_info(0x096e48, "", "[AT_TCP]: --TCPv6\n");
#ifdef AT_TLS_TEST
            remoteaddr6.sin6_family = protocolFamily;
            inet_pton(AF_INET6, (u8 *)paras_arr[index + 1], &addr6);
            aos_memcpy(&remoteaddr6.sin6_addr, &addr6, 16);
#endif
            sock = aic_create_tcp_socket(AF_INET6,
                                         (struct sockaddr *)&remoteaddr6,
                                         linkid);
        } else
#endif
#ifdef CONFIG_NET_AT_UDP
        if (strcmp((u8 *)paras_arr[index], "UDP") == 0) {
            protocolFamily = AF_INET;
            socket_type = SOCK_UDP;
            aicx_info(0x074783, "", "[AT_UDP]: --UDP\n");
#ifdef AT_TLS_TEST
            inet_aton((u8 *)paras_arr[index + 1], (struct in_addr *)&addr);
            remoteaddr.sin_addr.s_addr = addr;
#endif
            sock = aic_create_udp_socket(AF_INET,
                                         (struct sockaddr *)&remoteaddr,
                                         linkid);
        } else if (strcmp((u8 *)paras_arr[index], "UDPv6") == 0) {
            protocolFamily = AF_INET6;
            socket_type = SOCK_UDPv6;
            aicx_info(0x0cd7fb, "", "[AT_UDP]: --UDPv6\n");
#ifdef AT_TLS_TEST
            inet_aton((u8 *)paras_arr[index + 1], (struct in_addr *)&addr);
            aos_memcpy(&remoteaddr6.sin6_addr, &addr6, 16);
#endif
            sock = aic_create_udp_socket(AF_INET6,
                                         (struct sockaddr *)&remoteaddr6,
                                         linkid);
        } else
#endif
#ifdef CONFIG_NET_AT_TLS
        if (strcmp((u8 *)paras_arr[index], "SSL") == 0) {
#ifdef CONFIG_NET_SECURE
            protocolFamily = AF_INET;
            socket_type = SOCK_TLS;
            aicx_info(0x0d728f, "", "[AT_TLS]: --SSL\n");
#ifdef AT_TLS_TEST
            remoteaddr.sin_family = protocolFamily;
            inet_aton((u8 *)paras_arr[index + 1], (struct in_addr *)&addr);
            remoteaddr.sin_addr.s_addr = addr;
#endif
            sock = aic_at_create_tls_socket(AF_INET, (struct sockaddr *)&remoteaddr,
                                            sizeof(struct sockaddr_in), linkid);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            sock = aic_at_create_mbedtls_socket(AF_INET,
                                                (struct sockaddr *)&remoteaddr,
                                                NULL, linkid);
#endif
        } else if (strcmp((u8 *)paras_arr[index], "SSLv6") == 0) {
#ifdef CONFIG_NET_SECURE
            protocolFamily = AF_INET6;
            socket_type = SOCK_TLSv6;
            aicx_info(0x15f168, "", "[AT_TLS]: --SSLv6\n");
#ifdef AT_TLS_TEST
            remoteaddr6.sin6_family = protocolFamily;
            inet_pton(AF_INET6, (u8 *)paras_arr[index + 1], &addr6);
            aos_memcpy(&remoteaddr6.sin6_addr, &addr6, 16);
#endif
            sock = aic_at_create_tls_socket(AF_INET6,
                                            (struct sockaddr *)&remoteaddr6,
                                            sizeof(struct sockaddr_in6),
                                            linkid);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            sock = aic_at_create_mbedtls_socket(AF_INET6,
                                                (struct sockaddr *)&remoteaddr6,
                                                NULL, linkid);
#endif
        } else
#endif
        {
            aicx_info(0x11194c, "", "Not Support!\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_AT_CMD_NOT_SUPPORT);
            aic_free_paras_mem(paras_arr, cnt);
#ifndef CONFIG_NET_AT_TEST
            freeaddrinfo(res);
#endif
            return AIC_TCPIP_AT_ERR;
        }

        if (sock < 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL);
            aic_free_paras_mem(paras_arr, cnt);
#ifndef CONFIG_NET_AT_TEST
            freeaddrinfo(res);
#endif
            return AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL;
        }

        sockid = sock;
        length = 1460;
        if (aic_tcpip_at_cfg.ciprecvmode == 0)
            aos_resume_thread(at_socket_recv_thread);

        if (aic_tcpip_at_cfg.cipmode == 1)
            at_req_resp_ptr->at_mode = AT_DATA_MODE;

        if (aic_tcpip_at_cfg.cipmux == 1)
            snprintf(str, sizeof(str), "%d, CONNECT", linkid);
        else
            snprintf(str, sizeof(str), "CONNECT");

        aic_tcpip_at_cfg.com_id = at_req_resp_ptr->serial_port_id;
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_reply_ok_resp(at_req_resp_ptr);
        aic_free_paras_mem(paras_arr, cnt);
#ifndef CONFIG_NET_AT_TEST
        freeaddrinfo(res);
#endif
        return AIC_TCPIP_AT_OK;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE){
        if (at_req_resp_ptr->end_data_mode) {
            sockid = 0;
            length = 0;
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
            return AIC_TCPIP_AT_ERR;
        }

        index = aic_at_find_index_by_socket(sockid, AT_TCPIP);
        if (index < 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x124873, "", "[AT_TCPIP]: change to AT conmand mode\n");
            aic_reply_ok_resp(at_req_resp_ptr);
            return AIC_TCPIP_AT_OK;
        }

        len = at_req_resp_ptr->upstream_data_len;
        if (length != 0 && len > length)
            len = length;

#ifdef CONFIG_NET_AT_TCP
        if (sock_list[index].socket_type == SOCK_TCP ||
            sock_list[index].socket_type == SOCK_TCPv6) {
            ret = anet_send(sockid, at_req_resp_ptr->at_upstream_data, len, 0);
        } else
#endif
#ifdef CONFIG_NET_AT_UDP
        if (sock_list[index].socket_type == SOCK_UDP ||
            sock_list[index].socket_type == SOCK_UDPv6) {
            ret = aic_udp_socket_send(sockid,
                                      at_req_resp_ptr->at_upstream_data,
                                      len);
        } else
#endif
#ifdef CONFIG_NET_AT_TLS
        if (sock_list[index].socket_type == SOCK_TLS ||
                   sock_list[index].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
            ret = anet_send(sockid,
                            at_req_resp_ptr->at_upstream_data,
                            len, 0);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            ret = mbedtls_ssl_write((mbedtls_ssl_context *)sock_list[index].ssl,
                                    at_req_resp_ptr->at_upstream_data,
                                    len);
#endif
        } else
#endif
        {
            aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (ret < 0) {
            sprintf(str, "SEND ERROR,errno %d", (errno));
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", (errno));
        } else {
            strcpy(str, "SEND OK");
            aicx_info(0x1d42d6, "d", "[AT_TCPIP]: send ret %d\n", ret);
        }
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_OK;
    } else {
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

/**
 * handle_cipstartex_set_cmd() - Establish TCP/UDP/SSL connection.
 * @para_num: parameter.
 *
 * CALLS:
 *    socket
 *
 * CALLED BY:
 *    Application
 *
 * Establish TCP/UDP/SSL connection.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipstartex_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;
    int linkid, sock, index, ret;
#if defined(CONFIG_NET_AT_TCP) || defined(CONFIG_NET_AT_UDP) || defined(CONFIG_NET_AT_TLS)
    int protocolFamily = AF_INET, socket_type;
#endif
    struct sockaddr_in remoteaddr;
    struct sockaddr_in6 remoteaddr6;
    struct addrinfo *res;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSTARTEX=",
                                                 &cnt);
        index = 0;
        linkid = aic_at_find_empty_socket(AT_TCPIP);
        if (linkid < 0) {
            aic_free_paras_mem(paras_arr, cnt);
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL);
            return AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL;
        }
    } else {
        if (sock_list[0].socket_id != 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT);
            return AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT;
        }
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSTARTEX=",
                                                 &cnt);
        index = 0;
        linkid = AT_SOCK_LINKID_DEFAULT;
    }

    if (!paras_arr || cnt != 3) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    ret = getaddrinfo((u8 *)paras_arr[0], AIC_NULL, AIC_NULL, &res);
    if (ret != NX_SUCCESS || !res) {
        aicx_error(0x0a036e, "sd", "[AT_TCPIP]: get addr info fail %s ret %d\r\n",
                   (s8 *)paras_arr[0], ret);
        aic_free_paras_mem(paras_arr, cnt);
        aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_DNS_FAIL);
        return ret;
    }
    
    if (res->ai_family == AF_INET) {
        aos_memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
        remoteaddr.sin_family = AF_INET;
        remoteaddr.sin_port = htons(atoi((u8 *)paras_arr[index + 2]));
        remoteaddr.sin_addr.s_addr =
            ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
    } else {
        aos_memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
        remoteaddr6.sin6_family = AF_INET6;
        remoteaddr6.sin6_port = htons(atoi((u8 *)paras_arr[index + 2]));
        aos_memcpy(&remoteaddr6.sin6_addr,
                   &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
    }


#ifdef CONFIG_NET_AT_TCP
    if (strcmp((u8 *)paras_arr[index], "TCP") == 0 ){
        protocolFamily = AF_INET;
        socket_type = SOCK_STREAM;
        aicx_info(0x1770e8, "", "[AT_TCP]: --TCP\n");
        sock = aic_create_tcp_socket(protocolFamily,
                                     (struct sockaddr *)&remoteaddr,
                                     linkid);
    } else if (strcmp((u8 *)paras_arr[index], "TCPv6") == 0) {
        protocolFamily = AF_INET6;
        socket_type = SOCK_STREAM;
        aicx_info(0x096e48, "", "[AT_TCP]: --TCPv6\n");
        sock = aic_create_tcp_socket(protocolFamily,
                                     (struct sockaddr *)&remoteaddr6,
                                     linkid);
    } else
#endif
#ifdef CONFIG_NET_AT_UDP
    if (strcmp((u8 *)paras_arr[index], "UDP") == 0) {
        protocolFamily = AF_INET;
        socket_type = SOCK_DGRAM;
        aicx_info(0x074783, "", "[AT_UDP]: --UDP\n");
        sock = aic_create_udp_socket(protocolFamily,
                                     (struct sockaddr *)&remoteaddr,
                                     linkid);
    } else if (strcmp((u8 *)paras_arr[index], "UDPv6") == 0) {
        protocolFamily = AF_INET6;
        socket_type = SOCK_DGRAM;
        aicx_info(0x0cd7fb, "", "[AT_UDP]: --UDPv6\n");
        sock = aic_create_udp_socket(protocolFamily,
                                     (struct sockaddr *)&remoteaddr6,
                                     linkid);
    } else
#endif
#ifdef CONFIG_NET_AT_TLS
    if (strcmp((u8 *)paras_arr[index], "SSL") == 0) {
#ifdef CONFIG_NET_SECURE
        protocolFamily = AF_INET;
        socket_type = SOCK_STREAM;
        aicx_info(0x0d728f, "", "[AT_TLS]: --SSL\n");
        sock = aic_at_create_tls_socket(protocolFamily,
                                        (struct sockaddr *)&remoteaddr,
                                        sizeof(struct sockaddr_in), linkid);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
        sock = aic_at_create_mbedtls_socket(protocolFamily,
                                            (struct sockaddr *)&remoteaddr,
                                            NULL, linkid);
#endif
    } else if (strcmp((u8 *)paras_arr[index], "SSLv6") == 0) {
#ifdef CONFIG_NET_SECURE
        protocolFamily = AF_INET6;
        socket_type = SOCK_STREAM;
        aicx_info(0x15f168, "", "[AT_TLS]: --SSLv6\n");
        sock = aic_at_create_tls_socket(protocolFamily,
                                        (struct sockaddr *)&remoteaddr6,
                                        sizeof(struct sockaddr_in6), linkid);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
        sock = aic_at_create_mbedtls_socket(protocolFamily,
                                            (struct sockaddr *)&remoteaddr6,
                                            NULL, linkid);
#endif
    } else
#endif
    {
        aicx_info(0x0e1c8f, "", "[AT_TCPIP]: Not Support!\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_AT_CMD_NOT_SUPPORT);
        aic_free_paras_mem(paras_arr, cnt);
        freeaddrinfo(res);
        return AIC_TCPIP_AT_ERR;
    }

    if (sock < 0) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL);
        aic_free_paras_mem(paras_arr, cnt);
        freeaddrinfo(res);
        return AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL;
    }

    linkid = aic_at_find_linkid_by_socket(sock, AT_TCPIP);
    if (aic_tcpip_at_cfg.cipmux == 1) {
        snprintf(str, sizeof(str), "%d, CONNECT", linkid);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_reply_ok_resp(at_req_resp_ptr);
    } else {
        snprintf(str, sizeof(str), "CONNECT");
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_reply_ok_resp(at_req_resp_ptr);
    }
    freeaddrinfo(res);

    aic_tcpip_at_cfg.com_id = at_req_resp_ptr->serial_port_id;
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipsend_set_cmd() - Send TCP/UDP/SSL data.
 * @para_num: parameter.
 *
 * CALLS:
 *    soc_close
 *
 * CALLED BY:
 *    Application
 *
 * Close TCP/UDP/SSL connection.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 */
u32 handle_cipsend_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int link_id = 0;
    ulong *paras_arr;
    u16 cnt;
    int index;
    int len;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int ret;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if(at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        if (aic_tcpip_at_cfg.cipmux == 1) {
            paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                     "AT+CIPSEND=",
                                                     &cnt);
            index = 1;
            if (cnt != 2) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
        } else {
            paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                     "AT+CIPSEND=",
                                                     &cnt);
            index = 0;
            if (cnt != 1) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
        }

        if (!paras_arr) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }

        if (aic_tcpip_at_cfg.cipmux == 1) {
            if (strlen((u8 *)paras_arr[0]))
                link_id = atoi((u8 *)paras_arr[0]);

            if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_INVAL_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_INVAL_AT_PARAM;
            }
        }

        length = atoi((u8 *)paras_arr[index]);
        if (length < 1 || length > 1460) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }
        sockid = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
        if (!sockid){
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        at_req_resp_ptr->at_mode = AT_DATA_MODE;

        strcpy(str, ">");
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    } else if(at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        aicx_info(0x03b0ee, "d", "[AT_TCPIP]: recv data len %d\n", at_req_resp_ptr->upstream_data_len);

        if (at_req_resp_ptr->end_data_mode) {
            sockid = 0;
            length = 0;
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
            return AIC_TCPIP_AT_ERR;
        }

        index = aic_at_find_index_by_socket(sockid, AT_TCPIP);
        if (index < 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x124873, "", "[AT_TCPIP]: change to AT conmand mode\n");
            aic_reply_ok_resp(at_req_resp_ptr);
            return AIC_TCPIP_AT_OK;
        }

        len = at_req_resp_ptr->upstream_data_len;
        if (length != 0 && len > length)
            len = length;

#ifdef CONFIG_NET_AT_TCP
        if (sock_list[index].socket_type == SOCK_TCP ||
            sock_list[index].socket_type == SOCK_TCPv6) {
            ret = anet_send(sockid, at_req_resp_ptr->at_upstream_data, len, 0);
        } else
#endif
#ifdef CONFIG_NET_AT_UDP
        if (sock_list[index].socket_type == SOCK_UDP ||
            sock_list[index].socket_type == SOCK_UDPv6) {
            ret = aic_udp_socket_send(sockid,
                                      at_req_resp_ptr->at_upstream_data,
                                      len);
        } else
#endif
#ifdef CONFIG_NET_AT_TLS
        if (sock_list[index].socket_type == SOCK_TLS ||
                   sock_list[index].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
            ret = anet_send(sockid,
                            at_req_resp_ptr->at_upstream_data,
                            len, 0);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            ret = mbedtls_ssl_write((mbedtls_ssl_context *)sock_list[index].ssl,
                                    at_req_resp_ptr->at_upstream_data,
                                    len);
#endif
        } else
#endif
        {
            aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (ret < 0) {
            sprintf(str, "SEND ERROR,errno %d", (errno));
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", (errno));
        } else {
            strcpy(str, "SEND OK");
            aicx_info(0x1d42d6, "d", "[AT_TCPIP]: send ret %d\n", ret);
        }
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        at_req_resp_ptr->at_mode = AT_CMD_MODE;
        return AIC_TCPIP_AT_OK;
    } else {
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

u32 handle_cipqsend_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    strcpy(str, "+CIPQSEND: (0,1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipqsend_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPQSEND=",
                                             &cnt);

    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if ((*(u8 *)paras_arr[0] != '0') && (*(u8 *)paras_arr[0] != '1')) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }
    aic_tcpip_at_cfg.cipsendmode = atoi((u8 *)paras_arr[0]);

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipqsend_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPQSEND: %d", aic_tcpip_at_cfg.cipsendmode);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsendl_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int link_id = 0;
    ulong *paras_arr;
    u16 cnt;
    int index, ret, len;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if(at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        if (aic_tcpip_at_cfg.cipmux == 1) {
            paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                     "AT+CIPSENDL=",
                                                     &cnt);
            index = 1;
            if (cnt != 2) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
        } else {
            paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                     "AT+CIPSENDL=",
                                                     &cnt);
            index = 0;
            if (cnt != 1) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
        }

        if (!paras_arr) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }

        if (aic_tcpip_at_cfg.cipmux == 1) {
            link_id = atoi((u8 *)paras_arr[0]);
            if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_INVAL_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_INVAL_AT_PARAM;
            }
        }

        length = atoi((u8 *)paras_arr[index]);
        if (length < 1 || length > 1460) {
            aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_INVAL_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }
        sockid = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
        if (!sockid){
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        at_req_resp_ptr->at_mode = AT_DATA_MODE;

        strcpy(str, ">");
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        if (at_req_resp_ptr->end_data_mode) {
            sockid = 0;
            length = 0;
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
            return AIC_TCPIP_AT_ERR;
        }

        index = aic_at_find_index_by_socket(sockid, AT_TCPIP);
        if (index < 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x124873, "", "[AT_TCPIP]: change to AT conmand mode\n");
            aic_reply_ok_resp(at_req_resp_ptr);
            return AIC_TCPIP_AT_OK;
        }

        len = at_req_resp_ptr->upstream_data_len;
        if (length != 0 && len > length)
            len = length;

#ifdef CONFIG_NET_AT_TCP
        if (sock_list[index].socket_type == SOCK_TCP ||
            sock_list[index].socket_type == SOCK_TCPv6) {
            ret = anet_send(sockid, at_req_resp_ptr->at_upstream_data, len, 0);
        } else
#endif
#ifdef CONFIG_NET_AT_UDP
        if (sock_list[index].socket_type == SOCK_UDP ||
            sock_list[index].socket_type == SOCK_UDPv6) {
            ret = aic_udp_socket_send(sockid,
                                      at_req_resp_ptr->at_upstream_data,
                                      len);
        } else
#endif
#ifdef CONFIG_NET_AT_TLS
        if (sock_list[index].socket_type == SOCK_TLS ||
                   sock_list[index].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
            ret = anet_send(sockid,
                            at_req_resp_ptr->at_upstream_data,
                            len, 0);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            ret = mbedtls_ssl_write((mbedtls_ssl_context *)sock_list[index].ssl,
                                    at_req_resp_ptr->at_upstream_data,
                                    len);
#endif

        } else
#endif
        {
            aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (ret < 0) {
            sprintf(str, "SEND ERROR,errno %d", (errno));
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", errno);
        } else {
            strcpy(str, "SEND OK");
            aicx_info(0x19793d, "sd", "[AT_TCPIP]: send data %s,ret %d\n",
                      at_req_resp_ptr->at_upstream_data, ret);
        }
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_OK;
    } else {
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

u32 handle_cipsendlcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPSENDLCFG=",
                                             &cnt);
    if (!paras_arr || cnt != 2) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    aic_tcpip_at_cfg.report_size = atoi((u8 *)paras_arr[0]);
    aic_tcpip_at_cfg.strans_size = atoi((u8 *)paras_arr[1]);
    if (aic_tcpip_at_cfg.report_size < 100 ||
        aic_tcpip_at_cfg.report_size > 1460 ||
        aic_tcpip_at_cfg.strans_size < 100 ||
        aic_tcpip_at_cfg.strans_size > 1460) {
        aic_free_paras_mem(paras_arr, cnt);
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsendlcfg_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    snprintf(str, sizeof(str), "+CIPSENDLCFG: %d,%d",
             aic_tcpip_at_cfg.report_size, aic_tcpip_at_cfg.strans_size);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsendex_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int link_id = 0;
    ulong *paras_arr;
    u16 cnt;
    int index, ret, len;
    u8 *buf;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if(at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        if (aic_tcpip_at_cfg.cipmux == 1) {
            paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                     "AT+CIPSENDEX=",
                                                     &cnt);
            index = 1;
            if (cnt != 2) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
        } else {
            paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                     "AT+CIPSENDEX=",
                                                     &cnt);
            index = 0;
            if (cnt != 1) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
        }

        if (!paras_arr) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }

        if (aic_tcpip_at_cfg.cipmux == 1) {
            link_id = atoi((u8 *)paras_arr[0]);
            if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_INVAL_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_INVAL_AT_PARAM;
            }
        }

        length = atoi((u8 *)paras_arr[index]);
        if (length < 2 || length > 1460 || length % 2) {
            aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_INVAL_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }
        sockid = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
        if (!sockid){
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }
        at_req_resp_ptr->at_mode = AT_DATA_MODE;

        strcpy(str, ">");
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        if (at_req_resp_ptr->end_data_mode) {
            sockid = 0;
            length = 0;
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
            return AIC_TCPIP_AT_ERR;
        }

        index = aic_at_find_index_by_socket(sockid, AT_TCPIP);
        if (index < 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }
#if 0
        if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x124873, "", "[AT_TCPIP]: change to AT conmand mode\n");
            aic_reply_ok_resp(at_req_resp_ptr);
            return AIC_TCPIP_AT_OK;
        }
#endif
        len = at_req_resp_ptr->upstream_data_len;
        if (len % 2) {
            strcpy(str, "SEND DATA length error");
            goto SENDEX_EXIT;
        }
        if (length != 0 && len > length)
            len = length;

        buf = aos_zalloc_relaxed(AT_SOCKET_DATA_LEN_MAX);
        if (!buf) {
            strcpy(str, "SEND ERROR, NO MEM");
            goto SENDEX_EXIT;
        }

        aic_hexstring_to_char(buf, at_req_resp_ptr->at_upstream_data, len);
#ifdef CONFIG_NET_AT_TCP
        if (sock_list[index].socket_type == SOCK_TCP ||
            sock_list[index].socket_type == SOCK_TCPv6) {
            ret = anet_send(sockid, buf, len/2, 0);
        } else
#endif
#ifdef CONFIG_NET_AT_UDP
        if (sock_list[index].socket_type == SOCK_UDP ||
            sock_list[index].socket_type == SOCK_UDPv6) {
            ret = aic_udp_socket_send(sockid, buf, len/2);
        } else
#endif
#ifdef CONFIG_NET_AT_TLS
        if (sock_list[index].socket_type == SOCK_TLS ||
                   sock_list[index].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
            ret = anet_send(sockid, buf, len/2, 0);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            ret = mbedtls_ssl_write((mbedtls_ssl_context *)sock_list[index].ssl,
                                    buf, len/2);
#endif
        } else
#endif
        {
            aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (ret < 0) {
            sprintf(str, "SEND ERROR,errno %d", (errno));
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", errno);
        } else {
            strcpy(str, "SEND OK");
            aicx_info(0x19793d, "sd", "[AT_TCPIP]: send data %s,ret %d\n",
                      at_req_resp_ptr->at_upstream_data, ret);
        }
SENDEX_EXIT:
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        sockid = 0;
        length = 0;
        at_req_resp_ptr->at_mode = AT_CMD_MODE;
        return AIC_TCPIP_AT_OK;
    } else {
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

u32 handle_ciphead_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    strcpy(str, "+CIPHEAD: (0,1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_ciphead_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPHEAD=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if ((*(u8 *)paras_arr[0] != '0') && (*(u8 *)paras_arr[0] != '1')) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    aic_tcpip_at_cfg.ciphead = atoi((u8 *)paras_arr[0]);
    aic_reply_ok_resp(at_req_resp_ptr);

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_ciphead_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPHEAD: %d", aic_tcpip_at_cfg.ciphead);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipclose_set_cmd() - Close TCP/UDP/SSL connection.
 * @para_num: parameter.
 *
 * CALLS:
 *    soc_close
 *
 * CALLED BY:
 *    Application
 *
 * Close TCP/UDP/SSL connection.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 */
u32 handle_cipclose_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    int link_id = 0, i, n = 0, sock;
    ulong *paras_arr;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPCLOSE=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    link_id = atoi((u8 *)paras_arr[0]);
    if (link_id > AT_SOCKET_MAX_CONN_NUM) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (link_id == AT_SOCKET_MAX_CONN_NUM) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
            if (sock_list[i].socket_id) {
                sock = sock_list[i].socket_id;
#ifdef CONFIG_NET_AT_TLS
                if(sock_list[i].socket_type == SOCK_TLS ||
                   sock_list[i].socket_type == SOCK_TLSv6) {
                    aos_memset(&sock_list[i], 0, sizeof(struct SOCKET_LIST));
#ifdef CONFIG_NET_SECURE
                    if (sock_list[i].common_name)
                        aos_free(sock_list[i].common_name);

                    if (sock_list[i].ca_cert_der)
                        aos_free(sock_list[i].ca_cert_der);
                    aic_tls_ca_certificate_remove_all(sock);
                    aic_close_tls_socket(sock);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
                    aic_at_close_mbedtls_socket(sock);
#endif
                } else
#endif
                {
                    aos_memset(&sock_list[i], 0, sizeof(struct SOCKET_LIST));
                    soc_close(sock);
                }
            } else {
                n++;
                if (n == AT_SOCKET_MAX_CONN_NUM) {
                    aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
                    aic_free_paras_mem(paras_arr, cnt);
                    return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
                }
            }
        }
        strcpy(str, "CLOSED");
    } else {
        i = aic_at_find_index_by_linkid(link_id, AT_TCPIP);
        if (sock_list[i].socket_id == 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }
#ifdef CONFIG_NET_AT_TLS
        if(sock_list[i].socket_type == SOCK_TLS ||
           sock_list[i].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
            aic_tls_ca_certificate_remove_all(sock_list[i].socket_id);
            aic_close_tls_socket(sock_list[i].socket_id);
            if (sock_list[i].common_name)
                aos_free(sock_list[i].common_name);
            if (sock_list[i].ca_cert_der)
                aos_free(sock_list[i].ca_cert_der);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            aic_at_close_mbedtls_socket(sock_list[i].socket_id);
#endif
        } else
#endif
        {
            soc_close(sock_list[i].socket_id);
        }
        snprintf(str, sizeof(str), "%d,CLOSED", sock_list[i].link_id);
        aos_memset(&sock_list[i], 0, sizeof(struct SOCKET_LIST));
    }

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cifsr_exec_cmd() - Query local ip address and MAC address.
 * @para_num: parameter.
 *
 * CALLS:
 *    non
 *
 * CALLED BY:
 *    Application
 *
 * Query local ip address and MAC address.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 */
u32 handle_cifsr_exec_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_IP *ip_ptr;
    struct aic_network_ip_instance *net_ip;
    int ip_addr = 0, ip6_addr[4] = {0}, i;
    char addr[ADDR_BUFFERSIZE_IPV4];
    char addr6[ADDR_BUFFERSIZE_IPV6];
    u8 mac[6] = {0};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if (*(at_req_resp_ptr->at_req + 8) != 0x0D &&
        *(at_req_resp_ptr->at_req + 8) != 0x0A) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_CMD);
        return AIC_TCPIP_ERR_INVAL_AT_CMD;
    }

    net_ip = aic_get_pdp_ip_instance();
    ip_ptr = net_ip->ip_ptr;
    if (!ip_ptr) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    for (i = 0; i < NX_MAX_IP_INTERFACES; i++) {
        if (!ip_ptr->nx_ip_interface[i].nx_interface_valid)
            continue;

        ip_addr = ip_ptr->nx_ip_interface[i].nx_interface_ip_address;
        NX_CHANGE_ULONG_ENDIAN(ip_addr);
        if (ip_ptr->nx_ip_interface[i].nxd_interface_ipv6_address_list_head &&
            ip_ptr->nx_ip_interface[i].nxd_interface_ipv6_address_list_head->nxd_ipv6_address_next) {
            ip6_addr[0] = ip_ptr->nx_ip_interface[i].nxd_interface_ipv6_address_list_head->nxd_ipv6_address_next->nxd_ipv6_address[0];
            ip6_addr[1] = ip_ptr->nx_ip_interface[i].nxd_interface_ipv6_address_list_head->nxd_ipv6_address_next->nxd_ipv6_address[1];
            ip6_addr[2] = ip_ptr->nx_ip_interface[i].nxd_interface_ipv6_address_list_head->nxd_ipv6_address_next->nxd_ipv6_address[2];
            ip6_addr[3] = ip_ptr->nx_ip_interface[i].nxd_interface_ipv6_address_list_head->nxd_ipv6_address_next->nxd_ipv6_address[3];
        }
        NX_CHANGE_ULONG_ENDIAN(ip6_addr[0]);
        NX_CHANGE_ULONG_ENDIAN(ip6_addr[1]);
        NX_CHANGE_ULONG_ENDIAN(ip6_addr[2]);
        NX_CHANGE_ULONG_ENDIAN(ip6_addr[3]);
        mac[0] = (ip_ptr->nx_ip_interface[i].nx_interface_physical_address_msw &
                  0xFF00) >> 8;
        mac[1] = (ip_ptr->nx_ip_interface[i].nx_interface_physical_address_msw &
                  0xFF);
        mac[2] = (ip_ptr->nx_ip_interface[i].nx_interface_physical_address_lsw &
                  0xFF000000) >> 24;
        mac[3] = (ip_ptr->nx_ip_interface[i].nx_interface_physical_address_lsw &
                  0xFF0000) >> 16;
        mac[4] = (ip_ptr->nx_ip_interface[i].nx_interface_physical_address_lsw &
                  0xFF00) >> 8;
        mac[5] = (ip_ptr->nx_ip_interface[i].nx_interface_physical_address_lsw &
                  0xFF);

        inet_ntop(AF_INET, &ip_addr, addr, ADDR_BUFFERSIZE_IPV4);
        inet_ntop(AF_INET6, ip6_addr, addr6, ADDR_BUFFERSIZE_IPV6);
        snprintf(str, sizeof(str), "+CIFSR: addr %s,addr6 %s,mac %x:%x:%x:%x:%x:%x",
                 addr, addr6, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipmux_set_cmd() - Set connection mode.
 * @para_num: parameter.
 *
 * CALLS:
 *
 * CALLED BY:
 *    Application
 *
 * Set transparent transmission mode.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipmux_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPMUX=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if ((*(u8 *)paras_arr[0] != '0') && (*(u8 *)paras_arr[0] != '1')) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (*(u8 *)paras_arr[0] == '1') {
        aic_tcpip_at_cfg.cipmode = 0;
        aic_tcpip_at_cfg.cipmux = 1;
        aic_reply_ok_resp(at_req_resp_ptr);
    } else {
        aic_tcpip_at_cfg.cipmux = atoi((u8 *)paras_arr[0]);
        aic_reply_ok_resp(at_req_resp_ptr);
    }

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

/**
  * handle_cipmux_query_cmd() - Query connection mode.
  *
  * CALLS:
  *    non
  *
  * CALLED BY:
  *    Application
  *
  * Query whether IPv6 is support.
  *
  * Return: AIC_TCPIP_AT_OK for success, others for fail.
  *
  */
u32 handle_cipmux_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPMUX: %d", aic_tcpip_at_cfg.cipmux);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipserver_set_cmd() - Set transparent transmission server.
 * @para_num: parameter.
 *
 * CALLS:
 *    non
 *
 * CALLED BY:
 *    Application
 *
 * Set transparent transmission mode.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipserver_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int i, index, server = 0, sock= -1;
#ifdef CONFIG_NET_AT_TCP
    int protocolFamily = AF_INET, socket_type = 0;
    struct sockaddr_in localaddr;
    struct sockaddr_in6 localaddr6;
#endif

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPSERVER=",
                                             &cnt);
    if (!paras_arr || (cnt != 1 && cnt != 3)) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (strlen((u8 *)paras_arr[0]))
        server = atoi((u8 *)paras_arr[0]);
    if (server != 1 && server != 0) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 0) {
        index = aic_get_wan_net_by_cid(ANET_DEFAULT_PDP_CID);
        if (server == 1 && cnt == 3) {
            aic_at_tcp_server.port = atoi((u8 *)paras_arr[1]);
            aic_tcpip_at_cfg.cipserver = server;
#ifdef CONFIG_NET_AT_TCP
            if (strcmp((u8 *)paras_arr[2], "TCP")) {
                aos_memset(&localaddr, 0, sizeof(struct sockaddr_in));
                protocolFamily = AF_INET;
                socket_type = SOCK_TCP;
                localaddr.sin_family = AF_INET;
                localaddr.sin_addr.s_addr =
                    wan_net_list[index].wan_ipv4_address;
                localaddr.sin_port = htons(aic_at_tcp_server.port);
                sock = aic_create_tcp_socket(AF_INET,
                                             (struct sockaddr *)&localaddr,
                                             0);
            } else if (strcmp((u8 *)paras_arr[2], "TCPv6")) {
                aos_memset(&localaddr6, 0, sizeof(struct sockaddr_in6));
                protocolFamily = AF_INET6;
                socket_type = SOCK_TCPv6;
                localaddr.sin_family = AF_INET6;
                aos_memcpy(&localaddr6.sin6_addr,
                           wan_net_list[index].wan_ipv6_address1, 16);
                localaddr6.sin6_port = htons(aic_at_tcp_server.port);
                sock = aic_create_tcp_socket(AF_INET6,
                                             (struct sockaddr *)&localaddr6,
                                             0);
            } else
#endif
            {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_INVAL_AT_PARAM);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_INVAL_AT_PARAM;
            }

            if (sock < 0) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_SOCKET_CREATE_FAIL);
            } else {
                aic_tcpip_at_cfg.com_id = at_req_resp_ptr->serial_port_id;
                aic_reply_ok_resp(at_req_resp_ptr);
                aic_at_socket_listen_init();
                FD_SET(sock, &aic_at_tcp_server.master_list);
            }
        } else {
            if(server == aic_tcpip_at_cfg.cipserver) {
                aic_reply_ok_resp(at_req_resp_ptr);
            } else {
                aic_tcpip_at_cfg.cipserver = server;
                soc_close(aic_at_tcp_server.sockfd);
                FD_CLR(aic_at_tcp_server.sockfd,
                       &aic_at_tcp_server.master_list);
                aic_at_socket_listen_uninit();

                for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                    if (!sock_list[i].socket_id)
                        soc_close(sock_list[i].socket_id);

                    aos_memset(&sock_list[i], 0, sizeof(struct SOCKET_LIST));
                }
                aic_reply_ok_resp(at_req_resp_ptr);
            }
        }
    } else {
        aic_tcpip_at_cfg.cipserver = 0;
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_CMD);
    }

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipserver_query_cmd() - Query transparent transmission server.
 *
 * CALLS:
 *    non
 *
 * CALLED BY:
 *    Application
 *
 * Query DNS server configuration.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipserver_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int sock = 0, link_id = 0;
    int index;
    char *sock_type = AIC_NULL;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sock = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
    index = aic_at_find_index_by_socket(sock, AT_TCPIP);

    if(aic_tcpip_at_cfg.cipmux == 0) {
        if(aic_tcpip_at_cfg.cipserver == 1) {
            if (aic_at_tcp_server.family == AF_INET)
                sock_type = "TCP";
            else
                sock_type = "TCPv6";

            snprintf(str, sizeof(str), "+CIPSERVER: 1,%d,%s",
                     aic_at_tcp_server.port, sock_type);
        } else
            strcpy(str, "+CIPSERVER: 0");
    } else {
        strcpy(str, "+CIPSERVER: 0");
    }

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipservermaxconn_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int count;
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPSERVERMAXCONN=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    count = atoi((u8 *)paras_arr[0]);
    if (count > 0 && count <= AT_SOCKET_MAX_CONN_NUM) {
        aic_tcpip_at_cfg.server_max_count = count;
        aic_reply_ok_resp(at_req_resp_ptr);
    } else {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
    }

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipservermaxconn_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPSERVERMAXCONN: %d", aic_tcpip_at_cfg.server_max_count);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipmode_set_cmd() - Set transparent transmission mode.
 * @para_num: parameter.
 *
 * CALLS:
 *    non
 *
 * CALLED BY:
 *    Application
 *
 * Set transparent transmission mode.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipmode_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x0c9b8b, "s", "[AT_TCPIP]:--%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPMODE=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if ((*(u8 *)paras_arr[0] != '0') && (*(u8 *)paras_arr[0] != '1')) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1 && (*(u8 *)paras_arr[0] == '1')) {
        aic_tcpip_at_cfg.cipmode = 0;
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_CMD);
    } else {
        aic_tcpip_at_cfg.cipmode = atoi((u8 *)paras_arr[0]);
        aic_reply_ok_resp(at_req_resp_ptr);
    }

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipmode_query_cmd() - Query transparent transmission mode.
 *
 * CALLS:
 *    non
 *
 * CALLED BY:
 *    Application
 *
 * Query DNS server configuration.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipmode_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPMODE: %d", aic_tcpip_at_cfg.cipmode);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsto_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int timeout = 0;
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPSTO=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    timeout = atoi((u8 *)paras_arr[0]);
    if(timeout >= 0 && timeout <= TCP_SERVER_TIMEOUT) {
        aic_tcpip_at_cfg.cipsto = timeout;
        aic_reply_ok_resp(at_req_resp_ptr);
    } else {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
    }

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsto_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPSTO: %d", aic_tcpip_at_cfg.cipsto);

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipdinfo_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPDINFO=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if ((*(u8 *)paras_arr[0] != '0') && (*(u8 *)paras_arr[0] != '1')) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.ciphead == 1) {
        aic_tcpip_at_cfg.cipinfo = atoi((u8 *)paras_arr[0]);
        aic_reply_ok_resp(at_req_resp_ptr);
    } else
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_CMD);

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipdinfo_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPDINFO: %d", aic_tcpip_at_cfg.cipinfo);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipreconnintv_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int timeout = 0;
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPRECOONNINTV=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    timeout = atoi((u8 *)paras_arr[0]);

    if(timeout >= RECONN_INTV_DEFAULT && timeout <= RECONN_INTV_MAX) {
        aic_tcpip_at_cfg.cipreconnintv = timeout;
        aic_reply_ok_resp(at_req_resp_ptr);
    } else {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
    }

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipreconnintv_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPRECOONNINTV: %d", aic_tcpip_at_cfg.cipreconnintv);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);;
    return AIC_TCPIP_AT_OK;
}

u32 handle_ciprecvmode_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPRECVMODE=",
                                             &cnt);
    if (!paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if ((*(u8 *)paras_arr[0] != '0') && (*(u8 *)paras_arr[0] != '1')) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    aic_tcpip_at_cfg.com_id = at_req_resp_ptr->serial_port_id;
    aic_tcpip_at_cfg.ciprecvmode = atoi((u8 *)paras_arr[0]);
    if (aic_tcpip_at_cfg.ciprecvmode == 1) {
        aic_reply_ok_resp(at_req_resp_ptr);
        aos_suspend_thread(at_socket_recv_thread);
    } else {
        aic_reply_ok_resp(at_req_resp_ptr);
        aos_resume_thread(at_socket_recv_thread);
    }

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_ciprecvmode_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char str[BUFFER_SIZE] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    sprintf(str, "+CIPRECVMODE: %d", aic_tcpip_at_cfg.ciprecvmode);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_ciprecvdata_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index, i, addrlen, sock, length, link_id = 0;
    char *data;
    char addr[ADDR_BUFFERSIZE_IPV6];
    struct sockaddr_in remoteaddr;
    struct sockaddr_in6 remoteaddr6;
#ifdef CONFIG_NET_AT_UDP
    int fromaddrlen;
#endif
    char str[1536] = {'\0'};
    u16 port, cnt;
    ulong *paras_arr;
    int ret = -1;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPRECVDATA=",
                                                 &cnt);
        index = 1;
        if (cnt != 2) {
            aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    } else {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPRECVDATA=",
                                                 &cnt);
        index = 0;
        if (cnt != 1) {
            aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    }

    if (!paras_arr) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1) {
        link_id = atoi((u8 *)paras_arr[0]);
        if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }
    }

    length = atoi((u8 *)paras_arr[index]);
    if (length < 0 || length > 1500) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
    if (!sock) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    i = aic_at_find_index_by_socket(sock, AT_TCPIP);
    if (i < 0) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }
    sock = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);

    data = aos_zalloc(length);
    if (!data) {
        aicx_error(0x18a3e2, "", "[AT_TCPIP]: alloc recv buffer fail\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_RECV_FAIL;
    }
#ifdef CONFIG_NET_AT_TCP
    if (sock_list[i].socket_type == SOCK_TCP ||
        sock_list[i].socket_type == SOCK_TCPv6) {
        ret = anet_recv(sock, data, length, MSG_DONTWAIT);
    } else
#endif
#ifdef CONFIG_NET_AT_UDP
    if (sock_list[i].socket_type == SOCK_UDP) {
        fromaddrlen = sizeof(remoteaddr);
        ret = anet_recvfrom(sock, data, length, MSG_DONTWAIT,
                            (struct sockaddr *)&remoteaddr, &fromaddrlen);
    } else if ( sock_list[i].socket_type == SOCK_UDPv6) {
        fromaddrlen = sizeof(remoteaddr6);
        ret = anet_recvfrom(sock, data, length, MSG_DONTWAIT,
                            (struct sockaddr *)&remoteaddr6, &fromaddrlen);
    } else
#endif
#ifdef CONFIG_NET_AT_TLS
    if (sock_list[i].socket_type == SOCK_TLS ||
               sock_list[i].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
        ret = anet_recv(sock, data, length, MSG_DONTWAIT);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
        ret = mbedtls_ssl_read((mbedtls_ssl_context *)sock_list[i].ssl,
                               data, length);
#endif
    }else
#endif
    {
        aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
    }
    if (ret < 0) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
        aos_free(data);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_ERR;
    }

    aicx_info(0x0d98c1, "s", "[AT_TCPIP]: data is %s\n", data);
    if (aic_tcpip_at_cfg.ciphead == 1) {
        if (aic_tcpip_at_cfg.cipinfo == 1) {
            if (sock_list[i].family == AF_INET) {
                if (sock_list[i].socket_type != SOCK_UDP) {
                    addrlen = sizeof(struct sockaddr_in);
                    anet_getpeername(sock, (struct sockaddr *)&remoteaddr,
                                     &addrlen);
                }

                inet_ntop(AF_INET, &remoteaddr.sin_addr.s_addr,
                          addr, ADDR_BUFFERSIZE_IPV4);
                port = htons(remoteaddr.sin_port);
            } else {
                if (sock_list[i].socket_type != SOCK_UDPv6) {
                    addrlen = sizeof(struct sockaddr_in6);
                    anet_getpeername(sock, (struct sockaddr *)&remoteaddr6,
                                     &addrlen);
                }

                inet_ntop(AF_INET6, &remoteaddr6.sin6_addr, addr,
                         ADDR_BUFFERSIZE_IPV6);
                port = htons(remoteaddr6.sin6_port);
            }
            snprintf(str, sizeof(str), "+CIPRECVDATA: +IPD,%d,%s,%d,%s",
                     ret, addr, port, data);
        } else
            snprintf(str, sizeof(str), "+CIPRECVDATA: +IPD,%d,%s",
                     ret, data);
    } else
        snprintf(str, sizeof(str), "+CIPRECVDATA: %d,%s", ret, data);

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    aos_free(data);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_ciprecvlen_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int sock, i, recvlen, recvoptlen;
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    for(i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        sock = aic_at_find_socket_by_linkid(i, AT_TCPIP);
        if (sock == 0)
            continue;

        recvoptlen = sizeof(recvlen);
        anet_getsockopt(sock, SOL_SOCKET, SO_RXDATA, &recvlen, &recvoptlen);

        snprintf(str, sizeof(str), "+CIPRECVLEN: %d,%d", i, recvlen);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipdns_set_cmd() - Configure DNS server.
 * @para_num: parameter.
 *
 * CALLS:
 *    nxd_dns_server_add
 *
 * CALLED BY:
 *    Application
 *
 * Configure DNS server.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_cipdns_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt = 0;
    NX_DNS *dns_ptr;
    NXD_ADDRESS dns_server_address;
    struct in_addr dns[NX_DNS_MAX_SERVERS] = {0};
    ulong *paras_arr;
    int status;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPDNS=",
                                             &cnt);

    if (!paras_arr || (cnt < 2 || cnt > 4)) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    dns_ptr = aic_get_dns_client();
    if (cnt >= 2) {
        status = inet_pton(AF_INET, (u8 *)paras_arr[1], &dns[0]);
        if (status != 1) {
            aic_free_paras_mem(paras_arr, cnt);
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
        dns_server_address.nxd_ip_address.v4 = dns[0].s_addr;
        dns_server_address.nxd_ip_version = NX_IP_VERSION_V4;

        if (atoi((u8 *)paras_arr[0]) == 1) {
            nx_dns_server_remove_all(dns_ptr);
            nxd_dns_server_add(dns_ptr, &dns_server_address);
        }

        if (cnt >= 3) {
            status = inet_pton(AF_INET, (u8 *)paras_arr[2], &dns[0]);
            if (status != 1) {
                aic_free_paras_mem(paras_arr, cnt);
                aic_reply_error_resp(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
            dns_server_address.nxd_ip_address.v4 = dns[0].s_addr;
            dns_server_address.nxd_ip_version = NX_IP_VERSION_V4;
            if (atoi((u8 *)paras_arr[0]) == 1) {
                nxd_dns_server_add(dns_ptr, &dns_server_address);
            }

            if (cnt == 4) {
                status = inet_pton(AF_INET, (u8 *)paras_arr[3], &dns[0]);
                if (status != 1) {
                    aic_free_paras_mem(paras_arr, cnt);
                    aic_reply_error_resp(at_req_resp_ptr,
                                            AIC_TCPIP_ERR_PARSE_AT_PARAM);
                    return AIC_TCPIP_ERR_PARSE_AT_PARAM;
                }
                dns_server_address.nxd_ip_address.v4 = dns[0].s_addr;
                dns_server_address.nxd_ip_version = NX_IP_VERSION_V4;
                if (atoi((u8 *)paras_arr[0]) == 1) {
                    nxd_dns_server_add(dns_ptr, &dns_server_address);
                }
            }
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipdns_query_cmd() - Query DNS server configuration.
 *
 * CALLS:
 *    nxd_dns_server_get
 *
 * CALLED BY:
 *    Application
 *
 * Query DNS server configuration.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 */
u32 handle_cipdns_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NXD_ADDRESS dns_server_address;
    NX_DNS *dns_ptr;
    char addr[ADDR_BUFFERSIZE_IPV6] = {0};
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    int status;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    dns_ptr = aic_get_dns_client();
    status = nxd_dns_server_get(dns_ptr, 0, &dns_server_address);
    if (status != NX_SUCCESS) {
        aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    if(dns_server_address.nxd_ip_version == NX_IP_VERSION_V4){
        inet_ntop(AF_INET, (const VOID *)&dns_server_address.nxd_ip_address,
                addr, ADDR_BUFFERSIZE_IPV4);
    } else {
        inet_ntop(AF_INET6, (const VOID *)&dns_server_address.nxd_ip_address,
                addr, ADDR_BUFFERSIZE_IPV6);
    }

    sprintf(str, "+CIPDNS: %s", addr);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);;
    return AIC_TCPIP_AT_OK;
}

u32 handle_ciptcpopt_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index;
    int sock, link_id = 0;
    int nodelay;
    struct timeval sndtimeo;
    struct linger linger;
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPTCPOPT=",
                                                 &cnt);
        index = 1;
        if (cnt < 2 || cnt > 4) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    } else {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPTCPOPT=",
                                                 &cnt);
        index = 0;
        if (cnt < 1 || cnt > 3) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    }

    if (!paras_arr) {
        aicx_error(0x0e2c11, "", "parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1) {
        if (strlen((u8 *)paras_arr[0]))
            link_id = atoi((u8 *)paras_arr[0]);

        if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }
        cnt--;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
    if (!sock) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    if (cnt >= 1) {
        linger.l_linger = atoi((u8 *)paras_arr[index]);
        if (linger.l_linger < 0)
            linger.l_onoff = 0;
        else
            linger.l_onoff = 1;
        anet_setsockopt(sock, SOL_SOCKET, SO_LINGER,
                        (const VOID *)&linger, sizeof(linger));

        if (cnt >= 2) {
            nodelay = atoi((u8 *)paras_arr[index + 1]);
            anet_setsockopt(sock, IPPROTO_IP, TCP_NODELAY,
                            (const VOID *)&nodelay, sizeof(nodelay));

            if (cnt == 3) {
                sndtimeo.tv_sec = 0;
                sndtimeo.tv_usec = atoi((u8 *)paras_arr[index + 2]) * 1000;
                anet_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
                                (const VOID *)&sndtimeo, sizeof(sndtimeo));
            }
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_ciptcpopt_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int link_id;
    int sock;
    int nodelay = 0;
    struct timeval sndtimeo = {0};
    struct linger linger = {0};
    int optlen;
    int i;
    char str[BUFFER_SIZE] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        sock = aic_at_find_socket_by_linkid(i, AT_TCPIP);
    }

    if (sock != 0) {
        optlen = sizeof(linger);
        anet_getsockopt(sock, SOL_SOCKET, SO_LINGER, (VOID *)&linger,
                        &optlen);

        optlen = sizeof(nodelay);
        anet_getsockopt(sock, IPPROTO_IP, TCP_NODELAY, (VOID *)&nodelay,
                        &optlen);

        optlen = sizeof(sndtimeo);
        anet_getsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (VOID *)&sndtimeo,
                        &optlen);
    }

    if ( aic_tcpip_at_cfg.cipmux == 1 ) {
       link_id = aic_at_find_linkid_by_socket(sock, AT_TCPIP);
       snprintf(str, sizeof(str), "+CIPTCPOPT: %d,%d,%d,%ld", link_id,
                linger.l_linger, nodelay, (sndtimeo.tv_usec * 1000));
    } else {
       snprintf(str, sizeof(str), "+CIPTCPOPT: %d,%d,%ld", linger.l_linger,
                nodelay, (sndtimeo.tv_usec * 1000));
    }

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipack_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index, sock, i;
    u32 sendlen, acklen, uacklen;
    NX_TCP_SOCKET *tcp_sock;
    NX_IP *client_ip;
    char str[BUFFER_SIZE] = {'\0'};

    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

    index = aic_get_wan_net_by_cid(ANET_DEFAULT_PDP_CID);
    if (index == ANET_PDP_MAX_NUM) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    client_ip = wan_net_list[index].wan_ip_instance;
    if (!client_ip) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    tcp_sock = client_ip->nx_ip_tcp_created_sockets_ptr;
    if (!tcp_sock) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }
    while (tcp_sock) {
        sock = (int)tcp_sock->nx_tcp_socket_reserved_ptr +
                     NX_BSD_SOCKFD_START;
        i = aic_at_find_index_by_socket(sock, AT_TCPIP);
        if (i < 0) {
            goto NEXT;
        }

        sendlen = tcp_sock->nx_tcp_socket_bytes_sent;
        uacklen = tcp_sock->nx_tcp_socket_tx_outstanding_bytes;
        acklen = sendlen - uacklen;

        sprintf(str, "+CIPACK:%d,%d,%d,%d", i, sendlen, acklen, uacklen);
        aic_transmit_resp_normal(at_req_resp_ptr, str);

NEXT:
        tcp_sock = tcp_sock->nx_tcp_socket_created_next;
        if (client_ip->nx_ip_tcp_created_sockets_ptr == tcp_sock) {
            break;
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_keepalive_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt, link_id = 0;
    ulong *paras_arr;
    int index, sock;
    struct sock_keepalive keepalive = {0};

    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+KEEPALIVE=",
                                                 &cnt);
        index = 1;
        if (cnt < 2 || cnt > 5) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    } else {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+KEEPALIVE=",
                                                 &cnt);
        index = 0;
        if (cnt < 1 || cnt > 4) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    }

    if ( !paras_arr ) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1) {
        if (strlen((u8 *)paras_arr[0]))
            link_id = atoi((u8 *)paras_arr[0]);

        if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }
        cnt--;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
    if (!sock) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    if (strlen((u8 *)paras_arr[index]))
        keepalive.keepalive_enabled = atoi((u8 *)paras_arr[index]);

    if (keepalive.keepalive_enabled != 0 && keepalive.keepalive_enabled != 1) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (cnt >= 2) {
        if (strlen((u8 *)paras_arr[index + 1]))
            keepalive.keepalive_init = atoi((u8 *)paras_arr[index + 1]);

        if (cnt >= 3) {
            if (strlen((u8 *)paras_arr[index + 2]))
                keepalive.keepalive_retries = atoi((u8 *)paras_arr[index + 2]);

            if (cnt >= 4) {
                if (strlen((u8 *)paras_arr[index + 3]))
                    keepalive.keepalive_retry = atoi((u8 *)paras_arr[index + 3]);
            }
        }
    }

    anet_setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
                    (const VOID *)&keepalive, sizeof(keepalive));

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_send_keepalive_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int link_id, enable;
    u8 data[60] = {
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
        0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
        0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
        0x77, 0x78, 0x79, 0x7a, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38
    };

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+SENDDATA=",
                                             &cnt);
    if (!paras_arr || cnt < 1 || cnt > 6) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    enable = atoi((u8 *)paras_arr[0]);
    if (enable == 0) {
        if (aic_tcpip_at_cfg.period_sockid != 0) {
            //aic_at_tcpip_task_join(aic_tcpip_at_cfg.period_pthread);
            aic_tcpip_at_cfg.period_cnt = 0;
            aos_free(aic_tcpip_at_cfg.period_send_buff);
            aic_tcpip_at_cfg.period_sockid = 0;
        }

        aic_reply_ok_resp(at_req_resp_ptr);
        return AIC_TCPIP_AT_OK;
    }

    link_id = atoi((u8 *)paras_arr[1]);
    if (link_id >= AT_SOCKET_MAX_CONN_NUM || link_id < -1) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.period_sockid) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    aic_tcpip_at_cfg.period_sockid = sock_list[link_id].socket_id;

    if (cnt > 2) {
        aic_tcpip_at_cfg.period = atoi((u8 *)paras_arr[2]) * NX_IP_PERIODIC_RATE;
        aic_tcpip_at_cfg.period_cnt = atoi((u8 *)paras_arr[3]);
        aic_tcpip_at_cfg.period_send_size = atoi((u8 *)paras_arr[4]);
        aic_tcpip_at_cfg.period_send_buff = aos_zalloc_relaxed(aic_tcpip_at_cfg.period_send_size + 1);
        aos_memcpy(aic_tcpip_at_cfg.period_send_buff, (u8 *)paras_arr[5],
                   aic_tcpip_at_cfg.period_send_size);
    } else {
        aic_tcpip_at_cfg.period = 10 * 60 * NX_IP_PERIODIC_RATE;
        aic_tcpip_at_cfg.period_cnt = 0x7FFFFFFF;
        aic_tcpip_at_cfg.period_send_size = 60;
        aic_tcpip_at_cfg.period_send_buff = aos_zalloc_relaxed(60);
        aos_memcpy(aic_tcpip_at_cfg.period_send_buff, data, 60);
    }

    aos_set_event(at_socket_send_event, SOCKET_SEND_EVENT, TX_OR);
    /*aic_tcpip_at_cfg.period_pthread = aic_at_tcpip_task_create("at_tcpip",
                                                               aic_at_tcpip_process_period_data,
                                                               &aic_tcpip_at_cfg);*/

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_exit_exec_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aicx_info(0x07f7f5, "s", "[AT_TCPIP]: --%s\n", at_req_resp_ptr->at_req);

     if (*(at_req_resp_ptr->at_req + 8) != 0x0D &&
        *(at_req_resp_ptr->at_req + 8) != 0x0A) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_CMD);
        return AIC_TCPIP_ERR_INVAL_AT_CMD;
    }

    aic_tcpip_at_cfg.cipmode = 0;
    at_req_resp_ptr->at_mode = AT_CMD_MODE;
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

static void handle_select_event(int index, fd_set *readfds, fd_set *writefds,
                                fd_set *exceptfds, u8 *buffer, u8 *result_str)
{
    int ret, fromaddrlen;
    struct sockaddr_in fromaddr = {0};
    struct sockaddr_in6 fromaddr6 = {0};
    char addr[ADDR_BUFFERSIZE_IPV6];
    u16 port, len;

    if (sock_list[index].socket_id == 0)
        return;

    if (FD_ISSET(sock_list[index].socket_id, exceptfds)) {
#ifdef CONFIG_NET_AT_TLS
        if (sock_list[index].socket_type == SOCK_TLS ||
            sock_list[index].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
            aic_tls_ca_certificate_remove_all(sock_list[index].socket_id);
            aic_close_tls_socket(sock_list[index].socket_id);
            if (sock_list[index].common_name)
                aos_free(sock_list[index].common_name);

            if (sock_list[index].ca_cert_der)
                aos_free(sock_list[index].ca_cert_der);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            aic_at_close_mbedtls_socket(sock_list[index].socket_id);
#endif
        } else
#endif
        {
            soc_close(sock_list[index].socket_id);
        }
        aos_memset(&sock_list[index], 0, sizeof(struct SOCKET_LIST));
        snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                 "+CIPCLOSED: %d,CLOSED", sock_list[0].link_id);
        aic_transmit_at_cmd_urc(aic_tcpip_at_cfg.com_id,
                                result_str, strlen(result_str));
        return;
    }

    if (FD_ISSET(sock_list[index].socket_id, readfds)) {
#ifdef CONFIG_NET_AT_TLS
        if (sock_list[0].socket_type == SOCK_TLS ||
            sock_list[0].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
            ret = anet_recv(sock_list[index].socket_id, buffer,
                            AT_RECV_DATA_BUFFER, MSG_DONTWAIT);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
            ret = mbedtls_ssl_read((mbedtls_ssl_context *)sock_list[0].ssl,
                                   buffer, AT_RECV_DATA_BUFFER);
#endif
        } else
#endif
        if (sock_list[0].socket_type == SOCK_TCP ||
            sock_list[0].socket_type == SOCK_TCPv6) {
            ret = anet_recv(sock_list[index].socket_id, buffer,
                            AT_RECV_DATA_BUFFER, MSG_DONTWAIT);
        } else {
            if (sock_list[0].family == AF_INET) {
                fromaddrlen = sizeof(struct sockaddr_in);
                ret = anet_recvfrom(sock_list[index].socket_id, buffer,
                                    AT_RECV_DATA_BUFFER, MSG_DONTWAIT,
                                    (struct sockaddr *)&fromaddr,
                                    &fromaddrlen);
            } else {
                fromaddrlen = sizeof(struct sockaddr_in6);
                ret = anet_recvfrom(sock_list[index].socket_id, buffer,
                                    AT_RECV_DATA_BUFFER, MSG_DONTWAIT,
                                    (struct sockaddr *)&fromaddr6,
                                    &fromaddrlen);
            }
        }
        if (ret > 0) {
            if (aic_tcpip_at_cfg.ciphead == 1) {
                if (aic_tcpip_at_cfg.cipinfo == 1) {
                    if (sock_list[index].family == AF_INET) {
                        if (sock_list[index].socket_type != SOCK_UDP) {
                            fromaddrlen = sizeof(struct sockaddr_in);
                            anet_getpeername(sock_list[index].socket_id,
                                             (struct sockaddr *)&fromaddr,
                                             &fromaddrlen);
                        }

                        inet_ntop(AF_INET, &fromaddr.sin_addr.s_addr,
                                  addr, ADDR_BUFFERSIZE_IPV4);
                        port = htons(fromaddr.sin_port);
                    } else {
                        if (sock_list[index].socket_type != SOCK_UDPv6) {
                            fromaddrlen = sizeof(struct sockaddr_in6);
                            anet_getpeername(sock_list[index].socket_id,
                                             (struct sockaddr *)&fromaddr6,
                                             &fromaddrlen);
                        }

                        inet_ntop(AF_INET6, &fromaddr6.sin6_addr,
                                  addr, ADDR_BUFFERSIZE_IPV6);
                        port = htons(fromaddr6.sin6_port);
                    }
                    if (aic_tcpip_at_cfg.cipmux == 0)
                        snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                                 "+CIPRECV: +IPD,%d,%s,%d,", ret, addr, port);
                    else
                        snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                                 "+CIPRECV: +IPD,%d,%d,%s,%d,", index, ret,
                                 addr, port);
                } else {
                    if (aic_tcpip_at_cfg.cipmux == 0)
                        snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                                 "+CIPRECV: +IPD,%d,", ret);
                    else
                        snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                                 "+CIPRECV: +IPD,%d,%d,", index, ret);
                }
            } else {
                if (aic_tcpip_at_cfg.cipmux == 0)
                    snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                             "+CIPRECV: %d,", ret);
                else
                    snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                             "+CIPRECV: %d,%d,", index, ret);
            }

            len = strlen(result_str);
            aos_memcpy((result_str + len), buffer, ret);
            aic_transmit_at_data(aic_tcpip_at_cfg.com_id, result_str, (len + ret));
        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                return;

#ifdef CONFIG_NET_AT_TLS
            if (sock_list[index].socket_type == SOCK_TLS ||
                sock_list[index].socket_type == SOCK_TLSv6) {
#ifdef CONFIG_NET_SECURE
                aic_tls_ca_certificate_remove_all(sock_list[index].socket_id);
                aic_close_tls_socket(sock_list[index].socket_id);
                if (sock_list[index].common_name)
                    aos_free(sock_list[index].common_name);

                if (sock_list[index].ca_cert_der)
                    aos_free(sock_list[index].ca_cert_der);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
                aic_at_close_mbedtls_socket(sock_list[index].socket_id);
#endif
            } else
#endif
            {
                soc_close(sock_list[index].socket_id);
            }
            aos_memset(&sock_list[index], 0, sizeof(struct SOCKET_LIST));
            if (sock_list[index].socket_type != SOCK_UDP &&
                sock_list[index].socket_type != SOCK_UDPv6) {
                snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                         "+CIPCLOSED: %d,CLOSED", sock_list[0].link_id);
                aic_transmit_at_cmd_urc(aic_tcpip_at_cfg.com_id,
                                        result_str, strlen(result_str));
            }
        }
    }
}

#if 1
static void aic_at_tcpip_process_period_data(void *pdata)
{
    u8 *buf;
    int i, sockid, ret;
    struct timeval timeval;
    fd_set readfds, exceptfds;
    //struct tcpip_at_cfg *cfg = (struct tcpip_at_cfg *)pdata;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    u32 events = 0;

    while (1) {
        ret = aos_get_event(at_socket_send_event, SOCKET_SEND_EVENT, TX_OR_CLEAR,
                            &events, AOS_WAIT_FOREVER);
        if (ret != NX_SUCCESS)
            continue;

        buf = aic_tcpip_at_cfg.period_send_buff;
        sockid = aic_tcpip_at_cfg.period_sockid;

        for (i = 0; i < aic_tcpip_at_cfg.period_cnt; i++) {
            ret = send(sockid, buf, aic_tcpip_at_cfg.period_send_size, 0);
            if (ret) {
                aicx_warn(0x06db3b, "x", "[AT_TCPIP] send period data fail, errno 0x%x\n", errno);
                if (errno == ENOTCONN)
                    break;
            }

            FD_ZERO(&readfds);
            FD_ZERO(&exceptfds);
            FD_SET(sockid, &readfds);
            FD_SET(sockid, &exceptfds);
            timeval.tv_sec = 5;
            timeval.tv_usec = 0;
            ret = select(sockid + 1, &readfds, NULL, &exceptfds, &timeval);
            if (ret <= 0) {
                aicx_warn(0x145f53, "dx", "[AT_TCPIP] wait recv data fail %d errno 0x%x\n",
                          ret, errno);
            } else {
                if (FD_ISSET(sockid, &exceptfds)) {
                    aicx_warn(0x0e5100, "", "[AT_TCPIP] socket have exceptfds\n");
                    break;
                }

                ret = anet_recv(sockid, buf, aic_tcpip_at_cfg.period_send_size, 0);
                aicx_info(0x1c8ab3, "d", "[AT_TCPIP] recv data len %d\n", ret);
                snprintf(str, AT_RESP_RESULT_MAX, "+RECVRESP: %d", ret);
                aic_transmit_at_cmd_urc(aic_tcpip_at_cfg.com_id, str, strlen(str));
            }

            aos_sleep(aic_tcpip_at_cfg.period);
        }
    }
}

int aic_at_tcpip_task_create(void)
{
    at_socket_send_thread = aos_create_thread("AT TCPIP SEND Thread",
                                       aic_at_tcpip_process_period_data,
                                       NULL, SIZE_2K, 0, 0, 15, AOS_DONT_START);
    if (at_socket_send_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x0e5fb9, "", "[AT_TCPIP]: at socket send thread create fail\n");
        return AIC_EFAULT;
    }

    at_socket_send_event = aos_create_event("AT Socket recv event flags");
    if (!at_socket_send_event) {
        aicx_error(0x06d95d, "", "[AT_TCPIP]: at socket send event create fail\n");
        aos_delete_thread(at_socket_send_thread);
        return AIC_EFAULT;
    }

    aos_resume_thread(at_socket_send_thread);
    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_at_tcpip_task_create);

#if 0
void *aic_at_tcpip_task_create(char *name, void (*fn)(void *), void *arg)
{
    pthread_t       *pthread;
    pthread_attr_t  *ptattr;
    void *stack_ptr;
    struct sched_param param;

    pthread = aos_malloc_relaxed(sizeof(pthread_t) + sizeof(pthread_attr_t) +
                                 SIZE_2K);
    if (!pthread) {
        aicx_error(0x0ad698, "", "[AT_TCPIP] ERROR: no enough memory\n");
        return NULL;
    }

    ptattr = (pthread_attr_t *)((char *)pthread + sizeof(*pthread));
    stack_ptr = (void *)((char *)ptattr + sizeof(*ptattr));
    pthread_attr_init(ptattr);

    memset(&param, 0, sizeof(param));
    param.sched_priority = 20;
    pthread_attr_setschedparam(ptattr, &param);
    pthread_attr_setstackaddr(ptattr, stack_ptr);
    pthread_attr_setstacksize(ptattr, SIZE_2K);
    pthread_attr_setcanceltype(ptattr, PTHREAD_CANCEL_ASYNCHRONOUS);
    if (pthread_create(pthread, ptattr, (void *(*)(void*))fn, arg)) {
        aicx_error(0x1549a0, "", "[AT_TCPIP] thread create failed\n");
        return NULL;
    }

    aicx_error(0x0827a8, "pp", "[AT_TCPIP] thread=%p, thread_id=%p\n", pthread, *pthread);

    return (void *)(pthread);
}

void aic_at_tcpip_task_join(void *task)
{
    aicx_info(0x043f98, "p", "[AT_TCPIP] thread join, %p\n", task);
    if (task) {
        pthread_cancel(*((pthread_t *)task));
        aos_free(task);
    }
}
#endif
#endif
void aic_at_socket_recv_thread_entry(void* thread_input)
{
    int sock, i, ret;
    char *buffer;
    u8 *result_str;
    fd_set master_list, read_ready, except_ready;

    buffer = aos_malloc(AT_RECV_DATA_BUFFER);
    if (!buffer) {
        return;
    }

    result_str = aos_malloc(AT_RESP_RECV_DATA_BUFFER);
    if (!result_str) {
        return;
    }

    while (1) {
        FD_ZERO(&read_ready);
        FD_ZERO(&except_ready);
        FD_ZERO(&master_list);
        aos_memset(buffer, '\0', AT_RECV_DATA_BUFFER);
        aos_memset(result_str, '\0', AT_RESP_RECV_DATA_BUFFER);
        if (aic_tcpip_at_cfg.cipmux == 0) {
            sock = sock_list[0].socket_id;
            if (sock == 0) {
                aicx_info(0x1230ef, "", "[tcpip] no socket in used, suspend\n");
                aos_suspend_thread(at_socket_recv_thread);
            }

            sock = sock_list[0].socket_id;
            if (sock == 0)
                continue;

            FD_SET(sock, &master_list);
            read_ready = master_list;
            except_ready = master_list;
            /*timeout is NULL, wait forever*/
            ret = select(FD_SETSIZE, &read_ready, 0, &except_ready, NX_NULL);
            if (ret == ERROR || ret == 0) {
                continue;
            }

            handle_select_event(0, &read_ready, NULL, &except_ready,
                                buffer, result_str);
        } else {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                sock = sock_list[i].socket_id;
                if ((i == AT_SOCKET_MAX_CONN_NUM - 1) && sock == 0 &&
                    !master_list.fd_count) {
                    aicx_info(0x1a16fe, "", "[tcpip] at socket recv thread is suspend\n");
                    aos_suspend_thread(at_socket_recv_thread);
                }

                if (i == AT_SOCKET_MAX_CONN_NUM - 1 && sock == 0 &&
                    !master_list.fd_count) {
                    i = -1;
                    continue;
                }

                if (sock == 0)
                    continue;

                FD_SET(sock, &master_list);
            }

            read_ready = master_list;
            except_ready = master_list;
            ret = select(FD_SETSIZE, &read_ready, 0, &except_ready, NX_NULL);
            if (ret == ERROR || ret == 0) {
                continue;
            }

            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                sock = sock_list[i].socket_id;

                if (sock == 0)
                    continue;

                handle_select_event(i, &read_ready, NULL, &except_ready,
                                    buffer, result_str);
            }
        }
    }
    aos_free(buffer);
    aos_free(result_str);
}

int aic_at_socket_recv_init()
{
    at_socket_recv_thread = aos_create_thread("AT Socket recv Thread",
                                aic_at_socket_recv_thread_entry,
                                NULL, SIZE_4K,
                                0, 0, 15,
                                AOS_DONT_START);
    if (at_socket_recv_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x1971e6, "", "[AT_TCPIP]: at socket thread create fail\n");
        return EFAULT;
    }

    at_socket_recv_event = aos_create_event("AT Socket recv event flags");
    if (!at_socket_recv_event) {
        aicx_error(0x1b17a1, "", "[AT_TCPIP]: at socket event create fail\n");
        aos_delete_thread(at_socket_recv_thread);
        return EFAULT;
    }

    return AIC_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_at_socket_recv_init);

void aic_at_socket_listen_thread_entry(void* thread_input)
{
    int sock, conn_sock, i, ret, fromaddr_len;
    struct sockaddr_in fromaddr;
    struct sockaddr_in6 fromaddr6;
    fd_set read_ready;

    if (aic_tcpip_at_cfg.cipserver == 0)
        aos_suspend_thread(at_socket_listen_thread);

    FD_ZERO(&read_ready);
    //FD_SET(sock, &read_ready);

    ret = listen(aic_at_tcp_server.sockfd, aic_tcpip_at_cfg.server_max_count);
    if (ret < 0) {
        aicx_error(0x097010, "", "[AT_TCPIP]: at socket listen fail\n");
        return;
    }

    while (1) {
        read_ready = aic_at_tcp_server.master_list;
        sock = aic_at_tcp_server.sockfd;
        aos_sleep(20);
        ret = select(FD_SETSIZE, &read_ready, 0, 0, NX_NULL);
        if (ret == ERROR || ret == 0) {
            continue;
        }

        if (FD_ISSET(sock, &read_ready)) {
            if (aic_at_tcp_server.family == AF_INET) {
                fromaddr_len = sizeof(fromaddr);
                conn_sock = accept(sock, (struct sockaddr *)&fromaddr,
                                   &fromaddr_len);
            } else {
                fromaddr_len = sizeof(fromaddr6);
                conn_sock = accept(sock, (struct sockaddr *)&fromaddr6,
                                   &fromaddr_len);
            }

            if (conn_sock > 0) {
                if (aic_tcpip_at_cfg.ciprecvmode == 0)
                    aos_resume_thread(at_socket_recv_thread);
            }

            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                if (sock_list[i].socket_id == 0) {
                    if(aic_tcpip_at_cfg.cipmux == 0 ){
                        sock_list[i].link_id = 0;
                    } else {
                        sock_list[i].link_id = i;
                    }
                    sock_list[i].socket_id = conn_sock;
                    sock_list[i].family = aic_at_tcp_server.family;
                    if (aic_at_tcp_server.family == AF_INET)
                        sock_list[i].socket_type = SOCK_TCP;
                    else
                        sock_list[i].socket_type = SOCK_TCPv6;
                    break;
                }
            }
        }
    }
}

static void aic_at_socket_listen_init(void)
{
    at_socket_listen_thread = aos_create_thread("AT Socket listen Thread",
                                  aic_at_socket_listen_thread_entry,
                                  NULL, SIZE_2K,
                                  0, 0, 15,
                                  AOS_DONT_START);
    if (at_socket_listen_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x1971e6, "", "[AT_TCPIP]: at socket thread create fail\n");
        return;
    }

    at_socket_listen_event = aos_create_event("AT Socket listen event flags");
    if (!at_socket_listen_event) {
        aicx_error(0x1b17a1, "", "[AT_TCPIP]: at socket event create fail\n");
        aos_delete_thread(at_socket_listen_thread);
        return;
    }

    aos_resume_thread(at_socket_listen_thread);
}

static void aic_at_socket_listen_uninit(void)
{
    aos_delete_thread(at_socket_listen_thread);
    aos_delete_event(at_socket_listen_event);
}

#endif

