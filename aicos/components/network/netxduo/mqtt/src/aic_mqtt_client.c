/**************************************************************************/
/*                                                                        */
/*Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Network Component
 *  Messaging Queue Telemetry Transport(MQTT) Client
 */

#define AIC_THIS_FILE    0x02e3ca
#define LOG_MODULE    AMOD_NET

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <nx_ip.h>
#include <nx_ipv6.h>
#include <nxd_bsd.h>

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_api.h>
#include <aic_net_pdp_mgr.h>
#include <aic_mqtt_client.h>

#ifdef CONFIG_NET_MQTTS
#ifdef CONFIG_CRYPTO_MBEDTLS
static void mqtt_mbed_debug(void *ctx, int level, const char *file, int line,
                            const char *str)
{
    ((void) level);

    //alog_info("%s:%04d: %s\n", file, line, str);
    //alog_info("%04d: %s\n", line, str);
    return;
}
#endif /* CONFIG_CRYPTO_MBEDTLS */

static void aic_mqtts_mbedtls_reset(NXD_MQTT_CLIENT *client_ptr)
{
    mbedtls_ssl_free(&client_ptr->mqtt_mb_cfg->ssl);
    mbedtls_ssl_config_free(&client_ptr->mqtt_mb_cfg->conf);
    mbedtls_ctr_drbg_free(&client_ptr->mqtt_mb_cfg->ctr_drbg);
    mbedtls_entropy_free(&client_ptr->mqtt_mb_cfg->entropy);
    mbedtls_ssl_reset_socket(&client_ptr->mqtt_mb_cfg->socket_fd);
    mbedtls_net_free(&client_ptr->mqtt_mb_cfg->socket_fd);

    return;
}

void aic_mqtts_mbedtls_ssl_close(NXD_MQTT_CLIENT *mqtt_client)
{
    mbedtls_ssl_close_notify(&(mqtt_client->mqtt_mb_cfg->ssl));
    aic_mqtts_mbedtls_reset(mqtt_client);

}


u32 mqtt_tls_setup(NXD_MQTT_CLIENT *mqtt_client)
{
#ifdef CONFIG_CRYPTO_MBEDTLS
    int ret;
    const char *pers = "mqtts";
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_net_init(&mqtt_client->mqtt_mb_cfg->socket_fd);
    mbedtls_ssl_set_socket(&mqtt_client->mqtt_mb_cfg->socket_fd,
                   (int)(&mqtt_client->nxd_mqtt_client_socket));
    mqtt_client->mqtt_mb_cfg->wait_time = AIC_MQTT_DEFAULT_WAIT_TIME;

    /* Initialize the RNG and the session data */
    mbedtls_ssl_init(&mqtt_client->mqtt_mb_cfg->ssl);
    mbedtls_ssl_config_init(&mqtt_client->mqtt_mb_cfg->conf);
    mbedtls_ctr_drbg_init(&mqtt_client->mqtt_mb_cfg->ctr_drbg);

    mbedtls_entropy_init(&mqtt_client->mqtt_mb_cfg->entropy);
    mbedtls_ctr_drbg_seed(&mqtt_client->mqtt_mb_cfg->ctr_drbg, mbedtls_entropy_func,
                          &mqtt_client->mqtt_mb_cfg->entropy,
                          (const unsigned char *) pers, strlen(pers));

    ret = mbedtls_ssl_config_defaults(&mqtt_client->mqtt_mb_cfg->conf,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != AIC_SUCCESS) {
        return ret;
    }

    mbedtls_ssl_conf_authmode(&mqtt_client->mqtt_mb_cfg->conf, mqtt_client->auth_type);
    mbedtls_ssl_conf_rng(&mqtt_client->mqtt_mb_cfg->conf, mbedtls_ctr_drbg_random,
                         &mqtt_client->mqtt_mb_cfg->ctr_drbg);
    aic_ssl_set_recv_waitoption(mqtt_client->mqtt_mb_cfg->socket_fd.fd,
                                AIC_MQTT_DEFAULT_WAIT_TIME);
    mbedtls_ssl_conf_dbg(&mqtt_client->mqtt_mb_cfg->conf, mqtt_mbed_debug, NULL);
    mbedtls_ssl_conf_ca_chain(&mqtt_client->mqtt_mb_cfg->conf,
                              (mqtt_client->nxd_mqtt_tls_trusted_certificate),
                              NULL);

    ret = mbedtls_ssl_setup(&mqtt_client->mqtt_mb_cfg->ssl, &mqtt_client->mqtt_mb_cfg->conf);
    if (ret != AIC_SUCCESS) {
        return AIC_MQTT_MBEDTLS_SETUP_FAIL;
    }

    mbedtls_ssl_set_bio(&mqtt_client->mqtt_mb_cfg->ssl,
                        &mqtt_client->mqtt_mb_cfg->socket_fd,
                        mbedtls_net_send_ext, mbedtls_net_recv_ext,
                        mbedtls_net_recv_timeout_ext);
#endif

    return AIC_SUCCESS;
}
#endif

