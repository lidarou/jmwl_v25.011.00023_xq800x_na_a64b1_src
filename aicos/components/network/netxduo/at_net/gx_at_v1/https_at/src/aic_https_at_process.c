/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to HTTPS.
 *  This is the handler of AT commands relating to https,
 *  such as setting url.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10a462
#define LOG_MODULE    AMOD_NET

#include <string.h>
#include <strings.h>
#ifdef CONFIG_FS
#include <fs_api.h>
#endif

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_resp_handle.h>
#include <aic_at_parse_utils.h>
#include <aic_https_client.h>
#include <aic_https_at_process.h>

static AIC_HTTPS_AT_PARAS https_paras;
static AIC_HTTPS_AT_CRT_INFO crt_info;

static AT_CMD_HANDLE https_at_handles[] =
{
    {"AT+HTTPSCFG", aic_handle_https_cfg_test_cmd,
     aic_handle_https_cfg_query_cmd, aic_handle_https_cfg_set_cmd, AIC_NULL},

    {"AT+HTTPSGET", aic_handle_https_get_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_https_get_exe_cmd},

    {"AT+HTTPSPOST", aic_handle_https_post_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_https_post_exe_cmd},

    {"AT+HTTPSGETEXT", aic_handle_https_range_get_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_https_range_get_exe_cmd},

    {"AT+HTTPSDATA", aic_handle_https_data_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_https_data_exe_cmd},

    {"AT+HTTPSREAD", aic_handle_https_read_test_cmd, AIC_NULL,
     aic_handle_https_read_set_cmd, AIC_NULL},

    {"AT+HTTPSTERM", aic_handle_https_term_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_https_term_exe_cmd},

    {"AT+HTTPSSETCRT", aic_handle_https_setcrt_test_cmd, AIC_NULL,
     aic_handle_https_setcrt_set_cmd, AIC_NULL},

    {"AT+HTTPSINIT", aic_handle_https_init_test_cmd, AIC_NULL, AIC_NULL,
     aic_handle_https_init_exe_cmd},

    {"AT+HTTPSCRTADD", aic_handle_https_setcrt_add_test_cmd,
     AIC_NULL, aic_handle_https_setcrt_add_cmd, AIC_NULL},

    {AIC_NULL},
};

