/**************************************************************************/
/*                                                                        */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Network Component
 *  File Transfer Protocol (FTP)
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x02ca4d
#define LOG_MODULE    AMOD_NET

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <nx_ip.h>
#include <nx_ipv6.h>
#include <nxd_bsd.h>
#include <nxd_ftp_client.h>
#include <fs_api.h>
//#include <fs_cfg.h>

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_pdp_mgr.h>
#include <aic_ftp_client.h>

/* Flush the control socket queue, prevent receiving legacy status code.*/
static void ftp_control_socket_flush(NX_FTP_CLIENT *client)
{
    u32 status;
    NX_PACKET *packet_ptr;

    status = nx_tcp_socket_receive(&(client->nx_ftp_client_control_socket),
                                   &packet_ptr, FTP_FLUSH_TIME_OUT);
    if (status == NX_SUCCESS) {
        aicx_info(0x01be3c, "", "Flush the control socket queue\n");
        aic_ftp_packet_release(&packet_ptr);
    }

    return;
}

/* Process URL to get host */
static u32 ftp_client_process_url(u8 *url, NXD_ADDRESS *server_ip)
{
    u8 *ftp_url_head = AIC_NULL;
    u8 *temp = AIC_NULL;
    const u8 *flag = "/";
    char *next = AIC_NULL;
    u8 *buf = AIC_NULL;
    u8 *v6_str;
    struct addrinfo *res;
    u32 status;
    u8 host[FTP_MAX_HOST_LEN] = {'\0'};

    if (strlen(url) == 0 || strlen(url) > FTP_MAX_URL_LEN) {
        aicx_error(0x0a7211, "", "FTP url error\n");
        return AIC_FTP_URL_ERROR;
    }

    buf = (u8 *)aos_zalloc(FTP_MAX_URL_LEN);
    if (buf == AIC_NULL) {
        aicx_error(0x0bf42c, "", "buf allocate for url fail\n");
        return AIC_FTP_BUFFER_ALLOCATE_ERROR;
    }

    strncpy(buf, url, FTP_MAX_URL_LEN);
    ftp_url_head = strtok_r(buf, flag, &next);
    if (ftp_url_head == AIC_NULL) {
        aos_free(buf);
        return AIC_FTP_URL_ERROR;
    }

    if (strlen(ftp_url_head) == strlen(url)) {
        strncpy(host, url, FTP_MAX_HOST_LEN);
    } else if (strncmp(ftp_url_head, "ftp", 3) == 0 &&
               strncmp(ftp_url_head, "ftps", 4) != 0) {
        temp = strtok_r(AIC_NULL, flag, &next);
        strncpy(host, temp, FTP_MAX_HOST_LEN);
    } else {
        aicx_error(0x0c5488, "", "invalid url head\n");
        aos_free(buf);
        return AIC_FTP_URL_ERROR;
    }
    aicx_debug(0x14dad8, "s", "host is %s\n", host);
    aos_free(buf);

    if (strncmp(host, "[", 1) == 0) {
        aicx_info(0x1c52eb, "", "ftp_url_host is ipv6 addr\n");
        v6_str = (u8 *)aos_zalloc(FTP_MAX_HOST_LEN);
        if (v6_str == AIC_NULL) {
            aicx_error(0x17609a, "", "buf allocate for v6 str fail\n");
            return AIC_FTP_BUFFER_ALLOCATE_ERROR;
        }

        strncpy(v6_str, host + 1, FTP_MAX_HOST_LEN);
        v6_str[strlen(v6_str) - 1] = '\0';
        aicx_debug(0x0f3b04, "s", "v6_str is %s\n", v6_str);
        status = getaddrinfo(v6_str, NULL, NULL, &res);
        aos_free(v6_str);
        v6_str = AIC_NULL;
    } else {
        status = getaddrinfo(host, NULL, NULL, &res);
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
    } else {
        aicx_error(0x11484c, "x", "DNS resolution fail(0x%x)\n", status);
        return status;
    }

    return AIC_SUCCESS;
}

static void ftp_client_control_error_exit(AIC_FTP_CLIENT *client_ptr,
                                          u32 wait_option)
{
    u32 status;

    status = nx_tcp_socket_disconnect(
             &(client_ptr->ftp_client->nx_ftp_client_control_socket),
             wait_option);
    if (status)
        aicx_info(0x0af858, "x", "ctrl socket disconnect fail(0x%x)\n", status);

    status = nx_tcp_client_socket_unbind(
             &(client_ptr->ftp_client->nx_ftp_client_control_socket));
    if (status)
        aicx_error(0x19fca2, "x", "ctrl socket unbind fail(0x%x)\n", status);

    return;
}

/**
 * This function connects a previously created FTP instance with the
 * FTP server at the specified IP address.
 */
