/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to SNTP.
 *  This is the handler of AT commands relating to SNTP,
 *  such as query sntp time, etc.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e079d
#define LOG_MODULE    AMOD_NET

#include "aic_at_sntp.h"

#define SERVER_IP_ADDRESS  IP_ADDRESS(192,2,2,92)

int time_zone = AIC_DEFAULT_TIME_ZONE;
int sntpcfg_enable = 0;
u32 sntp_urc_comid;

extern int sntp_is_sync;
#ifdef CONFIG_NET_SNTP
extern NX_SNTP_CLIENT *aic_sntp_client;
extern aos_event_ptr  aic_sntp_client_event;
#endif
extern u32 sntp_server_ip;
extern NXD_ADDRESS sntp_server_address;
#ifdef CONFIG_NET_AT_SNTP
extern u32 aic_sntp_client_get_time(CHAR * buffer, UINT length, int time_zone);
#endif
extern s32 current_time_zone;
extern u8 time_zone_set_flag;

#ifdef CONFIG_NET_AT_SNTP
u32 handle_cipsntpcfg_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    struct addrinfo *res = AIC_NULL;
    u8 time_ze[8] = {'\0'};
    ulong *paras_arr;
    int ret;

    aicx_info(0x033953, "s", "[AT_SNTP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                        "AT+CIPSNTPCFG=",
                                        &cnt);
    if ( !paras_arr || (cnt < 2 || cnt > 3)) {
        aicx_error(0x128a98, "", "[AT_SNTP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if ((*(u8 *)paras_arr[0] != '0') && (*(u8 *)paras_arr[0] != '1')) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    } else
        sntpcfg_enable = atoi((u8 *)paras_arr[0]);

    if (*(u8 *)paras_arr[0] == '1' && cnt != 3) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (cnt == 3) {
        ret = getaddrinfo((u8 *)paras_arr[2], NULL, NULL, &res);
        if (ret != NX_SUCCESS || !res) {
            aicx_error(0x1449ba, "sd", "[AT_SNTP]: get addr info fail %s ret %d\r\n",
                       (s8 *)paras_arr[0], ret);
            aic_free_paras_mem(paras_arr, cnt);
            aic_reply_error_resp(at_req_resp_ptr,
                                    AIC_TCPIP_ERR_DNS_FAIL);
            return ret;
        }

        aicx_info(0x00fd90, "x", "[AT_SNTP]:sntp server addr is 0x%x\n",
                 ((struct sockaddr_in *)res->ai_addr)->sin_addr);
        if(res->ai_addr->sa_family == AF_INET) {
            memcpy(&sntp_server_ip,
                   &((struct sockaddr_in *)res->ai_addr)->sin_addr, 4);
            aic_sntp_client->nx_sntp_unicast_time_server.nxd_ip_address.v4
                = sntp_server_ip;
            aic_sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v4
                = sntp_server_ip;
        } else {
            memcpy(&sntp_server_address.nxd_ip_address.v6,
                   &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
            memcpy(aic_sntp_client->nx_sntp_unicast_time_server.nxd_ip_address.v6,
                   &sntp_server_address.nxd_ip_address.v6, 16);
            memcpy(aic_sntp_client->nx_sntp_server_ip_address.nxd_ip_address.v6,
                   &sntp_server_address.nxd_ip_address.v6, 16);
        }

        if (res)
            freeaddrinfo(res);
    }

    strcpy(time_ze, (u8 *)paras_arr[1]);
    if (strlen(time_ze) > 0) {
        time_zone = atoi(time_ze) * 4;
        if (aic_is_valid_time_zone(time_zone) && time_zone_set_flag != 1)
            current_time_zone = time_zone;
    } else {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if(time_zone < SNTP_TIMEZONE_MIN && time_zone > SNTP_TIMEZONE_MAX) {
        aicx_info(0x0e5faf, "", "[AT_SNTP]: time_zone is invalid\n");
        time_zone = AIC_DEFAULT_TIME_ZONE;
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    if (sntpcfg_enable) {
        /*set sntp request send event: SNTP_AT_SEND_EVENT*/
        aos_set_event(aic_sntp_client_event, 1 << 2, TX_OR);
        aic_sntp_client->nx_sntp_server_update_time_active = 1;
        sntp_urc_comid = at_req_resp_ptr->serial_port_id;
        sntp_is_sync = AIC_TRUE;
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsntpcfg_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    char addr[ADDR_BUFFERSIZE_IPV6] = {0};
    char str[AT_RESP_RESULT_MAX] = {0};

    aicx_info(0x033953, "s", "[AT_SNTP]: --%s\n", at_req_resp_ptr->at_req);

    if (!sntp_server_ip)
        sntp_server_ip = SERVER_IP_ADDRESS;

    if (sntp_server_ip)
        inet_ntop(AF_INET, &sntp_server_ip, addr, ADDR_BUFFERSIZE_IPV4);
    else
        inet_ntop(AF_INET6, &sntp_server_address.nxd_ip_address.v6,
                  addr, ADDR_BUFFERSIZE_IPV6);

    if(time_zone > 0)
        sprintf(str, "+CIPSNTPCFG: %d,%s,UTC+%d", sntpcfg_enable,
                addr, time_zone);
    else if (time_zone < 0)
        sprintf(str, "+CIPSNTPCFG: %d,%s,UTC%d", sntpcfg_enable,
                addr, time_zone);
    else
        sprintf(str, "+CIPSNTPCFG: %d,%s,UTC", sntpcfg_enable, addr);

    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}

/**
 * handle_cipsntptime_query_cmd() - query time.
 * @para_num: parameter.
 *
 * CALLS:
 *    non
 *
 * CALLED BY:
 *    Application
 *
 * Set transparent transmission mode.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 handle_cipsntptime_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_SNTP_CLIENT *client_ptr = aic_sntp_client;
    char buffer[SNTPTIME_SIZE] = {'\0'};
    char str[AT_RESP_RESULT_MAX] = {0};
    int timezone;

    aicx_info(0x033953, "s", "[AT_SNTP]: --%s\n", at_req_resp_ptr->at_req);

    if (!client_ptr) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    if (time_zone == AIC_INVALID_TIME_ZONE)
        timezone = 0;
    else
        timezone = time_zone;

    aic_sntp_client_get_time( buffer, SNTPTIME_SIZE, timezone);

    snprintf(str, sizeof(str), "+CIPSNTPTIME: %s", buffer);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}


u32 handle_cipsntpintv_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_SNTP_CLIENT * client_ptr = aic_sntp_client;
    u32 second;
    u16 cnt;
    ulong *paras_arr;

    aicx_info(0x033953, "s", "[AT_SNTP]: --%s\n", at_req_resp_ptr->at_req);

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                        "AT+CIPSNTPINTV=",
                                        &cnt);
    if ( !paras_arr || cnt != 1 || !strlen((u8 *)paras_arr[0])) {
        aicx_error(0x128a98, "", "[AT_SNTP]: parameter parse error\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    if (!client_ptr) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    second = atoi((u8 *)paras_arr[0]);
    if (second > 15 && second < 4294967)
        client_ptr->nx_sntp_client_unicast_poll_interval =
                                    second * NX_IP_PERIODIC_RATE;
    else {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_INVAL_AT_PARAM);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_ERR_INVAL_AT_PARAM;
    }

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;
}

u32 handle_cipsntpintv_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    NX_SNTP_CLIENT * client_ptr = aic_sntp_client;
    char str[AT_RESP_RESULT_MAX] = {0};
    u32 second;

    aicx_info(0x033953, "s", "[AT_SNTP]: --%s\n", at_req_resp_ptr->at_req);

    if (!client_ptr) {
        aic_reply_error_resp(at_req_resp_ptr,
                                AIC_TCPIP_ERR_PDP_NOT_ACTIVE);
        return AIC_TCPIP_ERR_PDP_NOT_ACTIVE;
    }

    second = (client_ptr->nx_sntp_client_unicast_poll_interval) / NX_IP_PERIODIC_RATE;

    sprintf(str, "+CIPSNTPINTV: %d", second);
    aic_transmit_resp_normal(at_req_resp_ptr, str);
    aic_reply_ok_resp(at_req_resp_ptr);
    return AIC_TCPIP_AT_OK;
}
#endif


