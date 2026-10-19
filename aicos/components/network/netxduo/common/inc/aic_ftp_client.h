/******************************************************************************/
/*                                                                            */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_FTP_CLIENT_H__
#define __AIC_FTP_CLIENT_H__

#include <aic_size.h>
#include <nxd_ftp_client.h>
#include <nx_api.h>
#include <aic_net_api.h>

#define FTP_PACKET_SIZE 1600
#define FTP_MAX_PACKET_NUM 10
/* packet pool size */
#define FTP_PACKET_POOL_SIZE FTP_PACKET_SIZE * FTP_MAX_PACKET_NUM
#define FTP_SOCKET_WINDOW_SIZE SIZE_64K
#define FTP_FLUSH_TIME_OUT (NX_IP_PERIODIC_RATE / 10)
#define AIC_FTP_OPEN_FOR_LIST 0x03

#define FTP_MAX_URL_LEN 255
#define FTP_MAX_HOST_LEN 68
#define FTP_MAX_FILE_NAME_LEN 99
#define FTP_MAX_USERNAME 20
#define FTP_MAX_PASSWORD 20

#define FTP_Binary 1
#define FTP_ASCII 2
#define FTP_CONTRAL_SOCKET_CLOSE_BY_CLIENT 0xFF
#define FTP_CONTRAL_SOCKET_CLOSE_BY_SERVER 0xFD

typedef enum {
    FTP_APPE = 1,
    FTP_STOU,
    FTP_STOR,
    FTP_USER,
    FTP_PASS,
    FTP_QUIT,
    FTP_MKD,
    FTP_RMD,
    FTP_CWD,
    FTP_PORT,
    FTP_EPRT,
    FTP_PASV,
    FTP_LIST,
    FTP_DELE,
    FTP_RNFR,
    FTP_RNTO,
    FTP_TYPE,
    FTP_REST,
    FTP_RETR,
} FTP_CMD;

/* define error num */
#define AIC_FTP_CLIENT_CREATE_FAIL 0
#define AIC_FTP_URL_ERROR 1
#define AIC_FTP_FILE_OPEN_FAIL 2
#define AIC_FTP_BUFFER_ALLOCATE_ERROR 3
#define AIC_FTP_BIND_ADDR_FAIL 4
#define AIC_FTP_FILE_SIZE_GET_FAIL -1
#define AIC_FTP_FILE_DATA_GET_FAIL -2

#define AIC_FTP_INVALID_COMMAND NX_FTP_INVALID_COMMAND
#define AIC_FTP_NOT_ENABLED NX_NOT_ENABLED
#define AIC_FTP_PTR_ERROR NX_PTR_ERROR
#define AIC_FTP_FAILED NX_FTP_FAILED
#define AIC_FTP_NOT_DISCONNECTED NX_FTP_NOT_DISCONNECTED
#define AIC_FTP_NOT_CONNECTED NX_FTP_NOT_CONNECTED
#define AIC_FTP_INVALID_PACKET NX_INVALID_PACKET
#define AIC_FTP_OPTION_ERROR NX_OPTION_ERROR
#define AIC_FTP_NOT_CLOSED NX_FTP_NOT_CLOSED
#define AIC_FTP_INVALID_PARAMETERS NX_INVALID_PARAMETERS
#define AIC_FTP_EXPECTED_22X_CODE NX_FTP_EXPECTED_22X_CODE
#define AIC_FTP_EXPECTED_2XX_CODE NX_FTP_EXPECTED_2XX_CODE
#define AIC_FTP_EXPECTED_23X_CODE NX_FTP_EXPECTED_23X_CODE
#define AIC_FTP_EXPECTED_33X_CODE NX_FTP_EXPECTED_33X_CODE
#define AIC_FTP_EXPECTED_3XX_CODE NX_FTP_EXPECTED_3XX_CODE
#define AIC_FTP_EXPECTED_1XX_CODE NX_FTP_EXPECTED_1XX_CODE