static u32 transmit_https_para_integer(AT_REQ_RESP *at_ptr, u8 *tag, u32 value)
{
    u8 result_line0[HTTPS_RESLEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, HTTPS_RESLEN, "+HTTPSCFG: \"%s\",%d", tag, value);
    line_p[0] = (ulong)(&result_line0);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_https_str(AT_REQ_RESP *at_ptr, u8 *tag, u8 *str)
{
    u8 result[HTTPS_MAX_PARA_RES_LEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    if (tag)
        snprintf(result, HTTPS_MAX_PARA_RES_LEN, "+HTTPSCFG: \"%s\",\"%s\"",
                 tag, str);
    else
        strncpy(result, str, HTTPS_MAX_PARA_RES_LEN);

    line_p[0] = (ulong)(&result);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_https_at_ok(AT_REQ_RESP *at_ptr)
{
    u8 result_line[HTTPS_MIN_RESLEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line, AT_RESULT_OK, strlen(AT_RESULT_OK));
    line_p[0] = (ulong)(&result_line);

    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_https_at_cme(AT_REQ_RESP *at_ptr, u32 err)
{
    u8 result_line1[HTTPS_CME_STR_LEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line1, HTTPS_CME_STR_LEN, "+CME ERROR: %d", err);
    line_p[0] = (ulong)(&result_line1);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_https_status(AT_REQ_RESP *at_ptr, u8 *tag, u8 *status,
                                 u32 content_len)
{
    u8 result_line0[HTTPS_RESLEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];
    snprintf(result_line0, HTTPS_RESLEN, "%s%s,%d", tag, status, content_len);
    line_p[0] = (ulong)(&result_line0);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 https_at_action_prepare(AT_REQ_RESP *at_ptr)
{

    if (https_paras.handle == 0) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_NO_CREATED_CLIENT);
        return AIC_HTTPS_NO_CREATED_CLIENT;
    }

    if (https_paras.crt_flag == AIC_HTTPS_INIT_STATE) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_CRT_INFO_EMPTY);
        return AIC_HTTPS_CRT_INFO_EMPTY;
    }

    if (https_paras.cid > 0 && https_paras.cid <= ANET_MAX_PDP_CID)
        aic_https_client_set_cid(https_paras.handle, https_paras.cid);

    if (https_paras.redir)
        https_paras.client_ptr->https_client->redir_flag = https_paras.redir;

    return AIC_SUCCESS;
}

static void https_response_process(AT_REQ_RESP *at_ptr, u8 method, u8 *urc_str)
{
    u32 status, get_status = AIC_SUCCESS;
    u32 content_length = 0;

    https_paras.redir_count = 0;
    do {
        if (method == AIC_HTTPS_METHOD_POST && https_paras.post_buf) {
            status = aic_https_client_put_data(https_paras.handle,
                                               https_paras.post_buf,
                                               https_paras.data_copied,
                                               https_paras.timeout);
            if (status) {
                aicx_error(0x1a0068, "x", "send packet to server fail(0x%x)\n", status);
                aos_free(https_paras.post_buf);
                https_paras.post_buf = AIC_NULL;
                transmit_https_at_cme(at_ptr, AIC_HTTPS_SEND_PACKET_FAIL);
                return;
            }
            aos_free(https_paras.post_buf);
            https_paras.post_buf = AIC_NULL;
        }

        aic_http_packet_release(&https_paras.client_ptr->res_data_packet);
        https_paras.client_ptr->packet_offset = 0;
        aos_memset(https_paras.client_ptr->status_code, 0, 4);
        /* Receive response data from the server, to get status_code. */
        get_status = aic_https_client_response_body_get(https_paras.handle,
                     &(https_paras.client_ptr->res_data_packet),
                     https_paras.client_ptr->status_code, https_paras.timeout);

        if (get_status != NX_SUCCESS && get_status != NX_WEB_HTTP_GET_DONE &&
            get_status != AIC_HTTPS_REDIR_STATE) {
            aicx_error(0x0f980a, "x", "receive response fail(0x%x)\n", get_status);
            if (strlen(https_paras.client_ptr->status_code) != 0) {
                transmit_https_status(at_ptr, urc_str,
                                      https_paras.client_ptr->status_code,
                                      content_length);
                transmit_https_at_ok(at_ptr);
            } else {
                transmit_https_at_cme(at_ptr, AIC_HTTPS_GET_RESPONSE_FAIL);
            }
            return;
        } else {
            content_length =
            https_paras.client_ptr->https_client->nx_web_http_client_total_receive_bytes;
        }
        https_paras.redir_count++;
        /* wait https redir process in http_client_redir_process_entry */
        if (https_paras.redir == 1)
            aos_sleep(500);
    } while (https_paras.redir ==1 &&  get_status == AIC_HTTPS_REDIR_STATE &&
             https_paras.redir_count <= HTTPS_MAX_REDIR_COUNT);

    if (https_paras.redir_count > HTTPS_MAX_REDIR_COUNT) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_REDIR_EXECUTE_FAIL);
    } else {
        transmit_https_status(at_ptr, urc_str,
                              https_paras.client_ptr->status_code,
                              content_length);
        transmit_https_at_ok(at_ptr);
    }

    return;
}

static void https_paras_reset(void)
{
    aos_memset(&https_paras, 0, sizeof(AIC_HTTPS_AT_PARAS));

    https_paras.cid = 1;
    https_paras.range_start = -1;
    https_paras.range_end = -1;
    https_paras.timeout = HTTPS_DEFAULT_TIMEOUT;
    https_paras.crt_flag = AIC_HTTPS_INIT_STATE;

    return;
}

static void https_at_crt_reset(void)
{
    if (crt_info.ca_crt)
        aos_free(crt_info.ca_crt);
    if (crt_info.client_crt)
        aos_free(crt_info.client_crt);
    if (crt_info.priv_key)
        aos_free(crt_info.priv_key);

    aos_memset(&crt_info, 0, sizeof(AIC_HTTPS_AT_CRT_INFO));
    return;
}

/**
 * Execution command set the HTTPS parameters value.
 * Such as service URL, user agent, etc.
 * For example: AT+HTTPSCFG = "CID",1
 *              AT+HTTPSCGF="CONTENT","text/plain"
 */
