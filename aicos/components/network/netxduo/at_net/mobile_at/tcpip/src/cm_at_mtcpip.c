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
#define AIC_THIS_FILE    0x001b89
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
#include "aic_at_mtcpip.h"
#include "aic_at_mtls.h"
#include "nx_icmpv6.h"
#include "nx_icmpv4.h"
#include "aic_net_api.h"
#include "aic_at_parse_utils.h"
#include "aic_at_resp_handle.h"
#include "aic_at_cmd_handle.h"
#include "aic_at_interact_cp.h"
#include "aic_net_pdp_mgr.h"
#include "connmgr_api.h"

#ifdef CONFIG_CRYPTO_MBEDTLS_2
#define AT_SOCKET_TASK_SIZE  SIZE_4K
#else
#define AT_SOCKET_TASK_SIZE  (SIZE_2K + SIZE_1K)
#endif

AIC_SOCKET_LIST sock_list[AT_SOCKET_MAX_CONN_NUM];
AT_TCPIP_CFG at_tcpip_cfg[AT_SOCKET_MAX_CONN_NUM];
struct at_sock_info sock_info = {0};
struct aic_at_listen_info listen_info[AT_SOCKET_MAX_CONN_NUM] = {0};
//aos_event_ptr at_listen_event;
struct aic_at_dnsinfo dnsinfo;
AT_PING_CFG at_ping_cfg;
int error_code;
struct at_dns_cfg dns_cfg = {1, AT_DNS_TIMEOUT_DEFAULT, AT_DNS_RETRY_CNT_DEFAULT};
aos_mutex_ptr at_socket_mutex = NULL;

static struct aic_at_stream_recv_info recv_info[AT_SOCKET_MAX_CONN_NUM] = {0};
static aos_thread_id at_socket_thread;
static aos_thread_id at_dns_thread;
static aos_thread_id at_ping_thread;
//static aos_thread_id at_listen_thread;
aos_event_ptr at_socket_event;
static aos_event_ptr at_ping_event;
static aos_event_ptr at_dns_event;

extern USHORT sntp_server_port;
extern int sntp_is_sync;
extern int sntp_sync_timeout;
extern NX_DNS *_nx_dns_instance_ptr;
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];
extern u32 sntp_server_ip;
extern NXD_ADDRESS sntp_server_address;
extern aos_event_ptr aic_sntp_client_event;
extern NX_BSD_SOCKET nx_bsd_socket_array[NX_BSD_MAX_SOCKETS];

static AT_CMD_HANDLE mtcpip_at_cmd[] = {
    {AT_IP_CFG, handle_mipcfg_test_cmd, AIC_NULL, handle_mipcfg_set_cmd, AIC_NULL},

    {AT_IP_TKA, handle_miptka_test_cmd, handle_miptka_query_cmd,
     handle_miptka_set_cmd, AIC_NULL},

    {AT_IP_OPEN, handle_mipopen_test_cmd, AIC_NULL, handle_mipopen_set_cmd, AIC_NULL},

    {AT_IP_CLOSE, handle_mipclose_test_cmd, AIC_NULL, handle_mipclose_set_cmd, AIC_NULL},

    {AT_IP_SEND, handle_mipsend_test_cmd, AIC_NULL, handle_mipsend_set_cmd, AIC_NULL},

    {AT_IP_READ, handle_miprd_test_cmd, AIC_NULL, handle_miprd_set_cmd, AIC_NULL},

    {AT_IP_MODE, handle_mipmode_test_cmd, AIC_NULL, handle_mipmode_set_cmd, AIC_NULL},

    {AT_IP_STATE, AIC_NULL, handle_mipstate_query_cmd, handle_mipstate_set_cmd, AIC_NULL},

    {AT_IP_SACK, handle_mipsack_test_cmd, AIC_NULL, handle_mipsack_set_cmd, AIC_NULL},

    {AT_IP_DNSCFG, handle_mdnscfg_test_cmd, AIC_NULL, handle_mdnscfg_set_cmd, AIC_NULL},

    {AT_IP_DNSGIP, handle_mdnsgip_test_cmd, AIC_NULL,handle_mdnsgip_set_cmd, AIC_NULL},

    {AT_IP_PING, handle_mping_test_cmd, AIC_NULL, handle_mping_set_cmd, AIC_NULL},

    {"+++", AIC_NULL, AIC_NULL, AIC_NULL, handle_transmit_mode_exit_cmd},

    {AT_IP_NTP, handle_mntp_test_cmd, AIC_NULL, handle_mntp_set_cmd, AIC_NULL},

    {AIC_NULL},
};

static void aic_tcpip_mat_cfg_init(void);
static void aic_mat_socket_autofree(int index);
static void aic_at_tcpip_socket_establish(int index);
static int aic_at_socket_send(int socket_id, char *data, int datalen, int link_id);
static void aic_mat_socket_recv_notify(int sock, void *data);
void aic_mat_socket_tcp_ack_notify(int sock, int len, void *data);
void aic_mat_socket_tcp_disconn_notify(int sock, void *data, int is_active);
void aic_mat_socket_tcp_connect_notify(int sock, u32 status, void *data);

extern NX_SNTP_CLIENT *aic_get_sntp_client(void);
extern u32 aic_sntp_client_get_time(CHAR * buffer, UINT length, int time_zone);
extern void aic_sntp_add_server_to_list(AIC_SNTP_SERVER_IP *aic_sntp_server);
extern void aic_sntp_del_all_server_from_list(void);
extern void aic_sntp_pdp_state_change_callback(u8 pdp_cid, u8 pdp_state);


int aic_tcpip_mat_cmd_init(void)
{
    aic_add_at_cmd_handle(mtcpip_at_cmd);
    aic_tcpip_mat_cfg_init();

    return AIC_TCPIP_AT_OK;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_tcpip_mat_cmd_init);

static void aic_tcpip_mat_cfg_init(void)
{
    int i;
    AT_TCPIP_CFG cfg;

    aos_memset(&cfg, 0, sizeof(AT_TCPIP_CFG));
    cfg.transpktsize = STRANS_SIZE_DEFAULT;
    cfg.transwaittm = STRANS_WAITTM_MAX;
    cfg.send_data_format = 0;
    cfg.recv_data_format = 0;
    cfg.send_timeout = 10;
    cfg.send_data_buffer = 1460;
    cfg.recv_data_buffer = AT_SOCKET_RCVBUF_MAX;
    cfg.connect_timeout = AT_SOCKET_CONN_TIMEOUT_DEFAULT;
    cfg.keepalive.keepalive_enabled = 0;
    cfg.keepalive.keepalive_init = AT_KEEPIDLE_DEFAULT;
    cfg.keepalive.keepalive_retry = AT_KEEPINTERVAL_DEFAULT;
    cfg.keepalive.keepalive_retries = AT_KEEPCOUNT_DEFAULT;

    aos_memset(sock_list, 0, sizeof(sock_list));
    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        sock_list[i].pdp_cid = ANET_DEFAULT_PDP_CID;
        aos_memcpy(&at_tcpip_cfg[i], &cfg, sizeof(AT_TCPIP_CFG));
    }

    //aic_set_dns_timeout(AT_DNS_TIMEOUT_DEFAULT, AT_DNS_RETRY_CNT_DEFAULT);
    at_socket_mutex = aos_create_mutex("at_socket_mutex");
    AIC_ASSERT(at_socket_mutex);
    /*Default use dns cache*/
    //aic_disable_dns_cache();
}

static void aic_get_socket_recv_bytes_info(int sockid, int *len)
{
    int unrecv, len_t, index;

    index = aic_at_find_index_by_socket(sockid, AT_MTCPIP);
    if (index < 0) {
        *len = 0;
        return;
    }

    getsockopt(sockid, SOL_SOCKET, SO_RXDATA, &unrecv, &len_t);
    if (sock_list[index].socket_type == SOCK_UDP) {
        if (recv_info[index].buf) {
            *len = unrecv + recv_info[index].len - recv_info[index].offset;
            return;
        }
    }

    *len = unrecv;
}

static void aic_get_socket_recv_packet_info(int sockid, int *count)
{
    NX_BSD_SOCKET   *bsd_socket_ptr;
    NX_PACKET       *packet_ptr;
    int             cnt = 0;

    if (sockid == 0) {
        *count = 0;
        return;
    }

    bsd_socket_ptr = &nx_bsd_socket_array[sockid - NX_BSD_SOCKFD_START];
    if (bsd_socket_ptr->nx_bsd_socket_udp_socket) {
        cnt = bsd_socket_ptr->nx_bsd_socket_udp_socket->nx_udp_socket_receive_count;
    }

    if (bsd_socket_ptr->nx_bsd_socket_tcp_socket)
        cnt = bsd_socket_ptr->nx_bsd_socket_tcp_socket->nx_tcp_socket_receive_queue_count;

    packet_ptr = bsd_socket_ptr->nx_bsd_socket_received_packet;
    while (packet_ptr) {
        cnt++;
        packet_ptr = packet_ptr->nx_packet_queue_next;
    }

    *count = cnt;
}

static void aic_mat_socket_recv_packet_cache_mode(int index, int com_id,
                                                  int ret, u8 *buf, u8 *hex_buf)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int sock, unrecv, count, len_t;

    sock = sock_list[index].socket_id;
    aic_get_socket_recv_bytes_info(sock, &unrecv);
    sock_list[index].recv_total = unrecv;
    sock_list[index].recv_cnt--;
    aic_get_socket_recv_packet_info(sock, &count);
    snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_READ "%d,%d,%d,", index, count,
             ret);
    len_t = strlen(str);
    if (at_tcpip_cfg[index].recv_data_format == 0) {
        aos_memcpy((buf + AT_SOCKET_RECV_BUF_OFFSET - len_t), str, len_t);
        aic_transmit_at_result(
            com_id, (buf + AT_SOCKET_RECV_BUF_OFFSET - len_t), ret + len_t);
    } else {
        aos_memcpy(hex_buf, str, len_t);
        aic_print_data_to_hex(buf + AT_SOCKET_RECV_BUF_OFFSET, ret,
                              hex_buf + len_t);
        aic_transmit_at_result(com_id, hex_buf, ret * 2 + len_t);
    }
    aos_memset(str, 0, AT_RESP_RESULT_MAX);
    if (buf)
        aos_free(buf);
    if (hex_buf)
        aos_free(hex_buf);
}

static void aic_mat_socket_recv_cache_mode(int index, int com_id, int ret,
                                           u8 *buf, u8 *hex_buf,
                                           int total_recv, u8 first)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int len_t;

    if (first) {
        str[0] = aic_get_cr_sign();
        str[1] = aic_get_lf_sign();
        snprintf(&str[2], (AT_RESP_RESULT_MAX - 2), RESP_IP_READ "%d,%d,%d,",
                 index, sock_list[index].recv_total - total_recv, total_recv);
        len_t = strlen(str);
    } else {
        len_t = 0;
    }
    sock_list[index].recv_total -= ret;
    if (at_tcpip_cfg[index].recv_data_format == 0) {
        if (first) {
            aos_memcpy((buf + AT_SOCKET_RECV_BUF_OFFSET - len_t), str, len_t);
        }
        if (com_id == conn_get_uart_at_conid)
            aic_send_to_at_port(
                com_id, (buf + AT_SOCKET_RECV_BUF_OFFSET - len_t), ret + len_t);
        else
            aic_transmit_at_data(
                com_id, (buf + AT_SOCKET_RECV_BUF_OFFSET - len_t), ret + len_t);
    } else {
        if (first) {
            aos_memcpy(hex_buf, str, len_t);
        }
        aic_print_data_to_hex(buf + AT_SOCKET_RECV_BUF_OFFSET, ret,
                              hex_buf + len_t);
        if (com_id == conn_get_uart_at_conid)
            aic_send_to_at_port(com_id, hex_buf, ret * 2 + len_t);
        else
            aic_transmit_at_data(com_id, hex_buf, ret * 2 + len_t);
    }


    if (buf)
        aos_free(buf);
    if (hex_buf)
        aos_free(hex_buf);

}

static void aic_mat_socket_send_transparent_mode(AT_REQ_RESP *at_req_resp_ptr)
{
    int index, len, ret;

    if (at_req_resp_ptr->end_data_mode) {
        index = aic_at_find_index_by_socket(sock_info.sockid, AT_MTCPIP);
        if (index >= 0) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_UNKNOW_ERR);
        }
        aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
        at_req_resp_ptr->at_mode = AT_CMD_MODE;
        return;
    }

    if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
        return;
    }

    index = aic_at_find_index_by_socket(sock_info.sockid, AT_MTCPIP);
    if (index < 0) {
        aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
        at_req_resp_ptr->at_mode = AT_CMD_MODE;
        return;
    }

    if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
        at_req_resp_ptr->at_mode = AT_CMD_MODE;
        sock_list[index].mode = 0;
        aos_memset(&sock_info, 0, sizeof(sock_info));
        aic_reply_ok_resp(at_req_resp_ptr);
        return;
    }

    len = at_req_resp_ptr->upstream_data_len;
    if (len > sock_info.length)
        len = sock_info.length;

    ret = aic_at_socket_send(sock_info.sockid,
                             at_req_resp_ptr->at_upstream_data, len, index);

    return;
}

void aic_mat_socket_tcp_ack_notify(int sock, int len, void *data)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int i, com_id;

    i = aic_at_find_index_by_socket(sock, AT_MTCPIP);
    if (i < 0) {
        return;
    }
    com_id = at_tcpip_cfg[i].com_id;

    if (at_tcpip_cfg[i].ack_mode) {
        snprintf(str, AT_RESP_RESULT_MAX, IP_URC"\"ack\",%d,%d", i, len);
        aic_transmit_at_cmd_urc(com_id, str, strlen(str));
    }
}

void aic_mat_socket_tcp_connect_notify(int sock, u32 status, void *data)
{
    int i;

    i = aic_at_find_index_by_socket(sock, AT_MTCPIP);
    if (i < 0) {
        return;
    }

    if (status == AIC_TRUE) {
        sock_list[i].status = SOCKET_CONNECT_SUCCESS;
        aos_set_event(at_socket_event, SOCKET_ESTABLISH_EVENT, TX_OR);
    } else {
        sock_list[i].status = SOCKET_CONNECT_FAIL;
        aos_set_event(at_socket_event, SOCKET_ESTABLISH_EVENT, TX_OR);
    }
}

static int aic_create_mtcp_socket(int family, struct sockaddr *addr,
                                  int linkid, struct sockaddr *srcaddr,
                                  int isclient)
{
    int i, sock, addrlen, ret;
    struct sock_winsize tcp_windows;

    sock = anet_socket(addr->sa_family, SOCK_STREAM, NX_PROTOCOL_TCP);
    if (sock == NX_SOC_ERROR) {
        error_code = TCPIP_SOCKET_CREATE_FAIL;
        return AIC_EFAULT;
    }

    if (family == AF_INET)
        addrlen = sizeof(struct sockaddr_in);
    else
        addrlen = sizeof(struct sockaddr_in6);

    if (srcaddr) {
        ret = anet_bind(sock, srcaddr, addrlen);
        if (ret < 0) {
            soc_close(sock);
            error_code = TCPIP_SOCKET_BIND_FAIL;
            return ret;
        }
    }

    if (at_tcpip_cfg[linkid].recv_data_buffer) {
        tcp_windows.winsize = at_tcpip_cfg[linkid].recv_data_buffer;
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &tcp_windows, sizeof(tcp_windows));
    }

    if (at_tcpip_cfg[linkid].keepalive.keepalive_enabled)
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
                   &at_tcpip_cfg[linkid].keepalive,
                   sizeof(struct sock_keepalive));

    ret = anet_fcntl(sock, F_SETFL, O_NONBLOCK);
    if (ret == NX_SOC_ERROR) {
        soc_close(sock);
        return AIC_EFAULT;
    }

    aic_socket_register_connect_notify(sock, aic_mat_socket_tcp_connect_notify, NULL);

    ret = anet_connect(sock, addr, addrlen);
    if (ret < 0) {
        if (errno != EINPROGRESS && errno != EALREADY) {
            error_code = TCPIP_SOCKET_CONNECT_FAIL;
            soc_close(sock);
            return ret;
        }
    }

    aic_socket_register_receive_notify(sock, aic_mat_socket_recv_notify, NULL);
    aic_socket_register_tcp_disconnect_notify(sock,
                                              aic_mat_socket_tcp_disconn_notify,
                                              NULL);
    aic_socket_register_tcp_ack_notify(sock, aic_mat_socket_tcp_ack_notify, NULL);

    i = linkid;
    if (sock_list[i].socket_id == 0) {
        sock_list[i].link_id = linkid;
        sock_list[i].socket_id = sock;
        sock_list[i].in_use = 1;
        sock_list[i].family = addr->sa_family;
        if (family == AF_INET) {
            sock_list[i].addr =
                ((struct sockaddr_in *)addr)->sin_addr.s_addr;
            sock_list[i].port =
                ((struct sockaddr_in *)addr)->sin_port;
        } else {
            aos_memcpy(sock_list[i].addr6,
                       &((struct sockaddr_in6 *)addr)->sin6_addr,
                       16);
            sock_list[i].port = ((struct sockaddr_in6 *)addr)->sin6_port;
        }
        sock_list[i].socket_type = SOCK_TCP;
    }

    return sock;
}

