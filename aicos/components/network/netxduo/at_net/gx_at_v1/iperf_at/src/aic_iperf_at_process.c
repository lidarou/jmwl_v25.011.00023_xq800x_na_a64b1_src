
/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to iperf.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x191d30
#define LOG_MODULE    AMOD_NET

#include <string.h>
#include "tx_api.h"
#include "nx_api.h"
#include "nx_tcp.h"
#include "nx_udp.h"
#include "aic_net_api.h"
#include "aic_net_pdp_mgr.h"

#include <aic_init.h>
#include <aic_size.h>
#include <aic_errno.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_resp_handle.h>
#include <aic_at_parse_utils.h>
#include <aic_iperf_at_process.h>
#include <nx_iperf.h>

#define IPERF_PACKET_POOL_SIZE     ((1600 + sizeof(NX_PACKET)) * 36)
#define IPERF_STACK_SIZE        SIZE_4K

struct aic_packet_pool iperf_pool = {0};

extern ctrl_info          nx_iperf_ctrl_info_rx;
extern ctrl_info          nx_iperf_ctrl_info_tx;
extern ctrl_info          iperf_ctrl_info_rx_list[IPERF_MAX_THREAD_NUM];
extern ctrl_info          iperf_ctrl_info_tx_list[IPERF_MAX_THREAD_NUM];
extern IPERF_SOCKET_LIST iperf_sockets[IPERF_MAX_THREAD_NUM];
extern IPERF_THREAD_ID_LIST iperf_res_thread_list[IPERF_MAX_THREAD_NUM];
extern NX_TCP_SOCKET      tcp_server_socket;
extern NX_TCP_SOCKET      tcp_client_socket;
extern NX_UDP_SOCKET      udp_server_socket;
extern NX_UDP_SOCKET      udp_client_socket;

static NX_IP *iperf_ip_ptr = AIC_NULL;
static struct aic_network_ip_instance *aic_ip_instance = AIC_NULL;
static u8 *iperf_rx_stack_area = AIC_NULL;
static u8 *iperf_tx_stack_area = AIC_NULL;
static IPERF_STACK_AREA_LIST iperf_stack_areas[IPERF_MAX_THREAD_NUM];
static aos_thread_id thread_iperf_rx_process;
static aos_thread_id thread_iperf_tx_process;
static aos_event_ptr aic_iperf_event;

