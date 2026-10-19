/*                                                                        */
/**************************************************************************/
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Implementation of AT Cmds regarding LWM2M of China Mobile (CM).
 *  This file implements some AT commands relating to LWM2M of CM,
 *  such as create device, register/deregister, add obj, del obj, and so on.
 *  Depends on:
 *      macros: [optional] CONFIG_LWM2M_CM_DEFAULT_HOST
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0993f4
#define LOG_MODULE    AMOD_NET

/*******************************************
 *              INCLUDES
 ******************************************/
#include "aic_at_cmd_handle.h"
#include "aic_at_process_common.h"
#include "aic_at_resp_handle.h"
#include "aic_at_tcpip_common.h"
#include "aic_list.h"
#include "cm_at_mlwm2m_obj_mgr.h"
#include "cm_at_split_paras.h"
#include <aic_errno.h>
#include <aic_init.h>

/*******************************************
 *              DEFINES
 ******************************************/
/* configures */
#ifdef CONFIG_LWM2M_CM_DEFAULT_HOST
#define LWM2M_DEFAULT_HOST CONFIG_LWM2M_CM_DEFAULT_HOST
#else
#define LWM2M_DEFAULT_HOST "nbiotbt.heclouds.com"
#endif
#define LWM2M_AT_AUTHCODE_SIZE_MAX 16
#define LWM2M_AT_PSK_SIZE_MAX 16
#define LWM2M_AT_PSK_ID_SIZE_MAX 30
#define LWM2M_AT_IMEI_SIZE_MAX 15
#define LWM2M_AT_IMSI_SIZE_MAX 15
#define LWM2M_AT_HOST_SIZE_MAX 128
#define LWM2M_AT_ACK_ID_MAX 10
#define LWM2M_AT_LIFETIME_DEFAULT 86400
#define MIPLOPEN_TIMEOUT_DEFAULT 30
#define MIPLDTLSNAT_TIMEOUT_DEFAULT 120
#define LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT 40
#define LWM2M_AT_DEFAULT_RES_OPERAIONS NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ_WRITE
#define LWM2M_CM_TASK_STK_SIZE 4 * 1024
#define LWM2M_CM_TASK_PRIORITY 16
/*******************************************
 *              TYPEDEFS
 ******************************************/
enum LWM2M_CM_EVENT {
    LWM2M_CM_EVENT_REGISTER = 0x00000001,
    LWM2M_CM_EVENT_AUTO_UPDATE = 0x00000002,

    LWM2M_CM_EVENT_TERM_TASK = 0x10000000,
};

enum LWM2M_CM_SERVER_STATE {
    LWM2M_CM_SERVER_STATE_NONE,
    LWM2M_CM_SERVER_STATE_BOOTSTRAP,
    LWM2M_CM_SERVER_STATE_LWM2M,
};

/* +CIS ERROR codes */
enum LWM2M_CM_AT_ERROR_CODE {
    LWM2M_CM_INVAL_AT_PARAM = 50,
    LWM2M_CM_UNKNOW_ERR = 100,
    LWM2M_CM_GRAMMAR_SYNTAX_ERR = 601,
    LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS = 602,
    LWM2M_CM_OPERATION_NOT_ALLOWED = 651,
    LWM2M_CM_UPLINK_BUSY = 652,
    LWM2M_CM_RESOURCE_OPERATION_ERR = 653
};

enum LWM2M_CM_LWM2M_CM_RES_FLAG {
    LWM2M_CM_RES_FLAG_LAST_MSG = 0,
    LWM2M_CM_RES_FLAG_FIRST_MSG = 1,
    LWM2M_CM_RES_FLAG_INTER_MSG = 2,
    LWM2M_CM_RES_FLAG_ENTER_IDLE_AFTER_SENT = 0x200,
    LWM2M_CM_RES_FLAG_ENTER_IDLE_AFTER_RECV_ACK = 0x300
};

enum LWM2M_CM_MIPLEVENT_EVTID {
    MIPLEVENT_EVTID_NONE = 0,
    MIPLEVENT_EVTID_BOOTSTRAP_SUCCESS = 2,
    MIPLEVENT_EVTID_BOOTSTRAP_FAIL = 3,
    MIPLEVENT_EVTID_REGISTERED_SUCCESS = 6,
    MIPLEVENT_EVTID_REGISTERED_FAIL = 7,
    MIPLEVENT_EVTID_UPDATE_SUCCESS = 11,
    MIPLEVENT_EVTID_DEREGISTERED_SUCCESS = 15,
    MIPLEVENT_EVTID_NOTIFYING = 26,
    MIPLEVENT_EVTID_DEV_DELETED_BY_PLATFORM = 31,
    MIPLEVENT_EVTID_DTLS_IP_EXPIRED = 32,
    MIPLEVENT_EVTID_RECV_LEN_TOO_LARGE = 33
};

enum LWM2M_CM_MIPLCFG_PLATFORM {
    MIPLCFG_PLATFORM_ONENET = 0, /* china mobile */
    MIPLCFG_PLATFORM_CTWING = 1, /* china telecom */
    MIPLCFG_PLATFORM_DMP = 2,    /* china unicom */
    MIPLCFG_PLATFORM_HWIOT = 3,  /* Huawei IOT */
    MIPLCFG_PLATFORM_CNT,        /* placeholder for available platform count */
    MIPLCFG_PLATFORM_OTHER = 10, /* Other platform */
};

enum LWM2M_CM_MIPLCFG_PATTERN {
    MIPLCFG_PATTERN_DEFAULT = 0,
    MIPLCFG_PATTERN_RAW_IMEI = 1,
    MIPLCFG_PATTERN_RAW_IMEI_IMSI = 2,
    MIPLCFG_PATTERN_URN_IMEI = 3,
    MIPLCFG_PATTERN_URN_IMEI_IMSI = 4,
    /* only available when platform is 10 */
    MIPLCFG_PATTERN_SPECIFIC_ENDPOINT = 0xff
};

enum LWM2M_CM_MIPLCFG_BINDING_MODE {
    MIPLCFG_BM_DEFAULT = 0,
    MIPLCFG_BM_UDP = 1,
    MIPLCFG_BM_UDP_QUEUE = 2,

    MIPLCFG_BM_TOTAL
};

enum LWM2M_CM_MIPLNMI_NNMI {
    MIPLNMI_NNMI_DEFAULT = 0, /* directy report to MCU */
    MIPLNMI_NNMI_CACHE = 1,   /* cache the commands and report +MIPLNMI */
    MIPLNMI_NNMI_SILENT = 2,  /* don't report anything */

    MIPLNMI_NNMI_TOTAL
};

enum LWM2M_CM_MIPLCREATEEX_PSK_ENCODE {
    MIPLCREATEEX_PSK_ENCODE_DEFAULT = 0,
    MIPLCREATEEX_PSK_ENCODE_HEX = 1,
};

typedef struct {
    void *data;
    UINT len;
} LWM2M_CM_TOKEN;

typedef struct {
    struct list_head node;
    u16 msg_id;
    LWM2M_CM_TOKEN token_entry;
} LWM2M_CM_TOKEN_ENTRY;

typedef struct {
    struct list_head list;
    aos_mutex_ptr mutex;
} LWM2M_CM_TOKEN_ENTRY_LIST;

typedef struct aic_lwm2m_cm_at_cfg {
    u32 miplopen_lifetime;
    u32 miplopen_timeout;
    u32 mipldtlsnat_timeout;
    u32 miplcfg_bindmode;
    u8 miplcfg_pattern;
    u8 miplnmi_nnmi;
    boolean enable_moniter;
    boolean miplnmi_nsmi;
    boolean miplupdateset_enable;
} LWM2M_CM_AT_CFG;

typedef struct {
    char *text;
    u32 payload_size;
    u16 mid;
} LWM2M_CM_NNMI_ENTRY;

typedef struct {
    u16 head;
    u16 total;
    u32 dropped;
    aos_mutex_ptr mutex;
    LWM2M_CM_NNMI_ENTRY nnmi_entries[LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT];
} LWM2M_CM_CACHE_NNMI;

typedef struct {
    NXD_ADDRESS addr;
    u16 port;
} LWM2M_CM_SERVER_INFO;

typedef struct {
    /* bootstrap related */
    u8 bs_key[LWM2M_AT_PSK_SIZE_MAX + 1];
    u8 bs_key_size;
    u8 bs_identity[LWM2M_AT_PSK_ID_SIZE_MAX + 1];
    u8 bs_identity_size;
    /* lwm2m related */
    u8 lwm2m_key[LWM2M_AT_PSK_SIZE_MAX + 1];
    u8 lwm2m_key_size;
    u8 lwm2m_identity[LWM2M_AT_PSK_ID_SIZE_MAX + 1];
    u8 lwm2m_identity_size;
    u8 authcode[LWM2M_AT_AUTHCODE_SIZE_MAX + 1];
} LWM2M_CM_DTLS_INFO;

typedef struct {
    u16 ack_id[LWM2M_AT_ACK_ID_MAX];
    u16 total;
    aos_mutex_ptr mutex;
} LWM2M_CM_ACK_ID_LIST;

typedef struct {
    u16 last_mid;
    u32 last_tickcnt;
} LWM2M_CM_RETRANS_INFO;

typedef struct {
    /*----------- APP RELATED -----------*/
    /* working thread id */
    aos_thread_id thread_id;
    /* event for working thread */
    aos_event_ptr event;
    /* com id for urc */
    u8 com_id;

    /*----------- CLIENT RELATED -----------*/
    /* configuration of client */
    LWM2M_CM_AT_CFG config;
    /* lwm2m client used in service */
    AIC_LWM2M_CLIENT_SESSION client_session;
    /* host: either domain name or ip addr */
    u8 host[LWM2M_AT_HOST_SIZE_MAX];
    /* dtls related info */
    LWM2M_CM_DTLS_INFO dtls_info;
    /* ip addr type. bootstrap or lwm2m depends on is_bootstrap*/
    LWM2M_CM_SERVER_INFO init_server;
    /* obj entry list */
    LWM2M_CM_OBJ_ENTRY_LIST obj_entry_list;
    /* initiated server type */
    boolean is_bootstrap;
    /* bootstrap state */
    boolean bootstrap_state;
    /* initiated server is dtls? */
    boolean is_dtls;
    /* imei cache */
    u8 imei[LWM2M_AT_IMEI_SIZE_MAX + 1];

    /*----------- SESSION RELATED -----------*/
    /* lwm2m server */
    LWM2M_CM_SERVER_INFO active_server;
    /* state of client session */
    u32 session_state;
    /* last session event */
    s32 last_evt_id;
    /* current server state */
    u8 server_state;
    /* nnmi cache list */
    LWM2M_CM_CACHE_NNMI nnmi_cache;
    /* token entry list */
    LWM2M_CM_TOKEN_ENTRY_LIST token_entry_list;
    /* READ request entry list */
    LWM2M_CM_READ_ENTRY_LIST read_entry_list;
    /* NOTIFY entry list */
    LWM2M_CM_NOTIFY_ENTRY_LIST notify_entry_list;
    /* ack id list */
    LWM2M_CM_ACK_ID_LIST ack_id_list;
    /* timer for auto update */
    aos_timer_ptr auto_update_timer;
    /* record the tickcnt of log in */
    u32 login_tickcnt;
    /* determine if the operation cmd is a retransaction */
    LWM2M_CM_RETRANS_INFO retrans_info;
} LWM2M_CM_CTX;

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
/* AT handler functions */
static u32 handle_miplcreate_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_mipldelete_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_mipladdobj_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_mipldelobj_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_mipldiscoverrsp_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplopen_query(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplopen_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplupdate_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplclose_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplnotify_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplreadrsp_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplwritersp_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplexecutersp_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplparameterrsp_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplobserversp_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplcfg_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplnmi_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplmgr_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplqmgr_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplupdateset_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplcreateex_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_mipldevinfo_query(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_mipldtlsnat_set(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_miplltrdp_execute(AT_REQ_RESP *at_req_resp_ptr);
/* Init functions */
static int aic_lwm2m_cm_at_cfg_init(void);
static void aic_lwm2m_cm_task_handle(void *thread_input);
static int aic_lwm2m_cm_at_cmd_init(void);
/* +CIS ERROR response function */
static u32 aic_lwm2m_cm_transmit_cis_err(AT_REQ_RESP *at_req_resp_ptr, int error);
/* callbacks */
static void aic_lwm2m_cm_session_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session, UINT state);
static u32 aic_lwm2m_cm_obj_operation(u32 operation, u16 object_id,
                                      NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                      NX_LWM2M_CLIENT_RESOURCE *resource, u32 *resource_count, void *args_ptr,
                                      u32 args_length, u16 mid, const void *token_ptr, u32 token_length);
static u32 aic_lwm2m_cm_observe_callback(u16 object_id, NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                         NX_LWM2M_CLIENT_RESOURCE *resource, u16 mid, boolean observe_flag);
static u32 aic_lwm2m_cm_write_attr_callback(u16 object_id, NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                            NX_LWM2M_CLIENT_RESOURCE *resource, u16 mid, AIC_LWM2M_PARA *para, u8 flag);
static u32 aic_lwm2m_cm_ack_id_callback(u16 ack_id);
static u32 aic_lwm2m_cm_dtls_age_callback(void);
/* patch for non-existing obj WRITE case */
static u32 aic_lwm2m_cm_write_callback(u16 object_id, u16 instance_id, NX_LWM2M_CLIENT_RESOURCE *resource,
                                       u32 *resource_count, void *args_ptr, u32 args_length, u16 msg_id,
                                       const void *token_ptr, u32 token_length);
/* endpoint related functions */
static u32 aic_lwm2m_cm_set_default_pattern(int platform);
static u32 aic_lwm2m_cm_client_imei_get(u8 *buf);
static u32 aic_lwm2m_cm_client_imsi_get(u8 *buf);
static u32 aic_lwm2m_cm_compose_endpoint(void);
/* command cache functions */
static void aic_lwm2m_cm_cache_nnmi_cleanup(void);
static void aic_lwm2m_cm_cache_nnmi_push(const char *in_nnmi, u32 payload_size, u16 mid);
static void aic_lwm2m_cm_cache_nnmi_get(char **out_nnmis);
static void aic_lwm2m_cm_cache_nnmi_remove_top(u16 mid);
static void aic_lwm2m_cm_cach_nnmi_get_miplqmgr_resp(char *str);
/* token list functions */
static u32 aic_lwm2m_cm_token_entry_list_add(u16 msg_id, const void *token, UINT token_length);
static u32 aic_lwm2m_cm_token_entry_list_get(u16 msg_id, LWM2M_CM_TOKEN *token);
static void aic_lwm2m_cm_token_entry_list_remove(u16 msg_id);
static void aic_lwm2m_cm_token_entry_list_cleanup(void);
/* ack id list functions */
static void aic_lwm2m_cm_ack_id_list_init(void);
static u32 aic_lwm2m_cm_ack_id_add(u16 ack_id);
static boolean aic_lwm2m_cm_is_ack_id(u16 mid);
static void aic_lwm2m_cm_ack_id_remove(u16 ack_id);
static void aic_lwm2m_cm_ack_id_cleanup(void);
/* lifetime related functions */
static void aic_lwm2m_cm_auto_update_timeout(void *param);
static void aic_lwm2m_cm_start_auto_update_timer(void);
static void aic_lwm2m_cm_kill_auto_update_timer(void);
/* other fucntions */
static u32 aic_lwm2m_cm_parse_res_id(char *data, u16 **resid, u32 *atts);
static u32 aic_lwm2m_cm_get_res_value_str(NX_LWM2M_CLIENT_RESOURCE *resource, u8 type, char **value_text);
static s32 aic_lwm2m_cm_host_parse(u8 *host, NXD_ADDRESS *addr);
static void aic_lwm2m_cm_reset_session(void);
static s32 aic_lwm2m_cm_server_init(void);
static u32 aic_lwm2m_cm_set_dtls_renegotiation_period(void);
static u32 aic_lwm2m_cm_register_client(void);
static void aic_lwm2m_cm_active_server_set(NXD_ADDRESS *server_addr, u16 udp_port, boolean clean);
static u32 aic_lwm2m_cm_session_is_idle(void);
static void aic_lwm2m_cm_miplsmi_urc(u32 ret, u16 mid);
static void aic_lwm2m_cm_adjust_decimal_precision(char *double_str);
static u8 aic_lwm2m_cm_convert_res_type(u8 aic_res_type);
static void aic_lwm2m_cm_set_default_pskid(boolean is_onenet);
/*******************************************
 *              STATIC VARIABLES
 ******************************************/
static AT_CMD_HANDLE s_lwm2m_cm_at_cmd[] = {
    {"AT+MIPLCREATE", AIC_NULL, AIC_NULL, handle_miplcreate_set, AIC_NULL},
    {"AT+MIPLDELETE", AIC_NULL, AIC_NULL, handle_mipldelete_set, AIC_NULL},
    {"AT+MIPLADDOBJ", AIC_NULL, AIC_NULL, handle_mipladdobj_set, AIC_NULL},
    {"AT+MIPLDELOBJ", AIC_NULL, AIC_NULL, handle_mipldelobj_set, AIC_NULL},
    {"AT+MIPLDISCOVERRSP", AIC_NULL, AIC_NULL, handle_mipldiscoverrsp_set, AIC_NULL},
    {"AT+MIPLOPEN", AIC_NULL, handle_miplopen_query, handle_miplopen_set, AIC_NULL},
    {"AT+MIPLUPDATE", AIC_NULL, AIC_NULL, handle_miplupdate_set, AIC_NULL},
    {"AT+MIPLCLOSE", AIC_NULL, AIC_NULL, handle_miplclose_set, AIC_NULL},
    {"AT+MIPLNOTIFY", AIC_NULL, AIC_NULL, handle_miplnotify_set, AIC_NULL},
    {"AT+MIPLREADRSP", AIC_NULL, AIC_NULL, handle_miplreadrsp_set, AIC_NULL},
    {"AT+MIPLWRITERSP", AIC_NULL, AIC_NULL, handle_miplwritersp_set, AIC_NULL},
    {"AT+MIPLEXECUTERSP", AIC_NULL, AIC_NULL, handle_miplexecutersp_set, AIC_NULL},
    {"AT+MIPLPARAMETERRSP", AIC_NULL, AIC_NULL, handle_miplparameterrsp_set, AIC_NULL},
    {"AT+MIPLOBSERVERSP", AIC_NULL, AIC_NULL, handle_miplobserversp_set, AIC_NULL},
    {"AT+MIPLCFG", AIC_NULL, AIC_NULL, handle_miplcfg_set, AIC_NULL},
    {"AT+MIPLNMI", AIC_NULL, AIC_NULL, handle_miplnmi_set, AIC_NULL},
    {"AT+MIPLMGR", AIC_NULL, AIC_NULL, handle_miplmgr_set, AIC_NULL},
    {"AT+MIPLQMGR", AIC_NULL, AIC_NULL, handle_miplqmgr_set, AIC_NULL},
    {"AT+MIPLUPDATESET", AIC_NULL, AIC_NULL, handle_miplupdateset_set, AIC_NULL},
    {"AT+MIPLCREATEEX", AIC_NULL, AIC_NULL, handle_miplcreateex_set, AIC_NULL},
    {"AT+MIPLDEVINFO", AIC_NULL, handle_mipldevinfo_query, AIC_NULL, AIC_NULL},
    {"AT+MIPLDTLSNAT", AIC_NULL, AIC_NULL, handle_mipldtlsnat_set, AIC_NULL},
    {"AT+MIPLLTRDP", AIC_NULL, AIC_NULL, AIC_NULL, handle_miplltrdp_execute},
    {AIC_NULL},
};

static LWM2M_CM_CTX s_lwm2m_cm_ctx = {0};
/*******************************************
 *              EXTERNAL VARIABLES
 ******************************************/

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
static int aic_lwm2m_cm_at_cfg_init(void)
{
    aos_memset(&s_lwm2m_cm_ctx.config, 0, sizeof(s_lwm2m_cm_ctx.config));
    s_lwm2m_cm_ctx.config.miplopen_lifetime = LWM2M_AT_LIFETIME_DEFAULT;
    s_lwm2m_cm_ctx.config.miplopen_timeout = MIPLOPEN_TIMEOUT_DEFAULT;
    s_lwm2m_cm_ctx.config.miplcfg_pattern = MIPLCFG_PATTERN_RAW_IMEI_IMSI;
    s_lwm2m_cm_ctx.config.miplnmi_nnmi = MIPLNMI_NNMI_DEFAULT;
    s_lwm2m_cm_ctx.config.miplnmi_nsmi = AIC_FALSE;
    s_lwm2m_cm_ctx.config.mipldtlsnat_timeout = MIPLDTLSNAT_TIMEOUT_DEFAULT;
    s_lwm2m_cm_ctx.config.miplcfg_bindmode = MIPLCFG_BM_UDP;
    s_lwm2m_cm_ctx.config.enable_moniter = AIC_TRUE;

    return AIC_SUCCESS;
}

static u32 aic_lwm2m_cm_uri_parse(const u8 *uri, NXD_ADDRESS *ip_addr, UINT *udp_port)
{
    u32 i, temp = 0;
    CHAR host_buffer[256] = {0};

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
    /* get host name */
    if (*uri == '[') {
        /* ipv6 address */
        char *bracket_r = strchr(uri + 1, ']');
        if (bracket_r) {
            if ((bracket_r - (char *)(uri + 1)) < 256) {
                strncpy(host_buffer, uri + 1, (bracket_r - (char *)(uri + 1)));
            } else {
                return AIC_LWM2M_CLIENT_NOT_SUPPORTED;
            }
        } else {
            return AIC_LWM2M_CLIENT_NOT_SUPPORTED;
        }
    } else {
        /* ipv4 or uri */
        char *colon = strrchr(uri, ':');
        if (colon) {
            i = 0;
            if ((colon - (char *)uri) < 256) {
                strncpy(host_buffer, uri, (colon - (char *)uri));
            } else {
                return AIC_LWM2M_CLIENT_NOT_SUPPORTED;
            }
            /* Get port number. */
            colon++;
            while (colon[i] >= '0' && colon[i] <= '9') {
                temp = temp * 10 + (colon[i++] - '0');
                if (temp > 0xFFFF) {
                    break;
                }
            }
            if (temp > 0 && temp <= 0xFFFF) {
                *udp_port = temp;
            }
        } else {
            if (strlen(uri) < 256) {
                strcpy(host_buffer, colon);
            } else {
                return AIC_LWM2M_CLIENT_NOT_SUPPORTED;
            }
        }
    }

    return aic_lwm2m_cm_host_parse(host_buffer, ip_addr);
}

static u32 aic_lwm2m_cm_register_client(void)
{
    u32 status = 0;
    u16 server_id = 0;
    u8 *server_url = NULL;
    u32 server_uri_len = 0;
    u8 security_mode = 0;
    u8 *pub_key_or_id = NX_NULL;
    u32 pub_key_or_id_len = 0;
    u8 *secret_key = NX_NULL;
    u32 secret_key_len = 0;
    u16 udp_port = 0;
    NXD_ADDRESS server_addr;

    /* get lwm2m server info */
    status = aic_lwm2m_client_session_register_info_get(
        &s_lwm2m_cm_ctx.client_session, AIC_LWM2M_CLIENT_RESERVED_ID, &server_id, &server_url, &server_uri_len,
        &security_mode, &pub_key_or_id, &pub_key_or_id_len, NX_NULL, NX_NULL, &secret_key, &secret_key_len);
    if (status != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x1d20b5, "", "[LWM2M CM] aic_lwm2m_cm_register_client failed to get lwm2m server");
        return status;
    }
    /* save dtls info if available */
    if (secret_key_len <= LWM2M_AT_PSK_SIZE_MAX && pub_key_or_id_len <= LWM2M_AT_PSK_ID_SIZE_MAX) {
        s_lwm2m_cm_ctx.dtls_info.lwm2m_key_size = secret_key_len;
        aos_memcpy(s_lwm2m_cm_ctx.dtls_info.lwm2m_key, secret_key, secret_key_len);
        s_lwm2m_cm_ctx.dtls_info.lwm2m_identity_size = pub_key_or_id_len;
        aos_memcpy(s_lwm2m_cm_ctx.dtls_info.lwm2m_identity, pub_key_or_id, pub_key_or_id_len);
    } else {
        aicx_error(0x0b21cf, "", "[LWM2M CM] too long key info size. skip saving.");
    }
    status = aic_lwm2m_cm_uri_parse(server_url, &server_addr, (u32 *)&udp_port);
    if (status == AIC_LWM2M_CLIENT_SUCCESS) {
        s_lwm2m_cm_ctx.bootstrap_state = AIC_TRUE;
        /* add obj instances */
        aic_lwm2m_cm_add_all_instances_to_service(&s_lwm2m_cm_ctx.obj_entry_list);
        aic_lwm2m_client_update_lifetime(s_lwm2m_cm_ctx.config.miplopen_lifetime);
        /* register ack id callback for NOTIFY handle */
        aic_lwm2m_register_session_ack_callback(&s_lwm2m_cm_ctx.client_session, aic_lwm2m_cm_ack_id_callback);
        /* register dtls aged callback */
        aic_lwm2m_register_session_dtls_age_callback(&s_lwm2m_cm_ctx.client_session, aic_lwm2m_cm_dtls_age_callback);
        /* register write callback for non-existing obj WRITE */
        aic_lwm2m_register_session_write_callback(&s_lwm2m_cm_ctx.client_session, aic_lwm2m_cm_write_callback);
        /* set authcode */
        aic_lwm2m_client_authcode_set(s_lwm2m_cm_ctx.dtls_info.authcode, strlen(s_lwm2m_cm_ctx.dtls_info.authcode));
        /* set dtls renegotiation period */
        aic_lwm2m_cm_set_dtls_renegotiation_period();
        status =
            aic_lwm2m_client_register(&s_lwm2m_cm_ctx.client_session, &server_addr, udp_port, server_id, secret_key,
                                      secret_key_len, pub_key_or_id, pub_key_or_id_len, s_lwm2m_cm_ctx.is_dtls);
        if (status != AIC_LWM2M_CLIENT_SUCCESS) {
            aicx_error(0x1c268f, "", "[LWM2M CM] aic_lwm2m_cm_register_client failed to register client.");
        } else {
            aic_lwm2m_cm_active_server_set(&server_addr, udp_port, AIC_FALSE);
        }
    } else {
        aicx_error(0x18087b, "s", "[LWM2M CM] aic_lwm2m_cm_register_client failed to parse uri. %s", server_url);
    }

    return status;
}

static void aic_lwm2m_cm_active_server_set(NXD_ADDRESS *server_addr, u16 udp_port, boolean clean)
{
    if (!server_addr) {
        /* clear active server info */
        aos_memset(&s_lwm2m_cm_ctx.active_server, 0, sizeof(LWM2M_CM_SERVER_INFO));
        if (clean) {
            s_lwm2m_cm_ctx.server_state = LWM2M_CM_SERVER_STATE_NONE;
        } else {
            if (s_lwm2m_cm_ctx.is_bootstrap) {
                s_lwm2m_cm_ctx.server_state = LWM2M_CM_SERVER_STATE_BOOTSTRAP;
            } else {
                s_lwm2m_cm_ctx.server_state = LWM2M_CM_SERVER_STATE_LWM2M;
            }
        }
    } else {
        s_lwm2m_cm_ctx.active_server.addr = *server_addr;
        s_lwm2m_cm_ctx.active_server.port = udp_port;
        s_lwm2m_cm_ctx.server_state = LWM2M_CM_SERVER_STATE_LWM2M;
    }
}

static void aic_lwm2m_cm_task_handle(void *thread_input)
{
    while (1) {
        u32 status = 0;
        u32 events = 0;
        u32 ret = 0;

        status = aos_get_event(s_lwm2m_cm_ctx.event, (u32)(~0), TX_OR_CLEAR, &events, AOS_WAIT_FOREVER);
        if (status != AOS_SUCCESS) {
            aicx_error(0x1982be, "", "[LWM2M CM] aic_lwm2m_cm_task_handle get event ERROR");
            break;
        }
        if (events & LWM2M_CM_EVENT_TERM_TASK) {
            aicx_info(0x06a426, "", "[LWM2M CM] RECEIVED LWM2M_CM_EVENT_TERM_TASK");
            break;
        } else if (events & LWM2M_CM_EVENT_REGISTER) {
            ret = aic_lwm2m_cm_register_client();
            if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
                aicx_error(0x1b71a0, "", "[LWM2M CM] LWM2M_CM_EVENT_REGISTER failed to reg");
                /*report register failed */
                aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, "+MIPLEVENT: 0,7", strlen("+MIPLEVENT: 0,7"));
            }
        } else if (events & LWM2M_CM_EVENT_AUTO_UPDATE) {
            ret = aic_lwm2m_client_update_active(&s_lwm2m_cm_ctx.client_session);
            if (ret != LWM2M_CM_EVENT_AUTO_UPDATE) {
                aicx_error(0x07b765, "", "[LWM2M CM] LWM2M_CM_EVENT_REGISTER failed to update");
            }
        }
    }
}

