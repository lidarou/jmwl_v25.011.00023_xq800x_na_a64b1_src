
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05159a
#define LOG_MODULE    AMOD_NET

#include "aic_lwm2m_client.h"
#include "aic_init.h"

#define NX_LWM2M_CLIENT_STACK_PRIORITY  15
#define USE_BOOTSTRAP     1
//#define USE_DTLS          1
#define AIC_LWM2M_REGISETR_EVENT  0x1

#if USE_BOOTSTRAP
#define AIC_LWM2M_DTLS_KEY        "1234abcd"
#define AIC_LWM2M_DTLS_IDENTITY   "868039050203644"
#else
#define AIC_LWM2M_DTLS_KEY        "2X5FYBTgpEUgN78x"
#define AIC_LWM2M_DTLS_IDENTITY   "lwm2m_868039050203644"
#endif
/* Set up client thread and network resources. */
aos_thread_id       aic_lwm2m_client_thread = AOS_INVALID_THREAD_ID;
aos_event_ptr       aic_lwm2m_client_event;

static void aic_lwm2m_pdp_state_change_callback(u8 pdp_cid, u8 pdp_state);
static void aic_lwm2m_client_thread_entry(void* thread_input);


int aic_lwm2m_client_test_init(void)
{
    aic_register_pdp_state_change_cb(aic_lwm2m_pdp_state_change_callback);
    return AIC_LWM2M_CLIENT_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_lwm2m_client_test_init);

u32 aic_lwm2m_client_imei_get(u8 *buf)
{
    AT_RESP_FROM_CP at_resp;
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16 cnt;

    at_resp.resp_buf = aos_zalloc(AT_MAX_RESP_LENGTH + 1);
    at_resp.buf_len = AT_MAX_RESP_LENGTH + 1;
    aic_send_cmd_and_wait("AT+GSN=1", &at_resp);
    if (strstr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR) ||
        strstr(at_resp.resp_buf, AT_SIMPLE_RESULT_ERROR)) {
        aicx_error(0x0cb743, "", "[lwm2m] get imei fail.\n");
        aos_free(at_resp.resp_buf);
        return AIC_LWM2M_CLIENT_GET_IMEI_ERROR;
    }

    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
    if (!line_arr) {
        aicx_error(0x097a8d, "", "[lwm2m] line_arr is null.\n");
        aos_free(at_resp.resp_buf);
        return AIC_LWM2M_CLIENT_GET_IMEI_ERROR;
    }

    /* parse imei info */
    para_arr = aic_split_paras_to_str_array((u8 *)line_arr[0],
                                            "+GSN: ",
                                            &cnt);
    if (para_arr && cnt == 1) {
        strncpy(buf, (u8 *)para_arr[0], strlen((u8 *)para_arr[0]));
    }

    aic_free_paras_mem(para_arr, cnt);
    aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);
    aos_free(at_resp.resp_buf);
    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_client_imsi_get(u8 *buf)
{
    AT_RESP_FROM_CP at_resp;
    ulong *line_arr = AIC_NULL;

    at_resp.resp_buf = aos_zalloc(AT_MAX_RESP_LENGTH + 1);
    at_resp.buf_len = AT_MAX_RESP_LENGTH + 1;
    aic_send_cmd_and_wait("AT+CIMI", &at_resp);
    if (strstr(at_resp.resp_buf, AT_VERBOSE_RESULT_ERROR)) {
        aicx_error(0x10e1b9, "", "[lem2m] get imsi fail!\n");
        aos_free(at_resp.resp_buf);
        return AIC_LWM2M_CLIENT_GET_IMSI_ERROR;
    }

    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
    if (!line_arr) {
        aicx_error(0x097a8d, "", "[lwm2m] line_arr is null.\n");
        aos_free(at_resp.resp_buf);
        return AIC_LWM2M_CLIENT_GET_IMSI_ERROR;
    }

    /*imsi max length is 15 */
    strncpy(buf, (char *)line_arr[0], strlen((char *)line_arr[0]));
    aicx_info(0x1a747f, "s", "[lwm2m] sim_imsi = %s.\n", buf);

    /* free params */
    aic_free_paras_mem(line_arr, at_resp.resp_line_cnt);
    aos_free(at_resp.resp_buf);

    return AIC_LWM2M_CLIENT_SUCCESS;
}

