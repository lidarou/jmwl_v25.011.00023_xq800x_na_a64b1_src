/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_AT_TCPIP_COMMON_H__
#define __AIC_AT_TCPIP_COMMON_H__

#include "aic_osa.h"
#include "nxd_bsd.h"
#include "aic_general_at_handle.h"
#include "aic_at_cmd_handle.h"

#ifdef CONFIG_CRYPTO_MBEDTLS
#ifdef CONFIG_CRYPTO_MBEDTLS_3
#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#include "mbedtls/net_sockets.h"
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS_2
#include "mbedtls/net.h"
#include "mbedtls/md5.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#endif
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#endif

#ifdef CONFIG_GX_AT_V2
#define RESP_IP_CFG "+GIPCFG: "
#define RESP_IP_OPEN "+GIPOPEN: "
#define RESP_IP_CLOSE "+GIPCLOSE: "
#define RESP_IP_READ "+GIPRD: "
#define RESP_IP_SEND "+GIPSEND: "
#define RESP_IP_STATE "+GIPSTATE: "
#define RESP_IP_DNSCFG "+GDNSCFG: "
#define RESP_IP_DNSGIP "+GDNSGIP: "
#define RESP_IP_PING "+GPING: "
#define RESP_IP_NTP "+GNTP: "
#define RESP_IP_TKA "+GIPTKA: "
#define RESP_IP_MODE "+GIPMODE: "
#define RESP_IP_SACK "+GIPSACK: "
#define IP_URC "+GIPURC: "

#define AT_IP_CFG "AT+GIPCFG"
#define AT_IP_OPEN "AT+GIPOPEN"
#define AT_IP_CLOSE "AT+GIPCLOSE"
#define AT_IP_READ "AT+GIPRD"
#define AT_IP_SEND "AT+GIPSEND"
#define AT_IP_STATE "AT+GIPSTATE"
#define AT_IP_DNSGIP "AT+GDNSGIP"
#define AT_IP_PING "AT+GPING"
#define AT_IP_NTP "AT+GNTP"
#define AT_IP_TKA "AT+GIPTKA"
#define AT_IP_MODE "AT+GIPMODE"
#define AT_IP_SACK "AT+GIPSACK"
#define AT_IP_DNSCFG "AT+GDNSCFG"

#define RESP_SSL_CFG "+GSSLCFG: "
#define RESP_SSL_CERTRD "+GSSLCERTRD: "
#define RESP_SSL_LIST "+GSSLLIST: "
#define RESP_SSL_CHECK "+GSSLCHECK: "
#define RESP_SSL_CIPHER "+GSSLCIPHER: "

#define AT_SSL_CFG "AT+GSSLCFG"
#define AT_SSL_CERTWR "AT+GSSLCERTWR"
#define AT_SSL_KEYWR "AT+GSSLKEYWR"
#define AT_SSL_CERTRD "AT+GSSLCERTRD"
#define AT_SSL_LIST "AT+GSSLLIST"
#define AT_SSL_RM "AT+GSSLRM"
#define AT_SSL_CHECK "AT+GSSLCHECK"
#define AT_SSL_CIPHER "AT+GSSLCIPHER"
#endif

#ifdef CONFIG_CUSTOMER_CM_AT
#define RESP_IP_CFG "+MIPCFG: "
#define RESP_IP_OPEN "+MIPOPEN: "
#define RESP_IP_CLOSE "+MIPCLOSE: "
#define RESP_IP_READ "+MIPRD: "
#define RESP_IP_SEND "+MIPSEND: "
#define RESP_IP_STATE "+MIPSTATE: "
#define RESP_IP_DNSCFG "+MDNSCFG: "
#define RESP_IP_DNSGIP "+MDNSGIP: "
#define RESP_IP_PING "+MPING: "
#define RESP_IP_NTP "+MNTP: "
#define RESP_IP_TKA "+MIPTKA: "
#define RESP_IP_MODE "+MIPMODE: "
#define RESP_IP_SACK "+MIPSACK: "
#define IP_URC "+MIPURC: "