static void mqtt_client_instance_release(AIC_MQTT_CLIENT *client_ptr)
{

#ifdef CONFIG_NET_MQTTS
   if (client_ptr->mqtt_client && client_ptr->mqtt_client->mqtt_mb_cfg)
        aos_free(client_ptr->mqtt_client->mqtt_mb_cfg);

    if (client_ptr->mqtt_client &&
        client_ptr->mqtt_client->nxd_mqtt_tls_trusted_certificate)
        aos_free(client_ptr->mqtt_client->nxd_mqtt_tls_trusted_certificate);
#endif

    if (client_ptr->stack)
        aos_free(client_ptr->stack);

    if (client_ptr->mqtt_client)
        aos_free(client_ptr->mqtt_client);

    aos_free(client_ptr);

    return;
}

/* Process URL to get NXD_ADDRESS */
static u32 mqtt_host_process(AIC_MQTT_CLIENT *client_ptr, u8 *host)
{
    u8 *v6_str;
    struct addrinfo *res;
    u32 status;
    struct addrinfo *hints = AIC_NULL;

    if (strlen(host) == 0)
        return AIC_MQTT_HOST_ERROR;

    hints = aos_zalloc(sizeof(struct addrinfo));
    if (aic_get_pdp_ip_type(client_ptr->cid) == ANET_DEFAULT_IP_TYPE_V4)
        hints->ai_family = AF_INET;
    else if (aic_get_pdp_ip_type(client_ptr->cid) == ANET_DEFAULT_IP_TYPE_V6)
        hints->ai_family = AF_INET6;
    else
        hints->ai_family = AF_UNSPEC;

    //alog_info(" mqtt mqtt_host_process cid is %d, pdp type is %d\n",
              //client_ptr->cid, aic_get_pdp_ip_type(client_ptr->cid));

    if (strncmp(host, "[", 1) == 0) {
        //alog_info("MQTT host is ipv6 addr\n");
        v6_str = (u8 *)aos_zalloc(AIC_MQTT_MAX_HOST_LEN + 1);
        if (v6_str == AIC_NULL) {
            aos_free(hints);
            return AIC_MQTT_BUFFER_ALLOCATE_ERROR;
        }

        strncpy(v6_str, host + 1, AIC_MQTT_MAX_HOST_LEN);
        v6_str[strlen(v6_str) - 1] = '\0';
        status = getaddrinfo(v6_str, NULL, hints, &res);
        aos_free(v6_str);
        v6_str = AIC_NULL;
    } else {
        status = getaddrinfo(host, NULL, hints, &res);
    }

    if (status == 0 && res) {
        //alog_debug("mqtt mqtt_host_process res->ai_family is %d\n", res->ai_family);
        if (res->ai_family == AF_INET) {
            client_ptr->server_ip.nxd_ip_version = NX_IP_VERSION_V4;
            client_ptr->server_ip.nxd_ip_address.v4 =
            ((struct sockaddr_in*)(res->ai_addr))->sin_addr.s_addr;

            NX_CHANGE_ULONG_ENDIAN(client_ptr->server_ip.nxd_ip_address.v4);
        } else if (res->ai_family == AF_INET6) {
            client_ptr->server_ip.nxd_ip_version = NX_IP_VERSION_V6;
            client_ptr->server_ip.nxd_ip_address.v6[0] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[0];

            client_ptr->server_ip.nxd_ip_address.v6[1] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[1];

            client_ptr->server_ip.nxd_ip_address.v6[2] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[2];

            client_ptr->server_ip.nxd_ip_address.v6[3] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[3];

            NX_CHANGE_ULONG_ENDIAN(client_ptr->server_ip.nxd_ip_address.v6[0]);
            NX_CHANGE_ULONG_ENDIAN(client_ptr->server_ip.nxd_ip_address.v6[1]);
            NX_CHANGE_ULONG_ENDIAN(client_ptr->server_ip.nxd_ip_address.v6[2]);
            NX_CHANGE_ULONG_ENDIAN(client_ptr->server_ip.nxd_ip_address.v6[3]);
        }

        freeaddrinfo(res);
        aos_free(hints);
    } else {
        aos_free(hints);
        return status;
    }

    return AIC_SUCCESS;
}

