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
#define AIC_THIS_FILE    0x11c27c
#define LOG_MODULE    AMOD_NET

#include <aic_init.h>
#include "aic_at_mtls.h"
#include "aic_at_mtcpip.h"
#ifdef CONFIG_PDP_MGR
#include "aic_net_pdp_mgr.h"
#endif

#ifdef CONFIG_FS
#include "fs_api.h"
#ifdef CONFIG_FS_FATFS
// #define AT_SSL_CERT_DIR   "/C/home/cert"
// #define AT_SSL_KEY_DIR    "/C/home/key"
#define AT_SSL_CERT_DIR   "/C/home"
#define AT_SSL_KEY_DIR    "/C/home"
#else
// #define AT_SSL_CERT_DIR   "/C/cert"
// #define AT_SSL_KEY_DIR    "/C/key"
#define AT_SSL_CERT_DIR   "/C"
#define AT_SSL_KEY_DIR    "/C"
#endif
#else
#include "aic_md_api.h"
#include "aic_md_id.h"
#endif

#define CRC_POLY 0xEDB88320
#define CRC32_CHECK
//#define CRC16_CHECK

struct mbedtls_config msslconf_list[SSL_CTX_ID_MAX];
static struct cert_info ssl_cert_info = {0};

#if (defined CONFIG_CM_SSL_AT && defined CONFIG_CM_TCPIP_AT) || (defined CONFIG_GX_SSL_AT && defined CONFIG_GX_TCPIP_AT)
extern AIC_SOCKET_LIST sock_list[AT_SOCKET_MAX_CONN_NUM];
extern AT_TCPIP_CFG at_tcpip_cfg[AT_SOCKET_MAX_CONN_NUM];
extern int error_code;
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];
extern aos_event_ptr at_socket_event;


extern void aic_mat_socket_tcp_ack_notify(int sock, int len, void *data);
extern void aic_mat_socket_tcp_disconn_notify(int sock, void *data, int is_active);
extern void aic_mat_socket_tcp_connect_notify(int sock, u32 status, void *data);
#endif

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
/* AT handler functions */
static u32 handle_msslcfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslcertwr_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslcertwr_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslkeywr_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslkeywr_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslcertrd_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_mssllist_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslrm_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
//static u32 handle_msslpsk_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslcheck_test_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslcheck_set_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_msslcipher_test_cmd(AT_REQ_RESP *at_req_resp_ptr);

static int aic_mat_mbedtls_config_init(void);
int aic_ssl_get_cert_key_by_name(u8 *file, u8 type, u16 *item_id, int *item_size);
void aic_mat_ssl_recv_notify(int sock, void *data);

AT_CMD_HANDLE mssl_at_cmd[] = {
    {AT_SSL_CFG, handle_msslcfg_test_cmd, AIC_NULL, handle_msslcfg_set_cmd,
     AIC_NULL},

    {AT_SSL_CERTWR, handle_msslcertwr_test_cmd, AIC_NULL,
     handle_msslcertwr_set_cmd, AIC_NULL},

    {AT_SSL_KEYWR, handle_msslkeywr_test_cmd, AIC_NULL,
     handle_msslkeywr_set_cmd, AIC_NULL},

    {AT_SSL_CERTRD, AIC_NULL, AIC_NULL, handle_msslcertrd_set_cmd, AIC_NULL},

    {AT_SSL_LIST, AIC_NULL, AIC_NULL, handle_mssllist_set_cmd, AIC_NULL},

    {AT_SSL_RM, AIC_NULL, AIC_NULL, handle_msslrm_set_cmd, AIC_NULL},

    //{"AT+MSSLPSK", AIC_NULL, AIC_NULL, handle_msslpsk_set_cmd, AIC_NULL},

    {AT_SSL_CHECK, handle_msslcheck_test_cmd, AIC_NULL,
     handle_msslcheck_set_cmd, AIC_NULL},

    {AT_SSL_CIPHER, handle_msslcipher_test_cmd, AIC_NULL, AIC_NULL, AIC_NULL},

    {AIC_NULL},
};

int aic_ssl_mat_cmd_init(void)
{
    aic_add_at_cmd_handle(mssl_at_cmd);
    aic_mat_mbedtls_config_init();

    return AIC_TCPIP_AT_OK;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_ssl_mat_cmd_init);

static int aic_mat_mbedtls_config_init(void)
{
    int i;
    struct mbedtls_config cfg;

    aos_memset(&cfg, 0, sizeof(struct mbedtls_config));
    cfg.ssl_version = SSL_VERSION_ALL;
    cfg.ciphersuite = SSL_CIPHERSUITE_DEFAULT;
    cfg.seclevel = SSL_SECLEVEL_DEFAULT;
    cfg.ignore_ltime = SSL_IGNORE_TIME;
    cfg.negotiate_time = SSL_NEGOTIATE_TIME;
    cfg.session = SSL_SESSION_DEFAULT;
    cfg.ignore_verify = 0;
    cfg.encoding = 2;
    strcpy(cfg.cacert_name, "NULL");
    strcpy(cfg.client_cert_name, "NULL");
    strcpy(cfg.client_key_name, "NULL");

    for (i = 0; i < SSL_CTX_ID_MAX; i++) {
        aos_memcpy(&msslconf_list[i], &cfg, sizeof(struct mbedtls_config));
    }

    return AIC_SUCCESS;
}


#if (defined CONFIG_CM_SSL_AT && defined CONFIG_CM_TCPIP_AT) || (defined CONFIG_GX_SSL_AT && defined CONFIG_GX_TCPIP_AT)
void aic_mat_ssl_recv_notify(int sock, void *data)
{
    int i;
    mbedtls_ssl_context *ssl;

    i = aic_at_find_index_by_socket(sock, AT_MTCPIP);
    if (i == AIC_EPERM || !sock_list[i].ssl) {
        return;
    }

    ssl = (mbedtls_ssl_context *)sock_list[i].ssl;
    if (sock_list[i].mode != 0 && sock_list[i].mode != 1) {
        if (ssl->in_msglen > 16) {
            return;
        }
    }

    at_tcpip_cfg[i].recv_flag = 1;
    aos_set_event(at_socket_event, SOCKET_RECV_EVENT, TX_OR);
}

int aic_ssl_net_connect(int family, struct sockaddr *addr,
                        struct sockaddr *srcaddr, int linkid)
{
    int i, sock, ret, addrlen;
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
            error_code = TCPIP_SOCKET_BIND_FAIL;
            return ret;
        }
    }

    tcp_windows.winsize = AT_SOCKET_RECV_WINDOWS;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &tcp_windows, sizeof(tcp_windows));

    ret = anet_fcntl(sock, F_SETFL, O_NONBLOCK);
    if (ret == NX_SOC_ERROR) {
        soc_close(sock);
        return ret;
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

    i = linkid;
    if (sock_list[i].socket_id == 0) {
        sock_list[i].link_id = linkid;
        sock_list[i].socket_id = sock;
        sock_list[i].family = family;
        sock_list[i].socket_type = SOCK_TCP;
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
    }

    return sock;
}

#if 0
static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    aicx_info(0x0d776a, "sds", "%s:%04d: %s", file, line, str);
}
#endif

int aic_mat_create_mbedtls_socket(/*int family, struct sockaddr *addr,
                                  struct sockaddr *srcaddr, */int linkid)
{
    int i, index, ret, count = 0;
    mbedtls_net_context *server_fd;
    const char *pers = "at_ssl_client";
    mbedtls_entropy_context *entropy;
    mbedtls_ctr_drbg_context *ctr_drbg;
    mbedtls_ssl_context *ssl;
    mbedtls_ssl_config *conf;
    mbedtls_x509_crt *cacert;
    mbedtls_pk_context *pkey = NULL;

    i = linkid;
    server_fd = &sock_list[i].sockfd;
    index = at_tcpip_cfg[i].ssl_ctx_id;
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

    mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                          (const unsigned char *) pers, strlen(pers));

    if (msslconf_list[index].seclevel >= 1 && msslconf_list[index].cacert) {
        ret = mbedtls_x509_crt_parse(cacert, msslconf_list[index].cacert,
                                     (size_t)msslconf_list[index].cacert_len);
        if( ret < 0 ) {
            aicx_error(0x13c5c2, "x", "crt0 parse failed -0x%x\n", (unsigned int) -ret);
            goto exit5;
        }
    }

    if (msslconf_list[index].seclevel > 1 && msslconf_list[index].client_cert &&
        msslconf_list[index].client_key) {
        ret = mbedtls_x509_crt_parse(cacert, msslconf_list[index].client_cert,
                                     (size_t)msslconf_list[index].client_cert_len);
        if( ret < 0 ) {
            aicx_error(0x032ae5, "x", "crt parse failed -0x%x\n", (unsigned int) -ret);
            goto exit5;
        }

        pkey = aos_zalloc(sizeof(mbedtls_pk_context));
        if (!pkey)
            goto exit5;
        mbedtls_pk_init(pkey);
#ifdef CONFIG_CRYPTO_MBEDTLS_3
        ret = mbedtls_pk_parse_key(pkey, (const unsigned char *)msslconf_list[index].client_key,
                                   msslconf_list[index].client_key_len, NULL, 0,
                                   mbedtls_ctr_drbg_random, &ctr_drbg);
#else
#ifdef CONFIG_CRYPTO_MBEDTLS_2
        ret = mbedtls_pk_parse_key(pkey, (const unsigned char *)msslconf_list[index].client_key,
                                   msslconf_list[index].client_key_len, NULL, 0);
#endif
#endif

        if (ret != 0) {
            aicx_error(0x09fcdc, "x", "pk parse failed -0x%x\n", (unsigned int) -ret);
            goto exit5;
        }
    }

    server_fd->fd = sock_list[i].socket_id;
    ret = mbedtls_ssl_config_defaults(conf,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        aicx_error(0x172b82, "d", "config failed %d\n", ret);
        goto exit5;
    }

    mbedtls_ssl_conf_authmode(conf, msslconf_list[index].seclevel);
    mbedtls_ssl_conf_ca_chain(conf, cacert, NULL);
    if (msslconf_list[index].seclevel > 1) {
        mbedtls_ssl_conf_own_cert(conf, cacert->next, pkey);
    }
    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);
    //mbedtls_ssl_conf_dbg(conf, my_debug, NULL);

    ret = mbedtls_ssl_setup(ssl, conf);
    if (ret != 0) {
        aicx_error(0x0404f6, "d", "setup failed %d\n", ret);
        goto exit5;
    }

    mbedtls_ssl_set_bio(ssl, server_fd, mbedtls_net_send,
                        mbedtls_net_recv, NULL);

    while (count < SSL_HANDSHARK_RETRANS_COUNT) {
        ret = mbedtls_ssl_handshake(ssl);
        if(ret == 0)
            break;

        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            aicx_error(0x0b37cf, "x", "handshake failed 0x%x\n", (unsigned int) -ret );
            goto exit5;
        }

        aos_sleep(500);
        count++;
        if (count == SSL_HANDSHARK_RETRANS_COUNT)
            goto exit5;
    }

    aic_socket_register_receive_notify(sock_list[i].socket_id, aic_mat_ssl_recv_notify, NULL);
    aic_socket_register_tcp_disconnect_notify(sock_list[i].socket_id,
                                              aic_mat_socket_tcp_disconn_notify,
                                              NULL);
    aic_socket_register_tcp_ack_notify(sock_list[i].socket_id, aic_mat_socket_tcp_ack_notify, NULL);

    sock_list[i].ssl = ssl;
    sock_list[i].key = pkey;

    return sock_list[i].socket_id;

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

    error_code = SSL_FAIL;
    return -1;
}

int aic_mat_mbedtls_socket_recv(int index, int len, AT_REQ_RESP *at_req_resp_ptr)
{
    int link_id = 0, ret = 0, com_id;
    u8 *buf = NULL, *hex_buf = NULL;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    mbedtls_ssl_context *ssl;

    com_id = at_req_resp_ptr->serial_port_id;
    ssl = (mbedtls_ssl_context *)sock_list[index].ssl;
    if (ssl->in_msglen == 0) {
        aic_reply_ok_resp(at_req_resp_ptr);
        goto RD_EXIT;
    }

    buf = aos_zalloc_relaxed(ssl->in_msglen + AT_SOCKET_RECV_BUF_OFFSET);
    if (!buf) {
        aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
        goto RD_EXIT;
    }

    if (at_tcpip_cfg[index].recv_data_format == 1) {
        hex_buf = aos_zalloc_relaxed(ssl->in_msglen * 2 + AT_SOCKET_RECV_BUF_OFFSET);
        if (!hex_buf) {
            aic_reply_error_resp(at_req_resp_ptr, TCPIP_SOCKET_RECV_FAIL);
            goto RD_EXIT;
        }
    }

    ret = mbedtls_ssl_read(ssl, (buf + AT_SOCKET_RECV_BUF_OFFSET),
                           ssl->in_msglen);
    snprintf(str, AT_RESP_RESULT_MAX, RESP_IP_READ"%d,%d,%d,",
             index, 0, ret);
    len = strlen(str);
    if (at_tcpip_cfg[index].recv_data_format == 0) {
        aos_memcpy((buf + AT_SOCKET_RECV_BUF_OFFSET - len), str, len);
        aic_transmit_at_result(com_id, (buf + AT_SOCKET_RECV_BUF_OFFSET - len),
                               ret + len);
    } else {
        aos_memcpy(hex_buf, str, len);
        aic_print_data_to_hex(buf + AT_SOCKET_RECV_BUF_OFFSET, ret,
                              hex_buf + len);
        aic_transmit_at_result(com_id, hex_buf, ret * 2 + len);
    }
    aic_reply_ok_resp(at_req_resp_ptr);

    ret = mbedtls_ssl_read((mbedtls_ssl_context *)sock_list[link_id].ssl,
                           (buf + AT_SOCKET_RECV_BUF_OFFSET), 0);
    if (ret < 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            goto RD_EXIT;
        }
    } else {
        if (ssl->in_msglen) {
            snprintf(str, AT_RESP_RESULT_MAX, IP_URC"\"rtcp\",%d,%d,%d",
                     link_id, ssl->in_msglen, ssl->in_msglen);
            aic_transmit_at_cmd_urc(com_id, str, strlen(str));
        }
        goto RD_EXIT;
    }

RD_EXIT:
    if (buf)
        aos_free(buf);
    if (hex_buf)
        aos_free(hex_buf);
    return AIC_TCPIP_AT_OK;
}

int aic_mat_close_mbedtls_socket(int socket_fd)
{
    int index;
    mbedtls_ssl_context *ssl;
    mbedtls_ssl_config *conf;
    mbedtls_ctr_drbg_context *ctr_drbg;

    index = aic_at_find_index_by_socket(socket_fd, AT_MTCPIP);
    if (index == AIC_EPERM) {
        return AIC_SUCCESS;
    }

    ssl = sock_list[index].ssl;
    if (!ssl)
        goto exit;

    mbedtls_ssl_close_notify(ssl);
    conf = (mbedtls_ssl_config *)ssl->conf;
    if (!conf)
        goto exit;

    ctr_drbg = (mbedtls_ctr_drbg_context *)conf->p_rng;
    if (ctr_drbg) {
        mbedtls_entropy_free(ctr_drbg->p_entropy);
        if (ctr_drbg->p_entropy) {
            aos_free(ctr_drbg->p_entropy);
            ctr_drbg->p_entropy = NULL;
        }

        mbedtls_ctr_drbg_free(ctr_drbg);
    }
    if (conf->authmode== MBEDTLS_SSL_VERIFY_REQUIRED) {
        mbedtls_pk_free(sock_list[index].key);
        if (sock_list[index].key) {
            aos_free(sock_list[index].key);
            sock_list[index].key = NULL;
        }
    }

    mbedtls_x509_crt_free(conf->ca_chain);
    if (conf->ca_chain) {
        aos_free(conf->ca_chain);
        conf->ca_chain = NULL;
    }
    mbedtls_ssl_config_free(conf);
    mbedtls_ssl_free(ssl);

    if (ctr_drbg) {
        aos_free(ctr_drbg);
        conf->p_rng = NULL;
    }

    if (conf) {
        aos_free(conf);
        ssl->conf = NULL;
    }

    if (ssl) {
        aos_free(ssl);
        sock_list[index].ssl = NULL;
    }

exit:
    mbedtls_net_free(&sock_list[index].sockfd);
    return AIC_SUCCESS;
}

void aic_at_ssl_socket_recv(int index)
{
    int ret, len;
    u8 *buffer = NULL, *str = NULL;
    u8 buf[4] = {0};
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};
    mbedtls_ssl_context *ssl;

    if (!sock_list[index].ssl) {
        return;
    }

    ssl = sock_list[index].ssl;
    aos_sleep(50);
recv_again:
    ret = mbedtls_ssl_read(ssl, buf, 0);
    if (ret < 0)
        return;

    if (ssl->in_msglen == 0)
        return;

    if (sock_list[index].mode == 0 || sock_list[index].mode == 1) {
        buffer = aos_zalloc_relaxed(ssl->in_msglen + 50);
        if (!buffer) {
            return;
        }

        ret = mbedtls_ssl_read((mbedtls_ssl_context *)sock_list[index].ssl,
                               (buffer + 50), ssl->in_msglen);
        if (ret > 0) {
            if (sock_list[index].mode == 0) {
                if (at_tcpip_cfg[index].recv_data_format == 0) {
                    snprintf(result_str, AT_RESP_RESULT_MAX,
                             IP_URC"\"rtcp\",%d,%d,", sock_list[index].link_id, ret);
                    len = strlen(result_str);
                    aos_memcpy((buffer + 50 - len), result_str, len);
                    aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                            (buffer + 50 - len), len + ret);
                } else {
                    str = aos_zalloc_relaxed(ret * 2 + 50);
                    if (!str) {
                        return;
                    }
                    snprintf(str, 50, IP_URC"\"rtcp\",%d,%d,", sock_list[index].link_id, ret);
                    len = strlen(str);
                    aic_print_data_to_hex((buffer + 50), ret, (str + len));
                    len = len + ret * 2;
                    aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id, str, len);
                    aos_free(str);
                }
            } else {
                aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                        (buffer + 50), ret);
            }
        }

        // if (str)
        //     aos_free(str);
        if (buffer)
            aos_free(buffer);

        goto recv_again;
    } else if (sock_list[index].mode == 3 && sock_list[index].socket_type == SOCK_TCP) {
        snprintf(result_str, AT_RESP_RESULT_MAX, IP_URC"\"rtcp\",%d,1", index);
        aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                result_str, strlen(result_str));
    } else {
        snprintf(result_str, AT_RESP_RESULT_MAX,
                  IP_URC"\"rtcp\",%d,%d,%d", sock_list[index].link_id,
                  ssl->in_msglen, ssl->in_msglen);
        aic_transmit_at_cmd_urc(at_tcpip_cfg[index].com_id,
                                result_str, strlen(result_str));
    }
}

