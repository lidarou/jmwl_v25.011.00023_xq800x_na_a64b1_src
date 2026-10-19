/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Network Component
 *
 *  Simple Network Time Protocol(sntp)
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a6e94
#define LOG_MODULE    AMOD_NET

#include <stdio.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <nxd_bsd.h>
#include "nxd_sntp_client.h"
#include "nx_api.h"
#include "nx_packet.h"
#include "nx_ipv4.h"
#include "nx_ipv6.h"
#include "nx_ip.h"
#include "aic_net_api.h"
#include "aic_net_pdp_mgr.h"
#include "aic_at_parse_utils.h"
#include "aic_general_at_handle.h"
#include "aic_at_urc_handle.h"
#ifdef CONFIG_NET_PCAP_DUMP
#include "aic_network_pcap.h"
#endif
#ifdef CONFIG_CUSTOMER_AT
#include "aic_at_qtcpip.h"
#include <aic_at_process_common.h>
#endif
#if defined CONFIG_CM_TCPIP_AT || defined CONFIG_GX_TCPIP_AT
#include "aic_at_mtcpip.h"
#include "aic_at_tcpip_common.h"
#include <aic_at_process_common.h>
#endif


#define AIC_CURRENT_YEAR     2022

/* Define the number of seconds from 01-01-1970 00:00:00 to
   01-01-2022 00:00:00 */
#define AIC_LOSS_OF_UTC_TIME 0x61CF9980

/* the number of seconds from 01-01-1900 00:00:00 to 01-01-1970 00:00:00 */
#define AIC_SNTP_TO_UTC_DIFF 0x83AA7E80
/* Define SNTP task size*/
//#define NX_SNTP_CLIENT_TASK_SIZE  2048
#define NX_SNTP_CLIENT_STACK_PRIORITY  15

/* Define how often the demo checks for SNTP updates. */
//#define SNTP_PERIODIC_CHECK_INTERVAL  (1 * NX_IP_PERIODIC_RATE)

/*
 * Define how often we check on SNTP server status. We expect updates from the
 * SNTP server about every hour using the SNTP Client defaults. For testing
 * make this (much) shorter.
 */
//#define CHECK_SNTP_UPDATES_TIMEOUT  (1800 * NX_IP_PERIODIC_RATE)

#define SNTP_ALL_EVENT       0xFF
#define SNTP_UPDATE_EVENT    0x1
#define SNTP_AT_UPDATE_EVENT 0x2
#define SNTP_AT_SEND_EVENT   0x4

#define USE_UNICAST

#define SNTP_TIME_SIZE 128
#define SNTP_TIMEOUT_CM_AT 2
#define SNTP_TIMEOUT_AT  550

#if !defined(CONFIG_CUSTOMER_CM_AT) && !defined(NX_SNTP_DISABLE_AUTO_START)
#define SNTP_URL_MAX 4
#define SNTP_URL_LEN_MAX 64
static u8 url[SNTP_URL_MAX][SNTP_URL_LEN_MAX] = {
    {"cn.ntp.org.cn"},
    {"time.windows.com"},
    {"49.7.229.32"},
    {"2001:da8:9000::81"}
};
#endif
static u32 origin_time = 0;
int sntp_is_sync = AIC_TRUE;
int sntp_sync_timeout = 20;

typedef void (*CUSTOM_NW_EVT_CB)(u32 eventId, void* pData, u32 len);
extern CUSTOM_NW_EVT_CB custom_nw_evt_cb;

/* Set up client thread and network resources. */
aos_thread_id       aic_sntp_client_thread = AOS_INVALID_THREAD_ID;
NX_SNTP_CLIENT      *aic_sntp_client;
aos_event_ptr       aic_sntp_client_event;

u32     sntp_server_ip = 0;
NXD_ADDRESS sntp_server_address = {0};
extern TX_EVENT_FLAGS_GROUP nx_sntp_client_events;
extern AIC_SNTP_SERVER_IP *aic_sntp_server_ip_ptr;

extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];

/* Set up generic network driver for demo program. */
//void    _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

/* Application defined services of the NetX SNTP Client. */
static u32 leap_second_handler(NX_SNTP_CLIENT *client_ptr, u32 leap_indicator);
static u32 kiss_of_death_handler(NX_SNTP_CLIENT *client_ptr, u32 KOD_code);
static VOID time_update_callback(NX_SNTP_TIME_MESSAGE *time_update_ptr,
                                    NX_SNTP_TIME *local_time);
