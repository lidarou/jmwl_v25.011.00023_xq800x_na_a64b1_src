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
#define AIC_THIS_FILE    0x0dd902
#define LOG_MODULE    AMOD_NET

#include <aic_init.h>
#include "aic_at_qtls.h"
#include "aic_at_qtcpip.h"
#ifdef CONFIG_FS
#include "fs_api.h"
#endif
#include "aic_net_api.h"
#ifdef CONFIG_PDP_MGR
#include "aic_net_pdp_mgr.h"
#endif
#include "aic_at_cmd_handle.h"

AIC_SOCKET_LIST ssl_list_q[AT_SOCKET_MAX_CONN_NUM_Q];
static struct at_sock_info ssl_sock_info = {0, 0, NULL, 0};
struct mbedtls_config qsslconf_list[SSL_CTX_ID_MAX];
static u8 recv_cnt;
static aos_thread_id ssl_recv_thread;
static aos_event_ptr ssl_recv_event;
static int error_code;
static AT_EVENT_FLAG at_event_flag[AT_SOCKET_MAX_CONN_NUM_Q];
static aos_mutex_ptr at_ssl_mutex = NULL;
#ifdef CONFIG_CRYPTO_MBEDTLS
static int ssl_ciphersuites[2] = {0, 0};
#endif
extern AT_TCPIP_CFG at_tcpip_cfg;
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];
extern aos_thread_id at_socket_recv_thread_q;

static int aic_qat_mbedtls_config_init(void);
static void aic_qat_ssl_recv_notify(int sock, void *data);
static void aic_qat_ssl_disconn_notify(int sock, void *data, int is_active);

#ifdef CONFIG_NET_AT_QTLS_TEST
static const unsigned char mbedtls_test_cas_pem[] = {                        \
  0x30, 0x82, 0x03, 0x41, 0x30, 0x82, 0x02, 0x29, 0xa0, 0x03, 0x02, 0x01,    \
  0x02, 0x02, 0x01, 0x03, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,    \
  0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x3b, 0x31, 0x0b, 0x30,    \
  0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x4e, 0x4c, 0x31, 0x11,    \
  0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x08, 0x50, 0x6f, 0x6c,    \
  0x61, 0x72, 0x53, 0x53, 0x4c, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55,    \
  0x04, 0x03, 0x0c, 0x10, 0x50, 0x6f, 0x6c, 0x61, 0x72, 0x53, 0x53, 0x4c,    \
  0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x41, 0x30, 0x1e, 0x17, 0x0d,    \
  0x31, 0x39, 0x30, 0x32, 0x31, 0x30, 0x31, 0x34, 0x34, 0x34, 0x30, 0x30,    \
  0x5a, 0x17, 0x0d, 0x32, 0x39, 0x30, 0x32, 0x31, 0x30, 0x31, 0x34, 0x34,    \
  0x34, 0x30, 0x30, 0x5a, 0x30, 0x3b, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,    \
  0x55, 0x04, 0x06, 0x13, 0x02, 0x4e, 0x4c, 0x31, 0x11, 0x30, 0x0f, 0x06,    \
  0x03, 0x55, 0x04, 0x0a, 0x0c, 0x08, 0x50, 0x6f, 0x6c, 0x61, 0x72, 0x53,    \
  0x53, 0x4c, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c,    \
  0x10, 0x50, 0x6f, 0x6c, 0x61, 0x72, 0x53, 0x53, 0x4c, 0x20, 0x54, 0x65,    \
  0x73, 0x74, 0x20, 0x43, 0x41, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06,    \
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00,    \
  0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01,    \
  0x01, 0x00, 0xc0, 0xdf, 0x37, 0xfc, 0x17, 0xbb, 0xe0, 0x96, 0x9d, 0x3f,    \
  0x86, 0xde, 0x96, 0x32, 0x7d, 0x44, 0xa5, 0x16, 0xa0, 0xcd, 0x21, 0xf1,    \
  0x99, 0xd4, 0xec, 0xea, 0xcb, 0x7c, 0x18, 0x58, 0x08, 0x94, 0xa5, 0xec,    \
  0x9b, 0xc5, 0x8b, 0xdf, 0x1a, 0x1e, 0x99, 0x38, 0x99, 0x87, 0x1e, 0x7b,    \
  0xc0, 0x8d, 0x39, 0xdf, 0x38, 0x5d, 0x70, 0x78, 0x07, 0xd3, 0x9e, 0xd9,    \
  0x93, 0xe8, 0xb9, 0x72, 0x51, 0xc5, 0xce, 0xa3, 0x30, 0x52, 0xa9, 0xf2,    \
  0xe7, 0x40, 0x70, 0x14, 0xcb, 0x44, 0xa2, 0x72, 0x0b, 0xc2, 0xe5, 0x40,    \
  0xf9, 0x3e, 0xe5, 0xa6, 0x0e, 0xb3, 0xf9, 0xec, 0x4a, 0x63, 0xc0, 0xb8,    \
  0x29, 0x00, 0x74, 0x9c, 0x57, 0x3b, 0xa8, 0xa5, 0x04, 0x90, 0x71, 0xf1,    \
  0xbd, 0x83, 0xd9, 0x3f, 0xd6, 0xa5, 0xe2, 0x3c, 0x2a, 0x8f, 0xef, 0x27,    \
  0x60, 0xc3, 0xc6, 0x9f, 0xcb, 0xba, 0xec, 0x60, 0x7d, 0xb7, 0xe6, 0x84,    \
  0x32, 0xbe, 0x4f, 0xfb, 0x58, 0x26, 0x22, 0x03, 0x5b, 0xd4, 0xb4, 0xd5,    \
  0xfb, 0xf5, 0xe3, 0x96, 0x2e, 0x70, 0xc0, 0xe4, 0x2e, 0xbd, 0xfc, 0x2e,    \
  0xee, 0xe2, 0x41, 0x55, 0xc0, 0x34, 0x2e, 0x7d, 0x24, 0x72, 0x69, 0xcb,    \
  0x47, 0xb1, 0x14, 0x40, 0x83, 0x7d, 0x67, 0xf4, 0x86, 0xf6, 0x31, 0xab,    \
  0xf1, 0x79, 0xa4, 0xb2, 0xb5, 0x2e, 0x12, 0xf9, 0x84, 0x17, 0xf0, 0x62,    \
  0x6f, 0x27, 0x3e, 0x13, 0x58, 0xb1, 0x54, 0x0d, 0x21, 0x9a, 0x73, 0x37,    \
  0xa1, 0x30, 0xcf, 0x6f, 0x92, 0xdc, 0xf6, 0xe9, 0xfc, 0xac, 0xdb, 0x2e,    \
  0x28, 0xd1, 0x7e, 0x02, 0x4b, 0x23, 0xa0, 0x15, 0xf2, 0x38, 0x65, 0x64,    \
  0x09, 0xea, 0x0c, 0x6e, 0x8e, 0x1b, 0x17, 0xa0, 0x71, 0xc8, 0xb3, 0x9b,    \
  0xc9, 0xab, 0xe9, 0xc3, 0xf2, 0xcf, 0x87, 0x96, 0x8f, 0x80, 0x02, 0x32,    \
  0x9e, 0x99, 0x58, 0x6f, 0xa2, 0xd5, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3,    \
  0x50, 0x30, 0x4e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x04, 0x05,    \
  0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e,    \
  0x04, 0x16, 0x04, 0x14, 0xb4, 0x5a, 0xe4, 0xa5, 0xb3, 0xde, 0xd2, 0x52,    \
  0xf6, 0xb9, 0xd5, 0xa6, 0x95, 0x0f, 0xeb, 0x3e, 0xbc, 0xc7, 0xfd, 0xff,    \
  0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80,    \
  0x14, 0xb4, 0x5a, 0xe4, 0xa5, 0xb3, 0xde, 0xd2, 0x52, 0xf6, 0xb9, 0xd5,    \
  0xa6, 0x95, 0x0f, 0xeb, 0x3e, 0xbc, 0xc7, 0xfd, 0xff, 0x30, 0x0d, 0x06,    \
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00,    \
  0x03, 0x82, 0x01, 0x01, 0x00, 0x38, 0xa8, 0x54, 0x82, 0xb6, 0x1d, 0xaa,    \
  0xdb, 0x6b, 0x89, 0x21, 0xd1, 0x38, 0x28, 0x61, 0xc9, 0xb0, 0x98, 0xd5,    \
  0x11, 0xfc, 0x36, 0xff, 0x19, 0xfe, 0x32, 0x44, 0xef, 0x08, 0xc7, 0xf1,    \
  0x56, 0xbb, 0xe6, 0x46, 0xfa, 0x82, 0xb6, 0x31, 0x38, 0xb4, 0xa0, 0xe5,    \
  0xa0, 0xae, 0x0c, 0xc4, 0x53, 0x9e, 0x93, 0x4e, 0xe8, 0x0d, 0x9c, 0x2f,    \
  0xb4, 0x04, 0xfd, 0x8a, 0x39, 0xf5, 0x84, 0x77, 0xed, 0x4c, 0xd4, 0xbb,    \
  0x44, 0x7f, 0x73, 0x77, 0xf7, 0xf1, 0x36, 0x97, 0xdc, 0x1e, 0x73, 0x19,    \
  0x5d, 0x50, 0xb8, 0xc3, 0x80, 0xcd, 0x03, 0x57, 0xd2, 0x00, 0xdb, 0x56,    \
  0xe6, 0xc6, 0x35, 0x24, 0x1e, 0x49, 0x7b, 0xd8, 0xd2, 0x72, 0xbb, 0x0b,    \
  0x49, 0x2f, 0xa6, 0x02, 0x3c, 0xaf, 0xd0, 0xec, 0x37, 0x1d, 0xbd, 0x81,    \
  0x8b, 0x1f, 0x30, 0xbb, 0xbc, 0x4d, 0x36, 0xb5, 0x79, 0x7c, 0x87, 0xfb,    \
  0x51, 0xb9, 0xbe, 0xc2, 0xde, 0x92, 0xa8, 0x40, 0x71, 0xbb, 0x72, 0x9b,    \
  0xf8, 0x47, 0xce, 0x6c, 0x04, 0xf8, 0x86, 0xe7, 0xf7, 0x73, 0x3c, 0xe7,    \
  0x84, 0x7d, 0xc2, 0xd7, 0xb7, 0x9d, 0xe8, 0xd4, 0x9b, 0x5f, 0x0a, 0x17,    \
  0x7d, 0xbc, 0xbb, 0xb2, 0xd5, 0x94, 0x0d, 0xe4, 0x49, 0xbf, 0x4f, 0x11,    \
  0x68, 0x53, 0xb2, 0x91, 0xff, 0xc0, 0x69, 0xee, 0xdb, 0x63, 0x93, 0xcb,    \
  0xc9, 0x35, 0x6b, 0x90, 0x09, 0xe2, 0x90, 0xc9, 0xed, 0x27, 0xd6, 0x08,    \
  0xfa, 0x13, 0x4d, 0x62, 0xdd, 0xe2, 0x9e, 0xaa, 0xb5, 0xd4, 0x0e, 0x5c,    \
  0x37, 0x4f, 0xab, 0x55, 0x3b, 0x2d, 0xf1, 0x42, 0x82, 0xc7, 0x34, 0x38,    \
  0x1a, 0x9b, 0xeb, 0xa1, 0x2c, 0x0f, 0x29, 0x31, 0x64, 0x6c, 0xcc, 0x38,    \
  0xfd, 0xa9, 0xd3, 0xd5, 0xd5, 0x71, 0xaf, 0xf0, 0x6d, 0xc0, 0x97, 0xe2,    \
  0x11, 0x2a, 0x0a, 0xdf, 0xfe, 0x02, 0x79, 0x74, 0x75                       \
};
/* END FILE */
static unsigned int mbedtls_test_cas_pem_len = sizeof(mbedtls_test_cas_pem);
#endif

