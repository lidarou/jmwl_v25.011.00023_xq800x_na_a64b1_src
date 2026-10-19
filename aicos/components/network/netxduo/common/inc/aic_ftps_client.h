/******************************************************************************/
/*                                                                            */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_FTPS_CLIENT_H__
#define __AIC_FTPS_CLIENT_H__

#include <aic_size.h>
#include <nxd_ftp_client.h>
#include <nx_api.h>
#ifdef CONFIG_NET_SECURE
#include "nx_secure_tls_api.h"
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#endif

#define FTPS_PACKET_SIZE 1600
#define FTPS_MAX_PACKET_NUM 10
/* packet pool size */
#define FTPS_PACKET_POOL_SIZE (FTPS_PACKET_SIZE * FTPS_MAX_PACKET_NUM)
#define FTPS_SOCKET_WINDOW_SIZE SIZE_32K
#define FTPS_TIME_OUT (5 * NX_IP_PERIODIC_RATE)
#define FTPS_FLUSH_TIME_OUT 100
#define AIC_FTPS_OPEN_FOR_LIST 0x03

#define FTPS_MAX_URL_LEN 255
#define FTPS_MAX_HOST_LEN 68
#define FTPS_MAX_FILE_NAME_LEN 99
#define FTPS_MAX_USERNAME 20
#define FTPS_MAX_PASSWORD 20

#define FTPS_TLS_METADATA_SIZE SIZE_16K
#define FTPS_TLS_PACKET_BUF_SIZE SIZE_4K
#define FTPS_REMOTE_CERT_BUF_SIZE SIZE_2K
#define FTPS_REMOTE_ISSUER_BUF_SIZE SIZE_2K

#define AUTH_SSL 1
#define AUTH_TLS 2
#define PROT_CLEAR 1
#define PROT_PRIVATE 2
#define FTPS_Binary 1
#define FTPS_ASCII 2
#define FTPS_DATA_SOCKET 0
#define FTPS_CTRL_SOCKET 1

#define ftps_utility_convert_IPv6_to_ascii _nx_ftp_utility_convert_IPv6_to_ascii
#define ftps_utility_convert_portnumber_ascii _nx_ftp_utility_convert_portnumber_ascii
#define ftps_utility_convert_number_ascii _nx_ftp_utility_convert_number_ascii

typedef enum {
    FTPS_APPE = 1,
    FTPS_STOU,
    FTPS_STOR,
} FTPS_CMD;

/* define error num */
#define AIC_FTPS_CLIENT_CREATE_FAIL 0
#define AIC_FTPS_URL_EMPTY 1
#define AIC_FTPS_FILE_OPEN_FAIL 2
#define AIC_FTPS_BUFFER_ALLOCATE_ERROR 3
#define AIC_FTPS_MBED_HANDSHAKE_FAIL 4
#define AIC_FTPS_CRT_ADD_ERROR 5
#define AIC_FTS_FILE_SIZE_GET_FAIL -1

#define AIC_FTPS_INVALID_COMMAND NX_FTP_INVALID_COMMAND
#define AIC_FTPS_NOT_ENABLED NX_NOT_ENABLED
#define AIC_FTPS_PTR_ERROR NX_PTR_ERROR
#define AIC_FTPS_FAILED NX_FTP_FAILED
#define AIC_FTPS_NOT_OPEN NX_FTP_NOT_OPEN
#define AIC_FTPS_NOT_DISCONNECTED NX_FTP_NOT_DISCONNECTED
#define AIC_FTPS_NOT_CONNECTED NX_FTP_NOT_CONNECTED
#define AIC_FTPS_INVALID_PACKET NX_INVALID_PACKET
#define AIC_FTPS_OPTION_ERROR NX_OPTION_ERROR
#define AIC_FTPS_NOT_CLOSED NX_FTP_NOT_CLOSED
#define AIC_FTPS_INVALID_PARAMETERS NX_INVALID_PARAMETERS
#define AIC_FTPS_EXPECTED_22X_CODE NX_FTP_EXPECTED_22X_CODE
#define AIC_FTPS_EXPECTED_2XX_CODE NX_FTP_EXPECTED_2XX_CODE
#define AIC_FTPS_EXPECTED_23X_CODE NX_FTP_EXPECTED_23X_CODE
#define AIC_FTPS_EXPECTED_33X_CODE NX_FTP_EXPECTED_33X_CODE
#define AIC_FTPS_EXPECTED_3XX_CODE NX_FTP_EXPECTED_3XX_CODE
#define AIC_FTPS_EXPECTED_1XX_CODE NX_FTP_EXPECTED_1XX_CODE