/**
 * Creates a MQTT(S) client on the specified IP.
 *
 * @client_name: mqtt(s) client name.
 * @client_name: mqtt(s) client name
 *
 * CALLED BY
 *    Application
 *
 * return mqtt(s) client handle for success.
 * return 0 for fail.
 */
u32 aic_mqtt_client_create(u8 *client_name, char *client_id)
{
    u32 status;
    u32 client_id_len = 0;
    AIC_MQTT_CLIENT *client_ptr;
    struct aic_network_ip_instance *aic_ip_instance = AIC_NULL;
    NX_IP *mqtt_ip_ptr = AIC_NULL;
    NX_PACKET_POOL *mqtt_packet_pool;

    if (client_name == AIC_NULL)
        return AIC_MQTT_CLIENT_CREATE_FAIL;

    aic_ip_instance = aic_get_default_ip_instance();
    if (!aic_ip_instance || !aic_ip_instance->ip_ptr ||
        !aic_ip_instance->packet_pool) {
        return AIC_MQTT_CLIENT_CREATE_FAIL;
    }

    if (client_id)
        client_id_len = strlen(client_id);

    mqtt_ip_ptr = aic_ip_instance->ip_ptr;
    mqtt_packet_pool = aic_ip_instance->packet_pool;
    client_ptr = aos_zalloc(sizeof(AIC_MQTT_CLIENT));
    client_ptr->mqtt_client = aos_zalloc(sizeof(NXD_MQTT_CLIENT));
    client_ptr->stack = aos_zalloc(SIZE_4K);

    status = nxd_mqtt_client_create(client_ptr->mqtt_client, client_name,
                                    client_id, client_id_len, mqtt_ip_ptr,
                                    mqtt_packet_pool, (void*)client_ptr->stack,
                                    SIZE_4K, AIC_MQTT_THREAD_PRIORTY, NX_NULL, 0);
    if (status) {
        mqtt_client_instance_release(client_ptr);
        client_ptr = AIC_NULL;
        return AIC_MQTT_CLIENT_CREATE_FAIL;
    }

    client_ptr->mqtt_client->ping_interval = AIC_MQTT_PING_INTERVAL;

#ifdef CONFIG_NET_MQTTS
#endif /* CONFIG_NET_MQTTS */

    return (u32)client_ptr;
}

/**
 * Creates a MQTT(S) client on the specified IP.
 *
 * @client_name: mqtt(s) client name.
 * @client_name: mqtt(s) client name.
 * @type: 1:mqtt client; 2:mqtts client.
 *
 * CALLED BY
 *    Application
 *
 * return mqtt(s) client handle for success.
 * return 0 for fail.
 */
