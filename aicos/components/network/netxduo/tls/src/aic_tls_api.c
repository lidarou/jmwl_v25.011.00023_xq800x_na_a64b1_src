/*****************************************************************************/
/*                                                                           */
/*     Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** Network Component                                                        */
/**                                                                          */
/**   API of Transport Layer Security(TLS)                                   */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1723c7
#define LOG_MODULE    AMOD_NET

#include <stdlib.h>
#include <string.h>
#include <nx_api.h>
#include <nx_ip.h>
#include <nxd_bsd.h>
#ifdef CONFIG_NET_SECURE
#include <nx_secure_tls.h>
#include <nx_secure_tls_api.h>
#endif
#include <aic_errno.h>
#include "aic_tls_api.h"

#include <aic_init.h>
#include <aic_size.h>

static struct aic_tls_session *aic_tls_session_ptr = NULL;
#ifdef NX_SECURE_TLS_ENABLE_TLS_1_3
extern const USHORT nx_crypto_ecc_supported_groups[];
extern const NX_CRYPTO_METHOD *nx_crypto_ecc_curves;
extern NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers_ecc;
#define AIC_SECURE_TLS_CIPHERS_TABLE nx_crypto_tls_ciphers_ecc
#else
extern NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;
#define AIC_SECURE_TLS_CIPHERS_TABLE nx_crypto_tls_ciphers
#endif

#define TLS_SESSION_LIST_FOR_EACH(sa_pos, sa_head, n) \
    for (n = 0, sa_pos = (sa_head); \
         (sa_pos != (sa_head) || !n) && (sa_pos != NULL); \
         sa_pos = sa_pos->tls_session_next, n++)

static void aic_tls_add_session_to_list(struct aic_tls_session *tls_session)
{
    AOS_INTERRUPT_DECLARE
    struct aic_tls_session *tail_session;

    AOS_INTERRUPT_DISABLE
    if (!aic_tls_session_ptr) {
        aic_tls_session_ptr = tls_session;
        aic_tls_session_ptr->tls_session_prev = tls_session;
        aic_tls_session_ptr->tls_session_next = tls_session;
    } else {
        tail_session = aic_tls_session_ptr->tls_session_prev;

        aic_tls_session_ptr->tls_session_prev = tls_session;
        tail_session->tls_session_next = tls_session;

        tls_session->tls_session_prev = tail_session;
        tls_session->tls_session_next = aic_tls_session_ptr;
    }
    AOS_INTERRUPT_RESTORE
}

static void aic_tls_delete_session_from_list(struct aic_tls_session *
                                             tls_session)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (tls_session == tls_session->tls_session_next) {
        aic_tls_session_ptr = NX_NULL;
    } else {
        (tls_session->tls_session_next)->tls_session_prev =
            tls_session->tls_session_prev;
        (tls_session->tls_session_prev)->tls_session_next =
            tls_session->tls_session_next;

        if (aic_tls_session_ptr == tls_session)
            aic_tls_session_ptr = tls_session->tls_session_next;
    }
    AOS_INTERRUPT_RESTORE
}

struct aic_tls_session *aic_get_tls_session_socket_id(u32 socket_id)
{
    int i;
    AOS_INTERRUPT_DECLARE
    struct aic_tls_session *tls_session = NULL;

    AOS_INTERRUPT_DISABLE
    TLS_SESSION_LIST_FOR_EACH(tls_session, aic_tls_session_ptr, i) {
        if (tls_session->socket_id == socket_id)
            break;
    }

    if (!tls_session || ((tls_session == aic_tls_session_ptr) &&
        (tls_session->socket_id != socket_id))) {
        AOS_INTERRUPT_RESTORE
        return NULL;
    }

    AOS_INTERRUPT_RESTORE

    return tls_session;
}