static int aic_mtcp_socket_recv(int index, int len, AT_REQ_RESP *at_req_resp_ptr)
{
    int sock, total_recv = 0, ret = 0, unrecv = 0, buf_len;
    int first = AIC_TRUE, com_id;  //len_t, count,
    u8 *buf = NULL, *hex_buf = NULL;
    u8 rcv_buf[4] = {'\0'};
    u8 str[4] = {'\r', '\n', '\0'};

    com_id = at_req_resp_ptr->serial_port_id;
    sock = sock_list[index].socket_id;
    aic_get_socket_recv_bytes_info(sock, &unrecv);
    if (len > unrecv)
        total_recv = unrecv;
    else
        total_recv = len;

    while (len > 0) {
        ret = recv(sock, rcv_buf, 0, MSG_DONTWAIT | MSG_PEEK);
        if (ret > 0) {
            buf_len = ret;
            buf = aos_zalloc_relaxed(buf_len + 50);
            if (!buf) {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                goto RD_EXIT;
            }

            if (at_tcpip_cfg[index].recv_data_format == 1) {
                hex_buf = aos_zalloc_relaxed(buf_len * 2 + AT_SOCKET_RECV_BUF_OFFSET);
                if (!hex_buf) {
                    aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                    goto RD_EXIT;
                }
            }
        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                aic_reply_ok_resp(at_req_resp_ptr);
                goto RD_EXIT;
            } else {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                goto RD_EXIT;
            }
        }

        if (sock_list[index].mode == 3)
            ret = recv(sock, (buf + AT_SOCKET_RECV_BUF_OFFSET),
                       buf_len, MSG_DONTWAIT);
        else
            ret = recv(sock, (buf + AT_SOCKET_RECV_BUF_OFFSET),
                       buf_len < len ? buf_len : len, MSG_DONTWAIT);

        if (ret < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                aic_reply_ok_resp(at_req_resp_ptr);
                goto RD_EXIT;
            } else {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                goto RD_EXIT;
            }
        } else {
            if (sock_list[index].mode == 3) {
                aic_mat_socket_recv_packet_cache_mode(index, com_id, ret, buf, hex_buf);
                len--;
                buf = AIC_NULL;
                hex_buf = AIC_NULL;
            } else {
                aic_mat_socket_recv_cache_mode(index, com_id, ret, buf,
                                               hex_buf, total_recv, first);
                if (first)
                    first = AIC_FALSE;

                len -= ret;
                buf = AIC_NULL;
                hex_buf = AIC_NULL;
                if (len <= 0) {
                    if (com_id == conn_get_uart_at_conid)
                        aic_send_to_at_port(com_id, str, 2);
                    else
                        aic_transmit_at_data(com_id, str, 2);
                    break;
                }
            }

            if (len == 0 && sock_list[index].mode == 3) {
                aic_reply_ok_resp(at_req_resp_ptr);
                goto RD_EXIT;
            }
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);
RD_EXIT:
    if (buf)
        aos_free(buf);
    if (hex_buf)
        aos_free(hex_buf);
    return AIC_TCPIP_AT_OK;
}

void aic_mat_socket_udp_drop_notify(int sock, int len, void *data)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int i, com_id;

    i = aic_at_find_index_by_socket(sock, AT_MTCPIP);
    if (i < 0)
        return;
    com_id = at_tcpip_cfg[i].com_id;

    snprintf(str, AT_RESP_RESULT_MAX, IP_URC"\"drop\",%d,%d", i, len);
    aic_transmit_at_cmd_urc(com_id, str, strlen(str));
}

static int aic_create_mudp_socket(int family, struct sockaddr *remoteaddr,
                                  int linkid, struct sockaddr *srcaddr,
                                  int isclient)
{
    int i, sock, ret, addrlen;
    struct sock_winsize udp_rcvbuf;
    struct timeval timeout;

    sock = anet_socket(remoteaddr->sa_family, SOCK_DGRAM, NX_PROTOCOL_UDP);
    if (sock == NX_SOC_ERROR) {
        error_code = TCPIP_SOCKET_CREATE_FAIL;
        return TCPIP_INVAL_AT_PARAM;
    }

    if (family == AF_INET)
        addrlen = sizeof(struct sockaddr_in);
    else
        addrlen = sizeof(struct sockaddr_in6);

    if (srcaddr) {
        ret = anet_bind(sock, srcaddr, addrlen);
        if (ret < 0) {
            soc_close(sock);
            error_code = TCPIP_SOCKET_BIND_FAIL;
            return ret;
        }
    }

    if (at_tcpip_cfg[linkid].recv_data_buffer) {
        udp_rcvbuf.winsize = at_tcpip_cfg[linkid].recv_data_buffer;
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &udp_rcvbuf, sizeof(udp_rcvbuf));
    }

    timeout.tv_sec = AT_SOCKET_SEND_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    aic_socket_register_receive_notify(sock, aic_mat_socket_recv_notify, NULL);
    aic_socket_register_udp_drop_notify(sock, aic_mat_socket_udp_drop_notify, NULL);

    i = linkid;
    if (sock_list[i].socket_id == 0) {
        sock_list[i].link_id = linkid;
        sock_list[i].socket_id = sock;
        sock_list[i].in_use = 1;
        sock_list[i].family = remoteaddr->sa_family;
        if (family == AF_INET) {
            sock_list[i].addr =
                ((struct sockaddr_in *)remoteaddr)->sin_addr.s_addr;
            sock_list[i].port =
                ((struct sockaddr_in *)remoteaddr)->sin_port;
        } else {
            aos_memcpy(sock_list[i].addr6,
                       &((struct sockaddr_in6 *)remoteaddr)->sin6_addr,
                       16);
            sock_list[i].port =
                ((struct sockaddr_in6 *)remoteaddr)->sin6_port;
        }
        if (isclient)
            sock_list[i].socket_type = SOCK_UDP;
        else
            sock_list[i].socket_type = SOCK_UDPS;
    }

    return sock;
}

static int aic_mudp_socket_send(int socket_id, char *data, int datalen)
{
    struct sockaddr_in destaddr;
    struct sockaddr_in6 destaddr6;
    int index, ret;

    index = aic_at_find_index_by_socket(socket_id, AT_MTCPIP);
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

static int aic_mudp_socket_recv(int index, int len, AT_REQ_RESP *at_req_resp_ptr)
{
    int sock, ret = 0, total_recv = 0, unrecv = 0, count, fromaddrlen;
    int buffer_len, first = AIC_TRUE, com_id;  // len_t,
    u8 *buf = NULL, *hex_buf = NULL;
    u8 rcv_buf[4] = {'\0'};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    struct sockaddr_in fromaddr;
    struct sockaddr_in6 fromaddr6;

    com_id = at_req_resp_ptr->serial_port_id;
    sock = sock_list[index].socket_id;
    aic_get_socket_recv_bytes_info(sock, &unrecv);
    if (len > unrecv)
        total_recv = unrecv;
    else
        total_recv = len;

    while (len >= 0) {
        if (sock_list[index].family == AF_INET) {
            fromaddrlen = sizeof(fromaddr);
            ret = recvfrom(sock, rcv_buf, 0, MSG_DONTWAIT|MSG_PEEK,
                           (struct sockaddr *)&fromaddr, &fromaddrlen);
        } else {
            fromaddrlen = sizeof(fromaddr6);
            ret = recvfrom(sock, rcv_buf, 0, MSG_DONTWAIT|MSG_PEEK,
                           (struct sockaddr *)&fromaddr6, &fromaddrlen);
        }
        if (sock_list[index].mode != 3 && recv_info[index].buf) {
            buffer_len = recv_info[index].len - recv_info[index].offset;
            ret = buffer_len;
        }

        if (ret >= 0) {
            buffer_len = ret;
            buf = aos_zalloc_relaxed(buffer_len + 50);
            if (!buf) {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                goto RD_EXIT;
            }

            if (at_tcpip_cfg[index].recv_data_format == 1) {
                hex_buf = aos_zalloc_relaxed(buffer_len * 2 + AT_SOCKET_RECV_BUF_OFFSET);
                if (!hex_buf) {
                    aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                    goto RD_EXIT;
                }
            }
        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                aic_reply_ok_resp(at_req_resp_ptr);
                goto RD_EXIT;
            } else {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                goto RD_EXIT;
            }
        }

        if (sock_list[index].mode != 3 && recv_info[index].buf) {
            if (len >= recv_info[index].len - recv_info[index].offset) {
                aos_memcpy((buf + AT_SOCKET_RECV_BUF_OFFSET),
                           recv_info[index].buf + recv_info[index].offset,
                           recv_info[index].len - recv_info[index].offset);
                ret = recv_info[index].len - recv_info[index].offset;
                aos_free(recv_info[index].buf);
                recv_info[index].buf = NULL;
                recv_info[index].len = 0;
                recv_info[index].offset = 0;
            } else {
                aos_memcpy((buf + AT_SOCKET_RECV_BUF_OFFSET),
                           recv_info[index].buf + recv_info[index].offset,
                           len);
                recv_info[index].offset += len;
                ret = len;
            }
            goto read_urc;
        }

        if (sock_list[index].family == AF_INET) {
            fromaddrlen = sizeof(fromaddr);
            ret = recvfrom(sock, (buf + AT_SOCKET_RECV_BUF_OFFSET),
                           buffer_len, MSG_DONTWAIT,
                           (struct sockaddr *)&fromaddr, &fromaddrlen);
        } else {
            fromaddrlen = sizeof(fromaddr6);
            ret = recvfrom(sock, (buf + AT_SOCKET_RECV_BUF_OFFSET),
                           buffer_len, MSG_DONTWAIT,
                           (struct sockaddr *)&fromaddr6, &fromaddrlen);
        }

read_urc:
        if (ret < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                aic_reply_ok_resp(at_req_resp_ptr);
                goto RD_EXIT;
            } else {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                goto RD_EXIT;
            }
        } else if (ret == 0) {
            if (sock_list[index].mode == 3) {
                sock_list[index].recv_cnt--;
                aic_get_socket_recv_packet_info(sock, &count);
                snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_READ"%d,%d,0,", index,
                         count);
                aic_transmit_at_result(com_id, str, strlen(str));
                aos_memset(str, 0, AT_RESP_RESULT_MAX);
                if (buf) {
                    aos_free(buf);
                    buf = AIC_NULL;
                }
                if (hex_buf) {
                    aos_free(hex_buf);
                    hex_buf = AIC_NULL;
                }
                len--;
            }
            if (len == 0 && sock_list[index].mode == 3) {
                aic_reply_ok_resp(at_req_resp_ptr);
                goto RD_EXIT;
            }
        } else {
            if (sock_list[index].mode == 3) {
                aic_mat_socket_recv_packet_cache_mode(index, com_id, ret, buf, hex_buf);
                buf = AIC_NULL;
                hex_buf = AIC_NULL;
                len--;
            } else {
                if (ret > len) {
                    recv_info[index].len = ret - len;
                    recv_info[index].buf = aos_zalloc_relaxed(recv_info[index].len);
                    if (!recv_info[index].buf) {
                        aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                        goto RD_EXIT;
                    }
                    aos_memcpy(recv_info[index].buf,
                               (buf + AT_SOCKET_RECV_BUF_OFFSET + len),
                               recv_info[index].len);
                    recv_info[index].offset = 0;
                    ret = len;
                }

                aic_mat_socket_recv_cache_mode(index, com_id, ret, buf, hex_buf, total_recv, first);
                if (first)
                    first = AIC_FALSE;

                len -= ret;
                buf = AIC_NULL;
                hex_buf = AIC_NULL;
                if (len <= 0) {
                    str[0] = aic_get_cr_sign();
                    str[1] = aic_get_lf_sign();
                    if (com_id == conn_get_uart_at_conid)
                        aic_send_to_at_port(com_id, str, 2);
                    else
                        aic_transmit_at_data(com_id, str, 2);
                    break;
                }
            }

            if (len == 0 && sock_list[index].mode == 3) {
                aic_reply_ok_resp(at_req_resp_ptr);
                goto RD_EXIT;
            }
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);
RD_EXIT:
    if (buf)
        aos_free(buf);
    if (hex_buf)
        aos_free(hex_buf);
    return AIC_TCPIP_AT_OK;
}

static int aic_at_socket_send(int socket_id, char *data, int datalen, int link_id)
{
    int ret;

    if (sock_list[link_id].socket_type == SOCK_UDP) {
        ret = aic_mudp_socket_send(socket_id, data, datalen);
    } else {
#if defined CONFIG_CM_SSL_AT || defined CONFIG_GX_SSL_AT
        if (at_tcpip_cfg[link_id].ssl_enable)
            ret = mbedtls_ssl_write((mbedtls_ssl_context *)sock_list[link_id].ssl,
                                    data, datalen);
        else
#endif
            ret = anet_send(socket_id, data, datalen, 0);
    }

    return ret;
}