static AT_CMD_HANDLE iperf_at_handles[] =
{
    {"AT+IPERFINIT", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_iperf_init_exe_cmd},

    {"AT+IPERFTCPRX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_iperf_tcprx_exe_cmd},

    {"AT+IPERFTCPTX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_iperf_tcptx_exe_cmd},

    {"AT+IPERFUDPRX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_iperf_udprx_exe_cmd},

    {"AT+IPERFUDPTX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_iperf_udptx_exe_cmd},

    {"AT+IPERFUDPRTX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_iperf_udprtx_exe_cmd},

    {"AT+IPERFTERM", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_iperf_term_exe_cmd},

    {"AT+IPERFMUDPTX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_multi_iperf_udptx_exe_cmd},

    {"AT+IPERFMUDPRX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_multi_iperf_udprx_exe_cmd},

    {"AT+IPERFMUDPRTX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_multi_iperf_udprtx_exe_cmd},

    {"AT+IPERFMTCPTX", AIC_NULL, AIC_NULL, AIC_NULL,
     aic_handle_multi_iperf_tcptx_exe_cmd},

    {AIC_NULL},
};

static u32 transmit_iperf_at_ok(AT_REQ_RESP *at_ptr)
{
    u8 result_line[10] = {'\0'};
    ulong line_p[5];

    strncpy(result_line, AT_RESULT_OK, strlen(AT_RESULT_OK));
    line_p[0] = (ulong)(&result_line);

    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_iperf_at_cme(AT_REQ_RESP *at_ptr, u32 err)
{
    u8 result[20] = {'\0'};
    ulong line_p[5];

    snprintf(result, 20, "+CME ERROR: %d", err);
    line_p[0] = (ulong)(&result);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static u32 transmit_iperf_res(AT_REQ_RESP *at_ptr, u8 thread_id, u8 *mode,
                              u32 run_time, u64 packet_num, u32 packet_speed,
                              u32 MBytes, u64 throughput)
{
    u8 result_line0[132] = {'\0'};
    ulong line_p[5];


    snprintf(result_line0, 132,
             "[IperfId:%d-%s] Run %u secs, Total %u MBytes, %llu Mbits/sec",
             thread_id, mode, run_time, MBytes, throughput);

    line_p[0] = (ulong)(&result_line0);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    return AIC_SUCCESS;
}

static int iperf_tx_application_init(void)
{
    int status;

    /* Create a packet pool. */
    if (!iperf_pool.packet_pool) {
        iperf_pool.per_packet_size = 1600;
        iperf_pool.packet_pool_size = 1600 * 10;
        iperf_pool.packet_pool_name = "iperf_packet_pool";

        status = aic_create_packet_pool(&iperf_pool);
        if (status) {
            aicx_error(0x041aa6, "x", "packet pool create fail(0x%x)\n", status);
            return 1;
        }
    }

    if(!iperf_ip_ptr) {
        aic_ip_instance = aic_get_default_ip_instance();
        if (!aic_ip_instance || !aic_ip_instance->ip_ptr) {
            aicx_error(0x1de78f, "", "network maybe not active\n");
            return 1;
        }
       iperf_ip_ptr = aic_ip_instance->ip_ptr;
    }

    if (!iperf_rx_stack_area)
        iperf_rx_stack_area = aos_zalloc(IPERF_STACK_SIZE);

    if (!iperf_tx_stack_area)
        iperf_tx_stack_area = aos_zalloc(IPERF_STACK_SIZE);

    aic_iperf_entry(iperf_pool.packet_pool, iperf_ip_ptr);
    return status;
}

/* Process URL to get NXD_ADDRESS */
static u32 iperf_host_process(u8 *host)
{
    u8 *v6_str;
    struct addrinfo *res;
    u32 status;
    struct addrinfo *hints = AIC_NULL;

    if (strlen(host) == 0)
        return 1;

    hints = aos_zalloc(sizeof(struct addrinfo));
    if (aic_get_pdp_ip_type(IPERF_DEFAULT_CID) == ANET_DEFAULT_IP_TYPE_V4)
        hints->ai_family = AF_INET;
    else if (aic_get_pdp_ip_type(IPERF_DEFAULT_CID) == ANET_DEFAULT_IP_TYPE_V6)
        hints->ai_family = AF_INET6;
    else
        hints->ai_family = AF_UNSPEC;

    if (strncmp(host, "[", 1) == 0) {
        v6_str = (u8 *)aos_zalloc(68 + 1);
        if (v6_str == AIC_NULL) {
            aos_free(hints);
            return 1;
        }

        strncpy(v6_str, host + 1, 68);
        v6_str[strlen(v6_str) - 1] = '\0';
        status = getaddrinfo(v6_str, NULL, hints, &res);
        aos_free(v6_str);
        v6_str = AIC_NULL;
    } else {
        status = getaddrinfo(host, NULL, hints, &res);
    }

    if (status == 0 && res) {
        if (res->ai_family == AF_INET) {
            nx_iperf_ctrl_info_tx.version = NX_IP_VERSION_V4;
            nx_iperf_ctrl_info_tx.ip =
            ((struct sockaddr_in*)(res->ai_addr))->sin_addr.s_addr;
            NX_CHANGE_ULONG_ENDIAN(nx_iperf_ctrl_info_tx.ip);
        } else if (res->ai_family == AF_INET6) {
            nx_iperf_ctrl_info_tx.version = NX_IP_VERSION_V6;
            nx_iperf_ctrl_info_tx.ipv6[0] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[0];

            nx_iperf_ctrl_info_tx.ipv6[1] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[1];

            nx_iperf_ctrl_info_tx.ipv6[2] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[2];

            nx_iperf_ctrl_info_tx.ipv6[3] =
            ((struct sockaddr_in6*)(res->ai_addr))->sin6_addr.s6_addr32[3];

            NX_CHANGE_ULONG_ENDIAN(nx_iperf_ctrl_info_tx.ipv6[0]);
            NX_CHANGE_ULONG_ENDIAN(nx_iperf_ctrl_info_tx.ipv6[1]);
            NX_CHANGE_ULONG_ENDIAN(nx_iperf_ctrl_info_tx.ipv6[2]);
            NX_CHANGE_ULONG_ENDIAN(nx_iperf_ctrl_info_tx.ipv6[3]);
        }

        freeaddrinfo(res);
        aos_free(hints);
    } else {
        aicx_error(0x11484c, "x", "DNS resolution fail(0x%x)\n", status);
        aos_free(hints);
        return status;
    }

    return AIC_SUCCESS;
}

u32 aic_handle_iperf_init_exe_cmd(AT_REQ_RESP *at_ptr)
{
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    if (at_ptr->at_req[strlen("AT+IPERFINIT")] != 0x0D &&
        at_ptr->at_req[strlen("AT+IPERFINIT")] != 0x0A) {
        transmit_iperf_at_cme(at_ptr, 1);
        return 1;
    }

    iperf_tx_application_init();

    transmit_iperf_at_ok(at_ptr);
    return AIC_SUCCESS;
}

static void iperf_rx_at_entry(void *thread_input)
{
    u32 status;
    u32 events;
    u32 total_MBytes = 0;
    u32 total_KBytes = 0;
    u32 run_count = 0;
    u64 pre_run_time = 0;
    u32 packet_speed = 0;
    u32 misstake_bytes = 0;

    while (1) {
        status = aos_get_event(aic_iperf_event, AIC_IPERF_ANY_EVENT,
                               AOS_OR_CLEAR, &events, AOS_WAIT_FOREVER);

        if (status != AOS_SUCCESS)
            continue;

        if (events & AIC_IPERF_UDP_RX) {
            total_MBytes = 0;
            run_count = 0;
            pre_run_time = 0;
            packet_speed = 0;
            misstake_bytes = 0;
            nx_iperf_udp_rx_test((u8 *)iperf_rx_stack_area, IPERF_STACK_SIZE);

            do {
                while (nx_iperf_ctrl_info_rx.RunTime < 1000) {
                    aos_sleep(100);
                }

                if (nx_iperf_ctrl_info_rx.RunTime >= 1000 &&
                    nx_iperf_ctrl_info_rx.RunTime != pre_run_time) {
                    pre_run_time = nx_iperf_ctrl_info_rx.RunTime;
                    misstake_bytes = nx_iperf_ctrl_info_rx.BytesRxed % (1024 * 1024);
                    total_MBytes = nx_iperf_ctrl_info_rx.BytesRxed / 1024 / 1024;

                    if (total_MBytes <= 0) {
                        total_KBytes = (nx_iperf_ctrl_info_rx.BytesRxed / 1024);
                        nx_iperf_ctrl_info_rx.ThroughPut = nx_iperf_ctrl_info_rx.BytesRxed /
                                                           (nx_iperf_ctrl_info_rx.RunTime / 1000) / 125;
                    } else {
                        total_KBytes = 0;
                        if (misstake_bytes / 1024 >= 512)
                            total_MBytes += 1;

                        nx_iperf_ctrl_info_rx.ThroughPut = nx_iperf_ctrl_info_rx.BytesRxed /
                                                           (nx_iperf_ctrl_info_rx.RunTime / 1000) / 125000;
                    }
                    run_count++;
                    packet_speed = nx_iperf_ctrl_info_rx.PacketsRxed / run_count;
                    transmit_iperf_res_by_port(nx_iperf_ctrl_info_rx.urc_port,
                                               0, "UDP-DL", run_count,
                                               nx_iperf_ctrl_info_rx.PacketsRxed,
                                               packet_speed, total_MBytes, total_KBytes,
                                               nx_iperf_ctrl_info_rx.ThroughPut);
                    aos_sleep(1000);
                }
            } while (!nx_iperf_ctrl_info_rx.ErrorCode);

            nx_iperf_udp_rx_cleanup();
            aic_transmit_at_result(nx_iperf_ctrl_info_rx.urc_port,
                                   "----Iperf-UDP-DL-Test-End----", 29);
        }

        if (events & AIC_IPERF_MULTI_UDP_RX) {
            nx_iperf_udp_multi_rx_test(iperf_rx_stack_area, IPERF_STACK_SIZE);

            while (!nx_iperf_ctrl_info_rx.ErrorCode) {
                aos_sleep(1000);
            }

            nx_iperf_udp_multi_rx_cleanup();
            aic_transmit_at_result(nx_iperf_ctrl_info_rx.urc_port,
                                   "----Iperf-UDP-DL-Test-End----", 29);
        }

    }
}

static void iperf_tx_at_entry(void *thread_input)
{
    u32 status;
    u32 events;
    u32 i;
    u32 total_MBytes = 0;
    u32 total_KBytes = 0;
    u32 run_count = 0;
    u64 pre_run_time = 0;
    u32 packet_speed = 0;
    u32 misstake_bytes = 0;

    while (1) {
        status = aos_get_event(aic_iperf_event, AIC_IPERF_ANY_EVENT,
                               AOS_OR_CLEAR, &events, AOS_WAIT_FOREVER);

        if (status != AOS_SUCCESS)
            continue;

        if (events & AIC_IPERF_UDP_TX) {
            total_MBytes = 0;
            run_count = 0;
            pre_run_time = 0;
            packet_speed = 0;
            misstake_bytes = 0;
            nx_iperf_udp_tx_test((u8 *)iperf_tx_stack_area, IPERF_STACK_SIZE);

            do {
                while (nx_iperf_ctrl_info_tx.RunTime < 1000) {
                    aos_sleep(100);
                }

                if (nx_iperf_ctrl_info_tx.RunTime >= 1000 &&
                    nx_iperf_ctrl_info_tx.RunTime != pre_run_time) {
                    pre_run_time = nx_iperf_ctrl_info_tx.RunTime;
                    misstake_bytes = nx_iperf_ctrl_info_tx.BytesTxed % (1024 * 1024);
                    total_MBytes = nx_iperf_ctrl_info_tx.BytesTxed / 1024 / 1024;

                    if (total_MBytes <= 0) {
                        total_KBytes = (nx_iperf_ctrl_info_tx.BytesTxed / 1024);
                        nx_iperf_ctrl_info_tx.ThroughPut = nx_iperf_ctrl_info_tx.BytesTxed /
                                                           (nx_iperf_ctrl_info_tx.RunTime / 1000) / 125;
                    } else {
                        total_KBytes = 0;
                        if (misstake_bytes / 1024 >= 512)
                            total_MBytes += 1;

                        nx_iperf_ctrl_info_tx.ThroughPut = nx_iperf_ctrl_info_tx.BytesTxed /
                                                           (nx_iperf_ctrl_info_tx.RunTime / 1000) / 125000;
                    }
                    run_count++;
                    packet_speed = nx_iperf_ctrl_info_tx.PacketsTxed / run_count;
                    transmit_iperf_res_by_port(nx_iperf_ctrl_info_tx.urc_port, 
                                               0, "UDP-UL", run_count,
                                               nx_iperf_ctrl_info_tx.PacketsTxed,
                                               packet_speed, total_MBytes, total_KBytes,
                                               nx_iperf_ctrl_info_tx.ThroughPut);
                    aos_sleep(1000);
                }
            } while (!nx_iperf_ctrl_info_tx.ErrorCode);

            nx_iperf_udp_tx_cleanup();
            aic_transmit_at_result(nx_iperf_ctrl_info_tx.urc_port,
                                   "----Iperf-UDP-UL-Test-End----", 29);
        }

        if (events & AIC_IPERF_MULTI_UDP_TX) {
            for (i = 0; i < nx_iperf_ctrl_info_tx.thread_count; i++) {
                nx_iperf_udp_multi_tx_test(iperf_stack_areas[i].iperf_tx_stack_area,
                                           IPERF_STACK_SIZE, i);
            }

            while (!iperf_ctrl_info_tx_list[0].ErrorCode) {
                aos_sleep(1000);
            }

            for (i = 0; i < nx_iperf_ctrl_info_tx.thread_count; i++) {
                if (iperf_ctrl_info_tx_list[i].ErrorCode) {
                    nx_iperf_udp_multi_tx_cleanup(i);
                }
            }
            aic_transmit_at_result(nx_iperf_ctrl_info_tx.urc_port,
                                   "----Iperf-UDP-UL-Test-End----", 29);
        }

        if (events & AIC_IPERF_MULTI_TCP_TX) {
            for (i = 0; i < nx_iperf_ctrl_info_tx.thread_count; i++) {
                nx_iperf_tcp_multi_tx_test(iperf_stack_areas[i].iperf_tx_stack_area,
                                           IPERF_STACK_SIZE, i);
            }
            while (!iperf_ctrl_info_tx_list[0].ErrorCode) {
                aos_sleep(1000);
            }

            for (i = 0; i < nx_iperf_ctrl_info_tx.thread_count; i++) {
                if (iperf_ctrl_info_tx_list[i].ErrorCode) {
                    nx_iperf_tcp_multi_tx_cleanup(i);
                }
            }

            aic_transmit_at_result(nx_iperf_ctrl_info_tx.urc_port,
                                   "----Iperf-TCP-UL-Test-End----", 29);
        }
    }
}

/**
 * Iperf tcp tx test.
 * For example: AT+IPERTCPTX=13,"192.168.0.17",5001
 */
u32 aic_handle_iperf_tcptx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u8 host[80] = {'\0'};
    u32 total_MBytes = 0;
    u32 run_count = 0;
    u64 pre_run_time = 0;
    u32 packet_speed = 0;
    u32 misstake_bytes = 0;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFTCPTX=",
                                        &count);
    if (!para || count != 3) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_tx_stack_area) {
        transmit_iperf_at_cme(at_ptr, 2);
        aic_free_paras_mem(para, count);
        return 1;
    }

    nx_iperf_ctrl_info_tx.TestTime = NX_IP_PERIODIC_RATE * atoi((u8 *)para[0]);
    strncpy(host, (u8 *)para[1], 80);
    iperf_host_process(host);

    if (atoi((u8 *)para[2]) != 0)
        nx_iperf_ctrl_info_tx.port = atoi((u8 *)para[2]);
    else
        nx_iperf_ctrl_info_tx.port = NX_IPERF_DESTINATION_PORT;

    nx_iperf_tcp_tx_test((u8 *)iperf_tx_stack_area, IPERF_STACK_SIZE);
    do {
        while (nx_iperf_ctrl_info_tx.RunTime < 1000) {
            aos_sleep(100);
        }

        if (nx_iperf_ctrl_info_tx.RunTime >= 1000 &&
            nx_iperf_ctrl_info_tx.RunTime != pre_run_time) {
            pre_run_time = nx_iperf_ctrl_info_tx.RunTime;
            misstake_bytes = nx_iperf_ctrl_info_tx.BytesTxed % (1024 * 1024);
            total_MBytes = nx_iperf_ctrl_info_tx.BytesTxed / 1024 / 1024;
            if (misstake_bytes / 1024 >= 512)
                total_MBytes += 1;

            nx_iperf_ctrl_info_tx.ThroughPut = nx_iperf_ctrl_info_tx.BytesTxed /
                                            (nx_iperf_ctrl_info_tx.RunTime / 1000) /
                                            125000;
            run_count++;
            packet_speed = nx_iperf_ctrl_info_tx.PacketsTxed / run_count;
            transmit_iperf_res(at_ptr, 0, "TCP-UL", run_count,
                               nx_iperf_ctrl_info_tx.PacketsTxed,
                               packet_speed, total_MBytes,
                               nx_iperf_ctrl_info_tx.ThroughPut);
            aos_sleep(1000);
        }
    } while (!nx_iperf_ctrl_info_tx.ErrorCode);

    nx_iperf_tcp_tx_cleanup();

    transmit_iperf_at_ok(at_ptr);
    aic_free_paras_mem(para, count);

    return AIC_SUCCESS;
}

/**
 * Iperf tcp rx test.
 * For example: AT+IPERFTCPRX=1001
 */
u32 aic_handle_iperf_tcprx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u32 total_MBytes = 0;
    u32 run_count = 0;
    u64 pre_run_time = 0;
    u32 packet_speed = 0;
    u32 misstake_bytes = 0;
    ULONG64 prev_BytesRxed = 0;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFTCPRX=",
                                        &count);
    if (!para || count != 1) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_rx_stack_area) {
        aic_free_paras_mem(para, count);
        transmit_iperf_at_cme(at_ptr, 2);
        return 1;
    }

    if (atoi((u8 *)para[0]) != 0)
        nx_iperf_ctrl_info_rx.tcp_rx_port = atoi((u8 *)para[0]);
    else
        nx_iperf_ctrl_info_rx.tcp_rx_port = NX_IPERF_TCP_RX_PORT;

    nx_iperf_ctrl_info_rx.TestTime = NX_IP_PERIODIC_RATE * 24 * 60 * 60;
    nx_iperf_tcp_rx_test((u8 *)iperf_rx_stack_area, IPERF_STACK_SIZE);

    do {
        while (nx_iperf_ctrl_info_rx.RunTime < 1000) {
            aos_sleep(100);
        }

        if (nx_iperf_ctrl_info_rx.RunTime >= 1000 &&
            nx_iperf_ctrl_info_rx.RunTime != pre_run_time) {
            misstake_bytes = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) % (1024 * 1024);
            total_MBytes = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) / 1024 / 1024;
            if (misstake_bytes / 1024 >= 512)
                total_MBytes += 1;

            nx_iperf_ctrl_info_rx.ThroughPut = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) /
                                            125000;
            prev_BytesRxed = nx_iperf_ctrl_info_rx.BytesRxed;
            pre_run_time = nx_iperf_ctrl_info_rx.RunTime;
            run_count++;
            packet_speed = nx_iperf_ctrl_info_rx.PacketsRxed / run_count;
            transmit_iperf_res(at_ptr, 0, "TCP-DL", run_count,
                               nx_iperf_ctrl_info_rx.PacketsRxed,
                               packet_speed, total_MBytes,
                               nx_iperf_ctrl_info_rx.ThroughPut);
            aos_sleep(1000);
        }
    } while (!nx_iperf_ctrl_info_rx.ErrorCode);

    if (nx_iperf_ctrl_info_rx.RunTime >= 1000 &&
        nx_iperf_ctrl_info_rx.RunTime != pre_run_time &&
        nx_iperf_ctrl_info_rx.BytesRxed != prev_BytesRxed) {
        misstake_bytes = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) % (1024 * 1024);
        total_MBytes = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) / 1024 / 1024;
        if (misstake_bytes / 1024 >= 512)
            total_MBytes += 1;

        nx_iperf_ctrl_info_rx.ThroughPut = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) /
                                        125000;
        run_count++;
        packet_speed = nx_iperf_ctrl_info_rx.PacketsRxed / run_count;
        transmit_iperf_res(at_ptr, 0, "TCP-DL", run_count,
                           nx_iperf_ctrl_info_rx.PacketsRxed,
                           packet_speed, total_MBytes,
                           nx_iperf_ctrl_info_rx.ThroughPut);
    }

    memset(&nx_iperf_ctrl_info_rx, 0, sizeof(nx_iperf_ctrl_info_rx));
    nx_iperf_tcp_rx_cleanup();
    aic_free_paras_mem(para, count);
    transmit_iperf_at_ok(at_ptr);

    return AIC_SUCCESS;
}

