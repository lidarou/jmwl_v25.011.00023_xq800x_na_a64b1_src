/******************************************************************************
 * MQTT(S) demo for ThreadX + NetX Duo.
 * Connect after PDP activation, publish periodically, and retry after failure.
 *****************************************************************************/
#include "my_common.h"
#include "my_mqtt.h"

#if CONFIG_USE_MQTTS
#include "ca_der.h"
#endif

static u32 g_mqtt_handle = 0;
static volatile bool g_mqtt_disconnected = FALSE;

static const char *demo_mqtt_status_name(u32 status)
{
    switch (status) {
    case NX_SUCCESS: return "success";
    case NXD_MQTT_NOT_CONNECTED: return "not connected";
    case NXD_MQTT_CONNECT_FAILURE: return "connect failure";
    case NXD_MQTT_COMMUNICATION_FAILURE: return "communication failure";
    case NXD_MQTT_NO_MESSAGE: return "no message";
    case NXD_MQTT_INSUFFICIENT_BUFFER_SPACE: return "receive buffer too small";
    case NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD: return "bad username/password";
    case NXD_MQTT_ERROR_NOT_AUTHORIZED: return "not authorized";
    default: return "SDK error; check status code";
    }
}

static u32 demo_mqtt_result(char *stage, u32 status)
{
    my_log_printf(1, "[MQTT] %s: %s, status=0x%X (%s)", stage,
                  status == NX_SUCCESS ? "OK" : "FAILED", status,
                  demo_mqtt_status_name(status));
    return status;
}

static void demo_mqtt_recv_notify(NXD_MQTT_CLIENT *client_ptr, u32 message_count)
{
    (void)client_ptr;
    my_log_printf(1, "[MQTT] RX notification: queued=%u", message_count);
}

static void demo_mqtt_disconnect_notify(NXD_MQTT_CLIENT *client_ptr)
{
    (void)client_ptr;
    g_mqtt_disconnected = TRUE;
    my_log_printf(1, "[MQTT] disconnected notification, PDP=%u", g_pdpState);
}

static void demo_mqtt_ack_notify(NXD_MQTT_CLIENT *client_ptr, u32 type,
                                 u16 packet_id, NX_PACKET *tx_pkt, void *context)
{
    (void)client_ptr;
    (void)tx_pkt;
    switch (type) {
    case MQTT_CONTROL_PACKET_TYPE_PUBACK:
        my_log_printf(1, "[MQTT] PUBACK: broker acknowledged publish, packet_id=%u",
                      packet_id);
        break;
    case MQTT_CONTROL_PACKET_TYPE_SUBACK:
        my_log_printf(1, "[MQTT] SUBACK: packet_id=%u, granted_qos=%u (%s)",
                      packet_id, (u32)context,
                      (u32)context <= 2 ? "accepted" : "rejected");
        break;
    default:
        my_log_printf(1, "[MQTT] ACK: type=%u, packet_id=%u", type, packet_id);
        break;
    }
}

/* Cleanup runs in the application task, never inside an SDK callback. */
static void demo_mqtt_cleanup(void)
{
    u32 status;

    if (g_mqtt_handle == 0)
        return;

    status = aic_mqtt_client_disconnect(g_mqtt_handle);
    demo_mqtt_result("disconnect", status);
    status = aic_mqtt_client_delete(g_mqtt_handle);
    demo_mqtt_result("delete client", status);
    g_mqtt_handle = 0;
}

static u32 demo_mqtt_connect(void)
{
    u32 status;

    g_mqtt_disconnected = FALSE;
    my_log_printf(1, "[MQTT] create client: id=%s, TLS=%u, PDP=%u",
                  DEMO_CLIENT_ID, CONFIG_USE_MQTTS, g_pdpState);
    g_mqtt_handle = aic_mqtt_client_create_ext((u8 *)DEMO_CLIENT_NAME,
                                               (char *)DEMO_CLIENT_ID,
                                               DEMO_CLIENT_TYPE);
    if (g_mqtt_handle == AIC_MQTT_CLIENT_CREATE_FAIL) {
        my_log_printf(1, "[MQTT] create client FAILED: SDK returned null handle");
        return NXD_MQTT_INTERNAL_ERROR;
    }
    my_log_printf(1, "[MQTT] client created: handle=0x%X", g_mqtt_handle);

    status = aic_mqtt_client_login_set(g_mqtt_handle,
                                       (char *)DEMO_USERNAME,
                                       (char *)DEMO_PASSWORD);
    if (demo_mqtt_result("configure login", status) != NX_SUCCESS)
        return status;

    status = aic_mqtt_client_receive_notify_set(g_mqtt_handle, demo_mqtt_recv_notify);
    if (demo_mqtt_result("register RX callback", status) != NX_SUCCESS)
        return status;
    status = aic_mqtt_client_disconnect_notify_set(g_mqtt_handle,
                                                   demo_mqtt_disconnect_notify);
    if (demo_mqtt_result("register disconnect callback", status) != NX_SUCCESS)
        return status;
    status = aic_mqtt_client_ack_receive_notify_set(g_mqtt_handle,
                                                    demo_mqtt_ack_notify);
    if (demo_mqtt_result("register ACK callback", status) != NX_SUCCESS)
        return status;

#if CONFIG_USE_MQTTS
    my_log_printf(1, "[MQTT] loading TLS CA: bytes=%u", ca_der_len);
    status = aic_mqtt_ca_crt_add(g_mqtt_handle, ca_der, ca_der_len);
    if (demo_mqtt_result("load TLS CA", status) != NX_SUCCESS)
        return status;
#endif

    my_log_printf(1, "[MQTT] connecting: host=%s port=%u TLS=%u keepalive=%us wait=%us",
                  DEMO_MQTT_HOST, DEMO_MQTT_PORT, CONFIG_USE_MQTTS,
                  DEMO_KEEP_ALIVE, (u32)(DEMO_WAIT / NX_IP_PERIODIC_RATE));
#if CONFIG_USE_MQTTS
    status = aic_mqtt_client_secure_connect(g_mqtt_handle, (u8 *)DEMO_MQTT_HOST,
                                             DEMO_MQTT_PORT, DEMO_KEEP_ALIVE,
                                             1, DEMO_WAIT);
#else
    status = aic_mqtt_client_connect(g_mqtt_handle, (u8 *)DEMO_MQTT_HOST,
                                      DEMO_MQTT_PORT, DEMO_KEEP_ALIVE,
                                      1, DEMO_WAIT);
#endif
    return demo_mqtt_result("broker connect", status);
}