#endif

#ifdef CONFIG_FS
int handle_cert_file(u8 *file, int ssl_ctx_id, int type)
{
    u8 *cert = NULL;
    int ret, fd, rsize, find, file_size, offset;
    FS_FILE_INFO *file_info = NULL;
    u8 file_name[128];

    offset = sizeof(struct ssl_cert_key_info);
    find = aic_ssl_get_cert_key_by_name(file, type, NULL, NULL);
    if (!find) {
        switch (type) {
        case SSL_CA_CERT:
            if (msslconf_list[ssl_ctx_id].cacert) {
                aos_free(msslconf_list[ssl_ctx_id].cacert - offset);
                msslconf_list[ssl_ctx_id].cacert = NULL;
                msslconf_list[ssl_ctx_id].cacert_len = 0;
            }
            aos_memset(msslconf_list[ssl_ctx_id].cacert_name, 0, SSL_CERTKEY_NAME_LEN);
            aos_memcpy(msslconf_list[ssl_ctx_id].cacert_name, file, strlen(file));
            break;
        case SSL_CLIENT_CERT:
            if (msslconf_list[ssl_ctx_id].client_cert) {
                aos_free(msslconf_list[ssl_ctx_id].client_cert - offset);
                msslconf_list[ssl_ctx_id].client_cert = NULL;
                msslconf_list[ssl_ctx_id].client_cert_len = 0;
            }
            aos_memset(msslconf_list[ssl_ctx_id].client_cert_name, 0, SSL_CERTKEY_NAME_LEN);
            aos_memcpy(msslconf_list[ssl_ctx_id].client_cert_name, file, strlen(file));
            break;
        case SSL_CLIENT_KEY:
            if (msslconf_list[ssl_ctx_id].client_key) {
                aos_free(msslconf_list[ssl_ctx_id].client_key - offset);
                msslconf_list[ssl_ctx_id].client_key = NULL;
                msslconf_list[ssl_ctx_id].client_key_len = 0;
            }
            aos_memset(msslconf_list[ssl_ctx_id].client_key_name, 0, SSL_CERTKEY_NAME_LEN);
            aos_memcpy(msslconf_list[ssl_ctx_id].client_key_name, file, strlen(file));
            break;
        default:
            return AIC_ENOTSUP;
        }
        return AIC_SUCCESS;
    }

    switch (type) {
    case SSL_CA_CERT:
    case SSL_CLIENT_CERT:
        snprintf(file_name, sizeof(file_name), "%s/%s", AT_SSL_CERT_DIR, file);
        break;
    case SSL_CLIENT_KEY:
        snprintf(file_name, sizeof(file_name), "%s/%s", AT_SSL_KEY_DIR, file);
        break;
    default:
        return AIC_ENOTSUP;
    }

    fd = fs_open(file_name, FS_O_RDONLY);
    if (fd < 0) {
        return AIC_EFAULT;
    }

    file_info = aos_zalloc_relaxed(sizeof(FS_FILE_INFO));
    if (!file_info) {
        fs_close(fd);
        goto ERR_EXIT;
    }
    ret = fs_getinfo(file_name, file_info);
    if (ret != AIC_SUCCESS) {
        fs_close(fd);
        goto ERR_EXIT;
    }

    ret = fs_seek(fd, 0, FS_SEEK_BEGIN);
    if (ret != 0) {
        fs_close(fd);
        goto ERR_EXIT;
    }

    cert = aos_zalloc_relaxed(file_info->size + 1);
    if (!cert) {
        fs_close(fd);
        goto ERR_EXIT;
    }
    rsize = fs_read(fd, cert, file_info->size);
    if (rsize <= 0) {
        fs_close(fd);
        goto ERR_EXIT;
    }

    ret = fs_close(fd);
    if (ret != AIC_SUCCESS) {
        goto ERR_EXIT;
    }

    file_size = rsize - sizeof(struct ssl_cert_key_info);
    if (type == SSL_CA_CERT) {
        if (msslconf_list[ssl_ctx_id].cacert) {
            aos_free(msslconf_list[ssl_ctx_id].cacert - offset);
            msslconf_list[ssl_ctx_id].cacert = NULL;
        }
        aos_memset(msslconf_list[ssl_ctx_id].cacert_name, 0, SSL_CERTKEY_NAME_LEN);
        aos_memcpy(msslconf_list[ssl_ctx_id].cacert_name, file, strlen(file));
        msslconf_list[ssl_ctx_id].cacert = cert + offset;
        msslconf_list[ssl_ctx_id].cacert_len = file_size + 1;
    } else if (type == SSL_CLIENT_CERT) {
        if (msslconf_list[ssl_ctx_id].client_cert) {
            aos_free(msslconf_list[ssl_ctx_id].client_cert - offset);
            msslconf_list[ssl_ctx_id].client_cert = NULL;
        }
        aos_memset(msslconf_list[ssl_ctx_id].client_cert_name, 0, SSL_CERTKEY_NAME_LEN);
        aos_memcpy(msslconf_list[ssl_ctx_id].client_cert_name, file, strlen(file));
        msslconf_list[ssl_ctx_id].client_cert = cert + offset;
        msslconf_list[ssl_ctx_id].client_cert_len = file_size + 1;
    } else {
        if (msslconf_list[ssl_ctx_id].client_key) {
            aos_free(msslconf_list[ssl_ctx_id].client_key - offset);
            msslconf_list[ssl_ctx_id].client_key = NULL;
        }
        aos_memset(msslconf_list[ssl_ctx_id].client_key_name, 0, SSL_CERTKEY_NAME_LEN);
        aos_memcpy(msslconf_list[ssl_ctx_id].client_key_name, file, strlen(file));
        msslconf_list[ssl_ctx_id].client_key = cert + offset;
        msslconf_list[ssl_ctx_id].client_key_len = file_size + 1;
    }

    aos_free(file_info);
    return AIC_SUCCESS;

ERR_EXIT:
    if (cert)
        aos_free(cert);
    if (file_info)
        aos_free(file_info);
    return AIC_EFAULT;
}
#else
int handle_cert_file(u8 *file, int ssl_ctx_id, int type)
{
    u8 *cert = NULL;
    int ret, rsize, find, offset;
    u16 item_id = 0, item_size = 0;

    offset = sizeof(struct ssl_cert_key_info);
    find = aic_ssl_get_cert_key_by_name(file, type, &item_id, &rsize);
    if (!find) {
        switch (type) {
        case SSL_CA_CERT:
            if (msslconf_list[ssl_ctx_id].cacert) {
                aos_free(msslconf_list[ssl_ctx_id].cacert - offset);
                msslconf_list[ssl_ctx_id].cacert = NULL;
                msslconf_list[ssl_ctx_id].cacert_len = 0;
            }
            aos_memset(msslconf_list[ssl_ctx_id].cacert_name, 0, SSL_CERTKEY_NAME_LEN);
            aos_memcpy(msslconf_list[ssl_ctx_id].cacert_name, file, strlen(file));
            break;
        case SSL_CLIENT_CERT:
            if (msslconf_list[ssl_ctx_id].client_cert) {
                aos_free(msslconf_list[ssl_ctx_id].client_cert - offset);
                msslconf_list[ssl_ctx_id].client_cert = NULL;
                msslconf_list[ssl_ctx_id].client_cert_len = 0;
            }
            aos_memset(msslconf_list[ssl_ctx_id].client_cert_name, 0, SSL_CERTKEY_NAME_LEN);
            aos_memcpy(msslconf_list[ssl_ctx_id].client_cert_name, file, strlen(file));
            break;
        case SSL_CLIENT_KEY:
            if (msslconf_list[ssl_ctx_id].client_key) {
                aos_free(msslconf_list[ssl_ctx_id].client_key - offset);
                msslconf_list[ssl_ctx_id].client_key = NULL;
                msslconf_list[ssl_ctx_id].client_key_len = 0;
            }
            aos_memset(msslconf_list[ssl_ctx_id].client_key_name, 0, SSL_CERTKEY_NAME_LEN);
            aos_memcpy(msslconf_list[ssl_ctx_id].client_key_name, file, strlen(file));
            break;
        default:
            return AIC_ENOTSUP;
        }
        return AIC_SUCCESS;
    }

    if (item_id == 0 || rsize == 0) {
        goto ERR_EXIT;
    }

    item_size = aic_md_get_length(item_id);
    if (item_size == 0) {
        goto ERR_EXIT;
    }
    cert = aos_zalloc_relaxed(item_size + 1);
    if (!cert) {
        goto ERR_EXIT;
    }

    ret = aic_md_read(item_id, item_size, cert);
    if (ret != AIC_SUCCESS) {
        goto ERR_EXIT;
    }

    cert[offset + rsize] = '\0';
    if (type == SSL_CA_CERT) {
        if (msslconf_list[ssl_ctx_id].cacert) {
            aos_free(msslconf_list[ssl_ctx_id].cacert - offset);
            msslconf_list[ssl_ctx_id].cacert = NULL;
        }
        aos_memset(msslconf_list[ssl_ctx_id].cacert_name, 0, SSL_CERTKEY_NAME_LEN);
        aos_memcpy(msslconf_list[ssl_ctx_id].cacert_name, file, strlen(file));
        msslconf_list[ssl_ctx_id].cacert = cert + offset;
        msslconf_list[ssl_ctx_id].cacert_len = rsize + 1;
    } else if (type == SSL_CLIENT_CERT) {
        if (msslconf_list[ssl_ctx_id].client_cert) {
            aos_free(msslconf_list[ssl_ctx_id].client_cert - offset);
            msslconf_list[ssl_ctx_id].client_cert = NULL;
        }
        aos_memset(msslconf_list[ssl_ctx_id].client_cert_name, 0, SSL_CERTKEY_NAME_LEN);
        aos_memcpy(msslconf_list[ssl_ctx_id].client_cert_name, file, strlen(file));
        msslconf_list[ssl_ctx_id].client_cert = cert + offset;
        msslconf_list[ssl_ctx_id].client_cert_len = rsize + 1;
    } else {
        if (msslconf_list[ssl_ctx_id].client_key) {
            aos_free(msslconf_list[ssl_ctx_id].client_key - offset);
            msslconf_list[ssl_ctx_id].client_key = NULL;
        }
        aos_memset(msslconf_list[ssl_ctx_id].client_key_name, 0, SSL_CERTKEY_NAME_LEN);
        aos_memcpy(msslconf_list[ssl_ctx_id].client_key_name, file, strlen(file));
        msslconf_list[ssl_ctx_id].client_key = cert + offset;
        msslconf_list[ssl_ctx_id].client_key_len = rsize + 1;
    }
    return AIC_SUCCESS;

ERR_EXIT:
    if (cert)
        aos_free(cert);
    return AIC_EFAULT;
}

