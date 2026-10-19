/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to HTTP.
 *  This is the handler of AT commands relating to http,
 *  such as setting url.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1831a6
#define LOG_MODULE    AMOD_NET

#include <string.h>
#include <strings.h>

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_resp_handle.h>
#include <aic_at_parse_utils.h>
#include <aic_http_comm.h>
#include <aic_http_at_process.h>
#include <aic_http_client.h>

static AIC_HTTP_AT_PARAS http_paras;

static AT_CMD_HANDLE http_at_handles[] =
{
    {"AT+HTTPINIT", aic_handle_http_init_test_cmd,
     AIC_NULL, AIC_NULL, aic_handle_http_init_exe_cmd},

    {"AT+HTTPPARA", aic_handle_http_para_test_cmd,
     aic_handle_http_para_query_cmd, aic_handle_http_para_set_cmd, AIC_NULL},

    {"AT+HTTPACTION", aic_handle_http_action_test_cmd, AIC_NULL,
     aic_handle_http_action_set_cmd, AIC_NULL},

    {"AT+HTTPDATA", aic_handle_http_data_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_http_data_exe_cmd},

    {"AT+HTTPREAD", aic_handle_http_read_test_cmd, AIC_NULL,
     aic_handle_http_read_set_cmd, AIC_NULL},

    {"AT+HTTPTERM", aic_handle_http_term_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_http_term_exe_cmd},

    {AIC_NULL},
};