static int aic_lwm2m_cm_at_cmd_init(void)
{
    aic_add_at_cmd_handle(s_lwm2m_cm_at_cmd);
    aic_lwm2m_cm_at_cfg_init();
    s_lwm2m_cm_ctx.nnmi_cache.mutex = aos_create_mutex("s_lwm2m_cm_nnmi_cache_mutex");
    AIC_ASSERT(s_lwm2m_cm_ctx.nnmi_cache.mutex);
    aic_lwm2m_cm_obj_entry_list_init(&s_lwm2m_cm_ctx.obj_entry_list, "s_lwm2m_cm_obj_entry_list_mutex");
    list_head_init(&s_lwm2m_cm_ctx.token_entry_list.list);
    s_lwm2m_cm_ctx.token_entry_list.mutex = aos_create_mutex("s_lwm2m_cm_token_entry_list_mutex");
    AIC_ASSERT(s_lwm2m_cm_ctx.token_entry_list.mutex);
    aic_lwm2m_cm_read_entry_list_init(&s_lwm2m_cm_ctx.read_entry_list);
    aic_lwm2m_cm_notify_entry_list_init(&s_lwm2m_cm_ctx.notify_entry_list);
    aic_lwm2m_cm_ack_id_list_init();
    /* create thread and event */
    s_lwm2m_cm_ctx.event = aos_create_event("lwm2m_cm_event");
    AIC_ASSERT(s_lwm2m_cm_ctx.event);
    s_lwm2m_cm_ctx.thread_id =
        aos_create_thread("lwm2m_cm_task", aic_lwm2m_cm_task_handle, NULL, LWM2M_CM_TASK_STK_SIZE, 0, NULL,
                          LWM2M_CM_TASK_PRIORITY, AOS_AUTO_START);
    AIC_ASSERT(s_lwm2m_cm_ctx.thread_id != AOS_INVALID_THREAD_ID);

    return AIC_TCPIP_AT_OK;
}

static s32 aic_lwm2m_cm_host_parse(u8 *host, NXD_ADDRESS *addr)
{
    struct addrinfo *res, *dns_res = NULL;
    s32 ret;
    int dns_prio;
    u8 pdp_type;
    boolean is_ipv4_exist = AIC_FALSE;
    boolean is_ipv6_exist = AIC_FALSE;
    NXD_ADDRESS destaddr1 = {0}, destaddr2 = {0};

    ret = getaddrinfo(host, AIC_NULL, AIC_NULL, &res);
    if (ret != NX_SUCCESS || !res) {
        return AIC_EFAULT;
    }
    dns_res = res;
    while (dns_res) {
        if (dns_res->ai_addr->sa_family == AF_INET) {
            destaddr1.nxd_ip_version = NX_IP_VERSION_V4;
            aos_memcpy(&destaddr1.nxd_ip_address.v4, &dns_res->ai_addr->sa_data[2], 4);
            NX_CHANGE_ULONG_ENDIAN(destaddr1.nxd_ip_address.v4);
            is_ipv4_exist = AIC_TRUE;
            break;
        }
        dns_res = dns_res->ai_next;
    }

    pdp_type = aic_get_pdp_ip_type(ANET_DEFAULT_PDP_CID);
    dns_prio = aic_get_dns_prio();
    if (is_ipv4_exist) {
        addr->nxd_ip_version = NX_IP_VERSION_V4;
        addr->nxd_ip_address.v4 = destaddr1.nxd_ip_address.v4;
    }
    if ((dns_prio == 1 && (pdp_type & ANET_DEFAULT_IP_TYPE_V6)) || !is_ipv4_exist) {
        dns_res = res;
        while (dns_res) {
            if (dns_res->ai_addr->sa_family == AF_INET6) {
                destaddr2.nxd_ip_version = NX_IP_VERSION_V6;
                aos_memcpy(destaddr2.nxd_ip_address.v6, &(((struct sockaddr_in6 *)dns_res->ai_addr)->sin6_addr), 16);
                NX_CHANGE_ULONG_ENDIAN(destaddr2.nxd_ip_address.v6[0]);
                NX_CHANGE_ULONG_ENDIAN(destaddr2.nxd_ip_address.v6[1]);
                NX_CHANGE_ULONG_ENDIAN(destaddr2.nxd_ip_address.v6[2]);
                NX_CHANGE_ULONG_ENDIAN(destaddr2.nxd_ip_address.v6[3]);
                is_ipv6_exist = AIC_TRUE;
                break;
            }
            dns_res = dns_res->ai_next;
        }
        if (is_ipv6_exist) {
            addr->nxd_ip_version = NX_IP_VERSION_V6;
            aos_memcpy(addr->nxd_ip_address.v6, destaddr2.nxd_ip_address.v6, 16);
        } else if (!is_ipv4_exist)
            ret = AIC_LWM2M_CLIENT_NOT_SUPPORTED;
    }

    freeaddrinfo(res);
    return ret;
}

static void aic_lwm2m_cm_reset_session(void)
{
    aic_lwm2m_cm_kill_auto_update_timer();
    aic_lwm2m_cm_cache_nnmi_cleanup();
    aic_lwm2m_cm_token_entry_list_cleanup();
    aic_lwm2m_cm_read_entry_list_cleanup(&s_lwm2m_cm_ctx.read_entry_list);
    aic_lwm2m_cm_notify_entry_list_cleanup(&s_lwm2m_cm_ctx.notify_entry_list);
    aic_lwm2m_cm_ack_id_cleanup();
    s_lwm2m_cm_ctx.session_state = NX_LWM2M_CLIENT_SESSION_INIT;
    s_lwm2m_cm_ctx.last_evt_id = MIPLEVENT_EVTID_NONE;
    aos_memset(&s_lwm2m_cm_ctx.retrans_info, 0, sizeof(LWM2M_CM_RETRANS_INFO));
    s_lwm2m_cm_ctx.login_tickcnt = 0;
}

static s32 aic_lwm2m_cm_server_init(void)
{
    return aic_lwm2m_cm_host_parse(s_lwm2m_cm_ctx.host, &s_lwm2m_cm_ctx.init_server.addr);
}

static u32 aic_lwm2m_cm_set_dtls_renegotiation_period(void)
{
    return aic_lwm2m_client_dtls_conf_set(s_lwm2m_cm_ctx.config.mipldtlsnat_timeout,
                                          s_lwm2m_cm_ctx.config.mipldtlsnat_timeout != 0 ? AIC_TRUE : AIC_FALSE);
}

static u32 aic_lwm2m_cm_transmit_cis_err(AT_REQ_RESP *at_req_resp_ptr, int error)
{
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_str, sizeof(result_str), "+CIS ERROR: %d", error);
    line_pointers[0] = (ulong)result_str;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
    return AIC_SUCCESS;
}

static void aic_lwm2m_cm_session_callback(AIC_LWM2M_CLIENT_SESSION *lwm2m_session, UINT state)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    s32 evt_id = -1;
    static boolean is_update = AIC_FALSE;

    aicx_info(0x067a6c, "d", "[LWM2M CM] session callback state is %d\n", state);
    switch (state) {
    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_REQUESTING:
        break;
    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_INITIATED:
        break;
    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_FINISHED:
        evt_id = MIPLEVENT_EVTID_BOOTSTRAP_SUCCESS;
        aos_set_event(s_lwm2m_cm_ctx.event, LWM2M_CM_EVENT_REGISTER, TX_OR);
        break;
    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_ERROR:
        aicx_error(0x125194, "x", "[LWM2M CM] Failed to bootstrap device, error=0x%02x\n",
                  aic_lwm2m_client_session_error_get(lwm2m_session));
        evt_id = MIPLEVENT_EVTID_BOOTSTRAP_FAIL;
        break;
    case NX_LWM2M_CLIENT_SESSION_REGISTERING:
        is_update = AIC_FALSE;
        break;
    case NX_LWM2M_CLIENT_SESSION_UPDATING:
        is_update = AIC_TRUE;
        break;
    case NX_LWM2M_CLIENT_SESSION_REGISTERED:
        if (is_update) {
            evt_id = MIPLEVENT_EVTID_UPDATE_SUCCESS;
        } else {
            evt_id = MIPLEVENT_EVTID_REGISTERED_SUCCESS;
        }
        /* start auto update timer */
        aic_lwm2m_cm_start_auto_update_timer();
        s_lwm2m_cm_ctx.login_tickcnt = aos_get_tickcount();
        break;
    case NX_LWM2M_CLIENT_SESSION_DISABLED:
        break;
    case NX_LWM2M_CLIENT_SESSION_DEREGISTERED:
        evt_id = MIPLEVENT_EVTID_DEREGISTERED_SUCCESS;
        aic_lwm2m_cm_kill_auto_update_timer();
        s_lwm2m_cm_ctx.login_tickcnt = 0;
        break;
    case NX_LWM2M_CLIENT_SESSION_ERROR:
        if (s_lwm2m_cm_ctx.session_state == NX_LWM2M_CLIENT_SESSION_REGISTERING) {
            evt_id = MIPLEVENT_EVTID_REGISTERED_FAIL;
        }
        aicx_info(0x1a41b3, "x", "[LWM2M CM] session error, error=0x%02x\n",
                  aic_lwm2m_client_session_error_get(lwm2m_session));
        break;
    default:
        break;
    }
    s_lwm2m_cm_ctx.session_state = state;
    if (evt_id != -1) {
        s_lwm2m_cm_ctx.last_evt_id = evt_id;
        snprintf(str, AT_RESP_RESULT_MAX, "+MIPLEVENT: 0,%d", evt_id);
        aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, strlen(str));
    }
}

static u32 aic_lwm2m_cm_session_is_idle(void)
{
    if (s_lwm2m_cm_ctx.session_state == NX_LWM2M_CLIENT_SESSION_INIT ||
        s_lwm2m_cm_ctx.session_state == NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_ERROR ||
        s_lwm2m_cm_ctx.session_state == NX_LWM2M_CLIENT_SESSION_ERROR ||
        s_lwm2m_cm_ctx.session_state == NX_LWM2M_CLIENT_SESSION_DEREGISTERED) {
        return AIC_TRUE;
    }
    return AIC_FALSE;
}