static void aic_lwm2m_pdp_state_change_callback(u8 pdp_cid, u8 pdp_state)
{
    //struct aic_network_ip_instance *ip_instance;

    if(pdp_cid != ANET_DEFAULT_PDP_CID || pdp_state != 1) {
        aicx_error(0x18e402, "", "[lwm2m_test]:PDP is not activated\n");
        return;
    }

    if (aic_lwm2m_client_thread != AOS_INVALID_THREAD_ID) {
        aicx_error(0x099e2e, "", "[lwm2m_test]:create thread already\n");
        aos_resume_thread(aic_lwm2m_client_thread);
        return;
    }

    /* Create the client thread */
    aic_lwm2m_client_thread = aos_create_thread("LWM2M Client Thread Test",
                                                aic_lwm2m_client_thread_entry,
                                                NULL, SIZE_4K, 0, 0,
                                                NX_LWM2M_CLIENT_STACK_PRIORITY,
                                                AOS_DONT_START);
    /* Check for errors */
    if (aic_lwm2m_client_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x12c59e, "", "[lwm2m_test]:create thread error\n");
        return;
    }

    /* Create the event flags. */
    aic_lwm2m_client_event = aos_create_event("LWM2M event flags");
    /* Check for errors */
    if (!aic_lwm2m_client_event) {
        aicx_error(0x081807, "", "[lwm2m_test]:create event error\n");
        aos_delete_thread(aic_lwm2m_client_thread);
        aic_lwm2m_client_thread = AOS_INVALID_THREAD_ID;
        return;
    }

    aos_resume_thread(aic_lwm2m_client_thread);
    return;
}

