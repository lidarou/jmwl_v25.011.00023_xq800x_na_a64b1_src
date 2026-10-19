/**************************************************************************/
/*                                                                        */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Network Component
 *  Code that can be reused for HTTP and HTTPS
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0cecca
#define LOG_MODULE    AMOD_NET

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <nx_api.h>
#include <nx_ip.h>
#include <nx_ipv6.h>
#include <nxd_bsd.h>

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_pdp_mgr.h>
#include <aic_http_comm.h>
#include <aic_https_client.h>
#include <aic_http_client.h>
#include <nx_packet.h>

#ifdef AIC_HTTP_REDIR
aos_queue_ptr *http_redir_q;
static aos_thread_id http_redir_t;

static void http_redir_entry(void *thread_input)
{
    HTTPS_CLIENT_QUEUE_MSG http_redir_msg;
    u32 ret, handle;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    u32 status = AIC_SUCCESS;
    AIC_HTTPS_REDIR_INFO *redir_info = AIC_NULL;

    while (1) {
        ret = aos_recv_from_queue(http_redir_q, &http_redir_msg,
                                  AOS_WAIT_FOREVER);

        handle = (u32)http_redir_msg.client_ptr;
        redir_info = &http_redir_msg.client_ptr->https_client->redir_info;
        if (!redir_info)
            return;

        client_ptr = http_redir_msg.client_ptr;

        if (client_ptr->https_client->redir_info.no_http_host) {
            snprintf(client_ptr->https_client->redir_info.url,
                     HTTPS_MAX_URL_LEN,"%s%s", client_ptr->prev_host,
                     client_ptr->https_client->redir_info.resource);
        }

        switch (http_redir_msg.msg) {
        case HTTPS_CLIENT_MSG_GET_REDIR:
#ifdef CONFIG_NET_HTTPS
            if (client_ptr->https_client->nx_web_http_client_is_https)
                status = aic_https_client_get_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->wait_option);
            else
#endif
                status = aic_http_client_get_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->wait_option);
            break;
        case HTTPS_CLIENT_MSG_RANGE_GET_REDIR:
#ifdef CONFIG_NET_HTTPS
            if (client_ptr->https_client->nx_web_http_client_is_https)
                status = aic_https_client_range_get_start(handle, redir_info->url,
                          redir_info->username, redir_info->password,
                          redir_info->server_port, redir_info->start,
                          redir_info->end, redir_info->wait_option);
            else
#endif
                status = aic_http_client_range_get_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->start,
                         redir_info->end, redir_info->wait_option);
            break;
        case HTTPS_CLIENT_MSG_POST_REDIR:
#ifdef CONFIG_NET_HTTPS
            if (client_ptr->https_client->nx_web_http_client_is_https)
                status = aic_https_client_post_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->total_size,
                         redir_info->wait_option);
            else
#endif
                status = aic_http_client_post_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->total_size,
                         redir_info->wait_option);
            break;
        case HTTPS_CLIENT_MSG_POST_EXT_REDIR:
#ifdef CONFIG_NET_HTTPS
            if (client_ptr->https_client->nx_web_http_client_is_https)
                status = aic_https_client_post_start_ext(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->total_size,
                         redir_info->content_type, redir_info->wait_option);
            else
#endif
                status = aic_http_client_post_start_ext(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->total_size,
                         redir_info->content_type, redir_info->wait_option);
            break;
        case HTTPS_CLIENT_MSG_PUT_REDIR:
#ifdef CONFIG_NET_HTTPS
            if (client_ptr->https_client->nx_web_http_client_is_https)
                status = aic_https_client_put_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->total_size,
                         redir_info->wait_option);
            else
#endif
                status = aic_http_client_put_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->total_size,
                         redir_info->wait_option);
            break;
        case HTTPS_CLIENT_MSG_DELETE_REDIR:
#ifdef CONFIG_NET_HTTPS
            if (client_ptr->https_client->nx_web_http_client_is_https)
                status = aic_https_client_delete_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->wait_option);
            else
#endif
                status = aic_http_client_delete_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->wait_option);
            break;
        case HTTPS_CLIENT_MSG_HEAD_REDIR:
#ifdef CONFIG_NET_HTTPS
            if (client_ptr->https_client->nx_web_http_client_is_https)
                status = aic_https_client_head_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->wait_option);
            else
#endif
                status = aic_http_client_head_start(handle, redir_info->url,
                         redir_info->username, redir_info->password,
                         redir_info->server_port, redir_info->wait_option);
            break;
        default:
            break;
        }

        client_ptr->https_client->redir_flag = 0;
        client_ptr->https_client->redir_reconnect = 0;
        if (status)
            aicx_error(0x178a9b, "x", "http redir fail(0x%x)\n", status);
    }
}
#endif

/* Define status maps. */
static AIC_HTTPS_STATUS_MAP aic_https_status_maps[] =
{
    {"200",    NX_SUCCESS},
    {"100",    NX_WEB_HTTP_STATUS_CODE_CONTINUE},
    {"101",    NX_WEB_HTTP_STATUS_CODE_SWITCHING_PROTOCOLS},
    {"201",    NX_WEB_HTTP_STATUS_CODE_CREATED},
    {"202",    NX_WEB_HTTP_STATUS_CODE_ACCEPTED},
    {"203",    NX_WEB_HTTP_STATUS_CODE_NON_AUTH_INFO},
    {"204",    NX_WEB_HTTP_STATUS_CODE_NO_CONTENT},
    {"205",    NX_WEB_HTTP_STATUS_CODE_RESET_CONTENT},
    {"206",    NX_WEB_HTTP_STATUS_CODE_PARTIAL_CONTENT},
    {"300",    NX_WEB_HTTP_STATUS_CODE_MULTIPLE_CHOICES},
    {"301",    NX_WEB_HTTP_STATUS_CODE_MOVED_PERMANETLY},
    {"302",    NX_WEB_HTTP_STATUS_CODE_FOUND},
    {"303",    NX_WEB_HTTP_STATUS_CODE_SEE_OTHER},
    {"304",    NX_WEB_HTTP_STATUS_CODE_NOT_MODIFIED},
    {"305",    NX_WEB_HTTP_STATUS_CODE_USE_PROXY},
    {"307",    NX_WEB_HTTP_STATUS_CODE_TEMPORARY_REDIRECT},
    {"400",    NX_WEB_HTTP_STATUS_CODE_BAD_REQUEST},
    {"401",    NX_WEB_HTTP_STATUS_CODE_UNAUTHORIZED},
    {"402",    NX_WEB_HTTP_STATUS_CODE_PAYMENT_REQUIRED},
    {"403",    NX_WEB_HTTP_STATUS_CODE_FORBIDDEN},
    {"404",    NX_WEB_HTTP_STATUS_CODE_NOT_FOUND},
    {"405",    NX_WEB_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED},
    {"406",    NX_WEB_HTTP_STATUS_CODE_NOT_ACCEPTABLE},
    {"407",    NX_WEB_HTTP_STATUS_CODE_PROXY_AUTH_REQUIRED},
    {"408",    NX_WEB_HTTP_STATUS_CODE_REQUEST_TIMEOUT},
    {"409",    NX_WEB_HTTP_STATUS_CODE_CONFLICT},
    {"410",    NX_WEB_HTTP_STATUS_CODE_GONE},
    {"411",    NX_WEB_HTTP_STATUS_CODE_LENGTH_REQUIRED},
    {"412",    NX_WEB_HTTP_STATUS_CODE_PRECONDITION_FAILED},
    {"413",    NX_WEB_HTTP_STATUS_CODE_ENTITY_TOO_LARGE},
    {"414",    NX_WEB_HTTP_STATUS_CODE_URL_TOO_LARGE},
    {"415",    NX_WEB_HTTP_STATUS_CODE_UNSUPPORTED_MEDIA},
    {"416",    NX_WEB_HTTP_STATUS_CODE_RANGE_NOT_SATISFY},
    {"417",    NX_WEB_HTTP_STATUS_CODE_EXPECTATION_FAILED},
    {"500",    NX_WEB_HTTP_STATUS_CODE_INTERNAL_ERROR},
    {"501",    NX_WEB_HTTP_STATUS_CODE_NOT_IMPLEMENTED},
    {"502",    NX_WEB_HTTP_STATUS_CODE_BAD_GATEWAY},
    {"503",    NX_WEB_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE},
    {"504",    NX_WEB_HTTP_STATUS_CODE_GATEWAY_TIMEOUT},
    {"505",    NX_WEB_HTTP_STATUS_CODE_VERSION_ERROR},
};

static u32 aic_https_status_maps_size = sizeof(aic_https_status_maps) /
                                        sizeof(AIC_HTTPS_STATUS_MAP);

