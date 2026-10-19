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
#define AIC_THIS_FILE    0x1a8c1d
#define LOG_MODULE    AMOD_NET

#include "aic_lwm2m_client.h"
#include <aic_size.h>
#include "aic_at_process_common.h"
#include "aic_at_resp_handle.h"

extern NX_LWM2M_INT32 nx_lifetime;
extern UCHAR nx_bindmode;

static AIC_LWM2M_CLIENT lwm2m_client = {
    NULL, NULL, NX_LWM2M_CLIENT_BINDING_U, {0}, 0, {0}, 0
};

#ifdef CONFIG_CRYPTO_MBEDTLS
static const int lwm2m_ciphersuites[3] = {
    MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8, MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256, 0};
#endif
static AIC_LWM2M_CLIENT_SESSION *aic_lwm2m_session_ptr;
static AIC_LWM2M_CLIENT_OBJ *aic_lwm2m_object_ptr;
static ULONG lwm2m_client_stack[SIZE_4K / sizeof(ULONG)];
AIC_LWM2M_CLIENT_DTLS_CFG aic_dtls_cfg;

#ifdef NX_SECURE_ENABLE_DTLS
extern NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;
#endif
#define AIC_SECURE_TLS_CIPHERS_TABLE nx_crypto_tls_ciphers

static u32 aic_lwm2m_client_init();
static void aic_lwm2m_client_uninit();

#ifdef MBEDTLS_SSL_RENEGOTIATION
#define AIC_PUT_UINT64_BE(out_be,in_le,i)                               \
{                                                                       \
    (out_be)[(i) + 0] = (unsigned char)( ( (in_le) >> 56 ) & 0xFF );    \
    (out_be)[(i) + 1] = (unsigned char)( ( (in_le) >> 48 ) & 0xFF );    \
    (out_be)[(i) + 2] = (unsigned char)( ( (in_le) >> 40 ) & 0xFF );    \
    (out_be)[(i) + 3] = (unsigned char)( ( (in_le) >> 32 ) & 0xFF );    \
    (out_be)[(i) + 4] = (unsigned char)( ( (in_le) >> 24 ) & 0xFF );    \
    (out_be)[(i) + 5] = (unsigned char)( ( (in_le) >> 16 ) & 0xFF );    \
    (out_be)[(i) + 6] = (unsigned char)( ( (in_le) >> 8  ) & 0xFF );    \
    (out_be)[(i) + 7] = (unsigned char)( ( (in_le) >> 0  ) & 0xFF );    \
}
#endif

static void aic_lwm2m_add_session_to_list(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    AOS_INTERRUPT_DECLARE
    AIC_LWM2M_CLIENT_SESSION *tail_session;

    AOS_INTERRUPT_DISABLE
    if (!aic_lwm2m_session_ptr) {
        aic_lwm2m_session_ptr = lwm2m_session;
        aic_lwm2m_session_ptr->lwm2m_session_prev = lwm2m_session;
        aic_lwm2m_session_ptr->lwm2m_session_next = lwm2m_session;
    } else {
        tail_session = aic_lwm2m_session_ptr->lwm2m_session_prev;

        aic_lwm2m_session_ptr->lwm2m_session_prev = lwm2m_session;
        tail_session->lwm2m_session_next = lwm2m_session;

        lwm2m_session->lwm2m_session_prev = tail_session;
        lwm2m_session->lwm2m_session_next = aic_lwm2m_session_ptr;
    }

    AOS_INTERRUPT_RESTORE
}

static void aic_lwm2m_delete_session_from_list(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (lwm2m_session == lwm2m_session->lwm2m_session_next) {
        aic_lwm2m_session_ptr = NX_NULL;
    } else {
        (lwm2m_session->lwm2m_session_next)->lwm2m_session_prev =
            lwm2m_session->lwm2m_session_prev;
        (lwm2m_session->lwm2m_session_prev)->lwm2m_session_next =
            lwm2m_session->lwm2m_session_next;

        if (aic_lwm2m_session_ptr == lwm2m_session)
            aic_lwm2m_session_ptr = lwm2m_session->lwm2m_session_next;
    }
    AOS_INTERRUPT_RESTORE
}

static AIC_LWM2M_CLIENT_SESSION *aic_get_lwm2m_session(NX_LWM2M_CLIENT_SESSION *session)
{
    int i;
    AOS_INTERRUPT_DECLARE
    AIC_LWM2M_CLIENT_SESSION *lwm2m_session;

    AOS_INTERRUPT_DISABLE
    lwm2m_session = aic_lwm2m_session_ptr;
    for(i = 0; lwm2m_session != NULL; lwm2m_session = lwm2m_session->lwm2m_session_next) {
        if(lwm2m_session->client_session == session) {
            AOS_INTERRUPT_RESTORE
            return lwm2m_session;
        }

        if (lwm2m_session->lwm2m_session_next == aic_lwm2m_session_ptr) {
            AOS_INTERRUPT_RESTORE
            return NULL;
        }
    }

    AOS_INTERRUPT_RESTORE
    return NULL;
}

static void aic_lwm2m_add_object_to_list(AIC_LWM2M_CLIENT_OBJ *lwm2m_object)
{
    AOS_INTERRUPT_DECLARE
    AIC_LWM2M_CLIENT_OBJ *tail_object;

    AOS_INTERRUPT_DISABLE
    if (!aic_lwm2m_object_ptr) {
        aic_lwm2m_object_ptr = lwm2m_object;
        aic_lwm2m_object_ptr->lwm2m_object_prev = lwm2m_object;
        aic_lwm2m_object_ptr->lwm2m_object_next = lwm2m_object;
    } else {
        tail_object = aic_lwm2m_object_ptr->lwm2m_object_prev;

        aic_lwm2m_object_ptr->lwm2m_object_prev = lwm2m_object;
        tail_object->lwm2m_object_next = lwm2m_object;

        lwm2m_object->lwm2m_object_prev = tail_object;
        lwm2m_object->lwm2m_object_next = aic_lwm2m_object_ptr;
    }
    AOS_INTERRUPT_RESTORE
}

static void aic_lwm2m_delete_all_object_from_list(void)
{
    AOS_INTERRUPT_DECLARE
    AIC_LWM2M_CLIENT_OBJ *lwm2m_obj;

    AOS_INTERRUPT_DISABLE
    lwm2m_obj = aic_lwm2m_object_ptr;
    while (lwm2m_obj) {
        if(lwm2m_obj == lwm2m_obj->lwm2m_object_next) {
            aos_free(lwm2m_obj);
            aic_lwm2m_object_ptr = AIC_NULL;
        } else {
            (lwm2m_obj->lwm2m_object_next)->lwm2m_object_prev =
                lwm2m_obj->lwm2m_object_prev;
            (lwm2m_obj->lwm2m_object_prev)->lwm2m_object_next =
                lwm2m_obj->lwm2m_object_next;

            if (aic_lwm2m_object_ptr == lwm2m_obj)
                aic_lwm2m_object_ptr = lwm2m_obj->lwm2m_object_next;

            aos_free(lwm2m_obj);
        }

        lwm2m_obj = aic_lwm2m_object_ptr;
    }
    AOS_INTERRUPT_RESTORE
}