static u32 transmit_http_para_integer(AT_REQ_RESP *at_ptr, u8 *tag, u32 value)
{
    u8 result_line0[HTTPS_RESLEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, HTTPS_RESLEN, "+HTTPPARA: \"%s\",%d", tag, value);
    line_p[0] = (ulong)(&result_line0);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_http_str(AT_REQ_RESP *at_ptr, u8 *tag, u8 *str)
{
    u8 result[HTTPS_MAX_PARA_RES_LEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    if (tag)
        snprintf(result, HTTPS_MAX_PARA_RES_LEN, "+HTTPPARA: \"%s\",\"%s\"",
                 tag, str);
    else
        strncpy(result, str, HTTPS_MAX_PARA_RES_LEN);

    line_p[0] = (ulong)(&result);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_http_at_ok(AT_REQ_RESP *at_ptr)
{
    u8 result_line[HTTPS_MIN_RESLEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line, AT_RESULT_OK, strlen(AT_RESULT_OK));
    line_p[0] = (ulong)(&result_line);

    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_http_at_cme(AT_REQ_RESP *at_ptr, u32 err)
{
    u8 result[HTTPS_CME_STR_LEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result, HTTPS_CME_STR_LEN, "+CME ERROR: %d", err);
    line_p[0] = (ulong)(&result);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_http_status(AT_REQ_RESP *at_ptr, u8 http_method,
                                u8 *status, u32 content_len)
{
    u8 result_line0[HTTPS_RESLEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, HTTPS_RESLEN, "+HTTPACTION: %d,%s,%d", http_method,
             status, content_len);
    line_p[0] = (ulong)(&result_line0);

    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static void http_paras_reset(void)
{
    aos_memset(&http_paras, 0, sizeof(http_paras));

    http_paras.cid = 1;
    http_paras.range_start = -1;
    http_paras.range_end = -1;
    http_paras.timeout = HTTP_DEFAULT_TIMEOUT;

    return;
}

static void http_response_process(AT_REQ_RESP *at_ptr, u8 method)
{
    u32 status, get_status = AIC_SUCCESS;
    u32 content_length = 0;

    http_paras.redir_count = 0;
    do {
        if (method == AIC_HTTPS_METHOD_POST || method == AIC_HTTPS_METHOD_PUT) {
            status = aic_https_client_put_packet(http_paras.handle,
                                                 http_paras.client_ptr->put_packet,
                                                 http_paras.timeout);
            if (status) {
                aicx_error(0x1a0068, "x", "send packet to server fail(0x%x)\n", status);
                if (status == NX_WEB_HTTP_NOT_READY ||
                    status == NX_WEB_HTTP_AUTHENTICATION_ERROR ||
                    status == NX_WEB_HTTP_INCOMPLETE_PUT_ERROR ||
                    status == NX_WEB_HTTP_BAD_PACKET_LENGTH)
                    aic_http_packet_release(&http_paras.client_ptr->put_packet);

                transmit_http_at_cme(at_ptr, AIC_HTTP_SEND_PACKET_FAIL);
                goto OUT;
            }
       }

        aic_http_packet_release(&http_paras.client_ptr->res_data_packet);
        http_paras.client_ptr->packet_offset = 0;
        aos_memset(http_paras.client_ptr->status_code, 0, 4);
        /* Receive response data from the server, to get https_status_str. */
        get_status = aic_https_client_response_body_get(http_paras.handle,
                     &(http_paras.client_ptr->res_data_packet),
                     http_paras.client_ptr->status_code, http_paras.timeout);

        if (get_status != NX_SUCCESS && get_status != NX_WEB_HTTP_GET_DONE &&
            get_status != AIC_HTTPS_REDIR_STATE) {
            aicx_error(0x0f980a, "x", "receive response fail(0x%x)\n", get_status);
            if (strlen(http_paras.client_ptr->status_code) != 0) {
                transmit_http_status(at_ptr, method,
                                     http_paras.client_ptr->status_code,
                                     content_length);
                transmit_http_at_ok(at_ptr);
            } else {
                transmit_http_at_cme(at_ptr, AIC_HTTP_GET_RESPONSE_FAIL);
            }
            goto OUT;
        } else {
            content_length =
            http_paras.client_ptr->https_client->nx_web_http_client_total_receive_bytes;
        }
        http_paras.redir_count++;
        /* wait http redir process in http_client_redir_process_entry */
        if (http_paras.redir == 1)
            aos_sleep(500);
    } while (http_paras.redir == 1 && get_status == AIC_HTTPS_REDIR_STATE &&
             http_paras.redir_count <= HTTP_MAX_REDIR_COUNT);

    if (http_paras.redir_count > HTTP_MAX_REDIR_COUNT) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_REDIR_EXECUTE_FAIL);
    } else {
        transmit_http_status(at_ptr, method,
                             http_paras.client_ptr->status_code, content_length);
        transmit_http_at_ok(at_ptr);
    }

OUT:
    if (method == AIC_HTTPS_METHOD_POST || method == AIC_HTTPS_METHOD_PUT) {
        http_paras.post_data_size = 0;
        http_paras.client_ptr->put_packet = AIC_NULL;
    }

    return;
}

/**
 * Execution command initialize the HTTP service.
 * Such as creat HTTP client.
 * For example: AT+HTTPINIT
 */
u32 aic_handle_http_init_exe_cmd(AT_REQ_RESP *at_ptr)
{

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+HTTPINIT")] != 0x0D &&
        at_ptr->at_req[strlen("AT+HTTPINIT")] != 0x0A) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_AT_PARAS_ERROR);
        return AIC_HTTP_AT_PARAS_ERROR;
    }

    if (http_paras.handle != 0) {
        aicx_error(0x0f7bcd, "", "HTTP Service is running, no need init\n");
        transmit_http_at_ok(at_ptr);
        return AIC_SUCCESS;
    }

    http_paras.handle = aic_http_client_create(HTTP_AT_CLIENT_NAME);
    if (http_paras.handle == 0) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_INIT_ERROR);
        return AIC_HTTP_INIT_ERROR;
    }

    http_paras.client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)(http_paras.handle);
    transmit_http_at_ok(at_ptr);
    return AIC_SUCCESS;
}

u32 aic_handle_http_init_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_http_str(at_ptr, AIC_NULL, "+HTTPINIT");
    transmit_http_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command set the HTTP parameters value.
 * Such as service URL, user agent, etc.
 * For example: AT+HTTPPARA = "CID",1
 *              AT+HTTPPARA="CONTENT","text/plain"
 */
