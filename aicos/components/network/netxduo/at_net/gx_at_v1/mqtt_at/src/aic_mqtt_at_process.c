/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to MQTT.
 *  This is the handler of AT commands relating to mqtt,
 *  such as setting url.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18ee9f
#define LOG_MODULE    AMOD_NET

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_resp_handle.h>
#include <aic_at_parse_utils.h>
#include <strings.h>

#include <string.h>
#include <aic_size.h>
#include <connmgr_api.h>
#include <aic_mqtt_at_process.h>

static aos_thread_id mqtt_client_event_process_thread;
static MQTT_AT_PARAS mqtt_paras;
static aos_event_ptr mqtt_at_event;

static MQTT_MESSAGE_INFO mqtt_message_list[MQTT_MAX_MESSAGE_NUM];
static MQTT_CA_CRT mqtt_crt;

static AT_CMD_HANDLE mqtt_at_handles[] =
{
    {"AT+MQTTCFG", aic_mqtt_paras_test_cmd, aic_mqtt_paras_query_cmd,
     aic_mqtt_paras_set_cmd, AIC_NULL},

    {"AT+MQTTOPEN", aic_mqtt_service_open_test_cmd, AIC_NULL,
     aic_mqtt_service_open_set_cmd, AIC_NULL},

    {"AT+MQTTSUB", aic_mqtt_subscribe_test_cmd, AIC_NULL,
     aic_mqtt_subscribe_set_cmd, AIC_NULL},

    {"AT+MQTTUNSUB", aic_mqtt_unsubscribe_test_cmd, AIC_NULL,
     aic_mqtt_unsubscribe_set_cmd, AIC_NULL},

    {"AT+MQTTMSGGET", aic_mqtt_message_get_test_cmd,
     aic_mqtt_message_get_query_cmd, AIC_NULL, aic_mqtt_message_get_exe_cmd},

    {"AT+MQTTPUB", aic_mqtt_publish_test_cmd, AIC_NULL,
     aic_mqtt_publish_set_cmd, AIC_NULL},

    {"AT+MQTTPUBEX", aic_mqtt_publish_ext_test_cmd, AIC_NULL,
     aic_mqtt_publish_ext_set_cmd, AIC_NULL},

    {"AT+MQTTTERM", aic_mqtt_term_test_cmd, AIC_NULL, AIC_NULL,
     aic_mqtt_term_exe_cmd},

    {"AT+MQTTCRTADD", aic_mqtt_crt_add_test_cmd, AIC_NULL,
     aic_mqtt_crt_add_set_cmd, AIC_NULL},

    {AIC_NULL},
};