u32 aic_at_data_convert(u8 mode, u8 *dst, u8 *data, u32 datalen, int *cnt)
{
    int actual_len;
    u32 status;

    switch (mode) {
    case 0:
        aos_memcpy(dst, data, datalen);
        actual_len = datalen;
        break;
    case 1:
        if (datalen % 2)
            return 0;
        status = aic_hexstring_to_char(dst, data, datalen);
        if (status == AIC_TCPIP_AT_ERR)
            return 0;
        actual_len = datalen/2;
        break;
    case 2:
        aic_escape_data_normal(dst, data, datalen, &actual_len);
        *cnt = 0;
        break;
    default:
        return 0;
    }

    return actual_len;
}
u32 handle_mipcfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_CFG"\"cid\",(0-5),(1-5)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_IP_CFG"\"encoding\",(0-5),(0-2),(0-1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_IP_CFG"\"timeout\",(0-5),(1-120)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_IP_CFG"\"autofree\",(0-5),(0-1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_IP_CFG"\"sndbuf\",(0-5),(1-1460)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_IP_CFG"\"rcvbuf\",(0-5),(1460-65536)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_IP_CFG"\"ackmode\",(0-5),(0-1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_IP_CFG"\"ssl\",(0-5),(0-1),(0-5)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mipcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int i, id = 0, str_len, num, num1;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    WAN_NET *net_ptr;
    u8 cmd[16] = {'\0'};
    AT_RESP_FROM_CP resp_to_save;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_IP_CFG"=",
                                             &cnt);
    if (!paras_arr) {
        goto EXIT;
    }

    if (cnt >= 2) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
            id = atoi((u8 *)paras_arr[1]);
            if (id >= AT_SOCKET_MAX_CONN_NUM) {
                goto EXIT;
            }
        } else {
            goto EXIT;
        }
    }

    str_len = strlen((u8*)paras_arr[0]);
    if (strncasecmp((u8*)paras_arr[0], "cid", 3) == 0 && str_len == 3) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT;
            num = atoi((u8*)paras_arr[2]);
            if (num > ANET_MAX_PDP_CID || num < 1)
                goto EXIT;

            net_ptr = aic_get_wan_net_per_pdp(num);
            if (!net_ptr) {
                snprintf(cmd, sizeof(cmd), "AT+CGACT=1,%d", num);
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
                aic_set_ipcall_act_flag(num);
#endif
                resp_to_save.resp_buf = aos_zalloc(AT_MAX_AP_RESP_LENGTH);
                resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
                aic_send_cmd_and_wait(cmd, &resp_to_save);
                if (strcasestr(resp_to_save.resp_buf, AT_RESULT_ERROR)) {
                    aos_sleep(5);
                    aic_send_cmd_and_wait(cmd, NULL);
                }

                aos_free(resp_to_save.resp_buf);
            }

            sock_list[id].pdp_cid = num;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_IP_CFG"\"cid\",%d,%d", id,
                         sock_list[id].pdp_cid);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                snprintf(str, sizeof(str), RESP_IP_CFG"\"cid\",%d,%d", i,
                         sock_list[i].pdp_cid);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "encoding", 8) == 0 &&
               str_len == 8) {
        if (cnt == 4) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT;
            num = atoi((u8*)paras_arr[2]);
            if (num > 2)
                goto EXIT;

            if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))
                goto EXIT;
            num1 = atoi((u8*)paras_arr[3]);
            if (num1 > 1)
                goto EXIT;

            at_tcpip_cfg[id].send_data_format = num;
            at_tcpip_cfg[id].recv_data_format = num1;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_IP_CFG"\"encoding\",%d,%d,%d", id,
                     at_tcpip_cfg[id].send_data_format,
                     at_tcpip_cfg[id].recv_data_format);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                snprintf(str, sizeof(str), RESP_IP_CFG"\"encoding\",%d,%d,%d", i,
                         at_tcpip_cfg[i].send_data_format,
                         at_tcpip_cfg[i].recv_data_format);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "timeout", 7) == 0 &&
               str_len == 7) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT;
            num = atoi((u8*)paras_arr[2]);
            if (num > AT_SOCKET_SEND_TIMEOUT_MAX || num < 1)
                goto EXIT;

            at_tcpip_cfg[id].send_timeout = num;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_IP_CFG"\"timeout\",%d,%d", id,
                     at_tcpip_cfg[id].send_timeout);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_IP_CFG"\"timeout\",%d,%d", i,
                         at_tcpip_cfg[i].send_timeout);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "autofree", 8) == 0 &&
               str_len == 8) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT;
            num = atoi((u8*)paras_arr[2]);
            if (num > 1)
                goto EXIT;

            at_tcpip_cfg[id].autofree = num;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_IP_CFG"\"autofree\",%d,%d", id,
                     at_tcpip_cfg[id].autofree);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                snprintf(str, sizeof(str), RESP_IP_CFG"\"autofree\",%d,%d", i,
                         at_tcpip_cfg[i].autofree);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "sndbuf", 6) == 0 &&
               str_len == 6) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT;
            num = atoi((u8*)paras_arr[2]);
            if (num < 1 || num > 1460)
                goto EXIT;

            at_tcpip_cfg[id].send_data_buffer = num;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_IP_CFG"\"sndbuf\",%d,%d", id,
                     at_tcpip_cfg[id].send_data_buffer);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_IP_CFG"\"sndbuf\",%d,%d", i,
                         at_tcpip_cfg[i].send_data_buffer);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "rcvbuf", 6) == 0 &&
               str_len == 6) {
        if (cnt == 3) {
            if (sock_list[id].socket_id != 0) {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_IS_USED);
                aic_free_paras_mem(paras_arr, cnt);
                return TCPIP_IS_USED;
            }

            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT;
            num = atoi((u8*)paras_arr[2]);
            if (num < AT_SOCKET_DATA_LEN_DEFAULT || num > AT_SOCKET_RCVBUF_MAX)
                goto EXIT;

            at_tcpip_cfg[id].recv_data_buffer = num;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_IP_CFG"\"rcvbuf\",%d,%d", id,
                     at_tcpip_cfg[id].recv_data_buffer);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_IP_CFG"\"rcvbuf\",%d,%d", i,
                         at_tcpip_cfg[i].recv_data_buffer);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ackmode", 7) == 0 &&
               str_len == 7) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT;

            num = atoi((u8*)paras_arr[2]);
            if (num > 1)
                goto EXIT;

            at_tcpip_cfg[id].ack_mode = num;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_IP_CFG"\"ackmode\",%d,%d", id,
                     at_tcpip_cfg[id].ack_mode);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                snprintf(str, sizeof(str), RESP_IP_CFG"\"ackmode\",%d,%d", i,
                         at_tcpip_cfg[i].ack_mode);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ssl", 3) == 0 &&
               str_len == 3) {
        if (cnt == 4) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT;
            num = atoi((u8*)paras_arr[2]);
#if defined(CONFIG_CM_SSL_AT) || defined(CONFIG_GX_SSL_AT)
            if (num > 1)
                goto EXIT;
#else
            if (num > 0)
                goto EXIT;
#endif
            if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))
                goto EXIT;
            num1 = atoi((u8*)paras_arr[3]);
            if (num1 >= SSL_CTX_ID_MAX)
                goto EXIT;

            at_tcpip_cfg[id].ssl_enable = num;
            at_tcpip_cfg[id].ssl_ctx_id = num1;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_IP_CFG"\"ssl\",%d,%d,%d", id,
                     at_tcpip_cfg[id].ssl_enable, at_tcpip_cfg[id].ssl_ctx_id);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                snprintf(str, sizeof(str), RESP_IP_CFG"\"ssl\",%d,%d,%d", i,
                         at_tcpip_cfg[i].ssl_enable, at_tcpip_cfg[i].ssl_ctx_id);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto EXIT;
        }
    } else {
        goto EXIT;
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
EXIT:
    aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return TCPIP_INVAL_AT_PARAM;
}

u32 handle_miptka_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_TKA"(0-5),(0-1),(30-7200),(30-600),(1-9)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_miptka_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int i;

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        if (at_tcpip_cfg[i].keepalive.keepalive_enabled)
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_TKA"%d,1,%d,%d,%d", i,
                     at_tcpip_cfg[i].keepalive.keepalive_init,
                     at_tcpip_cfg[i].keepalive.keepalive_retry,
                     at_tcpip_cfg[i].keepalive.keepalive_retries);
        else
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_TKA"%d,0", i);

        aic_transmit_resp_normal(at_req_resp_ptr, str);
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_miptka_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int sock, link_id = 0, num;
    struct sock_keepalive keepalive = {0, AT_KEEPIDLE_DEFAULT,
                                       AT_KEEPINTERVAL_DEFAULT,
                                       AT_KEEPCOUNT_DEFAULT};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_IP_TKA"=",
                                             &cnt);

    if ( !paras_arr || cnt < 1 || cnt > 5) {
        goto ERROR_EXIT;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto ERROR_EXIT;

    link_id = atoi((u8 *)paras_arr[0]);
    if (link_id >= AT_SOCKET_MAX_CONN_NUM  || link_id < 0) {
        goto ERROR_EXIT;
    }

    if (cnt == 1) {
        if (at_tcpip_cfg[link_id].keepalive.keepalive_enabled)
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_TKA"%d,1,%d,%d,%d", link_id,
                     at_tcpip_cfg[link_id].keepalive.keepalive_init,
                     at_tcpip_cfg[link_id].keepalive.keepalive_retry,
                     at_tcpip_cfg[link_id].keepalive.keepalive_retries);
        else
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_TKA"%d,0", link_id);

        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_reply_ok_resp(at_req_resp_ptr);
        goto EXIT;
    }

    if (cnt >= 2 && strlen((u8 *)paras_arr[1])) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            goto ERROR_EXIT;

        num = atoi((u8 *)paras_arr[1]);
        if (num != 0 && num != 1) {
            goto ERROR_EXIT;
        }

        keepalive.keepalive_enabled = num;
    }

    if (cnt >= 3 && strlen((u8 *)paras_arr[2])) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
            goto ERROR_EXIT;

        num = atoi((u8 *)paras_arr[2]);
        if (num < AT_KEEPIDLE_MIN || num > AT_KEEPIDLE_MAX)
            goto ERROR_EXIT;
        else
            keepalive.keepalive_init = num;
    }

    if (cnt >= 4 && strlen((u8 *)paras_arr[3])) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))
            goto ERROR_EXIT;

        num = atoi((u8 *)paras_arr[3]);
        if (num < AT_KEEPINTERVAL_MIN || num > AT_KEEPINTERVAL_MAX)
            goto ERROR_EXIT;
        else
            keepalive.keepalive_retry = num;
    }

    if (cnt >= 5 && strlen((u8 *)paras_arr[4])) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4])))
            goto ERROR_EXIT;

        num = atoi((u8 *)paras_arr[4]);
        if (num < AT_KEEPCOUNT_MIN || num > AT_KEEPCOUNT_MAX)
            goto ERROR_EXIT;
        else
            keepalive.keepalive_retries = num;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_MTCPIP);
    if (sock || sock_list[link_id].socket_type == SOCK_UDP) {
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_IS_USED);
        goto EXIT;
    }

    at_tcpip_cfg[link_id].keepalive.keepalive_enabled = keepalive.keepalive_enabled;
    if (keepalive.keepalive_init)
        at_tcpip_cfg[link_id].keepalive.keepalive_init = keepalive.keepalive_init;
    if (keepalive.keepalive_retry)
        at_tcpip_cfg[link_id].keepalive.keepalive_retry = keepalive.keepalive_retry;
    if (keepalive.keepalive_retries)
        at_tcpip_cfg[link_id].keepalive.keepalive_retries = keepalive.keepalive_retries;

    aic_reply_ok_resp(at_req_resp_ptr);
    goto EXIT;

ERROR_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    return TCPIP_INVAL_AT_PARAM;
}

u32 handle_mipopen_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str),
        RESP_IP_OPEN"(0-5),\"TCP/UDP\",\"host\",(0-65535),(1-180),(0-3),(0-65535)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mipopen_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int link_id = 0, ret, cid, mode = 0, port = 0;
    WAN_NET *net_ptr;

    /* AT mode */
    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 AT_IP_OPEN"=",
                                                 &cnt);

        if ( !paras_arr || cnt < 4 || cnt > 7) {
            goto ERR_EXIT;
        }

        if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
            goto ERR_EXIT;
        }

        link_id = atoi((u8 *)paras_arr[0]);
        if (link_id >= AT_SOCKET_MAX_CONN_NUM || link_id < 0) {
            goto ERR_EXIT;
        }

        if (listen_info[link_id].port != 0) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_IS_USED);
            goto OPEN_EXIT;
        }

        if (sock_list[link_id].in_use) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_IS_USED);
            goto OPEN_EXIT;
        }

        if (aic_at_find_socket_by_linkid(link_id, AT_MTCPIP)) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_IS_USED);
            goto OPEN_EXIT;
        }

        if (strlen((u8 *)paras_arr[2]) > 255) {
            goto ERR_EXIT;
        }

        if (cnt >= 4 && strlen((u8 *)paras_arr[3])) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))   {
                goto ERR_EXIT;
            }

            port = atoi((u8 *)paras_arr[3]);
            if (port > 65535) {
                goto ERR_EXIT;
            }
        }

        at_tcpip_cfg[link_id].connect_timeout = AT_SOCKET_CONN_TIMEOUT_DEFAULT;
        if (cnt >= 5 && strlen((u8 *)paras_arr[4])) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4])))   {
                goto ERR_EXIT;
            }

            at_tcpip_cfg[link_id].connect_timeout = atoi((u8 *)paras_arr[4]);
            if (at_tcpip_cfg[link_id].connect_timeout < AT_SOCKET_CONN_TIMEOUT_MIN ||
                at_tcpip_cfg[link_id].connect_timeout > AT_SOCKET_CONN_TIMEOUT_MAX) {
                goto ERR_EXIT;
            }
        }

        sock_list[link_id].mode = mode;
        if (cnt >= 6 && strlen((u8 *)paras_arr[5])) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[5], strlen((u8 *)paras_arr[5]))) {
                mode = atoi((u8 *)paras_arr[5]);

                if (mode < 0 || mode > 3) {
                    goto ERR_EXIT;
                } else {
                    sock_list[link_id].mode = mode;
                }
            } else {
                goto ERR_EXIT;
            }
        }

        if (cnt == 7 && strlen((u8 *)paras_arr[6])) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[6], strlen((u8 *)paras_arr[6])))   {
                goto ERR_EXIT;
            }

            port = atoi((u8 *)paras_arr[6]);
            if (port > 65535) {
                goto ERR_EXIT;
            }
        } else {
            port = NX_ANY_PORT;
        }

        net_ptr = aic_get_wan_net_per_pdp(sock_list[link_id].pdp_cid);
        if (!net_ptr) {
            cid = sock_list[link_id].pdp_cid;
            aos_memset(&sock_list[link_id], 0, sizeof(struct SOCKET_LIST));
            sock_list[link_id].pdp_cid = cid;
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_PDP_NOT_ACTIVE);
            goto OPEN_EXIT;
        }

        if (strcasecmp((u8 *)paras_arr[1], "TCP") == 0 && strlen((u8 *)paras_arr[1]) == 3) {
            listen_info[link_id].type = SOCK_TCP;
        } else if (strcasecmp((u8 *)paras_arr[1], "UDP") == 0 && strlen((u8 *)paras_arr[1]) == 3) {
            listen_info[link_id].type = SOCK_UDP;
        } else {
            goto ERR_EXIT;
        }

        listen_info[link_id].host = aos_zalloc_relaxed(AT_HOST_NAME_LEN_MAX + 1);
        if (!listen_info[link_id].host) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_CONNECT_FAIL);
            goto OPEN_EXIT;
        }

        if (mode == 1) {
            ret = aic_set_pre_data_mode(at_req_resp_ptr);
            if (ret != AIC_SUCCESS) {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_PORT_BUSY);
                goto OPEN_EXIT;
            }
        }

        strncpy(listen_info[link_id].host, (u8 *)paras_arr[2],
                strlen((u8 *)paras_arr[2]));
        listen_info[link_id].port = atoi((u8 *)paras_arr[3]);
        listen_info[link_id].src_port = port;
        listen_info[link_id].link_id = link_id;
        listen_info[link_id].timestamp = aos_get_tickcount();

        //aos_set_event(at_listen_event, 1 << link_id, TX_OR);
        aos_set_event(at_socket_event, SOCKET_ESTABLISH_EVENT, TX_OR);
        at_tcpip_cfg[link_id].com_id = at_req_resp_ptr->serial_port_id;
        aic_reply_ok_resp(at_req_resp_ptr);
        goto OPEN_EXIT;

ERR_EXIT:
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
OPEN_EXIT:
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        aic_mat_socket_send_transparent_mode(at_req_resp_ptr);
        return TCPIP_INVAL_AT_PARAM;
    } else {
        return AIC_TCPIP_AT_OK;
    }
}

u32 handle_mipclose_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_CLOSE"(0-5),(0-3)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mipclose_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    int link_id = 0, i, mode = 0, cid;
    ulong *paras_arr;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_IP_CLOSE"=",
                                             &cnt);
    if (!paras_arr || cnt < 1 || cnt > 2) {
        goto CLOSE_EXIT;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        goto CLOSE_EXIT;
    }
    link_id = atoi((u8 *)paras_arr[0]);
    if (link_id >= AT_SOCKET_MAX_CONN_NUM || link_id < 0) {
        goto CLOSE_EXIT;
    }

    if (cnt == 2) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
            goto CLOSE_EXIT;
        }
        mode = atoi((u8 *)paras_arr[1]);
        if (mode > 3 || mode < 0) {
            goto CLOSE_EXIT;
        }
    }

    i = link_id;
    if (sock_list[i].socket_id == 0 || sock_list[i].status == 0) {
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_NOT_USE);
        aic_free_paras_mem(paras_arr, cnt);
        return TCPIP_NOT_USE;
    }
    aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
#if defined CONFIG_CM_SSL_AT || defined CONFIG_GX_SSL_AT
    if (sock_list[i].ssl) {
        aic_mat_close_mbedtls_socket(sock_list[i].socket_id);
    } else
#endif
    {
        soc_close(sock_list[i].socket_id);
    }
    aic_socket_unregister_tcp_disconnect_notify(sock_list[i].socket_id);
    aic_socket_unregister_receive_notify(sock_list[i].socket_id);
    aic_socket_unregister_tcp_ack_notify(sock_list[i].socket_id);
    cid = sock_list[i].pdp_cid;
    if (recv_info[i].buf) {
        aos_free(recv_info[i].buf);
        recv_info[i].buf = NULL;
        recv_info[i].len = 0;
        recv_info[i].offset = 0;
    }
    aos_memset(&sock_list[i], 0, sizeof(struct SOCKET_LIST));
    sock_list[i].pdp_cid = cid;
    aos_put_mutex(at_socket_mutex);

    snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_CLOSE"%d", link_id);
    /*
    if (mode == 1)
        snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_CLOSE"%d", link_id);
    else
        snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_CLOSE"%d,%d", link_id, 0);*/

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_transmit_at_cmd_urc(at_req_resp_ptr->serial_port_id, str, strlen(str));
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
CLOSE_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return TCPIP_INVAL_AT_PARAM;
}

