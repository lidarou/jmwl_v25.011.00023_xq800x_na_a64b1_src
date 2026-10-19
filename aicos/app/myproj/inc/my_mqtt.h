#ifndef __MQTT_DEMO_H__
#define __MQTT_DEMO_H__

/*
 * 1 = MQTTS (8883, TLS, 必须 CA)
 * 0 = MQTT  (1883, 明文, 不需要 CA)
 */
#define CONFIG_USE_MQTTS   1

#if CONFIG_USE_MQTTS
#define DEMO_MQTT_HOST     "broker.emqx.io"
#define DEMO_MQTT_PORT     AIC_MQTTS_PORT      /* 8883 */
#define DEMO_CLIENT_TYPE   AIC_MQTTS_CLIENT_TYPE
#else
#define DEMO_MQTT_HOST     "broker.emqx.io"
#define DEMO_MQTT_PORT     AIC_MQTT_PORT       /* 1883 */
#define DEMO_CLIENT_TYPE   AIC_MQTT_CLIENT_TYPE
#endif

#define DEMO_CLIENT_NAME   "aic_mqtt_demo"
#define DEMO_CLIENT_ID     "aic_mqtt_demo_0001"

/* 公共 broker 匿名也可连，这里留空也行 */
#define DEMO_USERNAME      "qyrix"
#define DEMO_PASSWORD      "qyrixmqpassword"

#define DEMO_SUB_TOPIC     "demo/rx"
#define DEMO_PUB_TOPIC     "demo/tx"
#define DEMO_PUB_MSG       "hello from AIC MQTT client"

#define DEMO_KEEP_ALIVE    60
#define DEMO_WAIT          (5 * NX_IP_PERIODIC_RATE)
#define DEMO_QOS           1

/* Send immediately after connecting, then repeat at this interval. */
#define DEMO_PUBLISH_INTERVAL_SEC  10
#define DEMO_RECONNECT_DELAY_SEC   10

void my_mqtt_task(void *argv);

#endif
