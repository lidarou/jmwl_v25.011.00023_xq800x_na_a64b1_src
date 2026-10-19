/**************************************************************************/
/*                                                                        */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Network Component
 *  File Transfer Protocol Secure (FTP using TLS)
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1547df
#define LOG_MODULE    AMOD_NET

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <nx_ip.h>
#include <nx_ipv6.h>
#include <nxd_bsd.h>
#include <nxd_ftp_client.h>
#include <fs_api.h>
#include <fs_cfg.h>

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_api.h>
#include <aic_net_pdp_mgr.h>
#include <aic_ftps_client.h>

static struct aic_network_ip_instance *aic_ftps_ip_instance = AIC_NULL;
static struct aic_packet_pool ftps_client_pool = {0};
static NX_IP *ftps_ip_ptr = AIC_NULL;

/* Define global param */
#ifdef CONFIG_NET_SECURE
extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;
extern const u16 nx_crypto_ecc_supported_groups[];
extern const NX_CRYPTO_METHOD *nx_crypto_ecc_curves[];
extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers_ecc;
#endif

#ifdef CONFIG_NET_SECURE
/* Release the ftps control socket remote certificate data space. */
static void ftps_client_control_remote_crts_release(AIC_FTPS_CLIENT *client_ptr)
{
    if (client_ptr->ctrl_crts.remote_crt) {
        aos_free(client_ptr->ctrl_crts.remote_crt);
        client_ptr->ctrl_crts.remote_crt = AIC_NULL;
    }

    if (client_ptr->ctrl_crts.remote_issuer) {
        aos_free(client_ptr->ctrl_crts.remote_issuer);
        client_ptr->ctrl_crts.remote_issuer = AIC_NULL;
    }

    if (client_ptr->ctrl_crts.remote_crt_buf) {
        aos_free(client_ptr->ctrl_crts.remote_crt_buf);
        client_ptr->ctrl_crts.remote_crt_buf = AIC_NULL;
    }

    if (client_ptr->ctrl_crts.remote_issuer_buf) {
        aos_free(client_ptr->ctrl_crts.remote_issuer_buf);
        client_ptr->ctrl_crts.remote_issuer_buf = AIC_NULL;
    }

    return;
}