u32 aic_handle_http_para_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPPARA=",
                                        &count);
    if (!para || count != 2) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_AT_PARAS_ERROR);
        aic_free_paras_mem(para, count);
        return AIC_HTTP_AT_PARAS_ERROR;
    }

    if (strncasecmp((u8*)para[0], "CID", 3) == 0) {
        http_paras.cid = atoi((u8 *)para[1]);
        if (http_paras.cid <= 0 || http_paras.cid > ANET_MAX_PDP_CID)
            goto PARA_OUT_RANGE;
    } else if (strncasecmp((u8*)para[0], "URL", 3) == 0) {
        if (strlen((u8 *)para[1]) > HTTPS_MAX_INPUT_PARA_LEN)
            goto PARA_OUT_RANGE;
        strncpy(http_paras.url, (u8 *)para[1], HTTPS_MAX_INPUT_PARA_LEN);
    } else if (strncasecmp((u8*)para[0], "UA", 2) == 0) {
        if (strlen((u8 *)para[1]) > HTTPS_MAX_INPUT_PARA_LEN)
            goto PARA_OUT_RANGE;
        strncpy(http_paras.ua, (u8 *)para[1], HTTPS_MAX_INPUT_PARA_LEN);
    } else if (strncasecmp((u8*)para[0], "REDIR", 5) == 0) {
        http_paras.redir = atoi((u8 *)para[1]);
    } else if (strncasecmp((u8*)para[0], "BREAK", 5) == 0) {
        http_paras.range_start = atoi((u8 *)para[1]);
    } else if (strncasecmp((u8*)para[0], "END", 3) == 0) {
        http_paras.range_end = atoi((u8 *)para[1]);
    } else if (strncasecmp((u8*)para[0], "TIMEOUT", 7) == 0) {
        if (atoi((u8 *)para[1]) < 5 || atoi((u8 *)para[1]) > 1000)
            goto PARA_OUT_RANGE;
        http_paras.timeout = NX_IP_PERIODIC_RATE * atoi((u8 *)para[1]);
    } else if (strncasecmp((u8*)para[0], "CONTENT", 7) == 0) {
        if (strlen((u8 *)para[1]) > HTTPS_CONTENT_TYPE_LEN)
            goto PARA_OUT_RANGE;
        strncpy(http_paras.content_type, (u8 *)para[1],
                HTTPS_CONTENT_TYPE_LEN);
    } else if (strncasecmp((u8*)para[0], "USERDATA", 8) == 0) {
        if (strlen((u8 *)para[1]) > HTTPS_MAX_INPUT_PARA_LEN)
            goto PARA_OUT_RANGE;
        strncpy(http_paras.user_data, (u8 *)para[1],
                HTTPS_MAX_INPUT_PARA_LEN);
    } else {
        aicx_error(0x0e3a81, "", "http not support the AT cmd\n");
        goto PARA_OUT_RANGE;
    }

    transmit_http_at_ok(at_ptr);
    aic_free_paras_mem(para, count);

    return AIC_SUCCESS;

PARA_OUT_RANGE:
    transmit_http_at_cme(at_ptr, AIC_HTTP_PARA_OUT_OF_RANGE);
    aic_free_paras_mem(para, count);

    return AIC_HTTP_PARA_OUT_OF_RANGE;
}

u32 aic_handle_http_para_query_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_http_para_integer(at_ptr, "CID", http_paras.cid);
    transmit_http_str(at_ptr, "URL", http_paras.url);
    transmit_http_str(at_ptr, "UA", http_paras.ua);
    transmit_http_para_integer(at_ptr, "REDIR", http_paras.redir);
    transmit_http_para_integer(at_ptr, "BREAK", http_paras.range_start);
    transmit_http_para_integer(at_ptr, "END", http_paras.range_end);
    transmit_http_para_integer(at_ptr, "TIMEOUT", http_paras.timeout);
    transmit_http_str(at_ptr, "CONTENT", http_paras.content_type);
    transmit_http_str(at_ptr, "USERDATA", http_paras.user_data);
    transmit_http_at_ok(at_ptr);
    return AIC_SUCCESS;
}

