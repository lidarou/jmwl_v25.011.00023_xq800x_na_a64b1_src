/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Do Ping use AT Cmds.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0be5ee
#define LOG_MODULE    AMOD_NET

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <aic_init.h>
#include "aic_at_tcpip_common.h"
#include "aic_at_ping.h"
#include "aic_net_pdp_mgr.h"

static AT_PING_CFG at_ping_cfg;
static aos_event_ptr at_ping_event;
static aos_thread_id at_ping_thread;

/**
 * aic_at_ping_set_cmd_handle() - Do Ping.
 * @para_num: parameter.
 *
 * CALLS:
 *    getaddrinfo, nxd_icmp_ping
 *
 * CALLED BY:
 *    Application
 *
 * Do Ping, and info result.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_at_ping_set_cmd_handle(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt;
    ulong *paras_arr;
    u32 count = 4, interval = 1, timeout = 1;
    int packet_len = PING_DATA_SIZE_DEF;
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+PING=",
                                             &cnt);
    if (!paras_arr || (cnt < 1 || cnt > 5)) {
        aicx_error(0x1d1544, "", "[AT_PING]: parameter parse error\r\n");
        if (paras_arr)
            aic_free_paras_mem(paras_arr, cnt);

        aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_PARSE_AT_PARAM);
        return AIC_TCPIP_ERR_PARSE_AT_PARAM;
    }

    aicx_info(0x0afd9c, "d", "[AT_PING]: paras count %d\r\n", cnt);

    if (at_ping_cfg.cnt != 0) {
        aicx_info(0x1e3333, "d", "[AT_PING] ping cnt %d\n", at_ping_cfg.cnt);
        strcpy(str, "+CME ERROR: PING BUSY");
        aic_transmit_resp_normal(at_req_resp_ptr, str);
        aic_free_paras_mem(paras_arr, cnt);
        return AIC_TCPIP_AT_OK;
    }

    if (strlen((u8 *)paras_arr[0]) > 255)
        goto PING_ERROR;

    if (cnt >= 2) {
        if (strlen((u8 *)paras_arr[1]))
            timeout = atoi((u8 *)paras_arr[1]);
        if(timeout < PING_TIMEOUT_MIN || timeout > PING_TIMEOUT_MAX) {
            aicx_error(0x16f729, "", "[AT_PING]: timeout is invalid\r\n");
            goto PING_ERROR;
        }
    }

    aicx_info(0x068e7b, "d", "[AT_PING]: timeout is %d\r\n", timeout);

    if (cnt >= 3) {
        if (strlen((u8 *)paras_arr[2]))
            packet_len = atoi((u8 *)paras_arr[2]);
        if (packet_len >= PING_DATA_SIZE_MIN && packet_len <= PING_DATA_SIZE_MAX) {
            aicx_info(0x175026, "d", "[AT_PING]: packet data len %d\r\n", packet_len);
        } else {
            aicx_error(0x08e0e1, "", "[AT_PING]: packet data len is invalid\r\n");
            goto PING_ERROR;
        }
    }

    if (cnt >= 4) {
        if (strlen((u8 *)paras_arr[3]))
            count = atoi((u8 *)paras_arr[3]);
        if(count > PING_COUNT_MAX) {
            aicx_error(0x1cbc63, "", "[AT_PING]: count is invalid\r\n");
            goto PING_ERROR;
        }
    }

    if (cnt == 5){
        interval = atoi((u8 *)paras_arr[4]);
    }

    aos_memset(&at_ping_cfg, 0, sizeof(AT_PING_CFG));
    strncpy(at_ping_cfg.host, (u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]));
    at_ping_cfg.cid = ANET_DEFAULT_PDP_CID;
    at_ping_cfg.cnt = count;
    at_ping_cfg.ping_interval = interval;
    at_ping_cfg.ping_timeout = timeout;
    at_ping_cfg.ping_size = packet_len;
    at_ping_cfg.ping_comid = at_req_resp_ptr->serial_port_id;
    aos_set_event(at_ping_event, PING_PROCESS_EVENT, TX_OR);

    aic_reply_ok_resp(at_req_resp_ptr);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_AT_OK;

PING_ERROR:
    aic_reply_error_resp(at_req_resp_ptr, AIC_TCPIP_ERR_INVAL_AT_PARAM);
    aic_free_paras_mem(paras_arr, cnt);
    return AIC_TCPIP_ERR_INVAL_AT_PARAM;
}

static void aic_at_tcpip_process_ping(void *pdata)
{
    AT_PING_CFG *cfg = &at_ping_cfg;
    struct addrinfo *res, *dns_res, hints = {0};
    NXD_ADDRESS *destaddr = NULL, destaddr1, destaddr2;
    NX_IPV4_HEADER *ipv4_header;
    NX_IPV6_HEADER *ipv6_header;
    NXD_IPV6_ADDRESS *ipv6_addr = NULL;
    NX_PACKET *response_ptr;
    u8 pdp_type, *at_ping_payload, first_flag = TRUE;
    int ret, i = 0, time;
    u32 events = 0;
    WAN_NET *net_ptr = NULL;
    struct aic_network_ip_instance *net_ip_instance;
    struct aic_ping_result result;
    s8 at_ping_addr[PING_ADDR_BUFFERSIZE_IPV6] = {0};
    u8 str[AT_RESP_RESULT_MAX] = {'\0'};

    while (1) {
        aos_get_event(at_ping_event, PING_PROCESS_EVENT, TX_OR_CLEAR,
                      &events, AOS_WAIT_FOREVER);

        aos_memset(&result, 0, sizeof(struct aic_ping_result));
        destaddr = NULL;
        ipv6_addr = NULL;
        if (events & PING_PROCESS_EVENT) {
            aos_memset(&destaddr1, 0, sizeof(NXD_ADDRESS));
            aos_memset(&destaddr2, 0, sizeof(NXD_ADDRESS));
            net_ptr = aic_get_wan_net_per_pdp(cfg->cid);
            if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
                strcpy(str, "+PING: FAIL");
                aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                aos_memset(&at_ping_cfg, 0, sizeof(at_ping_cfg));
                events = events & ~(PING_PROCESS_EVENT);
                continue;
            }

            net_ip_instance = aic_get_pdp_ip_instance();
            hints.ai_family = AF_UNSPEC;
            ret = getaddrinfo(cfg->host, NULL, &hints, &res);
            if (ret != NX_SUCCESS || !res) {
                aicx_error(0x045947, "sd", "[AT_PING]: get %s addr info fail ret %d\r\n",
                           (s8 *)cfg->host, ret);
                aos_memset(str, '\0', sizeof(str));
                strcpy(str, "+PING: FAIL");
                aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                aos_memset(&at_ping_cfg, 0, sizeof(at_ping_cfg));
                events = events & ~(PING_PROCESS_EVENT);
                continue;
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

                    for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++) {
                        /* Compare the address. */
                        if ((net_ip_instance->ip_ptr->nx_ipv6_address[i].nxd_ipv6_address_valid) &&
                            (CHECK_IPV6_ADDRESSES_SAME(net_ip_instance->ip_ptr->nx_ipv6_address[i].nxd_ipv6_address,
                                                       net_ptr->wan_ipv6_address1))) {
                             /* Found a proper address. */
                             ipv6_addr = &(net_ip_instance->ip_ptr->nx_ipv6_address[i]);
                        }
                    }
                    break;
                }
                dns_res = dns_res->ai_next;
            }
            freeaddrinfo(res);

            pdp_type = aic_get_pdp_ip_type(cfg->cid);
            if ((pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
                pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) &&
                destaddr2.nxd_ip_version == NX_IP_VERSION_V6 &&
                ipv6_addr) {
                destaddr = &destaddr2;
                aicx_info(0x04652a, "xxxxxxxx", "[AT_PING]: get address is v6 %x:%x:%x:%x:%x:%x:%x:%x\r\n",
                          destaddr2.nxd_ip_address.v6[0] >> 16,
                          destaddr2.nxd_ip_address.v6[0] & 0xFFFF,
                          destaddr2.nxd_ip_address.v6[1] >> 16,
                          destaddr2.nxd_ip_address.v6[1] & 0xFFFF,
                          destaddr2.nxd_ip_address.v6[2] >> 16,
                          destaddr2.nxd_ip_address.v6[2] & 0xFFFF,
                          destaddr2.nxd_ip_address.v6[3] >> 16,
                          destaddr2.nxd_ip_address.v6[3] & 0xFFFF);
            } else if ((pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
                     pdp_type == ANET_DEFAULT_IP_TYPE_V4V6 ) &&
                     destaddr1.nxd_ip_version == NX_IP_VERSION_V4) {
                destaddr = &destaddr1;
                aicx_info(0x04de95, "dddd", "[AT_PING]: get address is v4 %d:%d:%d:%d\r\n",
                          (destaddr1.nxd_ip_address.v4 >> 24) & 0xFF,
                          (destaddr1.nxd_ip_address.v4 >> 16) & 0xFF,
                          (destaddr1.nxd_ip_address.v4 >> 8) & 0xFF,
                          (destaddr1.nxd_ip_address.v4) & 0xFF);
            } else {
                destaddr = NULL;
            }

            if (!destaddr) {
               snprintf(str, sizeof(str), "+PING: FAIL");
               aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
               aos_memset(&at_ping_cfg, 0, sizeof(at_ping_cfg));
               events = events & ~(PING_PROCESS_EVENT);
               continue;
           }


            if (cfg->ping_size) {
                at_ping_payload = aos_zalloc_relaxed(cfg->ping_size);
                if (!at_ping_payload) {
                    aos_memset(str, '\0', sizeof(str));
                    snprintf(str, sizeof(str), "+PING: FAIL");
                    aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                    cfg->cnt = 0;
                    continue;
                }
            }

            for (i = 0; i < cfg->ping_size; i++)
                *(at_ping_payload + i) = 'a' + i % 24;

            while (cfg->cnt) {
                if (first_flag) {
                    first_flag = FALSE;
                } else {
                    aos_sleep(cfg->ping_interval * NX_IP_PERIODIC_RATE);
                }

                time = aos_get_tickcount();
                result.send_cnt++;

                ret = nxd_icmp_ping(net_ip_instance->ip_ptr, destaddr,
                                    at_ping_payload, cfg->ping_size,
                                    &response_ptr,
                                    cfg->ping_timeout * NX_IP_PERIODIC_RATE);
                aos_memset(str, '\0', sizeof(str));
                if ((ret != NX_SUCCESS) || (response_ptr == NX_NULL)) {
                    if (response_ptr)
                        nx_packet_release(response_ptr);
                    strcpy(str, "+PING: TIMEOUT");
                } else {
                    result.recv_cnt++;
                    time = aos_get_tickcount() - time;
                    if ((result.shortest_time > time) || !result.shortest_time)
                        result.shortest_time = time;

                    if (result.longest_time < time)
                        result.longest_time = time;

                    result.average_time += time;
                    if (response_ptr->nx_packet_ip_version == NX_IP_VERSION_V4) {
                        ipv4_header =
                            (NX_IPV4_HEADER *)response_ptr->nx_packet_ip_header;
                        NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_source_ip);
                        inet_ntop(AF_INET, &ipv4_header->nx_ip_header_source_ip,
                                  at_ping_addr, PING_ADDR_BUFFERSIZE_IPV4);
                        snprintf(str, sizeof(str),
                                 "+PING: Reply from <%s>:byte=%d time=%d(ms) TTL=%ld",
                                 at_ping_addr, cfg->ping_size, time,
                                 (ipv4_header->nx_ip_header_word_2 >> 24));
                    } else if(response_ptr->nx_packet_ip_version == NX_IP_VERSION_V6){
                        ipv6_header =
                            (NX_IPV6_HEADER *)response_ptr->nx_packet_ip_header;
                        NX_CHANGE_ULONG_ENDIAN(ipv6_header->nx_ip_header_source_ip[0]);
                        NX_CHANGE_ULONG_ENDIAN(ipv6_header->nx_ip_header_source_ip[1]);
                        NX_CHANGE_ULONG_ENDIAN(ipv6_header->nx_ip_header_source_ip[2]);
                        NX_CHANGE_ULONG_ENDIAN(ipv6_header->nx_ip_header_source_ip[3]);
                        inet_ntop(AF_INET6, ipv6_header->nx_ip_header_source_ip,
                                  at_ping_addr, PING_ADDR_BUFFERSIZE_IPV6);
                        snprintf(str, sizeof(str), "+PING: Reply from <%s>:time=%d(ms)",
                                 at_ping_addr, time);
                    } else {
                        strcpy(str, "+PING: desttination address unreachable");
                    }
                    nx_packet_release(response_ptr);
                }
                aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
                cfg->cnt--;
            }

            if (result.recv_cnt)
                result.average_time /= result.recv_cnt;

            if (result.send_cnt > 1) {
                snprintf(str, sizeof(str),
                         "+PING: send_cnt: %d recv_cnt: %d lost: %d\r\nshortest_time: %dms longest_time: %dms average_time: %dms. END",
                         result.send_cnt, result.recv_cnt, (result.send_cnt - result.recv_cnt),
                         result.shortest_time, result.longest_time, result.average_time);
                aic_transmit_at_cmd_urc(cfg->ping_comid, str, strlen(str));
            }

            if (at_ping_payload) {
                aos_free(at_ping_payload);
                at_ping_payload = AIC_NULL;
            }

            events = events & ~(PING_PROCESS_EVENT);
        }
        aos_memset(&at_ping_cfg, 0, sizeof(at_ping_cfg));
    }

    return;
}

int aic_at_ping_task_create(void)
{
    at_ping_thread = aos_create_thread("AT PING Thread",
                                       aic_at_tcpip_process_ping,
                                       NULL, SIZE_2K, 0, 0, 15, AOS_DONT_START);
    if (at_ping_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x1d0865, "", "[AT_PING]: at ping thread create fail\n");
        return AIC_EFAULT;
    }

     at_ping_event = aos_create_event("AT PING event flags");
    if (!at_ping_event) {
        aicx_error(0x021812, "", "[AT_PING]: at ping event create fail\n");
        aos_delete_thread(at_ping_thread);
        return AIC_EFAULT;
    }

    aos_resume_thread(at_ping_thread);
    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_at_ping_task_create);