static u32 transmit_mqtt_para_integer(AT_REQ_RESP *at_req_res_ptr, u8 *tag,
                                      u32 value)
{
    u8 result_line0[MQTT_RESLEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, MQTT_RESLEN, "+MQTTCFG: \"%s\",%d", tag, value);
    line_p[0] = (ulong)(&result_line0);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_mqtt_at_ok(AT_REQ_RESP *at_ptr)
{
    u8 result_line[5] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line, AT_RESULT_OK, strlen(AT_RESULT_OK));
    line_p[0] = (ulong)(&result_line);

    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_mqtt_at_cme(AT_REQ_RESP *at_ptr, u32 error_num)
{
    u8 result_line1[MQTT_RES_SHORT_LEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line1, MQTT_RES_SHORT_LEN, "+CME ERROR: %d", error_num);
    line_p[0] = (ulong)(&result_line1);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_mqtt_str(AT_REQ_RESP *at_ptr, u8 *tag, u8 *str)
{
    u8 result[MQTT_MAX_PARA_RES_LEN] = {'\0'};
    ulong line_p[AT_REPLY_LINE_COUNT_ONCE];

    if (tag)
        snprintf(result, MQTT_MAX_PARA_RES_LEN, "+MQTTCFG: \"%s\",\"%s\"",
                 tag, str);
    else
        strncpy(result, str, MQTT_MAX_PARA_RES_LEN);

    line_p[0] = (ulong)(&result);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u8 mqtt_idle_message_info_index_get(void)
{
    int i;

    for (i = 0; i < MQTT_MAX_MESSAGE_NUM; i++) {
        if (mqtt_message_list[i].store_status == 0)
            return i;
    }

    if (i == MQTT_MAX_MESSAGE_NUM) {
        aicx_error(0x1b4a1b, "", "no idle index, empty old list\n");
        for (i = 0; i < MQTT_MAX_MESSAGE_NUM; i++) {
            mqtt_message_list[i].store_status = 0;
        }
        i = 0;
    }

    return i;
}

static void mqtt_message_info_space_allocate(u8 index)
{
    mqtt_message_list[index].message_id = index;
    mqtt_message_list[index].store_status = 0;

    if (mqtt_message_list[index].topic == AIC_NULL)
        mqtt_message_list[index].topic = aos_zalloc(MQTT_MAX_TOPIC_LEN);

    if (mqtt_message_list[index].message == AIC_NULL)
        mqtt_message_list[index].message = aos_zalloc(MQTT_MAX_MESSAGE_LEN);

    aos_memset(mqtt_message_list[index].topic, 0, MQTT_MAX_TOPIC_LEN);
    aos_memset(mqtt_message_list[index].message, 0, MQTT_MAX_MESSAGE_LEN);

    return;
}

static void mqtt_message_info_space_release()
{
    u32 i;

    for (i = 0;i < MQTT_MAX_MESSAGE_NUM; i++) {
        mqtt_message_list[i].store_status = 0;

        if (mqtt_message_list[i].topic) {
            aos_free(mqtt_message_list[i].topic);
            mqtt_message_list[i].topic = AIC_NULL;
        }

        if (mqtt_message_list[i].message) {
            aos_free(mqtt_message_list[i].message);
            mqtt_message_list[i].message = AIC_NULL;
        }

        if (mqtt_message_list[i].urc_message) {
            aos_free(mqtt_message_list[i].urc_message);
            mqtt_message_list[i].urc_message = AIC_NULL;
        }
    }

    return;
}

static void mqtt_paras_space_release()
{

    if (mqtt_paras.client_id) {
        aos_free(mqtt_paras.client_id);
        mqtt_paras.client_id = AIC_NULL;
    }

    if (mqtt_paras.sub_topic) {
        aos_free(mqtt_paras.sub_topic);
        mqtt_paras.sub_topic = AIC_NULL;
    }

    if (mqtt_paras.sub_message) {
        aos_free(mqtt_paras.sub_message);
        mqtt_paras.sub_message = AIC_NULL;
    }

    if (mqtt_paras.unsub_topic) {
        aos_free(mqtt_paras.unsub_topic);
        mqtt_paras.unsub_topic = AIC_NULL;
    }

    if (mqtt_paras.pub_topic) {
        aos_free(mqtt_paras.pub_topic);
        mqtt_paras.pub_topic = AIC_NULL;
    }

    if (mqtt_paras.pub_message) {
        aos_free(mqtt_paras.pub_message);
        mqtt_paras.pub_message = AIC_NULL;
    }

    if (mqtt_paras.will_message) {
        aos_free(mqtt_paras.will_message);
        mqtt_paras.will_message = AIC_NULL;
    }

    if (mqtt_paras.will_topic) {
        aos_free(mqtt_paras.will_topic);
        mqtt_paras.will_topic = AIC_NULL;
    }

    return;
}

static void mqtt_paras_reset(void)
{
    aos_memset(&mqtt_paras, 0, sizeof(MQTT_AT_PARAS));

    mqtt_paras.cid = 1;
    mqtt_paras.timeout = AIC_MQTT_DEFAULT_WAIT_TIME;
    mqtt_paras.server_port = AIC_MQTT_PORT;
    mqtt_paras.keep_alive_time = AIC_MQTT_KEEP_ALIVE_TIMER;
    mqtt_paras.ping_interval = AIC_MQTT_PING_INTERVAL;

    return;
}

static void mqtt_message_receive_cb(NXD_MQTT_CLIENT* client_ptr,
                                    u32 number_of_messages)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(number_of_messages);
    aos_set_event(mqtt_at_event, AIC_MQTT_MESSAGE_EVENT, AOS_OR);

    return;
}

static void mqtt_ack_receive_cb(NXD_MQTT_CLIENT *client_ptr, u32 type,
                                u16 packet_id, NX_PACKET *transmit_packet_ptr,
                                void *context)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(packet_id);
    NX_PARAMETER_NOT_USED(context);
    u8 res_str[12] = {'\0'};

    /* Process based on packet type. */
    switch (type) {
    case MQTT_CONTROL_PACKET_TYPE_PUBACK:
        strncpy(res_str, "PUBACK", 12);
        break;
    case MQTT_CONTROL_PACKET_TYPE_SUBACK:
        strncpy(res_str, "SUBACK", 12);
        break;
    case MQTT_CONTROL_PACKET_TYPE_UNSUBACK:
        strncpy(res_str, "UNSUBACK", 12);
        break;
    /* Publisher sender message type for QoS 2. */
    case MQTT_CONTROL_PACKET_TYPE_PUBREC:
        strncpy(res_str, "PUBREC", 12);
        break;
    case MQTT_CONTROL_PACKET_TYPE_PUBCOMP:
        strncpy(res_str, "PUBCOMP", 12);
        break;
    default:
        break;
    }

    if (strlen(res_str)) {
        if (conn_get_uart_at_conid != AIC_ENODEV)
            aic_transmit_at_result(conn_get_uart_at_conid, res_str,
                                   strlen(res_str));

        if (conn_get_usb_at_conid != AIC_ENODEV)
            aic_transmit_at_result(AT_USB_AT_PORT_ID, res_str,
                                   strlen(res_str));
    }

    return;
}

static void mqtt_message_get_entry(void)
{
    u32 status;
    u8 index;
    u8 urc_buf[MQTT_RES_SHORT_LEN] = {'\0'};

    if (mqtt_paras.sub_message == AIC_NULL)
        mqtt_paras.sub_message = aos_zalloc(MQTT_MAX_URC_MESSAGE_LEN);

    if (mqtt_paras.handle == 0) {
        aicx_error(0x078cdc, "", "no created MQTT Client\n");
        return;
    }

    index = mqtt_idle_message_info_index_get();
    mqtt_message_info_space_allocate(index);

    status = aic_mqtt_client_message_get(mqtt_paras.handle,
             mqtt_message_list[index].topic,  MQTT_MAX_TOPIC_LEN,
             &mqtt_message_list[index].actual_topic_len,
             mqtt_message_list[index].message, MQTT_MAX_MESSAGE_LEN,
             &mqtt_message_list[index].actual_message_len);
    if (status) {
        aicx_error(0x138ae5, "x", "message get fail(0x%x)\n", status);
    } else {
        if (mqtt_paras.message_mode == MESSAGE_OUT_BY_COM) {
            /*out message by com */
            snprintf(mqtt_paras.sub_message, MQTT_MAX_URC_MESSAGE_LEN,
                     "+MQTTSUB: %s,%d,%s", mqtt_message_list[index].topic,
                     mqtt_message_list[index].actual_message_len,
                     mqtt_message_list[index].message);

            aic_transmit_at_data(mqtt_paras.urc_port, mqtt_paras.sub_message,
                                 strlen(mqtt_paras.sub_message));

            mqtt_message_list[index].store_status = 0;
        } else if (mqtt_paras.message_mode == MESSAGE_STORED) {
            if (mqtt_message_list[index].urc_message == AIC_NULL)
                mqtt_message_list[index].urc_message =
                    aos_zalloc(MQTT_MAX_URC_MESSAGE_LEN);

            snprintf(mqtt_message_list[index].urc_message,
                     MQTT_MAX_URC_MESSAGE_LEN, "+MQTTSUB: %s,%d,%s\r\n",
                     mqtt_message_list[index].topic,
                     mqtt_message_list[index].actual_message_len,
                     mqtt_message_list[index].message);

            mqtt_message_list[index].store_status = 1;

            snprintf(urc_buf, MQTT_RES_SHORT_LEN, "+MQTTSUB: %d", index);
            aic_transmit_at_cmd_urc(mqtt_paras.urc_port, urc_buf,
                                    strlen(urc_buf));
        }
    }

    return;
}

void mqtt_at_event_process_entry(void *thread_input)
{
    u32 status;
    u32 events;

    while(1) {
        status = aos_get_event(mqtt_at_event, AIC_MQTT_ANY_EVENT,
                               AOS_OR_CLEAR, &events, AOS_WAIT_FOREVER);

        if (status != AOS_SUCCESS)
            continue;

        if (events & AIC_MQTT_MESSAGE_EVENT)
            mqtt_message_get_entry();
    }
}

/**
 * Set mqtt paras.
 * For example: AT+MQTTCFG="CID",1
                AT+MQTTCFG="USERNAME","Admin"
 */
u32 aic_mqtt_paras_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    u32 temp_len = 0;
    u32 cmd_len = 0;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);
    paras = aic_split_paras_to_str_array(at_ptr->at_req, "AT+MQTTCFG=", &count);

    if (!paras || count != 2) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_AT_PARAS_ERROR;
    }
    cmd_len = strlen((u8 *)paras[0]);

    if (strncasecmp((u8*)paras[0], "CID", 3) == 0 && cmd_len == 3) {
        mqtt_paras.cid = atoi((u8 *)paras[1]);

        if (mqtt_paras.cid <= 0 || mqtt_paras.cid > ANET_MAX_PDP_CID)
            goto PARA_OUT_RANGE;
    } else if (strncasecmp((u8*)paras[0], "USERNAME", 8) == 0 && cmd_len == 8) {
        if (strlen((u8 *)paras[1]) > MQTT_MAX_USERNAME_LEN)
            goto PARA_OUT_RANGE;

        strncpy(mqtt_paras.username, (u8 *)paras[1], MQTT_MAX_USERNAME_LEN);
    } else if (strncasecmp((u8*)paras[0], "PASSWORD", 8) == 0 && cmd_len == 8) {
        if (strlen((u8 *)paras[1]) > MQTT_MAX_PASSWORD_LEN)
            goto PARA_OUT_RANGE;

        strncpy(mqtt_paras.password, (u8 *)paras[1], MQTT_MAX_PASSWORD_LEN);
    } else if (strncasecmp((u8*)paras[0], "CLIENTID", 8) == 0 && cmd_len == 8) {
        temp_len = strlen((u8*)paras[1]);
        if (temp_len < 1 || temp_len > MQTT_MAX_CLIENT_ID_LEN)
            goto PARA_OUT_RANGE;

        if (mqtt_paras.client_id)
            aos_free(mqtt_paras.client_id);

        mqtt_paras.client_id = aos_zalloc(temp_len + 1);
        strncpy(mqtt_paras.client_id, (u8 *)paras[1], temp_len);
    } else if (strncasecmp((u8*)paras[0], "WILL_QOS", 8) == 0 && cmd_len == 8) {
        mqtt_paras.will_qos = atoi((u8 *)paras[1]);

        if (mqtt_paras.will_qos < 0 ||
            mqtt_paras.will_qos > MQTT_MAX_QOS_QUALITY)
            goto PARA_OUT_RANGE;
    } else if (strncasecmp((u8*)paras[0], "WILL_RETAIN", 11) == 0 &&
               cmd_len == 11) {
        mqtt_paras.will_retain = atoi((u8 *)paras[1]);

        if (mqtt_paras.will_retain < 0 || mqtt_paras.will_retain > 1)
            goto PARA_OUT_RANGE;
    } else if (strncasecmp((u8*)paras[0], "WILL_TOPIC", 10) == 0 &&
               cmd_len == 10) {
        temp_len = strlen((u8*)paras[1]);
        if (temp_len < 1 || temp_len > MQTT_MAX_TOPIC_LEN)
            goto PARA_OUT_RANGE;

        if (mqtt_paras.will_topic)
            aos_free(mqtt_paras.will_topic);

        mqtt_paras.will_topic = aos_zalloc(temp_len + 1);
        strncpy(mqtt_paras.will_topic, (u8 *)paras[1], temp_len);
    } else if (strncasecmp((u8*)paras[0], "WILL_LEN", 8) == 0 && cmd_len == 8) {
        mqtt_paras.will_len = atoi((u8 *)paras[1]);

        if (mqtt_paras.will_len < 0 || mqtt_paras.will_len > MQTT_MAX_WILL_LEN)
            goto PARA_OUT_RANGE;
    } else if (strncasecmp((u8*)paras[0], "WILL_FLAG", 9) == 0 &&
               cmd_len == 9) {
        mqtt_paras.will_flag = atoi((u8 *)paras[1]);

        if (mqtt_paras.will_flag < WILL_CLOSE ||
            mqtt_paras.will_flag > WILL_OPEN)
            goto PARA_OUT_RANGE;
    } else if (strncasecmp((u8*)paras[0], "WILL_MESSAGE", 12) == 0 &&
               cmd_len == 12) {
        temp_len = mqtt_paras.will_len;

        if (mqtt_paras.will_message)
            aos_free(mqtt_paras.will_message);

        mqtt_paras.will_message = aos_zalloc(mqtt_paras.will_len + 1);
        strncpy(mqtt_paras.will_message, (u8 *)paras[1],
                mqtt_paras.will_len);
    } else if (strncasecmp((u8*)paras[0], "PING_INTERVAL", 15) == 0 &&
               cmd_len == 15) {
        mqtt_paras.ping_interval = atoi((u8 *)paras[1]);

        if (mqtt_paras.ping_interval < 0 || mqtt_paras.ping_interval > 60)
            goto PARA_OUT_RANGE;

        mqtt_paras.ping_interval =
        mqtt_paras.ping_interval * NX_IP_PERIODIC_RATE;
    } else if (strncasecmp((u8*)paras[0], "SSL_ENABLE", 10) == 0 &&
               cmd_len == 10) {
        mqtt_paras.ssl_enable = atoi((u8 *)paras[1]);

        if (mqtt_paras.ssl_enable < MQTT_SSL_DISABLE ||
            mqtt_paras.ssl_enable > MQTT_SSL_ENABLE)
            goto PARA_OUT_RANGE;
    } else if (strncasecmp((u8*)paras[0], "TIMEOUT", 7) == 0 && cmd_len == 7) {
        if (atoi((u8 *)paras[1]) < 5 || atoi((u8 *)paras[1]) > 1000)
            goto PARA_OUT_RANGE;

        mqtt_paras.timeout = NX_IP_PERIODIC_RATE * atoi((u8 *)paras[1]);
    } else if (strncasecmp((u8*)paras[0], "MSGMODE", 7) == 0 && cmd_len == 7) {
        mqtt_paras.message_mode = atoi((u8 *)paras[1]);

        if (mqtt_paras.message_mode < MESSAGE_OUT_BY_COM ||
            mqtt_paras.message_mode > MESSAGE_STORED)
            goto PARA_OUT_RANGE;
    } else {
        aicx_error(0x0fcbd9, "", "not support the mqtt para\n");
        goto PARA_OUT_RANGE;
    }

    transmit_mqtt_at_ok(at_ptr);
    aic_free_paras_mem(paras, count);

    return AIC_SUCCESS;