//static void aic_sntp_delete(void);
/* Set up client thread entry point. */
static void aic_sntp_client_thread_entry(void *thread_input);
static u32 aic_sntp_client_set_time_to_system(NX_SNTP_CLIENT *client_ptr);

void aic_sntp_pdp_state_change_callback(u8 pdp_cid, u8 pdp_state);
void aic_sntp_add_server_to_list(AIC_SNTP_SERVER_IP *aic_sntp_server);
void aic_sntp_del_all_server_from_list(void);
NX_SNTP_CLIENT *aic_get_sntp_client(void);
ulong aic_get_utc_time(void);

#if !defined(CONFIG_CUSTOMER_CM_AT) && !defined(NX_SNTP_DISABLE_AUTO_START)
int aic_sntp_client_init(void)
{
    aic_register_pdp_state_change_cb(aic_sntp_pdp_state_change_callback);
    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_sntp_client_init);
#endif

void aic_sntp_pdp_state_change_callback(u8 pdp_cid, u8 pdp_state)
{
    u32  status = 0, iface_index = 0;
    NX_IP *client_ip;
    u8 index, mode = 0;
    struct aic_network_ip_instance *ip_instance;

    if(pdp_cid != ANET_DEFAULT_PDP_CID || pdp_state != 1) {
        return;
    }

    if (aic_boot_mode_is_basic())
        return;

    mode = aic_get_reg_mode();
    if (mode == 1)
        return;

    if (aic_sntp_client_thread != AOS_INVALID_THREAD_ID) {
        aos_resume_thread(aic_sntp_client_thread);
        return;
    }

    /* Create the client thread */
    aic_sntp_client_thread = aos_create_thread("SNTP Client Thread",
                              aic_sntp_client_thread_entry,
                              (&aic_sntp_client), (SIZE_1K + SIZE_512B),
                              0, 0, NX_SNTP_CLIENT_STACK_PRIORITY,
                              AOS_DONT_START);
    /* Check for errors */
    if (aic_sntp_client_thread == AOS_INVALID_THREAD_ID) {
        return;
    }

    /* Create the event flags. */
    aic_sntp_client_event = aos_create_event("SNTP");

    /* Check for errors */
    if (!aic_sntp_client_event) {
        aos_delete_thread(aic_sntp_client_thread);
        aic_sntp_client_thread = AOS_INVALID_THREAD_ID;
        return;
    }

    /* set the SNTP network interface to the primary interface. */
    index = aic_get_wan_net_by_cid(ANET_DEFAULT_PDP_CID);
    iface_index = wan_net_list[index].wan_ifc_index;
    ip_instance = aic_get_pdp_ip_instance();
    if (!ip_instance || !ip_instance->ip_ptr) {
        aos_delete_thread(aic_sntp_client_thread);
        aos_delete_event(aic_sntp_client_event);
        aic_sntp_client_thread = AOS_INVALID_THREAD_ID;
        aic_sntp_client_event = NULL;
        /* Bail out!*/
        return;
    }
    client_ip = ip_instance->ip_ptr;

    if (!aic_sntp_client ||
        aic_sntp_client->nx_sntp_client_id != NXD_SNTP_ID) {
        /* Create the SNTP Client to run in broadcast mode.. */
        if (!aic_sntp_client)
            aic_sntp_client = aos_zalloc(sizeof(NX_SNTP_CLIENT));

        status =  nx_sntp_client_create(aic_sntp_client, client_ip,
                                        iface_index,
                                        client_ip->nx_ip_default_packet_pool,
                                        leap_second_handler,
                                        kiss_of_death_handler,
                                        NULL);
        /* Check for error. */
        if (status != NX_SUCCESS) {
            aos_delete_thread(aic_sntp_client_thread);
            aos_delete_event(aic_sntp_client_event);
            aic_sntp_client_thread = AOS_INVALID_THREAD_ID;
            aic_sntp_client_event = NULL;
            /* Bail out!*/
            return;
        }
    }

    aos_resume_thread(aic_sntp_client_thread);
}

