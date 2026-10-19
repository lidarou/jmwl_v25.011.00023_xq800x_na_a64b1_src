/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to TLS.
 *  This is the handler of AT commands relating to TLS,
 *  such as set tls configure.etc.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09491d
#define LOG_MODULE    AMOD_NET

#include "aic_at_tls.h"
#ifdef CONFIG_FS
#include "fs_api.h"
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
static int ssl_ciphersuites[2] = {0, 0};
#endif
extern struct SOCKET_LIST sock_list[AT_SOCKET_MAX_CONN_NUM];
extern AIC_TCPIP_AT_CFG aic_tcpip_at_cfg;
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];
extern int error_code;

#ifdef CONFIG_NET_SECURE
extern NX_SECURE_TLS_SESSION *tls_session;
extern NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;
static int tls_auth_type;
#endif

#ifdef CONFIG_NX_RAM_DRIVER
extern VOID  _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);
#endif

u8 *ca_cert_der = AIC_NULL;
u32 ca_cert_der_len;

struct mbedtls_config sslconf_list[SSL_CTX_ID_MAX];

#ifdef AT_TLS_TEST
extern NX_IP tls_test_ip;

/* Trusted CA certificate for Client. */
static unsigned char ca_cert_der_test[] = {
  0x30, 0x82, 0x03, 0xc7, 0x30, 0x82, 0x02, 0xaf, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x09, 0x00, 0xa1, 0x79, 0xb0, 0x6a, 0x32, 0xbc, 0x48, 0x67,
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x0b, 0x05, 0x00, 0x30, 0x7a, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,
  0x55, 0x04, 0x08, 0x0c, 0x02, 0x43, 0x41, 0x31, 0x12, 0x30, 0x10, 0x06,
  0x03, 0x55, 0x04, 0x07, 0x0c, 0x09, 0x53, 0x61, 0x6e, 0x20, 0x44, 0x69,
  0x65, 0x67, 0x6f, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a,
  0x0c, 0x0d, 0x45, 0x78, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x4c, 0x6f,
  0x67, 0x69, 0x63, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x0b,
  0x0c, 0x0b, 0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72,
  0x65, 0x31, 0x1c, 0x30, 0x1a, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x13,
  0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x20,
  0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x31,
  0x36, 0x31, 0x31, 0x31, 0x31, 0x31, 0x39, 0x35, 0x30, 0x30, 0x38, 0x5a,
  0x17, 0x0d, 0x32, 0x36, 0x31, 0x31, 0x30, 0x39, 0x31, 0x39, 0x35, 0x30,
  0x30, 0x38, 0x5a, 0x30, 0x7a, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,
  0x55, 0x04, 0x08, 0x0c, 0x02, 0x43, 0x41, 0x31, 0x12, 0x30, 0x10, 0x06,
  0x03, 0x55, 0x04, 0x07, 0x0c, 0x09, 0x53, 0x61, 0x6e, 0x20, 0x44, 0x69,
  0x65, 0x67, 0x6f, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a,
  0x0c, 0x0d, 0x45, 0x78, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x4c, 0x6f,
  0x67, 0x69, 0x63, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x0b,
  0x0c, 0x0b, 0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72,
  0x65, 0x31, 0x1c, 0x30, 0x1a, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x13,
  0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x20,
  0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x41, 0x30, 0x82, 0x01, 0x22, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
  0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02,
  0x82, 0x01, 0x01, 0x00, 0xd1, 0xdc, 0x3c, 0xe1, 0x1c, 0x7a, 0x3d, 0xb7,
  0x76, 0xcf, 0xab, 0xd7, 0x3c, 0x38, 0xb0, 0x81, 0xb6, 0x37, 0x52, 0xa3,
  0x3d, 0x6f, 0xcd, 0x89, 0xa6, 0xa2, 0xf3, 0xa8, 0xb0, 0x8d, 0xee, 0x0b,
  0x36, 0x94, 0x83, 0x0e, 0x7f, 0x39, 0x87, 0x6e, 0xee, 0x19, 0xe2, 0x1f,
  0x92, 0x3d, 0x01, 0x05, 0x4f, 0x11, 0xcd, 0xcb, 0xa0, 0x79, 0xfc, 0x9d,
  0x6e, 0x93, 0xb1, 0xb7, 0x03, 0xf3, 0xfe, 0xeb, 0x30, 0x67, 0x38, 0x85,
  0x28, 0xdf, 0x93, 0xdb, 0xcb, 0xcb, 0xb1, 0xbe, 0xd3, 0xe1, 0xc2, 0x7d,
  0x8d, 0xbb, 0x70, 0x76, 0x99, 0x08, 0x7c, 0x3f, 0x21, 0x2f, 0x37, 0x97,
  0xf7, 0xe8, 0x6e, 0x8c, 0x7e, 0xbc, 0x30, 0x5f, 0xbf, 0x32, 0x51, 0x1d,
  0x66, 0x76, 0xad, 0x39, 0xfc, 0x94, 0xd4, 0x65, 0xf6, 0xd2, 0x0b, 0x37,
  0xd3, 0x4a, 0xe6, 0xe1, 0xdf, 0x4a, 0x8f, 0x3b, 0x33, 0x16, 0xbe, 0xf7,
  0xd9, 0xbd, 0x73, 0x64, 0xdf, 0x34, 0xa3, 0x55, 0xe7, 0xac, 0xab, 0xa7,
  0xae, 0xc2, 0x20, 0x46, 0xc2, 0xd1, 0xe3, 0x25, 0x3a, 0x47, 0x68, 0x92,
  0xac, 0xd6, 0x12, 0xa4, 0x0a, 0xce, 0xdc, 0xe2, 0x24, 0x12, 0xee, 0xe1,
  0xb2, 0xcd, 0x09, 0xa8, 0xef, 0x36, 0xea, 0x76, 0xf9, 0xb6, 0x63, 0xaa,
  0xac, 0xdd, 0x46, 0x06, 0x6e, 0xd9, 0x1e, 0x08, 0xac, 0x57, 0x12, 0x6c,
  0x21, 0xef, 0x8e, 0xae, 0xf0, 0x27, 0xf1, 0x5c, 0x79, 0xb4, 0xb6, 0x26,
  0x92, 0x11, 0xda, 0xca, 0x80, 0x5e, 0x92, 0x4c, 0xb5, 0xd8, 0xb5, 0x84,
  0x95, 0xe3, 0xef, 0xbc, 0x7e, 0x7d, 0x68, 0x74, 0x4c, 0x34, 0x1a, 0x50,
  0x6d, 0x2d, 0x5f, 0x1b, 0x0e, 0xbe, 0xf5, 0xb4, 0xf1, 0x32, 0x16, 0x44,
  0x24, 0x7a, 0x0e, 0x4b, 0xcd, 0xfa, 0xa5, 0x03, 0x95, 0x2e, 0x44, 0x65,
  0xa8, 0x74, 0xea, 0x17, 0xdd, 0x99, 0xbd, 0xcb, 0x02, 0x03, 0x01, 0x00,
  0x01, 0xa3, 0x50, 0x30, 0x4e, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e,
  0x04, 0x16, 0x04, 0x14, 0x1b, 0x8d, 0x06, 0xd9, 0x6b, 0xad, 0xee, 0x82,
  0x24, 0x26, 0x55, 0x9a, 0x1b, 0x03, 0x44, 0x92, 0x0a, 0x06, 0x92, 0x48,
  0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80,
  0x14, 0x1b, 0x8d, 0x06, 0xd9, 0x6b, 0xad, 0xee, 0x82, 0x24, 0x26, 0x55,
  0x9a, 0x1b, 0x03, 0x44, 0x92, 0x0a, 0x06, 0x92, 0x48, 0x30, 0x0c, 0x06,
  0x03, 0x55, 0x1d, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x70, 0xc7, 0x6a, 0x75, 0x27,
  0x14, 0xa0, 0x1c, 0xe0, 0xe0, 0x84, 0x7c, 0x6c, 0x16, 0xa9, 0x0d, 0x4d,
  0xb1, 0xc3, 0x87, 0x37, 0xf6, 0x86, 0x89, 0x6f, 0x73, 0xf0, 0x59, 0x9b,
  0x8c, 0xa4, 0x83, 0x10, 0x2d, 0xb7, 0x8b, 0xd0, 0x9a, 0x81, 0xe0, 0x5c,
  0xd7, 0x20, 0x6f, 0xdc, 0xfc, 0xc8, 0xa0, 0xc2, 0x8e, 0x54, 0xe6, 0xfb,
  0x61, 0x85, 0x37, 0x4b, 0x22, 0x47, 0x09, 0x95, 0x44, 0x12, 0x75, 0xf0,
  0xcf, 0x0b, 0x90, 0x48, 0xb0, 0x02, 0x4c, 0xef, 0x3f, 0xde, 0x6a, 0xfd,
  0xb1, 0x8b, 0x88, 0xd7, 0x84, 0xe5, 0x34, 0x02, 0x96, 0x0a, 0x3f, 0xa8,
  0x8c, 0xbd, 0x1a, 0xd8, 0xf7, 0xf9, 0xe5, 0x49, 0x87, 0xd0, 0x20, 0x4f,
  0xd8, 0xcd, 0xc0, 0xb9, 0x11, 0x2a, 0xd9, 0x0f, 0x75, 0xa6, 0xee, 0x76,
  0x15, 0x9f, 0x12, 0x50, 0x68, 0x4c, 0xc0, 0x05, 0x46, 0x8d, 0xdd, 0x93,
  0x74, 0x31, 0x82, 0x20, 0x37, 0x24, 0x58, 0xb2, 0x88, 0x9b, 0x21, 0xc1,
  0x48, 0xc4, 0x8d, 0x68, 0x3b, 0x91, 0x2c, 0x34, 0xcb, 0x94, 0xd0, 0xbc,
  0xe3, 0x05, 0x24, 0x05, 0xcc, 0xea, 0x05, 0xb1, 0x52, 0x74, 0x4a, 0x23,
  0x65, 0xc4, 0x40, 0x04, 0x86, 0xb1, 0x80, 0x61, 0x97, 0xdc, 0x94, 0x16,
  0x4e, 0x63, 0x31, 0x72, 0x4e, 0x45, 0xe8, 0x3e, 0x3b, 0xb6, 0x99, 0xae,
  0xd8, 0x91, 0x25, 0x3d, 0x62, 0x92, 0x6d, 0x72, 0x01, 0x2c, 0xca, 0x67,
  0x0a, 0xec, 0x00, 0xeb, 0x10, 0xff, 0x6d, 0xac, 0x89, 0x19, 0x2c, 0xb7,
  0xb3, 0xa5, 0xf7, 0xa1, 0x4a, 0xc3, 0xc1, 0xdd, 0xaf, 0xb5, 0x1a, 0x16,
  0x44, 0xdc, 0xa8, 0xb5, 0xca, 0xd0, 0x30, 0xaa, 0x7e, 0x73, 0xd5, 0x2e,
  0x65, 0xd6, 0xf9, 0xbf, 0x5f, 0xda, 0x6f, 0x13, 0xe9, 0xd7, 0x12, 0x6c,
  0x3a, 0x6c, 0x50, 0x26, 0x78, 0x6e, 0xc6, 0xeb, 0x75, 0xe1, 0x3c
};
static volatile unsigned int ca_cert_der_len_test = 971;
#endif