static u32 aic_http_process_header(NX_WEB_HTTP_CLIENT *client_ptr,
                                   NX_PACKET *packet_ptr)
{
u32     offset;
CHAR    *buf_ptr;
CHAR    *field_name;
u32     field_name_len = 0;
CHAR    *field_value;
u32     field_value_len = 0;
u32     headr_len = 0;
CHAR    *header_data;
ulong   bytes_recv = 0;

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
u32    version = 0;
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

    /* Default the content offset to zero.  */
    offset = 0;
    client_ptr->nx_web_http_client_response_chunked = NX_FALSE;

    header_data = malloc(packet_ptr->nx_packet_length + 4);
    memset(header_data, '\0', packet_ptr->nx_packet_length + 4);

    nx_packet_data_extract_offset(packet_ptr, 0, header_data,
                                  packet_ptr->nx_packet_length, &bytes_recv);

    /* Setup pointer to buffer.  */
    buf_ptr = header_data;

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
    version = (u8)((buf_ptr[5] - '0') << 4) | (u8)(buf_ptr[7] - '0');

    /* Is a valid HTTP version found?  */
    if(version == 0)
    {
        client_ptr->nx_web_http_client_keep_alive = NX_FALSE;
        free(header_data);
        return 0;
    }

    /* Initialize the keepalive flag.  */
    if(version > 0x10)
        client_ptr->nx_web_http_client_keep_alive = NX_TRUE;
    else
        client_ptr->nx_web_http_client_keep_alive = NX_FALSE;
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

    /* Skip over the first HTTP line (e.g. HTTP/1.1 200 OK).  */
    while((offset <= bytes_recv) && (*buf_ptr != (CHAR) 13) &&
          (*(buf_ptr + 1) !=  (CHAR) 10)) {
        buf_ptr++;
        offset++;
    }

    /* Skip over the CR,LF. */
    buf_ptr += 2;
    offset += 2;

    /* Loop until we find the "cr,lf,cr,lf" token.  */
    while (offset <= bytes_recv && (*buf_ptr != (CHAR) 0)) {
        /* Check for the <cr,lf,cr,lf> token.  This signals a blank line, which also
           specifies the start of the content.  */
        if ((*buf_ptr == (CHAR) 13) && (*(buf_ptr+1) == (CHAR) 10)) {
            /* Adjust the offset.  */
            offset =  offset + 2;
            break;
        }

        /* We haven't seen the <cr,lf,cr,lf> so we are still processing header data.
         * Extract the field name and it's value.
         */
        field_name = buf_ptr;
        field_name_len = 0;

        /* Look for the ':' that separates the field name from its value. */
        while(*buf_ptr != ':') {
            buf_ptr++;
            field_name_len++;
        }
        offset += field_name_len;

        /* Skip ':'.  */
        buf_ptr++;
        offset++;

        /* Now skip over white space. */
        while ((offset <= bytes_recv) && (*buf_ptr == ' ')) {
            buf_ptr++;
            offset++;
        }

        /* Now get the field value. */
        field_value = buf_ptr;
        field_value_len = 0;

        /* Loop until we see a <CR, LF>. */
        while((offset <= bytes_recv) &&
              (*buf_ptr != (CHAR) 13) && (*(buf_ptr+1) !=  (CHAR) 10)) {
            buf_ptr++;
            field_value_len++;
        }
        offset += field_value_len;

        /* Skip over the CR,LF. */
        buf_ptr += 2;
        offset += 2;

        /* Check if the response packet is chunked.  */
        if (nx_web_http_client_memicmp((u8 *)field_name, field_name_len,
            (u8 *)"Transfer-Encoding", 17) == 0) {
            if (nx_web_http_client_memicmp((u8 *)field_value,
                field_value_len, (u8 *)"chunked", 7) == 0) {
                client_ptr->nx_web_http_client_response_chunked = NX_TRUE;
            }
        }

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
        /* If the "connection" field exist, and the value is "keep-alive",
           set the keep-alive flag to TRUE. */
        if (nx_web_http_client_memicmp((u8 *)"connection", 10,
            (u8 *)field_name, field_name_len) == NX_SUCCESS) {
            if (nx_web_http_client_memicmp((u8 *)"keep-alive", 10,
                (u8 *)field_value, field_value_len) == NX_SUCCESS) {
                client_ptr->nx_web_http_client_keep_alive = NX_TRUE;
            } else if (nx_web_http_client_memicmp((u8 *)"close", 5,
                       (u8 *)field_value, field_value_len) == NX_SUCCESS) {
                client_ptr->nx_web_http_client_keep_alive = NX_FALSE;
            }
        }
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

        /* Invoke the header callback if present. */
        /* If it was set, invoke the application response callback. */
        if(client_ptr->nx_web_http_client_response_callback) {
            client_ptr->nx_web_http_client_response_callback(client_ptr,
                field_name, field_name_len, field_value, field_value_len);
        } else if (client_ptr->nx_web_http_client_response_callback_ext) {
            client_ptr->nx_web_http_client_response_callback_ext(field_name,
                field_name_len, field_value, field_value_len);
        }
        /* callback to find redir flag. */
        if ((client_ptr->nx_web_http_client_redir_callback)) {
            client_ptr->nx_web_http_client_redir_callback(client_ptr,
                field_name, field_name_len, field_value, field_value_len);
        }
    }

    /* callback to get http header data. */
    if ((client_ptr->nx_web_http_client_header_callback)) {
        //copy header data to user by header_callback
        memset(header_data, '\0', packet_ptr->nx_packet_length + 4);
        nx_packet_data_extract_offset(packet_ptr, 0, header_data,
                                      offset, &bytes_recv);
        headr_len = offset;
        client_ptr->nx_web_http_client_header_callback(client_ptr,
                                                       header_data, headr_len);
    }

    /* Not find the "cr,lf,cr,lf" token.  */
    if (offset > bytes_recv) {
        offset = 0;
    }

    free(header_data);
    /* Return the offset to the caller.  */
    return(offset);
}

#ifdef CONFIG_NET_HTTPS
static void my_debug(void *ctx, int level, const char *file, int line,
                     const char *str)
{
    ((void) level);

    //alog_info("%s:%04d: %s\n", file, line, str);
    //alog_info("%04d: %s\n", line, str);
    return;
}


static void https_mbed_ssl_reset(NX_WEB_HTTP_CLIENT *client_ptr)
{
    if (!client_ptr->http_mb_cfg)
        return;

    mbedtls_ssl_free(&client_ptr->http_mb_cfg->ssl);
    mbedtls_ssl_config_free(&client_ptr->http_mb_cfg->conf);
    mbedtls_ctr_drbg_free(&client_ptr->http_mb_cfg->ctr_drbg);
    mbedtls_entropy_free(&client_ptr->http_mb_cfg->entropy);

    return;
}


/**
 * TLS set callback, called by nx_web_http_client_get_secure_start.
 */
u32 https_tls_setup(NX_WEB_HTTP_CLIENT *client_ptr)
{
#ifdef CONFIG_CRYPTO_MBEDTLS
    int ret;
    const char *pers = "https";
    u8 count = 0;
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    if (!client_ptr->http_mb_cfg)
        return AIC_NOT_HTTPS_CLIENT;

    if (client_ptr->http_mb_cfg->status == AIC_HTTPS_MBED_SET_SUCCESS)
        goto HAND_SHAKE;

    client_ptr->http_mb_cfg->recv_wait_time = HTTPS_DEFAULT_TIMEOUT;
    mbedtls_net_init(&client_ptr->http_mb_cfg->socket_fd);
    mbedtls_ssl_set_socket(&client_ptr->http_mb_cfg->socket_fd,
                   (int)(&client_ptr->nx_web_http_client_socket));

    /* Initialize the RNG and the session data */
    mbedtls_ssl_init(&client_ptr->http_mb_cfg->ssl);
    mbedtls_ssl_config_init(&client_ptr->http_mb_cfg->conf);
    mbedtls_ctr_drbg_init(&client_ptr->http_mb_cfg->ctr_drbg);

    mbedtls_entropy_init(&client_ptr->http_mb_cfg->entropy);
    mbedtls_ctr_drbg_seed(&client_ptr->http_mb_cfg->ctr_drbg,
                          mbedtls_entropy_func,
                          &client_ptr->http_mb_cfg->entropy,
                          (const unsigned char *) pers,
                          strlen(pers));

    ret = mbedtls_ssl_config_defaults(&client_ptr->http_mb_cfg->conf,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret) {
        aicx_error(0x01c8fe, "x", "tls config fail(0x%x)\n", ret);
        return ret;
    }

    // mbedtls_ssl_conf_authmode(&client_ptr->mb_cfg.conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    if (client_ptr->redir_flag == 1)
        mbedtls_ssl_conf_authmode(&client_ptr->http_mb_cfg->conf, 0);
    else
        mbedtls_ssl_conf_authmode(&client_ptr->http_mb_cfg->conf, client_ptr->auth_type);

    mbedtls_ssl_conf_rng(&client_ptr->http_mb_cfg->conf, mbedtls_ctr_drbg_random,
                         &client_ptr->http_mb_cfg->ctr_drbg);
    aic_ssl_set_recv_waitoption(client_ptr->http_mb_cfg->socket_fd.fd,
                                client_ptr->http_mb_cfg->recv_wait_time);
    mbedtls_ssl_conf_dbg( &client_ptr->http_mb_cfg->conf, my_debug, NULL );


    if (client_ptr->ca_crts)
        mbedtls_ssl_conf_ca_chain(&client_ptr->http_mb_cfg->conf,
                                  client_ptr->ca_crts, NULL);

    if (client_ptr->client_crts && client_ptr->private_key) {
        mbedtls_ssl_conf_own_cert(&client_ptr->http_mb_cfg->conf,
                                  client_ptr->client_crts,
                                  client_ptr->private_key);
    }

    ret = mbedtls_ssl_setup(&client_ptr->http_mb_cfg->ssl, &client_ptr->http_mb_cfg->conf);
    if (ret) {
        return AIC_SSL_SETUP_FAIL;
    }
    client_ptr->http_mb_cfg->status = AIC_HTTPS_MBED_SET_SUCCESS;

    mbedtls_ssl_set_bio(&client_ptr->http_mb_cfg->ssl,
                        &client_ptr->http_mb_cfg->socket_fd,
                        mbedtls_net_send_ext, mbedtls_net_recv_ext,
                        mbedtls_net_recv_timeout_ext);

HAND_SHAKE:
    do {
        ret = mbedtls_ssl_handshake(&client_ptr->http_mb_cfg->ssl);
        aos_sleep(500);
        count++;
    } while((ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE) &&
            count < HTTPS_MAX_HAND_SHAKE_TIME);

    if (ret) {
        aicx_error(0x19f4fd, "d", "https handshake fail(0x%d)\n", ret);
        return AIC_SSL_HANDSHAKE_FAIL;
    }
#endif /* CONFIG_CRYPTO_MBEDTLS */

    return AIC_SUCCESS;
}
#endif

/**
 * Creates a HTTP client on the specified IP.
 * return http client handle for success;
 * return 0 for fail;
 */