AT_CMD_HANDLE qssl_at_cmd[] = {
    {"AT+QSSLCFG", AIC_NULL, AIC_NULL, handle_qsslcfg_set_cmd, AIC_NULL},

    {"AT+QSSLOPEN", handle_qsslopen_test_cmd, AIC_NULL, handle_qsslopen_set_cmd,
     AIC_NULL},

    {"AT+QSSLCLOSE", handle_qsslclose_test_cmd, AIC_NULL,
     handle_qsslclose_set_cmd, AIC_NULL},

    {"AT+QSSLSEND", handle_qsslsend_test_cmd, AIC_NULL, handle_qsslsend_set_cmd,
     AIC_NULL},

    {"AT+QSSLRECV", handle_qsslrecv_test_cmd, AIC_NULL, handle_qsslrecv_set_cmd,
     AIC_NULL},

    {"AT+QSSLSTATE", handle_qsslstate_test_cmd, AIC_NULL,
     handle_qsslstate_set_cmd, AIC_NULL},

    {"AT+QSSLCACERT", AIC_NULL, AIC_NULL, handle_ssl_ca_cert_set_cmd, AIC_NULL},

    {AIC_NULL},
};

int aic_ssl_qat_cmd_init(void)
{
    aic_add_at_cmd_handle(qssl_at_cmd);
    //aic_tcpip_at_cfg_init();
    aic_qat_mbedtls_config_init();

    return AIC_TCPIP_AT_OK;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_ssl_qat_cmd_init);

static int aic_qat_mbedtls_config_init(void)
{
    int i;

    for (i = 0; i < SSL_CTX_ID_MAX; i++) {
        aos_memset(&qsslconf_list[i], 0, sizeof(struct mbedtls_config));
        qsslconf_list[i].ssl_version = SSL_VERSION_ALL;
        qsslconf_list[i].ciphersuite = SSL_CIPHERSUITE_DEFAULT;
        qsslconf_list[i].seclevel = SSL_SECLEVEL_DEFAULT;
        qsslconf_list[i].ignore_ltime = SSL_IGNORE_TIME;
        qsslconf_list[i].negotiate_time = SSL_NEGOTIATE_TIME;
        qsslconf_list[i].sni = SSL_SNI_DEFAULT;
        qsslconf_list[i].closetimeout = SSL_IGNORE_TIME;
    }

    at_ssl_mutex = aos_create_mutex("at_ssl_mutex");
    AIC_ASSERT(at_ssl_mutex);

    return AIC_SUCCESS;
}

void aic_qat_mbedtls_config_reset(int inedx)
{
    aos_memset(&qsslconf_list[inedx], 0, sizeof(struct mbedtls_config));
    qsslconf_list[inedx].ssl_version = SSL_VERSION_ALL;
    qsslconf_list[inedx].ciphersuite = SSL_CIPHERSUITE_DEFAULT;
    qsslconf_list[inedx].seclevel = SSL_SECLEVEL_DEFAULT;
    qsslconf_list[inedx].ignore_ltime = SSL_IGNORE_TIME;
    qsslconf_list[inedx].negotiate_time = SSL_NEGOTIATE_TIME;
    qsslconf_list[inedx].sni = SSL_SNI_DEFAULT;
    qsslconf_list[inedx].closetimeout = SSL_IGNORE_TIME;
}

static void aic_qat_ssl_recv_notify(int sock, void *data)
{
    int i;
    mbedtls_ssl_context *ssl;

    i = aic_at_find_index_by_socket(sock, AT_QSSL);
    if (i == AIC_EPERM || !ssl_list_q[i].ssl)
        return;

    ssl = (mbedtls_ssl_context *)ssl_list_q[i].ssl;
    if (ssl_list_q[i].mode == 0) {
        if (ssl->MBEDTLS_PRIVATE(in_msglen) > 16) {
            aicx_info(0x028afe, "d", "[AT_TCPIP] recv notify len %d\n", ssl->MBEDTLS_PRIVATE(in_msglen));
            return;
        }
    }
    at_event_flag[i].recv_flag = 1;
    aos_set_event(ssl_recv_event, SSL_RECV_EVENT, TX_OR);
}

static void aic_qat_ssl_disconn_notify(int sock, void *data, int is_active)
{
    int i;

    if (is_active == TRUE)
        return;

    i = aic_at_find_index_by_socket(sock, AT_QSSL);
    if (i == AIC_EPERM)
        return;

    at_event_flag[i].close_flag = 1;
    aos_set_event(ssl_recv_event, SSL_CLOSE_EVENT, TX_OR);
}

static void aic_qat_ssl_close(int i)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int sock;

    sock = aic_at_find_socket_by_linkid(i, AT_QSSL);
    if (sock == AIC_EPERM)
        return;
    snprintf(str, AT_RESP_RECV_DATA_BUFFER,
             "+QSSLURC: \"closed\",%d", ssl_list_q[i].link_id);
    aic_transmit_at_cmd_urc(at_tcpip_cfg.com_id, str, strlen(str));

    aic_qat_close_mbedtls_socket(sock);
    aic_socket_unregister_tcp_disconnect_notify(ssl_list_q[i].socket_id);
    aic_socket_unregister_receive_notify(ssl_list_q[i].socket_id);

    aic_end_at_data_mode(at_tcpip_cfg.com_id);
    aos_memset(&ssl_list_q[i], 0, sizeof(struct SOCKET_LIST));
}

static int aic_ssl_net_connect(int family, struct sockaddr *addr,
                        struct sockaddr *srcaddr, int linkid)
{
    int sock, ret, addrlen, len;
    struct sock_errno so_error;
    struct timeval timeout;
    fd_set master_list, write_ready;

    sock = anet_socket(addr->sa_family, SOCK_STREAM, NX_PROTOCOL_TCP);
    if (sock == NX_SOC_ERROR) {
        error_code = AIC_TCPIP_ERR_SOCKET_CREATE_FAIL;
        return AIC_EFAULT;
    }

