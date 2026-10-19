/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  AIC Handling of AT Cmds Relating to TCPIP.
 *  This file implements some AT commands relating to TCPIP,
 *  such as establish a tcp connection.etc.
 *  Depends on:
 *      1. macor CONFIG_PDP_MGR
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x079973
#define LOG_MODULE    AMOD_NET

/*******************************************
 *              INCLUDES
 ******************************************/
#include "aic_at_cmd_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_at_tcpip_common.h"
#include "aic_net_api.h"
#include "aic_net_pdp_mgr.h"
#include "aic_osa_ex.h"
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <nxd_bsd.h>

/*******************************************
 *              DEFINES
 ******************************************/
#define AIC_CTCPIP_SOCKET_MAX_LINKS 10
#define AIC_CTCPIP_SOCKET_LISTEN_MAX 4
#define AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN 1500
#define AIC_CTCPIP_SOCKET_MAX_READ_HEX_DATA_LEN 750
#define AIC_CTCPIP_SOCKET_MAX_SEND_DATA_LEN 1500
#define AIC_CTCPIP_RESP_MAX_RECV_DATA_LEN (AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN + 1)
#define AIC_CTCPIP_SOCKET_RETRY_MAX_ATTEMPT_CNT 5
#define AT_CIPTIMEOUT_DEF_VALUE 120
#define AIC_CTCPIP_DATA_EXT_CHAR 0x03
#define AIC_CTCPIP_DATA_CTRL_Z_CHAR 0x1A
#define AIC_CTCPIP_DATA_ESC_CHAR 0x1B

/* <net_state> */
enum {
    AT_NETOPEN_NET_STATE_CLOSE = 0,
    AT_NETOPEN_NET_STATE_OPEN = 1,
};
/* <err> */
enum {
    AIC_CTCPIP_ERR_SUCCESS = 0,
    AIC_CTCPIP_ERR_NET_FAIL = 1,
    AIC_CTCPIP_ERR_NET_NOT_OPENED = 2,
    AIC_CTCPIP_ERR_WRONG_PARA = 3,
    AIC_CTCPIP_ERR_OPER_NOT_SUPPORT = 4,
    AIC_CTCPIP_ERR_FAIL_TO_CREATE_SOCKET = 5,
    AIC_CTCPIP_ERR_FAIL_TO_BIND_SOCKET = 6,
    AIC_CTCPIP_ERR_TCP_SRV_IS_ALREADY_LISTENING = 7,
    AIC_CTCPIP_ERR_BUSY = 8,
    AIC_CTCPIP_ERR_SOCKETS_OPENED = 9,
    AIC_CTCPIP_ERR_TIMEOUT = 10,
    AIC_CTCPIP_ERR_DNS_PARSE_FAILED = 11,
    AIC_CTCPIP_ERR_UNKNOWN = 12,
};
/* err_info */
enum {
    AIC_IPERROR_INFO_CONN_TIMEOUT = 0,
    AIC_IPERROR_INFO_BIND_PORT_FAILED = 1,
    AIC_IPERROR_INFO_PORT_OVERFLOW = 2,
    AIC_IPERROR_INFO_CREATE_SOCKET_FAILED = 3,
    AIC_IPERROR_INFO_NETWORK_ALREADY_OPENED = 4,
    AIC_IPERROR_INFO_NETWORK_ALREADY_CLOSED = 5,
    AIC_IPERROR_INFO_NO_CLIENTS_CONNECTED = 6,
    AIC_IPERROR_INFO_NO_ACTIVE_CLIENT = 7,
    AIC_IPERROR_INFO_NETWORK_NOT_OPENED = 8,
    AIC_IPERROR_INFO_CLIENT_INDEX_OVERFLOW = 9,
    AIC_IPERROR_INFO_CONN_ALREADY_CREATED = 10,
    AIC_IPERROR_INFO_CONN_NOT_CREATED = 11,
    AIC_IPERROR_INFO_INVALID_PARAM = 12,
    AIC_IPERROR_INFO_OPER_NOT_SUPPORTED = 13,
    AIC_IPERROR_INFO_DNS_QUERY_FAILED = 14,
    AIC_IPERROR_INFO_TCP_BUSY = 15,
    AIC_IPERROR_INFO_NET_CLOSE_FAILED_FOR_SOCKET_OPEN = 16,
    AIC_IPERROR_INFO_SENDING_TIME_OUT = 17,
    AIC_IPERROR_INFO_SENDING_FAIL_FOR_NETWORK_ERR = 18,
    AIC_IPERROR_INFO_OPEN_FAIL_FOR_NETWORK_ERR = 19,
    AIC_IPERROR_INFO_SRV_ALREADY_LISTENING = 20,
    AIC_IPERROR_INFO_OPER_FAILED = 21,
    AIC_IPERROR_INFO_NO_DATA = 22,
};
/* <mode> in +CIPRXGET */
enum {
    AT_CIPRXGET_MODE_AUTO = 0,
    AT_CIPRXGET_MODE_MANUAL,
    AT_CIPRXGET_MODE_READ_ASCII_DATA,
    AT_CIPRXGET_MODE_READ_HEX_DATA,
    AT_CIPRXGET_MODE_GET_REST_DATA_LEN,
    AT_CIPRXGET_MODE_ADD_IP_HEADER,

    AT_CIPRXGET_MODE_MAX,
};
/* <mode> in +CIPSRIP */
enum {
    AT_CIPSRIP_MODE_NOT_SHOW = 0,
    AT_CIPSRIP_MODE_SHOW,
};
/* <mode> in +CIPHEAD */
enum {
    AT_CIPHEAD_MODE_NOT_ADD_IP_HEAD = 0,
    AT_CIPHEAD_MODE_ADD_IP_HEAD,
};
/* <close_reason> in +IPCLOSE */
enum {
    AT_IPCLOSE_CLOSE_REASON_ACTIVE,
    AT_IPCLOSE_CLOSE_REASON_PASSIVE,
    AT_IPCLOSE_CLOSE_REASON_TIMEOUT,
};
/* <data format> */
enum {
    AT_CIPRXGET_DATA_FORMAT_ASCII = 0,
    AT_CIPRXGET_DATA_FORMAT_HEX,
};
/* <function character> */
enum {
    AIC_CTCPIP_CTRL_Z_HANDLE = 1,
    AIC_CTCPIP_ESC_HANDLE,
};

/*******************************************
 *              TYPEDEFS
 ******************************************/
typedef struct aic_ctcpip_at_cfg {
    u32 ciptimeout_netopen;
    u32 ciptimeout_cipopen;
    u32 ciptimeout_cipsend;
    u8 com_id;
    u8 recv_data_format;
    u8 ciprxget_mode;
    /* controlled by +CIPSRIP (RFU) */
    u8 cipsrip_mode;
    /* controlled by +CIPHEAD (RFU) */
    u8 ciphead_mode;
} AIC_CTCPIP_AT_CFG;

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
/* +NETOPEN */
static u32 handle_netopen_read_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_netopen_exe_cmd(AT_REQ_RESP *at_req_ptr);
/* +CIPOPEN */
static u32 handle_cipopen_test_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_cipopen_query_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_cipopen_exe_cmd(AT_REQ_RESP *at_req_ptr);
/* +CIPCLOSE */
static u32 handle_cipclose_test_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_cipclose_query_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_cipclose_exe_cmd(AT_REQ_RESP *at_req_ptr);
/* +IPADDR */
static u32 handle_ipaddr_test_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_ipaddr_exe_cmd(AT_REQ_RESP *at_req_ptr);
/* +CIPSEND */
static u32 handle_cipsend_test_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_cipsend_query_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_cipsend_exe_cmd(AT_REQ_RESP *at_req_ptr);
/* +CIPRXGET */
static u32 handle_ciprxget_test_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_ciprxget_query_cmd(AT_REQ_RESP *at_req_ptr);
static u32 handle_ciprxget_exe_cmd(AT_REQ_RESP *at_req_ptr);
/* socket releated functions */
static void aic_ctcpip_socket_listen_thread_handle(void *thread_input);
static void aic_ctcpip_socket_listen_init(void);
static void aic_ctcpip_socket_recv_cb(int sock, void *data);
static void aic_ctcpip_socket_tcp_disconnet_cb(int sock, void *data, int is_active);
static int aic_ctcpip_create_tcp_socket(int family, struct sockaddr *addr, int linkid, struct sockaddr *srcaddr,
                                        int isclient, s32 *err_code);
static int aic_ctcpip_create_udp_socket(int family, struct sockaddr *remoteaddr, int linkid, struct sockaddr *srcaddr,
                                        int isclient, s32 *err_code);
static int aic_ctcpip_udp_socket_send(int socket_id, char *data, int datalen);
static void aic_ctcpip_get_socket_state(int link_id, struct socket_state *state);
static int aic_ctcpip_find_index_by_socket(int socket);
/* other functions */
static void aic_ctcpip_transmit_iperror_urc(u32 serial_port_id, int err);
static void aic_ctcpip_transmit_cipopen_urc(u32 serial_port_id, int link, int err);
static void aic_ctcpip_transmit_cipclose_urc(u32 serial_port_id, int link, int err);
static void aic_ctcpip_transmit_ciperror_urc(u32 serial_port_id, int err);
static void aic_ctcpip_transmit_cipsend_urc(u32 serial_port_id, int link_id, int req_len, int sent_len);
static void aic_ctcpip_transmit_cipclose_resp(AT_REQ_RESP *at_req_ptr, int link, int err);
/* init functions */
static void aic_ctcpip_at_cfg_init(void);
static int aic_ctcpip_at_cmd_init(void);
/*******************************************
 *              STATIC VARIABLES
 ******************************************/
static AT_CMD_HANDLE s_ctcpip_at_cmds[] = {
    {"AT+NETOPEN", AIC_NULL, handle_netopen_read_cmd, AIC_NULL, handle_netopen_exe_cmd},
    {"AT+CIPOPEN", handle_cipopen_test_cmd, handle_cipopen_query_cmd, AIC_NULL, handle_cipopen_exe_cmd},
    {"AT+CIPCLOSE", handle_cipclose_test_cmd, handle_cipclose_query_cmd, AIC_NULL, handle_cipclose_exe_cmd},
    {"AT+IPADDR", handle_ipaddr_test_cmd, AIC_NULL, AIC_NULL, handle_ipaddr_exe_cmd},
    {"AT+CIPSEND", handle_cipsend_test_cmd, handle_cipsend_query_cmd, AIC_NULL, handle_cipsend_exe_cmd},
    {"AT+CIPRXGET", handle_ciprxget_test_cmd, handle_ciprxget_query_cmd, AIC_NULL, handle_ciprxget_exe_cmd},
    {AIC_NULL},
};

static AIC_CTCPIP_AT_CFG s_aic_ctcpip_cfg = {0};
static AIC_AT_TCP_SERVER s_at_tcp_server = {0};
static aos_thread_id s_at_socket_listen_thread = AOS_INVALID_THREAD_ID;
static aos_event_ptr s_at_socket_listen_event = AOS_NULL;
static AIC_SOCKET_LIST s_at_sock_entries[AIC_CTCPIP_SOCKET_MAX_LINKS] = {0};
static aos_mutex_ptr s_at_sock_entries_mutex = NULL;