u32 aic_handle_http_para_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_http_str(at_ptr, AIC_NULL, "+HTTPPARA: \"ParaTag\",\"ParaValue\"");
    transmit_http_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command to input the HTTP data.The data can be send out by
 * the HTTP action like POST.
 * For example: AT+HTTPDATA=1024
 */
u32 aic_handle_http_data_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u32 status = AIC_SUCCESS;
    ulong temp_size;
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (http_paras.handle == 0) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_NO_CREATED_CLIENT);
        return AIC_HTTP_NO_CREATED_CLIENT;
    }

    /*AT mode */
    if (at_ptr->at_mode == AT_CMD_MODE) {
        para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPDATA=",
                                            &count);
        if (!para || count != 1) {
            transmit_http_at_cme(at_ptr, AIC_HTTP_AT_PARAS_ERROR);
            goto CMD_OUT;
        }

        http_paras.post_data_size = atoi((u8 *)para[0]);
        if (http_paras.post_data_size < 1 ||
            http_paras.post_data_size > HTTP_MAX_POST_DATA_LEN) {
            transmit_http_at_cme(at_ptr, AIC_HTTP_AT_PARAS_ERROR);
            goto CMD_OUT;
        }

        at_ptr->at_mode = AT_DATA_MODE;
        aicx_info(0x00161d, "", "change to AT data mode\n");
        transmit_http_str(at_ptr, AIC_NULL, "DATA INPUT");
CMD_OUT:
        aic_free_paras_mem(para, count);
        return AIC_SUCCESS;
    }

    /*data mode*/
    if (at_ptr->at_mode == AT_DATA_MODE) {

        if (at_ptr->end_data_mode) {
            aic_http_packet_release(&http_paras.client_ptr->put_packet);
            at_ptr->at_mode = AT_CMD_MODE;
            return AIC_SUCCESS;
        }

        if (strncmp(at_ptr->at_upstream_data, "+++", 3) == 0) {
            at_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x157142, "", "change to AT command mode\n");
            transmit_http_str(at_ptr, AIC_NULL, "DATA INPUT END");
            return AIC_SUCCESS;
        }

        /* Allocate a packet. */
        if (http_paras.client_ptr->put_packet == AIC_NULL) {
            status = nx_packet_allocate(
                     http_paras.client_ptr->https_client->nx_web_http_client_packet_pool_ptr,
                     &(http_paras.client_ptr->put_packet), NX_TCP_PACKET,
                     http_paras.timeout);
            if (status) {
                aicx_error(0x1ddcdd, "x", "packet allocate fail(0x%x)\n", status);
                transmit_http_at_cme(at_ptr, AIC_HTTP_PACKET_ALLOCATE_ERROR);
                status = AIC_HTTP_PACKET_ALLOCATE_ERROR;
                goto OUT;
            }
        }
        temp_size = at_ptr->upstream_data_len;
        if (http_paras.client_ptr->put_packet->nx_packet_length +
            temp_size > http_paras.post_data_size) {
            temp_size = http_paras.post_data_size -
                        http_paras.client_ptr->put_packet->nx_packet_length;
        }

        /* Build the post packet. */
        status = nx_packet_data_append(http_paras.client_ptr->put_packet,
                 at_ptr->at_upstream_data, temp_size,
                 http_paras.client_ptr->https_client->nx_web_http_client_packet_pool_ptr,
                 http_paras.timeout);
        if (status) {
            aicx_error(0x11a347, "", "packet append fail\n");
            transmit_http_at_cme(at_ptr, AIC_HTTP_DATA_APPEND_ERROR);
            status = AIC_HTTP_DATA_APPEND_ERROR;
        }
    }

OUT:
    if (status) {
        aic_http_packet_release(&http_paras.client_ptr->put_packet);
        return status;
    } else {
        transmit_http_at_ok(at_ptr);
        return AIC_SUCCESS;
    }
}

