/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_AT_TLS_H__
#define __AIC_AT_TLS_H__

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <nx_api.h>
#include <nxd_bsd.h>
#include <nx_ip.h>
#ifdef CONFIG_NET_SECURE
#include <nx_secure_tls.h>
#include <nx_secure_tls_api.h>
#include "aic_tls_api.h"
#endif
#include <aic_size.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_type.h>
#include "aic_at_tcpip_common.h"
#include "aic_net_api.h"
#include "aic_at_parse_utils.h"
#include "aic_at_resp_handle.h"
#include "aic_at_cmd_handle.h"

#define SSL_COMMON_NAME_LEN  64
#define SSL_CTX_ID_MAX  6
#define SSL_VERSION_ALL  0xFF
#define SSL_CIPHERSUITE_DEFAULT 0
#define SSL_IGNORE_TIME  1
#define SSL_NEGOTIATE_TIME  300
#define SSL_SECLEVEL_DEFAULT  0
#define SSL_SESSION_DEFAULT  1
#define SSL_CERTKEY_NAME_LEN  64
#define SSL_CERT_OR_KEY_LEN  1536
#define SSL_HANDSHARK_RETRANS_COUNT  10
#define SSL_CERT_OR_KEY_MAX  9

#if 0
#define SSL_RECV_EVENT_ALL  0xFFFF
#define SSL_RECV_EVENT_1    0x1
#define SSL_RECV_EVENT_2    0x2
#define SSL_RECV_EVENT_3    0x4
#define SSL_RECV_EVENT_4    0x8
#define SSL_RECV_EVENT_5    0x10
#define SSL_RECV_EVENT_6    0x20
#endif
struct mbedtls_config {
    int ssl_context_id;
    u32 ssl_version;
    u32 ciphersuite;
    u8  cacert_name[SSL_CERTKEY_NAME_LEN];
    u8 *cacert;
    int cacert_len;
    u8  client_cert_name[SSL_CERTKEY_NAME_LEN];
    u8 *client_cert;
    int client_cert_len;
    u8  client_key_name[SSL_CERTKEY_NAME_LEN];
    u8 *client_key;
    int client_key_len;
#if 0
    u8  pskid[SSL_CERTKEY_NAME_LEN];
    u8 *psk;
    int psk_len;
#endif
    u32 seclevel;
    u32 ignore_ltime;
    u32 ignore_verify;
    u32 negotiate_time;
    u8 session;
    u8 encoding;
};

struct ssl_state {
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

enum ssl_cert_type {
    SSL_CA_CERT = 0,
    SSL_CLIENT_CERT,
    SSL_CLIENT_KEY
};

enum SSL_AT_ERROR_CODE {
    SSL_INVALID_PARAMETER = 50,
    SSL_UNKNOW_ERR = 750,
    SSL_INIT_FAIL,
    SSL_SERVER_CERT_CHECK_FAIL,
    SSL_TIMEOUT,
    SSL_FAIL,
    SSL_CERT_OR_KEY_UNKNOW_ERR = 760,
    SSL_CERT_OR_KEY_INVALID,
    SSL_CERT_OR_KEY_NOT_EXIST,
    SSL_CERT_OR_KEY_ALREADY_EXIST,
    SSL_CERT_OR_KEY_WRITE_ERR,
    SSL_CERT_OR_KEY_IS_WRITING,
    SSL_CERT_OR_KEY_READ_ERR,
    SSL_CERT_OR_KEY_DELETE_ERR,
    SSL_CERT_OR_KEY_TOO_LARGE,
    SSL_CERT_OR_KEY_LOAD_FAIL
};

struct cert_info {
    char name[96];
#ifdef CONFIG_FS
    int file;
#else
    int id;
#endif
    int length;
    int remain_length;
    int offset;
    u8 *buffer;
    u8 no_first;
};

struct ssl_cert_key_info {
    u8  name[SSL_CERTKEY_NAME_LEN];
    u32 len;
    u8  type;
};

struct ssl_psk_list{
    u8 pskid[SSL_CERTKEY_NAME_LEN];
    u8 *psk;
};

struct ssl_cert_key{
    char name[SSL_CERTKEY_NAME_LEN];
    int  len;
    char *data;
    struct ssl_cert_key *prev;
    struct ssl_cert_key *next;
};

enum verify_alg {
    MD5 = 0,
    SHA,
    SHA256,
    CRC,
    ALGO_MAX,
};

#ifdef CONFIG_CRYPTO_MBEDTLS
int aic_ssl_net_connect(int family, struct sockaddr *addr,
                        struct sockaddr *srcaddr, int linkid);

int aic_mat_create_mbedtls_socket(/*int family, struct sockaddr *addr,
                                  struct sockaddr *srcaddr, */int linkid);
int aic_mat_mbedtls_socket_recv(int index, int len,
                                AT_REQ_RESP *at_req_resp_ptr);
int aic_mat_close_mbedtls_socket(int socket_fd);
#endif
void aic_at_ssl_socket_recv(int index);
#endif