    if (family == AF_INET)
        addrlen = sizeof(struct sockaddr_in);
    else
        addrlen = sizeof(struct sockaddr_in6);

    if (srcaddr) {
        ret = anet_bind(sock, srcaddr, addrlen);
        if (ret < 0) {
            error_code = AIC_TCPIP_ERR_SOCKET_BIND_FAIL;
            return ret;
        }
    }

    ret = anet_fcntl(sock, F_SETFL, O_NONBLOCK);
    if (ret == NX_SOC_ERROR) {
        soc_close(sock);
        return ret;
    }

    ret = anet_connect(sock, addr, addrlen);
    if (ret < 0) {
        if (errno != EINPROGRESS && errno != EALREADY) {
            error_code = AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL;
            soc_close(sock);
            return ret;
        }
    }

    FD_ZERO(&write_ready);
    FD_ZERO(&master_list);
    FD_SET(sock, &master_list);
    write_ready = master_list;

    timeout.tv_sec = SSL_CONECT_TIMEOUT;
    timeout.tv_usec = 0;
    ret = anet_select(FD_SETSIZE, NULL, &write_ready, NULL, &timeout);
    if (ret == ERROR || ret == 0) {
        error_code = AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL;
        soc_close(sock);
        return AIC_EFAULT;
    } else {
        len = sizeof(struct sock_errno);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error.error != 0) {
            aicx_info(0x08ef28, "d", "[AT_TCPIP] socket connect fail error %d\n",
                      so_error.error);
            error_code = AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL;
            soc_close(sock);
            return AIC_EFAULT;
        }
    }

    return sock;
}

int aic_qat_create_mbedtls_socket(int family, struct sockaddr *addr,
                                  struct sockaddr *srcaddr, int linkid)
{
    int sock, i, ret, ssl_id, count = 0;
    struct timeval timeout;
    mbedtls_net_context *server_fd;
    const char *pers = "at_ssl_client";
    mbedtls_entropy_context *entropy;
    mbedtls_ctr_drbg_context *ctr_drbg;
    mbedtls_ssl_context *ssl;
    mbedtls_ssl_config *conf;
    mbedtls_x509_crt *cacert;
    mbedtls_pk_context *pkey = NULL;

    i = linkid;
    server_fd = &ssl_list_q[i].sockfd;
    ssl_id = ssl_list_q[i].ssl_ctx_id;
    mbedtls_net_init(server_fd);

    ssl = aos_zalloc(sizeof(mbedtls_ssl_context));
    if (!ssl)
        goto exit;
    mbedtls_ssl_init(ssl);

    conf = aos_zalloc(sizeof(mbedtls_ssl_config));
    if (!conf)
        goto exit1;
    mbedtls_ssl_config_init(conf);

    cacert = aos_zalloc(sizeof(mbedtls_x509_crt));
    if (!cacert)
        goto exit2;
    mbedtls_x509_crt_init(cacert);

    ctr_drbg = aos_zalloc(sizeof(mbedtls_ctr_drbg_context));
    if (!ctr_drbg)
        goto exit3;
    mbedtls_ctr_drbg_init(ctr_drbg);

    entropy = aos_zalloc(sizeof(mbedtls_entropy_context));
    if (!entropy)
        goto exit4;
    mbedtls_entropy_init(entropy);

    ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                                (const unsigned char *) pers,
                                strlen(pers));
    if(ret != 0 ) {
        aicx_error(0x133080, "d", "[AT_TLS] mbedtls_ctr_drbg_seed failed %d\n", ret);
    }
    if (qsslconf_list[i].seclevel >= 1 && qsslconf_list[ssl_id].cacert) {
#ifdef CONFIG_NET_AT_QTLS_TEST
        ret = mbedtls_x509_crt_parse(cacert,
                                     (const unsigned char *) mbedtls_test_cas_pem,
                                     mbedtls_test_cas_pem_len);
#else
        ret = mbedtls_x509_crt_parse(cacert, qsslconf_list[ssl_id].cacert,
                                     (size_t)qsslconf_list[ssl_id].cacert_len);
#endif
        if( ret < 0 ) {
            aicx_error(0x0831bc, "x", "[AT_TLS] mbedtls_x509_crt_parse failed -0x%x\n",
                       (unsigned int) -ret);
            goto exit5;
        }
    }

    if (qsslconf_list[ssl_id].seclevel > 1 && qsslconf_list[ssl_id].client_cert &&
        qsslconf_list[ssl_id].client_key) {
        ret = mbedtls_x509_crt_parse(cacert, qsslconf_list[ssl_id].client_cert,
                                     (size_t)qsslconf_list[ssl_id].client_cert_len);
        if( ret < 0 ) {
            aicx_error(0x0831bc, "x", "[AT_TLS] mbedtls_x509_crt_parse failed -0x%x\n",
                       (unsigned int) -ret);
            goto exit5;
        }

        pkey = aos_zalloc(sizeof(mbedtls_pk_context));
        if (!pkey)
            goto exit5;
        mbedtls_pk_init(pkey);
        ret = mbedtls_pk_parse_key(pkey, (const unsigned char *)qsslconf_list[ssl_id].client_key,
                                   qsslconf_list[ssl_id].client_key_len, NULL, 0,
                                   mbedtls_ctr_drbg_random, &ctr_drbg);
        if (ret != 0) {
            aicx_error(0x0831bc, "x", "[AT_TLS] mbedtls_x509_crt_parse failed -0x%x\n",
                       (unsigned int) -ret);
            goto exit5;
        }
    }

    sock = aic_ssl_net_connect(family, addr, srcaddr, linkid);
    //ret = mbedtls_net_connect(server_fd, addr, port, MBEDTLS_NET_PROTO_TCP);
    if (sock < 0) {
        aicx_error(0x033278, "d", "[AT_TLS] mbedtls_net_connect failed %d\n");
        goto exit5;
    }
    server_fd->fd = sock;
    aicx_info(0x0602a9, "d", "[AT_TLS]: socket %d connected", sock);

    ret = mbedtls_ssl_config_defaults(conf,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        aicx_error(0x02d51b, "d", "[AT_TLS] mbedtls_ssl_config_defaults failed %d\n", ret);
        goto exit5;
    }

    mbedtls_ssl_conf_authmode(conf, qsslconf_list[ssl_id].seclevel);
    mbedtls_ssl_conf_ca_chain(conf, cacert, NULL);
    if (qsslconf_list[ssl_id].seclevel > 1) {
        mbedtls_ssl_conf_own_cert(conf, cacert->next, pkey);
    }
    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);
    //mbedtls_ssl_conf_dbg(conf, my_debug, NULL);

    if (qsslconf_list[ssl_id].ciphersuite != 0xFFFF) {
        ssl_ciphersuites[0] = (int)qsslconf_list[ssl_id].ciphersuite;
        mbedtls_ssl_conf_ciphersuites(conf, (const int *)ssl_ciphersuites);
    }

    ret = mbedtls_ssl_setup(ssl, conf);
    if (ret != 0) {
        aicx_error(0x12a8fe, "d", "[AT_TLS] mbedtls_ssl_setup failed %d\n", ret);
        goto exit5;
    }

    mbedtls_ssl_set_bio(ssl, server_fd, mbedtls_net_send,
                        mbedtls_net_recv, NULL );

#if 0
    if (ssl_list_q[ssl_id].common_name) {
        ret = mbedtls_ssl_set_hostname(ssl, (const char *)ssl_list_q[ssl_id].common_name);
        if (ret != 0) {
            aicx_error(0x091132, "d", "[AT_TLS] mbedtls_ssl_set_hostname failed %d\n", ret);
            goto exit5;
        }
    }
#endif
    while (1) {
        ret = mbedtls_ssl_handshake(ssl);
        if(ret == 0)
            break;

        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            aicx_error(0x000443, "x", "mbedtls_ssl_handshake failed  0x%x\n",
                       (unsigned int) -ret );
            goto exit5;
        }

        aos_sleep(500);
        count++;
        if (count == SSL_HANDSHARK_RETRANS_COUNT)
            goto exit5;
    }

    timeout.tv_sec = AT_SOCKET_SEND_TIMEOUT;
    timeout.tv_usec = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret)
        aicx_info(0x19eb7e, "x", "[AT_TLS] set tls send timeout fail 0x%x\r\n", ret);

    ret = anet_fcntl(sock, F_SETFL, O_NONBLOCK);
    if (ret == NX_SOC_ERROR) {
        aicx_info(0x128b7a, "x", "[AT_TLS] set tls socket nonblockfail 0x%x\r\n", ret);
        mbedtls_ssl_close_notify(ssl);
        goto exit5;
    }

    aic_socket_register_receive_notify(sock, aic_qat_ssl_recv_notify, NULL);
    aic_socket_register_tcp_disconnect_notify(sock,
                                              aic_qat_ssl_disconn_notify,
                                              NULL);

    if (ssl_list_q[i].socket_id == 0) {
        ssl_list_q[i].link_id = linkid;
        ssl_list_q[i].socket_id = sock;
        ssl_list_q[i].family = family;
        ssl_list_q[i].ssl = ssl;
        ssl_list_q[i].key = pkey;
        if (family == AF_INET)
            ssl_list_q[i].socket_type = SOCK_TLS;
        else
            ssl_list_q[i].socket_type = SOCK_TLSv6;
    }

    return sock;