static int aic_tls_setup(struct aic_tls_session *tls_session)
{
    u32 i, status = AIC_SUCCESS;

    tls_session->secure_tls_session = aos_zalloc(sizeof(NX_SECURE_TLS_SESSION));
    if (!tls_session->secure_tls_session) {
        aicx_error(0x132e2d, "", "[TLS] alloc secure tls session fail\r\n");
        return AIC_ENOMEM;
    }

    tls_session->crypto_metadata_client = aos_zalloc(SIZE_16K);
    if (!tls_session->crypto_metadata_client) {
        aicx_error(0x174ccc, "", "[TLS] alloc crypto metadata client fail\r\n");
        status = AIC_ENOMEM;
        goto OUT_ERR1;
    }

    status = nx_secure_tls_session_create(tls_session->secure_tls_session,
                                          &AIC_SECURE_TLS_CIPHERS_TABLE,
                                          tls_session->crypto_metadata_client,
                                          SIZE_16K);
    if (status != NX_SUCCESS) {
        aicx_error(0x01a68e, "x", "[TLS] create tls session fail 0x%x\r\n", status);
        goto OUT_ERR2;
    }

    tls_session->secure_tls_session->nx_secure_tls_ciphersuite_info =
        aos_malloc(tls_session->secure_tls_session->nx_secure_tls_crypto_table->nx_secure_tls_ciphersuite_lookup_table_size *
                   sizeof(struct aic_tls_ciphersuite_info));

    for (i = 0;
         i < tls_session->secure_tls_session->nx_secure_tls_crypto_table->nx_secure_tls_ciphersuite_lookup_table_size;
         i++) {
        (tls_session->secure_tls_session->nx_secure_tls_ciphersuite_info + i)->aic_tls_ciphersuite =
            (tls_session->secure_tls_session->nx_secure_tls_crypto_table->nx_secure_tls_ciphersuite_lookup_table + i)->nx_secure_tls_ciphersuite;
        (tls_session->secure_tls_session->nx_secure_tls_ciphersuite_info + i)->issupport = 1;
    }

    tls_session->secure_tls_session->nx_secure_tls_ciphersuite_info_size =
        tls_session->secure_tls_session->nx_secure_tls_crypto_table->nx_secure_tls_ciphersuite_lookup_table_size;

    tls_session->tls_packet_buffer = aos_zalloc(SIZE_4K);
    if (!tls_session->tls_packet_buffer) {
        status = AIC_ENOMEM;
        goto OUT_ERR3;
    }

    /* In general, the buffer should always be set to at least 16KB(16KB TLS
       record size + 2KB for encryption padding) and only made smaller in
       controlled settings (e.g the remote host guarantee a maximum TLS record
       size) */
    status = nx_secure_tls_session_packet_buffer_set(tls_session->secure_tls_session,
                                                     tls_session->tls_packet_buffer,
                                                     SIZE_4K);
    if (status != NX_SUCCESS) {
        aicx_error(0x12eb62, "x", "[TLS] tls session packet buffer set fail 0x%x\r\n", status);
        goto OUT_ERR4;
    }

#if 0
    tls_session->trusted_certificate = aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    AIC_ASSERT(tls_session->trusted_certificate);
    tls_session->ca_cert_der = aos_zalloc(ca_cert_der_len);
    AIC_ASSERT(tls_session->ca_cert_der);
    nx_secure_x509_certificate_initialize(tls_session->trusted_certificate,
        tls_session->ca_cert_der, (USHORT)tls_session->ca_cert_der_len,
        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE);
    nx_secure_tls_trusted_certificate_add(tls_session->secure_tls_session,
        tls_session->trusted_certificate);
#endif
    tls_session->remote_certificate = aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (!tls_session->remote_certificate) {
        aicx_error(0x0b259e, "", "[TLS] alloc remote certificate fail\r\n");
        status = AIC_ENOMEM;
        goto OUT_ERR4;
    }

    tls_session->remote_cert_buffer = aos_zalloc(SIZE_2K);
    if (!tls_session->remote_cert_buffer) {
        aicx_error(0x1a3aca, "", "[TLS] alloc remote cert buffer fail\r\n");
        status = AIC_ENOMEM;
        goto OUT_ERR5;
    }

    status = nx_secure_tls_remote_certificate_allocate(
                 tls_session->secure_tls_session,
                 tls_session->remote_certificate,
                 tls_session->remote_cert_buffer,
                 SIZE_2K);
    if (status != NX_SUCCESS) {
        aicx_error(0x077c66, "x", "[TLS] tls remote certificate allocate fail 0x%x\r\n",
                   status);
        goto OUT_ERR6;
    }

    tls_session->remote_issuer = aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (!tls_session->remote_issuer) {
        aicx_error(0x093ea6, "", "[TLS] alloc remote issuer fail\r\n");
        status = AIC_ENOMEM;
        goto OUT_ERR7;
    }

    tls_session->remote_issuer_buffer = aos_zalloc(SIZE_2K);
    if (!tls_session->remote_issuer_buffer) {
        aicx_error(0x093ea6, "", "[TLS] alloc remote issuer fail\r\n");
        status = AIC_ENOMEM;
        goto OUT_ERR8;
    }

    status = nx_secure_tls_remote_certificate_allocate(tls_session->secure_tls_session,
                                                       tls_session->remote_issuer,
                                                       tls_session->remote_issuer_buffer,
                                                       SIZE_2K);
    if (status != NX_SUCCESS) {
        aicx_error(0x03518f, "", "[TLS] secure tls remote certificate allocate fail\r\n");
        goto OUT_ERR9;
    }

#ifdef NX_SECURE_TLS_ENABLE_TLS_1_3
    status = nx_secure_tls_ecc_initialize(tls_session->secure_tls_session,
                                          nx_crypto_ecc_supported_groups, 3,
                                          &nx_crypto_ecc_curves);
    if (status != NX_SUCCESS) {
        aicx_error(0x0da7a3, "x", "[TLS] secure tls ecc initialize fail 0x%x\r\n", status);
        goto OUT_ERR9;
    }
#endif

    return status;

OUT_ERR9:
    aos_free(tls_session->remote_issuer_buffer);
    tls_session->remote_issuer_buffer = NULL;
OUT_ERR8:
    aos_free(tls_session->remote_issuer);
    tls_session->remote_issuer = NULL;
OUT_ERR7:
    nx_secure_tls_remote_certificate_free_all(tls_session->secure_tls_session);
OUT_ERR6:
    aos_free(tls_session->remote_cert_buffer);
    tls_session->remote_cert_buffer = NULL;
OUT_ERR5:
    aos_free(tls_session->remote_certificate);
    tls_session->remote_certificate = NULL;
OUT_ERR4:
    aos_free(tls_session->tls_packet_buffer);
    tls_session->tls_packet_buffer = NULL;
OUT_ERR3:
    nx_secure_tls_session_delete(tls_session->secure_tls_session);
OUT_ERR2:
    aos_free(tls_session->secure_tls_session);
    tls_session->secure_tls_session = NULL;
OUT_ERR1:
    aos_free(tls_session->crypto_metadata_client);
    tls_session->crypto_metadata_client = NULL;

    return status;
}