PARA_OUT_RANGE:
    transmit_mqtt_at_cme(at_ptr, AIC_MQTT_PARA_OUT_OF_RANGE);
    aic_free_paras_mem(paras, count);

    return AIC_MQTT_PARA_OUT_OF_RANGE;
}

u32 aic_mqtt_paras_query_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_para_integer(at_ptr, "CID", mqtt_paras.cid);
    transmit_mqtt_para_integer(at_ptr, "WILL_QOS", mqtt_paras.will_qos);
    transmit_mqtt_para_integer(at_ptr, "WILL_LEN", mqtt_paras.will_len);
    transmit_mqtt_para_integer(at_ptr, "WILL_FLAG", mqtt_paras.will_flag);
    transmit_mqtt_para_integer(at_ptr, "SSL_ENABLE", mqtt_paras.ssl_enable);
    transmit_mqtt_para_integer(at_ptr, "PING_INTERVAL", mqtt_paras.ping_interval);
    transmit_mqtt_para_integer(at_ptr, "WILL_RETAIN", mqtt_paras.will_retain);
    transmit_mqtt_para_integer(at_ptr, "TIMEOUT", mqtt_paras.timeout);

    transmit_mqtt_str(at_ptr, "CLIENTID", mqtt_paras.client_id);
    transmit_mqtt_str(at_ptr, "WILL_TOPIC", mqtt_paras.will_topic);
    transmit_mqtt_str(at_ptr, "USERNAME", mqtt_paras.username);
    transmit_mqtt_str(at_ptr, "PASSWORD", mqtt_paras.password);
    transmit_mqtt_str(at_ptr, "WILL_MESSAGE", mqtt_paras.will_message);

    transmit_mqtt_at_ok(at_ptr);
    return AIC_SUCCESS;
}