u32 aic_handle_https_cfg_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);
    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPSCFG=", &count);
    if (!para || count != 2 ) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        aic_free_paras_mem(para, count);
        return AIC_HTTPS_AT_PARAS_ERROR;
    }

    if (strncasecmp((u8*)para[0], "CID", 3) == 0) {
        https_paras.cid = atoi((u8 *)para[1]);
        if (https_paras.cid <= 0 || https_paras.cid > ANET_MAX_PDP_CID)
            goto PARA_OUT_RANGE;
    } else if (strncasecmp((u8*)para[0], "URL", 3) == 0) {
        if (strlen((u8 *)para[1]) > HTTPS_MAX_INPUT_PARA_LEN)
            goto PARA_OUT_RANGE;
        strncpy(https_paras.url, (u8 *)para[1], HTTPS_MAX_INPUT_PARA_LEN);
    } else if (strncasecmp((u8*)para[0], "UA", 2) == 0) {
        if (strlen((u8 *)para[1]) > HTTPS_MAX_INPUT_PARA_LEN)
            goto PARA_OUT_RANGE;
        strncpy(https_paras.ua, (u8 *)para[1], HTTPS_MAX_INPUT_PARA_LEN);
    } else if (strncasecmp((u8*)para[0], "REDIR", 5) == 0) {
        https_paras.redir = atoi((u8 *)para[1]);
    } else if (strncasecmp((u8*)para[0], "BREAK", 5) == 0) {
        https_paras.range_start = atoi((u8 *)para[1]);
    } else if (strncasecmp((u8*)para[0], "END", 3) == 0) {
        https_paras.range_end = atoi((u8 *)para[1]);
    } else if (strncasecmp((u8*)para[0], "TIMEOUT", 7) == 0) {
        if (atoi((u8 *)para[1]) < 5 || atoi((u8 *)para[1]) > 1000)
            goto PARA_OUT_RANGE;
        https_paras.timeout = NX_IP_PERIODIC_RATE * atoi((u8 *)para[1]);
    } else if (strncasecmp((u8*)para[0], "CONTENT", 7) == 0) {
        if (strlen((u8 *)para[1]) > HTTPS_CONTENT_TYPE_LEN)
            goto PARA_OUT_RANGE;
        strncpy(https_paras.content_type, (u8 *)para[1],
                HTTPS_CONTENT_TYPE_LEN);
    } else if (strncasecmp((u8*)para[0], "USERDATA", 8) == 0) {
        if (strlen((u8 *)para[1]) > HTTPS_MAX_INPUT_PARA_LEN)
            goto PARA_OUT_RANGE;
        strncpy(https_paras.user_data, (u8 *)para[1],
                HTTPS_MAX_INPUT_PARA_LEN);
    } else {
        aicx_error(0x05213d, "", "https not support the AT cmd\n");
        goto PARA_OUT_RANGE;
    }

    transmit_https_at_ok(at_ptr);
    aic_free_paras_mem(para, count);

    return AIC_SUCCESS;

PARA_OUT_RANGE:
    transmit_https_at_cme(at_ptr, AIC_HTTPS_PARA_OUT_OF_RANGE);
    aic_free_paras_mem(para, count);

    return AIC_HTTPS_PARA_OUT_OF_RANGE;
}

u32 aic_handle_https_cfg_query_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_para_integer(at_ptr, "CID", https_paras.cid);
    transmit_https_str(at_ptr, "URL", https_paras.url);
    transmit_https_str(at_ptr, "UA", https_paras.ua);
    transmit_https_para_integer(at_ptr, "REDIR", https_paras.redir);
    transmit_https_para_integer(at_ptr, "BREAK", https_paras.range_start);
    transmit_https_para_integer(at_ptr, "END", https_paras.range_end);
    transmit_https_para_integer(at_ptr, "TIMEOUT", https_paras.timeout);
    transmit_https_str(at_ptr, "CONTENT", https_paras.content_type);
    transmit_https_str(at_ptr, "USERDATA", https_paras.user_data);
    transmit_https_at_ok(at_ptr);
    return AIC_SUCCESS;
}

u32 aic_handle_https_cfg_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSCFG: \"ParaTag\",\"ParaValue\"");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command initialize the HTTPS service.
 * Such as creat HTTPS client.
 * For example: AT+HTTPSINIT
 */
