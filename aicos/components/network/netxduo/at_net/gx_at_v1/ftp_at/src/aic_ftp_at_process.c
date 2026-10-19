/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  This is the handler of AT commands relating to ftp,
 *  such as setting password,username etc.
 */

/* Force error checking to be disabled in this module */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14c5bc
#define LOG_MODULE    AMOD_NET

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_resp_handle.h>
#include <aic_at_parse_utils.h>
#include <aic_net_api.h>
#include <aic_ftp_at_extended.h>
#include <aic_net_pdp_mgr.h>
#include <aic_ftp_at_process.h>

#include <string.h>
#include <nx_api.h>
#include <tx_api.h>
#include <fx_api.h>
#include <nxd_bsd.h>
#include <nxd_ftp_client.h>
#include <aic_size.h>
#include <connmgr_api.h>

static aos_thread_id thread_ftp_client_event_process;
static FTP_AT_PARAS ftp_paras;
static aos_event_ptr ftp_client_event;

static AT_CMD_HANDLE ftp_at_handles[] =
{
    {"AT+FTPPORT", aic_ftp_control_socket_port_test_cmd,
     aic_ftp_control_socket_port_query_cmd, aic_ftp_control_socket_port_set_cmd,
     AIC_NULL},

    {"AT+FTPMODE", aic_ftp_passive_mode_test_cmd, aic_ftp_passive_mode_query_cmd,
     aic_ftp_passive_mode_set_cmd, AIC_NULL},

    {"AT+FTPTYPE", aic_ftp_data_transfer_type_test_cmd,
     aic_ftp_data_transfer_type_query_cmd, aic_ftp_data_transfer_type_set_cmd,
     AIC_NULL},

    {"AT+FTPPUTOPT", aic_ftp_upload_option_test_cmd,
     aic_ftp_upload_option_query_cmd, aic_ftp_upload_option_set_cmd, AIC_NULL},

    {"AT+FTPCID", aic_ftp_cid_test_cmd, aic_ftp_cid_query_cmd,
     aic_ftp_cid_set_cmd, AIC_NULL},

    {"AT+FTPREST", aic_ftp_download_rest_test_cmd,
     aic_ftp_download_rest_query_cmd, aic_ftp_download_rest_set_cmd, AIC_NULL},

    {"AT+FTPSERV", aic_ftp_server_addr_test_cmd, aic_ftp_server_addr_query_cmd,
     aic_ftp_server_addr_set_cmd, AIC_NULL},

    {"AT+FTPUN", aic_ftp_username_test_cmd, aic_ftp_username_query_cmd,
     aic_ftp_username_set_cmd, AIC_NULL},

    {"AT+FTPPW", aic_ftp_password_test_cmd, aic_ftp_password_query_cmd,
     aic_ftp_password_set_cmd, AIC_NULL},

    {"AT+FTPGETNAME", aic_ftp_download_file_name_test_cmd,
     aic_ftp_download_file_name_query_cmd, aic_ftp_download_file_name_set_cmd,
     AIC_NULL},

    {"AT+FTPGETPATH", aic_ftp_download_file_path_test_cmd,
     aic_ftp_download_file_path_query_cmd, aic_ftp_download_file_path_set_cmd,
     AIC_NULL},

    {"AT+FTPPUTNAME", aic_ftp_upload_file_name_test_cmd,
     aic_ftp_upload_file_name_query_cmd, aic_ftp_upload_file_name_set_cmd,
     AIC_NULL},

    {"AT+FTPPUTPATH", aic_ftp_upload_file_path_test_cmd,
     aic_ftp_upload_file_path_query_cmd, aic_ftp_upload_file_path_set_cmd,
     AIC_NULL},

    {"AT+FTPMKD", aic_ftp_dir_creat_test_cmd, AIC_NULL, AIC_NULL,
     aic_ftp_dir_creat_exe_cmd},

    {"AT+FTPRMD", aic_ftp_dir_remove_test_cmd, AIC_NULL, AIC_NULL,
     aic_ftp_dir_remove_exe_cmd},

    {"AT+FTPGET", aic_ftp_download_file_test_cmd, AIC_NULL,
     aic_ftp_download_file_set_cmd, AIC_NULL},

    {"AT+FTPPUT", aic_ftp_upload_file_test_cmd, AIC_NULL,
     aic_ftp_upload_file_set_cmd, AIC_NULL},

    {"AT+FTPSIZE", aic_ftp_size_test_cmd, AIC_NULL, AIC_NULL,
     aic_ftp_size_exe_cmd},

    {"AT+FTPLIST", aic_ftp_list_test_cmd, AIC_NULL, aic_ftp_list_set_cmd,
     AIC_NULL},

    {"AT+FTPQUIT", aic_ftp_quit_test_cmd, AIC_NULL, AIC_NULL,
     aic_ftp_quit_exe_cmd},

    {"AT+FTPOPEN", aic_ftp_open_test_cmd, AIC_NULL, AIC_NULL,
     aic_ftp_open_exe_cmd},

    {AIC_NULL},
};