static void aic_lwm2m_cm_miplsmi_urc(u32 ret, u16 mid)
{
    int status = 0, num = 0;
    u8 str[30] = {'\0'};

    if (!s_lwm2m_cm_ctx.config.miplnmi_nsmi)
        return;
    if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
        status = 1;
    }
    num = mid & 0x3F;
    snprintf(str, 30, "+MIPLNSMI: 0,%d,%d", status, num);
    aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, strlen(str));
}

static u32 aic_lwm2m_cm_set_default_pattern(int platform)
{
    u32 result = AIC_LWM2M_CLIENT_SUCCESS;

    switch (platform) {
    case MIPLCFG_PLATFORM_ONENET:
        s_lwm2m_cm_ctx.config.miplcfg_pattern = MIPLCFG_PATTERN_RAW_IMEI_IMSI;
        break;
    case MIPLCFG_PLATFORM_CTWING:
        s_lwm2m_cm_ctx.config.miplcfg_pattern = MIPLCFG_PATTERN_RAW_IMEI;
        break;
    case MIPLCFG_PLATFORM_DMP:
        result = AIC_LWM2M_CLIENT_NOT_SUPPORTED;
        break;
    case MIPLCFG_PLATFORM_HWIOT:
        s_lwm2m_cm_ctx.config.miplcfg_pattern = MIPLCFG_PATTERN_RAW_IMEI;
        break;
    default:
        result = AIC_LWM2M_CLIENT_NOT_SUPPORTED;
        break;
    }

    return result;
}

static u32 aic_lwm2m_cm_client_imei_get(u8 *buf)
{
    AT_RESP_FROM_CP at_resp;
    ulong *line_arr = AIC_NULL, *para_arr = AIC_NULL;
    u16 cnt;
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;

    if (s_lwm2m_cm_ctx.imei[0]) {
        /* already retrieved before, copy the cache directly */
        strncpy(buf, s_lwm2m_cm_ctx.imei, LWM2M_AT_IMEI_SIZE_MAX);
        return ret;
    }
    at_resp.resp_buf = aos_zalloc(AT_MAX_RESP_LENGTH + 1);
    if (!at_resp.resp_buf) {
        return AIC_LWM2M_CLIENT_NO_MEMORY;
    }
    at_resp.buf_len = AT_MAX_RESP_LENGTH + 1;
    ret = aic_send_cmd_and_wait("AT+GSN=1", &at_resp);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x154b93, "", "[lwm2m cm][IMEI] failed to send AT command.\n");
        aos_free(at_resp.resp_buf);
        return AIC_LWM2M_CLIENT_GET_IMEI_ERROR;
    }
    line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
    if (!line_arr) {
        aicx_error(0x18548b, "", "[lwm2m cm] line_arr is null.\n");
        aos_free(at_resp.resp_buf);
        return AIC_LWM2M_CLIENT_GET_IMEI_ERROR;
    }
    /* parse imei info */
    para_arr = aic_cm_split_paras_to_str_array((u8 *)line_arr[0], "+GSN: ", &cnt, NULL);
    if (para_arr && cnt == 1) {
        strncpy(buf, (u8 *)para_arr[0], LWM2M_AT_IMEI_SIZE_MAX);
        /* cache the IMEI */
        strncpy(s_lwm2m_cm_ctx.imei, (u8 *)para_arr[0], LWM2M_AT_IMEI_SIZE_MAX);
    }
    aic_cm_free_paras_mem(para_arr, cnt);
    aic_cm_free_paras_mem(line_arr, at_resp.resp_line_cnt);
    aos_free(at_resp.resp_buf);

    return AIC_LWM2M_CLIENT_SUCCESS;
}

static u32 aic_lwm2m_cm_client_imsi_get(u8 *buf)
{
    AT_RESP_FROM_CP at_resp;
    ulong *line_arr = AIC_NULL;
    u32 ret = AIC_SUCCESS;

    at_resp.resp_buf = aos_zalloc(AT_MAX_RESP_LENGTH + 1);
    if (!at_resp.resp_buf) {
        return AIC_LWM2M_CLIENT_NO_MEMORY;
    }
    at_resp.buf_len = AT_MAX_RESP_LENGTH + 1;
    ret = aic_send_cmd_and_wait("AT+CIMI", &at_resp);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x16d27a, "", "[lwm2m cm][IMSI] failed to send AT command.\n");
        aos_free(at_resp.resp_buf);
        return AIC_LWM2M_CLIENT_GET_IMSI_ERROR;
    }
    if (at_resp.resp_line_cnt > 1) {
        line_arr = aic_get_each_line_from_resp(at_resp.resp_buf, at_resp.resp_line_cnt);
        if (!line_arr) {
            aicx_error(0x18548b, "", "[lwm2m cm] line_arr is null.\n");
            aos_free(at_resp.resp_buf);
            return AIC_LWM2M_CLIENT_GET_IMSI_ERROR;
        }
        /*imsi max length is 15 */
        strncpy(buf, (char *)line_arr[0], LWM2M_AT_IMSI_SIZE_MAX);
    }
    aicx_info(0x0438ea, "s", "[lwm2m cm] sim_imsi = %s.\n", buf);
    /* free params */
    aic_cm_free_paras_mem(line_arr, at_resp.resp_line_cnt);
    aos_free(at_resp.resp_buf);

    return AIC_LWM2M_CLIENT_SUCCESS;
}

static u32 aic_lwm2m_cm_compose_endpoint(void)
{
    u32 ret;
    u8 imei[LWM2M_AT_IMEI_SIZE_MAX + 1] = {0};
    u8 imsi[LWM2M_AT_IMSI_SIZE_MAX + 1] = {0};
    u8 str[100] = {'\0'};
    /* Get imei */
    ret = aic_lwm2m_cm_client_imei_get(imei);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x061e1e, "", "[lwm2m cm] Not able to get IMEI");
        return ret;
    }
    /* set endpoint */
    switch (s_lwm2m_cm_ctx.config.miplcfg_pattern) {
    case MIPLCFG_PATTERN_RAW_IMEI:
        ret = aic_lwm2m_client_endpoint_set(imei, strlen(imei));
        return ret;
    case MIPLCFG_PATTERN_URN_IMEI:
        snprintf(str, sizeof(str), "urn:imei:%s", imei);
        ret = aic_lwm2m_client_endpoint_set(str, strlen(str));
        return ret;
    case MIPLCFG_PATTERN_SPECIFIC_ENDPOINT:
        aicx_error(0x0c0291, "", "[lwm2m cm] no need to set endpoint here. since it should've already set before.");
        return AIC_LWM2M_CLIENT_SUCCESS;
    default:
        /* move on to get imsi */
        break;
    }
    /* Get imsi */
    ret = aic_lwm2m_cm_client_imsi_get(imsi);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x131887, "", "[lwm2m cm] Not able to get IMSI");
        return ret;
    }
    switch (s_lwm2m_cm_ctx.config.miplcfg_pattern) {
    case MIPLCFG_PATTERN_URN_IMEI_IMSI:
        snprintf(str, sizeof(str), "urn:imei-imsi:%s-%s", imei, imsi);
        break;
    case MIPLCFG_PATTERN_DEFAULT:
    case MIPLCFG_PATTERN_RAW_IMEI_IMSI:
    default:
        snprintf(str, sizeof(str), "%s;%s", imei, imsi);
        break;
    }
    ret = aic_lwm2m_client_endpoint_set(str, strlen(str));

    return ret;
}

static void aic_lwm2m_cm_set_default_pskid(boolean is_onenet)
{
    u8 *identity = s_lwm2m_cm_ctx.dtls_info.lwm2m_identity;
    u8 *identity_size = &s_lwm2m_cm_ctx.dtls_info.lwm2m_identity_size;
    u8 imei[LWM2M_AT_IMEI_SIZE_MAX + 1] = {0};
    boolean is_bootstrap = s_lwm2m_cm_ctx.is_bootstrap;
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;

    if (!s_lwm2m_cm_ctx.is_dtls) {
        /* no need to set the pskid if not dtls connection */
        return;
    }
    if (is_bootstrap) {
        identity = s_lwm2m_cm_ctx.dtls_info.bs_identity;
        identity_size = &s_lwm2m_cm_ctx.dtls_info.bs_identity_size;
    }
    /* check if PSKID is empty or not */
    if (identity[0]) {
        /* already filled, no need to set */
        return;
    }
    ret = aic_lwm2m_cm_client_imei_get(imei);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        return;
    }
    if (is_bootstrap) {
        /* bootstrap: IMEI only */
        strncpy(identity, imei, LWM2M_AT_PSK_ID_SIZE_MAX);
        s_lwm2m_cm_ctx.dtls_info.bs_identity_size = strlen(identity);
    } else {
        /* lwm2m: lwm2m_<IMEI> if onenet, otherwise <IMEI>*/
        if (is_onenet) {
            strcpy(s_lwm2m_cm_ctx.dtls_info.lwm2m_identity, "lwm2m_");
        }
        strcat(s_lwm2m_cm_ctx.dtls_info.lwm2m_identity, imei);
        s_lwm2m_cm_ctx.dtls_info.lwm2m_identity_size = strlen(identity);
    }
}

/* parse tlv variable value to text string */
static u32 aic_lwm2m_cm_get_res_value_str(NX_LWM2M_CLIENT_RESOURCE *resource, u8 type, char **value_text)
{
    u32 ret = AIC_LWM2M_CLIENT_ERROR;
    UINT value_length = 0;
    char *result_txt = NULL;
    const char *hex_tbl = "0123456789ABCDEF";

    // TBD: aic API cannot match requirment, so we use netx directly here.
    switch (type) {
    case AIC_LWM2M_CLIENT_RESOURCE_STRING:
        {
            const CHAR *string_value = NULL;

            nx_lwm2m_client_resource_string_get(resource, &string_value, &value_length);
            if (string_value) {
                result_txt = aos_zalloc(value_length + 1);
                if (result_txt) {
                    aos_memcpy(result_txt, string_value, value_length);
                    *value_text = result_txt;
                    ret = AIC_LWM2M_CLIENT_SUCCESS;
                }
            }
        }
        break;
    case AIC_LWM2M_CLIENT_RESOURCE_OPAQUE:
        {
            const u8 *opaque_value = NULL;

            nx_lwm2m_client_resource_opaque_get(resource, (const VOID **)&opaque_value, &value_length);
            if (opaque_value) {
                result_txt = aos_zalloc(value_length * 2 + 1);
                if (result_txt) {
                    UINT i = 0;
                    /* convert to hex string */
                    for (i = 0; i < value_length; i++) {
                        result_txt[i << 1] = hex_tbl[(opaque_value[i] & 0xF0) >> 4];
                        result_txt[(i << 1) + 1] = hex_tbl[opaque_value[i] & 0x0F];
                    }
                    *value_text = result_txt;
                    ret = AIC_LWM2M_CLIENT_SUCCESS;
                }
            }
        }
        break;
    case AIC_LWM2M_CLIENT_RESOURCE_INTEGER32:
        {
            NX_LWM2M_INT32 int32_value = 0;

            nx_lwm2m_client_resource_integer32_get(resource, &int32_value);
            result_txt = aos_zalloc(32);
            if (result_txt) {
                snprintf(result_txt, 32, "%ld", int32_value);
                *value_text = result_txt;
                ret = AIC_LWM2M_CLIENT_SUCCESS;
            }
        }
        break;
    case AIC_LWM2M_CLIENT_RESOURCE_FLOAT64:
        {
            NX_LWM2M_FLOAT64 float64_value = 0;

            /* TBD: it seems float is parsed with some tricky in NETX. Need to go through [IEEE 754-2008][FLOAT] */
            nx_lwm2m_client_resource_float64_get(resource, &float64_value);
            result_txt = aos_zalloc(64);
            if (result_txt) {
                snprintf(result_txt, 64, "%f", float64_value);
                *value_text = result_txt;
                ret = AIC_LWM2M_CLIENT_SUCCESS;
            }
        }
        break;
    case AIC_LWM2M_CLIENT_RESOURCE_BOOLEAN:
        {
            NX_LWM2M_BOOL bool_value = AIC_FALSE;

            nx_lwm2m_client_resource_boolean_get(resource, &bool_value);
            result_txt = aos_zalloc(10);
            if (result_txt) {
                snprintf(result_txt, 10, "%d", (int)bool_value);
                *value_text = result_txt;
                ret = AIC_LWM2M_CLIENT_SUCCESS;
            }
        }
        break;
    default:
        ret = AIC_LWM2M_CLIENT_ERROR;
        break;
    }

    return ret;
}

static u32 aic_lwm2m_cm_obj_operation(u32 operation, u16 object_id,
                                      NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                      NX_LWM2M_CLIENT_RESOURCE *resource, u32 *resource_count, void *args_ptr,
                                      u32 args_length, u16 mid, const void *token_ptr, u32 token_length)
{
    u8 str_normal[AT_RESP_RESULT_MAX] = {'\0'};
    u8 *str = str_normal;
    u8 *str_large = NULL;
    size_t buf_size = 0;
    boolean is_urc = AIC_FALSE;
    s32 object_instance_id = -1;
    s32 resource_id = -1;
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS, atts, acts, i;
    u16 *resid = NULL;
    LWM2M_CM_OBJ_INS_LIST *obj_ins_list = NULL;
    size_t len = 0;
    u8 *cmd = "";
    u32 payload_size = 0;

    /* check if it is retransmission message */
    if (token_ptr) {
        u32 now = aos_get_tickcount();

        if (mid == s_lwm2m_cm_ctx.retrans_info.last_mid) {
            /* within 5 min */
            if (now - s_lwm2m_cm_ctx.retrans_info.last_tickcnt < 300 * 1000) {
                s_lwm2m_cm_ctx.retrans_info.last_tickcnt = now;
                aicx_info(0x1a698a, "d", "[LWM2M CM] received a retransmitted message. %d", mid);
                return AIC_LWM2M_CLIENT_BAD_REQUEST;
            }
        } else {
            s_lwm2m_cm_ctx.retrans_info.last_mid = mid;
        }
        s_lwm2m_cm_ctx.retrans_info.last_tickcnt = now;
    }
    if (object_instance_ptr) {
        object_instance_id = (s32)object_instance_ptr->object_instance_id;
    }
    if (resource) {
        u16 temp_value = 0;
        aic_lwm2m_client_resource_info_get(resource, &temp_value, NULL);
        resource_id = (s32)temp_value;
    }
    switch (operation) {
    case NX_LWM2M_CLIENT_OBJECT_READ:
        /* NOTE: this callback can be invoked in case not only of server requests but of reading instance as well.
         * The difference is whether token_ptr is NULL or not */
        aicx_info(0x15bb29, "", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_READ entry");
        if (token_ptr) { /* server reqeusts */
            is_urc = AIC_TRUE;
            aic_lwm2m_cm_read_entry_list_add_req(&s_lwm2m_cm_ctx.read_entry_list, mid, object_id, object_instance_id,
                                                 resource_id);
            snprintf(str, AT_RESP_RESULT_MAX, "+MIPLREAD: 0,%d,%d,%d,%d", mid, object_id, object_instance_id,
                     resource_id);
        } else { /* read value */
            if (resource_id == -1 || object_instance_id == -1) {
                aicx_error(0x0f23b9, "", "[LWM2M CM] unexpected NX_LWM2M_CLIENT_OBJECT_READ cb");
                ret = NX_LWM2M_CLIENT_NOT_FOUND;
                break;
            }
            for (i = 0; i < *resource_count; i++) {
                u8 *value = NULL;
                u32 value_len = 0;
                u8 res_type = AIC_LWM2M_CLIENT_RESOURCE_NONE;
                u16 res_id = 0;

                aic_lwm2m_client_resource_info_get(&resource[i], &res_id, NULL);
                ret = aic_lwm2m_cm_obj_entry_list_get_res_entry(&s_lwm2m_cm_ctx.obj_entry_list, object_id,
                                                                (u16)object_instance_id, res_id, &value, &value_len,
                                                                &res_type);
                if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
                    aic_lwm2m_client_resource_set(&resource[i], value, value_len, res_type);
                } else {
                    ret = NX_LWM2M_CLIENT_NOT_FOUND;
                    aicx_error(0x057e0e, "ddd", "[LWM2M CM] aic_lwm2m_cm_obj_operation failed to get res entry.(%d/%d/%d)", object_id,
                               object_instance_id, res_id);
                    break;
                }
            }
        }
        break;
    case NX_LWM2M_CLIENT_OBJECT_WRITE:
        /*
         * Note: just compose AT commands to the north, we do not change the values of the object entries here.
         */
        aicx_info(0x07db77, "", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_WRITE entry");
        if (token_ptr) {
            u32 i = 0;
            boolean is_ok = AIC_TRUE;
            char **urc_array = NULL;

            urc_array = aos_zalloc(*resource_count * sizeof(char *));
            if (!urc_array) {
                aicx_error(0x0a21ad, "", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_WRITE - No memory");
                break;
            }
            for (i = 0; i < *resource_count; i++) {
                u16 res_id = 0;
                u8 type = AIC_LWM2M_CLIENT_RESOURCE_NONE;
                char *value_text = NULL;
                size_t value_len = 0;
                char flag[2] = {0};
                char *urc_line = NULL;

                if (i == *resource_count - 1) {
                    flag[0] = '0';
                } else if (i == 0) {
                    flag[0] = '1';
                } else {
                    flag[0] = '2';
                }
                aic_lwm2m_client_resource_info_get(&resource[i], &res_id, NULL);
                /* try to search the type of the resource */
                aic_lwm2m_cm_obj_entry_list_get_res_type(&s_lwm2m_cm_ctx.obj_entry_list, object_id, resource_id, &type);
                if (type == AIC_LWM2M_CLIENT_RESOURCE_NONE) {
                    aicx_error(0x1a5bf9, "ddd", "[LWM2M CM] aic_lwm2m_cm_obj_operation unknown resource type. /%d/%d/%d", object_id,
                               object_instance_id, res_id);
                    type = AIC_LWM2M_CLIENT_RESOURCE_OPAQUE;
                    aic_lwm2m_cm_get_res_value_str(&resource[i], type, &value_text);
                } else if (resource[i].resource_type == NX_LWM2M_CLIENT_RESOURCE_TEXT) {
                    /* ATTENTION: when the resource type is NX_LWM2M_CLIENT_RESOURCE_TEXT, it does not means the
                     * resource type is string. Any type could be formatted in text as OMA defined. Meanwhile, we need
                     * to make sure value_text is a null-terminated string */
                    aic_lwm2m_cm_get_res_value_str(&resource[i], AIC_LWM2M_CLIENT_RESOURCE_STRING, &value_text);
                } else {
                    aic_lwm2m_cm_get_res_value_str(&resource[i], type, &value_text);
                }
                if (!value_text) {
                    is_ok = AIC_FALSE;
                    aicx_error(0x080019, "", "[LWM2M CM] Failed to parse value text.");
                    break;
                }
                value_len = strlen(value_text);
                /* +MIPLWRITE: <ref>,<mid>,<objid>,<insid>,<resid>,<type>,<len>,<value>,<flag> */
                snprintf(str_normal, AT_RESP_RESULT_MAX, "+MIPLWRITE: 0,%d,%d,%d,%d,%d,%d", (s32)mid, (s32)object_id,
                         object_instance_id, res_id, (s32)aic_lwm2m_cm_convert_res_type(type), value_len);
                buf_size = value_len + strlen(str_normal) + 4; /*"4" here means ,,0\0*/
                /* create buffer */
                urc_line = aos_zalloc(buf_size);
                if (!urc_line) {
                    aos_free(value_text);
                    is_ok = AIC_FALSE;
                    aicx_error(0x05dfcb, "", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_WRITE no mem in obj write urc");
                    break;
                }
                snprintf(urc_line, buf_size, "%s,%s,%s", str_normal, value_text, flag);
                urc_array[i] = urc_line;
                aos_free(value_text);
            }
            if (is_ok) {
                size_t urc_len = 0;

                /* compose up all urc lines */
                for (i = 0; i < *resource_count; i++) {
                    urc_len += strlen(urc_array[i]) + 2; /* \r\n */
                }
                str_large = aos_zalloc(urc_len + 1);
                if (str_large) {
                    for (i = 0; i < *resource_count; i++) {
                        if (urc_array[i]) {
                            strcat(str_large, urc_array[i]);
                            if (i != *resource_count - 1) {
                                strcat(str_large, "\r\n");
                            }
                        }
                    }
                    str = str_large;
                    is_urc = AIC_TRUE;
                    /* Note: the args_length field is reused as payload size in netx service */
                    payload_size = args_length;
                } else {
                    aicx_error(0x1670f6, "", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_WRITE - No memory#2");
                }
            }
            /* clean up urc lines */
            for (i = 0; i < *resource_count; i++) {
                if (urc_array[i])
                    aos_free(urc_array[i]);
            }
        }
        break;
    case NX_LWM2M_CLIENT_OBJECT_EXECUTE:
        aicx_info(0x10ee6e, "", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_EXECUTE entry");
        if (args_ptr) {
            cmd = (u8 *)args_ptr;
            len = args_length;
            payload_size = args_length;
        }
        snprintf(str_normal, AT_RESP_RESULT_MAX, "+MIPLEXECUTE: 0,%d,%d,%d,%d,%d", mid, object_id, object_instance_id,
                 resource_id, len);
        buf_size = strlen(cmd) + strlen(str_normal) + 4; /*"4" here means ,""\0*/
        if (buf_size > AT_RESP_RESULT_MAX) {
            /* need large buffer */
            str_large = aos_zalloc(buf_size);
            if (!str_large) {
                aicx_error(0x1c44ed, "", "[LWM2M CM] aic_lwm2m_cm_obj_operation no mem in obj exe urc");
                break;
            }
            snprintf(str_large, buf_size, "%s,\"%s\"", str_normal, cmd);
            str = str_large;
        } else {
            strcat(str_normal, ",\"");
            strcat(str_normal, cmd);
            strcat(str_normal, "\"");
            str = str_normal;
        }
        is_urc = AIC_TRUE;
        break;
    case NX_LWM2M_CLIENT_OBJECT_DISCOVER:
        /* NOTE: this callback can be invoked in case not only of server requests but of reading instance as well.
         * The difference is whether token_ptr is NULL or not */
        aicx_info(0x1b6d3b, "p", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_DISCOVER entry, %p", token_ptr);
        ret = aic_lwm2m_cm_obj_entry_list_get_res_id_list(&s_lwm2m_cm_ctx.obj_entry_list, object_id, &resid, &atts,
                                                          &acts);
        if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
            aicx_error(0x02bd57, "d", "[LWM2M CM] aic_lwm2m_cm_obj_operation cannot find obj(%d) for DISCOVER", object_id);
            break;
        }
        if (token_ptr) { /* server requests */
            /* report URC directly */
            snprintf(str, AT_RESP_RESULT_MAX, "+MIPLDISCOVER: 0,%d,%d", mid, object_id);
            aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, strlen(str));
        }
        if (*resource_count < atts) {
            ret = AIC_LWM2M_CLIENT_BUFFER_TOO_SMALL;
            break;
        }
        /* response discover directly */
        for (i = 0; i < atts; i++) {
            aic_lwm2m_client_resource_info_set(&resource[i], resid[i], LWM2M_AT_DEFAULT_RES_OPERAIONS);
        }
        *resource_count = atts;
        aicx_info(0x030ee8, "d", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_DISCOVER atts is %d", atts);
        break;
    case NX_LWM2M_CLIENT_OBJECT_DELETE:
        aicx_info(0x0bcfd2, "", "[LWM2M CM] NX_LWM2M_CLIENT_OBJECT_DELETE entry.");
        ret = aic_lwm2m_cm_obj_entry_list_get_instance_list(&s_lwm2m_cm_ctx.obj_entry_list, object_id, &obj_ins_list);
        if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
            s32 i = 0;

            /* scan instance list to find if instance exists or not */
            ret = AIC_LWM2M_CLIENT_METHOD_NOT_ALLOWED;
            for (i = 0; i < obj_ins_list->count; i++) {
                if (obj_ins_list->entries[i].is_available &&
                    object_instance_ptr == (NX_LWM2M_CLIENT_OBJECT_INSTANCE *)&obj_ins_list->entries[i]) {
                    ret = aic_lwm2m_client_obj_instance_remove(object_id, object_instance_ptr);
                    if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
                        /* set as not in-use instance */
                        obj_ins_list->entries[i].is_available = AIC_FALSE;
                    }
                    break;
                }
            }
            if (ret == AIC_LWM2M_CLIENT_METHOD_NOT_ALLOWED) {
                aicx_error(0x0505a6, "", "[LWM2M CM] aic_lwm2m_cm_obj_operation not available instance.");
            }
        }
        break;
    case NX_LWM2M_CLIENT_OBJECT_CREATE:
        /* since there is no AT URC linked to this behavior, i.e. create obj instance triggered by LWM2M server, no
         * extra handling here except for log, because even if we create this instance, no one will handle that */
        aicx_info(0x180dec, "", "[LWM2M CM] receive CREATE method, rejected.");
        ret = AIC_LWM2M_CLIENT_NOT_ACCEPTABLE;
        break;
    default:
        break;
    }
    if (is_urc && str[0]) {
        len = strlen(str);
        /* cache token */
        aic_lwm2m_cm_token_entry_list_add(mid, token_ptr, token_length);
        /* do not cache the command, report directly */
        if (payload_size > 1024) {
            aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, len);
        } else {
            switch (s_lwm2m_cm_ctx.config.miplnmi_nnmi) {
            case MIPLNMI_NNMI_DEFAULT:
                aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, len);
                break;
            case MIPLNMI_NNMI_CACHE:
                aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, "+MIPLNMI: 0", strlen("+MIPLNMI: 0"));
                aic_lwm2m_cm_cache_nnmi_push(str, payload_size, mid);
                break;
            case MIPLNMI_NNMI_SILENT:
                aic_lwm2m_cm_cache_nnmi_push(str, payload_size, mid);
                break;
            default:
                break;
            }
        }
    }
    if (str_large)
        aos_free(str_large);

    return ret;
}