static u32 ftp_client_connect_internal(u32 handle, NXD_ADDRESS *server_ip,
                                       u8 *username, u8 *password, u32 port,
                                       ulong wait_option)
{
    u8 *buf_ptr;
    u32 length, status;
    NX_PACKET *packet_ptr = AIC_NULL;
    AIC_FTP_CLIENT *aic_ftp_client;
    NX_FTP_CLIENT *ftp_client;

    aic_ftp_client = (AIC_FTP_CLIENT *)handle;
    ftp_client = aic_ftp_client->ftp_client;

    status = nx_tcp_client_socket_bind(
             &(ftp_client->nx_ftp_client_control_socket),
             NX_FTP_CLIENT_SOURCE_PORT, wait_option);
    if ((status != NX_SUCCESS) && (status != NX_ALREADY_BOUND)) {
        aicx_error(0x04fbfb, "x", "ctrl socket bind fail(0x%x)\n", status);
        return status;
    }

    status = nxd_tcp_client_socket_connect(
             &(ftp_client->nx_ftp_client_control_socket), server_ip, port,
             wait_option);
    if (status) {
        aicx_error(0x1898e6, "x", "connect to FTP server fail(0x%x)\n", status);
        nx_tcp_client_socket_unbind(&(ftp_client->nx_ftp_client_control_socket));
        return status;
    }

    status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_control_socket),
                                     &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x019156, "x", "no connected response received(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buf_ptr = packet_ptr->nx_packet_prepend_ptr;
    length = packet_ptr->nx_packet_length;
    if ((length < 3) || (buf_ptr[0] != '2') || (buf_ptr[1] != '2')) {
        aicx_error(0x19923c, "", "connection received no 220 message\n");
        status = AIC_FTP_EXPECTED_22X_CODE;
        goto ERROR_OUT;
    }

    aos_memset(buf_ptr, 0, length);
    buf_ptr = packet_ptr->nx_packet_prepend_ptr;
    /* Determine if a user name was supplied.If No, use "USER" */
    if (username == NX_NULL) {
        strncpy(buf_ptr, "USER USER", 10);
        buf_ptr[9] = 13;
        buf_ptr[10]= 10;
        packet_ptr->nx_packet_length = 11;
    } else {
        snprintf(buf_ptr, strlen(username) + 6, "USER %s", username);
        buf_ptr[strlen(username) + 5] = 13;
        buf_ptr[strlen(username) + 6] = 10;
        packet_ptr->nx_packet_length = strlen(username) + 7;
    }
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = nx_tcp_socket_send(&(ftp_client->nx_ftp_client_control_socket),
                                packet_ptr, wait_option);
    if (status) {
        aicx_error(0x0decad, "x", "Send USER command fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /**
     * There may be multiple 220 responses from FTP server after establishing
     * connection. Flush mutiple 220 responses and wait for 331 response
     * from the FTP server.
     */
    while (1) {
        status = nx_tcp_socket_receive(
                 &(ftp_client->nx_ftp_client_control_socket), &packet_ptr,
                 wait_option);
        if (status) {
            aicx_error(0x18a736, "x", "Flush fail(0x%x)\n", status);
            goto ERROR_OUT;
        }

#ifndef NX_DISABLE_PACKET_CHAIN
        if (packet_ptr->nx_packet_next) {
            aic_ftp_packet_release(&packet_ptr);
            status = AIC_FTP_INVALID_PACKET;
            goto ERROR_OUT;
        }
#endif
        buf_ptr = packet_ptr->nx_packet_prepend_ptr;
        if ((packet_ptr->nx_packet_length >= 3) && (buf_ptr[0] == '2') &&
            (buf_ptr[1] == '2')) {
            aicx_info(0x176fea, "", "received 220 message, continue\n");
            aic_ftp_packet_release(&packet_ptr);
            continue;
        }

        /* Check for 331 message. */
        if ((packet_ptr->nx_packet_length < 3) || (buf_ptr[0] != '3') ||
            (buf_ptr[1] != '3')) {
            aicx_error(0x1d56d9, "", "FTP_USER received no 331 message\n");
            status = AIC_FTP_EXPECTED_33X_CODE;
            goto ERROR_OUT;
        } else {
            break;
        }
    }

    aos_memset(buf_ptr, 0, length);
    /* Determine if a password was specified.If no, use "PASS". */
    if (password == NX_NULL) {
        strncpy(buf_ptr, "PASS PASS", 10);
        buf_ptr[9] = 13;
        buf_ptr[10]= 10;
        packet_ptr->nx_packet_length = 11;
    } else {
        snprintf(buf_ptr, strlen(password) + 6, "PASS %s", password);
        buf_ptr[strlen(password) + 5] = 13;
        buf_ptr[strlen(password) + 6] = 10;
        packet_ptr->nx_packet_length = strlen(password) + 7;
    }
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = nx_tcp_socket_send(&(ftp_client->nx_ftp_client_control_socket),
                                packet_ptr, wait_option);
    if (status) {
        aicx_error(0x0c23e4, "x", "Send PASS command fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    status = nx_tcp_socket_receive(
             &(ftp_client->nx_ftp_client_control_socket), &packet_ptr,
             wait_option);
    if (status) {
        aicx_error(0x0add65, "x", "FTP_PASS receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* check response code */
    buf_ptr = packet_ptr->nx_packet_prepend_ptr;
    if ((packet_ptr->nx_packet_length < 3) || (buf_ptr[0] != '2') ||
        (buf_ptr[1] != '3')) {
        aicx_error(0x0a2524, "", "FTP_PASS received no 230 message\n");
        status = AIC_FTP_EXPECTED_23X_CODE;
        goto ERROR_OUT;
    }

    aic_ftp_packet_release(&packet_ptr);
    ftp_client->nx_ftp_client_state = NX_FTP_STATE_CONNECTED;

    return AIC_SUCCESS;

ERROR_OUT:
    aic_ftp_packet_release(&packet_ptr);
    if (status)
        ftp_client_control_error_exit(aic_ftp_client, wait_option);

    return status;
}

u32 ftp_client_data_socket_disconnect(NX_FTP_CLIENT *client_ptr)
{
    u32 status;

    /* Ensure the client is in the proper state for closing the socket. */
    if (client_ptr->nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return NX_FTP_NOT_CONNECTED;
    else if ((client_ptr->nx_ftp_client_state != NX_FTP_STATE_OPEN) &&
             (client_ptr->nx_ftp_client_state != NX_FTP_STATE_WRITE_OPEN) &&
             (client_ptr->nx_ftp_client_state != NX_FTP_STATE_CONNECTED))
        return NX_FTP_NOT_OPEN;

    /**
     * Disconnect the data socket.Do not judge the status,
     * ensure the every step has been executed.
     */
    status = nx_tcp_socket_disconnect(
             &(client_ptr->nx_ftp_client_data_socket), NX_NO_WAIT);
    if(status)
        aicx_info(0x06316c, "", "FTP data socket maybe closed by server\n");

    if (client_ptr->nx_ftp_client_passive_transfer_enabled == NX_FALSE) {
        status = nx_tcp_server_socket_unaccept(
                 &(client_ptr->nx_ftp_client_data_socket));
        if(status)
            aicx_error(0x0249a2, "x", "FTP data socket maybe unaccepted(0x%x)\n", status);

        status = nx_tcp_server_socket_unlisten(
                client_ptr->nx_ftp_client_ip_ptr,
                client_ptr->nx_ftp_client_data_socket.nx_tcp_socket_port);
        if(status)
            aicx_error(0x1208b3, "x", "FTP data socket maybe unlistend(0x%x)\n", status);
    } else {
        status = nx_tcp_client_socket_unbind(
                 &(client_ptr->nx_ftp_client_data_socket));
        if(status)
            aicx_error(0x1e42ad, "", "FTP data socket maybe unbinded fail\n", status);
    }

    client_ptr->nx_ftp_client_block_total_size = 0;
    client_ptr->nx_ftp_client_block_remaining_size = 0;

    /* NX_FTP_STATE_CONNECTED means FTP control socket is connected */
    client_ptr->nx_ftp_client_state = NX_FTP_STATE_CONNECTED;

    return NX_SUCCESS;
}

u32 ftp_cilent_data_socket_connect(u32 handle, u32 open_type, ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;
    ftp_client = client_ptr->ftp_client;

    if ((open_type != NX_FTP_OPEN_FOR_READ) &&
        (open_type != NX_FTP_OPEN_FOR_WRITE) &&
        (open_type != AIC_FTP_OPEN_FOR_LIST))
        return AIC_FTP_OPTION_ERROR;

    if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTP_NOT_CONNECTED;
    else if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_OPEN)
        return AIC_FTP_NOT_CLOSED;
    else if (ftp_client->nx_ftp_client_state == NX_FTP_STATE_WRITE_OPEN)
        return AIC_FTP_NOT_CLOSED;

    if (ftp_client->nx_ftp_client_passive_transfer_enabled) {
        /* Set up the passive transfer mode:
           1.Send PASV request,
           2.Get server IP and port to connect to,
           3.Open data socket based on server info in PASV response. */
        status = ftp_client_passive_transfer_set(ftp_client, wait_option);
    } else {
        status = ftp_client_active_transfer_set(ftp_client, wait_option);
    }

    if (status) {
        aicx_error(0x132730, "x", "passive or active set fail(0x%x)\n", status);
        ftp_client_data_socket_disconnect(ftp_client);
        return status;
    }

    if (ftp_client->nx_ftp_client_passive_transfer_enabled == NX_FALSE) {
        status = nx_tcp_server_socket_accept(
                 &(ftp_client->nx_ftp_client_data_socket), wait_option);
        if (status) {
            aicx_error(0x12d119, "x", "waitting for data connection fail(0x%x)\n", status);
            ftp_client_data_socket_disconnect(ftp_client);
            return status;
        }
    }

    if (open_type == NX_FTP_OPEN_FOR_WRITE)
        ftp_client->nx_ftp_client_state = NX_FTP_STATE_WRITE_OPEN;

    if (open_type == NX_FTP_OPEN_FOR_READ)
        ftp_client->nx_ftp_client_state = NX_FTP_STATE_OPEN;

    if (open_type == AIC_FTP_OPEN_FOR_LIST)
        ftp_client->nx_ftp_client_state = NX_FTP_STATE_CONNECTED;

    return AIC_SUCCESS;
}

static u32 ftp_client_list_command_send(u32 handle, u8 *file_name,
                                        u8 *dir_path, ulong wait_option)
{
    u32 status;
    u8 *buff_ptr;
    NX_PACKET *packet_ptr;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;
    ftp_client = client_ptr->ftp_client;

    if (file_name != AIC_NULL) {
        status = aic_ftp_client_working_dir_set(handle, dir_path, wait_option);
        if (status) {
            aicx_error(0x0f185d, "x", "LIST dir set fail(0x%x)\n", status);
            return status;
        }
    }

    status = ftp_cilent_data_socket_connect(handle, AIC_FTP_OPEN_FOR_LIST,
                                            wait_option);
    if (status) {
        aicx_error(0x0fa073, "x", "LIST data socket connect fail(0x%x)\n", status);
        return status;
    }

    /* Allocate a packet for sending the LIST command. */
    status = ftp_client_packet_allocate(ftp_client, &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x1ab12b, "x", "packet allocate for LIST fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    if (file_name != AIC_NULL) {
        snprintf(buff_ptr, strlen(file_name) + 6, "LIST %s", file_name);
        buff_ptr[strlen(file_name) + 5] = 13;
        buff_ptr[strlen(file_name) + 6] = 10;
        packet_ptr->nx_packet_length = strlen(file_name) + 7;
    } else if (dir_path != AIC_NULL) {
        snprintf(buff_ptr, strlen(dir_path) + 6, "LIST %s", dir_path);
        buff_ptr[strlen(dir_path) + 5] = 13;
        buff_ptr[strlen(dir_path) + 6] = 10;
        packet_ptr->nx_packet_length = strlen(dir_path) + 7;
    }
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = nx_tcp_socket_send(&(ftp_client->nx_ftp_client_control_socket),
                                packet_ptr, wait_option);
    if (status) {
        aicx_error(0x1a48ad, "x", "Send LIST command fail(0x%x)\n", status);
        goto EXE_ERROR;
    }

    status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_control_socket),
                                   &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x098daf, "x", "LIST receive no response(0x%x)\n", status);
        goto EXE_ERROR;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '1')) {
        aicx_error(0x0cbe61, "", "LIST receive no 1xx message\n");
        status = AIC_FTP_EXPECTED_1XX_CODE;
        goto EXE_ERROR;
    }

EXE_ERROR:
    aic_ftp_packet_release(&packet_ptr);
    if (status)
        ftp_client_data_socket_disconnect(ftp_client);

    return status;
}