/**
 * Iperf udp tx test.
 * For example: AT+IPERUDPTX=13,"192.168.0.17",5001
 */
u32 aic_handle_iperf_udptx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u8 host[80] = {'\0'};
    u32 total_MBytes = 0;
    u32 run_count = 0;
    u64 pre_run_time = 0;
    u32 packet_speed = 0;
    u32 misstake_bytes = 0;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFUDPTX=",
                                        &count);
    if (!para || count != 3) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_tx_stack_area) {
        transmit_iperf_at_cme(at_ptr, 2);
        aic_free_paras_mem(para, count);
        return 1;
    }

    nx_iperf_ctrl_info_tx.TestTime = NX_IP_PERIODIC_RATE * atoi((u8 *)para[0]);
    strncpy(host, (u8 *)para[1], 80);
    iperf_host_process(host);

    if (atoi((u8 *)para[2]) != 0)
        nx_iperf_ctrl_info_tx.port = atoi((u8 *)para[2]);
    else
        nx_iperf_ctrl_info_tx.port = NX_IPERF_DESTINATION_PORT;

    if (nx_iperf_ctrl_info_tx.version == NX_IP_VERSION_V4)
        nx_iperf_ctrl_info_tx.PacketSize = 1470;
    else
        nx_iperf_ctrl_info_tx.PacketSize = 1450;

    nx_iperf_udp_tx_test((u8 *)iperf_tx_stack_area, IPERF_STACK_SIZE);
    do {
        while (nx_iperf_ctrl_info_tx.RunTime < 1000) {
            aos_sleep(100);
        }

        if (nx_iperf_ctrl_info_tx.RunTime >= 1000 &&
            nx_iperf_ctrl_info_tx.RunTime != pre_run_time) {
            pre_run_time = nx_iperf_ctrl_info_tx.RunTime;
            misstake_bytes = nx_iperf_ctrl_info_tx.BytesTxed % (1024 * 1024);
            total_MBytes = nx_iperf_ctrl_info_tx.BytesTxed / 1024 / 1024;
            if (misstake_bytes / 1024 >= 512)
                total_MBytes += 1;

            nx_iperf_ctrl_info_tx.ThroughPut = nx_iperf_ctrl_info_tx.BytesTxed /
                                            (nx_iperf_ctrl_info_tx.RunTime / 1000) /
                                            125000;
            run_count++;
            packet_speed = nx_iperf_ctrl_info_tx.PacketsTxed / run_count;
            transmit_iperf_res(at_ptr, 0, "UDP-UL", run_count,
                               nx_iperf_ctrl_info_tx.PacketsTxed,
                               packet_speed, total_MBytes,
                               nx_iperf_ctrl_info_tx.ThroughPut);
            aos_sleep(1000);
        }
    } while (!nx_iperf_ctrl_info_tx.ErrorCode);

    nx_iperf_udp_tx_cleanup();

    transmit_iperf_at_ok(at_ptr);
    aic_free_paras_mem(para, count);

    return AIC_SUCCESS;
}

