/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_FTP_AT_PROCESS_H__
#define __AIC_FTP_AT_PROCESS_H__

#include <nxd_ftp_client.h>
#include <aic_net_api.h>
#include <aic_ftp_client.h>

#define DEFAULT_FTP_CLIENT_CONTROL_PORT 21
#define PASSIVE_MODE 1
#define ACTIVE_MODE  2

#define FTP_MAX_URL_LEN 255
#define FTP_MAX_USERNAME_LEN 20
#define FTP_MAX_PASSWORD_LEN 20
#define FTP_RES_SHORT_LEN 20
#define FTP_RESLEN 49
#define FTP_CME_STR_LEN 20
#define FTP_MAX_RES_STR_LEN 255
#define FTP_MAX_PORT_NUM 65535
#define FTP_MAX_DIR_INFO_LEN 1460
#define FTP_URC_RES_LEN 30
#define FTP_MAX_FILE_OR_PATH_LEN 99
#define FTP_AT_CLIENT_NAME "FTP AT Client"
#define FTP_DEFAULT_WAIT_TIME (10 * NX_IP_PERIODIC_RATE)
#define FTP_MIN_WAIT_TIME (25 * NX_IP_PERIODIC_RATE)

#define APPE 1
#define STOU 2
#define STOR 3
#define Binary 1
#define ASCII 2
#define FTP_SUCCESS 1
#define FTP_FAILED 2
#define FTP_INTERMISSION_TIME 100
#define CLOSE_BY_CLIENT 0xFF
#define CLOSE_BY_SERVER 0xFD

typedef struct ftp_at_paras {
    u8 host[FTP_MAX_URL_LEN];
    u8 username[FTP_MAX_USERNAME_LEN];
    u8 password[FTP_MAX_PASSWORD_LEN];
    u8 dl_file_name[FTP_MAX_FILE_OR_PATH_LEN];
    u8 dl_file_path[FTP_MAX_FILE_OR_PATH_LEN];
    u8 ul_file_name[FTP_MAX_FILE_OR_PATH_LEN];
    u8 ul_file_path[FTP_MAX_FILE_OR_PATH_LEN];
    ulong ul_total_bytes;
    ulong max_ul_data_size;
    ulong dl_total_bytes;
    ulong dl_offset;
    s32 timeout;
    ulong read_data_len;
    u32 handle;
    u32 control_port;
    ulong server_file_size;
    ulong dir_info_len;
    AIC_FTP_CLIENT *client_ptr;
    u8 cid;
    u8 urc_port;
    u8 passive_mode;
    u8 put_option;
    u8 data_type;
    u8 status;
    u8 close_onwer;
    u32 count;
} FTP_AT_PARAS;

/* define CME ERROR NUM */
#define AIC_FTP_AT_PARAS_ERROR 1060
#define AIC_FTP_THREAD_CREATE_ERROR 1061
#define AIC_FTP_CLIENT_CREATE_ERROR 1062
#define AIC_FTP_CONNECT_ERROR 1063
#define AIC_FTP_TYPE_SET_ERROR 1064
#define AIC_FTP_CLIENT_DELETE_ERROR 1065
#define AIC_FTP_DIRECTORY_INFO_GET_ERROR 1066
#define AIC_FTP_NO_CREATED_CLIENT 1067
#define AIC_FTP_EVENT_CREATE_FAIL 1068
#define AIC_FTP_PACKET_ALLOCATE_ERROR 1069
#define AIC_FTP_DATA_APPEND_ERROR 1070
#define AIC_FTP_MKDIR_ERROR 1071
#define AIC_FTP_RMDIR_ERROR 1072
#define AIC_FTP_DOWNLOAD_AND_SAVE_ERROR 1073
#define AIC_FTP_DOWNLOAD_TO_COM_ERROR 1074
#define AIC_FTP_UPLOAD_ERROR 1075
#define AIC_FTP_UPLOAD_BY_COM_ERROR 1076
#define AIC_FTP_FILE_SIZE_GET_ERROR 1077
#define AIC_FTP_PARA_OUT_OF_RANGE 1078
#define AIC_FTP_BUFFER_ALLOCATE_FAIL 1079
#define AIC_FTP_UPLOAD_FILE_SIZE_ERROR 1080
#define AIC_FTP_DISCONNECTED_BY_SERVER 1081

/* Define FTP Client thread events. */
#define AIC_FTP_MKDIR 0x01
#define AIC_FTP_RMDIR 0x02
#define AIC_FTP_DOWNLOAD_SAVE 0x04
#define AIC_FTP_DOWNLOAD_TO_COM 0x08
#define AIC_FTP_UPLOAD_FILE 0x10
#define AIC_FTP_UPLOAD_BY_COM 0x20
#define AIC_FTP_GET_FILE_SIZE 0x40
#define AIC_FTP_GET_DIRECTORY_INFO 0x80
#define AIC_FTP_ANY_EVENT 0xFF

/* Define global functions. */
u32 aic_ftp_control_socket_port_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_control_socket_port_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_control_socket_port_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_passive_mode_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_passive_mode_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_passive_mode_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_data_transfer_type_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_data_transfer_type_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_data_transfer_type_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_upload_option_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_upload_option_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_upload_option_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_cid_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_cid_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_cid_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_download_rest_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_download_rest_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_download_rest_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_server_addr_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_server_addr_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_server_addr_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_username_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_username_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_username_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_password_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_password_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_password_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_download_file_name_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_download_file_name_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_download_file_name_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_download_file_path_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_download_file_path_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_download_file_path_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_upload_file_name_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_upload_file_name_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_upload_file_name_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_upload_file_path_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_upload_file_path_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_upload_file_path_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_dir_creat_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_dir_creat_exe_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_dir_remove_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_dir_remove_exe_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_download_file_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_download_file_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_upload_file_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_upload_file_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_size_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_size_exe_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_list_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_list_set_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_quit_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_quit_exe_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 aic_ftp_open_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_ftp_open_exe_cmd(AT_REQ_RESP *at_req_res_ptr);

u32 transmit_ftp_cme_err_by_urc(u32 urc_port, u32 value);

#endif