static void ftp_client_free_instance_space(AIC_FTP_CLIENT *client_ptr)
{
    if (client_ptr->ftp_client) {
        aos_free(client_ptr->ftp_client);
        client_ptr->ftp_client = AIC_NULL;
    }

    aos_free(client_ptr);

    return;
}

/* bind the source address through cid */
static u32 ftp_bind_source_ip(AIC_FTP_CLIENT *client_ptr, ulong wait_option)
{
    u32 status;
    NX_TCP_SOCKET *control_socket;
    NX_TCP_SOCKET *data_socket;

    control_socket = &(client_ptr->ftp_client->nx_ftp_client_control_socket);
    data_socket = &(client_ptr->ftp_client->nx_ftp_client_data_socket);

    if (client_ptr->server_ip.nxd_ip_version == NX_IP_VERSION_V4) {
        status = aic_tcp_client_source_ip_bind(control_socket, client_ptr->cid,
                                               AF_INET, wait_option);
        status = aic_tcp_client_source_ip_bind(data_socket, client_ptr->cid,
                                               AF_INET, wait_option);
    } else if (client_ptr->server_ip.nxd_ip_version == NX_IP_VERSION_V6) {
        status = aic_tcp_client_source_ip_bind(control_socket, client_ptr->cid,
                                               AF_INET6, wait_option);
        status = aic_tcp_client_source_ip_bind(data_socket, client_ptr->cid,
                                               AF_INET6, wait_option);
    } else {
        aicx_error(0x1c6ffa, "", "no valid ip addr to bind\n");
        status = AIC_FTP_BIND_ADDR_FAIL;
    }

    return status;
}

/**
 * Set ftp client passive mode.
 * @handle: ftp client handle.
 * @passive_mode_enabled: True to enable;False to disable
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftp_client_passive_mode_set(u32 handle, u32 passive_mode_enabled)
{
    AIC_FTP_CLIENT *client_ptr;
    client_ptr = (AIC_FTP_CLIENT *)handle;

    if ((client_ptr == AIC_NULL) || client_ptr->ftp_client == AIC_NULL ||
        (client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTP_PTR_ERROR;

    if (passive_mode_enabled != NX_TRUE && passive_mode_enabled != NX_FALSE)
        return AIC_FTP_INVALID_PARAMETERS;

    client_ptr->ftp_client->nx_ftp_client_passive_transfer_enabled =
        passive_mode_enabled;

    return AIC_SUCCESS;
}

/**
 * Set ftp client transfer mode.
 * @handle: ftp client handle.
 * @transfer_mode: Transfer mode.Only support NX_FTP_TRANSFER_MODE_STREAM
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftp_client_transfer_mode_set(u32 handle, u32 transfer_mode)
{
    AIC_FTP_CLIENT *client_ptr;
    client_ptr = (AIC_FTP_CLIENT *)handle;

    if ((client_ptr == AIC_NULL) || client_ptr->ftp_client == AIC_NULL ||
        (client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTP_PTR_ERROR;

    if (transfer_mode != NX_FTP_TRANSFER_MODE_STREAM)
        return AIC_FTP_INVALID_PARAMETERS;

    client_ptr->ftp_client->nx_ftp_client_transfer_mode = transfer_mode;

    return AIC_SUCCESS;
}

/**
 * Set ftp data representation type.
 * @handle: ftp client handle.
 * @data_type:data representation type.FTP_Binary or FTP_ASCII
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftp_client_data_type_set(u32 handle, u8 data_type)
{
    AIC_FTP_CLIENT *client_ptr;
    client_ptr = (AIC_FTP_CLIENT *)handle;

    if ((client_ptr == AIC_NULL) || client_ptr->ftp_client == AIC_NULL ||
        (client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTP_PTR_ERROR;

    client_ptr->data_type = data_type;

    return AIC_SUCCESS;
}
/**
 * Set cid used by FTP Client.
 * @handle: ftp client handle.
 * @cid:Context id.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */

