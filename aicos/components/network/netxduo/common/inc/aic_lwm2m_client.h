/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Network Component
 *
 *  Lightweight Machine to Machine(LWM2M)
 *
 */
#ifndef __AIC_LWM2M_CLIENT_H__
#define __AIC_LWM2M_CLIENT_H__

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus
/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {
#endif

#include <aic_size.h>
#include "aic_osa.h"
#include "nx_lwm2m_client.h"
#include "nxd_dns.h"
#include "nx_api.h"
#include "aic_net_api.h"
#include "aic_at_process_common.h"
#include "aic_at_resp_handle.h"
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls_api.h"
#include "nx_crypto_aes.h"
#include "nx_crypto_sha2.h"
#include "nx_crypto_tls_prf_sha256.h"
#endif /* NX_SECURE_ENABLE_DTLS */



/* Port of LwM2M Server or LwM2M Bootstrap Server.  */
#ifndef LWM2M_SERVER_PORT
#define LWM2M_SERVER_PORT                       5683
#endif /* LWM2M_SERVER_PORT*/
#ifndef LWM2M_SERVER_DTLS_PORT
#define LWM2M_SERVER_DTLS_PORT                  5684
#endif /* LWM2M_SERVER_DTLS_PORT*/
#ifndef LWM2M_BOOTSTRAP_SERVER_PORT
#define LWM2M_BOOTSTRAP_SERVER_PORT             5783
#endif /* LWM2M_BOOTSTRAP_SERVER_PORT*/
#ifndef LWM2M_BOOTSTRAP_SERVER_DTLS_PORT
#define LWM2M_BOOTSTRAP_SERVER_DTLS_PORT        5784
#endif /* LWM2M_BOOTSTRAP_SERVER_DTLS_PORT */
/* LwM2M Client endpoint.  */
#ifndef LWM2M_CLIENT_ENDPOINT
#define LWM2M_CLIENT_ENDPOINT                   "lwm2mclient"
#endif /* LWM2M_CLIENT_ENDPOINT */

#define LWM2M_SERVER_BT       "nbiotbt.heclouds.com"
#define LWM2M_SERVER       "nbiotacc.heclouds.com"
#define LWM2M_SERVER_IP    IP_ADDRESS(183, 230, 40, 40)

#define AIC_LWM2M_ENDPOINT_LEN     64

#define AIC_LWM2M_OBJECT_COUNT_MAX 256
#define AIC_LWM2M_OBJECT_INSTANCE_MAX 8
#define AIC_LWM2M_INSTANCE_RESOURCE_MAX 32
#define AIC_LWM2M_SERVER_URL_LEN_MAX 128

/*object id*/
#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_OBJECT_ID 3200
#define AIC_LWM2M_CLIENT_ANALOG_INPUT_OBJECT_ID  3202
#define AIC_LWM2M_CLIENT_ANALOG_OUTPUT_OBJECT_ID 3203
//#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_OBJECT_ID 3204
#define AIC_LWM2M_CLIENT_LOAD_CONTROL_OBJECT_ID  3310
#define AIC_LWM2M_CLIENT_LIGHT_CONTROL_OBJECT_ID 3311
#define AIC_LWM2M_CLIENT_VOLTAGE_OBJECT_ID       3316
#define AIC_LWM2M_CLIENT_LOAD_OBJECT_ID          3322
#define AIC_LWM2M_CLIENT_DISTANCE_OBJECT_ID      3330
#define AIC_LWM2M_CLIENT_ENERGY_OBJECT_ID        3331
#define AIC_LWM2M_CLIENT_BUZZER_OBJECT_ID        3338
//#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_OBJECT_ID 10290
#define AIC_LWM2M_CLIENT_WEATHER_OBJECT_ID       22001
#define AIC_LWM2M_CLIENT_RAINFAIL_OBJECT_ID      22002
#define AIC_LWM2M_CLIENT_LOCATION_OBJECT_ID      3336

/*digital source id*/
#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_STATE_ID              5500
#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_COUNTER_ID            5501
#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_POLARITY_ID           5502
#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_DEBOUNCE_ID           5503
#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_EDGE_ID               5504
#define AIC_LWM2M_CLIENT_DIGITAL_INPUT_COUNTER_RESET_ID      5505
#define AIC_LWM2M_CLIENT_DIGITAL_APP_TYPE_ID                 5750
#define AIC_LWM2M_CLIENT_DIGITAL_SENSOR_TYPE_ID              5751

#define AIC_LWM2M_CLIENT_RESOURCE_NONE                       0x00
#define AIC_LWM2M_CLIENT_RESOURCE_STRING                     0x01
#define AIC_LWM2M_CLIENT_RESOURCE_INTEGER32                  0x02
#define AIC_LWM2M_CLIENT_RESOURCE_INTEGER64                  0x03
#define AIC_LWM2M_CLIENT_RESOURCE_FLOAT32                    0x04
#define AIC_LWM2M_CLIENT_RESOURCE_FLOAT64                    0x05
#define AIC_LWM2M_CLIENT_RESOURCE_BOOLEAN                    0x06
#define AIC_LWM2M_CLIENT_RESOURCE_OPAQUE                     0x07
#define AIC_LWM2M_CLIENT_RESOURCE_OBJLNK                     0x08
#define AIC_LWM2M_CLIENT_RESOURCE_TLV                        0x09
#define AIC_LWM2M_CLIENT_RESOURCE_TEXT                       0x0a
#define AIC_LWM2M_CLIENT_RESOURCE_OBJECT_INSTANCE            0x0b
#define AIC_LWM2M_CLIENT_RESOURCE_OBJECT_INSTANCE_TLV        0x0c
#define AIC_LWM2M_CLIENT_RESOURCE_MULTIPLE                   0x10
#define AIC_LWM2M_CLIENT_RESOURCE_MULTIPLE_TLV               0x11
#define AIC_LWM2M_CLIENT_RESOURCE_MULTIPLE_UCHAR             0x12
#define AIC_LWM2M_CLIENT_RESOURCE_MULTIPLE_ACL               0x13
#define AIC_LWM2M_CLIENT_RESOURCE_IS_MULTIPLE(t)             ((t) & 0x10)

#define AIC_LWM2M_CLIENT_RESERVED_ID                         65535

#define AT_MAX_RESP_LENGTH                                   128

/* attributes */
#define AIC_ATTR_FLAG_PMIN    (1<<0)
#define AIC_ATTR_FLAG_PMAX    (1<<1)
#define AIC_ATTR_FLAG_GT      (1<<2)
#define AIC_ATTR_FLAG_LT      (1<<3)
#define AIC_ATTR_FLAG_ST      (1<<4)

/* Add for mbedtls*/
#ifdef CONFIG_CRYPTO_MBEDTLS
#define READ_TIMEOUT_MS   (10 * NX_IP_PERIODIC_RATE)
#endif

typedef struct aic_lwm2m_client_instance {
    /* The LWM2M Object Instance */
    u16 instance_id;

    /* resources */
    /* The value of the resource */
    NX_LWM2M_CLIENT_RESOURCE resource[AIC_LWM2M_INSTANCE_RESOURCE_MAX];
} AIC_LWM2M_CLIENT_INSTANCE;

typedef struct aic_lwm2m_client_ptr {
    NX_LWM2M_CLIENT *client;
    NX_IP *lwm2m_ip;
    u32 bind_mode;
    u8 endpoint[AIC_LWM2M_ENDPOINT_LEN];
    u8 endpoint_name_length;
    u8 authcode[AIC_LWM2M_ENDPOINT_LEN];
    u8 authcode_len;
} AIC_LWM2M_CLIENT;

typedef struct aic_lwm2m_parameter{
    int32_t pim;
    int32_t pmax;
    double gt;
    double lt;
    double st;
} AIC_LWM2M_PARA;

typedef struct aic_lwm2m_client_session AIC_LWM2M_CLIENT_SESSION;
typedef void (*aic_session_callback)(AIC_LWM2M_CLIENT_SESSION *lwm2m_session_ptr, UINT state);
typedef u32 (*aic_obj_operation_callback)(u32 operation, u16 object_id,
                                          NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                          NX_LWM2M_CLIENT_RESOURCE *resource, u32 *res_count,
                                          void *args_ptr, u32 args_length,
                                          u16 mid, const void *token_ptr, u32 token_length);
typedef u32 (*aic_obj_observe_callback)(u16 object_id,
                                        NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                        NX_LWM2M_CLIENT_RESOURCE *resource,
                                        u16 mid, boolean observe_flag);
typedef u32 (*aic_obj_attrs_set_callback)(u16 object_id,
                                          NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                          NX_LWM2M_CLIENT_RESOURCE *resource,
                                          u16 mid, AIC_LWM2M_PARA *para, u8 flag);

typedef u32 (*aic_sesion_ack_callback)(u16 ack_id);
typedef u32 (*aic_sesion_write_callback)(u16 object_id, u16 instance_id,
                                         NX_LWM2M_CLIENT_RESOURCE *resource,
                                         u32 *resource_count, void *args_ptr,
                                         u32 args_length, u16 msg_id,
                                         const void *token_ptr, u32 token_length);
typedef u32 (*aic_sesion_dtls_age_callback)(void);

struct aic_lwm2m_client_session {
    NX_LWM2M_CLIENT_SESSION *client_session;
    aic_session_callback callback;
    NX_IP *lwm2m_ip;
#ifdef CONFIG_NET_SECURE
    char *dtls_metadata_buffer;
    UCHAR *dtls_packet_buffer;
#endif
    struct aic_lwm2m_client_session *lwm2m_session_prev;
    struct aic_lwm2m_client_session *lwm2m_session_next;
};

typedef struct aic_lwm2m_client_object {
    NX_LWM2M_CLIENT_OBJECT lwm2m_object;
    aic_obj_operation_callback object_operation;
    aic_obj_observe_callback   object_observe_ptr;
    aic_obj_attrs_set_callback object_attrs_set;
    AIC_LWM2M_CLIENT_INSTANCE lwm2m_instance[AIC_LWM2M_OBJECT_INSTANCE_MAX];
    NX_LWM2M_CLIENT_NOTIFY lwm2m_notify;
    NX_LWM2M_CLIENT_FIRMWARE lwm2m_firmware;
    NX_LWM2M_ID object_id;
    struct aic_lwm2m_client_object *lwm2m_object_prev;
    struct aic_lwm2m_client_object *lwm2m_object_next;
} AIC_LWM2M_CLIENT_OBJ;

typedef struct aic_lwm2m_client_dtls_config {
    u8 enable;
    u32 time_period;
} AIC_LWM2M_CLIENT_DTLS_CFG;

/*aic lwm2m result code*/
enum LWM2M_RESULT_CODE{
    AIC_LWM2M_CLIENT_RESULT_CONTENT_OK = 1,
    AIC_LWM2M_CLIENT_RESULT_STATUS_CHANGED,
    AIC_LWM2M_CLIENT_RESULT_BAD_REQUEST = 11,
    AIC_LWM2M_CLIENT_RESULT_UNAUTHORIZED,
    AIC_LWM2M_CLIENT_RESULT_NOT_FOUND,
    AIC_LWM2M_CLIENT_RESULT_METHOD_NOT_ALLOWED,
    AIC_LWM2M_CLIENT_RESULT_NOT_ACCEPTABLE
};

/*aic lwm2m send format*/
enum LWM2M_FORMAT {
    AIC_LWM2M_CLIENT_FORMAT_TEXT = 0,
    AIC_LWM2M_CLIENT_FORMAT_LINK,
    AIC_LWM2M_CLIENT_FORMAT_OPAQUE,
    AIC_LWM2M_CLIENT_FORMAT_TLV,
    AIC_LWM2M_CLIENT_FORMAT_JSON
};

/*aic lwm2m error code*/
enum LWM2M_ERR {
    AIC_LWM2M_CLIENT_SUCCESS = 0,
    AIC_LWM2M_CLIENT_PTR_NULL,
    AIC_LWM2M_CLIENT_PDP_NOT_ACTIVATE,
    AIC_LWM2M_CLIENT_GET_IMEI_ERROR,
    AIC_LWM2M_CLIENT_GET_IMSI_ERROR,
    AIC_LWM2M_CLIENT_ERROR = NX_LWM2M_CLIENT_ERROR,
    AIC_LWM2M_CLIENT_ALREADY_EXIST = NX_LWM2M_CLIENT_ALREADY_EXIST,
    AIC_LWM2M_CLIENT_BAD_ENCODING,
    AIC_LWM2M_CLIENT_BUFFER_TOO_SMALL,
    AIC_LWM2M_CLIENT_METHOD_NOT_ALLOWED,
    AIC_LWM2M_CLIENT_NO_MEMORY,
    AIC_LWM2M_CLIENT_NOT_FOUND,
    AIC_LWM2M_CLIENT_NOT_SUPPORTED,
    AIC_LWM2M_CLIENT_NOT_ACCEPTABLE,
    AIC_LWM2M_CLIENT_PORT_UNAVAILABLE,
    AIC_LWM2M_CLIENT_ADDRESS_ERROR,
    AIC_LWM2M_CLIENT_DTLS_ERROR,
    AIC_LWM2M_CLIENT_SERVER_INSTANCE_DELETED,
    AIC_LWM2M_CLIENT_TIMED_OUT,
    AIC_LWM2M_CLIENT_NOT_REGISTERED,
    AIC_LWM2M_CLIENT_BAD_REQUEST,
    AIC_LWM2M_CLIENT_UNAUTHORIZED,
    AIC_LWM2M_CLIENT_BAD_OPTION,
    AIC_LWM2M_CLIENT_FORBIDDEN,
    AIC_LWM2M_CLIENT_PRECONDITION_FAILED,
    AIC_LWM2M_CLIENT_UNSUPPORTED_CONTENT_FORMAT = NX_LWM2M_CLIENT_UNSUPPORTED_CONTENT_FORMAT
};

AIC_LWM2M_CLIENT *aic_lwm2m_client_get();
u32 aic_lwm2m_client_create(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                            aic_session_callback callback);
u32 aic_lwm2m_client_delete(AIC_LWM2M_CLIENT_SESSION *lwm2m_session);
u32 aic_lwm2m_client_update_lifetime(u32 lifetime);
u32 aic_lwm2m_client_update(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                            u32 lifetime, boolean obj_update_flag);
u32 aic_lwm2m_client_update_active(AIC_LWM2M_CLIENT_SESSION *lwm2m_session);
u32 aic_lwm2m_register_session_ack_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                            aic_sesion_ack_callback callback);
u32 aic_lwm2m_unregister_session_ack_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session);
u32 aic_lwm2m_register_session_write_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                              aic_sesion_write_callback callback);
u32 aic_lwm2m_unregister_session_write_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session);
u32 aic_lwm2m_register_session_dtls_age_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                                 aic_sesion_dtls_age_callback callback);
u32 aic_lwm2m_unregister_session_dtls_age_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session);
u32 aic_lwm2m_client_bootstrap(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                               NXD_ADDRESS *server_addr, u16 port, u8 *dtls_key,
                               u32 key_size, u8 *dtls_identity, u32 identity_size,
                               bool is_dtls);