/*******************************************
 *              EXTERNAL VARIABLES
 ******************************************/
extern aos_mutex_ptr pdp_info_mutex;

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
////////////////////////////////////////////////////////
//  AT related functions
////////////////////////////////////////////////////////
static u32 handle_netopen_read_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    s32 net_state = 0;
    boolean pdp_activated = AIC_FALSE;

    /* get state of default pdp context */
    aos_get_mutex(pdp_info_mutex, AOS_WAIT_FOREVER);
    pdp_activated = aic_is_default_pdp_activated();

    if (pdp_activated) {
        net_state = AT_NETOPEN_NET_STATE_OPEN;
    } else {
        net_state = AT_NETOPEN_NET_STATE_CLOSE;
    }
    aos_put_mutex(pdp_info_mutex);
    /* compose response */
    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+NETOPEN: %d", net_state);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

/* Execute Command: AT+NETOPEN */
static u32 handle_netopen_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong *paras_arr = AIC_NULL;
    u8 *tmp_ptr = AIC_NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    boolean pdp_activated = AIC_FALSE;
    u16 paras_count = 0;
    u8 cmd_str[20] = {'\0'};
    AT_RESP_FROM_CP resp_to_save = {0};

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req, "AT+NETOPEN", &paras_count);
    if (paras_arr) {
        /* There should be no additional parameters */
        goto HANDLE_ERROR;
    }
    /* check if default pdp context has been activated or not */
    aos_get_mutex(pdp_info_mutex, AOS_WAIT_FOREVER);
    pdp_activated = aic_is_default_pdp_activated();
    aos_put_mutex(pdp_info_mutex);
    if (pdp_activated) {
        aic_ctcpip_transmit_iperror_urc(at_req_ptr->serial_port_id, AIC_IPERROR_INFO_NETWORK_ALREADY_OPENED);
        aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
        return AIC_SUCCESS;
    }
    /* response OK first */
    aic_reply_ok_resp(at_req_ptr);
    /* in case pdp was not activated, try to send +CGACT to MODEM */
    resp_to_save.resp_buf = (u8 *)aos_zalloc(AT_MAX_AP_RESP_LENGTH);
    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    snprintf(cmd_str, sizeof(cmd_str), "AT+CGACT=1,%d", ANET_DEFAULT_PDP_CID);
    aic_send_cmd_and_wait(cmd_str, &resp_to_save);
    /* parse +CGACT command response */
    tmp_ptr = strstr(resp_to_save.resp_buf, AT_RESULT_OK);
    if (tmp_ptr) {
        /* success */
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+NETOPEN: %d", AIC_CTCPIP_ERR_SUCCESS);
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+NETOPEN: %d", AIC_CTCPIP_ERR_UNKNOWN);
    }
    aos_free(resp_to_save.resp_buf);
    /* URC +NETOPEN: <err> */
    aic_transmit_at_cmd_urc(at_req_ptr->serial_port_id, result_line, strlen(result_line));

    return AIC_SUCCESS;

HANDLE_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

static u32 handle_ipaddr_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

/* Execute Command: AT+IPADDR */
static u32 handle_ipaddr_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong *paras_arr = AIC_NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    boolean pdp_activated = AIC_FALSE;
    u16 paras_count = 0;
    NET_PDP_INFO *pdp_info = NULL;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req, "AT+IPADDR", &paras_count);
    if (paras_arr) {
        /* There should be no additional parameters */
        goto HANDLE_ERROR;
    }
    /* check if default pdp context has been activated or not */
    aos_get_mutex(pdp_info_mutex, AOS_WAIT_FOREVER);
    pdp_activated = aic_is_default_pdp_activated();
    if (!pdp_activated) {
        aos_put_mutex(pdp_info_mutex);
        aic_ctcpip_transmit_iperror_urc(at_req_ptr->serial_port_id, AIC_IPERROR_INFO_NETWORK_NOT_OPENED);
        aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
        return AIC_SUCCESS;
    }
    /* Note: aic_get_pdp_info_by_cid is only available to get IPv4 addr */
    /* prefer to call aic_get_wan_net_per_pdp in order to get IPv6 addr once needed in the future */
    pdp_info = aic_get_pdp_info_by_cid(ANET_DEFAULT_PDP_CID);
    if (pdp_info && pdp_info->pdp_ipv4_address[0]) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+IPADDR: %s", pdp_info->pdp_ipv4_address);
        aos_put_mutex(pdp_info_mutex);
        aic_transmit_resp_normal(at_req_ptr, result_line);
        aic_reply_ok_resp(at_req_ptr);
        return AIC_SUCCESS;
    }
    /* operation failed */
    aos_put_mutex(pdp_info_mutex);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
    return AIC_SUCCESS;

HANDLE_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

static u32 handle_cipopen_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+CIPOPEN: (0-%d),(\"TCP\",\"UDP\",\"UDP SERVER\")",
             AIC_CTCPIP_SOCKET_MAX_LINKS - 1);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

static u32 handle_cipopen_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    int i = 0;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct socket_state state;
    u8 *final_result = (u8 *)aos_zalloc(AT_MAX_AP_RESP_LENGTH * 5);

    if (!final_result) {
        aicx_error(0x0e4d91, "", "[+CIPOPEN]: no memory");
        aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
        return AIC_SUCCESS;
    }
    /* +CIPOPEN: <link_num>[,<type>,<serverIP>,<serverPort>,<index>] */
    aos_get_mutex(s_at_sock_entries_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < AIC_CTCPIP_SOCKET_MAX_LINKS; i++) {
        if (s_at_sock_entries[i].socket_id) {
            memset(&state, 0, sizeof(struct socket_state));
            aic_ctcpip_get_socket_state(i, &state);
            if (s_at_sock_entries[i].socket_type != SOCK_TCPS) {
                if (s_at_sock_entries[i].port != -1) {
                    snprintf(result_line, sizeof(result_line), "+CIPOPEN: %d,\"%s\",\"%s\",%d,-1", i, state.socket_type,
                             state.addr, state.remote_port);
                } else {
                    snprintf(result_line, sizeof(result_line), "+CIPOPEN: %d,\"%s\",,,-1", i, state.socket_type);
                }
            } else {
                /* need to add codes once +SERVERSTART is implemented */
            }
        } else {
            snprintf(result_line, sizeof(result_line), "+CIPOPEN: %d", i);
        }
        strcat(final_result, result_line);
        if (i != AIC_CTCPIP_SOCKET_MAX_LINKS - 1) {
            strcat(final_result, "\r\n");
        }
    }
    aic_transmit_resp_normal(at_req_ptr, final_result);
    aos_put_mutex(s_at_sock_entries_mutex);
    aos_free(final_result);
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

static boolean aic_judge_is_string_or_not(const char *at_req, u32 index)
{
    char *ptr = AIC_NULL;
    char *ptr_begin = AIC_NULL;
    char *ptr_end = AIC_NULL;
    u8 i, len, mark = 0;

    ptr = strstr(at_req, "=");

    if (ptr != NULL) {
        ptr_end = ptr;
        ptr_begin = ptr;
        len = strlen(ptr);

        for (i = 0; i < len; i++) {
            if (*ptr == ',') {
                mark++;
                if (mark == 1) {
                    ptr_end = ptr;
                } else {
                    ptr_begin = ptr_end;
                    ptr_end = ptr;
                }
                ptr++;
                if (mark == index + 1) {
                    if (*(ptr_begin + 1) == '"' && *(ptr_end - 1) == '"' && (ptr_begin + 1) != (ptr_end - 1)) {
                        return AIC_TRUE;
                    } else {
                        return AIC_FALSE;
                    }
                }
            } else {
                ptr++;
            }
        }
        if (mark == index && *ptr == '\0') {
            if (ptr_end - ptr_begin > 2 && *(ptr_begin + 1) == '"' && *(ptr_end - 1) == '"') {
                return AIC_TRUE;
            } else {
                return AIC_FALSE;
            }
        }
        return AIC_FALSE;
    } else {
        return AIC_FALSE;
    }
}

/* Themes:
 * 1. TCP connection:         AT+CIPOPEN=<link_num>,"TCP",<serverIP>,<serverPort>[,<localPort>]
 * 2. UDP Connection:         AT+CIPOPEN=<link_num>,"UDP",,,<localPort>
 * 3. UDP server Connection:  AT+CIPOPEN=<link_num>,"UDP SERVER",,,<localPort>
 */
