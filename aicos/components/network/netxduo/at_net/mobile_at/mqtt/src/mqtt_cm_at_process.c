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
#define AIC_THIS_FILE    0x051a41
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
#include <mqtt_cm_at_process.h>
#include <cm_at_common.h>
#include <aic_at_mtls.h>
#include <aic_at_tcpip_common.h>
#include <aic_net_pdp_mgr.h>

#ifdef CONFIG_CRYPTO_MBEDTLS_2
#define M_MQTT_TASK_SIZE  SIZE_4K
#else
#define M_MQTT_TASK_SIZE  (SIZE_2K + SIZE_1K)
#endif

#if CONFIG_CM_SSL_AT || CONFIG_GX_SSL_AT
extern struct mbedtls_config msslconf_list[SSL_CTX_ID_MAX];
#endif
static aos_thread_id m_mqtt_msg_t;
static MOBILE_MQTT_AT_PARAS m_mqtt_paras[MOBILE_MQTT_MAX_CLIENT_ID + 1];
static aos_queue_ptr *m_mqtt_q;

static AT_CMD_HANDLE m_mqtt_at_cmds[] =
{
    {MQTT_CFG_CMD, mobile_mqtt_paras_at_test_cmd, AIC_NULL,
     mobile_mqtt_paras_at_set_cmd, AIC_NULL},

    {MQTT_CONNECT_CMD, mobile_mqtt_connect_at_test_cmd, AIC_NULL,
     mobile_mqtt_connect_at_set_cmd, AIC_NULL},

    {MQTT_SUB_CMD, AIC_NULL, AIC_NULL,
     mobile_mqtt_subscribe_at_set_cmd, AIC_NULL},

    {MQTT_UNSUB_CMD, AIC_NULL, AIC_NULL,
     mobile_mqtt_unsubscribe_at_set_cmd, AIC_NULL},

    {MQTT_PUB_CMD, AIC_NULL, AIC_NULL,
     mobile_mqtt_publish_at_set_cmd, AIC_NULL},

    {MMQTT_READ_CMD, AIC_NULL, AIC_NULL,
     mobile_mqtt_message_read_at_set_cmd, AIC_NULL},

    {MQTT_STATE_CMD, AIC_NULL, AIC_NULL,
     mobile_mqtt_status_query_at_set_cmd, AIC_NULL},

    {MMQTT_DISCONNECT_CMD, AIC_NULL, AIC_NULL,
     mobile_mqtt_disconnect_at_set_cmd, AIC_NULL},

    {AIC_NULL},
};

static void mobile_mqtt_reconnect_timer_stop(int connect_id)
{
    if (aos_is_timer_created(m_mqtt_paras[connect_id].reconnect_timer)) {
        aos_deactivate_timer(m_mqtt_paras[connect_id].reconnect_timer);
        aos_delete_timer(m_mqtt_paras[connect_id].reconnect_timer);
        m_mqtt_paras[connect_id].reconnect_timer = AIC_NULL;
    }
}

static void mobile_mqtt_reconnect_timeout(void *input)
{
    int connect_id;
    MOBILE_MQTT_QUEUE_MSG mobile_mqtt_msg;

    connect_id = (int)input;
    mobile_mqtt_msg.connect_id = connect_id;
    mobile_mqtt_msg.msg_type = MOBILE_MQTT_CONNECT;
    mobile_mqtt_msg.urc_port = m_mqtt_paras[connect_id].urc_port;

    aos_send_to_queue(m_mqtt_q, &mobile_mqtt_msg, AOS_NO_WAIT);
}

static void mobile_mqtt_reconnect_timer_start(int connect_id, u32 timeout_period)
{
    if (!aos_is_timer_created(m_mqtt_paras[connect_id].reconnect_timer)) {
        m_mqtt_paras[connect_id].reconnect_timer = aos_create_timer(
            "mqtt_reconn_timer", mobile_mqtt_reconnect_timeout,
            (void *)connect_id, timeout_period, AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);

        AIC_ASSERT(m_mqtt_paras[connect_id].reconnect_timer);
    } else {
        aos_change_timer(m_mqtt_paras[connect_id].reconnect_timer,
                         timeout_period, AOS_NO_PERIOD);
    }
}

static void moble_mqtt_default_paras_set(int connect_id)
{
    m_mqtt_paras[connect_id].handle = 0;
    m_mqtt_paras[connect_id].version = 4;
    m_mqtt_paras[connect_id].cid = 1;
    m_mqtt_paras[connect_id].time_out = MOBILE_MQTT_DEFAULT_TIMEOUT;
    m_mqtt_paras[connect_id].keepalive_time = MOBILE_MQTT_DEFAULT_KEEPALIVE_TIME;
    m_mqtt_paras[connect_id].retrans_interval = MOBILE_MQTT_DEFAULT_RETRAN_INTERVAL;
    m_mqtt_paras[connect_id].ping_interval = MOBILE_MQTT_DEFAULT_PING_INTERVAL;
    m_mqtt_paras[connect_id].reconn_times = MOBILE_MQTT_DEFAULT_RECOONE_TIMES;
    m_mqtt_paras[connect_id].reconn_interval = MOBILE_MQTT_DEFAULT_RECONN_INTERVAL;
    m_mqtt_paras[connect_id].reconn_state = 0;
    m_mqtt_paras[connect_id].is_active_disconnect = 0;

    return;
}

static void moble_mqtt_paras_reset(int connect_id)
{
    m_mqtt_paras[connect_id].total_cached_size = 0;
    m_mqtt_paras[connect_id].total_msg_count = 0;
    m_mqtt_paras[connect_id].handle = 0;
    m_mqtt_paras[connect_id].client_ptr = AIC_NULL;
    m_mqtt_paras[connect_id].reconn_state = 0;

    return;
}

static u32 mobile_mqtt_input_data_format_process(int connect_id, u8 *dst_data,
                                                 u8 *src_data, u32 src_data_len,
                                                 u32 *dst_data_len)
{
    u32 status = 0;

    switch(m_mqtt_paras[connect_id].input_format) {
    case MOBILE_MQTT_ASCII:
        strncpy(dst_data, src_data, src_data_len);
        break;
    case MOBILE_MQTT_HEX:
        status = aic_hexstring_to_char(dst_data, src_data, src_data_len * 2);
        break;
    case MOBILE_MQTT_ESCAPE:
        aic_escape_data_normal(dst_data, src_data, src_data_len,
                               dst_data_len);
        break;
    default:
        break;
    }

    return status;
}

static u32 mobile_mqtt_idle_msg_index_get(MOBILE_MQTT_AT_PARAS *mqtt_paras)
{
    u32 i;

    for (i = 0; i < MOBILE_MQTT_STORAGE_MSG_NUM; i++) {
        if (mqtt_paras->recv_msg_list[i].store_status == 0)
            return i;
    }

    return i;
}

static void mobile_mqtt_urc_msg_mem_release(int connect_id)
{
    if (m_mqtt_paras[connect_id].urc_msg)
        aos_free(m_mqtt_paras[connect_id].urc_msg);

    m_mqtt_paras[connect_id].urc_msg = AIC_NULL;

    return;
}

static void mobile_mqtt_cached_msg_info_mem_alloc(int connect_id, int index,
                                                  u32 alloc_len)
{

    if (m_mqtt_paras[connect_id].recv_msg_list[index].store_msg == AIC_NULL)
        m_mqtt_paras[connect_id].recv_msg_list[index].store_msg =
            aos_zalloc(alloc_len);

    return;
}

static void mobile_mqtt_store_message_release(int connect_id)
{
    u32 i;

    for (i = 0;i < MOBILE_MQTT_STORAGE_MSG_NUM; i++) {

        if (m_mqtt_paras[connect_id].recv_msg_list[i].store_msg) {
            aos_free(m_mqtt_paras[connect_id].recv_msg_list[i].store_msg);
            m_mqtt_paras[connect_id].recv_msg_list[i].store_msg = AIC_NULL;
        }

        m_mqtt_paras[connect_id].recv_msg_list[i].store_status = 0;
        m_mqtt_paras[connect_id].recv_msg_list[i].store_msg_len = 0;
    }

    return;
}

static void mobile_mqtt_connect_info_mem_release(int connect_id)
{
    if (m_mqtt_paras[connect_id].connect_info.client_id) {
        aos_free(m_mqtt_paras[connect_id].connect_info.client_id);
        m_mqtt_paras[connect_id].connect_info.client_id = AIC_NULL;
    }

    if (m_mqtt_paras[connect_id].connect_info.host) {
        aos_free(m_mqtt_paras[connect_id].connect_info.host);
        m_mqtt_paras[connect_id].connect_info.host = AIC_NULL;
    }

    if (m_mqtt_paras[connect_id].connect_info.user) {
        aos_free(m_mqtt_paras[connect_id].connect_info.user);
        m_mqtt_paras[connect_id].connect_info.user = AIC_NULL;
    }

    if (m_mqtt_paras[connect_id].connect_info.passwd) {
        aos_free(m_mqtt_paras[connect_id].connect_info.passwd);
        m_mqtt_paras[connect_id].connect_info.passwd = AIC_NULL;
    }

    return;
}

static void mobile_mqtt_will_info_mem_release(int connect_id)
{
    if (m_mqtt_paras[connect_id].will_info.will_msg) {
        aos_free(m_mqtt_paras[connect_id].will_info.will_msg);
        m_mqtt_paras[connect_id].will_info.will_msg = AIC_NULL;
    }

    if (m_mqtt_paras[connect_id].will_info.will_topic) {
        aos_free(m_mqtt_paras[connect_id].will_info.will_topic);
        m_mqtt_paras[connect_id].will_info.will_topic = AIC_NULL;
    }

    return;
}

static void mobile_mqtt_pub_info_mem_release(int connect_id)
{
    if (m_mqtt_paras[connect_id].pub_info.pub_msg) {
        aos_free(m_mqtt_paras[connect_id].pub_info.pub_msg);
        m_mqtt_paras[connect_id].pub_info.pub_msg = AIC_NULL;
    }

    if (m_mqtt_paras[connect_id].pub_info.pub_topic) {
        aos_free(m_mqtt_paras[connect_id].pub_info.pub_topic);
        m_mqtt_paras[connect_id].pub_info.pub_topic = AIC_NULL;
    }

    return;
}

static void mobile_mqtt_sub_info_mem_release(int connect_id)
{
    int i;

    for (i = 0; i < MOBILE_MQTT_MAX_TOPIC_NUM; i++) {
        if (m_mqtt_paras[connect_id].sub_info[i].sub_topic) {
            aos_free(m_mqtt_paras[connect_id].sub_info[i].sub_topic);
            m_mqtt_paras[connect_id].sub_info[i].sub_topic = AIC_NULL;
        }
    }

    return;
}

static void mobile_mqtt_paras_mem_release(int connect_id)
{
    mobile_mqtt_connect_info_mem_release(connect_id);
    mobile_mqtt_will_info_mem_release(connect_id);
    mobile_mqtt_pub_info_mem_release(connect_id);
    mobile_mqtt_sub_info_mem_release(connect_id);
    mobile_mqtt_store_message_release(connect_id);
    mobile_mqtt_urc_msg_mem_release(connect_id);

    return;
}

static void mobile_mqtt_server_ip_set(AIC_MQTT_CLIENT *client_ptr,
                                      struct addrinfo *res)
{
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

    return;
}

static u32 mobile_mqtt_host_process(AIC_MQTT_CLIENT *client_ptr, u8 *host, u8 cid)
{
    u8 *v6_str;
    struct addrinfo *res;
    u32 status;
    struct addrinfo hints;
    u8 dns_prio = 0;
    u8 pdp_type = 0;

    if (strlen(host) == 0)
        return AIC_MQTT_HOST_ERROR;

    dns_prio = aic_get_dns_prio();
    pdp_type = aic_get_pdp_ip_type(cid);

    //alog_debug("mobile mqttp dns process cid is %d ;type is %d; dns_prio is %d\n", cid, pdp_type, dns_prio);

    aos_memset(&hints, 0, sizeof(struct addrinfo));

    if (dns_prio == MOBILE_DNS_PRIO_V4) {
        if (pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6)
            hints.ai_family = AF_INET;
        else if (ANET_DEFAULT_IP_TYPE_V6)
            hints.ai_family = AF_INET6;
    } else if (dns_prio == MOBILE_DNS_PRIO_V6) {
        if (pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6)
            hints.ai_family = AF_INET6;
        else if (ANET_DEFAULT_IP_TYPE_V4)
            hints.ai_family = AF_INET;
    }

    if (mobile_http_ipv4_addr_check(host))
        hints.ai_family = AF_INET;

    if (strncmp(host, "[", 1) == 0) {
        v6_str = (u8 *)aos_zalloc(AIC_MQTT_MAX_HOST_LEN + 1);
        if (v6_str == AIC_NULL) {
            return AIC_MQTT_BUFFER_ALLOCATE_ERROR;
        }

        strncpy(v6_str, host + 1, AIC_MQTT_MAX_HOST_LEN);
        v6_str[strlen(v6_str) - 1] = '\0';
        status = getaddrinfo(v6_str, NULL, &hints, &res);
        aos_free(v6_str);
        v6_str = AIC_NULL;
    } else {
        status = getaddrinfo(host, NULL, &hints, &res);
    }

    if (status == 0 && res) {
        mobile_mqtt_server_ip_set(client_ptr, res);
    } else if (status) {
        hints.ai_family = AF_UNSPEC;

        status = getaddrinfo(host, NULL, &hints, &res);
        if (status == 0 && res) {
            mobile_mqtt_server_ip_set(client_ptr, res);
        } else {
            return status;
        }
    }

    if (res)
        freeaddrinfo(res);

    return AIC_SUCCESS;
}