static u16 aic_ssl_find_empty_id(void)
{
    int i;
    u16 item_id, len;

    item_id = MD_ID_CERT_KEY0;
    for (i = 0; i < SSL_CERT_OR_KEY_MAX; i++) {
        len = aic_md_get_length(item_id + i);
        if (len == 0)
            return item_id + i;
    }

    return 0;
}

#endif
u32 aic_at_fs_write_normal(int fd, u8 *data, u32 data_len)
{
    int ret, actual_len;
    u8 *buffer = NULL;

    buffer = aos_zalloc_relaxed(data_len);
    if (!buffer)
        return AIC_ENOBUFS;

    aic_escape_data_normal(buffer, data, data_len, &actual_len);
#ifdef CONFIG_FS
    ret = fs_write(fd, buffer, actual_len);
#endif
    aos_free(buffer);
    return ret;
}

static u32 aic_mssl_cert_key_write(AT_REQ_RESP *at_req_resp_ptr, u8 type)
{
    int wsize, len;
    struct ssl_cert_key_info *ptr;
#ifndef CONFIG_FS
    int ret;
    u16 item_id;
    int item_len;
#endif

    if (at_req_resp_ptr->end_data_mode) {
#ifdef CONFIG_FS
        fs_delete(ssl_cert_info.name);
#endif
        goto WR_ERR_EXIT;
    }

    if (!at_req_resp_ptr->at_upstream_data)
        return AIC_TCPIP_AT_ERR;

    if (!ssl_cert_info.buffer) {
        ssl_cert_info.buffer = aos_zalloc_relaxed(ssl_cert_info.length + sizeof(struct ssl_cert_key_info));
        if (!ssl_cert_info.buffer) {
#ifdef CONFIG_FS
            fs_delete(ssl_cert_info.name);
#endif
            goto WR_ERR_EXIT;
        }
    }

    len = sizeof(struct ssl_cert_key_info);
    if (!ssl_cert_info.no_first) {
        ssl_cert_info.offset = len;
        ssl_cert_info.no_first = 1;
    }
    if (at_req_resp_ptr->upstream_data_len >= ssl_cert_info.length) {
        aos_memcpy(ssl_cert_info.buffer + ssl_cert_info.offset,
                   at_req_resp_ptr->at_upstream_data, ssl_cert_info.length);
    } else if (at_req_resp_ptr->upstream_data_len < ssl_cert_info.length) {
        if (at_req_resp_ptr->upstream_data_len >=
            (ssl_cert_info.length - (ssl_cert_info.offset - len))) {
            aos_memcpy(ssl_cert_info.buffer + ssl_cert_info.offset,
                       at_req_resp_ptr->at_upstream_data,
                       ssl_cert_info.length - (ssl_cert_info.offset - len));
        } else {
            aos_memcpy(ssl_cert_info.buffer + ssl_cert_info.offset,
                       at_req_resp_ptr->at_upstream_data,
                       at_req_resp_ptr->upstream_data_len);
            ssl_cert_info.offset += at_req_resp_ptr->upstream_data_len;
            return AIC_TCPIP_AT_OK;
        }
    }

#ifdef CONFIG_FS
    ptr = (struct ssl_cert_key_info *)ssl_cert_info.buffer;
    aos_memcpy(ptr->name, ssl_cert_info.name, strlen(ssl_cert_info.name));
    ptr->len = ssl_cert_info.length;
    ptr->type = type;
    wsize = fs_write(ssl_cert_info.file, (ssl_cert_info.buffer),
                     ssl_cert_info.length + len);
    if (wsize < 0) {
        fs_delete(ssl_cert_info.name);
        goto WR_ERR_EXIT;
    } else {
        aic_reply_ok_resp(at_req_resp_ptr);
        goto WR_EXIT;
    }
#else
    // if (ssl_cert_info.id)
    //     item_id = ssl_cert_info.id;
    // else
        item_id = aic_ssl_find_empty_id();
    wsize = ssl_cert_info.length + len;
    ptr = (struct ssl_cert_key_info *)ssl_cert_info.buffer;
    aos_memcpy(ptr->name, ssl_cert_info.name, strlen(ssl_cert_info.name));
    ptr->len = ssl_cert_info.length;
    ptr->type = type;
    item_len = wsize + 4 - (wsize % 4);
    ret = aic_md_write(item_id, item_len, ssl_cert_info.buffer);
    if (ret != AIC_SUCCESS) {
        goto WR_ERR_EXIT;
    } else {
        aic_reply_ok_resp(at_req_resp_ptr);
        goto WR_EXIT;
    }
#endif

WR_ERR_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
WR_EXIT:
#ifdef CONFIG_FS
    fs_close(ssl_cert_info.file);
#endif
    if (ssl_cert_info.buffer)
        aos_free(ssl_cert_info.buffer);
    aos_memset(&ssl_cert_info, 0, sizeof(ssl_cert_info));
    at_req_resp_ptr->at_mode = AT_CMD_MODE;
    return AIC_TCPIP_AT_OK;
}

int aic_ssl_get_cert_key_by_name(u8 *file, u8 type, u16 *item_id, int *item_len)
{
    int find = 0;
#ifdef CONFIG_FS
    struct list_head *pos, *next;
    struct list_head list;
    struct fs_dir_info *dir;

    list_head_init(&list);

    if (type == SSL_CA_CERT || type == SSL_CLIENT_CERT) {
        fs_lsdir(AT_SSL_CERT_DIR, &list);
    } else if (type == SSL_CLIENT_KEY) {
        fs_lsdir(AT_SSL_KEY_DIR, &list);
    } else {
        return AIC_ENOTSUP;
    }

    list_for_each_safe(pos, next, &list) {
        dir = list_entry(pos, struct fs_dir_info, node);

        if (strncmp(file, dir->dent.d_name, strlen(file)) == 0){
            find = 1;
        }

        list_del(&dir->node);
        aos_free(dir);
    }
#else
    int i;
    u16 id, len;
    u8 *buffer;
    struct ssl_cert_key_info *ptr = AIC_NULL;

    id = MD_ID_CERT_KEY0;
    for (i = 0; i < SSL_CERT_OR_KEY_MAX; i++) {
        len = aic_md_get_length(id + i);
        if (len == 0) {
            continue;
        } else {
            buffer = aos_zalloc_relaxed(len);
            if (buffer) {
                aic_md_read(id + i, len, buffer);
                ptr = (struct ssl_cert_key_info *)buffer;
                if (strncmp(file, ptr->name, strlen(file)) == 0 && 
                    ptr->type == type) {
                    if (item_id)
                        *item_id = id + i;
                    if (item_len)
                        *item_len = ptr->len;
                    find = 1;
                    aos_free(buffer);
                    buffer = NULL;
                    return find;
                }
                aos_free(buffer);
                buffer = NULL;
            }
        }
    }

#endif
    return find;
}

