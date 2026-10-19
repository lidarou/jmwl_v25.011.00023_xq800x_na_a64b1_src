/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
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
#define AIC_THIS_FILE    0x179260
#define LOG_MODULE    AMOD_NET

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <aic_size.h>
#include <nx_api.h>
#include <nxd_bsd.h>
#include <nxd_dns.h>
#ifdef CONFIG_NET_SNTP
#include <nxd_sntp_client.h>
#endif
#include <nx_ip.h>
#include <aic_errno.h>
#include <aic_init.h>
#include "aic_at_tcpip_common.h"
#include "aic_at_qtcpip.h"
#ifdef CONFIG_CUSTOMER_AT
#include "aic_at_qtls.h"
#endif
#include "nx_icmpv6.h"
#include "nx_icmpv4.h"
#include "aic_net_api.h"
#include "aic_at_parse_utils.h"
#include "aic_at_resp_handle.h"
#include "aic_at_cmd_handle.h"
#include "aic_net_pdp_mgr.h"

struct udp_data_info {
    char *data;
    u32 recv_len;
    u32 read_len;
};

AIC_SOCKET_LIST sock_list_q[AT_SOCKET_MAX_CONN_NUM_Q];
static AIC_AT_TCP_SERVER aic_at_tcp_server;
AT_TCPIP_CFG at_tcpip_cfg;
struct at_sock_info sock_info = {0};
extern u32 last_errcode;
static u8 recv_cnt;
static aos_thread_id at_socket_recv_thread;
static aos_event_ptr at_socket_recv_event;
static aos_thread_id at_socket_listen_thread;
static aos_event_ptr at_socket_listen_event;
static struct udp_data_info udp_data[AT_SOCKET_MAX_CONN_NUM_Q] = {0};
static AT_EVENT_FLAG at_event_flag[AT_SOCKET_MAX_CONN_NUM_Q];
static aos_mutex_ptr at_socket_mutex = NULL;

extern int sntp_is_sync;
extern NX_DNS *_nx_dns_instance_ptr;
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];
extern u32 sntp_server_ip;
extern NXD_ADDRESS sntp_server_address;
extern TX_EVENT_FLAGS_GROUP nx_sntp_client_events;
extern NX_BSD_SOCKET nx_bsd_socket_array[NX_BSD_MAX_SOCKETS];
extern AIC_SNTP_SERVER_IP *aic_sntp_server_ip_ptr;

static struct at_error_description at_error_description_table[] = {
    {AIC_TCPIP_AT_OK, "operate successfully"},
    {AIC_TCPIP_ERR_PARSE_AT_PARAM, "parameter parsing failed"},
    {AIC_TCPIP_ERR_INVAL_AT_PARAM, "invalid parameters"},
    {AIC_TCPIP_ERR_INVAL_AT_CMD, "invalid at command"},
    {AIC_TCPIP_ERR_AT_CMD_NOT_SUPPORT, "operation not support"},
    {AIC_TCPIP_ERR_PDP_NOT_ACTIVE, "PDP is not activated"},
    {AIC_TCPIP_ERR_SOCKET_NOT_EXIST, "the socket is not exist"},
    {AIC_TCPIP_ERR_SOCKET_CREATE_FAIL, "create socket failed"},
    {AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT, "socket already connected"},
    {AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL, "socket connect failed"},
    {AIC_TCPIP_ERR_SOCKET_RECV_FAIL, "socket recv failed"},
    {AIC_TCPIP_ERR_DNS_FAIL, "DNS parse failed"},
    {AIC_TCPIP_ERR_NO_IPV6_PREFIX, "No ipv6 address prefix"},
    {AIC_TCPIP_ERR_SOCKET_BIND_FAIL, "socket bind failed"},
    {AIC_TCPIP_ERR_ENTER_DATA_MODE_FAIL, "Switch data mode fail"},
    {AIC_TCPIP_ERR_MAX},
};

AT_CMD_HANDLE qtcpip_at_cmd[] = {
    {"AT+QIOPEN", handle_qiopen_test_cmd, handle_qiopen_query_cmd,
     handle_qiopen_set_cmd, AIC_NULL},

    {"AT+QICLOSE", handle_qiclose_test_cmd, AIC_NULL, handle_qiclose_set_cmd,
     AIC_NULL},

    {"AT+QISTATE", handle_qistate_test_cmd, handle_qistate_query_cmd,
     handle_qistate_set_cmd, AIC_NULL},

    {"AT+QISEND", handle_qisend_test_cmd, AIC_NULL, handle_qisend_set_cmd,
     AIC_NULL},

    {"AT+QIRD", handle_qird_test_cmd, AIC_NULL, handle_qird_set_cmd, AIC_NULL},

    {"AT+QISENDEX", handle_qisendex_test_cmd, AIC_NULL, handle_qisendex_set_cmd,
     AIC_NULL},

    {"AT+QISWTMD", handle_qiswtmd_test_cmd, handle_qistwmd_query_cmd,
     handle_qistwmd_set_cmd, AIC_NULL},

    {"AT+QICFG", handle_qicfg_test_cmd, AIC_NULL, handle_qicfg_set_cmd,
     AIC_NULL},

    {"AT+QISDE", handle_qisde_test_cmd, handle_qisde_query_cmd,
     handle_qisde_set_cmd, AIC_NULL},

    {"AT+QIDNSCFG", handle_qidnscfg_test_cmd, AIC_NULL, handle_qidnscfg_set_cmd,
     AIC_NULL},

    {"AT+QIDNSGIP", handle_qidnsgip_test_cmd, AIC_NULL,handle_qidnsgip_set_cmd,
     AIC_NULL},

    {"AT+QPING", handle_qping_test_cmd, AIC_NULL, handle_qping_set_cmd, AIC_NULL},

    {"AT+QNTP", handle_qntp_test_cmd, handle_qntp_query_cmd,
     handle_qntp_set_cmd, AIC_NULL},

    {"AT+QIGETERROR", handle_qigeterror_test_cmd, AIC_NULL, AIC_NULL,
     handle_qigeterror_exec_cmd},

    {AIC_NULL},
};

static void aic_tcpip_qat_cfg_init(void);
static void aic_qat_socket_listen_init(void);
static void aic_qat_socket_listen_uninit(void);
static void aic_qat_socket_recv_notify(int sock, void *data);
static void aic_qat_socket_tcp_disconn_notify(int sock, void *data, int is_active);

extern NX_SNTP_CLIENT *aic_get_sntp_client(void);

int aic_tcpip_qat_cmd_init(void)
{
    aic_add_at_cmd_handle(qtcpip_at_cmd);
    aic_tcpip_qat_cfg_init();

    return AIC_TCPIP_AT_OK;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_tcpip_qat_cmd_init);

static void aic_tcpip_qat_cfg_init(void)
{
    memset(sock_list_q, 0, sizeof(sock_list_q));
    memset(&at_tcpip_cfg, 0, sizeof(at_tcpip_cfg));
    at_tcpip_cfg.transpktsize = 1024;
    at_tcpip_cfg.transwaittm = 2;
    at_tcpip_cfg.max_backoffs = 8;
    at_tcpip_cfg.max_rto = 600;
    at_tcpip_cfg.cycle_time = 19;
    at_tcpip_cfg.sendecho = 1;

    at_socket_mutex = aos_create_mutex("at_socket_mutex");
    AIC_ASSERT(at_socket_mutex);
}

static int aic_create_qtcp_socket(int family, struct sockaddr *addr,
                                  int linkid, struct sockaddr *srcaddr,
                                  int isclient)
{
    int i, sock, addrlen, ret, len;
    struct sock_errno so_error;
    struct timeval timeout;
    fd_set master_list, write_ready;

    sock = anet_socket(addr->sa_family, SOCK_STREAM, NX_PROTOCOL_TCP);
    if (sock == NX_SOC_ERROR) {
        aicx_error(0x1a4b87, "d", "[AT_TCP]: --create tls socket error, errno %d\n", errno);
        return AIC_EFAULT;
    }

    if (family == AF_INET)
        addrlen = sizeof(struct sockaddr_in);
    else
        addrlen = sizeof(struct sockaddr_in6);

    if (srcaddr) {
        ret = anet_bind(sock, srcaddr, addrlen);
        if (ret < 0) {
            aicx_info(0x184e9d, "d", "[AT_TCPIP] socket bind fail error %d\n", errno);
            soc_close(sock);
            return ret;
        }
    }

    timeout.tv_sec = AT_SOCKET_SEND_TIMEOUT;
    timeout.tv_usec = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret)
        aicx_error(0x0ff24c, "x", "[AT_TCP] set tcp send timeout fail 0x%x\r\n", ret);

    if (isclient == 0) {
        anet_bind(sock, addr, addrlen);
        aic_at_tcp_server.sockfd = sock;
        aic_at_tcp_server.family = addr->sa_family;
        aic_qat_socket_listen_init();
        FD_SET(sock, &aic_at_tcp_server.master_list);
    } else {
        ret = anet_fcntl(sock, F_SETFL, O_NONBLOCK);
        if (ret == NX_SOC_ERROR) {
            soc_close(sock);
            return AIC_EFAULT;
        }

        ret = anet_connect(sock, addr, addrlen);
        if (ret < 0) {
            if (errno != EINPROGRESS && errno != EALREADY) {
                soc_close(sock);
                return ret;
            }
        }

        FD_ZERO(&write_ready);
        FD_ZERO(&master_list);
        FD_SET(sock, &master_list);
        write_ready = master_list;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        ret = anet_select(FD_SETSIZE, NULL, &write_ready, NULL, &timeout);
        if (ret == ERROR || ret == 0) {
            aicx_info(0x08ef28, "d", "[AT_TCPIP] socket connect fail error %d\n", errno);
            soc_close(sock);
            return AIC_EFAULT;
        } else {
            len = sizeof(struct sock_errno);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
            if (so_error.error != 0) {
                aicx_info(0x08ef28, "d", "[AT_TCPIP] socket connect fail error %d\n",
                          so_error.error);
                soc_close(sock);
                return AIC_EFAULT;
            }
        }
    }
    aic_socket_register_receive_notify(sock, aic_qat_socket_recv_notify, NULL);
    aic_socket_register_tcp_disconnect_notify(sock,
                                              aic_qat_socket_tcp_disconn_notify,
                                              NULL);

    i = linkid;
    if (sock_list_q[i].socket_id == 0) {
        sock_list_q[i].link_id = linkid;
        sock_list_q[i].socket_id = sock;
        sock_list_q[i].family = addr->sa_family;
        sock_list_q[i].in_use = TRUE;
        if (addr->sa_family == AF_INET) {
            sock_list_q[i].addr =
                ((struct sockaddr_in *)addr)->sin_addr.s_addr;
            sock_list_q[i].port =
                ((struct sockaddr_in *)addr)->sin_port;
        } else {
            aos_memcpy(sock_list_q[i].addr6,
                       &((struct sockaddr_in6 *)addr)->sin6_addr,
                       16);
            sock_list_q[i].port = ((struct sockaddr_in6 *)addr)->sin6_port;
        }
        if (isclient)
            sock_list_q[i].socket_type = SOCK_TCP;
        else
            sock_list_q[i].socket_type = SOCK_TCPS;
    }

    return sock;
}

static int aic_create_qudp_socket(int family, struct sockaddr *remoteaddr,
                                 int linkid, struct sockaddr *srcaddr,
                                 int isclient)
{
    int i, sock, ret, addrlen;
    struct timeval timeout;

    sock = anet_socket(remoteaddr->sa_family, SOCK_DGRAM, NX_PROTOCOL_UDP);
    if (sock == NX_SOC_ERROR) {
        aicx_error(0x1c07a0, "d", "[AT_UDP]: --create udp socket error, errno %d\n", errno);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (family == AF_INET)
        addrlen = sizeof(struct sockaddr_in);
    else
        addrlen = sizeof(struct sockaddr_in6);

    if (srcaddr) {
        ret = anet_bind(sock, srcaddr, addrlen);
        if (ret < 0) {
            aicx_info(0x184e9d, "d", "[AT_TCPIP] socket bind fail error %d\n", errno);
            return ret;
        }
    }

    timeout.tv_sec = AT_SOCKET_SEND_TIMEOUT;
    timeout.tv_usec = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret)
        aicx_error(0x09724e, "x", "[AT_UDP] set udp send timeout fail 0x%x\r\n", ret);

    aic_socket_register_receive_notify(sock, aic_qat_socket_recv_notify, NULL);

    i = linkid;
    if (sock_list_q[i].socket_id == 0) {
        sock_list_q[i].link_id = linkid;
        sock_list_q[i].socket_id = sock;
        sock_list_q[i].family = remoteaddr->sa_family;
        if (family == AF_INET) {
            sock_list_q[i].addr =
                ((struct sockaddr_in *)remoteaddr)->sin_addr.s_addr;
            sock_list_q[i].port =
                ((struct sockaddr_in *)remoteaddr)->sin_port;
        } else {
            aos_memcpy(sock_list_q[i].addr6,
                       &((struct sockaddr_in6 *)remoteaddr)->sin6_addr,
                       16);
            sock_list_q[i].port =
                ((struct sockaddr_in6 *)remoteaddr)->sin6_port;
        }
        if (isclient)
            sock_list_q[i].socket_type = SOCK_UDP;
        else
            sock_list_q[i].socket_type = SOCK_UDPS;
    }

    return sock;
}

