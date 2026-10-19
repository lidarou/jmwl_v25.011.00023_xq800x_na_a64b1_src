/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 * Aic FTP extended Component.
 * There are the core functions for FTP,
 * such as uploading, downloading etc.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x06eadd
#define LOG_MODULE    AMOD_NET

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_resp_handle.h>
#include <aic_at_parse_utils.h>
#include <aic_ftp_at_extended.h>
#include <nx_api.h>
#include <aic_ftp_client.h>
#include <aic_ftp_at_process.h>

u32 ftp_upload_by_com(u32 handle, u8 *file_name, u8 *file_path, u8 put_option,
                      ulong *bytes_upload, ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    /* Check for invalid input pointers. */
    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    ftp_client = client_ptr->ftp_client;

    if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTP_NOT_CONNECTED;
    else if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_OPEN)
        return AIC_FTP_NOT_CLOSED;
    else if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_WRITE_OPEN)
        return AIC_FTP_NOT_CLOSED;

    status = aic_ftp_client_upload_cmd_send(handle, file_name, file_path, 
                                            put_option, wait_option);
    if (status) {
        aicx_error(0x0d9880, "x", "upload prepare fail(0x%x)\n", status);
        return status;
    }

    *bytes_upload = client_ptr->put_packet->nx_packet_length;

    /* Send the file data to the server. */
    status = nx_ftp_client_file_write(ftp_client, client_ptr->put_packet,
                                      wait_option);
    if (status) {
        aicx_error(0x13eb9b, "x", "send data to server fail(0x%x)\n", status);
        return status;
    }

    if (client_ptr->put_packet)
        nx_packet_release(client_ptr->put_packet);

    ftp_client_data_socket_disconnect(ftp_client);

    return AIC_SUCCESS;
}

u32 ftp_download_to_com(u32 handle, u8 *file_name, u8 *file_path, u8 urc_port,
                        ulong offset, ulong * total_len, ulong wait_option)
{
    u32 status;
    u8 *buffer_ptr;
    u8 *receive_buffer;
    NX_PACKET *recv_packet_ptr;
    NX_PACKET *next_packet_ptr;
    NX_PACKET *response_ptr;
    ulong bytes_copied;
    ulong length;
    ulong temp_size = 0;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    /* Check for invalid input pointers. */
    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    ftp_client = client_ptr->ftp_client;

    if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTP_NOT_CONNECTED;
    else if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_OPEN)
        return AIC_FTP_NOT_CLOSED;
    else if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_WRITE_OPEN)
        return AIC_FTP_NOT_CLOSED;

    if (offset) {
        status = aic_ftp_client_download_offset_set(handle, offset, wait_option);
        if (status) {
            aicx_error(0x19c420, "x", "FTP download offset set fail(0x%x)\n", status);
            return status;
        }
    }

    status = aic_ftp_client_download_cmd_send(handle, file_path, file_name,
                                              wait_option);
    if (status) {
        aicx_error(0x15bfcb, "x", "FTP download cmd send fail(0x%x)\n", status);
        return status;
    }

    receive_buffer = (u8 *)aos_zalloc(MAX_PACKET_DATA_LEN);
    if (receive_buffer == AIC_NULL) {
        transmit_ftp_cme_err_by_urc(urc_port, AIC_FTP_BUFFER_ALLOCATE_FAIL);
        return AIC_FTP_BUFFER_ALLOCATE_FAIL;
    }

    /* Now see if this socket has data. If so, process all of it now! */
    do {
        status = nx_tcp_socket_receive(
                 &(ftp_client->nx_ftp_client_data_socket),
                 &recv_packet_ptr, wait_option);
        if (status != NX_SUCCESS) {
            aicx_info(0x0f01d5, "", "maybe no data in socket, check ctrl socket\n");
            status = nx_tcp_socket_receive(
                     &(ftp_client->nx_ftp_client_control_socket),
                     &response_ptr, 100);
            if (status) {
                aicx_error(0x073675, "x", "check ctrl socket fail(0x%x)\n", status);
                continue;
            }

            buffer_ptr = response_ptr->nx_packet_prepend_ptr;
            if ((response_ptr->nx_packet_length >= 3) &&
                (buffer_ptr[0] == '2')) {
                nx_packet_release(response_ptr);
                status = NX_FTP_END_OF_FILE;
                continue;
            } else {
                aicx_error(0x128a49, "", "RETR_E receive no success response\n");
                nx_packet_release(response_ptr);
                continue;
            }
        }

        length = recv_packet_ptr->nx_packet_length;
        next_packet_ptr = recv_packet_ptr;
        do {
            status = nx_packet_data_extract_offset(next_packet_ptr, 0,
                                                   receive_buffer,
                                                   MAX_PACKET_DATA_LEN,
                                                   &bytes_copied);
            length = length - (ulong)(next_packet_ptr->nx_packet_append_ptr -
                                      next_packet_ptr->nx_packet_prepend_ptr);
            aic_transmit_at_data(urc_port, receive_buffer, bytes_copied);
            aos_memset(receive_buffer, 0, MAX_PACKET_DATA_LEN);
            temp_size += bytes_copied;
#ifdef NX_DISABLE_PACKET_CHAIN
            next_packet_ptr = NX_NULL;
#else
            next_packet_ptr = next_packet_ptr->nx_packet_next;
#endif
        } while ((next_packet_ptr) && (length));
        nx_packet_release(recv_packet_ptr);
    } while (status == NX_SUCCESS);
    *total_len = temp_size;

    aos_free(receive_buffer);
    receive_buffer = NULL;

    ftp_client_data_socket_disconnect(ftp_client);

    return AIC_SUCCESS;
}