/**
 * Iperf udp rx test.
 * For example: AT+IPERFUDPRX=1001
 */
u32 aic_handle_iperf_udprx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u32 total_MBytes = 0;
    u32 run_count = 0;
    u64 pre_run_time = 0;
    u32 packet_speed = 0;
    u32 misstake_bytes = 0;
    u64 prev_BytesRxed = 0;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFUDPRX=",
                                        &count);
    if (!para || count != 1) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_rx_stack_area) {
        aic_free_paras_mem(para, count);
        transmit_iperf_at_cme(at_ptr, 2);
        return 1;
    }

    if (atoi((u8 *)para[0]) != 0)
        nx_iperf_ctrl_info_rx.udp_rx_port = atoi((u8 *)para[0]);
    else
        nx_iperf_ctrl_info_rx.udp_rx_port = NX_IPERF_UDP_RX_PORT;

    nx_iperf_ctrl_info_rx.TestTime = NX_IP_PERIODIC_RATE * 24 * 60 * 60;
    nx_iperf_udp_rx_test((u8 *)iperf_rx_stack_area, IPERF_STACK_SIZE);

    do {
        while (nx_iperf_ctrl_info_rx.RunTime < 1000) {
            aos_sleep(100);
        }

        if (nx_iperf_ctrl_info_rx.RunTime >= 1000 &&
            nx_iperf_ctrl_info_rx.RunTime != pre_run_time) {
            misstake_bytes = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) % (1024 * 1024);
            total_MBytes = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) / 1024 / 1024;
            if (misstake_bytes / 1024 >= 512)
                total_MBytes += 1;

            nx_iperf_ctrl_info_rx.ThroughPut = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) /
                                            125000;
            pre_run_time = nx_iperf_ctrl_info_rx.RunTime;
            prev_BytesRxed = nx_iperf_ctrl_info_rx.BytesRxed;
            run_count++;
            packet_speed = nx_iperf_ctrl_info_rx.PacketsRxed / run_count;
            transmit_iperf_res(at_ptr, 0, "UDP-DL", run_count,
                               nx_iperf_ctrl_info_rx.PacketsRxed,
                               packet_speed, total_MBytes,
                               nx_iperf_ctrl_info_rx.ThroughPut);
            aos_sleep(1000);
        }
    } while (!nx_iperf_ctrl_info_rx.ErrorCode);

    if (nx_iperf_ctrl_info_rx.RunTime >= 1000 &&
        nx_iperf_ctrl_info_rx.RunTime != pre_run_time &&
        prev_BytesRxed != nx_iperf_ctrl_info_rx.BytesRxed) {
        misstake_bytes = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) % (1024 * 1024);
        total_MBytes = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) / 1024 / 1024;
        if (misstake_bytes / 1024 >= 512)
            total_MBytes += 1;

        nx_iperf_ctrl_info_rx.ThroughPut = (nx_iperf_ctrl_info_rx.BytesRxed - prev_BytesRxed) /
                                        125000;
        run_count++;
        packet_speed = nx_iperf_ctrl_info_rx.PacketsRxed / run_count;
        transmit_iperf_res(at_ptr, 0, "UDP-DL", run_count,
                           nx_iperf_ctrl_info_rx.PacketsRxed,
                           packet_speed, total_MBytes,
                           nx_iperf_ctrl_info_rx.ThroughPut);
    }

    memset(&nx_iperf_ctrl_info_rx, 0, sizeof(nx_iperf_ctrl_info_rx));
    nx_iperf_udp_rx_cleanup();
    aic_free_paras_mem(para, count);
    transmit_iperf_at_ok(at_ptr);

    return AIC_SUCCESS;
}