u32 aic_http_client_common_create(u8 *client_name, boolean is_https)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    NX_IP *https_ip_ptr = AIC_NULL;
    struct aic_network_ip_instance *aic_ip_instance = AIC_NULL;
    NX_PACKET_POOL *https_packet_pool;

    if (!client_name)
        return AIC_HTTPS_CLIENT_CREATE_FAIL;

    aic_ip_instance = aic_get_default_ip_instance();
    if (!aic_ip_instance || !aic_ip_instance->ip_ptr ||
        !aic_ip_instance->packet_pool) {
        return AIC_HTTPS_CLIENT_CREATE_FAIL;
    }

    https_ip_ptr = aic_ip_instance->ip_ptr;
    https_packet_pool = aic_ip_instance->packet_pool;

    client_ptr = aos_zalloc(sizeof(AIC_HTTPS_CLIENT_INSTANCE));
    if (!client_ptr)
        return AIC_HTTPS_CLIENT_CREATE_FAIL;

    client_ptr->https_client = aos_zalloc(sizeof(NX_WEB_HTTP_CLIENT));
    if (!client_ptr->https_client)
        return AIC_HTTPS_CLIENT_CREATE_FAIL;

    status = nx_web_http_client_create(client_ptr->https_client, client_name,
                                       https_ip_ptr, https_packet_pool,
                                       HTTPS_SOCKET_WINDOW_SIZE);

    if (status) {
        aicx_error(0x0a2dbe, "x", "http create fail(0x%x)\n", status);
        aic_http_free_instance_space(client_ptr);
        client_ptr = AIC_NULL;
        return AIC_HTTPS_CLIENT_CREATE_FAIL;
    }

    client_ptr->login_info.host = aos_zalloc(HTTPS_MAX_HOST_LEN + 1);
    client_ptr->login_info.resource = aos_zalloc(HTTPS_MAX_RESOURCE_LEN + 1);
    client_ptr->prev_host = aos_zalloc(HTTPS_MAX_HOST_LEN +1);

#ifdef CONFIG_NET_HTTPS
    if (is_https) {
        client_ptr->https_client->mbedtls_ssl_send_cb = mbedtls_ssl_send;
        client_ptr->https_client->mbedtls_ssl_recv_cb = mbedtls_ssl_recv;
        client_ptr->https_client->mbedtls_ssl_close_notify_cb = mbedtls_ssl_close_notify;
        client_ptr->https_client->mbedtls_ssl_free_cb = https_mbed_ssl_reset;

        /* Only https need to apply space for mbed tls. */
        client_ptr->https_client->http_mb_cfg = aos_zalloc(sizeof(HTTPS_MBEDTLS_ENV));
        client_ptr->https_client->ca_crts = aos_zalloc(sizeof(mbedtls_x509_crt));
        client_ptr->https_client->client_crts = aos_zalloc(sizeof(mbedtls_x509_crt));
        client_ptr->https_client->private_key = aos_zalloc(sizeof(mbedtls_pk_context));
        client_ptr->https_client->nx_web_http_client_is_https = NX_TRUE;
        client_ptr->login_info.server_port = NX_WEB_HTTPS_SERVER_PORT;
    }
#endif

#ifdef AIC_HTTP_REDIR
    client_ptr->https_client->nx_web_http_client_redir_callback = aic_http_redir_cb;
#endif
    return (u32)client_ptr;
}

/* Deletes a previously created HTTP(S) client on the specified IP */
u32 aic_https_client_common_delete(u32 handle, boolean is_https)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    if (is_https && client_ptr->https_client->http_mb_cfg) {
#ifdef CONFIG_NET_HTTPS
        mbedtls_ssl_reset_socket(&client_ptr->https_client->http_mb_cfg->socket_fd);
        mbedtls_net_free(&client_ptr->https_client->http_mb_cfg->socket_fd);
#endif
    }

    aic_http_packet_release(&client_ptr->res_data_packet);
    aic_http_packet_release(&client_ptr->put_packet);
    aic_http_packet_release(&client_ptr->chunked_data_packet);

    nx_web_http_client_delete(client_ptr->https_client);

#ifdef CONFIG_NET_HTTPS
    if (client_ptr->https_client->nx_web_http_client_is_https)
    {
        https_mbed_ssl_reset(client_ptr->https_client);
        if (client_ptr->https_client->http_mb_cfg)
            aos_free(client_ptr->https_client->http_mb_cfg);

        if (client_ptr->https_client->ca_crts)
            aos_free(client_ptr->https_client->ca_crts);

        if (client_ptr->https_client->client_crts)
            aos_free(client_ptr->https_client->client_crts);

        if (client_ptr->https_client->private_key)
            aos_free(client_ptr->https_client->private_key);
    }
#endif

    /* Clear out the entire structure in case it is used again. */
    memset(client_ptr->https_client, 0, sizeof(NX_WEB_HTTP_CLIENT));

    aic_http_free_instance_space(client_ptr);
    client_ptr = AIC_NULL;

    return AIC_SUCCESS;
}

/* This function processes the application GET request. */
u32 aic_http_get_start(u32 handle, u8 *url, u8 *username, u8 *password,
                       u32 server_port, ulong wait_option, boolean is_https)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    status = aic_http_action_prepare(handle, url, username, password,
                                     server_port, 0, -1, -1, AIC_NULL,
                                     wait_option);
    if (status)
        return status;

#ifdef CONFIG_NET_HTTPS
    if (is_https)
        status = nx_web_http_client_get_secure_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username,password, https_tls_setup,
                 wait_option);
    else
#endif
        status = nx_web_http_client_get_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username,password, wait_option);

    return status;
}

/**
 * This function processes the application GET request.
 * Can specify the range.
 */
u32 aic_http_range_get_start(u32 handle, u8 *url, u8 *username,
                             u8 *password, u32 server_port, slong start,
                             slong end, ulong wait_option, boolean is_https)
{
    u32 status = 0;
    u8 value_str[HTTPS_MAX_RANGE_LEN] = {'\0'};
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    status = aic_http_action_prepare(handle, url, username, password,
                                     server_port,0, start, end, AIC_NULL,
                                     wait_option);
    if (status)
        return status;

    client_ptr->https_client->nx_web_http_client_connect_port = server_port;
#ifdef CONFIG_NET_HTTPS
    if (is_https)
        status = nx_web_http_client_secure_connect(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, https_tls_setup, wait_option);
    else
#endif
        status = nx_web_http_client_connect(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, wait_option);

    if (status)
        return status;

    /* Initialize HTTP request. */
    status = nx_web_http_client_request_initialize(client_ptr->https_client,
             NX_WEB_HTTP_METHOD_GET, client_ptr->login_info.resource,
             client_ptr->login_info.host, 0, NX_FALSE, username, password,
             wait_option);
    if (status)
        return status;

    /* Add approproate headers for HTTP RANGE GET. */
    if (start < 0 && end <0)
        aos_memset(value_str, 0, HTTPS_MAX_RANGE_LEN);
    else if (start >= 0 && start <= end)
        snprintf(value_str, HTTPS_MAX_RANGE_LEN, "bytes=%ld-%ld", start, end);
    else if (start >= 0 && start > end)
        snprintf(value_str, HTTPS_MAX_RANGE_LEN, "bytes=%ld-", start);
    else if (start < 0 && end >= 0)
        snprintf(value_str, HTTPS_MAX_RANGE_LEN, "bytes=-%ld", end);

    if(*value_str != 0) {
        nx_web_http_client_request_header_add(client_ptr->https_client, "Range",
                                              5, value_str, strlen(value_str),
                                              wait_option);
    }

    status = nx_web_http_client_request_send(client_ptr->https_client,
                                             wait_option);
    if (status)
        return status;

    client_ptr->https_client->nx_web_http_client_state =
    NX_WEB_HTTP_CLIENT_STATE_GET;

    return status;
}

/* This function processes an application POST request. */
u32 aic_http_post_start(u32 handle, u8 *url, u8* username, u8 *password,
                        u32 server_port, ulong total_bytes,
                        ulong wait_option, boolean is_https)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    status = aic_http_action_prepare(handle, url, username, password,
                                     server_port, total_bytes, -1, -1, AIC_NULL,
                                     wait_option);
    if (status)
        return status;

#ifdef CONFIG_NET_HTTPS
    if (is_https)
        status = nx_web_http_client_post_secure_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username,password, total_bytes,
                 https_tls_setup, wait_option);
    else
#endif
        status = nx_web_http_client_post_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username,password, total_bytes,
                 wait_option);

    return status;
}

/**
 * This function processes an application POST request.
 * Can specify Content Type.
 */
u32 aic_http_post_start_ext(u32 handle, u8 *url, u8 *username,
                                    u8 *password, u32 server_port,
                                    ulong total_bytes, u8 *content_type,
                                    ulong wait_option, boolean is_https)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    status = aic_http_action_prepare(handle, url, username, password,
                                     server_port, total_bytes, -1, -1,
                                     content_type, wait_option);
    if (status)
        return status;

    client_ptr->https_client->nx_web_http_client_connect_port = server_port;

#ifdef CONFIG_NET_HTTPS
    if (is_https)
        status = nx_web_http_client_secure_connect(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, https_tls_setup, wait_option);
    else
#endif
        status = nx_web_http_client_connect(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, wait_option);

    if (status)
        return status;

    /* Initialize HTTP request. */
    status = nx_web_http_client_request_initialize(client_ptr->https_client,
             NX_WEB_HTTP_METHOD_POST, client_ptr->login_info.resource,
             client_ptr->login_info.host, total_bytes, NX_FALSE, username,
             password, wait_option);

    if (status)
        return status;

    /* Add approproate headers from input data. */
    aic_https_client_content_type_header_add(client_ptr->https_client,
                                             client_ptr->login_info.resource,
                                             content_type, wait_option);

    status = nx_web_http_client_request_send(client_ptr->https_client,
                                             wait_option);
    if (status)
        return status;

    client_ptr->https_client->nx_web_http_client_state =
    NX_WEB_HTTP_CLIENT_STATE_POST;

    return status;
}

/* This function processes an application PUT request. */
u32 aic_http_put_start(u32 handle, u8 *url, u8 *username, u8 *password,
                               u32 server_port, ulong total_bytes,
                               ulong wait_option, boolean is_https)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    status = aic_http_action_prepare(handle, url, username, password,
                                     server_port, total_bytes, -1, -1, AIC_NULL,
                                     wait_option);
    if (status)
        return status;

#ifdef CONFIG_NET_HTTPS
    if (is_https)
        status = nx_web_http_client_put_secure_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username,password, total_bytes,
                 https_tls_setup, wait_option);
    else