u32 aic_mqtt_client_create_ext(u8 *client_name, char *client_id, u8 type)
{
    u32 status;
    u32 client_id_len = 0;
    AIC_MQTT_CLIENT *client_ptr;
    struct aic_network_ip_instance *aic_ip_instance = AIC_NULL;
    NX_IP *mqtt_ip_ptr = AIC_NULL;
    NX_PACKET_POOL *mqtt_packet_pool;

    if (client_name == AIC_NULL)
        return AIC_MQTT_CLIENT_CREATE_FAIL;

    aic_ip_instance = aic_get_default_ip_instance();
    if (!aic_ip_instance || !aic_ip_instance->ip_ptr ||
        !aic_ip_instance->packet_pool) {
        return AIC_MQTT_CLIENT_CREATE_FAIL;
    }

    if (client_id)
        client_id_len = strlen(client_id);

    mqtt_ip_ptr = aic_ip_instance->ip_ptr;
    mqtt_packet_pool = aic_ip_instance->packet_pool;
    client_ptr = aos_zalloc(sizeof(AIC_MQTT_CLIENT));
    client_ptr->mqtt_client = aos_zalloc(sizeof(NXD_MQTT_CLIENT));
    client_ptr->stack = aos_zalloc(SIZE_1K + SIZE_256B);

    status = nxd_mqtt_client_create(client_ptr->mqtt_client, client_name,
                                    client_id, client_id_len, mqtt_ip_ptr,
                                    mqtt_packet_pool, (void*)client_ptr->stack,
                                    (SIZE_1K + SIZE_256B),
                                    AIC_MQTT_THREAD_PRIORTY, NX_NULL, 0);
    if (status) {
        mqtt_client_instance_release(client_ptr);
        client_ptr = AIC_NULL;
        return AIC_MQTT_CLIENT_CREATE_FAIL;
    }

#ifdef CONFIG_NET_MQTTS
    /* Only mqtts need to apply space for mbed tls. */
    if (type == AIC_MQTTS_CLIENT_TYPE) {
        client_ptr->mqtt_client->mqtt_mb_cfg = aos_zalloc(sizeof(MQTTS_MBEDTLS_PARA));
        client_ptr->mqtt_client->nxd_mqtt_tls_trusted_certificate = aos_zalloc(sizeof(mbedtls_x509_crt));

        client_ptr->mqtt_client->mbedtls_ssl_send_cb = mbedtls_ssl_send;
        client_ptr->mqtt_client->mbedtls_ssl_recv_cb = mbedtls_ssl_recv;
        client_ptr->mqtt_client->mbedtls_ssl_close_cb = aic_mqtts_mbedtls_ssl_close;
        client_ptr->mqtt_client->mbedtls_ssl_handshake_cb = mbedtls_ssl_handshake;


    }
#endif
    client_ptr->mqtt_client->ping_interval = AIC_MQTT_PING_INTERVAL;
    return (u32)client_ptr;
}

/**
 * Delete a previously created MQTT(S) client on the specified IP.
 *
 * @handle: mqtt(s) client handle.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_delete(u32 handle)
{
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;

    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    client_ptr->mqtt_client->nxd_mqtt_client_socket.nx_tcp_receive_callback = AIC_NULL;

    nxd_mqtt_client_delete(client_ptr->mqtt_client);
    mqtt_client_instance_release(client_ptr);
    client_ptr = AIC_NULL;

    return AIC_SUCCESS;
}

/**
 * This function sets optional MQTT username and password.this information
 * must be set prior to calling connect function.
 *
 * @handle: mqtt(s) client handle.
 * @username: User name, or NULL if user name is not required.
 * @password: Password string, or NULL if password is not required.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_login_set(u32 handle, char *username, char *password)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_login_set(client_ptr->mqtt_client, username,
                                       strlen(username), password,
                                       strlen(password));

    return status;
}

/**
 * This function makes an initial connection to the MQTT server.
 *
 * @handle: mqtt(s) client handle.
 * @server_ip: Server IP address structure.
 * @server_port: Server port number, in host byte order.
 * @keepalive: Keepalive flag.
 * @clean_session: Clean session flag.
 * @wait_option: Timeout value.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_connect(u32 handle, u8 *host, u32 server_port,
                            u32 keepalive, u32 clean_session, ulong wait_option)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = mqtt_host_process(client_ptr, host);
    if (status)
        return status;

    status = nxd_mqtt_client_connect(client_ptr->mqtt_client,
                                     &client_ptr->server_ip, server_port,
                                     keepalive, clean_session, wait_option);

    return status;
}

#ifdef CONFIG_NET_MQTTS
/**
 * Add a CA Certificate to our trusted store for
 * verifying incoming server certificates.
 * Certificates in the trusted store will not have a private key.
 * For such certificates the private key parameter should be passed as NULL.
 *
 * @handle: mqtt(s) client handle.
 * @crt_data:certificate data.
 * @crt_len:certificate data length.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_ca_crt_add(u32 handle, u8 *crt_data, u16 crt_len)
{
    u32 status = AIC_SUCCESS;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    /* Check for invalid input pointers. */
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