static u32 handle_cipopen_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    int link_id = -1, socket_type = SOCK_TCP, sock = 0, ret, cid = 0, protocolFamily = AF_INET;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;
    struct addrinfo *server_ip = NULL;
    struct sockaddr_in remoteaddr, localaddr;
    struct sockaddr_in6 remoteaddr6, localaddr6;
    struct sockaddr *peeraddr = NULL, *srcaddr = NULL;
    WAN_NET *net_ptr = NULL;
    s32 cme_error = AT_ERROR_INVALID_PARAS, err_value = AIC_CTCPIP_ERR_WRONG_PARA;
    u8 string_ret = 0;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req, "AT+CIPOPEN=", &paras_count);
    if (!paras_arr) {
        aic_transmit_resp_error(at_req_ptr, AIC_TCPIP_ERR_INVAL_AT_CMD);
        return AIC_SUCCESS;
    }
    if (paras_count < 4 || paras_count > 5) {
        aicx_error(0x1572dc, "", "[+CIPOPEN]: Invalid parameter count");
        goto HANDLE_ERROR;
    }
    /* check 1st parameter */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        link_id = atoi((u8 *)paras_arr[0]);
        if ((unsigned)link_id >= AIC_CTCPIP_SOCKET_MAX_LINKS) {
            aic_ctcpip_transmit_iperror_urc(at_req_ptr->serial_port_id, AIC_IPERROR_INFO_INVALID_PARAM);
            aicx_error(0x15b178, "", "[+CIPOPEN]: Invalid parameter 1. invalid link id");
            goto HANDLE_ERROR;
        }
    } else {
        aicx_error(0x0661a6, "", "[+CIPOPEN]: Invalid parameter 1");
        goto HANDLE_ERROR;
    }
    /* check 2nd parameter */
    string_ret = aic_judge_is_string_or_not(at_req_ptr->at_req, 1);
    if (string_ret == AIC_TRUE) {
        if (strncasecmp((const char *)paras_arr[1], "TCP", sizeof("TCP")) == 0) {
            socket_type = SOCK_TCP;
        } else if (strncasecmp((const char *)paras_arr[1], "UDP SERVER", sizeof("UDP SERVER")) == 0) {
            socket_type = SOCK_UDPS;
        } else if (strncasecmp((const char *)paras_arr[1], "UDP", sizeof("UDP")) == 0) {
            socket_type = SOCK_UDP;
        } else {
            aicx_error(0x1bc070, "", "[+CIPOPEN]: Invalid parameter 2");
            goto HANDLE_ERROR;
        }
    } else {
        aicx_error(0x1bc070, "", "[+CIPOPEN]: Invalid parameter 2");
        goto HANDLE_ERROR;
    }
    /* check 3rd parameter */
    switch (socket_type) {
    case SOCK_TCP:
        string_ret = aic_judge_is_string_or_not(at_req_ptr->at_req, 2);
        if (string_ret == AIC_FALSE) {
            aicx_error(0x0e5a5b, "", "[+CIPOPEN]: Invalid parameter 3");
            goto HANDLE_ERROR;
        }
        ret = getaddrinfo((u8 *)paras_arr[2], AIC_NULL, AIC_NULL, &server_ip);
        if (ret != NX_SUCCESS) {
            aicx_error(0x046398, "", "[+CIPOPEN]: Invalid parameter 3. wrong ip address");
            goto HANDLE_ERROR;
        }
        protocolFamily = server_ip->ai_family;
        break;
    case SOCK_UDP:
    case SOCK_UDPS:
        if (strlen((const char *)paras_arr[2]) > 0) {
            aicx_error(0x0a8f96, "", "[+CIPOPEN] 3rd parameter is not empty in UDP case\n");
            goto HANDLE_ERROR;
        }
        break;
    default:
        break;
    }
    /* check 4th parameter */
    switch (socket_type) {
    case SOCK_TCP:
        if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3]))) {
            aicx_error(0x0ed20c, "", "[+CIPOPEN]: Invalid parameter 4");
            goto HANDLE_ERROR;
        }
        cid = atoi((u8 *)paras_arr[3]);
        if (cid < 0 || cid > 65535) {
            aicx_error(0x083708, "", "[+CIPOPEN]: Invalid port number");
            goto HANDLE_ERROR;
        }
        break;
    case SOCK_UDP:
    case SOCK_UDPS:
        if (strlen((const char *)paras_arr[3]) > 0) {
            aicx_error(0x037ab0, "", "[+CIPOPEN] 4th parameter is not empty in UDP case\n");
            goto HANDLE_ERROR;
        }
        break;
    default:
        break;
    }
    if (server_ip) {
        /* re-org input paramters for socket APIs */
        if (server_ip->ai_family == AF_INET) {
            aos_memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
            remoteaddr.sin_family = AF_INET;
            remoteaddr.sin_port = htons(cid);
            remoteaddr.sin_addr.s_addr = ((struct sockaddr_in *)server_ip->ai_addr)->sin_addr.s_addr;
            peeraddr = (struct sockaddr *)&remoteaddr;
        } else {
            aos_memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
            remoteaddr6.sin6_family = AF_INET6;
            remoteaddr6.sin6_port = htons(cid);
            aos_memcpy(&remoteaddr6.sin6_addr, &((struct sockaddr_in6 *)server_ip->ai_addr)->sin6_addr, 16);
            peeraddr = (struct sockaddr *)&remoteaddr6;
        }
    }
    /* check 5th parameter */
    if (paras_count < 5 && (socket_type == SOCK_UDP || socket_type == SOCK_UDPS)) {
        aicx_error(0x132dcb, "", "[+CIPOPEN]: Missing local port paramter.");
        goto HANDLE_ERROR;
    }
    if (paras_count == 5) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[4], strlen((u8 *)paras_arr[4]))) {
            cid = atoi((u8 *)paras_arr[4]);
        } else {
            aicx_error(0x01642a, "", "[+CIPOPEN]: Invalid parameter 5, which must be integer.");
            goto HANDLE_ERROR;
        }
        if (cid < 0 || cid > 65535) {
            aicx_error(0x0e8096, "", "[+CIPOPEN]: Invalid parameter 5. wrong local port number");
            goto HANDLE_ERROR;
        }
        net_ptr = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
        if (net_ptr) {
            if (protocolFamily == AF_INET) {
                localaddr.sin_family = AF_INET;
                localaddr.sin_port = htons(cid);
#ifdef CONFIG_CTCPIP_LOCAL_TEST_SERVER
                if (socket_type == SOCK_UDPS) {
                    localaddr.sin_addr.s_addr = htonl(0xC0A80001);
                } else {
                    localaddr.sin_addr.s_addr = htonl(net_ptr->wan_ipv4_address);
                }
#else
                localaddr.sin_addr.s_addr = htonl(net_ptr->wan_ipv4_address);
#endif
                srcaddr = (struct sockaddr *)&localaddr;
            } else {
                localaddr6.sin6_family = AF_INET6;
                localaddr6.sin6_port = htons(cid);
                aos_memcpy(&localaddr6.sin6_addr, net_ptr->wan_ipv6_address1, 16);
                NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[0]);
                NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[1]);
                NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[2]);
                NX_CHANGE_ULONG_ENDIAN(localaddr6.sin6_addr._S6_un._S6_u32[3]);
                srcaddr = (struct sockaddr *)&localaddr6;
            }
        } else {
            aicx_error(0x14ba8b, "", "[+CIPOPEN]: Failed to get ip address of wan.");
            aos_memset(&s_at_sock_entries[link_id], 0, sizeof(struct SOCKET_LIST));
            cme_error = AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
            err_value = AIC_CTCPIP_ERR_NET_NOT_OPENED;
            goto HANDLE_ERROR;
        }
    }
    /* check link is in use or not */
    if (s_at_sock_entries[link_id].socket_id || s_at_sock_entries[link_id].in_use) {
        err_value = AIC_CTCPIP_ERR_SOCKETS_OPENED;
        cme_error = AIC_TCPIP_ERR_SOCKET_ALREADY_CONNECT;
        aicx_error(0x1aa5bd, "", "[+CIPOPEN]: The specified socket link is already opened.");
        goto HANDLE_ERROR;
    }
    switch (socket_type) {
    case SOCK_TCP:
        sock = aic_ctcpip_create_tcp_socket(protocolFamily, peeraddr, link_id, srcaddr, NX_TRUE, &err_value);
        break;
    case SOCK_UDP:
        sock = aic_ctcpip_create_udp_socket(protocolFamily, peeraddr, link_id, srcaddr, NX_TRUE, &err_value);
        break;
    case SOCK_UDPS:
        sock = aic_ctcpip_create_udp_socket(protocolFamily, peeraddr, link_id, srcaddr, NX_FALSE, &err_value);
        break;
    default:
        break;
    }
    if (sock > 0) {
        /* response OK first */
        s_aic_ctcpip_cfg.com_id = at_req_ptr->serial_port_id;
        aic_reply_ok_resp(at_req_ptr);
        aic_ctcpip_transmit_cipopen_urc(at_req_ptr->serial_port_id, link_id, AIC_CTCPIP_ERR_SUCCESS);
    } else {
        aicx_error(0x0d769c, "", "[+CIPOPEN]: Failed to open socket.");
        cme_error = AIC_TCPIP_ERR_SOCKET_CREATE_FAIL;
        err_value = AIC_CTCPIP_ERR_FAIL_TO_CREATE_SOCKET;
        goto HANDLE_ERROR;
    }
    if (server_ip)
        freeaddrinfo(server_ip);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;

HANDLE_ERROR:
    if (server_ip)
        freeaddrinfo(server_ip);
    aic_free_paras_mem(paras_arr, paras_count);
    if ((unsigned)link_id < AIC_CTCPIP_SOCKET_MAX_LINKS) {
        aic_ctcpip_transmit_cipopen_urc(at_req_ptr->serial_port_id, link_id, err_value);
    }
    aic_reply_error_resp(at_req_ptr, cme_error);

    return AIC_SUCCESS;
}

static u32 handle_cipclose_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+CIPCLOSE: (0-%d)", AIC_CTCPIP_SOCKET_MAX_LINKS - 1);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

static u32 handle_cipclose_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    int i;
    size_t j;

    strcpy(result_line, "+CIPCLOSE: ");
    j = strlen(result_line);
    aos_get_mutex(s_at_sock_entries_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < AIC_CTCPIP_SOCKET_MAX_LINKS; i++) {
        if (i != 0) {
            result_line[j++] = ',';
        }
        if (s_at_sock_entries[i].socket_id) {
            result_line[j++] = '1';
        } else {
            result_line[j++] = '0';
        }
    }
    aos_put_mutex(s_at_sock_entries_mutex);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);
    return AIC_SUCCESS;
}

/* Write Command: AT+CIPCLOSE=<link_num> */
static u32 handle_cipclose_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    int link_id = -1;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0;
    s32 cme_error = AT_ERROR_INVALID_PARAS, err_value = AIC_CTCPIP_ERR_WRONG_PARA;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req, "AT+CIPCLOSE=", &paras_count);
    if (!paras_arr) {
        aicx_error(0x0f6ea2, "", "[+CIPCLOSE]: Invalid parameters");
        aic_transmit_resp_error(at_req_ptr, AIC_TCPIP_ERR_INVAL_AT_CMD);
        return AIC_SUCCESS;
    }
    if (paras_count != 1) {
        aicx_error(0x178a09, "", "[+CIPCLOSE]: Invalid parameter count");
        goto HANDLE_ERROR;
    }
    /* check 1st parameter */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        link_id = atoi((u8 *)paras_arr[0]);
        if ((unsigned)link_id >= AIC_CTCPIP_SOCKET_MAX_LINKS) {
            aicx_error(0x07c5ca, "", "[+CIPCLOSE]: Invalid parameter 1. invalid link id");
            goto HANDLE_ERROR;
        }
    } else {
        aicx_error(0x1130eb, "", "[+CIPCLOSE]: Invalid parameter 1");
        goto HANDLE_ERROR;
    }
    aos_get_mutex(s_at_sock_entries_mutex, AOS_WAIT_FOREVER);
    if (s_at_sock_entries[link_id].socket_id) {
        if (s_at_sock_entries[link_id].socket_type == SOCK_TCP) {
            aic_socket_unregister_tcp_disconnect_notify(s_at_sock_entries[link_id].socket_id);
        }
        aic_socket_unregister_receive_notify(s_at_sock_entries[link_id].socket_id);
        soc_close(s_at_sock_entries[link_id].socket_id);
        if (s_at_sock_entries[link_id].socket_type == SOCK_TCP) {
            /* TCP case */
            aic_reply_ok_resp(at_req_ptr);
            aic_ctcpip_transmit_cipclose_urc(at_req_ptr->serial_port_id, link_id, AIC_CTCPIP_ERR_SUCCESS);
        } else {
            /* UDP case */
            aic_ctcpip_transmit_cipclose_resp(at_req_ptr, link_id, AIC_CTCPIP_ERR_SUCCESS);
            aic_reply_ok_resp(at_req_ptr);
        }
        aos_memset(&s_at_sock_entries[link_id], 0, sizeof(struct SOCKET_LIST));
    } else {
        aos_put_mutex(s_at_sock_entries_mutex);
        err_value = AIC_CTCPIP_ERR_OPER_NOT_SUPPORT;
        aicx_error(0x1c7cd4, "", "[+CIPCLOSE]: specific socket is not opened.");
        goto HANDLE_ERROR;
    }
    aos_put_mutex(s_at_sock_entries_mutex);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;