#define AT_IP_CFG "AT+MIPCFG"
#define AT_IP_OPEN "AT+MIPOPEN"
#define AT_IP_CLOSE "AT+MIPCLOSE"
#define AT_IP_READ "AT+MIPRD"
#define AT_IP_SEND "AT+MIPSEND"
#define AT_IP_STATE "AT+MIPSTATE"
#define AT_IP_DNSGIP "AT+MDNSGIP"
#define AT_IP_PING "AT+MPING"
#define AT_IP_NTP "AT+MNTP"
#define AT_IP_TKA "AT+MIPTKA"
#define AT_IP_MODE "AT+MIPMODE"
#define AT_IP_SACK "AT+MIPSACK"
#define AT_IP_DNSCFG "AT+MDNSCFG"

#define RESP_SSL_CFG "+MSSLCFG: "
#define RESP_SSL_CERTRD "+MSSLCERTRD: "
#define RESP_SSL_LIST "+MSSLLIST: "
#define RESP_SSL_CHECK "+MSSLCHECK: "
#define RESP_SSL_CIPHER "+MSSLCIPHER: "

#define AT_SSL_CFG "AT+MSSLCFG"
#define AT_SSL_CERTWR "AT+MSSLCERTWR"
#define AT_SSL_KEYWR "AT+MSSLKEYWR"
#define AT_SSL_CERTRD "AT+MSSLCERTRD"
#define AT_SSL_LIST "AT+MSSLLIST"
#define AT_SSL_RM "AT+MSSLRM"
#define AT_SSL_CHECK "AT+MSSLCHECK"
#define AT_SSL_CIPHER "AT+MSSLCIPHER"
#endif

#define AT_CMD_CHAR_0             '0'
#define AT_CMD_CHAR_9             '9'
#define AT_CMD_QUESTION_MASK      '?'
#define AT_CMD_EQUAL_MASK         '='
#define AT_CMD_L_SQ_BRACKET       '['
#define AT_CMD_R_SQ_BRACKET       ']'
#define AT_CMD_L_ANGLE_BRACKET    '<'
#define AT_CMD_R_ANGLE_BRACKET    '>'
#define AT_CMD_COMMA_MASK         ','
#define AT_CMD_SEMICOLON          ';'
#define AT_CMD_CR                 '\r'
#define AT_CMD_LF                 '\n'

#define AT_SOCKET_MAX_CONN_NUM_Q  12
#define AT_RESP_RESULT_MAX  128
// #define BUFFER_SIZE  128

#define PING_DATA_SIZE_MIN  0
#define PING_DATA_SIZE_MAX  1452
#define PING_DATA_SIZE_DEF  32
#define PING_TIMEOUT_MIN  1
#define PING_TIMEOUT_MAX  255

#define PING_ADDR_BUFFERSIZE_IPV4  16
#define PING_ADDR_BUFFERSIZE_IPV6  48

#define PING_WAIT_TIME_MAX  6
#define PING_COUNT_MAX  65535

#define ADDR_BUFFERSIZE_IPV4  16
#define ADDR_BUFFERSIZE_IPV6  48

#define AIC_TCPIP_AT_OK 0
#define AIC_TCPIP_AT_ERR 1

#define AT_DNS_TIMEOUT_DEFAULT  10  //seconds
#define AT_DNS_RETRY_CNT_DEFAULT  3

/* TCPIP AT CME ERROR NUM */
enum AT_CMD_RESP_ERR {
    AIC_TCPIP_ERR_PARSE_AT_PARAM = 900,
    AIC_TCPIP_ERR_INVAL_AT_PARAM,
    AIC_TCPIP_ERR_INVAL_AT_CMD,
    AIC_TCPIP_ERR_AT_CMD_NOT_SUPPORT,
    AIC_TCPIP_ERR_PDP_NOT_ACTIVE,
    AIC_TCPIP_ERR_SOCKET_NOT_EXIST,
    AIC_TCPIP_ERR_SOCKET_CREATE_FAIL,
    AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT,
    AIC_TCPIP_ERR_SOCKET_CONNECT_FAIL,
    AIC_TCPIP_ERR_SOCKET_RECV_FAIL,
    AIC_TCPIP_ERR_DNS_FAIL,
    AIC_TCPIP_ERR_NO_IPV6_PREFIX,
    AIC_TCPIP_ERR_SOCKET_BIND_FAIL,
    AIC_TCPIP_ERR_ENTER_DATA_MODE_FAIL,
    AIC_TCPIP_ERR_MAX
};