static u32 transmit_ftp_at_ok(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line[5] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line, AT_RESULT_OK, strlen(AT_RESULT_OK));
    line_p[0] = (ulong)(&result_line);

    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_ftp_at_resp_integer(AT_REQ_RESP *at_req_res_ptr,
                                        u8 *command, u32 res)
{
    u8 result_line0[FTP_RES_SHORT_LEN] = {'\0'};
    ulong line_p[5];

    snprintf(result_line0, FTP_RES_SHORT_LEN, "%s %d", command, res);

    line_p[0] = (ulong)(&result_line0);

    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_ftp_specific_str(AT_REQ_RESP *at_req_res_ptr, u8 *str)
{
    u8 result_line0[FTP_RESLEN] = {'\0'};
    ulong line_p[5];

    strncpy(result_line0, str, FTP_RESLEN);
    line_p[0] = (ulong)(&result_line0);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_ftp_at_para_error(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line1[FTP_CME_STR_LEN] = {'\0'};
    ulong line_p[5];

    snprintf(result_line1, FTP_CME_STR_LEN, "+CME ERROR: %d",
             AIC_FTP_AT_PARAS_ERROR);
    line_p[0] = (ulong)(&result_line1);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_ftp_at_cme_error(AT_REQ_RESP *at_req_res_ptr,
                                     u32 error_num)
{
    u8 result_line1[FTP_CME_STR_LEN] = {'\0'};
    ulong line_p[5];

    snprintf(result_line1, FTP_CME_STR_LEN, "+CME ERROR: %d", error_num);
    line_p[0] = (ulong)(&result_line1);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_ftp_at_resp_str(AT_REQ_RESP *at_req_res_ptr,
                                    u8 *command, u8 *res)
{
    u8 result_line0[FTP_MAX_RES_STR_LEN] = {'\0'};
    ulong line_p[5];

    snprintf(result_line0, FTP_MAX_RES_STR_LEN, "%s %s", command, res);

    line_p[0] = (ulong)(&result_line0);

    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_ftp_at_resp_long_str(AT_REQ_RESP *at_req_res_ptr,
                                         u8 *command, u8 *res)
{
    u8 result_line0[FTP_MAX_FILE_OR_PATH_LEN] = {'\0'};
    ulong line_p[5];

    snprintf(result_line0, FTP_MAX_FILE_OR_PATH_LEN, "%s %s", command, res);

    line_p[0] = (ulong)(&result_line0);

    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_p, 2);

    return AIC_SUCCESS;
}

static u32 transmit_ftp_urc_res(u32 urc_port, u8 *tag, u32 value)
{
    u8 urc_buf[FTP_URC_RES_LEN] = {'\0'};
    snprintf(urc_buf, FTP_URC_RES_LEN, "%s,%d", tag, value);
    aic_transmit_at_cmd_urc(urc_port, urc_buf, strlen(urc_buf));

    return AIC_SUCCESS;
}

static u32 transmit_ftp_at_ok_by_urc(u32 urc_port)
{
    u8 urc_buf[5] = {'\0'};
    strncpy(urc_buf, AT_RESULT_OK, strlen(AT_RESULT_OK));
    aic_transmit_at_result(urc_port, urc_buf, strlen(urc_buf));
    return AIC_SUCCESS;
}

u32 transmit_ftp_cme_err_by_urc(u32 urc_port, u32 value)
{
    u8 urc_buf[FTP_CME_STR_LEN] = {'\0'};
    snprintf(urc_buf, FTP_CME_STR_LEN, "+CME ERROR: %d", value);
    aic_transmit_at_result(urc_port, urc_buf, strlen(urc_buf));

    return AIC_SUCCESS;
}

static void reset_ftp_paras(void)
{
    aos_memset(&ftp_paras, 0, sizeof(FTP_AT_PARAS));

    ftp_paras.cid = 1;
    ftp_paras.timeout = FTP_DEFAULT_WAIT_TIME;
    ftp_paras.passive_mode = PASSIVE_MODE;
    ftp_paras.control_port = DEFAULT_FTP_CLIENT_CONTROL_PORT;
    ftp_paras.put_option = STOR;
    ftp_paras.data_type = Binary;

    return;
}

static void ftp_status_check(u32 wait_time)
{
    ftp_paras.status = 0;
    ftp_paras.count = 0;
    do {
        if (ftp_paras.status == FTP_SUCCESS) {
            aicx_info(0x131fe8, "", "has get FTP SUCCESS status\n");
            break;
        } else if (ftp_paras.status == FTP_FAILED) {
            aicx_info(0x171063, "", "has get FTP FAILED status\n");
            break;
        }
        aos_sleep(FTP_INTERMISSION_TIME);
        ftp_paras.count++;
    } while (ftp_paras.status == 0);

    return;
}

static void ftp_control_socket_close_complete_cb(NX_TCP_SOCKET *socket_ptr)
{
    u32 status;

    if (ftp_paras.close_onwer == CLOSE_BY_CLIENT) {
        aicx_info(0x1bce3b, "", "control socket closed.No neet to run complete callback\n");
        return;
    }

    if (conn_get_uart_at_conid != AIC_ENODEV)
        transmit_ftp_cme_err_by_urc(conn_get_uart_at_conid,
                                    AIC_FTP_DISCONNECTED_BY_SERVER);

    if (conn_get_usb_at_conid != AIC_ENODEV)
        transmit_ftp_cme_err_by_urc(AT_USB_AT_PORT_ID,
                                    AIC_FTP_DISCONNECTED_BY_SERVER);

    status = aic_ftp_client_delete(ftp_paras.handle, ftp_paras.timeout);
    if (status) {
        aicx_error(0x01c052, "x", "ftp client delete fail(0x%x)\n", status);
        return;
    }

    ftp_paras.close_onwer = CLOSE_BY_SERVER;
    ftp_paras.handle = 0;

    return;
}

static void ftp_control_socket_close_cb(NX_TCP_SOCKET *socket_ptr)
{
    u32 status;

    if (ftp_paras.close_onwer == CLOSE_BY_CLIENT) {
        aicx_info(0x1864b9, "", "control socket closed.No neet to run close callback\n");
        return;
    }

    ftp_paras.client_ptr->ftp_client->nx_ftp_client_state =
        NX_FTP_STATE_NOT_CONNECTED;

    status = nx_tcp_socket_disconnect(socket_ptr, ftp_paras.timeout);
    if (status) {
        aicx_error(0x044853, "x", "control socket disconnect fail(0x%x)\n", status);
    }

    return;
}

static void ftp_client_directory_create_entry(void)
{
    u32 status;
    u8 *urc_str = "+FTPMKD: 1";

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_NO_CREATED_CLIENT);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    status = aic_ftp_client_dir_create(ftp_paras.handle, ftp_paras.dl_file_path,
                                       ftp_paras.dl_file_name,
                                       ftp_paras.timeout);
    if (status) {
        aicx_error(0x02d5e9, "x", "create directory fail(0x%x)\n", status);
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port, AIC_FTP_MKDIR_ERROR);
        ftp_paras.status = FTP_FAILED;
    } else {
        aic_transmit_at_result(ftp_paras.urc_port, urc_str, strlen(urc_str));
        transmit_ftp_at_ok_by_urc(ftp_paras.urc_port);
        ftp_paras.status = FTP_SUCCESS;
    }

    return;
}

static void ftp_client_directory_delete_entry(void)
{
    u32 status;
    u8 *urc_str = "+FTPRMD: 1";

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_NO_CREATED_CLIENT);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    status = aic_ftp_client_dir_delete(ftp_paras.handle, ftp_paras.dl_file_path,
                                       ftp_paras.dl_file_name,
                                       ftp_paras.timeout);
    if (status) {
        aicx_error(0x150c89, "x", "remove directory fail(0x%x)\n", status);
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port, AIC_FTP_RMDIR_ERROR);
        ftp_paras.status = FTP_FAILED;
    } else {
        aic_transmit_at_result(ftp_paras.urc_port, urc_str, strlen(urc_str));
        transmit_ftp_at_ok_by_urc(ftp_paras.urc_port);
        ftp_paras.status = FTP_SUCCESS;
    }

    return;
}