HANDLE_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    if ((unsigned)link_id < AIC_CTCPIP_SOCKET_MAX_LINKS) {
        aic_ctcpip_transmit_cipclose_resp(at_req_ptr, link_id, err_value);
    }
    aic_reply_error_resp(at_req_ptr, cme_error);

    return AIC_SUCCESS;
}

static u32 handle_cipsend_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+CIPSEND: (0-%d),(1-%d)", AIC_CTCPIP_SOCKET_MAX_LINKS - 1,
             AIC_CTCPIP_SOCKET_MAX_SEND_DATA_LEN);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

static u32 handle_cipsend_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    aic_reply_ok_resp(at_req_ptr);

    return AIC_SUCCESS;
}

static u32 aic_ctcpip_handle_ext_characters(const u8 *src, u32 src_len, u8 *dest, u32 dest_len, u8 *handle)
{
    u32 i, j;

    for (i = 0, j = 0; i < src_len; i++) {
        if (src[i] == AIC_CTCPIP_DATA_EXT_CHAR) {
            if (i + 1 != src_len) {
                switch (src[i + 1]) {
                case AIC_CTCPIP_DATA_EXT_CHAR:
                case AIC_CTCPIP_DATA_CTRL_Z_CHAR:
                case AIC_CTCPIP_DATA_ESC_CHAR:
                    if (j < dest_len) {
                        dest[j] = src[i + 1];
                        j++;
                    }
                    i++;
                    break;
                default:
                    if (j < dest_len) {
                        dest[j] = src[i];
                        j++;
                    }
                    break;
                }
            } else {
                /* last character */
                if (j < dest_len) {
                    dest[j] = src[i + 1];
                    j++;
                }
            }
        } else if (src[i] == AIC_CTCPIP_DATA_CTRL_Z_CHAR) {
            *handle = AIC_CTCPIP_CTRL_Z_HANDLE;
            break;
        } else if (src[i] == AIC_CTCPIP_DATA_ESC_CHAR) {
            *handle = AIC_CTCPIP_ESC_HANDLE;
            break;
        } else {
            if (j < dest_len) {
                dest[j] = src[i];
                j++;
            }
        }
    }

    return j;
}
/* Themes:
 * 1. TCP with changeable length:   AT+CIPSEND=<link_num>
 * 2. TCP with fixed length:        AT+CIPSEND=<link_num>,<length>
 * 3. UDP with changeable length:   AT+CIPSEND=<link_num>,,<serverIP>,<serverPort>
 * 4. UDP with fixed length:        AT+CIPSEND=<link_num>,<length>,<serverIP>,<serverPort>
 */
static u32 handle_cipsend_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    int link_id = -1, ret;
    ulong *paras_arr = AIC_NULL;
    u16 paras_count = 0, port = 0;
    s32 cme_error = AT_ERROR_INVALID_PARAS, err_value = AIC_CTCPIP_ERR_WRONG_PARA;
    /* NOTE: we can only handle single socket session at the same time */
    static AIC_SOCKET_INFO ongoing_socket_info = {0};
    struct addrinfo *server_ip = NULL;

    if (at_req_ptr->at_mode == AT_CMD_MODE) { /* command mode */
        paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req, "AT+CIPSEND=", &paras_count);
        if (!paras_arr) {
            aic_transmit_resp_error(at_req_ptr, AIC_TCPIP_ERR_INVAL_AT_CMD);
            aicx_error(0x09d763, "", "[+CIPSEND]: Invalid parameters");
            return AIC_SUCCESS;
        }
        if (paras_count < 1 || paras_count == 3 || paras_count > 4) {
            aicx_error(0x01e32d, "", "[+CIPSEND]: Invalid parameter count");
            goto HANDLE_ERROR;
        }
        /* check 1st parameter */
        if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
            link_id = atoi((u8 *)paras_arr[0]);
            if ((unsigned)link_id >= AIC_CTCPIP_SOCKET_MAX_LINKS) {
                aicx_error(0x11da22, "", "[+CIPSEND]: Invalid parameter 1. invalid link id");
                goto HANDLE_ERROR;
            }
        } else {
            aicx_error(0x0b48f7, "", "[+CIPSEND]: Invalid parameter 1");
            goto HANDLE_ERROR;
        }
        /* check if the socket was set up or not */
        if (s_at_sock_entries[link_id].socket_id == 0) {
            err_value = AIC_CTCPIP_ERR_OPER_NOT_SUPPORT;
            cme_error = AT_ERROR_HANDLE_FAILURE;
            aicx_error(0x0de818, "", "[+CIPSEND]: specific socket is not opened");
            goto HANDLE_ERROR;
        }
        ongoing_socket_info.sockid = s_at_sock_entries[link_id].socket_id;
        ongoing_socket_info.length = -1;
        if (paras_count > 1) {
            if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
                int length = atoi((u8 *)paras_arr[1]);
                if (length <= 0 || length > AIC_CTCPIP_SOCKET_MAX_SEND_DATA_LEN) {
                    aicx_error(0x0ddb69, "", "[+CIPSEND]: Invalid parameter 2. length is not valid");
                    goto HANDLE_ERROR;
                } else {
                    ongoing_socket_info.length = length;
                }
            } else if (strlen((u8 *)paras_arr[1]) != 0) {
                aicx_error(0x1d06cd, "", "[+CIPSEND]: Invalid parameter 2 for UDP cases");
                goto HANDLE_ERROR;
            }
        }
        /* check the socket type */
        switch (s_at_sock_entries[link_id].socket_type) {
        case SOCK_TCP:
            if (paras_count > 2) {
                aicx_error(0x0ce250, "", "[+CIPSEND]: Invalid parameter count for TCP case");
                goto HANDLE_ERROR;
            }
            break;
        case SOCK_UDP:
        case SOCK_UDPS:
            if (paras_count != 4) {
                aicx_error(0x01a2ed, "", "[+CIPSEND]: Invalid parameter count for UDP cases");
                goto HANDLE_ERROR;
            }
            /* move on to check server IP */
            ret = getaddrinfo((u8 *)paras_arr[2], AIC_NULL, AIC_NULL, &server_ip);
            if (ret != NX_SUCCESS || !server_ip) {
                aicx_error(0x1116b4, "", "[+CIPSEND]: Invalid parameter 3. wrong ip address");
                goto HANDLE_ERROR;
            }
            /* server port check */
            if (aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3]))) {
                port = htons(atoi((u8 *)paras_arr[3]));
            } else {
                aicx_error(0x18ac59, "", "[+CIPSEND]: Invalid parameter 4.");
                goto HANDLE_ERROR;
            }
            if (server_ip->ai_family == AF_INET) {
                s_at_sock_entries[link_id].addr = ((struct sockaddr_in *)server_ip->ai_addr)->sin_addr.s_addr;
            } else {
                aos_memcpy(s_at_sock_entries[link_id].addr6, &((struct sockaddr_in6 *)server_ip->ai_addr)->sin6_addr, 16);
            }
            s_at_sock_entries[link_id].family = server_ip->ai_family;
            s_at_sock_entries[link_id].port = port;
            break;
        default:
            aicx_error(0x003c23, "", "[+CIPSEND]: not supported socket mode");
            goto HANDLE_ERROR;
        }
        /* parameter check finished, send > and move to data mode */
        at_req_ptr->at_mode = AT_DATA_MODE;
        aic_set_data_mode_expire_time("AT+CIPSEND", s_aic_ctcpip_cfg.ciptimeout_cipsend * 1000);
        aic_transmit_resp_normal(at_req_ptr, ">");
        /* malloc data to socket buffer */
        if (ongoing_socket_info.buf) {
            aos_free(ongoing_socket_info.buf);
            ongoing_socket_info.buf = NULL;
        }
        ongoing_socket_info.buf = aos_zalloc(AIC_CTCPIP_SOCKET_MAX_SEND_DATA_LEN);
        ongoing_socket_info.offset = 0;
        if (!ongoing_socket_info.buf) {
            aicx_error(0x09e62d, "", "[+CIPSEND]: No memory to alloc socket buffer");
            err_value = AIC_CTCPIP_ERR_OPER_NOT_SUPPORT;
            cme_error = AT_ERROR_HANDLE_FAILURE;
            goto HANDLE_ERROR;
        }
        if (server_ip)
            freeaddrinfo(server_ip);
        aic_free_paras_mem(paras_arr, paras_count);
        return AIC_SUCCESS;
    } else if (at_req_ptr->at_mode == AT_DATA_MODE) { /* data mode */
        u32 len = 0;
        int index = 0, ret = -1;

        if (at_req_ptr->end_data_mode) { /* timeout */
            if (ongoing_socket_info.buf) {
                aos_free(ongoing_socket_info.buf);
                ongoing_socket_info.buf = NULL;
            }
            aos_memset(&ongoing_socket_info, 0, sizeof(struct at_sock_info));
            aicx_error(0x1d0e17, "", "[+CIPSEND]: data mode timeout.");
            at_req_ptr->at_mode = AT_CMD_MODE;
            return AIC_SUCCESS;
        }
        if (at_req_ptr->at_upstream_data == AIC_NULL) {
            aicx_error(0x1909a6, "", "[+CIPSEND]: ERROR - no upstream data");
            /* DO NOT REPLY ANY AT RESP HERE */
            return AIC_SUCCESS;
        }
        if (ongoing_socket_info.length == -1) {
            u8 character_handle = 0;
            len = aic_ctcpip_handle_ext_characters(at_req_ptr->at_upstream_data, at_req_ptr->upstream_data_len,
                                                   ongoing_socket_info.buf + ongoing_socket_info.offset,
                                                   AIC_CTCPIP_SOCKET_MAX_SEND_DATA_LEN - ongoing_socket_info.offset,
                                                   &character_handle);
            ongoing_socket_info.offset += len;
            if (character_handle == AIC_CTCPIP_CTRL_Z_HANDLE) {
                len = ongoing_socket_info.offset;
                goto SEND;
            } else if (character_handle == AIC_CTCPIP_ESC_HANDLE) {
                at_req_ptr->at_mode = AT_CMD_MODE;
                if (ongoing_socket_info.buf) {
                    aos_free(ongoing_socket_info.buf);
                    ongoing_socket_info.buf = NULL;
                }
                aos_memset(&ongoing_socket_info, 0, sizeof(struct at_sock_info));
                aic_transmit_resp_normal(at_req_ptr, "ERROR");
                aicx_error(0x15c56a, "", "[+CIPSEND]: ESC recevied.");
                return AIC_SUCCESS;
            } else {
                if (ongoing_socket_info.offset < AIC_CTCPIP_SOCKET_MAX_SEND_DATA_LEN) {
                    len = ongoing_socket_info.offset;
                    return AIC_SUCCESS;
                } else {
                    len = AIC_CTCPIP_SOCKET_MAX_SEND_DATA_LEN;
                    goto SEND;
                }
            }
        } else {
            len = at_req_ptr->upstream_data_len;
            if (len + ongoing_socket_info.offset < ongoing_socket_info.length) {
                aicx_error(0x1a0991, "ddd", "[+CIPSEND]: need more data, in(%d), offset(%d), expected len(%d)", len,
                           ongoing_socket_info.offset, ongoing_socket_info.length);
                aos_memcpy(ongoing_socket_info.buf + ongoing_socket_info.offset, at_req_ptr->at_upstream_data, len);
                ongoing_socket_info.offset += len;
                return AIC_SUCCESS;
            } else if (len + ongoing_socket_info.offset > ongoing_socket_info.length) {
                aos_memcpy(ongoing_socket_info.buf + ongoing_socket_info.offset, at_req_ptr->at_upstream_data,
                            ongoing_socket_info.length - ongoing_socket_info.offset);
                ongoing_socket_info.offset = ongoing_socket_info.length;
                len = ongoing_socket_info.length;
                goto SEND;
            } else {
                aos_memcpy(ongoing_socket_info.buf + ongoing_socket_info.offset, at_req_ptr->at_upstream_data, len);
                ongoing_socket_info.offset += len;
                len = ongoing_socket_info.length;
                goto SEND;
            }
        }
    SEND:
        index = aic_ctcpip_find_index_by_socket(ongoing_socket_info.sockid);
        if (s_at_sock_entries[index].socket_type == SOCK_TCP) {
            ret = send(ongoing_socket_info.sockid, ongoing_socket_info.buf, len, 0);
        } else if (s_at_sock_entries[index].socket_type == SOCK_UDP || s_at_sock_entries[index].socket_type == SOCK_UDPS) {
            ret = aic_ctcpip_udp_socket_send(ongoing_socket_info.sockid, ongoing_socket_info.buf, len);
        } else {
            aicx_error(0x00e50d, "dd", "[+CIPSEND]: invalid socket type to send data, index is %d, socket_type is %d", index,
                       s_at_sock_entries[index].socket_type);
        }
        at_req_ptr->at_mode = AT_CMD_MODE;
        if (ongoing_socket_info.buf) {
            aos_free(ongoing_socket_info.buf);
            ongoing_socket_info.buf = NULL;
        }
        aos_memset(&ongoing_socket_info, 0, sizeof(struct at_sock_info));
        if (ret < 0) {
            aicx_error(0x1137d1, "", "[+CIPSEND]: socket send failed.");
            aic_reply_error_resp(at_req_ptr, AT_ERROR_HANDLE_FAILURE);
        } else {
            aic_reply_ok_resp(at_req_ptr);
            aic_ctcpip_transmit_cipsend_urc(at_req_ptr->serial_port_id, index, len, len);
        }
        return AIC_SUCCESS;
    }
