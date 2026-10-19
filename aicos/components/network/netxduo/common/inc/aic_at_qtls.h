/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_AT_QTLS_H__
#define __AIC_AT_QTLS_H__

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <nx_api.h>
#include <nx_ip.h>
#include <nxd_bsd.h>
#include <nxd_dns.h>
#include <aic_errno.h>
#include <aic_type.h>
#include "aic_at_tcpip_common.h"
#include "aic_at_resp_handle.h"

#define CONFIG_NET_AT_QTLS

#define SSL_COMMON_NAME_LEN  64
#define SSL_CTX_ID_MAX  10
#define SSL_VERSION_ALL  4
#define SSL_CIPHERSUITE_DEFAULT 0xFFFF
#define SSL_IGNORE_TIME  1
#define SSL_NEGOTIATE_TIME  300
#define SSL_SECLEVEL_DEFAULT  0
#define SSL_SNI_DEFAULT  0
#define SSL_CONECT_TIMEOUT 20
#define SSL_HANDSHARK_RETRANS_COUNT 10

#define SSL_RECV_EVENT_ALL  0xFFFF
#define SSL_RECV_EVENT      0x1
#define SSL_CLOSE_EVENT     0x2


struct mbedtls_config {
    int ssl_context_id;
    u32 ssl_version;
    u32 ciphersuite;
    u8  cacert_path[128];
    u8 *cacert;
    int cacert_len;
    u8  client_cert_path[128];
    u8 *client_cert;
    int client_cert_len;
    u8  client_key_path[128];
    u8 *client_key;
    int client_key_len;
    u32 seclevel;
    u32 ignore_ltime;
    u32 negotiate_time;
    u32 closetimeout;
    u8 sni;
};

struct ssl_state{
    u8 addr[ADDR_BUFFERSIZE_IPV6];
    u16 remote_port;
    u16 local_port;
    int state;
    int pdp_cid;
    int server_id;
    int mode;
    u8 at_port[AT_URC_PORT_NAME_LEN];
    int ctx_id;
};

enum ssl_cert_type{
    SSL_CA_CERT = 0,
    SSL_CLIENT_CERT,
    SSL_CLIENT_KEY
};

#ifdef CONFIG_CRYPTO_MBEDTLS
int aic_qat_create_mbedtls_socket(int family, struct sockaddr *addr,
                                  struct sockaddr *srcaddr, int linkid);
int aic_qat_close_mbedtls_socket(int socket_fd);
#endif

#endif