u32 handle_mipsend_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_SEND"(0-5),(0-1460),,(0-2),(1-255)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mipsend_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index = 0, len, link_id = 0, ret, count, offset = 0, times = 0;
    ulong *paras_arr;
    u16 cnt = 0, str_len;
    u32 actual_len;
    u8 *dst = NULL;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_at_get_para_array(at_req_resp_ptr->at_req, AT_IP_SEND"=", &cnt, 3, 3);
        if (!paras_arr || cnt < 1 || cnt > 3) {
            goto ERR_EXIT;
        }

        if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
            goto ERR_EXIT;
        }

        link_id = atoi((u8 *)paras_arr[0]);
        if (link_id >= AT_SOCKET_MAX_CONN_NUM || link_id < 0) {
            goto ERR_EXIT;
        }

        sock_info.sockid = aic_at_find_socket_by_linkid(link_id, AT_MTCPIP);
        if (!sock_info.sockid){
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_NOT_USE);
            goto SEND_EXIT;
        }

        sock_info.length = AT_SOCKET_DATA_LEN_DEFAULT;
        sock_info.offset = 0;
        if (cnt >= 2 && strlen((u8 *)paras_arr[1])) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
                goto ERR_EXIT;
            }
            sock_info.length = atoi((u8 *)paras_arr[1]);
            if (sock_info.length == 0 && cnt == 2) {
                sock_info.length = AT_SOCKET_DATA_LEN_DEFAULT;
            } else if (sock_info.length > AT_SOCKET_DATA_LEN_MAX) {
                goto ERR_EXIT;
            }
        } else if (cnt >= 2 && !strlen((u8 *)paras_arr[1])) {
            sock_info.length = 0;
        }

        if (cnt <= 2) {
            at_req_resp_ptr->at_mode = AT_DATA_MODE;
            strcpy(str, ">");
            if (sock_info.length > SIZE_32K)
                aic_set_data_mode_expire_time(AT_IP_SEND,
                    AT_SOCKET_SEND_TIMEOUT_MAX * NX_IP_PERIODIC_RATE);
            else
                aic_set_data_mode_expire_time(AT_IP_SEND,
                    at_tcpip_cfg[link_id].send_timeout * NX_IP_PERIODIC_RATE);

            aic_transmit_resp_normal(at_req_resp_ptr, str);
            goto SEND_EXIT;
        }

        len = strlen((u8 *)paras_arr[2]);
        if (len) {
            dst = aos_zalloc_relaxed(len);
            if (!dst) {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_SEND_FAIL);
                goto SEND_EXIT;
            }
            actual_len = aic_at_data_convert(at_tcpip_cfg[link_id].send_data_format,
                                             dst, (u8 *)paras_arr[2], len, &count);
            switch (at_tcpip_cfg[link_id].send_data_format) {
            case 1:
                if (len % 2 || (sock_info.length != 0 && (sock_info.length * 2) != len)
                    || actual_len == 0) {
                    goto ERR_EXIT;
                }
                break;
            case 2:
                if (sock_info.length != 0 && sock_info.length != len) {
                    goto ERR_EXIT;
                }
                break;
            case 0:
                if (sock_info.length != 0 && sock_info.length != len) {
                    goto ERR_EXIT;
                } else if (sock_info.length == 0 && actual_len > AT_SOCKET_DATA_LEN_DEFAULT) {
                    goto ERR_EXIT;
                }
                break;
            default:
                goto ERR_EXIT;
            }
            aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
send_agin:
            ret = aic_at_socket_send(sock_info.sockid, dst + offset,
                                     actual_len - offset, link_id);
        } else {
            if (sock_list[link_id].socket_type == SOCK_UDP) {
                ret = aic_at_socket_send(sock_info.sockid, "", 0, link_id);
                if (ret < 0) {
                    aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_SEND_FAIL);
                }
            }
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_SEND"%d,0", link_id);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_reply_ok_resp(at_req_resp_ptr);
            goto SEND_EXIT;
        }

        if (ret < 0) {
            if (errno == EAGAIN) {
                goto send_agin;
            }
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_SEND_FAIL);
        } else {
            offset += ret;
            if (offset < actual_len) {
                goto send_agin;
            }
            aos_free(dst);
            if (at_req_resp_ptr->serial_port_id == conn_get_uart_at_conid) {
                str[0] = aic_get_cr_sign();
                str[1] = aic_get_lf_sign();
                snprintf(&str[2], AT_RESP_RESULT_MAX - 2, RESP_IP_SEND"%d,%d", link_id, offset);
                str_len = strlen(str);
                str[str_len] = aic_get_cr_sign();
                str[str_len + 1] = aic_get_lf_sign();
                aic_send_to_at_port(at_tcpip_cfg[index].com_id, str, strlen(str));
                snprintf(&str[2], AT_RESP_RESULT_MAX - 2, "OK");
                str[4] = aic_get_cr_sign();
                str[5] = aic_get_lf_sign();
                aic_send_to_at_port(at_tcpip_cfg[index].com_id, str, 6);
            } else {
                snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_SEND"%d,%d", link_id, offset);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
                aic_reply_ok_resp(at_req_resp_ptr);
            }
        }
        aos_put_mutex(at_socket_mutex);
        goto SEND_EXIT;
ERR_EXIT:
        if (dst)
            aos_free(dst);
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
SEND_EXIT:
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        index = aic_at_find_index_by_socket(sock_info.sockid, AT_MTCPIP);
        if (index < 0) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_SEND_FAIL);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return TCPIP_INVAL_AT_PARAM;
        }

        if (at_req_resp_ptr->end_data_mode) {
            if (sock_info.offset) {
                len = sock_info.offset;
                aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
                goto SEND;
            } else {
                snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_SEND"%d,0", index);
            }
            aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
            //aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_SEND_FAIL);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_reply_ok_resp(at_req_resp_ptr);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == AIC_NULL) {
            return AIC_TCPIP_AT_ERR;
        }

        len = at_req_resp_ptr->upstream_data_len;
        if (len > AT_SOCKET_DATA_LEN_MAX) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    TCPIP_INVAL_AT_PARAM);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return TCPIP_INVAL_AT_PARAM;
        } else if (len > sock_info.length && sock_info.length != 0xFFFF
                   && sock_info.length != 0) {
            len = sock_info.length;
        } else if (sock_info.length == 0) {
            sock_info.length = AT_SOCKET_DATA_LEN_DEFAULT;
        }

        if (sock_info.length == 0xFFFF) {
            if (!sock_info.buf)
                sock_info.buf = aos_zalloc_relaxed(AT_SOCKET_DATA_LEN_MAX);
        } else {
            if (!sock_info.buf)
                sock_info.buf = aos_zalloc_relaxed(sock_info.length);
        }
        if (!sock_info.buf) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_SEND_FAIL);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return TCPIP_INVAL_AT_PARAM;
        }

        if (sock_info.length == AT_SOCKET_DATA_LEN_DEFAULT) {
            if (*at_req_resp_ptr->at_upstream_data == 0x1B) {
                at_req_resp_ptr->at_mode = AT_CMD_MODE;
                sock_list[index].mode = 0;
                aos_free(sock_info.buf);
                aos_memset(&sock_info, 0, sizeof(struct at_sock_info));
                snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_SEND"%d,0", index);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
                aic_reply_ok_resp(at_req_resp_ptr);
                return AIC_TCPIP_AT_OK;
            }
            if (*at_req_resp_ptr->at_upstream_data == 0x1A) {
                len = sock_info.offset;
                aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
                goto SEND;
            }

            if (AT_SOCKET_DATA_LEN_DEFAULT - sock_info.offset <= len) {
                len = AT_SOCKET_DATA_LEN_DEFAULT - sock_info.offset;
                aos_memcpy(sock_info.buf + sock_info.offset,
                           at_req_resp_ptr->at_upstream_data, len);
                sock_info.offset += len;
                len = sock_info.offset;
                aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
                goto SEND;
            }

            aos_memcpy(sock_info.buf + sock_info.offset,
                       at_req_resp_ptr->at_upstream_data, len);
            sock_info.offset += len;
            return AIC_TCPIP_AT_OK;
        } else {
            if (sock_info.length - sock_info.offset <= len)
                len = sock_info.length - sock_info.offset;
            aos_memcpy(sock_info.buf + sock_info.offset,
                       at_req_resp_ptr->at_upstream_data, len);
            sock_info.offset += len;
            if (sock_info.length == sock_info.offset) {
                len = sock_info.offset;
                aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
                goto SEND;
            }

            return AIC_TCPIP_AT_OK;
        }

SEND:
        if (len == 0 && sock_list[index].socket_type == SOCK_TCP)
            ret = 0;
        else
            ret = aic_at_socket_send(sock_info.sockid, sock_info.buf + offset,
                                     len - offset, index);
        if (ret < 0) {
            // if (errno == EAGAIN) {
            //     aos_sleep(100);
            //     goto SEND;
            // }

            aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_SEND_FAIL);
        } else {
            offset += ret;
            if (offset < len) {
                aos_sleep(100);
                times++;
                if (times < 3)
                    goto SEND;
            }
            if (at_req_resp_ptr->serial_port_id == conn_get_uart_at_conid) {
                str[0] = aic_get_cr_sign();
                str[1] = aic_get_lf_sign();
                snprintf(&str[2], AT_RESP_RESULT_MAX - 2, RESP_IP_SEND"%d,%d", index, offset);
                str_len = strlen(str);
                str[str_len] = aic_get_cr_sign();
                str[str_len + 1] = aic_get_lf_sign();
                aic_send_to_at_port(at_tcpip_cfg[index].com_id, str, strlen(str));
                snprintf(&str[2], AT_RESP_RESULT_MAX - 2, "OK");
                str[4] = aic_get_cr_sign();
                str[5] = aic_get_lf_sign();
                aic_send_to_at_port(at_tcpip_cfg[index].com_id, str, 6);
            } else {
                snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_SEND"%d,%d", index, offset);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
                aic_reply_ok_resp(at_req_resp_ptr);
            }
        }
        aos_put_mutex(at_socket_mutex);
        at_req_resp_ptr->at_mode = AT_CMD_MODE;
        aos_free(sock_info.buf);
        sock_info.buf = NULL;
        return AIC_TCPIP_AT_OK;
    } else {
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

u32 handle_miprd_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_READ"(0-5),(0-65536)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_miprd_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int sock, len, link_id = 0, unrecv = 0, count;
    ulong *paras_arr;
    u16 cnt;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_IP_READ"=",
                                             &cnt);
    if (!paras_arr || cnt < 1 || cnt > 2) {
        goto ERR_EXIT;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        goto ERR_EXIT;
    }

    if (cnt == 1) {
        link_id = atoi((u8 *)paras_arr[0]);
        if (sock_list[link_id].socket_id == 0) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_NOT_USE);
            goto RD_EXIT;
        }
        if (link_id < AT_SOCKET_MAX_CONN_NUM || link_id < 0) {
            if (sock_list[link_id].mode != 3) {
                aic_get_socket_recv_bytes_info(sock_list[link_id].socket_id,
                                               &unrecv);
                snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_READ"%d,%d",
                         link_id, unrecv);
            } else {
                aic_get_socket_recv_packet_info(sock_list[link_id].socket_id,
                                                &count);
                snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_READ"%d,%d",
                         link_id, count);
            }
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else {
            goto ERR_EXIT;
        }
    } else {
        link_id = atoi((u8 *)paras_arr[0]);
        if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
            goto ERR_EXIT;
        }

        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
            goto RD_EXIT;
        }
        len = atoi((u8 *)paras_arr[1]);
        if (len > AT_SOCKET_RCVBUF_MAX) {
            goto ERR_EXIT;
        }

        aic_get_socket_recv_bytes_info(sock_list[link_id].socket_id, &unrecv);
        aic_get_socket_recv_packet_info(sock_list[link_id].socket_id, &count);

        if (len == 0) {
            if (sock_list[link_id].socket_id == 0) {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_NOT_USE);
                goto RD_EXIT;
            }
            len = unrecv;
            if (sock_list[link_id].mode == 3) {
                len = count;
            }
        }

        sock = aic_at_find_socket_by_linkid(link_id, AT_MTCPIP);
        if (sock == 0) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_NOT_USE);
            goto RD_EXIT;
        }
        if (sock_list[link_id].mode == 3) {
            if (len > count)
                len = count;
        } else {
            if (len > unrecv)
                len = unrecv;
        }
        if (sock_list[link_id].socket_type == SOCK_TCP &&
            !at_tcpip_cfg[link_id].ssl_enable) {
            aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
            aic_mtcp_socket_recv(link_id, len, at_req_resp_ptr);
            aos_put_mutex(at_socket_mutex);
            goto RD_EXIT;
        } else if (sock_list[link_id].socket_type == SOCK_UDP) {
            aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
            aic_mudp_socket_recv(link_id, len, at_req_resp_ptr);
            aos_put_mutex(at_socket_mutex);
            goto RD_EXIT;
        } else
#if defined CONFIG_CM_SSL_AT || defined CONFIG_GX_SSL_AT
        if (sock_list[link_id].socket_type == SOCK_TCP &&
            at_tcpip_cfg[link_id].ssl_enable) {
            aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
            aic_mat_mbedtls_socket_recv(link_id, len, at_req_resp_ptr);
            aos_put_mutex(at_socket_mutex);
            goto RD_EXIT;
        } else
#endif
        {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_NOT_USE);
            goto RD_EXIT;
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    goto RD_EXIT;