HANDLE_ERROR:
    if (server_ip)
        freeaddrinfo(server_ip);
    memset(&ongoing_socket_info, 0, sizeof(ongoing_socket_info));
    aic_free_paras_mem(paras_arr, paras_count);
    aic_ctcpip_transmit_ciperror_urc(at_req_ptr->serial_port_id, err_value);
    aic_reply_error_resp(at_req_ptr, cme_error);

    return AIC_SUCCESS;
}

static u32 handle_ciprxget_test_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+CIPRXGET: (0-%d),(0-%d),(1-%d)", AT_CIPRXGET_MODE_MAX - 1,
             AIC_CTCPIP_SOCKET_MAX_LINKS - 1, AIC_CTCPIP_SOCKET_MAX_SEND_DATA_LEN);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

static u32 handle_ciprxget_query_cmd(AT_REQ_RESP *at_req_ptr)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+CIPRXGET: %d", s_aic_ctcpip_cfg.ciprxget_mode);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;
    aic_fill_transmit_cmd_resp(at_req_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

/* Themes:
 * 1. AT+CIPRXGET=<mode> In this case, <mode> can only be 0 or 1
 * 2. AT+CIPRXGET=2,<link_num>[,<len>] Retrieve data in ACSII form
 * 3. AT+CIPRXGET=3,<link_num>[,<len>] Retrieve data in hex form
 * 4. AT+CIPRXGET=4,<link_num> get the rest data length
 * 5. AT+CIPRXGET=5 add IP header reporting based on mode=1, the format is +CIPRXGETIP:<ip>:<port>
 */
static u32 handle_ciprxget_exe_cmd(AT_REQ_RESP *at_req_ptr)
{
    int link_id = -1, addrlen, sock = 0, length = AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN;
    int mode, fromaddrlen, total_recv = 0;
    ulong *paras_arr = AIC_NULL;
    char *data = NULL;
    char addr[ADDR_BUFFERSIZE_IPV6] = {0};
    u16 port, paras_count = 0;
    struct sockaddr_in remoteaddr;
    struct sockaddr_in6 remoteaddr6;
    s32 cme_error = AT_ERROR_INVALID_PARAS, err_value = AIC_IPERROR_INFO_INVALID_PARAM;
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};
    int rest_len, rest_len_t;
    int ret = -1;

    paras_arr = aic_split_paras_to_str_array(at_req_ptr->at_req, "AT+CIPRXGET=", &paras_count);
    if (!paras_arr) {
        aic_transmit_resp_error(at_req_ptr, AIC_TCPIP_ERR_INVAL_AT_CMD);
        return AIC_SUCCESS;
    }
    if (paras_count < 1 || paras_count > 3) {
        aicx_error(0x0c10aa, "", "[+CIPRXGET]: invalid parameters\n");
        goto HANDLE_ERROR;
    }
    /* set <mode> or add IP header */
    if (aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))) {
        mode = atoi((u8 *)paras_arr[0]);
        if (paras_count == 1) {
            if (mode == AT_CIPRXGET_MODE_AUTO || mode == AT_CIPRXGET_MODE_MANUAL ||
                mode == AT_CIPRXGET_MODE_ADD_IP_HEADER) {
                s_aic_ctcpip_cfg.ciprxget_mode = mode;
                aic_free_paras_mem(paras_arr, paras_count);
                aic_reply_ok_resp(at_req_ptr);
                return AIC_SUCCESS;
            } else {
                aicx_error(0x13f55e, "", "[+CIPRXGET]: invalid parameter 1\n");
                goto HANDLE_ERROR;
            }
        } else {
            if (mode == AT_CIPRXGET_MODE_AUTO || mode == AT_CIPRXGET_MODE_MANUAL ||
                mode == AT_CIPRXGET_MODE_ADD_IP_HEADER) {
                goto HANDLE_ERROR;
            }
        }
    } else {
        goto HANDLE_ERROR;
    }
    /* check link id validity */
    if (aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]))) {
        link_id = atoi((u8 *)paras_arr[1]);
        if (link_id >= AIC_CTCPIP_SOCKET_MAX_LINKS || link_id < 0) {
            aicx_error(0x12ebdd, "", "[+CIPRXGET]: invalid parameter 2. wrong link id\n");
            goto HANDLE_ERROR;
        }
    } else {
        goto HANDLE_ERROR;
    }
    /* mode must be manual */
    if (s_aic_ctcpip_cfg.ciprxget_mode != AT_CIPRXGET_MODE_MANUAL &&
        s_aic_ctcpip_cfg.ciprxget_mode != AT_CIPRXGET_MODE_ADD_IP_HEADER) {
        err_value = AIC_IPERROR_INFO_OPER_NOT_SUPPORTED;
        cme_error = AT_ERROR_HANDLE_FAILURE;
        aicx_error(0x1bc32a, "", "[+CIPRXGET]: invalid parameter. mode is not expected one.\n");
        goto HANDLE_ERROR;
    }
    sock = s_at_sock_entries[link_id].socket_id;
    if (!sock) {
        err_value = AIC_IPERROR_INFO_CONN_NOT_CREATED;
        cme_error = AT_ERROR_HANDLE_FAILURE;
        aicx_error(0x05f4ad, "", "[+CIPRXGET]: socket was not created yet\n");
        goto HANDLE_ERROR;
    }
    /* in case mode is AT_CIPRXGET_MODE_GET_REST_DATA_LEN */
    if (mode == AT_CIPRXGET_MODE_GET_REST_DATA_LEN) {
        if (paras_count != 2) {
            aicx_error(0x1566db, "", "[+CIPRXGET]: invalid parameter count\n");
            goto HANDLE_ERROR;
        }
        if (NX_SOC_OK == getsockopt(sock, SOL_SOCKET, SO_RXDATA, &rest_len, &rest_len_t)) {
            /* +CIPRXGET: 4,<link_num>,<rest_len> */
            snprintf(result_str, AT_RESP_RESULT_MAX, "+CIPRXGET: 4,%d,%d", link_id, rest_len);
            aic_transmit_resp_normal(at_req_ptr, result_str);
            aic_reply_ok_resp(at_req_ptr);
            aic_free_paras_mem(paras_arr, paras_count);
            return AIC_SUCCESS;
        } else {
            aicx_error(0x002a9d, "", "[+CIPRXGET] Failed to get socket remained data.");
            err_value = AIC_IPERROR_INFO_TCP_BUSY;
            cme_error = AT_ERROR_HANDLE_FAILURE;
            goto HANDLE_ERROR;
        }
    }
    if (paras_count == 3) {
        if (aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2]))) {
            length = atoi((u8 *)paras_arr[2]);
            if (length <= 0 ||
                (mode == AT_CIPRXGET_MODE_READ_ASCII_DATA && length > AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN) ||
                (mode == AT_CIPRXGET_MODE_READ_HEX_DATA && length > AIC_CTCPIP_SOCKET_MAX_READ_HEX_DATA_LEN)) {
                aicx_error(0x16adcd, "", "[+CIPRXGET]: invalid parameter 3. wrong length\n");
                goto HANDLE_ERROR;
            }
        } else {
            aicx_error(0x03ac00, "", "[+CIPRXGET]: invalid parameter 3. not a decimal value.\n");
            goto HANDLE_ERROR;
        }
    } else {
        if (mode == AT_CIPRXGET_MODE_READ_HEX_DATA) {
            length = AIC_CTCPIP_SOCKET_MAX_READ_HEX_DATA_LEN;
        }
    }
    data = aos_zalloc(length + 1);
    if (!data) {
        aicx_error(0x05065f, "", "[+CIPRXGET]: no memory");
        aic_free_paras_mem(paras_arr, paras_count);
        aic_reply_error_resp(at_req_ptr, AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
        return AIC_SUCCESS;
    }
    if (s_at_sock_entries[link_id].socket_type == SOCK_TCP || s_at_sock_entries[link_id].socket_type == SOCK_TCPIN) {
        while (1) {
            ret = recv(sock, data + total_recv, length - total_recv, MSG_DONTWAIT);
            if (ret < 0) {
                break;
            } else {
                total_recv += ret;
                if (total_recv == length)
                    break;
            }
        }
    } else if (s_at_sock_entries[link_id].family == AF_INET && (s_at_sock_entries[link_id].socket_type == SOCK_UDP ||
                                                                s_at_sock_entries[link_id].socket_type == SOCK_UDPS)) {
        fromaddrlen = sizeof(remoteaddr);
        ret = recvfrom(sock, data, length, MSG_DONTWAIT, (struct sockaddr *)&remoteaddr, &fromaddrlen);
        if (ret > length) {
            ret = length;
            aicx_error(0x052cf4, "", "[+CIPRXGET]: ATTENTION - UDP cannot be fragmented. <request len> < <actual len>");
        }
    } else if (s_at_sock_entries[link_id].family == AF_INET6 && (s_at_sock_entries[link_id].socket_type == SOCK_UDP ||
                                                                 s_at_sock_entries[link_id].socket_type == SOCK_UDPS)) {
        fromaddrlen = sizeof(remoteaddr6);
        ret = recvfrom(sock, data, length, MSG_DONTWAIT, (struct sockaddr *)&remoteaddr6, &fromaddrlen);
        if (ret > length) {
            ret = length;
            aicx_error(0x052cf4, "", "[+CIPRXGET]: ATTENTION - UDP cannot be fragmented. <request len> < <actual len>");
        }
    } else {
        aicx_error(0x106e37, "", "[+CIPRXGET]: The socket is not tcp or udp!\n");
    }
    if (ret <= 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            if (total_recv > 0) {
                ret = total_recv;
                goto URC_DATA;
            }
        }
        err_value = AIC_IPERROR_INFO_NO_DATA;
        cme_error = AIC_TCPIP_ERR_SOCKET_RECV_FAIL;
        aicx_error(0x084ae5, "", "[+CIPRXGET]: receive data failed.\n");
        goto HANDLE_ERROR;
    } else {
        if (ret < total_recv)
            ret = total_recv;
    }