exit5:
    if (pkey) {
        mbedtls_pk_free(pkey);
        aos_free(pkey);
    }

    mbedtls_entropy_free(entropy);
    aos_free(entropy);
exit4:
    mbedtls_ctr_drbg_free(ctr_drbg);
    aos_free(ctr_drbg);
exit3:
    mbedtls_x509_crt_free(cacert);
    aos_free(cacert);
exit2:
    mbedtls_ssl_config_free(conf);
    aos_free(conf);
exit1:
    mbedtls_ssl_free(ssl);
    aos_free(ssl);
exit:
    mbedtls_net_free(server_fd);

    error_code = AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL;
    return -1;
}

int aic_qat_close_mbedtls_socket(int socket_fd)
{
    int index;
    mbedtls_ssl_context *ssl;
    mbedtls_ssl_config *conf;
    mbedtls_ctr_drbg_context *ctr_drbg;

    index = aic_at_find_index_by_socket(socket_fd, AT_QSSL);
    ssl = ssl_list_q[index].ssl;
    if (!ssl)
        goto exit;

    mbedtls_ssl_close_notify(ssl);
    conf = (mbedtls_ssl_config *)ssl->MBEDTLS_PRIVATE(conf);
    if (!conf)
        goto exit;

    ctr_drbg = (mbedtls_ctr_drbg_context *)conf->MBEDTLS_PRIVATE(p_rng);
    mbedtls_entropy_free(ctr_drbg->MBEDTLS_PRIVATE(p_entropy));
    if (ctr_drbg->MBEDTLS_PRIVATE(p_entropy)) {
        aos_free(ctr_drbg->MBEDTLS_PRIVATE(p_entropy));
        ctr_drbg->MBEDTLS_PRIVATE(p_entropy) = NULL;
    }
    mbedtls_ctr_drbg_free(ctr_drbg);
    if (conf->MBEDTLS_PRIVATE(authmode) == MBEDTLS_SSL_VERIFY_REQUIRED) {
        mbedtls_pk_free(ssl_list_q[index].key);
        if (ssl_list_q[index].key) {
            aos_free(ssl_list_q[index].key);
            ssl_list_q[index].key = NULL;
        }
    }
    mbedtls_x509_crt_free(conf->MBEDTLS_PRIVATE(ca_chain));
    if (conf->MBEDTLS_PRIVATE(ca_chain)) {
        aos_free(conf->MBEDTLS_PRIVATE(ca_chain));
        conf->MBEDTLS_PRIVATE(ca_chain) = NULL;
    }
    mbedtls_ssl_config_free(conf);
    mbedtls_ssl_free(ssl);

    if (ctr_drbg) {
        aos_free(ctr_drbg);
        conf->MBEDTLS_PRIVATE(p_rng) = NULL;
    }

    if (conf) {
        aos_free(conf);
        ssl->MBEDTLS_PRIVATE(conf) = NULL;
    }

    if (ssl) {
        aos_free(ssl);
        ssl_list_q[index].ssl = NULL;
    }

exit:
    mbedtls_net_free(&ssl_list_q[index].sockfd);
    //aic_qat_mbedtls_config_reset(index);
    return AIC_SUCCESS;
}


int aic_read_cert_file(u8 *file, int ssl_ctx_id, int type)
{
#ifdef CONFIG_FS
    int ret, fd, rsize;
    FS_FILE_INFO *file_info;
    u8 *cert;

    fd = fs_open(file, FS_O_RDONLY);
    if (fd < 0) {
        aicx_error(0x028249, "s", "[AT_TLS]: open file fail, file patch %s!\n", file);
        return AIC_EFAULT;
    }

    file_info = aos_malloc(sizeof(FS_FILE_INFO));
    ret = fs_getinfo(file, file_info);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x1c3e63, "s", "[AT_TLS]: get file %s info fail\n", file);
        fs_close(fd);
        aos_free(file_info);
        return AIC_EFAULT;
    }

    ret = fs_seek(fd, 0, FS_SEEK_BEGIN);
    if (ret != 0) {
        fs_close(fd);
        aos_free(file_info);
        return AIC_EFAULT;
    }

    cert = aos_zalloc(file_info->size + 1);
    rsize = fs_read(fd, cert, file_info->size);
    if (rsize <= 0) {
        aicx_error(0x149f8c, "", "[AT_TLS]: read file fail\n", file);
        fs_close(fd);
        aos_free(file_info);
        aos_free(cert);
        return AIC_EFAULT;
    }

    ret = fs_close(fd);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x113c52, "", "[AT_TLS]: close file fail\n", file);
        aos_free(file_info);
        aos_free(cert);
        return AIC_EFAULT;
    }

    if (type == SSL_CA_CERT) {
        if (qsslconf_list[ssl_ctx_id].cacert) {
            aos_free(qsslconf_list[ssl_ctx_id].cacert);
        }
        qsslconf_list[ssl_ctx_id].cacert = cert;
        qsslconf_list[ssl_ctx_id].cacert_len = file_info->size + 1;
    } else if (type == SSL_CLIENT_CERT) {
        if (qsslconf_list[ssl_ctx_id].client_cert) {
            aos_free(qsslconf_list[ssl_ctx_id].client_cert);
        }
        qsslconf_list[ssl_ctx_id].client_cert = cert;
        qsslconf_list[ssl_ctx_id].client_cert_len = file_info->size + 1;
    } else if (type == SSL_CLIENT_KEY) {
        if (qsslconf_list[ssl_ctx_id].client_key) {
            aos_free(qsslconf_list[ssl_ctx_id].client_key);
        }
        qsslconf_list[ssl_ctx_id].client_key = cert;
        qsslconf_list[ssl_ctx_id].client_key_len = file_info->size + 1;
    } else
        return AIC_ENOTSUP;

    aos_free(file_info);
    return AIC_SUCCESS;
#else
    return AIC_EFAULT;
#endif
}