u32 aic_handle_https_init_exe_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status = AIC_SUCCESS;
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+HTTPSINIT")] != 0x0D &&
        at_ptr->at_req[strlen("AT+HTTPSINIT")] != 0x0A) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        return AIC_HTTPS_AT_PARAS_ERROR;
    }

    if (https_paras.handle != 0) {
        aicx_error(0x14b8d8, "", "HTTPS Service is running, no need init\n");
        transmit_https_at_ok(at_ptr);
        return AIC_SUCCESS;
    }

    if (https_paras.crt_flag == AIC_HTTPS_INIT_STATE) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_CRT_INFO_EMPTY);
        return AIC_HTTPS_CRT_INFO_EMPTY;
    }

    https_paras.handle = aic_https_client_create(HTTPS_AT_CLIENT_NAME);
    if (https_paras.handle == 0) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_CLIENT_CREATE_ERROR);
        return AIC_HTTPS_CLIENT_CREATE_ERROR;
    }

    https_paras.client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)(https_paras.handle);

    https_paras.client_ptr->https_client->auth_type = MBEDTLS_SSL_VERIFY_OPTIONAL;

    if (https_paras.crt_flag == AIC_HTTPS_CA_CRT) {
        status = aic_https_ca_crt_data_add(https_paras.handle, crt_info.ca_crt,
                                           (u16)(crt_info.ca_crt_len),
                                           HTTPS_CA_CRT_ID);
    }

    if (https_paras.crt_flag == AIC_HTTPS_CLIENT_CRT ||
        https_paras.crt_flag == AIC_HTTPS_PRIV_KEY) {
#ifdef CONFIG_NET_SECURE
        status = aic_https_client_crt_data_add(https_paras.handle,
                 crt_info.client_crt, (u16)(crt_info.client_crt_len),
                 crt_info.priv_key, crt_info.priv_key_len,
                 NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER, HTTPS_CLIENT_CRT_ID);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
        https_paras.client_ptr->https_client->http_mb_cfg->recv_wait_time =
                                                    https_paras.timeout;

        status = aic_https_client_crt_data_add(https_paras.handle,
                 crt_info.client_crt, (u16)(crt_info.client_crt_len),
                 crt_info.priv_key, crt_info.priv_key_len, 0,
                 HTTPS_CLIENT_CRT_ID);
#endif
    }
    if (status) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_CRT_SET_ERROR);
        aic_https_client_delete(https_paras.handle);
        https_paras.handle = 0;
        return AIC_HTTPS_CRT_SET_ERROR;
    }

    transmit_https_at_ok(at_ptr);
    return AIC_SUCCESS;
}

u32 aic_handle_https_init_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSINIT");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Submit the HTTPS GET request.
 * For example: AT+HTTPSGET
 */
u32 aic_handle_https_get_exe_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status;
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+HTTPSGET")] != 0x0D &&
        at_ptr->at_req[strlen("AT+HTTPSGET")] != 0x0A) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        return AIC_HTTPS_AT_PARAS_ERROR;
    }

    if (https_at_action_prepare(at_ptr))
        return AIC_HTTPS_METHOD_EXECUTE_FAIL;

    status = aic_https_client_get_start(https_paras.handle, https_paras.url,
                                        AIC_NULL, AIC_NULL,
                                        NX_WEB_HTTPS_SERVER_PORT,
                                        https_paras.timeout);

    if (status) {
        aicx_info(0x17c8ef, "x", "HTTPS GET method fail(0x%x)\n", status);
        transmit_https_at_cme(at_ptr, AIC_HTTPS_METHOD_EXECUTE_FAIL);

        return AIC_HTTPS_METHOD_EXECUTE_FAIL;
    }

    https_response_process(at_ptr, AIC_HTTPS_METHOD_GET, "+HTTPSGET: ");

    return AIC_SUCCESS;
}

u32 aic_handle_https_get_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSGET");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Submit the HTTPS POST request.
 * For example: AT+HTTPSPOST
 */
u32 aic_handle_https_post_exe_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status;
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+HTTPSPOST")] != 0x0D &&
        at_ptr->at_req[strlen("AT+HTTPSPOST")] != 0x0A) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        return AIC_HTTPS_AT_PARAS_ERROR;
    }

    if (https_at_action_prepare(at_ptr))
        return AIC_HTTPS_METHOD_EXECUTE_FAIL;

    if (https_paras.post_buf == AIC_NULL) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_POST_DATA_EMPTY);
        return AIC_HTTPS_POST_DATA_EMPTY;
    }

    if (strlen(https_paras.content_type) != 0)
        status = aic_https_client_post_start_ext(https_paras.handle,
                                                 https_paras.url,
                                                 AIC_NULL, AIC_NULL,
                                                 NX_WEB_HTTPS_SERVER_PORT,
                                                 https_paras.data_copied,
                                                 https_paras.content_type,
                                                 https_paras.timeout);
    else
        status = aic_https_client_post_start(https_paras.handle,
                                             https_paras.url,
                                             AIC_NULL, AIC_NULL,
                                             NX_WEB_HTTPS_SERVER_PORT,
                                             https_paras.data_copied,
                                             https_paras.timeout);
    if (status) {
        aicx_info(0x18b10b, "x", "HTTPS POST method fail(0x%x)\n", status);
        transmit_https_at_cme(at_ptr, AIC_HTTPS_METHOD_EXECUTE_FAIL);
        return AIC_HTTPS_METHOD_EXECUTE_FAIL;
    }

    https_response_process(at_ptr, AIC_HTTPS_METHOD_POST, "+HTTPSPOST: ");

    return AIC_SUCCESS;
}

u32 aic_handle_https_post_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSPOST");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Submit the HTTPS RANGE GET request.
 * This AT can specify the GET range.
 * For example: AT+HTTPSGETEXT
 */