void aic_sntp_add_server_to_list(AIC_SNTP_SERVER_IP *aic_sntp_server)
{
    AOS_INTERRUPT_DECLARE
    AIC_SNTP_SERVER_IP *tail_aic_sntp_server;

    AOS_INTERRUPT_DISABLE
    if (!aic_sntp_server_ip_ptr) {
        aic_sntp_server_ip_ptr = aic_sntp_server;
        aic_sntp_server_ip_ptr->aic_sntp_server_addr_next = aic_sntp_server;
        aic_sntp_server_ip_ptr->aic_sntp_server_addr_prev = aic_sntp_server;
    } else {
        tail_aic_sntp_server = aic_sntp_server_ip_ptr->aic_sntp_server_addr_prev;

        aic_sntp_server_ip_ptr->aic_sntp_server_addr_prev = aic_sntp_server;
        tail_aic_sntp_server->aic_sntp_server_addr_next = aic_sntp_server;

        aic_sntp_server->aic_sntp_server_addr_prev = tail_aic_sntp_server;
        aic_sntp_server->aic_sntp_server_addr_next = aic_sntp_server_ip_ptr;
    }
    AOS_INTERRUPT_RESTORE
}

void aic_sntp_del_all_server_from_list(void)
{
    AOS_INTERRUPT_DECLARE
    AIC_SNTP_SERVER_IP *aic_sntp_server;

    AOS_INTERRUPT_DISABLE
    aic_sntp_server = aic_sntp_server_ip_ptr;
    while (aic_sntp_server) {
        if(aic_sntp_server == aic_sntp_server->aic_sntp_server_addr_next) {
            aos_free(aic_sntp_server);
            aic_sntp_server_ip_ptr = AIC_NULL;
        } else {
            (aic_sntp_server->aic_sntp_server_addr_next)->aic_sntp_server_addr_prev =
                aic_sntp_server->aic_sntp_server_addr_prev;
            (aic_sntp_server->aic_sntp_server_addr_prev)->aic_sntp_server_addr_next =
                aic_sntp_server->aic_sntp_server_addr_next;

            if (aic_sntp_server_ip_ptr == aic_sntp_server)
                aic_sntp_server_ip_ptr = aic_sntp_server->aic_sntp_server_addr_next;

            aos_free(aic_sntp_server);
        }

        aic_sntp_server = aic_sntp_server_ip_ptr;
    }
    AOS_INTERRUPT_RESTORE
}