/**
 * Iperf udp rx and tx test at sime time.
 * For example: AT+IPERFUDPRTX=1001,13,"192.168.0.17",5001
 */
u32 aic_handle_iperf_udprtx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u8 host[80] = {'\0'};

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFUDPRTX=",
                                        &count);
    if (!para || count != 4) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_rx_stack_area ||
        !iperf_tx_stack_area) {
        aic_free_paras_mem(para, count);
        transmit_iperf_at_cme(at_ptr, 2);
        return 1;
    }

    nx_iperf_ctrl_info_rx.urc_port = at_ptr->serial_port_id;
    nx_iperf_ctrl_info_tx.urc_port = at_ptr->serial_port_id;

    /* set rx test paras */
    if (atoi((u8 *)para[0]) != 0)
        nx_iperf_ctrl_info_rx.udp_rx_port = atoi((u8 *)para[0]);
    else
        nx_iperf_ctrl_info_rx.udp_rx_port = NX_IPERF_UDP_RX_PORT;

    nx_iperf_ctrl_info_rx.TestTime = NX_IP_PERIODIC_RATE * 24 * 60 * 60;

    /* set tx test paras */
    nx_iperf_ctrl_info_tx.TestTime = NX_IP_PERIODIC_RATE * atoi((u8 *)para[1]);
    strncpy(host, (u8 *)para[2], 80);
    iperf_host_process(host);

    if (atoi((u8 *)para[3]) != 0)
        nx_iperf_ctrl_info_tx.port = atoi((u8 *)para[3]);
    else
        nx_iperf_ctrl_info_tx.port = NX_IPERF_DESTINATION_PORT;

    if (nx_iperf_ctrl_info_tx.version == NX_IP_VERSION_V4)
        nx_iperf_ctrl_info_tx.PacketSize = 1470;
    else
        nx_iperf_ctrl_info_tx.PacketSize = 1450;

    aos_set_event(aic_iperf_event, AIC_IPERF_UDP_RX, AOS_OR);
    aos_set_event(aic_iperf_event, AIC_IPERF_UDP_TX, AOS_OR);
    aic_free_paras_mem(para, count);
    transmit_iperf_at_ok(at_ptr);
    return AIC_SUCCESS;

}