static u32 aic_lwm2m_cm_observe_callback(u16 object_id, NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                         NX_LWM2M_CLIENT_RESOURCE *resource, u16 mid, boolean observe_flag)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    s32 object_instance_id = -1;
    s32 resource_id = -1;

    if (object_instance_ptr) {
        object_instance_id = (s32)object_instance_ptr->object_instance_id;
    }
    if (resource) {
        u16 temp_value = 0;
        aic_lwm2m_client_resource_info_get(resource, &temp_value, NULL);
        resource_id = (s32)temp_value;
    }
    snprintf(str, AT_RESP_RESULT_MAX, "+MIPLOBSERVE: 0,%d,%d,%d,%d,%d", mid, observe_flag, object_id,
             object_instance_id, resource_id);
    aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, strlen(str));

    return AIC_LWM2M_CLIENT_SUCCESS;
}

/* Adjust precision*/
static void aic_lwm2m_cm_adjust_decimal_precision(char *double_str)
{
    char *dot_p = strstr(double_str, ".");
    char *end_p = double_str + strlen(double_str);
    char *iter_p = end_p - 1;

    if (!dot_p)
        return;
    while (iter_p - 2 >= dot_p) {
        if (*iter_p != '0')
            break;
        else {
            *iter_p = '\0';
            iter_p--;
        }
    }
}

static u8 aic_lwm2m_cm_convert_res_type(u8 aic_res_type)
{
    u8 ret = LWM2M_CM_RES_TYPE_OPAQUE;

    switch (aic_res_type) {
    case AIC_LWM2M_CLIENT_RESOURCE_STRING:
        ret = LWM2M_CM_RES_TYPE_STRING;
        break;
    case AIC_LWM2M_CLIENT_RESOURCE_INTEGER32:
    case AIC_LWM2M_CLIENT_RESOURCE_INTEGER64:
        ret = LWM2M_CM_RES_TYPE_INTEGER;
        break;
    case AIC_LWM2M_CLIENT_RESOURCE_FLOAT32:
    case AIC_LWM2M_CLIENT_RESOURCE_FLOAT64:
        ret = LWM2M_CM_RES_TYPE_FLOAT;
        break;
    case AIC_LWM2M_CLIENT_RESOURCE_BOOLEAN:
        ret = LWM2M_CM_RES_TYPE_BOOL;
        break;
    default:
        break;
    }

    return ret;
}

/* e.g. +MIPLPARAMETER: 0,18174,3200,0,5758,38,"pmin=18;pmax=90;lt=0.0;gt=60.0;stp=2.0" */
static u32 aic_lwm2m_cm_write_attr_callback(u16 object_id, NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr,
                                            NX_LWM2M_CLIENT_RESOURCE *resource, u16 mid, AIC_LWM2M_PARA *para, u8 flag)
{
    u8 attr_str[20] = {'\0'};
    u8 i = 0;
    u8 para_str[100] = {0};
    u8 *urc_str = NULL;
    s32 ins_id = -1;
    s32 res_id = -1;

    if (object_instance_ptr) {
        ins_id = object_instance_ptr->object_instance_id;
    }
    if (resource) {
        u16 temp_value = 0;
        aic_lwm2m_client_resource_info_get(resource, &temp_value, NULL);
        res_id = (s32)temp_value;
    }
    if (!flag) {
        aicx_error(0x1aec94, "", "[LWM2M CM] aic_lwm2m_cm_write_attr_callback invalid flag");
        return AIC_LWM2M_CLIENT_SUCCESS;
    }
    if (flag & AIC_ATTR_FLAG_PMIN) {
        snprintf(attr_str, 20, "pmin=%d", (int)para->pim);
        strcpy(para_str, attr_str);
        i++;
    }
    if (flag & AIC_ATTR_FLAG_PMAX) {
        snprintf(attr_str, 20, "pmax=%d", (int)para->pmax);
        if (i) {
            strcat(para_str, ";");
        }
        strcat(para_str, attr_str);
        i++;
    }
    if (flag & AIC_ATTR_FLAG_GT) {
        snprintf(attr_str, 20, "gt=%f", para->gt);
        aic_lwm2m_cm_adjust_decimal_precision(attr_str);
        if (i) {
            strcat(para_str, ";");
        }
        strcat(para_str, attr_str);
        i++;
    }
    if (flag & AIC_ATTR_FLAG_LT) {
        snprintf(attr_str, 20, "lt=%f", para->lt);
        aic_lwm2m_cm_adjust_decimal_precision(attr_str);
        if (i) {
            strcat(para_str, ";");
        }
        strcat(para_str, attr_str);
        i++;
    }
    if (flag & AIC_ATTR_FLAG_ST) {
        snprintf(attr_str, 20, "stp=%f", para->st);
        aic_lwm2m_cm_adjust_decimal_precision(attr_str);
        if (i) {
            strcat(para_str, ";");
        }
        strcat(para_str, attr_str);
        i++;
    }
    urc_str = aos_zalloc(200);
    if (!urc_str) {
        aicx_error(0x03424d, "", "[LWM2M CM] aic_lwm2m_cm_write_attr_callback no memory");
    } else {
        snprintf(urc_str, 200, "+MIPLPARAMETER: 0,%d,%d,%d,%d,%d,\"%s\"", mid, object_id, ins_id, res_id,
                 strlen(para_str), para_str);
        aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, urc_str, strlen(urc_str));
        aos_free(urc_str);
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

static u32 aic_lwm2m_cm_ack_id_callback(u16 ack_id)
{
    if (aic_lwm2m_cm_is_ack_id(ack_id)) {
        u8 str[30] = {'\0'};

        aic_lwm2m_cm_ack_id_remove(ack_id);
        snprintf(str, AT_RESP_RESULT_MAX, "+MIPLEVENT: 0,26,%d", (int)ack_id);
        aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, strlen(str));
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

static u32 aic_lwm2m_cm_dtls_age_callback(void)
{
    if (!aic_lwm2m_cm_session_is_idle()) {
        u8 *str = "+MIPLEVENT: 0,32";

        aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, strlen(str));
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

static u32 aic_lwm2m_cm_write_callback(u16 object_id, u16 object_instance_id, NX_LWM2M_CLIENT_RESOURCE *resource,
                                       u32 *resource_count, void *args_ptr, u32 args_length, u16 mid,
                                       const void *token_ptr, u32 token_length)
{
    u32 i = 0;
    boolean is_ok = AIC_TRUE;
    char **urc_array = NULL;
    u8 str_normal[AT_RESP_RESULT_MAX] = {'\0'};
    u32 payload_size = 0;
    u8 *str_large = NULL;

    aicx_info(0x08ebc6, "", "[LWM2M CM] receive a WRITE cmd for non-added obj");
    if (!resource || !resource_count || *resource_count == 0) {
        return AIC_LWM2M_CLIENT_SUCCESS;
    }
    urc_array = aos_zalloc(*resource_count * sizeof(char *));
    if (!urc_array) {
        aicx_error(0x09ca0d, "", "[LWM2M CM] aic_lwm2m_cm_write_callback - No memory");
        goto EXIT;
    }
    for (i = 0; i < *resource_count; i++) {
        u16 res_id = 0;
        u8 type = AIC_LWM2M_CLIENT_RESOURCE_OPAQUE;
        char *value_text = NULL;
        size_t value_len = 0;
        char flag[2] = {0};
        char *urc_line = NULL;
        size_t buf_size = 0;

        if (i == *resource_count - 1) {
            flag[0] = '0';
        } else if (i == 0) {
            flag[0] = '1';
        } else {
            flag[0] = '2';
        }
        aic_lwm2m_client_resource_info_get(&resource[i], &res_id, NULL);
        /* since the obj does not exist, we can only parse the value as opaque */
        aic_lwm2m_cm_get_res_value_str(&resource[i], type, &value_text);
        if (!value_text) {
            is_ok = AIC_FALSE;
            aicx_error(0x00e467, "", "[LWM2M CM] aic_lwm2m_cm_write_callback Failed to parse value text.");
            break;
        }
        value_len = strlen(value_text);
        /* +MIPLWRITE: <ref>,<mid>,<objid>,<insid>,<resid>,<type>,<len>,<value>,<flag> */
        snprintf(str_normal, AT_RESP_RESULT_MAX, "+MIPLWRITE: 0,%d,%d,%d,%d,%d,%d", (s32)mid, (s32)object_id,
                 object_instance_id, res_id, (s32)aic_lwm2m_cm_convert_res_type(type), value_len);
        buf_size = value_len + strlen(str_normal) + 4; /*"4" here means ,,0\0*/
        /* create buffer */
        urc_line = aos_zalloc(buf_size);
        if (!urc_line) {
            aos_free(value_text);
            is_ok = AIC_FALSE;
            aicx_error(0x031c19, "", "[LWM2M CM] aic_lwm2m_cm_write_callback no mem in obj write urc");
            break;
        }
        snprintf(urc_line, buf_size, "%s,%s,%s", str_normal, value_text, flag);
        urc_array[i] = urc_line;
        aos_free(value_text);
    }
    if (is_ok) {
        size_t urc_len = 0;

        /* compose up all urc lines */
        for (i = 0; i < *resource_count; i++) {
            urc_len += strlen(urc_array[i]) + 2; /* \r\n */
        }
        str_large = aos_zalloc(urc_len + 1);
        if (str_large) {
            for (i = 0; i < *resource_count; i++) {
                if (urc_array[i]) {
                    strcat(str_large, urc_array[i]);
                    if (i != *resource_count - 1) {
                        strcat(str_large, "\r\n");
                    }
                }
            }
            /* Note: the args_length field is reused as payload size in netx service */
            payload_size = args_length;
        } else {
            aicx_error(0x13d9df, "", "[LWM2M CM] aic_lwm2m_cm_write_callback - No memory#2");
        }
    }
    /* clean up urc lines */
    for (i = 0; i < *resource_count; i++) {
        if (urc_array[i])
            aos_free(urc_array[i]);
    }
    if (str_large) {
        size_t len = strlen(str_large);
        /* cache token */
        aic_lwm2m_cm_token_entry_list_add(mid, token_ptr, token_length);
        /* do not cache the command, report directly */
        if (payload_size > 1024) {
            aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str_large, len);
        } else {
            switch (s_lwm2m_cm_ctx.config.miplnmi_nnmi) {
            case MIPLNMI_NNMI_DEFAULT:
                aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str_large, len);
                break;
            case MIPLNMI_NNMI_CACHE:
                aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, "+MIPLNMI: 0", strlen("+MIPLNMI: 0"));
                aic_lwm2m_cm_cache_nnmi_push(str_large, payload_size, mid);
                break;
            case MIPLNMI_NNMI_SILENT:
                aic_lwm2m_cm_cache_nnmi_push(str_large, payload_size, mid);
                break;
            default:
                break;
            }
        }
        aos_free(str_large);
    }

EXIT:
    return AIC_LWM2M_CLIENT_SUCCESS;
}

static void aic_lwm2m_cm_cache_nnmi_push(const char *in_nnmi, u32 payload_size, u16 mid)
{
    u16 new_slot;
    char *new_entry = NULL;

    if (!in_nnmi || !in_nnmi[0]) {
        aicx_error(0x1bb0e6, "", "[LWM2M CM] invalid in_nnmi");
        return;
    }
    new_entry = aos_zalloc(strlen(in_nnmi) + 1);
    if (!new_entry) {
        aicx_error(0x03afbf, "", "[LWM2M CM] no memory");
        return;
    }
    strcpy(new_entry, in_nnmi);
    aos_get_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex, AOS_WAIT_FOREVER);
    if (s_lwm2m_cm_ctx.nnmi_cache.total == LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT) {
        /* list is full */
        if (s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[s_lwm2m_cm_ctx.nnmi_cache.head].text)
            aos_free(s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[s_lwm2m_cm_ctx.nnmi_cache.head].text);
        s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[s_lwm2m_cm_ctx.nnmi_cache.head].text = NULL;
        s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[s_lwm2m_cm_ctx.nnmi_cache.head].payload_size = 0;
        s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[s_lwm2m_cm_ctx.nnmi_cache.head].mid = 0;
        s_lwm2m_cm_ctx.nnmi_cache.head = (s_lwm2m_cm_ctx.nnmi_cache.head + 1) % LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT;
        s_lwm2m_cm_ctx.nnmi_cache.total--;
        s_lwm2m_cm_ctx.nnmi_cache.dropped++;
    }
    /* find the new slot to fill in */
    new_slot = (s_lwm2m_cm_ctx.nnmi_cache.head + s_lwm2m_cm_ctx.nnmi_cache.total) % LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT;
    s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[new_slot].text = new_entry;
    s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[new_slot].payload_size = payload_size;
    s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[new_slot].mid = mid;
    s_lwm2m_cm_ctx.nnmi_cache.total++;
    aos_put_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex);
}