/* Define the basic FTP Client structure. */
typedef struct aic_ftp_client_instance
{
    struct aic_network_ip_instance *aic_ftp_ip_instance;
    struct aic_packet_pool ftp_client_pool;
    NX_IP *ftp_ip_ptr;
    NX_FTP_CLIENT *ftp_client;
    NXD_ADDRESS server_ip;
    u8 data_type;
    u8 cid;
    NX_PACKET *put_packet;
    NX_PACKET *recv_data_packet;
    ulong recv_packet_offset;
} AIC_FTP_CLIENT;

#define ftp_client_passive_transfer_set _nx_ftp_client_passive_transfer_setup
#define ftp_client_active_transfer_set _nx_ftp_client_active_transfer_setup
#define ftp_client_packet_allocate _nx_ftp_client_packet_allocate
/*FTP API*/
u32 aic_ftp_client_passive_mode_set(u32 handle, u32 passive_mode_enabled);
u32 aic_ftp_client_transfer_mode_set(u32 handle, UINT transfer_mode);
u32 aic_ftp_client_data_type_set(u32 handle, u8 data_type);
u32 aic_ftp_client_cid_set(u32 handle, u8 cid);
u32 aic_ftp_client_create(u8 *client_name);
u32 aic_ftp_client_delete(u32 handle, ulong wait_option);
u32 aic_ftp_client_data_transfer_type_set(u32 handle, ulong wait_option);
u32 aic_ftp_client_disconnect(u32 handle, ulong wait_option);
u32 aic_ftp_client_working_dir_set(u32 handle, u8 *dir_path, ulong wait_option);
u32 aic_ftp_client_connect(u32 handle, u8 *url, u8 *username, u8 *password,
                           u32 port, ulong wait_option);

u32 aic_ftp_client_dir_create(u32 handle, u8 *dir_path, u8 *dir_name,
                              ulong wait_option);

u32 aic_ftp_client_dir_delete(u32 handle, u8 *dir_path, u8 *dir_name,
                              ulong wait_option);

u32 aic_ftp_client_dir_info_get(u32 handle, u8 *dir_path, u8* buf, u32 buf_len,
                                ulong *total_len, ulong wait_option);

long aic_ftp_client_file_size_get(u32 handle, u8 *dir_path, u8 *file_name,
                                  ulong wait_option);

u32 aic_ftp_client_file_delete(u32 handle, u8 *dir_path, u8 *file_name,
                               ulong wait_option);

u32 aic_ftp_client_file_rename(u32 handle, u8 *dir_path, u8 *filename,
                               u8 *new_filename, ulong wait_option);

u32 aic_ftp_client_download_offset_set(u32 handle, ulong offset,
                                       ulong wait_option);

u32 aic_ftp_client_file_download(u32 handle, u8 *dir_path, u8 *file_name,
                                 ulong offset, ulong *download_bytes,
                                 ulong wait_option);

u32 aic_ftp_client_file_upload(u32 handle, u8 *dir_path, u8 *file_name,
                               u8 upload_type, ulong *upload_bytes,
                               ulong wait_option);

u32 ftp_client_packet_allocate(NX_FTP_CLIENT *ftp_client_ptr,
                               NX_PACKET **packet_ptr, ulong wait_option);

u32 ftp_client_passive_transfer_set(NX_FTP_CLIENT *ftp_client_ptr,
                                    ulong wait_option);

u32 ftp_client_active_transfer_set(NX_FTP_CLIENT *ftp_client_ptr,
                                   ulong wait_option);

u32 ftp_client_data_socket_disconnect(NX_FTP_CLIENT *client_ptr);

u32 ftp_cilent_data_socket_connect(u32 handle, u32 open_type,
                                   ulong wait_option);

u32 ftp_client_data_socket_unbind(NX_FTP_CLIENT *client_ptr, ulong wait_option);
u32 aic_ftp_packet_release(NX_PACKET **packet_ptr_ptr);

u32 aic_ftp_client_download_cmd_send(u32 handle, u8 *dir_path, u8 *file_name,
                                     ulong wait_option);

u32 aic_ftp_client_upload_cmd_send(u32 handle, u8 *dir_path, u8 *file_name,
                                   u8 upload_type, ulong wait_option);

long aic_ftp_client_file_data_get(u32 handle, u8 *buf, u32 buf_len,
                                  ulong wait_option);

#endif