/**
 * Iperf multi udp tx test.
 * For example: AT+IPERFMUDPTX=13,"192.168.0.17",5001,10
 */
u32 aic_handle_multi_iperf_udptx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u8 host[80] = {'\0'};
    u32 i;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFMUDPTX=",
                                        &count);
    if (!para || count != 4) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_tx_stack_area) {
        transmit_iperf_at_cme(at_ptr, 2);
        aic_free_paras_mem(para, count);
        return 1;
    }

    nx_iperf_ctrl_info_tx.TestTime = NX_IP_PERIODIC_RATE * atoi((u8 *)para[0]);
    strncpy(host, (u8 *)para[1], 80);
    iperf_host_process(host);

    if (atoi((u8 *)para[2]) != 0)
        nx_iperf_ctrl_info_tx.port = atoi((u8 *)para[2]);
    else
        nx_iperf_ctrl_info_tx.port = NX_IPERF_DESTINATION_PORT;

    if (nx_iperf_ctrl_info_tx.version == NX_IP_VERSION_V4)
        nx_iperf_ctrl_info_tx.PacketSize = 1470;
    else
        nx_iperf_ctrl_info_tx.PacketSize = 1450;

    nx_iperf_ctrl_info_tx.thread_count = atoi((u8 *)para[3]);
    nx_iperf_ctrl_info_tx.urc_port = at_ptr->serial_port_id;

    for (i = 0; i < nx_iperf_ctrl_info_tx.thread_count; i++) {
        if (iperf_stack_areas[i].iperf_tx_stack_area == AIC_NULL)
            iperf_stack_areas[i].iperf_tx_stack_area = aos_zalloc(IPERF_STACK_SIZE);

        if (iperf_sockets[i].udp_client_socket == AIC_NULL)
            iperf_sockets[i].udp_client_socket = aos_zalloc(sizeof(NX_UDP_SOCKET));

        iperf_ctrl_info_tx_list[i].TestTime = nx_iperf_ctrl_info_tx.TestTime;
        iperf_ctrl_info_tx_list[i].port = nx_iperf_ctrl_info_tx.port;
        iperf_ctrl_info_tx_list[i].version = nx_iperf_ctrl_info_tx.version;
        iperf_ctrl_info_tx_list[i].PacketSize = nx_iperf_ctrl_info_tx.PacketSize;
        iperf_ctrl_info_tx_list[i].urc_port = nx_iperf_ctrl_info_tx.urc_port;
        iperf_ctrl_info_tx_list[i].thread_count = nx_iperf_ctrl_info_tx.thread_count;

        if (iperf_ctrl_info_tx_list[i].version == NX_IP_VERSION_V4)
            iperf_ctrl_info_tx_list[i].ip = nx_iperf_ctrl_info_tx.ip;
        else if (iperf_ctrl_info_tx_list[i].version == NX_IP_VERSION_V6) {
            iperf_ctrl_info_tx_list[i].ipv6[0] = nx_iperf_ctrl_info_tx.ipv6[0];
            iperf_ctrl_info_tx_list[i].ipv6[1] = nx_iperf_ctrl_info_tx.ipv6[1];
            iperf_ctrl_info_tx_list[i].ipv6[2] = nx_iperf_ctrl_info_tx.ipv6[2];
            iperf_ctrl_info_tx_list[i].ipv6[3] = nx_iperf_ctrl_info_tx.ipv6[3];
        }
    }

    aos_set_event(aic_iperf_event, AIC_IPERF_MULTI_UDP_TX, AOS_OR);
    transmit_iperf_at_ok(at_ptr);
    aic_free_paras_mem(para, count);

    return AIC_SUCCESS;
}



/**
 * Iperf multi udp rx test.
 * For example: AT+IPERFMUDPRX=5001,10
 */
u32 aic_handle_multi_iperf_udprx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    //u32 i;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFMUDPRX=",
                                        &count);
    if (!para || count != 2) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_tx_stack_area) {
        transmit_iperf_at_cme(at_ptr, 2);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (atoi((u8 *)para[0]) != 0)
        nx_iperf_ctrl_info_rx.udp_rx_port = atoi((u8 *)para[0]);
    else
        nx_iperf_ctrl_info_rx.udp_rx_port = NX_IPERF_UDP_RX_PORT;

    nx_iperf_ctrl_info_rx.TestTime = NX_IP_PERIODIC_RATE * 24 * 60 * 60;
    nx_iperf_ctrl_info_rx.thread_count = atoi((u8 *)para[1]);
    nx_iperf_ctrl_info_rx.urc_port = at_ptr->serial_port_id;


    aos_set_event(aic_iperf_event, AIC_IPERF_MULTI_UDP_RX, AOS_OR);
    transmit_iperf_at_ok(at_ptr);
    aic_free_paras_mem(para, count);

    return AIC_SUCCESS;
}