#ifdef CONFIG_CRYPTO_MBEDTLS
    int ret = mbedtls_x509_crt_parse(
              (client_ptr->mqtt_client->nxd_mqtt_tls_trusted_certificate),
              (const unsigned char *)crt_data, crt_len);

    if (ret != AIC_SUCCESS) {
        aicx_error(0x0cacfa, "d", "mqtt ca_crt parse fail(%d)\n", ret);
        status = AIC_MQTT_CRT_ADD_ERROR;
    }
#endif /* CONFIG_CRYPTO_MBEDTLS */

    return status;
}

/**
 * Remove an NX_SECURE_TLS_CERTIFICATE instance from
 * the trusted certificates store, keyed on the Common Name field.
 * If remove fails, the space allocated for storing certificate may not be
 * released.
 *
 * @handle: mqtt(s) client handle.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_ca_crt_remove(u32 handle)
{
    u32 status = AIC_SUCCESS;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    /* Check for invalid input pointers. */
    if (client_ptr == NX_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

#ifdef CONFIG_CRYPTO_MBEDTLS
    mbedtls_x509_crt_free((client_ptr->mqtt_client->nxd_mqtt_tls_trusted_certificate));
#endif

    return status;
}

/**
 * This function makes an initial connection to the MQTTS server.
 *
 * @handle: mqtt(s) client handle.
 * @server_ip: Server IP address structure.
 * @server_port: Server port number, in host byte order.
 * @keepalive: Keepalive flag.
 * @clean_session: Clean session flag.
 * @wait_option: Timeout value.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_secure_connect(u32 handle, u8 *host, u32 server_port,
                                   u32 keepalive, u32 clean_session,
                                   ulong wait_option)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = mqtt_host_process(client_ptr, host);
    if (status)
        return status;

    status = nxd_mqtt_client_secure_connect(client_ptr->mqtt_client,
                                            &client_ptr->server_ip, server_port,
                                            mqtt_tls_setup, keepalive,
                                            clean_session, wait_option);

    return status;
}
#endif /* CONFIG_NET_MQTTS */

