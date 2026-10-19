/**************************************************************************/
/*                                                                        */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Network Component
 *  Hypertext Transfer Protocol (HTTP) Client
 */

/* Enable HTTPS */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1b5d31
#define LOG_MODULE    AMOD_NET

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <nx_api.h>
#include <nx_ip.h>
#include <nx_ipv6.h>
#include <nx_web_http_client.h>
#include <nxd_bsd.h>

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_pdp_mgr.h>
#include <aic_http_comm.h>
#include <aic_http_client.h>

/**
 * Creates a HTTP client on the specified IP.
 * return http client handle for success;
 * return 0 for fail;
 */
u32 aic_http_client_create(u8 *client_name)
{
    return aic_http_client_common_create(client_name, AIC_FALSE);
}

/* Deletes a previously created HTTP client on the specified IP */
u32 aic_http_client_delete(u32 handle)
{
    return aic_https_client_common_delete(handle, AIC_FALSE);
}

/* This function processes the application GET request. */
u32 aic_http_client_get_start(u32 handle, u8 *url, u8 *username, u8 *password,
                              u32 server_port, ulong wait_option)
{
    return aic_http_get_start(handle, url, username, password,
                              server_port, wait_option, AIC_FALSE);
}

/**
 * This function processes the application GET request.
 * Can specify the range.
 */
u32 aic_http_client_range_get_start(u32 handle, u8 *url, u8 *username,
                                    u8 *password, u32 server_port, slong start,
                                    slong end, ulong wait_option)
{
    return aic_http_range_get_start(handle, url, username, password, server_port,
                                    start, end, wait_option, AIC_FALSE);
}

/* This function processes an application POST request. */
u32 aic_http_client_post_start(u32 handle, u8 *url, u8* username, u8 *password,
                               u32 server_port, ulong total_bytes,
                               ulong wait_option)
{
    return aic_http_post_start(handle, url, username, password, server_port,
                               total_bytes, wait_option, AIC_FALSE);
}

/**
 * This function processes an application POST request.
 * Can specify Content Type.
 */
u32 aic_http_client_post_start_ext(u32 handle, u8 *url, u8 *username,
                                   u8 *password, u32 server_port,
                                   ulong total_bytes, u8 *content_type,
                                   ulong wait_option)
{
    return aic_http_post_start_ext(handle, url, username, password, server_port,
                                   total_bytes, content_type,
                                   wait_option, AIC_FALSE);
}

/* This function processes an application PUT request. */
u32 aic_http_client_put_start(u32 handle, u8 *url, u8 *username, u8 *password,
                              u32 server_port, ulong total_bytes,
                              ulong wait_option)
{
    return aic_http_put_start(handle, url, username, password, server_port,
                              total_bytes, wait_option, AIC_FALSE);
}

/* This function processes an application HEAD request. */
u32 aic_http_client_head_start(u32 handle, u8 *url, u8 *username, u8 *password,
                               u32 server_port, ulong wait_option)
{
    return aic_http_head_start(handle, url, username, password, server_port,
                               wait_option, AIC_FALSE);
}

/* This function processes an application DELETE request. */
u32 aic_http_client_delete_start(u32 handle, u8 *url, u8 *username,
                                 u8 *password, u32 server_port,
                                 ulong wait_option)
{
    return aic_http_delete_start(handle, url, username, password, server_port,
                                 wait_option, AIC_FALSE);
}

/* This function connects a HTTP client on the specified IP. */
u32 aic_http_client_connect(u32 handle, ulong wait_option)
{
    u32 status;
    NX_WEB_HTTP_CLIENT *https_client;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    https_client = client_ptr->https_client;

    status = nx_web_http_client_connect(https_client,
             &(client_ptr->login_info.server_ip),
             client_ptr->login_info.server_port, wait_option);

    return status;
}