/**
 * aic_create_tls_socket - Create tls socket.
 *
 * @protocolFamily: Protocol family e.g AF_INET.
 * @type: Type of the socket, only support SOCK_STREAM.
 * @protocol: Socket protocol, only support IPPROTO_TCP.
 *
 * Create a TLS socket, the parameters are the same as BSD socket. The BSD
 * scoket API is called internally, and the resources needed for TLS are
 * prepared.
 *
 *
 * Return socket descriptor on success, return -1 on failure.
 */
int aic_create_tls_socket(int protocolFamily, int type, int protocol)
{
    int ret, socket_id;
    struct aic_tls_session *tls_session;

    if ((type != SOCK_STREAM) || (protocol != IPPROTO_TCP)) {
        aicx_error(0x05428d, "", "[TLS] only support tcp proto\r\n");
        return AIC_EPERM;
    }

    tls_session = aos_zalloc(sizeof(struct aic_tls_session));
    if (!tls_session) {
        aicx_error(0x1b1c43, "", "[TLS] alloc tls session memory fail\r\n");
        return AIC_EPERM;
    }

    ret = aic_tls_setup(tls_session);
    if (ret) {
        aicx_error(0x1a93ff, "x", "[TLS] tls setup failed 0x%x\r\n", ret);
        return AIC_EPERM;
    }

    aic_tls_add_session_to_list(tls_session);

    socket_id = socket(protocolFamily, type, protocol);
    if (socket_id == NX_SOC_ERROR) {
        aicx_error(0x07cb47, "", "[TLS] create socket fail\r\n");
        aos_free(tls_session);
        return socket_id;
    }

    tls_session->socket_id = socket_id;
    tls_session->tls_ciphersuite_set_count = 0;

    return socket_id;
}