/**
 * This function disconnects the MQTT(S) client from a server.
 *
 * @handle: mqtt(s) client handle.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_disconnect(u32 handle)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;
    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_disconnect(client_ptr->mqtt_client);

    return status;
}

/**
 * This function publishes a message to the connected broker.
 *
 * @handle: mqtt(s) client handle.
 * @topic_name: Name of the topic.
 * @message: Message string.
 * @retain: The retain flag, whether or not the broker should store this message.
 * @QoS: Expected QoS level. Range 0~2.
 * @wait_option:Suspension option.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_publish(u32 handle, char *topic_name, char *message,
                            u32 message_len, u32 retain, u32 QoS,
                            ulong wait_option)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_publish(client_ptr->mqtt_client, topic_name,
                                     strlen(topic_name), message,
                                     message_len, retain, QoS, wait_option);

    return status;
}

/**
 * This function sends a subscribe message to the broker.
 *
 * @handle: mqtt(s) client handle.
 * @topic_name: Name of the topic.
 * @QoS: Expected QoS level. Range 0~2.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_subscribe(u32 handle, char *topic_name, u32 QoS)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_subscribe(client_ptr->mqtt_client, topic_name,
                                       strlen(topic_name), QoS);

    return status;
}

u32 aic_mqtt_client_subscribe_ext(u32 handle, char *topic_name, u32 QoS,
                                  u16 *packet_id)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_subscribe_ext(client_ptr->mqtt_client, topic_name,
                                           strlen(topic_name), QoS, packet_id);

    return status;
}

/**
 * This function unsubscribes a topic from the broker.
 *
 * @handle: mqtt(s) client handle.
 * @topic_name: Name of the topic.
 * @QoS: Expected QoS level. Range 1~3.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_unsubscribe(u32 handle, char *topic_name)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_unsubscribe(client_ptr->mqtt_client, topic_name,
                                         strlen(topic_name));

    return status;
}

u32 aic_mqtt_client_unsubscribe_ext(u32 handle, char *topic_name, u16 *packet_id)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_unsubscribe_ext(client_ptr->mqtt_client, topic_name,
                                             strlen(topic_name), packet_id);

    return status;
}

/**
 * This function installs the MQTT(S) client publish notify callback function.
 *
 * @handle: mqtt(s) client handle.
 * @receive_notify: User-supplied callback function, which is invoked upon
 *                  receiving a publish message.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_receive_notify_set(u32 handle,
    void (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, u32 message_count))
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_receive_notify_set(client_ptr->mqtt_client,
                                                receive_notify);

    return status;
}

/**
 * This function retrieves a published MQTT message.
 *
 * @handle: mqtt(s) client handle.
 * @topic_buf: Pointer to the topic buffer where topic is copied to.
 * @topic_buf_size: Size of the topic buffer.
 * @actual_topic_len: Number of bytes copied into topic_buffer.
 * @message_buf: Pointer to the buffer where message is copied to.
 * @message_buf_size: Size of the message_buffer.
 * @actual_message_len: Number of bytes copied into the message buffer.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_message_get(u32 handle, u8 *topic_buf, u32 topic_buf_size,
                                u32 *actual_topic_len, u8 *message_buf,
                                u32 message_buf_size, u32 *actual_message_len)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || message_buf == AIC_NULL ||
        topic_buf == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_message_get(client_ptr->mqtt_client, topic_buf,
                                         topic_buf_size, actual_topic_len,
                                         message_buf, message_buf_size,
                                         actual_message_len);

    return status;
}

/**
 * This function sets the notify function for the disconnect event.
 *
 * @handle: mqtt(s) client handle.
 * @disconnect_notify: The notify function to be used when the client is
 *                     disconnected from the server.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_disconnect_notify_set( u32 handle,
    VOID (*disconnect_notify)(NXD_MQTT_CLIENT *))
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_disconnect_notify_set(client_ptr->mqtt_client,
                                                   disconnect_notify);

    return status;
}

/**
 * This function retrieves a published MQTT message.
 *
 * @handle: mqtt(s) client handle.
 * @will_topic: Will topic string.
 * @will_topic_len: Length of the will topic.
 * @will_message: Will message string.
 * @will_message_len: Length of the will message.
 * @will_retain_flag: Whether or not the will message is to be retained
 *                   when it is published. Valid values are NX_TRUE\NX_FALSE
 * @will_QoS: QoS level to be used when publishing will message. Range 0~2.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_will_message_set(u32 handle, const u8 *will_topic,
                                     u32 will_topic_len, u8 *will_message,
                                     u32 will_message_len, u32 will_retain_flag,
                                     u32 will_QoS)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = nxd_mqtt_client_will_message_set(client_ptr->mqtt_client,
                                              will_topic, will_topic_len,
                                              will_message, will_message_len,
                                              will_retain_flag, will_QoS);

    return status;
}

/**
 * This function installs the MQTT(S) client ack receive notify callback.
 *
 * @handle: mqtt(s) client handle.
 * @receive_notify: User-supplied callback function, which is invoked upon
 *                  receiving a ack packet from sever.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 */
u32 aic_mqtt_client_ack_receive_notify_set(u32 handle,
    void (*receive_notify)(NXD_MQTT_CLIENT *client_ptr,
                           u32 type, u16 packet_id,
                           NX_PACKET *transmit_packet_ptr, void *context))
{
    AIC_MQTT_CLIENT *client_ptr;
    client_ptr = (AIC_MQTT_CLIENT *)handle;

    if (client_ptr == AIC_NULL || receive_notify == AIC_NULL ||
        client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    tx_mutex_get(client_ptr->mqtt_client->nxd_mqtt_client_mutex_ptr,
                 NX_WAIT_FOREVER);

    client_ptr->mqtt_client->nxd_mqtt_ack_receive_notify = receive_notify;

    tx_mutex_put(client_ptr->mqtt_client->nxd_mqtt_client_mutex_ptr);

    return AIC_SUCCESS;
}

u32 aic_mqtt_client_ping_interval_set(u32 handle, u32 ping_interval)
{

    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    client_ptr->mqtt_client->ping_interval = ping_interval;

    return AIC_SUCCESS;
}