URC_DATA:
    if (s_aic_ctcpip_cfg.ciprxget_mode == AT_CIPRXGET_MODE_ADD_IP_HEADER) {
        if (s_at_sock_entries[link_id].family == AF_INET) {
            if (s_at_sock_entries[link_id].socket_type != SOCK_UDP &&
                s_at_sock_entries[link_id].socket_type != SOCK_UDPS) {
                addrlen = sizeof(struct sockaddr_in);
                getpeername(sock, (struct sockaddr *)&remoteaddr, &addrlen);
            }
            inet_ntop(AF_INET, &remoteaddr.sin_addr.s_addr, addr, ADDR_BUFFERSIZE_IPV4);
            port = htons(remoteaddr.sin_port);
        } else {
            if (s_at_sock_entries[link_id].socket_type != SOCK_UDP &&
                s_at_sock_entries[link_id].socket_type != SOCK_UDPS) {
                addrlen = sizeof(struct sockaddr_in6);
                getpeername(sock, (struct sockaddr *)&remoteaddr6, &addrlen);
            }
            inet_ntop(AF_INET6, &remoteaddr6.sin6_addr, addr, ADDR_BUFFERSIZE_IPV6);
            port = htons(remoteaddr6.sin6_port);
        }
        snprintf(result_str, sizeof(result_str), "+CIPRXGETIP:\"%s\":%d", addr, port);
        aic_transmit_at_data(at_req_ptr->serial_port_id, result_str, strlen(result_str));
    }
    /* compose response header: +CIPRXGET: <mode>,<link_num>,<read_len>,<rest_len> */
    if (NX_SOC_OK != getsockopt(sock, SOL_SOCKET, SO_RXDATA, &rest_len, &rest_len_t)) {
        rest_len = 0;
    }
    snprintf(result_str, sizeof(result_str), "+CIPRXGET: %d,%d,%d,%d", mode, link_id, ret, rest_len);
    s_at_sock_entries[link_id].recv_total = rest_len;
    /* encode data as specified format */
    if (mode == AT_CIPRXGET_MODE_READ_HEX_DATA) {
        char *hex_data = aos_zalloc(ret * 2 + 1);
        const char *hex_tbl = "0123456789ABCDEF";
        int i, j;

        if (!hex_data) {
            aicx_error(0x1abc00, "", "[+CIPRXGET]: no memory #2");
            if (data)
                aos_free(data);
            aic_free_paras_mem(paras_arr, paras_count);
            aic_reply_error_resp(at_req_ptr, AIC_TCPIP_ERR_SOCKET_RECV_FAIL);
            return AIC_SUCCESS;
        }
        for (i = 0, j = 0; i < ret; i++, j += 2) {
            hex_data[j] = hex_tbl[(data[i] & 0xF0) >> 4];
            hex_data[j + 1] = hex_tbl[data[i] & 0x0F];
        }
        aic_transmit_at_data(at_req_ptr->serial_port_id, result_str, strlen(result_str));
        aic_transmit_resp_normal(at_req_ptr, hex_data);
        aos_free(hex_data);
    } else {
        aic_transmit_at_data(at_req_ptr->serial_port_id, result_str, strlen(result_str));
        aic_transmit_resp_normal(at_req_ptr, data);
    }
    aic_reply_ok_resp(at_req_ptr);
    if (data)
        aos_free(data);
    aic_free_paras_mem(paras_arr, paras_count);
    return AIC_SUCCESS;

HANDLE_ERROR:
    if (data)
        aos_free(data);
    aic_free_paras_mem(paras_arr, paras_count);
    aic_ctcpip_transmit_iperror_urc(at_req_ptr->serial_port_id, err_value);
    aic_reply_error_resp(at_req_ptr, cme_error);
    return AIC_SUCCESS;
}

////////////////////////////////////////////////////////
//  Socket related functions
////////////////////////////////////////////////////////
static void aic_ctcpip_get_socket_state(int link_id, struct socket_state *state)
{
    int i, sock, addrlen;
    struct sockaddr_in remoteaddr, localaddr;
    struct sockaddr_in6 remoteaddr6, localaddr6;

    i = link_id;
    sock = s_at_sock_entries[i].socket_id;
    if (s_at_sock_entries[i].family == AF_INET) {
        memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
        memset(&localaddr, 0, sizeof(struct sockaddr_in));
        addrlen = sizeof(struct sockaddr_in);
        getpeername(sock, (struct sockaddr *)&remoteaddr, &addrlen);
        getsockname(sock, (struct sockaddr *)&localaddr, &addrlen);
        if (s_at_sock_entries[i].socket_type == SOCK_UDP) {
            remoteaddr.sin_addr.s_addr = s_at_sock_entries[i].addr;
            remoteaddr.sin_port = s_at_sock_entries[i].port;
        }
        inet_ntop(AF_INET, &remoteaddr.sin_addr.s_addr, state->addr, ADDR_BUFFERSIZE_IPV4);
        state->remote_port = htons(remoteaddr.sin_port);
        state->local_port = htons(localaddr.sin_port);
    } else { /* reserved for IPv6 */
        memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
        memset(&localaddr6, 0, sizeof(struct sockaddr_in6));
        addrlen = sizeof(struct sockaddr_in6);
        getpeername(sock, (struct sockaddr *)&remoteaddr6, &addrlen);
        getsockname(sock, (struct sockaddr *)&localaddr6, &addrlen);
        if (s_at_sock_entries[i].socket_type == SOCK_UDP) {
            aos_memcpy(remoteaddr6.sin6_addr._S6_un._S6_u32, s_at_sock_entries[i].addr6, 16);
            remoteaddr6.sin6_port = s_at_sock_entries[i].port;
        }
        inet_ntop(AF_INET6, &remoteaddr6.sin6_addr, state->addr, ADDR_BUFFERSIZE_IPV6);
        state->remote_port = htons(remoteaddr6.sin6_port);
        state->local_port = htons(localaddr6.sin6_port);
    }
    if (s_at_sock_entries[i].socket_type == SOCK_TCP) {
        strcpy(state->socket_type, "TCP");
        state->state = 2;
    } else if (s_at_sock_entries[i].socket_type == SOCK_TCPS) {
        strcpy(state->socket_type, "TCP LISTENER"); /* reserved for AT+SERVERSTART */
        state->state = 3;
    } else if (s_at_sock_entries[i].socket_type == SOCK_UDP) {
        strcpy(state->socket_type, "UDP");
        state->state = 2;
    } else if (s_at_sock_entries[i].socket_type == SOCK_UDPS) {
        strcpy(state->socket_type, "UDP SERVER");
        state->state = 2;
    } else if (s_at_sock_entries[i].socket_type == SOCK_TCPIN) {
        strcpy(state->socket_type, "TCP INCOMING"); /* reserved for AT+SERVERSTART */
        state->state = 2;
    } else {
        strcpy(state->socket_type, "NULL");
        state->state = 0;
    }

    state->pdp_cid = s_at_sock_entries[i].pdp_cid;
    state->mode = s_at_sock_entries[i].mode;
    state->server_id = 0;
    aos_memset(state->at_port, 0, AT_URC_PORT_NAME_LEN);
    aic_get_at_port_name(s_aic_ctcpip_cfg.com_id, state->at_port, AT_URC_PORT_NAME_LEN);
}

static int aic_ctcpip_find_index_by_socket(int socket)
{
    int i;

    for (i = 0; i < AIC_CTCPIP_SOCKET_MAX_LINKS; i++) {
        if (s_at_sock_entries[i].socket_id == socket) {
            return i;
        }
    }

    return AIC_EPERM;
}

/* reserved for AT+SERVERSTART */
static void aic_ctcpip_socket_listen_thread_handle(void *thread_input)
{
    int sock, conn_sock, port, i, ret, fromaddr_len;
    struct sockaddr_in fromaddr;
    struct sockaddr_in6 fromaddr6;
    fd_set read_ready;
    struct timeval timeout;
    u8 addr[ADDR_BUFFERSIZE_IPV6] = {'\0'};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    FD_ZERO(&read_ready);
    ret = listen(s_at_tcp_server.sockfd, AIC_CTCPIP_SOCKET_LISTEN_MAX);
    if (ret < 0) {
        aicx_error(0x1af7dd, "", "[aic_ctcpip_socket_listen_thread_handle]: at socket listen fail\n");
        return;
    }
    while (1) {
        read_ready = s_at_tcp_server.master_list;
        sock = s_at_tcp_server.sockfd;
        aos_sleep(20);
        timeout.tv_sec = 0;
        timeout.tv_usec = NX_IP_PERIODIC_RATE * 1000;
        ret = select(FD_SETSIZE, &read_ready, 0, 0, &timeout);
        if (ret == ERROR || ret == 0) {
            continue;
        }
        if (FD_ISSET(sock, &read_ready)) {
            if (s_at_tcp_server.family == AF_INET) {
                fromaddr_len = sizeof(fromaddr);
                conn_sock = accept(sock, (struct sockaddr *)&fromaddr, &fromaddr_len);
                inet_ntop(AF_INET, &fromaddr.sin_addr.s_addr, addr, ADDR_BUFFERSIZE_IPV4);
                port = htons(fromaddr.sin_port);
            } else { /* reserved for IPv6 */
                fromaddr_len = sizeof(fromaddr6);
                conn_sock = accept(sock, (struct sockaddr *)&fromaddr6, &fromaddr_len);
                inet_ntop(AF_INET6, &fromaddr6.sin6_addr, addr, ADDR_BUFFERSIZE_IPV6);
                port = htons(fromaddr6.sin6_port);
            }
            if (conn_sock > 0) {
                aic_socket_register_receive_notify(sock, aic_ctcpip_socket_recv_cb, NULL);
                aic_socket_register_tcp_disconnect_notify(sock, aic_ctcpip_socket_tcp_disconnet_cb, NULL);
            } else {
                aicx_error(0x07573e, "", "[CTCPIP]: accept TCP client failed. IGNORE IT.");
                continue;
            }
            /* get available slot first */
            aos_get_mutex(s_at_sock_entries_mutex, AOS_WAIT_FOREVER);
            for (i = 0; i < AIC_CTCPIP_SOCKET_MAX_LINKS; i++) {
                if (s_at_sock_entries[i].socket_id == 0) {
                    s_at_sock_entries[i].link_id = i;
                    s_at_sock_entries[i].socket_id = conn_sock;
                    s_at_sock_entries[i].family = s_at_tcp_server.family;
                    s_at_sock_entries[i].socket_type = SOCK_TCPIN;
                    snprintf(str, sizeof(str), "+CLIENT: %d,%d,%s:%d", i, sock, addr, port);
                    aic_transmit_at_cmd_urc(s_aic_ctcpip_cfg.com_id, str, strlen(str));
                    break;
                }
            }
            aos_put_mutex(s_at_sock_entries_mutex);
            if (i == AIC_CTCPIP_SOCKET_MAX_LINKS) {
                aicx_error(0x05f7e6, "", "[CTCPIP]: new client arrival but socket slot is full.");
                soc_close(conn_sock);
            }
        }
    }
}