static void ftp_download_and_save_file_entry(void)
{
    u32 status;
    u8 *urc_str = "+FTPGET: 1";

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_NO_CREATED_CLIENT);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    status = aic_ftp_client_file_download(ftp_paras.handle,
                                          ftp_paras.dl_file_path,
                                          ftp_paras.dl_file_name,
                                          ftp_paras.dl_offset,
                                          &(ftp_paras.dl_total_bytes),
                                          ftp_paras.timeout);
    if (status) {
        aicx_error(0x0d0ac1, "x", "download and save file fail(0x%x)\n", status);
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_DOWNLOAD_AND_SAVE_ERROR);
        ftp_paras.status = FTP_FAILED;
    } else {
        transmit_ftp_urc_res(ftp_paras.urc_port, urc_str,
                             ftp_paras.dl_total_bytes);
        ftp_paras.status = FTP_SUCCESS;
    }
    transmit_ftp_at_ok_by_urc(ftp_paras.urc_port);

    return;
}

static void ftp_download_to_com_entry(void)
{
    u32 status;
    u8 *urc_str = "+FTPGET: 2";

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_NO_CREATED_CLIENT);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    status = ftp_download_to_com(ftp_paras.handle, ftp_paras.dl_file_name,
                                 ftp_paras.dl_file_path, ftp_paras.urc_port,
                                 ftp_paras.dl_offset,
                                 &(ftp_paras.dl_total_bytes),
                                 ftp_paras.timeout);
    if (status) {
        aicx_error(0x180ecb, "x", "download file to com fail(0x%x)\n", status);
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_DOWNLOAD_TO_COM_ERROR);
        ftp_paras.status = FTP_FAILED;
    } else {
        transmit_ftp_urc_res(ftp_paras.urc_port, urc_str,
                             ftp_paras.dl_total_bytes);
        ftp_paras.status = FTP_SUCCESS;
    }
    transmit_ftp_at_ok_by_urc(ftp_paras.urc_port);

    return;
}

static void ftp_upload_file_entry(void)
{
    u32 status;
    u8 *urc_str = "+FTPPUT: 1";

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_NO_CREATED_CLIENT);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    status = aic_ftp_client_file_upload(ftp_paras.handle, ftp_paras.ul_file_path,
                                        ftp_paras.ul_file_name,
                                        ftp_paras.put_option,
                                        &ftp_paras.ul_total_bytes,
                                        ftp_paras.timeout);
    if (status) {
        aicx_error(0x149fcc, "x", "upload file fail(0x%x)\n", status);
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port, AIC_FTP_UPLOAD_ERROR);
        ftp_paras.status = FTP_FAILED;
    } else {
        transmit_ftp_urc_res(ftp_paras.urc_port, urc_str,
                             ftp_paras.ul_total_bytes);
        ftp_paras.status = FTP_SUCCESS;
    }
    transmit_ftp_at_ok_by_urc(ftp_paras.urc_port);

    return;
}