/* Define the client thread.  */
void aic_sntp_client_thread_entry(void* thread_input)
{
    u32 status, spin, server_status;
    ulong base_seconds, base_fraction;
    ulong seconds = 0, milliseconds = 0, microseconds, fraction;
    u32 error_counter = 0, events = 0, timeout = AOS_WAIT_FOREVER;
#if !defined(CONFIG_CUSTOMER_CM_AT) && !defined(NX_SNTP_DISABLE_AUTO_START)
    struct addrinfo *res, *dns_res;
    AIC_SNTP_SERVER_IP *aic_sntp_server;
    u32 i, retry_times = 0;
    static struct addrinfo hints = {0, AF_UNSPEC, 0, IPPROTO_UDP, 0,
                                    NX_NULL, NX_NULL, NX_NULL};
#endif
#if defined CONFIG_CUSTOMER_AT || defined NET_AT_MTCPIP || defined CONFIG_NET_AT_SNTP
    int time_zone, com_id;
    u8 *time_buffer, *str;
    NX_SNTP_DATE_TIME DisplayTime;
    NX_SNTP_TIME sntp_time;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

#if !defined(CONFIG_CUSTOMER_CM_AT) && !defined(NX_SNTP_DISABLE_AUTO_START)
RETRY:
    for (i = 0; i < SNTP_URL_MAX; i++) {
        status = getaddrinfo(url[i], NULL, &hints, &res);
        if (status != NX_SUCCESS || !res) {
            aicx_info(0x035ee1, "dx", "getaddrinfo url %d fail 0x%x\n", i, status);
            if (i == (SNTP_URL_MAX - 1) && !aic_sntp_server_ip_ptr) {
                aos_sleep(20000);
                retry_times++;

                if (retry_times == 3) {
                    aos_suspend_thread(aic_sntp_client_thread);
                    retry_times = 0;
                }

                goto RETRY;
            } else {
                continue;
            }
        }

        dns_res = res;
        while (dns_res) {
            aic_sntp_server = aos_zalloc_relaxed(sizeof(AIC_SNTP_SERVER_IP));
            if (!aic_sntp_server) {
                continue;
            }
            if(dns_res->ai_addr->sa_family == AF_INET) {
                if (sntp_server_ip == 0) {
                    aos_memcpy(&sntp_server_ip,
                               &((struct sockaddr_in *)dns_res->ai_addr)->sin_addr, 4);
                    NX_CHANGE_ULONG_ENDIAN(sntp_server_ip);
                }

                aos_memcpy(&aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v4,
                           &((struct sockaddr_in *)dns_res->ai_addr)->sin_addr, 4);
                NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v4);
                aic_sntp_server->aic_sntp_server_address.nxd_ip_version =
                    NX_IP_VERSION_V4;
            } else {
                if (sntp_server_address.nxd_ip_version != NX_IP_VERSION_V6) {
                    aos_memcpy(sntp_server_address.nxd_ip_address.v6,
                               &((struct sockaddr_in6 *)dns_res->ai_addr)->sin6_addr,
                               16);
                    sntp_server_address.nxd_ip_version = NX_IP_VERSION_V6;
                    NX_CHANGE_ULONG_ENDIAN(sntp_server_address.nxd_ip_address.v6[0]);
                    NX_CHANGE_ULONG_ENDIAN(sntp_server_address.nxd_ip_address.v6[1]);
                    NX_CHANGE_ULONG_ENDIAN(sntp_server_address.nxd_ip_address.v6[2]);
                    NX_CHANGE_ULONG_ENDIAN(sntp_server_address.nxd_ip_address.v6[3]);
                }

                aos_memcpy(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6,
                           &((struct sockaddr_in6 *)dns_res->ai_addr)->sin6_addr, 16);
                NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6[0]);
                NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6[1]);
                NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6[2]);
                NX_CHANGE_ULONG_ENDIAN(aic_sntp_server->aic_sntp_server_address.nxd_ip_address.v6[3]);
                aic_sntp_server->aic_sntp_server_address.nxd_ip_version =
                    NX_IP_VERSION_V6;
            }
            aic_sntp_add_server_to_list(aic_sntp_server);
            dns_res = dns_res ->ai_next;
        }
        freeaddrinfo(res);
    }
#endif

    /* Setup time update callback function. */
    nx_sntp_client_set_time_update_notify(aic_sntp_client,
                                          time_update_callback);

    /* Set up client time updates depending on mode. */
#ifdef USE_UNICAST
    if (sntp_server_address.nxd_ip_version == NX_IP_VERSION_V6)
        status = nxd_sntp_client_initialize_unicast(aic_sntp_client,
                                                    &sntp_server_address);
    else
        status = nx_sntp_client_initialize_unicast(aic_sntp_client,
                                                   sntp_server_ip);

#else   /* Broadcast mode */
#ifdef FEATURE_NX_IPV6
    status = nxd_sntp_client_initialize_broadcast(aic_sntp_client,
                                                  &sntp_server_address,
                                                  NX_NULL);
#else
    status = nx_sntp_client_initialize_broadcast(aic_sntp_client,
                                                 NX_NULL, sntp_server_ip);
#endif  /* FEATURE_NX_IPV6 */
#endif  /* USE_UNICAST */

    /* Check for error. */
    if (status != NX_SUCCESS) {
        return;
    }

    base_seconds =  0xe57a1800;  /* Jan 1, 2022 UTC */
    base_fraction = 0xa132db1e;

    /* Apply to the SNTP Client local time.  */
    status = nx_sntp_client_set_local_time(aic_sntp_client, base_seconds,
                                           base_fraction);

    /* Check for error. */
    if (status != NX_SUCCESS) {
        return;
    }

    /* Run whichever service the client is configured for. */
#ifdef USE_UNICAST
    status = nx_sntp_client_run_unicast(aic_sntp_client);
#else
    status = nx_sntp_client_run_broadcast(aic_sntp_client);