u32 aic_mqtt_paras_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL, "+MQTTCFG: <ParaTag>,<ParaValue>");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Start MQTT service, such as creating MQTT Client.
 * For example: AT+MQTTOPEN="47.101.171.33",1883,1,120
 */
u32 aic_mqtt_service_open_set_cmd(AT_REQ_RESP *at_ptr)
{
    u16 count;
    ulong *paras;
    u32 status;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (mqtt_paras.handle != 0) {
        aicx_error(0x17ac48, "", "MQTT Service is running, no need open\n");
        transmit_mqtt_at_ok(at_ptr);
        return AIC_SUCCESS;
    }

    paras = aic_split_paras_to_str_array(at_ptr->at_req, "AT+MQTTOPEN=",
                                         &count);
    if (!paras || count != 4) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_AT_PARAS_ERROR;
    }

    strncpy(mqtt_paras.host, (u8 *)paras[0], MQTT_MAX_URL_LEN);
    mqtt_paras.server_port = atoi((u8 *)paras[1]);
    mqtt_paras.clean_session = atoi((u8 *)paras[2]);
    mqtt_paras.keep_alive_time = atoi((u8 *)paras[3]);

    mqtt_paras.handle = aic_mqtt_client_create(MQTT_AT_CLIENT_NAME,
                                               mqtt_paras.client_id);
    if (mqtt_paras.handle == 0) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_CLIENT_CREATE_ERROR);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_CLIENT_CREATE_ERROR;
    }

    mqtt_paras.client_ptr = (AIC_MQTT_CLIENT *)mqtt_paras.handle;
    mqtt_paras.client_ptr->mqtt_client->mqtt_mb_cfg->wait_time = mqtt_paras.timeout;
    if (mqtt_paras.username && mqtt_paras.password) {
        status = aic_mqtt_client_login_set(mqtt_paras.handle,
                                           mqtt_paras.username,
                                           mqtt_paras.password);
        if (status) {
            aicx_error(0x1b25cf, "x", "mqtt login info set fail(0x%x)\n", status);
            goto OPEN_ERROR;
        }
    }

    if (mqtt_paras.ssl_enable == MQTT_SSL_ENABLE) {
        mqtt_paras.client_ptr->mqtt_client->auth_type = MBEDTLS_SSL_VERIFY_OPTIONAL;
        if (mqtt_crt.ca_crt == AIC_NULL) {
            aicx_error(0x163562, "", "need to set mqtt certificate firstly\n");
            goto OPEN_ERROR;
        }

        status = aic_mqtt_ca_crt_add(mqtt_paras.handle, mqtt_crt.ca_crt,
                                     mqtt_crt.ca_crt_len);
        if (status) {
            aicx_error(0x1bbf49, "x", "mqtt ca crt add fail(0x%x)\n", status);
            goto OPEN_ERROR;
        }
        status = aic_mqtt_client_secure_connect(mqtt_paras.handle,
                 mqtt_paras.host, mqtt_paras.server_port,
                 mqtt_paras.keep_alive_time, mqtt_paras.clean_session,
                 mqtt_paras.timeout);
    } else {
        status = aic_mqtt_client_connect(mqtt_paras.handle, mqtt_paras.host,
                                         mqtt_paras.server_port,
                                         mqtt_paras.keep_alive_time,
                                         mqtt_paras.clean_session,
                                         mqtt_paras.timeout);
    }

    if (status) {
        aicx_error(0x1403c7, "x", "mqtt client connect to server fail(0x%x)\n", status);
        goto OPEN_ERROR;
    }

    /* Set the ack receive notify function. */
    status = aic_mqtt_client_ack_receive_notify_set(mqtt_paras.handle,
                                                    mqtt_ack_receive_cb);
    if (status) {
        aicx_error(0x1de188, "x", "mqtt ack receive cb set fail(0x%x)\n", status);
        goto OPEN_ERROR;
    }

    /* Set the receive notify function to get pub message.
     *  After connecting to the mqtt server,it is possible to receive
     * subscription information, so set the callback in advance.
     */
    status = aic_mqtt_client_receive_notify_set(mqtt_paras.handle,
                                                mqtt_message_receive_cb);
    if (status) {
        aicx_error(0x17fda6, "x", "mqtt client message receive cb set fail(0x%x)\n", status);
        goto OPEN_ERROR;
    }

    if (mqtt_paras.will_flag && mqtt_paras.will_message &&
        mqtt_paras.will_topic) {
        status = aic_mqtt_client_will_message_set(mqtt_paras.handle,
                 mqtt_paras.will_topic, strlen(mqtt_paras.will_topic),
                 mqtt_paras.will_message, strlen(mqtt_paras.will_message),
                 mqtt_paras.will_retain, mqtt_paras.will_qos);
        if (status) {
            aicx_error(0x18dff3, "x", "mqtt will message set fail(0x%x)\n", status);
            goto OPEN_ERROR;
        }
    }

    transmit_mqtt_at_ok(at_ptr);
    aic_free_paras_mem(paras, count);

    return AIC_SUCCESS;