void aic_lwm2m_session_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session, UINT state)
{

    aicx_info(0x081f43, "d", "[lwm2m_test] Callback: -> %d\n", state);
    switch (state) {
    case NX_LWM2M_CLIENT_SESSION_INIT:
    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_WAITING:
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_REQUESTING:
        aicx_info(0x1af99d, "", "[lwm2m_test] Start client initiated bootstrap\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_INITIATED:
        aicx_info(0x1ae566, "", "[lwm2m_test] Got message from boostrap server\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_FINISHED:
        aos_set_event(aic_lwm2m_client_event, AIC_LWM2M_REGISETR_EVENT, TX_OR);
         /* Bootstrap session done, we can register to the LWM2M Server */
        aicx_info(0x1598ce, "", "[lwm2m_test] Boostrap finished.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_ERROR:
        /* Failed to Bootstrap the LWM2M Client. */
        aicx_info(0x1e590b, "x", "[lwm2m_test] Failed to boostrap device, error=0x%02x\n",
                  aic_lwm2m_client_session_error_get(lwm2m_session));
        break;

    case NX_LWM2M_CLIENT_SESSION_REGISTERED:
        /* Registration to the LWM2M Client done. */
        aicx_info(0x06c429, "", "[lwm2m_test] LWM2M device registered.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_REGISTERING:
    case NX_LWM2M_CLIENT_SESSION_UPDATING:
    case NX_LWM2M_CLIENT_SESSION_DISABLING:
    case NX_LWM2M_CLIENT_SESSION_DEREGISTERING:
        break;

    case NX_LWM2M_CLIENT_SESSION_DISABLED:
        /* Registration to the LWM2M Client done. */
        aicx_info(0x0470aa, "", "[lwm2m_test] LWM2M device disabled.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_DEREGISTERED:
        /* Registration to the LWM2M Client done. */
        aicx_info(0x072180, "", "[lwm2m_test] LWM2M device deregistered.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_ERROR:
        /* Failed to register to the LWM2M Client. */
        aicx_info(0x1413b3, "x", "[lwm2m_test] Failed to register device, error=0x%02x\n",
                  aic_lwm2m_client_session_error_get(lwm2m_session));
        break;

    default:
        aicx_info(0x175d11, "", "[lwm2m_test] not support state");
        return;
    }

    return;
}

#if 0
static u32 aic_lwm2m_obj_operation(u32 operation, u16 object_id,
                             u16 object_instance_id, u16 resource_id, u16 mid)
{
    switch (operation) {
    case NX_LWM2M_CLIENT_OBJECT_READ:
        aicx_info(0x103713, "", "[lwm2m_test] recv obj read msg\n");
        break;
    case NX_LWM2M_CLIENT_OBJECT_WRITE:
        aicx_info(0x0f2ada, "", "[lwm2m_test] recv obj write msg\n");
        break;
    case NX_LWM2M_CLIENT_OBJECT_EXECUTE:
        aicx_info(0x0f62f2, "", "[lwm2m_test] recv obj execute msg\n");
        break;
    case NX_LWM2M_CLIENT_OBJECT_DISCOVER:
        aicx_info(0x0fe501, "", "[lwm2m_test] recv obj discover msg\n");
        break;
    case NX_LWM2M_CLIENT_OBJECT_CREATE:
        aicx_info(0x15471c, "", "[lwm2m_test] recv obj create msg\n");
        break;
    case NX_LWM2M_CLIENT_OBJECT_DELETE:
        aicx_info(0x0c59e4, "", "[lwm2m_test] recv obj delete msg\n");
        break;
    default:
        aicx_info(0x0b6e13, "", "[lwm2m_test] recv obj other msg\n");
        break;
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}
#endif

#ifdef USE_BOOTSTRAP
/* LwM2M server uri parse.  */
static u32 uri_parse(const char *uri, NXD_ADDRESS *ip_addr, UINT *udp_port, bool is_dtls)
{
    u32 status, i, dot_count = 0, temp = 0, ip_address = 0;
    u8 address_found = NX_FALSE;
    NX_DNS *dns_client;
    CHAR host_buffer[256];

    /* coap-URI = "coap:" "//" host [ ":" port ] path-abempty [ "?" query ]
       coaps-URI = "coaps:" "//" host [ ":" port ] path-abempty [ "?" query ] */
    if (memcmp(uri, "coap://", sizeof("coap://") - 1) == 0) {
        /* Set the host ptr.  */
        uri += (sizeof("coap://") - 1);
        *udp_port = LWM2M_SERVER_PORT;
    } else if (memcmp(uri, "coaps://", sizeof("coaps://") - 1) == 0) {
        /* Set the host ptr.  */
        uri += (sizeof("coaps://") - 1);
        *udp_port = LWM2M_SERVER_DTLS_PORT;
    } else {
        return AIC_LWM2M_CLIENT_NOT_SUPPORTED;
    }

    /* Try to detect whether the host is numerical IP address. */
    for (i = 0; ; i++) {
        if (uri[i] >= '0' && uri[i] <= '9') {
            temp = temp * 10 + (uri[i] - '0');
            if (temp > 0xFF ) {
                break;
            }
        } else if (uri[i] == '.') {
            if (dot_count++ == 3)
            {
                break;
            }
            ip_address = (ip_address << 8) + temp;
            temp = 0;
        } else if (uri[i] == ':' || uri[i] == '/' || uri[i] == '\0') {
            if (dot_count != 3) {
                break;
            }
            ip_address = (ip_address << 8) + temp;

            /* Set the address.  */
            ip_addr->nxd_ip_version = NX_IP_VERSION_V4;
            ip_addr->nxd_ip_address.v4 = ip_address;
            address_found = NX_TRUE;
            break;
        } else {
            break;
        }
    }

    /* Split host and port. */
    while(1) {
        if (uri[i] == ':' || uri[i] == '/' || uri[i] == '\0') {

            /* Store the host uri.  */
            if (i == 0 || i >= 255) {
                return AIC_LWM2M_CLIENT_NOT_SUPPORTED;
            }
            memcpy(host_buffer, uri, i); /* Use case of memcpy is verified. */
            host_buffer[i] = NX_NULL;

            if (uri[i] == ':') {
                temp = 0;
                i++;

                /* Get port number. */
                while (uri[i] >= '0' && uri[i] <= '9') {
                    temp = temp * 10 + (uri[i++] - '0');
                    if (temp > 0xFFFF) {
                        break;
                    }
                }

                if (temp > 0 && temp <= 0xFFFF) {
                    *udp_port = temp;
                }
            }
            break;
        }

        i++;
    }

    /* Check if found the address.  */
    if (address_found == NX_FALSE) {
        dns_client = aic_get_dns_client();
        /* Resolve the host name by DNS.  */
        status = nxd_dns_host_by_name_get(dns_client, (UCHAR *)host_buffer,
                                          ip_addr, NX_IP_PERIODIC_RATE,
                                          NX_IP_VERSION_V4);
        if (status) {
            aicx_info(0x1acd54, "x", "[lwm2m] uri parse fail, status 0x%x\n", status);
            return AIC_LWM2M_CLIENT_ERROR;
        }
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}
#endif

static void aic_lwm2m_client_thread_entry(void* thread_input)
{
    int ret;
    struct addrinfo *res, *dns_res;
    NXD_ADDRESS destaddr1, destaddr2, *dest;
    AIC_LWM2M_CLIENT_SESSION aic_lwm2m_session = {0};
    u32 offset;
    u8 epname[64] = {0};
    u16 server_id = 1;  /*range: 1-65535*/
#ifdef USE_DTLS
    u8 *pub_key_or_id = NX_NULL;
    u32 pub_key_or_id_len = 0;
    u8 *secret_key = NX_NULL;
    u32 secret_key_len = 0;
#endif
#ifdef USE_BOOTSTRAP
    u8 *server_uri = NX_NULL;
    u32 server_uri_len = 0;
    u8 security_mode = 0;
    NXD_ADDRESS server_addr;
    u16 udp_port;
    u32 events = 0;
#endif

#ifdef USE_BOOTSTRAP
    ret = getaddrinfo(LWM2M_SERVER_BT, AIC_NULL, AIC_NULL, &res);
#else
    ret = getaddrinfo(LWM2M_SERVER, AIC_NULL, AIC_NULL, &res);
#endif
    if (ret != NX_SUCCESS || !res) {
        aicx_info(0x18b11f, "d", "[lwm2m_test] getaddrinfo fail, ret %d\n", ret);
        return;
    }

    dns_res = res;
    while (dns_res) {
        if (dns_res->ai_addr->sa_family == AF_INET) {
            destaddr1.nxd_ip_version = NX_IP_VERSION_V4;
            aos_memcpy(&destaddr1.nxd_ip_address.v4, &dns_res->ai_addr->sa_data[2], 4);
            NX_CHANGE_ULONG_ENDIAN(destaddr1.nxd_ip_address.v4);
            break;
        }
        dns_res = dns_res->ai_next;
    }

    dns_res = res;
    while (dns_res) {
        if (dns_res->ai_addr->sa_family == AF_INET6) {
            destaddr2.nxd_ip_version = NX_IP_VERSION_V6;
            aos_memcpy(destaddr2.nxd_ip_address.v6,
                       &(((struct sockaddr_in6*)dns_res->ai_addr)->sin6_addr), 16);
            NX_CHANGE_ULONG_ENDIAN(destaddr2.nxd_ip_address.v6[0]);
            NX_CHANGE_ULONG_ENDIAN(destaddr2.nxd_ip_address.v6[1]);
            NX_CHANGE_ULONG_ENDIAN(destaddr2.nxd_ip_address.v6[2]);
            NX_CHANGE_ULONG_ENDIAN(destaddr2.nxd_ip_address.v6[3]);
            break;
        }
        dns_res = dns_res->ai_next;
    }
    freeaddrinfo(res);

    if (destaddr1.nxd_ip_version == NX_IP_VERSION_V4)
        dest = &destaddr1;
    else if (destaddr2.nxd_ip_version == NX_IP_VERSION_V6)
        dest = &destaddr2;
    else
        dest = NULL;

    if (!dest)
        return;

    aos_sleep(3000);
    aic_lwm2m_client_imei_get(epname);
    offset = strlen(epname);
    epname[offset] = ';';
    aic_lwm2m_client_imsi_get(&epname[offset + 1]);
    aicx_info(0x0fe1f9, "s", "[lwm2m_test] epname is %s\n", epname);
    aic_lwm2m_client_endpoint_set(epname, strlen(epname));
    aic_lwm2m_client_bindmode_set(NX_LWM2M_CLIENT_BINDING_U);

    ret = aic_lwm2m_client_create(&aic_lwm2m_session,
                                  aic_lwm2m_session_callback);
    if (ret) {
        aicx_info(0x045bb0, "x", "[lwm2m_test] create client fail,ret 0x%x\n", ret);
        return;
    }

    aicx_info(0x1c1141, "", "[lwm2m_test] create client success\n");
#ifdef USE_DTLS
    secret_key_len = strlen(AIC_LWM2M_DTLS_KEY);
    secret_key = AIC_LWM2M_DTLS_KEY;
    //aos_memcpy(secret_key, LWM2M_DTLS_KEY, secret_key_len);
    pub_key_or_id_len = strlen(AIC_LWM2M_DTLS_IDENTITY);
    pub_key_or_id = AIC_LWM2M_DTLS_IDENTITY;
    //aos_memcpy(pub_key_or_id, LWM2M_DTLS_IDENTITY, pub_key_or_id_len);
#ifdef USE_BOOTSTRAP
    ret = aic_lwm2m_client_bootstrap(&aic_lwm2m_session, dest, LWM2M_SERVER_DTLS_PORT,
                                     secret_key, secret_key_len, pub_key_or_id,
                                     pub_key_or_id_len, AIC_TRUE);
    if (ret) {
        aicx_info(0x1ad5ae, "x", "[lwm2m_test] client bootstrap fail,ret 0x%x\n", ret);
        aic_lwm2m_client_delete(&aic_lwm2m_session);
        return;
    }

    ret = aos_get_event(aic_lwm2m_client_event, AIC_LWM2M_REGISETR_EVENT,
                           TX_OR_CLEAR, &events, AOS_WAIT_FOREVER);
    if (ret != NX_SUCCESS)
        return;

    ret = aic_lwm2m_client_session_register_info_get(&aic_lwm2m_session,
                                                     NX_LWM2M_CLIENT_RESERVED_ID,
                                                     &server_id, &server_uri,
                                                     &server_uri_len, &security_mode,
                                                     &pub_key_or_id, &pub_key_or_id_len,
                                                     NULL, NULL,
                                                     &secret_key, &secret_key_len);
    if (ret) {
        aicx_info(0x0c2e95, "", "[lwm2m_test] client register info get fail\n");
        aic_lwm2m_client_delete(&aic_lwm2m_session);
        return;
    }

    ret = uri_parse(server_uri, &server_addr, (u32 *)&udp_port, AIC_TRUE);
    if (ret)
    {
        return;
    }
    dest = &server_addr;
    aos_sleep(1000);
#endif
    ret = aic_lwm2m_client_register(&aic_lwm2m_session, dest, LWM2M_SERVER_DTLS_PORT,
                                    server_id, secret_key, secret_key_len,
                                    pub_key_or_id, pub_key_or_id_len, AIC_TRUE);

#else
#ifdef USE_BOOTSTRAP
    ret = aic_lwm2m_client_bootstrap(&aic_lwm2m_session, dest, LWM2M_SERVER_PORT,
                                      NULL, 0, NULL, 0, AIC_FALSE);
    if (ret) {
        aicx_info(0x1ad5ae, "x", "[lwm2m_test] client bootstrap fail,ret 0x%x\n", ret);
        aic_lwm2m_client_delete(&aic_lwm2m_session);
        return;
    }

    ret = aos_get_event(aic_lwm2m_client_event, AIC_LWM2M_REGISETR_EVENT,
                        TX_OR_CLEAR, &events, AOS_WAIT_FOREVER);
    if (ret != NX_SUCCESS)
        return;

    ret = aic_lwm2m_client_session_register_info_get(&aic_lwm2m_session,
                                                     NX_LWM2M_CLIENT_RESERVED_ID,
                                                     &server_id, &server_uri,
                                                     &server_uri_len, &security_mode,
                                                     NULL, NULL, NULL, NULL, NULL, NULL);
    if (ret) {
        aicx_info(0x0c2e95, "", "[lwm2m_test] client register info get fail\n");
        aic_lwm2m_client_delete(&aic_lwm2m_session);
        return;
    }

    ret = uri_parse(server_uri, &server_addr, (u32 *)&udp_port, AIC_TRUE);
    if (ret)
    {
        return;
    }
    dest = &server_addr;
#endif
    aic_lwm2m_client_update_lifetime(3000);
    ret = aic_lwm2m_client_register(&aic_lwm2m_session, dest, LWM2M_SERVER_PORT,
                                    server_id, NULL, 0, NULL, 0, AIC_FALSE);
    if (ret) {
        aicx_info(0x13666d, "x", "[lwm2m_test] client register fail,ret 0x%x\n", ret);
        aic_lwm2m_client_delete(&aic_lwm2m_session);
        return;
    }
#endif

    aicx_info(0x1c4990, "", "[lwm2m_test] client login success\n");
    while (1) {
        aos_sleep(1000 * 60 * 60);
        aicx_info(0x02be61, "", "[lwm2m_test] client logout\n");
        aic_lwm2m_client_logout(&aic_lwm2m_session);
        aic_lwm2m_client_delete(&aic_lwm2m_session);
        return;
    }
}