#ifdef CONFIG_FS
void aic_ssl_cert_or_key_loopup(AT_REQ_RESP *at_req_resp_ptr, int type)
{
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    struct list_head list;
    struct list_head *pos, *next;
    struct fs_dir_info *dir;
    u8 *cert = NULL;
    int fd, rsize, header_size;
    u8 file_name[128];
    struct ssl_cert_key_info *info_ptr = AIC_NULL;
    int ret;

    list_head_init(&list);
    if (type == SSL_CA_CERT || type == SSL_CLIENT_CERT) {
        fs_lsdir(AT_SSL_CERT_DIR, &list);
    } else if (type == SSL_CLIENT_KEY) {
        fs_lsdir(AT_SSL_KEY_DIR, &list);
    } else {
        return;
    }

    list_for_each_safe(pos, next, &list) {
        dir = list_entry(pos, struct fs_dir_info, node);

        aos_memset(file_name, '\0', sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%s/%s", AT_SSL_CERT_DIR, 
                 dir->dent.d_name);
        fd = fs_open(file_name, FS_O_RDONLY);
        if (fd < 0) {
            continue;
        }

        ret = fs_seek(fd, 0, FS_SEEK_BEGIN);
        if (ret != 0) {
            goto RUN;
        }

        header_size = sizeof(struct ssl_cert_key_info);
        cert = aos_zalloc_relaxed(header_size + 1);
        if (!cert) {
            goto RUN;
        }
        rsize = fs_read(fd, cert, header_size);
        if (rsize <= 0) {
            goto RUN;
        }

        info_ptr = (struct ssl_cert_key_info *)cert;
        if (strncmp(file_name, info_ptr->name, strlen(info_ptr->name)) != 0) {
            aicx_info(0x05f5c4, "ss", "dir d_name %s, cert name %s", dir->dent.d_name, info_ptr->name);
            goto RUN;
        }

        if (((type == SSL_CA_CERT || type == SSL_CLIENT_CERT) &&
             info_ptr->type != SSL_CA_CERT && info_ptr->type != SSL_CLIENT_CERT) ||
             (type == SSL_CLIENT_KEY && info_ptr->type != SSL_CLIENT_KEY)) {
            goto RUN;
        }

        snprintf(str, AT_RESP_RESULT_MAX, RESP_SSL_LIST"\"%s\",%d", dir->dent.d_name,
                 info_ptr->len);
        aic_transmit_resp_normal(at_req_resp_ptr, str);

RUN:
        list_del(&dir->node);
        aos_free(dir);
        fs_close(fd);
        if (cert) {
            aos_free(cert);
            cert = NULL;
        }
        continue;
    }
}
#else
void aic_ssl_cert_or_key_loopup(AT_REQ_RESP *at_req_resp_ptr, int type)
{
    int i;
    u16 id, len;
    u8 *buffer;
    struct ssl_cert_key_info *ptr = AIC_NULL;
    char str[AT_RESP_RESULT_MAX] = {'\0'};

    id = MD_ID_CERT_KEY0;
    for (i = 0; i < SSL_CERT_OR_KEY_MAX; i++) {
        len = aic_md_get_length(id + i);
        if (len == 0) {
            continue;
        } else {
            buffer = aos_zalloc_relaxed(len);
            if (buffer) {
                aic_md_read(id + i, len, buffer);
                ptr = (struct ssl_cert_key_info *)buffer;
                if (ptr->type == type) {
                    snprintf(str, AT_RESP_RESULT_MAX, RESP_SSL_LIST"\"%s\",%d", ptr->name,
                             ptr->len);
                    aic_transmit_resp_normal(at_req_resp_ptr, str);
                }
                aos_free(buffer);
                buffer = NULL;
            }
        }
    }
}
#endif

#ifdef CRC32_CHECK
static u32 crc32_check(u32 *data, u32 len)
{
    u32 cksum = 0;
    u32 i, j;

    for (i = 0; i < len; i++) {
        cksum ^= data[i];
        for (j = 0; j < 8; j++) {
            if (cksum & 0x00000001)
                cksum = (cksum >> 1) ^ CRC_POLY;
            else
                cksum = cksum >> 1;
        }
    }

    return ~cksum;
}
#else
#ifdef CRC16_CHECK
static u16 crc16_check(u16 *data, u32 len)
{
    unsigned long cksum = 0;

    while (len > 1) {
        cksum += *data++;
        len -= sizeof(u16);
    }

    if (len) {
        cksum += *(u8 *)data;
    }

    //covert 32 bit to 16 bit
    while (cksum >> 16)
        cksum = (cksum >> 16) + (cksum & 0xffff);

    return (u16) (~cksum);
}
#endif
#endif