typedef struct SOCKET_LIST {
    int link_id;
    int pdp_cid;
    int socket_id;
    int socket_type;
    int family;
    int mode;
    int addr;
    int addr6[4];
    short port;
    int ssl_ctx_id;
#ifdef CONFIG_NET_SECURE
    UCHAR *common_name;
    u8 *ca_cert_der;
    u32 ca_cert_der_len;
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_ssl_context *ssl;
    mbedtls_net_context sockfd;
    mbedtls_pk_context *key;
#endif
    u8 in_use;
    u8 autofree;
    u32 recv_total;
    u32 recv_cnt;
    u8 status;
} AIC_SOCKET_LIST;

typedef struct AT_TCP_SERVER_SOCKET{
    int sockfd;
    int family;
    u16 port;
    fd_set master_list;
} AIC_AT_TCP_SERVER;

typedef struct at_sock_info{
    int sockid;
    int length;
    u8 *buf;
    int offset;
    u8  is_send;
}AIC_SOCKET_INFO;

struct socket_state{
    u8 socket_type[16];
    u8 addr[ADDR_BUFFERSIZE_IPV6];
    u16 remote_port;
    u16 local_port;
    int state;
    int pdp_cid;
    int server_id;
    int mode;
    u8 at_port[AT_URC_PORT_NAME_LEN];
    u8 socket_state[16];
};

enum SOCK_TYPE{
    SOCK_TCP = 1,
    SOCK_TCPv6,
    SOCK_TCPS,
    SOCK_TCPIN,
    SOCK_UDP,
    SOCK_UDPv6,
    SOCK_UDPS,
    SOCK_TLS,
    SOCK_TLSv6
};

enum AT_TYPE{
    AT_TCPIP = 1,
    AT_QTCPIP,
    AT_QSSL,
    AT_MTCPIP
};

struct at_dns_cfg{
    int dns_prio;
    u32 timeout;
    u8  retry_times;
};

#ifdef CONFIG_NET_AT_QTCPIP
u32 aic_get_send_echo_enable(void);
#endif

int aic_at_find_socket_by_linkid(int link_id, int type);
int aic_at_find_linkid_by_socket(int socket, int type);
int aic_at_find_empty_socket(int type);
int aic_at_find_index_by_socket(int socket, int type);
int aic_at_find_index_by_linkid(int link_id, int type);
bool aic_get_socket_is_valid(void);
u32 aic_transmit_resp_normal(AT_REQ_RESP *at_req_resp_ptr, char *resp);
u32 aic_transmit_resp_ok(AT_REQ_RESP *at_req_resp_ptr);
u32 aic_transmit_resp_error(AT_REQ_RESP *at_req_resp_ptr, int error);
u32 aic_transmit_resp_exit(AT_REQ_RESP *at_req_resp_ptr);

int aic_at_string_to_num(u8 *src);
u32 aic_hexstring_to_char(u8 *dest, u8 *src, u32 len);
//u32 aic_escape_data_normal(u8 *buffer, u8 *data, int datalen, int *actual_len);
u32 aic_escape_data(u8 *buffer, u8 *data, int datalen, int *actual_len, int *cnt);
void aic_print_data_to_hex(u8 *data, u32 length, u8 *out_data);

#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
ulong *aic_at_get_para_array(u8 *raw_str, u8 *start_str, u16 *paras_cnt,
                              u16 max_cnt, u16 data_index);
boolean aic_is_transparent_mode(void);
int aic_get_dns_prio(void);
#endif
#endif