u32 aic_lwm2m_client_session_register_info_get(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                               u32 security_instance_id, u16 *server_id,
                                               u8 **server_uri, u32 *server_uri_len, u8 *security_mode,
                                               u8 **pub_key_or_id, u32 *pub_key_or_id_len,
                                               u8 **server_pub_key, u32 *server_pub_key_len,
                                               u8 **secret_key, u32 *secret_key_len);
u32 aic_lwm2m_client_register(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                NXD_ADDRESS *server_addr, u16 port, u16 server_id,
                                u8 *dtls_key, u32 key_size, u8 *dtls_identity,
                                u32 identity_size, bool is_dtls);
u32 aic_lwm2m_client_logout(AIC_LWM2M_CLIENT_SESSION *lwm2m_session);
u32 aic_lwm2m_client_state_get(AIC_LWM2M_CLIENT_SESSION *lwm2m_session, u32 *session_state);
u32 aic_lwm2m_client_session_send_response(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                           u8 result_code, u16 msg_id,
                                           const VOID * token_ptr, UINT token_length,
                                           boolean observe, UINT format, u16 obj_id,
                                           NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instances,
                                           const NX_LWM2M_CLIENT_RESOURCE *resource_ptr);
u32 aic_lwm2m_client_session_error_get(AIC_LWM2M_CLIENT_SESSION *lwm2m_session);
u32 aic_lwm2m_register_obj_observe_callback(u16 obj_id, aic_obj_observe_callback callback);
u32 aic_lwm2m_unregister_obj_observe_callback(u16 obj_id);
u32 aic_lwm2m_register_obj_attrs_set_callback(u16 obj_id, aic_obj_attrs_set_callback callback);
u32 aic_lwm2m_unregister_obj_attrs_set_callback(u16 obj_id);
u32 aic_lwm2m_client_obj_add(u16 object_id, aic_obj_operation_callback object_operation);
u32 aic_lwm2m_client_obj_remove(u16 object_id);
u32 aic_lwm2m_client_obj_read(u16 object_id, u16 instance_id, u16 resource_id,
                              NX_LWM2M_CLIENT_RESOURCE *resource,
                              u32 resource_count);