/* Add for mbed*/
#ifdef CONFIG_CRYPTO_MBEDTLS
typedef struct ftps_mbedtls_para_t {
    mbedtls_ssl_context data_ssl;
    mbedtls_ssl_context ctrl_ssl;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_config conf;
    mbedtls_net_context data_socket_fd;
    mbedtls_net_context ctrl_socket_fd;
} FTPS_MBEDTLS_PARA;
#endif

#ifdef CONFIG_NET_SECURE
typedef struct ftps_tls_info
{
    VOID *tls_metadata_area;
    UCHAR *tls_packet_buffer;
} FTPS_TLS_INFO;

typedef struct ftps_remote_crt_info
{
    NX_SECURE_X509_CERT *remote_crt;
    NX_SECURE_X509_CERT *remote_issuer;
    UCHAR *remote_crt_buf;
    UCHAR *remote_issuer_buf;
} FTPS_REMOTE_CRT_INFO;
#endif /* CONFIG_NET_SECURE */

/* Define the basic FTPS Client structure. */
typedef struct aic_ftps_client_instance
{
    NX_FTP_CLIENT *ftps_client;
    NXD_ADDRESS server_ip;
    u8 auth_type;
    u8 prot_level;
    u8 data_type;
    u8 cid;
#ifdef CONFIG_NET_SECURE
    FTPS_TLS_INFO control_tls_info;
    FTPS_TLS_INFO data_tls_info;
    FTPS_REMOTE_CRT_INFO ctrl_crts;
    FTPS_REMOTE_CRT_INFO data_crts;
    NX_SECURE_X509_CERT *trusted_crt;
    NX_SECURE_TLS_SESSION ftps_control_tls_session;
    NX_SECURE_TLS_SESSION ftps_data_tls_session;
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    FTPS_MBEDTLS_PARA mb_cfg;

    mbedtls_x509_crt trusted_crt;
#endif
} AIC_FTPS_CLIENT;

/*FTPS API*/
u32 aic_ftps_client_passive_mode_set(u32 handle, u32 passive_mode_enabled);
u32 aic_ftps_client_transfer_mode_set(u32 handle, UINT transfer_mode);
u32 aic_ftps_client_auth_type_set(u32 handle, u8 auth_type);
u32 aic_ftps_client_prot_level_set(u32 handle, u8 prot_type);
u32 aic_ftps_client_data_type_set(u32 handle, u8 data_type);
u32 aic_ftps_client_ca_crt_add(u32 handle, u8 *crt_data, u16 crt_len);
u32 aic_ftps_client_ca_crt_remove(u32 handle);
u32 aic_ftps_client_create(u8 *client_name);
u32 aic_ftps_client_delete(u32 handle, ulong wait_option);
u32 aic_ftps_client_data_transfer_type_set(u32 handle, ulong wait_option);
u32 aic_ftps_client_connect(u32 handle, u8 *url, u8 *username, u8 *password,
                            u32 port, ulong wait_option);
u32 aic_ftps_client_disconnect(u32 handle, ulong wait_option);
u32 aic_ftps_client_dir_create(u32 handle, u8 *dir_path, u8 *dir_name,
                               ulong wait_option);
u32 aic_ftps_client_dir_delete(u32 handle, u8 *dir_path, u8 *dir_name,
                               ulong wait_option);
u32 aic_ftps_client_working_dir_set(u32 handle, u8 *dir_path, ulong wait_option);
u32 aic_ftps_client_dir_info_get(u32 handle, u8 *dir_path, u8* buf, u32 buf_len,
                                 ulong wait_option);
long aic_ftps_client_file_size_get(u32 handle, u8 *dir_path, u8 *file_name,
                                   ulong wait_option);
u32 aic_ftps_client_file_delete(u32 handle, u8 *dir_path, u8 *file_name,
                                ulong wait_option);
u32 aic_ftps_client_file_rename(u32 handle, u8 *dir_path, u8 *filename,
                                u8 *new_filename, ulong wait_option);
u32 aic_ftps_client_download_offset_set(u32 handle, ulong offset,
                                        ulong wait_option);
u32 aic_ftps_client_file_download(u32 handle, u8 *dir_path, u8 *file_name,
                                  ulong offset, ulong *download_bytes,
                                  ulong wait_option);
u32 aic_ftps_client_file_upload(u32 handle, u8 *dir_path, u8 *file_name,
                                u8 upload_type, ulong *upload_bytes,
                                ulong wait_option);
u32 ftps_utility_convert_IPv6_to_ascii(NX_TCP_SOCKET *socket_ptr, char *buffer,
                                       u32 buffer_length, u32 *size);
u32 ftps_utility_convert_portnumber_ascii(u32 number, char *numstring,
                                          u32 *numstring_length);
u32 ftps_utility_convert_number_ascii(ulong number, char *numstring);
u32 aic_ftps_packet_release(NX_PACKET **packet_ptr_ptr);
#endif