static int mobile_mqtt_using_connectid_find(NXD_MQTT_CLIENT *mqtt_client)
{
    int i;

    for (i = 0; i < MOBILE_MQTT_MAX_CLIENT_MUN; i++) {
        if (m_mqtt_paras[i].client_ptr &&
            m_mqtt_paras[i].client_ptr->mqtt_client == mqtt_client)
            break;
    }

    return i;
}


static u32 mobile_mqtt_connect(u32 handle, u8 cid, u8 *host, u32 server_port,
                               u32 keepalive, u32 clean_session,
                               ulong wait_option)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = mobile_mqtt_host_process(client_ptr, host, cid);
    if (status)
        return status;

    status = nxd_mqtt_client_connect(client_ptr->mqtt_client,
                                     &client_ptr->server_ip, server_port,
                                     keepalive, clean_session, wait_option);

    return status;
}

#if CONFIG_NET_MQTTS && (CONFIG_CM_SSL_AT || CONFIG_GX_SSL_AT)
static u32 mobile_mqtts_connect(u32 handle, u8 cid, u8 *host, u32 server_port,
                                u32 keepalive, u32 clean_session,
                                ulong wait_option)
{
    u32 status;
    AIC_MQTT_CLIENT *client_ptr;

    client_ptr = (AIC_MQTT_CLIENT *)handle;
    if (client_ptr == AIC_NULL || client_ptr->mqtt_client == AIC_NULL)
        return AIC_MQTT_PTR_ERROR;

    status = mobile_mqtt_host_process(client_ptr, host, cid);
    if (status)
        return status;

    status = nxd_mqtt_client_secure_connect(client_ptr->mqtt_client,
                                            &client_ptr->server_ip, server_port,
                                            mqtt_tls_setup, keepalive,
                                            clean_session, wait_option);

    return status;
}
#endif

static void mobile_mqtt_connect_cb(NXD_MQTT_CLIENT *client_ptr, u32 status,
                                   void *context)
{
    NX_PARAMETER_NOT_USED(context);
    u8 res_str[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};
    int connect_id;
    u32 ret = 0;

    connect_id = mobile_mqtt_using_connectid_find(client_ptr);
    if (connect_id > MOBILE_MQTT_MAX_CLIENT_ID)
        return;

    /* Process based on packet type. */
    switch (status) {
    case NXD_MQTT_SUCCESS:
        ret = MOBILE_MQTT_CONNECTED;
        break;
    case NXD_MQTT_CONNECT_FAILURE:
    case NXD_MQTT_INTERNAL_ERROR:
        ret = MOBILE_MQTT_NETWORK_ERROR;
        break;
    case NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD:
        ret = MOBILE_MQTT_CONNECT_REJECT;
        break;
    default:
        ret = MOBILE_MQTT_DISCONNECTED_UNKNOW_ERROR;
        break;
    }

    if (status == NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD) {
        snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN, MQTT_URC_RESP"\"conn\",%d,%d",
                 connect_id, MOBILE_MQTT_CONNECT_REJECT);
    }

    if (status == NXD_MQTT_SUCCESS) {
        snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN, MQTT_URC_RESP"\"conn\",%d,%d",
                 connect_id, MOBILE_MQTT_CONNECTED);
    }

    if (strlen(res_str))
        aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port, res_str,
                                strlen(res_str));

    return;
}

static void mobile_mqtt_disconnect_cb(NXD_MQTT_CLIENT *client_ptr)
{
    u8 res_str[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};
    int connect_id;
    MOBILE_MQTT_QUEUE_MSG mobile_mqtt_msg = {'\0'};

    connect_id = mobile_mqtt_using_connectid_find(client_ptr);
    if (connect_id > MOBILE_MQTT_MAX_CLIENT_ID)
        return;

    //alog_info("mobile mqtt id %d disconnect is active[%d]\n",
              //index, m_mqtt_paras[index].is_active_disconnect);

    if (m_mqtt_paras[connect_id].is_active_disconnect == 0) {
        mobile_mqtt_msg.msg_type = MOBILE_MQTT_DISCONNECT_BY_SERVER;
        mobile_mqtt_msg.connect_id = connect_id;
        m_mqtt_paras[connect_id].is_active_disconnect = 0;
        mobile_mqtt_msg.urc_port = m_mqtt_paras[connect_id].urc_port;
        aos_send_to_queue(m_mqtt_q, &mobile_mqtt_msg, MOBILE_MQTT_DEFAULT_TIMEOUT);
    } else if (m_mqtt_paras[connect_id].is_active_disconnect == 1) {
        snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN,
                 MQTT_URC_RESP"\"conn\",%d,%d", connect_id,
                 MOBILE_MQTT_DISCONNECTED_BY_USER);

        if (strlen(res_str))
            aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port, res_str,
                                    strlen(res_str));
    }

    m_mqtt_paras[connect_id].is_active_disconnect = 0;
    return;
}

static void mobile_mqtt_message_receive_cb(NXD_MQTT_CLIENT* client_ptr,
                                           u32 qos, u32 remaining_length)
{
    MOBILE_MQTT_QUEUE_MSG mobile_mqtt_msg;
    int connect_id;

    connect_id = mobile_mqtt_using_connectid_find(client_ptr);
    if (connect_id > MOBILE_MQTT_MAX_CLIENT_ID)
        return;

    if (remaining_length >= MOBILE_MQTT_MSG_LEN)
        remaining_length = MOBILE_MQTT_MSG_LEN;

    mobile_mqtt_msg.remaining_length = remaining_length;
    mobile_mqtt_msg.client_ptr = client_ptr;
    mobile_mqtt_msg.connect_id = connect_id;
    mobile_mqtt_msg.msg_type = MOBILE_MQTT_MESSAGE_DATA_RECEIVE;
    /* nxd_mqtt_client_packet_identifier need to - 1,
     * because packet id Update by netx after sending */
    mobile_mqtt_msg.mid = client_ptr->nxd_mqtt_client_packet_identifier - 1;
    if (mobile_mqtt_msg.mid < 0)
        mobile_mqtt_msg.mid = 0;

    if (qos == 0)
        mobile_mqtt_msg.mid = 0;

    aos_send_to_queue(m_mqtt_q, &mobile_mqtt_msg, MOBILE_MQTT_DEFAULT_TIMEOUT);

    return;
}

static void mobile_mqtt_ack_receive_cb(NXD_MQTT_CLIENT *client_ptr, u32 type,
                                       u16 packet_id,
                                       NX_PACKET *transmit_packet_ptr,
                                       void *context)
{
    u8 res_str[48] = {'\0'};
    int connect_id;
    u32 qos = 0;

    connect_id = mobile_mqtt_using_connectid_find(client_ptr);
    if (connect_id > MOBILE_MQTT_MAX_CLIENT_ID)
        return;

    /* Process based on packet type. */
    switch (type) {
    case MQTT_CONTROL_PACKET_TYPE_PUBACK:
        snprintf(res_str, 48, MQTT_URC_RESP"\"puback\",%d,%d,%d", connect_id,
                 packet_id, m_mqtt_paras[connect_id].pub_info.dup_flag);
        break;
    case MQTT_CONTROL_PACKET_TYPE_SUBACK:
        qos = (u32)context;
        if (qos > 2) /* qos: 0, 1, 2; if faild, code:128 */
            qos = 128;
        snprintf(res_str, 48, MQTT_URC_RESP"\"suback\",%d,%d,%u", connect_id, packet_id,
                 qos);
        break;
    case MQTT_CONTROL_PACKET_TYPE_UNSUBACK:
        snprintf(res_str, 48, MQTT_URC_RESP"\"unsuback\",%d,%d", connect_id, packet_id);
        break;
    /* Publisher sender message type for QoS 2. */
    case MQTT_CONTROL_PACKET_TYPE_PUBREC:
        snprintf(res_str, 48, MQTT_URC_RESP"\"pubrec\",%d,%d,%d", connect_id,
                 packet_id, m_mqtt_paras[connect_id].pub_info.dup_flag);
        break;
    case MQTT_CONTROL_PACKET_TYPE_PUBCOMP:
        snprintf(res_str, 48, MQTT_URC_RESP"\"pubcomp\",%d,%d,%d",
                 connect_id, packet_id, m_mqtt_paras[connect_id].pub_info.dup_flag);
        break;
    case MQTT_CONTROL_PACKET_TYPE_PINGRESP:
        if (m_mqtt_paras[connect_id].pingack) {
            m_mqtt_paras[connect_id].ping_ret =
            m_mqtt_paras[connect_id].client_ptr->mqtt_client->nxd_mqtt_ping_not_responded;

            snprintf(res_str, 48, MQTT_URC_RESP"\"pingresp\",%d,%d", connect_id,
                     m_mqtt_paras[connect_id].ping_ret);
        }
        break;
    default:
        break;
    }

    aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port, res_str,
                            strlen(res_str));

    return;
}

static void mobile_mqtt_publish_cb(NXD_MQTT_CLIENT *client_ptr, u32 qos,
                                   u16 packet_id, u32 pub_packet_len)
{
    u8 res_str[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};
    int index;

    index = mobile_mqtt_using_connectid_find(client_ptr);
    if (index > MOBILE_MQTT_MAX_CLIENT_ID)
        return;

    if (qos == 0)
        packet_id = 0;

    snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN, MQTT_PUB_RESP"%d,%d,%d", index,
             packet_id, pub_packet_len);

    aic_transmit_at_cmd_urc(m_mqtt_paras[index].urc_port, res_str,
                            strlen(res_str));

    transmit_mobile_at_ok_urc(m_mqtt_paras[index].urc_port);

    return;
}

static u32 mobile_mqtt_packet_receive_cb(NXD_MQTT_CLIENT *client_ptr,
                                         NX_PACKET *packet_ptr, VOID *context)
{
    u8 res_str[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};
    int index;
    u8 packet_type;
    ulong bytes_copied;

    index = mobile_mqtt_using_connectid_find(client_ptr);
    if (index > MOBILE_MQTT_MAX_CLIENT_ID)
        return AIC_SUCCESS;

    /* Get packet type. */
    nx_packet_data_extract_offset(packet_ptr, 0, &packet_type, 1, &bytes_copied);

    /* Right shift 4 bits to get the packet type. */
    packet_type = packet_type >> 4;
    //alog_debug("mobile MQTT cb receive packet type is %d\n", packet_type);

    if (packet_type == MQTT_CONTROL_PACKET_TYPE_PINGRESP) {
        if (m_mqtt_paras[index].pingack) {
            snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN,
                     MQTT_URC_RESP"\"pingresp\",%d,%d", index,
                     MOBILE_MQTT_PING_SUCCESS);

            aic_transmit_at_cmd_urc(m_mqtt_paras[index].urc_port, res_str,
                                    strlen(res_str));
        }
    }

    return AIC_SUCCESS;
}

/* bind the source address through cid */
static u32 mobile_mqtt_bind_source_ip(int connect_id, ulong wait_option)
{
    u32 status;

    status = aic_tcp_client_source_ip_bind(
             &(m_mqtt_paras[connect_id].client_ptr->mqtt_client->nxd_mqtt_client_socket),
             m_mqtt_paras[connect_id].cid, AF_INET, wait_option);

    if (status)
        status = aic_tcp_client_source_ip_bind(
                 &(m_mqtt_paras[connect_id].client_ptr->mqtt_client->nxd_mqtt_client_socket),
                  m_mqtt_paras[connect_id].cid, AF_INET6, wait_option);

    return status;
}