static u32 handle_msslcfg_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    snprintf(str, sizeof(str), RESP_SSL_CFG"\"auth\",(0-5),(0-2)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_SSL_CFG"\"cert\",(0-5),,,");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_SSL_CFG"\"encoding\",(0-5),(2)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_SSL_CFG"\"negotime\",(0-5),(10-300)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_SSL_CFG"\"version\",(0-5),(0-3)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_SSL_CFG"\"ciphersuite\",(0-5),");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_SSL_CFG"\"session\",(0-5),(0-1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_SSL_CFG"\"ignorestamp\",(0-5),(0-1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aos_memset(str, '\0', AT_RESP_RESULT_MAX);
    snprintf(str, sizeof(str), RESP_SSL_CFG"\"ignoreverify\",(0-5),(0-1)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

static u32 handle_msslcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int i, ret, id = 0, str_len, num;
    u8 paras[8] = {'\0'};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_SSL_CFG"=",
                                             &cnt);
    if (!paras_arr) {
        goto AT_TLS_EXIT;;
    }

    if (cnt >= 2) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
            goto AT_TLS_EXIT;
        }
        if (strlen((u8 *)paras_arr[1])) {
            id = atoi((u8 *)paras_arr[1]);
            if (id >= SSL_CTX_ID_MAX) {
                goto AT_TLS_EXIT;
            }
            msslconf_list[id].ssl_context_id = id;
        } else {
            goto AT_TLS_EXIT;
        }
    }

    str_len = strlen((u8*)paras_arr[0]);
    if (strncasecmp((u8*)paras_arr[0], "auth", 4) == 0 && str_len == 4) {
        if (cnt == 3) {
            num = 0;
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                goto AT_TLS_EXIT;
            }
            if (strlen((u8 *)paras_arr[2]))
                num = atoi((u8*)paras_arr[2]);

            if (num > 2) {
                goto AT_TLS_EXIT;
            }

            msslconf_list[id].seclevel = num;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"auth\",%d,%d",
                     id, msslconf_list[id].seclevel);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"auth\",%d,%d",
                         i, msslconf_list[i].seclevel);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "cert", 4) == 0 &&
        str_len == 4) {
        if (cnt > 2 && cnt <= 5) {
            if (cnt >= 3) {
                ret = handle_cert_file((u8*)paras_arr[2], id, SSL_CA_CERT);
                if (ret != AIC_SUCCESS)
                    goto AT_TLS_EXIT;
            }

            if (cnt >= 4) {
                ret = handle_cert_file((u8*)paras_arr[3], id, SSL_CLIENT_CERT);
                if (ret != AIC_SUCCESS)
                    goto AT_TLS_EXIT;
            }

            if (cnt == 5) {
                ret = handle_cert_file((u8*)paras_arr[4], id, SSL_CLIENT_KEY);
                if (ret != AIC_SUCCESS)
                    goto AT_TLS_EXIT;
            }
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"cert\",%d,\"%s\",\"%s\",\"%s\"",
                     id, msslconf_list[id].cacert_name,
                     msslconf_list[id].client_cert_name,
                     msslconf_list[id].client_key_name);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"cert\",%d,\"%s\",\"%s\",\"%s\"",
                         i, msslconf_list[i].cacert_name,
                         msslconf_list[i].client_cert_name,
                         msslconf_list[i].client_key_name);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    }/* else if (strncasecmp((u8*)paras_arr[0], "psk", 3) == 0 &&
               str_len == 3) {
        if (cnt == 3) {
            if (strlen((u8 *)paras_arr[2]) < 64)
                strncpy(msslconf_list[id].pskid, (u8*)paras_arr[2],
                        strlen((u8 *)paras_arr[2]));
            else
                goto AT_TLS_EXIT;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"psk\",%d,%s",
                     id, msslconf_list[id].pskid);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"psk\",%d,%s",
                         i, msslconf_list[i].pskid);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    }*/ else if (strncasecmp((u8*)paras_arr[0], "encoding", 8) == 0 &&
               str_len == 8) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                goto AT_TLS_EXIT;
            }
            num = atoi((u8*)paras_arr[2]);
            if (num <= 2)
                msslconf_list[id].encoding = atoi((u8*)paras_arr[2]);
            else
                goto AT_TLS_EXIT;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"encoding\",%d,%d",
                     id, msslconf_list[id].encoding);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"encoding\",%d,%d",
                         i, msslconf_list[i].encoding);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "negotime", 8) == 0 &&
               str_len == 8) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                goto AT_TLS_EXIT;
            }
            num = atoi((u8*)paras_arr[2]);
            if (num >= 10 && num <= SSL_NEGOTIATE_TIME)
                msslconf_list[id].negotiate_time = num;
            else
                goto AT_TLS_EXIT;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"negotime\",%d,%d",
                     id, msslconf_list[id].negotiate_time);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"negotime\",%d,%d",
                         i, msslconf_list[i].negotiate_time);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "version", 7) == 0 &&
        str_len == 7) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                goto AT_TLS_EXIT;
            }
            num = atoi((u8*)paras_arr[2]);
            if (num < 4 || num == 0xFF)
                msslconf_list[id].ssl_version = num;
            else
                goto AT_TLS_EXIT;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"version\",%d,%d",
                     id, msslconf_list[id].ssl_version);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"version\",%d,%d",
                         i, msslconf_list[i].ssl_version);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ciphersuite", 11) == 0 &&
               str_len == 11) {
        if (cnt == 3) {
            if (strlen((u8 *)paras_arr[2]) <= 6 && strlen((u8 *)paras_arr[2]) > 0) {
                if (strncasecmp((u8 *)paras_arr[2], "0x", 2) == 0)
                    aos_memcpy(paras, (u8*)paras_arr[2] + 2, strlen((u8 *)paras_arr[2]) - 2);
                else
                    aos_memcpy(paras, (u8*)paras_arr[2], strlen((u8 *)paras_arr[2]));
                num = aic_at_string_to_num(paras);
                if (num >= 0)
                    msslconf_list[id].ciphersuite = num;
                else
                    goto AT_TLS_EXIT;
            } else {
                goto AT_TLS_EXIT;
            }

            if (msslconf_list[id].ciphersuite > 0xFFFF) {
                goto AT_TLS_EXIT;
            }
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"ciphersuite\",%d,%X",
                     id, msslconf_list[id].ciphersuite);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"ciphersuite\",%d,%X",
                         i, msslconf_list[i].ciphersuite);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "session", 7) == 0 &&
               str_len == 7) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                goto AT_TLS_EXIT;
            }
            num = atoi((u8*)paras_arr[2]);
            if (num < 2)
                msslconf_list[id].session = num;
            else
                goto AT_TLS_EXIT;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"session\",%d,%d",
                     id, msslconf_list[id].session);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"session\",%d,%d",
                         i, msslconf_list[i].session);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ignorestamp", 11) == 0 &&
               str_len == 11) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                goto AT_TLS_EXIT;
            }
            num = atoi((u8*)paras_arr[2]);
            if (num < 2)
                msslconf_list[id].ignore_ltime = num;
            else
                goto AT_TLS_EXIT;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"ignorestamp\",%d,%d",
                     id, msslconf_list[id].ignore_ltime);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                aos_memset(str, '\0', sizeof(str));
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"ignorestamp\",%d,%d",
                         i, msslconf_list[i].ignore_ltime);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ignoreverify", 12) == 0 &&
               str_len == 12) {
        if (cnt == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
                goto AT_TLS_EXIT;
            }
            num = atoi((u8*)paras_arr[2]);
            if (num < 2)
                msslconf_list[id].ignore_verify = num;
            else
                goto AT_TLS_EXIT;
        } else if (cnt == 2) {
            snprintf(str, sizeof(str), RESP_SSL_CFG"\"ignoreverify\",%d,%d",
                     id, msslconf_list[id].ignore_verify);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else if (cnt == 1) {
            for (i = 0; i < SSL_CTX_ID_MAX; i++) {
                snprintf(str, sizeof(str), RESP_SSL_CFG"\"ignoreverify\",%d,%d",
                         i, msslconf_list[i].ignore_verify);
                aic_transmit_resp_normal(at_req_resp_ptr, str);
            }
        } else {
            goto AT_TLS_EXIT;
        }
    } else {
        goto AT_TLS_EXIT;
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

AT_TLS_EXIT:
    aic_reply_error_resp(at_req_resp_ptr,
                            SSL_INVALID_PARAMETER);
    aic_free_paras_mem(paras_arr, cnt);
    return SSL_INVALID_PARAMETER;

}

static u32 handle_msslcertwr_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}


static u32 handle_msslcertwr_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras = AIC_NULL;
    u16 cnt;
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    int len, ret;
    u16 item_id = 0;
#ifdef CONFIG_FS
    u8 cert_name[128] = {'\0'};
    int fd, id = 0, wsize;
#endif

    /* AT mode */
    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_SSL_CERTWR"=",
                                             &cnt);
        if (!paras || (cnt < 3 || cnt > 4)) {
            goto ERR_EXIT;
        }

        len = strlen((u8 *)paras[0]);
        if (len > SSL_CERTKEY_NAME_LEN) {
            goto ERR_EXIT;
        }

        ret = aic_ssl_get_cert_key_by_name((u8 *)paras[0], SSL_CA_CERT, &item_id, NULL);
        if (ret == 1) {
#ifdef CONFIG_FS
            snprintf(cert_name, sizeof(cert_name), "%s/%s", AT_SSL_CERT_DIR, (u8 *)paras[0]);
            fs_delete(cert_name);
#else
            aic_md_delete(item_id);
#endif
        }

#ifdef CONFIG_FS
        snprintf(cert_name, sizeof(cert_name), "%s/%s", AT_SSL_CERT_DIR, (u8 *)paras[0]);
        fd = fs_open(cert_name, FS_O_WRONLY | FS_O_CREATE);
        if (fd < 0) {
            aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
            aic_free_paras_mem(paras, cnt);
            return SSL_INVALID_PARAMETER;
        }

        ret = fs_seek(fd, 0, FS_SEEK_BEGIN);
        if (ret != 0) {
            fs_close(fd);
            aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
            aic_free_paras_mem(paras, cnt);
            return SSL_INVALID_PARAMETER;
        }
#endif
        if (!aic_is_valid_integer_para((u8 *)paras[1], strlen((u8 *)paras[1])) ||
            !aic_is_valid_integer_para((u8 *)paras[2], strlen((u8 *)paras[2])) ) {
            goto ERR_EXIT;
        }
#ifdef CONFIG_FS
        strncpy(ssl_cert_info.name, cert_name, strlen(cert_name));
        ssl_cert_info.file = fd;
#else
        strncpy(ssl_cert_info.name, (u8 *)paras[0], strlen((u8 *)paras[0]));
        ssl_cert_info.id = item_id;
#endif
        ssl_cert_info.remain_length = atoi((u8 *)paras[1]);
        ssl_cert_info.length = atoi((u8 *)paras[2]);
        ssl_cert_info.offset = 0;
        if (cnt == 4) {
#ifdef CONFIG_FS
            if (ssl_cert_info.length > 0) {
                if (msslconf_list[id].encoding == 2 && 
                    ((ssl_cert_info.length * 2) != strlen((u8 *)paras[3]))) {
                    fs_close(fd);
                    fs_delete(cert_name);
                    goto ERR_EXIT;
                } else if (ssl_cert_info.length != strlen((u8 *)paras[3])) {
                    fs_close(fd);
                    fs_delete(cert_name);
                    goto ERR_EXIT;
                }
            }

            wsize = fs_write(fd, (u8 *)paras[3], ssl_cert_info.length);
            if (wsize < 0) {
                aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
                aic_free_paras_mem(paras, cnt);
                fs_close(fd);
                fs_delete(cert_name);
                return SSL_INVALID_PARAMETER;
            }
            fs_close(fd);
            aic_reply_ok_resp(at_req_resp_ptr);
#else
            goto ERR_EXIT;
#endif
        } else {
            if (!ssl_cert_info.length) {
#ifdef CONFIG_FS
                fs_close(fd);
                fs_delete(cert_name);
#endif
                goto ERR_EXIT;
            }
            strcpy(str, ">");
            at_req_resp_ptr->at_mode = AT_DATA_MODE;
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        }
        goto EXIT;
ERR_EXIT:
        aic_reply_error_resp(at_req_resp_ptr, SSL_INVALID_PARAMETER);
EXIT:
        aic_free_paras_mem(paras, cnt);
        return AIC_TCPIP_AT_OK;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        aic_mssl_cert_key_write(at_req_resp_ptr, SSL_CA_CERT);
        return AIC_TCPIP_AT_OK;
    } else {
        aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
        return AIC_TCPIP_AT_ERR;
    }
}

static u32 handle_msslkeywr_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

static u32 handle_msslkeywr_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras = AIC_NULL;
    u16 cnt;
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    int len, ret;
    u16 item_id = 0;
#ifdef CONFIG_FS
    u8 key_name[128] = {'\0'};
    int fd, wsize, id = 0;