int aic_qudp_socket_send(int socket_id, char *data, int datalen)
{
    struct sockaddr_in destaddr;
    struct sockaddr_in6 destaddr6;
    int index, ret;

    index = aic_at_find_index_by_socket(socket_id, AT_QTCPIP);
    if (index < 0) {
        return AIC_EINVAL;
    }

    if (sock_list_q[index].family == AF_INET) {
        destaddr.sin_family = AF_INET;
        destaddr.sin_port = sock_list_q[index].port;
        destaddr.sin_addr.s_addr = sock_list_q[index].addr;
        ret = anet_sendto(socket_id, data, datalen, 0,
                          (struct sockaddr *)&destaddr,
                          sizeof(destaddr));
    } else {
        destaddr6.sin6_family = AF_INET6;
        destaddr6.sin6_port = sock_list_q[index].port;
        aos_memcpy(destaddr6.sin6_addr._S6_un._S6_u32,
                   sock_list_q[index].addr6, 16);
        ret = anet_sendto(socket_id, data, datalen, 0,
                          (struct sockaddr *)&destaddr6,
                          sizeof(destaddr6));
    }

    return ret;
}

u32 handle_qiopen_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str),
        "+QIOPEN: (1-5),(0-11),\"TCP/UDP/TCP LISTENER/UDP SERVICE\",\"<IP_address>/<domain_name>\",<remote_port>,<local_port>,(0-2)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qiopen_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_qiopen_set_cmd() - Establish TCP/UDP connection.
 * @para_num: parameter.
 *
 * CALLS:
 *    socket
 *
 * CALLED BY:
 *    Application
 *
 * Establish TCP/UDP connection.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 *
 */
u32 handle_qiopen_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;
    u32 len, mode;
    int linkid = 0, protocolFamily, sock, index = 0, ret, cid;
    struct sockaddr_in remoteaddr, localaddr;
    struct sockaddr_in6 remoteaddr6, localaddr6;
    struct sockaddr *peeraddr, *srcaddr = NULL;
    struct addrinfo *res = NULL;
    WAN_NET *net_ptr;
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+QIOPEN=",
                                                 &cnt);
        if (!paras_arr || cnt < 5 || cnt > 7) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            if (paras_arr)
                aic_free_paras_mem(paras_arr, cnt);

            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }

        if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            linkid = atoi((u8 *)paras_arr[1]);
        else
            goto ERROR_EXIT;

        if (linkid >= AT_SOCKET_MAX_CONN_NUM_Q || linkid < 0) {
            aicx_error(0x09acca, "", "[AT_TCPIP]: socket link id error\n");
            goto ERROR_EXIT;
        }

        index = linkid;
        if (sock_list_q[index].socket_id || sock_list_q[index].in_use) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT;
        }

        if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
            cid = atoi((u8 *)paras_arr[0]);
            if (cid > 0 && cid <= ANET_MAX_PDP_CID) {
                sock_list_q[index].pdp_cid = cid;
            } else {
                goto ERROR_EXIT;
            }
        } else {
            goto ERROR_EXIT;
        }

        ret = getaddrinfo((u8 *)paras_arr[3], AIC_NULL, AIC_NULL, &res);
        if (ret != NX_SUCCESS) {
            aicx_error(0x0a036e, "sd", "[AT_TCPIP]: get addr info fail %s ret %d\r\n",
                       (s8 *)paras_arr[3], ret);
            aic_free_paras_mem(paras_arr, cnt);
            aic_transmit_resp_error(at_req_resp_ptr, AIC_TCPIP_ERR_DNS_FAIL);
            return ret;
        }

        if (!aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4])))
            goto ERROR_EXIT;

        cid = atoi((u8 *)paras_arr[4]);
        if (cid < 0 || cid > 65535)
            goto ERROR_EXIT;

        if (res->ai_family == AF_INET) {
            protocolFamily = AF_INET;
            aos_memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
            remoteaddr.sin_family = AF_INET;
            remoteaddr.sin_port = htons(cid);
            remoteaddr.sin_addr.s_addr =
                ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
            peeraddr = (struct sockaddr *)&remoteaddr;
        } else {
            protocolFamily = AF_INET6;
            aos_memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
            remoteaddr6.sin6_family = AF_INET6;
            remoteaddr6.sin6_port = htons(cid);
            aos_memcpy(&remoteaddr6.sin6_addr,
                       &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
            peeraddr = (struct sockaddr *)&remoteaddr6;
        }

        if (cnt >= 6) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[5], strlen((u8 *)paras_arr[5])))
                cid = atoi((u8 *)paras_arr[5]);
            else
                goto ERROR_EXIT;

            if (cid < 0 || cid > 65535)
                goto ERROR_EXIT;
            net_ptr = aic_get_wan_net_per_pdp(sock_list_q[index].pdp_cid);
            if (net_ptr) {
                if (protocolFamily == AF_INET) {
                    localaddr.sin_family = AF_INET;
                    localaddr.sin_port = htons(cid);
                    localaddr.sin_addr.s_addr = htonl(net_ptr->wan_ipv4_address);
                    srcaddr = (struct sockaddr *)&localaddr;
                } else {
                    localaddr6.sin6_family = AF_INET6;
                    localaddr6.sin6_port = htons(cid);
                    aos_memcpy(&localaddr6.sin6_addr,
                               net_ptr->wan_ipv6_address1,
                               16);
                    NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[0]);
                    NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[1]);
                    NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[2]);
                    NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[3]);
                    srcaddr = (struct sockaddr *)&localaddr6;
                }
            } else {
                aos_memset(&sock_list_q[index], 0, sizeof(struct SOCKET_LIST));
                aic_transmit_resp_error(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
                goto OPEN_EXIT;
            }
        }
        if (cnt == 7) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[6], strlen((u8 *)paras_arr[6]))) {
                mode = atoi((u8 *)paras_arr[6]);
                if (mode < 0 || mode > 2)
                    goto ERROR_EXIT;
                else
                    sock_list_q[index].mode = mode;
            } else {
                goto ERROR_EXIT;
            }
        } else {
            sock_list_q[index].mode = 0;
        }

        len = strlen((u8 *)paras_arr[2]);
        if ((len == sizeof("TCP LISTENER") - 1) &&
            (strcasecmp((u8 *)paras_arr[2], "TCP LISTENER") == 0)) {
            sock = aic_create_qtcp_socket(protocolFamily, peeraddr, linkid,
                                          srcaddr, NX_FALSE);
        } else if ((len == sizeof("TCP") - 1) &&
                    (strcasecmp((u8 *)paras_arr[2], "TCP") == 0)) {
            sock = aic_create_qtcp_socket(protocolFamily, peeraddr, linkid,
                                          srcaddr, NX_TRUE);
        } else if ((len == sizeof("UDP SERVICE") - 1) &&
                   (strcasecmp((u8 *)paras_arr[2], "UDP SERVICE") == 0)) {
            sock = aic_create_qudp_socket(protocolFamily, peeraddr, linkid,
                                         srcaddr, NX_FALSE);
        } else if ((len == sizeof("UDP") - 1) &&
                   (strcasecmp((u8 *)paras_arr[2], "UDP") == 0)) {
            sock = aic_create_qudp_socket(protocolFamily, peeraddr, linkid,
                                         srcaddr, NX_TRUE);
        } else {
            aicx_info(0x11194c, "", "Not Support!\n");
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_AT_CMD_NOT_SUPPORT);
             goto OPEN_EXIT;
        }

        if (sock < 0) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL);
            goto OPEN_EXIT;
        }

        at_tcpip_cfg.com_id = at_req_resp_ptr->serial_port_id;
        if (sock_list_q[index].mode == 2) {
            ret = aic_allowed_to_enter_data_mode(at_req_resp_ptr->serial_port_id);
            if (ret == FALSE) {
                aic_transmit_resp_error(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_ENTER_DATA_MODE_FAIL);
                soc_close(sock);
                aos_memset(&sock_list_q[index], 0, sizeof(struct SOCKET_LIST));
                goto OPEN_EXIT;
            }
            at_req_resp_ptr->at_mode = AT_DATA_MODE;
            sock_info.sockid = sock;
            sock_info.offset = 0;
            strcpy(str, "CONNECT");
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            goto OPEN_EXIT;
        }

        snprintf(str, sizeof(str), "+QIOPEN: %d,%d", linkid, 0);
        aic_transmit_resp_ok(at_req_resp_ptr);
        aic_transmit_at_cmd_urc(at_req_resp_ptr->serial_port_id, str, strlen(str));
OPEN_EXIT:
        aic_free_paras_mem(paras_arr, cnt);
        freeaddrinfo(res);
        return AIC_TCPIP_AT_OK;