static void aic_lwm2m_cm_cache_nnmi_get(char **out_nnmis)
{
    u16 index;

    *out_nnmis = NULL;
    /* no buffer */
    if (!s_lwm2m_cm_ctx.nnmi_cache.total) {
        aicx_error(0x08c28e, "", "[LWM2M CM] aic_lwm2m_cm_cache_nnmi_get no entry available");
        return;
    }
    /* First in First Out */
    aos_get_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex, AOS_WAIT_FOREVER);
    index = s_lwm2m_cm_ctx.nnmi_cache.head;
    *out_nnmis = s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].text;
    aos_put_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex);
}

static void aic_lwm2m_cm_cache_nnmi_remove_top(u16 mid)
{
    u16 index;
    if (s_lwm2m_cm_ctx.config.miplnmi_nnmi == MIPLNMI_NNMI_DEFAULT) {
        return;
    }
    /* no buffer */
    if (!s_lwm2m_cm_ctx.nnmi_cache.total) {
        aicx_error(0x06cfa7, "", "[LWM2M CM] aic_lwm2m_cm_cache_nnmi_remove_top no entry available");
        return;
    }
    /* First in First Out */
    aos_get_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex, AOS_WAIT_FOREVER);
    index = s_lwm2m_cm_ctx.nnmi_cache.head;
    if (s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].mid == mid) {
        if (s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].text)
            aos_free(s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].text);
        s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].text = NULL;
        s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].payload_size = 0;
        s_lwm2m_cm_ctx.nnmi_cache.head = (index + 1) % LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT;
        s_lwm2m_cm_ctx.nnmi_cache.total--;
    }
    aos_put_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex);
}

/* get buffers size of WRITE/EXECUTE commands */
static void aic_lwm2m_cm_cach_nnmi_get_miplqmgr_resp(char *str)
{
    u32 buf_size = 0, i = 0;

    aos_get_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex, AOS_WAIT_FOREVER);
    while (i < s_lwm2m_cm_ctx.nnmi_cache.total) {
        u16 index = (s_lwm2m_cm_ctx.nnmi_cache.head + i) % LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT;

        if (strstr(s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].text, "+MIPLWRITE") ||
            strstr(s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].text, "+MIPLEXECUTE")) {
            buf_size += s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[index].payload_size;
        }
        i++;
    }
    snprintf(str, AT_RESP_RESULT_MAX, "+MIPLQMGR: 0,QUEUES=%d,DROPPED=%u,BUFFERS=%u",
             (int)s_lwm2m_cm_ctx.nnmi_cache.total, s_lwm2m_cm_ctx.nnmi_cache.dropped, buf_size);
    aos_put_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex);
}

static void aic_lwm2m_cm_cache_nnmi_cleanup(void)
{
    u16 i = 0;

    aos_get_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT; i++) {
        if (s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[i].text) {
            aos_free(s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[i].text);
            s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[i].text = NULL;
        }
        s_lwm2m_cm_ctx.nnmi_cache.nnmi_entries[i].payload_size = 0;
    }
    s_lwm2m_cm_ctx.nnmi_cache.head = 0;
    s_lwm2m_cm_ctx.nnmi_cache.total = 0;
    s_lwm2m_cm_ctx.nnmi_cache.dropped = 0;
    aos_put_mutex(s_lwm2m_cm_ctx.nnmi_cache.mutex);
}

static u32 aic_lwm2m_cm_token_entry_list_add(u16 msg_id, const void *token, UINT token_length)
{
    LWM2M_CM_TOKEN_ENTRY *new_token = NULL;
    void *new_data = NULL;

    new_token = aos_zalloc(sizeof(LWM2M_CM_TOKEN_ENTRY));
    if (!new_token)
        goto ERROR_EXIT;
    if (token_length > 0) {
        new_data = aos_zalloc(token_length);
        if (!new_data)
            goto ERROR_EXIT;
        aos_memcpy(new_data, token, token_length);
    }
    new_token->msg_id = msg_id;
    new_token->token_entry.data = new_data;
    new_token->token_entry.len = token_length;
    /* in case objid already exists in the list, remove the old one first */
    aic_lwm2m_cm_token_entry_list_remove(msg_id);
    aos_get_mutex(s_lwm2m_cm_ctx.token_entry_list.mutex, AOS_WAIT_FOREVER);
    /* add into list */
    list_add_tail(&new_token->node, &s_lwm2m_cm_ctx.token_entry_list.list);
    aos_put_mutex(s_lwm2m_cm_ctx.token_entry_list.mutex);

    return AIC_LWM2M_CLIENT_SUCCESS;

ERROR_EXIT:
    if (new_data) {
        aos_free(new_data);
    }
    if (new_token) {
        aos_free(new_token);
    }

    return AIC_LWM2M_CLIENT_NO_MEMORY;
}

static void aic_lwm2m_cm_token_entry_list_remove(u16 msg_id)
{
    LWM2M_CM_TOKEN_ENTRY *entry = NULL;

    aos_get_mutex(s_lwm2m_cm_ctx.token_entry_list.mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &s_lwm2m_cm_ctx.token_entry_list.list, node, LWM2M_CM_TOKEN_ENTRY)
    {
        if (entry->msg_id == msg_id) {
            /* removed from list */
            list_del(&entry->node);
            if (entry->token_entry.data) {
                aos_free(entry->token_entry.data);
            }
            aos_free(entry);
            break;
        }
    }
    aos_put_mutex(s_lwm2m_cm_ctx.token_entry_list.mutex);
}

static u32 aic_lwm2m_cm_token_entry_list_get(u16 msg_id, LWM2M_CM_TOKEN *token)
{
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;
    LWM2M_CM_TOKEN_ENTRY *entry = NULL;

    aos_get_mutex(s_lwm2m_cm_ctx.token_entry_list.mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &s_lwm2m_cm_ctx.token_entry_list.list, node, LWM2M_CM_TOKEN_ENTRY)
    {
        if (entry->msg_id == msg_id) {
            if (entry->token_entry.len) {
                token->data = aos_zalloc(entry->token_entry.len);
                if (!token->data) {
                    ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                    break;
                }
                aos_memcpy(token->data, entry->token_entry.data, entry->token_entry.len);
                token->len = entry->token_entry.len;
            }
            ret = AIC_LWM2M_CLIENT_SUCCESS;
            break;
        }
    }
    aos_put_mutex(s_lwm2m_cm_ctx.token_entry_list.mutex);

    return ret;
}

static void aic_lwm2m_cm_token_entry_list_cleanup(void)
{
    LWM2M_CM_TOKEN_ENTRY *entry = NULL;
    LWM2M_CM_TOKEN_ENTRY *next_entry = NULL;

    aos_get_mutex(s_lwm2m_cm_ctx.token_entry_list.mutex, AOS_WAIT_FOREVER);
    list_for_each_entry_safe(entry, next_entry, &s_lwm2m_cm_ctx.token_entry_list.list, node, LWM2M_CM_TOKEN_ENTRY)
    {
        list_del(&entry->node);
        if (entry->token_entry.data) {
            aos_free(entry->token_entry.data);
        }
        aos_free(entry);
    }
    aos_put_mutex(s_lwm2m_cm_ctx.token_entry_list.mutex);
}

static void aic_lwm2m_cm_ack_id_list_init(void)
{
    aos_memset(&s_lwm2m_cm_ctx.ack_id_list, 0, sizeof(s_lwm2m_cm_ctx.ack_id_list));
    s_lwm2m_cm_ctx.ack_id_list.mutex = aos_create_mutex("s_lwm2m_cm_ack_id_list_mutex");
    AIC_ASSERT(s_lwm2m_cm_ctx.ack_id_list.mutex);
}

static u32 aic_lwm2m_cm_ack_id_add(u16 ack_id)
{
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;

    aos_get_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex, AOS_WAIT_FOREVER);
    if (s_lwm2m_cm_ctx.ack_id_list.total < LWM2M_AT_ACK_ID_MAX && ack_id != 0) {
        u16 i = 0;

        for (i = 0; i < LWM2M_AT_ACK_ID_MAX; i++) {
            if (s_lwm2m_cm_ctx.ack_id_list.ack_id[i] == 0) {
                s_lwm2m_cm_ctx.ack_id_list.ack_id[i] = ack_id;
                s_lwm2m_cm_ctx.ack_id_list.total++;
                break;
            }
        }
    } else {
        ret = AIC_LWM2M_CLIENT_ERROR;
    }
    aos_put_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex);
    aicx_error(0x06d023, "dd", "[LWM2M CM] aic_lwm2m_cm_ack_id_add -- ack_id is %d, ret is %d", ack_id, ret);

    return ret;
}

static boolean aic_lwm2m_cm_is_ack_id(u16 mid)
{
    boolean ret = AIC_FALSE;
    u16 i = 0;

    aos_get_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex, AOS_WAIT_FOREVER);
    if (s_lwm2m_cm_ctx.ack_id_list.total == 0) {
        aos_put_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex);
        return AIC_FALSE;
    }
    for (i = 0; i < LWM2M_AT_ACK_ID_MAX; i++) {
        if (s_lwm2m_cm_ctx.ack_id_list.ack_id[i] == mid) {
            ret = AIC_TRUE;
            break;
        }
    }
    aos_put_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex);

    return ret;
}

static void aic_lwm2m_cm_ack_id_remove(u16 ack_id)
{
    u16 i = 0;

    aos_get_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex, AOS_WAIT_FOREVER);
    if (s_lwm2m_cm_ctx.ack_id_list.total == 0) {
        aos_put_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex);
        return;
    }
    for (i = 0; i < LWM2M_AT_ACK_ID_MAX; i++) {
        if (s_lwm2m_cm_ctx.ack_id_list.ack_id[i] == ack_id) {
            s_lwm2m_cm_ctx.ack_id_list.ack_id[i] = 0;
            s_lwm2m_cm_ctx.ack_id_list.total--;
            break;
        }
    }
    aos_put_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex);
}

static void aic_lwm2m_cm_ack_id_cleanup(void)
{
    aos_get_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex, AOS_WAIT_FOREVER);
    aos_memset(s_lwm2m_cm_ctx.ack_id_list.ack_id, 0, sizeof(s_lwm2m_cm_ctx.ack_id_list.ack_id));
    s_lwm2m_cm_ctx.ack_id_list.total = 0;
    aos_put_mutex(s_lwm2m_cm_ctx.ack_id_list.mutex);
}

static void aic_lwm2m_cm_auto_update_timeout(void *param)
{
    if (s_lwm2m_cm_ctx.event) {
        aos_set_event(s_lwm2m_cm_ctx.event, LWM2M_CM_EVENT_AUTO_UPDATE, TX_OR);
    }
}

static void aic_lwm2m_cm_start_auto_update_timer(void)
{
    if (!s_lwm2m_cm_ctx.config.miplupdateset_enable || s_lwm2m_cm_ctx.config.miplopen_lifetime < 60) {
        aicx_info(0x1d7af5, "dd", "[LWM2M CM] no need to auto update. enable(%d), lifetime(%d)",
                  s_lwm2m_cm_ctx.config.miplupdateset_enable, s_lwm2m_cm_ctx.config.miplopen_lifetime);
        return;
    }
    if (!s_lwm2m_cm_ctx.auto_update_timer) {
        s_lwm2m_cm_ctx.auto_update_timer =
            aos_create_timer("lwm2m_cm_auto_update_timer", aic_lwm2m_cm_auto_update_timeout, NULL,
                             (s_lwm2m_cm_ctx.config.miplopen_lifetime - 30) * 1000, AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);
    } else {
        aos_change_timer(s_lwm2m_cm_ctx.auto_update_timer, (s_lwm2m_cm_ctx.config.miplopen_lifetime - 30) * 1000,
                         AOS_NO_PERIOD);
    }
}

static void aic_lwm2m_cm_kill_auto_update_timer(void)
{
    if (s_lwm2m_cm_ctx.auto_update_timer) {
        aos_delete_timer(s_lwm2m_cm_ctx.auto_update_timer);
        s_lwm2m_cm_ctx.auto_update_timer = NULL;
    }
}

static u32 aic_lwm2m_cm_parse_res_id(char *data, u16 **resid, u32 *atts)
{
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;
    char *token = NULL;
    char *state = NULL;
    int id = 0;
    u32 i = 0;
    u16 *temp = NULL;
    const char *separator = ";";
    const char *p = data;

    *atts = 0;
    *resid = NULL;
    if (strlen(data) == 0) {
        return AIC_LWM2M_CLIENT_ERROR;
    }
    if (strstr(data, ",")) {
        separator = ",";
    }
    /* need to scan */
    while (p) {
        p = strstr(p, separator);
        if (p) {
            (*atts)++;
            p++;
        }
    }
    (*atts)++;
    temp = aos_zalloc(sizeof(u16) * (*atts));
    if (!temp) {
        return AIC_LWM2M_CLIENT_NO_MEMORY;
    }
    token = strtok_r(data, separator, &state);
    while (token) {
        if (aic_is_valid_integer_para((u8 *)token, strlen(token))) {
            id = atoi(token);
            if (id > AIC_LWM2M_CLIENT_RESERVED_ID) {
                ret = AIC_LWM2M_CLIENT_ERROR;
                goto ERROR_EXIT;
            }
            temp[i++] = id;
        } else {
            ret = AIC_LWM2M_CLIENT_ERROR;
            goto ERROR_EXIT;
        }
        token = strtok_r(NULL, separator, &state);
    }
    *atts = i;
    *resid = temp;
    return AIC_LWM2M_CLIENT_SUCCESS;

ERROR_EXIT:
    if (temp) {
        aos_free(temp);
    }
    return ret;
}

static int aic_lwm2m_cm_parse_string(u8 *src, int len)
{
    u32 i, num = 0;

    for (i = 0; i < len; i++) {
        if (*src == '\"')
            return AIC_EFAULT;

        src++;
    }

    return num;
}
/* Themes:
 * 1. set command: create an device.
 *      AT+MIPLCREATE=<totalsize>,<config>,<index>,<currentsize>,<flag>
 * 2. execute command: connect to LWM2M bootstrap default server
 *      AT+MIPLCREATE
 */
static u32 handle_miplcreate_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt = 0;
    ulong *paras_arr = NULL;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int ret, err_code = LWM2M_CM_INVAL_AT_PARAM;

    /* check exe cmd or set cmd */
    if (strncasecmp(at_req_resp_ptr->at_req, "AT+MIPLCREATE=", strlen("AT+MIPLCREATE=")) == 0) {
        /* set cmd */
        paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLCREATE=", &cnt, NULL);
        if (!paras_arr || cnt != 5) {
            aicx_error(0x136b72, "", "[+MIPLCREATE]: parameter parse error\n");
            goto ERROR_EXIT;
        }
        if (s_lwm2m_cm_ctx.server_state != LWM2M_CM_SERVER_STATE_NONE) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
        /* Since we have no idea how to deal with the parameters, do nothing but to go ahead as execute cmd */
    }
    /* exe cmd */
    if (s_lwm2m_cm_ctx.server_state != LWM2M_CM_SERVER_STATE_NONE) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        aicx_error(0x1062f2, "", "[+MIPLCREATE]: client already created.\n");
        goto ERROR_EXIT;
    }
    s_lwm2m_cm_ctx.is_bootstrap = AIC_TRUE;
    aos_memcpy(s_lwm2m_cm_ctx.host, LWM2M_DEFAULT_HOST, sizeof(LWM2M_DEFAULT_HOST));
    s_lwm2m_cm_ctx.init_server.port = LWM2M_SERVER_PORT;
    ret = aic_lwm2m_cm_server_init();
    if (ret < 0) {
        aicx_error(0x0d0330, "", "[+MIPLCREATE]: failed to init server.\n");
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    /* create client and session */
    ret = aic_lwm2m_client_create(&s_lwm2m_cm_ctx.client_session, aic_lwm2m_cm_session_callback);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x18c59a, "", "[+MIPLCREATE]: failed to create client.\n");
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    s_lwm2m_cm_ctx.server_state = LWM2M_CM_SERVER_STATE_BOOTSTRAP;
    s_lwm2m_cm_ctx.com_id = at_req_resp_ptr->serial_port_id;

    aic_cm_free_paras_mem(paras_arr, cnt);
    strcpy(str, "+MIPLCREATE: 0");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: delete specific client.
 *      AT+MIPLDELETE=<ref>
 */