static void aic_ctcpip_socket_listen_init(void)
{
    s_at_socket_listen_thread = aos_create_thread("AT_CTCPIP_SOCKET_THREAD", aic_ctcpip_socket_listen_thread_handle,
                                                  NULL, SIZE_2K, 0, 0, 15, AOS_DONT_START);
    if (s_at_socket_listen_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x044b05, "", "[aic_ctcpip_socket_listen_init]: create thread fail\n");
        return;
    }
    s_at_socket_listen_event = aos_create_event("AT_CTCPIP_SOCKET_LISTEN_EVENT");
    if (!s_at_socket_listen_event) {
        aicx_error(0x056e3a, "", "[aic_ctcpip_socket_listen_init]: create event fail\n");
        aos_delete_thread(s_at_socket_listen_thread);
        return;
    }
    aos_resume_thread(s_at_socket_listen_thread);
}

static void aic_ctcpip_socket_recv_cb(int sock, void *data)
{
    int i, ret, fromaddrlen;
    int len, len_t, total_read_cnt = 0; /* total_read_cnt is the offset of readed data from socket */
    u8 *buffer = AIC_NULL, *result_str = AIC_NULL;
    int offset = 0; /* offset is the iterator of pointer buffer */
    struct sockaddr_in fromaddr;
    struct sockaddr_in6 fromaddr6;
    char addr[ADDR_BUFFERSIZE_IPV6];
    u16 port;
    boolean end_loop = AIC_FALSE;

    if (NX_SOC_OK != getsockopt(sock, SOL_SOCKET, SO_RXDATA, &len, &len_t)) {
        aicx_error(0x09ec5d, "", "[aic_ctcpip_socket_recv_cb]: failed to get socket len.");
        return;
    }
    buffer = aos_zalloc(AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN + 1);
    if (!buffer) {
        aicx_error(0x16e957, "", "[aic_ctcpip_socket_recv_cb] allocate buffer failed.");
        goto EXIT;
    }
    result_str = aos_zalloc(AIC_CTCPIP_RESP_MAX_RECV_DATA_LEN + 50);
    if (!result_str) {
        aicx_error(0x142efa, "", "[aic_ctcpip_socket_recv_cb] allocate result_str failed.");
        goto EXIT;
    }
    i = aic_ctcpip_find_index_by_socket(sock);
    if ((unsigned)i > AIC_CTCPIP_SOCKET_MAX_LINKS) {
        aicx_error(0x0f1cee, "", "[aic_ctcpip_socket_recv_cb]: invalid socket index.");
        goto EXIT;
    }
    while (total_read_cnt < len && !end_loop) {
        if (s_aic_ctcpip_cfg.ciprxget_mode == AT_CIPRXGET_MODE_AUTO) {
            aicx_info(0x186de9, "", "[aic_ctcpip_socket_recv_cb] auto mode");
        RECV_MORE:
            if (s_at_sock_entries[i].socket_type == SOCK_TCP || s_at_sock_entries[i].socket_type == SOCK_TCPv6) {
                ret = recv(sock, buffer + offset, AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN - offset, MSG_DONTWAIT);
            } else {
                if (s_at_sock_entries[i].family == AF_INET) {
                    fromaddrlen = sizeof(struct sockaddr_in);
                    ret = recvfrom(sock, buffer + offset, AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN - offset, MSG_DONTWAIT,
                                   (struct sockaddr *)&fromaddr, &fromaddrlen);
                } else {
                    fromaddrlen = sizeof(struct sockaddr_in6);
                    ret = recvfrom(sock, buffer + offset, AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN - offset, MSG_DONTWAIT,
                                   (struct sockaddr *)&fromaddr6, &fromaddrlen);
                }
            }
            if (ret > 0) {
                offset += ret;
                total_read_cnt += ret;
                if (total_read_cnt < len && offset < AIC_CTCPIP_SOCKET_MAX_READ_DATA_LEN) {
                    aicx_error(0x1655ae, "dddd", "[aic_ctcpip_socket_recv_cb]: recv (%d). buf filled (%d). Total(%d/%d) bytes.", ret,
                               offset, total_read_cnt, len);
                    /* read data as as much as possible */
                    goto RECV_MORE;
                }
            } else {
                aicx_error(0x0260b1, "ddd", "[aic_ctcpip_socket_recv_cb] recv 0 bytes, ret is %d, offset is %d, errno is %d", ret,
                           offset, errno);
                /* DO NOT break directly here because there remains some more data to report */
                end_loop = AIC_TRUE;
            }
            /* URC handle */
            if (offset > 0) {
                char temp_buf[50] = {0};
                if (s_at_sock_entries[i].family == AF_INET) {
                    if (s_at_sock_entries[i].socket_type != SOCK_UDP && s_at_sock_entries[i].socket_type != SOCK_UDPS) {
                        fromaddrlen = sizeof(struct sockaddr_in);
                        getpeername(sock, (struct sockaddr *)&fromaddr, &fromaddrlen);
                    }
                    inet_ntop(AF_INET, &fromaddr.sin_addr.s_addr, addr, ADDR_BUFFERSIZE_IPV4);
                    port = htons(fromaddr.sin_port);
                } else {
                    if (s_at_sock_entries[i].socket_type != SOCK_UDP && s_at_sock_entries[i].socket_type != SOCK_UDPS) {
                        fromaddrlen = sizeof(struct sockaddr_in6);
                        getpeername(sock, (struct sockaddr *)&fromaddr6, &fromaddrlen);
                    }
                    inet_ntop(AF_INET6, &fromaddr6.sin6_addr, addr, ADDR_BUFFERSIZE_IPV6);
                    port = htons(fromaddr6.sin6_port);
                }
                /* add "RECV FROM" header */
                if (s_aic_ctcpip_cfg.cipsrip_mode == AT_CIPSRIP_MODE_SHOW) {
                    snprintf(temp_buf, 50, "RECV FROM:\"%s\":%d\r\n", addr, port);
                    strcat(result_str, temp_buf);
                }
                /* add "+IPD" header */
                if (s_aic_ctcpip_cfg.ciphead_mode == AT_CIPHEAD_MODE_ADD_IP_HEAD) {
                    snprintf(temp_buf, 50, "+IPD(%d)\r\n", offset);
                    strcat(result_str, temp_buf);
                }
                /* report data */
                strcat(result_str, buffer);
                aic_transmit_at_cmd_urc(s_aic_ctcpip_cfg.com_id, result_str, strlen(result_str));
                offset = 0;
                memset(buffer, 0, AIC_CTCPIP_RESP_MAX_RECV_DATA_LEN);
            }
        } else {
            if (s_at_sock_entries[i].recv_total == 0) {
                snprintf(result_str, AIC_CTCPIP_RESP_MAX_RECV_DATA_LEN, "+CIPRXGET: 1,%d",
                         s_at_sock_entries[i].link_id);
                aic_transmit_at_cmd_urc(s_aic_ctcpip_cfg.com_id, result_str, strlen(result_str));
            }
            s_at_sock_entries[i].recv_total = len;
            break;
        }
    }

EXIT:
    if (buffer) {
        aos_free(buffer);
    }
    if (result_str) {
        aos_free(result_str);
    }
}

static void aic_ctcpip_socket_tcp_disconnet_cb(int sock, void *data, int is_active)
{
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};
    int i;
    int close_reason = AT_IPCLOSE_CLOSE_REASON_PASSIVE;

    i = aic_ctcpip_find_index_by_socket(sock);
    if ((unsigned)i > AIC_CTCPIP_SOCKET_MAX_LINKS) {
        aicx_error(0x05679c, "", "[aic_ctcpip_socket_tcp_disconnet_cb]: invalid socket index.");
        return;
    }
    /* check close reason */
    if (is_active) {
        close_reason = AT_IPCLOSE_CLOSE_REASON_ACTIVE;
    } else {
        struct sock_errno sock_err = {0};
        INT option_len = sizeof(sock_err);

        if (NX_SOC_OK == getsockopt(sock, SOL_SOCKET, SO_ERROR, &sock_err, &option_len)) {
            if (sock_err.error == ETIMEDOUT) {
                close_reason = AT_IPCLOSE_CLOSE_REASON_TIMEOUT;
            }
        }
    }
    snprintf(str, AIC_CTCPIP_RESP_MAX_RECV_DATA_LEN, "+IPCLOSE: %d,%d", s_at_sock_entries[i].link_id, close_reason);
    aic_transmit_at_cmd_urc(s_aic_ctcpip_cfg.com_id, str, strlen(str));
    if (close_reason == AT_IPCLOSE_CLOSE_REASON_ACTIVE) {
        return;
    }
    /* Enable CONFIG_CTCPIP_CLOSE_SOCKET_MANUALLY if the customer requires to keep socket open
     * until right +CIPCLOSE AT cmd is set. */
#ifndef CONFIG_CTCPIP_CLOSE_SOCKET_MANUALLY
    soc_close(sock);
    if (s_at_sock_entries[i].socket_type == SOCK_TCP || s_at_sock_entries[i].socket_type == SOCK_TCPIN) {
        aic_socket_unregister_tcp_disconnect_notify(s_at_sock_entries[i].socket_id);
    }
    aic_socket_unregister_receive_notify(s_at_sock_entries[i].socket_id);
    aos_memset(&s_at_sock_entries[i], 0, sizeof(struct SOCKET_LIST));
#endif
}