/**
 * aic_tls_connect - Establishes a connection between a client tls socket and a
 *                   remote server.
 *
 * @socket_id: Socket descriptor.
 * @tcp_socket: Netxduo tcp socket pointer.
 *
 * Starts a TLS session given a TCP socket. The TCP connection must be
 * established before calling this function. The BSD socket is called
 * internally, and the applications don't need to call it.
 *
 * Return NX_SUCCESS on success, others on failure.
 */
int aic_tls_connect(u32 socket_id, NX_TCP_SOCKET *tcp_socket)
{
    int ret;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);

    if (!tls_session)
        return AIC_SUCCESS;

    ret = nx_secure_tls_session_start(tls_session->secure_tls_session,
                                      tcp_socket, NX_WAIT_FOREVER);
    if (ret != NX_SUCCESS) {
        aicx_info(0x06b8e7, "x", "[AIC_TLS]: tls handshake fail, status is 0x%x\n", ret);
        return ret;
    }

    return ret;
}

/**
 * aic_tls_check_socket - Check whether the current socket is a tls socket.
 *
 * @socket_id: Socket descriptor.
 * @tls: In the case of a tls socket, its tls session is assigned to this
 *       pointer.
 *
 * Check whether the current socket is a tls socket.
 *
 * Return NX_SUCCESS on success, others on failure.
 */
int aic_tls_check_socket(int socket_id, struct aic_tls_session **tls)
{
    *tls = aic_get_tls_session_socket_id(socket_id);

    if (!(*tls))
        return AIC_EPERM;

    return AIC_SUCCESS;
}

/**
 * aic_close_tls_socket - Close a tls socket.
 *
 * @socket_id: Socket descriptor.
 *
 * Terminates communications on the supplied BSD socket, release resources
 * related to tls.
 *
 * Return ZERO on success, others on failure.
 */
int aic_close_tls_socket(u32 socket_id)
{
    u32 status = AIC_SUCCESS;
    struct aic_tls_session *tls_session;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_SUCCESS;

    status = nx_secure_tls_session_end(tls_session->secure_tls_session,
                                       NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
        aicx_error(0x08df55, "x", "[TLS] nx secure tls session end fail 0x%x\n", status);

    status = nx_secure_tls_session_delete(tls_session->secure_tls_session);
    if (status != NX_SUCCESS)
        aicx_error(0x0265cd, "x", "[TLS] nx secure tls session delete fail 0x%x\n", status);

    aic_tls_ca_certificate_remove_all(socket_id);
    if (tls_session->crypto_metadata_client)
        aos_free(tls_session->crypto_metadata_client);

    if (tls_session->secure_tls_session)
        aos_free(tls_session->secure_tls_session);

    if (tls_session->tls_packet_buffer)
        aos_free(tls_session->tls_packet_buffer);
#if 0
    if (tls_session->ca_cert_der)
        aos_free(tls_session->ca_cert_der);

    if (tls_session->trusted_certificate)
        aos_free(tls_session->trusted_certificate);
#endif
    if (tls_session->remote_certificate)
        aos_free(tls_session->remote_certificate);

    if (tls_session->remote_issuer)
        aos_free(tls_session->remote_issuer);

    if (tls_session->remote_cert_buffer)
        aos_free(tls_session->remote_cert_buffer);

    if (tls_session->remote_issuer_buffer)
        aos_free(tls_session->remote_issuer_buffer);

    aic_tls_delete_session_from_list(tls_session);

    aos_free(tls_session);

    return soc_close(socket_id);
}

u32 aic_secure_tls_trusted_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT **certificate,
                                            UCHAR *common_name,
                                            UINT name_length)
{
    UINT                              status;
    NX_SECURE_X509_CERT              *list_head;
    NX_SECURE_X509_CERTIFICATE_STORE *store;
    NX_SECURE_X509_DISTINGUISHED_NAME name;

    /* Get the remote certificate store from our TLS session. */
    store = &tls_session->nx_secure_tls_credentials.nx_secure_tls_certificate_store;

    /* Get the first certificate in the local store. */
    list_head = store->nx_secure_x509_trusted_certificates;

    name.nx_secure_x509_common_name = common_name;
    name.nx_secure_x509_common_name_length = (USHORT)name_length;

    /* Find the certificate using it's name. */
    status = _nx_secure_x509_certificate_list_find(&list_head, &name,
                                                   0, certificate);
    /* Translate some X.509 return values into TLS return values. */
    if (status == NX_SECURE_X509_CERTIFICATE_NOT_FOUND)
    {
        return NX_SECURE_TLS_CERTIFICATE_NOT_FOUND;
    }

    /* Return completion status.  */
    return status;
}