u32 aic_handle_http_data_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_http_str(at_ptr, AIC_NULL, "+HTTPDATA: (1~12288)");
    transmit_http_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Submit the HTTP request.
 * It executes the GET/POST/HEAD/DELETE operations
 * For example: AT+HTTPACTION=1
 */
u32 aic_handle_http_action_set_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status = AIC_SUCCESS;
    ulong *para;
    u16 count;
    u8 mode = 0;
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPACTION=",
                                        &count);
    if (!para || count != 1) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_AT_PARAS_ERROR);
        aic_free_paras_mem(para, count);
        return AIC_HTTP_AT_PARAS_ERROR;
    }

    mode = atoi((u8 *)para[0]);

    if (http_paras.handle == 0) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_NO_CREATED_CLIENT);
        goto OUT;
    }

    if (http_paras.cid > 0 && http_paras.cid <= ANET_MAX_PDP_CID)
        aic_https_client_set_cid(http_paras.handle, http_paras.cid);

    if (http_paras.redir)
        http_paras.client_ptr->https_client->redir_flag = http_paras.redir;

    switch (mode) {
    case AIC_HTTPS_METHOD_GET:
        if (http_paras.range_start < 0 && http_paras.range_end < 0)
            status = aic_http_client_get_start(http_paras.handle, http_paras.url,
                     AIC_NULL, AIC_NULL, 0, http_paras.timeout);
        else
            status = aic_http_client_range_get_start(http_paras.handle,
                     http_paras.url, AIC_NULL, AIC_NULL, 0,
                     http_paras.range_start, http_paras.range_end,
                     http_paras.timeout);
        break;
    case AIC_HTTPS_METHOD_POST:
        if (http_paras.client_ptr->put_packet == AIC_NULL) {
            transmit_http_at_cme(at_ptr, AIC_HTTP_INVALID_SEND_PACKET);
            goto OUT;
        }
        if (strlen(http_paras.content_type) != 0)
            status = aic_http_client_post_start_ext(http_paras.handle,
                     http_paras.url, AIC_NULL, AIC_NULL, 0,
                     http_paras.client_ptr->put_packet->nx_packet_length,
                     http_paras.content_type, http_paras.timeout);
        else
            status = aic_http_client_post_start(http_paras.handle,
                     http_paras.url, AIC_NULL, AIC_NULL, 0,
                     http_paras.client_ptr->put_packet->nx_packet_length,
                     http_paras.timeout);
        break;
    case AIC_HTTPS_METHOD_PUT:
        if (http_paras.client_ptr->put_packet == AIC_NULL) {
            transmit_http_at_cme(at_ptr, AIC_HTTP_INVALID_SEND_PACKET);
            goto OUT;
        }
        status = aic_http_client_put_start(http_paras.handle, http_paras.url,
                 AIC_NULL, AIC_NULL, 0,
                 http_paras.client_ptr->put_packet->nx_packet_length,
                 http_paras.timeout);
        break;
    case AIC_HTTPS_METHOD_HEAD:
        status = aic_http_client_head_start(http_paras.handle, http_paras.url,
                 AIC_NULL, AIC_NULL, 0, http_paras.timeout);
        break;
    case AIC_HTTPS_METHOD_DELETE:
        status = aic_http_client_delete_start(http_paras.handle, http_paras.url,
                 AIC_NULL, AIC_NULL, 0, http_paras.timeout);
        break;
    default :
        aicx_error(0x062dc5, "", "invalid HTTP method\n");
        transmit_http_at_cme(at_ptr, AIC_HTTP_INVALID_METHOD);
        break;
    }

    if (status) {
        aicx_error(0x112825, "x", "HTTP method fail(0x%x)\n", status);
        aic_http_packet_release(&http_paras.client_ptr->put_packet);
        transmit_http_at_cme(at_ptr, AIC_HTTP_METHOD_EXECUTE_FAIL);
        goto OUT;
    }

    http_response_process(at_ptr, mode);

OUT:
    aic_free_paras_mem(para, count);
    return AIC_SUCCESS;
}