static void mobile_mqtt_message_get(int connect_id, u32 mid, u32 msg_len)
{
    u32 status = 0;
    u32 cache_index;
    u8 urc_buf[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};
    u8 *urc_topic = AIC_NULL;
    u8 *urc_msg = AIC_NULL;
    u32 urc_topic_len = 0;
    u32 urc_msg_len = 0;
    u8 *res = AIC_NULL;
    u32 offset_len = 0;
    u8 cached_mode = 0;
    u8 out_format = 0;
    u32 total_zalloc_size = 0;

    if (m_mqtt_paras[connect_id].handle == 0) {
        aicx_error(0x1a5ce1, "", "no MQTT Client\n");
        return;
    }

    cached_mode = m_mqtt_paras[connect_id].cached_mode;
    out_format = m_mqtt_paras[connect_id].out_format;

    /* out message by com */
    if (cached_mode == MOBILE_MSG_OUT_BY_COM) {
        urc_msg = aos_zalloc_relaxed(MOBILE_MQTT_MSG_LEN);
        if (!urc_msg)
            goto ZALLOC_FAIL;

        urc_topic = aos_zalloc(MOBILE_MQTT_TOPIC_LEN + 1);

        status = aic_mqtt_client_message_get(m_mqtt_paras[connect_id].handle,
                                             urc_topic, MOBILE_MQTT_TOPIC_LEN,
                                             &urc_topic_len, urc_msg,
                                             MOBILE_MQTT_MSG_LEN, &urc_msg_len);

        //alog_info("[debug] connect_id is %d\n", connect_id);
        //alog_info("[debug] urc_topic_len is %d\n", urc_topic_len);
        //alog_info("[debug] urc_msg_len is %d\n", urc_msg_len);
        //alog_info("[debug] urc_msg is %s\n", urc_msg);
        //alog_info("[debug] urc_topic is %s\n", urc_topic);

        if (status) {
            aicx_error(0x0fd4c1, "x", "m_mqtt pub msg get fail(0x%x)\n", status);
        } else {
            if (out_format == MOBILE_MQTT_HEX) {
                total_zalloc_size = urc_msg_len * 2 + urc_topic_len * 2 +
                                    MOBILE_MQTT_URC_HEADER_LEN;
                res = aos_zalloc_relaxed(total_zalloc_size);
                if (!res)
                    goto ZALLOC_FAIL;

                status =  mobile_mem_alloc(&m_mqtt_paras[connect_id].urc_msg,
                                           urc_msg_len * 2 + 64);
                if (status)
                    goto ZALLOC_FAIL;

                aic_print_data_to_hex(urc_msg, urc_msg_len, m_mqtt_paras[connect_id].urc_msg);

                if (strlen(m_mqtt_paras[connect_id].urc_msg)) {
                    snprintf(res, total_zalloc_size,
                             MQTT_URC_RESP"\"publish\",%d,%d,\"%s\",%d,%d,%s", connect_id, mid,
                             urc_topic, urc_msg_len, urc_msg_len, m_mqtt_paras[connect_id].urc_msg);
                } else {
                    snprintf(res, MOBILE_MQTT_URC_HEADER_LEN,
                             MQTT_URC_RESP"\"publish\",%d,%d,\"%s\",%d,%d,", connect_id, mid,
                             urc_topic, urc_msg_len, urc_msg_len);
                }

                aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port, res,
                                        strlen(res));
                mobile_mqtt_urc_msg_mem_release(connect_id);
            } else if (out_format == MOBILE_MQTT_ASCII) {
                total_zalloc_size = urc_msg_len + urc_topic_len + MOBILE_MQTT_URC_HEADER_LEN;
                res = aos_zalloc_relaxed(total_zalloc_size);
                if (!res)
                    goto ZALLOC_FAIL;

                snprintf(res, total_zalloc_size,
                         MQTT_URC_RESP"\"publish\",%d,%d,\"%s\",%d,%d,", connect_id, mid,
                         urc_topic, urc_msg_len, urc_msg_len);

                offset_len = strlen(res);

                aos_memcpy(res + offset_len, urc_msg, urc_msg_len);

                aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port,
                                        res, offset_len + urc_msg_len);
            }
        }

        if (urc_topic)
            aos_free(urc_topic);
        if (urc_msg)
            aos_free(urc_msg);
        if (res)
            aos_free(res);

        urc_topic = AIC_NULL;
        urc_msg = AIC_NULL;
        res = AIC_NULL;
    }

    if (cached_mode == MOBILE_MSG_STORED) {
        urc_msg = aos_zalloc_relaxed(MOBILE_MQTT_MSG_LEN);
        if (!urc_msg)
            goto ZALLOC_FAIL;

        urc_topic = aos_zalloc(MOBILE_MQTT_TOPIC_LEN + 1);

        cache_index = mobile_mqtt_idle_msg_index_get(&m_mqtt_paras[connect_id]);
        /* no memory, drop the receive message */
        if (cache_index == MOBILE_MQTT_STORAGE_MSG_NUM ||
            m_mqtt_paras[connect_id].total_cached_size >= MOBILE_MQTT_MAX_CACHED_DATA_SIZE) {
            status = aic_mqtt_client_message_get(m_mqtt_paras[connect_id].handle,
                                                 urc_topic, MOBILE_MQTT_TOPIC_LEN,
                                                 &urc_topic_len, urc_msg,
                                                 MOBILE_MQTT_MSG_LEN, &urc_msg_len);
            if (status) {
                aicx_error(0x166a28, "x", "m_mqtt msg get fail(0x%x)\n", status);
                goto FREE_OUT;
            } else {
                snprintf(urc_buf, MOBILE_MQTT_URC_SHORT_LEN,
                         MQTT_URC_RESP"\"drop\",%d,%d", connect_id, urc_msg_len);

                aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port,
                                        urc_buf, strlen(urc_buf));
                goto FREE_OUT;
            }
        } else {
            status = aic_mqtt_client_message_get(m_mqtt_paras[connect_id].handle,
                                                 urc_topic, MOBILE_MQTT_TOPIC_LEN,
                                                 &urc_topic_len, urc_msg,
                                                 MOBILE_MQTT_MSG_LEN, &urc_msg_len);
            if (status) {
                aicx_error(0x0fd4c1, "x", "m_mqtt pub msg get fail(0x%x)\n", status);
                goto FREE_OUT;
            } else {
                if (urc_msg_len >
                    (MOBILE_MQTT_MAX_CACHED_DATA_SIZE -
                    m_mqtt_paras[connect_id].total_cached_size)) {
                    snprintf(urc_buf, MOBILE_MQTT_URC_SHORT_LEN,
                             MQTT_URC_RESP"\"drop\",%d,%d", connect_id, urc_msg_len);

                    aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port,
                                            urc_buf, strlen(urc_buf));
                    goto FREE_OUT;
                }

                if (cache_index < MOBILE_MQTT_STORAGE_MSG_NUM)
                    mobile_mqtt_cached_msg_info_mem_alloc(connect_id, cache_index,
                    (urc_topic_len + urc_msg_len + 32));

                snprintf(m_mqtt_paras[connect_id].recv_msg_list[cache_index].store_msg,
                         (urc_topic_len + urc_msg_len + 32),
                         MQTT_READ_RESP"%d,%d,\"%s\",%d,%s", connect_id, mid,
                         urc_topic, urc_msg_len, urc_msg);

                m_mqtt_paras[connect_id].recv_msg_list[cache_index].store_status = 1;
                m_mqtt_paras[connect_id].total_msg_count += 1;

                m_mqtt_paras[connect_id].recv_msg_list[cache_index].store_msg_len += urc_topic_len;
                m_mqtt_paras[connect_id].recv_msg_list[cache_index].store_msg_len += urc_msg_len;
                m_mqtt_paras[connect_id].total_cached_size += urc_topic_len;
                m_mqtt_paras[connect_id].total_cached_size += urc_msg_len;

                snprintf(urc_buf, MOBILE_MQTT_URC_SHORT_LEN,
                         MQTT_URC_RESP"\"pubnmi\",%d,%d,%d", connect_id,
                         mid, (urc_msg_len + urc_topic_len));

                aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port, urc_buf,
                                        strlen(urc_buf));
            }
        }

FREE_OUT:
        if (urc_topic)
            aos_free(urc_topic);
        if (urc_msg)
            aos_free(urc_msg);
    }

    return;

ZALLOC_FAIL:
    aicx_error(0x038ba7, "", "m_mqtt msg mem zalloc fail\n");
    snprintf(urc_buf, MOBILE_MQTT_URC_SHORT_LEN, MQTT_URC_RESP"\"drop\",%d,%d",
             connect_id, urc_msg_len);

    aic_transmit_at_cmd_urc(m_mqtt_paras[connect_id].urc_port, urc_buf,
                            strlen(urc_buf));

    if (urc_topic)
        aos_free(urc_topic);
    if (urc_msg)
        aos_free(urc_msg);
    if (res)
        aos_free(res);

    return;
}

static void mobile_mqtt_connect_process(MOBILE_MQTT_QUEUE_MSG *mobile_mqtt_msg)
{
    static u32 wait_time = 0;
    static int reconn_times = -1;
    u32 status = AIC_SUCCESS;
    int index = MOBILE_MQTT_INITIAL_ID;
    u8 res_str[MOBILE_MQTT_URC_SHORT_LEN];
    u32 net_err = MOBILE_MQTT_NETWORK_ERROR;

    index = mobile_mqtt_msg->connect_id;
    if (reconn_times == -1)
        reconn_times = m_mqtt_paras[index].reconn_times;
    if (!wait_time)
        wait_time = m_mqtt_paras[index].reconn_interval;

    if (m_mqtt_paras[index].handle == 0) {
        if (m_mqtt_paras[index].is_active_disconnect == 1) {
            snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN, MQTT_URC_RESP"\"conn\",%d,%d",
                    index, MOBILE_MQTT_DISCONNECTED_BY_USER);
            aic_transmit_at_cmd_urc(mobile_mqtt_msg->urc_port, res_str,
                                    strlen(res_str));
            reconn_times = -1;
            wait_time = 0;
            m_mqtt_paras[index].reconn_state = 0;
            m_mqtt_paras[index].is_active_disconnect = 0;
            mobile_mqtt_reconnect_timer_stop(index);
        } else {
            snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN, MQTT_URC_RESP"\"conn\",%d,%d",
                     index, MOBILE_MQTT_DISCONNECTED_UNKNOW_ERROR);
            aic_transmit_at_cmd_urc(mobile_mqtt_msg->urc_port, res_str,
                                    strlen(res_str));
        }
        return;
    }

#if (CONFIG_NET_MQTTS) && (CONFIG_CM_SSL_AT || CONFIG_GX_SSL_AT)
    /* mqtts */
    if (m_mqtt_paras[index].ssl_enable) {
        if (m_mqtt_paras[index].client_ptr)
            m_mqtt_paras[index].client_ptr->mqtt_client->auth_type =
                msslconf_list[m_mqtt_paras[index].ssl_id].seclevel;

        if (msslconf_list[m_mqtt_paras[index].ssl_id].cacert &&
            m_mqtt_paras[index].client_ptr->mqtt_client->auth_type != 0) {
            status = aic_mqtt_ca_crt_add(m_mqtt_paras[index].handle,
                     msslconf_list[m_mqtt_paras[index].ssl_id].cacert,
                     (u16)msslconf_list[m_mqtt_paras[index].ssl_id].cacert_len);
            if (status)
                goto CONN_ERROR;
        }

        if (m_mqtt_paras[index].cid > 1 &&
            m_mqtt_paras[index].cid <= ANET_MAX_PDP_CID) {
            status = mobile_mqtt_bind_source_ip(index,
                                                MOBILE_MQTT_DEFAULT_TIMEOUT);
            if (status && reconn_times > 0) {
                goto MQTT_RETRY;
            } else if (status && reconn_times == 0) {
                goto CONN_ERROR;
            }
        }
        status = mobile_mqtts_connect(m_mqtt_paras[index].handle,
                    m_mqtt_paras[index].cid,
                    m_mqtt_paras[index].connect_info.host,
                    m_mqtt_paras[index].connect_info.server_port,
                    m_mqtt_paras[index].keepalive_time,
                    m_mqtt_paras[index].clean_session,
                    MOBILE_MQTT_DEFAULT_TIMEOUT);
    }
#endif

    /* mqtt */
    if (!m_mqtt_paras[index].ssl_enable) {
        if (m_mqtt_paras[index].cid > 1 &&
            m_mqtt_paras[index].cid <= ANET_MAX_PDP_CID) {
            status = mobile_mqtt_bind_source_ip(index,
                                                MOBILE_MQTT_DEFAULT_TIMEOUT);
            if (status && reconn_times > 0) {
                goto MQTT_RETRY;
            } else if (status && reconn_times == 0) {
                goto CONN_ERROR;
            }
        }
        status = mobile_mqtt_connect(m_mqtt_paras[index].handle,
                    m_mqtt_paras[index].cid,
                    m_mqtt_paras[index].connect_info.host,
                    m_mqtt_paras[index].connect_info.server_port,
                    m_mqtt_paras[index].keepalive_time,
                    m_mqtt_paras[index].clean_session,
                    MOBILE_MQTT_DEFAULT_TIMEOUT);
    }

    if (status) {
        aicx_error(0x0ae610, "x", "m_mqtt connect fail(0x%x)\n", status);

        if (status == NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD)
            goto SERVER_ERROR;
        if(status == NXD_MQTT_SERVER_MESSAGE_FAILURE)
            goto CONN_ERROR;
        if (status == NXD_MQTT_ERROR_IDENTIFYIER_REJECTED) {
            net_err = MOBILE_MQTT_CONNECT_REJECT;
            goto CONN_ERROR;
        }

MQTT_RETRY:
        if(reconn_times != 0 && m_mqtt_paras[index].is_active_disconnect == 0) {
            snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN,
                     MQTT_URC_RESP"\"conn\",%d,%d",
                     index, MOBILE_MQTT_RECONNECTING);
            m_mqtt_paras[index].reconn_state = 1;
            aic_transmit_at_cmd_urc(mobile_mqtt_msg->urc_port, res_str,
                                    strlen(res_str));
            reconn_times --;
            mobile_mqtt_reconnect_timer_start(index, wait_time * 1000);
            if (m_mqtt_paras[index].reconn_mode)
                wait_time += m_mqtt_paras[index].reconn_interval;
            return;
        } else if (m_mqtt_paras[index].is_active_disconnect == 1) {
            snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN,
                     MQTT_URC_RESP"\"conn\",%d,%d", index,
                     MOBILE_MQTT_DISCONNECTED_BY_USER);
            reconn_times = -1;
            wait_time = 0;
            m_mqtt_paras[index].reconn_state = 0;
            m_mqtt_paras[index].is_active_disconnect = 0;
            mobile_mqtt_reconnect_timer_stop(index);
            return;
        }
    }

    if (status)
        goto CONN_ERROR;

    reconn_times = -1;
    wait_time = 0;
    m_mqtt_paras[index].reconn_state = 0;
    mobile_mqtt_reconnect_timer_stop(index);
    m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_client_publish_notify =
        mobile_mqtt_publish_cb;

    /* Set the receive notify function to get pub message. and release packet which is not sub*/
    m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_client_receive_notify_ext =
                                                    mobile_mqtt_message_receive_cb;

    return;

CONN_ERROR:
    if (status) {
        snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN,
                 MQTT_URC_RESP"\"conn\",%d,%d", index, net_err);

        aic_transmit_at_cmd_urc(mobile_mqtt_msg->urc_port, res_str,
                                strlen(res_str));
    }

SERVER_ERROR:
    reconn_times = -1;
    wait_time = 0;
    m_mqtt_paras[index].reconn_state = 0;
    mobile_mqtt_reconnect_timer_stop(index);

    if (m_mqtt_paras[index].client_ptr) {
#ifdef CONFIG_NET_MQTTS
        if (m_mqtt_paras[index].ssl_enable ||
            m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_client_use_tls)
            aic_mqtt_client_ca_crt_remove(m_mqtt_paras[index].handle);
#endif

        aic_mqtt_client_delete(m_mqtt_paras[index].handle);
        m_mqtt_paras[index].handle = 0;
        m_mqtt_paras[index].client_ptr = NULL;
    }
    return;
}