/**
 * aic_tls_ca_certificate_add - The tls socket add a CA certificate.
 *
 * @socket_id: Socket descriptor.
 * @certificate: Certificate pointer.
 * @certificate_len: Certificate length.
 *
 * The tls socket add a certificate.
 *
 * Return ZERO on success, others on failure.
 */
int aic_tls_ca_certificate_add(u32 socket_id, char *certificate,
                               int certificate_len)
{
    UINT status = NX_SUCCESS;
    NX_SECURE_X509_CERT *trusted_certificate;
    struct aic_tls_session *tls_session = NULL;

    if (!certificate) {
        return AIC_EPERM;
    }

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    trusted_certificate = aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (!trusted_certificate)
        return AIC_ENOMEM;
    status = nx_secure_x509_certificate_initialize(trusted_certificate,
                                                   certificate,
                                                   (USHORT)certificate_len,
                                                   NX_NULL, 0, NX_NULL, 0,
                                                   NX_SECURE_X509_KEY_TYPE_NONE);
    if (status != NX_SUCCESS)
        aicx_error(0x1d2669, "x", "[TLS] certificate initialize fail 0x%x\r\n", status);

    status =
        nx_secure_tls_trusted_certificate_add(tls_session->secure_tls_session,
                                              trusted_certificate);
    if (status != NX_SUCCESS)
        aicx_error(0x136252, "x", "[TLS] trusted certificate add fail 0x%x\r\n", status);

    return status;
}

/**
 * aic_tls_certificate_remove - The tls socket remove a CA certificate.
 *
 * @socket_id: Socket descriptor.
 * @common_name: Certificate name.
 * @common_name_len: Certificate name length.
 *
 * The tls socket remove a certificate.
 *
 * Return ZERO on success, others on failure.
 */
int aic_tls_ca_certificate_remove(u32 socket_id,
                                  char *common_name, int common_name_len)
{
    int ret;
    NX_SECURE_X509_CERT *certificate;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    ret = aic_secure_tls_trusted_certificate_find(tls_session->secure_tls_session,
                                                  &certificate, common_name,
                                                  common_name_len);
    if (ret != NX_SECURE_TLS_SUCCESS) {
        aicx_info(0x16b77e, "x", "[AIC_TLS]: certificate not find, ret 0x%x\n", ret);
        return ret;
    }

    ret = nx_secure_tls_trusted_certificate_remove(tls_session->secure_tls_session,
                                                   common_name,
                                                   common_name_len);
    if (ret != NX_SECURE_TLS_SUCCESS) {
        aicx_info(0x0dcffb, "x", "[AIC_TLS]: trusted certificate remove fail, ret 0x%x\n",
                  ret);
        return ret;
    }

    aos_free(certificate);

    return AIC_SUCCESS;
}

int aic_tls_ca_certificate_remove_all(u32 socket_id)
{
    NX_SECURE_X509_CERTIFICATE_STORE *store;
    NX_SECURE_X509_CERT *certificate;
    NX_SECURE_X509_CERT *ca_certificate;
    char *common_name;
    u16 common_name_len;
    int ret;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    store = &tls_session->secure_tls_session->nx_secure_tls_credentials.nx_secure_tls_certificate_store;
    certificate = store->nx_secure_x509_trusted_certificates;
    while (certificate != NX_NULL) {
        common_name =
            (char *)certificate->nx_secure_x509_distinguished_name.nx_secure_x509_common_name;
        common_name_len =
            certificate->nx_secure_x509_distinguished_name.nx_secure_x509_common_name_length;
        ret =
            aic_secure_tls_trusted_certificate_find(tls_session->secure_tls_session,
                                                    &ca_certificate,
                                                    common_name,
                                                    common_name_len);
        if (ret != NX_SECURE_TLS_SUCCESS) {
            return ret;
        }

        ret = nx_secure_tls_trusted_certificate_remove(tls_session->secure_tls_session,
                                                       common_name,
                                                       common_name_len);
        if (ret != NX_SECURE_TLS_SUCCESS)
            return ret;
        else
            aos_free(ca_certificate);

        certificate = store->nx_secure_x509_trusted_certificates;
    }

    return ret;
}