u32 aic_handle_https_range_get_exe_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status;
    aicx_info(0x129483, "s", "at cmd is:--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+HTTPSGETEXT")] != 0x0D &&
        at_ptr->at_req[strlen("AT+HTTPSGETEXT")] != 0x0A) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        return AIC_HTTPS_AT_PARAS_ERROR;
    }

    if (https_at_action_prepare(at_ptr))
        return AIC_HTTPS_METHOD_EXECUTE_FAIL;

    status = aic_https_client_range_get_start(https_paras.handle,
                                              https_paras.url,
                                              AIC_NULL, AIC_NULL,
                                              NX_WEB_HTTPS_SERVER_PORT,
                                              https_paras.range_start,
                                              https_paras.range_end,
                                              https_paras.timeout);

    if (status) {
        aicx_info(0x01a4c5, "x", "HTTPS GET_R method fail(0x%x)\n", status);
        transmit_https_at_cme(at_ptr, AIC_HTTPS_METHOD_EXECUTE_FAIL);
        return AIC_HTTPS_METHOD_EXECUTE_FAIL;
    }

    https_response_process(at_ptr, AIC_HTTPS_METHOD_GET, "+HTTPSGET: ");

    return AIC_SUCCESS;
}

u32 aic_handle_https_range_get_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSGETEXT");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command to input the HTTPS data.The data can be send out by
 * the HTTPS POST.
 * For example: AT+HTTPSDATA=1024
 */
u32 aic_handle_https_data_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para = AIC_NULL;
    u16 count;
    ulong temp_size;

    aicx_info(0x129483, "s", "at cmd is:--%s\n", at_ptr->at_req);

    if (https_paras.handle == 0) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_NO_CREATED_CLIENT);
        return AIC_HTTPS_NO_CREATED_CLIENT;
    }

    /* AT mode */
    if (at_ptr->at_mode == AT_CMD_MODE) {

        para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPSDATA=",
                                            &count);
        if (!para || count != 1) {
            transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
            goto OUT;
        }

        https_paras.post_data_size = atoi((u8 *)para[0]);
        if (https_paras.post_data_size <= 0) {
            transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
            goto OUT;
        }

        if (https_paras.post_buf) {
            aicx_info(0x0ccff7, "", "clearn old post buf\n");
            aos_free(https_paras.post_buf);
            https_paras.post_buf = AIC_NULL;
        }

        https_paras.post_buf =
                    (u8 *)aos_zalloc(https_paras.post_data_size);
        if (https_paras.post_buf == AIC_NULL) {
            transmit_https_at_cme(at_ptr, AIC_HTTPS_BUF_ALLOCATE_FAIL);
            goto OUT;
        }
        https_paras.data_copied = 0;

        at_ptr->at_mode = AT_DATA_MODE;
        aicx_info(0x00161d, "", "change to AT data mode\n");
        transmit_https_str(at_ptr, AIC_NULL, "DATA INPUT");
OUT:
        aic_free_paras_mem(para, count);
        return AIC_SUCCESS;
    }

    /* data mode */
    if (at_ptr->at_mode == AT_DATA_MODE) {

        if (at_ptr->end_data_mode) {
            https_paras.data_copied = 0;
            if (https_paras.post_buf) {
                aos_free(https_paras.post_buf);
                https_paras.post_buf = AIC_NULL;
            }
            at_ptr->at_mode = AT_CMD_MODE;
            return AIC_SUCCESS;
        }

        if (strncmp(at_ptr->at_upstream_data, "+++", 3) == 0) {
            at_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x157142, "", "change to AT command mode\n");
            transmit_https_str(at_ptr, AIC_NULL, "DATA INPUT END");
            return AIC_SUCCESS;
        }

        temp_size = at_ptr->upstream_data_len;
        //alog_info("temp_size is %d\n", temp_size);
        if (https_paras.data_copied + temp_size > https_paras.post_data_size) {
            temp_size = https_paras.post_data_size - https_paras.data_copied;
        }
        aos_memcpy(https_paras.post_buf + https_paras.data_copied,
                   at_ptr->at_upstream_data, temp_size);
        https_paras.data_copied += temp_size;
        //alog_info("bytes_copied is %d\n", https_paras.data_copied);
    }

    transmit_https_at_ok(at_ptr);
    return AIC_SUCCESS;
}

u32 aic_handle_https_data_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSDATA: (1~12288)");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command read the HTTPS service response.
 * For example: AT+HTTPSREAD=20
 */