#endif

    /* AT mode */
    if (at_req_resp_ptr->at_mode == AT_CMD_MODE) {
        paras = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             AT_SSL_KEYWR"=",
                                             &cnt);
        if (!paras || (cnt < 3 || cnt > 4)) {
            goto ERR_EXIT;
        }

        len = strlen((u8 *)paras[0]);
        if (len > SSL_CERTKEY_NAME_LEN) {
            goto ERR_EXIT;
        }

        ret = aic_ssl_get_cert_key_by_name((u8 *)paras[0], SSL_CLIENT_KEY, &item_id, NULL);
        if (ret == 1) {
#ifdef CONFIG_FS
            snprintf(key_name, sizeof(key_name), "%s/%s", AT_SSL_KEY_DIR, (u8 *)paras[0]);
            fs_delete(key_name);
#else
            aic_md_delete(item_id);
#endif
        }
#ifdef CONFIG_FS
        snprintf(key_name, sizeof(key_name), "%s/%s", AT_SSL_KEY_DIR, (u8 *)paras[0]);
        fd = fs_open(key_name, FS_O_WRONLY | FS_O_CREATE);
        if (fd < 0) {
            aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
            aic_free_paras_mem(paras, cnt);
            return SSL_INVALID_PARAMETER;
        }

        ret = fs_seek(fd, 0, FS_SEEK_BEGIN);
        if (ret != 0) {
            fs_close(fd);
            aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
            aic_free_paras_mem(paras, cnt);
            return SSL_INVALID_PARAMETER;
        }
#endif
        if (!aic_is_valid_integer_para((u8 *)paras[1], strlen((u8 *)paras[1])) ||
            !aic_is_valid_integer_para((u8 *)paras[2], strlen((u8 *)paras[2])) ) {
            goto ERR_EXIT;
        }
#ifdef CONFIG_FS
        strncpy(ssl_cert_info.name, key_name, strlen(key_name));
        ssl_cert_info.file = fd;
#else
        strncpy(ssl_cert_info.name, (u8 *)paras[0], strlen((u8 *)paras[0]));
        ssl_cert_info.id = item_id;
#endif
        ssl_cert_info.remain_length = atoi((u8 *)paras[1]);
        ssl_cert_info.length = atoi((u8 *)paras[2]);
        ssl_cert_info.offset = 0;
        if (cnt == 4) {
#ifdef CONFIG_FS
            if (ssl_cert_info.length > 0) {
                if (msslconf_list[id].encoding == 2 && 
                    ((ssl_cert_info.length * 2) != strlen((u8 *)paras[3]))) {

                    fs_close(fd);
                    fs_delete(key_name);

                    goto ERR_EXIT;
                } else if (ssl_cert_info.length != strlen((u8 *)paras[3])) {

                    fs_close(fd);
                    fs_delete(key_name);

                    goto ERR_EXIT;
                }
            }
            wsize = fs_write(fd, (u8 *)paras[3], ssl_cert_info.length);
            if (wsize < 0) {
                aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
                aic_free_paras_mem(paras, cnt);
                fs_close(fd);
                fs_delete(key_name);
                return SSL_INVALID_PARAMETER;
            }
            fs_close(fd);
            aic_reply_ok_resp(at_req_resp_ptr);
#else
            goto ERR_EXIT;
#endif
        } else {
            if (!ssl_cert_info.length) {
#ifdef CONFIG_FS
                fs_close(fd);
                fs_delete(key_name);
#endif
                goto ERR_EXIT;
            }
            strcpy(str, ">");
            at_req_resp_ptr->at_mode = AT_DATA_MODE;
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        }
        goto EXIT;
ERR_EXIT:
        aic_reply_error_resp(at_req_resp_ptr, SSL_INVALID_PARAMETER);
EXIT:
        aic_free_paras_mem(paras, cnt);
        return AIC_TCPIP_AT_OK;
    } else if (at_req_resp_ptr->at_mode == AT_DATA_MODE) {
        aic_mssl_cert_key_write(at_req_resp_ptr, SSL_CLIENT_KEY);
        return AIC_TCPIP_AT_OK;
    } else {
        aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_WRITE_ERR);
        return AIC_TCPIP_AT_ERR;
    }
}

static u32 handle_msslcertrd_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras = AIC_NULL;
    u16 cnt, len;
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    u8 *cert = AIC_NULL;
    int ret, rsize = 0, offset = 0;
    u16 item_id = 0;
#ifdef CONFIG_FS
    int fd;
    u8 cert_name[128];
    FS_FILE_INFO *file_info = AIC_NULL;
    struct ssl_cert_key_info *ptr = AIC_NULL;
#else
    u16 item_size = 0;
#endif

    paras = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                         AT_SSL_CERTRD"=",
                                         &cnt);
    if (!paras || cnt != 1) {
        aic_reply_error_resp(at_req_resp_ptr, SSL_INVALID_PARAMETER);
        goto EXIT;
    }

    ret = aic_ssl_get_cert_key_by_name((u8 *)paras[0], SSL_CA_CERT, &item_id, &rsize);
    if (ret != 1) {
        aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_NOT_EXIST);
        goto EXIT;
    }

#ifdef CONFIG_FS
    snprintf(cert_name, sizeof(cert_name), "%s/%s", AT_SSL_CERT_DIR, (u8 *)paras[0]);
    file_info = aos_malloc(sizeof(FS_FILE_INFO));
    ret = fs_getinfo(cert_name, file_info);
    if (ret != AIC_SUCCESS) {
        goto ERR_EXIT;
    }

    fd = fs_open(cert_name, FS_O_RDONLY);
    if (fd < 0) {
        goto ERR_EXIT;
    }

    ret = fs_seek(fd, 0, FS_SEEK_BEGIN);
    if (ret != 0) {
        fs_close(fd);
        goto ERR_EXIT;
    }

    offset = sizeof(struct ssl_cert_key_info);
    cert = aos_zalloc(file_info->size);
    rsize = fs_read(fd, cert, file_info->size);
    if (rsize <= 0) {
        fs_close(fd);
        goto ERR_EXIT;
    }

    ret = fs_close(fd);
    if (ret != AIC_SUCCESS) {
        goto ERR_EXIT;
    }

    ptr = (struct ssl_cert_key_info *)cert;
    if (ptr->type == SSL_CLIENT_KEY) {
        aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_NOT_EXIST);
        goto EXIT;
    }
    rsize -= offset;
#else
    if (item_id == 0 || rsize == 0) {
        goto ERR_EXIT;
    }

    item_size = aic_md_get_length(item_id);
    if (item_size == 0) {
        goto ERR_EXIT;
    }
    cert = aos_zalloc_relaxed(item_size);
    if (!cert) {
        goto ERR_EXIT;
    }

    ret = aic_md_read(item_id, item_size, cert);
    if (ret != AIC_SUCCESS) {
        goto ERR_EXIT;
    }
    offset = sizeof(struct ssl_cert_key_info);
#endif
    snprintf(str, SSL_CERT_OR_KEY_LEN, RESP_SSL_CERTRD"%d,", rsize);
    len = strlen(str);
    aos_memcpy(cert + offset - len, str, len);
    aic_transmit_at_data(at_req_resp_ptr->serial_port_id,
                         (cert + offset - len), len + rsize);
    aic_reply_ok_resp(at_req_resp_ptr);
    goto EXIT;

ERR_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_READ_ERR);
EXIT:
#ifdef CONFIG_FS
    if (file_info)
        aos_free(file_info);
#endif
    if (cert)
        aos_free(cert);
    aic_free_paras_mem(paras, cnt);
    return AIC_TCPIP_AT_OK;
}

static u32 handle_mssllist_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras = AIC_NULL;
    u16 cnt;
    int type;

    paras = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                         AT_SSL_LIST"=",
                                         &cnt);
    if (!paras || cnt != 1) {
        goto ERR_EXIT;
    }

    if (!aic_is_valid_integer_para((u8 *)paras[0], strlen((u8 *)paras[0]))) {
        goto ERR_EXIT;
    }

    type = atoi((u8 *)paras[0]);
    if (type == 1) {
        aic_ssl_cert_or_key_loopup(at_req_resp_ptr, SSL_CA_CERT);
        goto SSLLIST_EXIT;
    } else if (type == 2) {
        aic_ssl_cert_or_key_loopup(at_req_resp_ptr, SSL_CLIENT_KEY);
        goto SSLLIST_EXIT;
    } else if (type == 3) {
        goto SSLLIST_EXIT;
    } else {
        goto ERR_EXIT;
    }

SSLLIST_EXIT:
    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras, cnt);
    return AIC_TCPIP_AT_OK;

ERR_EXIT:
    aic_reply_error_resp(at_req_resp_ptr, SSL_INVALID_PARAMETER);
    aic_free_paras_mem(paras, cnt);
    return SSL_INVALID_PARAMETER;
}

static u32 handle_msslrm_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras = AIC_NULL;
    u16 cnt, item_id;
    int ret;
#ifdef CONFIG_FS
    u8 cert_name[128];