/**
 * aic_tls_local_certificate_add - The tls socket add a local certificate.
 *
 * @socket_id: Socket descriptor.
 * @certificate: Certificate pointer.
 * @certificate_len: Certificate length.
 * @private_key: Private key pointer.
 * @key_len: Private key length.
 * @key_type: Private key type.
 *
 * The tls socket add a certificate.
 *
 * Return ZERO on success, others on failure.
 */
int aic_tls_local_certificate_add(u32 socket_id, char *certificate,
                                  int certificate_len, char *private_key,
                                  int key_len, u32 key_type)
{
    u32 status = NX_SUCCESS;
    NX_SECURE_X509_CERT *local_certificate;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    local_certificate = aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (!local_certificate)
        return AIC_ENOMEM;
    status = nx_secure_x509_certificate_initialize(local_certificate,
                                                   certificate,
                                                   (USHORT)certificate_len,
                                                   NX_NULL, 0, private_key,
                                                   (USHORT)key_len, key_type);
    if (status != NX_SUCCESS)
        aicx_error(0x1d2669, "x", "[TLS] certificate initialize fail 0x%x\r\n", status);

    status =
        nx_secure_tls_local_certificate_add(tls_session->secure_tls_session,
                                            local_certificate);
    if (status != NX_SUCCESS)
        aicx_error(0x183f18, "x", "[TLS] local certificate add fail 0x%x\r\n", status);

    return status;
}

/**
 * aic_tls_local_certificate_remove - The tls socket remove a
 * local certificate.
 *
 * @socket_id: Socket descriptor.
 * @common_name: Certificate name.
 * @common_name_len: Certificate name length.
 *
 * The tls socket remove a certificate.
 *
 * Return ZERO on success, others on failure.
 */
int aic_tls_local_certificate_remove(u32 socket_id,
                                     char *common_name, int common_name_len)
{
    int ret;
    NX_SECURE_X509_CERT *certificate;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    ret = nx_secure_tls_local_certificate_find(tls_session->secure_tls_session,
                                               &certificate, common_name,
                                               common_name_len);
    if (ret != NX_SECURE_TLS_SUCCESS) {
        aicx_info(0x16b77e, "x", "[AIC_TLS]: certificate not find, ret 0x%x\n", ret);
        return ret;
    }

    ret = nx_secure_tls_local_certificate_remove(tls_session->secure_tls_session,
                                                 common_name,
                                                 common_name_len);
    if (ret != NX_SECURE_TLS_SUCCESS) {
        aicx_info(0x046f29, "x", "[AIC_TLS]: local certificate remove fail, ret 0x%x\n",
                  ret);
        return ret;
    }

    aos_free(certificate);

    return AIC_SUCCESS;
}

/**
 * aic_tls_ciphersuite_set - The tls socket set ciphersuite.
 *
 * @socket_id: Socket descriptor.
 * @aic_tls_ciphersuite: TLS ciphersuite.
 *
 * The tls socket set a ciphersuite.
 *
 * Return ZERO on success, others on failure.
 */
int aic_tls_ciphersuite_set(u32 socket_id, USHORT aic_tls_ciphersuite)
{
    int i;
    struct aic_tls_session *tls_session = NULL;
    struct aic_tls_ciphersuite_info *aic_ciphersuite_table;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    aic_ciphersuite_table =
        tls_session->secure_tls_session->nx_secure_tls_ciphersuite_info;
    if (tls_session->tls_ciphersuite_set_count == 0) {
        for (i = 0; i < tls_session->secure_tls_session->nx_secure_tls_crypto_table->nx_secure_tls_ciphersuite_lookup_table_size;
             i++)
            aic_ciphersuite_table[i].issupport = 0;
    }

    for (i = 0; i < tls_session->secure_tls_session->nx_secure_tls_crypto_table->nx_secure_tls_ciphersuite_lookup_table_size;
         i++) {
        if(aic_ciphersuite_table[i].aic_tls_ciphersuite ==
            aic_tls_ciphersuite) {
            aic_ciphersuite_table[i].issupport = 1;
            tls_session->tls_ciphersuite_set_count++;
            break;
        }
    }
    return AIC_SUCCESS;
}