/* Allocate space for the certificate coming in from the remote host. */
static u32 ftps_client_control_tls_remote_crt_set(AIC_FTPS_CLIENT *client_ptr)
{
    u32 status = AIC_FTPS_BUFFER_ALLOCATE_ERROR;

    client_ptr->ctrl_crts.remote_crt = aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (client_ptr->ctrl_crts.remote_crt == AIC_NULL)
        goto ERROR_OUT;

    client_ptr->ctrl_crts.remote_crt_buf = aos_zalloc(FTPS_REMOTE_CERT_BUF_SIZE);
    if (client_ptr->ctrl_crts.remote_crt_buf == AIC_NULL)
        goto ERROR_OUT;

    client_ptr->ctrl_crts.remote_issuer =
        aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (client_ptr->ctrl_crts.remote_issuer == AIC_NULL)
        goto ERROR_OUT;

    client_ptr->ctrl_crts.remote_issuer_buf =
        aos_zalloc(FTPS_REMOTE_ISSUER_BUF_SIZE);
    if (client_ptr->ctrl_crts.remote_issuer_buf == AIC_NULL)
        goto ERROR_OUT;

    status = nx_secure_tls_remote_certificate_allocate(
             &(client_ptr->ftps_control_tls_session),
             client_ptr->ctrl_crts.remote_crt,
             client_ptr->ctrl_crts.remote_crt_buf, FTPS_REMOTE_CERT_BUF_SIZE);
    if (status != AIC_SUCCESS) {
        aicx_error(0x006c33, "x", "ctrl remote crt allocate fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    status = nx_secure_tls_remote_certificate_allocate(
             &(client_ptr->ftps_control_tls_session),
             client_ptr->ctrl_crts->remote_issuer,
             client_ptr->ctrl_crts->remote_issuer_buf,
             FTPS_REMOTE_ISSUER_BUF_SIZE);

    if (status != AIC_SUCCESS) {
        aicx_error(0x1a7151, "x", "ctrl issuer crt allocate fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

ERROR_OUT:
    if (status)
        ftps_client_control_remote_crts_release(client_ptr);

    return status;
}

/* Release the ftps data socket remote certificate data space. */
static void ftps_client_data_remote_crts_release(AIC_FTPS_CLIENT *client_ptr)
{
    if (client_ptr->data_crts.remote_crt) {
        aos_free(client_ptr->data_crts.remote_crt);
        client_ptr->data_crts.remote_crt = AIC_NULL;
    }

    if (client_ptr->data_crts.remote_issuer) {
        aos_free(client_ptr->data_crts.remote_issuer);
        client_ptr->data_crts.remote_issuer = AIC_NULL;
    }

    if (client_ptr->data_crts.remote_crt_buf) {
        aos_free(client_ptr->data_crts.remote_crt_buf);
        client_ptr->data_crts.remote_crt_buf = AIC_NULL;
    }

    if (client_ptr->data_crts.remote_issuer_buf) {
        aos_free(client_ptr->data_crts.remote_issuer_buf);
        client_ptr->data_crts.remote_issuer_buf = AIC_NULL;
    }

    return;
}

/* Allocate space for the certificate coming in from the remote host. */
static u32 ftps_client_data_tls_remote_crt_set(AIC_FTPS_CLIENT *client_ptr)
{
    u32 status = AIC_FTPS_BUFFER_ALLOCATE_ERROR;

    client_ptr->data_crts.remote_crt = aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (client_ptr->data_crts.remote_crt == AIC_NULL)
        goto ERROR_OUT;

    client_ptr->data_crts.remote_crt_buf = aos_zalloc(FTPS_REMOTE_CERT_BUF_SIZE);
    if (client_ptr->data_crts.remote_crt_buf == AIC_NULL)
        goto ERROR_OUT;

    client_ptr->data_crts.remote_issuer =
        aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (client_ptr->data_crts.remote_issuer == AIC_NULL)
        goto ERROR_OUT;

    client_ptr->data_crts.remote_issuer_buf =
        aos_zalloc(FTPS_REMOTE_ISSUER_BUF_SIZE);
    if (client_ptr->data_crts.remote_issuer_buf == AIC_NULL)
        goto ERROR_OUT;

    status = nx_secure_tls_remote_certificate_allocate(
             &(client_ptr->ftps_data_tls_session),
             client_ptr->data_crts.remote_crt,
             client_ptr->data_crts.remote_crt_buf, FTPS_REMOTE_CERT_BUF_SIZE);

    if (status != AIC_SUCCESS) {
        aicx_error(0x035655, "x", "data remote crt allocate fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    status = nx_secure_tls_remote_certificate_allocate(
             &(client_ptr->ftps_data_tls_session),
             client_ptr->data_crts.remote_issuer,
             client_ptr->data_crts.remote_issuer_buf,
             FTPS_REMOTE_ISSUER_BUF_SIZE);
    if (status != AIC_SUCCESS) {
        aicx_error(0x0c93f1, "x", "data issuer crt allocate fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

ERROR_OUT:
    if (status)
        ftps_client_data_remote_crts_release(client_ptr);

    return status;
}

/* Release the ftps control tls session space. */
static void ftps_client_control_tls_session_release(AIC_FTPS_CLIENT *client_ptr)
{
    if (client_ptr->control_tls_info.tls_metadata_area) {
        aos_free(client_ptr->control_tls_info.tls_metadata_area);
        client_ptr->control_tls_info.tls_metadata_area = AIC_NULL;
    }

    if (client_ptr->control_tls_info.tls_packet_buffer) {
        aos_free(client_ptr->control_tls_info.tls_packet_buffer);
        client_ptr->control_tls_info.tls_packet_buffer = AIC_NULL;
    }

    return;
}

/* Release the ftps data tls session space. */
static void ftps_client_data_tls_session_release(AIC_FTPS_CLIENT *client_ptr)
{
    if (client_ptr->data_tls_info.tls_metadata_area) {
        aos_free(client_ptr->data_tls_info.tls_metadata_area);
        client_ptr->data_tls_info->tls_metadata_area = AIC_NULL;
    }

    if (client_ptr->data_tls_info.tls_packet_buffer) {
        aos_free(client_ptr->data_tls_info.tls_packet_buffer);
        client_ptr->data_tls_info.tls_packet_buffer = AIC_NULL;
    }

    return;
}

/* Allocate the control tls session space. Initialize and create TLS session. */
static u32 ftps_client_control_tls_session_set(AIC_FTPS_CLIENT *client_ptr)
{
    u32 status = AIC_FTPS_BUFFER_ALLOCATE_ERROR;

    client_ptr->control_tls_info.tls_metadata_area =
        aos_zalloc(FTPS_TLS_METADATA_SIZE);
    if (client_ptr->control_tls_info.tls_metadata_area == AIC_NULL)
        goto TLS_SET_ERROR;

    client_ptr->control_tls_info.tls_packet_buffer =
        aos_zalloc(FTPS_TLS_PACKET_BUF_SIZE);
    if (client_ptr->control_tls_info.tls_packet_buffer == AIC_NULL)
        goto TLS_SET_ERROR;

    status = nx_secure_tls_session_create(
             &(client_ptr->ftps_control_tls_session), &nx_crypto_tls_ciphers,
             client_ptr->control_tls_info.tls_metadata_area,
             FTPS_TLS_METADATA_SIZE);
    if (status != AIC_SUCCESS) {
        aicx_error(0x1df698, "x", "ftps ctrl socket tls session create fail(0x%x)\n", status);
        goto TLS_SET_ERROR;
    }

    /* Allocate space for packet reassembly. */
    status = nx_secure_tls_session_packet_buffer_set(
             &(client_ptr->ftps_control_tls_session),
             client_ptr->control_tls_info.tls_packet_buffer,
             FTPS_TLS_PACKET_BUF_SIZE);
    if (status != AIC_SUCCESS) {
        aicx_error(0x112849, "x", "ftps crtl tls packet buffer set fail(0x%x)\n", status);
        goto TLS_SET_ERROR;
    }

TLS_SET_ERROR:
    if (status)
        ftps_client_control_tls_session_release(client_ptr);

    return status;
}


/* Allocate the data tls session space. Initialize and create TLS session. */
static u32 ftps_client_data_tls_session_set(AIC_FTPS_CLIENT *client_ptr)
{
    u32 status = AIC_FTPS_BUFFER_ALLOCATE_ERROR;

    client_ptr->data_tls_info.tls_metadata_area =
        aos_zalloc(FTPS_TLS_METADATA_SIZE);
    if (client_ptr->data_tls_info.tls_metadata_area == AIC_NULL)
        goto TLS_SET_ERROR;


    client_ptr->data_tls_info.tls_packet_buffer =
        aos_zalloc(FTPS_TLS_PACKET_BUF_SIZE);
    if (client_ptr->data_tls_info->tls_packet_buffer == AIC_NULL)
        goto TLS_SET_ERROR;

    status = nx_secure_tls_session_create(
             &(client_ptr->ftps_data_tls_session), &nx_crypto_tls_ciphers,
             client_ptr->data_tls_info.tls_metadata_area,
             FTPS_TLS_METADATA_SIZE);
    if (status != AIC_SUCCESS) {
        aicx_error(0x0cbd61, "x", "ftps data socket tls session create fail(0x%x)\n", status);
        goto TLS_SET_ERROR;
    }

    /* Allocate space for packet reassembly. */
    status = nx_secure_tls_session_packet_buffer_set(
             &(client_ptr->ftps_data_tls_session),
             client_ptr->data_tls_info.tls_packet_buffer,
             FTPS_TLS_PACKET_BUF_SIZE);
    if (status != AIC_SUCCESS) {
        aicx_error(0x070981, "x", "ftps data tls packet buffer set fail(0x%x)\n", status);
        goto TLS_SET_ERROR;
    }

TLS_SET_ERROR:
    if (status)
        ftps_client_data_tls_session_release(client_ptr);

    return status;
}

/**
 * Allocate a packet for sending FTPS Client control command.
 * @handle: Ftps client handle.
 * @packet_ptr: allocated packet.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
static u32 ftps_client_control_tls_packet_allocate(u32 handle,
                                                   NX_PACKET **packet_ptr,
                                                   ulong wait_option)
{
    u32 status;

    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    ftps_client_ptr = client_ptr->ftps_client;

    if (packet_ptr == NX_NULL)
        return AIC_FTPS_PTR_ERROR;

    status = nx_secure_tls_packet_allocate(
             &(client_ptr->ftps_control_tls_session),
             ftps_client_ptr->nx_ftp_client_packet_pool_ptr, packet_ptr,
             wait_option);

    return status;
}

/**
 * Allocate a packet for sending FTPS Client Client data.
 * @handle: Ftps client handle.
 * @packet_ptr: allocated packet.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
static u32 ftps_client_data_tls_packet_allocate(u32 handle,
                                                NX_PACKET **packet_ptr,
                                                ulong wait_option)
{
    u32 status;

    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    ftps_client_ptr = client_ptr->ftps_client;

    if (packet_ptr == NX_NULL)
        return AIC_FTPS_PTR_ERROR;

    status = nx_secure_tls_packet_allocate(
             &(client_ptr->ftps_data_tls_session),
             ftps_client_ptr->nx_ftp_client_packet_pool_ptr, packet_ptr,
             wait_option);

    return status;
}

#endif

#ifdef CONFIG_CRYPTO_MBEDTLS

static void ftps_debug(void *ctx, int level, const char *file, int line,
                       const char *str)
{
    ((void) level);

    //alog_info("%s:%04d: %s\n", file, line, str);
    //alog_info("%04d: %s\n", line, str);
    return;
}

static void ftps_mbed_ctrl_ssl_reset(AIC_FTPS_CLIENT *client_ptr)
{
    mbedtls_ssl_free(&client_ptr->mb_cfg.ctrl_ssl);
    mbedtls_ssl_config_free(&client_ptr->mb_cfg.conf);
    mbedtls_ctr_drbg_free(&client_ptr->mb_cfg.ctr_drbg);
    mbedtls_entropy_free(&client_ptr->mb_cfg.entropy);

    return;
}

static void ftps_mbed_data_ssl_reset(AIC_FTPS_CLIENT *client_ptr)
{
    mbedtls_ssl_free(&client_ptr->mb_cfg.data_ssl);

    return;
}

static u32 ftps_client_packet_allocate(NX_FTP_CLIENT *client_ptr,
                                       NX_PACKET **packet_ptr,
                                       ULONG wait_option)
{
    u32 packet_type;
    u32 status;

    packet_type = NX_IPv4_TCP_PACKET;

    if (client_ptr->nx_ftp_client_control_socket.nx_tcp_socket_connect_ip.nxd_ip_version ==
        NX_IP_VERSION_V6) {

#ifndef FEATURE_NX_IPV6
        return NX_NOT_ENABLED;
#else
        packet_type = NX_IPv6_TCP_PACKET;
#endif
    }

    status = nx_packet_allocate(client_ptr->nx_ftp_client_packet_pool_ptr,
                                packet_ptr, packet_type, wait_option);

    return status;
}
#endif /* CONFIG_CRYPTO_MBEDTLS */

static u32 ftps_tls_setup(AIC_FTPS_CLIENT *client_ptr)
{

#ifdef CONFIG_CRYPTO_MBEDTLS
    int ret;
    const char *pers = "aic_ftps_client";
#endif

#ifdef CONFIG_NET_SECURE
    u32 status = ftps_client_control_tls_session_set(client_ptr);
    if (status) {
        aicx_error(0x1d6331, "x", "ctrl tls session set fail(0x%x)\n", status);
        return status;
    }

    status = ftps_client_control_tls_remote_crt_set(client_ptr);
    if (status != AIC_SUCCESS) {
        aicx_error(0x0b12bc, "x", "ctrl remote crt set fail(0x%x)\n", status);
        return status;
    }

    status = ftps_client_data_tls_session_set(client_ptr);
    if (status) {
       aicx_error(0x038e0a, "x", "data tls session set fail(0x%x)\n", status);
       return status;
    }

    status = ftps_client_data_tls_remote_crt_set(client_ptr);
    if (status != AIC_SUCCESS) {
       aicx_error(0x001684, "x", "data remote crt set fail(0x%x)\n", status);
       return status;
    }
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    /* Initialize the RNG and the session data */
    mbedtls_ssl_init(&client_ptr->mb_cfg.ctrl_ssl);
    mbedtls_ssl_config_init(&client_ptr->mb_cfg.conf);
    mbedtls_ctr_drbg_init(&client_ptr->mb_cfg.ctr_drbg);
    mbedtls_entropy_init(&client_ptr->mb_cfg.entropy);
    mbedtls_ctr_drbg_seed(&client_ptr->mb_cfg.ctr_drbg,
                          mbedtls_entropy_func, &client_ptr->mb_cfg.entropy,
                          (const unsigned char *) pers, strlen(pers));

    ret = mbedtls_ssl_config_defaults(&client_ptr->mb_cfg.conf,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x14d4c2, "x", "tls default config fail(0x%x)\n", ret);
        return ret;
    }

    mbedtls_ssl_conf_authmode(&client_ptr->mb_cfg.conf,
                              MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_rng(&client_ptr->mb_cfg.conf, mbedtls_ctr_drbg_random,
                         &client_ptr->mb_cfg.ctr_drbg);
    aic_ssl_set_recv_waitoption(client_ptr->mb_cfg.ctrl_socket_fd.fd,
                                FTPS_TIME_OUT);
    mbedtls_ssl_conf_dbg(&client_ptr->mb_cfg.conf, ftps_debug, NULL);
    mbedtls_ssl_conf_ca_chain(&client_ptr->mb_cfg.conf,
                              &(client_ptr->trusted_crt), NULL);

    ret = mbedtls_ssl_setup(&client_ptr->mb_cfg.ctrl_ssl,
                            &client_ptr->mb_cfg.conf);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0f53eb, "d", "ftps mbed ssl setup fail(0x%d)\n", ret);
        return ret;
    }

    mbedtls_ssl_set_bio(&client_ptr->mb_cfg.ctrl_ssl,
                        &client_ptr->mb_cfg.ctrl_socket_fd,
                        mbedtls_net_send_ext, mbedtls_net_recv_ext,
                        mbedtls_net_recv_timeout_ext);
#endif

    return AIC_SUCCESS;
}


static u32 ftps_client_tls_receive(AIC_FTPS_CLIENT *client_ptr,
                                   NX_PACKET **packet_ptr, u8 socket_type,
                                   ulong wait_option)
{
    u32 status =AIC_SUCCESS;

#ifdef CONFIG_NET_SECURE
    status = nx_secure_tls_session_receive(
             &(client_ptr->ftps_control_tls_session), packet_ptr,
             wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    if (socket_type == FTPS_CTRL_SOCKET) {
        aic_ssl_set_recv_waitoption(client_ptr->mb_cfg.ctrl_socket_fd.fd, wait_option);
        status = mbedtls_ssl_recv(&(client_ptr->mb_cfg.ctrl_ssl), packet_ptr);
    } else if (socket_type == FTPS_DATA_SOCKET) {
        aic_ssl_set_recv_waitoption(client_ptr->mb_cfg.data_socket_fd.fd, wait_option);
        status = mbedtls_ssl_recv(&(client_ptr->mb_cfg.data_ssl), packet_ptr);
    }
#endif

    return status;
}

static u32 ftps_client_tls_send(AIC_FTPS_CLIENT *client_ptr,
                                NX_PACKET *packet_ptr, ulong wait_option)
{
    u32 status;

#ifdef CONFIG_NET_SECURE
    status = nx_secure_tls_session_send(&(client_ptr->ftps_control_tls_session),
                                        packet_ptr, wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    ulong temp_len = packet_ptr->nx_packet_length;
    aic_ssl_set_send_waitoption(client_ptr->mb_cfg.ctrl_socket_fd.fd, wait_option);
    int res = mbedtls_ssl_send(&(client_ptr->mb_cfg.ctrl_ssl), packet_ptr);
    if (res <= 0 || res != temp_len) {
        status = NX_SIZE_ERROR;
    } else {
        status = AIC_SUCCESS;
    }
#endif

    return status;
}

static u32 ftps_client_data_tls_send(AIC_FTPS_CLIENT *client_ptr,
                                     NX_PACKET *packet_ptr, ulong wait_option)
{
    u32 status;

#ifdef CONFIG_NET_SECURE
    status = nx_secure_tls_session_send(&(client_ptr->ftps_data_tls_session),
                                        packet_ptr, wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    ulong temp_len = packet_ptr->nx_packet_length;
    aic_ssl_set_send_waitoption(client_ptr->mb_cfg.data_socket_fd.fd, wait_option);
    int res = mbedtls_ssl_send(&(client_ptr->mb_cfg.data_ssl), packet_ptr);
    if (res <= 0 || res != temp_len) {
        status = NX_SIZE_ERROR;
    } else {
        status = AIC_SUCCESS;
    }
#endif

    return status;
}

static void ftps_client_data_tls_error_exit(AIC_FTPS_CLIENT *client_ptr,
                                            u32 wait_option)
{
    u32 status;

#ifdef CONFIG_NET_SECURE
    status = nx_secure_tls_session_end(&(client_ptr->ftps_data_tls_session),
                                       wait_option);
    if (status)
        aicx_info(0x1ccbc2, "x", "data tls socket end fail(0x%x)\n", status);

    status = nx_secure_tls_session_delete(&(client_ptr->ftps_data_tls_session));
    if (status)
        aicx_info(0x1a3eaf, "x", "data tls socket delete fail(0x%x)\n", status);
#endif /* CONFIG_NET_SECURE */

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_ssl_close_notify(&client_ptr->mb_cfg.data_ssl);
    ftps_mbed_data_ssl_reset(client_ptr);
#endif
    status = nx_tcp_socket_disconnect(
                 &(client_ptr->ftps_client->nx_ftp_client_data_socket),
                 NX_NO_WAIT);

    if (client_ptr->ftps_client->nx_ftp_client_passive_transfer_enabled ==
        NX_FALSE) {
        status = nx_tcp_server_socket_unaccept(
                 &(client_ptr->ftps_client->nx_ftp_client_data_socket));
        if (status)
            aicx_error(0x0ba50c, "x", "data socket unaccept fail(0x%x)\n", status);

        nx_tcp_server_socket_unlisten(
        client_ptr->ftps_client->nx_ftp_client_ip_ptr,
        client_ptr->ftps_client->nx_ftp_client_data_socket.nx_tcp_socket_port);
        if (status)
            aicx_error(0x09a534, "x", "data socket unlisten fail(0x%x)\n", status);
    } else {
        status = nx_tcp_client_socket_unbind(
                 &(client_ptr->ftps_client->nx_ftp_client_data_socket));
        if (status)
            aicx_error(0x1743ba, "x", "data socket unbind fail(0x%x)\n", status);
    }

    return;
}

static void ftps_client_control_tls_error_exit(AIC_FTPS_CLIENT *client_ptr,
                                               u32 wait_option)
{
    u32 status;

#ifdef CONFIG_NET_SECURE
    status = nx_secure_tls_session_end(&(client_ptr->ftps_control_tls_session),
                                       wait_option);
    if (status)
        aicx_info(0x00bdb7, "x", "ctrl socket TLS end fail(0x%x)\n", status);

    status = nx_secure_tls_session_delete(
             &(client_ptr->ftps_control_tls_session));
    if (status)
        aicx_info(0x0a9b5e, "x", "ctrl socket TLS delete fail(0x%x)\n", status);

#endif /* CONFIG_NET_SECURE */

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_ssl_close_notify(&client_ptr->mb_cfg.ctrl_ssl);
    ftps_mbed_ctrl_ssl_reset(client_ptr);
#endif
    status = nx_tcp_socket_disconnect(
             &(client_ptr->ftps_client->nx_ftp_client_control_socket),
             wait_option);
    if (status)
        aicx_info(0x0af858, "x", "ctrl socket disconnect fail(0x%x)\n", status);

    status = nx_tcp_client_socket_unbind(
             &(client_ptr->ftps_client->nx_ftp_client_control_socket));
    if (status)
        aicx_error(0x19fca2, "x", "ctrl socket unbind fail(0x%x)\n", status);

    return;
}

/* Process URL to get host */
static u32 ftps_client_process_url(u32 handle, u8 *url, NXD_ADDRESS *server_ip)
{
    u8 *ftps_url_head = AIC_NULL;
    u8 *temp = AIC_NULL;
    const u8 *flag = "/";
    char *next = AIC_NULL;
    u8 *buf = AIC_NULL;
    u8 *v6_str;
    struct addrinfo *res;
    u32 status;
    u8 host[FTPS_MAX_HOST_LEN] = {'\0'};
    struct addrinfo *hints = AIC_NULL;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (strlen(url) == 0) {
        aicx_error(0x01dbc5, "", "FTPS url is empty\n");
        return AIC_FTPS_URL_EMPTY;
    }

    buf = (u8 *)aos_zalloc(FTPS_MAX_URL_LEN);
    if (buf == AIC_NULL) {
        aicx_error(0x08518b, "", "ftps url buf allocate fail\n");
        return AIC_FTPS_BUFFER_ALLOCATE_ERROR;
    }

    strncpy(buf, url, FTPS_MAX_URL_LEN);

    ftps_url_head = strtok_r(buf, flag, &next);
    if (strlen(ftps_url_head) == strlen(url)) {
        strncpy(host, url, FTPS_MAX_HOST_LEN);
    } else if (strncmp(ftps_url_head, "ftps", 4) == 0) {
        temp = strtok_r(AIC_NULL, flag, &next);
        strncpy(host, temp, FTPS_MAX_HOST_LEN);
    }

    aicx_debug(0x14dad8, "s", "host is %s\n", host);
    aos_free(buf);

    //DNS may be initialized when pdp is activated.
    status = aic_init_dns_client();
    if (status) {
        aicx_error(0x062052, "", "ftps init DNS fail\n");
        return status;
    }

    hints = aos_zalloc(sizeof(struct addrinfo));
    if (aic_get_pdp_ip_type(client_ptr->cid) == ANET_DEFAULT_IP_TYPE_V4)
        hints->ai_family = AF_INET;
    else if (aic_get_pdp_ip_type(client_ptr->cid) == ANET_DEFAULT_IP_TYPE_V6)
        hints->ai_family = AF_INET6;
    else
        hints->ai_family = AF_UNSPEC;

    if (strncmp(host, "[", 1) == 0) {
        aicx_info(0x02ed59, "", "ftps_url_host is ipv6 addr\n");
        v6_str = (u8 *)aos_zalloc(FTPS_MAX_HOST_LEN);
        if (v6_str == AIC_NULL) {
            aos_free(hints);
            aicx_error(0x1953cc, "", "v6_str buf allocate fail\n");
            return AIC_FTPS_BUFFER_ALLOCATE_ERROR;
        }

        strncpy(v6_str, host + 1, FTPS_MAX_HOST_LEN);
        v6_str[strlen(v6_str) - 1] = '\0';
        aicx_debug(0x0f3b04, "s", "v6_str is %s\n", v6_str);
        status = getaddrinfo(v6_str, NULL, hints, &res);
        aos_free(v6_str);
        v6_str = AIC_NULL;
    } else {
        status = getaddrinfo(host, NULL, hints, &res);
    }

    if (status == 0 && res) {
        if (res->ai_family == AF_INET) {
            server_ip->nxd_ip_version = NX_IP_VERSION_V4;
            server_ip->nxd_ip_address.v4 =
            ((struct sockaddr_in*)(res->ai_addr))->sin_addr.s_addr;

            NX_CHANGE_ULONG_ENDIAN(server_ip->nxd_ip_address.v4);
        } else if (res->ai_family == AF_INET6) {
            server_ip->nxd_ip_version = NX_IP_VERSION_V6;
            server_ip->nxd_ip_address.v6[0] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[0];

            server_ip->nxd_ip_address.v6[1] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[1];

            server_ip->nxd_ip_address.v6[2] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[2];

            server_ip->nxd_ip_address.v6[3] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[3];

            NX_CHANGE_ULONG_ENDIAN(server_ip->nxd_ip_address.v6[0]);
            NX_CHANGE_ULONG_ENDIAN(server_ip->nxd_ip_address.v6[1]);
            NX_CHANGE_ULONG_ENDIAN(server_ip->nxd_ip_address.v6[2]);
            NX_CHANGE_ULONG_ENDIAN(server_ip->nxd_ip_address.v6[3]);
        }

        freeaddrinfo(res);
        aos_free(hints);
    } else {
        aicx_error(0x0348c0, "x", "ftps DNS resolution fail(0x%x)\n", status);
        aos_free(hints);
        return status;
    }

    return AIC_SUCCESS;
}

static void ftps_client_control_error_exit(AIC_FTPS_CLIENT *client_ptr,
                                           u32 wait_option)
{
    u32 status;

    status = nx_tcp_socket_disconnect(
             &(client_ptr->ftps_client->nx_ftp_client_control_socket),
             wait_option);
    if (status)
        aicx_info(0x0af858, "x", "ctrl socket disconnect fail(0x%x)\n", status);

    status = nx_tcp_client_socket_unbind(
             &(client_ptr->ftps_client->nx_ftp_client_control_socket));
    if (status)
        aicx_error(0x19fca2, "x", "ctrl socket unbind fail(0x%x)\n", status);

    return;
}


static u32 ftps_auth_cmd_send(u32 handle, NXD_ADDRESS *server_ip, u32 port,
                              ulong wait_option)
{
    u8 *buff_ptr;
    u32 status;
    NX_PACKET *packet_ptr;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    ftps_client = client_ptr->ftps_client;

    status = nx_tcp_client_socket_bind(
             &(ftps_client->nx_ftp_client_control_socket),
             NX_FTP_CLIENT_SOURCE_PORT, wait_option);

    if ((status != AIC_SUCCESS) && (status != NX_ALREADY_BOUND)) {
        aicx_error(0x16fc66, "x", " bind socket with port fail(0x%x)\n", status);
        return status;
    }

    status = nxd_tcp_client_socket_connect(
             &(ftps_client->nx_ftp_client_control_socket), server_ip, port,
             wait_option);

    if (status) {
        aicx_error(0x11a53e, "x", "connect to FTPS server fail(0x%x)\n", status);
        nx_tcp_client_socket_unbind(&(ftps_client->nx_ftp_client_control_socket));
        return status;
    }

    status = nx_tcp_socket_receive(&(ftps_client->nx_ftp_client_control_socket),
                                   &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x1d72e0, "x", "AUTH cmd no response packet received(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '2') ||
        (buff_ptr[1] != '2')) {
        aicx_error(0x11e063, "", "socket connect received no 220 message\n");
        status = AIC_FTPS_EXPECTED_22X_CODE;
        goto ERROR_OUT;
    }

    aos_memset(buff_ptr, 0, packet_ptr->nx_packet_length);
    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 10) {
        aicx_error(0x1c281d, "", "packt buf for AUTH is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    /* Now build the AUTH message. */
    buff_ptr[0] = 'A';
    buff_ptr[1] = 'U';
    buff_ptr[2] = 'T';
    buff_ptr[3] = 'H';
    buff_ptr[4] = ' ';

    if (client_ptr->auth_type == AUTH_SSL) {
        buff_ptr[5] = 'S';
        buff_ptr[6] = 'S';
        buff_ptr[7] = 'L';
    } else if (client_ptr->auth_type == AUTH_TLS) {
        buff_ptr[5] = 'T';
        buff_ptr[6] = 'L';
        buff_ptr[7] = 'S';
    }
    buff_ptr[8] = 13;
    buff_ptr[9]= 10;
    packet_ptr->nx_packet_length = 10;

    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;
    status = nx_tcp_socket_send(&(ftps_client->nx_ftp_client_control_socket),
                                packet_ptr, wait_option);
    if (status) {
        aicx_error(0x01e91b, "x", "Send AUTH command fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* check whether the server accept the security mechanism */
    status = nx_tcp_socket_receive(&(ftps_client->nx_ftp_client_control_socket),
                                   &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x194fd8, "x", "AUTH receive no response(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    if ((packet_ptr->nx_packet_length < 3) ||
        (buff_ptr[0] != '2' && buff_ptr[0] != '3') || (buff_ptr[1] != '3')) {
        aicx_error(0x181554, "", "received no 234 or 334 message\n");
        status = AIC_FTPS_EXPECTED_23X_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    if (status)
        ftps_client_control_error_exit(client_ptr, wait_option);

    return status;
}

/**
 * This function connects a previously created FTPS instance with the
 * FTPS server at the specified IP address.
 */
static u32 ftps_client_connect_internal(u32 handle, NXD_ADDRESS *server_ip,
                                        u8 *username, u8 *password, u32 port,
                                        u32 (*tls_setup)(AIC_FTPS_CLIENT *client_ptr),
                                        ulong wait_option)
{
    u8 *buffer_ptr;
    u32 i, status;
    NX_PACKET *tls_packet;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client;
    int ret;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    ftps_client = client_ptr->ftps_client;

    status = ftps_auth_cmd_send(handle, server_ip, port, wait_option);
    if (status) {
        aicx_error(0x197741, "x", "ftps auth cmd send fail(0x%x)\n", status);
        return status;
    }

    /* Set up TLS session information. */
    status = (*tls_setup)(client_ptr);
    if (status)
        return(status);

    /* Start TLS session after receive 234 or 334. */
#ifdef CONFIG_NET_SECURE
    status = nx_secure_tls_session_start(
             &(client_ptr->ftps_control_tls_session),
             &(ftps_client->nx_ftp_client_control_socket), wait_option);

    if (status != AIC_SUCCESS) {
        aicx_error(0x02f014, "x", "ctrl tls session start fail(0x%x)\n", status);
        goto ERROR_OUT;
    }
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    do ret = mbedtls_ssl_handshake(&client_ptr->mb_cfg.ctrl_ssl);
        while( ret == MBEDTLS_ERR_SSL_WANT_READ ||
               ret == MBEDTLS_ERR_SSL_WANT_WRITE );

        if (ret != AIC_SUCCESS) {
            aicx_error(0x17aef2, "d", "ctrl mbed tls handshake fail(%d)\n", ret);
            status = AIC_FTPS_MBED_HANDSHAKE_FAIL;
            goto ERROR_OUT;
        }
#endif

    /**
     * At this point, the tls is successfully enabled.
     * The data transmission will be protected.
     */
    /* Allocate a tls packet for sending the USER. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &tls_packet,
                                                     wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(ftps_client, &tls_packet, wait_option);
#endif
    if (status) {
        aicx_error(0x0c2ffc, "x", "packet allocate for USER fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    if (((!username) && ((u32)(tls_packet->nx_packet_data_end -
                               tls_packet->nx_packet_prepend_ptr) < 11)) ||
        ((username) && ((u32)(tls_packet->nx_packet_data_end -
                              tls_packet->nx_packet_prepend_ptr) < 7))) {
        aicx_error(0x19fbfe, "", "username out of boundary\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buffer_ptr = tls_packet->nx_packet_prepend_ptr;
    buffer_ptr[0] = 'U';
    buffer_ptr[1] = 'S';
    buffer_ptr[2] = 'E';
    buffer_ptr[3] = 'R';
    buffer_ptr[4] = ' ';

    /* Determine if a user name was supplied.If No, use "USER" */
    if (username == NX_NULL) {
        buffer_ptr[5] = 'U';
        buffer_ptr[6] = 'S';
        buffer_ptr[7] = 'E';
        buffer_ptr[8] = 'R';
        buffer_ptr[9] = 13;
        buffer_ptr[10]= 10;

        tls_packet->nx_packet_length = 11;
    } else {
        for (i = 0; username[i]; i++) {
            if ((i + 7) >= (u32)(tls_packet->nx_packet_data_end -
                                 tls_packet->nx_packet_prepend_ptr)) {
                aicx_error(0x0dea0c, "", "username len out of boundary\n");
                status = AIC_FTPS_FAILED;
                goto ERROR_OUT;
            }
            buffer_ptr[5 + i] = (u8)username[i];
        }
        buffer_ptr[5 + i] = 13;
        buffer_ptr[5 + i + 1] = 10;

        tls_packet->nx_packet_length = i + 7;
    }
    tls_packet->nx_packet_append_ptr = tls_packet->nx_packet_prepend_ptr +
                                       tls_packet->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, tls_packet, wait_option);
    if (status) {
        aicx_error(0x0decad, "x", "Send USER command fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /**
     * There may be multiple 220 responses from FTPS server
     * after establishing connection.
     * Flush mutiple 220 responses and wait for 331 response from the FTPS server.
     */
    while (1) {
        status = ftps_client_tls_receive(client_ptr, &tls_packet,
                                         FTPS_CTRL_SOCKET, wait_option);
        if (status) {
            aicx_error(0x18a736, "x", "Flush fail(0x%x)\n", status);
            goto ERROR_OUT;
        }

#ifndef NX_DISABLE_PACKET_CHAIN
        if (tls_packet->nx_packet_next) {
            status = AIC_FTPS_INVALID_PACKET;
            goto ERROR_OUT;
        }
#endif
        buffer_ptr = tls_packet->nx_packet_prepend_ptr;
        if ((tls_packet->nx_packet_length >= 3) && (buffer_ptr[0] == '2') &&
            (buffer_ptr[1] == '2')) {
            aicx_info(0x176fea, "", "received 220 message, continue\n");
            aic_ftps_packet_release(&tls_packet);
            continue;
        }

        /* Check for 331 or 232 message. */
        if ((tls_packet->nx_packet_length < 3) ||
            (buffer_ptr[0] != '3' && buffer_ptr[0] != '2') ||
            (buffer_ptr[1] != '3')) {
            aicx_error(0x067e7e, "", "user cmd received no 331 or 232 message\n");
            status = AIC_FTPS_EXPECTED_33X_CODE;
            goto ERROR_OUT;
        } else {
            break;
        }
    }

    aos_memset(buffer_ptr, 0, tls_packet->nx_packet_length);
    /* Check if out of boundary. */
    if (((!password) && ((u32)(tls_packet->nx_packet_data_end -
                               tls_packet->nx_packet_prepend_ptr) < 11)) ||
        ((password) && ((u32)(tls_packet->nx_packet_data_end -
                              tls_packet->nx_packet_prepend_ptr) < 7))) {
        aicx_error(0x0a8adf, "", "password out of boundary\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buffer_ptr[0] = 'P';
    buffer_ptr[1] = 'A';
    buffer_ptr[2] = 'S';
    buffer_ptr[3] = 'S';
    buffer_ptr[4] = ' ';

    /* Determine if a password was specified.If no, use "PASS". */
    if (password == NX_NULL) {
        buffer_ptr[5] = 'P';
        buffer_ptr[6] = 'A';
        buffer_ptr[7] = 'S';
        buffer_ptr[8] = 'S';
        buffer_ptr[9] = 13;
        buffer_ptr[10]= 10;

        tls_packet->nx_packet_length = 7;
    } else {
        for (i = 0; password[i]; i++) {
            /* Check if out of boundary. */
            if ((i + 7) >= (u32)(tls_packet->nx_packet_data_end -
                                 tls_packet->nx_packet_prepend_ptr)) {
                aicx_error(0x013c4b, "", "password len out of boundary\n");
                status = AIC_FTPS_FAILED;
                goto ERROR_OUT;
            }
            buffer_ptr[5 + i] = (u8)password[i];
        }
        buffer_ptr[5 + i] = 13;
        buffer_ptr[5 + i + 1] = 10;

        tls_packet->nx_packet_length = i + 7;
    }
    tls_packet->nx_packet_append_ptr = tls_packet->nx_packet_prepend_ptr +
                                       tls_packet->nx_packet_length;

    /* Send the password to the FTPS server. */
     status = ftps_client_tls_send(client_ptr, tls_packet, wait_option);
    if (status) {
         aicx_error(0x0c23e4, "x", "Send PASS command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &tls_packet, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x14ba1e, "x", "FTPS_PASS receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* check response code */
    buffer_ptr = tls_packet->nx_packet_prepend_ptr;
    if ((tls_packet->nx_packet_length < 3) || (buffer_ptr[0] != '2') ||
        (buffer_ptr[1] != '3')) {
        aicx_error(0x0d23df, "", "FTPS_PASS received no 230 message\n");
        status = AIC_FTPS_EXPECTED_23X_CODE;
        goto ERROR_OUT;
    }

    aos_memset(buffer_ptr, 0, tls_packet->nx_packet_length);
    /* Check if out of boundary. */
    if ((u32)(tls_packet->nx_packet_data_end -
        tls_packet->nx_packet_prepend_ptr) < 8) {
        aicx_error(0x0140c5, "", "packt buf for PBSZ is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buffer_ptr[0] = 'P';
    buffer_ptr[1] = 'B';
    buffer_ptr[2] = 'S';
    buffer_ptr[3] = 'Z';
    buffer_ptr[4] = ' ';
    buffer_ptr[5] = '0';
    buffer_ptr[6] = 13;
    buffer_ptr[7] = 10;
    tls_packet->nx_packet_length = 8;
    tls_packet->nx_packet_append_ptr = tls_packet->nx_packet_prepend_ptr +
                                       tls_packet->nx_packet_length;

    /* send PBSZ command */
     status = ftps_client_tls_send(client_ptr, tls_packet, wait_option);
    if (status) {
         aicx_error(0x1df5a3, "x", "Send PBSZ command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }
    status = ftps_client_tls_receive(client_ptr, &tls_packet, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x07c2e1, "x", "FTPS_PBSZ receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* check response code */
    buffer_ptr = tls_packet->nx_packet_prepend_ptr;
    if (tls_packet->nx_packet_length < 3 || buffer_ptr[0] != '2' ||
        buffer_ptr[1] != '0' || buffer_ptr[2] != '0') {
        aicx_error(0x00ca54, "", "FTPS_PBSZ received no 200 message\n");
        status = AIC_FTPS_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

    aos_memset(buffer_ptr, 0, tls_packet->nx_packet_length);
    /* Check if out of boundary. */
    if ((u32)(tls_packet->nx_packet_data_end -
        tls_packet->nx_packet_prepend_ptr) < 7) {
        aicx_error(0x1174a0, "", "packt buf for PORT is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buffer_ptr[0] = 'P';
    buffer_ptr[1] = 'R';
    buffer_ptr[2] = 'O';
    buffer_ptr[3] = 'T';
    buffer_ptr[4] = ' ';

    if (client_ptr->prot_level == PROT_CLEAR) {
        buffer_ptr[5] = 'C';
    } else if (client_ptr->prot_level == PROT_PRIVATE) {
        buffer_ptr[5] = 'P';
    }
    buffer_ptr[6] = 13;
    buffer_ptr[7] = 10;
    tls_packet->nx_packet_length = 8;
    tls_packet->nx_packet_append_ptr = tls_packet->nx_packet_prepend_ptr +
                                       tls_packet->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, tls_packet, wait_option);
    if (status) {
         aicx_error(0x1ac062, "x", "Send PROT command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &tls_packet, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x146118, "x", "FTPS_PROT receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buffer_ptr = tls_packet->nx_packet_prepend_ptr;
    if (tls_packet->nx_packet_length < 3 || buffer_ptr[0] != '2' ||
        buffer_ptr[1] != '0' || buffer_ptr[2] != '0') {
        aicx_error(0x1cc9b0, "", "FTPS_PROT received no 200 message\n");
        status = AIC_FTPS_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

    ftps_client->nx_ftp_client_state = NX_FTP_STATE_CONNECTED;

ERROR_OUT:
    aic_ftps_packet_release(&tls_packet);
    if (status)
        ftps_client_control_tls_error_exit(client_ptr, wait_option);

    return status;
}

/* Send PASV command and connect to the port specifitied by the server. */
static u32 ftps_client_passive_transfer_set(u32 handle, ulong wait_option)
{
    u8 *res_buf;
    u8 send_buf[24];
    NX_PACKET *packet_ptr;
    u32 status;
    u32 i;
    u32 data_port;
#ifndef NX_DISABLE_IPV4
    u32 commas = 0;
    ulong ip_address = 0;
    ulong temp = 0;
#endif /* NX_DISABLE_IPV4  */
    NX_TCP_SOCKET *control_socket;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    ftps_client = client_ptr->ftps_client;

    /* Allocate a packet for sending the PASV command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(ftps_client, &packet_ptr, wait_option);
#endif

    if (status) {
        aicx_error(0x1a4eb7, "x", "packet allocate for PASV fail(0x%x)\n", status);
        return status;;
    }

    /* Check if out of boundary.  */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr ->nx_packet_prepend_ptr) < 6) {
        aicx_error(0x1d6459, "", "packt buf for PASV is too short\n");
        aic_ftps_packet_release(&packet_ptr);
        return AIC_FTPS_FAILED;
    }

    control_socket = &(ftps_client->nx_ftp_client_control_socket);
    if (control_socket->nx_tcp_socket_connect_ip.nxd_ip_version ==
        NX_IP_VERSION_V6) {
#ifndef FEATURE_NX_IPV6
        return AIC_FTPS_NOT_ENABLED;
#else
        /* Now build "EPSV" message. RFC2428. */
        send_buf[0] = 'E';
        send_buf[1] = 'P';
        send_buf[2] = 'S';
        send_buf[3] = 'V';
#endif /*  FEATURE_NX_IPV6 */
    } else {
#ifdef NX_DISABLE_IPV4
        return AIC_FTPS_NOT_ENABLED;
#else
        /* Now build "PASV" message. */
        send_buf[0] = 'P';
        send_buf[1] = 'A';
        send_buf[2] = 'S';
        send_buf[3] = 'V';
#endif /* NX_DISABLE_IPV4 */
    }
    send_buf[4] = 13;
    send_buf[5] = 10;

   status = nx_packet_data_append(packet_ptr, send_buf, 6,
            client_ptr->ftps_client->nx_ftp_client_packet_pool_ptr, wait_option);
    if (status != AIC_SUCCESS) {
        aicx_error(0x108475, "x", "packet append for PASV fail(0x%x)\n", status);
        aic_ftps_packet_release(&packet_ptr);
        return status;
    }

    /* Send the PASV or EPSV message. */
    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status != AIC_SUCCESS) {
        aicx_error(0x1ab181, "x", "Send PASV command fail(0x%x)\n", status);
        aic_ftps_packet_release(&packet_ptr);
        return status;
    }

    /* Wait for response from the FTPS server. */
    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status != AIC_SUCCESS) {
        aicx_error(0x1993c4, "x", "FTPS_PASV receive res fail(0x%x)\n", status);
        return status;
    }

    /* Setup pointer to the buffer area. */
    res_buf = packet_ptr->nx_packet_prepend_ptr;
    control_socket = &(ftps_client->nx_ftp_client_control_socket);
    if (control_socket->nx_tcp_socket_connect_ip.nxd_ip_version ==
        NX_IP_VERSION_V6) {
#ifndef FEATURE_NX_IPV6
        return AIC_FTPS_NOT_ENABLED;
#else
        /* EPSV command, RFC2428. */
        /* Check for 2xx message, signaling the "EPSV" was received properly. */
        if ((packet_ptr->nx_packet_length < 3) || (res_buf[0] != '2') ||
            (res_buf[1] != '2') || (res_buf[2] != '9')) {
            aic_ftps_packet_release(&packet_ptr);
            return AIC_FTPS_EXPECTED_2XX_CODE;
        }

        /* Check the length. command + <sp> + passive key word "(|||"*/
        if (packet_ptr->nx_packet_length < 8) {
            aic_ftps_packet_release(&packet_ptr);
            return AIC_FTPS_INVALID_COMMAND;
        }

        /* Set the index. */
        i = 7;

        /* Skip the text to locate the key word "(|||". */
        while (i < packet_ptr->nx_packet_length) {
            /* Check if match the key word.  */
            if ((res_buf[i - 3] == '(') && (res_buf[i - 2] == '|') &&
                (res_buf[i - 1] == '|') && (res_buf[i] == '|')) {
                break;
            }
            i++;
        }

        if (i >= packet_ptr->nx_packet_length) {
            aic_ftps_packet_release(&packet_ptr);
            return AIC_FTPS_INVALID_COMMAND;
        }

        data_port =  0;
        i++;
        while ((i < packet_ptr->nx_packet_length) && (res_buf[i] != '|')) {
            /* Is a numeric character present? */
            if ((res_buf[i] < '0') || (res_buf[i] > '9'))
                break;

            data_port = (data_port * 10) + (u32) (res_buf[i] - '0');

            /* Move to next character.  */
            i++;
        }

        /* At this point we are done with the packet. */
        aic_ftps_packet_release(&packet_ptr);

        /* Determine if an error occurred. */
        if ((res_buf[i] != '|') || (data_port == 0))
            return AIC_FTPS_INVALID_COMMAND;

        /* Save the passive IP address and  data port. */
        COPY_NXD_ADDRESS(
        &(ftps_client->nx_ftp_client_control_socket.nx_tcp_socket_connect_ip),
        &(ftps_client->nx_ftp_client_data_socket.nx_tcp_socket_connect_ip));

        ftps_client->nx_ftp_client_data_socket.nx_tcp_socket_connect_port = data_port;
#endif /*  FEATURE_NX_IPV6 */
    } else {
#ifdef NX_DISABLE_IPV4
        return AIC_FTPS_NOT_ENABLED;
#else
        /* Check for 2xx message, signaling the "PASV" was received properly. */
        if ((packet_ptr->nx_packet_length < 3) || (res_buf[0] != '2') ||
            (res_buf[1] != '2') || (res_buf[2] != '7')) {
            aic_ftps_packet_release(&packet_ptr);
            return AIC_FTPS_EXPECTED_22X_CODE;
        }

        /* Now parse the address and port for the Client to connect to on the
         * data socket. IP address and port are comma delimited and
         * port number is the last two numbers, PASV port = (p1 * 256) + p2.
         * Skip non numeric text and whitespace. */
        i = 3;
        while (((res_buf[i] > '9') || (res_buf[i] < '0')) &&
                (i < packet_ptr->nx_packet_length)) {
            i++;
        }

        /* Check for properly formated command from server. */
        if (i >= packet_ptr->nx_packet_length) {
            aic_ftps_packet_release(&packet_ptr);
            return AIC_FTPS_INVALID_COMMAND;
        }

        /* First, pickup the IP address. */
        commas = 0;
        ip_address = 0;
        temp = 0;
        status = AIC_SUCCESS;

        while (i < packet_ptr->nx_packet_length) {
            /* Is a numeric character present? */
            if ((res_buf[i] >= '0') && (res_buf[i] <= '9')) {
                /* Yes, numeric character is present.  Update the IP address. */
                temp =  (temp*10) + (ULONG) (res_buf[i] - '0');
            }

            /* Determine if a CR/LF is present. */
            if ((res_buf[i] == 0x0D) || (res_buf[i] == 0x0A) ||
                (res_buf[i] == 0)) {
                status = AIC_FTPS_INVALID_COMMAND;
                break;
            }

            /* Determine if a comma is present. */
            if (res_buf[i] == ',') {
                /* Increment the comma count. */
                commas++;
                /* Setup next byte of IP address. */
                ip_address =  (ip_address << 8) & 0xFFFFFFFF;
                ip_address =  ip_address | (temp & 0xFF);
                temp =  0;

                /* Have we finished with the IP address? */
                if (commas == 4) {
                    /* Finished with IP address. */
                    i++;
                    break;
                }
            }
            /* Move to next character. */
            i++;
        }

        /* Now pickup the port number. */
        data_port =  0;
        temp =  0;
        while (i < packet_ptr->nx_packet_length) {
            /* Is a numeric character present?  */
            if ((res_buf[i] >= '0') && (res_buf[i] <= '9')) {
                /* Yes, numeric character is present.  Update the IP port. */
                temp =  (temp*10) + (UINT) (res_buf[i] - '0');
            }

            /* Determine if a CR/LF is present. */
            if ((res_buf[i] == 0x0D) || (res_buf[i] == 0x0A) ||
                (res_buf[i] == 0)) {
                /* Good condition on the port number! */
                break;
            }

            /* Determine if a comma is present. */
            if (res_buf[i] == ',') {
                /* Increment the comma count. */
                commas++;

                /* Move the data port number up. */
                data_port =  (data_port << 8) & 0xFFFFFFFF;
                data_port =  data_port | (temp & 0xFF);
                temp =  0;

                /* Have we finished with the port? */
                if (commas >= 6) {
                    /* Error, get out of the loop. */
                    status =  NX_FTP_INVALID_ADDRESS;
                    break;
                }
            }
            /* Move to next character. */
            i++;
        }
        /* Move port number up. */
        data_port =  (data_port << 8) & 0xFFFFFFFF;
        data_port =  data_port | (temp & 0xFF);

        /* At this point we are done with the packet. */
        aic_ftps_packet_release(&packet_ptr);

        /* Determine if an error occurred.  */
        if ((status != AIC_SUCCESS) || (res_buf[i] != 13) || (commas != 5) ||
            (ip_address == 0) || (data_port == 0))
            return AIC_FTPS_INVALID_COMMAND;

        /* Save the passive IP address and  data port. */
        ftps_client->nx_ftp_client_data_socket.nx_tcp_socket_connect_ip.nxd_ip_version = NX_IP_VERSION_V4;
        ftps_client->nx_ftp_client_data_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v4 = ip_address;
        ftps_client->nx_ftp_client_data_socket.nx_tcp_socket_connect_port =  data_port;

#endif /* NX_DISABLE_IPV4 */
    }

    /* Bind the client data socket to any port. */
    status = nx_tcp_client_socket_bind(
             (&ftps_client->nx_ftp_client_data_socket), NX_ANY_PORT,
             wait_option);

    /* Check for an error. */
    if (status != AIC_SUCCESS)
        return status;

    /* Now connect to the IP address and port the server asked us to. */
    status = nxd_tcp_client_socket_connect(
             &(ftps_client->nx_ftp_client_data_socket),
             &(ftps_client->nx_ftp_client_data_socket.nx_tcp_socket_connect_ip),
             data_port, wait_option);

    /* Check for an error. */
    if (status != AIC_SUCCESS) {
        nx_tcp_client_socket_unbind(&(ftps_client->nx_ftp_client_data_socket));
    }

    return status;
}

/* Send PORT command and listen the port, waitting connection from server. */
static u32 ftps_client_active_transfer_set(u32 handle, ulong wait_option)
{
    u32 status;
    u32 data_port;
#ifndef NX_DISABLE_IPV4
    ulong ip_address;
#endif /* NX_DISABLE_IPV4 */
    u8 *res_buf;
    u8 send_buf[32];
    NX_PACKET *packet_ptr;
#ifdef FEATURE_NX_IPV6
    u32 ipduo_size;
#endif
    NX_TCP_SOCKET *control_socket;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    ftps_client = client_ptr->ftps_client;

    /* Pickup the next free port for the data socket. */
    status = nx_tcp_free_port_find(ftps_client->nx_ftp_client_ip_ptr,
                                   ftps_client->nx_ftp_client_data_port + 1,
                                   &data_port);
    if (status != AIC_SUCCESS) {
        aicx_error(0x0dc184, "x", "active type free port find fail(0x%x)\n", status);
        return status;
    }

    /* Save the data port to the client record. */
    ftps_client->nx_ftp_client_data_port = data_port;

    /* Start listening on the data port. */
    status = nx_tcp_server_socket_listen(ftps_client->nx_ftp_client_ip_ptr,
             data_port, &(ftps_client->nx_ftp_client_data_socket), 5, NX_NULL);

    /* Determine if the listen is successful. */
    if (status != AIC_SUCCESS) {
        aicx_error(0x10375a, "x", "active type socket listen fail(0x%x)\n", status);
        return status;
    }

    /* Allocate a packet for sending the PORT/EPRT command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(ftps_client, &packet_ptr, wait_option);
#endif

    if (status) {
        aicx_error(0x0886dc, "x", "active type packet allocate fail(0x%x)\n", status);
        nx_tcp_server_socket_unlisten(ftps_client->nx_ftp_client_ip_ptr,
                                      data_port);
        return status;
    }

    /* Now build the IP and port number FTPS message. */
    control_socket = &(ftps_client->nx_ftp_client_control_socket);
    if (control_socket->nx_tcp_socket_connect_ip.nxd_ip_version ==
        NX_IP_VERSION_V6) {
#ifndef FEATURE_NX_IPV6
        return AIC_FTPS_NOT_ENABLED;
#else
        char ipduo_buffer[NX_FTP_IPV6_ADDRESS_BUFSIZE];
        u32 index;

        /* EPRT command: EPRT |2|1080::8:800:200C:417A|5282|
           RFC2428, Section2, Page3. */

        /* Check if out of boundary.
           Max EPRT command sring: 8 ("EPRT |2|") + 39 (IPv6 address string) +
           1 ('|') + 5 (Max port string) + 1 ('|') + 2 ("\r\n") = 56. */
        if ((u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr) < 56) {
            aicx_error(0x190522, "", "packet buf is too short for EPRT\n");
            nx_tcp_server_socket_unlisten(ftps_client->nx_ftp_client_ip_ptr,
                                          data_port);
            aic_ftps_packet_release(&packet_ptr);
            return AIC_FTPS_INVALID_PACKET;
        }

        send_buf[0] =  'E';
        send_buf[1] =  'P';
        send_buf[2] =  'R';
        send_buf[3] =  'T';
        send_buf[4] =  ' ';
        send_buf[5] =  '|';
        send_buf[6] =  '2';
        send_buf[7] =  '|';

        /* Convert the PORT command ipv6 address and port number format
           to ascii text. */

        /* Clear our scratch buffer. */
        memset(&ipduo_buffer[0], 0, NX_FTP_IPV6_ADDRESS_BUFSIZE);

        /* Convert the IPv6 address to text. */
        status = ftps_utility_convert_IPv6_to_ascii(
                 &(ftps_client->nx_ftp_client_control_socket),
                 &ipduo_buffer[0], NX_FTP_IPV6_ADDRESS_BUFSIZE, &ipduo_size);

        /* Append to the packet buffer. */
        memcpy(send_buf + 8, ipduo_buffer, ipduo_size);

        /* Update the index past the IPv6 address. */
        index = 8 + ipduo_size;

        /* Add the required space character. */
        send_buf[index++] = '|';

        /* Clear the scratch buffer again. */
        memset(&ipduo_buffer[0], 0, NX_FTP_IPV6_ADDRESS_BUFSIZE);

        /* Convert the preferred port to ascii. */
        status = ftps_utility_convert_portnumber_ascii(data_port,
                                                       &ipduo_buffer[0],
                                                       &ipduo_size);

        /* Append to the packet buffer. */
        memcpy(send_buf + index, ipduo_buffer, ipduo_size);

        /* Update the index past the port. */
        index += ipduo_size;

        /* Add the required space character. */
        send_buf[index++] = '|';

        /* Set the CR/LF. */
        send_buf[index++] = 13;
        send_buf[index++] = 10;
        status = nx_packet_data_append(packet_ptr, send_buf, index,
                 client_ptr->ftps_client->nx_ftp_client_packet_pool_ptr,
                 wait_option);
        if (status != AIC_SUCCESS) {
            aicx_error(0x05442c, "x", "packet append for EPRT fail(0x%x)\n", status);
            return status;
        }
#endif /*  FEATURE_NX_IPV6 */
    }
    else {
        /* IPv4 FTPS connection */
#ifdef NX_DISABLE_IPV4
        return AIC_FTPS_NOT_ENABLED;
#else
        /* Check if out of boundary. */
        if ((u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr) < 30) {
            aicx_error(0x190522, "", "packet buf is too short for EPRT\n");
            nx_tcp_server_socket_unlisten(ftps_client ->nx_ftp_client_ip_ptr,
                                          data_port);
            aic_ftps_packet_release(&packet_ptr);
            return AIC_FTPS_INVALID_PACKET;
        }

        /* Pickup the IPv4 address of this IP instance. */
        control_socket = &(ftps_client->nx_ftp_client_control_socket);
        ip_address =
        control_socket->nx_tcp_socket_connect_interface->nx_interface_ip_address;

        /* Now build the IP and port number FTPS message. */
        send_buf[0] =  'P';
        send_buf[1] =  'O';
        send_buf[2] =  'R';
        send_buf[3] =  'T';
        send_buf[4] =  ' ';

        send_buf[5] =  (u8)('0' + (ip_address >> 24)/100);
        send_buf[6] =  (u8)('0' + ((ip_address >> 24)/10)%10);
        send_buf[7] =  (u8)('0' + (ip_address >> 24)%10);
        send_buf[8] =  ',';

        send_buf[9]  = (u8)('0' + ((ip_address >> 16) & 0xFF)/100);
        send_buf[10] = (u8)('0' + (((ip_address >> 16) & 0xFF)/10)%10);
        send_buf[11] = (u8)('0' + ((ip_address >> 16) & 0xFF)%10);
        send_buf[12] = ',';

        send_buf[13] = (u8)('0' + ((ip_address >> 8) & 0xFF)/100);
        send_buf[14] = (u8)('0' + (((ip_address >> 8) & 0xFF)/10)%10);
        send_buf[15] = (u8)('0' + ((ip_address >> 8) & 0xFF)%10);
        send_buf[16] = ',';

        send_buf[17] = (u8)('0' + (ip_address & 0xFF)/100);
        send_buf[18] = (u8)('0' + ((ip_address & 0xFF)/10)%10);
        send_buf[19] = (u8)('0' + (ip_address & 0xFF)%10);
        send_buf[20] = ',';

        send_buf[21] = (u8)('0' + (data_port >> 8)/100);
        send_buf[22] = (u8)('0' + ((data_port >> 8)/10)%10);
        send_buf[23] = (u8)('0' + ((data_port >> 8)%10));
        send_buf[24] = ',';

        send_buf[25] = (u8)('0' + (data_port & 255)/100);
        send_buf[26] = (u8)('0' + ((data_port & 255)/10)%10);
        send_buf[27] = (u8)('0' + ((data_port & 255)%10));

        /* Set the CR/LF. */
        send_buf[28] = 13;
        send_buf[29] = 10;
        status = nx_packet_data_append(packet_ptr, send_buf, 30,
                 client_ptr->ftps_client->nx_ftp_client_packet_pool_ptr,
                 wait_option);
        if (status != AIC_SUCCESS) {
            aicx_error(0x195884, "x", "packet append for PORT fail(0x%x)\n", status);
            return status;
        }
#endif /* NX_DISABLE_IPV4 */
    }

    /* Send the PORT/EPRT message. */
    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status != AIC_SUCCESS) {
        aicx_error(0x067cd9, "x", "Send PORT command fail(0x%x)\n", status);
        aic_ftps_packet_release(&packet_ptr);
        return status;
    }

    /* Determine if the send was unsuccessful. */
    if (status != AIC_SUCCESS) {
        /* Stop listening on the data port. */
        nx_tcp_server_socket_unlisten(ftps_client->nx_ftp_client_ip_ptr,
                                      data_port);
        aic_ftps_packet_release(&packet_ptr);
        return status;
    }

    /* Wait for response from the FTPS server. */
    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);

    /* Determine if a packet was not received. */
    if (status != AIC_SUCCESS) {
        nx_tcp_server_socket_unlisten(ftps_client->nx_ftp_client_ip_ptr,
                                      data_port);
        return status;
    }

    /* We have a packet, setup pointer to the buffer area. */
    res_buf = packet_ptr->nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the IP/port was received properly. */
    if ((packet_ptr->nx_packet_length < 3) || (res_buf[0] != '2')) {
        nx_tcp_server_socket_unlisten(ftps_client->nx_ftp_client_ip_ptr,
                                      data_port);
        aic_ftps_packet_release(&packet_ptr);
        return AIC_FTPS_EXPECTED_2XX_CODE;
    }

    aic_ftps_packet_release(&packet_ptr);
    return AIC_SUCCESS;
}

static u32 ftps_client_data_socket_disconnect(u32 handle, ulong wait_option)
{
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    ftps_client = client_ptr->ftps_client;

    /* Ensure the client is in the proper state for closing the socket. */
    if (ftps_client->nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;
    else if ((ftps_client->nx_ftp_client_state != NX_FTP_STATE_OPEN) &&
             (ftps_client->nx_ftp_client_state != NX_FTP_STATE_WRITE_OPEN) &&
             (ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED))
        return AIC_FTPS_NOT_OPEN;

    /**
     * Disconnect the data socket.Do not judge the status,
     * ensure the every step has been executed.
     */
    ftps_client_data_tls_error_exit(client_ptr, wait_option);

    ftps_client->nx_ftp_client_block_total_size = 0;
    ftps_client->nx_ftp_client_block_remaining_size = 0;

    /* NX_FTP_STATE_CONNECTED means FTPS control socket is connected */
    ftps_client->nx_ftp_client_state = NX_FTP_STATE_CONNECTED;

    return AIC_SUCCESS;
}

static u32 ftps_cilent_data_socket_connect(u32 handle, u32 open_type,
                                           ulong wait_option)
{
    u32 status;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    ftps_client_ptr = client_ptr->ftps_client;

    if ((open_type != NX_FTP_OPEN_FOR_READ) &&
        (open_type != NX_FTP_OPEN_FOR_WRITE) &&
        (open_type != AIC_FTPS_OPEN_FOR_LIST))
        return AIC_FTPS_OPTION_ERROR;

    if (ftps_client_ptr->nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;
    else if (ftps_client_ptr->nx_ftp_client_state == NX_FTP_STATE_OPEN)
        return AIC_FTPS_NOT_CLOSED;
    else if (ftps_client_ptr->nx_ftp_client_state == NX_FTP_STATE_WRITE_OPEN)
        return AIC_FTPS_NOT_CLOSED;

    if (ftps_client_ptr->nx_ftp_client_passive_transfer_enabled) {
        /* Set up the passive transfer mode:
           Send PASV request,
           Get server IP and port to connect to,
           Open data socket based on server info in PASV response. */
        status = ftps_client_passive_transfer_set(handle, wait_option);
    } else {
        status = ftps_client_active_transfer_set(handle, wait_option);
    }

    if (status) {
        aicx_error(0x132730, "x", "passive or active set fail(0x%x)\n", status);
        return status;
    }

    if (ftps_client_ptr->nx_ftp_client_passive_transfer_enabled == NX_FALSE) {
        aicx_debug(0x043238, "", "ftps data socket connect is active mode\n");
        status = nx_tcp_server_socket_accept(
                 &(ftps_client_ptr->nx_ftp_client_data_socket), wait_option);
        if (status) {
            aicx_error(0x12d119, "x", "waitting for data connection fail(0x%x)\n", status);
            ftps_client_data_socket_disconnect(handle, wait_option);
            return status;
        }
    }

    if (open_type == NX_FTP_OPEN_FOR_WRITE)
        ftps_client_ptr->nx_ftp_client_state = NX_FTP_STATE_WRITE_OPEN;

    if (open_type == NX_FTP_OPEN_FOR_READ)
        ftps_client_ptr->nx_ftp_client_state = NX_FTP_STATE_OPEN;

    if (open_type == AIC_FTPS_OPEN_FOR_LIST)
        ftps_client_ptr->nx_ftp_client_state = NX_FTP_STATE_CONNECTED;

    return AIC_SUCCESS;
}

static u32 ftps_client_list_command_send(u32 handle, u8 *file_name,
                                         u8 *dir_path, ulong wait_option)
{
    u32 i, status;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr->ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;

    if (file_name != AIC_NULL) {
        status = aic_ftps_client_working_dir_set(handle, dir_path, wait_option);
        if (status) {
            aicx_error(0x05f9b1, "x", "list cmd dir set fail(0x%x)\n", status);
            return status;
        }
    }

    status = ftps_cilent_data_socket_connect(handle, AIC_FTPS_OPEN_FOR_LIST,
                                             wait_option);
    if (status) {
        aicx_error(0x19a27b, "x", "list cmd data socket connect fail(0x%x)\n", status);
        return status;
    }

    /* Allocate a packet for sending the LIST command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client,
                                         &packet_ptr, wait_option);
#endif
    if (status) {
        aicx_error(0x059362, "x", "list cmd tls packet allocate fail(0x%x)\n", status);
        goto ERROR_OUT;
    }
    buff_ptr = packet_ptr->nx_packet_prepend_ptr;

    buff_ptr[0] = 'L';
    buff_ptr[1] = 'I';
    buff_ptr[2] = 'S';
    buff_ptr[3] = 'T';
    buff_ptr[4] = ' ';

    if (file_name != AIC_NULL) {
        for (i = 0; file_name[i]; i++) {
            if ((i + 7) >= (u32)(packet_ptr->nx_packet_data_end -
                packet_ptr->nx_packet_prepend_ptr)) {
                aicx_error(0x0aae2d, "", "list cmd file name out of boundary\n");
                status = AIC_FTPS_FAILED;
                goto ERROR_OUT;
            }
            buff_ptr[5 + i] = (u8)file_name[i];
        }

        buff_ptr[5 + i] = 13;
        buff_ptr[5 + i + 1] = 10;
        packet_ptr->nx_packet_length = i + 7;
        packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                           packet_ptr->nx_packet_length;
    } else if (dir_path != AIC_NULL) {
        for (i = 0; dir_path[i]; i++) {
            if ((i + 7) >= (u32)(packet_ptr->nx_packet_data_end -
                packet_ptr->nx_packet_prepend_ptr)) {
                aicx_error(0x0aed11, "", "list cmd directory path out of boundary\n");
                status = AIC_FTPS_FAILED;
                goto ERROR_OUT;
            }
            buff_ptr[5 + i] = (u8)dir_path[i];
        }

        buff_ptr[5 + i] = 13;
        buff_ptr[5 + i + 1] = 10;
        packet_ptr->nx_packet_length = i + 7;
        packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                           packet_ptr->nx_packet_length;
    }

    /* Send the LIST message. */
     status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x1a48ad, "x", "Send LIST command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x0c56fb, "x", "FTPS_LIST receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 1xx message, signaling the data port was connected properly
       and ready for transfer.  */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '1')) {
        aicx_error(0x0cbe61, "", "LIST receive no 1xx message\n");
        status = AIC_FTPS_EXPECTED_1XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    if (status)
        ftps_client_data_socket_disconnect(handle, wait_option);

    return status;
}

static void ftps_client_instance_space_release(AIC_FTPS_CLIENT *client_ptr)
{
#ifdef CONFIG_NET_SECURE
    if (client_ptr->trusted_crt) {
        aos_free(client_ptr->trusted_crt);
        client_ptr->trusted_crt = AIC_NULL;
    }
#endif

    if (client_ptr->ftps_client) {
        aos_free(client_ptr->ftps_client);
        client_ptr->ftps_client = AIC_NULL;
    }

    aos_free(client_ptr);

    return;
}

static u32 ftps_data_socket_tls_start(AIC_FTPS_CLIENT *client_ptr,
                                      ulong wait_option)
{
    u32 status;
#ifdef CONFIG_NET_SECURE
    /* start data socket tls session */
    status = nx_secure_tls_session_start(&(client_ptr->ftps_data_tls_session),
             &(client_ptr->ftps_client->nx_ftp_client_data_socket),
             wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_ssl_init(&client_ptr->mb_cfg.data_ssl);
    aic_ssl_set_recv_waitoption(client_ptr->mb_cfg.data_socket_fd.fd,
                                FTPS_TIME_OUT);
    status = mbedtls_ssl_setup(&client_ptr->mb_cfg.data_ssl,
                               &client_ptr->mb_cfg.conf);

    mbedtls_ssl_set_bio(&client_ptr->mb_cfg.data_ssl,
                        &client_ptr->mb_cfg.data_socket_fd,
                        mbedtls_net_send_ext, mbedtls_net_recv_ext,
                        mbedtls_net_recv_timeout_ext);
#endif

    return status;
}

static void ftps_file_action_status_check(AIC_FTPS_CLIENT *client_ptr)
{
    NX_PACKET *packet_ptr;
    u8 *buf;
    u32 status;

    /* Flush and check 226 response. */
    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     FTPS_FLUSH_TIME_OUT);
    if (status == AIC_SUCCESS && packet_ptr) {
        buf = packet_ptr->nx_packet_prepend_ptr;

        if (buf[0] == '2' && buf[1] == '2') {
            aicx_error(0x0cb501, "", "LIST_F receive file action successful response\n");
        }
        aic_ftps_packet_release(&packet_ptr);
    }
}

/**
 * Set ftps client passive mode.
 * @handle: ftps client handle.
 * @passive_mode_enabled: True to enable;False to disable
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_passive_mode_set(u32 handle, u32 passive_mode_enabled)
{
    AIC_FTPS_CLIENT *client_ptr;
    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

    if (passive_mode_enabled != AIC_TRUE && passive_mode_enabled != AIC_FALSE)
        return AIC_FTPS_INVALID_PARAMETERS;

    /* Set the transfer status according to the passive mode enabled input. */
    client_ptr->ftps_client->nx_ftp_client_passive_transfer_enabled =
        passive_mode_enabled;

    return AIC_SUCCESS;
}

/**
 * Set ftps client transfer mode.
 * @handle: ftps client handle.
 * @transfer_mode: Transfer mode.FTPS only support NX_FTP_TRANSFER_MODE_STREAM
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_transfer_mode_set(u32 handle, UINT transfer_mode)
{
    AIC_FTPS_CLIENT *client_ptr;
    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

    /* Check for transfer mode. */
    if (transfer_mode != NX_FTP_TRANSFER_MODE_STREAM &&
        transfer_mode != NX_FTP_TRANSFER_MODE_BLOCK)
        return AIC_FTPS_INVALID_PARAMETERS;

    client_ptr->ftps_client->nx_ftp_client_transfer_mode = transfer_mode;

    return AIC_SUCCESS;
}

/**
 * Set ftps client AUTH type.
 * @handle: ftps client handle.
 * @auth_type: Encryption mode.AUTH_SSL or AUTH_TLS
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_auth_type_set(u32 handle, u8 auth_type)
{
    AIC_FTPS_CLIENT *client_ptr;
    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

    client_ptr->auth_type = auth_type;

    return AIC_SUCCESS;
}

/**
 * Set ftps client data socket protection level.
 * @handle: ftps client handle.
 * @prot_level:data socket protection level.
 *             PROT_CLEAR: Clear    PROT_PRIVATE:Private
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_prot_level_set(u32 handle, u8 prot_level)
{
    AIC_FTPS_CLIENT *client_ptr;
    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

    client_ptr->prot_level = prot_level;

    return AIC_SUCCESS;
}

/**
 * Set ftps data representation type.
 * @handle: ftps client handle.
 * @data_type:data representation type.FTPS_Binary or FTPS_ASCII
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_data_type_set(u32 handle, u8 data_type)
{
    AIC_FTPS_CLIENT *client_ptr;
    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

    client_ptr->data_type = data_type;

    return AIC_SUCCESS;
}

/**
 * Add a CA Certificate to our trusted store for
 * verifying incoming server certificates.
 * Certificates in the trusted store will not have a private key.
 * For such certificates the private key parameter should be passed as NULL.
 *
 * @handle: ftps client handle.
 * @crt_data:certificate data.
 * @crt_len:certificate data length.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_ftps_client_ca_crt_add(u32 handle, u8 *crt_data, u16 crt_len)
{
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

#ifdef CONFIG_NET_SECURE
    client_ptr->trusted_crt = aos_zalloc(sizeof(NX_SECURE_X509_CERT));
    if (client_ptr->trusted_crt == AIC_NULL) {
        aicx_error(0x19410d, "", "trusted_crt buf allocate fail\n");
        return AIC_FTPS_BUFFER_ALLOCATE_ERROR;
    }

    u32 status = nx_secure_x509_certificate_initialize(client_ptr->trusted_crt,
                                                   crt_data, crt_len, NX_NULL,
                                                   0, NX_NULL, NX_NULL, NX_NULL);
    if (status != AIC_SUCCESS) {
        aicx_error(0x1e0cd0, "x", "trusted_crt initialize fail(0x%x)\n", status);
        aos_free(client_ptr->trusted_crt);
        return status;
    }
#endif /* CONFIG_NET_SECURE */

#ifdef CONFIG_CRYPTO_MBEDTLS
    int ret = mbedtls_x509_crt_parse(&(client_ptr->trusted_crt),
              (const unsigned char *)crt_data, crt_len);

    if (ret != AIC_SUCCESS) {
        aicx_error(0x10c0ef, "d", "ftps ca_crt parse fail(%d)\n", ret);
        return AIC_FTPS_CRT_ADD_ERROR;
    }
#endif /* CONFIG_CRYPTO_MBEDTLS */

    return AIC_SUCCESS;
}

/**
 * Remove an NX_SECURE_TLS_CERTIFICATE instance from
 * the trusted certificates store, keyed on the Common Name field.
 * If remove fails, the space allocated for storing certificate may not be
 * released.
 *
 * @handle: ftps client handle.
 * @common_name:String to match "CN" field.
 * @common_name_len:Length of name.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_ftps_client_ca_crt_remove(u32 handle)
{
    u32 status = AIC_SUCCESS;

    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

#ifdef CONFIG_NET_SECURE
    NX_SECURE_X509_CERT *crt = client_ptr->trusted_crt;

    status = nx_secure_tls_trusted_certificate_remove(
             &(client_ptr->ftps_control_tls_session),
             (u8 *)crt->nx_secure_x509_distinguished_name.nx_secure_x509_common_name,
             crt->nx_secure_x509_distinguished_name.nx_secure_x509_common_name_length);

    status += nx_secure_tls_trusted_certificate_remove(
              &(client_ptr->ftps_data_tls_session),
              (u8 *)crt->nx_secure_x509_distinguished_name.nx_secure_x509_common_name,
              crt->nx_secure_x509_distinguished_name.nx_secure_x509_common_name_length);
    if (status != NX_SECURE_TLS_SUCCESS) {
        aicx_info(0x04a13a, "x", "trusted crt remove fail(0x%x)\n", status);
        return status;
    }

    aos_free(client_ptr->trusted_crt);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_x509_crt_free(&(client_ptr->trusted_crt));
#endif

    return status;
}

/**
 * Creates a FTPS client on the specified IP.
 *
 * @client_name: ftps client name.
 *
 * CALLED BY
 *    Application
 *
 * return ftps client handle for success.
 * return 0 for fail.
 */
u32 aic_ftps_client_create(u8 *client_name)
{
    u32 status;
    AIC_FTPS_CLIENT *client_ptr;

    if (client_name == NX_NULL)
        return AIC_FTPS_CLIENT_CREATE_FAIL;

    if (!ftps_client_pool.packet_pool) {
        ftps_client_pool.per_packet_size = FTPS_PACKET_SIZE;
        ftps_client_pool.packet_pool_size = FTPS_PACKET_POOL_SIZE;
        ftps_client_pool.packet_pool_name = "ftps_client_packet_pool";

        status = aic_create_packet_pool(&ftps_client_pool);
        if (status) {
            aicx_error(0x1d93da, "x", "ftps packet pool create fail(0x%x)\n", status);
            return AIC_FTPS_CLIENT_CREATE_FAIL;
        }
    }

    if (!ftps_ip_ptr) {
        aic_ftps_ip_instance = aic_get_default_ip_instance();
        if (!aic_ftps_ip_instance || !aic_ftps_ip_instance->ip_ptr) {
            aicx_error(0x13961e, "", "ftps network maybe not active\n");
            return AIC_FTPS_CLIENT_CREATE_FAIL;
        }

       ftps_ip_ptr = aic_ftps_ip_instance->ip_ptr;
    }

    client_ptr = aos_zalloc(sizeof(AIC_FTPS_CLIENT));
    if (client_ptr == AIC_NULL) {
        aicx_error(0x15285d, "", "AIC_FTPS_CLIENT buf allocate fail\n");
        return AIC_FTPS_CLIENT_CREATE_FAIL;
    }

    client_ptr->ftps_client = aos_zalloc(sizeof(NX_FTP_CLIENT));
    if (client_ptr->ftps_client == AIC_NULL) {
        aicx_error(0x1788c0, "", "ftps_client buf allocate fail\n");
        return AIC_FTPS_CLIENT_CREATE_FAIL;
    }

    status = nx_ftp_client_create(client_ptr->ftps_client, client_name,
                                  ftps_ip_ptr, FTPS_SOCKET_WINDOW_SIZE,
                                  ftps_client_pool.packet_pool);
    if (status) {
        aicx_error(0x0f79c3, "x", "ftps client create fail(0x%x)\n", status);
        ftps_client_instance_space_release(client_ptr);
        client_ptr = AIC_NULL;
        return AIC_FTPS_CLIENT_CREATE_FAIL;
    }

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_net_init(&client_ptr->mb_cfg.ctrl_socket_fd);
    status = mbedtls_ssl_set_socket(&client_ptr->mb_cfg.ctrl_socket_fd,
                 (int)(&client_ptr->ftps_client->nx_ftp_client_control_socket));
    status = mbedtls_ssl_set_socket(&client_ptr->mb_cfg.data_socket_fd,
                 (int)(&client_ptr->ftps_client->nx_ftp_client_data_socket));
    if (status)
        aicx_error(0x168794, "", "ftps mbed ssl socket set fail\n");
#endif /* CONFIG_CRYPTO_MBEDTLS */

    /* set default paras */
    client_ptr->ftps_client->nx_ftp_client_passive_transfer_enabled = AIC_TRUE;
    client_ptr->auth_type = AUTH_TLS;
    client_ptr->prot_level = PROT_PRIVATE;
    client_ptr->data_type = FTPS_Binary;
    client_ptr->ftps_client->nx_ftp_client_transfer_mode =
        NX_FTP_TRANSFER_MODE_STREAM;

    return (u32)client_ptr;
}

/**
 * Delete a previously created FTPS client on the specified IP.
 *
 * @handle: ftps client handle.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_ftps_client_delete(u32 handle, ulong wait_option)
{
    u32 status;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if ((client_ptr == AIC_NULL) || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

    if (client_ptr->ftps_client->nx_ftp_client_state !=
        NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTPS_NOT_DISCONNECTED;

    /* End TLS session. */
#ifdef CONFIG_NET_SECURE
    nx_secure_tls_session_end(&(client_ptr->ftps_control_tls_session),
                              wait_option);
    nx_secure_tls_session_delete(&(client_ptr->ftps_control_tls_session));

    nx_secure_tls_session_end(&(client_ptr->ftps_data_tls_session),
                              wait_option);
    nx_secure_tls_session_delete(&(client_ptr->ftps_data_tls_session));
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_ssl_close_notify(&client_ptr->mb_cfg.ctrl_ssl);
    ftps_mbed_ctrl_ssl_reset(client_ptr);
    ftps_mbed_data_ssl_reset(client_ptr);
    mbedtls_ssl_reset_socket(&client_ptr->mb_cfg.ctrl_socket_fd);
    mbedtls_ssl_reset_socket(&client_ptr->mb_cfg.data_socket_fd);
    mbedtls_net_free(&client_ptr->mb_cfg.ctrl_socket_fd);
    mbedtls_net_free(&client_ptr->mb_cfg.data_socket_fd);
#endif

    /* Disconnect and unbind sockets. */
    nx_tcp_socket_disconnect(
        &(client_ptr->ftps_client->nx_ftp_client_control_socket),
        wait_option);

    nx_tcp_client_socket_unbind(
        &(client_ptr->ftps_client->nx_ftp_client_control_socket));

    nx_tcp_socket_disconnect(
        &(client_ptr->ftps_client->nx_ftp_client_data_socket),
        wait_option);

    nx_tcp_client_socket_unbind(
        &(client_ptr->ftps_client->nx_ftp_client_data_socket));

    /* Delete the control and data sockets. */
    nx_tcp_socket_delete(&(client_ptr->ftps_client->nx_ftp_client_control_socket));
    nx_tcp_socket_delete(&(client_ptr->ftps_client->nx_ftp_client_data_socket));

    status = aic_delete_packet_pool(&ftps_client_pool);
    if (status)
        aicx_error(0x033d28, "x", "ftps packet pool delete fail(0x%x)\n", status);

    aos_memset(&ftps_client_pool, 0, sizeof(struct aic_packet_pool));

    ftps_ip_ptr = AIC_NULL;

#ifdef NX_SECURE_TLS_SESSION
    ftps_client_control_tls_session_release(client_ptr);
    ftps_client_data_tls_session_release(client_ptr);
    ftps_client_control_remote_crts_release(client_ptr);
    ftps_client_data_remote_crts_release(client_ptr);
#endif
    ftps_client_instance_space_release(client_ptr);
    client_ptr = AIC_NULL;

    return status;
}

/**
 * Send TYPE command to server to set data representation type.
 * @handle: ftps client handle.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_data_transfer_type_set(u32 handle, ulong wait_option)
{
    u32 status;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    if (client_ptr->ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;

    /* Allocate a packet for sending the TYPE command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client,
                                         &packet_ptr, wait_option);
#endif
    if (status) {
        aicx_error(0x015718, "x", "tls packet allocate for TYPE fail(0x%x)\n", status);
        return status;
    }

    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 8) {
        aicx_error(0x0ac600, "", "packt buf for TYPE is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }
    buff_ptr = packet_ptr->nx_packet_prepend_ptr;

    buff_ptr[0] = 'T';
    buff_ptr[1] = 'Y';
    buff_ptr[2] = 'P';
    buff_ptr[3] = 'E';
    buff_ptr[4] = ' ';
    if (client_ptr->data_type == FTPS_Binary)
        buff_ptr[5] = 'I';

    if (client_ptr->data_type == FTPS_ASCII)
        buff_ptr[5] = 'A';

    buff_ptr[6] = 13;
    buff_ptr[7] = 10;
    packet_ptr->nx_packet_length = 8;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x14e0cd, "x", "Send TYPE command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x1e2e0a, "x", "FTPS_TYPE receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 2xx message, signaling the "TYPE" was received properly. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '2')) {
        aicx_error(0x00406c, "", "FTPS_TYPE received no 200 message\n");
        status = AIC_FTPS_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    return status;
}

/**
 * connect a previously created FTPS instance with the FTPS server.
 * @handle: ftps client handle.
 * @url: FTPS URL,such as ftps://192.168.0.17​.
 * @username:Pointer to login username.
 * @password:Pointer to login password.
 * @port:socket port.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_connect(u32 handle, u8 *url, u8 *username, u8 *password,
                            u32 port, ulong wait_option)
{
    u32 status;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if ((client_ptr == AIC_NULL) || client_ptr->ftps_client == AIC_NULL ||
        (client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTPS_PTR_ERROR;

    status = ftps_client_process_url(handle, url, &(client_ptr->server_ip));
    if (status) {
        aicx_error(0x129025, "x", "ftps process url fail(0x%x)\n", status);
        return status;
    }

    /* Call actual client connect function. */
    status = ftps_client_connect_internal(handle, &(client_ptr->server_ip),
                                          username, password, port,
                                          ftps_tls_setup, wait_option);

    return status;
}

/**
 * Disconnect a previously established FTPS connection.
 * @handle: ftps client handle.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_disconnect(u32 handle, ulong wait_option)
{
    NX_PACKET *packet_ptr;
    u8 *buff_ptr;
    u32 status;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    ftps_client = client_ptr->ftps_client;

    if (ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;

    ftps_client->nx_ftp_client_state = NX_FTP_STATE_NOT_CONNECTED;

    /* Allocate a packet for sending QUIT. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client,
                                         &packet_ptr, wait_option);
#endif
    if (status) {
        aicx_error(0x1b10d9, "x", "tls packet allocate for QUIT fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 6) {
        aicx_error(0x01bc00, "", "packt buf for QUIT is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }
    buff_ptr = packet_ptr->nx_packet_prepend_ptr;

    buff_ptr[0] = 'Q';
    buff_ptr[1] = 'U';
    buff_ptr[2] = 'I';
    buff_ptr[3] = 'T';
    buff_ptr[4] = 13;
    buff_ptr[5] = 10;

    packet_ptr->nx_packet_length = 6;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x02d67d, "x", "Send QUIT command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x1c7a40, "x", "FTPS_QUIT receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 2xx message, signaling the QUIT was processed successfully. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '2')) {
        status = AIC_FTPS_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    ftps_client_control_tls_error_exit(client_ptr, wait_option);

    return status;
}

/**
 * Create the specified directory name on the FTPS server.
 * @handle: Ftps client handle.
 * @dir_path: Working dir.
 * @dir_name: New directory name.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_dir_create(u32 handle, u8 *dir_path, u8 *dir_name,
                               ulong wait_option)
{
    u32 status;
    u32 i;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr = AIC_NULL;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    if (client_ptr->ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;

    /* change working dir. */
    status = aic_ftps_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x07e35f, "x", "MKD working dir set fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* Allocate a packet for sending the directory create command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client, &packet_ptr,
                                         wait_option);
#endif
    if (status) {
        aicx_error(0x02ca1a, "x", "tls packet allocate for MKD fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 6) {
        aicx_error(0x1cda76, "", "packt buf for MKD is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    buff_ptr[0] = 'M';
    buff_ptr[1] = 'K';
    buff_ptr[2] = 'D';
    buff_ptr[3] = ' ';

    /* Copy the new directory name into the message. */
    for (i = 0; dir_name[i]; i++) {
        if ((i + 6) >= (u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr)) {
            aicx_error(0x01c605, "", "MKD packt buf for dir_name is too short\n");
            status = AIC_FTPS_FAILED;
            goto ERROR_OUT;
        }
        buff_ptr[4+i] = (u8)dir_name[i];
    }

    buff_ptr[i + 4] = 13;
    buff_ptr[i + 5] = 10;
    packet_ptr->nx_packet_length = i + 6;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x04d6a4, "x", "Send MKD command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x14e9a4, "x", "FTPS_MKD receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 2xx message, signaling the "MKD" was processed successfully. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '2')) {
        aicx_error(0x1c9b70, "", "FTPS_MKD received no 257 message\n");
        status = AIC_FTPS_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    return status;
}

/**
 * Delete the specified directory name on the FTPS server.
 * @handle: Ftps client handle.
 * @dir_path: Working dir.
 * @dir_name: Directory name to delete.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_dir_delete(u32 handle, u8 *dir_path, u8 *dir_name,
                               ulong wait_option)
{
    u32 status;
    u32 i;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    if (client_ptr->ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;

    /* change working dir. */
    status = aic_ftps_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x00e572, "x", "RMD working dir set fail(0x%x)\n", status);
        return status;
    }

    /* Allocate a packet for sending the directory delete command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client, &packet_ptr,
                                         wait_option);
#endif

    if (status) {
        aicx_error(0x00817d, "x", "tls packet allocate for RMD fail(0x%x)\n", status);
        return status;
    }

    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 6) {
        aicx_error(0x1539c2, "", "packt buf for RMD is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    buff_ptr[0] = 'R';
    buff_ptr[1] = 'M';
    buff_ptr[2] = 'D';
    buff_ptr[3] = ' ';

    /* Copy the directory name into the message. */
    for (i = 0; dir_name[i]; i++) {
        /* Check if out of boundary.  */
        if ((i + 6) >= (u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr)) {
            aicx_error(0x1a817d, "", "RMD packt buf for dir_name is too short\n");
            status = AIC_FTPS_FAILED;
            goto ERROR_OUT;
        }
        buff_ptr[4 + i] =  (u8)dir_name[i];
    }

    buff_ptr[i + 4] = 13;
    buff_ptr[i + 5] = 10;
    packet_ptr->nx_packet_length = i + 6;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x137f35, "x", "Send RMD command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x108db3, "x", "FTPS_RMD receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 2xx message, signaling the "RMD" was processed successfully. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '2')) {
        aicx_error(0x0068d0, "", "RMD received no 250 message\n");
        status = AIC_FTPS_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    return status;
}

/**
 * Set the default working directory on the FTPS server.
 * @handle: ftps client handle.
 * @dir_path: New default directory path.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_working_dir_set(u32 handle, u8 *dir_path, ulong wait_option)
{
    u32 status;
    u32 i;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    if (client_ptr->ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;

    /* Allocate a packet for sending the change directory command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client, &packet_ptr,
                                         wait_option);
#endif
    if (status) {
        aicx_error(0x0bc4b6, "x", "tls packet allocate for CWD fail(0x%x)\n", status);
        return status;
    }

    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 6) {
        aicx_error(0x1a0a44, "", "packt buf for CWD is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    buff_ptr[0] =  'C';
    buff_ptr[1] =  'W';
    buff_ptr[2] =  'D';
    buff_ptr[3] =  ' ';

    /* Copy the new default directory into the message. */
    for (i = 0; dir_path[i]; i++) {
        if ((i + 6) >= (u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr)) {
            aicx_error(0x111c95, "", "CWD packt buf for dir_path is too short\n");
            status = AIC_FTPS_FAILED;
            goto ERROR_OUT;
        }
        buff_ptr[4 + i] =  (u8)dir_path[i];
    }

    buff_ptr[i + 4] =  13;
    buff_ptr[i + 5] =  10;

    packet_ptr->nx_packet_length = i + 6;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

     status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x00e59a, "x", "Send CWD command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x126a88, "x", "FTPS_CWD receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 2xx message, signaling the "CWD" was processed successfully. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '2') ||
         (buff_ptr[1] != '5')) {
        aicx_error(0x15b130, "", "FTPS_CWD received no 250 message\n");
        status = AIC_FTPS_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    return status;
}

/**
 * Get a listing for the specified directory on the FTPS server.
 * @handle: ftps client handle.
 * @dir_path: Directory to get listing for.
 * @buf: buffer to save directory info​.
 * @buf_len: buffer length.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_dir_info_get(u32 handle, u8 *dir_path, u8* buf, u32 buf_len,
                                 ulong wait_option)
{
    u32 status;
    NX_PACKET *data_res_packet;
    ulong bytes_copied = 0;
    ulong total_copied_bytes = 0;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    /* Flush the control socket queue for any old messages before starting
       a new file open request. */
    status = ftps_client_tls_receive(client_ptr, &data_res_packet,
                                     FTPS_CTRL_SOCKET, FTPS_FLUSH_TIME_OUT);
    if (status == AIC_SUCCESS) {
        aicx_info(0x0283e4, "", "LIST_D Flush the control socket queue\n");
        aic_ftps_packet_release(&data_res_packet);
    }

    status = ftps_client_list_command_send(handle, AIC_NULL, dir_path,
                                           wait_option);
    if (status) {
        aicx_error(0x141d9d, "x", "LIST_D command send fail(0x%x)\n", status);
        return status;
    }

    /* start data socket tls session */
    status = ftps_data_socket_tls_start(client_ptr, wait_option);
    if (status != AIC_SUCCESS) {
        aicx_error(0x052e0e, "x", "LIST_D tls session start fail(0x%x)\n", status);
        ftps_client_data_socket_disconnect(handle, wait_option);
        return status;
    }

    /* Now see if this socket has data. If so, process all of it now! */
     do {
        status = ftps_client_tls_receive(client_ptr, &data_res_packet,
                                         FTPS_DATA_SOCKET, wait_option);
        if (status != AIC_SUCCESS) {
            aicx_info(0x06837b, "", "LIST_D maybe no data in socket\n");
            break;
        }

        status = nx_packet_data_extract_offset(data_res_packet, 0,
                                               buf + total_copied_bytes,
                                               buf_len - total_copied_bytes ,
                                               &bytes_copied);
        if (status) {
            aicx_error(0x1aa87a, "x", "LIST_D extract packet fail(0x%x)\n", status);
            aic_ftps_packet_release(&data_res_packet);
            break;
        }
        total_copied_bytes += bytes_copied;

        aic_ftps_packet_release(&data_res_packet);
    } while (status == AIC_SUCCESS && (total_copied_bytes < buf_len));

    ftps_client_data_socket_disconnect(handle, wait_option);
    /* Flush and check 226 response. */
    ftps_file_action_status_check(client_ptr);

    return AIC_SUCCESS;
}

/**
 * Get the size of the specified file on the FTPS server.
 * @handle: ftps client handle.
 * @dir_path: File path.
 * @file_name: File to get size.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return file size for success.
 * Return less than 0 for fail.
 *
 */
long aic_ftps_client_file_size_get(u32 handle, u8 *dir_path, u8 *file_name,
                                   ulong wait_option)
{
    u32 i = 0;
    u32 status;
    u8 *buffer_ptr;
    u8 *temp = AIC_NULL;
    const u8 *flag = " ";
    char *next = AIC_NULL;
    NX_PACKET *data_res_packet;
    long bytes = AIC_FTS_FILE_SIZE_GET_FAIL;
    AIC_FTPS_CLIENT *client_ptr;
    ulong bytes_copied;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    /* Flush the control socket queue for any old messages before starting
       a new file open request. */
    status = ftps_client_tls_receive(client_ptr, &data_res_packet,
                                     FTPS_CTRL_SOCKET, FTPS_FLUSH_TIME_OUT);
    if (status == AIC_SUCCESS) {
        aicx_info(0x1ce94e, "", "LIST_F Flush the control socket queue\n");
        aic_ftps_packet_release(&data_res_packet);
    }

    status = ftps_client_list_command_send(handle, file_name, dir_path,
                                           wait_option);
    if (status) {
        aicx_error(0x08a96d, "x", "LIST_F command send fail(0x%x)\n", status);
        return AIC_FTS_FILE_SIZE_GET_FAIL;
    }

    status = ftps_data_socket_tls_start(client_ptr, wait_option);
    if (status != AIC_SUCCESS) {
        aicx_error(0x154fff, "x", "LIST_F tls session start fail(0x%x)\n", status);
        ftps_client_data_socket_disconnect(handle, wait_option);
        return AIC_FTS_FILE_SIZE_GET_FAIL;
    }

    /* Now see if this socket has data.  If so, process all of it now! */
    do {
        status = ftps_client_tls_receive(client_ptr, &data_res_packet,
                                         FTPS_DATA_SOCKET, wait_option);
        if (status != AIC_SUCCESS) {
            aicx_info(0x01a6b3, "", "LIST_F maybe no data left in socket\n");
            break;
        }

        buffer_ptr = aos_zalloc(data_res_packet->nx_packet_length);
        if (buffer_ptr == AIC_NULL) {
            aicx_error(0x0c8f4c, "", "LIST_F buf allocate fail\n");
            aic_ftps_packet_release(&data_res_packet);
            break;
        }

        status = nx_packet_data_extract_offset(data_res_packet, 0, buffer_ptr,
                                               data_res_packet->nx_packet_length,
                                               &bytes_copied);
        if (status) {
            aicx_error(0x117fb7, "x", "LIST_F extract packet fail(0x%x)\n", status);
            aic_ftps_packet_release(&data_res_packet);
            break;
        }

        temp = strtok_r(buffer_ptr, flag, &next);
        do {
            temp = strtok_r(AIC_NULL, flag, &next);
            i++;
        } while (next && i < 4);
        bytes = atoi(temp);
        aic_ftps_packet_release(&data_res_packet);
    } while (status == AIC_SUCCESS);

    ftps_client_data_socket_disconnect(handle, wait_option);
    /* Flush and check 2XX response. */
    ftps_file_action_status_check(client_ptr);

    return bytes;
}

/**
 * Deletes the specified file on the FTPS server.
 * @handle: ftps client handle.
 * @file_name: File to delete.
 * @dir_path: File path.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_file_delete(u32 handle, u8 *dir_path, u8 *file_name,
                                ulong wait_option)
{
    u32 status;
    u32 i;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID ||
        file_name == NX_NULL)
        return AIC_FTPS_PTR_ERROR;

    if (client_ptr->ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;

    /* Flush the control socket queue for any old messages before starting
       a new file open request. */
    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     FTPS_FLUSH_TIME_OUT);
    if (status == AIC_SUCCESS) {
        aicx_info(0x1a30d5, "", "DELE Flush the control socket queue\n");
        aic_ftps_packet_release(&packet_ptr);
    }

    /* change working dir. */
    status = aic_ftps_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x14509d, "x", "DELE working dir set fail(0x%x)\n", status);
        return status;
    }

    /* Allocate a packet for sending DELE command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client, &packet_ptr,
                                         wait_option);
#endif

    if (status) {
        aicx_error(0x061440, "x", "tls packet allocate for DELE fail(0x%x)\n", status);
        return status;
    }

    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 7) {
        aicx_error(0x1c265c, "", "packt buf for DELE is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    buff_ptr[0] = 'D';
    buff_ptr[1] = 'E';
    buff_ptr[2] = 'L';
    buff_ptr[3] = 'E';
    buff_ptr[4] = ' ';

    /* Copy the file name into the message. */
    for (i = 0; file_name[i]; i++) {
        /* Check if out of boundary. */
        if ((i + 7) >= (u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr)) {
            aicx_error(0x12bdcf, "", "DELE packt buf for file_name is too short\n");
            status = AIC_FTPS_FAILED;
            goto ERROR_OUT;
        }
        buff_ptr[5 + i] = (u8)file_name[i];
    }

    buff_ptr[i + 5] =  13;
    buff_ptr[i + 6] =  10;
    packet_ptr->nx_packet_length = i + 7;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

     status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x1b8e29, "x", "Send DELETE command fail(0x%x)\n", status);
         goto ERROR_OUT;
    }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x0098b3, "x", "FTPS_DELE receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 2xx message, signaling the DELE was processed successfully. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '2')) {
        aicx_error(0x16faf3, "", "DELE received no 250 message\n");
        status = AIC_FTPS_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    return status;
}

/**
 * Rename the specified file with the supplied filename.
 * @handle: ftps client handle.
 * @dir_path: File path.
 * @filename: Current file name.
 * @new_filename: New file name.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_file_rename(u32 handle, u8 *dir_path, u8 *filename,
                                u8 *new_filename, ulong wait_option)
{
    u32 status;
    u32 i;
    u8 *buff_ptr;
    NX_PACKET   *packet_ptr;
    AIC_FTPS_CLIENT *client_ptr;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID ||
        filename == NX_NULL || new_filename == NX_NULL)
        return AIC_FTPS_PTR_ERROR;

    if (client_ptr->ftps_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;

    /* Flush the control socket queue for any old messages before starting
       a new file open request. */
    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     FTPS_FLUSH_TIME_OUT);
    if (status == AIC_SUCCESS) {
        aicx_info(0x05bca4, "", "RNFR Flush the control socket queue\n");
        aic_ftps_packet_release(&packet_ptr);
    }

    /* change working dir. */
    status = aic_ftps_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x1b32a8, "x", "RNFR working dir set fail(0x%x)\n", status);
        return status;
    }

    /* Allocate a packet for sending RNFR command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client, &packet_ptr,
                                         wait_option);
#endif
    if (status) {
        aicx_error(0x0ed87b, "x", "tls packet allocate for RNFR fail(0x%x)\n", status);
        return status;
    }
    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    buff_ptr[0] = 'R';
    buff_ptr[1] = 'N';
    buff_ptr[2] = 'F';
    buff_ptr[3] = 'R';
    buff_ptr[4] = ' ';

    /* Copy the old file name into the message. */
    for (i = 0; filename[i]; i++) {
        /* Check if out of boundary. */
        if ((i + 7) >= (u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr)) {
            aicx_error(0x0c7e49, "", "RNFR packt buf for filename is too short\n");
            status = AIC_FTPS_FAILED;
            goto ERROR_OUT;
        }
        buff_ptr[5 + i] = (u8)filename[i];
    }

    buff_ptr[i + 5] = 13;
    buff_ptr[i + 6] = 10;
    packet_ptr->nx_packet_length = i + 7;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x0211d8, "x", "Send RNFR command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x096ccc, "x", "FTPS_RNFR receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 3xx message, signaling the "RNFR" was received properly. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '3')) {
        aicx_error(0x0cd1ef, "", "RNFR received no 350 message\n");
        status = AIC_FTPS_EXPECTED_3XX_CODE;
        goto ERROR_OUT;
    }

    aos_memset(buff_ptr, 0, packet_ptr->nx_packet_length);
    buff_ptr[0] = 'R';
    buff_ptr[1] = 'N';
    buff_ptr[2] = 'T';
    buff_ptr[3] = 'O';
    buff_ptr[4] = ' ';

    /* Copy the new file name into the message. */
    for (i = 0; new_filename[i]; i++) {
        /* Check if out of boundary. */
        if ((i + 7) >= (u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr)) {
            aicx_error(0x1b1876, "", "RNTO buf for new_filename is too short\n");
            status = AIC_FTPS_FAILED;
            goto ERROR_OUT;
        }
        buff_ptr[5 + i] = (u8)new_filename[i];
    }

    buff_ptr[i + 5] = 13;
    buff_ptr[i + 6] = 10;
    packet_ptr->nx_packet_length = i + 7;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x105da8, "x", "Send RNTO command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x1a4af1, "x", "FTPS_RNTO receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 2xx message, signaling the "RNTO" was processed successfully. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '2')) {
        aicx_error(0x1c3dca, "", "RNTO received no 250 message\n");
        status = AIC_FTPS_EXPECTED_2XX_CODE;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    return status;
}