/**
 * Iperf multi udp rx and tx test at sime time.
 * For example: AT+IPERFMUDPRTX=1001,13,"192.168.0.17",5001,10
 */
u32 aic_handle_multi_iperf_udprtx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u8 host[80] = {'\0'};
    u32 i;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFMUDPRTX=",
                                        &count);
    if (!para || count != 5) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_rx_stack_area ||
        !iperf_tx_stack_area) {
        aic_free_paras_mem(para, count);
        transmit_iperf_at_cme(at_ptr, 2);
        return 1;
    }

    nx_iperf_ctrl_info_rx.urc_port = at_ptr->serial_port_id;
    nx_iperf_ctrl_info_tx.urc_port = at_ptr->serial_port_id;

    /* set rx test paras */
    if (atoi((u8 *)para[0]) != 0)
        nx_iperf_ctrl_info_rx.udp_rx_port = atoi((u8 *)para[0]);
    else
        nx_iperf_ctrl_info_rx.udp_rx_port = NX_IPERF_UDP_RX_PORT;

    nx_iperf_ctrl_info_rx.TestTime = NX_IP_PERIODIC_RATE * 24 * 60 * 60;
    nx_iperf_ctrl_info_rx.thread_count = atoi((u8 *)para[4]);

    /* set tx test paras */
    nx_iperf_ctrl_info_tx.TestTime = NX_IP_PERIODIC_RATE * atoi((u8 *)para[1]);
    strncpy(host, (u8 *)para[2], 80);
    iperf_host_process(host);

    if (atoi((u8 *)para[3]) != 0)
        nx_iperf_ctrl_info_tx.port = atoi((u8 *)para[3]);
    else
        nx_iperf_ctrl_info_tx.port = NX_IPERF_DESTINATION_PORT;

    if (nx_iperf_ctrl_info_tx.version == NX_IP_VERSION_V4)
        nx_iperf_ctrl_info_tx.PacketSize = 1470;
    else
        nx_iperf_ctrl_info_tx.PacketSize = 1450;

    nx_iperf_ctrl_info_tx.thread_count = atoi((u8 *)para[4]);

    for (i = 0; i < nx_iperf_ctrl_info_tx.thread_count; i++) {
        if (iperf_stack_areas[i].iperf_tx_stack_area == AIC_NULL)
            iperf_stack_areas[i].iperf_tx_stack_area = aos_zalloc(IPERF_STACK_SIZE);

        if (iperf_sockets[i].udp_client_socket == AIC_NULL)
            iperf_sockets[i].udp_client_socket = aos_zalloc(sizeof(NX_UDP_SOCKET));

        iperf_ctrl_info_tx_list[i].TestTime = nx_iperf_ctrl_info_tx.TestTime;
        iperf_ctrl_info_tx_list[i].port = nx_iperf_ctrl_info_tx.port;
        iperf_ctrl_info_tx_list[i].version = nx_iperf_ctrl_info_tx.version;
        iperf_ctrl_info_tx_list[i].PacketSize = nx_iperf_ctrl_info_tx.PacketSize;
        iperf_ctrl_info_tx_list[i].urc_port = nx_iperf_ctrl_info_tx.urc_port;
        iperf_ctrl_info_tx_list[i].thread_count = nx_iperf_ctrl_info_tx.thread_count;

        if (iperf_ctrl_info_tx_list[i].version == NX_IP_VERSION_V4)
            iperf_ctrl_info_tx_list[i].ip = nx_iperf_ctrl_info_tx.ip;
        else if (iperf_ctrl_info_tx_list[i].version == NX_IP_VERSION_V6) {
            iperf_ctrl_info_tx_list[i].ipv6[0] = nx_iperf_ctrl_info_tx.ipv6[0];
            iperf_ctrl_info_tx_list[i].ipv6[1] = nx_iperf_ctrl_info_tx.ipv6[1];
            iperf_ctrl_info_tx_list[i].ipv6[2] = nx_iperf_ctrl_info_tx.ipv6[2];
            iperf_ctrl_info_tx_list[i].ipv6[3] = nx_iperf_ctrl_info_tx.ipv6[3];
        }
    }

    aos_set_event(aic_iperf_event, AIC_IPERF_MULTI_UDP_RX, AOS_OR);
    aos_set_event(aic_iperf_event, AIC_IPERF_MULTI_UDP_TX, AOS_OR);
    aic_free_paras_mem(para, count);
    transmit_iperf_at_ok(at_ptr);
    return AIC_SUCCESS;

}

/**
 * Iperf multi udp tx test.
 * For example: AT+IPERFMTCPTX=13,"192.168.0.17",5001,10
 */