static void aic_lwm2m_delete_object_from_list(AIC_LWM2M_CLIENT_OBJ *lwm2m_object)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (lwm2m_object == lwm2m_object->lwm2m_object_next) {
        aic_lwm2m_object_ptr = NX_NULL;
    } else {
        (lwm2m_object->lwm2m_object_next)->lwm2m_object_prev =
            lwm2m_object->lwm2m_object_prev;
        (lwm2m_object->lwm2m_object_prev)->lwm2m_object_next =
            lwm2m_object->lwm2m_object_next;

        if (aic_lwm2m_object_ptr == lwm2m_object)
            aic_lwm2m_object_ptr = lwm2m_object->lwm2m_object_next;
    }
    AOS_INTERRUPT_RESTORE
}

AIC_LWM2M_CLIENT_OBJ *aic_get_lwm2m_object_by_id(u16 object_id)
{
    int i;
    AOS_INTERRUPT_DECLARE
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;

    AOS_INTERRUPT_DISABLE
    lwm2m_object = aic_lwm2m_object_ptr;
    for(i = 0; lwm2m_object != NULL; lwm2m_object = lwm2m_object->lwm2m_object_next) {
        if(lwm2m_object->object_id == object_id) {
            AOS_INTERRUPT_RESTORE
            return lwm2m_object;
        }

        if (lwm2m_object->lwm2m_object_next == aic_lwm2m_object_ptr) {
            AOS_INTERRUPT_RESTORE
            return NULL;
        }
    }

    AOS_INTERRUPT_RESTORE
    return NULL;

}

#if 0
static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    aicx_info(0x0d776a, "sds", "%s:%04d: %s", file, line, str);
}
#endif

static u32 aic_lwm2m_dtls_setup(AIC_LWM2M_CLIENT_SESSION *client_session,
                                u8 *key, u32 key_len, u8 *identity,
                                u32 identity_len)
{
    u32 status;
#ifdef CONFIG_NET_SECURE
    NX_SECURE_DTLS_SESSION dtls_session;
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
    const char *pers = "lwm2m_dtls_client";
    //u8 renego_period[8] = {0, 1, 0, 0, 0, 0, 7, 8};
#endif

#ifdef CONFIG_NET_SECURE
    if (client_session->client_session &&
        !client_session->client_session->nx_lwm2m_client_session_dtls_session) {
        dtls_session = aos_zalloc_relaxed(sizeof(NX_SECURE_DTLS_SESSION));
        if (!dtls_session) {
            aicx_error(0x103cf4, "", "[LWM2M] alloc dtls session fail\n");
            return AIC_LWM2M_CLIENT_NO_MEMORY;
        }
        client_session->client_session->nx_lwm2m_client_session_dtls_session = dtls_session;
        client_session->dtls_metadata_buffer = aos_zalloc(SIZE_8K);
        client_session->dtls_packet_buffer = aos_zalloc(SIZE_4K);
    }

    /* Create the DTLS Client Session.  */
    status = nx_secure_dtls_session_create(dtls_session,
                                           &AIC_SECURE_TLS_CIPHERS_TABLE,
                                           client_session->dtls_metadata_buffer,
                                           SIZE_8K,
                                           client_session->dtls_packet_buffer,
                                           SIZE_4K, 0, NX_NULL, 0);
    if (status)
    {
        aicx_info(0x170163, "x", "[lwm2m] set psk error status 0x%x\n", status);
        return status;
    }

    /* Set the PSK.  */
    status = nx_secure_tls_psk_add(&(dtls_session->nx_secure_dtls_tls_session),
                                   key, key_len, identity, identity_len, NX_NULL, 0);
    status += nx_secure_tls_client_psk_set(&(dtls_session->nx_secure_dtls_tls_session),
                                           key, key_len, identity, identity_len, NX_NULL, 0);
    if (status)
    {
        aicx_info(0x170163, "x", "[lwm2m] set psk error status 0x%x\n", status);
        nx_secure_dtls_session_delete(dtls_session);
        return(status);
    }
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS
    aicx_info(0x19f1ee, "", "[lwm2m] mbedtls setup dtls\n");
    //sclient_session.client_session->lwm2m_mbedtls = aos_zalloc(sizeof(LWM2M_MBEDTLS_SESSION));
    client_session->client_session->lwm2m_use_dtls = NX_TRUE;
    mbedtls_net_init(&client_session->client_session->lwm2m_mbedtls.socket_fd);
    mbedtls_ssl_init(&client_session->client_session->lwm2m_mbedtls.ssl);
    mbedtls_ssl_config_init(&client_session->client_session->lwm2m_mbedtls.conf);
    mbedtls_ctr_drbg_init(&client_session->client_session->lwm2m_mbedtls.ctr_drbg);
    mbedtls_entropy_init(&client_session->client_session->lwm2m_mbedtls.entropy);
    //mbedtls_debug_set_threshold(8);

    mbedtls_ctr_drbg_seed(&client_session->client_session->lwm2m_mbedtls.ctr_drbg,
                          mbedtls_entropy_func,
                          &client_session->client_session->lwm2m_mbedtls.entropy,
                          (const unsigned char *) pers,
                          strlen(pers));

    status = mbedtls_ssl_config_defaults(&client_session->client_session->lwm2m_mbedtls.conf,
                                         MBEDTLS_SSL_IS_CLIENT,
                                         MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                         MBEDTLS_SSL_PRESET_DEFAULT);
    if (status != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x04ccec, "x", "[lwm2m] dtls default config fail(0x%x)\n", status);
        return status;
    }

    mbedtls_ssl_conf_authmode(&client_session->client_session->lwm2m_mbedtls.conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_rng(&client_session->client_session->lwm2m_mbedtls.conf, mbedtls_ctr_drbg_random,
                         &client_session->client_session->lwm2m_mbedtls.ctr_drbg);
    aic_ssl_set_recv_waitoption(client_session->client_session->lwm2m_mbedtls.socket_fd.fd,
                                client_session->client_session->lwm2m_mbedtls.recv_wait_time);
    //mbedtls_ssl_conf_dbg(&client_session->client_session->lwm2m_mbedtls.conf, my_debug, NULL);
    //mbedtls_ssl_conf_read_timeout(&client_session->client_session->lwm2m_mbedtls.conf, READ_TIMEOUT_MS);
    status = mbedtls_ssl_conf_psk(&client_session->client_session->lwm2m_mbedtls.conf, key, key_len, identity, identity_len);
    if (status != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x09260c, "x", "[lwm2m] dtls config psk fail(0x%x)\n", -status);
        return status;
    }
    //mbedtls_ssl_set_hs_psk(&client_session->client_session->lwm2m_mbedtls.ssl, key, key_len);
    mbedtls_ssl_conf_ciphersuites(&client_session->client_session->lwm2m_mbedtls.conf, lwm2m_ciphersuites);

#ifdef MBEDTLS_SSL_RENEGOTIATION
    mbedtls_ssl_conf_legacy_renegotiation(&client_session->client_session->lwm2m_mbedtls.conf, MBEDTLS_SSL_LEGACY_ALLOW_RENEGOTIATION);
    mbedtls_ssl_conf_renegotiation(&client_session->client_session->lwm2m_mbedtls.conf,
                                   aic_dtls_cfg.enable); //MBEDTLS_SSL_RENEGOTIATION_ENABLED
    client_session->client_session->nx_lwm2m_client_session_dtls_cfg.enable_renegotiation = aic_dtls_cfg.enable;
    client_session->client_session->nx_lwm2m_client_session_dtls_cfg.time_period = aic_dtls_cfg.time_period;
    //mbedtls_ssl_conf_renegotiation_period(&client_session->client_session->lwm2m_mbedtls.conf,
                                          //renego_period);