#ifdef CONFIG_NET_AT_TLS
#ifdef CONFIG_CRYPTO_MBEDTLS
int aic_at_mbedtls_config_init(void)
{
    int i;

    for (i = 0; i < SSL_CTX_ID_MAX; i++) {
        aos_memset(&sslconf_list[i], 0, sizeof(struct mbedtls_config));
        sslconf_list[i].ssl_version = SSL_VERSION_ALL;
        sslconf_list[i].ciphersuite = SSL_CIPHERSUITE_DEFAULT;
        sslconf_list[i].seclevel = SSL_SECLEVEL_DEFAULT;
        sslconf_list[i].ignore_ltime = SSL_IGNORE_TIME;
        sslconf_list[i].negotiate_time = SSL_NEGOTIATE_TIME;
        sslconf_list[i].sni = SSL_SNI_DEFAULT;
        sslconf_list[i].closetimeout = SSL_IGNORE_TIME;
    }

    return AIC_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_at_mbedtls_config_init);

void aic_at_mbedtls_config_reset(int inedx)
{
    aos_memset(&sslconf_list[inedx], 0, sizeof(struct mbedtls_config));
    sslconf_list[inedx].ssl_version = SSL_VERSION_ALL;
    sslconf_list[inedx].ciphersuite = SSL_CIPHERSUITE_DEFAULT;
    sslconf_list[inedx].seclevel = SSL_SECLEVEL_DEFAULT;
    sslconf_list[inedx].ignore_ltime = SSL_IGNORE_TIME;
    sslconf_list[inedx].negotiate_time = SSL_NEGOTIATE_TIME;
    sslconf_list[inedx].sni = SSL_SNI_DEFAULT;
    sslconf_list[inedx].closetimeout = SSL_IGNORE_TIME;
}

