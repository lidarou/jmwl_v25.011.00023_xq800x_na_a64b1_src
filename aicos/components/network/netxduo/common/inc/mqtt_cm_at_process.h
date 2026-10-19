/******************************************************************************/
/*                                                                            */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __MQTT_CM_AT_PROCESS_H__
#define __MQTT_CM_AT_PROCESS_H__

#include <aic_mqtt_client.h>
#include <aic_at_process_common.h>

#define MOBILE_MQTT_INITIAL_ID 10
#define MOBILE_MQTT_STORAGE_MSG_NUM 20
#define MOBILE_MQTT_QUEUE_NUM 12
#define MOBILE_MQTT_MAX_CLIENT_ID 5
#define MOBILE_MQTT_MAX_CLIENT_MUN 6
#define MOBILE_MQTT_MAX_SSL_ID 5
#define MOBILE_MQTT_MAX_TOPIC_NUM 16
#define MOBILE_MQTT_MAX_UNSUB_TOPIC_NUM 4
#define MOBILE_MQTT_DEFAULT_TIMEOUT (5 * NX_IP_PERIODIC_RATE)
#define MOBILE_MATT_MAX_SSL_ID 4
#define MOBILE_MQTT_HOST_LEN 128
#define MOBILE_MQTT_USERNAME_LEN 128
#define MOBILE_MQTT_PASSWORD_LEN 256
#define MOBILE_MQTT_CLIENT_ID_LEN 128
#define MOBILE_MQTT_TOPIC_LEN 256
#define MOBILE_MQTT_WILL_MSG_LEN 256
#define MOBILE_MQTT_WILL_TOPIC_LEN 256
#define MOBILE_MQTT_MSG_LEN (12 * 1024)
#define MOBILE_MQTT_URC_MSG_LEN 2096
#define MOBILE_MQTT_MAX_ALLOC_SIZE 3000
#define MOBILE_MQTT_MAX_CACHED_DATA_SIZE 8192
#define MOBILE_MQTT_MAX_PUB_MSG_LEN 1024

#define MOBILE_MQTT_DEFAULT_RETRAN_INTERVAL 20
#define MOBILE_MQTT_DEFAULT_PING_INTERVAL 120
#define MOBILE_MQTT_DEFAULT_KEEPALIVE_TIME 120
#define MOBILE_MQTT_DEFAULT_RECOONE_TIMES 3
#define MOBILE_MQTT_DEFAULT_RECONN_INTERVAL 20
#define MOBILE_MQTT_URC_SHORT_LEN 32
#define MOBILE_MQTT_URC_HEADER_LEN 64

#define MOBILE_MQTT_ESC 0x1B
#define MOBILE_MQTT_CTRL_Z 0x1A
#define MOBILE_MQTT_ASCII 0
#define MOBILE_MQTT_HEX 1
#define MOBILE_MQTT_ESCAPE 2
#define MOBILE_MSG_OUT_BY_COM 0
#define MOBILE_MSG_STORED 1
#define MOBILE_MQTT_CONN_STATUS_CHECK 1
#define MOBILE_MQTT_NO_CONN_STATUS 0
#define MOBILE_MQTT_SSL_ENABLE 1
#define MOBILE_MQTT_SSL_DISABLE 0

/* Define MQTT MSG TYPE. */
#define MOBILE_MQTT_MESSAGE_DATA_RECEIVE 0x01
#define MOBILE_MQTT_CONNECT 0x02
#define MOBILE_MQTT_DISCONNECT 0x04
#define MOBILE_MQTT_DISCONNECT_BY_SERVER 0x08
#define MOBILE_MQTT_ANY_EVENT 0xFF

/* Define Mobile MQTT client status. */
#define MOBILE_MQTT_CLIENT_CONNECTING 0x01
#define MOBILE_MQTT_CLIENT_CONNECTED 0x02
#define MOBILE_MQTT_CLIENT_DISCONNECTED 0x03

/* define CME ERROR NUM */
#define MOBILE_MQTT_UNKNOW_ERROR 600
#define MOBILE_MQTT_INVALID_PARAMETER 601
#define MOBILE_MQTT_NOT_CONN_OR_CONN_ERROR 602
#define MOBILE_MQTT_IS_CONNECTING 603
#define MOBILE_MQTT_IS_CONNECTED 604
#define MOBILE_MQTT_IS_NETWORK_ERROR 605
#define MOBILE_MQTT_STORAGE_ERROR 606
#define MOBILE_MQTT_STATUS_ERROR 607
#define MOBILE_MQTT_DNS_ERROR 608