u32 handle_qsslcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ret, id, str_len, num;
    u8 paras[8] = {'\0'};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QSSLCFG=",
                                             &cnt);
    if (!paras_arr || cnt < 2 || cnt > 3) {
        aicx_error(0x023651, "", "[AT_TLS]: parameter parse error\n");
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        id = atoi((u8 *)paras_arr[1]);
        if (id >= SSL_CTX_ID_MAX) {
            goto AT_TLS_EXIT;
        }
        qsslconf_list[id].ssl_context_id = id;
    } else {
        goto AT_TLS_EXIT;
    }

    str_len = strlen((u8*)paras_arr[0]);
    if (strncasecmp((u8*)paras_arr[0], "sslversion", 10) == 0 &&
        str_len == 10) {
        if (cnt > 2) {
            num = SSL_VERSION_ALL;
            if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                num = atoi((u8*)paras_arr[2]);
            } else {
                goto AT_TLS_EXIT;
            }
            if (num > 4 || num < 0) {
                goto AT_TLS_EXIT;
            } else {
                qsslconf_list[id].ssl_version = num;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"sslversion\",%d,%d",
                     id, qsslconf_list[id].ssl_version);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ciphersuite", 11) == 0 &&
               str_len == 11) {
        if (cnt > 2) {
            num = SSL_CIPHERSUITE_DEFAULT;
            if (strlen((u8 *)paras_arr[2]) == 6) {
                if (strncasecmp((u8 *)paras_arr[2], "0x", 2) == 0) {
                    aos_memcpy(paras, (u8*)paras_arr[2], 6);
                    num = aic_at_string_to_num(&paras[2]);
                } else {
                    goto AT_TLS_EXIT;
                }
            } else {
                goto AT_TLS_EXIT;
            }

            if (num > 0xFFFF || num < 0) {
                goto AT_TLS_EXIT;
            } else {
                qsslconf_list[id].ciphersuite = num;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"ciphersuite\",%d,0x%x",
                     id, qsslconf_list[id].ciphersuite);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "cacert", 6) == 0 &&
               str_len == 6) {
        if (cnt > 2) {
            ret = aic_read_cert_file((u8*)paras_arr[2], id, SSL_CA_CERT);
            if (ret != AIC_SUCCESS)
                goto AT_TLS_EXIT;
            if (strcasestr((u8*)paras_arr[2], ".pem") || strcasestr((u8*)paras_arr[2], ".cer") ||
                strcasestr((u8*)paras_arr[2], ".crt") || strcasestr((u8*)paras_arr[2], ".der") ||
                strcasestr((u8*)paras_arr[2], ".key")) {
                strncpy(qsslconf_list[id].cacert_path, (u8*)paras_arr[2], 128);
                aicx_info(0x04ff66, "dd", "cacert_path:%d,paras[2]:%d\n",qsslconf_list[id].cacert_path,(u8*)paras_arr[2]);
            } else {
                aicx_info(0x04ff66, "dd", "cacert_path:%d,paras[2]:%d\n",qsslconf_list[id].cacert_path,(u8*)paras_arr[2]);
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"cacert\",%d,%s",
                     id, qsslconf_list[id].cacert_path);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "clientcert", 10) == 0 &&
               str_len == 10) {
        if (cnt > 2) {
            ret = aic_read_cert_file((u8*)paras_arr[2], id, SSL_CLIENT_CERT);
            if (ret != AIC_SUCCESS)
                goto AT_TLS_EXIT;
            if (strcasestr((u8*)paras_arr[2], ".pem") || strcasestr((u8*)paras_arr[2], ".cer") ||
                strcasestr((u8*)paras_arr[2], ".crt") || strcasestr((u8*)paras_arr[2], ".der") ||
                strcasestr((u8*)paras_arr[2], ".key")) {
                strncpy(qsslconf_list[id].client_cert_path, (u8*)paras_arr[2], 128);
            } else {
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"clientcert\",%d,%s",
                     id, qsslconf_list[id].client_cert_path);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "clientkey", 9) == 0 &&
               str_len == 9) {
        if (cnt > 2) {
            ret = aic_read_cert_file((u8*)paras_arr[2], id, SSL_CLIENT_KEY);
            if (ret != AIC_SUCCESS)
                goto AT_TLS_EXIT;
            if (strcasestr((u8*)paras_arr[2], ".pem") || strcasestr((u8*)paras_arr[2], ".cer") ||
                strcasestr((u8*)paras_arr[2], ".crt") || strcasestr((u8*)paras_arr[2], ".der") ||
                strcasestr((u8*)paras_arr[2], ".key")) {
                strncpy(qsslconf_list[id].client_key_path, (u8*)paras_arr[2], 128);
            } else {
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"clientkey\",%d,%s",
                     id, qsslconf_list[id].client_key_path);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "seclevel", 8) == 0 &&
               str_len == 8) {
        if (cnt > 2) {
            num = SSL_SECLEVEL_DEFAULT;
            if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                num = atoi((u8*)paras_arr[2]);
            } else {
                goto AT_TLS_EXIT;
            }
            if (num > 2) {
                goto AT_TLS_EXIT;
            } else {
                qsslconf_list[id].seclevel = num;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"seclevel\",%d,%d",
                     id, qsslconf_list[id].seclevel);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ignorelocaltime", 15) == 0 &&
               str_len == 15) {
        if (cnt > 2) {
            num = 1;
            if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                num = atoi((u8*)paras_arr[2]);
            } else {
                goto AT_TLS_EXIT;
            }
            if (num > 1) {
                goto AT_TLS_EXIT;
            } else {
                qsslconf_list[id].ignore_ltime = num;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"ignorelocaltime\",%d,%d",
                     id, qsslconf_list[id].ignore_ltime);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "negotiatetime", 13) == 0 &&
               str_len == 13) {
        if (cnt > 2) {
            num = SSL_NEGOTIATE_TIME;
            if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                num = atoi((u8*)paras_arr[2]);
            } else {
                goto AT_TLS_EXIT;
            }
            if (num < 10 || num > SSL_NEGOTIATE_TIME) {
                goto AT_TLS_EXIT;
            } else {
                qsslconf_list[id].negotiate_time = num;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"negotiatetime\",%d,%d",
                     id, qsslconf_list[id].negotiate_time);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "sni", 3) == 0 && str_len == 3) {
        if (cnt > 2) {
            num = 0;
            if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                num = atoi((u8*)paras_arr[2]);
            } else {
                goto AT_TLS_EXIT;
            }
            if (num != 0 && num != 1) {
                goto AT_TLS_EXIT;
            } else {
                qsslconf_list[id].sni = num;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"sni\",%d,%d",
                     id, qsslconf_list[id].sni);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "closetimemode", 13) == 0 &&
               str_len == 13) {
        if (cnt > 2) {
            num = 1;
            if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                num = atoi((u8*)paras_arr[2]);
            } else {
                goto AT_TLS_EXIT;
            }
            if (num > 1) {
                goto AT_TLS_EXIT;
            } else {
                qsslconf_list[id].closetimeout = num;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG: \"closetimemode\",%d,%d",
                     id, qsslconf_list[id].closetimeout);
        }
    } else {
        goto AT_TLS_EXIT;
    }

    if (str[0] != '\0')
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

AT_TLS_EXIT:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;

}

u32 handle_qsslopen_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str),
        "+QSSLOPEN: (1-5),(0-9),(0-11),\"ipv4/ipv6\",(1-65535),(0-2)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
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
u32 handle_qsslopen_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras_arr;
    u16 cnt;
    u32 len, protocolFamily;
    struct addrinfo *res = NULL;
    struct sockaddr *peeraddr, *srcaddr = NULL;
    struct sockaddr_in remoteaddr, localaddr;
    struct sockaddr_in6 remoteaddr6, localaddr6;
    int linkid = 0, sock, index = 0, ret, cid, mode, port;
    WAN_NET *net_ptr;
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+QSSLOPEN=",
                                                 &cnt);
        if (!paras_arr || cnt < 5 || cnt > 6) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            if (paras_arr)
                aic_free_paras_mem(paras_arr, cnt);

            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }

        if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
            linkid = atoi((u8 *)paras_arr[2]);
        else
            goto ERROR_EXIT;

        if (linkid >= AT_SOCKET_MAX_CONN_NUM_Q || linkid < 0) {
            goto ERROR_EXIT;
        }

        index = linkid;
        if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
            cid = atoi((u8 *)paras_arr[0]);
        else
            goto ERROR_EXIT;

        if (cid > 0 && cid <= ANET_MAX_PDP_CID) {
            ssl_list_q[index].pdp_cid = cid;
        } else {
            goto ERROR_EXIT;
        }

        if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            cid = atoi((u8 *)paras_arr[1]);
        else
            goto ERROR_EXIT;

        if (cid >= SSL_CTX_ID_MAX || cid < 0) {
            aicx_error(0x17b523, "", "[AT_TCPIP]: socket ssl_ctx id error\n");
            goto ERROR_EXIT;
        } else {
            ssl_list_q[index].ssl_ctx_id = cid;
        }

        if (ssl_list_q[index].in_use || ssl_list_q[index].socket_id) {
            goto ERROR_EXIT;
        }

        ret = getaddrinfo((u8 *)paras_arr[3], AIC_NULL, AIC_NULL, &res);
        if (ret != NX_SUCCESS) {
            aicx_error(0x1c0e8f, "sd", "[AT_SSL]: get addr info fail %s ret %d\r\n",
                       (s8 *)paras_arr[3], ret);
            aic_free_paras_mem(paras_arr, cnt);
            aic_transmit_resp_error(at_req_resp_ptr, AIC_TCPIP_ERR_DNS_FAIL);
            return ret;
        }

        if (aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4]))) {
            port = atoi((u8 *)paras_arr[4]);
            if (port > 65535 || port < 1)
                goto ERROR_EXIT;
        } else {
            goto ERROR_EXIT;
        }

        if (res->ai_family == AF_INET) {
            protocolFamily = AF_INET;
            aos_memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
            remoteaddr.sin_family = AF_INET;
            remoteaddr.sin_port = htons(port);
            remoteaddr.sin_addr.s_addr =
                ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
            peeraddr = (struct sockaddr *)&remoteaddr;
        } else {
            protocolFamily = AF_INET6;
            aos_memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
            remoteaddr6.sin6_family = AF_INET6;
            remoteaddr6.sin6_port = htons(port);
            aos_memcpy(&remoteaddr6.sin6_addr,
                       &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
            peeraddr = (struct sockaddr *)&remoteaddr6;
        }

        if (cnt == 6) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[5], strlen((u8 *)paras_arr[5])))
                mode = atoi((u8 *)paras_arr[5]);
            else
                goto ERROR_EXIT;

            if (mode >= 0 && mode < 3)
                ssl_list_q[index].mode = mode;
            else
                goto ERROR_EXIT;
        }

        net_ptr = aic_get_wan_net_per_pdp(ssl_list_q[index].pdp_cid);
        if (net_ptr) {
            if (protocolFamily == AF_INET) {
                localaddr.sin_family = AF_INET;
                localaddr.sin_port = htons(NX_ANY_PORT);
                localaddr.sin_addr.s_addr = htonl(net_ptr->wan_ipv4_address);
                srcaddr = (struct sockaddr *)&localaddr;
            } else {
                localaddr6.sin6_family = AF_INET6;
                localaddr6.sin6_port = htons(NX_ANY_PORT);
                aos_memcpy(&localaddr6.sin6_addr,
                           net_ptr->wan_ipv6_address1,
                           16);
                NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[0]);
                NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[1]);
                NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[2]);
                NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[3]);
                srcaddr = (struct sockaddr *)&localaddr6;
            }
        }
        sock = aic_qat_create_mbedtls_socket(protocolFamily, peeraddr,
                                             srcaddr, linkid);
        if (sock < 0) {
            aic_transmit_resp_error(at_req_resp_ptr, error_code);
            goto OPEN_EXIT;
        }

        ssl_sock_info.sockid = sock;
        at_tcpip_cfg.com_id = at_req_resp_ptr->serial_port_id;
        if (ssl_list_q[index].mode == 2) {
            ret = aic_allowed_to_enter_data_mode(at_req_resp_ptr->serial_port_id);
            if (ret == FALSE) {
                aic_transmit_resp_error(at_req_resp_ptr,
                                        AIC_TCPIP_ERR_ENTER_DATA_MODE_FAIL);
                aic_qat_close_mbedtls_socket(sock);
                aos_memset(&ssl_list_q[index], 0, sizeof(struct SOCKET_LIST));
                goto OPEN_EXIT;
            }
            at_req_resp_ptr->at_mode = AT_DATA_MODE;
            ssl_sock_info.offset = 0;
            strcpy(str, "CONNECT");
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            goto OPEN_EXIT;
        }

        snprintf(str, sizeof(str), "+QSSLOPEN: %d,%d", linkid, 0);
        aic_transmit_resp_ok(at_req_resp_ptr);
        aic_transmit_at_cmd_urc(at_req_resp_ptr->serial_port_id, str, strlen(str));