#endif  /* USE_UNICAST */

    if (status != NX_SUCCESS) {
        return;
    }

    spin = NX_TRUE;

    /* Now check periodically for time changes. */
    while(spin) {
        /* Wait for a server update event. */
        status = aos_get_event(aic_sntp_client_event, SNTP_ALL_EVENT, TX_OR_CLEAR,
                               &events, timeout);
        if (status != NX_SUCCESS) {
#if defined CONFIG_CUSTOMER_AT || defined NET_AT_MTCPIP || defined CONFIG_NET_AT_SNTP
            if (aic_sntp_client->nx_sntp_server_update_time_active) {
                aic_sntp_client->nx_sntp_server_update_time_active--;
                if (aic_sntp_client->nx_sntp_server_update_time_active == 0)
                    timeout = AOS_WAIT_FOREVER;
#ifdef CONFIG_CUSTOMER_AT
                str = aos_zalloc(SNTP_TIME_SIZE);
                snprintf(str, SNTP_TIME_SIZE, "+QNTP: %d", SNTP_TIMEOUT_AT);
                extern AT_TCPIP_CFG at_tcpip_cfg;
                com_id = at_tcpip_cfg.com_id;
#endif
#ifdef NET_AT_MTCPIP
                str = aos_zalloc(SNTP_TIME_SIZE);
                snprintf(str, SNTP_TIME_SIZE, RESP_IP_NTP"%d", SNTP_TIMEOUT_CM_AT);
                extern AT_TCPIP_CFG at_tcpip_cfg[AT_SOCKET_MAX_CONN_NUM];
                com_id = at_tcpip_cfg[0].com_id;
#endif
#ifdef CONFIG_NET_AT_SNTP
                str = aos_zalloc(SNTP_TIME_SIZE);
                strncpy(str, "+CIPSNTP: timeout", SNTP_TIME_SIZE);
                extern u32 sntp_urc_comid;
                com_id = sntp_urc_comid;
#endif

                aic_transmit_at_cmd_urc(com_id, str, strlen(str));
                aos_free(str);
            }
#endif
            continue;
        }

        if (events & SNTP_UPDATE_EVENT) {
            /* Check for valid SNTP server status. */
            status = nx_sntp_client_receiving_updates(aic_sntp_client,
                                                      &server_status);
            if ((status != NX_SUCCESS) || (server_status == NX_FALSE)) {
                continue;
            }
#ifndef CONFIG_CUSTOMER_CM_AT
            aic_sntp_client_set_time_to_system(aic_sntp_client);
#endif
            /* We have a valid update.  Get the SNTP Client time.  */
            status = nx_sntp_client_get_local_time_extended(aic_sntp_client,
                                                            &seconds,
                                                            &fraction,
                                                            NX_NULL, 0);
            /* Convert fraction to microseconds. */
            nx_sntp_client_utility_fraction_to_usecs(fraction, &microseconds);
            milliseconds = ((microseconds + 500) / 1000);
            if (status != NX_SUCCESS) {

                error_counter++;
            } else {
                aicx_info(0x19a6fc, "", "SNTP updated\n");
#ifdef CONFIG_NET_PCAP_DUMP
                aic_network_pcap_update_time(seconds - AIC_SNTP_TO_UTC_DIFF,
                                             milliseconds);
#endif
                if (NULL != custom_nw_evt_cb)
                {
                    custom_nw_evt_cb(NW_EVENT_NETTIME_SYNC_IND, NULL, 0);
                }
            }

            /* Clear all events in our event flag. */
            events = events & ~(SNTP_UPDATE_EVENT);
        }

        if (events & SNTP_AT_UPDATE_EVENT) {
            status = nx_sntp_client_receiving_updates(aic_sntp_client,
                                                      &server_status);
            if ((status != NX_SUCCESS) || (server_status == NX_FALSE)) {
                continue;
            }

            if (sntp_is_sync)
                aic_sntp_client_set_time_to_system(aic_sntp_client);
#if defined CONFIG_CUSTOMER_AT || defined NET_AT_MTCPIP || defined CONFIG_NET_AT_SNTP
            time_zone = aic_get_time_zone_in_quarters();
#if CONFIG_CM_TCPIP_AT || CONFIG_GX_TCPIP_AT
            time_zone = aic_get_cclk_time_zone();
#else
            time_zone = aic_get_time_zone_in_quarters();
#endif
            time_buffer = aos_zalloc(SNTP_TIME_SIZE - 28);

#ifdef CONFIG_CM_TCPIP_AT
            sntp_time.seconds = aic_sntp_client->nx_sntp_client_local_ntp_time.seconds;
#else
            sntp_time.seconds = aic_sntp_client->nx_sntp_client_local_ntp_time.seconds +
                                (time_zone * 3600 / 4);
#endif

            sntp_time.fraction = aic_sntp_client->nx_sntp_client_local_ntp_time.fraction;
            nx_sntp_client_utility_convert_seconds_to_date(&sntp_time,
                                                           AIC_CURRENT_YEAR,
                                                           &DisplayTime);
            if (time_zone > 0) {
                snprintf(time_buffer, SNTP_TIME_SIZE - 28,
                         "%d/%02d/%02u,%02d:%02d:%02d+%02d",
                         DisplayTime.year, DisplayTime.month, DisplayTime.day,
                         DisplayTime.hour, DisplayTime.minute, DisplayTime.second,
                         time_zone);
            } else if (time_zone < 0) {
                snprintf(time_buffer, SNTP_TIME_SIZE - 28,
                         "%d/%02d/%02u,%02d:%02d:%02d-%02d",
                         DisplayTime.year, DisplayTime.month, DisplayTime.day,
                         DisplayTime.hour, DisplayTime.minute, DisplayTime.second,
                         -time_zone);
            } else {
                snprintf(time_buffer, SNTP_TIME_SIZE - 28,
                         "%d/%02d/%02u,%02d:%02d:%02d+00",
                         DisplayTime.year, DisplayTime.month, DisplayTime.day,
                         DisplayTime.hour, DisplayTime.minute, DisplayTime.second);
            }

#ifdef CONFIG_CUSTOMER_AT
            str = aos_zalloc(SNTP_TIME_SIZE);
            snprintf(str, SNTP_TIME_SIZE, "+QNTP: 0,\"%s\"", time_buffer);
            extern AT_TCPIP_CFG at_tcpip_cfg;
            com_id = at_tcpip_cfg.com_id;
#endif
#ifdef NET_AT_MTCPIP
            str = aos_zalloc(SNTP_TIME_SIZE);
            snprintf(str, SNTP_TIME_SIZE, RESP_IP_NTP"0,\"%s\"", (time_buffer + 2));
            extern AT_TCPIP_CFG at_tcpip_cfg[AT_SOCKET_MAX_CONN_NUM];
            com_id = at_tcpip_cfg[0].com_id;
#endif
#ifdef CONFIG_NET_AT_SNTP
            str = aos_zalloc(SNTP_TIME_SIZE);
            snprintf(str, SNTP_TIME_SIZE, "+CIPSNTP: 0,\"%s\"", (time_buffer + 2));
            extern u32 sntp_urc_comid;
            com_id = sntp_urc_comid;
#endif

            aic_transmit_at_cmd_urc(com_id, str, strlen(str));
            aos_free(time_buffer);
            aos_free(str);
#endif
            if (aic_sntp_client->nx_sntp_server_update_time_active == 0)
                timeout = AOS_WAIT_FOREVER;
        }

        if (events & SNTP_AT_SEND_EVENT) {
            tx_event_flags_set(&nx_sntp_client_events, NX_SNTP_CLIENT_SEND_EVENT, TX_OR);
            timeout = sntp_sync_timeout * NX_IP_PERIODIC_RATE;
        }
    }

    /**
     * We can stop the SNTP service if for example we think the SNTP server
     * has stopped sending updates. To restart the SNTP Client, simply call
     * the nx_sntp_client_initialize_unicast or
     * nx_sntp_client_initialize_broadcast using another SNTP server IP
     * address as input, and resume the SNTP Client by
     * calling nx_sntp_client_run_unicast or nx_sntp_client_run_braodcast.
     */
    status = nx_sntp_client_stop(aic_sntp_client);
    if (status != NX_SUCCESS) {
        error_counter++;
    }

    /* When done with the SNTP Client, we delete it */
    status = nx_sntp_client_delete(aic_sntp_client);

    return;
}