OPEN_ERROR:
    aic_mqtt_client_delete(mqtt_paras.handle);
    mqtt_paras.handle = 0;
    if (mqtt_crt.ca_crt) {
        aos_free(mqtt_crt.ca_crt);
        mqtt_crt.ca_crt = AIC_NULL;
        mqtt_crt.ca_crt_len = 0;
    }
    transmit_mqtt_at_cme(at_ptr, AIC_MQTT_SERVICE_OPEN_FAIL);
    aic_free_paras_mem(paras, count);
    return AIC_MQTT_SERVICE_OPEN_FAIL;
}

u32 aic_mqtt_service_open_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL,
                      "+MQTTOPEN: <host>,(1~65535),(0~1),(0~3600)");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Subscribe a topic from server.
 * For example: AT+MQTTSUB="egg",1
 */
u32 aic_mqtt_subscribe_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    u32 status;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (mqtt_paras.handle == 0) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_NO_CREATED_CLIENT);
        return AIC_MQTT_NO_CREATED_CLIENT;
    }

    paras = aic_split_paras_to_str_array(at_ptr->at_req, "AT+MQTTSUB=", &count);
    if (!paras || count != 2) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_AT_PARAS_ERROR;
    }

    if (mqtt_paras.sub_topic)
        aos_free(mqtt_paras.sub_topic);

    mqtt_paras.sub_topic = aos_zalloc(strlen((u8 *)paras[0]) + 1);
    strncpy(mqtt_paras.sub_topic, (u8 *)paras[0],
            strlen((u8 *)paras[0]));

    mqtt_paras.sub_qos = atoi((u8 *)paras[1]);
    if (mqtt_paras.sub_qos < 0 || mqtt_paras.sub_qos > 2) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_PARA_OUT_OF_RANGE;
    }

    status = aic_mqtt_client_subscribe(mqtt_paras.handle, mqtt_paras.sub_topic,
                                       mqtt_paras.sub_qos);
    if (status) {
        aicx_error(0x02bf1e, "x", "mqtt subscribe from server fail(0x%x)\n", status);
        goto SUBSCRIBE_FAIL;
    }

    mqtt_paras.urc_port = at_ptr->serial_port_id;
    transmit_mqtt_at_ok(at_ptr);
    aic_free_paras_mem(paras, count);

    return AIC_SUCCESS;

