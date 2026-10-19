/******************************************************************************/
/*                                                                            */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_HTTPS_CLIENT_H__
#define __AIC_HTTPS_CLIENT_H__

#include <aic_net_api.h>
#include <aic_size.h>
#include <aic_http_comm.h>

#define HTTPS_DEFAULT_TIMEOUT (5 * NX_IP_PERIODIC_RATE)
#define HTTPS_MAX_HAND_SHAKE_TIME 3
/*HTTPS API*/

u32 aic_https_ca_crt_data_add(u32 handle, u8 *crt_data, u16 crt_len, u8 crt_id);
u32 aic_https_ca_crt_remove(u32 handle, u8 crt_id);

u32 aic_https_client_crt_data_add(u32 handle, u8 *crt_data, u16 crt_len,
                                  const u8 *priv_key, u16 priv_len,
                                  u32 key_type, u8 crt_id);
u32 aic_https_client_crt_remove(u32 handle, u8 crt_id);
#ifdef CONFIG_FS
u32 aic_https_ca_crt_file_add(u32 handle, u8 *crt_path, u8 crt_id);
u32 aic_https_client_crt_file_add(u32 handle, u32 key_type, u8 *crt_path,
                                  u8 *key_path, u8 crt_id);
#endif
u32 aic_https_client_create(u8 *client_name);

u32 aic_https_client_delete(u32 handle);

u32 aic_https_client_get_start(u32 handle, u8 *url, u8 *username, u8 *password,
                               u32 server_port, ulong wait_option);

u32 aic_https_client_range_get_start(u32 handle, u8 *url, u8 *username,
                                     u8 *password, u32 server_port, slong start,
                                     slong end, ulong wait_option);

u32 aic_https_client_put_start(u32 handle, u8 *url, u8 *username, u8 *password,
                               u32 server_port, ulong total_bytes,
                               ulong wait_option);

u32 aic_https_client_post_start_ext(u32 handle, u8 *url, u8 *username,
                                    u8 *password, u32 server_port,
                                    ulong total_bytes, u8 *content_type,
                                    ulong wait_option);

u32 aic_https_client_post_start(u32 handle, u8 *url, u8* username, u8 *password,
                                u32 server_port, ulong total_bytes,
                                ulong wait_option);

u32 aic_https_client_head_start(u32 handle, u8 *url, u8 *username, u8 *password,
                                u32 server_port, ulong wait_option);

u32 aic_https_client_delete_start(u32 handle, u8 *url, u8 *username,
                                  u8 *password, u32 server_port,
                                  ulong wait_option);

u32 aic_https_client_secure_connect(u32 handle, ulong wait_option);

#endif