#endif
        status = nx_web_http_client_put_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username,password, total_bytes,
                 wait_option);

    return status;
}

/* This function processes an application HEAD request. */
u32 aic_http_head_start(u32 handle, u8 *url, u8 *username, u8 *password,
                        u32 server_port, ulong wait_option, boolean is_https)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    status = aic_http_action_prepare(handle, url, username, password,
                                     server_port, 0, -1, -1, AIC_NULL,
                                     wait_option);
    if (status)
        return status;

#ifdef CONFIG_NET_HTTPS
    if (is_https)
        status = nx_web_http_client_head_secure_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username,password, https_tls_setup,
                 wait_option);
    else
#endif
        status = nx_web_http_client_head_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username,password,
                 wait_option);

    return status;
}

/* This function processes an application DELETE request. */
u32 aic_http_delete_start(u32 handle, u8 *url, u8 *username,
                          u8 *password, u32 server_port, ulong wait_option,
                          boolean is_https)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    status = aic_http_action_prepare(handle, url, username, password,
                                     server_port, 0, -1, -1, AIC_NULL,
                                     wait_option);
    if (status)
        return status;

#ifdef CONFIG_NET_HTTPS
    if (is_https)
        status = nx_web_http_client_delete_secure_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username, password, https_tls_setup,
                 wait_option);
    else
#endif
        status = nx_web_http_client_delete_start(client_ptr->https_client,
                 &(client_ptr->login_info.server_ip),
                 client_ptr->login_info.server_port, client_ptr->login_info.resource,
                 client_ptr->login_info.host, username, password,
                 wait_option);

    return status;
}

/**
 * This function adds a custom header to an HTTP(S) request that was
 * previously initialized with nx_web_http_client_request_initialize.
 * The field name and value are placed directly into the HTTP header
 * so it is up to the caller to assure that they are valid values.
 */
u32 aic_https_client_content_type_header_add(NX_WEB_HTTP_CLIENT *client_ptr,
                                             u8 *resource, u8 *content_type,
                                             ulong wait_option)
{
    u32 i, j;
    u8 *string1 = NULL;

    i = strlen(content_type);
    if (i > HTTPS_CONTENT_TYPE_LEN)
        return NX_SIZE_ERROR;

    string1 = aos_zalloc(HTTPS_CONTENT_TYPE_LEN + 1);
    if (i) {
        strncpy(string1, content_type, i);
        j = i;
    } else {
        j = nx_web_http_client_type_get(resource, string1);
    }

    nx_web_http_client_request_header_add(client_ptr, "Content-Type", 12,
                                          string1, j, wait_option);

    aos_free(string1);
    return AIC_SUCCESS;
}

#ifdef AIC_HTTP_REDIR
void aic_https_store_redir_info(u32 handle, u8 *username, u8 *password,
                                u32 port, u32 total_size, slong start,
                                slong end, u8 *content_type, ulong wait_option)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;

    client_ptr->https_client->redir_info.wait_option = wait_option;
    client_ptr->https_client->redir_info.start = start;
    client_ptr->https_client->redir_info.end = end;
    client_ptr->https_client->redir_info.server_port = port;

    if (username)
        aos_memcpy(client_ptr->https_client->redir_info.username, username,
                   strlen(username));

    if (password)
        aos_memcpy(client_ptr->https_client->redir_info.password, password,
                   strlen(password));

    if (total_size)
        client_ptr->https_client->redir_info.total_size = total_size;

    if (content_type) {
        if (!client_ptr->https_client->redir_info.content_type) {
            client_ptr->https_client->redir_info.content_type =
                aos_zalloc(HTTPS_CONTENT_TYPE_LEN + 1);
            if (!client_ptr->https_client->redir_info.content_type)
                return;
        } else {
            aos_memset(client_ptr->https_client->redir_info.content_type, '\0',
                       HTTPS_CONTENT_TYPE_LEN);
        }

        aos_memcpy(client_ptr->https_client->redir_info.content_type,
                   content_type, strlen(content_type));
    }

    return;
}
#endif

static void aic_http_server_port_find(u8 *host, AIC_HTTPS_CLIENT_INSTANCE *client_ptr)
{
    u8 *found = AIC_NULL;
    u8 *temp = AIC_NULL;
    u8 host_len = 0;
    u8 *check_v6 = AIC_NULL;

    temp = aos_zalloc(HTTPS_MAX_HOST_LEN);
    if (!temp)
        return;

    aos_memcpy(temp, host, HTTPS_MAX_HOST_LEN);

    found = strchr(host, ':');
    if (found) {
        host_len = strlen(found);
        found++;
        check_v6 = strchr(found, ':');
        if (check_v6)
            goto OUT;

        client_ptr->login_info.server_port = atoi(found);
        aos_memset(client_ptr->login_info.host, 0, HTTPS_MAX_HOST_LEN);
        aos_memcpy(client_ptr->login_info.host, temp, strlen(temp) - host_len);
    }

OUT:
    aos_free(temp);
    return;
}

void http_server_ip_set(NXD_ADDRESS *server_ip, struct addrinfo *res)
{
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

    return;
}

/* Process URL to get host and resource*/
u32 aic_process_https_url(u32 handle, u8 *url, u8 type)
{
    u8 *https_url_head = AIC_NULL;
    u8 *temp = AIC_NULL;
    const u8 *flag = "/";
    char *next = AIC_NULL;
    u8 *buf = AIC_NULL;
    u8 *v6_str;
    struct addrinfo *res;
    u32 status;
    struct addrinfo hints;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;

    if (strlen(url) == 0)
        return AIC_HTTPS_URL_ERROR;

    buf = (u8 *)aos_zalloc(HTTPS_MAX_URL_LEN);
    if (buf == AIC_NULL)
        return AIC_HTTPS_BUFFER_ALLOCATE_ERROR;

    strncpy(buf, url, HTTPS_MAX_URL_LEN);
    aos_memset(client_ptr->login_info.host, 0, HTTPS_MAX_HOST_LEN);

    https_url_head = strtok_r(buf, flag, &next);
    if (https_url_head == AIC_NULL)
        goto ERROR_OUT;

    if (strlen(https_url_head) == strlen(url)) {
        /* no http header and redource such as 192.168.0.1 */
        strncpy(client_ptr->login_info.host, url, HTTPS_MAX_HOST_LEN);
        strncpy(client_ptr->login_info.resource, "/", 2);
    } else if (strncasecmp(https_url_head, "http", 4) != 0) {
        /*no http header such as 192.168.0.1/doc/test.txt */
        strncpy(client_ptr->login_info.host, https_url_head, HTTPS_MAX_HOST_LEN);
        temp = next;
        if (temp)
            snprintf(client_ptr->login_info.resource, strlen(temp) + 2, "/%s", temp);
        aic_http_server_port_find(client_ptr->login_info.host, client_ptr);
    } else if (type == AIC_HTTP_HEAD && strlen(https_url_head) == 5 &&
               strncasecmp(https_url_head, "http", 4) == 0) {
        /* such as http://192.168.0.1/doc/test.txt */
        temp = strtok_r(AIC_NULL, flag, &next);
        if (temp)
            strncpy(client_ptr->login_info.host, temp, HTTPS_MAX_HOST_LEN);
        if (next)
            snprintf(client_ptr->login_info.resource, strlen(next) + 2, "/%s", next);
        else
            strncpy(client_ptr->login_info.resource, "/", 2);

        aic_http_server_port_find(client_ptr->login_info.host, client_ptr);
    } else if (type == AIC_HTTPS_HEAD && strlen(https_url_head) == 6 &&
               strncasecmp(https_url_head, "https", 5) == 0) {
        /* such as https://192.168.0.1/doc/test.txt */
        temp = strtok_r(AIC_NULL, flag, &next);
        if (temp)
            strncpy(client_ptr->login_info.host, temp, HTTPS_MAX_HOST_LEN);
        if (next)
            snprintf(client_ptr->login_info.resource, strlen(next) + 2, "/%s", next);
        else
            strncpy(client_ptr->login_info.resource, "/", 2);
        aic_http_server_port_find(client_ptr->login_info.host, client_ptr);
    } else {
        goto ERROR_OUT;
    }
    aos_free(buf);

    aicx_error(0x14dad8, "s", "host is %s\n", client_ptr->login_info.host);
    aicx_error(0x0c3a7b, "s", "resource is %s\n", client_ptr->login_info.resource);

    aos_memset(&hints, 0, sizeof(struct addrinfo));
    if (aic_get_pdp_ip_type(client_ptr->cid) == ANET_DEFAULT_IP_TYPE_V4)
        hints.ai_family = AF_INET;
    else if (aic_get_pdp_ip_type(client_ptr->cid) == ANET_DEFAULT_IP_TYPE_V6)
        hints.ai_family = AF_INET6;
    else
        hints.ai_family = AF_UNSPEC;

    if (strncmp(client_ptr->login_info.host, "[", 1) == 0) {
        //alog_info("http_url_host is ipv6 addr\n");
        v6_str = (u8 *)aos_zalloc(HTTPS_MAX_HOST_LEN + 1);
        if (v6_str == AIC_NULL)
            return AIC_HTTPS_BUFFER_ALLOCATE_ERROR;

        strncpy(v6_str, client_ptr->login_info.host + 1, HTTPS_MAX_HOST_LEN);
        v6_str[strlen(v6_str) - 1] = '\0';
        //alog_info("v6_str is %s\n", v6_str);
        status = getaddrinfo(v6_str, NULL, &hints, &res);
        aos_free(v6_str);
        v6_str = AIC_NULL;
    } else {
        status = getaddrinfo(client_ptr->login_info.host, NULL, &hints, &res);
    }

    if (status == 0 && res) {
        http_server_ip_set(&client_ptr->login_info.server_ip, res);
        freeaddrinfo(res);
    } else {
        return status;
    }

    return AIC_SUCCESS;

ERROR_OUT:
    aos_free(buf);
    return AIC_HTTPS_URL_ERROR;
}

/**
 * This function is used to get the total number of bytes will be
 * sent specified by the previous POST start request.
 */
ulong aic_https_client_total_transfer_bytes_get(u32 handle)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;

    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    return client_ptr->https_client->nx_web_http_client_total_transfer_bytes;
}