/* Set cid used by HTTPS Client. */
u32 aic_ftp_client_cid_set(u32 handle, u8 cid)
{
    AIC_FTP_CLIENT *client_ptr;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == NX_NULL || client_ptr->ftp_client == NX_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    client_ptr->cid = cid;

    return AIC_SUCCESS;
}

/**
 * Creates a FTP client on the specified IP.
 *
 * @client_name: ftp client name.
 *
 * CALLED BY
 *    Application
 *
 * return ftp client handle for success.
 * return 0 for fail.
 */
u32 aic_ftp_client_create(u8 *client_name)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;

    if (client_name == NX_NULL)
        return AIC_FTP_CLIENT_CREATE_FAIL;

    client_ptr = aos_zalloc(sizeof(AIC_FTP_CLIENT));
    if (client_ptr == AIC_NULL) {
        aicx_error(0x0f76fe, "", "aic ftp client buf allocate fail\n");
        return AIC_FTP_CLIENT_CREATE_FAIL;
    }

    client_ptr->ftp_client_pool.per_packet_size = FTP_PACKET_SIZE;
    client_ptr->ftp_client_pool.packet_pool_size = FTP_PACKET_POOL_SIZE;
    client_ptr->ftp_client_pool.packet_pool_name = "ftp_client_packet_pool";

    status = aic_create_packet_pool(&(client_ptr->ftp_client_pool));
    if (status) {
        aicx_error(0x12734e, "x", "ftp packet pool create fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    client_ptr->aic_ftp_ip_instance = aic_get_default_ip_instance();
    if (!client_ptr->aic_ftp_ip_instance ||
        !client_ptr->aic_ftp_ip_instance->ip_ptr) {
        aicx_error(0x1de78f, "", "network maybe not active\n");
        goto ERROR_OUT;
    }
    client_ptr->ftp_ip_ptr = client_ptr->aic_ftp_ip_instance->ip_ptr;

    //DNS may be initialized when pdp is activated.
    status = aic_init_dns_client();
    if (status) {
        aicx_error(0x17ef14, "", "init DNS fail\n");
        goto ERROR_OUT;
    }

    client_ptr->ftp_client = aos_zalloc(sizeof(NX_FTP_CLIENT));
    if (client_ptr->ftp_client == AIC_NULL) {
        aicx_error(0x0db130, "", "ftp_client buf allocate fail\n");
        goto ERROR_OUT;
    }

    status = nx_ftp_client_create(client_ptr->ftp_client, client_name,
                                  client_ptr->ftp_ip_ptr,
                                  FTP_SOCKET_WINDOW_SIZE,
                                  client_ptr->ftp_client_pool.packet_pool);
    if (status) {
        aicx_error(0x0514b2, "x", "ftp client create fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    /* set default paras */
    client_ptr->ftp_client->nx_ftp_client_passive_transfer_enabled = AIC_TRUE;
    client_ptr->data_type = FTP_Binary;
    client_ptr->ftp_client->nx_ftp_client_transfer_mode =
        NX_FTP_TRANSFER_MODE_STREAM;

    return (u32)client_ptr;

ERROR_OUT:
    ftp_client_free_instance_space(client_ptr);
    client_ptr = AIC_NULL;
    return AIC_FTP_CLIENT_CREATE_FAIL;
}

/**
 * Delete a previously created FTP client on the specified IP.
 *
 * @handle: ftp client handle.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_ftp_client_delete(u32 handle, ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if ((client_ptr == AIC_NULL) || client_ptr->ftp_client == AIC_NULL ||
        (client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTP_PTR_ERROR;

    if (client_ptr->ftp_client->nx_ftp_client_state !=
        NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTP_NOT_DISCONNECTED;

    nx_tcp_socket_disconnect(
        &(client_ptr->ftp_client->nx_ftp_client_control_socket),
        wait_option);

    nx_tcp_client_socket_unbind(
        &(client_ptr->ftp_client->nx_ftp_client_control_socket));

    nx_tcp_socket_disconnect(
        &(client_ptr->ftp_client->nx_ftp_client_data_socket),
        wait_option);

    nx_tcp_client_socket_unbind(
        &(client_ptr->ftp_client->nx_ftp_client_data_socket));

    nx_tcp_socket_delete(&(client_ptr->ftp_client->nx_ftp_client_control_socket));
    nx_tcp_socket_delete(&(client_ptr->ftp_client->nx_ftp_client_data_socket));

    status = aic_delete_packet_pool(&(client_ptr->ftp_client_pool));
    if (status)
        aicx_error(0x120f3c, "x", "ftp packet pool delete fail(0x%x)\n", status);

    ftp_client_free_instance_space(client_ptr);
    client_ptr = AIC_NULL;

    return status;
}

/**
 * Send TYPE command to server to set data representation type.
 * @handle: ftp client handle.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftp_client_data_transfer_type_set(u32 handle, ulong wait_option)
{
    u32 status;
    u8 *buf_ptr;
    NX_PACKET *packet_ptr;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    if (client_ptr->ftp_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return AIC_FTP_NOT_CONNECTED;

    ftp_client = client_ptr->ftp_client;

    /* Allocate a packet for sending the TYPE command. */
    status = ftp_client_packet_allocate(ftp_client, &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x0a038a, "x", "packet allocate for TYPE fail(0x%x)\n", status);
        return status;
    }

    buf_ptr = packet_ptr->nx_packet_prepend_ptr;
    if (client_ptr->data_type == FTP_Binary)
        snprintf(buf_ptr, 7, "TYPE %c", 'I');

    if (client_ptr->data_type == FTP_ASCII)
        snprintf(buf_ptr, 7, "TYPE %c", 'A');

    buf_ptr[6] = 13;
    buf_ptr[7] = 10;
    packet_ptr->nx_packet_length = 8;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

    status = nx_tcp_socket_send(&(ftp_client->nx_ftp_client_control_socket),
                                packet_ptr, wait_option);
    if (status) {
        aicx_error(0x14e0cd, "x", "Send TYPE command fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_control_socket),
                                   &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x09e343, "x", "FTP_TYPE receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    buf_ptr = packet_ptr->nx_packet_prepend_ptr;
    if ((packet_ptr->nx_packet_length < 3) || (buf_ptr[0] != '2')) {
        aicx_error(0x037c04, "", "FTP_TYPE received no 200 message\n");
        status = AIC_FTP_EXPECTED_2XX_CODE;
        goto ERROR_OUT;
    }

ERROR_OUT:
    aic_ftp_packet_release(&packet_ptr);
    return status;
}

/**
 * connect a previously created FTP instance with the FTP server.
 * @handle: ftp client handle.
 * @url: FTP URL,such as ftp://192.168.0.17​.
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
u32 aic_ftp_client_connect(u32 handle, u8 *url, u8 *username, u8 *password,
                           u32 port, ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;

    client_ptr = (AIC_FTP_CLIENT *)handle;
    if ((client_ptr == AIC_NULL) || client_ptr->ftp_client == AIC_NULL ||
        (client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return AIC_FTP_PTR_ERROR;

    if (client_ptr->ftp_client->nx_ftp_client_state !=
        NX_FTP_STATE_NOT_CONNECTED)
        return AIC_FTP_NOT_DISCONNECTED;

    status = ftp_client_process_url(url, &(client_ptr->server_ip));
    if (status) {
        aicx_error(0x0e5a3e, "x", "process url fail(0x%x)\n", status);
        return status;
    }

    if (client_ptr->cid > 1 && client_ptr->cid <= ANET_MAX_PDP_CID) {
        status = ftp_bind_source_ip(client_ptr, wait_option);
        if (status != AIC_SUCCESS) {
            aicx_error(0x1966d4, "x", "bind source ip addr fail(0x%x)\n", status);
            return status;
        }
    }

    status = ftp_client_connect_internal(handle, &(client_ptr->server_ip),
                                         username, password, port,
                                         wait_option);

    return status;
}

/**
 * Disconnect a previously established FTP connection.
 * @handle: ftp client handle.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftp_client_disconnect(u32 handle, ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    ftp_client = client_ptr->ftp_client;
    status = nx_ftp_client_disconnect(ftp_client, wait_option);

    return status;
}

/**
 * Create the specified directory name on the FTP server.
 * @handle: FTP client handle.
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
u32 aic_ftp_client_dir_create(u32 handle, u8 *dir_path, u8 *dir_name,
                              ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    status = aic_ftp_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x07e35f, "x", "MKD working dir set fail(0x%x)\n", status);
        return status;
    }

    status = nx_ftp_client_directory_create(client_ptr->ftp_client, dir_name,
                                            wait_option);

    return status;
}

/**
 * Delete the specified directory name on the FTP server.
 * @handle: Ftp client handle.
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
u32 aic_ftp_client_dir_delete(u32 handle, u8 *dir_path, u8 *dir_name,
                              ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    status = aic_ftp_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x00e572, "x", "RMD working dir set fail(0x%x)\n", status);
        return status;
    }

    status = nx_ftp_client_directory_delete(client_ptr->ftp_client, dir_name,
                                            wait_option);

    return status;
}

/**
 * Set the default working directory on the FTP server.
 * @handle: ftp client handle.
 * @dir_path: New default directory path.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftp_client_working_dir_set(u32 handle, u8 *dir_path, ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    status = nx_ftp_client_directory_default_set(client_ptr->ftp_client,
                                                 dir_path, wait_option);

    return status;
}

/**
 * Get a listing for the specified directory on the FTP server.
 * @handle: ftp client handle.
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
u32 aic_ftp_client_dir_info_get(u32 handle, u8 *dir_path, u8* buf, u32 buf_len,
                                ulong *total_len, ulong wait_option)
{
    u32 status;
    NX_PACKET *data_res_packet;
    ulong bytes_copied = 0;
    ulong total_copied_bytes = 0;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    ftp_client = client_ptr->ftp_client;

    status = ftp_client_list_command_send(handle, AIC_NULL, dir_path,
                                          wait_option);
    if (status) {
        aicx_error(0x161655, "x", "LIST_D, list command send fail(0x%x)\n", status);
        return status;
    }

    /* Now see if this socket has data. If so, process all of it now! */
     do {
        status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_data_socket),
                 &data_res_packet, wait_option);
        if (status != NX_SUCCESS) {
            aicx_info(0x14cc39, "", "LIST_D, maybe no data in socket\n");
            break;
        }

        status = nx_packet_data_extract_offset(data_res_packet, 0,
                                               buf + total_copied_bytes,
                                               buf_len - total_copied_bytes,
                                               &bytes_copied);
        if (status) {
            aicx_error(0x12067c, "x", "LIST_D, extract packet fail(0x%x)\n", status);
            aic_ftp_packet_release(&data_res_packet);
            break;
        }
        total_copied_bytes += bytes_copied;
        aic_ftp_packet_release(&data_res_packet);
    } while (status == NX_SUCCESS && (total_copied_bytes < buf_len));
    *total_len = total_copied_bytes;

    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);

    return AIC_SUCCESS;
}