u32 aic_lwm2m_client_obj_discover(u16 object_id, u16 instance_id,
                                  u16 resource_id,
                                  NX_LWM2M_CLIENT_RESOURCE *resource,
                                  u32 *resource_count);
u32 aic_lwm2m_client_obj_write(u16 object_id, u16 instance_id,
                               u16 resource_id,
                               NX_LWM2M_CLIENT_RESOURCE *resource,
                               u32 resource_count, u32 wriite_flags);
u32 aic_lwm2m_client_obj_execute(u16 object_id, u16 instance_id,
                                 u16 resource_id, const u8 * args_ptr,
                                 u32 args_length);
u32 aic_lwm2m_client_obj_create(u16 object_id, u16 *instance_id_ptr, u32 num_values, const NX_LWM2M_CLIENT_RESOURCE *values);
u32 aic_lwm2m_client_obj_delete(u16 object_id, u16 instance_id);
u32 aic_lwm2m_client_obj_instance_add(u16 object_id,
                                      NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr,
                                      u16 *instance_id_ptr);
u32 aic_lwm2m_client_obj_instance_remove(u16 object_id, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr);
u32 aic_lwm2m_client_obj_resource_changed(u16 object_id,
                                          NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr,
                                          const NX_LWM2M_CLIENT_RESOURCE * resource);
u32 aic_lwm2m_client_resource_info_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr,
                                       u16 *resource_id, u32 *operation);
u32 aic_lwm2m_client_resource_info_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr,
                                       u16 resource_id, u32 operation);
u32 aic_lwm2m_client_resource_set(NX_LWM2M_CLIENT_RESOURCE *resource,
                                  u8 *value, u32 value_length, u8 type);
u32 aic_lwm2m_client_resource_get(NX_LWM2M_CLIENT_RESOURCE *resource,
                                  u8 *value, u32 *value_length, u8 type);
u32 aic_lwm2m_client_endpoint_set(u8 *epname, u32 name_len);
u32 aic_lwm2m_client_bindmode_set(u32 bindmode);
u32 aic_lwm2m_client_dtls_conf_set(u32 time_period, u8 enable);
void aic_lwm2m_client_set_msg_id(u16 id);
u32 aic_lwm2m_client_authcode_set(u8 *authcode, u32 authcode_len);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
        }
#endif

#endif