static void ftp_upload_by_com_entry(void)
{
    u32 status;
    u8 *urc_str = "+FTPPUT: 2";

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_NO_CREATED_CLIENT);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    status = ftp_upload_by_com(ftp_paras.handle, ftp_paras.ul_file_name,
                               ftp_paras.ul_file_path, ftp_paras.put_option,
                               &ftp_paras.ul_total_bytes, ftp_paras.timeout);
    if (status) {
        aicx_error(0x069827, "x", "upload file by com fail(0x%x)\n", status);
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_UPLOAD_BY_COM_ERROR);
        ftp_paras.status = FTP_FAILED;
    } else {
        transmit_ftp_urc_res(ftp_paras.urc_port, urc_str,
                             ftp_paras.ul_total_bytes);
        ftp_paras.status = FTP_SUCCESS;
    }
    transmit_ftp_at_ok_by_urc(ftp_paras.urc_port);

    return;
}

static void ftp_get_file_size_entry(void)
{
    u8 urc_buf[FTP_URC_RES_LEN] = {'\0'};
    long fize_size;

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_NO_CREATED_CLIENT);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    fize_size = aic_ftp_client_file_size_get(ftp_paras.handle,
                                             ftp_paras.dl_file_path,
                                             ftp_paras.dl_file_name,
                                             ftp_paras.timeout);

    if (fize_size < 0) {
        aicx_error(0x0a585e, "", "get file size fail\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_FILE_SIZE_GET_ERROR);
        ftp_paras.status = FTP_FAILED;
    } else {
        snprintf(urc_buf, FTP_URC_RES_LEN, "+FTPSIZE: %ld", fize_size);
        aic_transmit_at_result(ftp_paras.urc_port, urc_buf, strlen(urc_buf));
        transmit_ftp_at_ok_by_urc(ftp_paras.urc_port);
        ftp_paras.status = FTP_SUCCESS;
    }

    return;
}

static void ftp_get_directory_info_entry(void)
{
    u32 status;
    u8 *dir_info;
    ulong total_len;

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_NO_CREATED_CLIENT);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    dir_info = aos_zalloc(ftp_paras.dir_info_len);
    if (dir_info == AIC_NULL) {
        aicx_error(0x0b88df, "", "dir_info buf allocate fail\n");
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_BUFFER_ALLOCATE_FAIL);
        ftp_paras.status = FTP_FAILED;
        return;
    }

    status = aic_ftp_client_dir_info_get(ftp_paras.handle,
                                         ftp_paras.dl_file_path, dir_info,
                                         ftp_paras.dir_info_len, &total_len,
                                         ftp_paras.timeout);
    if (status) {
        aicx_error(0x0c721b, "x", "get directory info fail(0x%x)\n", status);
        transmit_ftp_cme_err_by_urc(ftp_paras.urc_port,
                                    AIC_FTP_DIRECTORY_INFO_GET_ERROR);
        ftp_paras.status = FTP_FAILED;
    } else {
        aic_transmit_at_data(ftp_paras.urc_port, dir_info, total_len);
        transmit_ftp_at_ok_by_urc(ftp_paras.urc_port);
        ftp_paras.status = FTP_SUCCESS;
    }

    return;
}

void aic_ftp_client_event_process_entry(void *thread_input)
{
    u32 status;
    u32 events;

    while(1) {
        status = aos_get_event(ftp_client_event, AIC_FTP_ANY_EVENT,
                               AOS_OR_CLEAR, &events, AOS_WAIT_FOREVER);

        if (status != AOS_SUCCESS)
            continue;

        if (events & AIC_FTP_MKDIR)
            ftp_client_directory_create_entry();

        if (events & AIC_FTP_RMDIR)
            ftp_client_directory_delete_entry();

        if (events & AIC_FTP_DOWNLOAD_SAVE)
            ftp_download_and_save_file_entry();

        if (events & AIC_FTP_DOWNLOAD_TO_COM)
            ftp_download_to_com_entry();

        if (events & AIC_FTP_UPLOAD_FILE)
            ftp_upload_file_entry();

        if (events & AIC_FTP_UPLOAD_BY_COM)
            ftp_upload_by_com_entry();

        if (events & AIC_FTP_GET_FILE_SIZE)
            ftp_get_file_size_entry();

        if (events & AIC_FTP_GET_DIRECTORY_INFO)
            ftp_get_directory_info_entry();
    }
}

/**
 * Set ftp control port.
 * For example: AT+FTPPORT=21
 */
u32 aic_ftp_control_socket_port_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong *paras_arr;
    u16 paras_count;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);
    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPPORT=", &paras_count);

    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.control_port = atoi((u8 *)paras_arr[0]);
    if (ftp_paras.control_port < 1 ||
        ftp_paras.control_port > FTP_MAX_PORT_NUM) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_control_socket_port_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPPORT:";
    transmit_ftp_at_resp_integer(at_req_res_ptr, cmd, ftp_paras.control_port);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_control_socket_port_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPPORT: (1~65535)";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp passive or active mode.
 * For example: AT+FTPMODE=1
 * 1:passive mode   2:active mode
 */