u32 aic_handle_https_read_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u8 *receive_buffer;
    u16 count;
    long read_bytes = 0;
    long offset = 0;
    long bytes_to_copied = 0;
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];
    u8 result_line0[HTTPS_RESLEN] = {'\0'};

    aicx_info(0x129483, "s", "at cmd is:--%s\n", at_ptr->at_req);
    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPSREAD=", &count);
    if (!para || count != 1) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        goto OUT;
    }

    https_paras.read_data_len = atoi((u8 *)para[0]);
    bytes_to_copied = https_paras.read_data_len;
    if (bytes_to_copied < 1 || bytes_to_copied > 319488) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        goto OUT;
    }

    if (https_paras.handle == 0) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_NO_CREATED_CLIENT);
        goto OUT;
    }

    receive_buffer = (u8 *)aos_zalloc(https_paras.read_data_len);
    if (receive_buffer == AIC_NULL) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_BUF_ALLOCATE_FAIL);
        goto OUT;
    }

    do {
        read_bytes = aic_https_client_response_data_get(https_paras.handle,
                                                        receive_buffer + offset,
                                                        bytes_to_copied,
                                                        https_paras.timeout);

        if (read_bytes <= 0) {
            aicx_error(0x173c06, "", "receive no data from server\n");
            break;
        }
        offset += read_bytes;
        bytes_to_copied -= read_bytes;
    } while (bytes_to_copied);

    if (offset >= https_paras.read_data_len) {
        snprintf(result_line0, HTTPS_RESLEN, "+HTTPSREAD: %lu",
                 https_paras.read_data_len);
        line_p[0] = (ulong)(&result_line0);
        aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);
        aic_transmit_at_data(at_ptr->serial_port_id, receive_buffer,
                             https_paras.read_data_len);
    } else {
        snprintf(result_line0, HTTPS_RESLEN, "+HTTPSREAD: %ld", offset);
        line_p[0] = (ulong)(&result_line0);
        aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);
        aic_transmit_at_data(at_ptr->serial_port_id, receive_buffer,
                             offset);
    }

    aos_free(receive_buffer);
    receive_buffer = AIC_NULL;
    transmit_https_at_ok(at_ptr);

OUT:
    aic_free_paras_mem(para, count);
    return AIC_SUCCESS;
}

u32 aic_handle_https_read_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSREAD: (1~319488)");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command set the HTTPS ca certificate.
 * For example: AT+HTTPSSETCRT=1,"fs:\\crts\\test.der"
 */
u32 aic_handle_https_setcrt_set_cmd(AT_REQ_RESP *at_ptr)
{
#ifdef CONFIG_FS
    ulong *para;
    u32 status = AIC_HTTPS_CRT_SET_ERROR;
    int ret;
    u16 count;
    int fd = -1;
    FS_FILE_INFO *file_info = AIC_NULL;
    ssize_t rsize;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);
    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPSSETCRT=",
                                        &count);
    if (!para || count != 2) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        goto OUT;
    }

    https_paras.crt_flag = atoi((u8 *)para[0]);
    if (https_paras.crt_flag < 1 || https_paras.crt_flag > 3) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        goto OUT;
    }

    if (strlen((u8 *)para[1]) > AIC_HTTPS_MAX_CRT_PATH_LEN ||
        strlen((u8 *)para[1]) <= 0) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        goto OUT;
    }

    fd = fs_open((u8 *)para[1], FS_O_RDONLY);
    if (fd < 0) {
        aicx_error(0x071b0d, "s", "open crt file fail, patch is %s!\n", (u8 *)para[1]);
        goto OUT;
    }

    file_info = aos_zalloc(sizeof(FS_FILE_INFO));
    ret = fs_getinfo((u8 *)para[1], file_info);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0f9a7e, "", "get crt file info fail!\n");
        goto OUT;
    }

    /* set ca crt */
    if (https_paras.crt_flag == AIC_HTTPS_CA_CRT) {
        crt_info.ca_crt = aos_zalloc(file_info->size);
        if (crt_info.ca_crt == AIC_NULL)
            goto OUT;

        crt_info.ca_crt_len = file_info->size;
        rsize = fs_read(fd, crt_info.ca_crt, crt_info.ca_crt_len);
        if (rsize <= 0 || rsize != crt_info.ca_crt_len) {
            aicx_error(0x146b55, "", "read CA crt file fail\n");
            goto OUT;
        }
    }

    /* set client crt */
    if (https_paras.crt_flag == AIC_HTTPS_CLIENT_CRT) {
        crt_info.client_crt = aos_zalloc(file_info->size);
        if (crt_info.client_crt == AIC_NULL)
            goto OUT;

        crt_info.client_crt_len = file_info->size;
        rsize = fs_read(fd, crt_info.client_crt, crt_info.client_crt_len);
        if (rsize <= 0 || rsize != crt_info.client_crt_len) {
            aicx_error(0x131596, "", "read Client crt file fail\n");
            goto OUT;
        }
    }

    /*set private key */
    if (https_paras.crt_flag == AIC_HTTPS_PRIV_KEY) {
        crt_info.priv_key = aos_zalloc(file_info->size);
        if (crt_info.priv_key == AIC_NULL)
            goto OUT;

        crt_info.priv_key_len = file_info->size;
        rsize = fs_read(fd, crt_info.priv_key, crt_info.priv_key_len);
        if (rsize <= 0 || rsize != crt_info.priv_key_len) {
            aicx_error(0x034305, "", "read Client key file fail\n");
            goto OUT;
        }
    }

    status = AIC_SUCCESS;