static u32 handle_mipldelete_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    int ref;
    ulong *paras_arr;
    u32 ret;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLDELETE=", &cnt, NULL);
    if (!paras_arr || cnt != 1) {
        aicx_error(0x1c9342, "", "[+MIPLDELETE]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_client_delete(&s_lwm2m_cm_ctx.client_session);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x00cf45, "d", "[+MIPLDELETE]: failed to delete client. ret is %d\n", ret);
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    /* clean up session related context */
    aic_lwm2m_cm_reset_session();
    aic_lwm2m_cm_active_server_set(NULL, 0, AIC_TRUE);
    /* clear created context */
    aic_lwm2m_cm_obj_entry_list_cleanup(&s_lwm2m_cm_ctx.obj_entry_list);
    /* MUST RESET DEVICE STATE HERE */
    aos_memset(s_lwm2m_cm_ctx.host, 0, sizeof(s_lwm2m_cm_ctx.host));
    aos_memset(&s_lwm2m_cm_ctx.dtls_info, 0, sizeof(s_lwm2m_cm_ctx.dtls_info));
    aos_memset(&s_lwm2m_cm_ctx.init_server, 0, sizeof(s_lwm2m_cm_ctx.init_server));
    aos_memset(s_lwm2m_cm_ctx.imei, 0, sizeof(s_lwm2m_cm_ctx.imei));
    s_lwm2m_cm_ctx.is_dtls = AIC_FALSE;
    s_lwm2m_cm_ctx.is_bootstrap = AIC_FALSE;
    s_lwm2m_cm_ctx.bootstrap_state = AIC_FALSE;
    aic_lwm2m_cm_at_cfg_init();
    aic_reply_ok_resp(at_req_resp_ptr);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: create a obj and its instance(s)
 *      AT+MIPLADDOBJ=<ref><objid><inscount><bitmap><atts><acts>
 */
static u32 handle_mipladdobj_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, i = 0, objid = 0, inscount = 0, atts = 0, acts = 0;
    u32 ret;
    const char *bitmap = "";
    int err_code = LWM2M_CM_INVAL_AT_PARAM;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLADDOBJ=", &cnt, NULL);
    if (!paras_arr || ((cnt != 6) && (cnt != 3))) {
        aicx_error(0x06a64a, "", "[+MIPLADDOBJ]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <objid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        objid = atoi((u8 *)paras_arr[1]);
        if (objid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <inscount> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        inscount = atoi((u8 *)paras_arr[2]);
        if (inscount >= 32) {
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    if (cnt == 6) {
        /* parse <bitmap> */
        bitmap = (const char *)paras_arr[3];
        if (strlen(bitmap) != inscount) {
            aicx_error(0x02f21c, "", "[+MIPLADDOBJ] invalid bitmap");
            goto ERROR_EXIT;
        }
        for (i = 0; i < inscount; i++) {
            if (bitmap[i] != '1' && bitmap[i] != '0') {
                aicx_error(0x09d091, "s", "[+MIPLADDOBJ] invalid bitmap %s", bitmap);
                goto ERROR_EXIT;
            }
        }
        /* parse <atts> */
        if (aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4]))) {
            atts = atoi((u8 *)paras_arr[4]);
        } else {
            goto ERROR_EXIT;
        }
        /* parse <acts> */
        if (aic_is_valid_integer_para((u8 *)paras_arr[5], strlen((u8 *)paras_arr[5]))) {
            acts = atoi((u8 *)paras_arr[5]);
        } else {
            goto ERROR_EXIT;
        }
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        aicx_error(0x1b75d7, "", "[+MIPLADDOBJ] no device created yet.");
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_client_obj_add(objid, aic_lwm2m_cm_obj_operation);
    if (ret == AIC_LWM2M_CLIENT_ALREADY_EXIST || ret == AIC_LWM2M_CLIENT_SUCCESS) {
        ret = aic_lwm2m_cm_obj_entry_list_add(&s_lwm2m_cm_ctx.obj_entry_list, objid, inscount, bitmap, acts, atts);
        if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
            aicx_error(0x09e08e, "", "[+MIPLADDOBJ] failed to add obj entry.");
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
        /* Don't add this obj instances to service here */
    }
    if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
        aic_lwm2m_register_obj_observe_callback(objid, aic_lwm2m_cm_observe_callback);
        aic_lwm2m_register_obj_attrs_set_callback(objid, aic_lwm2m_cm_write_attr_callback);
        aic_cm_free_paras_mem(paras_arr, cnt);
        aic_reply_ok_resp(at_req_resp_ptr);
        return AIC_SUCCESS;
    } else {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);

    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: delete a object along with its instances and resources
 *      AT+MIPLDELOBJ=<ref>,<objid>
 */
static u32 handle_mipldelobj_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, objid = 0;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    u32 ret;
    LWM2M_CM_OBJ_INS_LIST *obj_ins_list = NULL;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLDELOBJ=", &cnt, NULL);
    if (!paras_arr || cnt != 2) {
        aicx_error(0x0648ae, "", "[+MIPLDELOBJ]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <objid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        objid = atoi((u8 *)paras_arr[1]);
        if (objid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        aicx_error(0x10fca4, "", "[+MIPLDELOBJ] no device created yet.");
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    // remove obj instances first
    ret = aic_lwm2m_cm_obj_entry_list_get_instance_list(&s_lwm2m_cm_ctx.obj_entry_list, objid, &obj_ins_list);
    if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
        s32 i = 0;

        /* scan instance list to find if instance exists or not */
        ret = AIC_LWM2M_CLIENT_METHOD_NOT_ALLOWED;
        for (i = 0; i < obj_ins_list->count; i++) {
            NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance = NULL;

            if (obj_ins_list->entries[i].is_available) {
                object_instance = (NX_LWM2M_CLIENT_OBJECT_INSTANCE *)&obj_ins_list->entries[i];
                ret = aic_lwm2m_client_obj_instance_remove(objid, object_instance);
                if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
                    /* set as not in-use instance */
                    obj_ins_list->entries[i].is_available = AIC_FALSE;
                }
            }
        }
        if (ret == AIC_LWM2M_CLIENT_METHOD_NOT_ALLOWED) {
            aicx_error(0x0505a6, "", "[LWM2M CM] aic_lwm2m_cm_obj_operation not available instance.");
        }
    }
    /* remove obj from service */
    ret = aic_lwm2m_client_obj_remove(objid);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x04b029, "d", "[+MIPLDELOBJ]: failed to delete obj. ret is %d\n", ret);
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    aic_lwm2m_cm_obj_entry_list_remove(&s_lwm2m_cm_ctx.obj_entry_list, objid);
    aic_reply_ok_resp(at_req_resp_ptr);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: specify the resources of the specific object
 *      AT+MIPLDISCOVERRSP=<ref>,<objid>,<result>,<length>,<data>
 */
static u32 handle_mipldiscoverrsp_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, objid = 0, result, length;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    char *data = NULL;
    u16 *resid = NULL;
    u32 atts = 0;
    u32 ret;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLDISCOVERRSP=", &cnt, NULL);
    if (!paras_arr || cnt != 5) {
        aicx_error(0x1cc482, "", "[+MIPLDISCOVERRSP]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <objid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        objid = atoi((u8 *)paras_arr[1]);
        if (objid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <result> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        result = atoi((u8 *)paras_arr[2]);
        if (result != 1) {
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <length> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3]))) {
        length = atoi((u8 *)paras_arr[3]);
    } else {
        goto ERROR_EXIT;
    }
    /* check <data> */
    data = (char *)paras_arr[4];
    if (length != strlen(data)) {
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        aicx_error(0x1a6e0a, "", "[+MIPLOPEN] no device created yet.");
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_cm_parse_res_id(data, &resid, &atts);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS && atts > 0) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        aicx_error(0x1a6d58, "", "[+MIPLDISCOVERRSP]: invalid res id");
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_cm_obj_entry_list_attach_resid(&s_lwm2m_cm_ctx.obj_entry_list, objid, resid, atts);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        aicx_error(0x029e39, "", "[+MIPLDISCOVERRSP]: failed to attach resid");
        if (resid)
            aos_free(resid);
        goto ERROR_EXIT;
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    if (resid) {
        aos_free(resid);
    }
    return AIC_SUCCESS;
}

/* Themes:
 * 1. read command: query login status
 *      AT+MIPLOPEN?
 */
static u32 handle_miplopen_query(AT_REQ_RESP *at_req_resp_ptr)
{
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE || aic_lwm2m_cm_session_is_idle()) {
        aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS);
    } else {
        u8 str[40] = {'\0'};

        aic_reply_ok_resp(at_req_resp_ptr);
        snprintf(str, 40, "+MIPLEVENT: 0,%d", s_lwm2m_cm_ctx.last_evt_id);
        aic_transmit_at_cmd_urc(s_lwm2m_cm_ctx.com_id, str, strlen(str));
    }

    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: start login
 *      AT+MIPLOPEN=<ref>,<lifetime>[,<timeout>]
 */
static u32 handle_miplopen_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, err_code = LWM2M_CM_INVAL_AT_PARAM, lifetime, timeout;
    u32 ret;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLOPEN=", &cnt, NULL);
    if (!paras_arr || cnt < 2 || cnt > 3) {
        aicx_error(0x18988a, "", "[+MIPLOPEN]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <lifetime> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        lifetime = atoi((u8 *)paras_arr[1]);
        if (lifetime == 0) {
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    s_lwm2m_cm_ctx.config.miplopen_lifetime = (u32)lifetime;
    /* parse <timeout> */
    if (cnt == 3) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
            timeout = atoi((u8 *)paras_arr[2]);
            if (timeout == 0) {
                goto ERROR_EXIT;
            }
        } else {
            goto ERROR_EXIT;
        }
        // TBD: how to use timeout
        s_lwm2m_cm_ctx.config.miplopen_timeout = (u32)timeout;
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        aicx_error(0x1a6e0a, "", "[+MIPLOPEN] no device created yet.");
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    /* try to get endpoint info before create client */
    ret = aic_lwm2m_cm_compose_endpoint();
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x118cd7, "", "[+MIPLOPEN] Not able to set endpoint ");
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    /* try to set bind mode */
    switch (s_lwm2m_cm_ctx.config.miplcfg_bindmode) {
    case MIPLCFG_BM_UDP:
        ret = aic_lwm2m_client_bindmode_set(NX_LWM2M_CLIENT_BINDING_U);
        break;
    case MIPLCFG_BM_UDP_QUEUE:
        ret = aic_lwm2m_client_bindmode_set(NX_LWM2M_CLIENT_BINDING_UQ);
        break;
    default:
        ret = aic_lwm2m_client_bindmode_set(NX_LWM2M_CLIENT_BINDING_U);
        break;
    }
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x089a39, "", "[+MIPLOPEN] Not able to set bind mode");
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    /* set PSKID in case it is still empty */
    aic_lwm2m_cm_set_default_pskid(AIC_TRUE);
    if (s_lwm2m_cm_ctx.is_bootstrap && !s_lwm2m_cm_ctx.bootstrap_state) {
        ret = aic_lwm2m_client_bootstrap(&s_lwm2m_cm_ctx.client_session, &s_lwm2m_cm_ctx.init_server.addr,
                                         s_lwm2m_cm_ctx.init_server.port, s_lwm2m_cm_ctx.dtls_info.bs_key,
                                         s_lwm2m_cm_ctx.dtls_info.bs_key_size, s_lwm2m_cm_ctx.dtls_info.bs_identity,
                                         s_lwm2m_cm_ctx.dtls_info.bs_identity_size, s_lwm2m_cm_ctx.is_dtls);
    } else {
        /* add obj instances */
        aic_lwm2m_cm_add_all_instances_to_service(&s_lwm2m_cm_ctx.obj_entry_list);
        aic_lwm2m_client_update_lifetime(s_lwm2m_cm_ctx.config.miplopen_lifetime);
        /* register ack id callback for NOTIFY handle */
        aic_lwm2m_register_session_ack_callback(&s_lwm2m_cm_ctx.client_session, aic_lwm2m_cm_ack_id_callback);
        /* register dtls aged callback */
        aic_lwm2m_register_session_dtls_age_callback(&s_lwm2m_cm_ctx.client_session, aic_lwm2m_cm_dtls_age_callback);
        /* register write callback for non-existing obj WRITE */
        aic_lwm2m_register_session_write_callback(&s_lwm2m_cm_ctx.client_session, aic_lwm2m_cm_write_callback);
        /* set authcode */
        aic_lwm2m_client_authcode_set(s_lwm2m_cm_ctx.dtls_info.authcode, strlen(s_lwm2m_cm_ctx.dtls_info.authcode));
        /* set dtls renegotiation period */
        aic_lwm2m_cm_set_dtls_renegotiation_period();
        if (s_lwm2m_cm_ctx.is_bootstrap && s_lwm2m_cm_ctx.bootstrap_state)
            ret = aic_lwm2m_client_register(&s_lwm2m_cm_ctx.client_session, &s_lwm2m_cm_ctx.active_server.addr,
                                            s_lwm2m_cm_ctx.active_server.port, AIC_LWM2M_CLIENT_RESERVED_ID,
                                            s_lwm2m_cm_ctx.dtls_info.lwm2m_key, s_lwm2m_cm_ctx.dtls_info.lwm2m_key_size,
                                            s_lwm2m_cm_ctx.dtls_info.lwm2m_identity,
                                            s_lwm2m_cm_ctx.dtls_info.lwm2m_identity_size, s_lwm2m_cm_ctx.is_dtls);
        else
            ret = aic_lwm2m_client_register(&s_lwm2m_cm_ctx.client_session, &s_lwm2m_cm_ctx.init_server.addr,
                                            s_lwm2m_cm_ctx.init_server.port, AIC_LWM2M_CLIENT_RESERVED_ID,
                                            s_lwm2m_cm_ctx.dtls_info.lwm2m_key, s_lwm2m_cm_ctx.dtls_info.lwm2m_key_size,
                                            s_lwm2m_cm_ctx.dtls_info.lwm2m_identity,
                                            s_lwm2m_cm_ctx.dtls_info.lwm2m_identity_size, s_lwm2m_cm_ctx.is_dtls);
        if (ret == AIC_LWM2M_CLIENT_SUCCESS && !s_lwm2m_cm_ctx.is_bootstrap) {
            aic_lwm2m_cm_active_server_set(&s_lwm2m_cm_ctx.init_server.addr, s_lwm2m_cm_ctx.init_server.port,
                                           AIC_FALSE);
        }
    }
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: send update request
 *      AT+MIPLUPDATE=<ref>,<lifetime>,<withObjectFlag>
 */
static u32 handle_miplupdate_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    int ref, withObjectFlag = 0, lifetime;
    ulong *paras_arr;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLUPDATE=", &cnt, NULL);
    if (!paras_arr || cnt != 3) {
        aicx_error(0x15b8ea, "", "[+MIPLUPDATE]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <lifetime> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        lifetime = atoi((u8 *)paras_arr[1]);
        /* As specified in AT manual, lifetime should be set as default on when less than 10 */
        if (lifetime < 10) {
            lifetime = LWM2M_AT_LIFETIME_DEFAULT;
        }
        s_lwm2m_cm_ctx.config.miplopen_lifetime = (u32)lifetime;
    } else {
        goto ERROR_EXIT;
    }
    /* parse <withObjectFlag> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        withObjectFlag = atoi((u8 *)paras_arr[2]);
    } else {
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.session_state != NX_LWM2M_CLIENT_SESSION_REGISTERED) {
        aicx_error(0x1284a2, "", "[LWM2M CM] NOT REGISTERED.");
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_client_update(&s_lwm2m_cm_ctx.client_session, s_lwm2m_cm_ctx.config.miplopen_lifetime,
                                  (boolean)withObjectFlag);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: send de-register request
 *      AT+MIPLCLOSE=<ref>
 */
static u32 handle_miplclose_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, err_code = LWM2M_CM_INVAL_AT_PARAM;
    u32 ret;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLCLOSE=", &cnt, NULL);
    if (!paras_arr || cnt != 1) {
        aicx_error(0x1326b6, "", "[+MIPLCLOSE]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_client_logout(&s_lwm2m_cm_ctx.client_session);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    /* clean up all things created for the session, but don't clean up objects and configs */
    aic_lwm2m_cm_reset_session();
    aic_lwm2m_cm_active_server_set(NULL, 0, AIC_FALSE);
    /* deregister callbacks */
    aic_lwm2m_unregister_session_ack_callback(&s_lwm2m_cm_ctx.client_session);
    aic_lwm2m_unregister_session_write_callback(&s_lwm2m_cm_ctx.client_session);
    aic_lwm2m_unregister_session_dtls_age_callback(&s_lwm2m_cm_ctx.client_session);
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: notify specific resource
 *      AT+MIPLNOTIFY=<ref>,<mid>,<objid>,<insid>,<resid>,<type>,<len>,<value>,<index>,<flag>[,<ackid>[,<contentType>]]
 */
static u32 handle_miplnotify_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt = 0;
    ulong *paras_arr;
    int ref, mid, objid = 0, insid, resid, type, len, index, flag, ackid = 0, contentType;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    u8 *value = NULL, *buf = NULL;
    u32 ret;

    //paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLNOTIFY=", &cnt);
    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLNOTIFY=", &cnt, NULL);
    if (!paras_arr || cnt < 10 || cnt > 12) {
        aicx_error(0x19a1cb, "", "[+MIPLNOTIFY]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            aicx_error(0x044334, "", "[+MIPLNOTIFY]: invalid ref\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x1c990d, "", "[+MIPLNOTIFY]: invalid ref #2\n");
        goto ERROR_EXIT;
    }
    /* parse <mid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        mid = atoi((u8 *)paras_arr[1]);
        if (mid < 0) {
            aicx_error(0x0193d4, "", "[+MIPLNOTIFY]: invalid mid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0142bf, "", "[+MIPLNOTIFY]: invalid mid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <objid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        objid = atoi((u8 *)paras_arr[2]);
        if (objid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            aicx_error(0x122092, "", "[+MIPLNOTIFY]: invalid objid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0be4d7, "", "[+MIPLNOTIFY]: invalid objid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <insid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3]))) {
        insid = atoi((u8 *)paras_arr[3]);
        if (insid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            aicx_error(0x15b81c, "", "[+MIPLNOTIFY]: invalid insid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x182deb, "", "[+MIPLNOTIFY]: invalid insid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <resid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4]))) {
        resid = atoi((u8 *)paras_arr[4]);
        if (resid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            aicx_error(0x101113, "", "[+MIPLNOTIFY]: invalid resid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x1441a3, "", "[+MIPLNOTIFY]: invalid resid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <type> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[5], strlen((u8 *)paras_arr[5]))) {
        type = atoi((u8 *)paras_arr[5]);
        if (type < 0 || type >= LWM2M_CM_RES_TYPE_TOTAL) {
            aicx_error(0x01d778, "", "[+MIPLNOTIFY]: invalid type\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x1ac97a, "", "[+MIPLNOTIFY]: invalid type #2\n");
        goto ERROR_EXIT;
    }
    /* parse <len>: no use parameter as specified in AT manual */
    if (aic_is_valid_integer_para((u8 *)paras_arr[6], strlen((u8 *)paras_arr[6]))) {
        len = atoi((u8 *)paras_arr[6]);
    } else {
        aicx_error(0x1551d1, "", "[+MIPLNOTIFY]: invalid len\n");
        goto ERROR_EXIT;
    }
    /* check data length */
    if (strlen((u8 *)paras_arr[7]) > LWM2M_AT_NOTIFY_DATALEN_MAX)
        goto ERROR_EXIT;
    /* check string type */
    if (type == LWM2M_CM_RES_TYPE_STRING) { // && !aic_cm_split_para_is_quotated_str(paras_arr[7])) {
        ret = aic_lwm2m_cm_parse_string((u8 *)paras_arr[7], strlen((u8 *)paras_arr[7]));
        if (ret == AIC_EFAULT)
            goto ERROR_EXIT;
    }

    /* check hex-string type */
    if (type == LWM2M_CM_RES_TYPE_HEX_STR) {
        buf = aos_zalloc(strlen((u8 *)paras_arr[7])/2 + 1);
        ret = aic_hexstring_to_char(buf, (u8 *)paras_arr[7], strlen((u8 *)paras_arr[7]));
        aos_free(buf);
        if (ret != AIC_TCPIP_AT_OK)
            goto ERROR_EXIT;
    }
    value = (u8 *)paras_arr[7];
    /* parse <index> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[8], strlen((u8 *)paras_arr[8]))) {
        index = atoi((u8 *)paras_arr[8]);
    } else {
        aicx_error(0x011edb, "", "[+MIPLNOTIFY]: invalid index\n");
        goto ERROR_EXIT;
    }
    /* parse <flag> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[9], strlen((u8 *)paras_arr[9]))) {
        flag = atoi((u8 *)paras_arr[9]);
        if (flag != LWM2M_CM_RES_FLAG_LAST_MSG && flag != LWM2M_CM_RES_FLAG_FIRST_MSG &&
            flag != LWM2M_CM_RES_FLAG_INTER_MSG && flag != LWM2M_CM_RES_FLAG_ENTER_IDLE_AFTER_SENT &&
            flag != LWM2M_CM_RES_FLAG_ENTER_IDLE_AFTER_RECV_ACK) {
            aicx_error(0x195fee, "", "[+MIPLNOTIFY]: invalid flag\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0c1d54, "", "[+MIPLNOTIFY]: invalid flag #2\n");
        goto ERROR_EXIT;
    }
    /* parse <ackid> */
    if (cnt > 10) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[10], strlen((u8 *)paras_arr[10]))) {
            ackid = atoi((u8 *)paras_arr[10]);
        } else {
            aicx_error(0x1b21aa, "", "[+MIPLNOTIFY]: invalid ackid\n");
            goto ERROR_EXIT;
        }
    }
    /* parse <contentType> */
    if (cnt == 12) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[11], strlen((u8 *)paras_arr[11]))) {
            contentType = atoi((u8 *)paras_arr[11]);
        } else {
            aicx_error(0x0f92a2, "", "[+MIPLNOTIFY]: invalid contentType\n");
            goto ERROR_EXIT;
        }
    }
    /* Note: notify command can be recevied even if not registered */
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    /* add resource value into obj entry list */
    ret = aic_lwm2m_cm_obj_entry_list_set_res_value(&s_lwm2m_cm_ctx.obj_entry_list, objid, insid, resid, type, value);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x0a41d0, "", "[+MIPLNOTIFY]: failed to add to obj entry list.");
        err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
        goto ERROR_EXIT;
    }
    /* add to notify entry list */
    ret = aic_lwm2m_cm_notify_entry_list_add(&s_lwm2m_cm_ctx.notify_entry_list, objid, insid, resid);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x0aa9b9, "", "[+MIPLNOTIFY]: failed to add to notify entry list.");
        err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
        goto ERROR_EXIT;
    }
    /* Do not set resource until flag is last one */
    if (flag == LWM2M_CM_RES_FLAG_LAST_MSG) {
        LWM2M_CM_NOTIFY_ENTRY *notify_entry = NULL;
        LWM2M_CM_OBJ_INS_ENTRY *ins_entry = NULL;

        /* get obj instance */
        ret = aic_lwm2m_cm_obj_entry_list_get_instance(&s_lwm2m_cm_ctx.obj_entry_list, objid, insid, &ins_entry);
        if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
            aicx_error(0x0cd2ac, "", "[+MIPLNOTIFY]: failed to get obj instance");
            err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
            goto ERROR_EXIT;
        }
        list_for_each_entry(notify_entry, &s_lwm2m_cm_ctx.notify_entry_list.list, node, LWM2M_CM_NOTIFY_ENTRY)
        {
            NX_LWM2M_CLIENT_RESOURCE nx_res_value;
            u8 *res_value = NULL;
            u32 res_value_len = 0;
            u8 res_type = AIC_LWM2M_CLIENT_RESOURCE_NONE;

            aic_lwm2m_client_resource_info_set(&nx_res_value, notify_entry->res_id, 0);
            ret = aic_lwm2m_cm_obj_entry_list_get_res_entry(&s_lwm2m_cm_ctx.obj_entry_list, objid, (u16)insid,
                                                            (u16)notify_entry->res_id, &res_value, &res_value_len,
                                                            &res_type);
            if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
                aic_lwm2m_client_resource_set(&nx_res_value, res_value, res_value_len, res_type);
            } else {
                aicx_error(0x1e6d70, "", "[+MIPLNOTIFY] no resource");
                err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
                goto ERROR_EXIT;
            }
            aic_lwm2m_client_set_msg_id(ackid);
            aic_lwm2m_client_obj_resource_changed(objid, &ins_entry->object_instance, &nx_res_value);
        }
        if (ackid) {
            /* cache ackid */
            aic_lwm2m_cm_ack_id_add(ackid);
        }
        /* free notify entry list since the req has been handled */
        aic_lwm2m_cm_notify_entry_list_cleanup(&s_lwm2m_cm_ctx.notify_entry_list);
    } else {
        aicx_info(0x062c16, "", "[+MIPLNOTIFY] new NOTIFY fragment received and cached.");
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: acknowledge the read response
 *      AT+MIPLREADRSP=<ref>,<mid>,<result>[,<objid>,<insid>,<resid>,<type>,<len>,<value>,<index>,<flag>]
 */
static u32 handle_miplreadrsp_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, mid, result, objid = 0, insid = 0, resid = 0, type = 0, len, index, flag = 0;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    u8 *value = NULL;
    u32 ret;
    LWM2M_CM_TOKEN token = {0};
    boolean to_send_resp = AIC_FALSE;
    UINT format = AIC_LWM2M_CLIENT_FORMAT_TLV;
    NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instances = NULL;
    NX_LWM2M_CLIENT_RESOURCE temp_res = {0};
    NX_LWM2M_CLIENT_RESOURCE *resource_ptr = NULL;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLREADRSP=", &cnt, NULL);
    if (!paras_arr || (cnt != 3 && cnt != 11)) {
        aicx_error(0x05995a, "", "[+MIPLREADRSP]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            aicx_error(0x1d4c5b, "", "[+MIPLREADRSP]: invalid ref\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0a6ae5, "", "[+MIPLREADRSP]: invalid ref #2\n");
        goto ERROR_EXIT;
    }
    /* parse <mid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        mid = atoi((u8 *)paras_arr[1]);
        if (mid < 0) {
            aicx_error(0x048c70, "", "[+MIPLREADRSP]: invalid mid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x177f16, "", "[+MIPLREADRSP]: invalid mid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <result> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        result = atoi((u8 *)paras_arr[2]);
        if (result != AIC_LWM2M_CLIENT_RESULT_CONTENT_OK && result != AIC_LWM2M_CLIENT_RESULT_BAD_REQUEST &&
            result != AIC_LWM2M_CLIENT_RESULT_UNAUTHORIZED && result != AIC_LWM2M_CLIENT_RESULT_NOT_FOUND &&
            result != AIC_LWM2M_CLIENT_RESULT_METHOD_NOT_ALLOWED && result != AIC_LWM2M_CLIENT_RESULT_NOT_ACCEPTABLE) {
            aicx_error(0x14cdf2, "", "[+MIPLREADRSP]: invalid result\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x122947, "", "[+MIPLREADRSP]: invalid result #2\n");
        goto ERROR_EXIT;
    }
    if (cnt == 3)
        goto FINISH_CHECK;
    /* parse <objid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3]))) {
        objid = atoi((u8 *)paras_arr[3]);
        if (objid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            aicx_error(0x1b46f9, "", "[+MIPLREADRSP]: invalid objid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x04ccd3, "", "[+MIPLREADRSP]: invalid objid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <insid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4]))) {
        insid = atoi((u8 *)paras_arr[4]);
        if (insid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            aicx_error(0x0ff010, "", "[+MIPLREADRSP]: invalid insid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x1d3f03, "", "[+MIPLREADRSP]: invalid insid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <resid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[5], strlen((u8 *)paras_arr[5]))) {
        resid = atoi((u8 *)paras_arr[5]);
        if (resid >= AIC_LWM2M_CLIENT_RESERVED_ID) {
            aicx_error(0x136c22, "", "[+MIPLREADRSP]: invalid resid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0ec3fa, "", "[+MIPLREADRSP]: invalid resid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <type> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[6], strlen((u8 *)paras_arr[6]))) {
        type = atoi((u8 *)paras_arr[6]);
        if (type < 0 || type >= LWM2M_CM_RES_TYPE_TOTAL) {
            aicx_error(0x1aade0, "", "[+MIPLREADRSP]: invalid type\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0286a7, "", "[+MIPLREADRSP]: invalid type #2\n");
        goto ERROR_EXIT;
    }
    /* parse <len>: no use parameter as specified in AT manual */
    if (aic_is_valid_integer_para((u8 *)paras_arr[7], strlen((u8 *)paras_arr[7]))) {
        len = atoi((u8 *)paras_arr[7]);
    } else {
        aicx_error(0x02fcc5, "", "[+MIPLREADRSP]: invalid len\n");
        goto ERROR_EXIT;
    }
    /* skip parse <value> here */
    value = (u8 *)paras_arr[8];
    /* parse <index> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[9], strlen((u8 *)paras_arr[9]))) {
        index = atoi((u8 *)paras_arr[9]);
    } else {
        aicx_error(0x063fb0, "", "[+MIPLREADRSP]: invalid index\n");
        goto ERROR_EXIT;
    }
    /* parse <flag> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[10], strlen((u8 *)paras_arr[10]))) {
        flag = atoi((u8 *)paras_arr[10]);
        if (flag != LWM2M_CM_RES_FLAG_LAST_MSG && flag != LWM2M_CM_RES_FLAG_FIRST_MSG &&
            flag != LWM2M_CM_RES_FLAG_INTER_MSG && flag != LWM2M_CM_RES_FLAG_ENTER_IDLE_AFTER_SENT &&
            flag != LWM2M_CM_RES_FLAG_ENTER_IDLE_AFTER_RECV_ACK) {
            aicx_error(0x00702d, "", "[+MIPLREADRSP]: invalid flag\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x1c0d65, "", "[+MIPLREADRSP]: invalid flag #2\n");
        goto ERROR_EXIT;
    }

FINISH_CHECK:
    if (s_lwm2m_cm_ctx.session_state != NX_LWM2M_CLIENT_SESSION_REGISTERED) {
        aicx_error(0x1284a2, "", "[LWM2M CM] NOT REGISTERED.");
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    if (result == AIC_LWM2M_CLIENT_RESULT_CONTENT_OK) {
        LWM2M_CM_READ_ENTRY *read_entry = NULL;

        ret = aic_lwm2m_cm_read_entry_list_get_req(&s_lwm2m_cm_ctx.read_entry_list, mid, objid, &read_entry);
        if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
            aicx_error(0x0ecd5f, "", "[+MIPLREADRSP]: invalid read response.");
            err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
            goto ERROR_EXIT;
        }
        /* add resource value into list */
        ret =
            aic_lwm2m_cm_obj_entry_list_set_res_value(&s_lwm2m_cm_ctx.obj_entry_list, objid, insid, resid, type, value);
        if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
            aicx_error(0x01368e, "", "[+MIPLREADRSP]: failed to add res entry list.");
            err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
            goto ERROR_EXIT;
        }
        /* check flag if it is the last one */
        if (flag == LWM2M_CM_RES_FLAG_LAST_MSG) {
            /* estimate format of response */
            if (read_entry->req.res_id != -1) {
                switch (type) {
                case LWM2M_CM_RES_TYPE_STRING:
                    format = AIC_LWM2M_CLIENT_FORMAT_TEXT;
                    break;
                case LWM2M_CM_RES_TYPE_HEX_STR:
                case LWM2M_CM_RES_TYPE_OPAQUE:
                    format = AIC_LWM2M_CLIENT_FORMAT_OPAQUE;
                    break;
                case LWM2M_CM_RES_TYPE_INTEGER:
                case LWM2M_CM_RES_TYPE_FLOAT:
                case LWM2M_CM_RES_TYPE_BOOL:
                default:
                    break;
                }
            }
            /* get obj instance */
            if (read_entry->req.obj_ins_id != -1) {
                LWM2M_CM_OBJ_INS_LIST *ins_list = NULL;
                s32 i = 0;

                ret = aic_lwm2m_cm_obj_entry_list_get_instance_list(&s_lwm2m_cm_ctx.obj_entry_list, objid, &ins_list);
                if (ret != AIC_LWM2M_CLIENT_SUCCESS || !ins_list) {
                    aicx_error(0x07faaf, "", "[+MIPLREADRSP] no instance #1");
                    err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
                    goto ERROR_EXIT;
                }
                for (i = 0; i < ins_list->count; i++) {
                    if (ins_list->entries[i].is_available &&
                        ins_list->entries[i].object_instance.object_instance_id == read_entry->req.obj_ins_id) {
                        object_instances = &ins_list->entries[i].object_instance;
                        break;
                    }
                }
                if (!object_instances) {
                    aicx_error(0x0a7308, "", "[+MIPLREADRSP] no instance #2");
                    err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
                    goto ERROR_EXIT;
                }
            }
            /* get resource id */
            if (read_entry->req.obj_ins_id != -1 && read_entry->req.res_id != -1) {
                u8 *res_value = NULL;
                u32 res_value_len = 0;
                u8 res_type = AIC_LWM2M_CLIENT_RESOURCE_NONE;

                ret = aic_lwm2m_cm_obj_entry_list_get_res_entry(&s_lwm2m_cm_ctx.obj_entry_list, objid, (u16)insid,
                                                                (u16)resid, &res_value, &res_value_len, &res_type);
                if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
                    aic_lwm2m_client_resource_set(&temp_res, res_value, res_value_len, res_type);
                    resource_ptr = &temp_res;
                } else {
                    aicx_error(0x18e0bb, "", "[+MIPLREADRSP] no resource");
                    err_code = LWM2M_CM_RESOURCE_OPERATION_ERR;
                    goto ERROR_EXIT;
                }
            }
            to_send_resp = AIC_TRUE;
        }
    } else {
        to_send_resp = AIC_TRUE;
    }
    if (to_send_resp) {
        /* get token */
        aic_lwm2m_cm_token_entry_list_get(mid, &token);
        if (!token.data) {
            aicx_error(0x097e3e, "", "[+MIPLREADRSP]: no available token ");
            err_code = LWM2M_CM_UNKNOW_ERR;
            goto ERROR_EXIT;
        }
        ret = aic_lwm2m_client_session_send_response(&s_lwm2m_cm_ctx.client_session, result, mid, token.data, token.len,
                                                     AIC_FALSE, format, objid, object_instances, resource_ptr);
        if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
            aicx_error(0x1d4059, "", "[+MIPLREADRSP]: failed to send response ");
            err_code = LWM2M_CM_UNKNOW_ERR;
            goto ERROR_EXIT;
        }
        aic_lwm2m_cm_cache_nnmi_remove_top(mid);
        if (token.data) {
            aos_free(token.data);
        }
    }

    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr); 
    /* handle MIPLSMI */
    aic_lwm2m_cm_miplsmi_urc(ret, mid);
    return AIC_SUCCESS;