u32 aic_ftp_passive_mode_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPMODE=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.passive_mode = atoi((u8 *)paras_arr[0]);
    if (ftp_paras.passive_mode != 1 && ftp_paras.passive_mode != 2) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_passive_mode_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPMOD:";
    transmit_ftp_at_resp_integer(at_req_res_ptr, cmd, ftp_paras.passive_mode);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_passive_mode_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPMOD: (1~2)";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp data transfer type.
 * For example: AT+FTPTYPE=1
 * 1: Binary   2: ASCII
 */
u32 aic_ftp_data_transfer_type_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong *paras_arr;
    u16 paras_count;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);
    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPTYPE=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.data_type = atoi((u8 *)paras_arr[0]);
    if (ftp_paras.data_type != 1 && ftp_paras.data_type != 2) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_data_transfer_type_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPTYPE:";
    transmit_ftp_at_resp_integer(at_req_res_ptr, cmd, ftp_paras.data_type);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_data_transfer_type_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPTYPE: (1~2)";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp put option.
 * AT+FTPPUTOPT=1
 */
u32 aic_ftp_upload_option_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPPUTOPT=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.put_option = atoi((u8 *)paras_arr[0]);
    if (ftp_paras.put_option < 1 || ftp_paras.put_option > 3) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_upload_option_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPPUTOPT:";
    transmit_ftp_at_resp_integer(at_req_res_ptr, cmd, ftp_paras.put_option);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_upload_option_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPPUTOPT: (1~3)";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set cid used for ftp.
 * For example: AT+FTPCID=1
 */
u32 aic_ftp_cid_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPCID=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.cid = atoi((u8 *)paras_arr[0]);
    if (ftp_paras.cid < 1 || ftp_paras.cid > ANET_MAX_PDP_CID) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_cid_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPCID:";
    transmit_ftp_at_resp_integer(at_req_res_ptr, cmd, ftp_paras.cid);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_cid_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPCID: (1~16)";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Start the data transfer by specific offset.
 * For example: AT+FTPREST=100
 */
u32 aic_ftp_download_rest_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPREST=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.dl_offset = atoi((u8 *)paras_arr[0]);
    if (ftp_paras.dl_offset < 0 ||
        ftp_paras.dl_offset > 102400) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_download_rest_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPREST:";
    transmit_ftp_at_resp_integer(at_req_res_ptr, cmd, ftp_paras.dl_offset);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_download_rest_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPREST: (0~102400)";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp server ip address.
 * For example: AT+FTPSERV=192.168.1.1
 * For example: AT+FTPSERV=www.test.com
 */
u32 aic_ftp_server_addr_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong *paras_arr;
    u16 paras_count;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPSERV=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    strncpy(ftp_paras.host, (u8 *)paras_arr[0], FTP_MAX_URL_LEN);

    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_server_addr_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPSERV:";

    transmit_ftp_at_resp_str(at_req_res_ptr, cmd, ftp_paras.host);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_server_addr_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPSERV: <hostname>";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp username.
 * For example: AT+FTPUN="admin"
 */
u32 aic_ftp_username_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPUN=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    if (strlen((u8 *)paras_arr[0]) > FTP_MAX_USERNAME_LEN) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    strncpy(ftp_paras.username, (u8 *)paras_arr[0], FTP_MAX_USERNAME_LEN);
    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_username_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPUN:";
    transmit_ftp_at_resp_str(at_req_res_ptr, cmd, ftp_paras.username);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_username_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPUN: <username>";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp password.
 * For example: AT+FTPPW="123456"
 */
u32 aic_ftp_password_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPPW=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    if (strlen((u8 *)paras_arr[0]) > FTP_MAX_PASSWORD_LEN) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    strncpy(ftp_paras.password, (u8 *)paras_arr[0], FTP_MAX_PASSWORD_LEN);
    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_password_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPPW:";
    transmit_ftp_at_resp_str(at_req_res_ptr, cmd, ftp_paras.password);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_password_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPPW: <password>";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp download file name.
 * For example: AT+FTPGETNAME="test.txt"
 */
u32 aic_ftp_download_file_name_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);
    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPGETNAME=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    if (strlen((u8 *)paras_arr[0]) > FTP_MAX_FILE_OR_PATH_LEN) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    strncpy(ftp_paras.dl_file_name, (u8 *)paras_arr[0], FTP_MAX_FILE_OR_PATH_LEN);
    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_download_file_name_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPGETNAME:";
    transmit_ftp_at_resp_long_str(at_req_res_ptr, cmd,
                                  ftp_paras.dl_file_name);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_download_file_name_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPGETNAME: <file_name>";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp download file path.
 * For example: AT+FTPGETPATH="/work"
 */
u32 aic_ftp_download_file_path_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPGETPATH=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    if (strlen((u8 *)paras_arr[0]) > FTP_MAX_FILE_OR_PATH_LEN) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    strncpy(ftp_paras.dl_file_path, (u8 *)paras_arr[0], FTP_MAX_FILE_OR_PATH_LEN);
    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_download_file_path_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPGETPATH:";
    transmit_ftp_at_resp_long_str(at_req_res_ptr, cmd,
                                  ftp_paras.dl_file_path);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_download_file_path_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPGETPATH: <path>";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp upload file name.
 * For example: AT+FTPPUTNAME="test.txt"
 */