static void mobile_mqtt_disconnect_process(MOBILE_MQTT_QUEUE_MSG *mobile_mqtt_msg)
{
    int index = MOBILE_MQTT_INITIAL_ID;

    index = mobile_mqtt_msg->connect_id;

    if (m_mqtt_paras[index].handle != 0) {
        aic_mqtt_client_disconnect(m_mqtt_paras[index].handle);
        mobile_mqtt_paras_mem_release(index);

        if (m_mqtt_paras[index].client_ptr) {
#ifdef CONFIG_NET_MQTTS
            if (m_mqtt_paras[index].ssl_enable ||
                m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_client_use_tls)
                aic_mqtt_client_ca_crt_remove(m_mqtt_paras[index].handle);
#endif

            aic_mqtt_client_delete(m_mqtt_paras[index].handle);
            m_mqtt_paras[index].handle = 0;
            m_mqtt_paras[index].client_ptr = NULL;
            moble_mqtt_paras_reset(index);
        }
    }

    return;
}

static void mobile_mqtt_disconnect_by_server_process(MOBILE_MQTT_QUEUE_MSG *mobile_mqtt_msg)
{
    int index = MOBILE_MQTT_INITIAL_ID;
    u8 res_str[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};

    index = mobile_mqtt_msg->connect_id;

    snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN, MQTT_URC_RESP"\"conn\",%d,%d",
             index, MOBILE_MQTT_DISCONNECTED_BY_SERVER);

    if (strlen(res_str))
        aic_transmit_at_cmd_urc(mobile_mqtt_msg->urc_port, res_str,
                                strlen(res_str));

    mobile_mqtt_paras_mem_release(index);
    m_mqtt_paras[index].total_cached_size = 0;
    m_mqtt_paras[index].total_msg_count = 0;
    m_mqtt_paras[index].urc_port = 0;

    if (m_mqtt_paras[index].handle != 0) {
#ifdef CONFIG_NET_MQTTS
        if (m_mqtt_paras[index].ssl_enable ||
            m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_client_use_tls)
            aic_mqtt_client_ca_crt_remove(m_mqtt_paras[index].handle);
#endif

        aic_mqtt_client_delete(m_mqtt_paras[index].handle);
        m_mqtt_paras[index].handle = 0;
        m_mqtt_paras[index].client_ptr = NULL;
    }

    return;
}

void mobile_mqtt_msg_process_entry(void *thread_input)
{
    u32 ret;
    MOBILE_MQTT_QUEUE_MSG mobile_mqtt_msg;

    while(1) {
        ret = aos_recv_from_queue(m_mqtt_q, &mobile_mqtt_msg,
                                  AOS_WAIT_FOREVER);

        if(AOS_QUEUE_EMPTY == ret) {
            continue;
        } else {
            if (mobile_mqtt_msg.msg_type == MOBILE_MQTT_MESSAGE_DATA_RECEIVE)
                mobile_mqtt_message_get(mobile_mqtt_msg.connect_id,
                                        mobile_mqtt_msg.mid,
                                        mobile_mqtt_msg.remaining_length);

            if (mobile_mqtt_msg.msg_type == MOBILE_MQTT_CONNECT)
                mobile_mqtt_connect_process(&mobile_mqtt_msg);

            if (mobile_mqtt_msg.msg_type == MOBILE_MQTT_DISCONNECT)
                mobile_mqtt_disconnect_process(&mobile_mqtt_msg);

            if (mobile_mqtt_msg.msg_type == MOBILE_MQTT_DISCONNECT_BY_SERVER)
                mobile_mqtt_disconnect_by_server_process(&mobile_mqtt_msg);
        }
    }
}

static u32 mobile_mqtt_paras_value_transmit (AT_REQ_RESP *at_ptr, int connect_id,
                                             u8 *tag)
{
    u8 print_all = 0;
    u8 *res_buf;
    u32 offset_len = 0;

    if (strncasecmp(tag, "query", 5) == 0)
        print_all = 1;

    if (print_all || strncasecmp(tag, "version", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "version",
                                   m_mqtt_paras[connect_id].version,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "cid", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "cid",
                                   m_mqtt_paras[connect_id].cid,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE);

#ifdef CONFIG_NET_MQTTS
    if (print_all || strncasecmp(tag, "ssl", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "ssl",
                                   m_mqtt_paras[connect_id].ssl_enable,
                                   m_mqtt_paras[connect_id].ssl_id,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE);
#endif

    if (print_all || strncasecmp(tag, "keepalive", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "keepalive",
                                   m_mqtt_paras[connect_id].keepalive_time,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "clean", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "clean",
                                   m_mqtt_paras[connect_id].clean_session,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "retrans", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "retrans",
                                   m_mqtt_paras[connect_id].retrans_interval,
                                   m_mqtt_paras[connect_id].retry_times,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "willoption", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "willoption",
                                   m_mqtt_paras[connect_id].will_info.will_flag,
                                   m_mqtt_paras[connect_id].will_info.will_qos,
                                   m_mqtt_paras[connect_id].will_info.will_retain,
                                   MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "willpayload", strlen(tag)) == 0) {
        if (!m_mqtt_paras[connect_id].will_info.will_topic &&
            !m_mqtt_paras[connect_id].will_info.will_msg)
            transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "willpayload",
                                   ",", AIC_NULL, AIC_NULL, AIC_NULL);
        else {
            res_buf = aos_zalloc(1024);
            snprintf(res_buf, 1024, "+%s: \"willpayload\",\"%s\",\"",
                     MQTT_CFG_STR, m_mqtt_paras[connect_id].will_info.will_topic);

            offset_len = strlen(res_buf);

            aos_memcpy(res_buf + offset_len,
                       m_mqtt_paras[connect_id].will_info.will_msg,
                       m_mqtt_paras[connect_id].will_info.will_msg_len);

            offset_len += m_mqtt_paras[connect_id].will_info.will_msg_len;
            aos_memcpy(res_buf + offset_len, "\"", 1);

            aic_transmit_at_cmd_urc(at_ptr->serial_port_id, res_buf,
                                    offset_len + 1);
            aos_free(res_buf);
        }
    }

    if (print_all || strncasecmp(tag, "pingreq", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "pingreq",
                                   m_mqtt_paras[connect_id].ping_interval,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE, MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "pingresp", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "pingresp",
                                   m_mqtt_paras[connect_id].pingack,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE, MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "encoding", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "encoding",
                                   m_mqtt_paras[connect_id].input_format,
                                   m_mqtt_paras[connect_id].out_format,
                                   MOBILE_RESERVED_VALUE, MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "cached", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "cached",
                                   m_mqtt_paras[connect_id].cached_mode,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE, MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "reconn", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, MQTT_CFG_STR, "reconn",
                                   m_mqtt_paras[connect_id].reconn_times,
                                   m_mqtt_paras[connect_id].reconn_interval,
                                   m_mqtt_paras[connect_id].reconn_mode,
                                   MOBILE_RESERVED_VALUE);

    return AIC_SUCCESS;
}

static u32 mobile_mqtt_sub_info_query(AT_REQ_RESP *at_ptr, int connect_id)
{
    u8 *query_res;
    int i;

    query_res = aos_zalloc(MOBILE_MQTT_TOPIC_LEN + 32);

    for (i = 0; i < MOBILE_MQTT_MAX_TOPIC_NUM; i++) {
        if (m_mqtt_paras[connect_id].sub_info[i].sub_topic) {
            aos_memset(query_res, '\0', MOBILE_MQTT_TOPIC_LEN + 32);
            snprintf(query_res, MOBILE_MQTT_TOPIC_LEN + 32, MQTT_SUB_RESP"\"%s\",%d",
                     m_mqtt_paras[connect_id].sub_info[i].sub_topic,
                     m_mqtt_paras[connect_id].sub_info[i].sub_qos);

            transmit_mobile_resp_str(at_ptr, query_res);
        }
    }

    aic_reply_ok_resp(at_ptr);
    aos_free(query_res);
    return AIC_SUCCESS;
}

static u8 mobile_mqtt_idle_sub_id_find(int connect_id)
{
    u32 i;

    for (i = 0; i < MOBILE_MQTT_MAX_TOPIC_NUM; i++) {
        if (!m_mqtt_paras[connect_id].sub_info[i].sub_topic)
            break;
    }

    return i;
}

static u8 mobile_mqtt_sub_topic_is_same_check(int connect_id, u8 *sub_topic,
                                              u8 *topic_id)
{
    int i;
    for (i = 0; i < MOBILE_MQTT_MAX_TOPIC_NUM; i++) {
        if (m_mqtt_paras[connect_id].sub_info[i].sub_topic &&
            strncmp(sub_topic,  m_mqtt_paras[connect_id].sub_info[i].sub_topic,
                    strlen(sub_topic)) == 0) {
            *topic_id = i;
            //alog_debug("mobile mqtt sub topic is same\n");
            return 1;
        }
    }

    return 0;
}

static u32 mobile_mqtt_loop_subscribe(int connect_id, u8 *topic, u8 qos,
                                      u16 *packet_id, int retry_times,
                                      u8 wait_time)
{
    u32 status;
    u8 idle_sub_id;
    u8 same_topic_id = 0;

    idle_sub_id = mobile_mqtt_idle_sub_id_find(connect_id);
    if (idle_sub_id == MOBILE_MQTT_MAX_TOPIC_NUM)
        return idle_sub_id;

    if (mobile_mqtt_sub_topic_is_same_check(connect_id, topic, &same_topic_id)) {
        m_mqtt_paras[connect_id].sub_info[same_topic_id].sub_qos = qos;
        goto OUT;
    }

    status =  mobile_mem_alloc(&m_mqtt_paras[connect_id].sub_info[idle_sub_id].sub_topic,
                               MOBILE_MQTT_TOPIC_LEN);
    if (status)
        return status;

    strncpy(m_mqtt_paras[connect_id].sub_info[idle_sub_id].sub_topic, topic,
            MOBILE_MQTT_TOPIC_LEN);

    m_mqtt_paras[connect_id].sub_info[idle_sub_id].sub_qos = qos;

OUT:
    do {
        status = aic_mqtt_client_subscribe_ext(m_mqtt_paras[connect_id].handle,
                                               topic,qos, packet_id);
        if (status) {
            retry_times --;
            aos_sleep(wait_time * 1000);
            wait_time += wait_time;
        }
    } while(status && retry_times >= 0);

    return status;
}

static u32 mobile_mqtt_connect_info_check(AT_REQ_RESP *at_ptr, int connect_id,
                                          u8 conn_status_check)
{
    if (connect_id < 0 || connect_id > MOBILE_MQTT_MAX_CLIENT_ID) {
        aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
        return MOBILE_MQTT_INVALID_PARAMETER;
    }

    if (conn_status_check) {
        if (m_mqtt_paras[connect_id].handle == 0) {
            aic_reply_error_resp(at_ptr, MOBILE_MQTT_NOT_CONN_OR_CONN_ERROR);
            return MOBILE_MQTT_NOT_CONN_OR_CONN_ERROR;
        }
    }

    return AIC_SUCCESS;
}

static void mobile_mqtt_sub_status_check(AT_REQ_RESP *at_ptr, u32 status,
                                         int connect_id, u16 packet_id)
{
    u8 res[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};

    if (!status) {
        snprintf(res, MOBILE_MQTT_URC_SHORT_LEN, MQTT_SUB_RESP"%d,%d", connect_id,
                 packet_id);
        transmit_mobile_resp_str(at_ptr, res);
    } else if (status) {
        snprintf(res, MOBILE_MQTT_URC_SHORT_LEN, MQTT_URC_RESP"\"timeout\",%d,%d",
                 connect_id, packet_id);
        transmit_mobile_resp_str(at_ptr, res);
    }

    return;
}

/* Define mqtt paras maps. */
static MOBILE_MQTT_AT_PARAS_MAP mobile_mqtt_paras_map[] =
{
    {"ssl",          MOBILE_MQTT_AT_SSL,          3},
    {"cid",          MOBILE_MQTT_AT_CID,          3},
    {"clean",        MOBILE_MQTT_AT_CLEAN,        5},
    {"cached",       MOBILE_MQTT_AT_CACHED,       6},
    {"reconn",       MOBILE_MQTT_AT_RECCON,       6},
    {"version",      MOBILE_MQTT_AT_VERSION,      7},
    {"retrans",      MOBILE_MQTT_AT_RETRANS,      7},
    {"pingreq",      MOBILE_MQTT_AT_PINGREQ,      7},
    {"pingresp",     MOBILE_MQTT_AT_PINGRESP,     8},
    {"encoding",     MOBILE_MQTT_AT_ENCODING,     8},
    {"keepalive",    MOBILE_MQTT_AT_KEEPALIVE,    9},
    {"willoption",   MOBILE_MQTT_AT_WILLOPTION,  10},
    {"willpayload",  MOBILE_MQTT_AT_WILLPAYLOAD, 11},
};

static u32 MOBILE_MQTT_AT_PARAS_MAP_SIZE = sizeof(mobile_mqtt_paras_map) /
                                           sizeof(MOBILE_MQTT_AT_PARAS_MAP);

static u32 mobile_mqtt_paras_set(AT_REQ_RESP *at_ptr)
{
    u8 *tag;
    ulong *paras;
    u16 count;
    int connect_id;
    u8 i;
    u8 para_code = 0;
    u32 status = AIC_SUCCESS;
    u32 escape_data_len = 0;
    u8 *temp = AIC_NULL;
    MOBILE_MQTT_AT_PARAS_TEMP temp_paras = {0};

    if (mobile_specific_cmd_check(at_ptr->at_req))
        paras = mobile_split_paras_to_str_array(at_ptr->at_req, MQTT_CFG_CMD"=",
                                                &count, 4, 3, 1);
    else
        paras = aic_split_paras_to_str_array(at_ptr->at_req, MQTT_CFG_CMD"=",
                                             &count);

    if (!paras || count < 2)
        goto ERROR_OUT;

    tag = (u8*)paras[0];
    connect_id = atoi((u8*)paras[1]);

    /* Determine which para is neet to set. */
    for (i = 0; i < MOBILE_MQTT_AT_PARAS_MAP_SIZE; i++)
    {
        if (strncasecmp(tag, mobile_mqtt_paras_map[i].mobile_mqtt_at_paras_str,
            mobile_mqtt_paras_map[i].paras_len) == 0)
        {
            para_code = mobile_mqtt_paras_map[i].mobile_mqtt_at_paras_code;
            break;
        }
    }

    if (connect_id > MOBILE_MQTT_MAX_CLIENT_ID)
        goto ERROR_OUT;

    m_mqtt_paras[connect_id].connect_id = connect_id;
    switch(para_code) {
#ifdef CONFIG_NET_MQTTS
    case MOBILE_MQTT_AT_SSL:
        if (count >= 3) {
            if (*(u8 *)paras[2] == 0)
                temp_paras.ssl_enable = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                    goto ERROR_OUT;

                temp_paras.ssl_enable = atoi((u8*)paras[2]);
            }
        }

        if (count == 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.ssl_id = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 0, MOBILE_MQTT_MAX_SSL_ID))
                    goto ERROR_OUT;

                temp_paras.ssl_id = atoi((u8*)paras[3]);
            }
        }

        if (count > 4)
            goto ERROR_OUT;

        if (count >= 3) {
            m_mqtt_paras[connect_id].ssl_enable = temp_paras.ssl_enable;
            m_mqtt_paras[connect_id].ssl_id = 0;
        }

        if (count == 4)
            m_mqtt_paras[connect_id].ssl_id = temp_paras.ssl_id;

        break;