/**
 * This application defined handler for handling an impending leap second is
 * not required by the SNTP Client. The default handler below only logs the
 * event for every time stamp received with the leap indicator set.
 */
static u32 leap_second_handler(NX_SNTP_CLIENT *client_ptr, u32 leap_indicator)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(leap_indicator);

    /* Handle the leap second handler... */

    return NX_SUCCESS;
}

/**
 * This application defined handler for handling a Kiss of Death packet is not
 * required by the SNTP Client. A KOD handler should determine
 * if the Client task should continue vs. abort sending/receiving time data
 * from its current time server, and if aborting if it should remove
 * the server from its active server list.
 * Note that the KOD list of codes is subject to change. The list
 * below is current at the time of this software release.
 */
static u32 kiss_of_death_handler(NX_SNTP_CLIENT *client_ptr, u32 KOD_code)
{

    u32 remove_server_from_list = NX_FALSE;
    u32 status = NX_SUCCESS;

    NX_PARAMETER_NOT_USED(client_ptr);

    /* Handle kiss of death by code group. */
    switch (KOD_code) {
    case NX_SNTP_KOD_RATE:
    case NX_SNTP_KOD_NOT_INIT:
    case NX_SNTP_KOD_STEP:
        /* Find another server while this one is temporarily out of service.*/
        status =  NX_SNTP_KOD_SERVER_NOT_AVAILABLE;
        break;
    case NX_SNTP_KOD_AUTH_FAIL:
    case NX_SNTP_KOD_NO_KEY:
    case NX_SNTP_KOD_CRYP_FAIL:
        /**
         * These indicate the server will not service client with time updates
         * without successful authentication.
         */
        remove_server_from_list =  NX_TRUE;
        break;
    default:
        /* All other codes. Remove server before resuming time updates. */
        remove_server_from_list =  NX_TRUE;
        break;
    }

    /* Removing the server from the active server list? */
    if (remove_server_from_list)
    {
        /**
         * Let the caller know it has to bail on this server before
         * resuming service.
         */
        status = NX_SNTP_KOD_REMOVE_SERVER;
    }

    return status;
}