u32 aic_ftp_upload_file_name_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPPUTNAME=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    if (strlen((u8 *)paras_arr[0]) > FTP_MAX_FILE_OR_PATH_LEN) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    strncpy(ftp_paras.ul_file_name, (u8 *)paras_arr[0],
            FTP_MAX_FILE_OR_PATH_LEN);
    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_upload_file_name_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPPUTNAME:";
    transmit_ftp_at_resp_long_str(at_req_res_ptr, cmd,
                                  ftp_paras.ul_file_name);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_upload_file_name_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPPUTNAME: <file_name>";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Set ftp upload file path.
 * For example: AT+FTPPUTPATH="/work"
 */
u32 aic_ftp_upload_file_path_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPPUTPATH=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    if (strlen((u8 *)paras_arr[0]) > FTP_MAX_FILE_OR_PATH_LEN) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    strncpy(ftp_paras.ul_file_path, (u8 *)paras_arr[0], FTP_MAX_FILE_OR_PATH_LEN);
    transmit_ftp_at_ok(at_req_res_ptr);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_upload_file_path_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *cmd = "+FTPPUTPATH:";
    transmit_ftp_at_resp_long_str(at_req_res_ptr, cmd, ftp_paras.ul_file_path);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_ftp_upload_file_path_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPPUTPATH: <path>";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Create directory on FTP server.
 * For example: AT+FTPMKD
 * The directory name is defined by AT+FTPGETPATH.
 */
u32 aic_ftp_dir_creat_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    if (at_req_res_ptr->at_req[strlen("AT+FTPMKD")] != 0x0D &&
        at_req_res_ptr->at_req[strlen("AT+FTPMKD")] != 0x0A) {
        transmit_ftp_at_para_error(at_req_res_ptr);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.urc_port = at_req_res_ptr->serial_port_id;

    aos_set_event(ftp_client_event, AIC_FTP_MKDIR, AOS_OR);
    ftp_status_check(FTP_MIN_WAIT_TIME);

    return AIC_SUCCESS;
}

u32 aic_ftp_dir_creat_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPMKD";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Remove directory on FTP server.
 * For example: AT+FTPRMD
 * The directory name is defined by AT+FTPGETPATH.
 */
u32 aic_ftp_dir_remove_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    if (at_req_res_ptr->at_req[strlen("AT+FTPRMD")] != 0x0D &&
        at_req_res_ptr->at_req[strlen("AT+FTPRMD")] != 0x0A) {
        transmit_ftp_at_para_error(at_req_res_ptr);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.urc_port = at_req_res_ptr->serial_port_id;

    aos_set_event(ftp_client_event, AIC_FTP_RMDIR, AOS_OR);
    ftp_status_check(FTP_MIN_WAIT_TIME);

    return AIC_SUCCESS;
}

u32 aic_ftp_dir_remove_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPRMD";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Start download file from ftp server.
 * For example: AT+FTPGET=1;Download and save file.
 * For example: AT+FTPGET=2;Download file to com.
 */
u32 aic_ftp_download_file_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;
    u32 mode;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);
    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPGET=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    mode = atoi((u8 *)paras_arr[0]);
    if (mode != 1 && mode != 2) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    ftp_paras.urc_port = at_req_res_ptr->serial_port_id;
    ftp_paras.dl_total_bytes = 0;

    if (mode == 1)
        aos_set_event(ftp_client_event, AIC_FTP_DOWNLOAD_SAVE, AOS_OR);

    if (mode == 2)
        aos_set_event(ftp_client_event, AIC_FTP_DOWNLOAD_TO_COM, AOS_OR);

    ftp_status_check(FTP_MIN_WAIT_TIME);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 aic_ftp_download_file_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPGET: (1~2)";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Start upload file.
 * For example: AT+FTPPUT=1;Upload local file;
 * For example: AT+FTPPUT=2,1024;Upload file by com;
 */