static int aic_ssl_net_connect(int family, struct sockaddr *addr,
                        struct sockaddr *srcaddr, int linkid)
{
    int sock, ret, addrlen;
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
    }

    return sock;
}

int aic_at_create_mbedtls_socket(int family, struct sockaddr *addr,
                                 struct sockaddr *srcaddr, int linkid)
{
    int sock, i, ret, count = 0;
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
    server_fd = &sock_list[i].sockfd;
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

    if (sslconf_list[i].seclevel >= 1 && sslconf_list[i].cacert){
        ret = mbedtls_x509_crt_parse(cacert, sslconf_list[i].cacert,
                                     (size_t)sslconf_list[i].cacert_len);
        if( ret < 0 ) {
            aicx_error(0x0831bc, "x", "[AT_TLS] mbedtls_x509_crt_parse failed -0x%x\n",
                       (unsigned int) -ret);
            goto exit5;
        }
    }

    if (sslconf_list[i].seclevel > 1 && sslconf_list[i].client_cert &&
        sslconf_list[i].client_key) {
        ret = mbedtls_x509_crt_parse(cacert, sslconf_list[i].client_cert,
                                     (size_t)sslconf_list[i].client_cert_len);
        if( ret < 0 ) {
            aicx_error(0x0831bc, "x", "[AT_TLS] mbedtls_x509_crt_parse failed -0x%x\n",
                       (unsigned int) -ret);
            goto exit5;
        }

        pkey = aos_zalloc(sizeof(mbedtls_pk_context));
        if (!pkey)
            goto exit5;
        mbedtls_pk_init(pkey);
        ret = mbedtls_pk_parse_key(pkey, (const unsigned char *)sslconf_list[i].client_key,
                                   sslconf_list[i].client_key_len, NULL, 0,
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

    mbedtls_ssl_conf_authmode(conf, sslconf_list[i].seclevel);
    mbedtls_ssl_conf_ca_chain(conf, cacert, NULL);
    if (sslconf_list[i].seclevel > 1) {
        mbedtls_ssl_conf_own_cert(conf, cacert->next, pkey);
    }
    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);
    //mbedtls_ssl_conf_dbg(conf, my_debug, NULL);

    if (sslconf_list[i].ciphersuite != 0xFFFF) {
        ssl_ciphersuites[0] = (int)sslconf_list[i].ciphersuite;
        mbedtls_ssl_conf_ciphersuites(conf, (const int *)ssl_ciphersuites);
    }

    ret = mbedtls_ssl_setup(ssl, conf);
    if (ret != 0) {
        aicx_error(0x12a8fe, "d", "[AT_TLS] mbedtls_ssl_setup failed %d\n", ret);
        goto exit5;
    }

    mbedtls_ssl_set_bio(ssl, server_fd, mbedtls_net_send,
                        mbedtls_net_recv, NULL );

    while (1) {
        ret = mbedtls_ssl_handshake(ssl);
        if(ret == 0)
            break;

        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            aicx_error(0x149cb8, "d", "mbedtls_ssl_handshake failed %d\n",
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

    ret = fcntl(sock, F_SETFL, O_NONBLOCK);
    if (ret == NX_SOC_ERROR) {
        aicx_info(0x0bedcc, "d", "[AT_TLS] set tls socket nonblock fail errno %d\r\n", errno);
        mbedtls_ssl_close_notify(ssl);
        goto exit5;
    }

    if (sock_list[i].socket_id == 0) {
        if (aic_tcpip_at_cfg.cipmux == 0 ) {
            sock_list[i].link_id = 0;
        } else {
            sock_list[i].link_id = linkid;
        }
        sock_list[i].socket_id = sock;
        sock_list[i].family = family;
        sock_list[i].ssl = ssl;
        sock_list[i].key = pkey;
        if (family == AF_INET)
            sock_list[i].socket_type = SOCK_TLS;
        else
            sock_list[i].socket_type = SOCK_TLSv6;
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

int aic_at_close_mbedtls_socket(int socket_fd)
{
    int index;
    mbedtls_ssl_context *ssl;
    mbedtls_ssl_config *conf;
    mbedtls_ctr_drbg_context *ctr_drbg;

    index = aic_at_find_index_by_socket(socket_fd, AT_TCPIP);
    if (index < 0 || index >= AT_SOCKET_MAX_CONN_NUM) {
        aicx_warn(0x1069f3, "x", "[AT_TLS] get index by socket fail 0x%x\r\n", index);
        return AIC_ENODEV;
    }

    ssl = sock_list[index].ssl;
    if (!ssl)
        goto exit;

    mbedtls_ssl_close_notify(ssl);
    conf = (mbedtls_ssl_config *)ssl->MBEDTLS_PRIVATE(conf);
    if (!conf)
        goto exit;

    ctr_drbg = (mbedtls_ctr_drbg_context *)conf->MBEDTLS_PRIVATE(p_rng);
    if (ctr_drbg) {
        mbedtls_entropy_free(ctr_drbg->MBEDTLS_PRIVATE(p_entropy));
        if (ctr_drbg->MBEDTLS_PRIVATE(p_entropy)) {
            aos_free(ctr_drbg->MBEDTLS_PRIVATE(p_entropy));
            ctr_drbg->MBEDTLS_PRIVATE(p_entropy) = NULL;
        }
        mbedtls_ctr_drbg_free(ctr_drbg);
    }
    if (conf->MBEDTLS_PRIVATE(authmode) == MBEDTLS_SSL_VERIFY_REQUIRED) {
        mbedtls_pk_free(sock_list[index].key);
        if (sock_list[index].key) {
            aos_free(sock_list[index].key);
            sock_list[index].key = NULL;
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
        sock_list[index].ssl = NULL;
    }

exit:
    mbedtls_net_free(&sock_list[index].sockfd);
    aic_at_mbedtls_config_reset(index);
    return AIC_SUCCESS;
}
#endif

#ifdef CONFIG_NET_SECURE
int aic_at_create_tls_socket(int family, struct sockaddr *addr,
                             int addrlen, int linkid)
{
    int sock, i, ret;
    struct timeval timeout;

    sock = aic_create_tls_socket(family, SOCK_STREAM, NX_PROTOCOL_TCP);
    if (sock == NX_SOC_ERROR) {
        aicx_error(0x185158, "d", "[AT_TLS]: --create tls socket error, errno %d\n", errno);
        return AIC_EFAULT;
    } else {
        aicx_info(0x0687fd, "d", "[AT_TLS]: --create socket success, socket %d\n", sock);
    }

    timeout.tv_sec = AT_SOCKET_SEND_TIMEOUT;
    timeout.tv_usec = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret)
        aicx_info(0x19eb7e, "x", "[AT_TLS] set tls send timeout fail 0x%x\r\n", ret);


    i = linkid;
    if (sock_list[i].ca_cert_der)
        aic_tls_ca_certificate_add(sock, sock_list[i].ca_cert_der,
                                   sock_list[i].ca_cert_der_len);

    ret = fcntl(sock, F_SETFL, O_NONBLOCK);
    if (ret == NX_SOC_ERROR) {
        aic_close_tls_socket(sock);
        return AIC_EFAULT;
    }

    for (int i = 0; i <= AT_SCKET_CONN_COUNT_MAX; i++) {
        ret = anet_connect(sock, addr, addrlen);
        if (ret != NX_SOC_ERROR) {
            aicx_info(0x1254e7, "", "[AT_TLS]:--connect ok\n");
            break;
        } else {
            aicx_error(0x1bf1bf, "d", "[AT_TLS]:--connect fail, errno %d\n", errno);
            if (errno == EISCONN)
                break;

            if (i == AT_SCKET_CONN_COUNT_MAX) {
                aic_close_tls_socket(sock);
                return AIC_EFAULT;
            }

            if (errno == EINPROGRESS || errno == EALREADY)
                aos_sleep(2 * NX_IP_PERIODIC_RATE);
        }
    }

    if (sock_list[i].socket_id == 0) {
        if (aic_tcpip_at_cfg.cipmux == 0 ) {
            sock_list[i].link_id = 0;
        } else {
            sock_list[i].link_id = linkid;
        }
        sock_list[i].socket_id = sock;
        sock_list[i].family = family;
        if (family == AF_INET)
            sock_list[i].socket_type = SOCK_TLS;
        else
            sock_list[i].socket_type = SOCK_TLSv6;
    }

    return sock;
}
#endif

#ifdef AT_TLS_TEST
u32 handle_cipsslca_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int status, index, link_id, i;
    u16 cnt;
    ulong *paras_arr;
    int fd;
    FS_FILE_INFO *file_info;
    ssize_t rsize;

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);
    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSSLCA=",
                                                 &cnt);
        index = 1;
        if (cnt != 2) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    } else {
        if (sock_list[0].socket_id != 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT);
            return AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT;
        }
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSSLCA=",
                                                 &cnt);
        index = 0;
        link_id = AT_SOCK_LINKID_DEFAULT;
        if (cnt != 1) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    }

    if ( !paras_arr ) {
        aicx_error(0x023651, "", "[AT_TLS]: parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1) {
        link_id = atoi((u8 *)paras_arr[0]);
        if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_PARAM );
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_INVAL_AT_PARAM;
        }

        if (sock_list[link_id].socket_id != 0) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT;
        }
    }