/**
 * aic_tls_session_protocol_version_override - set tls socket protocol version.
 *
 * @socket_id: Socket descriptor.
 * @protocol_version: Tls protocol version.
 *
 * Set tls socket protocol version, need to be called before connect.
 *
 * Return ZERO on success, others on failure.
 */
int aic_tls_session_protocol_version_override(int socket_id,
                                              u16 protocol_version)
{
    int ret;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    ret = nx_secure_tls_session_protocol_version_override(tls_session->secure_tls_session,
                                                          protocol_version);
    if (ret)
        aicx_error(0x19d892, "dx", "[TLS] socket %d set tls protocol version fail 0x%x\r\n",
                   socket_id, ret);

    return ret;
}

/**
 * aic_secure_tls_session_renegotiate - set tls session renegotiate.
 *
 * @socket_id: Socket descriptor.
 * @wait_option: Timeout.
 *
 * Return ZERO on success, others on failure.
 */
int aic_secure_tls_session_renegotiate(int socket_id, u32 wait_option)
{
    int ret;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    ret = nx_secure_tls_session_renegotiate(tls_session->secure_tls_session,
                                            wait_option);
    if (ret)
        aicx_error(0x19d892, "dx", "[TLS] socket %d set tls protocol version fail 0x%x\r\n",
                   socket_id, ret);

    return ret;
}

/**
 * aic_secure_tls_session_time_function_set - set time function.
 *
 * @socket_id: Socket descriptor.
 * @time_func_ptr: Pointer to time function.
 *
 * This function sets up a function pointer that TLS will invoke when
 * it needs to get the current time, which is used in various TLS
 * handshake messages and for verification of certificates.
 *
 * The function is expected to return the current GMT in UNIX 32-bit
 * format (seconds since the midnight starting Jan 1, 1970, UTC,
 * ignoring leap seconds), as per the ClientHello requirements in the
 * TLS RFC 5246.
 *
 * Return ZERO on success, others on failure.
 */
int aic_secure_tls_session_time_function_set(int socket_id,
                                             ulong (*time_func_ptr)(void))
{
    int ret;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    ret = _nx_secure_tls_session_time_function_set(tls_session->secure_tls_session,
                                                   time_func_ptr);
    if (ret)
        aicx_error(0x09c55f, "dx", "[TLS] socket %d set tls session time function fail 0x%x\r\n",
                   socket_id, ret);

    return ret;
}

/**
 * aic_tls_set_session_sni_extension - Server name indication.
 *
 * @socket_id: Socket descriptor.
 * @dns_url: DNS name string.
 *
 * This function initializes an X509 DNS name for use with extensions that
 * use domain name entries.
 *
 * This function checks sets the DNS Name in a TLS client for the Server Name
 * Indication (SNI) extension (RFC 6066). The current RFC specifies that only
 * a single DNS Name entry is allowed in the SNI extension data. If other name
 * types are added in the future, new API will be added.
 *
 * Return ZERO on success, others on failure.
 */
int aic_tls_set_session_sni_extension(int socket_id, s8 *dns_url)
{
    int ret;
    struct aic_tls_session *tls_session = NULL;

    tls_session = aic_get_tls_session_socket_id(socket_id);
    if (!tls_session)
        return AIC_EPERM;

    ret = nx_secure_x509_dns_name_initialize(&tls_session->dns_name, dns_url,
                                             strlen(dns_url));
    if (ret) {
        aicx_error(0x1c50bb, "dx", "[TLS] socket %d init x509 dns name fail 0x%x\r\n",
                   socket_id, ret);
        return ret;
    }

    ret = nx_secure_tls_session_sni_extension_set(tls_session->secure_tls_session,
                                                  &tls_session->dns_name);
    if (ret) {
        aicx_error(0x1d8214, "dx", "[TLS] socket %d set session sni extension fail 0x%x\r\n",
                   socket_id, ret);
        return ret;
    }

    return ret;
}

int aic_tls_init(void)
{
    nx_secure_tls_initialize();

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_tls_init);

