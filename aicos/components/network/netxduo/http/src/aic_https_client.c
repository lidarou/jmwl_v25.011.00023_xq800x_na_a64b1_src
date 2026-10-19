/**************************************************************************/
/*                                                                        */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Network Component
 *  Hypertext Transfer Protocol Secure (HTTPS using TLS) Client
 */

/* Enable HTTPS */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1527d2
#define LOG_MODULE    AMOD_NET

#ifndef NX_WEB_HTTPS_ENABLE
#define NX_WEB_HTTPS_ENABLE
#endif

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <nx_api.h>
#include <nx_ip.h>
#include <nx_ipv6.h>
#ifdef CONFIG_FS
#include <fs_api.h>
#endif
#ifdef CONFIG_NET_SECURE
#include <aic_tls_api.h>
#endif
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_pdp_mgr.h>
#include <aic_http_comm.h>
#include <aic_https_client.h>

#ifdef CONFIG_FS
u32 aic_https_ca_crt_file_add(u32 handle, u8 *crt_path, u8 crt_id)
{
    u32 status = AIC_HTTPS_CRT_INFO_ERROR;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    u8 *local_crt_data = AIC_NULL;
    ssize_t rsize;
    int fd = 0;
    FS_FILE_INFO *file_info = AIC_NULL;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr) || crt_path == AIC_NULL ||
        strlen(crt_path) > AIC_HTTPS_MAX_CRT_PATH_LEN)
        return AIC_HTTPS_PTR_ERROR;

#ifdef CONFIG_CRYPTO_MBEDTLS
    if (!client_ptr->https_client->ca_crts)
        goto CRT_ADD_ERROR;

    mbedtls_x509_crt_init(client_ptr->https_client->ca_crts);
#endif

    fd = fs_open(crt_path, FS_O_RDONLY);
    if (fd < 0) {
        status = AIC_HTTPS_OPEN_LOCAL_CRT_FAIL;
        goto CRT_ADD_ERROR;
    }

    file_info = aos_zalloc(sizeof(FS_FILE_INFO));
    status = fs_getinfo(crt_path, file_info);
    if (status != AIC_SUCCESS) {
        status = AIC_HTTPS_OPEN_LOCAL_CRT_FAIL;
        goto CRT_ADD_ERROR;
    }

    local_crt_data = aos_zalloc(file_info->size);
    rsize = fs_read(fd, local_crt_data, file_info->size);
    if (rsize <= 0 || rsize != file_info->size) {
        goto CRT_ADD_ERROR;
    }

#ifdef CONFIG_CRYPTO_MBEDTLS
    int ret = mbedtls_x509_crt_parse(client_ptr->https_client->ca_crts,
                                     (const unsigned char *)local_crt_data,
                                     file_info->size);
    if (ret < NX_SUCCESS) {
        aicx_error(0x0f5197, "d", "ca_crt parse fail(%d)\n", ret);
        goto CRT_ADD_ERROR;
    } else
        status = AIC_SUCCESS;
#endif /* CONFIG_CRYPTO_MBEDTLS */

CRT_ADD_ERROR:
    if (fd > 0)
        fs_close(fd);
    if (file_info)
        aos_free(file_info);
    if (local_crt_data)
        aos_free(local_crt_data);

    return status;
}
#endif

/**
 * Add an initialized CA certificate to crt list.
 */
u32 aic_https_ca_crt_data_add(u32 handle, u8 *crt_data, u16 crt_len, u8 crt_id)
{
    NX_PARAMETER_NOT_USED(crt_id);
    u32 status = AIC_HTTPS_CRT_INFO_ERROR;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

#ifdef CONFIG_CRYPTO_MBEDTLS
    if (!client_ptr->https_client->ca_crts)
        goto CRT_ADD_ERROR;

    mbedtls_x509_crt_init(client_ptr->https_client->ca_crts);
#endif

    if (crt_data != AIC_NULL && crt_len != 0) {
#ifdef CONFIG_CRYPTO_MBEDTLS
        int ret = mbedtls_x509_crt_parse(client_ptr->https_client->ca_crts,
                                         (const unsigned char *)crt_data,
                                         crt_len);
        if (ret < NX_SUCCESS) {
            aicx_error(0x0f5197, "d", "ca_crt parse fail(%d)\n", ret);
            goto CRT_ADD_ERROR;
        }
#endif /* CONFIG_CRYPTO_MBEDTLS */
    }
    return AIC_SUCCESS;

CRT_ADD_ERROR:
    return status;
}

/**
 * This function removes an NX_SECURE_TLS_CERTIFICATE instance from
 * the trusted certificates store.
 * If remove fails, the space allocated for storing certificate may not be
 * released.
 */
u32 aic_https_ca_crt_remove(u32 handle, u8 crt_id)
{
    NX_PARAMETER_NOT_USED(crt_id);
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_x509_crt_free(client_ptr->https_client->ca_crts);
#endif

    return AIC_SUCCESS;
}