#ifdef AT_TLS_TEST
    fd = fs_open((u8 *)paras_arr[index], FS_O_WRONLY | FS_O_CREATE);
    if (fd < 0) {
        aicx_error(0x1bddb4, "s", "[AT_TLS]: open CA file fail, file patch %s!\n",
                   (u8 *)paras_arr[index]);
        goto AT_TLS_ERROR;
    }

    rsize = fs_write(fd, ca_cert_der_test, ca_cert_der_len_test);
    if (rsize != ca_cert_der_len_test) {
        aicx_error(0x0d1801, "", "[AT_TLS]: write CA file fail\n");
        goto AT_TLS_ERROR;
    }

    fs_flush(fd);
    fs_close(fd);
#endif
    fd = fs_open((u8 *)paras_arr[index], FS_O_RDONLY);
    if (fd < 0) {
        aicx_error(0x1bddb4, "s", "[AT_TLS]: open CA file fail, file patch %s!\n",
                   (u8 *)paras_arr[index]);
        goto AT_TLS_ERROR;
    }

    file_info = aos_malloc(sizeof(FS_FILE_INFO));
    status = fs_getinfo((u8 *)paras_arr[index], file_info);
    if (status != AIC_SUCCESS) {
        aicx_error(0x0899cf, "", "[AT_TLS]: get CA file info fail!\n");
        aos_free(file_info);
        goto AT_TLS_ERROR;
    }

    i = link_id;
    sock_list[i].ca_cert_der = aos_zalloc(file_info->size);
    if (sock_list[i].ca_cert_der == NULL) {
        aicx_error(0x0e5b74, "", "[AT_TLS]: CA alloc fail\n");
        aos_free(file_info);
        goto AT_TLS_ERROR;
    }
    sock_list[i].ca_cert_der_len = file_info->size;
    rsize = fs_read(fd, sock_list[i].ca_cert_der,
                    sock_list[i].ca_cert_der_len);
    if (rsize <= 0) {
        aicx_error(0x1e2df1, "", "[AT_TLS]: read CA file fail\n");
        aos_free(file_info);
        aos_free(sock_list[i].ca_cert_der);
        sock_list[i].ca_cert_der = AIC_NULL;
        goto AT_TLS_ERROR;
    }

    status = fs_close(fd);
    if (status != AIC_SUCCESS) {
        aicx_error(0x0e6924, "", "[AT_TLS]: close CA file fail\n");
        aos_free(file_info);
        aos_free(sock_list[i].ca_cert_der);
        goto AT_TLS_ERROR;
    }

    aic_reply_ok_resp(at_req_resp_ptr);

    aos_free(file_info);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