/* This application defined handler for notifying SNTP time update event.*/
static VOID time_update_callback(NX_SNTP_TIME_MESSAGE *time_update_ptr,
                                 NX_SNTP_TIME *local_time)
{
    u32 time;
    NX_PARAMETER_NOT_USED(time_update_ptr);
    NX_PARAMETER_NOT_USED(local_time);

    time = aos_get_tickcount();
    if (((time - origin_time) / NX_IP_PERIODIC_RATE) - 60 > NX_SNTP_UPDATE_TIMEOUT_INTERVAL
        || ((time - origin_time) / NX_IP_PERIODIC_RATE) + 60 > NX_SNTP_UPDATE_TIMEOUT_INTERVAL
        || origin_time == 0) {
        origin_time = aos_get_tickcount();
        aos_set_event(aic_sntp_client_event, SNTP_UPDATE_EVENT, TX_OR);
    }
}

void aic_sntp_delete(void)
{
    AIC_SNTP_SERVER_IP *aic_sntp_server;

    if (!aic_sntp_client) {
        nx_sntp_client_stop(aic_sntp_client);
        nx_sntp_client_delete(aic_sntp_client);
        aos_free(aic_sntp_client);
        aic_sntp_client = AIC_NULL;
    }

    if (aic_sntp_client_thread &&
        aic_sntp_client_thread != AOS_INVALID_THREAD_ID) {
        aos_delete_thread(aic_sntp_client_thread);
        aic_sntp_client_thread = AOS_INVALID_THREAD_ID;
    }

    if (aic_sntp_client_event) {
        aos_delete_event(aic_sntp_client_event);
    }

    aic_sntp_server = aic_sntp_server_ip_ptr;
    while (aic_sntp_server) {
        if(aic_sntp_server == aic_sntp_server->aic_sntp_server_addr_next) {
            aos_free(aic_sntp_server);
            aic_sntp_server_ip_ptr = AIC_NULL;
        } else {
            (aic_sntp_server->aic_sntp_server_addr_next)->aic_sntp_server_addr_prev =
                aic_sntp_server->aic_sntp_server_addr_prev;
            (aic_sntp_server->aic_sntp_server_addr_prev)->aic_sntp_server_addr_next =
                aic_sntp_server->aic_sntp_server_addr_next;

            if (aic_sntp_server_ip_ptr == aic_sntp_server)
                aic_sntp_server_ip_ptr = aic_sntp_server->aic_sntp_server_addr_next;

            aos_free(aic_sntp_server);
        }

        aic_sntp_server = aic_sntp_server_ip_ptr;
    }
}