#endif

    status = mbedtls_ssl_setup(&client_session->client_session->lwm2m_mbedtls.ssl,
                               &client_session->client_session->lwm2m_mbedtls.conf);
    if (status != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x021178, "d", "[lwm2m] mbedtsl setup fail(0x%d)\n", status);
        //aos_free(client_session->client_session->lwm2m_mbedtls);
        aos_memset(&client_session->client_session->lwm2m_mbedtls, 0, sizeof(LWM2M_MBEDTLS_SESSION));
        return AIC_LWM2M_CLIENT_DTLS_ERROR;
    }

    aicx_info(0x0d78f0, "", "[lwm2m] mbedtls setup dtls sucess\n");
    mbedtls_ssl_set_bio(&client_session->client_session->lwm2m_mbedtls.ssl,
                        &client_session->client_session->lwm2m_mbedtls.socket_fd,
                        mbedtls_net_send_ext, mbedtls_net_recv_ext,
                        mbedtls_net_recv_timeout_ext);

    mbedtls_ssl_set_timer_cb(&client_session->client_session->lwm2m_mbedtls.ssl,
                             &client_session->client_session->lwm2m_mbedtls.timer,
                             mbedtls_timing_set_delay,
                             mbedtls_timing_get_delay);

#endif
    return AIC_LWM2M_CLIENT_SUCCESS;
}

static u32 aic_lwm2m_dtls_reset(AIC_LWM2M_CLIENT_SESSION *client_session)
{
    if (!client_session || !client_session->client_session)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    aicx_info(0x0b47ba, "", "[lwm2m] aic_lwm2m_dtls_reset\n");
    client_session->client_session->lwm2m_use_dtls = NX_FALSE;
    mbedtls_ssl_reset_socket(&client_session->client_session->lwm2m_mbedtls.socket_fd);
    mbedtls_net_free(&client_session->client_session->lwm2m_mbedtls.socket_fd);
    mbedtls_ssl_config_free(&client_session->client_session->lwm2m_mbedtls.conf);
    mbedtls_ctr_drbg_free(&client_session->client_session->lwm2m_mbedtls.ctr_drbg);
    mbedtls_entropy_free(&client_session->client_session->lwm2m_mbedtls.entropy);
    mbedtls_ssl_free(&client_session->client_session->lwm2m_mbedtls.ssl);
    return AIC_LWM2M_CLIENT_SUCCESS;
}