#endif
    case MOBILE_MQTT_AT_CID:
        if (count == 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 1, ANET_MAX_PDP_CID))
                goto ERROR_OUT;

            m_mqtt_paras[connect_id].cid = atoi((u8*)paras[2]);
        } else {
            goto ERROR_OUT;
        }
        break;
    case MOBILE_MQTT_AT_ENCODING:
        if (count >= 3) {
            if (*(u8 *)paras[2] == 0)
                temp_paras.input_format = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[2], 0, 2))
                    goto ERROR_OUT;

                temp_paras.input_format = atoi((u8 *)paras[2]);
            }
        }

        if (count == 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.out_format = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 0, 1))
                    goto ERROR_OUT;

                temp_paras.out_format = atoi((u8 *)paras[3]);
            }
        }

        if (count > 4)
            goto ERROR_OUT;

        if (count >= 3) {
            m_mqtt_paras[connect_id].input_format = temp_paras.input_format;
            m_mqtt_paras[connect_id].out_format = 0;
        }
        if (count >= 4)
            m_mqtt_paras[connect_id].out_format = temp_paras.out_format;

        break;
    case MOBILE_MQTT_AT_CACHED:
        if (count == 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                goto ERROR_OUT;

            m_mqtt_paras[connect_id].cached_mode = atoi((u8 *)paras[2]);
        } else
            goto ERROR_OUT;
        break;
    case MOBILE_MQTT_AT_VERSION:
        if (count == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras[2], strlen((u8 *)paras[2])))
                goto ERROR_OUT;

            if (atoi((u8 *)paras[2]) != 4)
                goto ERROR_OUT;

            m_mqtt_paras[connect_id].version = atoi((u8 *)paras[2]);
        } else {
            goto ERROR_OUT;
        }
        break;
    case MOBILE_MQTT_AT_RETRANS:
        if (count >= 3) {
            if (*(u8 *)paras[2] == 0)
                temp_paras.retrans_interval = MOBILE_MQTT_DEFAULT_RETRAN_INTERVAL;
            else {
                if (mobile_at_para_range_check((u8 *)paras[2], 20, 60))
                    goto ERROR_OUT;

                temp_paras.retrans_interval = atoi((u8 *)paras[2]);
            }
        }

        if (count == 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.retry_times = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 0, 3))
                    goto ERROR_OUT;

                temp_paras.retry_times = atoi((u8 *)paras[3]);
            }
        }

        if (count > 4)
            goto ERROR_OUT;

        if (count >= 3) {
            m_mqtt_paras[connect_id].retrans_interval = temp_paras.retrans_interval;
            m_mqtt_paras[connect_id].retry_times = 0;
        }

        if (count >= 4)
            m_mqtt_paras[connect_id].retry_times = temp_paras.retry_times;

        break;
    case MOBILE_MQTT_AT_PINGREQ:
        if (count == 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 60, 86400))
                goto ERROR_OUT;

            m_mqtt_paras[connect_id].ping_interval = atoi((u8 *)paras[2]);
        } else
            goto ERROR_OUT;
        break;
    case MOBILE_MQTT_AT_PINGRESP:
        if (count == 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                goto ERROR_OUT;

            m_mqtt_paras[connect_id].pingack = atoi((u8 *)paras[2]);
        } else
            goto ERROR_OUT;
        break;
    case MOBILE_MQTT_AT_KEEPALIVE:
        if (count == 3) {
            if (!aic_is_valid_integer_para((u8 *)paras[2], strlen((u8 *)paras[2])))
                goto ERROR_OUT;

            if (atoi((u8 *)paras[2]) < 0 || atoi((u8 *)paras[2]) > 65535 ||
                (atoi((u8 *)paras[2]) < 60 && atoi((u8 *)paras[2]) > 0))
                goto ERROR_OUT;

            m_mqtt_paras[connect_id].keepalive_time = atoi((u8 *)paras[2]);
        } else
            goto ERROR_OUT;
        break;
    case MOBILE_MQTT_AT_CLEAN:
        if (count == 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                goto ERROR_OUT;

            if (atoi((u8 *)paras[2]) != 0 && atoi((u8 *)paras[2]) != 1)
                goto ERROR_OUT;

            m_mqtt_paras[connect_id].clean_session = atoi((u8 *)paras[2]);
        } else
            goto ERROR_OUT;
        break;
    case MOBILE_MQTT_AT_RECCON:
        if (count >= 3) {
            if (*(u8 *)paras[2] == 0)
                temp_paras.reconn_times = MOBILE_MQTT_DEFAULT_RECOONE_TIMES;
            else {
                if (mobile_at_para_range_check((u8 *)paras[2], 0, MOBILE_MQTT_DEFAULT_RECOONE_TIMES))
                    goto ERROR_OUT;

                temp_paras.reconn_times = atoi((u8 *)paras[2]);
            }
        }

        if (count >= 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.reconn_interval = MOBILE_MQTT_DEFAULT_RECONN_INTERVAL;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 20, 60))
                    goto ERROR_OUT;

                temp_paras.reconn_interval = atoi((u8 *)paras[3]);
            }
        }

        if (count == 5) {
            if (*(u8 *)paras[4] == 0)
                temp_paras.reconn_mode = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[4], 0, 1))
                    goto ERROR_OUT;

                temp_paras.reconn_mode = atoi((u8 *)paras[4]);
            }
        }

        if (count > 5)
            goto ERROR_OUT;

        if (count >= 3) {
            m_mqtt_paras[connect_id].reconn_times = temp_paras.reconn_times;
            m_mqtt_paras[connect_id].reconn_interval = MOBILE_MQTT_DEFAULT_RECONN_INTERVAL;
            m_mqtt_paras[connect_id].reconn_mode = 0;
        }

        if (count >= 4)
            m_mqtt_paras[connect_id].reconn_interval = temp_paras.reconn_interval;
        if (count >= 5)
            m_mqtt_paras[connect_id].reconn_mode = temp_paras.reconn_mode;

        break;
    case MOBILE_MQTT_AT_WILLOPTION:
        if (count >= 3) {
            if (*(u8 *)paras[2] == 0)
                temp_paras.will_flag = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                    goto ERROR_OUT;

                temp_paras.will_flag = atoi((u8 *)paras[2]);
            }
        }

        if (count >= 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.will_qos = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 0, 2))
                    goto ERROR_OUT;

                temp_paras.will_qos = atoi((u8 *)paras[3]);
            }
        }

        if (count == 5) {
            if (*(u8 *)paras[4] == 0)
                temp_paras.will_retain = 0;
            else {
                if (mobile_at_para_range_check((u8 *)paras[4], 0, 1))
                    goto ERROR_OUT;

                temp_paras.will_retain = atoi((u8 *)paras[4]);
            }
        }

        if (count > 5)
            goto ERROR_OUT;

        if (count >= 3) {
            m_mqtt_paras[connect_id].will_info.will_flag = temp_paras.will_flag;
            m_mqtt_paras[connect_id].will_info.will_qos = 0;
            m_mqtt_paras[connect_id].will_info.will_retain = 0;
        }

        if (count >= 4)
            m_mqtt_paras[connect_id].will_info.will_qos = temp_paras.will_qos;
        if (count >= 5)
            m_mqtt_paras[connect_id].will_info.will_retain = temp_paras.will_retain;

        break;
    case MOBILE_MQTT_AT_WILLPAYLOAD:
        if (count == 4) {
            if (strlen((u8*)paras[2]) > MOBILE_MQTT_WILL_TOPIC_LEN ||
                strlen((u8*)paras[3]) > MOBILE_MQTT_WILL_MSG_LEN)
                goto ERROR_OUT;

            if (m_mqtt_paras[connect_id].input_format == MOBILE_MQTT_HEX) {
                if ((strlen((u8*)paras[3]) & 1) != 0) {
                    goto ERROR_OUT;
                }

                temp = (u8*)paras[3];
                for (int i = 0; i < strlen((u8*)paras[3]); i++) {
                    if (!mobile_is_hex_check(temp[i]))
                        goto ERROR_OUT;
                }
            }

            status =  mobile_mem_alloc(&m_mqtt_paras[connect_id].will_info.will_topic,
                                       MOBILE_MQTT_WILL_TOPIC_LEN);
            if (status)
                goto ERROR_OUT;

            status =  mobile_mem_alloc(&m_mqtt_paras[connect_id].will_info.will_msg,
                                       MOBILE_MQTT_WILL_MSG_LEN);
            if (status)
                goto ERROR_OUT;

            m_mqtt_paras[connect_id].will_info.will_topic_len = strlen((u8*)paras[2]);
            m_mqtt_paras[connect_id].will_info.will_msg_len = strlen((u8*)paras[3]);

            if (m_mqtt_paras[connect_id].input_format == MOBILE_MQTT_HEX) {
                m_mqtt_paras[connect_id].will_info.will_msg_len =
                   strlen((u8*)paras[3]) >> 1;
            }

            strncpy(m_mqtt_paras[connect_id].will_info.will_topic,
                    (u8 *)paras[2], MOBILE_MQTT_WILL_TOPIC_LEN);

            status = mobile_mqtt_input_data_format_process(connect_id,
                     m_mqtt_paras[connect_id].will_info.will_msg,
                     (u8 *)paras[3], m_mqtt_paras[connect_id].will_info.will_msg_len,
                     &escape_data_len);
            if (status)
                goto ERROR_OUT;

            if (m_mqtt_paras[connect_id].input_format == MOBILE_MQTT_ESCAPE)
                 m_mqtt_paras[connect_id].will_info.will_msg_len = escape_data_len;
        } else
            goto ERROR_OUT;

        break;
    default:
        goto ERROR_OUT;
        break;
    }

    aic_free_paras_mem(paras, count);
    return status;

ERROR_OUT:
    status = 1;
    aic_free_paras_mem(paras, count);
    return status;
}

static void mobile_mqtt_unsub_status_check(AT_REQ_RESP *at_ptr, u32 status,
                                           int connect_id, u8 info_id)
{
    u8 res[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};

    if (!status) {
        snprintf(res, MOBILE_MQTT_URC_SHORT_LEN, MQTT_UNSUB_RESP"%d,%d", connect_id,
                 m_mqtt_paras[connect_id].unsub_info[info_id].packet_id);
        transmit_mobile_resp_str(at_ptr, res);
        aic_reply_ok_resp(at_ptr);
        return;
    }

    if (status == NXD_MQTT_COMMUNICATION_FAILURE) {
        snprintf(res, MOBILE_MQTT_URC_SHORT_LEN,
                 MQTT_URC_RESP"\"timeout\",%d,%d", connect_id,
                 m_mqtt_paras[connect_id].unsub_info[info_id].packet_id);
        transmit_mobile_resp_str(at_ptr, res);
        return;
    }

    if (status && status!= NXD_MQTT_COMMUNICATION_FAILURE)
        aic_reply_error_resp(at_ptr, MOBILE_MQTT_STATUS_ERROR);

    return;
}