OUT:
    if (fd > 0)
        fs_close(fd);
    if (file_info) {
        aos_free(file_info);
        file_info = AIC_NULL;
    }

    if (status) {
        https_at_crt_reset();
        transmit_https_at_cme(at_ptr, AIC_HTTPS_CRT_SET_ERROR);
    } else {
        transmit_https_at_ok(at_ptr);
    }

    aic_free_paras_mem(para, count);
    return status;
#endif
    transmit_https_at_cme(at_ptr, AIC_HTTPS_CRT_SET_ERROR);
    return AIC_HTTPS_CRT_SET_ERROR;
}

u32 aic_handle_https_setcrt_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL,
                       "+HTTPSSETCRT: (1~3),\"crt_path\"");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command add the HTTPS certificate.
 * For example: AT+HTTPSCRTADD=1,767
 */
u32 aic_handle_https_setcrt_add_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para = AIC_NULL;
    u16 count;
    u32 temp_size;

    aicx_info(0x129483, "s", "at cmd is:--%s\n", at_ptr->at_req);

    /* AT mode */
    if (at_ptr->at_mode == AT_CMD_MODE) {
        para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+HTTPSCRTADD=",
                                            &count);
        if (!para || count != 2) {
            transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
            goto CRT_ADD_ERROR;
        }

        https_paras.crt_flag = atoi((u8 *)para[0]);
        if (https_paras.crt_flag < 1 || https_paras.crt_flag > 3) {
            transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
            goto CRT_ADD_ERROR;
        }

        https_paras.crt_len = atoi((u8 *)para[1]);
        if (https_paras.crt_len <= 0) {
            transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);;
            goto CRT_ADD_ERROR;
        }

        switch(https_paras.crt_flag) {
        case AIC_HTTPS_CA_CRT:
            if (crt_info.ca_crt) {
                aos_free(crt_info.ca_crt);
                crt_info.ca_crt = AIC_NULL;
                crt_info.ca_crt_len = 0;
            }
            crt_info.ca_crt = aos_zalloc(https_paras.crt_len);
            if (crt_info.ca_crt == AIC_NULL) {
                transmit_https_at_cme(at_ptr, AIC_HTTPS_BUF_ALLOCATE_FAIL);
                goto CRT_ADD_ERROR;
            }
            break;
        case AIC_HTTPS_CLIENT_CRT:
            if (crt_info.client_crt) {
                aos_free(crt_info.client_crt);
                crt_info.client_crt = AIC_NULL;
                crt_info.ca_crt_len = 0;
            }
            crt_info.client_crt = aos_zalloc(https_paras.crt_len);
            if (crt_info.client_crt == AIC_NULL) {
                transmit_https_at_cme(at_ptr, AIC_HTTPS_BUF_ALLOCATE_FAIL);
                goto CRT_ADD_ERROR;
            }
            break;
        case AIC_HTTPS_PRIV_KEY:
            if (crt_info.priv_key) {
                aos_free(crt_info.priv_key);
                crt_info.priv_key = AIC_NULL;
                crt_info.priv_key_len = 0;
            }
            crt_info.priv_key = aos_zalloc(https_paras.crt_len);
            if (crt_info.priv_key == AIC_NULL) {
                transmit_https_at_cme(at_ptr, AIC_HTTPS_BUF_ALLOCATE_FAIL);
                goto CRT_ADD_ERROR;
            }
            break;
        default:
            goto CRT_ADD_ERROR;
        }

        at_ptr->at_mode = AT_DATA_MODE;
        aicx_info(0x00161d, "", "change to AT data mode\n");
        transmit_https_str(at_ptr, AIC_NULL, "CRT SET START");