/* Define the session state callback */
static void session_callback(NX_LWM2M_CLIENT_SESSION *session_ptr, UINT state)
{
    AIC_LWM2M_CLIENT_SESSION *lwm2m_session;

    aicx_info(0x0d30d0, "d", "[LWM2M] Callback: -> %d\n", state);
    switch (state) {
    case NX_LWM2M_CLIENT_SESSION_INIT:
    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_WAITING:
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_REQUESTING:
        aicx_info(0x131eb7, "", "[lwm2m] Start client initiated bootstrap\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_INITIATED:
        aicx_info(0x0a18f1, "", "[lwm2m] Got message from boostrap server\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_FINISHED:
        //aos_set_event(aic_lwm2m_event, AIC_LWM2M_REGISETR_EVENT, TX_OR);
         /* Bootstrap session done, we can register to the LWM2M Server */
        aicx_info(0x0475e6, "", "[lwm2m] Boostrap finished.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_ERROR:
        /* Failed to Bootstrap the LWM2M Client. */
        aicx_info(0x119b51, "x", "[lwm2m] Failed to boostrap device, error=0x%02x\n",
                  nx_lwm2m_client_session_error_get(session_ptr));
        break;

    case NX_LWM2M_CLIENT_SESSION_REGISTERED:
        /* Registration to the LWM2M Client done. */
        aicx_info(0x15f8df, "", "[lwm2m] LWM2M device registered.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_REGISTERING:
    case NX_LWM2M_CLIENT_SESSION_UPDATING:
    case NX_LWM2M_CLIENT_SESSION_DISABLING:
    case NX_LWM2M_CLIENT_SESSION_DEREGISTERING:
        break;

    case NX_LWM2M_CLIENT_SESSION_DISABLED:
        /* Registration to the LWM2M Client done. */
        aicx_info(0x1714a6, "", "[lwm2m] LWM2M device disabled.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_DEREGISTERED:
        /* Registration to the LWM2M Client done. */
        aicx_info(0x0bd8bc, "", "[lwm2m] LWM2M device deregistered.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_ERROR:
        /* Failed to register to the LWM2M Client. */
        aicx_info(0x01ddc8, "x", "[lwm2m] Failed to register device, error=0x%02x\n",
                  nx_lwm2m_client_session_error_get(session_ptr));
        break;

    default:
        aicx_info(0x17ba1f, "", "[lwm2m] not support state");
        break;
    }

    lwm2m_session = aic_get_lwm2m_session(session_ptr);
    if (lwm2m_session && lwm2m_session->callback) {
        aicx_info(0x0266f0, "d", "[lwm2m] Callback notify: -> %d\n", state);
        lwm2m_session->callback(lwm2m_session, state);
    }
}

AIC_LWM2M_CLIENT *aic_lwm2m_client_get()
{
    return &lwm2m_client;
}


u32 aic_lwm2m_client_create(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                            aic_session_callback callback)
{
    u32 ret;

    if (!lwm2m_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    if (!lwm2m_client.client) {
        ret = aic_lwm2m_client_init();
        if (ret) {
            aicx_info(0x0f4917, "x", "[lwm2m] client init fail, ret 0x%x\n", ret);
            return AIC_LWM2M_CLIENT_ERROR;
        }
    } else {
        aicx_info(0x0e75e9, "", "[lwm2m] client already exist\n");
        return AIC_LWM2M_CLIENT_ALREADY_EXIST;
    }

    lwm2m_session->client_session = aos_zalloc_relaxed(sizeof(NX_LWM2M_CLIENT_SESSION));
    if (!lwm2m_session->client_session) {
        aicx_error(0x0458e1, "", "[lwm2m] ERROR no enough memory\n");
        return AIC_LWM2M_CLIENT_NO_MEMORY;
    }

    ret = nx_lwm2m_client_session_create(lwm2m_session->client_session,
                                         lwm2m_client.client, session_callback);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aos_free(lwm2m_session->client_session);
        aicx_info(0x08fd3d, "x", "[lwm2m] client session create fail, ret 0x%x\n", ret);
        return AIC_LWM2M_CLIENT_ERROR;
    }

    lwm2m_session->callback = callback;
    aic_lwm2m_add_session_to_list(lwm2m_session);

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_delete(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

#ifdef CONFIG_CRYPTO_MBEDTLS
    if (lwm2m_session->client_session->lwm2m_use_dtls) {
        lwm2m_session->client_session->lwm2m_use_dtls = NX_FALSE;
        mbedtls_net_free(&lwm2m_session->client_session->lwm2m_mbedtls.socket_fd);
        mbedtls_ssl_config_free(&lwm2m_session->client_session->lwm2m_mbedtls.conf);
        mbedtls_ctr_drbg_free(&lwm2m_session->client_session->lwm2m_mbedtls.ctr_drbg);
        mbedtls_entropy_free(&lwm2m_session->client_session->lwm2m_mbedtls.entropy);
        mbedtls_ssl_free(&lwm2m_session->client_session->lwm2m_mbedtls.ssl);
    }
#endif

    nx_lwm2m_client_session_delete(lwm2m_session->client_session);
    aic_lwm2m_delete_session_from_list(lwm2m_session);
    aos_free(lwm2m_session->client_session);
    lwm2m_session->client_session = AIC_NULL;
    aic_lwm2m_client_uninit();
    aicx_info(0x15459b, "", "[AIC_LWM2M] aic_lwm2m_client_delete\n");

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_update_lifetime(u32 lifetime)
{
    NX_LWM2M_CLIENT_RESOURCE resource;
    u32 ret;

    if (!lwm2m_client.client) {
        return AIC_LWM2M_CLIENT_NOT_FOUND;
    }

    nx_lifetime = lifetime;
    nx_lwm2m_client_resource_info_set(&resource, NX_LWM2M_CLIENT_SERVER_LIFETIME_ID, NX_NULL);
    nx_lwm2m_client_resource_integer32_set(&resource, lifetime);
    ret = nx_lwm2m_client_object_write(lwm2m_client.client,
                                 NX_LWM2M_CLIENT_SERVER_OBJECT_ID, 0,
                                 NX_LWM2M_CLIENT_SERVER_LIFETIME_ID,
                                 &resource, NX_LWM2M_CLIENT_OBJECT_WRITE_UPDATE);
    aicx_error(0x12c383, "d", "[lwm2m] update_lifetime %d\n", lifetime);
    return ret;
}

u32 aic_lwm2m_client_update_timeout(u32 timeout)
{
    NX_LWM2M_CLIENT_RESOURCE resource;
    u32 ret;

    if (!lwm2m_client.client) {
        return AIC_LWM2M_CLIENT_NOT_FOUND;
    }

    nx_lwm2m_client_resource_info_set(&resource, NX_LWM2M_CLIENT_SERVER_DISABLE_TIMEOUT_ID, NX_NULL);
    nx_lwm2m_client_resource_integer32_set(&resource, timeout);
    ret = nx_lwm2m_client_object_write(lwm2m_client.client,
                                 NX_LWM2M_CLIENT_SERVER_OBJECT_ID, 0,
                                 NX_LWM2M_CLIENT_SERVER_DISABLE_TIMEOUT_ID,
                                 &resource, NX_LWM2M_CLIENT_OBJECT_WRITE_UPDATE);
    aicx_error(0x1e0356, "d", "[lwm2m] update_timeout %d\n", timeout);
    return ret;
}

u32 aic_lwm2m_client_get_lifetime(u32 *lifetime)
{
    NX_LWM2M_CLIENT_RESOURCE resource;
    u32 ret;

    if (!lwm2m_client.client) {
        return AIC_LWM2M_CLIENT_NOT_FOUND;
    }

    ret = nx_lwm2m_client_object_read(lwm2m_client.client,
                                      NX_LWM2M_CLIENT_SERVER_OBJECT_ID, 0,
                                      NX_LWM2M_CLIENT_SERVER_LIFETIME_ID,
                                      &resource);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        return ret;
    }

    nx_lwm2m_client_resource_integer32_get(&resource, (NX_LWM2M_INT32 *)lifetime);
    aicx_error(0x00e36f, "d", "[lwm2m] get_lifetime %d\n", *lifetime);
    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_update(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                            u32 lifetime, boolean obj_update_flag)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    aic_lwm2m_client_update_lifetime(lifetime);

    if (obj_update_flag) {
        /* Update list of object */
        _nx_lwm2m_client_session_update_flags(lwm2m_session->client_session,
                                              NX_LWM2M_CLIENT_SESSION_UPDATE_OBJECTS_LIST);
    }
    //nx_lwm2m_client_session_update(lwm2m_session->client_session);

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_update_active(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    return nx_lwm2m_client_session_update(lwm2m_session->client_session);
}

u32 aic_lwm2m_register_session_ack_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                            aic_sesion_ack_callback callback)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    lwm2m_session->client_session->nx_lwm2m_client_session_ack_notify = callback;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_unregister_session_ack_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    lwm2m_session->client_session->nx_lwm2m_client_session_ack_notify = AIC_NULL;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_register_session_write_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                              aic_sesion_write_callback callback)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    lwm2m_session->client_session->nx_lwm2m_client_session_write_notify = callback;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_unregister_session_write_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    lwm2m_session->client_session->nx_lwm2m_client_session_write_notify = AIC_NULL;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_register_session_dtls_age_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                                 aic_sesion_dtls_age_callback callback)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    lwm2m_session->client_session->nx_lwm2m_client_session_dtls_age_notify = callback;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_unregister_session_dtls_age_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    lwm2m_session->client_session->nx_lwm2m_client_session_dtls_age_notify = AIC_NULL;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_bootstrap(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                               NXD_ADDRESS *server_addr, u16 port, u8 *dtls_key,
                               u32 key_size, u8 *dtls_identity, u32 identity_size,
                               bool is_dtls)
{
    u32 status;

    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    if (is_dtls) {
#ifdef CONFIG_CRYPTO_MBEDTLS
        /* Re-create the DTLS Client Session.  */
        if (lwm2m_session->client_session->lwm2m_use_dtls)
            aic_lwm2m_dtls_reset(lwm2m_session);
#endif
        /* Create the DTLS Client Session.  */
        aicx_info(0x0def78, "ss", "[lwm2m] dtls_key %s, dtls_identity %s\n", dtls_key, dtls_identity);
        aic_lwm2m_dtls_setup(lwm2m_session, dtls_key, key_size, dtls_identity,
                             identity_size);

        aicx_info(0x0ddd54, "", "[lwm2m] Start boostraping with DTLS\n");
#ifdef CONFIG_NET_SECURE
        status = nx_lwm2m_client_session_bootstrap_dtls(lwm2m_session->client_session,
                                                        0, server_addr, port,
                                                        lwm2m_session->client_session->nx_lwm2m_client_session_dtls_session);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
        status = nx_lwm2m_client_session_bootstrap(lwm2m_session->client_session,
                                                   0, server_addr, port);
#endif
        if (status)
        {
            return status;
        }
    } else {
        aicx_info(0x0408b2, "", "[lwm2m] Start boostraping\n");
        lwm2m_session->client_session->lwm2m_use_dtls = NX_FALSE;
        status = nx_lwm2m_client_session_bootstrap(lwm2m_session->client_session,
                                                   0, server_addr, port);
        if (status) {
            aicx_info(0x1ad258, "x", "[lwm2m] Start boostrap fail status 0x%x\n", status);
            return status;
        }
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_session_register_info_get(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                               u32 security_instance_id, u16 *server_id,
                                               u8 **server_uri, u32 *server_uri_len, u8 *security_mode,
                                               u8 **pub_key_or_id, u32 *pub_key_or_id_len,
                                               u8 **server_pub_key, u32 *server_pub_key_len,
                                               u8 **secret_key, u32 *secret_key_len)
{
    u32 status;

    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    status = nx_lwm2m_client_session_register_info_get(lwm2m_session->client_session,
                                                       security_instance_id,
                                                       server_id, (char **)server_uri,
                                                       server_uri_len, security_mode,
                                                       pub_key_or_id, pub_key_or_id_len,
                                                       server_pub_key, server_pub_key_len,
                                                       secret_key, secret_key_len);
    if (status) {
        aicx_info(0x081eec, "x", "[lwm2m] register info get fail, status 0x%x\n", status);
        return AIC_LWM2M_CLIENT_NOT_FOUND;
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_register(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                NXD_ADDRESS *server_addr, u16 port, u16 server_id,
                                u8 *dtls_key, u32 key_size, u8 *dtls_identity,
                                u32 identity_size, bool is_dtls)
{
    u32 status;

    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    if (is_dtls) {
#ifdef CONFIG_NET_SECURE
        /* Re-create the DTLS Client Session.  */
        nx_secure_dtls_session_delete(lwm2m_session->client_session->nx_lwm2m_client_session_dtls_session);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
        /* Re-create the DTLS Client Session.  */
        if (lwm2m_session->client_session->lwm2m_use_dtls)
            aic_lwm2m_dtls_reset(lwm2m_session);
#endif
        aicx_info(0x0def78, "ss", "[lwm2m] dtls_key %s, dtls_identity %s\n", dtls_key, dtls_identity);
        aic_lwm2m_dtls_setup(lwm2m_session, dtls_key, key_size, dtls_identity,
                             identity_size);

        aicx_info(0x09cd9a, "", "[lwm2m] Register to LWM2M server with DTLS\n");
#ifdef CONFIG_NET_SECURE
        status = nx_lwm2m_client_session_register_dtls(lwm2m_session->client_session,
                                                       server_id, server_addr,
                                                       port,
                                                       lwm2m_session->client_session->nx_lwm2m_client_session_dtls_session);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS
        status = nx_lwm2m_client_session_register(lwm2m_session->client_session,
                                                  server_id, server_addr,
                                                  port);
#endif
    } else {
        aicx_info(0x0218b9, "", "[lwm2m] Register to LWM2M server\r\n");
        lwm2m_session->client_session->lwm2m_use_dtls = NX_FALSE;
        status = nx_lwm2m_client_session_register(lwm2m_session->client_session,
                                                  server_id, (const NXD_ADDRESS *)server_addr,
                                                  port);
        if (status) {
            return status;
        }
    }
    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_logout(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    u32 status;
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    status = nx_lwm2m_client_session_deregister(lwm2m_session->client_session);
    if (status) {
        aicx_info(0x0066a4, "x", "[lwm2m] client deregister fail status 0x%x\n", status);
        return AIC_LWM2M_CLIENT_NOT_REGISTERED;
    }

#ifdef CONFIG_NET_SECURE
    if (lwm2m_session->client_session->nx_lwm2m_client_session_dtls_session) {
        aos_free(lwm2m_session->client_session->nx_lwm2m_client_session_dtls_session);
        lwm2m_session->client_session->nx_lwm2m_client_session_dtls_session = NULL;
        aos_free(lwm2m_session->dtls_metadata_buffer);
        aos_free(lwm2m_session->dtls_packet_buffer);
    }
#endif
    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_state_get(AIC_LWM2M_CLIENT_SESSION *lwm2m_session, u32 *session_state)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    *session_state = lwm2m_session->client_session->nx_lwm2m_client_session_state;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_session_send_response(AIC_LWM2M_CLIENT_SESSION *lwm2m_session,
                                           u8 result_code, u16 msg_id,
                                           const VOID * token_ptr, UINT token_length,
                                           boolean observe, UINT format, u16 obj_id,
                                           NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instances,
                                           const NX_LWM2M_CLIENT_RESOURCE *resource_ptr)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;
    NX_LWM2M_CLIENT_OBJECT *obj_ptr = AIC_NULL;
    u32 status, ul_format;
    u8 code;


    if (obj_id != AIC_LWM2M_CLIENT_RESERVED_ID) {
        lwm2m_object = aic_get_lwm2m_object_by_id(obj_id);
        if (lwm2m_object)
            obj_ptr = &lwm2m_object->lwm2m_object;
        else
            return AIC_LWM2M_CLIENT_NOT_FOUND;
    }

    switch (result_code) {
            /* Successful operation */
    case AIC_LWM2M_CLIENT_RESULT_CONTENT_OK:
        code = NX_LWM2M_CLIENT_COAP_STATUS_CONTENT;
        break;

    case AIC_LWM2M_CLIENT_RESULT_STATUS_CHANGED:
        code = NX_LWM2M_CLIENT_COAP_STATUS_CHANGED;
        break;

    /* 4.01 Unauthorized */
    case AIC_LWM2M_CLIENT_RESULT_UNAUTHORIZED:
        code = NX_LWM2M_CLIENT_COAP_STATUS_UNAUTHORIZED;
        break;

    /* 4.04 Not Found */
    case AIC_LWM2M_CLIENT_RESULT_NOT_FOUND:
        code = NX_LWM2M_CLIENT_COAP_STATUS_NOT_FOUND;
        break;

    /* 4.05 Method Not Allowed */
    case AIC_LWM2M_CLIENT_RESULT_METHOD_NOT_ALLOWED:
        code = NX_LWM2M_CLIENT_COAP_STATUS_METHOD_NOT_ALLOWED;
        break;

    /* 4.06 Not Acceptable */
    case AIC_LWM2M_CLIENT_RESULT_NOT_ACCEPTABLE:
        code = NX_LWM2M_CLIENT_COAP_STATUS_NOT_ACCEPTABLE;
        break;

    /* 5.00 Internal server error */
    case NX_LWM2M_CLIENT_BUFFER_TOO_SMALL:
    case NX_LWM2M_CLIENT_NO_MEMORY:
        code = NX_LWM2M_CLIENT_COAP_STATUS_INTERNAL_SERVER_ERROR;
        break;

    /* Other errors: 4.00 Bad Request */
    case AIC_LWM2M_CLIENT_RESULT_BAD_REQUEST:
    default:
        code = NX_LWM2M_CLIENT_COAP_STATUS_BAD_REQUEST;
        break;
    }

    switch (format) {
    case AIC_LWM2M_CLIENT_FORMAT_TEXT:
        ul_format = NX_LWM2M_CLIENT_COAP_CONTENT_TEXT_PLAIN;
        break;
    case AIC_LWM2M_CLIENT_FORMAT_LINK:
        ul_format = NX_LWM2M_CLIENT_COAP_CONTENT_LINK_FORMAT;
        break;
    case AIC_LWM2M_CLIENT_FORMAT_OPAQUE:
        ul_format = NX_LWM2M_CLIENT_COAP_CONTENT_OCTET_STREAM;
        break;
    case AIC_LWM2M_CLIENT_FORMAT_TLV:
        ul_format = NX_LWM2M_CLIENT_COAP_CONTENT_VND_OMA_LWM2M_TLV;
        break;
    case AIC_LWM2M_CLIENT_FORMAT_JSON:
        ul_format = NX_LWM2M_CLIENT_COAP_CONTENT_VND_OMA_LWM2M_JSON;
        break;
    default:
        return AIC_LWM2M_CLIENT_UNSUPPORTED_CONTENT_FORMAT;
    }

    status = _nx_lwm2m_client_session_send_response(lwm2m_session->client_session,
                code, msg_id, NX_TRUE, token_ptr, token_length, observe, format,
                obj_ptr, object_instances, resource_ptr);
    if (status != NX_SUCCESS)
        return AIC_LWM2M_CLIENT_ERROR;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_session_error_get(AIC_LWM2M_CLIENT_SESSION *lwm2m_session)
{
    u32 error;

    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    error = nx_lwm2m_client_session_error_get(lwm2m_session->client_session);
    switch (error) {
    case NX_SUCCESS:
        return AIC_LWM2M_CLIENT_SUCCESS;
    case NX_LWM2M_CLIENT_SERVER_INSTANCE_DELETED:
        return AIC_LWM2M_CLIENT_SERVER_INSTANCE_DELETED;
    case NX_LWM2M_CLIENT_DTLS_ERROR:
        return AIC_LWM2M_CLIENT_DTLS_ERROR;
    case NX_LWM2M_CLIENT_TIMED_OUT:
        return AIC_LWM2M_CLIENT_TIMED_OUT;
    case NX_LWM2M_CLIENT_ERROR:
    default:
        return AIC_LWM2M_CLIENT_ERROR;
    }
}
/*
u32 aic_lwm2m_register_dev_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session, operation_callback)
{
    if (!lwm2m_session || !lwm2m_session->client_session)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    nx_lwm2m_client_device_callback_set(lwm2m_session->client_session->nx_lwm2m_client_session_client_ptr,
                                        operation_callback);
}*/

/*u32 aic_lwm2m_register_obj_operation_callback(u16 obj_id, NX_LWM2M_CLIENT_OBJECT_OPERATION_CALLBACK_EXT callback)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;

    lwm2m_object = aic_get_lwm2m_object_by_id(obj_id);
    if (!lwm2m_object)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    lwm2m_object->lwm2m_object.object_operation_ext = callback;

    return AIC_LWM2M_CLIENT_SUCCESS;
}*/

u32 aic_lwm2m_register_obj_observe_callback(u16 obj_id, aic_obj_observe_callback callback)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;

    lwm2m_object = aic_get_lwm2m_object_by_id(obj_id);
    if (!lwm2m_object)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    lwm2m_object->object_observe_ptr = callback;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_unregister_obj_observe_callback(u16 obj_id)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;

    lwm2m_object = aic_get_lwm2m_object_by_id(obj_id);
    if (!lwm2m_object)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    lwm2m_object->object_observe_ptr = NULL;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_register_obj_attrs_set_callback(u16 obj_id, aic_obj_attrs_set_callback callback)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;

    lwm2m_object = aic_get_lwm2m_object_by_id(obj_id);
    if (!lwm2m_object)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    lwm2m_object->object_attrs_set = callback;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_unregister_obj_attrs_set_callback(u16 obj_id)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;

    lwm2m_object = aic_get_lwm2m_object_by_id(obj_id);
    if (!lwm2m_object)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    lwm2m_object->object_attrs_set = NULL;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

static u32 aic_lwm2m_client_obj_observe(NX_LWM2M_CLIENT_OBJECT *object_ptr,
                                        NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                        NX_LWM2M_CLIENT_RESOURCE *resource,
                                        u16 mid, char observe_flag)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;
    u16 object_id;

    lwm2m_object = aic_get_lwm2m_object_by_id(object_ptr->object_id);
    if (!lwm2m_object)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    if (lwm2m_object->object_observe_ptr) {
        if (object_ptr)
            object_id = object_ptr->object_id;
        else
            return AIC_LWM2M_CLIENT_PTR_NULL;

        lwm2m_object->object_observe_ptr(object_id, object_instance_ptr,
                                         resource, mid, (boolean)observe_flag);
    }

    return AIC_LWM2M_CLIENT_SUCCESS;

}

static u32 aic_lwm2m_client_obj_attrs_set(NX_LWM2M_CLIENT_REQUEST_OPTIONS *option,
                                          NX_LWM2M_CLIENT_OBJECT *object_ptr,
                                          NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                          NX_LWM2M_CLIENT_RESOURCE *resource, NX_LWM2M_ID msg_id)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;
    u16 object_id;
    AIC_LWM2M_PARA para;
    u8 flag;

    lwm2m_object = aic_get_lwm2m_object_by_id(object_ptr->object_id);
    if (!lwm2m_object)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    if (lwm2m_object->object_attrs_set) {
        if (object_ptr)
            object_id = object_ptr->object_id;
        else
            return AIC_LWM2M_CLIENT_PTR_NULL;

        flag = option->attr_set;
        para.pim = option->pmin;
        para.pmax = option->pmax;
        para.gt = option->gt;
        para.lt = option->lt;
        para.st = option->stp;

        lwm2m_object->object_attrs_set(object_ptr->object_id, object_instance_ptr,
                                       resource, msg_id, &para, flag);
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

static u32 aic_lwm2m_client_obj_operation(u32 operation,
                                   NX_LWM2M_CLIENT_OBJECT *object_ptr,
                                   NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                   NX_LWM2M_CLIENT_RESOURCE *resource,
                                   u32 *resource_count, void *args_ptr,
                                   u32 args_length, u16 msg_id,
                                   const void *token_ptr, u32 token_length)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_object;
    u16 object_id;

    lwm2m_object = aic_get_lwm2m_object_by_id(object_ptr->object_id);
    if (!lwm2m_object)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    if (lwm2m_object->object_operation) {
        if (object_ptr)
            object_id = object_ptr->object_id;
        else
            return AIC_LWM2M_CLIENT_PTR_NULL;

        lwm2m_object->object_operation(operation, object_id, object_instance_ptr,
                                       resource, resource_count,args_ptr,
                                       args_length, msg_id, token_ptr,
                                       token_length);
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_obj_add(u16 object_id, aic_obj_operation_callback object_operation)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_obj;
    u32 ret;

    if (!lwm2m_client.client)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    if (aic_get_lwm2m_object_by_id(object_id))
        return AIC_LWM2M_CLIENT_ALREADY_EXIST;

    lwm2m_obj = aos_zalloc_relaxed(sizeof(AIC_LWM2M_CLIENT_OBJ));
    if (!lwm2m_obj) {
        aicx_error(0x027142, "", "[lwm2m] ERROR: no enough memory\n");
        return AIC_LWM2M_CLIENT_NO_MEMORY;
    }

    lwm2m_obj->object_id = object_id;
    lwm2m_obj->object_operation = object_operation;
    lwm2m_obj->lwm2m_object.object_operation_ext = aic_lwm2m_client_obj_operation;
    lwm2m_obj->lwm2m_object.object_observe = aic_lwm2m_client_obj_observe;
    lwm2m_obj->lwm2m_object.object_attrs = aic_lwm2m_client_obj_attrs_set;
    aic_lwm2m_add_object_to_list(lwm2m_obj);

    switch (object_id) {
    case NX_LWM2M_CLIENT_SECURITY_OBJECT_ID:
    case NX_LWM2M_CLIENT_SERVER_OBJECT_ID:
    case NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_ID:
    case NX_LWM2M_CLIENT_DEVICE_OBJECT_ID:
        return AIC_LWM2M_CLIENT_ALREADY_EXIST;

    case NX_LWM2M_CLIENT_FIRMWARE_OBJECT_ID:
        ret = nx_lwm2m_client_firmware_create(&lwm2m_obj->lwm2m_firmware,
                                              lwm2m_client.client,
                                              NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_HTTP|NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_HTTPS,
                                              NX_NULL, NX_NULL, NX_NULL);
        break;
    default:
        ret = nx_lwm2m_client_object_add(lwm2m_client.client,
                                         &lwm2m_obj->lwm2m_object,
                                         object_id, NULL);
    }

    return ret;
}

u32 aic_lwm2m_client_obj_remove(u16 object_id)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_obj;
    u32 ret;

    if (!lwm2m_client.client)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    lwm2m_obj = aic_get_lwm2m_object_by_id(object_id);
    if (!lwm2m_obj)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    aic_lwm2m_delete_object_from_list(lwm2m_obj);

    switch (object_id) {
    case NX_LWM2M_CLIENT_SECURITY_OBJECT_ID:
    case NX_LWM2M_CLIENT_SERVER_OBJECT_ID:
    case NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_ID:
    case NX_LWM2M_CLIENT_DEVICE_OBJECT_ID:
        return AIC_LWM2M_CLIENT_METHOD_NOT_ALLOWED;

    case NX_LWM2M_CLIENT_FIRMWARE_OBJECT_ID:
    default:
        ret = nx_lwm2m_client_object_remove(lwm2m_client.client,
                                            &lwm2m_obj->lwm2m_object);
    }

    aos_free(lwm2m_obj);
    return ret;
}

u32 aic_lwm2m_client_obj_read(u16 object_id, u16 instance_id, u16 resource_id,
                              NX_LWM2M_CLIENT_RESOURCE *resource,
                              u32 resource_count)
{
    u32 ret;

    if (!lwm2m_client.client)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    ret = nx_lwm2m_client_object_read(lwm2m_client.client, object_id,
                                      instance_id, resource_count, resource);

    return ret;
}

u32 aic_lwm2m_client_obj_discover(u16 object_id, u16 instance_id,
                                  u16 resource_id,
                                  NX_LWM2M_CLIENT_RESOURCE *resource,
                                  u32 *resource_count)
{
    u32 ret;

    if (!lwm2m_client.client)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    ret = nx_lwm2m_client_object_discover(lwm2m_client.client, object_id,
                                          instance_id, resource_count, resource);

    return ret;
}

u32 aic_lwm2m_client_obj_write(u16 object_id, u16 instance_id,
                                  u16 resource_id,
                                  NX_LWM2M_CLIENT_RESOURCE *resource,
                                  u32 resource_count, u32 wriite_flags)
{
    u32 ret;

    if (!lwm2m_client.client)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    ret = nx_lwm2m_client_object_write(lwm2m_client.client, object_id,
                                       instance_id, resource_count, resource,
                                       wriite_flags);

    return ret;
}

u32 aic_lwm2m_client_obj_execute(u16 object_id, u16 instance_id,
                                 u16 resource_id, const u8 * args_ptr,
                                 u32 args_length)
{
    u32 ret;

    if (!lwm2m_client.client)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    ret = nx_lwm2m_client_object_execute(lwm2m_client.client, object_id,
                                         instance_id, resource_id, args_ptr,
                                         args_length);

    return ret;
}

u32 aic_lwm2m_client_obj_create(u16 object_id, u16 *instance_id_ptr, u32 num_values, const NX_LWM2M_CLIENT_RESOURCE *values)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_obj;
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;

    if (!lwm2m_client.client)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    lwm2m_obj = aic_get_lwm2m_object_by_id(object_id);
    if (!lwm2m_obj)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    switch (object_id) {
    case NX_LWM2M_CLIENT_FIRMWARE_OBJECT_ID:
        break;
    case NX_LWM2M_CLIENT_SECURITY_OBJECT_ID:
    case NX_LWM2M_CLIENT_SERVER_OBJECT_ID:
    case NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_ID:
    case NX_LWM2M_CLIENT_DEVICE_OBJECT_ID:
    default:
        ret = nx_lwm2m_client_object_create(lwm2m_client.client, object_id,
                                      instance_id_ptr, num_values, values);
    }

    return ret;
}

u32 aic_lwm2m_client_obj_delete(u16 object_id, u16 instance_id)
{
    //AIC_LWM2M_CLIENT_OBJ *lwm2m_object;

    if (!lwm2m_client.client)
        return AIC_LWM2M_CLIENT_NOT_FOUND;

    switch (object_id) {
    case NX_LWM2M_CLIENT_FIRMWARE_OBJECT_ID:
    case NX_LWM2M_CLIENT_SECURITY_OBJECT_ID:
    case NX_LWM2M_CLIENT_SERVER_OBJECT_ID:
    case NX_LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_ID:
    case NX_LWM2M_CLIENT_DEVICE_OBJECT_ID:
    default:
        nx_lwm2m_client_object_delete(lwm2m_client.client, object_id, instance_id);
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_obj_instance_add(u16 object_id, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, u16 *instance_id_ptr)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_obj;
    u32 ret;

    lwm2m_obj = aic_get_lwm2m_object_by_id(object_id);
    if (!lwm2m_obj)
        return AIC_LWM2M_CLIENT_ERROR;

    ret = nx_lwm2m_client_object_instance_add(&lwm2m_obj->lwm2m_object,
                                              instance_ptr, instance_id_ptr);

    return ret;
}

u32 aic_lwm2m_client_obj_instance_remove(u16 object_id, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_obj;
    u32 ret;

    lwm2m_obj = aic_get_lwm2m_object_by_id(object_id);
    if (!lwm2m_obj)
        return AIC_LWM2M_CLIENT_ERROR;

    ret = nx_lwm2m_client_object_instance_remove(&lwm2m_obj->lwm2m_object,
                                                 instance_ptr);

    return ret;
}

u32 aic_lwm2m_client_obj_resource_changed(u16 object_id,
                                          NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr,
                                          const NX_LWM2M_CLIENT_RESOURCE * resource)
{
    AIC_LWM2M_CLIENT_OBJ *lwm2m_obj;
    u32 ret;

    lwm2m_obj = aic_get_lwm2m_object_by_id(object_id);
    if (!lwm2m_obj)
        return AIC_LWM2M_CLIENT_ERROR;

    ret = nx_lwm2m_client_object_resource_changed(&lwm2m_obj->lwm2m_object,
                                                  instance_ptr, resource);

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_resource_info_get(NX_LWM2M_CLIENT_RESOURCE *resource_ptr,
                                       u16 *resource_id, u32 *operation)
{
    return  nx_lwm2m_client_resource_info_get(resource_ptr, resource_id, (ulong *)operation);
}

u32 aic_lwm2m_client_resource_info_set(NX_LWM2M_CLIENT_RESOURCE *resource_ptr,
                                       u16 resource_id, u32 operation)
{
    return nx_lwm2m_client_resource_info_set(resource_ptr, resource_id, operation);
}

u32 aic_lwm2m_client_resource_set(NX_LWM2M_CLIENT_RESOURCE *resource,
                                  u8 *value, u32 value_length, u8 type)
{
    if (!resource || !value)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    switch (type) {
    case AIC_LWM2M_CLIENT_RESOURCE_NONE:
        return nx_lwm2m_client_resource_info_set(resource, *(NX_LWM2M_ID *)value, value_length);
    case AIC_LWM2M_CLIENT_RESOURCE_STRING:
        return nx_lwm2m_client_resource_string_set(resource, value, value_length);
    case AIC_LWM2M_CLIENT_RESOURCE_INTEGER32:
        return nx_lwm2m_client_resource_integer32_set(resource, *(NX_LWM2M_INT32 *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_INTEGER64:
        return nx_lwm2m_client_resource_integer64_set(resource, *(NX_LWM2M_INT64 *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_FLOAT32:
        return nx_lwm2m_client_resource_float32_set(resource, *(NX_LWM2M_FLOAT32 *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_FLOAT64:
        return nx_lwm2m_client_resource_float64_set(resource, *(NX_LWM2M_FLOAT64 *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_BOOLEAN:
        return nx_lwm2m_client_resource_boolean_set(resource, *(NX_LWM2M_BOOL *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_OPAQUE:
        return nx_lwm2m_client_resource_opaque_set(resource, value, value_length);
    case AIC_LWM2M_CLIENT_RESOURCE_OBJLNK:
        return nx_lwm2m_client_resource_objlnk_set(resource, *(NX_LWM2M_ID *)value, (NX_LWM2M_ID)value_length);
    case AIC_LWM2M_CLIENT_RESOURCE_MULTIPLE:
        return nx_lwm2m_client_resource_instances_set(resource, (NX_LWM2M_CLIENT_RESOURCE *)value, value_length);
    default:
        return AIC_LWM2M_CLIENT_NOT_SUPPORTED;
    }
}

u32 aic_lwm2m_client_resource_get(NX_LWM2M_CLIENT_RESOURCE *resource,
                                  u8 *value, u32 *value_length, u8 type)
{
    if (!resource || !value)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    switch (type) {
    case AIC_LWM2M_CLIENT_RESOURCE_NONE:
        return nx_lwm2m_client_resource_info_get(resource, (u16 *)value, (ulong *)value_length);
    case AIC_LWM2M_CLIENT_RESOURCE_STRING:
        return nx_lwm2m_client_resource_string_get(resource, (const char **)&value, value_length);
    case AIC_LWM2M_CLIENT_RESOURCE_INTEGER32:
        return nx_lwm2m_client_resource_integer32_get(resource, (NX_LWM2M_INT32 *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_INTEGER64:
        return nx_lwm2m_client_resource_integer64_get(resource, (NX_LWM2M_INT64 *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_FLOAT32:
        return nx_lwm2m_client_resource_float32_get(resource, (NX_LWM2M_FLOAT32 *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_FLOAT64:
        return nx_lwm2m_client_resource_float64_get(resource, (NX_LWM2M_FLOAT64 *)value);
    case AIC_LWM2M_CLIENT_RESOURCE_BOOLEAN:
        return nx_lwm2m_client_resource_boolean_get(resource, value);
    case AIC_LWM2M_CLIENT_RESOURCE_OPAQUE:
        return nx_lwm2m_client_resource_opaque_get(resource, (const void **)&value, value_length);
    case AIC_LWM2M_CLIENT_RESOURCE_OBJLNK:
        return nx_lwm2m_client_resource_objlnk_get(resource, (u16 *)value, (u16 *)value_length);
    case AIC_LWM2M_CLIENT_RESOURCE_MULTIPLE:
        return nx_lwm2m_client_resource_instances_get(resource,
                                                      (NX_LWM2M_CLIENT_RESOURCE *)value,
                                                      value_length);
    default:
        return AIC_LWM2M_CLIENT_NOT_SUPPORTED;
    }
}

/*
u32 aic_lwm2m_client_resource_add(u8 *ptr, u8 *ptr_max, u8 type,
                                  const NX_LWM2M_CLIENT_RESOURCE *resource,
                                  u32 *tlv_length)
{
    alog_info("[lwm2m] add resource\n");
    return _nx_lwm2m_client_add_resource(ptr, ptr_max, type, resource, tlv_length);
}*/

u32 aic_lwm2m_client_endpoint_set(u8 *epname, u32 name_len)
{
    if (!epname)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    if (!lwm2m_client.client) {
        aos_memcpy(lwm2m_client.endpoint, epname, name_len);
        lwm2m_client.endpoint_name_length = name_len;
    } else {
        aos_memset((char *)lwm2m_client.client->nx_lwm2m_client_name, 0,
                   lwm2m_client.client->nx_lwm2m_client_name_length);
        aos_memcpy((char *)lwm2m_client.client->nx_lwm2m_client_name, epname, name_len);
        lwm2m_client.client->nx_lwm2m_client_name_length = name_len;
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_bindmode_set(u32 bindmode)
{
    NX_LWM2M_CLIENT_RESOURCE resource;
    char *binding_mode = NULL;
    u32 ret;

    lwm2m_client.bind_mode = bindmode;

    if (bindmode == NX_LWM2M_CLIENT_BINDING_UQ)
        binding_mode = "UQ";
    else
        binding_mode = "U";

    nx_lwm2m_client_resource_info_set(&resource, NX_LWM2M_CLIENT_SERVER_BINDING_ID, NX_NULL);
    nx_lwm2m_client_resource_string_set(&resource, binding_mode, strlen(binding_mode));
    ret = nx_lwm2m_client_object_write(lwm2m_client.client,
                                 NX_LWM2M_CLIENT_SERVER_OBJECT_ID, 0,
                                 NX_LWM2M_CLIENT_SERVER_BINDING_ID,
                                 &resource, NX_LWM2M_CLIENT_OBJECT_WRITE_UPDATE);
    nx_bindmode = bindmode;
    lwm2m_client.client->nx_lwm2m_client_device.device_object_binding_modes = bindmode;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_authcode_set(u8 *authcode, u32 authcode_len)
{
    if (!authcode)
        return AIC_LWM2M_CLIENT_PTR_NULL;

    aos_memcpy(lwm2m_client.authcode, authcode, authcode_len);
    lwm2m_client.authcode_len = authcode_len;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

/* ensure to set before registration */
u32 aic_lwm2m_client_dtls_conf_set(u32 time_period, u8 enable)
{
    aic_dtls_cfg.enable = enable;
    aic_dtls_cfg.time_period = time_period;

    return AIC_LWM2M_CLIENT_SUCCESS;
}

void aic_lwm2m_client_set_msg_id(u16 id)
{
    _nx_lwm2m_client_set_msg_id(id);
}

static u32 aic_lwm2m_client_init()
{
    struct aic_network_ip_instance *ip_instance;
    u32 ret;

    aicx_info(0x1123f0, "", "[lwm2m] client init\n");

    ip_instance = aic_get_pdp_ip_instance();
    if (!ip_instance || !ip_instance->ip_ptr) {
        aicx_info(0x14c7f4, "", "[lwm2m] pdp not activated\n");
        return AIC_LWM2M_CLIENT_PDP_NOT_ACTIVATE;
    } else {
        lwm2m_client.lwm2m_ip = ip_instance->ip_ptr;
    }

    lwm2m_client.client = aos_zalloc(sizeof(NX_LWM2M_CLIENT));
    ret = nx_lwm2m_client_create(lwm2m_client.client, lwm2m_client.lwm2m_ip,
                                 lwm2m_client.lwm2m_ip->nx_ip_default_packet_pool,
                                 lwm2m_client.endpoint,
                                 lwm2m_client.endpoint_name_length, NX_NULL, 0,
                                 lwm2m_client.authcode, lwm2m_client.authcode_len,
                                 lwm2m_client.bind_mode, lwm2m_client_stack,
                                 SIZE_4K, 15);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aos_free(lwm2m_client.client);
        aicx_info(0x016d30, "x", "[lwm2m] client create fail, ret 0x%x\n", ret);
        return AIC_LWM2M_CLIENT_ERROR;
    }

    /*aic_lwm2m_event = aos_create_event("AT Socket listen event flags");
    if (!aic_lwm2m_event) {
        alog_error("[AT_TCP]: at socket event create fail\n");
        aos_free(lwm2m_client.client);
        return AIC_LWM2M_CLIENT_ERROR;
    }*/

    aicx_info(0x0dbb35, "", "[lwm2m] client create success\n");
    return AIC_LWM2M_CLIENT_SUCCESS;
}

static void aic_lwm2m_client_uninit()
{
    nx_lwm2m_client_delete(lwm2m_client.client);
    aic_lwm2m_delete_all_object_from_list();
    aos_free(lwm2m_client.client);
    aos_memset(&lwm2m_client, 0, sizeof(AIC_LWM2M_CLIENT));
    //aos_delete_event(aic_lwm2m_event);
    //aic_lwm2m_event = NULL;
}