static void mobile_mqtt_pub_msg_set(AT_REQ_RESP *at_ptr, int index)
{
    u32 temp_size = 0;
    u32 last_char = 0;

    /* end by ctrl+Z */
    if (m_mqtt_paras[index].pub_info.pub_msg &&
        m_mqtt_paras[index].pub_info.end_mob == 1) {
        temp_size = at_ptr->upstream_data_len;

        if (m_mqtt_paras[index].pub_info.actual_pub_len + temp_size >
            MOBILE_MQTT_MSG_LEN) {
            temp_size = MOBILE_MQTT_MSG_LEN -
                        m_mqtt_paras[index].pub_info.actual_pub_len;
        }
        if (temp_size != 0) {
            aos_memcpy(m_mqtt_paras[index].pub_info.pub_msg +
                       m_mqtt_paras[index].pub_info.actual_pub_len,
                       at_ptr->at_upstream_data, temp_size);
            m_mqtt_paras[index].pub_info.actual_pub_len += temp_size;
            m_mqtt_paras[index].pub_info.pub_msg_len =
                m_mqtt_paras[index].pub_info.actual_pub_len;
        }

        last_char = m_mqtt_paras[index].pub_info.actual_pub_len - 1;

        if (m_mqtt_paras[index].pub_info.pub_msg[last_char] == MOBILE_MQTT_ESC) {
            /* release pub msg when receive ESC */
            m_mqtt_paras[index].pub_info.ready = 0;
            m_mqtt_paras[index].pub_info.actual_pub_len = 0;
            aic_reply_ok_resp(at_ptr);
            at_ptr->at_mode = AT_CMD_MODE;
        }

        if (m_mqtt_paras[index].pub_info.pub_msg[last_char] == MOBILE_MQTT_CTRL_Z) {
            m_mqtt_paras[index].pub_info.pub_msg[last_char] = '\0';
            at_ptr->at_mode = AT_CMD_MODE;
            m_mqtt_paras[index].pub_info.ready = 1;
            m_mqtt_paras[index].pub_info.pub_msg_len -= 1;
            //alog_debug("mobile mqtt pub can be send\n");
        }

        /* length greater than the max pub len, do need to wait the end flag */
        if (m_mqtt_paras[index].pub_info.actual_pub_len == MOBILE_MQTT_MSG_LEN) {
            at_ptr->at_mode = AT_CMD_MODE;
            m_mqtt_paras[index].pub_info.ready = 1;
        }
    }

    /* set mqtt pub msg by length */
    if (m_mqtt_paras[index].pub_info.pub_msg &&
        m_mqtt_paras[index].pub_info.end_mob == 0 &&
        m_mqtt_paras[index].pub_info.pub_msg_len > 0) {

        temp_size = at_ptr->upstream_data_len;

        if (m_mqtt_paras[index].pub_info.actual_pub_len + temp_size >
            m_mqtt_paras[index].pub_info.pub_msg_len) {
            temp_size = m_mqtt_paras[index].pub_info.pub_msg_len -
                        m_mqtt_paras[index].pub_info.actual_pub_len;
        }
        if (temp_size != 0) {
            aos_memcpy(m_mqtt_paras[index].pub_info.pub_msg +
                       m_mqtt_paras[index].pub_info.actual_pub_len,
                       at_ptr->at_upstream_data, temp_size);
            m_mqtt_paras[index].pub_info.actual_pub_len += temp_size;
        }

        if (m_mqtt_paras[index].pub_info.actual_pub_len >=
            m_mqtt_paras[index].pub_info.pub_msg_len) {
            at_ptr->at_mode = AT_CMD_MODE;
            m_mqtt_paras[index].pub_info.ready = 1;
        }
    }

    return;
}

static void mobile_mqtt_sub_topic_remove(int connect_id, u8 *unsub_topic)
{
    int i;
    for (i = 0; i < MOBILE_MQTT_MAX_TOPIC_NUM; i++) {
        if (m_mqtt_paras[connect_id].sub_info[i].sub_topic &&
            strncmp(unsub_topic,  m_mqtt_paras[connect_id].sub_info[i].sub_topic,
                    strlen(unsub_topic)) == 0) {

            //alog_debug("mobile mqtt remove unsub topic\n");
            aos_free(m_mqtt_paras[connect_id].sub_info[i].sub_topic);
            m_mqtt_paras[connect_id].sub_info[i].sub_topic = AIC_NULL;
            m_mqtt_paras[connect_id].sub_info[i].sub_topic_len = 0;
            break;
        }
    }

    return;
}

u32 mobile_mqtt_paras_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u8 *tag;
    u16 count;
    int index = MOBILE_MQTT_INITIAL_ID;
    u32 status = 0;

    if (mobile_specific_cmd_check(at_ptr->at_req))
        paras = mobile_split_paras_to_str_array(at_ptr->at_req, MQTT_CFG_CMD"=",
                                                &count, 4, 3, 1);
    else
        paras = aic_split_paras_to_str_array(at_ptr->at_req, MQTT_CFG_CMD"=",
                                             &count);

    if (!paras || count < 2)
        goto ERROR_OUT;

    tag = (u8*)paras[0];
    if (mobile_at_para_range_check((u8 *)paras[1], 0, MOBILE_MQTT_MAX_CLIENT_ID))
        goto ERROR_OUT;

    index = atoi((u8*)paras[1]);
    if (count == 2) {
        mobile_mqtt_paras_value_transmit(at_ptr, index, tag);
        goto SUCCESS_OUT;
    }

    /* set para */
    if (count > 2) {
        status = mobile_mqtt_paras_set(at_ptr);
        if (status)
            goto ERROR_OUT;
    }

SUCCESS_OUT:
    aic_reply_ok_resp(at_ptr);
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
    aic_free_paras_mem(paras, count);
    return MOBILE_MQTT_INVALID_PARAMETER;

}

u32 mobile_mqtt_paras_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "version", "(0-5)", "(4)", AIC_NULL,
                           AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "cid", "(0-5)", "(1-5)", AIC_NULL,
                           AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "ssl", "(0-5)", "(0-1)", "(0-5)",
                           AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "keepalive", "(0-5)", "(0,60-65535)",
                           AIC_NULL, AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "clean", "(0-5)", "(0-1)", AIC_NULL,
                           AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "retrans", "(0-5)", "(20-60)",
                           "(0-3)", AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "willoption", "(0-5)", "(0-1)",
                           "(0-2)", "(0-1)");

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "willpayload", "(0-5),,",
                           AIC_NULL, AIC_NULL, AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "pingreq", "(0-5)", "(60-86400)",
                           AIC_NULL, AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "pingresp", "(0-5)", "(0-1)",
                           AIC_NULL, AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "encoding", "(0-5)", "(0-2)",
                           "(0-1)", AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "cached", "(0-5)", "(0-1)",
                           AIC_NULL, AIC_NULL);

    transmit_mobile_at_str(at_ptr, MQTT_CFG_STR, "reconn", "(0-5)", "(0-3)",
                           "(20-60)", "(0-1)");

    aic_reply_ok_resp(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Start MQTT service, such as creating MQTT Client.
 */
u32 mobile_mqtt_connect_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    u16 count = 0;
    ulong *paras = AIC_NULL;
    u32 status = 0;
    u8 client_name[24] = {'\0'};
    int index = MOBILE_MQTT_INITIAL_ID;
    u8 *client_id = NULL;
    MOBILE_MQTT_QUEUE_MSG mobile_mqtt_msg;

    paras = aic_split_paras_to_str_array(at_ptr->at_req, MQTT_CONNECT_CMD"=",
                                         &count);
    if (!paras || count < 2 || count > 6)
        goto PARA_ERROR;

    if (paras && count > 1) {
        if (!aic_is_valid_integer_para((u8 *)paras[0], strlen((u8 *)paras[0])))
            goto PARA_ERROR;

        index = atoi((u8*)paras[0]);
        status = mobile_mqtt_connect_info_check(at_ptr, index, MOBILE_MQTT_NO_CONN_STATUS);
        if (status) {
            aic_free_paras_mem(paras, count);
            return status;
        }

        if (m_mqtt_paras[index].handle != 0) {
            aic_free_paras_mem(paras, count);
            aic_reply_error_resp(at_ptr, MOBILE_MQTT_IS_CONNECTED);
            return MOBILE_MQTT_IS_CONNECTED;
        }

        status =  mobile_mem_alloc(&m_mqtt_paras[index].connect_info.host,
                                   MOBILE_MQTT_HOST_LEN);
        if (status)
            goto PARA_ERROR;

        strncpy(m_mqtt_paras[index].connect_info.host, (u8*)paras[1],
                MOBILE_MQTT_HOST_LEN);
    }

    if (paras && count > 2) {
        if (strlen((u8 *)paras[2]) == 0) {
            /* if port is omitted, set default value s*/
            if (m_mqtt_paras[index].ssl_enable)
                m_mqtt_paras[index].connect_info.server_port = NXD_MQTT_TLS_PORT;
            else
                m_mqtt_paras[index].connect_info.server_port = NXD_MQTT_PORT;
        } else if (mobile_at_para_range_check((u8 *)paras[2], 0, 65535)) {
            goto PARA_ERROR;
        } else {
            m_mqtt_paras[index].connect_info.server_port = atoi((u8*)paras[2]);
        }
    }

    if (paras && count > 3) {
        if (strlen((u8*)paras[3]) > MOBILE_MQTT_CLIENT_ID_LEN)
            goto PARA_ERROR;

        status =  mobile_mem_alloc(&m_mqtt_paras[index].connect_info.client_id,
                                   MOBILE_MQTT_CLIENT_ID_LEN);
        if (status)
            goto PARA_ERROR;

        strncpy(m_mqtt_paras[index].connect_info.client_id, (u8*)paras[3],
                MOBILE_MQTT_CLIENT_ID_LEN);
        client_id = m_mqtt_paras[index].connect_info.client_id;
    }

    if (paras && count > 4) {
        if (strlen((u8*)paras[4]) > MOBILE_MQTT_USERNAME_LEN)
            goto PARA_ERROR;

        status =  mobile_mem_alloc(&m_mqtt_paras[index].connect_info.user,
                                   MOBILE_MQTT_USERNAME_LEN);
        if (status)
            goto PARA_ERROR;

        strncpy(m_mqtt_paras[index].connect_info.user, (u8*)paras[4],
                MOBILE_MQTT_USERNAME_LEN);
    }

    if (paras && count > 5) {
        if (strlen((u8*)paras[5]) > MOBILE_MQTT_PASSWORD_LEN)
            goto PARA_ERROR;

        status =  mobile_mem_alloc(&m_mqtt_paras[index].connect_info.passwd,
                                   MOBILE_MQTT_PASSWORD_LEN);
        if (status)
            goto PARA_ERROR;

        strncpy(m_mqtt_paras[index].connect_info.passwd, (u8*)paras[5],
                MOBILE_MQTT_PASSWORD_LEN);
    }

    snprintf(client_name, 24, "m_mqtt_c_%d", index);

    if (m_mqtt_paras[index].ssl_enable)
        m_mqtt_paras[index].handle = aic_mqtt_client_create_ext(client_name, client_id,
                                                                AIC_MQTTS_CLIENT_TYPE);
    else
        m_mqtt_paras[index].handle = aic_mqtt_client_create_ext(client_name, client_id,
                                                                AIC_MQTT_CLIENT_TYPE);

    if (m_mqtt_paras[index].handle == 0) {
        aic_reply_error_resp(at_ptr, MOBILE_MQTT_NOT_CONN_OR_CONN_ERROR);
        aic_free_paras_mem(paras, count);
        mobile_mqtt_connect_info_mem_release(index);
        return MOBILE_MQTT_NOT_CONN_OR_CONN_ERROR;
    }

    m_mqtt_paras[index].client_ptr = (AIC_MQTT_CLIENT *)m_mqtt_paras[index].handle;
    m_mqtt_paras[index].connect_id = index;
    m_mqtt_paras[index].client_ptr->connect_id = index;

#ifdef CONFIG_NET_MQTTS
    if (m_mqtt_paras[index].ssl_enable)
        m_mqtt_paras[index].client_ptr->mqtt_client->mqtt_mb_cfg->wait_time =
                MOBILE_MQTT_DEFAULT_TIMEOUT;
#endif

    m_mqtt_paras[index].urc_port = at_ptr->serial_port_id;

    m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_connect_notify =
        mobile_mqtt_connect_cb;

    m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_packet_receive_notify =
        mobile_mqtt_packet_receive_cb;

    aic_mqtt_client_ping_interval_set(m_mqtt_paras[index].handle,
                                      m_mqtt_paras[index].ping_interval);

    m_mqtt_paras[index].client_ptr->cid = m_mqtt_paras[index].cid;

    if (count == 6) { /* count == 6 means user/pwd/clientID should be exist */
        status = aic_mqtt_client_login_set(m_mqtt_paras[index].handle,
                 m_mqtt_paras[index].connect_info.user,
                 m_mqtt_paras[index].connect_info.passwd);
        if (status)
            goto CONN_ERROR;
    }

    /* Set the ack receive notify function. */
    status = aic_mqtt_client_ack_receive_notify_set(m_mqtt_paras[index].handle,
                                                    mobile_mqtt_ack_receive_cb);
    if (status) {
        goto CONN_ERROR;
    }

    if (m_mqtt_paras[index].will_info.will_flag &&
        m_mqtt_paras[index].will_info.will_msg &&
        m_mqtt_paras[index].will_info.will_topic) {
        status = aic_mqtt_client_will_message_set(m_mqtt_paras[index].handle,
                 m_mqtt_paras[index].will_info.will_topic,
                 strlen(m_mqtt_paras[index].will_info.will_topic),
                 m_mqtt_paras[index].will_info.will_msg,
                 m_mqtt_paras[index].will_info.will_msg_len,
                 m_mqtt_paras[index].will_info.will_retain,
                 m_mqtt_paras[index].will_info.will_qos);
        if (status)
            goto CONN_ERROR;
    }

    aic_mqtt_client_disconnect_notify_set(m_mqtt_paras[index].handle,
                                          mobile_mqtt_disconnect_cb);

    aic_reply_ok_resp(at_ptr);
    mobile_mqtt_msg.connect_id = index;
    mobile_mqtt_msg.msg_type = MOBILE_MQTT_CONNECT;
    mobile_mqtt_msg.urc_port = at_ptr->serial_port_id;
    aos_send_to_queue(m_mqtt_q, &mobile_mqtt_msg, MOBILE_MQTT_DEFAULT_TIMEOUT);

    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

PARA_ERROR:
    if (index >= 0 && index < MOBILE_MQTT_MAX_CLIENT_MUN)
        mobile_mqtt_connect_info_mem_release(index);
    aic_free_paras_mem(paras, count);
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
    return MOBILE_MQTT_INVALID_PARAMETER;

CONN_ERROR:
    aic_mqtt_client_delete(m_mqtt_paras[index].handle);
    mobile_mqtt_connect_info_mem_release(index);
    m_mqtt_paras[index].handle = 0;
    m_mqtt_paras[index].client_ptr = AIC_NULL;
    aic_free_paras_mem(paras, count);
    return MOBILE_MQTT_NOT_CONN_OR_CONN_ERROR;
}