#ifdef CONFIG_FS
u32 aic_https_client_crt_file_add(u32 handle, u32 key_type, u8 *crt_path,
                                  u8 *key_path, u8 crt_id)
{
    NX_PARAMETER_NOT_USED(crt_id);
    u32 status = AIC_HTTPS_CRT_INFO_ERROR;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    u8 *local_crt_data = AIC_NULL;
    u32 local_crt_len;
    u8 *local_key_data = AIC_NULL;
    u32 local_key_len;
    ssize_t rsize;
    int fd = 0;
    FS_FILE_INFO *file_info = AIC_NULL;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr) || crt_path == AIC_NULL ||
        strlen(crt_path) > AIC_HTTPS_MAX_CRT_PATH_LEN ||
        key_path == AIC_NULL || strlen(key_path) > AIC_HTTPS_MAX_CRT_PATH_LEN)
        return AIC_HTTPS_PTR_ERROR;

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_x509_crt_init(client_ptr->https_client->client_crts);
#endif

    fd = fs_open(crt_path, FS_O_RDONLY);
    if (fd < 0) {
        status = AIC_HTTPS_OPEN_LOCAL_CRT_FAIL;
        goto CRT_ADD_ERROR;
    }

    file_info = aos_zalloc(sizeof(FS_FILE_INFO));

    status = fs_getinfo(crt_path, file_info);
    if (status != AIC_SUCCESS) {
        status = AIC_HTTPS_OPEN_LOCAL_CRT_FAIL;
        goto CRT_ADD_ERROR;
    }

    local_crt_len = file_info->size;
    local_crt_data = aos_zalloc(local_crt_len);
    rsize = fs_read(fd, local_crt_data, local_crt_len);
    if (rsize <= 0 || rsize != local_crt_len) {
        goto CRT_ADD_ERROR;
    }

    fd = fs_open(key_path, FS_O_RDONLY);
    if (fd < 0) {
        status = AIC_HTTPS_OPEN_LOCAL_CRT_FAIL;
        goto CRT_ADD_ERROR;
    }

    aos_memset(file_info, 0, sizeof(FS_FILE_INFO));
    status = fs_getinfo(key_path, file_info);
    if (status != AIC_SUCCESS) {
        status = AIC_HTTPS_OPEN_LOCAL_CRT_FAIL;
        goto CRT_ADD_ERROR;
    }

    local_key_len = file_info->size;
    local_key_data = aos_zalloc(local_key_len);
#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_pk_init(client_ptr->https_client->private_key);
#endif
    rsize = fs_read(fd, local_key_data, local_key_len);
    if (rsize <= 0 || rsize != local_key_len) {
        goto CRT_ADD_ERROR;
    }

#ifdef CONFIG_CRYPTO_MBEDTLS
    int ret = mbedtls_x509_crt_parse(client_ptr->https_client->client_crts,
                                     (const unsigned char *)local_crt_data,
                                     local_crt_len);
    if (ret < NX_SUCCESS) {
        goto CRT_ADD_ERROR;
    }
#ifdef CONFIG_CRYPTO_MBEDTLS_3
    ret = mbedtls_pk_parse_key(client_ptr->https_client->private_key,
                               (const unsigned char *)local_key_data,
                               local_key_len, NULL, 0,
                               mbedtls_ctr_drbg_random,
                               &client_ptr->https_client->http_mb_cfg->ctr_drbg);
#else
#ifdef CONFIG_CRYPTO_MBEDTLS_2
    ret = mbedtls_pk_parse_key(client_ptr->https_client->private_key,
                               (const unsigned char *)local_key_data,
                               local_key_len, NULL, 0);
#endif
#endif
    if (ret < NX_SUCCESS) {
        goto CRT_ADD_ERROR;
    } else
        status = AIC_SUCCESS;
#endif

CRT_ADD_ERROR:
    if (fd > 0)
        fs_close(fd);
    if (file_info)
        aos_free(file_info);
    if (local_crt_data)
        aos_free(local_crt_data);
    if (local_key_data)
        aos_free(local_key_data);

    return status;
}
#endif

/**
 * Add an initialized client certificate to crt list.
 *key_type: NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER
 */