u32 aic_handle_multi_iperf_tcptx_exe_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *para;
    u16 count;
    u8 host[80] = {'\0'};
    u32 i;

    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);

    para = aic_split_paras_to_str_array(at_ptr->at_req, "AT+IPERFMTCPTX=",
                                        &count);
    if (!para || count != 4) {
        transmit_iperf_at_cme(at_ptr, 1);
        aic_free_paras_mem(para, count);
        return 1;
    }

    if (!iperf_pool.packet_pool || !iperf_ip_ptr || !iperf_tx_stack_area) {
        transmit_iperf_at_cme(at_ptr, 2);
        aic_free_paras_mem(para, count);
        return 1;
    }

    nx_iperf_ctrl_info_tx.TestTime = NX_IP_PERIODIC_RATE * atoi((u8 *)para[0]);
    strncpy(host, (u8 *)para[1], 80);
    iperf_host_process(host);

    if (atoi((u8 *)para[2]) != 0)
        nx_iperf_ctrl_info_tx.port = atoi((u8 *)para[2]);
    else
        nx_iperf_ctrl_info_tx.port = NX_IPERF_DESTINATION_PORT;

    nx_iperf_ctrl_info_tx.thread_count = atoi((u8 *)para[3]);
    nx_iperf_ctrl_info_tx.urc_port = at_ptr->serial_port_id;

    for (i = 0; i < nx_iperf_ctrl_info_tx.thread_count; i++) {
        if (iperf_stack_areas[i].iperf_tx_stack_area == AIC_NULL)
            iperf_stack_areas[i].iperf_tx_stack_area = aos_zalloc(IPERF_STACK_SIZE);

        if (iperf_sockets[i].tcp_client_socket == AIC_NULL)
            iperf_sockets[i].tcp_client_socket = aos_zalloc(sizeof(NX_TCP_SOCKET));

        iperf_ctrl_info_tx_list[i].TestTime = nx_iperf_ctrl_info_tx.TestTime;
        iperf_ctrl_info_tx_list[i].port = nx_iperf_ctrl_info_tx.port;
        iperf_ctrl_info_tx_list[i].version = nx_iperf_ctrl_info_tx.version;
        iperf_ctrl_info_tx_list[i].PacketSize = nx_iperf_ctrl_info_tx.PacketSize;
        iperf_ctrl_info_tx_list[i].urc_port = nx_iperf_ctrl_info_tx.urc_port;
        iperf_ctrl_info_tx_list[i].thread_count = nx_iperf_ctrl_info_tx.thread_count;

        if (iperf_ctrl_info_tx_list[i].version == NX_IP_VERSION_V4)
            iperf_ctrl_info_tx_list[i].ip = nx_iperf_ctrl_info_tx.ip;
        else if (iperf_ctrl_info_tx_list[i].version == NX_IP_VERSION_V6) {
            iperf_ctrl_info_tx_list[i].ipv6[0] = nx_iperf_ctrl_info_tx.ipv6[0];
            iperf_ctrl_info_tx_list[i].ipv6[1] = nx_iperf_ctrl_info_tx.ipv6[1];
            iperf_ctrl_info_tx_list[i].ipv6[2] = nx_iperf_ctrl_info_tx.ipv6[2];
            iperf_ctrl_info_tx_list[i].ipv6[3] = nx_iperf_ctrl_info_tx.ipv6[3];
        }
    }

    aos_set_event(aic_iperf_event, AIC_IPERF_MULTI_TCP_TX, AOS_OR);

    transmit_iperf_at_ok(at_ptr);
    aic_free_paras_mem(para, count);

    return AIC_SUCCESS;
}


/**
 * Execution command terminate the actived iperf service.
 * For example: AT+IPERFTERM
 */
u32 aic_handle_iperf_term_exe_cmd(AT_REQ_RESP *at_ptr)
{
    u32 status;
    aicx_info(0x14033e, "s", "at cmd is--%s\n", at_ptr->at_req);
    u32 i;

    if (at_ptr->at_req[strlen("AT+IPERFTERM")] != 0x0D &&
        at_ptr->at_req[strlen("AT+IPERFTERM")] != 0x0A) {
        transmit_iperf_at_cme(at_ptr, 1);
        return 1;
    }

    if (tcp_server_socket.nx_tcp_socket_id == NX_TCP_ID &&
        tcp_server_socket.nx_tcp_socket_ip_ptr->nx_ip_tcp_packet_receive)
        nx_iperf_tcp_rx_cleanup();

    if (tcp_client_socket.nx_tcp_socket_id == NX_TCP_ID &&
        tcp_client_socket.nx_tcp_socket_ip_ptr->nx_ip_tcp_packet_receive)
        nx_iperf_tcp_tx_cleanup();

    if (udp_client_socket.nx_udp_socket_id == NX_UDP_ID &&
        udp_client_socket.nx_udp_socket_ip_ptr->nx_ip_udp_packet_receive)
        nx_iperf_udp_tx_cleanup();

    if (udp_server_socket.nx_udp_socket_id == NX_UDP_ID &&
        udp_server_socket.nx_udp_socket_ip_ptr->nx_ip_udp_packet_receive)
        nx_iperf_udp_rx_cleanup();

    if (iperf_rx_stack_area) {
        aos_free(iperf_rx_stack_area);
        iperf_rx_stack_area = AIC_NULL;
    }

    if (iperf_tx_stack_area) {
        aos_free(iperf_tx_stack_area);
        iperf_tx_stack_area = AIC_NULL;
    }

    for (i = 0; i < IPERF_MAX_THREAD_NUM; i++) {
        if (iperf_stack_areas[i].iperf_tx_stack_area) {
            aos_free(iperf_stack_areas[i].iperf_tx_stack_area);
            iperf_stack_areas[i].iperf_tx_stack_area = AIC_NULL;
        }

        if (iperf_sockets[i].udp_client_socket) {
            aos_free(iperf_sockets[i].udp_client_socket);
            iperf_sockets[i].udp_client_socket = AIC_NULL;
        }

        if (iperf_sockets[i].tcp_client_socket) {
            aos_free(iperf_sockets[i].tcp_client_socket);
            iperf_sockets[i].tcp_client_socket = AIC_NULL;
        }
    }

    status = aic_delete_packet_pool(&iperf_pool);
    if (!status)
        aos_memset(&iperf_pool, 0, sizeof(struct aic_packet_pool));

    iperf_ip_ptr = AIC_NULL;

    transmit_iperf_at_ok(at_ptr);

    return AIC_SUCCESS;
}

int aic_iperf_service_init(void)
{
    thread_iperf_rx_process = aos_create_thread(
                                  "thread_iperf_event_process",
                                  iperf_rx_at_entry, NULL, SIZE_2K, 0, NULL,
                                  15, AOS_AUTO_START);

    if (thread_iperf_rx_process == AOS_INVALID_THREAD_ID) {
        aicx_error(0x173b92, "", "iperf udp rx process thread create fail\n");
        return 1;
    }

    thread_iperf_tx_process = aos_create_thread("thread_iperf_event_process",
                                  iperf_tx_at_entry, NULL, SIZE_2K, 0, NULL,
                                  15, AOS_AUTO_START);

    if (thread_iperf_tx_process == AOS_INVALID_THREAD_ID) {
        aicx_error(0x005d79, "", "iperf udp tx process thread create fail\n");
        return 1;
    }

    aic_iperf_event = aos_create_event("Iperf AT Events");
    if (aic_iperf_event == AOS_NULL) {
        aicx_error(0x007dbe, "", "iperf at event create error\n");
        aos_delete_thread(thread_iperf_rx_process);
        aos_delete_thread(thread_iperf_tx_process);
        return 1;
    }

    aic_add_at_cmd_handle(&iperf_at_handles[0]);

    return AIC_SUCCESS;
}

DIS_CHARGE_APP_P2_INIT_EXPORT(aic_iperf_service_init);