u32 mobile_mqtt_connect_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_at_str(at_ptr, MQTT_CONNECT_STR, "(0-5)", "",
                           "(0-65535)", "", ",");

    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

/**
 * Subscribe a topic from server.
 */
u32 mobile_mqtt_subscribe_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    u32 status = 0;
    u32 wait_time = 0;
    int index = MOBILE_MQTT_INITIAL_ID;
    int retry_times = 0;
    u16 packet_id = 0;

    paras = aic_split_paras_to_str_array(at_ptr->at_req, MQTT_SUB_CMD"=", &count);
    if (!paras || (count != 1 && (count < 3 || count > 7)))
        goto PARA_ERROR;

    if (count > 0) {
        if (!aic_is_valid_integer_para((u8 *)paras[0], strlen((u8 *)paras[0])))
            goto PARA_ERROR;

        index = atoi((u8*)paras[0]);
        status = mobile_mqtt_connect_info_check(at_ptr, index,
                                                MOBILE_MQTT_CONN_STATUS_CHECK);
        if (status) {
            aic_free_paras_mem(paras, count);
            return status;
        }
    }

    m_mqtt_paras[index].urc_port = at_ptr->serial_port_id;

    /* query topic */
    if (count == 1) {
        mobile_mqtt_sub_info_query(at_ptr, index);
        aic_free_paras_mem(paras, count);
        return AIC_SUCCESS;
    }

    if (count == 2 || count == 4 || count == 6)
        goto PARA_ERROR;

    if (count == 3) {
        if(mobile_at_para_range_check((u8 *)paras[2], 0, 2))
            goto PARA_ERROR;

        if (strlen((u8*)paras[1]) > MOBILE_MQTT_TOPIC_LEN)
            goto PARA_ERROR;
    }
    if (count == 5) {
        if(mobile_at_para_range_check((u8 *)paras[4], 0, 2))
            goto PARA_ERROR;

        if (strlen((u8*)paras[3]) > MOBILE_MQTT_TOPIC_LEN)
            goto PARA_ERROR;
    }
    if (count == 7) {
        if(mobile_at_para_range_check((u8 *)paras[6], 0, 2))
            goto PARA_ERROR;

        if (strlen((u8*)paras[5]) > MOBILE_MQTT_TOPIC_LEN)
            goto PARA_ERROR;
    }

    wait_time = m_mqtt_paras[index].retrans_interval;
    if (paras && count > 2) {
        retry_times = m_mqtt_paras[index].retry_times;
        status = mobile_mqtt_loop_subscribe(index, (u8*)paras[1], atoi((u8*)paras[2]),
                                            &packet_id, retry_times, wait_time);

        mobile_mqtt_sub_status_check(at_ptr, status, index, packet_id);
    }

    if (paras && count > 4) {
        retry_times = m_mqtt_paras[index].retry_times;
        status = mobile_mqtt_loop_subscribe(index, (u8*)paras[3], atoi((u8*)paras[4]),
                                            &packet_id, retry_times, wait_time);

        mobile_mqtt_sub_status_check(at_ptr, status, index, packet_id);
    }

    if (paras && count > 6) {
        retry_times = m_mqtt_paras[index].retry_times;
        status = mobile_mqtt_loop_subscribe(index, (u8*)paras[5], atoi((u8*)paras[6]),
                                            &packet_id, retry_times, wait_time);

        mobile_mqtt_sub_status_check(at_ptr, status, index, packet_id);
    }

    aic_reply_ok_resp(at_ptr);
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras, count);
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
    return MOBILE_MQTT_INVALID_PARAMETER;
}

/**
 * Unubscribe a topic from server.
 */
u32 mobile_mqtt_unsubscribe_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    u32 status = 0;
    u32 wait_time = 0;
    int index = MOBILE_MQTT_INITIAL_ID;
    int retry_times = 0;

    paras = aic_split_paras_to_str_array(at_ptr->at_req, MQTT_UNSUB_CMD"=",
                                         &count);

    if (!paras || count < 2 || count > 4)
        goto PARA_ERROR;

    if (paras && count > 1) {
        if (!aic_is_valid_integer_para((u8 *)paras[0], strlen((u8 *)paras[0])))
            goto PARA_ERROR;

        index = atoi((u8*)paras[0]);
        status = mobile_mqtt_connect_info_check(at_ptr, index,
                                                MOBILE_MQTT_CONN_STATUS_CHECK);
        if (status) {
            aic_free_paras_mem(paras, count);
            return status;
        }
        m_mqtt_paras[index].urc_port = at_ptr->serial_port_id;

        wait_time = m_mqtt_paras[index].retrans_interval;
        retry_times = m_mqtt_paras[index].retry_times;
        mobile_mqtt_sub_topic_remove(index, (u8 *)paras[1]);
        do {
            status = aic_mqtt_client_unsubscribe_ext(m_mqtt_paras[index].handle,
                     (u8 *)paras[1], &m_mqtt_paras[index].unsub_info[0].packet_id);

            if (status) {
                retry_times --;
                aos_sleep(wait_time * 1000);
                wait_time += wait_time;
            }
        } while (status && retry_times >= 0);

        mobile_mqtt_unsub_status_check(at_ptr, status, index, 0);
    }

    if (paras && count >= 3) {
        retry_times = m_mqtt_paras[index].retry_times;
        mobile_mqtt_sub_topic_remove(index, (u8 *)paras[2]);
        do {
            status = aic_mqtt_client_unsubscribe_ext(m_mqtt_paras[index].handle,
                     (u8 *)paras[2], &m_mqtt_paras[index].unsub_info[1].packet_id);

            if (status) {
                retry_times --;
                aos_sleep(wait_time * 1000);
                wait_time += wait_time;
            }
        } while(status && retry_times >= 0);

       mobile_mqtt_unsub_status_check(at_ptr, status, index, 1);
    }

    if (paras && count == 4) {
        retry_times = m_mqtt_paras[index].retry_times;
        mobile_mqtt_sub_topic_remove(index, (u8 *)paras[3]);
        do {
            status = aic_mqtt_client_unsubscribe_ext(m_mqtt_paras[index].handle,
                     (u8 *)paras[3], &m_mqtt_paras[index].unsub_info[2].packet_id);

            if (status) {
                retry_times --;
                aos_sleep(wait_time * 1000);
                wait_time += wait_time;
            }
        } while(status && retry_times >= 0);

        mobile_mqtt_unsub_status_check(at_ptr, status, index, 2);
    }

    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras, count);
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
    return MOBILE_MQTT_INVALID_PARAMETER;
}

static u32 mobile_mqtt_loop_publish(NXD_MQTT_CLIENT *client_ptr, u8 *topic_name,
                                    u32 topic_name_length, u8 *message,
                                    u32 message_length, u32 retain, u32 QoS,
                                    ulong timeout, int connect_id, int retry_times,
                                    u32 wait_time)
{
    u32 status = 0;

    do {
        status = nxd_mqtt_client_publish(client_ptr, topic_name, topic_name_length,
                                         message, message_length, retain, QoS,
                                         timeout);

        if (status) {
            m_mqtt_paras[connect_id].pub_info.dup_flag = 1;
            retry_times --;
            aos_sleep(wait_time * 1000);
            wait_time += wait_time;
        }
    } while(status && retry_times >= 0);

    return status;
}

/**
 * Publish a message to server.
 */
u32 mobile_mqtt_publish_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras = AIC_NULL;
    u16 count = 0;
    u32 status = 0;
    u8 res[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};
    u32 wait_time = 0;
    static int index = MOBILE_MQTT_INITIAL_ID;
    u8 *pub_msg = AIC_NULL;
    int retry_times = 0;
    u32 escape_data_len = 0;
    int mid = 0;

    /* AT Mode */
    if (at_ptr->at_mode == AT_CMD_MODE) {
        paras = mobile_split_paras_to_str_array(at_ptr->at_req, MQTT_PUB_CMD"=",
                                                &count, 7, 7, 0);

        if (!paras || count < 6 || count > 7)
            goto PARA_ERROR;

        if (paras && count > 0) {
            if (!aic_is_valid_integer_para((u8 *)paras[0], strlen((u8 *)paras[0]))) {
                goto PARA_ERROR;
            }

            index = atoi((u8*)paras[0]);
            status = mobile_mqtt_connect_info_check(at_ptr, index,
                                                    MOBILE_MQTT_CONN_STATUS_CHECK);
            if (status) {
                aic_free_paras_mem(paras, count);
                at_ptr->at_mode = AT_CMD_MODE;
                return status;
            }
        }

        if (paras && count > 5) {
            status = mobile_mqtt_connect_info_check(at_ptr, index,
                                                 MOBILE_MQTT_CONN_STATUS_CHECK);
            if (status) {
                goto OUT;
            }

            status =  mobile_mem_alloc(&m_mqtt_paras[index].pub_info.pub_topic,
                                       MOBILE_MQTT_TOPIC_LEN);
            if (status)
                goto PARA_ERROR;

            strncpy(m_mqtt_paras[index].pub_info.pub_topic, (u8 *)paras[1],
                    MOBILE_MQTT_TOPIC_LEN);

            if(mobile_at_para_range_check((u8 *)paras[2], 0, 2))
                goto PARA_ERROR;

            if(mobile_at_para_range_check((u8 *)paras[3], 0, 1))
                goto PARA_ERROR;

            if(mobile_at_para_range_check((u8 *)paras[4], 0, 1))
                goto PARA_ERROR;

            m_mqtt_paras[index].pub_info.pub_qos = atoi((u8 *)paras[2]);
            m_mqtt_paras[index].pub_info.pub_retain = atoi((u8 *)paras[3]);
            m_mqtt_paras[index].pub_info.dup_flag = atoi((u8 *)paras[4]);
            m_mqtt_paras[index].pub_info.actual_pub_len = 0;

            m_mqtt_paras[index].urc_port = at_ptr->serial_port_id;
        }
        wait_time = m_mqtt_paras[index].retrans_interval;

        if (paras && count == 7) {
            if(mobile_at_para_range_check((u8 *)paras[5], 0, MOBILE_MQTT_MAX_PUB_MSG_LEN))
                goto PARA_ERROR;

            m_mqtt_paras[index].pub_info.pub_msg_len = atoi((u8 *)paras[5]);

            if (m_mqtt_paras[index].pub_info.pub_msg_len == 0) {
                m_mqtt_paras[index].pub_info.pub_msg_len = strlen((u8 *)paras[6]);

                if (m_mqtt_paras[index].input_format == MOBILE_MQTT_ASCII) {
                    if (m_mqtt_paras[index].pub_info.pub_msg_len > MOBILE_MQTT_MAX_PUB_MSG_LEN)
                        goto PARA_ERROR;
                }

                if (m_mqtt_paras[index].input_format == MOBILE_MQTT_HEX) {
                    if ((m_mqtt_paras[index].pub_info.pub_msg_len & 1) != 0)
                        goto PARA_ERROR;

                    m_mqtt_paras[index].pub_info.pub_msg_len =
                        m_mqtt_paras[index].pub_info.pub_msg_len >> 1;

                    if (m_mqtt_paras[index].pub_info.pub_msg_len >
                        MOBILE_MQTT_MAX_PUB_MSG_LEN)
                        goto PARA_ERROR;

                    if (m_mqtt_paras[index].pub_info.pub_msg_len !=
                        (strlen((u8 *)paras[6]) >> 1)) {
                        goto PARA_ERROR;
                    }
                } else {
                    if (m_mqtt_paras[index].pub_info.pub_msg_len !=
                        strlen((u8 *)paras[6])) {
                        goto PARA_ERROR;
                    }
                }
            } else {
                if (m_mqtt_paras[index].input_format != MOBILE_MQTT_HEX) {
                    if (m_mqtt_paras[index].pub_info.pub_msg_len !=
                        strlen((u8 *)paras[6])) {
                        goto PARA_ERROR;
                    }
                } else {
                    if (m_mqtt_paras[index].pub_info.pub_msg_len !=
                        (strlen((u8 *)paras[6]) >> 1) ||
                        (strlen((u8 *)paras[6]) & 1) != 0) {
                        goto PARA_ERROR;
                    }
                }
            }

            if ((u8 *)paras[6] && strlen((u8 *)paras[6]) == 0) {
                retry_times = m_mqtt_paras[index].retry_times;
                status = mobile_mqtt_loop_publish(m_mqtt_paras[index].client_ptr->mqtt_client,
                                                  m_mqtt_paras[index].pub_info.pub_topic,
                                                  strlen(m_mqtt_paras[index].pub_info.pub_topic),
                                                  "\0", 0, m_mqtt_paras[index].pub_info.pub_retain,
                                                  m_mqtt_paras[index].pub_info.pub_qos,
                                                  MOBILE_MQTT_DEFAULT_TIMEOUT, index,
                                                  retry_times, wait_time);

                goto STATUS_CHECK;
            }

            pub_msg = aos_zalloc_relaxed(2 * m_mqtt_paras[index].pub_info.pub_msg_len);
            if (!pub_msg)
                goto PUB_INFO_ERROR;

            status = mobile_mqtt_input_data_format_process(index, pub_msg,
                     (u8 *)paras[6], m_mqtt_paras[index].pub_info.pub_msg_len,
                     &escape_data_len);
            if (status) {
                if (pub_msg) {
                    aos_free(pub_msg);
                    pub_msg = NULL;
                }
                goto PARA_ERROR;
            }

            if (m_mqtt_paras[index].input_format == MOBILE_MQTT_ESCAPE)
                m_mqtt_paras[index].pub_info.pub_msg_len = escape_data_len;

            retry_times = m_mqtt_paras[index].retry_times;
            status = mobile_mqtt_loop_publish(m_mqtt_paras[index].client_ptr->mqtt_client,
                                              m_mqtt_paras[index].pub_info.pub_topic,
                                              strlen(m_mqtt_paras[index].pub_info.pub_topic),
                                              pub_msg, m_mqtt_paras[index].pub_info.pub_msg_len,
                                              m_mqtt_paras[index].pub_info.pub_retain,
                                              m_mqtt_paras[index].pub_info.pub_qos,
                                              MOBILE_MQTT_DEFAULT_TIMEOUT, index,
                                              retry_times, wait_time);

            if (pub_msg) {
                aos_free(pub_msg);
            }
            goto STATUS_CHECK;
        }

        if (paras && count == 6) {
            if(mobile_at_para_range_check((u8 *)paras[5], 0, MOBILE_MQTT_MSG_LEN))
                goto PARA_ERROR;

            m_mqtt_paras[index].pub_info.pub_msg_len = atoi((u8 *)paras[5]);

            if (m_mqtt_paras[index].pub_info.pub_msg) {
                aos_free(m_mqtt_paras[index].pub_info.pub_msg);
                m_mqtt_paras[index].pub_info.pub_msg = AIC_NULL;
            }

            if (m_mqtt_paras[index].pub_info.pub_msg_len == 0) {
                m_mqtt_paras[index].pub_info.pub_msg = aos_zalloc_relaxed(MOBILE_MQTT_MSG_LEN);
                if (!m_mqtt_paras[index].pub_info.pub_msg)
                    goto PUB_INFO_ERROR;

                m_mqtt_paras[index].pub_info.end_mob = 1;
            } else if (m_mqtt_paras[index].pub_info.pub_msg_len > 0) {
                m_mqtt_paras[index].pub_info.end_mob = 0;
                m_mqtt_paras[index].pub_info.pub_msg =
                       aos_zalloc_relaxed(m_mqtt_paras[index].pub_info.pub_msg_len + 1);

                if (!m_mqtt_paras[index].pub_info.pub_msg)
                    goto PUB_INFO_ERROR;
            }

            at_ptr->at_mode = AT_DATA_MODE;
            transmit_mobile_resp_str(at_ptr, ">");
            aic_set_data_mode_expire_time(MQTT_PUB_CMD, 10000);

            goto OUT;
        }
    } else if (at_ptr->at_mode == AT_DATA_MODE) {
        /* data mode */
        if (at_ptr->end_data_mode) {
            m_mqtt_paras[index].pub_info.ready = 1;
            at_ptr->at_mode = AT_CMD_MODE;
            goto PUB_SEND;
        }

        mobile_mqtt_pub_msg_set(at_ptr, index);
    }