/**
 * Get the size of the specified file on the FTP server.
 * @handle: ftp client handle.
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
long aic_ftp_client_file_size_get(u32 handle, u8 *dir_path, u8 *file_name,
                                  ulong wait_option)
{
    u32 i = 0;
    u32 status;
    u8 *buffer_ptr;
    u8 *temp = AIC_NULL;
    const u8 *flag = " ";
    char *next = AIC_NULL;
    NX_PACKET *data_res_packet;
    long bytes = AIC_FTP_FILE_SIZE_GET_FAIL;
    ulong bytes_copied;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_FILE_SIZE_GET_FAIL;

    ftp_client = client_ptr->ftp_client;

    status = ftp_client_list_command_send(handle, file_name, dir_path,
                                          wait_option);
    if (status) {
        aicx_error(0x11fc81, "x", "LIST_F list command send fail(0x%x)\n", status);
        return AIC_FTP_FILE_SIZE_GET_FAIL;
    }

    /* Now see if this socket has data. If so, process all of it now! */
    do {
        status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_data_socket),
                 &data_res_packet, wait_option);
        if (status != NX_SUCCESS) {
            aicx_info(0x14fd35, "", "LIST_F maybe no data in socket\n");
            break;
        }

        buffer_ptr = aos_zalloc(data_res_packet->nx_packet_length);
        if (buffer_ptr == AIC_NULL) {
            aicx_error(0x1810ab, "", "LIST_F,buf allocate fail\n");
            aic_ftp_packet_release(&data_res_packet);
            break;
        }

        status = nx_packet_data_extract_offset(data_res_packet, 0, buffer_ptr,
                                               data_res_packet->nx_packet_length,
                                               &bytes_copied);
        if (status) {
            aicx_error(0x151444, "x", "LIST_F, extract packet fail(0x%x)\n", status);
            aic_ftp_packet_release(&data_res_packet);
            break;
        }

        temp = strtok_r(buffer_ptr, flag, &next);
        do {
            temp = strtok_r(AIC_NULL, flag, &next);
            aicx_debug(0x060e6d, "s", "LIST_F, temp is %s\n", temp);
            i++;
        } while (next && i < 4);
        bytes = atoi(temp);
        aic_ftp_packet_release(&data_res_packet);
        data_res_packet = AIC_NULL;
    } while (status == NX_SUCCESS);

    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);

    return bytes;
}