ERR_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
RD_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mipmode_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_MODE"(0-5),(0-3),(512-1460),(100-2000)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mipmode_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int len, sock, link_id = 0, ret = 0, mode = 0, paras;
    ulong *paras_arr;
    u16 cnt;
    int unrecv, fromaddrlen, count, offset = 0;
    u8 *buf = NULL;
    struct sockaddr_in fromaddr;
    struct sockaddr_in6 fromaddr6;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 AT_IP_MODE"=",
                                                 &cnt);
        if (!paras_arr || cnt < 1 || cnt > 4) {
            goto ERR_EXIT;
        }

        if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
            goto ERR_EXIT;
        }
        link_id = atoi((u8 *)paras_arr[0]);
        if (link_id >= AT_SOCKET_MAX_CONN_NUM || link_id < 0) {
            goto ERR_EXIT;
        }

        sock = aic_at_find_socket_by_linkid(link_id, AT_MTCPIP);
        if (sock == 0) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_NOT_USE);
            goto MODE_EXIT;
        }
        if (cnt == 1) {
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_MODE"%d,%d", link_id,
                     sock_list[link_id].mode);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_reply_ok_resp(at_req_resp_ptr);
            goto MODE_EXIT;
        }

        if (cnt >= 2) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
                goto ERR_EXIT;
            }
            mode = atoi((u8 *)paras_arr[1]);
            if (mode < 0 || mode > 3) {
                goto ERR_EXIT;
            } else {
                if ((sock_list[link_id].mode == 3 && mode == 2) ||
                    (sock_list[link_id].mode == 2 && mode == 3)) {
                    aic_reply_error_resp(at_req_resp_ptr, TCPIP_UNKNOW_ERR);
                    goto MODE_EXIT;
                }
            }
        }

        if (mode == 1) {
            if (cnt >= 3) {
                if (!aic_is_valid_integer_para((u8 *)paras_arr[2],
                                               strlen((u8 *)paras_arr[2]))) {
                    goto ERR_EXIT;
                }
                paras = atoi((u8 *)paras_arr[2]);
                if (paras < STRANS_SIZE_MIN || paras > STRANS_SIZE_MAX) {
                    goto ERR_EXIT;
                } else {
                    at_tcpip_cfg[link_id].transpktsize = paras;
                }
            }

            if (cnt == 4) {
                if (!aic_is_valid_integer_para((u8 *)paras_arr[3],
                                                strlen((u8 *)paras_arr[3]))) {
                    goto ERR_EXIT;
                }
                paras = atoi((u8 *)paras_arr[3]);
                if (paras < STRANS_WAITTM_MIN || paras > STRANS_WAITTM_MAX) {
                    goto ERR_EXIT;
                } else {
                    at_tcpip_cfg[link_id].transwaittm = paras;
                }
            }

            sock_info.sockid = sock;
            sock_info.length = at_tcpip_cfg[link_id].transpktsize;
            at_req_resp_ptr->at_mode = AT_DATA_MODE;
        } else {
            if (cnt > 2) {
                aic_reply_error_resp(at_req_resp_ptr,
                                        TCPIP_INVAL_AT_PARAM);
                goto MODE_EXIT;
            }
        }

        if (mode == 0 || mode == 1) {
            aic_get_socket_recv_bytes_info(sock_list[link_id].socket_id,
                                           &unrecv);

            buf = aos_zalloc_relaxed(unrecv + 50);
            if (!buf) {
                aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
                goto MODE_EXIT;
            }

rcv_again:
            if (unrecv > 0) {
                if (sock_list[link_id].socket_type == SOCK_TCP) {
#if defined CONFIG_CM_SSL_AT || defined CONFIG_GX_SSL_AT
                    if (at_tcpip_cfg[link_id].ssl_enable)
                        ret = mbedtls_ssl_read((mbedtls_ssl_context *)sock_list[link_id].ssl,
                                               (buf + 50 + offset), unrecv);
                    else
#endif
                        ret = recv(sock, (buf + 50 + offset), unrecv, MSG_DONTWAIT);
                } else {
                    if (recv_info[link_id].buf) {
                        aos_memcpy((buf + 50), recv_info[link_id].buf + recv_info[link_id].offset,
                                   recv_info[link_id].len - recv_info[link_id].offset);
                        ret = recv_info[link_id].len - recv_info[link_id].offset;
                        aos_free(recv_info[link_id].buf);
                        recv_info[link_id].buf = NULL;
                        recv_info[link_id].len = 0;
                        recv_info[link_id].offset = 0;
                    } else if (sock_list[link_id].family == AF_INET) {
                        fromaddrlen = sizeof(fromaddr);
                        ret = recvfrom(sock, (buf + 50 + offset), unrecv, MSG_DONTWAIT,
                                       (struct sockaddr *)&fromaddr, &fromaddrlen);
                    } else {
                        fromaddrlen = sizeof(fromaddr6);
                        ret = recvfrom(sock,  (buf + 50 + offset), unrecv, MSG_DONTWAIT,
                                       (struct sockaddr *)&fromaddr6, &fromaddrlen);
                    }
                }

                if (ret >= 0) {
                    if (sock_list[link_id].mode == 2) {
                        aic_get_socket_recv_bytes_info(sock_list[link_id].socket_id,
                                                       &unrecv);
                        offset += ret;
                        if (unrecv) {
                            goto rcv_again;
                        }
                        ret = offset;
                        snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_READ"%d,%d,%d,",
                                 link_id, unrecv, ret);
                        len = strlen(str);
                        aos_memcpy((buf + 50 - len), str, len);
                        sock_list[link_id].recv_total = unrecv;
                        aic_transmit_at_result(at_req_resp_ptr->serial_port_id,
                                             (buf + 50 - len), ret + len);
                    } else {
                        aic_get_socket_recv_bytes_info(sock_list[link_id].socket_id,
                                                       &unrecv);
                        sock_list[link_id].recv_total = unrecv;
                        aic_get_socket_recv_packet_info(sock_list[link_id].socket_id,
                                                        &count);
                        sock_list[link_id].recv_cnt = count;
                        snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_READ"%d,%d,%d,",
                                 link_id, count, ret);
                        len = strlen(str);
                        aos_memcpy((buf + 50 - len), str, len);
                        aic_transmit_at_result(at_req_resp_ptr->serial_port_id,
                                             (buf + 50 - len), ret + len);
                        aos_memset(buf + 50 - len, 0, ret + len);
                        if (count) {
                            goto rcv_again;
                        }
                    }
                }
            }
            aos_free(buf);
        }

        sock_list[link_id].mode = mode;
        aic_reply_ok_resp(at_req_resp_ptr);

        if (mode == 1) {
            snprintf(str, AT_RESP_RESULT_MAX, "CONNECT");
            // aic_set_data_mode_expire_time(AT_IP_MODE,
            //                               at_tcpip_cfg[link_id].transwaittm * NX_IP_PERIODIC_RATE);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        }
        goto MODE_EXIT;
ERR_EXIT:
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
MODE_EXIT:
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        aic_mat_socket_send_transparent_mode(at_req_resp_ptr);
        return AIC_TCPIP_AT_OK;
    } else {
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

void aic_get_socket_state(int link_id, struct socket_state *state)
{
    int i;

    i = link_id;
    if (sock_list[i].family == AF_INET)
        inet_ntop(AF_INET, &sock_list[i].addr, state->addr, ADDR_BUFFERSIZE_IPV4);
    else if (sock_list[i].family == AF_INET6)
        inet_ntop(AF_INET6, sock_list[i].addr6, state->addr, ADDR_BUFFERSIZE_IPV6);

    state->remote_port = htons(sock_list[i].port);

    if (sock_list[i].socket_type == SOCK_TCP) {
        strcpy(state->socket_type, "TCP");
        if (sock_list[i].autofree)
            strcpy(state->socket_state, "CLOSED");
        else
            strcpy(state->socket_state, "CONNECTED");
    } else if (sock_list[i].socket_type == SOCK_UDP) {
        strcpy(state->socket_type, "UDP");
        strcpy(state->socket_state, "CONNECTED");
    } else {
        strcpy(state->socket_state, "INITIAL");
    }
}

u32 handle_mipstate_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int i;
    struct socket_state state;

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        aos_memset(&state, 0, sizeof(struct socket_state));
        aic_get_socket_state(i, &state);
        snprintf(str, sizeof(str), RESP_IP_STATE"%d,\"%s\",\"%s\",%d,\"%s\"", i,
                 state.socket_type, state.addr, state.remote_port,
                 state.socket_state);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mipstate_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    int link_id, i;
    struct socket_state state;

    if (strncasecmp(at_req_resp_ptr->at_req, AT_IP_STATE"=",
                    strlen(AT_IP_STATE"=")) == 0) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 AT_IP_STATE"=",
                                                 &cnt);
        if (!paras_arr || cnt != 1) {
            goto ERR_EXIT;
        }

        if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
            goto ERR_EXIT;
        }
        link_id = atoi((u8 *)paras_arr[0]);
        if (link_id >= AT_SOCKET_MAX_CONN_NUM || link_id < 0) {
            goto ERR_EXIT;
        }

        aos_memset(&state, 0, sizeof(struct socket_state));
        aic_get_socket_state(link_id, &state);
        if (strncmp(state.socket_state, "INITIAL", 7) == 0)
            snprintf(str, sizeof(str), RESP_IP_STATE"%d,,,,\"INITIAL\"", link_id);
        else
            snprintf(str, sizeof(str), RESP_IP_STATE"%d,\"%s\",\"%s\",%d,\"%s\"", link_id,
                     state.socket_type, state.addr, state.remote_port,
                     state.socket_state);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_reply_ok_resp(at_req_resp_ptr);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    } else {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
            if (sock_list[i].socket_id == 0)
                continue;

            aos_memset(&state, 0, sizeof(struct socket_state));
            aic_get_socket_state(i, &state);
            snprintf(str, sizeof(str), RESP_IP_STATE"%d,\"%s\",\"%s\",%d,\"%s\"", i,
                     state.socket_type, state.addr, state.remote_port,
                     state.socket_state);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        }
        aic_reply_ok_resp(at_req_resp_ptr);
        return AIC_TCPIP_AT_OK;
    }

ERR_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return TCPIP_INVAL_AT_PARAM;
}

u32 handle_mipsack_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_SACK"(0-5)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

static void aic_get_socket_send_info(int sockid, int *sent, int *acked, int *unack)
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
        *acked = 0;
        *unack = *sent;
    }
}

static void aic_get_socket_recv_info(int sockid, int *total_recv)
{
    NX_BSD_SOCKET   *bsd_socket_ptr;

    bsd_socket_ptr = &nx_bsd_socket_array[sockid - NX_BSD_SOCKFD_START];
    if (bsd_socket_ptr->nx_bsd_socket_tcp_socket) {
        *total_recv = bsd_socket_ptr->nx_bsd_socket_tcp_socket->nx_tcp_socket_bytes_received;
    }

    if (bsd_socket_ptr->nx_bsd_socket_udp_socket) {
        *total_recv = bsd_socket_ptr->nx_bsd_socket_udp_socket->nx_udp_socket_bytes_received;
    }
}

u32 handle_mipsack_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    int link_id, sock;
    int sent = 0, acked = 0, unack = 0, total_recv = 0;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_IP_SACK"=",
                                             &cnt);
    if (!paras_arr || cnt != 1) {
        goto ACK_EXIT;
    }

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        goto ACK_EXIT;
    }

    link_id = atoi((u8 *)paras_arr[0]);
    if (link_id >= AT_SOCKET_MAX_CONN_NUM || link_id < 0) {
        goto ACK_EXIT;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_MTCPIP);
    if (!sock) {
        aic_free_paras_mem(paras_arr, cnt);
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_NOT_USE);
        return TCPIP_NOT_USE;
    }
    aic_get_socket_send_info(sock, &sent, &acked, &unack);
    aic_get_socket_recv_info(sock, &total_recv);

    snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_SACK "%d,%d,%d,%d", sent, acked,
             unack, total_recv);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

ACK_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return TCPIP_INVAL_AT_PARAM;
}

u32 handle_mdnscfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_DNSCFG"\"ip\",,");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    snprintf(str, sizeof(str), RESP_IP_DNSCFG"\"ipv6\",,");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    snprintf(str, sizeof(str), RESP_IP_DNSCFG"\"priority\",(0,1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    snprintf(str, sizeof(str), RESP_IP_DNSCFG"\"cached\",(0,1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    snprintf(str, sizeof(str), RESP_IP_DNSCFG"\"timeout\",(1-60),(1-9)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mdnscfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ret, i, str_len, prio, num, times = DNS_RETRIES_DEFAULT;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    char addr[ADDR_BUFFERSIZE_IPV6] = {0};
    int addr6[4], addr4;
    u8 *dns1, *dns2;
    NX_DNS *dns_ptr;
    NXD_ADDRESS dns_server_address = {0};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_IP_DNSCFG"=",
                                             &cnt);
    if (!paras_arr) {
        goto ERROR_EXIT;
    }

    dns_ptr = aic_get_dns_client();
    if (dns_ptr == AIC_NULL) {
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_PDP_NOT_ACTIVE);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    }

    str_len = strlen((u8*)paras_arr[0]);
    if (strncasecmp((u8*)paras_arr[0], "ip", 2) == 0 && str_len == 2) {
        if (cnt == 1) {
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_DNSCFG"\"ip\"");
            for (i = 0; i < NX_DNS_MAX_SERVERS; i++) {
                ret = nxd_dns_server_get(dns_ptr, i, &dns_server_address);
                if (ret == NX_SUCCESS &&
                    dns_server_address.nxd_ip_version == NX_IP_VERSION_V4) {
                    NX_CHANGE_ULONG_ENDIAN(dns_server_address.nxd_ip_address.v4);
                    inet_ntop(AF_INET, &dns_server_address.nxd_ip_address.v4,
                              addr, ADDR_BUFFERSIZE_IPV4);
                    strcat(str, ",\"");
                    strcat(str, addr);
                    strcat(str, "\"");
                    aos_memset(&dns_server_address, 0, sizeof(NXD_ADDRESS));
                }
            }

            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_reply_ok_resp(at_req_resp_ptr);
        } else {
            if (cnt == 2) {
                if (!strlen((u8 *)paras_arr[1])) {
                    ret = aic_set_dns_server_ip(ANET_IP_TYPE_V4, AT_DNS1_SERVER_V4,
                                                AT_DNS2_SERVER_V4);
                    if (ret != AIC_SUCCESS) {
                        goto ERROR_EXIT;
                    }
                }

                if (!inet_pton(AF_INET, (u8 *)paras_arr[1], &addr4)) {
                    goto ERROR_EXIT;
                }
                ret = aic_set_dns_server_ip(ANET_IP_TYPE_V4, (u8 *)paras_arr[1],
                                            AT_DNS2_SERVER_V4);
                if (ret != AIC_SUCCESS) {
                    goto ERROR_EXIT;
                }
            }

            if (cnt == 3) {
                if (!strlen((u8 *)paras_arr[1])) {
                    dns1 = AT_DNS1_SERVER_V4;
                } else {
                    if (!inet_pton(AF_INET, (u8 *)paras_arr[1], &addr4)) {
                        goto ERROR_EXIT;
                    }
                    dns1 = (u8 *)paras_arr[1];
                }

                if (!strlen((u8 *)paras_arr[2])) {
                    dns2 = AT_DNS2_SERVER_V4;
                } else {
                    if (!inet_pton(AF_INET, (u8 *)paras_arr[2], &addr4)) {
                        goto ERROR_EXIT;
                    }
                    dns2 = (u8 *)paras_arr[2];
                }

                ret = aic_set_dns_server_ip(ANET_IP_TYPE_V4, dns1, dns2);
                if (ret != AIC_SUCCESS) {
                    goto ERROR_EXIT;
                }
            }
            aic_reply_ok_resp(at_req_resp_ptr);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ipv6", 4) == 0 && str_len == 4) {
        if (cnt == 1) {
            int count = 0;
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_DNSCFG"\"ipv6\"");
            for (i = 0; i < NX_DNS_MAX_SERVERS; i++) {
                ret = nxd_dns_server_get(dns_ptr, i, &dns_server_address);
                if (ret == NX_SUCCESS &&
                    dns_server_address.nxd_ip_version == NX_IP_VERSION_V6) {
                    NX_CHANGE_ULONG_ENDIAN(dns_server_address.nxd_ip_address.v6[0]);
                    NX_CHANGE_ULONG_ENDIAN(dns_server_address.nxd_ip_address.v6[1]);
                    NX_CHANGE_ULONG_ENDIAN(dns_server_address.nxd_ip_address.v6[2]);
                    NX_CHANGE_ULONG_ENDIAN(dns_server_address.nxd_ip_address.v6[3]);
                    inet_ntop(AF_INET6, dns_server_address.nxd_ip_address.v6,
                              addr, ADDR_BUFFERSIZE_IPV6);
                    strcat(str, ",\"");
                    strcat(str, addr);
                    strcat(str, "\"");
                    aos_memset(&dns_server_address, 0, sizeof(NXD_ADDRESS));
                    count++;
                }
            }

            if (count == 1) {
                strcat(str, ",\"2001:4860:4860::8888\"");
            }

            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_reply_ok_resp(at_req_resp_ptr);
        } else {
            if (cnt == 2) {
                if (!strlen((u8 *)paras_arr[1])) {
                    ret = aic_set_dns_server_ip(ANET_IP_TYPE_V6, "2400:3200::1",
                                                AT_DNS2_SERVER_V6);
                    if (ret != AIC_SUCCESS) {
                        goto ERROR_EXIT;
                    }
                }

                if (!inet_pton(AF_INET6, (u8 *)paras_arr[1], addr6)) {
                    goto ERROR_EXIT;
                }
                ret = aic_set_dns_server_ip(ANET_IP_TYPE_V6, (u8 *)paras_arr[1],
                                            AT_DNS2_SERVER_V6);
                if (ret != AIC_SUCCESS) {
                    goto ERROR_EXIT;
                }
            }

            if (cnt == 3) {
                if (!strlen((u8 *)paras_arr[1])) {
                    dns1 = AT_DNS1_SERVER_V6;
                } else {
                    if (!inet_pton(AF_INET6, (u8 *)paras_arr[1], addr6)) {
                        goto ERROR_EXIT;
                    }
                    dns1 = (u8 *)paras_arr[1];
                }

                if (!strlen((u8 *)paras_arr[2])) {
                    dns2 = AT_DNS2_SERVER_V6;
                } else {
                    if (!inet_pton(AF_INET6, (u8 *)paras_arr[2], addr6)) {
                        goto ERROR_EXIT;
                    }
                    dns2 = (u8 *)paras_arr[2];
                }

                ret = aic_set_dns_server_ip(ANET_IP_TYPE_V6, dns1, dns2);
                if (ret != AIC_SUCCESS) {
                    goto ERROR_EXIT;
                }
            }
            aic_reply_ok_resp(at_req_resp_ptr);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "priority", 8) == 0 && str_len == 8) {
        if (cnt == 1) {
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_DNSCFG"\"priority\",%d", dns_cfg.dns_prio);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_reply_ok_resp(at_req_resp_ptr);
        } else {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
                goto ERROR_EXIT;
            }
            prio = atoi((u8 *)paras_arr[1]);
            if (prio != 0 && prio != 1) {
                goto ERROR_EXIT;
            }

            dns_cfg.dns_prio = prio;
            aic_reply_ok_resp(at_req_resp_ptr);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "cached", 6) == 0 && str_len == 6) {
        if (cnt == 1) {
            num = aic_is_dns_cache_enabled();
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_DNSCFG"\"cached\",%d", !num);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_reply_ok_resp(at_req_resp_ptr);
        } else {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
                goto ERROR_EXIT;
            }
            num = atoi((u8 *)paras_arr[1]);
            switch (num) {
            case 0:
                aic_enable_dns_cache();
                break;
            case 1:
                aic_disable_dns_cache();
                break;
            default:
                goto ERROR_EXIT;
            }
            aic_reply_ok_resp(at_req_resp_ptr);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "timeout", 7) == 0 && str_len == 7) {
        if (cnt == 1) {
            snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_DNSCFG"\"timeout\",%d,%d",
                     dns_cfg.timeout, dns_cfg.retry_times);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            aic_reply_ok_resp(at_req_resp_ptr);
        } else {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
                goto ERROR_EXIT;
            }
            num = atoi((u8 *)paras_arr[1]);
            if (num < DNS_TIMEOUT_MIN || num > DNS_TIMEOUT_MAX)
                goto ERROR_EXIT;

            if (cnt == 3) {
                if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                    goto ERROR_EXIT;
                }
                times = atoi((u8 *)paras_arr[2]);
                if (times < DNS_RETRIES_MIN || times > DNS_RETRIES_MAX)
                    goto ERROR_EXIT;
            }

            dns_cfg.timeout = num;
            dns_cfg.retry_times = times;
            aic_reply_ok_resp(at_req_resp_ptr);
        }
    } else {
        goto ERROR_EXIT;
    }

    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