u32 aic_https_client_crt_data_add(u32 handle, u8 *crt_data, u16 crt_len,
                                  const u8 *priv_key, u16 priv_len,
                                  u32 key_type, u8 crt_id)
{
    NX_PARAMETER_NOT_USED(crt_id);
    u32 status = AIC_HTTPS_CRT_INFO_ERROR;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_x509_crt_init(client_ptr->https_client->client_crts);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    int ret = mbedtls_x509_crt_parse(client_ptr->https_client->client_crts,
                                     (const unsigned char *)crt_data, crt_len);
    if (ret < NX_SUCCESS) {
        aicx_error(0x0f7cdf, "x", "c_crt paras fail(0x%x)\n", ret);
        goto CRT_ADD_ERROR;
    }

#ifdef CONFIG_CRYPTO_MBEDTLS_3
    ret = mbedtls_pk_parse_key(client_ptr->https_client->private_key,
                               (const unsigned char *)priv_key, priv_len,
                               NULL, 0, mbedtls_ctr_drbg_random,
                               &client_ptr->https_client->http_mb_cfg->ctr_drbg);
#else
#ifdef CONFIG_CRYPTO_MBEDTLS_2
    ret = mbedtls_pk_parse_key(client_ptr->https_client->private_key,
                               (const unsigned char *)priv_key, priv_len,
                               NULL, 0);
#endif
#endif
    if (ret < NX_SUCCESS) {
        aicx_error(0x19d44d, "x", "p_key paras fail(0x%x)\n", ret);

        mbedtls_x509_crt_free(client_ptr->https_client->client_crts);
        goto CRT_ADD_ERROR;
    }
#endif
    return AIC_SUCCESS;

CRT_ADD_ERROR:
    return status;
}

/**
 * This function removes an NX_SECURE_TLS_CERTIFICATE instance from
 * the local certificates store.
 * If remove fails, the space allocated for storing certificate may not be
 * released.
 */
u32 aic_https_client_crt_remove(u32 handle, u8 crt_id)
{
    NX_PARAMETER_NOT_USED(crt_id);
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_x509_crt_free(client_ptr->https_client->client_crts);
    mbedtls_pk_free(client_ptr->https_client->private_key);
#endif

    return AIC_SUCCESS;
}

/**
 * Creates a HTTPS client on the specified IP.
 * return https client handle for success;
 * return 0 for fail;
 */
u32 aic_https_client_create(u8 *client_name)
{
    return aic_http_client_common_create(client_name, AIC_TRUE);
}

/* Deletes a previously created HTTPS client on the specified IP */
u32 aic_https_client_delete(u32 handle)
{
    return aic_https_client_common_delete(handle, AIC_TRUE);
}

/* This function processes the application GET request. */
u32 aic_https_client_get_start(u32 handle, u8 *url, u8 *username, u8 *password,
                               u32 server_port, ulong wait_option)
{
    return aic_http_get_start(handle, url, username, password,
                              server_port, wait_option, AIC_TRUE);
}

/**
 * This function processes the application GET request.
 * Can specify the range.
 */
u32 aic_https_client_range_get_start(u32 handle, u8 *url, u8 *username,
                                     u8 *password, u32 server_port, slong start,
                                     slong end, ulong wait_option)
{
    return aic_http_range_get_start(handle, url, username, password, server_port,
                                    start, end, wait_option, AIC_TRUE);
}

/* This function processes an application POST request. */
u32 aic_https_client_post_start(u32 handle, u8 *url, u8* username, u8 *password,
                                u32 server_port, ulong total_bytes,
                                ulong wait_option)
{
    return aic_http_post_start(handle, url, username, password, server_port,
                               total_bytes, wait_option, AIC_TRUE);
}

/**
 * This function processes an application POST request.
 * Can specify Content Type.
 */
u32 aic_https_client_post_start_ext(u32 handle, u8 *url, u8 *username,
                                    u8 *password, u32 server_port,
                                    ulong total_bytes, u8 *content_type,
                                    ulong wait_option)
{
    return aic_http_post_start_ext(handle, url, username, password, server_port,
                                   total_bytes, content_type,
                                   wait_option, AIC_TRUE);
}

/* This function processes an application PUT request. */
u32 aic_https_client_put_start(u32 handle, u8 *url, u8 *username, u8 *password,
                               u32 server_port, ulong total_bytes,
                               ulong wait_option)
{
    return aic_http_put_start(handle, url, username, password, server_port,
                              total_bytes, wait_option, AIC_TRUE);
}

/* This function processes an application HEAD request. */
u32 aic_https_client_head_start(u32 handle, u8 *url, u8 *username, u8 *password,
                                u32 server_port, ulong wait_option)
{
    return aic_http_head_start(handle, url, username, password, server_port,
                               wait_option, AIC_TRUE);
}

/* This function processes an application DELETE request. */
u32 aic_https_client_delete_start(u32 handle, u8 *url, u8 *username,
                                  u8 *password, u32 server_port,
                                  ulong wait_option)
{
    return aic_http_delete_start(handle, url, username, password, server_port,
                                 wait_option, AIC_TRUE);
}

/* This function connects a HTTPS client on the specified IP. */
u32 aic_https_client_secure_connect(u32 handle, ulong wait_option)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    status = nx_web_http_client_secure_connect(client_ptr->https_client,
             &(client_ptr->login_info.server_ip),
             client_ptr->login_info.server_port, https_tls_setup,
             wait_option);

    return status;
}