u32 aic_ftp_upload_file_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count;
    ulong *paras_arr;
    u32 mode;
    u32 status = 0;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    /* AT mode */
    if (at_req_res_ptr->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                                 "AT+FTPPUT=", &paras_count);
        if (!paras_arr) {
            aicx_error(0x16868b, "", "cmd paras fail\n");
            transmit_ftp_at_para_error(at_req_res_ptr);
            aic_free_paras_mem(paras_arr, paras_count);
            return AIC_FTP_AT_PARAS_ERROR;
        }

        mode = atoi((u8 *)paras_arr[0]);
        if (mode != 1 && mode != 2) {
            transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
            aic_free_paras_mem(paras_arr, paras_count);
            return AIC_FTP_PARA_OUT_OF_RANGE;
        }

        ftp_paras.urc_port = at_req_res_ptr->serial_port_id;
        ftp_paras.ul_total_bytes = 0;

        if (mode == 1 && paras_count == 1) {
            aos_set_event(ftp_client_event, AIC_FTP_UPLOAD_FILE, AOS_OR);
            ftp_status_check(FTP_MIN_WAIT_TIME);
        } else if (mode == 2 && paras_count == 2) {
            if (ftp_paras.handle == 0) {
                aicx_error(0x129257, "", "no created FTP Client\n");
                transmit_ftp_at_cme_error(at_req_res_ptr,
                                          AIC_FTP_NO_CREATED_CLIENT);
                return AIC_FTP_NO_CREATED_CLIENT;
            }

            aic_ftp_packet_release(&ftp_paras.client_ptr->put_packet);
            ftp_paras.max_ul_data_size = atoi((u8 *)paras_arr[1]);
            at_req_res_ptr->at_mode = AT_DATA_MODE;
            aicx_info(0x00161d, "", "change to AT data mode\n");
            transmit_ftp_specific_str(at_req_res_ptr, "UPLOAD START");
        } else {
            transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
            aic_free_paras_mem(paras_arr, paras_count);
            return AIC_FTP_PARA_OUT_OF_RANGE;
        }
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_SUCCESS;
    }

    /* data mode */
    if (at_req_res_ptr->at_mode == AT_DATA_MODE) {

        if (at_req_res_ptr->end_data_mode) {
            aic_ftp_packet_release(&ftp_paras.client_ptr->put_packet);
            at_req_res_ptr->at_mode = AT_CMD_MODE;
            return AIC_SUCCESS;
        }

        if(strncmp(at_req_res_ptr->at_upstream_data, "+++", 3) == 0) {
            at_req_res_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x157142, "", "change to AT command mode\n");
            transmit_ftp_specific_str(at_req_res_ptr, "UPLOAD STOP");

            if (ftp_paras.client_ptr->put_packet->nx_packet_length <=
                ftp_paras.max_ul_data_size) {
                aos_set_event(ftp_client_event, AIC_FTP_UPLOAD_BY_COM, AOS_OR);
                ftp_status_check(FTP_MIN_WAIT_TIME);
            } else {
                transmit_ftp_at_cme_error(at_req_res_ptr,
                                          AIC_FTP_UPLOAD_FILE_SIZE_ERROR);
            }

            return AIC_SUCCESS;
        }

        if (ftp_paras.client_ptr->put_packet == AIC_NULL) {
            status = nx_packet_allocate(
                     ftp_paras.client_ptr->ftp_client->nx_ftp_client_packet_pool_ptr,
                     &(ftp_paras.client_ptr->put_packet), NX_TCP_PACKET,
                     ftp_paras.timeout);

            if (status) {
                aicx_error(0x05bc74, "x", "ftp put packet allocate fail(0x%x)\n", status);
                transmit_ftp_at_cme_error(at_req_res_ptr,
                                          AIC_FTP_PACKET_ALLOCATE_ERROR);
                status = AIC_FTP_PACKET_ALLOCATE_ERROR;
                goto EXE_ERROR;
            }
        }

        status = nx_packet_data_append(ftp_paras.client_ptr->put_packet,
                 at_req_res_ptr->at_upstream_data,
                 at_req_res_ptr->upstream_data_len,
                 ftp_paras.client_ptr->ftp_client->nx_ftp_client_packet_pool_ptr,
                 ftp_paras.timeout);
        if (status) {
            aicx_error(0x128e28, "x", "write data to packet fail(0x%x)\n", status);
            transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_DATA_APPEND_ERROR);
            status = AIC_FTP_DATA_APPEND_ERROR;
            goto EXE_ERROR;
        } else {
            transmit_ftp_at_ok(at_req_res_ptr);
            aicx_debug(0x10ce5f, "d", "packet len is %d;\n",
                      ftp_paras.client_ptr->put_packet->nx_packet_length);
        }
    }

EXE_ERROR:
    if (status)
        aic_ftp_packet_release(&ftp_paras.client_ptr->put_packet);

    return status;
}

u32 aic_ftp_upload_file_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPPUT: (1~2)";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Get size of file on FTP server.
 * For example: AT+FTPSIZE
 */
u32 aic_ftp_size_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    if (at_req_res_ptr->at_req[strlen("AT+FTPSIZE")] != 0x0D &&
        at_req_res_ptr->at_req[strlen("AT+FTPSIZE")] != 0x0A) {
        transmit_ftp_at_para_error(at_req_res_ptr);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.urc_port = at_req_res_ptr->serial_port_id;

    aos_set_event(ftp_client_event, AIC_FTP_GET_FILE_SIZE, AOS_OR);
    ftp_status_check(FTP_MIN_WAIT_TIME);

    return AIC_SUCCESS;
}

u32 aic_ftp_size_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPSIZE";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Get directory list.
 * For example: AT+FTPLIST=1024
 */
u32 aic_ftp_list_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{

    u16 paras_count;
    ulong *paras_arr;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+FTPLIST=", &paras_count);
    if (!paras_arr || paras_count != 1) {
        aicx_error(0x16868b, "", "cmd paras fail\n");
        transmit_ftp_at_para_error(at_req_res_ptr);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    ftp_paras.dir_info_len = atoi((u8 *)paras_arr[0]);
    if (ftp_paras.dir_info_len < 1 ||
        ftp_paras.dir_info_len > FTP_MAX_DIR_INFO_LEN) {
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_FTP_PARA_OUT_OF_RANGE;
    }

    ftp_paras.urc_port = at_req_res_ptr->serial_port_id;

    aos_set_event(ftp_client_event, AIC_FTP_GET_DIRECTORY_INFO, AOS_OR);
    ftp_status_check(FTP_MIN_WAIT_TIME);

    return AIC_SUCCESS;
}

u32 aic_ftp_list_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPLIST: <data_len>";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Exit the current ftp session.
 * For example: AT+FTPQUIT
 */
u32 aic_ftp_quit_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u32 status;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    if (at_req_res_ptr->at_req[strlen("AT+FTPQUIT")] != 0x0D &&
        at_req_res_ptr->at_req[strlen("AT+FTPQUIT")] != 0x0A) {
        transmit_ftp_at_para_error(at_req_res_ptr);
        return AIC_FTP_AT_PARAS_ERROR;
    }

    if (ftp_paras.handle == 0) {
        aicx_error(0x129257, "", "no created FTP Client\n");
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_NO_CREATED_CLIENT);
        return AIC_FTP_NO_CREATED_CLIENT;
    }

    aic_ftp_packet_release(&ftp_paras.client_ptr->put_packet);
    ftp_paras.close_onwer = CLOSE_BY_CLIENT;
    status = aic_ftp_client_disconnect(ftp_paras.handle, ftp_paras.timeout);
    status += aic_ftp_client_delete(ftp_paras.handle, ftp_paras.timeout);

    if (status) {
        aicx_error(0x094e43, "x", "ftp quit fail(0x%x)\n", status);
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_CLIENT_DELETE_ERROR);
        return AIC_FTP_CLIENT_DELETE_ERROR;
    } else {
        ftp_paras.handle = 0;
        transmit_ftp_at_ok(at_req_res_ptr);
    }

    return AIC_SUCCESS;
}