PUB_SEND:
    if (m_mqtt_paras[index].pub_info.ready == 1 &&
        m_mqtt_paras[index].pub_info.pub_msg) {

        if (strlen(m_mqtt_paras[index].pub_info.pub_msg) == 0) {
            aic_reply_ok_resp(at_ptr);
            mobile_mqtt_pub_info_mem_release(index);
            goto OUT;
        }

        if (m_mqtt_paras[index].pub_info.actual_pub_len <
            m_mqtt_paras[index].pub_info.pub_msg_len)
            m_mqtt_paras[index].pub_info.pub_msg_len =
            m_mqtt_paras[index].pub_info.actual_pub_len;

        retry_times = m_mqtt_paras[index].retry_times;
        status = mobile_mqtt_loop_publish(m_mqtt_paras[index].client_ptr->mqtt_client,
                                          m_mqtt_paras[index].pub_info.pub_topic,
                                          strlen(m_mqtt_paras[index].pub_info.pub_topic),
                                          m_mqtt_paras[index].pub_info.pub_msg,
                                          m_mqtt_paras[index].pub_info.pub_msg_len,
                                          m_mqtt_paras[index].pub_info.pub_retain,
                                          m_mqtt_paras[index].pub_info.pub_qos,
                                          MOBILE_MQTT_DEFAULT_TIMEOUT, index,
                                          retry_times, wait_time);

STATUS_CHECK:
        m_mqtt_paras[index].pub_info.actual_pub_len = 0;
        mobile_mqtt_pub_info_mem_release(index);
        if (status) {
            mid = 0;
            snprintf(res, MOBILE_MQTT_URC_SHORT_LEN,
                     MQTT_URC_RESP"\"timeout\",%d,%d", index, mid);
            transmit_mobile_resp_str(at_ptr, res);
        }
    }

OUT:
    m_mqtt_paras[index].pub_info.ready = 0;
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

PUB_INFO_ERROR:
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_STORAGE_ERROR);
    mobile_mqtt_pub_info_mem_release(index);
    aic_free_paras_mem(paras, count);
    return MOBILE_MQTT_STORAGE_ERROR;

PARA_ERROR:
    at_ptr->at_mode = AT_CMD_MODE;
    if (index >= 0 && index < MOBILE_MQTT_MAX_CLIENT_MUN)
        mobile_mqtt_pub_info_mem_release(index);
    aic_free_paras_mem(paras, count);
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
    return MOBILE_MQTT_INVALID_PARAMETER;
}

static void mobile_mqtt_store_msg_remove(int connect_id, u8 store_msg_id)
{
    if (m_mqtt_paras[connect_id].recv_msg_list[store_msg_id].store_msg) {
        aos_free(m_mqtt_paras[connect_id].recv_msg_list[store_msg_id].store_msg);
        m_mqtt_paras[connect_id].recv_msg_list[store_msg_id].store_msg = AIC_NULL;
        m_mqtt_paras[connect_id].recv_msg_list[store_msg_id].store_status = 0;
    }

    return;
}

/* Get received subscription information. */
u32 mobile_mqtt_message_read_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status;
    ulong *paras;
    u16 count;
    u8 res[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};
    int index = MOBILE_MQTT_INITIAL_ID;
    int to_read_count = 0;
    int has_read_count = 0;
    int i = 0;

    paras = aic_split_paras_to_str_array(at_ptr->at_req, MMQTT_READ_CMD"=",
                                         &count);
    if (!paras || count > 2)
        goto PARA_ERROR;

    if (paras && count > 0) {
        if (!aic_is_valid_integer_para((u8 *)paras[0], strlen((u8 *)paras[0]))) {
            goto PARA_ERROR;
        }

        index = atoi((u8*)paras[0]);
        status = mobile_mqtt_connect_info_check(at_ptr, index,
                                                MOBILE_MQTT_CONN_STATUS_CHECK);
        if (status) {
            aic_free_paras_mem(paras, count);
            return status;
        }
    }

    /* one para, AT querry */
    if (paras && count == 1) {
        if (m_mqtt_paras[index].total_msg_count == 0) {
            m_mqtt_paras[index].total_cached_size = 0;
        }

        snprintf(res, MOBILE_MQTT_URC_SHORT_LEN, MQTT_READ_RESP"%d,%d,%d", index,
                 m_mqtt_paras[index].total_msg_count,
                 m_mqtt_paras[index].total_cached_size);

        transmit_mobile_resp_str(at_ptr, res);
        aic_reply_ok_resp(at_ptr);
    }

    /* two paras, read msg data */
    if (paras && count == 2) {
        if (mobile_at_para_range_check((u8 *)paras[1], 1, m_mqtt_paras[index].total_msg_count))
            goto PARA_ERROR;

        to_read_count = atoi((u8*)paras[1]);
        if (m_mqtt_paras[index].total_msg_count > 0 &&
            m_mqtt_paras[index].total_cached_size > 0) {
            do {
                if (m_mqtt_paras[index].recv_msg_list[i].store_msg) {
                    aic_transmit_at_cmd_urc(at_ptr->serial_port_id,
                    m_mqtt_paras[index].recv_msg_list[i].store_msg,
                    strlen(m_mqtt_paras[index].recv_msg_list[i].store_msg));

                    mobile_mqtt_store_msg_remove(index, i);
                    has_read_count ++;

                    m_mqtt_paras[index].total_msg_count --;
                    m_mqtt_paras[index].total_cached_size -=
                        m_mqtt_paras[index].recv_msg_list[i].store_msg_len;
                    m_mqtt_paras[index].recv_msg_list[i].store_msg_len = 0;
                }

                i++;
            } while (has_read_count < to_read_count &&
                     i < MOBILE_MQTT_STORAGE_MSG_NUM);
        }

        aic_reply_ok_resp(at_ptr);
    }

    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras, count);
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
    return MOBILE_MQTT_INVALID_PARAMETER;
}

/**
 * Query the mqtt status.
 */
u32 mobile_mqtt_status_query_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    u8 res[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};
    int index = MOBILE_MQTT_INITIAL_ID;
    u8 mqtt_client_status;

    paras = aic_split_paras_to_str_array(at_ptr->at_req, MQTT_STATE_CMD"=",
                                         &count);
    if (!paras || count != 1)
        goto PARA_ERROR;

    if (paras && count == 1) {
        if (mobile_at_para_range_check((u8 *)paras[0], 0, MOBILE_MQTT_MAX_CLIENT_ID))
            goto PARA_ERROR;

        index = atoi((u8*)paras[0]);
        if (m_mqtt_paras[index].handle == 0) {
            snprintf(res, MOBILE_MQTT_URC_SHORT_LEN, MQTT_STATE_RESP"%d",
                     MOBILE_MQTT_CLIENT_DISCONNECTED);

            aic_transmit_at_result(at_ptr->serial_port_id, res, strlen(res));
            goto OUT;
        }

        if (m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_client_state ==
            NXD_MQTT_CLIENT_STATE_CONNECTING || m_mqtt_paras[index].reconn_state == 1)
            mqtt_client_status = MOBILE_MQTT_CLIENT_CONNECTING;
        else if (m_mqtt_paras[index].client_ptr->mqtt_client->nxd_mqtt_client_state ==
            NXD_MQTT_CLIENT_STATE_CONNECTED)
            mqtt_client_status = MOBILE_MQTT_CLIENT_CONNECTED;
        else
            mqtt_client_status = MOBILE_MQTT_CLIENT_DISCONNECTED;

        snprintf(res, MOBILE_MQTT_URC_SHORT_LEN, MQTT_STATE_RESP"%d",
                 mqtt_client_status);
        aic_transmit_at_result(at_ptr->serial_port_id, res, strlen(res));
    }

OUT:
    aic_reply_ok_resp(at_ptr);
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras, count);
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
    return MOBILE_MQTT_INVALID_PARAMETER;
}

/**
 * Exit the current mqtt session.
 */
u32 mobile_mqtt_disconnect_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count;
    int index = MOBILE_MQTT_INITIAL_ID;
    MOBILE_MQTT_QUEUE_MSG mobile_mqtt_msg = {'\0'};
    u8 res_str[MOBILE_MQTT_URC_SHORT_LEN] = {'\0'};

    paras = aic_split_paras_to_str_array(at_ptr->at_req, MMQTT_DISCONNECT_CMD"=",
                                         &count);

    if (!paras || count != 1) {
        goto ERROR_OUT;
    }

    if (paras && count == 1) {
        if (mobile_at_para_range_check((u8 *)paras[0], 0, MOBILE_MQTT_MAX_CLIENT_ID))
            goto ERROR_OUT;

        index = atoi((u8*)paras[0]);
    }

    if (m_mqtt_paras[index].handle == 0) {
        if (m_mqtt_paras[index].client_ptr) {
            //alog_info("mobile mqtt id[%d] maybe disconnect by server, delete client\n",index);

            aic_reply_ok_resp(at_ptr);
            aic_mqtt_client_delete((u32)m_mqtt_paras[index].client_ptr);
            m_mqtt_paras[index].client_ptr = NULL;
            mobile_mqtt_paras_mem_release(index);
            moble_mqtt_paras_reset(index);

            snprintf(res_str, MOBILE_MQTT_URC_SHORT_LEN,
                     MQTT_URC_RESP"\"conn\",%d,%d", index,
                     MOBILE_MQTT_DISCONNECTED_BY_USER);

            if (strlen(res_str))
                aic_transmit_at_cmd_urc(at_ptr->serial_port_id, res_str,
                                        strlen(res_str));

            goto SUCCESS_OUT;
        } else {
            aic_reply_error_resp(at_ptr, MOBILE_MQTT_NOT_CONN_OR_CONN_ERROR);
            aic_free_paras_mem(paras, count);
            return MOBILE_MQTT_NOT_CONN_OR_CONN_ERROR;
        }
    }

    m_mqtt_paras[index].is_active_disconnect = 1;
    mobile_mqtt_msg.connect_id = index;
    mobile_mqtt_msg.msg_type = MOBILE_MQTT_DISCONNECT;
    mobile_mqtt_msg.urc_port = at_ptr->serial_port_id;
    aic_reply_ok_resp(at_ptr);

    aos_send_to_queue(m_mqtt_q, &mobile_mqtt_msg, MOBILE_MQTT_DEFAULT_TIMEOUT);

    if (m_mqtt_paras[index].reconn_state == 1)
        mobile_mqtt_reconnect_timer_start(index, 5);

SUCCESS_OUT:
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_reply_error_resp(at_ptr, MOBILE_MQTT_INVALID_PARAMETER);
    aic_free_paras_mem(paras, count);
    return MOBILE_MQTT_INVALID_PARAMETER;
}

int mobile_mqtt_init(void)
{
    u32 i;

    m_mqtt_q = aos_create_queue("m_mqtt_q", sizeof(MOBILE_MQTT_QUEUE_MSG),
                                MOBILE_MQTT_QUEUE_NUM);
    AOS_ASSERT(m_mqtt_q);

    m_mqtt_msg_t = aos_create_thread("m_mqtt_event_process_t",
                                     mobile_mqtt_msg_process_entry,
                                     NULL, M_MQTT_TASK_SIZE, 0, NULL, 18,
                                     AOS_AUTO_START);

    AOS_ASSERT(m_mqtt_msg_t != AOS_INVALID_THREAD_ID);

    aic_add_at_cmd_handle(&m_mqtt_at_cmds[0]);

    for (i = 0; i < MOBILE_MQTT_MAX_CLIENT_MUN; i++) {
        moble_mqtt_default_paras_set(i);
    }

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(mobile_mqtt_init);