ERROR_EXIT:
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        if (res)
            freeaddrinfo(res);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        if (at_req_resp_ptr->end_data_mode) {
            if (sock_info.buf)
                aos_free(sock_info.buf);
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
            return AIC_TCPIP_AT_ERR;
        }
        index = aic_at_find_index_by_socket(sock_info.sockid, AT_QTCPIP);
        if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
            if (sock_info.buf)
                aos_free(sock_info.buf);
            sock_info.buf = NULL;
            sock_info.offset = 0;
            sock_list_q[index].mode = 0;
            at_req_resp_ptr->at_mode = AT_TMP_CMD_MODE;
            aicx_info(0x124873, "", "[AT_TCPIP]: change to AT conmand mode\n");
            //aic_transmit_resp_exit(at_req_resp_ptr);
            aic_transmit_resp_ok(at_req_resp_ptr);
            return AIC_TCPIP_AT_OK;
        } else if (strncmp(at_req_resp_ptr->at_upstream_data, "+", 1) == 0) {
            recv_cnt++;
            if (recv_cnt == 3) {
                if (sock_info.buf)
                    aos_free(sock_info.buf);
                sock_info.buf = NULL;
                sock_info.offset = 0;
                sock_list_q[index].mode = 0;
                at_req_resp_ptr->at_mode = AT_TMP_CMD_MODE;
                aic_transmit_resp_ok(at_req_resp_ptr);
                return AIC_TCPIP_AT_OK;
            }

            if (!sock_info.buf)
                sock_info.buf = aos_zalloc(AT_SOCKET_DATA_LEN_MAX);

            len = at_req_resp_ptr->upstream_data_len;
            if (len > AT_SOCKET_DATA_LEN_MAX) {
                len = AT_SOCKET_DATA_LEN_MAX;
            }
            aos_memcpy(sock_info.buf + sock_info.offset,
                       at_req_resp_ptr->at_upstream_data, len);
            sock_info.offset += len;
            return AIC_TCPIP_AT_OK;
        } else {
            recv_cnt = 0;
        }

        len = at_req_resp_ptr->upstream_data_len;
        if (len > AT_SOCKET_DATA_LEN_MAX) {
            len = AT_SOCKET_DATA_LEN_MAX;
        }

        if (!sock_info.buf)
            sock_info.buf = aos_zalloc(AT_SOCKET_DATA_LEN_MAX);

        aos_memcpy(sock_info.buf + sock_info.offset,
                   at_req_resp_ptr->at_upstream_data, len);
        sock_info.offset += len;

        if (sock_list_q[index].socket_type == SOCK_TCP ||
            sock_list_q[index].socket_type == SOCK_TCPS) {
            ret = anet_send(sock_info.sockid, sock_info.buf, sock_info.offset, 0);
        } else if (sock_list_q[index].socket_type == SOCK_UDP ||
            sock_list_q[index].socket_type == SOCK_UDPS) {
            ret = aic_qudp_socket_send(sock_info.sockid, sock_info.buf, sock_info.offset);
        } else {
            aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            aos_free(sock_info.buf);
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (ret < 0) {
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", (errno));
        } else {
            aicx_info(0x1d42d6, "d", "[AT_TCPIP]: send ret %d\n", ret);
        }
        if (sock_info.buf)
            aos_free(sock_info.buf);
        sock_info.buf = NULL;
        sock_info.offset = 0;
        return AIC_TCPIP_AT_OK;
    } else {
        if (at_req_resp_ptr->at_mode == AT_TMP_CMD_MODE) {
            if (sock_info.buf)
                aos_free(sock_info.buf);
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            return AIC_TCPIP_AT_OK;
        }
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

u32 handle_qisend_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QISEND: (0-11),(0-1460)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

void aic_get_socket_send_info(int sockid, int *sent, int *acked, int *unack)
{
    NX_BSD_SOCKET   *bsd_socket_ptr;

    bsd_socket_ptr = &nx_bsd_socket_array[sockid - NX_BSD_SOCKFD_START];
    if (bsd_socket_ptr->nx_bsd_socket_tcp_socket) {
        *sent = bsd_socket_ptr->nx_bsd_socket_tcp_socket->nx_tcp_socket_bytes_sent;
        *unack = bsd_socket_ptr->nx_bsd_socket_tcp_socket->nx_tcp_socket_tx_outstanding_bytes;
        *acked = *sent - *unack;
    }

    if (bsd_socket_ptr->nx_bsd_socket_udp_socket) {
        *sent = bsd_socket_ptr->nx_bsd_socket_udp_socket->nx_udp_socket_bytes_sent;
        *acked = *sent;
        *unack = *sent - *acked;
    }
}

/**
 * handle_qisend_set_cmd() - Send TCP/UDP/SSL data.
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
u32 handle_qisend_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index, len, link_id = 0, ret, sent = 0, acked = 0, unack = 0;
    ulong *paras_arr;
    u16 cnt;
    struct sockaddr_in destaddr;
    struct sockaddr_in6 destaddr6;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+QISEND=",
                                                 &cnt);
        if (!paras_arr || cnt < 1 || cnt > 4) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            if (paras_arr)
                aic_free_paras_mem(paras_arr, cnt);

            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }

        ret = aic_allowed_to_enter_data_mode(at_req_resp_ptr->serial_port_id);
        if (ret == FALSE) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_ENTER_DATA_MODE_FAIL);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_ENTER_DATA_MODE_FAIL;
        }
        if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
            link_id = atoi((u8 *)paras_arr[0]);
        else
            goto ERROR_EXIT;

        if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0)
            goto ERROR_EXIT;

        if (sock_list_q[link_id].ssl) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        sock_info.sockid = aic_at_find_socket_by_linkid(link_id, AT_QTCPIP);
        if (!sock_info.sockid){
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        sock_info.length = AT_SOCKET_DATA_LEN_DEFAULT;
        sock_info.offset = 0;
        if (cnt == 2) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                sock_info.length = atoi((u8 *)paras_arr[1]);
            else
                goto ERROR_EXIT;
            if (sock_info.length == 0) {
                aic_get_socket_send_info(sock_info.sockid, &sent, &acked, &unack);
                snprintf(str, sizeof(str), "+QISEND: %d,%d,%d", sent,
                         acked, unack);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
                aic_transmit_resp_ok(at_req_resp_ptr);
                goto SEND_EXIT;
            } else if (sock_info.length > AT_SOCKET_DATA_LEN_MAX ||
                       sock_info.length < 0) {
                goto ERROR_EXIT;
            }

            sock_info.is_send = AIC_TRUE;
        }

        if (cnt == 4) {
            inet_pton(AF_INET, (u8 *)paras_arr[2], &sock_list_q[link_id].addr);
            sock_list_q[link_id].port = htons(atoi((u8 *)paras_arr[3]));
        }

        at_req_resp_ptr->at_mode = AT_DATA_MODE;
        strcpy(str, ">");
        aic_transmit_resp_normal(at_req_resp_ptr, str);

SEND_EXIT:
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
ERROR_EXIT:
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        aicx_info(0x03b0ee, "d", "[AT_TCPIP]: recv data len %d\n", at_req_resp_ptr->upstream_data_len);
        if (at_req_resp_ptr->end_data_mode) {
            if (sock_info.buf)
                aos_free(sock_info.buf);
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
            return AIC_TCPIP_AT_ERR;
        }

        index = aic_at_find_index_by_socket(sock_info.sockid, AT_QTCPIP);
        len = at_req_resp_ptr->upstream_data_len;
        if (len > AT_SOCKET_DATA_LEN_MAX) {
            len = AT_SOCKET_DATA_LEN_MAX;
        } else if (len > sock_info.length && sock_info.length != 0xFFFF) {
            len = sock_info.length;
        }

        if (at_tcpip_cfg.sendecho) {
            aic_transmit_at_data(at_req_resp_ptr->serial_port_id,
                                 at_req_resp_ptr->at_upstream_data,
                                 at_req_resp_ptr->upstream_data_len);
        }

        if (!sock_info.buf)
             sock_info.buf = aos_zalloc_relaxed(sock_info.length);

        if (!sock_info.buf) {
            strcpy(str, "ERROR");
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (sock_info.length == AT_SOCKET_DATA_LEN_DEFAULT) {
            if (*at_req_resp_ptr->at_upstream_data == 0x1B) {
                at_req_resp_ptr->at_mode = AT_CMD_MODE;
                sock_list_q[index].mode = 0;
                aos_free(sock_info.buf);
                aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
                strcpy(str, "ERROR");
                aic_transmit_resp_normal(at_req_resp_ptr, str);
                return AIC_TCPIP_AT_OK;
            }
            if (*at_req_resp_ptr->at_upstream_data == 0x1A) {
                len = sock_info.offset;
                goto SEND;
            }

            if (AT_SOCKET_DATA_LEN_DEFAULT - sock_info.offset <= len) {
                len = AT_SOCKET_DATA_LEN_DEFAULT - sock_info.offset;
                aos_memcpy(sock_info.buf + sock_info.offset,
                           at_req_resp_ptr->at_upstream_data, len);
            } else {
                aos_memcpy(sock_info.buf + sock_info.offset,
                           at_req_resp_ptr->at_upstream_data, len);
            }
            sock_info.offset += len;
            if (sock_info.length == sock_info.offset && sock_info.is_send) {
                len = sock_info.offset;
                goto SEND;
            }
            return AIC_TCPIP_AT_OK;
        } else {
            if (sock_info.length - sock_info.offset <= len)
                len = sock_info.length - sock_info.offset;
            aos_memcpy(sock_info.buf + sock_info.offset,
                       at_req_resp_ptr->at_upstream_data, len);
            sock_info.offset += len;
            if (sock_info.length == sock_info.offset) {
                len = sock_info.offset;
                goto SEND;
            }

            return AIC_TCPIP_AT_OK;
        }

SEND:
        if (sock_list_q[index].socket_type == SOCK_TCP ||
            sock_list_q[index].socket_type == SOCK_TCPIN) {
            ret = anet_send(sock_info.sockid, sock_info.buf, len, 0);
        } else if (sock_list_q[index].socket_type == SOCK_UDP) {
            ret = aic_qudp_socket_send(sock_info.sockid, sock_info.buf, len);
        } else if (sock_list_q[index].socket_type == SOCK_UDPS) {
            if (sock_list_q[index].family == AF_INET) {
                destaddr.sin_addr.s_addr = sock_list_q[index].addr;
                destaddr.sin_port = sock_list_q[index].port;
                ret = anet_sendto(sock_info.sockid, sock_info.buf, len, 0,
                                  (const struct sockaddr *)&destaddr,
                                  sizeof(destaddr));
            } else {
                aos_memcpy(&destaddr6.sin6_addr, sock_list_q[index].addr6, 16);
                destaddr6.sin6_port = sock_list_q[index].port;
                ret = anet_sendto(sock_info.sockid, sock_info.buf, len, 0,
                                  (const struct sockaddr *)&destaddr6,
                                  sizeof(destaddr6));
            }
        } else {
            aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            if (sock_info.buf) {
                aos_free(sock_info.buf);
                aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            }
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (ret < 0) {
            strcpy(str, "SEND FAIL");
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", (errno));
        } else {
            strcpy(str, "SEND OK");
            aicx_info(0x1d42d6, "d", "[AT_TCPIP]: send ret %d\n", ret);
        }
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        at_req_resp_ptr->at_mode = AT_CMD_MODE;
        if (sock_info.buf) {
            aos_free(sock_info.buf);
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
        }
        return AIC_TCPIP_AT_OK;
    } else {
        if (at_req_resp_ptr->at_mode == AT_TMP_CMD_MODE) {
            if (sock_info.buf) {
                aos_free(sock_info.buf);
                aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            }
            return AIC_TCPIP_AT_OK;
        }
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

u32 handle_qisendex_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QISENDEX: (0-11),<hex_string>");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qisendex_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int sockid, link_id = 0;
    ulong *paras_arr;
    u16 cnt;
    int index, ret, len;
    u8 buf[BUFFER_SIZE] = {0};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QISENDEX=",
                                             &cnt);
    if (!paras_arr || cnt != 2) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        link_id = atoi((u8 *)paras_arr[0]);
    else
        goto EXIT;

    if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
        goto EXIT;
    }

    len = strlen((u8 *)paras_arr[1]);
    if (len < 2 || len > 256 || len % 2) {
        goto EXIT;
    }

    ret = aic_hexstring_to_char(buf, (u8 *)paras_arr[1], len);
    if (ret == AIC_TCPIP_AT_ERR) {
        goto EXIT;
    }
    sockid = aic_at_find_socket_by_linkid(link_id, AT_QTCPIP);
    if (!sockid){
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    index = aic_at_find_index_by_socket(sockid, AT_QTCPIP);
    if (sock_list_q[index].socket_type == SOCK_TCP ||
        sock_list_q[index].socket_type == SOCK_TCPIN) {
        ret = anet_send(sockid, buf, len / 2, 0);
    } else if (sock_list_q[index].socket_type == SOCK_UDP ||
               sock_list_q[index].socket_type == SOCK_UDPS) {
        ret = aic_qudp_socket_send(sockid, buf, len / 2);
    } else {
        aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    if (ret < 0) {
        sprintf(str, "SEND FAIL");
        aicx_error(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", errno);
    } else {
        strcpy(str, "SEND OK");
    }
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

EXIT:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 handle_qiclose_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QICLOSE: (0-11),(0-65535)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);;
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_qiclose_set_cmd() - Close TCP/UDP connection.
 * @para_num: parameter.
 *
 * CALLS:
 *    soc_close
 *
 * CALLED BY:
 *    Application
 *
 * Close TCP/UDP connection.
 *
 * Return: AIC_TCPIP_AT_OK for success, others for fail.
 */
u32 handle_qiclose_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    int link_id = 0;
    int i;
    ulong *paras_arr;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QICLOSE=",
                                             &cnt);
    if (!paras_arr || cnt < 1 || cnt > 2 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        link_id = atoi((u8 *)paras_arr[0]);
    else
        goto EXIT;

    if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
        goto EXIT;;
    }

    i = link_id;
    if (sock_list_q[i].ssl){
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    if (sock_list_q[i].socket_type == SOCK_TCPS)
        aic_qat_socket_listen_uninit();

    if (sock_list_q[i].socket_type == SOCK_TCP ||
        sock_list_q[i].socket_type == SOCK_TCPIN) {
        aic_socket_unregister_tcp_disconnect_notify(sock_list_q[i].socket_id);
    }

    aic_socket_unregister_receive_notify(sock_list_q[i].socket_id);
    aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
    soc_close(sock_list_q[i].socket_id);
    aic_end_at_data_mode(at_tcpip_cfg.com_id);
    aos_put_mutex(at_socket_mutex);
    aos_memset(&sock_list_q[i], 0, sizeof(struct SOCKET_LIST));

    /* release udp data buffer for AT+QIRD */
    if (udp_data[i].data) {
        aos_free(udp_data[i].data);
        udp_data[i].data = NULL;
        udp_data[i].recv_len = 0;
        udp_data[i].read_len = 0;
    }

    aic_transmit_resp_ok(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

EXIT:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 handle_qistate_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aic_transmit_resp_ok(at_req_resp_ptr);;
    return AIC_TCPIP_AT_OK;
}

void aic_get_socket_state(int link_id, struct socket_state *state)
{
    int i, sock, addrlen;
    struct sockaddr_in remoteaddr, localaddr;
    struct sockaddr_in6 remoteaddr6, localaddr6;

    i = aic_at_find_index_by_linkid(link_id, AT_QTCPIP);
    sock = sock_list_q[i].socket_id;
    if (sock_list_q[i].family == AF_INET) {
        memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
        memset(&localaddr, 0, sizeof(struct sockaddr_in));
        addrlen = sizeof(struct sockaddr_in);
        // anet_getpeername(sock, (struct sockaddr *)&remoteaddr, &addrlen);
        anet_getsockname(sock, (struct sockaddr *)&localaddr, &addrlen);
        // if (sock_list_q[i].socket_type == SOCK_UDP || ) {
            remoteaddr.sin_addr.s_addr = sock_list_q[i].addr;
            remoteaddr.sin_port = sock_list_q[i].port;
        // }
        inet_ntop(AF_INET, &remoteaddr.sin_addr.s_addr,
                  state->addr, ADDR_BUFFERSIZE_IPV4);
        state->remote_port = htons(remoteaddr.sin_port);
        state->local_port = htons(localaddr.sin_port);
    } else {
        memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
        memset(&localaddr6, 0, sizeof(struct sockaddr_in6));
        addrlen = sizeof(struct sockaddr_in6);
        // anet_getpeername(sock, (struct sockaddr *)&remoteaddr6, &addrlen);
        anet_getsockname(sock, (struct sockaddr *)&localaddr6, &addrlen);
        // if (sock_list_q[i].socket_type == SOCK_UDP) {
            aos_memcpy(remoteaddr6.sin6_addr._S6_un._S6_u32, sock_list_q[i].addr6, 16);
            remoteaddr6.sin6_port = sock_list_q[i].port;
        // }
        inet_ntop(AF_INET6, &remoteaddr6.sin6_addr, state->addr,
                  ADDR_BUFFERSIZE_IPV6);
        state->remote_port = htons(remoteaddr6.sin6_port);
        state->local_port = htons(localaddr6.sin6_port);
    }

    if (sock_list_q[i].socket_type == SOCK_TCP) {
        strcpy(state->socket_type, "TCP");
        if (sock_list_q[i].status == SOCKET_STATE_CLOSING)
            state->state = SOCKET_STATE_CLOSING;
        else
            state->state = SOCKET_STATE_CONNECTED;
    } else if (sock_list_q[i].socket_type == SOCK_TCPS) {
        strcpy(state->socket_type, "TCP LISTENER");
        state->state = SOCKET_STATE_LISTENING;
    } else if (sock_list_q[i].socket_type == SOCK_UDP) {
        strcpy(state->socket_type, "UDP");
        state->state = SOCKET_STATE_CONNECTED;
    } else if (sock_list_q[i].socket_type == SOCK_UDPS) {
        strcpy(state->socket_type, "UDP SERVICE");
        state->state = SOCKET_STATE_CONNECTED;
    } else if (sock_list_q[i].socket_type == SOCK_TCPIN) {
        strcpy(state->socket_type, "TCP INCOMING");
        state->state = SOCKET_STATE_CONNECTED;
    } else {
        strcpy(state->socket_type, "NULL");
        state->state = SOCKET_STATE_INITIAL;
    }

    state->pdp_cid = sock_list_q[i].pdp_cid;
    state->mode = sock_list_q[i].mode;
    state->server_id = link_id;
    aos_memset(state->at_port, 0 , AT_URC_PORT_NAME_LEN);
    aic_get_at_port_name(at_tcpip_cfg.com_id, state->at_port,
                         AT_URC_PORT_NAME_LEN);
}

/**
 * handle_qistate_query_cmd() - Query connection information.
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
u32 handle_qistate_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int i = 0, sock, link_id;
    struct socket_state state;
    char str[BUFFER_SIZE] = {'\0'};

    while (i < AT_SOCKET_MAX_CONN_NUM_Q) {
        sock = sock_list_q[i].socket_id;

        if (sock == 0) {
            i++;
            continue;
        }

        link_id = sock_list_q[i].link_id;
        aos_memset(&state, 0, sizeof(struct socket_state));
        aic_get_socket_state(link_id, &state);

        aos_memset(str, '\0', AT_RESP_RESULT_MAX);
        snprintf(str, sizeof(str), "+QISTATE: %d,\"%s\",\"%s\",%d,%d,%d,%d,%d,%d,\"%s\"",
                 link_id, state.socket_type, state.addr, state.remote_port,
                 state.local_port, state.state, state.pdp_cid,
                 state.server_id, state.mode, state.at_port);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        i++;
    }

    aicx_info(0x0efe3e, "", "handle_qistate_query_cmd return\n");
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qistate_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    struct socket_state state;
    int i, query_type, pdp_id = 1, link_id = 0;
    char str[BUFFER_SIZE];

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QISTATE=",
                                             &cnt);
    if (!paras_arr || cnt != 2) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        goto STATE_EXIT;
    }
    query_type = atoi((u8 *)paras_arr[0]);
    if (query_type == 0) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
            pdp_id = atoi((u8 *)paras_arr[1]);
        } else {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            goto STATE_EXIT;
        }

        if (pdp_id < ANET_MIN_PDP_CID || pdp_id >= ANET_MAX_PDP_CID) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            goto STATE_EXIT;
        }

        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (sock_list_q[i].pdp_cid != pdp_id)
                continue;

            if (sock_list_q[i].ssl){
                i++;
                continue;
            }
            link_id = sock_list_q[i].link_id;
            memset(&state, 0, sizeof(struct socket_state));
            aic_get_socket_state(link_id, &state);
            aos_memset(str, '\0', AT_RESP_RESULT_MAX);
            snprintf(str, sizeof(str),
                     "+QISTATE: %d,\"%s\",\"%s\",%d,%d,%d,%d,%d,%d,\"%s\"",
                     link_id, state.socket_type, state.addr, state.remote_port,
                     state.local_port, state.state, state.pdp_cid,
                     state.server_id, state.mode, state.at_port);

            aic_transmit_resp_normal(at_req_resp_ptr, str);
        }
    } else if (query_type == 1) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
            link_id = atoi((u8 *)paras_arr[1]);
        } else {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            goto STATE_EXIT;
        }

        if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            goto STATE_EXIT;
        }

        i = aic_at_find_index_by_linkid(link_id, AT_QTCPIP);
        if (sock_list_q[i].socket_id == 0)
            goto EXIT;

        if (sock_list_q[i].ssl){
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        aic_get_socket_state(link_id, &state);
        aos_memset(str, '\0', AT_RESP_RESULT_MAX);
        snprintf(str, sizeof(str), "+QISTATE: %d,\"%s\",\"%s\",%d,%d,%d,%d,%d,%d,\"%s\"",
                 link_id, state.socket_type, state.addr, state.remote_port,
                 state.local_port, state.state, state.pdp_cid, state.server_id,
                 state.mode, state.at_port);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    } else {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        goto STATE_EXIT;
    }

EXIT:
    aic_transmit_resp_ok(at_req_resp_ptr);

STATE_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qird_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QIRD: (0-11),(0-1500)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

void aic_get_socket_recv_info(int sockid, int *total_recv, int *recved,
                              int *unrecv)
{
    NX_BSD_SOCKET   *bsd_socket_ptr;
    int len = 0, i;

    i = aic_at_find_index_by_socket(sockid, AT_QTCPIP);
    if (i == AIC_EPERM)
        return;

    getsockopt(sockid, SOL_SOCKET, SO_RXDATA, unrecv, &len);
    bsd_socket_ptr = &nx_bsd_socket_array[sockid - NX_BSD_SOCKFD_START];
    if (bsd_socket_ptr->nx_bsd_socket_tcp_socket) {
        *total_recv = bsd_socket_ptr->nx_bsd_socket_tcp_socket->nx_tcp_socket_bytes_received;
        *recved = *total_recv - *unrecv;
    }

    if (bsd_socket_ptr->nx_bsd_socket_udp_socket) {
        *total_recv = bsd_socket_ptr->nx_bsd_socket_udp_socket->nx_udp_socket_bytes_received;
        *unrecv = *unrecv + udp_data[i].recv_len - udp_data[i].read_len;
        *recved = *total_recv - *unrecv;
    }
}

u32 handle_qird_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int i = 0, addrlen, sock, length, link_id = 0;
    char *data = NULL;
    char addr[ADDR_BUFFERSIZE_IPV6] = {0};
    struct sockaddr_in remoteaddr;
    struct sockaddr_in6 remoteaddr6;
    int fromaddrlen, total_recv = 0, recved = 0, unrecv = 0;
    char str[1536] = {'\0'};
    u16 port, cnt;
    ulong *paras_arr;
    int ret = -1;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QIRD=",
                                             &cnt);
    if (!paras_arr || cnt < 1 || cnt > 2) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        goto ERROR_EXIT;
    }

    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        link_id = atoi((u8 *)paras_arr[0]);
        if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }


    if (sock_list_q[link_id].ssl) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_QTCPIP);
    if (!sock) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    if (cnt == 2) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            goto ERROR_EXIT;

        length = atoi((u8 *)paras_arr[1]);
        if (length == 0) {
            aic_get_socket_recv_info(sock, &total_recv, &recved, &unrecv);
            snprintf(str, sizeof(str), "+QIRD: %d,%d,%d", total_recv, recved,
                     unrecv);
            goto RD_EXIT;
        } else if (length > 1500 || length < 0) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }
    } else {
        length = 1500;
    }

    i = aic_at_find_index_by_socket(sock, AT_QTCPIP);
    if (i < 0) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }
    sock = aic_at_find_socket_by_linkid(link_id, AT_QTCPIP);

    data = aos_zalloc(length + 1);
    if (!data) {
        aicx_error(0x18a3e2, "", "[AT_TCPIP]: alloc recv buffer fail\n");
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_RECV_FAIL;
    }

    if (sock_list_q[i].socket_type == SOCK_TCP ||
        sock_list_q[i].socket_type == SOCK_TCPIN) {
        while (1) {
            ret = anet_recv(sock, data + total_recv, length - total_recv,
                            MSG_DONTWAIT);
            if (ret < 0) {
                break;
            } else {
                total_recv += ret;
                if (total_recv == length)
                    break;
            }
        }
    } else if (sock_list_q[i].socket_type == SOCK_UDP ||
               sock_list_q[i].socket_type == SOCK_UDPS) {
        if (!udp_data[i].data) {
            u32 len = 0;
            getsockopt(sock, SOL_SOCKET, SO_RXDATA, &udp_data[i].recv_len, &len);
            aicx_info(0x1d90ca, "dd", "tcpip: unrecv: %d, len: %d\n", udp_data[i].recv_len, len);
            if (udp_data[i].recv_len) {
                udp_data[i].data = aos_zalloc(udp_data[i].recv_len);
                if (!udp_data[i].data) {
                    aos_free(data);
                    aicx_error(0x113f4d, "", "[AT_TCPIP]: alloc udp data buffer fail\n");
                    aic_transmit_resp_error(at_req_resp_ptr,
                                            AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
                    aic_free_paras_mem(paras_arr, cnt);
                    return AIC_TCPIP_ERR_SOCKET_RECV_FAIL;
                }

                if (sock_list_q[i].family == AF_INET) {
                    fromaddrlen = sizeof(remoteaddr);
                    ret = anet_recvfrom(sock, udp_data[i].data, udp_data[i].recv_len, MSG_DONTWAIT,
                                    (struct sockaddr *)&remoteaddr, &fromaddrlen);
                } else if (sock_list_q[i].family == AF_INET6) {
                    fromaddrlen = sizeof(remoteaddr6);
                    ret = anet_recvfrom(sock, udp_data[i].data, udp_data[i].recv_len, MSG_DONTWAIT,
                                    (struct sockaddr *)&remoteaddr6, &fromaddrlen);
                } else {
                    aicx_info(0x0692d9, "d", "[AT_TCPIP]: Family error: %d\n", sock_list_q[i].family);
                }
                udp_data[i].read_len = 0;
                udp_data[i].recv_len = ret;
            } else {
                udp_data[i].recv_len = ret = 0;
            }
        }

        if (udp_data[i].recv_len > 0) {
            if (length <= (udp_data[i].recv_len - udp_data[i].read_len)) {
                memcpy(data, udp_data[i].data + udp_data[i].read_len, length);
                udp_data[i].read_len += length;
            } else {
                length = udp_data[i].recv_len - udp_data[i].read_len;
                memcpy(data, udp_data[i].data + udp_data[i].read_len, length);
                udp_data[i].read_len += length;
                aos_free(udp_data[i].data);
                udp_data[i].data = NULL;
                udp_data[i].recv_len = 0;
                udp_data[i].read_len = 0;
            }

            ret = length;
        }
    } else {
        aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
    }

    if (ret < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            if (total_recv == 0) {
                snprintf(str, sizeof(str), "+QIRD: 0");
                goto RD_EXIT;
            } else {
                ret = total_recv;
                goto RD_EXIT0;
            }
        }
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
        aos_free(data);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_ERR;
    } else {
        if (ret < total_recv)
            ret = total_recv;
    }

RD_EXIT0:
    if (sock_list_q[i].family == AF_INET) {
        if (sock_list_q[i].socket_type != SOCK_UDP &&
            sock_list_q[i].socket_type != SOCK_UDPS) {
            addrlen = sizeof(struct sockaddr_in);
            anet_getpeername(sock, (struct sockaddr *)&remoteaddr,
                             &addrlen);
        }

        inet_ntop(AF_INET, &remoteaddr.sin_addr.s_addr, addr,
                  ADDR_BUFFERSIZE_IPV4);
        port = htons(remoteaddr.sin_port);
    } else {
        if (sock_list_q[i].socket_type != SOCK_UDP &&
            sock_list_q[i].socket_type != SOCK_UDPS) {
            addrlen = sizeof(struct sockaddr_in6);
            anet_getpeername(sock, (struct sockaddr *)&remoteaddr6,
                             &addrlen);
        }

        inet_ntop(AF_INET6, &remoteaddr6.sin6_addr, addr, ADDR_BUFFERSIZE_IPV6);
        port = htons(remoteaddr6.sin6_port);
    }

    if (sock_list_q[i].socket_type == SOCK_UDPS)
        snprintf(str, sizeof(str), "+QIRD: %d,%s,%d\r\n%s",
                 ret, addr, port, data);
    else
        snprintf(str, sizeof(str), "+QIRD: %d\r\n%s", ret, data);

RD_EXIT:
    aic_get_socket_recv_info(sock, &total_recv, &recved, &unrecv);
    sock_list_q[i].recv_total = unrecv;
    aic_transmit_at_data(at_req_resp_ptr->serial_port_id, str, strlen(str));
    aic_transmit_resp_ok(at_req_resp_ptr);
    if (data)
        aos_free(data);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

ERROR_EXIT:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 handle_qiswtmd_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    sprintf(str, "+QISWTMD: (0-11),(0-2)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);;
    return AIC_TCPIP_AT_OK;
}

u32 handle_qistwmd_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aic_transmit_resp_ok(at_req_resp_ptr);;
    return AIC_TCPIP_AT_OK;
}

u32 handle_qistwmd_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int link_id = 0, index, len, ret, mode, fromaddrlen;
    int unrecv, len_t, offset = 0;
    struct sockaddr_in destaddr;
    struct sockaddr_in6 destaddr6;
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    u8 *buf;

    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+QISWTMD=",
                                                 &cnt);
        if (!paras_arr || cnt != 2) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            if (paras_arr)
                aic_free_paras_mem(paras_arr, cnt);

            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }

        if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
            link_id = atoi((u8 *)paras_arr[0]);
        } else {
            goto ERROR_EXIT;
        }

        if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
            goto ERROR_EXIT;
        }

        index = aic_at_find_index_by_linkid(link_id, AT_QTCPIP);
        if (!sock_list_q[index].socket_id) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
            mode = atoi((u8 *)paras_arr[1]);
            if (mode < 0 || mode > 2) {
                goto ERROR_EXIT;
            } else {
                sock_list_q[index].mode = mode;
            }
        } else {
            goto ERROR_EXIT;
        }

        if (sock_list_q[index].mode == 2) {
            ret = aic_allowed_to_enter_data_mode(at_req_resp_ptr->serial_port_id);
            if (ret == FALSE) {
                aic_transmit_resp_error(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_ENTER_DATA_MODE_FAIL);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
            }
            sock_info.sockid = sock_list_q[index].socket_id;
            at_req_resp_ptr->at_mode = AT_DATA_MODE;
            sock_info.offset = 0;
            at_tcpip_cfg.com_id = at_req_resp_ptr->serial_port_id;
            strcpy(str, "CONNECT");
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_AT_OK;
        }

        if (sock_list_q[index].mode == 1) {
            getsockopt(sock_list_q[index].socket_id, SOL_SOCKET,
                       SO_RXDATA, &unrecv, &len_t);
            len = unrecv;

            if (len > 0) {
                buf = aos_zalloc_relaxed(len + 50);
                if (!buf) {
                    aicx_info(0x036f5b, "", "[AT_TCPIP] alloc buffer fail\n");
                    aic_transmit_resp_error(at_req_resp_ptr,
                                            AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
                    aic_free_paras_mem(paras_arr, cnt);
                    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
                }
recv_again:
                if (sock_list_q[index].socket_type == SOCK_TCP ||
                    sock_list_q[index].socket_type == SOCK_TCPv6) {
                    ret = anet_recv(sock_list_q[index].socket_id,
                                    buf + 50 + offset, len, MSG_DONTWAIT);
                } else {
                    if (sock_list_q[index].family == AF_INET) {
                        fromaddrlen = sizeof(struct sockaddr_in);
                        ret = anet_recvfrom(sock_list_q[index].socket_id,
                                            buf + 50 + offset, len, MSG_DONTWAIT,
                                            (struct sockaddr *)&destaddr,
                                            &fromaddrlen);
                    } else {
                        fromaddrlen = sizeof(struct sockaddr_in6);
                        ret = anet_recvfrom(sock_list_q[index].socket_id,
                                            buf + 50 + offset, len, MSG_DONTWAIT,
                                            (struct sockaddr *)&destaddr6,
                                            &fromaddrlen);
                    }
                }

                if (ret < len && offset + ret < len) {
                    offset += ret;
                    goto recv_again;
                } else {
                    offset += ret;
                }

                if (offset >= 0) {
                    getsockopt(sock_list_q[index].socket_id, SOL_SOCKET,
                               SO_RXDATA, &unrecv, &len);
                    snprintf(str, AT_RESP_RESULT_MAX, "+QIURC: \"recv\",%d,%d\r\n",
                             link_id, offset);
                    len = strlen(str);
                    aos_memcpy((buf + 50 - len), str, len);
                }
                aic_transmit_at_data(at_req_resp_ptr->serial_port_id,
                                     (buf + 50 - len), offset + len);
                aos_free(buf);
            }
        }

        aic_transmit_resp_ok(at_req_resp_ptr);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;

ERROR_EXIT:
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        index = aic_at_find_index_by_socket(sock_info.sockid, AT_QTCPIP);
        if (at_req_resp_ptr->end_data_mode) {
            if (sock_info.buf)
                aos_free(sock_info.buf);
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
            return AIC_TCPIP_AT_ERR;
        }

        index = aic_at_find_index_by_socket(sock_info.sockid, AT_QTCPIP);
        if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
            if (sock_info.buf)
                aos_free(sock_info.buf);
            sock_info.buf = NULL;
            sock_info.offset = 0;
            at_req_resp_ptr->at_mode = AT_TMP_CMD_MODE;
            aicx_info(0x124873, "", "[AT_TCPIP]: change to AT conmand mode\n");
            //aic_transmit_resp_exit(at_req_resp_ptr);
            aic_transmit_resp_ok(at_req_resp_ptr);
            return AIC_TCPIP_AT_OK;
        } else if (strncmp(at_req_resp_ptr->at_upstream_data, "+", 1) == 0) {
            recv_cnt++;
            if (recv_cnt == 3) {
                if (sock_info.buf)
                    aos_free(sock_info.buf);
                sock_info.buf = NULL;
                sock_info.offset = 0;
                at_req_resp_ptr->at_mode = AT_TMP_CMD_MODE;
                aic_transmit_resp_ok(at_req_resp_ptr);
                return AIC_TCPIP_AT_OK;
            }
            len = at_req_resp_ptr->upstream_data_len;
            if (len > AT_SOCKET_DATA_LEN_MAX) {
                len = AT_SOCKET_DATA_LEN_MAX;
            }
            if (!sock_info.buf)
                sock_info.buf = aos_zalloc(AT_SOCKET_DATA_LEN_MAX);

            aos_memcpy(sock_info.buf + sock_info.offset,
                       at_req_resp_ptr->at_upstream_data, len);
            sock_info.offset += len;
            return AIC_TCPIP_AT_OK;
        } else {
            recv_cnt = 0;
        }

        len = at_req_resp_ptr->upstream_data_len;
        if (len > AT_SOCKET_DATA_LEN_MAX) {
            len = AT_SOCKET_DATA_LEN_MAX;
        }
        if (!sock_info.buf)
            sock_info.buf = aos_zalloc(AT_SOCKET_DATA_LEN_MAX);

        aos_memcpy(sock_info.buf + sock_info.offset,
                   at_req_resp_ptr->at_upstream_data, len);
        sock_info.offset += len;

        if (sock_list_q[index].socket_type == SOCK_TCP ||
            sock_list_q[index].socket_type == SOCK_TCPIN) {
            ret = anet_send(sock_info.sockid, sock_info.buf, sock_info.offset, 0);
        } else if (sock_list_q[index].socket_type == SOCK_UDP) {
            ret = aic_qudp_socket_send(sock_info.sockid, sock_info.buf, sock_info.offset);
        } else if (sock_list_q[index].socket_type == SOCK_UDPS) {
            if (sock_list_q[index].family == AF_INET) {
                destaddr.sin_addr.s_addr = sock_list_q[index].addr;
                destaddr.sin_port = sock_list_q[index].port;
                ret = anet_sendto(sock_info.sockid, sock_info.buf, sock_info.offset, 0,
                                  (const struct sockaddr *)&destaddr,
                                  sizeof(destaddr));
            } else {
                aos_memcpy(&destaddr6.sin6_addr, sock_list_q[index].addr6, 16);
                destaddr6.sin6_port = sock_list_q[index].port;
                ret = anet_sendto(sock_info.sockid, sock_info.buf, sock_info.offset, 0,
                                  (const struct sockaddr *)&destaddr6,
                                  sizeof(destaddr6));
            }
        } else {
            aicx_info(0x062b65, "", "[AT_TCPIP]: The socket is not tcp or udp!\n");
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            aos_free(sock_info.buf);
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (ret < 0) {
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", (errno));
        } else {
            aicx_info(0x1d42d6, "d", "[AT_TCPIP]: send ret %d\n", ret);
        }
        if (sock_info.buf)
            aos_free(sock_info.buf);
        sock_info.buf = NULL;
        sock_info.offset = 0;
        return AIC_TCPIP_AT_OK;
    }  else {
        if (at_req_resp_ptr->at_mode == AT_TMP_CMD_MODE) {
            if (sock_info.buf)
                aos_free(sock_info.buf);
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            return AIC_TCPIP_AT_OK;
        }
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

/**
 * handle_qidnscfg_query_cmd() - Query DNS server configuration.
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
u32 handle_qidnscfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    sprintf(str, "+QIDNSCFG: (1-5),<pridnsaddr>,<secdnsaddr>");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);;
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
u32 handle_qidnscfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt = 0;
    NX_DNS *dns_ptr;
    NXD_ADDRESS dns_server_address;
    struct in_addr dns[NX_DNS_MAX_SERVERS] = {0};
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    char str1[AT_RESP_RESULT_MAX] = {'\0'};
    char addr[ADDR_BUFFERSIZE_IPV6] = {0};
    ulong *paras_arr, dns_addr;
    int cid = 0, status;
    WAN_NET *net_ptr;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QIDNSCFG=",
                                             &cnt);

    if (!paras_arr || (cnt < 1 || cnt > 3)) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        cid = atoi((u8 *)paras_arr[0]);

    if (cid < 1 || cid > ANET_MAX_PDP_CID) {
        aic_free_paras_mem(paras_arr, cnt);
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    net_ptr = aic_get_wan_net_per_pdp(cid);
    if (!net_ptr) {
        snprintf(str, sizeof(str), "+QIDNSCFG: %d,\"\",\"\"", cid);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        goto EXIT;
    }

    dns_ptr = aic_get_dns_client();
    if (cnt == 1) {
        dns_addr = htonl(net_ptr->wan_ipv4_dns1_address);
        inet_ntop(AF_INET, &dns_addr, addr, ADDR_BUFFERSIZE_IPV4);
        snprintf(str, sizeof(str), "+QIDNSCFG: %d,\"%s\",", cid, addr);
        dns_addr = htonl(net_ptr->wan_ipv4_dns2_address);
        inet_ntop(AF_INET, &dns_addr, addr, ADDR_BUFFERSIZE_IPV4);
        snprintf(str1, sizeof(str1), "\"%s\"", addr);
        strcat(str, str1);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    }

    if (cnt >= 2) {
        status = inet_pton(AF_INET, (u8 *)paras_arr[1], &dns[0]);
        if (status != 1) {
            aic_free_paras_mem(paras_arr, cnt);
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }

        net_ptr->wan_ipv4_dns1_address = htonl(dns[0].s_addr);
        if (cid == 1) {
            dns_server_address.nxd_ip_address.v4 = htonl(dns[0].s_addr);
            dns_server_address.nxd_ip_version = NX_IP_VERSION_V4;

            nx_dns_server_remove_all(dns_ptr);
            nxd_dns_server_add(dns_ptr, &dns_server_address);
        }
        if (cnt == 3) {
            status = inet_pton(AF_INET, (u8 *)paras_arr[2], &dns[0]);
            if (status != 1) {
                aic_free_paras_mem(paras_arr, cnt);
                aic_transmit_resp_error(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_PARSE_AT_PARAM);
                return AIC_TCPIP_ERR_PARSE_AT_PARAM;
            }
            net_ptr->wan_ipv4_dns2_address = htonl(dns[0].s_addr);
            if (cid == 1) {
                dns_server_address.nxd_ip_address.v4 = htonl(dns[0].s_addr);
                dns_server_address.nxd_ip_version = NX_IP_VERSION_V4;
                nxd_dns_server_add(dns_ptr, &dns_server_address);
            }
        }
    }

EXIT:
    aic_transmit_resp_ok(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qidnsgip_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QIDNSGIP: (1-5),<hostname>");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_qidnsgip_set_cmd() - Do dns query.
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
u32 handle_qidnsgip_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;
    struct addrinfo *res;
    char addr[ADDR_BUFFERSIZE_IPV6] = {0};
    char str[BUFFER_SIZE] = {'\0'};
    int ret, cid;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QIDNSGIP=",
                                             &cnt);
    if (!paras_arr || cnt != 2) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto ERROR_EXIT;

    cid = atoi((u8 *)paras_arr[0]);
    if (cid < 1 || cid > ANET_MAX_PDP_CID){
        goto ERROR_EXIT;
    }

    ret = getaddrinfo((u8 *)paras_arr[1], NULL, NULL, &res);
    if (ret != NX_SUCCESS || !res) {
        aicx_error(0x0a036e, "sd", "[AT_TCPIP]: get addr info fail %s ret %d\r\n",
                   (s8 *)paras_arr[1], ret);

        aic_free_paras_mem(paras_arr, cnt);
        aic_transmit_resp_error(at_req_resp_ptr, AIC_TCPIP_ERR_DNS_FAIL);
        return ret;
    }

    if(res->ai_addr->sa_family == AF_INET) {
        inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr,
                  addr, ADDR_BUFFERSIZE_IPV4);
    } else {
        inet_ntop(AF_INET6, &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr,
                  addr, ADDR_BUFFERSIZE_IPV6);
    }

    aic_transmit_resp_ok(at_req_resp_ptr);
    snprintf(str, sizeof(str), "+QIURC: \"dnsgip\",%s", addr);
    aic_transmit_at_cmd_urc(at_req_resp_ptr->serial_port_id, str, strlen(str));

    aic_free_paras_mem(paras_arr, cnt);
    freeaddrinfo(res);
    return AIC_TCPIP_AT_OK;

ERROR_EXIT:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 handle_qicfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QICFG: \"transpktsize\",(1-1460)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), "+QICFG: \"transwaittm\",(0-20)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), "+QICFG: \"dataformat\",(0,1),(0,1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), "+QICFG: \"viewmode\",(0,1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), "+QICFG: \"tcp/retranscfg\",(3-20),(5-1000)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str),
             "+QICFG: \"send/auto\",(1-5),(20-86400),<msg_auto>");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qicfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt, str_len;
    ulong *paras_arr;
    int link_id = 0, sock, paras = -1;
    struct sock_keepalive keepalive = {0};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QICFG=",
                                             &cnt);
    if (!paras_arr || cnt < 1 || cnt > 4) {
        aicx_error(0x023651, "", "[AT_TLS]: parameter parse error\n");
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    str_len = strlen((u8*)paras_arr[0]);
    if (strncasecmp((u8*)paras_arr[0], "transpktsize", 12) == 0 &&
        str_len == 12) {
        if (cnt == 2) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                paras = atoi((u8*)paras_arr[1]);

            if (paras < 1 || paras > 1460)
                goto error;
            else
                at_tcpip_cfg.transpktsize = paras;
        } else if (cnt == 1) {
            snprintf(str, sizeof(str), "+QICFG: \"transpktsize\",%d",
                     at_tcpip_cfg.transpktsize);
        } else
            goto error;
    } else if (strncasecmp((u8*)paras_arr[0], "transwaittm", 11) == 0 &&
               str_len == 11) {
        if (cnt == 2) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                paras = atoi((u8*)paras_arr[1]);  //unit: 100ms

            if (paras < 0 || paras > 20)
                goto error;

            at_tcpip_cfg.transwaittm = paras * 100/1000;  //unit: s
        } else if (cnt == 1) {
            snprintf(str, sizeof(str), "+QICFG: \"transwaittm\",%d",
                     at_tcpip_cfg.transwaittm);
        } else
            goto error;
    } else if (strncasecmp((u8*)paras_arr[0], "dataformat", 10) == 0 &&
               str_len == 10) {
        if (cnt == 3) {
            if (strcmp((u8 *)paras_arr[1], "0") == 0 ||
                strcmp((u8 *)paras_arr[1], "1") == 0)
                at_tcpip_cfg.send_data_format = atoi((u8*)paras_arr[1]);
            else
                goto error;

            if (strcmp((u8 *)paras_arr[2], "0") == 0 ||
                strcmp((u8 *)paras_arr[2], "1") == 0)
                at_tcpip_cfg.recv_data_format = atoi((u8*)paras_arr[2]);
            else
                goto error;
        } else if (cnt == 1) {
            snprintf(str, sizeof(str), "+QICFG: \"dataformat\",%d,%d",
                     at_tcpip_cfg.send_data_format, at_tcpip_cfg.recv_data_format);
        } else
            goto error;
    } else if (strncasecmp((u8*)paras_arr[0], "viewmode", 8) == 0 &&
               str_len == 8) {
        if (cnt == 2) {
            if (strcmp((u8 *)paras_arr[1], "0") == 0 ||
                strcmp((u8 *)paras_arr[1], "1") == 0)
                at_tcpip_cfg.view_mode = atoi((u8*)paras_arr[1]);
            else
                goto error;
        } else if (cnt == 1) {
            snprintf(str, sizeof(str), "+QICFG: \"viewmode\",%d",
                     at_tcpip_cfg.view_mode);
        } else
            goto error;
    } else if (strncasecmp((u8*)paras_arr[0], "tcp/retranscfg", 14) == 0 &&
               str_len == 14) {
        if (cnt == 3) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                paras = atoi((u8*)paras_arr[1]);

            if (paras < 3 || paras > 20)
                goto error;

            at_tcpip_cfg.max_backoffs = paras;

            if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                paras = atoi((u8*)paras_arr[2]);

            if (paras < 5 || paras > 1000)
                goto error;

            at_tcpip_cfg.max_rto = paras;
        } else if (cnt == 1) {
            snprintf(str, sizeof(str), "+QICFG: \"tcp/retranscfg\",%d,%d",
                     at_tcpip_cfg.max_backoffs, at_tcpip_cfg.max_rto);
        } else
            goto error;
    } else if (strncasecmp((u8*)paras_arr[0], "send/auto", 9) == 0 &&
               str_len == 9) {
        if (cnt >= 3) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                link_id = atoi((u8*)paras_arr[1]);

            if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0)
                goto error;

            if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                paras = atoi((u8*)paras_arr[2]);

            if (paras < 0 || paras > 86400)
                goto error;

            sock = aic_at_find_socket_by_linkid(link_id, AT_QTCPIP);
            if (sock == 0) {
                aic_transmit_resp_error(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
                aic_free_paras_mem(paras_arr, cnt);
                return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
            }

            at_tcpip_cfg.cycle_time = paras;
            if (paras < 20) {
                keepalive.keepalive_enabled = NX_FALSE;
            } else {
                keepalive.keepalive_enabled = NX_TRUE;
            }
            anet_setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
                            (const VOID *)&keepalive, sizeof(keepalive));

            if (cnt == 4) {
                str_len = strlen((u8*)paras_arr[3]);
                if (str_len > AT_AUTO_MSG_LEN_MAX)
                    goto error;
                else
                    strncpy(at_tcpip_cfg.msg_auto, (const char*)paras_arr[3],
                            str_len);
            }
        } else if (cnt == 2) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                link_id = atoi((u8*)paras_arr[1]);

            if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0)
                goto error;

            snprintf(str, sizeof(str), "+QICFG: \"send/auto\",%d,%d,%s",
                     link_id, at_tcpip_cfg.cycle_time, at_tcpip_cfg.msg_auto);
        } else
            goto error;
    } else {
        goto error;
    }

    if (str[0] != '\0')
        aic_transmit_resp_normal(at_req_resp_ptr, str);

    aic_transmit_resp_ok(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

error:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}


u32 handle_qisde_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QISDE: (0,1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 aic_get_send_echo_enable(void)
{
    return at_tcpip_cfg.sendecho;
}

u32 handle_qisde_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QISDE: %d", at_tcpip_cfg.sendecho);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qisde_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    u8 send_echo;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QISDE=",
                                             &cnt);
    if ( !paras_arr || cnt != 1) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        if(paras_arr)
            aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }
    send_echo = atoi((u8 *)paras_arr[0]);
    if (send_echo != 0 && send_echo != 1) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    at_tcpip_cfg.sendecho = send_echo;
    aic_transmit_resp_ok(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qping_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QPING: (1-5),<host>,(1-255),(1-10)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qping_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    struct addrinfo *res = NULL, *hints = NULL;
    NXD_ADDRESS destaddr;
    NX_IPV4_HEADER *ipv4_header;
    NX_IPV6_HEADER *ipv6_header;
    NXD_IPV6_ADDRESS *ipv6_addr = NULL;
    NX_PACKET *response_ptr;
    u8 pdp_type, pdp_cid, *at_ping_payload, first_flag = TRUE;
    int ret, packet_len = PING_DATA_SIZE_DEF, timeout = 1;
    int i = 0, count = 4, time, send_cnt = 0, recv_cnt = 0;
    u32 shortest_time = 0, longest_time = 0, average_time = 0;
    WAN_NET *net_ptr = NULL;
    NX_INTERFACE *infc;
    s8 at_ping_addr[PING_ADDR_BUFFERSIZE_IPV6] = {0};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    u32 addr4;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QPING=",
                                             &cnt);
    if ( !paras_arr || cnt < 2 || cnt > 4) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        if(paras_arr)
            aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto PING_ERROR;

    pdp_cid = atoi((u8 *)paras_arr[0]);
    if (pdp_cid < 1 || pdp_cid > ANET_MAX_PDP_CID){
        goto PING_ERROR;
    }

    net_ptr = aic_get_wan_net_per_pdp(pdp_cid);
    if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
        aic_free_paras_mem(paras_arr, cnt);
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    hints = aos_zalloc(sizeof(struct addrinfo));
    pdp_type = aic_get_pdp_ip_type(pdp_cid);
    if (pdp_type == ANET_DEFAULT_IP_TYPE_V4) {
        hints->ai_family = AF_INET;
    } else if (pdp_type == ANET_DEFAULT_IP_TYPE_V6) {
        while (i < PING_WAIT_TIME_MAX) {
            if (i != 0)
                aos_sleep(1000);

            if (aic_is_valid_ipv6_addr(net_ptr->wan_ipv6_address1))
                break;

            i++;
        }

        if (i == PING_WAIT_TIME_MAX) {
            aic_free_paras_mem(paras_arr, cnt);
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_NO_IPV6_PREFIX);
            aos_free(hints);
            return AIC_TCPIP_ERR_NO_IPV6_PREFIX;
        }
        hints->ai_family = AF_INET6;
    } else {
        hints->ai_family = AF_UNSPEC;
    }

    if (pdp_type != ANET_DEFAULT_IP_TYPE_V6) {
        if (inet_pton(AF_INET, (u8 *)paras_arr[1], &addr4) &&
            AIC_TRUE != aic_is_valid_ipv4_addr((u8 *)paras_arr[1])) {
            aic_free_paras_mem(paras_arr, cnt);
            aic_transmit_resp_normal(at_req_resp_ptr, "ERROR");
            aos_free(hints);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }
    }

    ret = getaddrinfo((u8 *)paras_arr[1], NULL, hints, &res);
    if (ret != NX_SUCCESS || !res) {
        aicx_error(0x045947, "sd", "[AT_PING]: get %s addr info fail ret %d\r\n",
                   (s8 *)paras_arr[1], ret);
        aic_free_paras_mem(paras_arr, cnt);
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_DNS_FAIL);
        aos_free(hints);
        return ret;
    }

    if(res->ai_addr->sa_family == AF_INET) {
        destaddr.nxd_ip_version = NX_IP_VERSION_V4;
        aos_memcpy(&destaddr.nxd_ip_address.v4, &res->ai_addr->sa_data[2], 4);
        NX_CHANGE_ULONG_ENDIAN(destaddr.nxd_ip_address.v4);
    } else {
        destaddr.nxd_ip_version = NX_IP_VERSION_V6;
        aos_memcpy(destaddr.nxd_ip_address.v6,
                   &(((struct sockaddr_in6*)res->ai_addr)->sin6_addr), 16);
        NX_CHANGE_ULONG_ENDIAN(destaddr.nxd_ip_address.v6[0]);
        NX_CHANGE_ULONG_ENDIAN(destaddr.nxd_ip_address.v6[1]);
        NX_CHANGE_ULONG_ENDIAN(destaddr.nxd_ip_address.v6[2]);
        NX_CHANGE_ULONG_ENDIAN(destaddr.nxd_ip_address.v6[3]);

        for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++) {
            /* Compare the address. */
            if ((net_ptr->wan_ip_instance->nx_ipv6_address[i].nxd_ipv6_address_valid) &&
                (CHECK_IPV6_ADDRESSES_SAME(net_ptr->wan_ip_instance->nx_ipv6_address[i].nxd_ipv6_address,
                                           net_ptr->wan_ipv6_address1))) {
                 /* Found a proper address. */
                 ipv6_addr = &(net_ptr->wan_ip_instance->nx_ipv6_address[i]);
            }
        }
    }

    if (cnt >= 3) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
            goto PING_ERROR;

        timeout = atoi((u8 *)paras_arr[2]);
        if (timeout < PING_TIMEOUT_MIN || timeout > PING_TIMEOUT_MAX) {
            aicx_info(0x031a10, "", "[AT] ping timeout error");
            goto PING_ERROR;
        }

        if (cnt == 4) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))
                goto PING_ERROR;
            count = atoi((u8 *)paras_arr[3]);
            if(count > AT_PING_COUNT_MAX || count < 1) {
                aicx_info(0x174308, "", "[AT] ping count error");
                goto PING_ERROR;
            }
        }
    }

    if (packet_len) {
        at_ping_payload = aos_malloc(packet_len);
        if (!at_ping_payload) {
            goto PING_ERROR;
        }
    }

    for (i = 0; i < packet_len; i++)
        *(at_ping_payload + i) = 'a' + i % 24;

    aic_transmit_resp_ok(at_req_resp_ptr);
    infc = &net_ptr->wan_ip_instance->nx_ip_interface[net_ptr->wan_ifc_index];
    while (count--) {
        if (first_flag) {
            first_flag = FALSE;
        } else {
            aos_sleep(1000);
        }

        time = aos_get_tickcount();
        send_cnt++;
        if (destaddr.nxd_ip_version == NX_IP_VERSION_V4) {
            ret = _nx_icmp_interface_ping(net_ptr->wan_ip_instance,
                                          destaddr.nxd_ip_address.v4,
                                          infc, destaddr.nxd_ip_address.v4,
                                          at_ping_payload, packet_len,
                                          &response_ptr,
                                          timeout * NX_IP_PERIODIC_RATE);
        } else {
            ret = _nx_icmp_interface_ping6(net_ptr->wan_ip_instance,
                                           &destaddr, at_ping_payload,
                                           packet_len, ipv6_addr,
                                           &response_ptr,
                                           timeout * NX_IP_PERIODIC_RATE);
        }

        if ((ret != NX_SUCCESS) || (response_ptr == NX_NULL)) {
            if (response_ptr)
                nx_packet_release(response_ptr);
            strcpy(str, "+QPING: TIMEOUT");
        } else {
            recv_cnt++;
            time = aos_get_tickcount() - time;
            if ((shortest_time > time) || !shortest_time)
                shortest_time = time;

            if (longest_time < time)
                longest_time = time;

            average_time += time;
            if(response_ptr->nx_packet_ip_version == NX_IP_VERSION_V4) {
                ipv4_header =
                    (NX_IPV4_HEADER *)response_ptr->nx_packet_ip_header;
                NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_source_ip);
                inet_ntop(AF_INET, &ipv4_header->nx_ip_header_source_ip,
                          at_ping_addr, PING_ADDR_BUFFERSIZE_IPV4);
                snprintf(str, sizeof(str), "+QPING: %d,\"%s\",%d,%d,%ld",
                         0, at_ping_addr, packet_len, time,
                         (ipv4_header->nx_ip_header_word_2 >> 24));
            } else if(response_ptr->nx_packet_ip_version == NX_IP_VERSION_V6){
                ipv6_header =
                    (NX_IPV6_HEADER *)response_ptr->nx_packet_ip_header;
                NX_CHANGE_ULONG_ENDIAN(ipv6_header->nx_ip_header_source_ip[0]);
                NX_CHANGE_ULONG_ENDIAN(ipv6_header->nx_ip_header_source_ip[1]);
                NX_CHANGE_ULONG_ENDIAN(ipv6_header->nx_ip_header_source_ip[2]);
                NX_CHANGE_ULONG_ENDIAN(ipv6_header->nx_ip_header_source_ip[3]);
                inet_ntop(AF_INET6, ipv6_header->nx_ip_header_source_ip,
                          at_ping_addr, PING_ADDR_BUFFERSIZE_IPV6);
                snprintf(str, sizeof(str), "+QPING: %d,\"%s\",%d",
                         0, at_ping_addr, time);
            } else {
                strcpy(str, "+PING: desttination address unreachable");
            }
            nx_packet_release(response_ptr);
        }
        aic_transmit_at_cmd_urc(at_req_resp_ptr->serial_port_id, str, strlen(str));
    }

    if (recv_cnt)
        average_time /= recv_cnt;

    snprintf(str, sizeof(str), "+QPING: %d,%d,%d,%d,%d,%d,%d",
             0, send_cnt, recv_cnt, (send_cnt - recv_cnt), shortest_time,
             longest_time, average_time);
    aic_transmit_at_cmd_urc(at_req_resp_ptr->serial_port_id, str, strlen(str));
    freeaddrinfo(res);
    if (at_ping_payload) {
        aos_free(at_ping_payload);
        at_ping_payload = AIC_NULL;
    }

    aos_free(hints);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