u32 aic_ftp_quit_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPQUIT";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

/**
 * Open the current ftp session.
 * For example: AT+FTPOPEN
 */
u32 aic_ftp_open_exe_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u32 status;
    NX_FTP_CLIENT *ftp_client;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_req_res_ptr->at_req);

    if (ftp_paras.handle != 0) {
        aicx_info(0x1dbc9e, "", "FTP Client has been created\n");
        transmit_ftp_at_ok(at_req_res_ptr);
        return AIC_SUCCESS;
    }

    ftp_paras.handle = aic_ftp_client_create(FTP_AT_CLIENT_NAME);
    if (ftp_paras.handle == 0) {
        aicx_error(0x199bf0, "", "ftp client create fail\n");
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_CLIENT_CREATE_ERROR);
        return AIC_FTP_CLIENT_CREATE_ERROR;
    }
    ftp_paras.client_ptr = (AIC_FTP_CLIENT *)ftp_paras.handle;
    ftp_client = ftp_paras.client_ptr->ftp_client;

    ftp_client->nx_ftp_client_control_socket.nx_tcp_disconnect_callback =
                                            ftp_control_socket_close_cb;
    ftp_client->nx_ftp_client_control_socket.nx_tcp_disconnect_complete_notify =
                                    ftp_control_socket_close_complete_cb;

    aic_ftp_client_passive_mode_set(ftp_paras.handle, ftp_paras.passive_mode);
    aic_ftp_client_data_type_set(ftp_paras.handle, ftp_paras.data_type);
    if (ftp_paras.cid > 0 && ftp_paras.cid <= ANET_MAX_PDP_CID)
        aic_ftp_client_cid_set(ftp_paras.handle, ftp_paras.cid);

    status = aic_ftp_client_connect(ftp_paras.handle, ftp_paras.host,
                                    ftp_paras.username, ftp_paras.password,
                                    ftp_paras.control_port, ftp_paras.timeout);
    if (status) {
        aicx_error(0x0b1f5d, "x", "ftp connect to server fail(0x%x)\n", status);
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_CONNECT_ERROR);
        goto ERROR_OUT;
    }

    status = aic_ftp_client_data_transfer_type_set(ftp_paras.handle,
                                                   ftp_paras.timeout);
    if (status) {
        aicx_error(0x0be9d6, "x", "data transfer type set fail(0x%x)\n", status);
        transmit_ftp_at_cme_error(at_req_res_ptr, AIC_FTP_TYPE_SET_ERROR);
        goto ERROR_OUT;
    }
    ftp_paras.close_onwer = 0;

    transmit_ftp_at_ok(at_req_res_ptr);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_ftp_client_disconnect(ftp_paras.handle, ftp_paras.timeout);
    aic_ftp_client_delete(ftp_paras.handle, ftp_paras.timeout);
    ftp_paras.handle = 0;
    ftp_paras.client_ptr = AIC_NULL;

    return status;
}

u32 aic_ftp_open_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *str = "+FTPOPEN";

    transmit_ftp_specific_str(at_req_res_ptr, str);
    transmit_ftp_at_ok(at_req_res_ptr);

    return AIC_SUCCESS;
}

int aic_ftp_init(void)
{
    thread_ftp_client_event_process = aos_create_thread(
                                      "thread_ftp_client_event_process",
                                      aic_ftp_client_event_process_entry, NULL,
                                      SIZE_2K, 0, NULL, 15, AOS_AUTO_START);

    if (thread_ftp_client_event_process == AOS_INVALID_THREAD_ID) {
        aicx_error(0x0cf911, "", "ftp event process thread create fail\n");
        return AIC_FTP_THREAD_CREATE_ERROR;
    }

    /* These will be used to driver the FTP client thread. */
    ftp_client_event = aos_create_event("FTP Client Thread Events");
    if (ftp_client_event == AOS_NULL) {
        aicx_error(0x16e86e, "", "ftp event create error\n");
        aos_delete_thread(thread_ftp_client_event_process);
        return AIC_FTP_EVENT_CREATE_FAIL;
    }

    reset_ftp_paras();
    aic_add_at_cmd_handle(&ftp_at_handles[0]);

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_ftp_init);