u32 aic_handle_http_action_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_http_str(at_ptr, AIC_NULL, "+HTTPACTION: (1~5)");
    transmit_http_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command read the HTTP service response.
 * For example: AT+HTTPREAD=20
 */
u32 aic_handle_http_read_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u8 *receive_buffer;
    u16 count;
    long read_bytes = 0;
    long offset = 0;
    long bytes_to_copied = 0;
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];
    u8 result_line0[HTTPS_RESLEN] = {'\0'};

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPREAD=", &count);
    if (!para || count != 1) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_AT_PARAS_ERROR);
        goto OUT;
    }

    if (http_paras.handle == 0) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_NO_CREATED_CLIENT);
        goto OUT;
    }

    http_paras.read_data_len = atoi((u8 *)para[0]);
    bytes_to_copied = http_paras.read_data_len;
    if (bytes_to_copied < 1 || bytes_to_copied > 319488) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_AT_PARAS_ERROR);
        goto OUT;
    }

    receive_buffer = (u8 *)aos_zalloc(http_paras.read_data_len);
    if (receive_buffer == AIC_NULL) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_BUFFER_ALLOCATE_FAIL);
        goto OUT;
    }

    do {
        read_bytes = aic_https_client_response_data_get(http_paras.handle,
                                                        receive_buffer + offset,
                                                        bytes_to_copied,
                                                        http_paras.timeout);

        if (read_bytes <= 0) {
            aicx_error(0x173c06, "", "receive no data from server\n");
            break;
        }
        offset += read_bytes;
        bytes_to_copied -= read_bytes;
    } while (bytes_to_copied);

    if (offset >= http_paras.read_data_len) {
        snprintf(result_line0, HTTPS_RESLEN, "+HTTPREAD: %ld",
                 http_paras.read_data_len);
        line_p[0] = (ulong)(&result_line0);
        aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);
        aic_transmit_at_data(at_ptr->serial_port_id, receive_buffer,
                             http_paras.read_data_len);
    } else {
        snprintf(result_line0, HTTPS_RESLEN, "+HTTPREAD: %ld", offset);
        line_p[0] = (ulong)(&result_line0);
        aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);
        aic_transmit_at_data(at_ptr->serial_port_id, receive_buffer,
                             offset);
    }

    aos_free(receive_buffer);
    receive_buffer = AIC_NULL;
    transmit_http_at_ok(at_ptr);
OUT:
    aic_free_paras_mem(para, count);

    return AIC_SUCCESS;
}

u32 aic_handle_http_read_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_http_str(at_ptr, AIC_NULL, "+HTTPREAD: (1~319488)");
    transmit_http_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command terminate the actived HTTP service.
 * For example: AT+HTTPTERM
 */
u32 aic_handle_http_term_exe_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status = AIC_SUCCESS;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+HTTPTERM")] != 0x0D &&
        at_ptr->at_req[strlen("AT+HTTPTERM")] != 0x0A) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_AT_PARAS_ERROR);
        return AIC_HTTP_AT_PARAS_ERROR;
    }

    if (http_paras.handle == 0) {
        transmit_http_at_cme(at_ptr, AIC_HTTP_NO_CREATED_CLIENT);
        return AIC_HTTP_NO_CREATED_CLIENT;
    }

    aic_http_packet_release(&http_paras.client_ptr->put_packet);
    aic_http_packet_release(&http_paras.client_ptr->res_data_packet);

    /* Clear out the HTTP client. */
    status = aic_http_client_delete(http_paras.handle);
    if (status)
        transmit_http_at_cme(at_ptr, AIC_HTTP_TERM_ERROR);
    else
        transmit_http_at_ok(at_ptr);

    http_paras_reset();
    return AIC_SUCCESS;
}

u32 aic_handle_http_term_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_http_str(at_ptr, AIC_NULL, "+HTTPTERM");
    transmit_http_at_ok(at_ptr);

    return AIC_SUCCESS;
}

int aic_http_service_init(void)
{
    http_paras_reset();
    aic_add_at_cmd_handle(&http_at_handles[0]);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_http_service_init);
