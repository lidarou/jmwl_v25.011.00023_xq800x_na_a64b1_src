/******************************************************************************/
/*                                                                            */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_MQTT_CLIENT_H__
#define __AIC_MQTT_CLIENT_H__

#include <nxd_mqtt_client.h>
#include <aic_size.h>

#define AIC_MQTT_NULL 0
#define AIC_MQTT_THREAD_PRIORTY 15
#define AIC_MQTT_MAX_HOST_LEN 68
#define AIC_MQTT_KEEP_ALIVE_TIMER (300 * NX_IP_PERIODIC_RATE)
#define AIC_MQTT_PING_INTERVAL 120 //120s
#define AIC_MQTT_DEFAULT_WAIT_TIME (5 * NX_IP_PERIODIC_RATE)
#define AIC_MQTT_MAX_CLIENT_NUM 8 // 5 mqtt and 3 mqtts

#define AIC_MQTT_PORT 1883
#define AIC_MQTTS_PORT 8883
#define AIC_MQTT_TLS_CREATED 1
#define AIC_MQTT_TLS_NO_CREATED 0
#define AIC_MQTT_MAX_CRT_NUM 6
#define AIC_MQTT_CLIENT_TYPE 1
#define AIC_MQTTS_CLIENT_TYPE 2


#define MQTT_TLS_METADATA_SIZE SIZE_16K
#define MQTT_TLS_PACKET_BUF_SIZE SIZE_4K
#define MQTT_REMOTE_CERT_BUF_SIZE SIZE_2K

/* define error num */
#define AIC_MQTT_CLIENT_CREATE_FAIL 0
#define AIC_MQTT_PTR_ERROR 1
#define AIC_MQTT_HOST_ERROR 2
#define AIC_MQTT_BUFFER_ALLOCATE_ERROR 3
#define AIC_MQTT_HANDLE_INFO_GET_FAIL 4
#define AIC_MQTT_MBEDTLS_SETUP_FAIL 5
#define AIC_MQTT_CRT_ADD_ERROR 6

/* Define the basic MQTT(S) Client structure. */
typedef struct aic_mqtt_client_instance
{
    NXD_MQTT_CLIENT *mqtt_client;
    NXD_ADDRESS server_ip;
    struct aic_mqtt_client_instance *client_ptr_prev;
    struct aic_mqtt_client_instance *client_ptr_next;
    void *tls_metadata_area;
    u8 *tls_packet_buffer;
    u8 *remote_crt_buf;
    u8 *stack;
    u8 cid;
    u8 connect_id;
} AIC_MQTT_CLIENT;

/*MQTT API*/
u32 aic_mqtt_client_create(u8 *client_name, char *client_id);
u32 aic_mqtt_client_create_ext(u8 *client_name, char *client_id, u8 type);

u32 aic_mqtt_client_delete(u32 handle);
u32 aic_mqtt_client_login_set(u32 handle, char *username, char *password);
u32 aic_mqtt_client_connect(u32 handle, u8 *host, u32 server_port,
                            u32 keepalive, u32 clean_session, ulong wait_option);

#ifdef CONFIG_NET_MQTTS
u32 aic_mqtt_ca_crt_add(u32 handle, u8 *crt_data, u16 crt_len);
u32 aic_mqtt_client_ca_crt_remove(u32 handle);
u32 aic_mqtt_client_secure_connect(u32 handle, u8 *host, u32 server_port,
                                   u32 keepalive, u32 clean_session,
                                   ulong wait_option);
#endif

u32 aic_mqtt_client_disconnect(u32 handle);
u32 aic_mqtt_client_publish(u32 handle, char *topic_name, char *message,
                            u32 message_len, u32 retain, u32 QoS,
                            ulong wait_option);

u32 aic_mqtt_client_subscribe(u32 handle, char *topic_name, u32 QoS);
u32 aic_mqtt_client_unsubscribe(u32 handle, char *topic_name);
u32 aic_mqtt_client_receive_notify_set(u32 handle,
    void (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, u32 message_count));

u32 aic_mqtt_client_message_get(u32 handle, u8 *topic_buf, u32 topic_buf_size,
                                u32 *actual_topic_len, u8 *message_buf,
                                u32 message_buf_size, u32 *actual_message_len);

u32 aic_mqtt_client_disconnect_notify_set( u32 handle,
    VOID (*disconnect_notify)(NXD_MQTT_CLIENT *));

u32 aic_mqtt_client_will_message_set(u32 handle, const u8 *will_topic,
                                     u32 will_topic_len, u8 *will_message,
                                     u32 will_message_len, u32 will_retain_flag,
                                     u32 will_QoS);

u32 aic_mqtt_client_ack_receive_notify_set(u32 handle,
    void (*receive_notify)(NXD_MQTT_CLIENT *client_ptr,
                           u32 type, u16 packet_id,
                           NX_PACKET *transmit_packet_ptr, void *context));

u32 aic_mqtt_client_subscribe_ext(u32 handle, char *topic_name, u32 QoS,
                                  u16 *packet_id);

u32 aic_mqtt_client_unsubscribe_ext(u32 handle, char *topic_name,
                                    u16 *packet_id);

u32 mqtt_tls_setup(NXD_MQTT_CLIENT *mqtt_client);

u32 aic_mqtt_client_ping_interval_set(u32 handle, u32 ping_interval);

#endif
