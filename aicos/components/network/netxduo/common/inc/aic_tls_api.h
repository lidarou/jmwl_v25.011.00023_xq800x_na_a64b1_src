/******************************************************************************/
/*                                                                            */
/*     Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_TLS_API_H__
#define __AIC_TLS_API_H__

#include <aic_osa.h>

#include <nx_api.h>
#include <nx_secure_tls.h>
#include <nx_secure_tls_api.h>

#define AIC_SECURE_TLS_VERSION_TLS_1_0   NX_SECURE_TLS_VERSION_TLS_1_0
#define AIC_SECURE_TLS_VERSION_TLS_1_1   NX_SECURE_TLS_VERSION_TLS_1_1
#define AIC_SECURE_TLS_VERSION_TLS_1_2   NX_SECURE_TLS_VERSION_TLS_1_2
#define AIC_SECURE_TLS_VERSION_TLS_1_3   NX_SECURE_TLS_VERSION_TLS_1_3

/* Private key type */
#define AIC_X509_KEY_TYPE_NONE           NX_SECURE_X509_KEY_TYPE_NONE
#define AIC_X509_KEY_TYPE_RSA_PKCS1_DER  NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER
#define AIC_X509_KEY_TYPE_EC_DER         NX_SECURE_X509_KEY_TYPE_EC_DER
#define AIC_X509_KEY_TYPE_HARDWARE       NX_SECURE_X509_KEY_TYPE_HARDWARE

struct aic_tls_session {
    u32 socket_id;
    char *crypto_metadata_client;
    UCHAR *tls_packet_buffer;

    NX_SECURE_TLS_SESSION *secure_tls_session;
#if 0
    NX_SECURE_X509_CERT *trusted_certificate;
    char *ca_cert_der;
    int ca_cert_der_len;
#endif
    NX_SECURE_X509_CERT *remote_certificate, *remote_issuer;
    UCHAR *remote_cert_buffer;
    UCHAR *remote_issuer_buffer;
    struct aic_tls_session *tls_session_prev;
    struct aic_tls_session *tls_session_next;
    int tls_ciphersuite_set_count;

    NX_SECURE_X509_DNS_NAME dns_name;
};

#if 0
struct aic_tls_ciphersuite_info {
    USHORT aic_tls_ciphersuite;
    int issupport;
};
#endif

struct aic_tls_session *aic_get_tls_session_socket_id(u32 socket_id);

int aic_tls_check_socket(int socket_id, struct aic_tls_session **tls);

int aic_create_tls_socket(int protocolFamily, int type, int protocol);

int aic_close_tls_socket(u32 socket_id);

int aic_tls_connect(u32 socket_id, NX_TCP_SOCKET *tcp_socket);

u32 aic_secure_tls_trusted_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT **certificate,
                                            UCHAR *common_name,
                                            UINT name_length);

int aic_tls_ca_certificate_add(u32 socket_id, char *certificate,
                               int certificate_len);

int aic_tls_ca_certificate_remove(u32 socket_id,
                                  char *common_name, int common_name_len);

int aic_tls_ca_certificate_remove_all(u32 socket_id);

int aic_tls_local_certificate_add(u32 socket_id, char *certificate,
                                  int certificate_len, char *private_key,
                                  int key_len, u32 key_type);

int aic_tls_local_certificate_remove(u32 socket_id,
                                     char *common_name, int common_name_len);

int aic_tls_ciphersuite_set(u32 socket_id, USHORT aic_tls_ciphersuite);

int aic_tls_session_protocol_version_override(int socket_id,
                                              u16 protocol_version);

int aic_secure_tls_session_renegotiate(int socket_id, u32 wait_option);

int aic_secure_tls_session_time_function_set(int socket_id,
                                             ulong (*time_func_ptr)(void));

int aic_tls_set_session_sni_extension(int socket_id, s8 *dns_url);

#endif
