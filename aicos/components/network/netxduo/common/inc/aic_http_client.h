/******************************************************************************/
/*                                                                            */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_HTTP_CLIENT_H__
#define __AIC_HTTP_CLIENT_H__

#define HTTP_DEFAULT_TIMEOUT (5 * NX_IP_PERIODIC_RATE)

/*HTTP API*/
u32 aic_http_client_create(u8 *client_name);
u32 aic_http_client_delete(u32 handle);
u32 aic_http_client_get_start(u32 handle, u8 *url, u8 *username, u8 *password,
                              u32 server_port, ulong wait_option);
u32 aic_http_client_range_get_start(u32 handle, u8 *url, u8 *username,
                                    u8 *password, u32 server_port, slong start,
                                    slong end, ulong wait_option);
u32 aic_http_client_post_start(u32 handle, u8 *url, u8* username, u8 *password,
                               u32 server_port, ulong total_bytes,
                               ulong wait_option);
u32 aic_http_client_post_start_ext(u32 handle, u8 *url, u8 *username,
                                   u8 *password, u32 server_port,
                                   ulong total_bytes, u8 *content_type,
                                   ulong wait_option);
u32 aic_http_client_put_start(u32 handle, u8 *url, u8 *username, u8 *password,
                              u32 server_port, ulong total_bytes,
                              ulong wait_option);
u32 aic_http_client_head_start(u32 handle, u8 *url, u8 *username, u8 *password,
                               u32 server_port, ulong wait_option);
u32 aic_http_client_delete_start(u32 handle, u8 *url, u8 *username,
                                 u8 *password, u32 server_port,
                                 ulong wait_option);
u32 aic_http_client_connect(u32 handle, ulong wait_option);

#endif