CRT_ADD_ERROR:
        aic_free_paras_mem(para, count);
        return AIC_SUCCESS;
    }

    /* data mode */
    if (at_ptr->at_mode == AT_DATA_MODE) {

        if (at_ptr->end_data_mode) {
            transmit_https_str(at_ptr, AIC_NULL, "SET CRT TIMEOUT");
            at_ptr->at_mode = AT_CMD_MODE;
            return AIC_SUCCESS;
        }

        if (strncmp(at_ptr->at_upstream_data, "+++", 3) == 0) {
            at_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x157142, "", "change to AT command mode\n");
            transmit_https_str(at_ptr, AIC_NULL, "CRT SET END");
            return AIC_SUCCESS;
        }

        /* set ca crt */
        if (https_paras.crt_flag == AIC_HTTPS_CA_CRT && crt_info.ca_crt) {
            temp_size = at_ptr->upstream_data_len;
            //alog_info("temp_size is %d\n", temp_size);

            if (crt_info.ca_crt_len + temp_size > https_paras.crt_len)
                temp_size = https_paras.crt_len - crt_info.ca_crt_len;

            aos_memcpy(crt_info.ca_crt + crt_info.ca_crt_len,
                       at_ptr->at_upstream_data, temp_size);
            crt_info.ca_crt_len += temp_size;

            //alog_info("crt_info.ca_crt_len is %d\n", crt_info.ca_crt_len);
        } else if (https_paras.crt_flag == AIC_HTTPS_CLIENT_CRT &&
                   crt_info.client_crt) {
            temp_size = at_ptr->upstream_data_len;
            if (crt_info.client_crt_len + temp_size > https_paras.crt_len)
                temp_size = https_paras.crt_len - crt_info.client_crt_len;

            aos_memcpy(crt_info.client_crt + crt_info.client_crt_len,
                       at_ptr->at_upstream_data, temp_size);
            crt_info.client_crt_len += temp_size;
        } else if (https_paras.crt_flag == AIC_HTTPS_PRIV_KEY &&
                   crt_info.priv_key) {
            temp_size = at_ptr->upstream_data_len;
            if (crt_info.priv_key_len + temp_size > https_paras.crt_len)
                temp_size = https_paras.crt_len - crt_info.priv_key_len;

            aos_memcpy(crt_info.priv_key + crt_info.priv_key_len,
                       at_ptr->at_upstream_data, temp_size);
            crt_info.priv_key_len += temp_size;
        }
    }

    return AIC_SUCCESS;
}

u32 aic_handle_https_setcrt_add_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSCRTADD: (1~3),<crt_len>");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command terminate the actived HTTPS service.
 * For example: AT+HTTPSTERM
 */
u32 aic_handle_https_term_exe_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status = 0;
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+HTTPSTERM")] != 0x0D &&
        at_ptr->at_req[strlen("AT+HTTPSTERM")] != 0x0A) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_AT_PARAS_ERROR);
        return AIC_HTTPS_AT_PARAS_ERROR;
    }

    if (https_paras.handle == 0) {
        transmit_https_at_cme(at_ptr, AIC_HTTPS_NO_CREATED_CLIENT);
        return AIC_HTTPS_NO_CREATED_CLIENT;
    }

    /* Clear out the post buffer. */
    if (https_paras.post_buf) {
        aos_free(https_paras.post_buf);
        https_paras.post_buf = AIC_NULL;
    }

    aic_http_packet_release(&https_paras.client_ptr->put_packet);
    aic_http_packet_release(&https_paras.client_ptr->res_data_packet);

    /* Clear out the HTTPS certificates. */
    if (https_paras.crt_flag == AIC_HTTPS_CA_CRT)
        status = aic_https_ca_crt_remove(https_paras.handle, HTTPS_CA_CRT_ID);
    else if (https_paras.crt_flag == AIC_HTTPS_CLIENT_CRT ||
             https_paras.crt_flag == AIC_HTTPS_PRIV_KEY)
        status = aic_https_client_crt_remove(https_paras.handle,
                                             HTTPS_CLIENT_CRT_ID);

    if (status) {
        aicx_info(0x01a4a7, "x", "crt remove status is 0x%x\n", status);
        transmit_https_at_cme(at_ptr, AIC_HTTPS_TERM_ERROR);
        return AIC_HTTPS_TERM_ERROR;
    }

    /* Clear out the HTTPS client. */
    status = aic_https_client_delete(https_paras.handle);
    if (status)
        transmit_https_at_cme(at_ptr, AIC_HTTPS_TERM_ERROR);
    else
        transmit_https_at_ok(at_ptr);

    https_at_crt_reset();
    https_paras_reset();
    return AIC_SUCCESS;
}

u32 aic_handle_https_term_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_https_str(at_ptr, AIC_NULL, "+HTTPSTERM");
    transmit_https_at_ok(at_ptr);

    return AIC_SUCCESS;
}

int aic_https_at_service_init(void)
{
    https_paras_reset();
    aic_add_at_cmd_handle(&https_at_handles[0]);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_https_at_service_init);