/* define CONN URC ERROR NUM */
#define MOBILE_MQTT_CONNECTED 0
#define MOBILE_MQTT_RECONNECTING 1
#define MOBILE_MQTT_DISCONNECTED_BY_USER 2
#define MOBILE_MQTT_CONNECT_REJECT 3
#define MOBILE_MQTT_DISCONNECTED_BY_SERVER 4
#define MOBILE_MQTT_PING_RESP_TIMEOUT 5
#define MOBILE_MQTT_NETWORK_ERROR 6
#define MOBILE_MQTT_DISCONNECTED_UNKNOW_ERROR 255
#define MOBILE_MQTT_PING_SUCCESS 0

#ifdef  CONFIG_GX_AT_V2
#define MQTT_CONNECT_STR        "GMQTTCONN"
#define MQTT_CFG_STR            "GMQTTCFG"
#define MQTT_URC_RESP           "+GMQTTURC: "
#define MQTT_STATE_RESP         "+GMQTTSTATE: "
#define MQTT_READ_RESP          "+GMQTTREAD: "
#define MQTT_UNSUB_RESP         "+GMQTTUNSUB: "
#define MQTT_SUB_RESP           "+GMQTTSUB: "
#define MQTT_PUB_RESP           "+GMQTTPUB: "

#define MQTT_CONNECT_CMD        "AT+GMQTTCONN"
#define MQTT_CFG_CMD            "AT+GMQTTCFG"
#define MQTT_SUB_CMD            "AT+GMQTTSUB"
#define MQTT_UNSUB_CMD          "AT+GMQTTUNSUB"
#define MQTT_PUB_CMD            "AT+GMQTTPUB"
#define MMQTT_READ_CMD          "AT+GMQTTREAD"
#define MQTT_STATE_CMD          "AT+GMQTTSTATE"
#define MMQTT_DISCONNECT_CMD    "AT+GMQTTDISC"
#endif

#ifdef  CONFIG_CUSTOMER_CM_AT
#define MQTT_CONNECT_STR        "MQTTCONN"
#define MQTT_CFG_STR            "MQTTCFG"
#define MQTT_URC_RESP           "+MQTTURC: "
#define MQTT_STATE_RESP         "+MQTTSTATE: "
#define MQTT_READ_RESP          "+MQTTREAD: "
#define MQTT_UNSUB_RESP         "+MQTTUNSUB: "
#define MQTT_SUB_RESP           "+MQTTSUB: "
#define MQTT_PUB_RESP           "+MQTTPUB: "

#define MQTT_CONNECT_CMD        "AT+MQTTCONN"
#define MQTT_CFG_CMD            "AT+MQTTCFG"
#define MQTT_SUB_CMD            "AT+MQTTSUB"
#define MQTT_UNSUB_CMD          "AT+MQTTUNSUB"
#define MQTT_PUB_CMD            "AT+MQTTPUB"
#define MMQTT_READ_CMD          "AT+MQTTREAD"
#define MQTT_STATE_CMD          "AT+MQTTSTATE"
#define MMQTT_DISCONNECT_CMD    "AT+MQTTDISC"
#endif

typedef struct MOBILE_MQTT_AT_PARAS_MAP_STRUCT
{
    u8 *mobile_mqtt_at_paras_str;      /* String of paras    */
    u8  mobile_mqtt_at_paras_code;     /* Code of paras      */
    u8 paras_len;
} MOBILE_MQTT_AT_PARAS_MAP;

typedef enum {
    MOBILE_MQTT_AT_SSL = 1,
    MOBILE_MQTT_AT_CID,
    MOBILE_MQTT_AT_CLEAN,
    MOBILE_MQTT_AT_CACHED,
    MOBILE_MQTT_AT_RECCON,
    MOBILE_MQTT_AT_VERSION,
    MOBILE_MQTT_AT_RETRANS,
    MOBILE_MQTT_AT_PINGREQ,
    MOBILE_MQTT_AT_PINGRESP,
    MOBILE_MQTT_AT_ENCODING,
    MOBILE_MQTT_AT_KEEPALIVE,
    MOBILE_MQTT_AT_WILLOPTION,
    MOBILE_MQTT_AT_WILLPAYLOAD,
    MOBILE_MQTT_AT_MAX,
} MOBILE_MQTT_AT_PARAS_CODE;