/* This function is used to get the actual number of bytes sent by HTTPS POST */
ulong aic_https_client_actual_bytes_transferred_get(u32 handle)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;

    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    return client_ptr->https_client->nx_web_http_client_actual_bytes_transferred;
}

/**
 * This function is used to get the total number of bytes will be
 * received from HTTPS Server.
 * This value is equal to Content-Length in HTTPS response.
 */
ulong aic_https_client_total_receive_bytes_get(u32 handle)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;

    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    return client_ptr->https_client->nx_web_http_client_total_receive_bytes;
}

/**
 * This function is used to get the actual number of bytes received
 * from HTTPS Server.
 */
ulong aic_https_client_actual_bytes_received_get(u32 handle)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;

    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    return client_ptr->https_client->nx_web_http_client_actual_bytes_received;
}

/* Set cid used by HTTPS Client. */
u32 aic_https_client_set_cid(u32 handle, u8 cid)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;

    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    client_ptr->cid = cid;

    return AIC_SUCCESS;
}

/**
 * This function gets a data packet associated with the resource
 * specified by the previous GET start request.
 */
u32 aic_https_client_response_body_get(u32 handle, NX_PACKET **packet_pptr,
                                       u8 *status_str, ulong wait_option)
{
    u32 status = AIC_SUCCESS;
    u32 length = 0, offset = 0;
    u32 i = 0;
    u32 status_code = 0;
    NX_PACKET *response_packet_ptr = AIC_NULL;
    u8 *buf_ptr = AIC_NULL;
    NX_WEB_HTTP_CLIENT *client_ptr = AIC_NULL;
    AIC_HTTPS_CLIENT_INSTANCE *https_client_ptr = AIC_NULL;
    u8 *no_header_data = AIC_NULL;
    ulong bytes_recv = 0;
    u32 ret = AIC_SUCCESS;

    https_client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(https_client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    client_ptr = https_client_ptr->https_client;
    if ((client_ptr->nx_web_http_client_state < NX_WEB_HTTP_CLIENT_STATE_GET) ||
        (client_ptr->nx_web_http_client_state > NX_WEB_HTTP_CLIENT_STATE_DELETE))
        return AIC_HTTPS_ERROR;

    *packet_pptr = NX_NULL;
    if (!(client_ptr->nx_web_http_client_response_header_received)) {
        status = nx_web_http_client_get_server_response(client_ptr,
                                                        &response_packet_ptr,
                                                        wait_option);
        if (status == AIC_SUCCESS) {
            if (response_packet_ptr->nx_packet_append_ptr -
                response_packet_ptr->nx_packet_prepend_ptr < 12)
                status = AIC_HTTPS_ERROR;
        }

        if (status) {
            if (status != NX_NO_PACKET) {
                nx_web_http_client_error_exit(client_ptr, wait_option);
                client_ptr->nx_web_http_client_state =
                NX_WEB_HTTP_CLIENT_STATE_READY;
            }
            return status;
        }

        buf_ptr = (u8 *) response_packet_ptr->nx_packet_prepend_ptr;

        /* Determine which status code is reveived. */
        for (i = 0; i < aic_https_status_maps_size; i++) {
            if ((buf_ptr[9] ==
                aic_https_status_maps[i].https_status_str[0]) &&
                (buf_ptr[10] ==
                aic_https_status_maps[i].https_status_str[1]) &&
                (buf_ptr[11] ==
                aic_https_status_maps[i].https_status_str[2])) {
                status_code = aic_https_status_maps[i].https_status_code;
                if (status_str != AIC_NULL) {
                    strncpy(status_str,
                            aic_https_status_maps[i].https_status_str, 3);
                }
                break;
            }
        }

        if (i == aic_https_status_maps_size) {
            status = NX_WEB_HTTP_REQUEST_UNSUCCESSFUL_CODE;
        }
        /* Pickup the content length if it exists. */
        length = nx_web_http_client_content_length_get(client_ptr,
                                                       response_packet_ptr);
        /* Pickup the content offset and process other headers. */
        offset = aic_http_process_header(client_ptr, response_packet_ptr);

        if (status_str && status_str[0] == '3' && status_str[1] == '0') {
            aic_http_packet_release(&response_packet_ptr);
            return AIC_HTTPS_REDIR_STATE;
        }

        if (!offset || (response_packet_ptr->nx_packet_length < offset) ||
            ((response_packet_ptr->nx_packet_length > (offset + length)) &&
             (!client_ptr->nx_web_http_client_response_chunked))) {

            status = NX_WEB_HTTP_BAD_PACKET_LENGTH;
        } else {
            client_ptr->nx_web_http_client_response_header_received = NX_TRUE;

            if (client_ptr->nx_web_http_client_state !=
                NX_WEB_HTTP_CLIENT_STATE_HEAD) {
                client_ptr->nx_web_http_client_total_receive_bytes = length;
            } else {
                client_ptr->nx_web_http_client_total_receive_bytes = 0;
            }
            client_ptr->nx_web_http_client_actual_bytes_received = 0;

            if (response_packet_ptr->nx_packet_length > offset) {
                no_header_data = aos_zalloc(response_packet_ptr->nx_packet_length);
                ret = nx_packet_data_extract_offset(response_packet_ptr, offset, no_header_data,
                                                    response_packet_ptr->nx_packet_length,
                                                    &bytes_recv);
                if (ret)
                    goto ERROR_OUT;

                aic_http_packet_release(&response_packet_ptr);
                /* Allocate a packet to save http content data. */
#ifdef CONFIG_NET_SECURE
                if (client_ptr->nx_web_http_client_is_https) {
                    ret = nx_secure_tls_packet_allocate(
                          &(client_ptr->nx_web_http_client_tls_session),
                          client_ptr->nx_web_http_client_packet_pool_ptr,
                          &response_packet_ptr, 500);

                    if (ret)
                        goto ERROR_OUT;

                } else
#endif
                if (client_ptr->nx_web_http_client_server_address.nxd_ip_version ==
                    NX_IP_VERSION_V4) {
                    ret = nx_packet_allocate(client_ptr->nx_web_http_client_packet_pool_ptr,
                                             &response_packet_ptr, NX_IPv4_TCP_PACKET, 500);
                } else {
                    ret = nx_packet_allocate(client_ptr->nx_web_http_client_packet_pool_ptr,
                                             &response_packet_ptr, NX_IPv6_TCP_PACKET, 500);
                }

                if (ret)
                    goto ERROR_OUT;

                /* Rebuild the packet. */
                ret = nx_packet_data_append(response_packet_ptr, no_header_data,
                                            bytes_recv,
                                            client_ptr->nx_web_http_client_packet_pool_ptr,
                                            500);
                aos_free(no_header_data);

                if (ret) {
                    aic_http_packet_release(&response_packet_ptr);
                    return ret;
                }
            } else {
                /* Adjust the pointers to skip over the response header.  */
                response_packet_ptr ->nx_packet_prepend_ptr = response_packet_ptr -> nx_packet_prepend_ptr + offset;

                /* Reduce the length. */
                response_packet_ptr ->nx_packet_length = response_packet_ptr -> nx_packet_length - offset;
            }

            if (client_ptr->nx_web_http_client_response_chunked) {
                client_ptr->nx_web_http_client_response_packet =
                response_packet_ptr;
                client_ptr->nx_web_http_client_chunked_response_remaining_size =
                response_packet_ptr->nx_packet_length;
            }
        }
    } else if (!client_ptr->nx_web_http_client_response_chunked) {
        status = nx_web_http_client_receive(client_ptr, &response_packet_ptr,
                                            wait_option);
        if (status) {
            if (status != NX_NO_PACKET) {
                client_ptr->nx_web_http_client_response_header_received =
                                                              NX_FALSE;
                nx_web_http_client_error_exit(client_ptr, wait_option);
                client_ptr->nx_web_http_client_state =
                NX_WEB_HTTP_CLIENT_STATE_READY;
            }
            return status;
        }

        if (response_packet_ptr->nx_packet_length >
            (client_ptr->nx_web_http_client_total_receive_bytes -
            client_ptr->nx_web_http_client_actual_bytes_received)) {
            status = NX_WEB_HTTP_BAD_PACKET_LENGTH;
        }
    }

    if (status == AIC_SUCCESS) {
        if (client_ptr->nx_web_http_client_response_chunked) {
            status = nx_web_http_client_response_chunked_get(client_ptr,
                     &response_packet_ptr, wait_option);

            if (status == NX_NO_PACKET)
                return status;

            *packet_pptr = response_packet_ptr;
        } else {
            client_ptr->nx_web_http_client_actual_bytes_received =
            client_ptr->nx_web_http_client_actual_bytes_received +
            response_packet_ptr->nx_packet_length;

            if (client_ptr->nx_web_http_client_total_receive_bytes ==
                client_ptr->nx_web_http_client_actual_bytes_received) {
                status = NX_WEB_HTTP_GET_DONE;
            }
            *packet_pptr = response_packet_ptr;
        }
    }

    if (status) {
        if ((status != NX_WEB_HTTP_GET_DONE) && response_packet_ptr) {
            aic_http_packet_release(&response_packet_ptr);
    }

        if (client_ptr->nx_web_http_client_response_chunked) {
            if (status != NX_WEB_HTTP_GET_DONE &&
                client_ptr->nx_web_http_client_response_packet) {
                aic_http_packet_release(
                &client_ptr->nx_web_http_client_response_packet);
            }
            client_ptr->nx_web_http_client_response_packet = NX_NULL;
            client_ptr->nx_web_http_client_chunked_response_remaining_size = 0;
            client_ptr->nx_web_http_client_response_chunked = NX_FALSE;
        }
        client_ptr->nx_web_http_client_response_header_received = NX_FALSE;

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
        if ((status != NX_WEB_HTTP_GET_DONE) ||
            !(client_ptr->nx_web_http_client_keep_alive))
#endif
        {
            nx_web_http_client_error_exit(client_ptr, wait_option);
        }

        client_ptr->nx_web_http_client_state = NX_WEB_HTTP_CLIENT_STATE_READY;
    }
    return status;

ERROR_OUT:
    aic_http_packet_release(&response_packet_ptr);
    aos_free(no_header_data);
    return ret;
}

/**
 * Read specified length of data from the HTTPS response
 * return number of bytes received for success;
 * return less than 0 for fail;
 * This function do not support http redir.
 */
long aic_https_client_response_data_get(u32 handle, u8 *buf, u32 buf_bytes,
                                        ulong wait_option)
{
    u32 status = AIC_SUCCESS;
    ulong offset = 0;
    ulong bytes_recv = AIC_HTTPS_RECEIVE_PACKET_ERROR;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr = AIC_NULL;
    u32 try_times = 0;
    u8 status_code[4] = {'\0'};

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_RECEIVE_PACKET_ERROR;

    wait_option /= (AIC_HTTPS_MAX_RECEIVE_DATA_TIMES - 1);

    do {
        if (client_ptr->res_data_packet != AIC_NULL &&
            client_ptr->res_data_packet->nx_packet_length != 0) {
            break;
        } else if (client_ptr->res_data_packet != AIC_NULL &&
                   client_ptr->res_data_packet->nx_packet_length == 0) {
            aic_http_packet_release(&client_ptr->res_data_packet);
        } else if (client_ptr->res_data_packet == AIC_NULL) {
            status = aic_https_client_response_body_get(handle,
                     &client_ptr->res_data_packet, status_code, wait_option);

            if ((status == AIC_SUCCESS || status == NX_WEB_HTTP_GET_DONE) &&
                client_ptr->res_data_packet &&
                client_ptr->res_data_packet->nx_packet_length != 0) {
                client_ptr->recv_offset = 0;
                break;
            } else if (status) {
                aicx_error(0x14b375, "x", "http get data fail(0x%x)\n", status);

                if (status == NX_WEB_HTTP_GET_DONE && client_ptr->res_data_packet &&
                    client_ptr->res_data_packet->nx_packet_length == 0) {
                    aic_http_packet_release(&client_ptr->res_data_packet);
                    return AIC_HTTPS_CHUNKED_MODE_DATA_GET_DONE;
                }

                /* only try 3 times when receice NX_NO_PACKET */
                if (status == NX_NO_PACKET) {
                    if (try_times < 3) {
                        aos_sleep(300);
                        try_times++;
                        continue;
                    } else {
                        return AIC_HTTPS_RECEIVE_PACKET_ERROR;
                    }
                }

                if (client_ptr->https_client->nx_web_http_client_state ==
                    NX_WEB_HTTP_CLIENT_STATE_READY || status_code[0] != 2) {
                    return AIC_HTTPS_RECEIVE_PACKET_ERROR;
                }
            }
        }
        aos_sleep(300);
        try_times++;
    } while(try_times < AIC_HTTPS_MAX_RECEIVE_DATA_TIMES);

    if (try_times == AIC_HTTPS_MAX_RECEIVE_DATA_TIMES)
        return AIC_HTTPS_RECEIVE_PACKET_ERROR;

    offset = client_ptr->recv_offset;
    status = nx_packet_data_extract_offset(client_ptr->res_data_packet, offset,
                                           buf, buf_bytes, &bytes_recv);
    if (status) {
        aic_http_packet_release(&client_ptr->res_data_packet);
        client_ptr->recv_offset = 0;
        return AIC_HTTPS_EXTRACT_OFFSET_ERROR;
    }

    offset =  offset + bytes_recv;
    if(client_ptr->res_data_packet->nx_packet_length <= offset)
    {
        aic_http_packet_release(&client_ptr->res_data_packet);
        client_ptr->recv_offset = 0;
    } else {
        client_ptr->recv_offset = offset;
    }

    return bytes_recv;
}

#ifdef AIC_HTTP_REDIR
/**
 * Read specified length of data from the HTTPS response
 * return number of bytes received for success;
 * return less than 0 for fail;
 * This function support http redir.
 */
long aic_https_client_redir_response_data_get(u32 handle, u8 *buf, u32 buf_bytes,
                                              ulong wait_option)
{
    u32 status = AIC_SUCCESS;
    ulong offset = 0;
    ulong bytes_recv = AIC_HTTPS_RECEIVE_PACKET_ERROR;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr = AIC_NULL;
    u32 try_times = 0;
    u8 status_code[4] = {'\0'};
    u8 method = 0;
    HTTPS_CLIENT_QUEUE_MSG http_redir_msg = {'\0'};
    u32 redir_try_times = 0;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_RECEIVE_PACKET_ERROR;

    wait_option /= (AIC_HTTPS_MAX_RECEIVE_DATA_TIMES - 1);

    do {
        if (client_ptr->res_data_packet != AIC_NULL &&
            client_ptr->res_data_packet->nx_packet_length != 0) {
            break;
        } else if (client_ptr->res_data_packet != AIC_NULL &&
                   client_ptr->res_data_packet->nx_packet_length == 0) {
            aic_http_packet_release(&client_ptr->res_data_packet);
        } else if (client_ptr->res_data_packet == AIC_NULL) {
            status = aic_https_client_response_body_get(handle,
                     &client_ptr->res_data_packet, status_code, wait_option);

            if ((status == AIC_SUCCESS || status == NX_WEB_HTTP_GET_DONE) &&
                client_ptr->res_data_packet &&
                client_ptr->res_data_packet->nx_packet_length != 0) {
                client_ptr->recv_offset = 0;
                break;
            } else if (status == AIC_HTTPS_REDIR_STATE) {
                method = client_ptr->https_client->nx_web_http_client_method;

                if (client_ptr->https_client->redir_flag == 1) {
                    switch (method) {
                    case NX_WEB_HTTP_METHOD_GET:
                        if (client_ptr->https_client->redir_info.end != -1 &&
                            client_ptr->https_client->redir_info.start != -1) {
                            http_redir_msg.msg = HTTPS_CLIENT_MSG_RANGE_GET_REDIR;
                        } else {
                            http_redir_msg.msg = HTTPS_CLIENT_MSG_GET_REDIR;
                        }
                        break;
                    case NX_WEB_HTTP_METHOD_PUT:
                        http_redir_msg.msg = HTTPS_CLIENT_MSG_PUT_REDIR;
                        break;
                    case NX_WEB_HTTP_METHOD_POST:
                        if (client_ptr->https_client->redir_info.content_type) {
                            http_redir_msg.msg = HTTPS_CLIENT_MSG_POST_EXT_REDIR;
                        } else {
                            http_redir_msg.msg = HTTPS_CLIENT_MSG_POST_REDIR;
                        }
                        break;
                    case NX_WEB_HTTP_METHOD_HEAD:
                        http_redir_msg.msg = HTTPS_CLIENT_MSG_HEAD_REDIR;
                        break;
                    case NX_WEB_HTTP_METHOD_DELETE:
                        http_redir_msg.msg = HTTPS_CLIENT_MSG_DELETE_REDIR;
                        break;
                    default :
                        break;
                    }
                    http_redir_msg.client_ptr = client_ptr;
                    aos_send_to_queue(http_redir_q, &http_redir_msg,
                                      AOS_WAIT_FOREVER);

                }
                while (client_ptr->https_client->redir_flag &&
                       redir_try_times < AIC_HTTPS_MAX_RECEIVE_DATA_TIMES) {
                    redir_try_times++;
                    aos_sleep(600);
                }
                /* if redir fail, close socket */
                if (client_ptr->https_client->redir_flag &&
                    redir_try_times == AIC_HTTPS_MAX_RECEIVE_DATA_TIMES) {
                    client_ptr->https_client->nx_web_http_client_state =
                                                 NX_WEB_HTTP_CLIENT_STATE_READY;

                    nx_web_http_client_error_exit(client_ptr->https_client, 1000);
                    return AIC_HTTPS_RECEIVE_PACKET_ERROR;
                } else
                    continue; /* redir success, try to get new http response data */
            } else if (status) {
                aicx_error(0x14b375, "x", "http get data fail(0x%x)\n", status);

                if (status == NX_WEB_HTTP_GET_DONE && client_ptr->res_data_packet &&
                    client_ptr->res_data_packet->nx_packet_length == 0) {
                    aic_http_packet_release(&client_ptr->res_data_packet);
                    return AIC_HTTPS_CHUNKED_MODE_DATA_GET_DONE;
                }

                /* only try 3 times when receice NX_NO_PACKET */
                if (status == NX_NO_PACKET) {
                    if (try_times < 3) {
                        aos_sleep(300);
                        try_times++;
                        continue;
                    } else {
                        return AIC_HTTPS_RECEIVE_PACKET_ERROR;
                    }
                }

                if (client_ptr->https_client->nx_web_http_client_state ==
                    NX_WEB_HTTP_CLIENT_STATE_READY || status_code[0] != 2) {
                    return AIC_HTTPS_RECEIVE_PACKET_ERROR;
                }
            }
        }
        aos_sleep(300);
        try_times++;
    } while(try_times < AIC_HTTPS_MAX_RECEIVE_DATA_TIMES);

    if (try_times == AIC_HTTPS_MAX_RECEIVE_DATA_TIMES)
        return AIC_HTTPS_RECEIVE_PACKET_ERROR;

    offset = client_ptr->recv_offset;
    status = nx_packet_data_extract_offset(client_ptr->res_data_packet, offset,
                                           buf, buf_bytes, &bytes_recv);
    if (status) {
        aic_http_packet_release(&client_ptr->res_data_packet);
        client_ptr->recv_offset = 0;
        return AIC_HTTPS_EXTRACT_OFFSET_ERROR;
    }

    offset =  offset + bytes_recv;
    if(client_ptr->res_data_packet->nx_packet_length <= offset)
    {
        aic_http_packet_release(&client_ptr->res_data_packet);
        client_ptr->recv_offset = 0;
    } else {
        client_ptr->recv_offset = offset;
    }

    return bytes_recv;
}
#endif

/* Allocate a packet for HTTPS Client requests */
u32 aic_https_client_request_packet_allocate(u32 handle, NX_PACKET **packet_ptr,
                                             ulong wait_option)
{
    u32 status;
    NXD_ADDRESS *server_ip;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    if (packet_ptr == NX_NULL)
        return AIC_HTTPS_PTR_ERROR;

    server_ip = &(client_ptr->https_client->nx_web_http_client_server_address);

    /* Allocate a packet for the GET (or POST) message. */
#ifdef CONFIG_NET_SECURE
    if (client_ptr->https_client->nx_web_http_client_is_https) {
        status = nx_secure_tls_packet_allocate(
                 &(client_ptr->https_client->nx_web_http_client_tls_session),
                 client_ptr->https_client->nx_web_http_client_packet_pool_ptr,
                 packet_ptr, wait_option);
    } else
#endif
    if (server_ip->nxd_ip_version == NX_IP_VERSION_V4) {
        status = nx_packet_allocate(
                 client_ptr->https_client->nx_web_http_client_packet_pool_ptr,
                 packet_ptr, NX_IPv4_TCP_PACKET, wait_option);
    } else {
        status = nx_packet_allocate(
                 client_ptr->https_client->nx_web_http_client_packet_pool_ptr,
                 packet_ptr, NX_IPv6_TCP_PACKET, wait_option);
    }

    return status;
}

/**
 * This function processes a packet of resource data
 * associated with the previous PUT/POST request.
 */
u32 aic_https_client_put_packet(u32 handle, NX_PACKET *packet_ptr,
                                ulong wait_option)
{
    u32 status;
    u32 header_size;
    NXD_ADDRESS *server_ip;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr) || !packet_ptr)
        return AIC_HTTPS_PTR_ERROR;

    server_ip = &(client_ptr->https_client->nx_web_http_client_server_address);