/**
 * Deletes the specified file on the FTP server.
 * @handle: ftp client handle.
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
u32 aic_ftp_client_file_delete(u32 handle, u8 *dir_path, u8 *file_name,
                               ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID ||
        file_name == NX_NULL)
        return AIC_FTP_PTR_ERROR;

    status = aic_ftp_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x041a94, "sx", "%s, working dir set fail(0x%x)\n", __func__, status);
        return status;
    }

    status = nx_ftp_client_file_delete(client_ptr->ftp_client, file_name,
                                       wait_option);

    return status;
}

/**
 * Rename the specified file with the supplied filename.
 * @handle: ftp client handle.
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
u32 aic_ftp_client_file_rename(u32 handle, u8 *dir_path, u8 *filename,
                               u8 *new_filename, ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID ||
        filename == NX_NULL || new_filename == NX_NULL)
        return AIC_FTP_PTR_ERROR;

    status = aic_ftp_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x041a94, "sx", "%s, working dir set fail(0x%x)\n", __func__, status);
        return status;
    }

    status = nx_ftp_client_file_rename(client_ptr->ftp_client, filename,
                                       new_filename, wait_option);

    return status;
}

/**
 * Deletes the specified file on the FTP server.
 * @handle: ftp client handle.
 * @offset: Download offset.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftp_client_download_offset_set(u32 handle, ulong offset,
                                       ulong wait_option)
{
    u32 status;
    u8 *res_buf;
    u8 send_buf[20] = {'\0'};
    NX_PACKET *packet_ptr;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;
    u32 size;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    ftp_client = client_ptr->ftp_client;

    if (ftp_client->nx_ftp_client_state != NX_FTP_STATE_CONNECTED &&
        ftp_client->nx_ftp_client_state != NX_FTP_STATE_OPEN &&
        ftp_client->nx_ftp_client_state != NX_FTP_STATE_WRITE_OPEN )
        return AIC_FTP_NOT_CONNECTED;

    status = ftp_client_packet_allocate(ftp_client, &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x0dba1d, "x", "packet allocate for REST fail(0x%x)\n", status);
        return status;
    }

    /* Check if out of boundary. */
    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 7) {
        aicx_error(0x007a2b, "", "packt buf for REST is too short\n");
        aic_ftp_packet_release(&packet_ptr);
        return AIC_FTP_FAILED;
    }

    /* Now build "REST" message to set offset. */
    send_buf[0] = 'R';
    send_buf[1] = 'E';
    send_buf[2] = 'S';
    send_buf[3] = 'T';
    send_buf[4] = ' ';

    snprintf(send_buf + 5, 10, "%ld", offset);
    size = strlen(send_buf);
    send_buf[size] = 13;
    send_buf[size + 1] = 10;
    status = nx_packet_data_append(packet_ptr, send_buf, size + 2,
                                   ftp_client->nx_ftp_client_packet_pool_ptr,
                                   wait_option);
    if (status != NX_SUCCESS) {
        aicx_error(0x06c9dd, "x", "packet append for REST fail(0x%x)\n", status);
        return status;
    }

    /* Send the REST message. */
     status = nx_tcp_socket_send(&(ftp_client->nx_ftp_client_control_socket),
                                 packet_ptr, wait_option);
     if (status) {
         aicx_error(0x04dd3a, "x", "Send REST command fail(0x%x)\n", status);
         aic_ftp_packet_release(&packet_ptr);
         return status;
     }

    status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_control_socket),
                                   &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x031f02, "x", "FTP_REST receive res fail(0x%x)\n", status);
        return status;
    }

    res_buf = packet_ptr->nx_packet_prepend_ptr;
    /* Check for 3xx message, signaling the "REST" was received properly. */
    if ((packet_ptr->nx_packet_length < 3) || (res_buf[0] != '3')) {
        aicx_error(0x105732, "", "FTP_REST received no 350 message\n");
        aic_ftp_packet_release(&packet_ptr);
        return AIC_FTP_EXPECTED_3XX_CODE;
    }

    aic_ftp_packet_release(&packet_ptr);
    return AIC_SUCCESS;
}

/**
 * Start ftp download.The downloaded file will be stored in fs:\ftp.
 * @handle: ftp client handle.
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
u32 aic_ftp_client_file_download(u32 handle, u8 *dir_path, u8 *file_name,
                                 ulong offset, ulong *download_bytes,
                                 ulong wait_option)
{
    u32 status;
    NX_PACKET *response_ptr;
    NX_PACKET *next_packet_ptr;
    u8 *local_file_name;
    int fd;
    ulong length;
    ulong count = 0;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;
    ssize_t rsize;
    long roffset;

    client_ptr = (AIC_FTP_CLIENT *)handle;

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

    status = aic_ftp_client_download_cmd_send(handle, dir_path, file_name,
                                              wait_option);
    if (status) {
        aicx_error(0x15bfcb, "x", "FTP download cmd send fail(0x%x)\n", status);
        return status;
    }

    /* now we can start file open and download */
    local_file_name = (u8 *)aos_zalloc(FTP_MAX_FILE_NAME_LEN + 8);
    snprintf(local_file_name, FTP_MAX_FILE_NAME_LEN + 8, "fs:\\ftp\\%s",
             file_name);

    aicx_debug(0x16e883, "s", "DL local_file_name is %s\n", local_file_name);

    fd = fs_open(local_file_name, FS_O_WRONLY | FS_O_CREATE);
    if (fd < 0) {
        aicx_error(0x1b79b9, "", "DL, open file fail\n");
        aos_free(local_file_name);
        local_file_name = AIC_NULL;
        status = AIC_FTP_FILE_OPEN_FAIL;
        goto ERROR_OUT1;
    } else {
        aicx_info(0x156b52, "", "DL, open file success\n");
    }

    if (offset) {
        roffset = fs_seek(fd, offset, FS_SEEK_BEGIN);
        if (roffset < 0 || roffset != offset) {
            aicx_error(0x03b46a, "", "DL, seek file fail\n");
            goto ERROR_OUT2;
        }
    }
