/******************************************************************************/
/*                                                                            */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_MQTT_AT_PROCESS_H__
#define __AIC_MQTT_AT_PROCESS_H__

#include <aic_mqtt_client.h>

#define WILL_OPEN 1
#define WILL_CLOSE 0
#define MESSAGE_OUT_BY_COM 0
#define MESSAGE_STORED 1
#define MQTT_SSL_ENABLE 1
#define MQTT_SSL_DISABLE 0
#define MQTT_MAX_QOS_QUALITY 3
#define MQTT_MAX_TOPIC_LEN 256
#define MQTT_MAX_WILL_LEN 256
#define MQTT_MAX_CLIENT_ID_LEN 256
#define MQTT_MAX_PARA_RES_LEN 300
#define MQTT_MAX_MESSAGE_LEN 1024
#define MQTT_MAX_URC_MESSAGE_LEN 1500
#define MQTT_MAX_KEEP_ALIVE_TIME (3600 * NX_IP_PERIODIC_RATE)

#define MQTT_MAX_MESSAGE_NUM 5
#define MQTT_MAX_URL_LEN 100
#define MQTT_MAX_USERNAME_LEN 20
#define MQTT_MAX_PASSWORD_LEN 20
#define MQTT_RES_SHORT_LEN 20
#define MQTT_RESLEN 52
#define MQTT_MAX_WILL_TOPIC_LEN 256
#define MQTT_MAX_RES_STR_LEN 256
#define MQTT_MAX_PORT_NUM 65535
#define MQTT_AT_CLIENT_NAME "MQTT AT Client"

/* Define MQTT AT thread events. */
#define AIC_MQTT_MESSAGE_EVENT 0x01
#define AIC_MQTT_ANY_EVENT 0xFF

/* define CME ERROR NUM */
#define AIC_MQTT_AT_PARAS_ERROR 2000
#define AIC_MQTT_PARA_OUT_OF_RANGE 2001
#define AIC_MQTT_CLIENT_CREATE_ERROR 2002
#define AIC_MQTT_SERVICE_OPEN_FAIL 2003
#define AIC_MQTT_SUBSCRIBE_FAIL 2004
#define AIC_MQTT_THREAD_CREATE_ERROR 2005
#define AIC_MQTT_EVENT_CREATE_FAIL 2006
#define AIC_MQTT_NO_CREATED_CLIENT 2007
#define AIC_MQTT_UNSUBSCRIBE_FAIL 2009
#define AIC_MQTT_PUBLISH_FAIL 2010
#define AIC_MQTT_CRT_SET_ERROR 2011

typedef struct mqtt_message_info {
    u8 *topic;
    u32 actual_topic_len;
    u32 actual_message_len;
    u8 *message;
    u8 *urc_message;
    u8 message_id;
    u8 store_status;
} MQTT_MESSAGE_INFO;

typedef struct mqtt_ca_crt {
    u8 *ca_crt;
    u32 ca_crt_len;
} MQTT_CA_CRT;

typedef struct mqtt_at_paras {
    u8 host[MQTT_MAX_URL_LEN];
    u8 username[MQTT_MAX_USERNAME_LEN];
    u8 password[MQTT_MAX_PASSWORD_LEN];
    u32 urc_port;
    s32 timeout;
    u32 handle;
    u32 server_port;
    u32 keep_alive_time;
    AIC_MQTT_CLIENT *client_ptr;
    u8 message_mode;
    u8 cid;
    u8 *client_id;
    u8 will_flag;
    u8 will_qos;
    u8 will_retain;
    u8 *will_topic;
    u8 *will_message;
    u8 will_len;
    u8 clean_session;
    u8 ssl_enable;
    u32 ping_interval;
    u8 sub_qos;
    u8 *sub_topic;
    u8 *sub_message;
    u8 pub_qos;
    u8 pub_retain;
    u32 pub_message_len;
    u32 actual_pub_len;
    u8 *pub_topic;
    u8 *pub_message;
    u8 *unsub_topic;
    u32 input_crt_len;
} MQTT_AT_PARAS;

/* Define global functions. */
u32 aic_mqtt_paras_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_paras_query_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_paras_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_service_open_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_service_open_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_subscribe_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_subscribe_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_unsubscribe_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_unsubscribe_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_message_get_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_message_get_query_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_message_get_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_publish_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_publish_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_publish_ext_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_publish_ext_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_term_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_term_test_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_crt_add_set_cmd(AT_REQ_RESP *at_ptr);
u32 aic_mqtt_crt_add_test_cmd(AT_REQ_RESP *at_ptr);

#endif