ERROR_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mdnsgip_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_DNSGIP"\"domainname\",(1-5)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mdnsgip_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;
    u8 pdp_cid;
    WAN_NET *net_ptr = NULL;
    int err_code;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_IP_DNSGIP"=",
                                             &cnt);
    if (!paras_arr || cnt < 1 || cnt > 2) {
        err_code = TCPIP_INVAL_AT_PARAM;
        goto ERR_EXIT;
    }

    if (cnt == 2){
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
            err_code = TCPIP_INVAL_AT_PARAM;
            goto ERR_EXIT;
        }
        pdp_cid = atoi((u8 *)paras_arr[1]);
        if (pdp_cid < 1 || pdp_cid > ANET_MAX_PDP_CID) {
            err_code = TCPIP_INVAL_AT_PARAM;
            goto ERR_EXIT;
        }
    } else {
        pdp_cid = ANET_DEFAULT_PDP_CID;
    }

    net_ptr = aic_get_wan_net_per_pdp(pdp_cid);
    if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
        err_code = TCPIP_OPERATION_NOT_SUPPORT;
        goto ERR_EXIT;
    }

    if (strlen((u8 *)paras_arr[0]) > 255) {
        err_code = TCPIP_INVAL_AT_PARAM;
        goto ERR_EXIT;
    }

    if (dnsinfo.cid != 0) {
        err_code = TCPIP_DNS_BUSY;
        goto ERR_EXIT;
    }

    dnsinfo.host = aos_zalloc_relaxed(AT_HOST_NAME_LEN_MAX + 1);
    if (!dnsinfo.host) {
        err_code = TCPIP_UNKNOW_ERR;
        goto ERR_EXIT;
    }

    strncpy(dnsinfo.host, (u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]));
    dnsinfo.comid = at_req_resp_ptr->serial_port_id;
    dnsinfo.cid = pdp_cid;
    aos_set_event(at_dns_event, DNS_PROCESS_EVENT, TX_OR);

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

ERR_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, err_code);
    aic_free_paras_mem(paras_arr, cnt);
    return TCPIP_INVAL_AT_PARAM;
}

u32 handle_mping_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_PING"\"host\",(1-60),(1-65535),(1-1400),(1-5)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mping_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    AT_RESP_FROM_CP at_resp;
    u8 pdp_cid;
    int packet_len = PING_PLYLOAD_SIZE_DEFAULT, timeout = 1, count = 4;
    WAN_NET *net_ptr = NULL;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    u8 cmd[16] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_IP_PING"=",
                                             &cnt);
    if ( !paras_arr || cnt < 1 || cnt > 5) {
        goto PING_ERROR;
    }

    if (at_ping_cfg.ping_timeout != 0) {
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_PING_BUSY);
        aic_free_paras_mem(paras_arr, cnt);
        return TCPIP_INVAL_AT_PARAM;
    }

    if (strlen((u8 *)paras_arr[0]) > 255)
        goto PING_ERROR;

    if (cnt == 5 && strlen((u8 *)paras_arr[4])){
        if (!aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4]))) {
            goto PING_ERROR;
        }
        pdp_cid = atoi((u8 *)paras_arr[4]);
        if (pdp_cid < 1 || pdp_cid > ANET_MAX_PDP_CID) {
            goto PING_ERROR;
        }
    } else {
        pdp_cid = ANET_DEFAULT_PDP_CID;
    }

    if (cnt >= 2 && strlen((u8 *)paras_arr[1])) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            goto PING_ERROR;

        timeout = atoi((u8 *)paras_arr[1]);
        if (timeout < PING_TIMEOUT_MIN || timeout > 60) {
            goto PING_ERROR;
        }
    }

    if (cnt >= 3 && strlen((u8 *)paras_arr[2])) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
            goto PING_ERROR;
        count = atoi((u8 *)paras_arr[2]);
        if(count > PING_COUNT_SIZE_MAX || count < PING_COUNT_SIZE_MIN) {
            goto PING_ERROR;
        }
    }

    if (cnt >= 4 && strlen((u8 *)paras_arr[3])) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))
            goto PING_ERROR;
        packet_len = atoi((u8 *)paras_arr[3]);
        if (packet_len > PING_PLYLOAD_SIZE_MAX || packet_len < PING_PLYLOAD_SIZE_MIN) {
            goto PING_ERROR;
        }
    }

    net_ptr = aic_get_wan_net_per_pdp(pdp_cid);
    if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
        at_resp.resp_buf = str;
        at_resp.buf_len = AT_RESP_RESULT_MAX;
        snprintf(cmd, sizeof(cmd), "AT+CGACT=1,%d", pdp_cid);
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
        aic_set_ipcall_act_flag(pdp_cid);
#endif
        aic_send_cmd_and_wait(cmd, &at_resp);
        if(strstr(at_resp.resp_buf, "ERROR")) {
            aos_memset(str, '\0', sizeof(str));
            aic_reply_ok_resp(at_req_resp_ptr);
            snprintf(str, sizeof(str), RESP_IP_PING"%d", PING_DNS_FAIL);
            aic_transmit_at_cmd_urc(at_req_resp_ptr->serial_port_id, str, strlen(str));
            //aic_reply_error_resp(at_req_resp_ptr, TCPIP_PDP_ACTIVE_FAIL);
            aic_free_paras_mem(paras_arr, cnt);
            return TCPIP_INVAL_AT_PARAM;
        }

        aos_sleep(1000);
        net_ptr = aic_get_wan_net_per_pdp(pdp_cid);
        if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_PDP_ACTIVE_FAIL);
            aic_free_paras_mem(paras_arr, cnt);
            return TCPIP_INVAL_AT_PARAM;
        }
    }

    aos_memset(&at_ping_cfg, 0, sizeof(AT_PING_CFG));
    at_ping_cfg.host = aos_zalloc_relaxed(AT_HOST_NAME_LEN_MAX + 1);
    if (!at_ping_cfg.host) {
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_UNKNOW_ERR);
        aic_free_paras_mem(paras_arr, cnt);
        return TCPIP_INVAL_AT_PARAM;
    }
    aicx_info(0x150351, "s", "recv ping host %s\n", (u8 *)paras_arr[0]);
    strncpy(at_ping_cfg.host, (u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]));
    at_ping_cfg.cid = pdp_cid;
    at_ping_cfg.cnt = count;
    at_ping_cfg.ping_interval = 1;
    at_ping_cfg.ping_timeout = timeout;
    at_ping_cfg.ping_size = packet_len;
    at_ping_cfg.ping_comid = at_req_resp_ptr->serial_port_id;
    aos_set_event(at_ping_event, PING_PROCESS_EVENT, TX_OR);

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
PING_ERROR:
    aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return TCPIP_INVAL_AT_PARAM;
}

u32 handle_transmit_mode_exit_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
     if (*(at_req_resp_ptr->at_req + 8) != 0x0D &&
        *(at_req_resp_ptr->at_req + 8) != 0x0A) {
        aic_reply_error_resp(at_req_resp_ptr,
                                TCPIP_INVAL_AT_PARAM);
        return AIC_TCPIP_ERR_INVAL_AT_CMD;
    }

    at_req_resp_ptr->at_mode = AT_CMD_MODE;
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_mntp_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_IP_NTP"\"server\",(0-65535),(0,1),(1-300)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 aic_mntp_at_process_dns(u8 *host)
{
    struct addrinfo *res = AIC_NULL, *dns_res;
    int ret;
    AIC_SNTP_SERVER_IP *aic_sntp_server;

    ret = getaddrinfo(host, NULL, NULL, &res);
    if (ret != NX_SUCCESS || !res) {
        return ret;
    }
    aic_sntp_del_all_server_from_list();
    dns_res = res;
    while (dns_res) {
        aic_sntp_server = aos_zalloc_relaxed(sizeof(AIC_SNTP_SERVER_IP));
        if (!aic_sntp_server) {
            continue;
        }
        if(dns_res->ai_addr->sa_family == AF_INET) {
            if (sntp_server_ip == 0) {
                aos_memcpy(&sntp_server_ip,
                           &((struct sockaddr_in *)dns_res->ai_addr)->sin_addr, 4);
                NX_CHANGE_ULONG_ENDIAN(sntp_server_ip);
            }

            aos_memcpy(&aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v4,
                       &((struct sockaddr_in *)dns_res->ai_addr)->sin_addr, 4);
            NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v4);
            aic_sntp_server->aic_sntp_server_address.nxd_ip_version =
                NX_IP_VERSION_V4;
        } else {
            if (sntp_server_address.nxd_ip_version != NX_IP_VERSION_V6) {
                aos_memcpy(sntp_server_address.nxd_ip_address.v6,
                           &((struct sockaddr_in6 *)dns_res->ai_addr)->sin6_addr,
                           16);
                sntp_server_address.nxd_ip_version = NX_IP_VERSION_V6;
                NX_CHANGE_ULONG_ENDIAN(sntp_server_address.nxd_ip_address.v6[0]);
                NX_CHANGE_ULONG_ENDIAN(sntp_server_address.nxd_ip_address.v6[1]);
                NX_CHANGE_ULONG_ENDIAN(sntp_server_address.nxd_ip_address.v6[2]);
                NX_CHANGE_ULONG_ENDIAN(sntp_server_address.nxd_ip_address.v6[3]);
            }

            aos_memcpy(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6,
                       &((struct sockaddr_in6 *)dns_res->ai_addr)->sin6_addr, 16);
            NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6[0]);
            NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6[1]);
            NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6[2]);
            NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6[3]);
            aic_sntp_server->aic_sntp_server_address.nxd_ip_version =
                NX_IP_VERSION_V6;
        }
        aic_sntp_add_server_to_list(aic_sntp_server);
        dns_res = dns_res ->ai_next;
    }
    freeaddrinfo(res);

    return AIC_TCPIP_AT_OK;
}

static int aic_mntp_at_process(u8 *host)
{
    int ret;
    NX_SNTP_CLIENT *sntp_client;
    WAN_NET *net_ptr = NULL;

    ret = aic_mntp_at_process_dns(host);
    if (ret != NX_SUCCESS) {
        return TCPIP_DNS_PARSING_FAIL;
    }

    net_ptr = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
    if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
        return TCPIP_PDP_NOT_ACTIVE;
    }

    sntp_client = aic_get_sntp_client();
    if (!sntp_client) {
        aic_sntp_pdp_state_change_callback(ANET_DEFAULT_PDP_CID,
                                           net_ptr->pdp_activated);
        sntp_client = aic_get_sntp_client();
        if (sntp_client) {
            aos_set_event(aic_sntp_client_event, 1 << 2, TX_OR);
            sntp_client->nx_sntp_server_update_time_active++;
        }
        return AIC_TCPIP_AT_OK;
    }

    /*set sntp request send event: SNTP_AT_SEND_EVENT*/
    aos_set_event(aic_sntp_client_event, 1 << 2, TX_OR);
    sntp_client-> nx_sntp_client_retransmiss_times = 0;
    sntp_client->nx_sntp_server_update_time_active++;
    return AIC_TCPIP_AT_OK;
}

u32 handle_mntp_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr = NULL;
    int ret, paras, timeout, port;

    if (strncasecmp(at_req_resp_ptr->at_req, AT_IP_NTP"=",
                    strlen(AT_IP_NTP"=")) == 0) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 AT_IP_NTP"=",
                                                 &cnt);
        if ( !paras_arr || cnt < 1 || cnt > 4) {
            goto EXIT_ERROR;
        }

        if (strlen((u8 *)paras_arr[0]) > 255) {
            goto EXIT_ERROR;
        }

        if (cnt >= 2 && strlen((u8 *)paras_arr[1])) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                goto EXIT_ERROR;

            port = atoi((u8 *)paras_arr[1]);
            if (port > 65535)
                goto EXIT_ERROR;

            sntp_server_port = port;
        } else {
            sntp_server_port = NX_SNTP_SERVER_UDP_PORT;
        }

        if (cnt >= 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                goto EXIT_ERROR;

            paras = atoi((u8 *)paras_arr[2]);
            if (paras != 0 && paras != 1)
                goto EXIT_ERROR;

            sntp_is_sync = paras;
        } else {
            sntp_is_sync = AIC_TRUE;
        }

        if (cnt == 4) {
            timeout = atoi((u8 *)paras_arr[3]);
            if (timeout < 1 || timeout > 300)
                goto EXIT_ERROR;

            sntp_sync_timeout = timeout;
        } else {
            sntp_sync_timeout = 30;
        }

        ret = aic_mntp_at_process((u8 *)paras_arr[0]);
        if (ret != AIC_TCPIP_AT_OK) {
            aic_free_paras_mem(paras_arr, cnt);
            aic_reply_error_resp(at_req_resp_ptr, ret);
            return ret;
        }

        at_tcpip_cfg[0].com_id = at_req_resp_ptr->serial_port_id;
        aic_reply_ok_resp(at_req_resp_ptr);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
EXIT_ERROR:
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return TCPIP_INVAL_AT_PARAM;
    } else {
        sntp_is_sync = AIC_TRUE;
        sntp_sync_timeout = 30;
        sntp_server_port = NX_SNTP_SERVER_UDP_PORT;

        ret = aic_mntp_at_process(AT_SNTP_DEFAULT_SERVER);
        if (ret != AIC_TCPIP_AT_OK) {
            aic_reply_error_resp(at_req_resp_ptr, ret);
            return ret;
        }
        at_tcpip_cfg[0].com_id = at_req_resp_ptr->serial_port_id;
        aic_reply_ok_resp(at_req_resp_ptr);
        return AIC_TCPIP_AT_OK;
    }
}

static void aic_mat_socket_recv_notify(int sock, void *data)
{
    int i;

    i = aic_at_find_index_by_socket(sock, AT_MTCPIP);
    if (i == AIC_EPERM)
        return;

    at_tcpip_cfg[i].recv_flag = 1;
    aos_set_event(at_socket_event, SOCKET_RECV_EVENT, TX_OR);
}