static int aic_ctcpip_create_tcp_socket(int family, struct sockaddr *addr, int linkid, struct sockaddr *srcaddr,
                                        int isclient, s32 *err_code)
{
    int i, sock, addrlen, ret;
    struct timeval timeout;

    if (!addr) {
        if (err_code)
            *err_code = AIC_CTCPIP_ERR_FAIL_TO_CREATE_SOCKET;
        return AIC_EFAULT;
    }
    sock = socket(addr->sa_family, SOCK_STREAM, NX_PROTOCOL_TCP);
    if (sock == NX_SOC_ERROR) {
        aicx_error(0x07bfa5, "d", "[aic_ctcpip_create_tcp_socket]: create tcp socket error, errno %d\n", errno);
        if (err_code)
            *err_code = AIC_CTCPIP_ERR_FAIL_TO_CREATE_SOCKET;
        return AIC_EFAULT;
    }
    if (family == AF_INET)
        addrlen = sizeof(struct sockaddr_in);
    else
        addrlen = sizeof(struct sockaddr_in6);
    if (srcaddr) {
        ret = bind(sock, srcaddr, addrlen);
        if (ret < 0) {
            if (err_code)
                *err_code = AIC_CTCPIP_ERR_FAIL_TO_BIND_SOCKET;
            soc_close(sock);
            aicx_error(0x0fa783, "d", "[aic_ctcpip_create_tcp_socket] socket bind fail error %d\n", errno);
            return ret;
        }
    }
    timeout.tv_sec = s_aic_ctcpip_cfg.ciptimeout_cipsend;
    timeout.tv_usec = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret)
        aicx_error(0x09382c, "x", "[aic_ctcpip_create_tcp_socket] set tcp send timeout fail 0x%x\r\n", ret);
    if (isclient == 0) { /* reserved for AT+SERVERSTART */
        bind(sock, addr, addrlen);
        s_at_tcp_server.sockfd = sock;
        s_at_tcp_server.family = addr->sa_family;
        aic_ctcpip_socket_listen_init();
        FD_SET(sock, &s_at_tcp_server.master_list);
    } else {
        ret = fcntl(sock, F_SETFL, O_NONBLOCK);
        if (ret == NX_SOC_ERROR) {
            if (err_code)
                *err_code = AIC_CTCPIP_ERR_FAIL_TO_CREATE_SOCKET;
            soc_close(sock);
            return AIC_EFAULT;
        }
        /* Try to connect, up to <AIC_CTCPIP_SOCKET_RETRY_MAX_ATTEMPT_CNT> attempts */
        for (int i = 0; i <= AIC_CTCPIP_SOCKET_RETRY_MAX_ATTEMPT_CNT; i++) {
            ret = connect(sock, addr, addrlen);
            if (ret != NX_SOC_ERROR) {
                break;
            } else {
                aicx_error(0x01819a, "d", "[aic_ctcpip_create_tcp_socket]: connect fail, errno %d\n", errno);
                if (errno == EISCONN)
                    break;
                if (i == AIC_CTCPIP_SOCKET_RETRY_MAX_ATTEMPT_CNT) {
                    if (err_code)
                        *err_code = AIC_CTCPIP_ERR_FAIL_TO_CREATE_SOCKET;
                    soc_close(sock);
                    return AIC_EFAULT;
                }
                if (errno == EINPROGRESS || errno == EALREADY)
                    aos_sleep(2 * NX_IP_PERIODIC_RATE);
            }
        }
    }
    aic_socket_register_receive_notify(sock, aic_ctcpip_socket_recv_cb, NULL);
    aic_socket_register_tcp_disconnect_notify(sock, aic_ctcpip_socket_tcp_disconnet_cb, NULL);
    i = linkid;
    if (s_at_sock_entries[i].socket_id == 0) {
        s_at_sock_entries[i].link_id = linkid;
        s_at_sock_entries[i].socket_id = sock;
        s_at_sock_entries[i].family = addr->sa_family;
        s_at_sock_entries[i].in_use = TRUE;
        if (isclient)
            s_at_sock_entries[i].socket_type = SOCK_TCP;
        else
            s_at_sock_entries[i].socket_type = SOCK_TCPS;
    }

    return sock;
}

static int aic_ctcpip_create_udp_socket(int family, struct sockaddr *remoteaddr, int linkid, struct sockaddr *srcaddr,
                                        int isclient, s32 *err_code)
{
    int i, sock, ret, addrlen;
    struct timeval timeout;

    sock = socket((USHORT)family, SOCK_DGRAM, NX_PROTOCOL_UDP);
    if (sock == NX_SOC_ERROR) {
        aicx_error(0x13f47a, "d", "[aic_ctcpip_create_udp_socket]: --create udp socket error, errno %d\n", errno);
        if (err_code)
            *err_code = AIC_CTCPIP_ERR_FAIL_TO_CREATE_SOCKET;
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }
    if (family == AF_INET)
        addrlen = sizeof(struct sockaddr_in);
    else
        addrlen = sizeof(struct sockaddr_in6);
    if (srcaddr) {
        ret = bind(sock, srcaddr, addrlen);
        if (ret < 0) {
            if (err_code)
                *err_code = AIC_CTCPIP_ERR_FAIL_TO_CREATE_SOCKET;
            aicx_error(0x0e1cf8, "d", "[aic_ctcpip_create_udp_socket] socket bind fail error %d\n", errno);
            return ret;
        }
    }
    timeout.tv_sec = s_aic_ctcpip_cfg.ciptimeout_cipsend;
    timeout.tv_usec = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (ret)
        aicx_error(0x11b0ba, "x", "[aic_ctcpip_create_udp_socket] set udp send timeout fail 0x%x\r\n", ret);
    aic_socket_register_receive_notify(sock, aic_ctcpip_socket_recv_cb, NULL);
    i = linkid;
    if (s_at_sock_entries[i].socket_id == 0) {
        s_at_sock_entries[i].link_id = linkid;
        s_at_sock_entries[i].socket_id = sock;
        if (remoteaddr) {
            s_at_sock_entries[i].family = remoteaddr->sa_family;
            if (family == AF_INET) {
                s_at_sock_entries[i].addr = ((struct sockaddr_in *)remoteaddr)->sin_addr.s_addr;
                s_at_sock_entries[i].port = ((struct sockaddr_in *)remoteaddr)->sin_port;
            } else {
                aos_memcpy(s_at_sock_entries[i].addr6, &((struct sockaddr_in6 *)remoteaddr)->sin6_addr, 16);
                s_at_sock_entries[i].port = ((struct sockaddr_in6 *)remoteaddr)->sin6_port;
            }
        } else {
            s_at_sock_entries[i].family = (USHORT)family;
            s_at_sock_entries[i].port = -1;
        }

        if (isclient)
            s_at_sock_entries[i].socket_type = SOCK_UDP;
        else
            s_at_sock_entries[i].socket_type = SOCK_UDPS;
    }

    return sock;
}

static int aic_ctcpip_udp_socket_send(int socket_id, char *data, int datalen)
{
    struct sockaddr_in destaddr;
    struct sockaddr_in6 destaddr6;
    int index, ret;

    index = aic_ctcpip_find_index_by_socket(socket_id);
    if ((unsigned)index > AIC_CTCPIP_SOCKET_MAX_LINKS) {
        return AIC_EINVAL;
    }
    if (s_at_sock_entries[index].family == AF_INET) {
        destaddr.sin_family = AF_INET;
        destaddr.sin_port = s_at_sock_entries[index].port;
        destaddr.sin_addr.s_addr = s_at_sock_entries[index].addr;
        ret = sendto(socket_id, data, datalen, 0, (struct sockaddr *)&destaddr, sizeof(destaddr));
    } else {
        destaddr6.sin6_family = AF_INET6;
        destaddr6.sin6_port = s_at_sock_entries[index].port;
        aos_memcpy(destaddr6.sin6_addr._S6_un._S6_u32, s_at_sock_entries[index].addr6, 16);
        ret = sendto(socket_id, data, datalen, 0, (struct sockaddr *)&destaddr6, sizeof(destaddr6));
    }

    return ret;
}

////////////////////////////////////////////////////////
//  other functions
////////////////////////////////////////////////////////
static void aic_ctcpip_transmit_iperror_urc(u32 serial_port_id, int err)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, sizeof(result_line), "+IP ERROR: %d", err);
    aic_transmit_at_cmd_urc(serial_port_id, result_line, strlen(result_line));
}

static void aic_ctcpip_transmit_cipopen_urc(u32 serial_port_id, int link, int err)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, sizeof(result_line), "+CIPOPEN: %d,%d", link, err);
    aic_transmit_at_cmd_urc(serial_port_id, result_line, strlen(result_line));
}

static void aic_ctcpip_transmit_cipclose_urc(u32 serial_port_id, int link, int err)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, sizeof(result_line), "+CIPCLOSE: %d,%d", link, err);
    aic_transmit_at_cmd_urc(serial_port_id, result_line, strlen(result_line));
}

static void aic_ctcpip_transmit_ciperror_urc(u32 serial_port_id, int err)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, sizeof(result_line), "+CIPERROR: %d", err);
    aic_transmit_at_cmd_urc(serial_port_id, result_line, strlen(result_line));
}

static void aic_ctcpip_transmit_cipsend_urc(u32 serial_port_id, int link_id, int req_len, int sent_len)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, sizeof(result_line), "+CIPSEND: %d,%d,%d", link_id, req_len, sent_len);
    aic_transmit_at_cmd_urc(serial_port_id, result_line, strlen(result_line));
}

static void aic_ctcpip_transmit_cipclose_resp(AT_REQ_RESP *at_req_ptr, int link, int err)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(result_line, sizeof(result_line), "+CIPCLOSE: %d,%d", link, err);
    aic_transmit_resp_normal(at_req_ptr, result_line);
}

////////////////////////////////////////////////////////
//  Init functions
////////////////////////////////////////////////////////
static void aic_ctcpip_at_cfg_init(void)
{
    s_aic_ctcpip_cfg.ciptimeout_netopen = AT_CIPTIMEOUT_DEF_VALUE;
    s_aic_ctcpip_cfg.ciptimeout_cipopen = AT_CIPTIMEOUT_DEF_VALUE;
    s_aic_ctcpip_cfg.ciptimeout_cipsend = AT_CIPTIMEOUT_DEF_VALUE;
    s_aic_ctcpip_cfg.cipsrip_mode = AT_CIPSRIP_MODE_SHOW;
    s_aic_ctcpip_cfg.ciphead_mode = AT_CIPHEAD_MODE_ADD_IP_HEAD;
    s_aic_ctcpip_cfg.ciprxget_mode = AT_CIPRXGET_MODE_AUTO;
}

static int aic_ctcpip_at_cmd_init(void)
{
    aic_add_at_cmd_handle(s_ctcpip_at_cmds);
    aic_ctcpip_at_cfg_init();
    s_at_sock_entries_mutex = aos_create_mutex("s_at_sock_entries_mutex");
    AIC_ASSERT(s_at_sock_entries_mutex);

    return AIC_SUCCESS;
}

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_ctcpip_at_cmd_init);