OPEN_EXIT:
        freeaddrinfo(res);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
ERROR_EXIT:
        if (res)
            freeaddrinfo(res);
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        if (at_req_resp_ptr->end_data_mode) {
            if (ssl_sock_info.buf)
                aos_free(ssl_sock_info.buf);
            aos_memset(&ssl_sock_info, 0, sizeof(struct at_sock_info));
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == NULL)
            return AIC_TCPIP_AT_ERR;

        if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
            if (ssl_sock_info.buf)
                aos_free(ssl_sock_info.buf);
            ssl_sock_info.buf = NULL;
            ssl_sock_info.offset = 0;
            ssl_list_q[index].mode = 0;
            at_req_resp_ptr->at_mode = AT_TMP_CMD_MODE;
            aicx_info(0x124873, "", "[AT_TCPIP]: change to AT conmand mode\n");
            //aic_transmit_resp_exit(at_req_resp_ptr);
            aic_transmit_resp_ok(at_req_resp_ptr);
            return AIC_TCPIP_AT_OK;
        }  else if (strncmp(at_req_resp_ptr->at_upstream_data, "+", 1) == 0) {
            recv_cnt++;
            if (recv_cnt == 3) {
                if (ssl_sock_info.buf)
                    aos_free(ssl_sock_info.buf);
                ssl_sock_info.buf = NULL;
                ssl_sock_info.offset = 0;
                ssl_list_q[index].mode = 0;
                at_req_resp_ptr->at_mode = AT_TMP_CMD_MODE;
                aic_transmit_resp_ok(at_req_resp_ptr);
                return AIC_TCPIP_AT_OK;
            }

            if (!ssl_sock_info.buf)
                ssl_sock_info.buf = aos_zalloc(AT_SOCKET_DATA_LEN_MAX);

            len = at_req_resp_ptr->upstream_data_len;
            aos_memcpy(ssl_sock_info.buf + ssl_sock_info.offset,
                       at_req_resp_ptr->at_upstream_data, len);
            ssl_sock_info.offset += len;
            return AIC_TCPIP_AT_OK;
        } else {
            recv_cnt = 0;
        }

        len = at_req_resp_ptr->upstream_data_len;
        if (len > AT_SOCKET_DATA_LEN_MAX) {
            len = AT_SOCKET_DATA_LEN_MAX;
        }

        len = at_req_resp_ptr->upstream_data_len;
        if (!ssl_sock_info.buf)
            ssl_sock_info.buf = aos_zalloc(AT_SOCKET_DATA_LEN_MAX);

        aos_memcpy(ssl_sock_info.buf + ssl_sock_info.offset,
                   at_req_resp_ptr->at_upstream_data, len);
        ssl_sock_info.offset += len;

        ret = mbedtls_ssl_write((mbedtls_ssl_context *)ssl_list_q[index].ssl,
                                ssl_sock_info.buf, ssl_sock_info.offset);
        if (ret < 0) {
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", (errno));
        } else {
            aicx_info(0x1d42d6, "d", "[AT_TCPIP]: send ret %d\n", ret);
        }

        if (ssl_sock_info.buf)
            aos_free(ssl_sock_info.buf);
        ssl_sock_info.buf = NULL;
        ssl_sock_info.offset = 0;
        return AIC_TCPIP_AT_OK;
    } else {
        if (at_req_resp_ptr->at_mode == AT_TMP_CMD_MODE) {
            if (ssl_sock_info.buf)
                aos_free(ssl_sock_info.buf);
            aos_memset(&ssl_sock_info, 0, sizeof(struct at_sock_info));
            return AIC_TCPIP_AT_OK;
        }
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

u32 handle_qsslclose_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str),
        "+QSSLCLOSE: (0-11),(0-65535)");
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
u32 handle_qsslclose_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    int sock = 0, link_id = 0, close_timeout = 0;
    int index;
    ulong *paras_arr;
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QSSLCLOSE=",
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
        goto ERROR_EXIT;

    if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
        goto ERROR_EXIT;
    }

    if (cnt == 2) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            close_timeout = atoi((u8 *)paras_arr[1]);
        else
            goto ERROR_EXIT;

        if (close_timeout >= 65535 || close_timeout < 0) {
            goto ERROR_EXIT;
        }
    }
    index = aic_at_find_index_by_linkid(link_id, AT_QSSL);

    if (!ssl_list_q[index].ssl){
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    sock = ssl_list_q[index].socket_id;
    aos_get_mutex(at_ssl_mutex, AOS_WAIT_FOREVER);
    aic_socket_unregister_tcp_disconnect_notify(sock);
    aic_socket_unregister_receive_notify(sock);
    aic_qat_close_mbedtls_socket(sock);
    aic_end_at_data_mode(at_tcpip_cfg.com_id);
    aos_put_mutex(at_ssl_mutex);
    aos_memset(&ssl_list_q[index], 0, sizeof(struct SOCKET_LIST));

    aic_transmit_resp_ok(at_req_resp_ptr);
    snprintf(str, sizeof(str), "+QSSLURC: \"closed\",%d", link_id);
    aic_transmit_at_cmd_urc(at_req_resp_ptr->serial_port_id, str, strlen(str));
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

ERROR_EXIT:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 handle_qsslsend_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QSSLSEND: (0-11),(1-1460)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
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
u32 handle_qsslsend_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index, len, link_id = 0, ret;
    ulong *paras_arr;
    u16 cnt;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+QSSLSEND=",
                                                 &cnt);
        if (!paras_arr || cnt < 1 || cnt > 2) {
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
            goto ERROR_EXIT;

        if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
            goto ERROR_EXIT;
        }

        ssl_sock_info.sockid = aic_at_find_socket_by_linkid(link_id, AT_QSSL);
        if (!ssl_sock_info.sockid){
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (!ssl_list_q[link_id].ssl) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        ssl_sock_info.length = AT_SOCKET_DATA_LEN_DEFAULT;
        ssl_sock_info.offset = 0;
        ssl_sock_info.is_send = AIC_FALSE;
        if (cnt == 2) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                ssl_sock_info.length = atoi((u8 *)paras_arr[1]);
            else
                goto ERROR_EXIT;
            if (ssl_sock_info.length > AT_SOCKET_DATA_LEN_MAX ||
                ssl_sock_info.length < AT_SOCKET_DATA_LEN_MIN) {
                goto ERROR_EXIT;
            }

            ssl_sock_info.is_send = AIC_TRUE;
        }

        at_req_resp_ptr->at_mode = AT_DATA_MODE;
        strcpy(str, ">");
        aic_transmit_resp_normal(at_req_resp_ptr, str);
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
            if (ssl_sock_info.buf)
                aos_free(ssl_sock_info.buf);
            aos_memset(&ssl_sock_info, 0, sizeof(struct at_sock_info));
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (at_req_resp_ptr->at_upstream_data == NULL)
            return AIC_TCPIP_AT_ERR;

        index = aic_at_find_index_by_socket(ssl_sock_info.sockid, AT_QSSL);

        len = at_req_resp_ptr->upstream_data_len;
        if (len > AT_SOCKET_DATA_LEN_MAX) {
            len = AT_SOCKET_DATA_LEN_MAX;
        }

        if (!ssl_sock_info.buf)
            ssl_sock_info.buf = aos_zalloc_relaxed(ssl_sock_info.length);

        if (!ssl_sock_info.buf) {
            strcpy(str, "ERROR");
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            return AIC_TCPIP_AT_OK;
        }

        if (ssl_sock_info.length == AT_SOCKET_DATA_LEN_DEFAULT) {
            if (*at_req_resp_ptr->at_upstream_data == 0x1B) {
                at_req_resp_ptr->at_mode = AT_CMD_MODE;
                ssl_list_q[index].mode = 0;
                aos_free(ssl_sock_info.buf);
                aos_memset(&ssl_sock_info, 0, sizeof(struct at_sock_info));
                strcpy(str, "ERROR");
                aic_transmit_resp_normal(at_req_resp_ptr, str);
                return AIC_TCPIP_AT_OK;
            }
            if (*at_req_resp_ptr->at_upstream_data == 0x1A) {
                len = ssl_sock_info.offset;
                goto SSl_SEND;
            }

            if (AT_SOCKET_DATA_LEN_DEFAULT - ssl_sock_info.offset <= len) {
                len = AT_SOCKET_DATA_LEN_DEFAULT - ssl_sock_info.offset;
                aos_memcpy(ssl_sock_info.buf + ssl_sock_info.offset,
                           at_req_resp_ptr->at_upstream_data, len);
            } else {
                aos_memcpy(ssl_sock_info.buf + ssl_sock_info.offset,
                           at_req_resp_ptr->at_upstream_data, len);
            }
            ssl_sock_info.offset += len;
            if (ssl_sock_info.length == ssl_sock_info.offset && ssl_sock_info.is_send) {
                len = ssl_sock_info.offset;
                goto SSl_SEND;
            }
            return AIC_TCPIP_AT_OK;
        } else {
            if (ssl_sock_info.length - ssl_sock_info.offset <= len)
                len = ssl_sock_info.length - ssl_sock_info.offset;
            aos_memcpy(ssl_sock_info.buf + ssl_sock_info.offset,
                       at_req_resp_ptr->at_upstream_data, len);
            ssl_sock_info.offset += len;
            if (ssl_sock_info.length == ssl_sock_info.offset) {
                len = ssl_sock_info.offset;
                goto SSl_SEND;
            }

            return AIC_TCPIP_AT_OK;
        }