#ifndef NX_DISABLE_IPV4
    if (server_ip->nxd_ip_version == NX_IP_VERSION_V4) {
        header_size = NX_IPv4_TCP_PACKET;
    }
    else
#endif

#ifdef FEATURE_NX_IPV6
    if (server_ip->nxd_ip_version == NX_IP_VERSION_V6) {
        header_size = NX_IPv6_TCP_PACKET;
    }
    else
#endif
    {
        return NX_NOT_CONNECTED;
    }

    /* Ensure there is enough room for the TCP packet header. */
    if ((u32)(packet_ptr->nx_packet_prepend_ptr -
              packet_ptr->nx_packet_data_start) < header_size)
        return NX_INVALID_PACKET;

    status = nx_web_http_client_put_packet(client_ptr->https_client, packet_ptr,
                                           wait_option);

    return status;
}

/* Send the specified length of data to the server.
   When data length lager than 64K, suggest to use this api. */
u32 aic_https_client_stream_data_put(u32 handle, u8 *data, slong data_len,
                                     ulong wait_option)
{
    u32 status;
    u32 header_size;
    NXD_ADDRESS *server_ip;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    slong offset_len = 0;
    u32 send_packet_len = 0;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    server_ip = &(client_ptr->https_client->nx_web_http_client_server_address);

#ifndef NX_DISABLE_IPV4
    if (server_ip->nxd_ip_version == NX_IP_VERSION_V4) {
        header_size = NX_IPv4_TCP_PACKET;
    }
    else
#endif

#ifdef FEATURE_NX_IPV6
    if (server_ip->nxd_ip_version == NX_IP_VERSION_V6) {
        header_size = NX_IPv6_TCP_PACKET;
    }
    else
#endif
    {
        return NX_NOT_CONNECTED;
    }

    if (client_ptr->put_packet) {
        aic_http_packet_release(&client_ptr->put_packet);
    }

    while (data_len > 0) {
        /* Allocate a packet. */
        status = aic_https_client_request_packet_allocate(handle,
                 &(client_ptr->put_packet), wait_option);
        if (status)
            return status;

        if (data_len > HTTP_ONE_RESPONSE_DATA_BUF_LEN)
            send_packet_len = HTTP_ONE_RESPONSE_DATA_BUF_LEN;
        else
            send_packet_len = data_len;

        /* Build the post packet. */
        status = nx_packet_data_append(client_ptr->put_packet, (data + offset_len),
                                       send_packet_len,
                 client_ptr->https_client->nx_web_http_client_packet_pool_ptr,
                 wait_option);

        if (status)
            goto ERROR_EXIT;

        /* Ensure there is enough room for the TCP packet header. */
        if ((u32)(client_ptr->put_packet->nx_packet_prepend_ptr -
                  client_ptr->put_packet->nx_packet_data_start) < header_size) {
            status = NX_INVALID_PACKET;
            goto ERROR_EXIT;
        }

        status = nx_web_http_client_put_packet(client_ptr->https_client,
                                               client_ptr->put_packet, wait_option);

        if (status) {
            if (status == NX_WEB_HTTP_NOT_READY ||
                status == NX_WEB_HTTP_AUTHENTICATION_ERROR ||
                status == NX_WEB_HTTP_INCOMPLETE_PUT_ERROR ||
                status == NX_WEB_HTTP_BAD_PACKET_LENGTH)
                aic_http_packet_release(&client_ptr->put_packet);

            break;
        }

        data_len -= send_packet_len;
        offset_len += send_packet_len;
    }

    /* put packet has been send and released, set ptr to null. */
    client_ptr->put_packet = AIC_NULL;

    return status;

ERROR_EXIT:
    aic_http_packet_release(&client_ptr->put_packet);
    return status;
}