SUBSCRIBE_FAIL:
    transmit_mqtt_at_cme(at_ptr, AIC_MQTT_SUBSCRIBE_FAIL);
    aic_free_paras_mem(paras, count);
    return AIC_MQTT_SUBSCRIBE_FAIL;
}

u32 aic_mqtt_subscribe_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL, "+MQTTSUB: <topic>,<qos>");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Unubscribe a topic from server.
 * For example: AT+MQTTUNSUB="egg"
 */
u32 aic_mqtt_unsubscribe_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    u32 status;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (mqtt_paras.handle == 0) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_NO_CREATED_CLIENT);
        return AIC_MQTT_NO_CREATED_CLIENT;
    }

    paras = aic_split_paras_to_str_array(at_ptr->at_req, "AT+MQTTUNSUB=",
                                         &count);
    if (!paras || count != 1) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_AT_PARAS_ERROR;
    }

    if (mqtt_paras.unsub_topic)
        aos_free(mqtt_paras.unsub_topic);

    mqtt_paras.unsub_topic = aos_zalloc(strlen((u8 *)paras[0]) + 1);
    strncpy(mqtt_paras.unsub_topic, (u8 *)paras[0],
            strlen((u8 *)paras[0]));

    status = aic_mqtt_client_unsubscribe(mqtt_paras.handle,
                                         mqtt_paras.unsub_topic);
    if (status) {
        aicx_error(0x13ec40, "x", "mqtt unsubscribe from server fail(0x%x)\n", status);
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_UNSUBSCRIBE_FAIL);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_UNSUBSCRIBE_FAIL;
    }

    transmit_mqtt_at_ok(at_ptr);
    aic_free_paras_mem(paras, count);

    return AIC_SUCCESS;
}

u32 aic_mqtt_unsubscribe_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL, "+MQTTUNSUB: <topic>");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Get received subscription information.
 * For example: AT+MQTTMSGGET
 */
u32 aic_mqtt_message_get_exe_cmd(AT_REQ_RESP *at_ptr)
{
    u32 i;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+MQTTMSGGET")] != 0x0D &&
        at_ptr->at_req[strlen("AT+MQTTMSGGET")] != 0x0A) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
        return AIC_MQTT_AT_PARAS_ERROR;
    }

    for (i = 0;i < MQTT_MAX_MESSAGE_NUM; i++) {
        if (mqtt_message_list[i].store_status == 1) {
        aic_transmit_at_data(at_ptr->serial_port_id,
                             mqtt_message_list[i].urc_message,
                             strlen(mqtt_message_list[i].urc_message));
        mqtt_message_list[i].store_status = 0;
        }
    }

    transmit_mqtt_at_ok(at_ptr);
    return AIC_SUCCESS;
}

u32 aic_mqtt_message_get_query_cmd(AT_REQ_RESP *at_ptr)
{
    u32 i;
    u8 res[MQTT_RESLEN];

    for (i = 0;i < MQTT_MAX_MESSAGE_NUM; i++) {
        snprintf(res, MQTT_RESLEN, "+MQTTMSGGET: %d,%d", i,
                 (u32)mqtt_message_list[i].store_status);
        transmit_mqtt_str(at_ptr, AIC_NULL, res);
    }

    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

u32 aic_mqtt_message_get_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL, "+MQTTMSGGET");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Publish a message to server.
 * For example: AT+MQTTPUB="egg",1,0,"good,egg"
 */
u32 aic_mqtt_publish_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    u32 status, ret;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (mqtt_paras.handle == 0) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_NO_CREATED_CLIENT);
        return AIC_MQTT_NO_CREATED_CLIENT;
    }

    paras = aic_split_paras_to_str_array(at_ptr->at_req, "AT+MQTTPUB=",
                                         &count);
    if (!paras || count != 4) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_AT_PARAS_ERROR;
    }

    if (mqtt_paras.pub_topic)
        aos_free(mqtt_paras.pub_topic);

    mqtt_paras.pub_topic = aos_zalloc(strlen((u8 *)paras[0]) + 1);
    strncpy(mqtt_paras.pub_topic, (u8 *)paras[0], strlen((u8 *)paras[0]));

    mqtt_paras.pub_qos = atoi((u8 *)paras[1]);
    if (mqtt_paras.pub_qos < 0 || mqtt_paras.pub_qos > 2) {
        ret = AIC_MQTT_PARA_OUT_OF_RANGE;
        goto PUB_FAIL;
    }

    mqtt_paras.pub_retain = atoi((u8 *)paras[2]);
    if (mqtt_paras.pub_retain < 0 || mqtt_paras.pub_retain > 1) {
        ret = AIC_MQTT_PARA_OUT_OF_RANGE;
        goto PUB_FAIL;
    }

    if (mqtt_paras.pub_message)
        aos_free(mqtt_paras.pub_message);

    mqtt_paras.pub_message = aos_zalloc(strlen((u8 *)paras[3]) + 1);
    strncpy(mqtt_paras.pub_message, (u8 *)paras[3],
            strlen((u8 *)paras[3]));

    status = aic_mqtt_client_publish(mqtt_paras.handle, mqtt_paras.pub_topic,
                                     mqtt_paras.pub_message,
                                     strlen(mqtt_paras.pub_message),
                                     mqtt_paras.pub_retain, mqtt_paras.pub_qos,
                                     mqtt_paras.timeout);
    if (status) {
        aicx_error(0x10f25c, "x", "mqtt publish message fail(0x%x)\n", status);
        ret = AIC_MQTT_PUBLISH_FAIL;
        goto PUB_FAIL;
    }

    transmit_mqtt_at_ok(at_ptr);
    aic_free_paras_mem(paras, count);

    return AIC_SUCCESS;