SSl_SEND:
        ret = mbedtls_ssl_write((mbedtls_ssl_context *)ssl_list_q[index].ssl,
                                ssl_sock_info.buf, len);
        if (ret < 0) {
            sprintf(str, "SEND FAIL,errno %d", (errno));
            aicx_info(0x1d0117, "d", "[AT_TCPIP]: send data fail,errno %d\n", (errno));
        } else {
            strcpy(str, "SEND OK");
            aicx_info(0x1d42d6, "d", "[AT_TCPIP]: send ret %d\n", ret);
        }
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        at_req_resp_ptr->at_mode = AT_CMD_MODE;
        if (ssl_sock_info.buf)
            aos_free(ssl_sock_info.buf);
        aos_memset(&ssl_sock_info, 0, sizeof(struct at_sock_info));
        return AIC_TCPIP_AT_OK;
    } else {
        strcpy(str, AT_VERBOSE_RESULT_ERROR);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        return AIC_TCPIP_AT_ERR;
    }
}

u32 handle_qsslrecv_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), "+QSSLRECV: (0-11),(1-1500)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_transmit_resp_ok(at_req_resp_ptr);;
    return AIC_TCPIP_AT_OK;
}

u32 handle_qsslrecv_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int i, sock, length, link_id = 0, offset = 0;
    char *data;
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    u16 cnt, len;
    ulong *paras_arr;
    int ret = -1;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QSSLRECV=",
                                             &cnt);
    if (!paras_arr || cnt < 1 || cnt > 2) {
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
        goto ERROR_EXIT;
    if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
        goto ERROR_EXIT;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_QSSL);
    if (!sock) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    if (cnt == 2) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            length = atoi((u8 *)paras_arr[1]);
        else
            goto ERROR_EXIT;

        if (length < 1 || length > 1500) {
            goto ERROR_EXIT;
        }
    } else
        length = 1500;

    i = aic_at_find_index_by_socket(sock, AT_QSSL);
    if (i < 0) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }
    if (!ssl_list_q[i].ssl) {
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }
    sock = aic_at_find_socket_by_linkid(link_id, AT_QSSL);

    data = aos_zalloc_relaxed(length + 50);
    if (!data) {
        aicx_error(0x18a3e2, "", "[AT_TCPIP]: alloc recv buffer fail\n");
        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_RECV_FAIL;
    }

recv_again:
    ret = mbedtls_ssl_read((mbedtls_ssl_context *)ssl_list_q[i].ssl,
                           data + 50 + offset, length - offset);
    if (ret < 0) {
        if (offset > 0) {
            goto EXIT;
        }
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            snprintf(str, sizeof(str), "+QSSLRECV: 0");
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            goto RD_EXIT;
        }

        aic_transmit_resp_error(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
        aos_free(data);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_ERR;
    }

    if (ret == 0) {
        if (offset > 0) {
            goto EXIT;
        }
        snprintf(str, sizeof(str), "+QSSLRECV: 0");
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        goto RD_EXIT;
    }

    if (ret >= 0) {
        offset += ret;
        if (offset < length)
            goto recv_again;
    }

EXIT:
    snprintf(str, sizeof(str), "+QSSLRECV: %d\r\n", offset);
    len = strlen(str);
    aos_memcpy((data + 50 - len), str, len);
    aic_transmit_at_data(at_req_resp_ptr->serial_port_id, (data + 50 - len), offset + len);

RD_EXIT:
    aic_transmit_resp_ok(at_req_resp_ptr);
    aos_free(data);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
ERROR_EXIT:
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 handle_qsslstate_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aic_transmit_resp_ok(at_req_resp_ptr);;
    return AIC_TCPIP_AT_OK;
}

void aic_get_ssl_state(int link_id, struct ssl_state *state)
{
    int sock, addrlen;
    struct sockaddr_in remoteaddr, localaddr;
    struct sockaddr_in6 remoteaddr6, localaddr6;

    sock = ssl_list_q[link_id].socket_id;
    if (ssl_list_q[link_id].family == AF_INET) {
        memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
        memset(&localaddr, 0, sizeof(struct sockaddr_in));
        addrlen = sizeof(struct sockaddr_in);
        anet_getpeername(sock, (struct sockaddr *)&remoteaddr, &addrlen);
        anet_getsockname(sock, (struct sockaddr *)&localaddr, &addrlen);
        inet_ntop(AF_INET, &remoteaddr.sin_addr.s_addr,
                  state->addr, ADDR_BUFFERSIZE_IPV4);
        state->remote_port = htons(remoteaddr.sin_port);
        state->local_port = htons(localaddr.sin_port);
    } else if (ssl_list_q[link_id].family == AF_INET6){
        memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
        memset(&localaddr6, 0, sizeof(struct sockaddr_in6));
        addrlen = sizeof(struct sockaddr_in6);
        anet_getpeername(sock, (struct sockaddr *)&remoteaddr6, &addrlen);
        anet_getsockname(sock, (struct sockaddr *)&localaddr6, &addrlen);
        inet_ntop(AF_INET6, &remoteaddr6.sin6_addr, state->addr,
                  ADDR_BUFFERSIZE_IPV6);
        state->remote_port = htons(remoteaddr6.sin6_port);
        state->local_port = htons(localaddr6.sin6_port);
    } else {
        memset(state->addr, 0, ADDR_BUFFERSIZE_IPV6);
        state->remote_port = 0;
        state->local_port = 0;
    }

    state->pdp_cid = ssl_list_q[link_id].pdp_cid;
    state->ctx_id = ssl_list_q[link_id].ssl_ctx_id;
    state->mode = ssl_list_q[link_id].mode;
    state->state = 2;
    state->server_id = link_id;
}