NX_SNTP_CLIENT *aic_get_sntp_client(void)
{
    return aic_sntp_client;
}

ulong aic_get_utc_time(void)
{
    ulong total_seconds = 0;
    int tz;
    aos_systime local_time;

    aos_memset(&local_time, 0, sizeof(aos_systime));
    aos_get_systime(&local_time);
    tz = aic_get_time_zone_in_quarters();

    total_seconds = aic_sntp_client_convert_date_to_seconds(&local_time, tz);

    return total_seconds;
}

ulong aic_sntp_client_convert_date_to_seconds(aos_systime *local_time, int tz)
{
    int i, leaps;
    ulong total_seconds = 0;

    if (AIC_CURRENT_YEAR > local_time->year) {
        return 0;
    }

    total_seconds += AIC_LOSS_OF_UTC_TIME;

    for (i = AIC_CURRENT_YEAR; i < local_time->year; i++) {
        leaps = (!(i & 0x3) && (i % 100)) || (!(i % 400));
        if (leaps)
            total_seconds += SECONDS_PER_LEAPYEAR;
        else
            total_seconds += SECONDS_PER_NONLEAPYEAR;
    }

    leaps = (!(local_time->year & 0x3) && (local_time->year % 100)) ||
            (!(local_time->year % 400));

    for (i = 1; i < local_time->month; i++) {
        switch (i) {
        case JANUARY:
            total_seconds += SEC_IN_JAN;
            break;
        case FEBRUARY:
            if (!leaps)
                total_seconds += SEC_IN_NONLEAPFEB;
            else
                total_seconds += SEC_IN_LEAPFEB;
            break;
        case MARCH:
            total_seconds += SEC_IN_MAR;
            break;
        case APRIL:
            total_seconds += SEC_IN_APR;
            break;
        case MAY:
            total_seconds += SEC_IN_MAY;
            break;
        case JUNE:
            total_seconds += SEC_IN_JUN;
            break;
        case JULY:
            total_seconds += SEC_IN_JUL;
            break;
        case AUGUST:
            total_seconds += SEC_IN_AUG;
            break;
        case SEPTEMBER:
            total_seconds += SEC_IN_SEP;
            break;
        case OCTOBER:
            total_seconds += SEC_IN_OCT;
            break;
        case NOVEMBER:
            total_seconds += SEC_IN_NOV;
            break;
        case DECEMBER:
            total_seconds += SEC_IN_DEC;
            break;
        }
    }

    total_seconds += ((local_time->day - 1) * SECONDS_PER_DAY);
    total_seconds += (local_time->hour * SECONDS_PER_HOUR);
    total_seconds += (local_time->minute * SECONDS_PER_MINUTE);
    total_seconds += local_time->second;
    /* tz is the quarters of an hour. */
    total_seconds -= (tz * SECONDS_PER_HOUR / 4);

    return total_seconds;
}

static u32 aic_sntp_client_set_time_to_system(NX_SNTP_CLIENT *client_ptr)
{
    u32 status;
    NX_SNTP_DATE_TIME DisplayTime;
    NX_SNTP_TIME *local_time;
    aos_systime time;

    /* Verify the client has set a local time. */
    if (client_ptr->nx_sntp_client_local_ntp_time.seconds == 0)
       return NX_SNTP_ERROR_CONVERTING_DATETIME;

    local_time = &(client_ptr->nx_sntp_client_local_ntp_time);
    status =
        nx_sntp_client_utility_convert_seconds_to_date(local_time,
                                                       AIC_CURRENT_YEAR,
                                                       &DisplayTime);
    if (status != NX_SUCCESS)
        return status;

    time.year = DisplayTime.year;
    time.month = DisplayTime.month;
    time.day = DisplayTime.day;
    time.hour = DisplayTime.hour;
    time.minute = DisplayTime.minute;
    time.second = DisplayTime.second;
    time.msecond = DisplayTime.millisecond;
    aicx_info(0x0f4300, "ddudddd", "sntp set time %d/%02d/%02u %02d:%02d:%02d.%d to system\n",
              time.year, time.month, time.day, time.hour, time.minute,
              time.second, time.msecond);
    aos_set_utctime(&time);

    return NX_SUCCESS;
}