ERROR_EXIT:
    if (token.data) {
        aos_free(token.data);
    }
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: acknowledge the write response
 *      AT+MIPLWRITERSP=<ref>,<mid>,<result>
 */
static u32 handle_miplwritersp_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, mid, result;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    LWM2M_CM_TOKEN token = {0};
    u32 ret;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLWRITERSP=", &cnt, NULL);
    if (!paras_arr || cnt != 3) {
        aicx_error(0x1d93de, "", "[+MIPLWRITERSP]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            aicx_error(0x1c78bd, "", "[+MIPLWRITERSP]: invalid ref\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x11123a, "", "[+MIPLWRITERSP]: invalid ref #2\n");
        goto ERROR_EXIT;
    }
    /* parse <mid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        mid = atoi((u8 *)paras_arr[1]);
        if (mid < 0) {
            aicx_error(0x1030d1, "", "[+MIPLWRITERSP]: invalid mid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x04e1d0, "", "[+MIPLWRITERSP]: invalid mid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <result> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        result = atoi((u8 *)paras_arr[2]);
        if (result != AIC_LWM2M_CLIENT_RESULT_STATUS_CHANGED && result != AIC_LWM2M_CLIENT_RESULT_BAD_REQUEST &&
            result != AIC_LWM2M_CLIENT_RESULT_UNAUTHORIZED && result != AIC_LWM2M_CLIENT_RESULT_NOT_FOUND &&
            result != AIC_LWM2M_CLIENT_RESULT_METHOD_NOT_ALLOWED) {
            aicx_error(0x0cf380, "", "[+MIPLWRITERSP]: invalid result\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0a8f39, "", "[+MIPLWRITERSP]: invalid result #2\n");
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.session_state != NX_LWM2M_CLIENT_SESSION_REGISTERED) {
        aicx_error(0x1284a2, "", "[LWM2M CM] NOT REGISTERED.");
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    /* get token */
    aic_lwm2m_cm_token_entry_list_get(mid, &token);
    if (!token.data) {
        aicx_error(0x11cc54, "", "[+MIPLWRITERSP]: no available token ");
        err_code = LWM2M_CM_UNKNOW_ERR;
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_client_session_send_response(&s_lwm2m_cm_ctx.client_session, result, mid, token.data, token.len,
                                                 AIC_FALSE, 0, AIC_LWM2M_CLIENT_RESERVED_ID, NULL, NULL);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x03ac6c, "", "[+MIPLWRITERSP]: failed to send response ");
        err_code = LWM2M_CM_UNKNOW_ERR;
        goto ERROR_EXIT;
    }
    aic_lwm2m_cm_cache_nnmi_remove_top(mid);
    if (token.data) {
        aos_free(token.data);
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    /* handle MIPLSMI */
    aic_lwm2m_cm_miplsmi_urc(ret, mid);
    return AIC_SUCCESS;

ERROR_EXIT:
    if (token.data) {
        aos_free(token.data);
    }
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: acknowledge the execute response
 *      AT+MIPLEXECUTERSP=<ref>,<mid>,<result>
 */
static u32 handle_miplexecutersp_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, mid, result;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    LWM2M_CM_TOKEN token = {0};
    u32 ret;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLEXECUTERSP=", &cnt, NULL);
    if (!paras_arr || cnt != 3) {
        aicx_error(0x0997f9, "", "[+MIPLEXECUTERSP]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            aicx_error(0x1c1e90, "", "[+MIPLEXECUTERSP]: invalid ref\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x122935, "", "[+MIPLEXECUTERSP]: invalid ref #2\n");
        goto ERROR_EXIT;
    }
    /* parse <mid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        mid = atoi((u8 *)paras_arr[1]);
        if (mid < 0) {
            aicx_error(0x171e04, "", "[+MIPLEXECUTERSP]: invalid mid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x12c205, "", "[+MIPLEXECUTERSP]: invalid mid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <result> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        result = atoi((u8 *)paras_arr[2]);
        if (result != AIC_LWM2M_CLIENT_RESULT_STATUS_CHANGED && result != AIC_LWM2M_CLIENT_RESULT_BAD_REQUEST &&
            result != AIC_LWM2M_CLIENT_RESULT_UNAUTHORIZED && result != AIC_LWM2M_CLIENT_RESULT_NOT_FOUND &&
            result != AIC_LWM2M_CLIENT_RESULT_METHOD_NOT_ALLOWED) {
            aicx_error(0x07538f, "", "[+MIPLEXECUTERSP]: invalid result\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x125b2e, "", "[+MIPLEXECUTERSP]: invalid result #2\n");
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.session_state != NX_LWM2M_CLIENT_SESSION_REGISTERED) {
        aicx_error(0x1284a2, "", "[LWM2M CM] NOT REGISTERED.");
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    /* get token */
    aic_lwm2m_cm_token_entry_list_get(mid, &token);
    if (!token.data) {
        aicx_error(0x1d1307, "", "[+MIPLEXECUTERSP]: no available token ");
        err_code = LWM2M_CM_UNKNOW_ERR;
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_client_session_send_response(&s_lwm2m_cm_ctx.client_session, result, mid, token.data, token.len,
                                                 AIC_FALSE, 0, AIC_LWM2M_CLIENT_RESERVED_ID, NULL, NULL);
    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
        aicx_error(0x0a3b63, "", "[+MIPLEXECUTERSP]: failed to send response ");
        err_code = LWM2M_CM_UNKNOW_ERR;
        goto ERROR_EXIT;
    }
    aic_lwm2m_cm_cache_nnmi_remove_top(mid);
    if (token.data) {
        aos_free(token.data);
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    /* handle MIPLSMI */
    aic_lwm2m_cm_miplsmi_urc(ret, mid);
    return AIC_SUCCESS;

ERROR_EXIT:
    if (token.data) {
        aos_free(token.data);
    }
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: acknowledge the write-attribute response
 *      AT+MIPLPARAMETERRSP=<ref>,<mid>,<result>
 * Do nothing at all except check if the parameters are valid or not, since WRITE ATTRIBUTE has been handled by service.
 */
static u32 handle_miplparameterrsp_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, mid, result;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLPARAMETERRSP=", &cnt, NULL);
    if (!paras_arr || cnt != 3) {
        aicx_error(0x12a1b9, "", "[+MIPLPARAMETERRSP]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            aicx_error(0x16c5bf, "", "[+MIPLPARAMETERRSP]: invalid ref\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x1e2947, "", "[+MIPLPARAMETERRSP]: invalid ref #2\n");
        goto ERROR_EXIT;
    }
    /* parse <mid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        mid = atoi((u8 *)paras_arr[1]);
        if (mid < 0) {
            aicx_error(0x0f115c, "", "[+MIPLPARAMETERRSP]: invalid mid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0ce597, "", "[+MIPLPARAMETERRSP]: invalid mid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <result> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        result = atoi((u8 *)paras_arr[2]);
        if (result < 0) {
            aicx_error(0x1d1d3b, "", "[+MIPLPARAMETERRSP]: invalid result\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x0e418b, "", "[+MIPLPARAMETERRSP]: invalid result #2\n");
        goto ERROR_EXIT;
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: acknowledge the write-attribute response
 *      AT+MIPLOBSERVERSP=<ref>,<msgid>,<result>
 * Do nothing at all except check if the parameters are valid or not, since OBSERVE RESP has been handled by service.
 */
static u32 handle_miplobserversp_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, mid, result;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLOBSERVERSP=", &cnt, NULL);
    if (!paras_arr || cnt != 3) {
        aicx_error(0x108720, "", "[+MIPLOBSERVERSP]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            aicx_error(0x0f9f14, "", "[+MIPLOBSERVERSP]: invalid ref\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x091788, "", "[+MIPLOBSERVERSP]: invalid ref #2\n");
        goto ERROR_EXIT;
    }
    /* parse <mid> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        mid = atoi((u8 *)paras_arr[1]);
        if (mid < 0) {
            aicx_error(0x0d03a8, "", "[+MIPLOBSERVERSP]: invalid mid\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x14e338, "", "[+MIPLOBSERVERSP]: invalid mid #2\n");
        goto ERROR_EXIT;
    }
    /* parse <result> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        result = atoi((u8 *)paras_arr[2]);
        if (result < 0) {
            aicx_error(0x11458b, "", "[+MIPLOBSERVERSP]: invalid result\n");
            goto ERROR_EXIT;
        }
    } else {
        aicx_error(0x151566, "", "[+MIPLOBSERVERSP]: invalid result #2\n");
        goto ERROR_EXIT;
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    aic_cm_free_paras_mem(paras_arr, cnt);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: config login parameters
 *      a. AT+MIPLCFG="platform",<ref>,<platform>[,<pattern>]
 *      b. AT+MIPLCFG="platform",<ref>,<platform>[,<epname>]
 *      c. AT+MIPLCFG="binding",<ref>,<binding_mode>
 */
static u32 handle_miplcfg_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int str_len, pattern = MIPLCFG_PATTERN_DEFAULT, binding_mode, platform, ref;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    u32 ret;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLCFG=", &cnt, NULL);
    if (!paras_arr || cnt < 3 || cnt > 4) {
        aicx_error(0x1ac827, "", "[+MIPLCFG]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
        goto ERROR_EXIT;
    ref = atoi((u8 *)paras_arr[1]);
    if (ref != 0) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    str_len = strlen((u8 *)paras_arr[0]);
    if (strncasecmp((u8 *)paras_arr[0], "platform", sizeof("platform")) == 0) {
        /* parse <platform> */
        if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
            goto ERROR_EXIT;
        platform = atoi((u8 *)paras_arr[2]);
        if (platform < 0 || (platform >= MIPLCFG_PLATFORM_CNT && platform != MIPLCFG_PLATFORM_OTHER))
            goto ERROR_EXIT;
        if (platform == MIPLCFG_PLATFORM_OTHER) {
            if (cnt != 4)
                goto ERROR_EXIT;
            /* <endpoint> */
            s_lwm2m_cm_ctx.config.miplcfg_pattern = MIPLCFG_PATTERN_SPECIFIC_ENDPOINT;
            aic_lwm2m_client_endpoint_set((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3]));
        } else {
            if (cnt == 4) {
                if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))
                    goto ERROR_EXIT;
                pattern = atoi((u8 *)paras_arr[3]);
            }
            if (pattern <= MIPLCFG_PATTERN_URN_IMEI_IMSI) {
                if (pattern == MIPLCFG_PATTERN_DEFAULT) {
                    ret = aic_lwm2m_cm_set_default_pattern(platform);
                    if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
                        goto ERROR_EXIT;
                    }
                } else {
                    s_lwm2m_cm_ctx.config.miplcfg_pattern = pattern;
                }
            } else {
                goto ERROR_EXIT;
            }
        }
        /* set pskid in case it was not specified in device creation */
        aic_lwm2m_cm_set_default_pskid(platform == MIPLCFG_PLATFORM_ONENET);
    } else if (strncasecmp((u8 *)paras_arr[0], "binding", sizeof("binding")) == 0) {
        if (cnt != 3)
            goto ERROR_EXIT;
        if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
            goto ERROR_EXIT;
        binding_mode = atoi((u8 *)paras_arr[2]);
        if (binding_mode >= MIPLCFG_BM_TOTAL) {
            goto ERROR_EXIT;
        } else {
            s_lwm2m_cm_ctx.config.miplcfg_bindmode = (u32)binding_mode;
        }
    } else {
        goto ERROR_EXIT;
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: config notification type
 *      AT+MIPLNMI=<ref>,<nnmi>,<nsmi>
 */
static u32 handle_miplnmi_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, err_code = LWM2M_CM_INVAL_AT_PARAM;
    int nnmi, nsmi;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLNMI=", &cnt, NULL);
    if (!paras_arr || cnt != 3) {
        aicx_error(0x05621b, "", "[+MIPLNMI]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <nnmi> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        nnmi = atoi((u8 *)paras_arr[1]);
        if (nnmi < MIPLNMI_NNMI_DEFAULT || nnmi >= MIPLNMI_NNMI_TOTAL)
            goto ERROR_EXIT;
    } else {
        goto ERROR_EXIT;
    }
    /* parse <nsmi> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
        nsmi = atoi((u8 *)paras_arr[2]);
        if (nsmi < 0 || nsmi > 1)
            goto ERROR_EXIT;
    } else {
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    s_lwm2m_cm_ctx.config.miplnmi_nnmi = (u8)nnmi;
    if (nsmi) {
        s_lwm2m_cm_ctx.config.miplnmi_nsmi = AIC_TRUE;
    } else {
        s_lwm2m_cm_ctx.config.miplnmi_nsmi = AIC_FALSE;
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: fetch a cached command
 *      AT+MIPLMGR=<ref>
 */
static u32 handle_miplmgr_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, err_code = LWM2M_CM_INVAL_AT_PARAM;
    char *cmd = NULL;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLMGR=", &cnt, NULL);
    if (!paras_arr || cnt != 1) {
        aicx_error(0x162fad, "", "[+MIPLMGR]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    aic_lwm2m_cm_cache_nnmi_get(&cmd);
    if (cmd) {
        aic_transmit_resp_normal(at_req_resp_ptr, cmd);
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: Query statistics of cached urc commands
 *      AT+MIPLQMGR=<ref>
 */
static u32 handle_miplqmgr_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, err_code = LWM2M_CM_INVAL_AT_PARAM;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLQMGR=", &cnt, NULL);
    if (!paras_arr || cnt != 1) {
        aicx_error(0x02ce77, "", "[+MIPLQMGR]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.session_state != NX_LWM2M_CLIENT_SESSION_REGISTERED) {
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    } else {
        u8 str[AT_RESP_RESULT_MAX] = {'\0'};

        aic_lwm2m_cm_cach_nnmi_get_miplqmgr_resp(str);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    }

    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: enable auto update mechanism
 *      AT+MIPLUPDATESET=<ref>,<enable>
 */
static u32 handle_miplupdateset_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    int ref, enable, err_code = LWM2M_CM_INVAL_AT_PARAM;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLUPDATESET=", &cnt, NULL);
    if (!paras_arr || cnt != 2) {
        aicx_error(0x0955c6, "", "[+MIPLUPDATESET]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <enable> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        enable = atoi((u8 *)paras_arr[1]);
        if (enable < 0 || enable > 1) {
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE || !aic_lwm2m_cm_session_is_idle()) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    if (enable) {
        s_lwm2m_cm_ctx.config.miplupdateset_enable = AIC_TRUE;
    } else {
        s_lwm2m_cm_ctx.config.miplupdateset_enable = AIC_FALSE;
    }
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: enable auto update mechanism
 *      AT+MIPLCREATEEX=<host>,<flag>[,<authcode>,<PSK>[,<PSKID>[,<psk_encode>]]]
 */
static u32 handle_miplcreateex_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    u8 *ptr;
    int psk_encode, ret, flag, port, err_code = LWM2M_CM_INVAL_AT_PARAM;
    size_t str_len = 0;
    boolean is_bootstrap = AIC_FALSE;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLCREATEEX=", &cnt, NULL);
    if (!paras_arr || cnt < 2 || cnt > 6) {
        aicx_error(0x1850e9, "", "[+MIPLCREATEEX]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <host> */
    ptr = strchr((u8 *)paras_arr[0], ':');
    if (!ptr) {
        str_len = strlen((u8 *)paras_arr[0]);
        s_lwm2m_cm_ctx.init_server.port = LWM2M_SERVER_PORT;
    } else {
        str_len = ptr - (u8 *)paras_arr[0];
        if (!aic_is_valid_integer_para(ptr + 1, strlen(ptr + 1)))
            goto ERROR_EXIT;
        port = atoi(ptr + 1);
        if (port > 65535 || port < 0)
            goto ERROR_EXIT;
        s_lwm2m_cm_ctx.init_server.port = port;
    }
    if (str_len > LWM2M_AT_HOST_SIZE_MAX - 1 || str_len == 0)
        goto ERROR_EXIT;
    aos_memcpy(s_lwm2m_cm_ctx.host, (u8 *)paras_arr[0], str_len);
    s_lwm2m_cm_ctx.host[str_len] = '\0';
    /* parse <flag> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        flag = atoi((u8 *)paras_arr[1]);
        if (flag > 3) {
            goto ERROR_EXIT;
        }
        if (flag & 0x01) {
            is_bootstrap = AIC_TRUE;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <authcode> */
    if (cnt >= 3) {
        if (strlen((u8 *)paras_arr[2]) > LWM2M_AT_AUTHCODE_SIZE_MAX)
            goto ERROR_EXIT;
        strcpy(s_lwm2m_cm_ctx.dtls_info.authcode, (u8 *)paras_arr[2]);
    }
    /* parse <psk_encode> */
    if (cnt == 6) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[5], strlen((u8 *)paras_arr[5])))
            goto ERROR_EXIT;
        psk_encode = atoi((u8 *)paras_arr[5]);
        if (psk_encode != MIPLCREATEEX_PSK_ENCODE_DEFAULT && psk_encode != MIPLCREATEEX_PSK_ENCODE_HEX)
            goto ERROR_EXIT;
    } else {
        psk_encode = MIPLCREATEEX_PSK_ENCODE_DEFAULT;
    }
    /* parse <PSK> */
    if (cnt >= 4) {
        u8 *psk_key = s_lwm2m_cm_ctx.dtls_info.lwm2m_key;
        u8 *psk_key_size = &s_lwm2m_cm_ctx.dtls_info.lwm2m_key_size;

        if (is_bootstrap) {
            psk_key = s_lwm2m_cm_ctx.dtls_info.bs_key;
            psk_key_size = &s_lwm2m_cm_ctx.dtls_info.bs_key_size;
        }
        str_len = strlen((u8 *)paras_arr[3]);

        aos_memset(psk_key, 0, LWM2M_AT_PSK_SIZE_MAX + 1);
        if (psk_encode == MIPLCREATEEX_PSK_ENCODE_DEFAULT) {
            /* ASCII */
            if (str_len > LWM2M_AT_PSK_SIZE_MAX)
                goto ERROR_EXIT;
            aos_memcpy(psk_key, (u8 *)paras_arr[3], (u32)str_len);
            *psk_key_size = str_len;
        } else {
            /* HEX */
            if ((str_len & 0x1) || str_len > LWM2M_AT_PSK_SIZE_MAX * 2)
                goto ERROR_EXIT;
            aic_hexstring_to_char(psk_key, (u8 *)paras_arr[3], (u32)str_len);
            *psk_key_size = (str_len >> 1);
        }
        s_lwm2m_cm_ctx.is_dtls = AIC_TRUE;
    }
    /* parse <PSKID> */
    if (cnt >= 5) {
        u8 *identity = s_lwm2m_cm_ctx.dtls_info.lwm2m_identity;
        u8 *identity_size = &s_lwm2m_cm_ctx.dtls_info.lwm2m_identity_size;

        str_len = strlen((u8 *)paras_arr[4]);

        if (str_len > LWM2M_AT_PSK_ID_SIZE_MAX)
            goto ERROR_EXIT;
        if (is_bootstrap) {
            identity = s_lwm2m_cm_ctx.dtls_info.bs_identity;
            identity_size = &s_lwm2m_cm_ctx.dtls_info.bs_identity_size;
        }
        strcpy(identity, (u8 *)paras_arr[4]);
        *identity_size = str_len;
    }
    if (s_lwm2m_cm_ctx.server_state != LWM2M_CM_SERVER_STATE_NONE) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    ret = aic_lwm2m_cm_server_init();
    if (ret != NX_SUCCESS) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        aicx_error(0x03afec, "s", "[+MIPLCREATEEX]: failed to init server (%s)", s_lwm2m_cm_ctx.host);
        goto ERROR_EXIT;
    }
    s_lwm2m_cm_ctx.is_bootstrap = is_bootstrap;
    if (flag & 0x2)
        s_lwm2m_cm_ctx.config.enable_moniter = AIC_FALSE;
    else
        s_lwm2m_cm_ctx.config.enable_moniter = AIC_TRUE;
    ret = aic_lwm2m_client_create(&s_lwm2m_cm_ctx.client_session, aic_lwm2m_cm_session_callback);
    if (ret) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.is_bootstrap) {
        s_lwm2m_cm_ctx.server_state = LWM2M_CM_SERVER_STATE_BOOTSTRAP;
    } else {
        s_lwm2m_cm_ctx.server_state = LWM2M_CM_SERVER_STATE_LWM2M;
    }
    s_lwm2m_cm_ctx.com_id = at_req_resp_ptr->serial_port_id;
    strcpy(str, "+MIPLCREATEEX: 0");
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. query command: query parameters of the device instance
 *      AT+MIPLDEVINFO?
 */
static u32 handle_mipldevinfo_query(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    u8 ip_str[ADDR_BUFFERSIZE_IPV6] = {0};
    NXD_ADDRESS *addr = NX_NULL;
    u16 port = 0;
    int flag = 0;
    boolean is_ipv6 = AIC_FALSE;
    u8 *secret_key = NX_NULL;
    u32 secret_key_len = 0;
    int err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;

    if (s_lwm2m_cm_ctx.config.enable_moniter == AIC_FALSE) {
        flag |= 0x2;
    }
    switch (s_lwm2m_cm_ctx.server_state) {
    case LWM2M_CM_SERVER_STATE_BOOTSTRAP:
        flag |= 0x1;
        addr = &s_lwm2m_cm_ctx.init_server.addr;
        port = s_lwm2m_cm_ctx.init_server.port;
        if (s_lwm2m_cm_ctx.is_dtls) {
            secret_key = s_lwm2m_cm_ctx.dtls_info.bs_key;
            secret_key_len = s_lwm2m_cm_ctx.dtls_info.bs_key_size;
        }
        break;
    case LWM2M_CM_SERVER_STATE_LWM2M:
        addr = &s_lwm2m_cm_ctx.active_server.addr;
        port = s_lwm2m_cm_ctx.active_server.port;
        if (s_lwm2m_cm_ctx.is_dtls) {
            secret_key = s_lwm2m_cm_ctx.dtls_info.lwm2m_key;
            secret_key_len = s_lwm2m_cm_ctx.dtls_info.lwm2m_key_size;
        }
        break;
    case LWM2M_CM_SERVER_STATE_NONE:
    default:
        goto ERROR_EXIT;
    }

    if (NX_IP_VERSION_V4 == addr->nxd_ip_version) {
        ULONG temp = addr->nxd_ip_address.v4;

        NX_CHANGE_ULONG_ENDIAN(temp);
        inet_ntop(AF_INET, &temp, ip_str, ADDR_BUFFERSIZE_IPV6);
    } else {
        ULONG temp_arr[4] = {0};

        aos_memcpy(temp_arr, &addr->nxd_ip_address.v6, sizeof(addr->nxd_ip_address.v6));
        NX_CHANGE_ULONG_ENDIAN(temp_arr[0]);
        NX_CHANGE_ULONG_ENDIAN(temp_arr[1]);
        NX_CHANGE_ULONG_ENDIAN(temp_arr[2]);
        NX_CHANGE_ULONG_ENDIAN(temp_arr[3]);
        inet_ntop(AF_INET6, &temp_arr, ip_str, ADDR_BUFFERSIZE_IPV6);
        strupr(ip_str);
        is_ipv6 = AIC_TRUE;
    }
    if (s_lwm2m_cm_ctx.is_dtls) {
        snprintf(str, AT_RESP_RESULT_MAX, "+MIPLDEVINFO: \"%s%s%s:%d\",%d,%s", is_ipv6 ? "[" : "", ip_str,
                 is_ipv6 ? "]" : "", port, flag, s_lwm2m_cm_ctx.dtls_info.authcode);
        if (secret_key) {
            strcat(str, ",\"");
            strncat(str, secret_key, secret_key_len);
            strcat(str, "\"");
        }
    } else {
        snprintf(str, AT_RESP_RESULT_MAX, "+MIPLDEVINFO: \"%s%s%s:%d\",%d,,", is_ipv6 ? "[" : "", ip_str,
                 is_ipv6 ? "]" : "", port, flag);
    }
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. set command: set dtls nat
 *      AT+MIPLDTLSNAT=<ref>,<timeout>
 */
static u32 handle_mipldtlsnat_set(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    int ref, err_code = LWM2M_CM_INVAL_AT_PARAM, timeout;
    ulong *paras_arr;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLDTLSNAT=", &cnt, NULL);
    if (!paras_arr || cnt != 2) {
        aicx_error(0x089c11, "", "[+MIPLDTLSNAT]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    /* parse <ref> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        ref = atoi((u8 *)paras_arr[0]);
        if (ref != 0) {
            err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    /* parse <timeout> */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        timeout = atoi((u8 *)paras_arr[1]);
        if (timeout < 60 && timeout > 0) {
            err_code = LWM2M_CM_GRAMMAR_SYNTAX_ERR;
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    if (s_lwm2m_cm_ctx.server_state == LWM2M_CM_SERVER_STATE_NONE) {
        err_code = LWM2M_CM_OPERATION_NOT_ALLOWED;
        goto ERROR_EXIT;
    }
    /* Note: this timeout can not be enabled until next registration */
    s_lwm2m_cm_ctx.config.mipldtlsnat_timeout = timeout;
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}

/* Themes:
 * 1. execute command: query remaining lifetime
 *      AT+MIPLLTRDP
 */
static u32 handle_miplltrdp_execute(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    int err_code = LWM2M_CM_INVAL_AT_PARAM;
    ulong *paras_arr;

    paras_arr = aic_cm_split_paras_to_str_array(at_req_resp_ptr->at_req, "AT+MIPLLTRDP", &cnt, NULL);
    if (paras_arr) {
        aicx_error(0x1d6ee5, "", "[+MIPLLTRDP]: invalid parameters \n");
        goto ERROR_EXIT;
    }

    if (s_lwm2m_cm_ctx.login_tickcnt != 0) {
        u32 deadline = s_lwm2m_cm_ctx.login_tickcnt + s_lwm2m_cm_ctx.config.miplopen_lifetime * 1000;
        u32 cur_tick = aos_get_tickcount();
        u32 remains = 0;
        u8 str[40] = {'\0'};

        if (cur_tick <= deadline) {
            remains = (deadline - cur_tick) / 1000;
        }
        snprintf(str, 40, "+MIPLLTRDP: %u", remains);
        aic_transmit_resp_normal(at_req_resp_ptr, str);
    } else {
        err_code = LWM2M_CM_DEV_NOT_LOGGED_OR_INPROGRESS;
        goto ERROR_EXIT;
    }
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_SUCCESS;

ERROR_EXIT:
    aic_cm_free_paras_mem(paras_arr, cnt);
    aic_lwm2m_cm_transmit_cis_err(at_req_resp_ptr, err_code);
    return AIC_SUCCESS;
}
/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_lwm2m_cm_at_cmd_init);