PUB_FAIL:
    transmit_mqtt_at_cme(at_ptr, ret);
    aic_free_paras_mem(paras, count);
    return ret;
}

u32 aic_mqtt_publish_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL,
                      "+MQTTSUB: <topic>,(0~2),(0~1),<message>");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Publish a message(specific length) to server.
 * For example: AT+MQTTPUBEX="egg",1,0,1024
 */
u32 aic_mqtt_publish_ext_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    u32 status;
    u32 temp_size;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (mqtt_paras.handle == 0) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_NO_CREATED_CLIENT);
        return AIC_MQTT_NO_CREATED_CLIENT;
    }

    /* AT mode */
    if (at_ptr->at_mode == AT_CMD_MODE) {
        paras = aic_split_paras_to_str_array(at_ptr->at_req, "AT+MQTTPUBEX=",
                                             &count);
        if (!paras || count != 4) {
            transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
            aic_free_paras_mem(paras, count);
            return AIC_MQTT_AT_PARAS_ERROR;
        }

        if (mqtt_paras.pub_topic)
            aos_free(mqtt_paras.pub_topic);

        mqtt_paras.pub_topic = aos_zalloc(strlen((u8 *)paras[0]) + 1);
        strncpy(mqtt_paras.pub_topic, (u8 *)paras[0],
                strlen((u8 *)paras[0]));

        mqtt_paras.pub_qos = atoi((u8 *)paras[1]);
        if (mqtt_paras.pub_qos < 0 || mqtt_paras.pub_qos > 2)
            goto OUT_RANGE;

        mqtt_paras.pub_retain = atoi((u8 *)paras[2]);
        if (mqtt_paras.pub_retain < 0 || mqtt_paras.pub_retain > 1)
            goto OUT_RANGE;

        mqtt_paras.pub_message_len = atoi((u8 *)paras[3]);
        if (mqtt_paras.pub_message_len < 0 ||
            mqtt_paras.pub_message_len > MQTT_MAX_MESSAGE_LEN)
            goto OUT_RANGE;

        if (mqtt_paras.pub_message)
            aos_free(mqtt_paras.pub_message);

        mqtt_paras.pub_message = aos_zalloc(mqtt_paras.pub_message_len);
        mqtt_paras.actual_pub_len = 0;

        at_ptr->at_mode = AT_DATA_MODE;
        transmit_mqtt_str(at_ptr, AIC_NULL, "INPUT DATA");
        aic_free_paras_mem(paras, count);
        return AIC_SUCCESS;

OUT_RANGE:
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_PARA_OUT_OF_RANGE);
        aic_free_paras_mem(paras, count);
        return AIC_MQTT_PARA_OUT_OF_RANGE;

    } else if (at_ptr->at_mode == AT_DATA_MODE) {

        if (at_ptr->end_data_mode) {
            at_ptr->at_mode = AT_CMD_MODE;
            return AIC_SUCCESS;
        }

        if (strncmp(at_ptr->at_upstream_data, "+++", 3) == 0) {
            at_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x157142, "", "change to AT command mode\n");
            transmit_mqtt_str(at_ptr, AIC_NULL, "INPUT DATA END");

            status = aic_mqtt_client_publish(mqtt_paras.handle,
                     mqtt_paras.pub_topic, mqtt_paras.pub_message,
                     mqtt_paras.actual_pub_len, mqtt_paras.pub_retain,
                     mqtt_paras.pub_qos, mqtt_paras.timeout);
            if (status) {
                aicx_error(0x06ae2c, "x", "mqtt publish(ext) message fail(0x%x)\n", status);
                transmit_mqtt_at_cme(at_ptr, AIC_MQTT_PUBLISH_FAIL);
                return AIC_MQTT_PUBLISH_FAIL;
            }

            transmit_mqtt_at_ok(at_ptr);
            return AIC_SUCCESS;
        }

        /* set message */
        if (mqtt_paras.pub_message) {
            temp_size = at_ptr->upstream_data_len;

            if (mqtt_paras.actual_pub_len + temp_size >
                mqtt_paras.pub_message_len) {
                aicx_info(0x06f44c, "", "pub_message_len out of range, cut off!\n");
                temp_size = mqtt_paras.pub_message_len -
                    mqtt_paras.actual_pub_len;
            }
            if (temp_size != 0) {
                aos_memcpy(mqtt_paras.pub_message + mqtt_paras.actual_pub_len,
                           at_ptr->at_upstream_data, temp_size);
                mqtt_paras.actual_pub_len += temp_size;
                transmit_mqtt_at_ok(at_ptr);
                //alog_info("actual msg len is %d\n", mqtt_paras.actual_pub_len);
            }
        }
    }

    return AIC_SUCCESS;
}