u32 mqtt_demo_run(void)
{
    u32 status;
    u32 sequence = 0;
    u32 sent = 0;
    u32 failed = 0;
    u32 last_publish;
    u32 topic_len, msg_len;
    u32 received;
    u8 topic[128];
    u8 msg[256];
    const u32 interval_ticks = DEMO_PUBLISH_INTERVAL_SEC * NX_IP_PERIODIC_RATE;

    status = demo_mqtt_connect();
    if (status != NX_SUCCESS)
        return status;

    my_log_printf(1, "[MQTT] subscribing: topic=%s qos=%u", DEMO_SUB_TOPIC, DEMO_QOS);
    status = aic_mqtt_client_subscribe(g_mqtt_handle, DEMO_SUB_TOPIC, DEMO_QOS);
    if (demo_mqtt_result("subscribe request", status) != NX_SUCCESS)
        return status;

    my_log_printf(1, "[MQTT] periodic publish enabled: interval=%us topic=%s qos=%u",
                  DEMO_PUBLISH_INTERVAL_SEC, DEMO_PUB_TOPIC, DEMO_QOS);
    /* Unsigned subtraction also handles the ThreadX tick counter wrapping. */
    last_publish = (u32)tx_time_get() - interval_ticks;
    while (1) {
        if (g_pdpState != ANET_PDP_ACTIVATED || g_mqtt_disconnected) {
            my_log_printf(1, "[MQTT] session lost: PDP=%u disconnected=%u sent=%u failed=%u",
                          g_pdpState, g_mqtt_disconnected, sent, failed);
            return NXD_MQTT_NOT_CONNECTED;
        }

        if ((u32)((u32)tx_time_get() - last_publish) >= interval_ticks) {
            sequence++;
            my_log_printf(1, "[MQTT] TX #%u: topic=%s bytes=%u qos=%u",
                          sequence, DEMO_PUB_TOPIC, (u32)strlen(DEMO_PUB_MSG), DEMO_QOS);
            my_log_printf(1, "[MQTT] TX payload: %s", DEMO_PUB_MSG);
            status = aic_mqtt_client_publish(g_mqtt_handle, DEMO_PUB_TOPIC,
                                              DEMO_PUB_MSG, (u32)strlen(DEMO_PUB_MSG),
                                              0, DEMO_QOS, DEMO_WAIT);
            last_publish = (u32)tx_time_get();
            if (status != NX_SUCCESS) {
                failed++;
                my_log_printf(1, "[MQTT] TX #%u FAILED: status=0x%X (%s), sent=%u failed=%u",
                              sequence, status, demo_mqtt_status_name(status), sent, failed);
                return status;
            }
            sent++;
            my_log_printf(1, "[MQTT] TX #%u API OK: sent=%u; QoS1 delivery confirmed by PUBACK",
                          sequence, sent);
        }

        /* Drain a bounded batch so an RX burst cannot starve periodic TX. */
        for (received = 0; received < 8; received++) {
            topic_len = 0;
            msg_len = 0;
            status = aic_mqtt_client_message_get(g_mqtt_handle,
                                                  topic, sizeof(topic), &topic_len,
                                                  msg, sizeof(msg), &msg_len);
            if (status == NXD_MQTT_NO_MESSAGE)
                break;
            if (status != NX_SUCCESS) {
                demo_mqtt_result("receive message", status);
                if (status == NXD_MQTT_NOT_CONNECTED)
                    return status;
                break;
            }
            my_log_printf(1, "[MQTT] RX: topic=%.*s bytes=%u",
                          (int)topic_len, topic, msg_len);
            my_log_printf(1, "[MQTT] RX payload: %.*s", (int)msg_len, msg);
        }
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }
}

void my_mqtt_task(void *argv)
{
    u32 status;
    u32 attempt = 0;
    u32 waiting;

    (void)argv;
    my_log_printf(1, "[MQTT] task started: TLS=%u publish_interval=%us retry_delay=%us",
                  CONFIG_USE_MQTTS, DEMO_PUBLISH_INTERVAL_SEC, DEMO_RECONNECT_DELAY_SEC);
    while (1) {
        waiting = 0;
        while (g_pdpState != ANET_PDP_ACTIVATED) {
            if (waiting % 10 == 0)
                my_log_printf(1, "[MQTT] waiting for network/PDP: state=%u waited=%us",
                              g_pdpState, waiting);
            JM_Sleep(1000);
            waiting++;
        }
        attempt++;
        my_log_printf(1, "[MQTT] network ready; connection attempt #%u", attempt);
        status = mqtt_demo_run();
        demo_mqtt_result("session ended", status);
        demo_mqtt_cleanup();
        my_log_printf(1, "[MQTT] retry in %us; waiting for PDP if unavailable",
                      DEMO_RECONNECT_DELAY_SEC);
        JM_Sleep(DEMO_RECONNECT_DELAY_SEC * 1000);
    }
}