/* Send the specified length of data to the server. */
u32 aic_https_client_put_data(u32 handle, u8 *data, ulong data_len,
                              ulong wait_option)
{
    u32 status;
    u32 header_size;
    NXD_ADDRESS *server_ip;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    server_ip = &(client_ptr->https_client->nx_web_http_client_server_address);

#ifndef NX_DISABLE_IPV4
    if (server_ip->nxd_ip_version == NX_IP_VERSION_V4) {
        header_size = NX_IPv4_TCP_PACKET;
    }
    else
#endif

#ifdef FEATURE_NX_IPV6
    if (server_ip->nxd_ip_version == NX_IP_VERSION_V6) {
        header_size = NX_IPv6_TCP_PACKET;
    }
    else
#endif
    {
        return NX_NOT_CONNECTED;
    }

    if (client_ptr->put_packet) {
        aic_http_packet_release(&client_ptr->put_packet);
    }

    /* Allocate a packet. */
    status = aic_https_client_request_packet_allocate(handle,
             &(client_ptr->put_packet), wait_option);
    if (status)
        return status;

    /* Build the post packet. */
    status = nx_packet_data_append(client_ptr->put_packet, data, data_len,
             client_ptr->https_client->nx_web_http_client_packet_pool_ptr,
             wait_option);

    if (status)
        goto ERROR_EXIT;

    /* Ensure there is enough room for the TCP packet header. */
    if ((u32)(client_ptr->put_packet->nx_packet_prepend_ptr -
              client_ptr->put_packet->nx_packet_data_start) < header_size) {
        status = NX_INVALID_PACKET;
        goto ERROR_EXIT;
    }

    status = nx_web_http_client_put_packet(client_ptr->https_client,
                                           client_ptr->put_packet, wait_option);
    if (status) {
        if (status == NX_WEB_HTTP_NOT_READY ||
            status == NX_WEB_HTTP_AUTHENTICATION_ERROR ||
            status == NX_WEB_HTTP_INCOMPLETE_PUT_ERROR ||
            status == NX_WEB_HTTP_BAD_PACKET_LENGTH)
            aic_http_packet_release(&client_ptr->put_packet);
    }

    /* put packet has been send and released, set ptr to null. */
    client_ptr->put_packet = AIC_NULL;

    return status;

ERROR_EXIT:
    aic_http_packet_release(&client_ptr->put_packet);
    return status;
}

/* This function sets the chunked information of chunked request. */
u32 aic_https_client_request_chunked_set(u32 handle, u32 chunk_size,
                                         NX_PACKET *packet_ptr)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr) || !packet_ptr)
        return AIC_HTTPS_PTR_ERROR;

    status = nx_web_http_client_request_chunked_set(client_ptr->https_client,
                                                    chunk_size, packet_ptr);

    return status;
}

/* This function sets the chunked data information of chunked request. */
u32 aic_https_client_request_chunked_packet_set(u32 handle, u32 chunk_size,
                                                ulong wait_option)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    status = aic_https_client_request_packet_allocate(handle,
             &(client_ptr->chunked_data_packet), wait_option);
    if (status) {
        return status;
    }

    status = nx_web_http_client_request_chunked_set(client_ptr->https_client,
                                                    chunk_size,
                                                    client_ptr->chunked_data_packet);

    return status;
}

/**
 * This function adds a custom header to an HTTPS request that was
 * previously initialized
 */
u32 aic_https_client_request_header_add(u32 handle, u8 *field_name,
                                        u32 name_length, u8 *field_value,
                                        u32 value_length, ulong wait_option)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr) || !field_name || !field_value)
        return AIC_HTTPS_PTR_ERROR;

    status = nx_web_http_client_request_header_add(client_ptr->https_client,
                                                   field_name, name_length,
                                                   field_value, value_length,
                                                   wait_option);

    return status;
}

/**
 * This function initializes an HTTPS request using information
 * supplied by the caller.
 */
u32 aic_https_client_request_initialize(u32 handle, u32 method, u32 input_size,
                                        u32 encoding_chunked, u8 *username,
                                        u8 *password, ulong wait_option)
{
    u32 status = AIC_SUCCESS;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    switch(method) {
    case AIC_HTTPS_METHOD_POST:
    case AIC_HTTPS_METHOD_PUT:
        if (!input_size && !encoding_chunked) {
            status = NX_WEB_HTTP_METHOD_ERROR;
        }

        break;
    case AIC_HTTPS_METHOD_GET:
    case AIC_HTTPS_METHOD_DELETE:
    case AIC_HTTPS_METHOD_HEAD:
        break;
    case AIC_HTTPS_METHOD_NONE:
    default:
        status = NX_WEB_HTTP_METHOD_ERROR;
    }

    if (status != AIC_SUCCESS)
        return status;

    status = nx_web_http_client_request_initialize(client_ptr->https_client,
             method, client_ptr->login_info.resource, client_ptr->login_info.host,
             input_size, encoding_chunked, username, password, wait_option);

    return status;
}