typedef struct mobile_mqtt_queue_msg {
    u8 msg_type;
    int mid; //msg id
    NXD_MQTT_CLIENT* client_ptr;
    int connect_id;
    u32 urc_port;
    u32 remaining_length;
} MOBILE_MQTT_QUEUE_MSG;

typedef struct mobile_mqtt_msg_info {
    u8 *store_msg;
    u32 store_msg_len;
    u8 store_status;
} MOBILE_MQTT_MSG_INFO;

typedef struct mqtt_ca_crt {
    u8 *ca_crt;
    u32 ca_crt_len;
} MQTT_CA_CRT;

typedef struct mobile_mqtt_will_msg_info {
    u8 *will_topic;
    u8 *will_msg;
    u8 will_flag;
    u8 will_qos;
    u8 will_retain;
    u32 will_topic_len;
    u32 will_msg_len;
} MOBILE_MQTT_WILL_MSG_INFO;

typedef struct mobile_mqtt_pub_msg_info {
    u32 pub_msg_len;
    u32 actual_pub_len;
    u8 *pub_topic;
    u8 *pub_msg;
    u8 pub_qos;
    u8 pub_retain;
    u8 dup_flag;
    u8 end_mob; //0:end by len or time; 1: end by ctrl+z or esc
    u8 ready; //1: pub msg is ready to be send
} MOBILE_MQTT_PUB_MSG_INFO;

typedef struct mobile_mqtt_sub_msg_info {
    u8 *sub_topic;
    u16 sub_topic_len;
    u8 sub_qos;
} MOBILE_MQTT_SUB_MSG_INFO;

typedef struct mobile_mqtt_usub_msg_info {
    u16 packet_id;
} MOBILE_MQTT_USUB_MSG_INFO;

typedef struct mobile_mqtt_connect_info {
    u8 *host;
    u8 *user;
    u8 *passwd;
    u8 *client_id;
    u32 server_port;
} MOBILE_MQTT_CONNECT_INFO;

typedef struct mobile_mqtt_at_paras {
    MOBILE_MQTT_CONNECT_INFO connect_info;
    MOBILE_MQTT_WILL_MSG_INFO will_info;
    MOBILE_MQTT_PUB_MSG_INFO pub_info;
    MOBILE_MQTT_SUB_MSG_INFO sub_info[MOBILE_MQTT_MAX_TOPIC_NUM];
    MOBILE_MQTT_USUB_MSG_INFO unsub_info[MOBILE_MQTT_MAX_UNSUB_TOPIC_NUM];
    MOBILE_MQTT_MSG_INFO recv_msg_list[MOBILE_MQTT_STORAGE_MSG_NUM];
    AIC_MQTT_CLIENT *client_ptr;
    u32 handle;
    u32 keepalive_time;
    u32 ping_interval;
    u32 retrans_interval;
    u32 time_out;
    u32 urc_port;
    u8 *urc_msg;
    u32 ping_ret;
    int connect_id;
    u8 cid;
    u8 cached_mode;
    u8 clean_session;
    u8 input_format;
    u8 reconn_mode;
    u8 out_format;
    u8 pingack;
    int reconn_times;
    u8 reconn_interval;
    int retry_times;
    u8 ssl_enable;
    u8 ssl_id;
    int total_msg_count;
    u32 total_cached_size; //max 8K
    u8 version;
    aos_timer_ptr reconnect_timer;
    u8 is_active_disconnect; //0:disconnect by server; 1: disconnect by client
    u8 reconn_state;
} MOBILE_MQTT_AT_PARAS;

typedef struct mobile_mqtt_at_paras_temp {
    u32 retrans_interval;
    u8 input_format;
    u8 reconn_mode;
    u8 out_format;
    int reconn_times;
    u8 reconn_interval;
    char retry_times;
    u8 ssl_enable;
    u8 ssl_id;
    u8 will_flag;
    u8 will_qos;
    u8 will_retain;
} MOBILE_MQTT_AT_PARAS_TEMP;


/* Define global functions. */
u32 mobile_mqtt_paras_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_mqtt_paras_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_mqtt_connect_at_set_cmd(AT_REQ_RESP *at_ptr);
u32 mobile_mqtt_connect_at_test_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_mqtt_subscribe_at_set_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_mqtt_unsubscribe_at_set_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_mqtt_publish_at_set_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_mqtt_message_read_at_set_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_mqtt_status_query_at_set_cmd(AT_REQ_RESP *at_ptr);

u32 mobile_mqtt_disconnect_at_set_cmd(AT_REQ_RESP *at_ptr);

#endif