static void aic_at_tcpip_socket_recv(int index)
{
    int i, sock, ret, fromaddrlen, len, strl, unrcv, buffer_len = SIZE_8K;
    u8 *buffer = NULL, *str = NULL, *type = NULL;
    struct sockaddr_in fromaddr;
    struct sockaddr_in6 fromaddr6;
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};

    sock = sock_list[index].socket_id;
    if (sock == 0) {
        return;
    }

    aic_get_socket_recv_bytes_info(sock, &len);
    if (sock_list[index].mode == 0 || sock_list[index].mode == 1) {
        sock_list[index].recv_total = len;
        if (len >= 0) {
            /*need to apply add 2 bytes, AND add later \r\n */
            buffer = aos_zalloc_relaxed(buffer_len + 50 + 2);
            if (!buffer) {
                return;
            }
        }
recv_again:
        if (sock_list[index].socket_type == SOCK_TCP) {
            type = "rtcp";
            ret = anet_recv(sock, (buffer + 50), buffer_len, MSG_DONTWAIT);
        } else {
            type = "rudp";
            if (sock_list[index].family == AF_INET) {
                fromaddrlen = sizeof(struct sockaddr_in);
                ret = anet_recvfrom(sock, (buffer + 50), buffer_len, MSG_DONTWAIT,
                                    (struct sockaddr *)&fromaddr,
                                    &fromaddrlen);
            } else {
                fromaddrlen = sizeof(struct sockaddr_in6);
                ret = anet_recvfrom(sock, (buffer + 50), buffer_len, MSG_DONTWAIT,
                                    (struct sockaddr *)&fromaddr6,
                                    &fromaddrlen);
            }
        }
        if (ret > 0) {
            sock_list[index].recv_total -= ret;
            if (sock_list[index].mode == 0) {
                if (at_tcpip_cfg[index].recv_data_format == 0) {
                    aos_memset(result_str, 0, AT_RESP_RESULT_MAX);
                    if (at_tcpip_cfg[index].com_id == conn_get_uart_at_conid) {
                        result_str[0] = aic_get_cr_sign();
                        result_str[1] = aic_get_lf_sign();
                        snprintf((result_str + 2), AT_RESP_RESULT_MAX,
                                 IP_URC"\"%s\",%d,%d,", type,
                                 sock_list[index].link_id, ret);
                    } else
                        snprintf(result_str, AT_RESP_RESULT_MAX,
                                 IP_URC"\"%s\",%d,%d,", type,
                                 sock_list[index].link_id, ret);

                    strl = strlen(result_str);
                    aos_memcpy((buffer + 50 - strl), result_str, strl);
                    if (at_tcpip_cfg[index].com_id == conn_get_uart_at_conid) {
                        *(buffer + 50 + ret) = aic_get_cr_sign();
                        *(buffer + 50 + ret + 1) = aic_get_lf_sign();
                    }

                    if (at_tcpip_cfg[index].com_id == conn_get_uart_at_conid) {
                        aic_send_to_at_port(at_tcpip_cfg[index].com_id,
                                            (buffer + 50 - strl), (strl + ret + 2));
                    } else {
                        aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                                (buffer + 50 - strl), strl + ret);
                    }
                } else {
                    str = aos_zalloc_relaxed(ret * 2 + 50);
                    if (!str) {
                        if (buffer)
                            aos_free(buffer);
                        return;
                    }
                    snprintf(str, 50, IP_URC"\"%s\",%d,%d,", type,
                             sock_list[index].link_id, ret);
                    strl = strlen(str);
                    aic_print_data_to_hex((buffer + 50), ret, (str + strl));
                    strl = strl + ret * 2;
                    aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id, str, strl);
                    aos_free(str);
                }
            } else {
                aic_transmit_at_data(at_tcpip_cfg[index].com_id, (buffer + 50), ret);
            }
        } else if (ret < 0) {
            if (buffer)
                aos_free(buffer);

            return;
        } else {
            if (sock_list[index].socket_type == SOCK_UDP) {
                snprintf(result_str, AT_RESP_RESULT_MAX,
                         IP_URC"\"rudp\",%d,0,", sock_list[index].link_id);
                aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                        result_str, strlen(result_str));
            }
        }

        aic_get_socket_recv_bytes_info(sock, &len);
        aic_get_socket_recv_packet_info(sock, &unrcv);
        sock_list[index].recv_total = len;
        if (unrcv) {
            goto recv_again;
        } else {
            if (buffer)
                aos_free(buffer);
        }
    } else if ((sock_list[index].mode == 2 && sock_list[index].socket_type == SOCK_UDP) ||
        (sock_list[index].mode == 3 && sock_list[index].socket_type == SOCK_TCP)) {
        if (sock_list[index].socket_type == SOCK_TCP) {
            aic_get_socket_recv_packet_info(sock_list[index].socket_id, &unrcv);
            for (i = sock_list[index].recv_cnt + 1; i <= unrcv; i++) {
                snprintf(result_str, AT_RESP_RESULT_MAX,
                         IP_URC"\"rtcp\",%d,%d", sock_list[index].link_id, i);
                aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                        result_str, strlen(result_str));
            }
            sock_list[index].recv_cnt = unrcv;
        } else {
urc_again:
            if (sock_list[index].recv_total == len)
                return;
            snprintf(result_str, AT_RESP_RESULT_MAX,
                     IP_URC"\"rudp\",%d,%d,%d", sock_list[index].link_id,
                     len - sock_list[index].recv_total, len);
            sock_list[index].recv_total = len;
            aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                    result_str, strlen(result_str));
            aic_get_socket_recv_bytes_info(sock, &len);
            if (len > sock_list[index].recv_total)
                goto urc_again;
        }

    } else {
        if (sock_list[index].socket_type == SOCK_TCP) {
urc_again1:
            if (sock_list[index].recv_total == len)
                return;
            snprintf(result_str, AT_RESP_RESULT_MAX,
                     IP_URC"\"rtcp\",%d,%d,%d", sock_list[index].link_id,
                     len - sock_list[index].recv_total, len);
            sock_list[index].recv_total = len;
            aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                    result_str, strlen(result_str));
            aic_get_socket_recv_bytes_info(sock, &len);
            if (len > sock_list[index].recv_total)
                goto urc_again1;
        } else {
            aic_get_socket_recv_packet_info(sock_list[index].socket_id, &unrcv);
            for (i = sock_list[index].recv_cnt + 1; i <= unrcv; i++) {
                snprintf(result_str, AT_RESP_RESULT_MAX,
                         IP_URC"\"rudp\",%d,%d", sock_list[index].link_id, i);
                aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                        result_str, strlen(result_str));
            }
            sock_list[index].recv_cnt = unrcv;
        }
    }
}

static void aic_at_tcpip_socket_close(int index)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int sock, state, mode;
    NX_BSD_SOCKET   *bsd_socket_ptr;

    sock = sock_list[index].socket_id;
    if (sock == 0) {
        return;
    }
    bsd_socket_ptr = &nx_bsd_socket_array[sock - NX_BSD_SOCKFD_START];
    if (bsd_socket_ptr->nx_bsd_socket_tcp_socket &&
        bsd_socket_ptr->nx_bsd_socket_tcp_socket->nx_tcp_socket_connect_interface &&
        !bsd_socket_ptr->nx_bsd_socket_tcp_socket->nx_tcp_socket_connect_interface->nx_interface_valid)
        state = 3;
    else
        state = 1;

    mode = sock_list[index].mode;
    aic_mat_socket_autofree(index);
    if (mode == 1) {
        aic_end_at_data_mode(at_tcpip_cfg[index].com_id);
    }
    snprintf(str, AT_RESP_RESULT_MAX, IP_URC"\"disconn\",%d,%d", index, state);
    aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id, str, strlen(str));
}

static void aic_at_tcpip_socket_connect_ok(int index)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    struct timeval timeout;

    if (!at_tcpip_cfg[index].ssl_enable) {
        anet_fcntl(sock_list[index].socket_id, F_SETFL, (~O_NONBLOCK));
        timeout.tv_sec = AT_SOCKET_SEND_TIMEOUT;
        timeout.tv_usec = 0;
        setsockopt(sock_list[index].socket_id, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    }

    if (sock_list[index].mode == 1) {
        sock_info.sockid = sock_list[index].socket_id;
        sock_info.length = at_tcpip_cfg[index].transwaittm;
        sock_info.offset = 0;
        strcpy(str, "CONNECT");
        aic_enter_data_mode_after_connect(AIC_TRUE, str, strlen(str));
    } else {
        snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_OPEN"%d,0", index);
        aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id, str, strlen(str));
    }

    aos_free(listen_info[index].host);
    aos_memset(&listen_info[index], 0, sizeof(struct aic_at_listen_info));
}

static void aic_at_tcpip_socket_connect_fail(int index, int error)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int cid;

    snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_OPEN"%d,%d", index, error);
    if (sock_list[index].mode == 1)
        aic_enter_data_mode_after_connect(AIC_FALSE, str, strlen(str));
    else
        aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id, str, strlen(str));

    aos_free(listen_info[index].host);
    aos_memset(&listen_info[index], 0, sizeof(struct aic_at_listen_info));
    cid = sock_list[index].pdp_cid;
    aos_memset(&sock_list[index], 0, sizeof(AIC_SOCKET_LIST));
    sock_list[index].pdp_cid = cid;
}

static void aic_at_tcpip_process(void *pdata)
{
    u32 events = 0, timestamp = 0, status;
    int i = 0, unrcv, index = 0;
    u32 timeout = AOS_WAIT_FOREVER;
#if CONFIG_GX_SSL_AT || CONFIG_CM_SSL_AT
    int ret;
#endif

    while (1) {
        status = aos_get_event(at_socket_event, SOCKET_EVENT_ALL,
                               TX_OR_CLEAR, &events, timeout);
        if (status != NX_SUCCESS) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                if (listen_info[i].timeout == 1) {
                    soc_close(sock_list[i].socket_id);
                    if (at_tcpip_cfg[i].connect_timeout < 75)
                        aic_at_tcpip_socket_connect_fail(i, TCPIP_SOCKET_CONNECT_TIMEOUT);
                    else
                        aic_at_tcpip_socket_connect_fail(i, TCPIP_SOCKET_CONNECT_FAIL);
                }
            }
            timeout = AOS_WAIT_FOREVER;
        }

        aos_get_mutex(at_socket_mutex, AOS_WAIT_FOREVER);
        if (events & SOCKET_RECV_EVENT) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                if (sock_list[i].socket_id == 0)
                    continue;

                aic_get_socket_recv_packet_info(sock_list[i].socket_id, &unrcv);
                if (!at_tcpip_cfg[i].recv_flag && !unrcv)
                    continue;
#if defined CONFIG_CM_SSL_AT || defined CONFIG_GX_SSL_AT
                if (sock_list[i].ssl) {
                    aic_at_ssl_socket_recv(i);
                } else
#endif
                {
                    aic_at_tcpip_socket_recv(i);
                }
                at_tcpip_cfg[i].recv_flag = 0;
            }
        }

        if (events & SOCKET_CLOSE_EVENT) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                if (!at_tcpip_cfg[i].close_flag)
                    continue;

                aic_at_tcpip_socket_close(i);
                at_tcpip_cfg[i].close_flag = 0;
            }
        }

        if (events & SOCKET_ESTABLISH_EVENT) {
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                if (listen_info[i].timeout == 1) {
                    if (sock_list[i].status == SOCKET_CONNECT_SUCCESS) {
                        if (at_tcpip_cfg[i].ssl_enable) {
#if CONFIG_GX_SSL_AT || CONFIG_CM_SSL_AT
                            ret = aic_mat_create_mbedtls_socket(i);
                            if (ret < 0)
                                aic_at_tcpip_socket_connect_fail(i, TCPIP_SOCKET_CONNECT_FAIL);
                            else
                                aic_at_tcpip_socket_connect_ok(i);
#endif
                        } else {
                            aic_at_tcpip_socket_connect_ok(i);
                        }
                    } else if (sock_list[i].status == SOCKET_CONNECT_FAIL) {
                        soc_close(sock_list[i].socket_id);
                        aic_at_tcpip_socket_connect_fail(i, TCPIP_SOCKET_CONNECT_FAIL);
                    } else {
                        continue;
                    }
                    timeout = AOS_WAIT_FOREVER;
                }
            }

            timestamp = 0;
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
                if (listen_info[i].timestamp == 0) {
                    continue;
                } else {
                    if (timestamp == 0)
                        timestamp = listen_info[i].timestamp;
                    else if (timestamp > listen_info[i].timestamp)
                        timestamp = listen_info[i].timestamp;

                    index = i;
                }
            }

            if (timestamp != 0) {
                aic_at_tcpip_socket_establish(index);
                if (listen_info[index].type == SOCK_TCP) {
                    listen_info[index].timeout = 1;
                    if (at_tcpip_cfg[index].connect_timeout < 75)
                        timeout =
                            at_tcpip_cfg[index].connect_timeout * NX_IP_PERIODIC_RATE;
                    else
                        timeout = 75 * NX_IP_PERIODIC_RATE;
                }
            }
        }

        aos_put_mutex(at_socket_mutex);
    }
}

int aic_at_socket_task_create(void)
{
    at_socket_thread = aos_create_thread("AT Socket", aic_at_tcpip_process,
                                         NULL, AT_SOCKET_TASK_SIZE, 0, 0,
                                         15, AOS_DONT_START);
    if (at_socket_thread == AOS_INVALID_THREAD_ID) {
        return AIC_EFAULT;
    }

    at_socket_event = aos_create_event("AT Socket");
    if (!at_socket_event) {
        aos_delete_thread(at_socket_thread);
        return AIC_EFAULT;
    }

    aos_resume_thread(at_socket_thread);
    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_at_socket_task_create);

static void aic_mat_socket_autofree(int index)
{
    int cid;

    cid = sock_list[index].pdp_cid;
    if (!at_tcpip_cfg[index].autofree) {
#if defined CONFIG_CM_SSL_AT || defined CONFIG_GX_SSL_AT
        if (sock_list[index].ssl) {
            aic_mat_close_mbedtls_socket(sock_list[index].socket_id);
        } else
#endif
        {
            soc_close(sock_list[index].socket_id);
        }
        aic_socket_unregister_tcp_disconnect_notify(sock_list[index].socket_id);
        aic_socket_unregister_receive_notify(sock_list[index].socket_id);
        aos_memset(&sock_list[index], 0, sizeof(struct SOCKET_LIST));
        sock_list[index].pdp_cid = cid;
    } else {
        sock_list[index].autofree = 1;
    }
}

void aic_mat_socket_tcp_disconn_notify(int sock, void *data, int is_active)
{
    int i;

    if (is_active == TRUE)
        return;

    i = aic_at_find_index_by_socket(sock, AT_MTCPIP);
    if (i < 0) {
        return;
    }

    at_tcpip_cfg[i].close_flag = 1;
    aos_set_event(at_socket_event, SOCKET_CLOSE_EVENT, TX_OR);
}

static void aic_fill_dns_result(u8 *str, u16 str_len, u8 *host, u8 *addr1, u8 *addr2)
{
    if (strlen(addr1) && strlen(addr2))
        snprintf(str, str_len, RESP_IP_DNSGIP"\"%s\",\"%s\",\"%s\"",
                 host, addr1, addr2);
    else if (strlen(addr1) && (strlen(addr2) == 0))
        snprintf(str, str_len, RESP_IP_DNSGIP"\"%s\",\"%s\"", host, addr1);
    else if ((strlen(addr1) == 0) && strlen(addr2))
        snprintf(str, str_len, RESP_IP_DNSGIP"\"%s\",\"%s\"", host, addr2);
    else
        snprintf(str, str_len, RESP_IP_DNSGIP"\"%s\"", host);
}

static void aic_at_tcpip_process_dnsgip(void *pdata)
{
    struct addrinfo *res, *dns_res;
    struct addrinfo hint = {0, AF_UNSPEC, 0, 0, 0, NX_NULL, NX_NULL, NX_NULL};
    char addr4[ADDR_BUFFERSIZE_IPV4] = {'\0'};
    char addr6[ADDR_BUFFERSIZE_IPV6] = {'\0'};
    // char str[AT_RESP_RESULT_MAX] = {'\0'};
    u8 *urc_str = NULL;
    WAN_NET *net_ptr = NULL;
    u32 events = 0;
    int ret;
    u8 pdp_type;

    urc_str = aos_zalloc_relaxed(SIZE_512B);
    if (!urc_str) {
        return;
    }

    while (1) {
        aos_memset(urc_str, 0, SIZE_512B);
        aos_get_event(at_dns_event, DNS_PROCESS_EVENT, TX_OR_CLEAR,
                      &events, AOS_WAIT_FOREVER);

        if (events & DNS_PROCESS_EVENT) {
            net_ptr = aic_get_wan_net_per_pdp(dnsinfo.cid);
            if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
                goto ERR_HANDLE;
            }

            ret = getaddrinfo_by_pdp_cid(dnsinfo.host, NULL, &hint, &res,
                                         dnsinfo.cid, NX_DNS_LOOKUP_CACHE_FIRST);
            if (ret != NX_SUCCESS || !res) {
                goto ERR_HANDLE;
            }

            dns_res = res;
            while (dns_res) {
                if (dns_res->ai_addr->sa_family == AF_INET) {
                    inet_ntop(AF_INET, &((struct sockaddr_in *)dns_res->ai_addr)->sin_addr,
                          addr4, ADDR_BUFFERSIZE_IPV4);
                    break;
                }
                dns_res = dns_res->ai_next;
            }

            dns_res = res;
            while (dns_res) {
                if (dns_res->ai_addr->sa_family == AF_INET6) {
                    inet_ntop(AF_INET6, &((struct sockaddr_in6 *)dns_res->ai_addr)->sin6_addr,
                              addr6, ADDR_BUFFERSIZE_IPV6);
                    break;
                }
                dns_res = dns_res->ai_next;
            }

            pdp_type = aic_get_pdp_ip_type(dnsinfo.cid);
            if (pdp_type == ANET_DEFAULT_IP_TYPE_V4)
                aos_memset(addr6, '\0', ADDR_BUFFERSIZE_IPV6);
            else if (pdp_type == ANET_DEFAULT_IP_TYPE_V6)
                aos_memset(addr4, '\0', ADDR_BUFFERSIZE_IPV4);

            if (aic_get_dns_prio() == 1)
                aic_fill_dns_result(urc_str, SIZE_512B, dnsinfo.host, addr6, addr4);
            else
                aic_fill_dns_result(urc_str, SIZE_512B, dnsinfo.host, addr4, addr6);

            aic_transmit_at_cmd_urc(dnsinfo.comid, urc_str, strlen(urc_str));
            freeaddrinfo(res);

            aos_memset(addr4, '\0', ADDR_BUFFERSIZE_IPV4);
            aos_memset(addr6, '\0', ADDR_BUFFERSIZE_IPV6);
            aos_free(dnsinfo.host);
            aos_memset(&dnsinfo, 0, sizeof(dnsinfo));
            continue;

ERR_HANDLE:
            snprintf(urc_str, SIZE_512B, RESP_IP_DNSGIP "\"%s\"", (s8 *)dnsinfo.host);
            aicx_info(0x027696, "ds", "urc str len %d, data %s\n", strlen(urc_str), urc_str);
            aic_transmit_at_cmd_urc(dnsinfo.comid, urc_str, strlen(urc_str));
            aos_free(dnsinfo.host);
            aos_memset(&dnsinfo, 0, sizeof(dnsinfo));
            continue;
        }
    }

    aos_free(urc_str);
}

int aic_at_dns_task_create(void)
{
    /*dns task space need 2048 */
    at_dns_thread = aos_create_thread("AT DNS",
                                      aic_at_tcpip_process_dnsgip,
                                      NULL, SIZE_2K, 0, 0,
                                      15, AOS_DONT_START);
    if (at_dns_thread == AOS_INVALID_THREAD_ID) {
        return AIC_EFAULT;
    }

     at_dns_event = aos_create_event("AT Dns");
    if (!at_dns_event) {
        aos_delete_thread(at_dns_thread);
        return AIC_EFAULT;
    }

    aos_resume_thread(at_dns_thread);
    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_at_dns_task_create);