/**
 * Deletes the specified file on the FTPS server.
 * @handle: Ftps client handle.
 * @offset: Download offset.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_download_offset_set(u32 handle, ulong offset,
                                        ulong wait_option)
{
    u32 status;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client_ptr;
    u32 size;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    ftps_client_ptr = client_ptr->ftps_client;

    if (ftps_client_ptr->nx_ftp_client_state != NX_FTP_STATE_CONNECTED &&
        ftps_client_ptr->nx_ftp_client_state != NX_FTP_STATE_OPEN &&
        ftps_client_ptr->nx_ftp_client_state != NX_FTP_STATE_WRITE_OPEN )
        return AIC_FTPS_NOT_CONNECTED;

    /* Allocate a packet for sending REST command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(client_ptr->ftps_client, &packet_ptr,
                                         wait_option);
#endif
    if (status) {
        aicx_error(0x08f27b, "x", "tls packet allocate for REST fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 7) {
        aicx_error(0x007a2b, "", "packt buf for REST is too short\n");
        status = AIC_FTPS_FAILED;
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    buff_ptr[0] = 'R';
    buff_ptr[1] = 'E';
    buff_ptr[2] = 'S';
    buff_ptr[3] = 'T';
    buff_ptr[4] = ' ';

    snprintf(buff_ptr + 5, 10, "%ld", offset);
    size = strlen(buff_ptr);

    buff_ptr[size] = 13;
    buff_ptr[size + 1] = 10;
    packet_ptr->nx_packet_length = size + 2;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x04dd3a, "x", "Send REST command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x105efe, "x", "FTPS_REST receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 3xx message, signaling the "REST" was received properly. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '3')) {
        aicx_error(0x16958e, "", "REST received no 350 message\n");
        status = AIC_FTPS_EXPECTED_3XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftps_packet_release(&packet_ptr);
    return status;
}

/**
 * Start ftps download.The downloaded file will be stored in fs:\ftps.
 * @handle: Ftps client handle.
 * @dir_path: File path.
 * @file_name: File to download.
 * @offset: Download offset.
 * @download_bytes: Total download file size.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_file_download(u32 handle, u8 *dir_path, u8 *file_name,
                                  ulong offset, ulong *download_bytes,
                                  ulong wait_option)
{
    int i, status;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr;
    NX_PACKET *next_packet_ptr;
    u8 *local_file_name = AIC_NULL;
    int fd = 0;
    ulong length;
    ulong count = 0;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client;
    ssize_t rsize;
    long roffset;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    ftps_client = client_ptr->ftps_client;

    if (ftps_client->nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;
    else if (ftps_client->nx_ftp_client_state == NX_FTP_STATE_OPEN)
        return AIC_FTPS_NOT_CLOSED;
    else if (ftps_client->nx_ftp_client_state == NX_FTP_STATE_WRITE_OPEN)
        return AIC_FTPS_NOT_CLOSED;

    /* Flush the control socket queue for any old messages before starting
       a new file open request. */
    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     FTPS_FLUSH_TIME_OUT);
    if (status == AIC_SUCCESS) {
        aicx_info(0x13a950, "", "RETR Flush the control socket queue\n");
        aic_ftps_packet_release(&packet_ptr);
    }

    /* set working directory */
    status = aic_ftps_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x19e998, "x", "RETR directory setup fail(0x%x)\n", status);
        return status;
    }

    /* connect data socket */
    status = ftps_cilent_data_socket_connect(handle, NX_FTP_OPEN_FOR_READ,
                                             wait_option);
    if (status) {
        aicx_error(0x013f18, "x", "RETR data socket connect fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    if (offset) {
        status = aic_ftps_client_download_offset_set(handle, offset, wait_option);
        if (status) {
            aicx_error(0x1c066d, "x", "FTPS download offset set fail fail(0x%x)\n", status);
            goto EXE_ERROR;
        }
    }

    /* Allocate a packet for sending RETR command. */
#ifdef CONFIG_NET_SECURE
    status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                     wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    status = ftps_client_packet_allocate(ftps_client, &packet_ptr, wait_option);
#endif

    if (status) {
        aicx_error(0x1a4c4c, "x", "tls packet allocate for RETR fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    buff_ptr[0] = 'R';
    buff_ptr[1] = 'E';
    buff_ptr[2] = 'T';
    buff_ptr[3] = 'R';
    buff_ptr[4] = ' ';

    for (i = 0; file_name[i]; i++) {
        if ((i + 7) >= (u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr)) {
            aicx_error(0x01844f, "", "RETR file name out of boundary\n");
            status = AIC_FTPS_FAILED;
            goto EXE_ERROR;
        }
        buff_ptr[5 + i] = (u8)file_name[i];
    }

    buff_ptr[5 + i] = 13;
    buff_ptr[5 + i + 1] = 10;
    packet_ptr->nx_packet_length = i + 7;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x1432ca, "x", "Send RETR command fail(0x%x)\n", status);
         goto EXE_ERROR;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x1e6d38, "x", "FTPS_RETR receive res fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 1xx message, signaling the data port was connected properly
       and ready for transfer. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '1')) {
        aicx_error(0x147487, "", "FTPS_RETR command receive no 1xx message\n");
        status = AIC_FTPS_EXPECTED_1XX_CODE;
        goto EXE_ERROR;
    }

    /* release old packet */
    aic_ftps_packet_release(&packet_ptr);
    /* start data tls */
    status = ftps_data_socket_tls_start(client_ptr, wait_option);
    if (status != AIC_SUCCESS) {
        aicx_error(0x0a40ba, "x", "RETR tls session start fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    /* now we can start file open and download */
    local_file_name = (u8 *)aos_zalloc(FTPS_MAX_FILE_NAME_LEN + 9);
    snprintf(local_file_name, FTPS_MAX_FILE_NAME_LEN + 9, "fs:\\ftps\\%s",
             file_name);

    aicx_info(0x1e0d25, "s", "local_file_name is %s\n", local_file_name);
    fd = fs_open(local_file_name, FS_O_WRONLY | FS_O_CREATE);
    if (fd < 0) {
        aicx_error(0x0ae37c, "", "RETR open file fail\n");
        status = AIC_FTPS_FILE_OPEN_FAIL;
        goto EXE_ERROR;
    } else {
        aicx_info(0x171a00, "", "RETR open file success\n");
    }

    if (offset) {
        roffset = fs_seek(fd, offset, FS_SEEK_BEGIN);
        if (roffset < 0 || roffset != offset) {
            aicx_error(0x1689e6, "", "RETR seek file fail\n");
            goto EXE_ERROR;
        }
    } else {
        /* Truncate the file to a size of 0. */
        status = fs_truncate(fd, 0);
        if (status) {
            aicx_error(0x1485a8, "", "RETR truncate file fail\n");
            goto EXE_ERROR;
        }
    }

    /* Now see if this socket has data. If so, process all of it now! */
    do {
        status = ftps_client_tls_receive(client_ptr, &packet_ptr,
                                         FTPS_DATA_SOCKET, wait_option);
        if (status != AIC_SUCCESS) {
            aicx_info(0x04213a, "", "RETR maybe no data in socket\n");
            break;
        }

        length = packet_ptr->nx_packet_length;
        next_packet_ptr = packet_ptr;
        do {
            rsize = fs_write(fd, next_packet_ptr->nx_packet_prepend_ptr,
                             (next_packet_ptr->nx_packet_append_ptr -
                              next_packet_ptr->nx_packet_prepend_ptr));
            if (rsize == (next_packet_ptr->nx_packet_append_ptr -
                          next_packet_ptr->nx_packet_prepend_ptr))
                count += (ulong)(next_packet_ptr->nx_packet_append_ptr -
                                 next_packet_ptr->nx_packet_prepend_ptr);
            else
                aicx_error(0x08b79b, "x", "file write fail(0x%x)\n", status);

            length = length - (ulong)(next_packet_ptr->nx_packet_append_ptr -
                                      next_packet_ptr->nx_packet_prepend_ptr);

#ifdef NX_DISABLE_PACKET_CHAIN
            next_packet_ptr = NX_NULL;
#else
            next_packet_ptr = next_packet_ptr->nx_packet_next;
#endif
        } while ((next_packet_ptr) && (length));
        aic_ftps_packet_release(&packet_ptr);
    } while (status == AIC_SUCCESS);
    *download_bytes = count;

EXE_ERROR:
    if (fd)
        fs_close(fd);
    if (local_file_name)
        aos_free(local_file_name);
    aic_ftps_packet_release(&packet_ptr);
    local_file_name = AIC_NULL;
    ftps_client_data_socket_disconnect(handle, wait_option);

    /* Flush and check 2XX response. */
    ftps_file_action_status_check(client_ptr);

    return status;
}

/**
 * Start ftps upload.The file will be upload is stored in fs:\ftps.
 * @handle: Ftps client handle.
 * @dir_path: File path.
 * @file_name: File to download.
 * @upload_type: Upload type.FTPS_APPE or FTPS_STOU or FTPS_STOR.
 * @upload_bytes: Total upload file size.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftps_client_file_upload(u32 handle, u8 *dir_path, u8 *file_name,
                                u8 upload_type, ulong *upload_bytes,
                                ulong wait_option)
{
    u32 i, status;
    u8 *buff_ptr;
    u8 *local_file_name = AIC_NULL;
    NX_PACKET *packet_ptr;
    int fd = 0;
    ulong length, bytes_to_copied;
    ulong total_bytes = 0;
    FS_FILE_INFO file_info;
    AIC_FTPS_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftps_client;
    ssize_t rsize;

    client_ptr = (AIC_FTPS_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftps_client == AIC_NULL ||
        client_ptr->ftps_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTPS_PTR_ERROR;

    ftps_client = client_ptr->ftps_client;

    if (ftps_client->nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTPS_NOT_CONNECTED;
    else if (ftps_client->nx_ftp_client_state == NX_FTP_STATE_OPEN)
        return AIC_FTPS_NOT_CLOSED;
    else if (ftps_client->nx_ftp_client_state == NX_FTP_STATE_WRITE_OPEN)
        return AIC_FTPS_NOT_CLOSED;

    /* Flush the control socket queue for any old messages before starting
       a new file open request. */
    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     FTPS_FLUSH_TIME_OUT);
    if (status == AIC_SUCCESS) {
        aicx_info(0x03b277, "", "upload Flush the control socket queue\n");
        aic_ftps_packet_release(&packet_ptr);
    }

    /* set working directory */
    status = aic_ftps_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x1a6126, "x", "upload directory setup fail(0x%x)\n", status);
        return status;
    }

    /* connect data socket */
    status = ftps_cilent_data_socket_connect(handle, NX_FTP_OPEN_FOR_WRITE,
                                             wait_option);
    if (status) {
        aicx_error(0x1db3b1, "x", "upload data socket connect fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    /* Allocate a packet for sending STOR/STOU/APPE command. */
 #ifdef CONFIG_NET_SECURE
     status = ftps_client_control_tls_packet_allocate(handle, &packet_ptr,
                                                      wait_option);
 #endif
 #ifdef CONFIG_CRYPTO_MBEDTLS
     status = ftps_client_packet_allocate(ftps_client, &packet_ptr, wait_option);
 #endif

     if (status) {
         aicx_error(0x0ce8ba, "x", " tls packet allocate for upload fail(0x%x)\n", status);
         goto EXE_ERROR;
     }
    buff_ptr = packet_ptr->nx_packet_prepend_ptr;

    if (upload_type == FTPS_APPE) {
        buff_ptr[0] = 'A';
        buff_ptr[1] = 'P';
        buff_ptr[2] = 'P';
        buff_ptr[3] = 'E';
        buff_ptr[4] = ' ';
    }

    if (upload_type == FTPS_STOU) {
        buff_ptr[0] = 'S';
        buff_ptr[1] = 'T';
        buff_ptr[2] = 'O';
        buff_ptr[3] = 'U';
        buff_ptr[4] = ' ';
    }

    if (upload_type == FTPS_STOR) {
        buff_ptr[0] = 'S';
        buff_ptr[1] = 'T';
        buff_ptr[2] = 'O';
        buff_ptr[3] = 'R';
        buff_ptr[4] = ' ';
    }

    for (i = 0; file_name[i]; i++) {
        if ((i + 7) >= (u32)(packet_ptr->nx_packet_data_end -
            packet_ptr->nx_packet_prepend_ptr)) {
            aicx_error(0x07f667, "", "upload file name out of boundary\n");
            status = AIC_FTPS_FAILED;
            goto EXE_ERROR;
        }
        buff_ptr[5 + i] = (u8)file_name[i];
    }
    buff_ptr[5 + i] = 13;
    buff_ptr[5 + i + 1] = 10;
    packet_ptr->nx_packet_length = i + 7;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = ftps_client_tls_send(client_ptr, packet_ptr, wait_option);
    if (status) {
         aicx_error(0x170ed6, "x", "Send upload command fail(0x%x)\n", status);
         goto EXE_ERROR;
     }

    status = ftps_client_tls_receive(client_ptr, &packet_ptr, FTPS_CTRL_SOCKET,
                                     wait_option);
    if (status) {
        aicx_error(0x024fa7, "x", "upload receive res fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;

    /* Check for 1xx message, signaling the data port was connected properly
       and ready for transfer. */
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '1')) {
        aicx_error(0x1a6478, "", "upload command receive no 1xx message\n");
        status = AIC_FTPS_EXPECTED_1XX_CODE;
        goto EXE_ERROR;
    }

    /* release old packet */
    aic_ftps_packet_release(&packet_ptr);
    /* start data tls */
    status = ftps_data_socket_tls_start(client_ptr, wait_option);
    if (status != AIC_SUCCESS) {
        aicx_error(0x1361d3, "x", "upload tls session start fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    /* now we can start file open and upload */
    local_file_name = (u8 *)aos_zalloc(FTPS_MAX_FILE_NAME_LEN + 9);
    snprintf(local_file_name, FTPS_MAX_FILE_NAME_LEN + 9, "fs:\\ftps\\%s",
             file_name);

    fd = fs_open(local_file_name, FS_O_RDONLY);
    if (fd < 0) {
        aicx_error(0x15938c, "", "upload open file fail\n");
        status = AIC_FTPS_FILE_OPEN_FAIL;
        goto EXE_ERROR;
    }

    status = fs_getinfo(local_file_name, &file_info);
    if (status) {
        aicx_error(0x1b1d8a, "", "upload file info get fail\n");
        goto EXE_ERROR;
    }

    bytes_to_copied = file_info.size;
    if (bytes_to_copied <= 0) {
        aicx_error(0x00ed5e, "", "upload file size is 0 or less than 0\n");
        status = AIC_FTPS_FAILED;
        goto EXE_ERROR;
    }

    /* Now read the file and send the contents to the server. */
    while (status == AIC_SUCCESS && bytes_to_copied) {
#ifdef CONFIG_NET_SECURE
        status = ftps_client_data_tls_packet_allocate(handle, &packet_ptr,
                                                      wait_option);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
        status = ftps_client_packet_allocate(ftps_client, &packet_ptr,
                                             wait_option);
#endif
        if (status) {
            aicx_error(0x184303, "x", "upload allocate packet fail(0x%x)\n", status);
            goto EXE_ERROR;
        }

        length = ((ulong) (packet_ptr->nx_packet_data_end -
                           packet_ptr->nx_packet_prepend_ptr)) -
                           NX_PHYSICAL_TRAILER;

        rsize = fs_read(fd, packet_ptr->nx_packet_prepend_ptr, length);
        if (rsize > 0) {
            /* Now send the packet on the data socket. */
            /* Set the packet length. */
            packet_ptr->nx_packet_length = rsize;
            packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                               packet_ptr->nx_packet_length;

            status = ftps_client_data_tls_send(client_ptr, packet_ptr,
                                               wait_option);
            if (status) {
                aicx_error(0x191309, "x", "upload send data to server fail(0x%x)\n", status);
                goto EXE_ERROR;
            } else {
                /* Update the remaining bytes in the file. */
                bytes_to_copied = bytes_to_copied - rsize;
                total_bytes += rsize;
            }
        } else {
            aicx_error(0x0caf89, "", "upload file read fail\n");
            goto EXE_ERROR;
        }
    }
    *upload_bytes = total_bytes;

EXE_ERROR:
    if (fd)
        fs_close(fd);
    if (local_file_name)
        aos_free(local_file_name);
    local_file_name = AIC_NULL;
    aic_ftps_packet_release(&packet_ptr);

    ftps_client_data_socket_disconnect(handle, wait_option);

    /* Flush and check 2XX response. */
    ftps_file_action_status_check(client_ptr);

    return status;
}

u32 aic_ftps_packet_release(NX_PACKET **packet_ptr_ptr)
{
    u32       status;
    NX_PACKET *packet_ptr;

    packet_ptr =  *packet_ptr_ptr;

    if (packet_ptr == AIC_NULL)
        return AIC_FTPS_PTR_ERROR;

    status = nx_packet_release(packet_ptr);
    if (status == NX_SUCCESS)
        *packet_ptr_ptr = AIC_NULL;

    return status;
}