u32 aic_mqtt_publish_ext_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL,
                      "+MQTTSUB: <topic>,(0~2),(0~1),(1~1024)");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Exit the current mqtt session.
 * For example: AT+MQTTTERM
 */
u32 aic_mqtt_term_exe_cmd(AT_REQ_RESP *at_ptr)
{
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+MQTTTERM")] != 0x0D &&
        at_ptr->at_req[strlen("AT+MQTTTERM")] != 0x0A) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
        return AIC_MQTT_AT_PARAS_ERROR;
    }

    if (mqtt_paras.handle == 0) {
        transmit_mqtt_at_cme(at_ptr, AIC_MQTT_NO_CREATED_CLIENT);
        return AIC_MQTT_NO_CREATED_CLIENT;
    }

    mqtt_message_info_space_release();
    mqtt_paras_space_release();

    aic_mqtt_client_disconnect(mqtt_paras.handle);

    if (mqtt_paras.ssl_enable)
        aic_mqtt_client_ca_crt_remove(mqtt_paras.handle);
    aic_mqtt_client_delete(mqtt_paras.handle);

    mqtt_paras.handle = 0;
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

u32 aic_mqtt_term_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL, "+MQTTTERM");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Execution command add the MQTTS certificate.
 * For example: AT+MQTTCRTADD=767
 */
u32 aic_mqtt_crt_add_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras = AIC_NULL;
    u16 count;
    u32 temp_size;

    aicx_info(0x129483, "s", "at cmd is:--%s\n", at_ptr->at_req);

    /* AT mode */
    if (at_ptr->at_mode == AT_CMD_MODE) {
        paras = aic_split_paras_to_str_array(at_ptr->at_req, "AT+MQTTCRTADD=",
                                             &count);
        if (!paras || count != 1) {
            transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
            goto CRT_ADD_ERROR;
        }

        mqtt_paras.input_crt_len = atoi((u8 *)paras[0]);
        if (mqtt_paras.input_crt_len <= 0) {
            transmit_mqtt_at_cme(at_ptr, AIC_MQTT_AT_PARAS_ERROR);
            goto CRT_ADD_ERROR;
        }

        if (mqtt_crt.ca_crt) {
            aos_free(mqtt_crt.ca_crt);
            mqtt_crt.ca_crt = AIC_NULL;
            mqtt_crt.ca_crt_len = 0;
        }
        mqtt_crt.ca_crt = aos_zalloc(mqtt_paras.input_crt_len);

        at_ptr->at_mode = AT_DATA_MODE;
        aicx_info(0x00161d, "", "change to AT data mode\n");
        transmit_mqtt_str(at_ptr, AIC_NULL, "CRT SET START");

CRT_ADD_ERROR:
        aic_free_paras_mem(paras, count);
        return AIC_SUCCESS;
    }

    /* data mode */
    if (at_ptr->at_mode == AT_DATA_MODE) {

        if (at_ptr->end_data_mode) {
            at_ptr->at_mode = AT_CMD_MODE;
            return AIC_SUCCESS;
        }

        if (strncmp(at_ptr->at_upstream_data, "+++", 3) == 0) {
            at_ptr->at_mode = AT_CMD_MODE;
            aicx_info(0x157142, "", "change to AT command mode\n");
            transmit_mqtt_str(at_ptr, AIC_NULL, "CRT SET END");
            return AIC_SUCCESS;
        }

        /* set ca crt */
        if (mqtt_crt.ca_crt) {
            temp_size = at_ptr->upstream_data_len;
            //alog_debug("temp_size is %d\n", temp_size);
            if (mqtt_crt.ca_crt_len + temp_size > mqtt_paras.input_crt_len) {
                aicx_info(0x12e8e2, "", "ca_crt_len out of range, cut off! \n");
                temp_size = mqtt_paras.input_crt_len - mqtt_crt.ca_crt_len;
            }
            if (temp_size != 0) {
                aos_memcpy(mqtt_crt.ca_crt + mqtt_crt.ca_crt_len,
                           at_ptr->at_upstream_data, temp_size);
                mqtt_crt.ca_crt_len += temp_size;
                //alog_info("mqtt_crt.ca_crt_len is %d\n", mqtt_crt.ca_crt_len);
            }
        }
    }

    return AIC_SUCCESS;
}

u32 aic_mqtt_crt_add_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mqtt_str(at_ptr, AIC_NULL, "+MQTTCRTADD: <crt_len>");
    transmit_mqtt_at_ok(at_ptr);

    return AIC_SUCCESS;
}

int aic_mqtt_init(void)
{
    mqtt_client_event_process_thread = aos_create_thread(
                                      "mqtt_client_event_process_thread",
                                      mqtt_at_event_process_entry, NULL,
                                      SIZE_1K, 0, NULL, 15, AOS_AUTO_START);

    if (mqtt_client_event_process_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x1c63a6, "", "mqtt event process thread create fail\n");
        return AIC_MQTT_THREAD_CREATE_ERROR;
    }

    mqtt_at_event = aos_create_event("MQTT AT Thread Events");
    if (mqtt_at_event == AOS_NULL) {
        aicx_error(0x15ef3b, "", "mqtt at event create error\n");
        aos_delete_thread(mqtt_client_event_process_thread);
        return AIC_MQTT_EVENT_CREATE_FAIL;
    }

    mqtt_paras_reset();
    aic_add_at_cmd_handle(&mqtt_at_handles[0]);

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_mqtt_init);