static int aic_at_tcpip_gethostbyname(u8 *host, struct sockaddr_in *remoteaddr,
                                      struct sockaddr_in6 *remoteaddr6,
                                      NXD_ADDRESS *addrv4, NXD_ADDRESS *addrv6)
{
    struct addrinfo *res, *dns_res;
    int ret;

    ret = getaddrinfo(host, AIC_NULL, AIC_NULL, &res);
    if (ret != NX_SUCCESS || !res) {
        return AIC_EFAULT;
    }

    dns_res = res;
    while (dns_res) {
        if (dns_res->ai_addr->sa_family == AF_INET) {
            if (remoteaddr) {
                aos_memset(remoteaddr, 0, sizeof(struct sockaddr_in));
                remoteaddr->sin_family = AF_INET;
                remoteaddr->sin_addr.s_addr =
                    ((struct sockaddr_in *)dns_res->ai_addr)->sin_addr.s_addr;
            }
            if (addrv4) {
                aos_memset(addrv4, 0, sizeof(NXD_ADDRESS));
                addrv4->nxd_ip_version = NX_IP_VERSION_V4;
                addrv4->nxd_ip_address.v4 =
                    ((struct sockaddr_in *)dns_res->ai_addr)->sin_addr.s_addr;
                NX_CHANGE_ULONG_ENDIAN(addrv4->nxd_ip_address.v4);
            }
            break;
        }
        dns_res = dns_res->ai_next;
    }

    dns_res = res;
    while (dns_res) {
        if (dns_res->ai_addr->sa_family == AF_INET6) {
            if (remoteaddr6) {
                aos_memset(remoteaddr6, 0, sizeof(struct sockaddr_in6));
                remoteaddr6->sin6_family = AF_INET6;
                aos_memcpy(remoteaddr6->sin6_addr._S6_un._S6_u8,
                    &((struct sockaddr_in6 *)dns_res->ai_addr)->sin6_addr._S6_un._S6_u8, 16);
            }
            if (addrv6) {
                aos_memset(addrv6, 0, sizeof(NXD_ADDRESS));
                addrv6->nxd_ip_version = NX_IP_VERSION_V6;
                aos_memcpy(addrv6->nxd_ip_address.v6,
                    &(((struct sockaddr_in6 *)dns_res->ai_addr)->sin6_addr), 16);
                NX_CHANGE_ULONG_ENDIAN(addrv6->nxd_ip_address.v6[0]);
                NX_CHANGE_ULONG_ENDIAN(addrv6->nxd_ip_address.v6[1]);
                NX_CHANGE_ULONG_ENDIAN(addrv6->nxd_ip_address.v6[2]);
                NX_CHANGE_ULONG_ENDIAN(addrv6->nxd_ip_address.v6[3]);
            }
            break;
        }
        dns_res = dns_res->ai_next;
    }
    freeaddrinfo(res);
    return AIC_SUCCESS;
}

static void aic_at_tcpip_process_ping(void *pdata)
{
    AT_PING_CFG *cfg = &at_ping_cfg;
    NXD_ADDRESS *destaddr = NULL, destaddr1, destaddr2;
    NX_IPV4_HEADER *ipv4_header;
    NX_IPV6_HEADER *ipv6_header;
    NXD_IPV6_ADDRESS *ipv6_addr = NULL;
    NX_PACKET *response_ptr;
    u8 pdp_type, *at_ping_payload, first_flag = TRUE;
    int ret, i = 0, time;
    u32 events = 0;
    WAN_NET *net_ptr = NULL;
    NX_INTERFACE *infc;
    struct aic_ping_result result;
    struct aic_network_ip_instance *net_ip_instance;
    s8 at_ping_addr[PING_ADDR_BUFFERSIZE_IPV6] = {0};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    while (1) {
        aos_get_event(at_ping_event, PING_PROCESS_EVENT, TX_OR_CLEAR,
                      &events, AOS_WAIT_FOREVER);

        aos_memset(&result, 0, sizeof(struct aic_ping_result));
        destaddr = NULL;
        ipv6_addr = NULL;
        if (events & PING_PROCESS_EVENT) {
            aos_memset(&destaddr1, 0, sizeof(NXD_ADDRESS));
            aos_memset(&destaddr2, 0, sizeof(NXD_ADDRESS));
            net_ptr = aic_get_wan_net_per_pdp(cfg->cid);
            if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
                snprintf(str, sizeof(str), RESP_IP_PING"%d", PING_OTHER_ERROR);
                aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                aos_free(at_ping_cfg.host);
                aos_memset(&at_ping_cfg, 0, sizeof(at_ping_cfg));
                continue;
            }

            net_ip_instance = aic_get_pdp_ip_instance();
            ret = aic_at_tcpip_gethostbyname(cfg->host, NULL, NULL, &destaddr1, &destaddr2);
            if (ret != AIC_SUCCESS) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_IP_PING"%d", PING_DNS_FAIL);
                aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                aos_free(at_ping_cfg.host);
                aos_memset(&at_ping_cfg, 0, sizeof(at_ping_cfg));
                continue;
            }

            for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++) {
                /* Compare the address. */
                if ((net_ip_instance->ip_ptr->nx_ipv6_address[i].nxd_ipv6_address_valid) &&
                    (CHECK_IPV6_ADDRESSES_SAME(net_ip_instance->ip_ptr->nx_ipv6_address[i].nxd_ipv6_address,
                                               net_ptr->wan_ipv6_address1))) {
                    /* Found a proper address. */
                    ipv6_addr = &(net_ip_instance->ip_ptr->nx_ipv6_address[i]);
                }
            }
            pdp_type = aic_get_pdp_ip_type(cfg->cid);
            if (aic_get_dns_prio() == 1) {
                if ((pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
                    pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) &&
                    destaddr2.nxd_ip_version == NX_IP_VERSION_V6 &&
                    ipv6_addr)
                    destaddr = &destaddr2;
                else if ((pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
                         pdp_type == ANET_DEFAULT_IP_TYPE_V4V6 ) &&
                         destaddr1.nxd_ip_version == NX_IP_VERSION_V4)
                    destaddr = &destaddr1;
            } else {
                if ((pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
                    pdp_type == ANET_DEFAULT_IP_TYPE_V4V6 ) &&
                    destaddr1.nxd_ip_version == NX_IP_VERSION_V4)
                    destaddr = &destaddr1;
                else if ((pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
                         pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) &&
                         destaddr2.nxd_ip_version == NX_IP_VERSION_V6
                         && ipv6_addr)
                    destaddr = &destaddr2;
            }


            if (cfg->ping_size) {
                at_ping_payload = aos_zalloc_relaxed(cfg->ping_size);
                if (!at_ping_payload) {
                    aos_memset(str, '\0', sizeof(str));
                    snprintf(str, sizeof(str), RESP_IP_PING"%d", PING_OTHER_ERROR);
                    aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                    cfg->cnt = 0;
                    continue;
                }
            }

            for (i = 0; i < cfg->ping_size; i++)
                *(at_ping_payload + i) = 'a' + i % 24;

            infc = &net_ip_instance->ip_ptr->nx_ip_interface[net_ptr->wan_ifc_index];
            while (cfg->cnt) {
                if (first_flag) {
                    first_flag = FALSE;
                } else {
                    aos_sleep(cfg->ping_interval * NX_IP_PERIODIC_RATE);
                }

                time = aos_get_tickcount();
                result.send_cnt++;
                if (destaddr && destaddr->nxd_ip_version == NX_IP_VERSION_V4) {
                    ret = _nx_icmp_interface_ping(net_ip_instance->ip_ptr,
                                                  destaddr1.nxd_ip_address.v4,
                                                  infc, destaddr1.nxd_ip_address.v4,
                                                  at_ping_payload, cfg->ping_size,
                                                  &response_ptr,
                                                  cfg->ping_timeout * NX_IP_PERIODIC_RATE);
                } else if (destaddr && destaddr->nxd_ip_version == NX_IP_VERSION_V6) {
                    ret = _nx_icmp_interface_ping6(net_ip_instance->ip_ptr,
                                                   &destaddr2, at_ping_payload,
                                                   cfg->ping_size, ipv6_addr, &response_ptr,
                                                   cfg->ping_timeout * NX_IP_PERIODIC_RATE);
                } else {
                    aos_memset(str, '\0', sizeof(str));
                    snprintf(str, sizeof(str), RESP_IP_PING"%d", PING_DNS_FAIL);
                    aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                    cfg->cnt = 0;
                    continue;
                }

                aos_memset(str, '\0', sizeof(str));
                if ((ret != NX_SUCCESS) || (response_ptr == NX_NULL)) {
                    if (response_ptr)
                        nx_packet_release(response_ptr);
                    snprintf(str, sizeof(str), RESP_IP_PING"%d", PING_RSP_TIMEOUT);
                } else {
                    result.recv_cnt++;
                    time = aos_get_tickcount() - time;
                    if ((result.shortest_time > time) || !result.shortest_time)
                        result.shortest_time = time;

                    if (result.longest_time < time)
                        result.longest_time = time;

                    result.average_time += time;
                    if (response_ptr->nx_packet_ip_version == NX_IP_VERSION_V4) {
                        ipv4_header =
                            (NX_IPV4_HEADER *)response_ptr->nx_packet_ip_header;
                        NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_source_ip);
                        inet_ntop(AF_INET, &ipv4_header->nx_ip_header_source_ip,
                                  at_ping_addr, PING_ADDR_BUFFERSIZE_IPV4);
                        snprintf(str, sizeof(str), RESP_IP_PING"%d,\"%s\",%d,%d,%ld",
                                 PING_SUCCESS, at_ping_addr, cfg->ping_size, time,
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
                        snprintf(str, sizeof(str), RESP_IP_PING"%d,\"%s\",%d,%d,%ld",
                                 PING_SUCCESS, at_ping_addr, cfg->ping_size, time,
                                 (ipv6_header->nx_ip_header_word_1 & 0xFF));
                    } else {
                        snprintf(str, sizeof(str), RESP_IP_PING"%d", PING_RSP_ERROR);
                    }
                    nx_packet_release(response_ptr);
                }
                aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                cfg->cnt--;
            }

            if (result.recv_cnt)
                result.average_time /= result.recv_cnt;

            if (result.send_cnt > 1) {
                snprintf(str, sizeof(str), RESP_IP_PING"\"statistics\",%d,%d,%d,%d,%d",
                         result.send_cnt, (result.send_cnt - result.recv_cnt),
                         result.shortest_time, result.longest_time, result.average_time);
                aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
            }

            if (at_ping_payload) {
                aos_free(at_ping_payload);
                at_ping_payload = AIC_NULL;
            }

            events = events & ~(PING_PROCESS_EVENT);
        }
        aos_free(at_ping_cfg.host);
        aos_memset(&at_ping_cfg, 0, sizeof(at_ping_cfg));
    }

    return;
}

int aic_at_ping_task_create(void)
{
    /*ping task need to be larger than 1024*/
    at_ping_thread = aos_create_thread("AT PING",
                                       aic_at_tcpip_process_ping,
                                       NULL, (SIZE_1K + SIZE_512B),
                                       0, 0, 15, AOS_DONT_START);
    if (at_ping_thread == AOS_INVALID_THREAD_ID) {
        return AIC_EFAULT;
    }

     at_ping_event = aos_create_event("AT PING");
    if (!at_ping_event) {
        aos_delete_thread(at_ping_thread);
        return AIC_EFAULT;
    }

    aos_resume_thread(at_ping_thread);
    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_at_ping_task_create);

static void aic_at_tcpip_socket_establish(int index)
{
    int sock, ret, protocolFamily = AF_INET, pdp_type, cid;
    struct sockaddr_in remoteaddr = {0}, localaddr = {0};
    struct sockaddr_in6 remoteaddr6 = {0}, localaddr6 = {0};
    struct sockaddr *peeraddr = NULL, *srcaddr = NULL;
    // struct addrinfo *res, *dns_res;
    WAN_NET *net_ptr;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    cid = sock_list[index].pdp_cid;
    ret = aic_at_tcpip_gethostbyname(listen_info[index].host, &remoteaddr,
                                     &remoteaddr6, NULL, NULL);
    if (ret != AIC_SUCCESS) {
        snprintf(str, sizeof(str), RESP_IP_OPEN"%d,%d", index,
                 TCPIP_DNS_PARSING_FAIL);
        goto EXIT;
    }

    pdp_type = aic_get_pdp_ip_type(cid);
    net_ptr = aic_get_wan_net_per_pdp(cid);
    if (net_ptr) {
        aos_memset(&localaddr, 0, sizeof(struct sockaddr_in));
        if (pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) {
            localaddr.sin_port = htons(listen_info[index].src_port);
            localaddr.sin_family = AF_INET;
            localaddr.sin_addr.s_addr = htonl(net_ptr->wan_ipv4_address);
            remoteaddr.sin_port = htons(listen_info[index].port);
        }

        aos_memset(&localaddr6, 0, sizeof(struct sockaddr_in6));
        if (pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) {
            localaddr6.sin6_port = htons(listen_info[index].src_port);
            localaddr6.sin6_family = AF_INET6;
            aos_memcpy(&localaddr6.sin6_addr,
                       net_ptr->wan_ipv6_address1,
                       16);
            NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[0]);
            NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[1]);
            NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[2]);
            NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[3]);
            remoteaddr6.sin6_port = htons(listen_info[index].port);
        }
    }

    if (aic_get_dns_prio() == 0) {
        if ((pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) &&
            remoteaddr.sin_family == AF_INET &&
            localaddr.sin_family == AF_INET) {
            protocolFamily = AF_INET;
            peeraddr = (struct sockaddr *)&remoteaddr;
            srcaddr = (struct sockaddr *)&localaddr;
        } else if ((pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
                   pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) &&
                   remoteaddr6.sin6_family == AF_INET6 &&
                   localaddr6.sin6_family == AF_INET6) {
            protocolFamily = AF_INET6;
            peeraddr = (struct sockaddr *)&remoteaddr6;
            srcaddr = (struct sockaddr *)&localaddr6;
        }
    } else {
        if ((pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) &&
            remoteaddr6.sin6_family == AF_INET6 &&
            localaddr6.sin6_family == AF_INET6) {
            protocolFamily = AF_INET6;
            peeraddr = (struct sockaddr *)&remoteaddr6;
            srcaddr = (struct sockaddr *)&localaddr6;
        } else if ((pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
                   pdp_type == ANET_DEFAULT_IP_TYPE_V4V6 ) &&
                   remoteaddr.sin_family == AF_INET &&
                   localaddr.sin_family == AF_INET) {
            protocolFamily = AF_INET;
            peeraddr = (struct sockaddr *)&remoteaddr;
            srcaddr = (struct sockaddr *)&localaddr;
        }
    }

    if (!srcaddr) {
        snprintf(str, sizeof(str), RESP_IP_OPEN"%d,%d", index,
                 TCPIP_IPV4_IPV6_NOT_SUPPORT);
        goto EXIT;
    }

    if (listen_info[index].type == SOCK_TCP) {
#if defined CONFIG_CM_SSL_AT || defined CONFIG_GX_SSL_AT
        if (at_tcpip_cfg[index].ssl_enable)
            //sock = aic_mat_create_mbedtls_socket(protocolFamily,
                                                 //peeraddr, srcaddr, index);
            sock = aic_ssl_net_connect(protocolFamily, peeraddr, srcaddr, index);
        else
#endif
            sock = aic_create_mtcp_socket(protocolFamily, peeraddr,
                                          index, srcaddr, NX_TRUE);
    } else if (listen_info[index].type == SOCK_UDP) {
        sock = aic_create_mudp_socket(protocolFamily, peeraddr,
                                      index, srcaddr, NX_TRUE);
        //sock = aic_create_mudp_socket(protocolFamily, peeraddr,
                                      //i, NULL, NX_TRUE);
    } else {
        snprintf(str, sizeof(str), RESP_IP_OPEN"%d,%d", index, TCPIP_UNKNOW_ERR);
        goto EXIT;
    }

    if (sock < 0) {
        snprintf(str, sizeof(str), RESP_IP_OPEN"%d,%d", index, error_code);
        goto EXIT;
    } else {
        if (listen_info[index].type == SOCK_UDP) {
            sock_list[index].status = SOCKET_CONNECT_SUCCESS;
            aic_at_tcpip_socket_connect_ok(index);
        }
    }

    return;

EXIT:
    if (sock_list[index].mode == 1) {
        aic_enter_data_mode_after_connect(AIC_FALSE, str, strlen(str));
    } else {
        aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id, str, strlen(str));
    }

    aos_free(listen_info[index].host);
    aos_memset(&listen_info[index], 0, sizeof(struct aic_at_listen_info));
}