PING_ERROR:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    if (hints)
        aos_free(hints);
    if (res)
        freeaddrinfo(res);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 handle_qntp_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str),
             "+QNTP: (1-5),<server>,(1-65535),(0,1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qntp_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    u8 addr[ADDR_BUFFERSIZE_IPV6] = {'\0'};
    NX_SNTP_CLIENT *sntp_client;

    sntp_client = aic_get_sntp_client();
    if (!sntp_client) {
        aic_transmit_resp_error(at_req_resp_ptr, AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    if (aic_sntp_server_ip_ptr->aic_sntp_server_address.nxd_ip_version ==
        NX_IP_VERSION_V4) {
        NX_CHANGE_ULONG_ENDIAN(aic_sntp_server_ip_ptr->aic_sntp_server_address.nxd_ip_address.v4);
        inet_ntop(AF_INET,
                  &aic_sntp_server_ip_ptr->aic_sntp_server_address.nxd_ip_address.v4,
                  addr, ADDR_BUFFERSIZE_IPV4);
    } else {
        NX_CHANGE_ULONG_ENDIAN(aic_sntp_server_ip_ptr->aic_sntp_server_address.nxd_ip_address.v6[0]);
        NX_CHANGE_ULONG_ENDIAN(aic_sntp_server_ip_ptr->aic_sntp_server_address.nxd_ip_address.v6[1]);
        NX_CHANGE_ULONG_ENDIAN(aic_sntp_server_ip_ptr->aic_sntp_server_address.nxd_ip_address.v6[2]);
        NX_CHANGE_ULONG_ENDIAN(aic_sntp_server_ip_ptr->aic_sntp_server_address.nxd_ip_address.v6[3]);
        inet_ntop(AF_INET6,
                  aic_sntp_server_ip_ptr->aic_sntp_server_address.nxd_ip_address.v6,
                  addr, ADDR_BUFFERSIZE_IPV6);
    }

    snprintf(str, sizeof(str), "+QNTP: \"%s\",%d",addr, 123);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qntp_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    struct addrinfo *res = AIC_NULL;
    ulong *paras_arr;
    int ret, cid, auto_set;
    NX_SNTP_CLIENT *sntp_client;
    WAN_NET *net_ptr = NULL;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QNTP=",
                                             &cnt);
    if ( !paras_arr || cnt < 2 || cnt > 4) {
        aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        if(paras_arr)
            aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto ERROR_EXIT;

    cid = atoi((u8 *)paras_arr[0]);
    if (cid < 1 || cid > ANET_MAX_PDP_CID){
        aicx_info(0x1548ec, "", "[AT] pdp not activated");
        goto ERROR_EXIT;
    }

    if (cnt == 4) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))
            goto ERROR_EXIT;
        auto_set = atoi((u8 *)paras_arr[3]);
        if (auto_set != 0 && auto_set != 1)
            goto ERROR_EXIT;

        sntp_is_sync = auto_set;
    }

    net_ptr = aic_get_wan_net_per_pdp(cid);
    if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
        aic_free_paras_mem(paras_arr, cnt);
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    ret = getaddrinfo((u8 *)paras_arr[1], NULL, NULL, &res);
    if (ret != NX_SUCCESS || !res) {
        aicx_info(0x1d90b5, "sd", "[AT_NTP] get %s addr info fail %d", (u8 *)paras_arr[1], ret);
        aic_free_paras_mem(paras_arr, cnt);
        aic_transmit_resp_error(at_req_resp_ptr, AIC_TCPIP_ERR_DNS_FAIL);
        return ret;
    }

    sntp_client = aic_get_sntp_client();
    if (!sntp_client) {
        aic_free_paras_mem(paras_arr, cnt);
        freeaddrinfo(res);
        aic_transmit_resp_error(at_req_resp_ptr, AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    if(res->ai_addr->sa_family == AF_INET) {
        sntp_server_ip =
            ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
        sntp_client->nx_sntp_server_ip_address.nxd_ip_version =
            NX_IP_VERSION_V4;
        sntp_client->nx_sntp_unicast_time_server.nxd_ip_address.v4
            = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
        sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v4
            = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
        NX_CHANGE_ULONG_ENDIAN(sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v4);
    } else {
        sntp_server_address.nxd_ip_version = NX_IP_VERSION_V6;
        memcpy(&sntp_server_address.nxd_ip_address.v6,
               &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
        sntp_client->nx_sntp_server_ip_address.nxd_ip_version =
            NX_IP_VERSION_V6;
        memcpy(sntp_client->nx_sntp_unicast_time_server.nxd_ip_address.v6,
               &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
        memcpy(sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v6,
               &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
        NX_CHANGE_ULONG_ENDIAN(sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v6[0]);
        NX_CHANGE_ULONG_ENDIAN(sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v6[1]);
        NX_CHANGE_ULONG_ENDIAN(sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v6[2]);
        NX_CHANGE_ULONG_ENDIAN(sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v6[3]);
    }

    tx_event_flags_set(&nx_sntp_client_events, NX_SNTP_CLIENT_SEND_EVENT, TX_OR);
    sntp_client-> nx_sntp_client_retransmiss_times = 0;
    sntp_client->nx_sntp_server_update_time_active = 1;
    at_tcpip_cfg.com_id = at_req_resp_ptr->serial_port_id;
    aic_transmit_resp_ok(at_req_resp_ptr);
    freeaddrinfo(res);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 handle_qigeterror_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_qigeterror_exec_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    struct at_error_description *error_ptr;
    u8 *err_desc = NULL;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    error_ptr = at_error_description_table;
    while (error_ptr->num != AIC_TCPIP_ERR_MAX) {
        if (error_ptr->num == last_errcode) {
            err_desc = error_ptr->str;
            break;
        }

        error_ptr++;
    }

    if(error_ptr->num == AIC_TCPIP_ERR_MAX) {
        aic_transmit_resp_ok(at_req_resp_ptr);
        return AIC_TCPIP_AT_OK;
    }

    snprintf(str, sizeof(str), "+QIGETERROR: %d,%s", last_errcode, err_desc);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

static void aic_qat_socket_recv_notify(int sock, void *data)
{
    int i, len, len_t;
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};

    getsockopt(sock, SOL_SOCKET, SO_RXDATA, &len, &len_t);
    i = aic_at_find_index_by_socket(sock, AT_QTCPIP);
    if (i == AIC_EPERM)
        return;

    if (sock_list_q[i].mode == 1 || sock_list_q[i].mode == 2) {
        at_event_flag[i].recv_flag = 1;
        aos_set_event(at_socket_recv_event, SOCKET_RECV_EVENT, TX_OR);
    } else if (sock_list_q[i].recv_total == 0) {
        snprintf(result_str, AT_RESP_RECV_DATA_BUFFER,
                 "+QIURC: \"recv\",%d",
                 sock_list_q[i].link_id);
        aic_transmit_at_cmd_urc(at_tcpip_cfg.com_id,
            result_str, strlen(result_str));
    }
    sock_list_q[i].recv_total = len;
}

static void aic_qat_socket_tcp_disconn_notify(int sock, void *data, int is_active)
{
    int i;

    if (is_active == TRUE)
        return;

    i = aic_at_find_index_by_socket(sock, AT_QTCPIP);
    if (i == AIC_EPERM)
        return;

    at_event_flag[i].close_flag = 1;
    aos_set_event(at_socket_recv_event, SOCKET_CLOSE_EVENT, TX_OR);
}

static void aic_at_tcpip_socket_close(int i)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int sock;

    sock = aic_at_find_socket_by_linkid(i, AT_QTCPIP);
    snprintf(str, AT_RESP_RECV_DATA_BUFFER,
             "+QIURC: \"closed\",%d", sock_list_q[i].link_id);

    if (sock_list_q[i].socket_type == SOCK_TCP ||
        sock_list_q[i].socket_type == SOCK_TCPIN) {
        aic_socket_unregister_tcp_disconnect_notify(sock_list_q[i].socket_id);
    }
    aic_socket_unregister_receive_notify(sock_list_q[i].socket_id);
    soc_close(sock);

    aic_transmit_at_cmd_urc(at_tcpip_cfg.com_id, str, strlen(str));
    aic_end_at_data_mode(at_tcpip_cfg.com_id);
    sock_list_q[i].status = SOCKET_STATE_CLOSING;
}


static void aic_at_tcpip_socket_recv(int index)
{
    int sock, ret, fromaddrlen, len, len_t, strl;
    u8 *buffer = NULL, *str = NULL;
    struct sockaddr_in fromaddr;
    struct sockaddr_in6 fromaddr6;
    char addr[ADDR_BUFFERSIZE_IPV6];
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};
    u16 port;

    sock = sock_list_q[index].socket_id;
    if (sock == 0) {
        aicx_error(0x0e739e, "", "[AT_TCPIP] socket is already closed\n");
        return;
    }

    getsockopt(sock, SOL_SOCKET, SO_RXDATA, &len, &len_t);
    if (sock_list_q[index].mode == 1 || sock_list_q[index].mode == 2) {
RECV_AGAIN:
        buffer = aos_zalloc_relaxed(len + AT_RESP_RESULT_MAX);
        if (!buffer) {
            return;
        }

        if (sock_list_q[index].socket_type == SOCK_TCP ||
            sock_list_q[index].socket_type == SOCK_TCPv6) {
            ret = anet_recv(sock, buffer + AT_RESP_RESULT_MAX, len, MSG_DONTWAIT);
        } else {
            if (sock_list_q[index].family == AF_INET) {
                fromaddrlen = sizeof(struct sockaddr_in);
                ret = anet_recvfrom(sock, buffer + AT_RESP_RESULT_MAX, len,
                                    MSG_DONTWAIT, (struct sockaddr *)&fromaddr,
                                    &fromaddrlen);
            } else {
                fromaddrlen = sizeof(struct sockaddr_in6);
                ret = anet_recvfrom(sock, buffer + AT_RESP_RESULT_MAX, len,
                                    MSG_DONTWAIT, (struct sockaddr *)&fromaddr6,
                                    &fromaddrlen);
            }
        }
        if (ret > 0) {
            if (sock_list_q[index].family == AF_INET) {
                if (sock_list_q[index].socket_type != SOCK_UDP) {
                    fromaddrlen = sizeof(struct sockaddr_in);
                    anet_getpeername(sock,
                                     (struct sockaddr *)&fromaddr,
                                     &fromaddrlen);
                }

                inet_ntop(AF_INET, &fromaddr.sin_addr.s_addr,
                          addr, ADDR_BUFFERSIZE_IPV4);
                port = htons(fromaddr.sin_port);
            } else {
                if (sock_list_q[index].socket_type != SOCK_UDP) {
                    fromaddrlen = sizeof(struct sockaddr_in6);
                    anet_getpeername(sock,
                                     (struct sockaddr *)&fromaddr6,
                                     &fromaddrlen);
                }

                inet_ntop(AF_INET6, &fromaddr6.sin6_addr,
                          addr, ADDR_BUFFERSIZE_IPV6);
                port = htons(fromaddr6.sin6_port);
            }

            if (sock_list_q[index].mode == 1) {
                if (sock_list_q[index].socket_type == SOCK_UDPS)
                    snprintf(&result_str[0], AT_RESP_RESULT_MAX,
                             "\r\n+QIURC: \"recv\",%d,%d,\"%s\",%d\r\n",
                             sock_list_q[index].link_id, ret, addr,
                             port);
                else
                    snprintf(&result_str[0], AT_RESP_RESULT_MAX,
                             "\r\n+QIURC: \"recv\",%d,%d\r\n",
                             sock_list_q[index].link_id, ret);

                if (at_tcpip_cfg.recv_data_format == 0){
                    strl = strlen(result_str);
                    aos_memcpy((buffer + AT_RESP_RESULT_MAX - strl), result_str, strl);
                    aic_reply_to_certain_com(at_tcpip_cfg.com_id,
                                         (buffer + AT_RESP_RESULT_MAX - strl), strl + ret);
                } else {
                    str = aos_zalloc_relaxed(len * 2 + AT_RESP_RESULT_MAX);
                    if (!str) {
                        if (buffer)
                            aos_free(buffer);
                        return;
                    }
                    strl = strlen(result_str);
                    aos_memcpy(str, result_str, strl);
                    aic_print_data_to_hex((buffer + AT_RESP_RESULT_MAX), ret, (str + strl));
                    strl = strl + ret * 2;
                    aic_reply_to_certain_com(at_tcpip_cfg.com_id, str, strl);
                    aos_free(str);
                }
            } else {
                aic_transmit_at_data(at_tcpip_cfg.com_id,
                                     (buffer + AT_RESP_RESULT_MAX), ret);
            }
            sock_list_q[index].recv_total -= ret;
        } else {
            if (buffer)
                aos_free(buffer);
            return;
        }

        if (buffer)
            aos_free(buffer);

        if (len > ret) {
            len -= ret;
            goto RECV_AGAIN;
        }
    }
}

static void aic_at_tcpip_process_recv(void *pdata)
{
    u32 events = 0;
    int i, unrcv, len;

    while (1) {
        aos_get_event(at_socket_recv_event, SOCKET_RECV_EVENT_ALL, TX_OR_CLEAR,
                      &events, AOS_WAIT_FOREVER);
        aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
        if (events & SOCKET_RECV_EVENT) {
            aicx_info(0x1bb9c6, "", "[AT_TCPIP] recv event SOCKET_RECV_EVENT\n");
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
                if (sock_list_q[i].socket_id == 0)
                    continue;

                getsockopt(sock_list_q[i].socket_id, SOL_SOCKET, SO_RXDATA, &unrcv, &len);
                if (!at_event_flag[i].recv_flag && !unrcv)
                    continue;

                aic_at_tcpip_socket_recv(i);
                at_event_flag[i].recv_flag = 0;
            }
        }

        if (events & SOCKET_CLOSE_EVENT) {
            aicx_info(0x1bb9c6, "", "[AT_TCPIP] recv event SOCKET_RECV_EVENT\n");
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
                if (!at_event_flag[i].close_flag)
                    continue;

                aic_at_tcpip_socket_close(i);
                at_event_flag[i].close_flag = 0;
            }
        }
        aos_put_mutex(at_socket_mutex);
    }
}