#endif

    paras = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                         AT_SSL_RM"=",
                                         &cnt);
    if (!paras || cnt != 1) {
        aic_reply_error_resp(at_req_resp_ptr, SSL_INVALID_PARAMETER);
        aic_free_paras_mem(paras, cnt);
        return SSL_INVALID_PARAMETER;
    }

    // aic_ssl_delete_all_id();
    ret = aic_ssl_get_cert_key_by_name((u8 *)paras[0], SSL_CA_CERT, &item_id, NULL);
    if (ret != 1) {
        ret = aic_ssl_get_cert_key_by_name((u8 *)paras[0], SSL_CLIENT_KEY, &item_id, NULL);
        if (ret != 1) {
            aic_reply_error_resp(at_req_resp_ptr, SSL_CERT_OR_KEY_NOT_EXIST);
        } else {
#ifdef CONFIG_FS
            snprintf(cert_name, sizeof(cert_name), "%s/%s", AT_SSL_KEY_DIR, (u8 *)paras[0]);
            fs_delete(cert_name);
#else
            aic_md_delete(item_id);
#endif
            aic_reply_ok_resp(at_req_resp_ptr);
        }
    } else {
#ifdef CONFIG_FS
        snprintf(cert_name, sizeof(cert_name), "%s/%s", AT_SSL_CERT_DIR, (u8 *)paras[0]);
        fs_delete(cert_name);
#else
        aic_md_delete(item_id);
#endif
        aic_reply_ok_resp(at_req_resp_ptr);
    }

    aic_free_paras_mem(paras, cnt);
    return AIC_TCPIP_AT_OK;
}

static u32 handle_msslcheck_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    strcpy(str, RESP_SSL_CHECK",(0)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

static u32 handle_msslcheck_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    ulong *paras = AIC_NULL;
    u16 cnt;
    int ret, algo, error, rsize = 0, offset = 0;
    u32 result;
#ifdef CONFIG_CRYPTO_MBEDTLS
    u32 output[8] = {0};
#endif
    char str[AT_RESP_RESULT_MAX] = {'\0'};
    u8 *cert = NULL;
    u16 item_id = 0;
#ifdef CONFIG_FS
    u8 cert_name[128];
    int fd;
    FS_FILE_INFO *file_info = NULL;
#else
    u16 item_size = 0;
#endif

    paras = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                         AT_SSL_CHECK"=",
                                         &cnt);
    if (!paras || cnt < 1 || cnt > 2) {
        error = SSL_INVALID_PARAMETER;
        goto ERR_EXIT;
    }

    if (cnt >= 1) {
        ret = aic_ssl_get_cert_key_by_name((u8 *)paras[0], SSL_CA_CERT, &item_id, &rsize);
        if (ret != 1) {
            ret = aic_ssl_get_cert_key_by_name((u8 *)paras[0], SSL_CLIENT_KEY, &item_id, &rsize);
            if (ret != 1) {
                error = SSL_CERT_OR_KEY_NOT_EXIST;
                goto ERR_EXIT;
            }
#ifdef CONFIG_FS
            else {
                snprintf(cert_name, sizeof(cert_name), "%s/%s", AT_SSL_KEY_DIR, (u8 *)paras[0]);
            }
#endif
        }
#ifdef CONFIG_FS
        else {
            snprintf(cert_name, sizeof(cert_name), "%s/%s", AT_SSL_CERT_DIR, (u8 *)paras[0]);
        }
#endif
        if (cnt == 1)
            algo = MD5;
    }
#ifdef CONFIG_FS
    file_info = aos_malloc(sizeof(FS_FILE_INFO));
    ret = fs_getinfo(cert_name, file_info);
    if (ret != AIC_SUCCESS) {
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }

    fd = fs_open(cert_name, FS_O_RDONLY);
    if (fd < 0) {
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }

    ret = fs_seek(fd, 0, FS_SEEK_BEGIN);
    if (ret != 0) {
        fs_close(fd);
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }

    cert = aos_zalloc(file_info->size);
    rsize = fs_read(fd, cert, file_info->size);
    if (rsize <= 0) {
        fs_close(fd);
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }

    ret = fs_close(fd);
    if (ret != AIC_SUCCESS) {
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }

    offset = sizeof(struct ssl_cert_key_info);
    rsize -= offset;
#else
    if (item_id == 0 || rsize == 0) {
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }

    item_size = aic_md_get_length(item_id);
    if (item_size == 0) {
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }
    cert = aos_zalloc_relaxed(item_size);
    if (!cert) {
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }

    ret = aic_md_read(item_id, item_size, cert);
    if (ret != AIC_SUCCESS) {
        error = SSL_CERT_OR_KEY_READ_ERR;
        goto ERR_EXIT;
    }
    offset = sizeof(struct ssl_cert_key_info);
#endif
    if (cnt == 2) {
        if (!aic_is_valid_integer_para((u8 *)paras[1], strlen((u8 *)paras[1]))) {
            error = SSL_INVALID_PARAMETER;
            goto ERR_EXIT;
        }
        algo = atoi((u8 *)paras[1]);
        if (algo >= ALGO_MAX) {
            error = SSL_INVALID_PARAMETER;
            goto ERR_EXIT;
        }
    }

    switch(algo) {
#ifdef CONFIG_CRYPTO_MBEDTLS
    case MD5:
    {
        mbedtls_md5_context md5_ctx;
        mbedtls_md5_starts(&md5_ctx);
        mbedtls_md5_update(&md5_ctx, cert + offset, rsize);
        mbedtls_md5_finish(&md5_ctx, (u8 *)output);
        NX_CHANGE_ULONG_ENDIAN(output[0]);
        NX_CHANGE_ULONG_ENDIAN(output[1]);
        NX_CHANGE_ULONG_ENDIAN(output[2]);
        NX_CHANGE_ULONG_ENDIAN(output[3]);
        snprintf(str, AT_RESP_RESULT_MAX, RESP_SSL_CHECK"%08X%08X%08X%08X", output[0],
                 output[1], output[2], output[3]);
        break;
    }
    case SHA:
    {
        mbedtls_sha1_context sha1_ctx;
        mbedtls_sha1_starts(&sha1_ctx);
        mbedtls_sha1_update(&sha1_ctx, cert + offset, rsize);
        mbedtls_sha1_finish(&sha1_ctx, (u8 *)output);
        NX_CHANGE_ULONG_ENDIAN(output[0]);
        NX_CHANGE_ULONG_ENDIAN(output[1]);
        NX_CHANGE_ULONG_ENDIAN(output[2]);
        NX_CHANGE_ULONG_ENDIAN(output[3]);
        NX_CHANGE_ULONG_ENDIAN(output[4]);
        snprintf(str, AT_RESP_RESULT_MAX, RESP_SSL_CHECK"%08X%08X%08X%08X%08X", output[0],
                 output[1], output[2], output[3], output[4]);
        break;
    }
    case SHA256:
    {
        mbedtls_sha256_context sha256_ctx;
        mbedtls_sha256_starts(&sha256_ctx, AIC_FALSE);
        mbedtls_sha256_update(&sha256_ctx, cert + offset, rsize);
        mbedtls_sha256_finish(&sha256_ctx, (u8 *)output);
        NX_CHANGE_ULONG_ENDIAN(output[0]);
        NX_CHANGE_ULONG_ENDIAN(output[1]);
        NX_CHANGE_ULONG_ENDIAN(output[2]);
        NX_CHANGE_ULONG_ENDIAN(output[3]);
        NX_CHANGE_ULONG_ENDIAN(output[4]);
        NX_CHANGE_ULONG_ENDIAN(output[5]);
        NX_CHANGE_ULONG_ENDIAN(output[6]);
        NX_CHANGE_ULONG_ENDIAN(output[7]);
        snprintf(str, AT_RESP_RESULT_MAX, RESP_SSL_CHECK"%08X%08X%08X%08X%08X%08X%08X%08X",
                 output[0], output[1], output[2], output[3], output[4],
                 output[5], output[6], output[7]);
        break;
    }
#endif
    case CRC:
#ifdef CRC32_CHECK
        result = crc32_check((u32 *)(cert + offset), rsize);
#else
#ifdef CRC16_CHECK
        result = crc16_check((u16 *)(cert + offset), rsize)
#endif
#endif
        snprintf(str, AT_RESP_RESULT_MAX, RESP_SSL_CHECK"%08X", result);
        break;
    default:
        break;
    }

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
#ifdef CONFIG_FS
    aos_free(file_info);
#endif
    aos_free(cert);
    aic_free_paras_mem(paras, cnt);
    return AIC_TCPIP_AT_OK;

ERR_EXIT:
#ifdef CONFIG_FS
    if (file_info)
        aos_free(file_info);
#endif
    if (cert)
        aos_free(cert);
    aic_reply_error_resp(at_req_resp_ptr, error);
    aic_free_paras_mem(paras, cnt);
    return AIC_TCPIP_AT_OK;
}

static u32 handle_msslcipher_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    strcpy(str, RESP_SSL_CIPHER"(0x3C,0x3D,0x35,0x2F,0xA,0xC0A8,0x8C,0xC0AA,0xC035,0xC02C)");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}