u32 handle_qsslstate_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr = NULL;
    int i = 0, sock, link_id = 0;
    struct ssl_state state = {0};
    char str[BUFFER_SIZE] = {'\0'};

    if (strncasecmp(at_req_resp_ptr->at_req, "AT+QSSLSTATE=",
                    strlen("AT+QSSLSTATE=")) == 0) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+QSSLSTATE=",
                                                 &cnt);
        if (!paras_arr || cnt != 1) {
            aicx_error(0x000086, "", "[AT_TCPIP]: parameter parse error\n");
            if (paras_arr)
                aic_free_paras_mem(paras_arr, cnt);

            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }

        if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
            link_id = atoi((u8 *)paras_arr[0]);
        else
            goto ERROR_EXIT;

        if (link_id >= AT_SOCKET_MAX_CONN_NUM_Q || link_id < 0) {
            goto ERROR_EXIT;
        }

        if (ssl_list_q[link_id].socket_id && !ssl_list_q[link_id].ssl) {
            aic_free_paras_mem(paras_arr, cnt);
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }
        aic_get_ssl_state(link_id, &state);
        aic_get_at_port_name(at_req_resp_ptr->serial_port_id, state.at_port,
                             AT_URC_PORT_NAME_LEN);
        snprintf(str, sizeof(str),
                 "+QSSLSTATE: %d,\"SSLClient\",\"%s\",%d,%d,%d,%d,%d,%d,\"%s\",%d",
                 link_id, state.addr, state.remote_port, state.local_port,
                 state.state, state.pdp_cid, state.server_id, state.mode,
                 state.at_port, state.ctx_id);

        aic_transmit_resp_normal(at_req_resp_ptr, str);
        goto exit;
    } else {
        while (i < AT_SOCKET_MAX_CONN_NUM_Q) {
            sock = ssl_list_q[i].socket_id;
            if (sock == 0)
                goto loop;

            if (!ssl_list_q[i].ssl) {
                i++;
                continue;
            }

loop:
            aos_memset(&state, 0, sizeof(struct ssl_state));
            aic_get_ssl_state(i, &state);
            aic_get_at_port_name(at_req_resp_ptr->serial_port_id, state.at_port,
                                 AT_URC_PORT_NAME_LEN);
            aos_memset(str, '\0', AT_RESP_RESULT_MAX);
            snprintf(str, sizeof(str),
                     "+QSSLSTATE: %d,\"SSLClient\",\"%s\",%d,%d,%d,%d,%d,%d,\"%s\",%d",
                     i, state.addr, state.remote_port, state.local_port,
                     state.state, state.pdp_cid, state.server_id, state.mode,
                     state.at_port, state.ctx_id);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            i++;
        }
        goto exit;
    }

exit:
    aic_transmit_resp_ok(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_transmit_resp_error(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

u32 input_crt_len;
u32 handle_ssl_ca_cert_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras = AIC_NULL;
    u16 count;
    u32 temp_size;
    int i = 0;
    char str[BUFFER_SIZE] = {'\0'};

    /* AT mode */
    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+QSSLCACERT=",
                                             &count);
        if (!paras || count != 2) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            goto SSLCACERT_ERROR;
        }

        i = atoi((u8 *)paras[0]);
        input_crt_len = atoi((u8 *)paras[1]);
        if (input_crt_len <= 0) {
            aic_transmit_resp_error(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM);
            goto SSLCACERT_ERROR;
        }

        if (qsslconf_list[i].cacert) {
            aos_free(qsslconf_list[i].cacert);
            qsslconf_list[i].cacert = AIC_NULL;
            qsslconf_list[i].cacert_len = 0;
        }
        qsslconf_list[i].cacert = aos_zalloc(input_crt_len);

        at_req_resp_ptr->at_mode = AT_DATA_MODE;
        sprintf(str, "CRT SET START");
        aic_transmit_resp_normal(at_req_resp_ptr, str);

SSLCACERT_ERROR:
        aic_free_paras_mem(paras, count);
        return AIC_SUCCESS;
    }

    /* data mode */
    if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        if (strncmp(at_req_resp_ptr->at_upstream_data, "+++", 3) == 0) {
            at_req_resp_ptr->at_mode = AT_CMD_MODE;
            sprintf(str, "CRT SET END");
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            return AIC_SUCCESS;
        }

        /* set ca crt */
        if (qsslconf_list[i].cacert) {
            temp_size = at_req_resp_ptr->upstream_data_len;
            //alog_debug("temp_size is %d\n", temp_size);
            if (qsslconf_list[i].cacert_len + temp_size > input_crt_len) {
                temp_size = input_crt_len - qsslconf_list[i].cacert_len;
            }
            if (temp_size != 0) {
                aos_memcpy(qsslconf_list[i].cacert + qsslconf_list[i].cacert_len,
                           at_req_resp_ptr->at_upstream_data, temp_size);
                qsslconf_list[i].cacert_len += temp_size;
            }
        }
    }

    return AIC_SUCCESS;
}

static void aic_at_ssl_socket_recv(int index)
{
    int ret, len;
    u8 *buffer = NULL, *str = NULL;
    u8 buf[4] = {0};
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};
    mbedtls_ssl_context *ssl;

    if (!ssl_list_q[index].ssl)
        return;

    ssl = (mbedtls_ssl_context *)ssl_list_q[index].ssl;
recv_again:
    ret = mbedtls_ssl_read(ssl, buf, 0);
    if (ret < 0 || (ssl->MBEDTLS_PRIVATE(in_msglen) == 0))
        return;

    if (ssl_list_q[index].mode == 1 || ssl_list_q[index].mode == 2) {
        buffer = aos_zalloc_relaxed(ssl->MBEDTLS_PRIVATE(in_msglen) + 50);
        if (!buffer) {
            return;
        }

        ret = mbedtls_ssl_read((mbedtls_ssl_context *)ssl_list_q[index].ssl,
                               (buffer + 50), ssl->MBEDTLS_PRIVATE(in_msglen));
        if (ret > 0) {
            if (ssl_list_q[index].mode == 1) {
                if (at_tcpip_cfg.recv_data_format == 0) {
                    snprintf(result_str, AT_RESP_RESULT_MAX,
                             "+QSSLURC: \"recv\",%d,%d\r\n", ssl_list_q[index].link_id, ret);
                    len = strlen(result_str);
                    aos_memcpy((buffer + 50 - len), result_str, len);
                    aic_transmit_at_result(at_tcpip_cfg.com_id,
                                         (buffer + 50 - len), len + ret);
                } else {
                    str = aos_zalloc_relaxed(ret * 2 + 50);
                    if (!str) {
                        return;
                    }
                    snprintf(str, 50, "+QSSLURC: \"recv\",%d,%d\r\n",
                             ssl_list_q[index].link_id, ret);
                    len = strlen(str);
                    aic_print_data_to_hex((buffer + 50), ret, (str + len));
                    len = len + ret * 2;
                    aic_transmit_at_result(at_tcpip_cfg.com_id, str, len);
                    aos_free(str);
                }
            } else {
                aic_transmit_at_result(at_tcpip_cfg.com_id,
                                       (buffer + 50), ret);
            }
        }

        if (buffer)
            aos_free(buffer);

        goto recv_again;
    }  else {
        if (ssl->MBEDTLS_PRIVATE(in_msglen) > 16) {
            aicx_info(0x028afe, "d", "[AT_TCPIP] recv notify len %d\n", ssl->MBEDTLS_PRIVATE(in_msglen));
            return;
        }
        snprintf(result_str, AT_RESP_RESULT_MAX,
                  "+QSSLURC: \"recv\",%d", ssl_list_q[index].link_id);
        aic_transmit_at_cmd_urc(at_tcpip_cfg.com_id,
                                result_str, strlen(result_str));
    }
}

static void aic_at_ssl_process_recv(void *pdata)
{
    u32 events = 0;
    int i, unrcv, len;

    while (1) {
        aos_get_event(ssl_recv_event, SSL_RECV_EVENT_ALL, TX_OR_CLEAR,
                      &events, AOS_WAIT_FOREVER);
        aos_get_mutex(at_ssl_mutex, AOS_WAIT_FOREVER);
        if (events & SSL_RECV_EVENT) {
            aicx_info(0x12a788, "", "[AT_TCPIP] recv event SSL_RECV_EVENT\n");

            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
                if (ssl_list_q[i].socket_id == 0)
                    continue;

                getsockopt(ssl_list_q[i].socket_id, SOL_SOCKET, SO_RXDATA, &unrcv, &len);
                if (!at_event_flag[i].recv_flag && !unrcv)
                    continue;

                aicx_info(0x0add21, "", "[AT_TCPIP] recv start\n");
                aic_at_ssl_socket_recv(i);
                at_event_flag[i].recv_flag = 0;
            }
        }

        if (events & SSL_CLOSE_EVENT) {
            aicx_info(0x0fb6a5, "", "[AT_TCPIP] recv event SSL_CLOSE_EVENT\n");
            for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
                if (!at_event_flag[i].close_flag)
                    continue;

                aic_qat_ssl_close(i);
                at_event_flag[i].close_flag = 0;
            }
        }
        aos_put_mutex(at_ssl_mutex);
    }
}

int aic_at_ssl_recv_task_create(void)
{
    ssl_recv_thread = aos_create_thread("AT SSL recv Thread",
                                       aic_at_ssl_process_recv,
                                       NULL, SIZE_2K, 0, 0, 15, AOS_DONT_START);
    if (ssl_recv_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x028a0a, "", "[AT_TCPIP]: at ssl recv thread create fail\n");
        return AIC_EFAULT;
    }

    ssl_recv_event = aos_create_event("AT SSL recv event flags");
    if (!ssl_recv_event) {
        aicx_error(0x09cf44, "", "[AT_TCPIP]: at ssl recv event create fail\n");
        aos_delete_thread(ssl_recv_thread);
        return AIC_EFAULT;
    }

    aos_resume_thread(ssl_recv_thread);
    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_at_ssl_recv_task_create);