int aic_at_recv_task_create(void)
{
    at_socket_recv_thread = aos_create_thread("AT Socket recv Thread",
                                       aic_at_tcpip_process_recv,
                                       NULL, SIZE_2K, 0, 0, 15, AOS_DONT_START);
    if (at_socket_recv_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x07733d, "", "[AT_TCPIP]: at socket recv thread create fail\n");
        return AIC_EFAULT;
    }

    at_socket_recv_event = aos_create_event("AT Socket recv event flags");
    if (!at_socket_recv_event) {
        aicx_error(0x10cd58, "", "[AT_TCPIP]: at socket recv event create fail\n");
        aos_delete_thread(at_socket_recv_thread);
        return AIC_EFAULT;
    }

    aos_resume_thread(at_socket_recv_thread);
    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_at_recv_task_create);

void aic_qat_socket_listen_thread_entry(void* thread_input)
{
    int sock, conn_sock, port, i, ret, fromaddr_len;
    struct sockaddr_in fromaddr;
    struct sockaddr_in6 fromaddr6;
    fd_set read_ready;
    struct timeval timeout;
    u8 addr[ADDR_BUFFERSIZE_IPV6] = {'\0'};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    //aos_suspend_thread(at_socket_listen_thread);

    FD_ZERO(&read_ready);
    //FD_SET(sock, &read_ready);

    ret = listen(aic_at_tcp_server.sockfd, AT_SOCKET_LISTEN_MAX);
    if (ret < 0) {
        aicx_error(0x097010, "", "[AT_TCPIP]: at socket listen fail\n");
        return;
    }

    while (1) {
        read_ready = aic_at_tcp_server.master_list;
        sock = aic_at_tcp_server.sockfd;
        aos_sleep(20);
        timeout.tv_sec = 0;
        timeout.tv_usec = NX_IP_PERIODIC_RATE * 1000;
        ret = select(FD_SETSIZE, &read_ready, 0, 0, &timeout);
        if (ret == ERROR || ret == 0) {
            continue;
        }

        if (FD_ISSET(sock, &read_ready)) {
            if (aic_at_tcp_server.family == AF_INET) {
                fromaddr_len = sizeof(fromaddr);
                conn_sock = accept(sock, (struct sockaddr *)&fromaddr,
                                   &fromaddr_len);
                inet_ntop(AF_INET, &fromaddr.sin_addr.s_addr, addr,
                          ADDR_BUFFERSIZE_IPV4);
                port = htons(fromaddr.sin_port);
            } else {
                fromaddr_len = sizeof(fromaddr6);
                conn_sock = accept(sock, (struct sockaddr *)&fromaddr6,
                                   &fromaddr_len);
                inet_ntop(AF_INET6, &fromaddr6.sin6_addr, addr,
                          ADDR_BUFFERSIZE_IPV6);
                port = htons(fromaddr6.sin6_port);
            }

            if (conn_sock > 0) {
                aic_socket_register_receive_notify(sock,
                                                   aic_qat_socket_recv_notify,
                                                   NULL);
                aic_socket_register_tcp_disconnect_notify(sock,
                                                          aic_qat_socket_tcp_disconn_notify,
                                                          NULL);
            } else {
                continue;
            }

            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
                if (sock_list_q[i].socket_id == 0) {
                    sock_list_q[i].link_id = i;
                    sock_list_q[i].socket_id = conn_sock;
                    sock_list_q[i].family = aic_at_tcp_server.family;
                    sock_list_q[i].socket_type = SOCK_TCPIN;
                    snprintf(str, sizeof(str),
                             "+QIURC: \"incoming\",%d,%d,%s,%d", i, sock,
                             addr, port);
                    aic_transmit_at_cmd_urc(at_tcpip_cfg.com_id, str,
                                            strlen(str));
                    break;
                }
            }
            if (i == AT_SOCKET_MAX_CONN_NUM_Q) {
                soc_close(conn_sock);
                snprintf(str, sizeof(str), "+QIURC: \"incoming full\"");
                aic_transmit_at_cmd_urc(at_tcpip_cfg.com_id, str, strlen(str));
            }
        }
    }
}

static void aic_qat_socket_listen_init(void)
{
    at_socket_listen_thread = aos_create_thread("AT Socket listen Thread",
                                  aic_qat_socket_listen_thread_entry,
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

static void aic_qat_socket_listen_uninit(void)
{
    aos_delete_thread(at_socket_listen_thread);
    aos_delete_event(at_socket_listen_event);
}