#if 0
        else {
        /* Truncate the file to a size of 0. */
        status = fs_truncate(fd, 0);
        if (status) {
            aicx_error(0x15927c, "", "DL, truncate file fail\n");
            goto ERROR_OUT2;
        }
    }
#endif

    /* Now see if this socket has data. If so, process all of it now! */
    do {
        status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_data_socket),
                 &response_ptr, wait_option);
        if (status != NX_SUCCESS) {
            aicx_info(0x133064, "", "DL, maybe no data in socket\n");
            break;
        }

        length = response_ptr->nx_packet_length;
        next_packet_ptr = response_ptr;

        do {
            rsize = fs_write(fd, next_packet_ptr->nx_packet_prepend_ptr,
                             (next_packet_ptr->nx_packet_append_ptr -
                              next_packet_ptr->nx_packet_prepend_ptr));
            if (rsize == (next_packet_ptr->nx_packet_append_ptr -
                          next_packet_ptr->nx_packet_prepend_ptr))
                count += (ulong)(next_packet_ptr->nx_packet_append_ptr -
                                 next_packet_ptr->nx_packet_prepend_ptr);
            else
                aicx_error(0x1ba438, "x", "DL:file write fail(0x%x)\n", status);

            length = length - (ulong)(next_packet_ptr->nx_packet_append_ptr -
                                      next_packet_ptr->nx_packet_prepend_ptr);

#ifdef NX_DISABLE_PACKET_CHAIN
            next_packet_ptr = NX_NULL;
#else
            next_packet_ptr = next_packet_ptr->nx_packet_next;
#endif
        } while ((next_packet_ptr) && (length));
        aic_ftp_packet_release(&response_ptr);
    } while (status == NX_SUCCESS);
    *download_bytes = count;
    aicx_error(0x0e65b0, "d", "DL:download_bytes is %d\n", count);
    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);
    return AIC_SUCCESS;

ERROR_OUT1:
    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);
    return status;

ERROR_OUT2:
    fs_close(fd);
    aos_free(local_file_name);
    local_file_name = AIC_NULL;
    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);
    return status;
}

/**
 * Start ftp upload.The file will be upload is stored in fs:\ftp.
 * @handle: Ftp client handle.
 * @dir_path: File path.
 * @file_name: File to download.
 * @upload_type: Upload type.FTP_APPE or FTP_STOU or FTP_STOR.
 * @upload_bytes: Total upload file size.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_ftp_client_file_upload(u32 handle, u8 *dir_path, u8 *file_name,
                               u8 upload_type, ulong *upload_bytes,
                               ulong wait_option)
{
    u32 status;
    u8 *local_file_name = AIC_NULL;
    NX_PACKET *packet_ptr;
    int fd = 0;
    ulong length, bytes_to_copied;
    ulong total_bytes = 0;
    FS_FILE_INFO file_info;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;
    ssize_t rsize;

    client_ptr = (AIC_FTP_CLIENT *)handle;

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

    status = aic_ftp_client_download_cmd_send(handle, dir_path, file_name,
                                              wait_option);
    if (status) {
        aicx_error(0x1d93b8, "x", "FTP Upload cmd send fail(0x%x)\n", status);
        return status;
    }

    /* now we can start file open and upload */
    local_file_name = (u8 *)aos_zalloc(FTP_MAX_FILE_NAME_LEN + 8);
    snprintf(local_file_name, FTP_MAX_FILE_NAME_LEN + 8, "fs:\\ftp\\%s",
             file_name);

    fd = fs_open(local_file_name, FS_O_RDONLY);
    if (fd < 0) {
        aicx_error(0x093493, "", "UL open file fail\n");
        aos_free(local_file_name);
        local_file_name = NULL;
        status = AIC_FTP_FILE_OPEN_FAIL;
        goto ERROR_OUT;
    }

    status = fs_getinfo(local_file_name, &file_info);
    if (status) {
        aicx_error(0x03772d, "", "UL file info get fail\n");
        goto ERROR_OUT;
    }

    bytes_to_copied = file_info.size;
    aicx_debug(0x0259e9, "ld", "UL total_bytes is %ld", bytes_to_copied);

    if (bytes_to_copied <= 0) {
        aicx_error(0x11547d, "", "UL, upload file size is 0 or less than 0\n");
        status = AIC_FTP_FAILED;
        goto ERROR_OUT;
    }

    /* Now read the file and send the contents to the server. */
    while (status == NX_SUCCESS && bytes_to_copied) {
        status = ftp_client_packet_allocate(ftp_client, &packet_ptr,
                                            wait_option);
        if (status) {
            aicx_error(0x0aafd0, "x", "allocate packet for send file fail(0x%x)\n", status);
            goto ERROR_OUT;
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

            status = nx_ftp_client_file_write(ftp_client, packet_ptr,
                                              wait_option);
            if (status) {
                aicx_error(0x0cfd69, "x", "UL, send data to server fail(0x%x)\n", status);
                goto ERROR_OUT;
            } else {
                /* Update the remaining bytes in the file. */
                bytes_to_copied = bytes_to_copied - rsize;
                total_bytes += rsize;
            }
        } else {
            aicx_error(0x115a62, "", "UL, file read fail\n");
            goto ERROR_OUT;
        }
    }
    *upload_bytes = total_bytes;
    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);
    return AIC_SUCCESS;

ERROR_OUT:
    if (fd != 0)
        fs_close(fd);
    if (local_file_name)
        aos_free(local_file_name);
    local_file_name = AIC_NULL;
    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);
    aic_ftp_packet_release(&packet_ptr);

    return status;
}

u32 aic_ftp_client_download_cmd_send(u32 handle, u8 *dir_path, u8 *file_name,
                                     ulong wait_option)
{
    u32 status;
    NX_PACKET *packet_ptr;
    u8 *buff_ptr;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    ftp_client = client_ptr->ftp_client;
    status = aic_ftp_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x1297ef, "x", "DL directory setup fail(0x%x)\n", status);
        return status;
    }

    status = ftp_cilent_data_socket_connect(handle, NX_FTP_OPEN_FOR_READ,
                                            wait_option);
    if (status) {
        aicx_error(0x1185f3, "x", "DL data socket connect fail(0x%x)\n", status);
        return status;
    }

    ftp_control_socket_flush(ftp_client);
    status = ftp_client_packet_allocate(ftp_client, &packet_ptr,
                                        wait_option);
    if (status) {
        aicx_error(0x1c90a1, "x", "packet allocation for RETR fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 7) {
        aicx_error(0x14f4aa, "", "packt buf for RETR is too short\n");
        status = AIC_FTP_FAILED;
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    snprintf(buff_ptr, strlen(file_name) + 6, "RETR %s", file_name);
    buff_ptr[strlen(file_name) + 5] = 13;
    buff_ptr[strlen(file_name) + 6] = 10;
    packet_ptr->nx_packet_length = strlen(file_name) + 7;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;
     status = nx_tcp_socket_send(&(ftp_client->nx_ftp_client_control_socket),
                                 packet_ptr, wait_option);
     if (status) {
         aicx_error(0x1432ca, "x", "Send RETR command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }

    status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_control_socket),
                                   &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x1b2b62, "x", "FTP_RETR receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }
    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '1')) {
        aicx_error(0x0477c8, "", "FTP_RETR command receive no 1xx message\n");
        status = AIC_FTP_EXPECTED_1XX_CODE;
        goto ERROR_OUT;
    }
    aic_ftp_packet_release(&packet_ptr);
    return AIC_SUCCESS;

ERROR_OUT:
    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);
    aic_ftp_packet_release(&packet_ptr);
    return status;
}