AT_TLS_ERROR:
    aic_reply_error_resp(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
int handle_cert_file(u8 *file, int ssl_ctx_id, int type)
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
        sslconf_list[ssl_ctx_id].cacert = cert;
        sslconf_list[ssl_ctx_id].cacert_len = file_info->size + 1;
    } else if (type == SSL_CLIENT_CERT) {
        sslconf_list[ssl_ctx_id].client_cert = cert;
        sslconf_list[ssl_ctx_id].client_cert_len = file_info->size + 1;
    } else if (type == SSL_CLIENT_KEY) {
        sslconf_list[ssl_ctx_id].client_key = cert;
        sslconf_list[ssl_ctx_id].client_key_len = file_info->size + 1;
    } else
        return AIC_ENOTSUP;

    aos_free(file_info);
    return AIC_SUCCESS;
#else
    return AIC_EFAULT;
#endif
}

u32 handle_cipsslcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ret, id, str_len, num = 0;
    u8 paras[8] = {'\0'};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+CIPSSLCFG=",
                                             &cnt);
    if (!paras_arr || cnt < 2 || cnt > 3) {
        aicx_error(0x023651, "", "[AT_TLS]: parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (strlen((u8 *)paras_arr[1])) {
        id = atoi((u8 *)paras_arr[1]);
        if (id > SSL_CTX_ID_MAX) {
            goto AT_TLS_EXIT;
        }
        sslconf_list[id].ssl_context_id = id;
    } else {
        goto AT_TLS_EXIT;
    }

    str_len = strlen((u8*)paras_arr[0]);
    if (strncasecmp((u8*)paras_arr[0], "sslversion", 10) == 0 &&
        str_len == 10) {
        if (cnt > 2) {
            if (strlen((u8 *)paras_arr[2]))
                sslconf_list[id].ssl_version = atoi((u8*)paras_arr[2]);

            if (sslconf_list[id].ssl_version > 4) {
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG:\"sslversion\",%d,%d",
                     id, sslconf_list[id].ssl_version);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ciphersuite", 11) == 0 &&
               str_len == 11) {
        if (cnt > 2) {
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
                sslconf_list[id].ciphersuite = num;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG:\"ciphersuite\",%d,0x%x",
                     id, sslconf_list[id].ciphersuite);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "cacert", 6) == 0 &&
               str_len == 6) {
        if (cnt > 2) {
            ret = handle_cert_file((u8*)paras_arr[2], id, SSL_CA_CERT);
            if (ret != AIC_SUCCESS)
                goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "clientcert", 10) == 0 &&
               str_len == 10) {
        if (cnt > 2) {
            ret = handle_cert_file((u8*)paras_arr[2], id, SSL_CLIENT_CERT);
            if (ret != AIC_SUCCESS)
                goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "clientkey", 9) == 0 &&
               str_len == 9) {
        if (cnt > 2) {
            ret = handle_cert_file((u8*)paras_arr[2], id, SSL_CLIENT_KEY);
            if (ret != AIC_SUCCESS)
                goto AT_TLS_EXIT;
        }
    } else if (strncasecmp((u8*)paras_arr[0], "seclevel", 8) == 0 &&
               str_len == 8) {
        if (cnt > 2) {
            if (strlen((u8 *)paras_arr[2]))
                sslconf_list[id].seclevel = atoi((u8*)paras_arr[2]);

            if (sslconf_list[id].seclevel > 2) {
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG:\"seclevel\",%d,%d",
                     id, sslconf_list[id].seclevel);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "ignorelocaltime", 15) == 0 &&
               str_len == 15) {
        if (cnt > 2) {
            if (strlen((u8 *)paras_arr[2]))
                sslconf_list[id].ignore_ltime = atoi((u8*)paras_arr[2]);

            if (sslconf_list[id].ignore_ltime > 1) {
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG:\"ignorelocaltime\",%d,%d",
                     id, sslconf_list[id].ignore_ltime);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "negotiatetime", 13) == 0 &&
               str_len == 13) {
        if (cnt > 2) {
            if (strlen((u8 *)paras_arr[2]))
                sslconf_list[id].negotiate_time = atoi((u8*)paras_arr[2]);

            if (sslconf_list[id].negotiate_time < 10 || 
                sslconf_list[id].negotiate_time > SSL_NEGOTIATE_TIME) {
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG:\"negotiatetime\",%d,%d",
                     id, sslconf_list[id].negotiate_time);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "sni", 3) == 0 && str_len == 3) {
        if (cnt > 2) {
            if (strlen((u8 *)paras_arr[2]))
                sslconf_list[id].sni = atoi((u8*)paras_arr[2]);

            if (sslconf_list[id].sni != 0 && sslconf_list[id].sni != 1) {
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG:\"sni\",%d,%d",
                     id, sslconf_list[id].sni);
        }
    } else if (strncasecmp((u8*)paras_arr[0], "closetimemode", 13) == 0 &&
               str_len == 13) {
        if (cnt > 2) {
            if (strlen((u8 *)paras_arr[2]))
                sslconf_list[id].closetimeout = atoi((u8*)paras_arr[2]);

            if (sslconf_list[id].closetimeout > 1) {
                goto AT_TLS_EXIT;
            }
        } else {
            snprintf(str, sizeof(str), "+QSSLCFG:\"closetimemode\",%d,%d",
                     id, sslconf_list[id].closetimeout);
        }
    } else {
        goto AT_TLS_EXIT;
    }

    if (str[0] != '\0')
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

AT_TLS_EXIT:
    aic_reply_error_resp(at_req_resp_ptr,
                            AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;

}
#endif

#ifdef CONFIG_NET_SECURE
/**
 * handle_cipsslcconf_set_cmd() - Set SSL configuration.
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
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 handle_cipsslcconf_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index, sock, auth_type, link_id = 0;
    NX_SECURE_TLS_SESSION *tls_session;
    struct aic_tls_session *aic_tls_session;
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);

    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSSLCCONF=",
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
                                                 "AT+CIPSSLCCONF=",
                                                 &cnt);
        index = 0;
        link_id = AT_SOCK_LINKID_DEFAULT;
        if (cnt != 1) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    }

    if ( !paras_arr ) {
        aicx_error(0x023651, "", "[AT_TLS]: parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1)
        link_id = atoi((u8 *)paras_arr[0]);

    if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM );
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    auth_type = atoi((u8 *)paras_arr[index]);
    if (auth_type > 3) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
    if (sock == 0) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    aic_tls_session = aic_get_tls_session_socket_id(sock);
    if (!aic_tls_session) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }
    tls_session = aic_tls_session->secure_tls_session;
    if (tls_session) {
        tls_auth_type = auth_type;
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsslcconf_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_SECURE_TLS_SESSION *tls_session;
    struct aic_tls_session *aic_tls_session;
    int link_id, sock, i;
    char *auth;
    NX_TCP_SOCKET *tls_tcp_socket;
    char str[BUFFER_SIZE] = {0};

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        sock = sock_list[i].socket_id;
        if (sock == 0)
            continue;

        aic_tls_session = aic_get_tls_session_socket_id(sock);
        if (!aic_tls_session)
            continue;

        tls_session = aic_tls_session->secure_tls_session;
        if (tls_session == AIC_NULL && i == AT_SOCKET_MAX_CONN_NUM) {
            aicx_info(0x1c7061, "", "[AT_TLS]: no fund tls session\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            return AIC_TCPIP_AT_ERR;
        }

        if (tls_session == AIC_NULL)
            continue;

        auth = (char *)tls_session->nx_secure_tls_session_ciphersuite->nx_secure_tls_public_auth;
        link_id = sock_list[i].link_id;
        if (aic_tcpip_at_cfg.cipmux == 1) {
            tls_tcp_socket = tls_session->nx_secure_tls_tcp_socket;
            sock = (int)tls_tcp_socket->nx_tcp_socket_reserved_ptr
                   + NX_BSD_SOCKFD_START;
            link_id = aic_at_find_linkid_by_socket(sock, AT_TCPIP);
            snprintf(str, sizeof(str), "+CIPSSLCCONF: %d,%s", link_id, auth);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else {
            snprintf(str, sizeof(str), "+CIPSSLCCONF: %s", auth);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            return AIC_TCPIP_AT_OK;
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsslccn_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index,i;
    int link_id = 0;
    NX_SECURE_TLS_SESSION *tls_session = AIC_NULL;
    struct aic_tls_session *aic_tls_session;
    NX_SECURE_TLS_CREDENTIALS credentials;
    NX_SECURE_X509_CERT *certificate = AIC_NULL;
    NX_SECURE_X509_DISTINGUISHED_NAME common_name;
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);

    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSSLCCN=",
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
                                                 "AT+CIPSSLCCN=",
                                                 &cnt);
        index = 0;
        link_id = AT_SOCK_LINKID_DEFAULT;
        if (cnt != 1) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    }

    if ( !paras_arr ) {
        aicx_error(0x023651, "", "[AT_TLS]: parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (strlen((u8 *)paras_arr[index]) > 64) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1)
        link_id = atoi((u8 *)paras_arr[0]);

    if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM );
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (strlen((u8 *)paras_arr[index]) > SSL_COMMON_NAME_LEN) {
        aicx_info(0x16cb95, "", "[AT_TLS]: common name length too long\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        if(sock_list[i].link_id != link_id)
            continue;

        aic_tls_session = aic_get_tls_session_socket_id(sock_list[i].socket_id);
        if (!aic_tls_session) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        sock_list[i].common_name = aos_zalloc(strlen((u8 *)paras_arr[index]));
        AIC_ASSERT(sock_list[i].common_name);
        strncpy(sock_list[i].common_name, (u8 *)paras_arr[index],
                strlen((u8 *)paras_arr[index]));
        tls_session = aic_tls_session->secure_tls_session;
        break;
    }

    credentials = tls_session->nx_secure_tls_credentials;
    certificate = credentials.nx_secure_tls_active_certificate;
    common_name = certificate->nx_secure_x509_distinguished_name;
    common_name.nx_secure_x509_common_name =
        sock_list[i - 1].common_name;
    common_name.nx_secure_x509_common_name_length =
        strlen((u8 *)paras_arr[index]);

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsslccn_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_SECURE_TLS_SESSION *tls_session = AIC_NULL;
    struct aic_tls_session *aic_tls_session;
    int link_id, sock, i;
    NX_SECURE_X509_DISTINGUISHED_NAME common_name;
    char name[SSL_COMMON_NAME_LEN];
    NX_SECURE_TLS_CREDENTIALS credentials;
    NX_SECURE_X509_CERT *certificate;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        sock = sock_list[i].socket_id;
        if (sock == 0)
            continue;

        aic_tls_session = aic_get_tls_session_socket_id(sock);
        if (!aic_tls_session)
            continue;

        tls_session = aic_tls_session->secure_tls_session;
        if (tls_session == AIC_NULL && i == AT_SOCKET_MAX_CONN_NUM) {
            aicx_info(0x1c7061, "", "[AT_TLS]: no fund tls session\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            return AIC_TCPIP_AT_ERR;
        }

        if (tls_session == AIC_NULL)
            continue;

        credentials = tls_session->nx_secure_tls_credentials;
        certificate = credentials.nx_secure_tls_active_certificate;
        common_name = certificate->nx_secure_x509_distinguished_name;
        aos_memcpy(name, common_name.nx_secure_x509_common_name,
                   common_name.nx_secure_x509_common_name_length);
        if (aic_tcpip_at_cfg.cipmux == 1) {
            link_id = aic_at_find_linkid_by_socket(sock, AT_TCPIP);
            sprintf(str, "+CIPSSLCCN: %d,%s", link_id, name);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else {
            sprintf(str, "+CIPSSLCCN: %s", name);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            return AIC_TCPIP_AT_OK;
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);

    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsslcsni_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index;
    int sock, link_id = 0;
    NX_SECURE_TLS_SESSION *tls_session;
    struct aic_tls_session *aic_tls_session;
    NX_SECURE_X509_DNS_NAME *dns_name;
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);

    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSSLCSNI=",
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
                                                 "AT+CIPSSLCSNI=",
                                                 &cnt);
        index = 0;
        link_id = AT_SOCK_LINKID_DEFAULT;
        if (cnt != 1) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    }

    if ( !paras_arr ) {
        aicx_error(0x023651, "", "[AT_TLS]: parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (strlen((u8 *)paras_arr[index]) > 64) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1)
        link_id = atoi((u8 *)paras_arr[0]);

    if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM );
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    sock = aic_at_find_socket_by_linkid(link_id, AT_TCPIP);
    if (sock == 0){
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
    }

    aic_tls_session = aic_get_tls_session_socket_id(sock);
    if (!aic_tls_session) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;

    }
    tls_session = aic_tls_session->secure_tls_session;
    dns_name = tls_session->nx_secure_tls_sni_extension_server_name;

    strncpy((char *)&dns_name->nx_secure_x509_dns_name,
            (u8 *)paras_arr[index], strlen((u8 *)paras_arr[index]));
    dns_name->nx_secure_x509_dns_name_length = strlen((u8 *)paras_arr[index]);

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);

    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsslcsni_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_SECURE_TLS_SESSION *tls_session;
    struct aic_tls_session *aic_tls_session;
    int link_id, sock, i;
    char *sni;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        sock = sock_list[i].socket_id;

        if (sock == 0)
            continue;

        aic_tls_session = aic_get_tls_session_socket_id(sock);
        if (!aic_tls_session)
            continue;

        tls_session = aic_tls_session->secure_tls_session;
        if (tls_session == AIC_NULL && i == AT_SOCKET_MAX_CONN_NUM) {
            aicx_info(0x1c7061, "", "[AT_TLS]: no fund tls session\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_INVAL_AT_CMD);
            return AIC_TCPIP_ERR_INVAL_AT_CMD;
        }

        if (tls_session == AIC_NULL)
            continue;

        link_id = sock_list[i].link_id;

        sni = (char *)tls_session->nx_secure_tls_sni_extension_server_name;
        if (aic_tcpip_at_cfg.cipmux == 1) {
            snprintf(str, sizeof(str), "+CIPSSLCSNI: %d,%s", link_id, sni);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else {
            snprintf(str, sizeof(str), "+CIPSSLCSNI: %s", sni);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            return AIC_TCPIP_AT_OK;
        }
    }

    aic_reply_ok_resp(at_req_resp_ptr);

    return AIC_TCPIP_AT_OK;
}

#if 0
u32 handle_cipsslcalpn_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    return;
}

u32 handle_cipsslcalpn_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    return;
}
#endif

u32 handle_cipsslcpsk_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    int index, i, link_id = 0;
    NX_SECURE_TLS_SESSION *tls_session;
    struct aic_tls_session *aic_tls_session;
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || (NX_SECURE_TLS_TLS_1_3_ENABLED)
    NX_SECURE_TLS_PSK_STORE tls_client_psk;
#endif
    u16 cnt;
    ulong *paras_arr;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);

    if (aic_tcpip_at_cfg.cipmux == 1) {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSSLCPSK=",
                                                 &cnt);
        index = 1;
        if (cnt != 3) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    } else {
        paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                                 "AT+CIPSSLCPSK=",
                                                 &cnt);
        index = 0;
        link_id = AT_SOCK_LINKID_DEFAULT;
        if (cnt != 2) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_PARSE_AT_PARAM);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_PARSE_AT_PARAM;
        }
    }

    if ( !paras_arr ) {
        aicx_error(0x023651, "", "[AT_TLS]: parameter parse error\n");
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (strlen((u8 *)paras_arr[index]) > NX_SECURE_TLS_MAX_PSK_ID_SIZE ||
        strlen((u8 *)paras_arr[index + 1]) > NX_SECURE_TLS_MAX_PSK_ID_SIZE) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (aic_tcpip_at_cfg.cipmux == 1)
        link_id = atoi((u8 *)paras_arr[0]);

    if (link_id >= AT_SOCKET_MAX_CONN_NUM) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM );
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        if(sock_list[i].link_id != link_id)
            continue;

        aic_tls_session = aic_get_tls_session_socket_id(sock_list[i].socket_id);
        if (!aic_tls_session) {
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            aic_free_paras_mem(paras_arr, cnt);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        tls_session = aic_tls_session->secure_tls_session;

        strcpy(str, AT_VERBOSE_RESULT_ERROR);
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || (NX_SECURE_TLS_TLS_1_3_ENABLED)
        tls_client_psk =
            tls_session->nx_secure_tls_credentials.nx_secure_tls_client_psk;
        strncpy(tls_client_psk.nx_secure_tls_psk_id, (u8 *)paras_arr[index],
                strlen((u8 *)paras_arr[index]));
        strncpy(tls_client_psk.nx_secure_tls_psk_id_hint,
            (u8 *)paras_arr[index + 1], strlen((u8 *)paras_arr[index + 1]));
        strcpy(str, AT_RESULT_OK);
#endif

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || (NX_SECURE_TLS_TLS_1_3_ENABLED)
        tls_client_psk =
            tls_session->nx_secure_tls_credentials.nx_secure_tls_psk_store[0];
        strncpy(tls_client_psk.nx_secure_tls_psk_id, (u8 *)paras_arr[index],
                strlen((u8 *)paras_arr[index]));
        strncpy(tls_client_psk.nx_secure_tls_psk_id_hint,
               (u8 *)paras_arr[index + 1], strlen((u8 *)paras_arr[index + 1]));
        strcpy(str, AT_RESULT_OK);
#endif
    }

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_free_paras_mem(paras_arr, cnt);

    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsslcpsk_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_SECURE_TLS_SESSION *tls_session;
    struct aic_tls_session *aic_tls_session;
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || (NX_SECURE_TLS_TLS_1_3_ENABLED)
    NX_SECURE_TLS_PSK_STORE tls_client_psk;
#endif
    int link_id, sock, i;
    char *psk = "\0";
    char *hint= "\0";
    u8 str[150] = {'\0'};

    aicx_info(0x1477cd, "s", "[AT_TLS]: --%s\n", at_req_resp_ptr->at_req);

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        sock = sock_list[i].socket_id;

        if (sock == 0)
            continue;

        aic_tls_session = aic_get_tls_session_socket_id(sock);
        if (!aic_tls_session)
            continue;

        tls_session = aic_tls_session->secure_tls_session;
        if (tls_session == AIC_NULL && i == AT_SOCKET_MAX_CONN_NUM) {
            aicx_info(0x1c7061, "", "[AT_TLS]: no fund tls session\n");
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_SOCKET_NOT_EXIST);
            return AIC_TCPIP_ERR_SOCKET_NOT_EXIST;
        }

        if (tls_session == AIC_NULL)
            continue;

        link_id = sock_list[i].link_id;
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || (NX_SECURE_TLS_TLS_1_3_ENABLED)
        tls_client_psk =
            tls_session->nx_secure_tls_credentials.nx_secure_tls_client_psk;
        strncpy(psk, tls_client_psk.nx_secure_tls_psk_id,
                NX_SECURE_TLS_MAX_PSK_ID_SIZE);
        strncpy(hint, tls_client_psk.nx_secure_tls_psk_id_hint,
                NX_SECURE_TLS_MAX_PSK_ID_SIZE);
#endif

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || (NX_SECURE_TLS_TLS_1_3_ENABLED)
        tls_client_psk =
            tls_session->nx_secure_tls_credentials.nx_secure_tls_psk_store[0];
        strncpy(psk, tls_client_psk.nx_secure_tls_psk_id,
                NX_SECURE_TLS_MAX_PSK_ID_SIZE);
        strncpy(hint, tls_client_psk.nx_secure_tls_psk_id_hint,
                NX_SECURE_TLS_MAX_PSK_ID_SIZE);

#endif

        if (aic_tcpip_at_cfg.cipmux == 1) {
            snprintf(str, sizeof(str), "+CIPSSLCPSK: %d,%s,%s",
                     link_id, psk, hint);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
        } else {
            snprintf(str, sizeof(str), "+CIPSSLCPSK: %s,%s", psk, hint);
            aic_transmit_resp_normal(at_req_resp_ptr, str);
            return AIC_TCPIP_AT_OK;
        }
    }
    aic_reply_ok_resp(at_req_resp_ptr);

    return AIC_TCPIP_AT_OK;
}
#endif
#endif