/* This function sends an HTTPS request to a remote server. */
u32 aic_https_client_request_send(u32 handle, ulong wait_option)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    status = nx_web_http_client_request_send(client_ptr->https_client,
                                             wait_option);

    return status;
}

/**
 * This function sets a callback routine to be invoked when the HTTPS
 * client receives a response from the remote server.
 */
u32 aic_https_client_response_header_callback_set(u32 handle,
    void (*callback_function)(char *field_name, u32 field_name_len,
                              char *field_value, u32 field_value_len))
{
    NX_WEB_HTTP_CLIENT *client_ptr;
    AIC_HTTPS_CLIENT_INSTANCE *https_client_ptr;

    https_client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    /* Check for invalid input pointers. */
    if (!https_client_ptr || !https_client_ptr->https_client ||
        https_client_ptr->https_client->nx_web_http_client_id !=
        NX_WEB_HTTP_CLIENT_ID)
        return AIC_HTTPS_PTR_ERROR;

    client_ptr = https_client_ptr->https_client;
    client_ptr->nx_web_http_client_response_callback_ext = callback_function;

    return AIC_SUCCESS;
}

u32 aic_http_packet_release(NX_PACKET **packet_ptr_ptr)
{
    u32       status;
    NX_PACKET *packet_ptr;

    packet_ptr = *packet_ptr_ptr;

    if (!packet_ptr || !packet_ptr->nx_packet_pool_owner ||
        (packet_ptr->nx_packet_ip_version != NX_IP_VERSION_V4 &&
        packet_ptr->nx_packet_ip_version != NX_IP_VERSION_V6
        )) {
        return AIC_HTTPS_PTR_ERROR;
    }

    status = nx_packet_release(packet_ptr);
    if (status == AIC_SUCCESS)
        *packet_ptr_ptr = AIC_NULL;

    return status;
}

u32 aic_http_input_paras_check(AIC_HTTPS_CLIENT_INSTANCE *client_ptr)
{
    if (!client_ptr || !client_ptr->https_client ||
        client_ptr->https_client->nx_web_http_client_id !=
        NX_WEB_HTTP_CLIENT_ID)
        return AIC_HTTPS_PTR_ERROR;
    else
        return AIC_SUCCESS;
}

void aic_http_host_check(AIC_HTTPS_CLIENT_INSTANCE *client_ptr, u8 *host,
                         ulong wait_option)
{

    u32 prev_host_len = strlen(client_ptr->prev_host);
    u32 connect_host_len = strlen(host);

    if (prev_host_len == connect_host_len &&
        strncmp(client_ptr->prev_host, host, prev_host_len) == 0) {
        //alog_info("host has not been changed\n");
    } else {
        //alog_info("host has been changed\n");
        if (prev_host_len == 0) {
            //alog_info("no prev url, no need to close socket\n");
            strncpy(client_ptr->prev_host, host, HTTPS_MAX_HOST_LEN);
            return;
        }
        /* Clean up and reset the state. */
        nx_web_http_client_cleanup(client_ptr->https_client);

        /* disconnected the old host connection. */
        nx_web_http_client_error_exit(client_ptr->https_client, wait_option);

        /* store the new url */
        strncpy(client_ptr->prev_host, host, HTTPS_MAX_HOST_LEN);
    }

    return;
}

u32 aic_http_free_instance_space(AIC_HTTPS_CLIENT_INSTANCE *client_ptr)
{
    if (!client_ptr)
        return AIC_HTTPS_PTR_ERROR;

    if (client_ptr->login_info.host)
        aos_free(client_ptr->login_info.host);

    if (client_ptr->login_info.resource)
        aos_free(client_ptr->login_info.resource);

    if (client_ptr->prev_host)
        aos_free(client_ptr->prev_host);

    if (client_ptr->https_client) {
        if (client_ptr->https_client->redir_info.url)
            aos_free(client_ptr->https_client->redir_info.url);

        if (client_ptr->https_client->redir_info.content_type)
            aos_free(client_ptr->https_client->redir_info.content_type);

        aos_free(client_ptr->https_client);
    }

    aos_free(client_ptr);
    return AIC_SUCCESS;
}

/* bind the source address through cid */
u32 aic_http_bind_source_ip(AIC_HTTPS_CLIENT_INSTANCE *client_ptr, u8 cid,
                            NXD_ADDRESS *server_ip, ulong wait_option)
{
    u32 status;

    if (server_ip->nxd_ip_version == NX_IP_VERSION_V4) {
        status = aic_tcp_client_source_ip_bind(
                 &(client_ptr->https_client->nx_web_http_client_socket),
                 cid, AF_INET, wait_option);
    } else if (server_ip->nxd_ip_version == NX_IP_VERSION_V6) {
        status = aic_tcp_client_source_ip_bind(
                 &(client_ptr->https_client->nx_web_http_client_socket),
                 cid, AF_INET6, wait_option);
    } else {
        return AIC_HTTPS_BIND_ADDR_FAIL;
    }

    return status;
}

#ifdef AIC_HTTP_REDIR
void aic_http_redir_cb(NX_WEB_HTTP_CLIENT *client_ptr, char *field_name,
                       u32 field_name_len, char *field_value, u32 field_value_len)
{
    u8 name[100];
    u8 *found = AIC_NULL;
    u8 http_type = 0;

    if (field_name_len >= sizeof(name))
        return;

    aos_memset(name, 0, sizeof(name));
    aos_memcpy(name, field_name, field_name_len);

    if (strncasecmp(name, "LOCATION", 8) == 0) {
        if (!client_ptr->redir_info.url)
            client_ptr->redir_info.url = aos_zalloc(HTTPS_MAX_URL_LEN);
        if (!client_ptr->redir_info.url) {
            client_ptr->redir_flag = 0;
            return;
        }

        if (!client_ptr->redir_info.resource)
            client_ptr->redir_info.resource = aos_zalloc(HTTPS_MAX_URL_LEN);
        if (!client_ptr->redir_info.resource) {
            client_ptr->redir_flag = 0;
            return;
        }

        http_type = client_ptr->nx_web_http_client_is_https;

        found = strstr(field_value, "http://");
        if (!found) {
            found = strstr(field_value, "https://");
            if (found) {
                client_ptr->nx_web_http_client_is_https = AIC_TRUE;
                if (http_type == AIC_FALSE)
                    client_ptr->redir_reconnect = 1;
            }
        } else
            client_ptr->nx_web_http_client_is_https = AIC_FALSE;

        if (!found) {
        /* no http host in redir url */
            aos_memset(client_ptr->redir_info.resource, 0, HTTPS_MAX_URL_LEN);
            aos_memcpy(client_ptr->redir_info.resource, field_value,
                       field_value_len);
            client_ptr->redir_info.no_http_host = 1;
        } else {
            aos_memset(client_ptr->redir_info.url, 0, HTTPS_MAX_URL_LEN);
            aos_memcpy(client_ptr->redir_info.url, field_value,
                       field_value_len);
            client_ptr->redir_info.no_http_host = 0;
        }
        client_ptr->redir_flag = 1;
    }

    return;
}
#endif

u32 aic_http_action_prepare(u32 handle, u8 *url, u8 *username, u8 *password,
                            u32 server_port, ulong total_size, slong start,
                            slong end, u8 *content_type, ulong wait_option)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr) || !url)
        return AIC_HTTPS_PTR_ERROR;

    if (client_ptr->https_client->nx_web_http_client_is_https)
        status = aic_process_https_url(handle, url, AIC_HTTPS_HEAD);
    else
        status = aic_process_https_url(handle, url, AIC_HTTP_HEAD);

    if (status)
        return status;

    aic_http_host_check(client_ptr, client_ptr->login_info.host, wait_option);
    if(server_port != 0)
        client_ptr->login_info.server_port = server_port;
    else if (server_port == 0 && client_ptr->login_info.server_port == 0) {
        if (client_ptr->https_client->nx_web_http_client_is_https)
            client_ptr->login_info.server_port = NX_WEB_HTTPS_SERVER_PORT;
        else
            client_ptr->login_info.server_port = NX_WEB_HTTP_SERVER_PORT;
    }

#ifdef AIC_HTTP_REDIR
    aic_https_store_redir_info(handle, username, password, server_port,
                               total_size, start, end, content_type,
                               wait_option);
#endif

    if (client_ptr->cid > 1 && client_ptr->cid <= ANET_MAX_PDP_CID) {
        status = aic_http_bind_source_ip(client_ptr, client_ptr->cid,
                                         &client_ptr->login_info.server_ip,
                                         wait_option);
    }

    return status;
}

/**
 * This function adds a custom header to an HTTP(S) request that was
 * previously initialized with nx_web_http_client_request_initialize.
 * The field name and value are placed directly into the HTTP header
 * so it is up to the caller to assure that they are valid values.
 */
u32 aic_http_client_specific_header_add(NX_WEB_HTTP_CLIENT *client_ptr,
                                        u8 *header_name, u8 *header_value,
                                        ulong wait_option)
{
    u32 status = AIC_SUCCESS;

    status = nx_web_http_client_request_header_add(client_ptr, header_name,
                                          strlen(header_name), header_value,
                                          strlen(header_value), wait_option);

    return status;
}

u8 aic_http_client_chunked_mode_check(u32 handle)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    return client_ptr->https_client->nx_web_http_client_response_chunked;
}

u32 aic_http_client_chunked_response_remaining_size_get(u32 handle)
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    return client_ptr->https_client->nx_web_http_client_chunked_response_remaining_size;
}

#ifdef AIC_HTTP_REDIR
int aic_http_redir_init(void)
{
    http_redir_q = aos_create_queue("http_redir_q", sizeof(HTTPS_CLIENT_QUEUE_MSG),
                                    HTTPS_CLIENT_QUEUE_NUM);
    AOS_ASSERT(http_redir_q);

    http_redir_t = aos_create_thread("http_redir_t", http_redir_entry,
                                     NULL, SIZE_4K, 0, NULL, 15, AOS_AUTO_START);
    AOS_ASSERT(http_redir_t != AOS_INVALID_THREAD_ID);

    return AIC_SUCCESS;
}

DIS_CHARGE_APP_P2_INIT_EXPORT(aic_http_redir_init);
#endif