u32 aic_ftp_client_upload_cmd_send(u32 handle, u8 *dir_path, u8 *file_name,
                                   u8 upload_type, ulong wait_option)
{
    u32 status;
    NX_PACKET *packet_ptr;
    u8 *buff_ptr;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;

    client_ptr = (AIC_FTP_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_PTR_ERROR;

    ftp_client = client_ptr->ftp_client;
    status = aic_ftp_client_working_dir_set(handle, dir_path, wait_option);
    if (status) {
        aicx_error(0x1297ef, "x", "DL directory setup fail(0x%x)\n", status);
        return status;
    }

    status = ftp_cilent_data_socket_connect(handle, NX_FTP_OPEN_FOR_READ,
                                            wait_option);
    if (status) {
        aicx_error(0x1185f3, "x", "DL data socket connect fail(0x%x)\n", status);
        return status;
    }

    ftp_control_socket_flush(ftp_client);
    status = ftp_client_packet_allocate(ftp_client, &packet_ptr,
                                        wait_option);
    if (status) {
        aicx_error(0x1c90a1, "x", "packet allocation for RETR fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    if ((u32)(packet_ptr->nx_packet_data_end -
        packet_ptr->nx_packet_prepend_ptr) < 7) {
        aicx_error(0x14f4aa, "", "packt buf for RETR is too short\n");
        status = AIC_FTP_FAILED;
        goto ERROR_OUT;
    }

    buff_ptr = packet_ptr->nx_packet_prepend_ptr;

    if (upload_type == FTP_APPE)
        snprintf(buff_ptr, strlen(file_name) + 6, "APPE %s", file_name);
    if (upload_type == FTP_STOU)
        snprintf(buff_ptr, strlen(file_name) + 6, "STOU %s", file_name);
    if (upload_type == FTP_STOR)
        snprintf(buff_ptr, strlen(file_name) + 6, "STOR %s", file_name);

    buff_ptr[strlen(file_name) + 5] = 13;
    buff_ptr[strlen(file_name) + 6] = 10;
    packet_ptr->nx_packet_length = strlen(file_name) + 7;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
                                       packet_ptr->nx_packet_length;

     status = nx_tcp_socket_send(&(ftp_client->nx_ftp_client_control_socket),
                                 packet_ptr, wait_option);
     if (status) {
         aicx_error(0x0854f4, "x", "Send UL command fail(0x%x)\n", status);
         goto ERROR_OUT;
     }
    status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_control_socket),
                                   &packet_ptr, wait_option);
    if (status) {
        aicx_error(0x192753, "x", "FTP UL receive res fail(0x%x)\n", status);
        goto ERROR_OUT;
    }
    buff_ptr = packet_ptr->nx_packet_prepend_ptr;
    if ((packet_ptr->nx_packet_length < 3) || (buff_ptr[0] != '1')) {
        aicx_error(0x19504d, "", "FTP UL command receive no 1xx message\n");
        status = AIC_FTP_EXPECTED_1XX_CODE;
        goto ERROR_OUT;
    }
    aic_ftp_packet_release(&packet_ptr);
    return AIC_SUCCESS;

ERROR_OUT:
    ftp_client_data_socket_disconnect(ftp_client);
    ftp_control_socket_flush(ftp_client);
    aic_ftp_packet_release(&packet_ptr);
    return status;
}

u32 aic_ftp_packet_release(NX_PACKET **packet_ptr_ptr)
{
    u32       status;
    NX_PACKET *packet_ptr;

    packet_ptr =  *packet_ptr_ptr;

    if (packet_ptr == AIC_NULL)
        return AIC_FTP_PTR_ERROR;

    status = nx_packet_release(packet_ptr);
    if (status == NX_SUCCESS)
        *packet_ptr_ptr = AIC_NULL;

    return status;
}

long aic_ftp_client_file_data_get(u32 handle, u8 *buf, u32 buf_len,
                                   ulong wait_option)
{
    u32 status;
    AIC_FTP_CLIENT *client_ptr;
    NX_FTP_CLIENT *ftp_client;
    ulong offset;
    long bytes_received = AIC_FTP_FILE_DATA_GET_FAIL;

    client_ptr = (AIC_FTP_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->ftp_client == AIC_NULL ||
        client_ptr->ftp_client->nx_ftp_client_id != NXD_FTP_CLIENT_ID)
        return AIC_FTP_FILE_DATA_GET_FAIL;

    ftp_client = client_ptr->ftp_client;
    do {
        if (client_ptr->recv_data_packet != AIC_NULL){
            break;
        } else {
            status = nx_tcp_socket_receive(&(ftp_client->nx_ftp_client_data_socket),
                                           &client_ptr->recv_data_packet, wait_option);

            if (status == NX_SUCCESS && client_ptr->recv_data_packet) {
                client_ptr->recv_packet_offset = 0;
                break;
            } else {
                aicx_error(0x073f32, "x", "ftp get data from server fail(0x%x)\n", status);
                ftp_client_data_socket_disconnect(ftp_client);
                ftp_control_socket_flush(ftp_client);
                return AIC_FTP_FILE_DATA_GET_FAIL;
            }
        }
    } while(1);

    offset = client_ptr->recv_packet_offset;
    status = nx_packet_data_extract_offset(client_ptr->recv_data_packet, offset,
                                           buf, buf_len, &bytes_received);
    if (status) {
        aicx_error(0x058944, "x", "ftp data extract fail(0x%x)\n", status);
        aic_ftp_packet_release(&client_ptr->recv_data_packet);
        client_ptr->recv_packet_offset = 0;
        return AIC_FTP_FILE_DATA_GET_FAIL;
    }

    offset = offset + bytes_received;
    if(client_ptr->recv_data_packet->nx_packet_length <= offset)
    {
        aic_ftp_packet_release(&client_ptr->recv_data_packet);
        client_ptr->recv_packet_offset = 0;
    } else {
        client_ptr->recv_packet_offset = offset;
    }

    return bytes_received;
}

