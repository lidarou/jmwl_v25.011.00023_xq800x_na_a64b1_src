/**************************************************************************/
/*                                                                        */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ALTE Component                                                        */
/**                                                                       */
/**   ALTE Network Driver for NETX 5.3 and above.                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x188636
#define LOG_MODULE    AMOD_DRV

#include <aic_bitmap.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aipc_api.h>
#include <aic_cmd.h>
#include <aic_osa.h>
#include <aos_mem.h>
#include <timer_inc.h>

#include "nx_api.h"
#include "nx_tcp.h"
#include "nx_packet.h"
#include "nx_ipv4.h"
#include "nx_ipv6.h"
#include "nx_icmpv4.h"
#include "nx_icmpv6.h"

#include "aic_net_api.h"
#include "nx_alte_driver.h"
#include "aic_packet_management.h"
#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
#include "aic_ipv6_forward.h"
#endif
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
#include "aic_net_direct_forward.h"
#endif

#include "aic_general_at_handle.h"

#ifdef CONFIG_NET_PCAP_DUMP
#include <aic_network_pcap.h>
#endif

#ifdef CONFIG_NET_VIRT_NIC
#include <aic_virt_nic.h>
#include <aic_boot_mode.h>
#endif

__attribute__((weak)) int nx_bsd_find_udp_socket_by_port(USHORT src_port);
__attribute__((weak)) void nx_bsd_notify_udp_socket_ack(u32 sock_id, u16 length);
__attribute__((weak)) int nx_bsd_check_udp_socket_need_ack(USHORT src_port);

struct alte_network_driver alte_nic_devices;
static void nx_alte_send_task(void *p);
#ifdef CONFIG_D2D
void (*_nx_driver_hardware_packet_received_callback)(NX_PACKET *packet_ptr, u8 *consumed);
extern void aic_d2d_notify_link_up(u8 link_id);
extern void aic_d2d_notify_link_down(u8 link_id);
#endif
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
static void nx_alte_zero_flight_params();
#endif

static int nx_alte_link_up_callback(u8 link_id, link_state_t link_status)
{
    AOS_INTERRUPT_DECLARE
    struct alte_network_driver *nic_dev = &alte_nic_devices;

    if (link_status == ALINK_STATE_RECV_BLKS)
        return AIC_SUCCESS;

    aicx_info(0x04d41e, "dd", "[driver]callback:link %d state %d\n", link_id, link_status);

    AOS_INTERRUPT_DISABLE
    if (link_status) {
        nic_dev->s_link_up_status |= BIT(link_id);
        AOS_INTERRUPT_RESTORE
        aos_set_event(nic_dev->send_event, ALTE_SEND_LINK_UP_EVENT,
                      AOS_OR);
    } else {
        nic_dev->s_link_up_status &= ~BIT(link_id);
        AOS_INTERRUPT_RESTORE
    }

#ifdef CONFIG_D2D
    if (link_status) {
        aic_d2d_notify_link_up(link_id);
    }

    if (!link_status) {
        aic_d2d_notify_link_down(link_id);
    }
#endif
    return AIC_SUCCESS;
}

static int nx_alte_release_blk(struct alte_network_driver *nic_dev,
                               u32 link_id)
{
    ABLK_T blk;
    u32 block_cnt;
    int i, ret = AIC_SUCCESS;

    block_cnt = ablk_get_recv_block_count(nic_dev->aipc_recv_handle,
                                          link_id);

    for (i = 0; i < block_cnt; i++) {
        ret = ablk_recv_block(nic_dev->aipc_recv_handle, link_id,
                              &blk, AIPC_NO_WAIT);
        if (!ret)
            ablk_release_block(nic_dev->aipc_recv_handle, &blk);
        else
            aicx_error(0x1e24a1, "x", "[driver] recv blk fail 0x%x when link not ready\r\n",
                       ret);
    }

    return ret;
}

static int nx_alte_recv(struct alte_network_driver *nic_dev,
                        struct alte_network_driver_cfg *nic_cfg,
                        u32 link_id, u32 netid, u32 direct)
{
    ABLK_T blk;
    static u8 non_ip_cnt = 0;
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
    u8 protocol;
    u32 ip_header_len;
#endif
#if defined(CONFIG_LOW_THROUGHPUT) || defined(AIC_ENABLE_IP_SHARE_COPY)
    u8 *data_ptr = NULL;
    int retry_cnt = 0;
#endif
    int ret, i, block_cnt;
    NX_IP *ip_ptr;
    struct aos_mbuf_st *mbuf;
    AOS_INTERRUPT_DECLARE
    NX_PACKET *packet_ptr = NULL, *new_packet_ptr = NULL;
    struct alte_network_driver *net_dev = &alte_nic_devices;

    block_cnt = ablk_get_recv_block_count(nic_dev->aipc_recv_handle, link_id);
    aicx_debug(0x17860e, "dd", "[driver] recv block cnt %d link_id %d\n", block_cnt, link_id);

    for (i = 0; i < block_cnt; i++) {
        memset(&blk, 0, sizeof(blk));
        ret = ablk_recv_block(nic_dev->aipc_recv_handle, link_id, &blk,
                              AIPC_NO_WAIT);
        mbuf = blk.addr;
        if (!mbuf || ret) {
            aicx_error(0x16c0e5, "d", "[driver] recv mbuf is null ret %d\n", ret);
            continue;
        }

        if (net_dev->enhance_timer_enable) {
            u32 remaining_time = 0;

            aos_get_remaining_time(net_dev->enhance_timer, &remaining_time);
            alog_info("[NIC] DL Enhance remaining_time %d\n",
                      remaining_time);

            if (remaining_time > 0) {
                alog_info("[NIC] DL Enhance restart stage %d idle %d retry %d\n",
                          net_dev->enhance_stage, net_dev->enhance_idle_time,
                          net_dev->enhance_retry_time);
                if ((net_dev->enhance_stage == AIC_NETWORK_ENHANCE_RETRY_TIME_STAGE) &&
                    (net_dev->enhance_retry_time > net_dev->enhance_idle_time)) {
                    aos_change_timer(net_dev->enhance_timer,
                                     net_dev->enhance_retry_time - net_dev->enhance_idle_time,
                                     AOS_NO_PERIOD);
                } else {
                    aos_change_timer(net_dev->enhance_timer,
                                     net_dev->enhance_idle_time, AOS_NO_PERIOD);
                }
            }
        }

        ip_ptr = nic_cfg->nx_nic_ip_instance;

#if defined(CONFIG_LOW_THROUGHPUT) || defined(AIC_ENABLE_IP_SHARE_COPY)
        if (!net_dev->disable_copy) {
ALLOC_AGAIN:
            data_ptr = aos_dma_malloc(mbuf->data_len + mbuf->data_offset);
            if (!data_ptr && (retry_cnt++ > 3)) {
                aicx_info(0x14c315, "d", "[LTE_DL] drop packet when recv retry cnt %d\n", retry_cnt);
                ablk_release_block(nic_dev->aipc_recv_handle, &blk);
                retry_cnt = 0;
                continue;
            } else if (!data_ptr) {
                aos_sleep(10);
                aicx_info(0x04f1bd, "d", "[LTE_DL] alloc packet fail when recv retry cnt %d\n", retry_cnt);
                goto ALLOC_AGAIN;
            }

            retry_cnt = 0;
            memcpy(data_ptr + mbuf->data_offset, mbuf->data_ptr + mbuf->data_offset, mbuf->data_len);
            packet_ptr = aic_packet_build(data_ptr,
                                          mbuf->data_len + mbuf->data_offset,
                                          mbuf->data_offset,
                                          ip_ptr->nx_ip_default_packet_pool,
                                          AIC_PACKET_DYNAMIC_MEM);
            if (!packet_ptr) {
                ablk_release_block(nic_dev->aipc_recv_handle, &blk);
                aicx_error(0x1509dd, "x", "[LTE_DL] build packet fail 0x%x\n", ret);
                continue;
            }
            ablk_release_block(nic_dev->aipc_recv_handle, &blk);
        } else {
            packet_ptr = aic_packet_build(mbuf->data_ptr,
                                          mbuf->data_len + mbuf->data_offset,
                                          mbuf->data_offset,
                                          ip_ptr->nx_ip_default_packet_pool,
                                          AIC_PACKET_SHARE_MEM);
            if (!packet_ptr) {
                ablk_release_block(nic_dev->aipc_recv_handle, &blk);
                aicx_error(0x03dd97, "x", "[driver] alloc packet fail 0x%x\n", ret);
                continue;
            }
        }
#else

        aicx_debug(0x0c8238, "xxxxx", "[LTE_DL] recv data ptr 0x%x offset 0x%x len 0x%x val 0x%x 0x%x\n",
                   mbuf->data_ptr, mbuf->data_offset, mbuf->data_len,
                   *(u32 *)(mbuf->data_ptr + mbuf->data_offset),
                   *(u32 *)(mbuf->data_ptr + mbuf->data_offset + 4));
        packet_ptr = aic_packet_build(mbuf->data_ptr,
                                      mbuf->data_len + mbuf->data_offset,
                                      mbuf->data_offset,
                                      ip_ptr->nx_ip_default_packet_pool,
                                      AIC_PACKET_SHARE_MEM);
        if (!packet_ptr) {
            ablk_release_block(nic_dev->aipc_recv_handle, &blk);
            aicx_error(0x03dd97, "x", "[driver] alloc packet fail 0x%x\n", ret);
            continue;
        }
#endif

        packet_ptr->nx_packet_length = mbuf->data_len;
        aicx_debug(0x0fa00c, "xx", "recv packet_ptr->nx_packet_data_start 0x%x 0x%x\n",
                   packet_ptr->nx_packet_data_start, mbuf->data_ptr);

        if (net_dev->work_mode != AIC_D2D_DATA_MODE) {
            if ((packet_ptr->nx_packet_prepend_ptr[0] >> 4) ==
                NX_IP_VERSION_V4) {
                packet_ptr->nx_packet_ip_version = NX_IP_VERSION_V4;
            } else if ((packet_ptr->nx_packet_prepend_ptr[0] >> 4) ==
                       NX_IP_VERSION_V6) {
                packet_ptr->nx_packet_ip_version = NX_IP_VERSION_V6;
            } else {
                non_ip_cnt++;
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
                if (non_ip_cnt >= 10) {
                    non_ip_cnt = 0;
                    if ((aic_get_tick() - net_dev->last_trigger_fast_rebuild) > 3000) {
                        aicx_warn(0x0ce558, "", "[NIC] too many non-ip packet\n");
                        aos_set_event(net_dev->send_event, ALTE_FAST_REBUILD_EVENT, AOS_OR);
                        net_dev->last_trigger_fast_rebuild = aic_get_tick();
                    }
                }
#endif
                _nx_packet_release(packet_ptr);
                aicx_error(0x0e5041, "d", "[driver] recv packet neither ipv4 nor ipv6 cnt %d\n",
                           non_ip_cnt);
                continue;
            }
        }

        non_ip_cnt = 0;
        packet_ptr->nx_packet_ip_header =
            packet_ptr->nx_packet_prepend_ptr;
        packet_ptr->nx_packet_ip_interface =
            nic_cfg->nx_nic_interface_ptr;
        packet_ptr->nx_packet_append_ptr =
            packet_ptr->nx_packet_prepend_ptr +
            packet_ptr->nx_packet_length;
        if (!packet_ptr->nx_packet_ip_interface) {
            aicx_info(0x06759a, "d", "[NIC] interface maybe detach %d\r\n",
                      nic_cfg->nx_nic_activated);
            nx_packet_release(packet_ptr);
            continue;
        }

        AOS_INTERRUPT_DISABLE
        if (nic_cfg->nx_nic_interface_ptr) {
                nic_cfg->nx_nic_interface_ptr->nx_interface_packets_received++;
                nic_cfg->nx_nic_interface_ptr->nx_interface_bytes_received +=
                    packet_ptr->nx_packet_length;
        }
        AOS_INTERRUPT_RESTORE

#ifndef AIC_DISABLE_NET_CONNECT_CHECK
        alte_nic_devices.monitor_timestamp = aic_get_tick();
        nx_alte_zero_flight_params();
        ret = aic_net_get_trans_params(packet_ptr, &protocol, &ip_header_len,
                                       NULL, NULL);
        if (net_dev->network_access_status && !ret) {
            switch (protocol) {
            case NX_PROTOCOL_TCP:
                net_dev->icmp_monitor_cnt = 0;
                net_dev->tcp_monitor_cnt = 0;
                net_dev->others_monitor_cnt = 0;
                break;
            case NX_PROTOCOL_ICMP:
            case NX_PROTOCOL_ICMPV6:
                if ((protocol == NX_PROTOCOL_ICMP &&
                     (*(packet_ptr->nx_packet_prepend_ptr + ip_header_len) ==
                     NX_ICMP_ECHO_REQUEST_TYPE ||
                     *(packet_ptr->nx_packet_prepend_ptr + ip_header_len) ==
                     NX_ICMP_ECHO_REPLY_TYPE)) ||
                    (protocol == NX_PROTOCOL_ICMPV6 &&
                     (*(packet_ptr->nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER)) ==
                     NX_ICMPV6_ECHO_REPLY_TYPE ||
                     *(packet_ptr->nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER)) ==
                     NX_ICMPV6_ECHO_REQUEST_TYPE))) {
                    net_dev->icmp_monitor_cnt = 0;
                    net_dev->tcp_monitor_cnt = 0;
                    net_dev->others_monitor_cnt = 0;
                }
                break;
            default:
                net_dev->icmp_monitor_cnt = 0;
                net_dev->tcp_monitor_cnt = 0;
                net_dev->others_monitor_cnt = 0;
                break;
            }
        }
#endif
#ifdef CONFIG_NET_PCAP_DUMP
        aic_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr,
                              packet_ptr->nx_packet_length,
                              AIC_PDP_NIC_FLAG);
#endif

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
        if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4) {
            ret = aic_net_down_link_direct_dispatch(ip_ptr, packet_ptr, netid);
            if (ret == DIRECT_FORWARD_CONSUMER)
                continue;
        }
#endif

#ifdef CONFIG_NET_VIRT_NIC
        if (!aic_boot_mode_is_basic() &&
            (net_dev->work_mode != AIC_LTE_AT_SOCKET_MODE)) {
            aic_virt_nic_send(packet_ptr);
            continue;
        }
#endif

#ifdef CONFIG_D2D
        u8 consumer;
        aicx_debug(0x0a0501, "dd", "[D2D] work mode %d recv packet len %d\n", net_dev->work_mode,
                   packet_ptr->nx_packet_length);
        if (net_dev->work_mode == AIC_D2D_DATA_MODE) {
            extern int aic_d2d_nic_send(NX_PACKET *packet_ptr, u8 link_id);
            ret = aic_d2d_nic_send(packet_ptr, link_id);
            if (!ret)
                continue;

            _nx_driver_hardware_packet_received_callback(packet_ptr, &consumer);
            if (!consumer)
                nx_packet_release(packet_ptr);

            continue;
        }
#endif

        if ((packet_ptr->nx_packet_reserved & AIC_PACKET_SHARE_MEM) &&
            aic_net_check_fragment_packet(packet_ptr)) {
            ret = nx_packet_copy(packet_ptr, &new_packet_ptr,
                                 ip_ptr->nx_ip_default_packet_pool, 0);
            if (ret == NX_SUCCESS) {
                nx_packet_release(packet_ptr);
                packet_ptr = new_packet_ptr;
            }
        }

        _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);
    }

    return AIC_SUCCESS;
}

/**
 * nx_alte_recv_task - Receive data from AIPC.
 */
static void nx_alte_recv_task(void *p)
{
    int i;
    u32 links, link_id;
    struct alte_network_driver *nic_dev = p;
    struct alte_network_driver_cfg *nic_cfg;

    while (1) {
        links = ablk_select(nic_dev->aipc_recv_handle, nic_dev->s_set,
                            AIPC_WAIT_FOREVER);

        for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
            if (nic_dev->alte_nic_devs[i].nx_nic_activated &&
                (nic_dev->alte_nic_devs[i].s_set & links)) {
                nic_cfg = &nic_dev->alte_nic_devs[i];
                if (links & BIT((nic_cfg->netid << 1) + ALTE_HIGH)) {
                    link_id = (nic_cfg->netid << 1) + ALTE_HIGH;
                    aicx_debug(0x177f45, "d", "[driver] recv from link_id %d\n", link_id);
                    nx_alte_recv(nic_dev, nic_cfg, link_id, nic_cfg->netid, 1);
                    links &= ~BIT((nic_cfg->netid << 1) + ALTE_HIGH);
                }

                if (links & BIT(nic_cfg->netid << 1)) {
                    link_id = (nic_cfg->netid << 1);
                    aicx_debug(0x177f45, "d", "[driver] recv from link_id %d\n", link_id);
                    nx_alte_recv(nic_dev, nic_cfg, link_id, nic_cfg->netid, 1);
                    links &= ~BIT(nic_cfg->netid << 1);
                }
            }
        }

        if (links) {
            for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
                if (links & BIT((i << 1) + ALTE_HIGH)) {
                    aicx_error(0x067d3f, "xddx", "[driver] links 0x%x not open i %d %d 0x%x\r\n",
                               links, i,
                               nic_dev->alte_nic_devs[i].nx_nic_activated,
                               nic_dev->alte_nic_devs[i].s_set);
                    link_id = (i << 1) + ALTE_HIGH;
                    nx_alte_release_blk(nic_dev, link_id);
                }

                if (links & BIT(i << 1)) {
                    aicx_error(0x067d3f, "xddx", "[driver] links 0x%x not open i %d %d 0x%x\r\n",
                               links, i,
                               nic_dev->alte_nic_devs[i].nx_nic_activated,
                               nic_dev->alte_nic_devs[i].s_set);
                    link_id = (i << 1);
                    nx_alte_release_blk(nic_dev, link_id);
                }
            }
        }
    }
}

static void nx_alte_destroy_thread(void)
{
    struct alte_network_driver *net_dev = &alte_nic_devices;

    if (!net_dev->thread_cnt) {
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
        memset(&net_dev->tcp_monitor_cnt, 0,
               (u32)&net_dev->current_send_cache_size -
               (u32)&net_dev->tcp_monitor_cnt + 4);
#else
        net_dev->current_send_cache_size = 0;
#endif
#if 0
        net_dev->tcp_monitor_cnt = 0;
        net_dev->icmp_monitor_cnt = 0;
        net_dev->others_monitor_cnt = 0;
        net_dev->notify_rebuild_cnt = 0;
        net_dev->flight_icmp_cnt = 0;
        net_dev->flight_tcp_cnt = 0;
        net_dev->current_send_cache_size = 0;
        net_dev->monitor_timestamp = 0;
#endif
        aos_set_event(net_dev->send_event, 0, AOS_AND);
    }
}

static void nx_alte_create_thread(void)
{
    struct alte_network_driver *net_dev = &alte_nic_devices;

    if (net_dev->thread_cnt) {
        net_dev->thread_cnt++;
        return;
    }

    net_dev->thread_cnt++;
}

static void nx_alte_reduce_statistics(struct alte_network_driver_cfg *nic_cfg,
                                      NX_PACKET *packet_ptr)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    nic_cfg->packet_queue_count--;
    if (packet_ptr->nx_packet_reserved & AIC_PACKET_SHARE_MEM) {
        nic_cfg->share_packet_queue_count--;
    } else if (packet_ptr->nx_packet_reserved & AIC_PACKET_DYNAMIC_MEM) {
            nic_cfg->heap_packet_queue_count--;
            alte_nic_devices.current_send_cache_size -=
                (packet_ptr->nx_packet_data_end -
                 packet_ptr->nx_packet_data_start);
            if ((int)alte_nic_devices.current_send_cache_size < 0)
                alte_nic_devices.current_send_cache_size = 0;
    } else {
        nic_cfg->nx_packet_queue_count--;
        alte_nic_devices.current_send_cache_size -=
            (packet_ptr->nx_packet_data_end -
             packet_ptr->nx_packet_data_start);
        if ((int)alte_nic_devices.current_send_cache_size < 0)
            alte_nic_devices.current_send_cache_size = 0;
    }
    AOS_INTERRUPT_RESTORE
}

/**
 * _nx_alte_driver_send - Send packet.
 *
 * @nx_ip_driver: The structure of the network card driver in NETX DUO.
 * @nic: Network card information.
 *
 * Send the data packet through AIPC, and if it is an ACK packet, send it
 * through the high-priority channel.
 */
static int _nx_alte_driver_send(NX_PACKET *packet_ptr,
                                struct alte_network_driver_cfg *nic_cfg,
                                u8 is_high, u32 timeout, u32 in_queue)
{
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
    u8 protocol;
    u32 ip_header_len;
#endif
    u32 actual_flags_ptr;
    bool share_mem = FALSE;
    int ret, link_id;
    ABLK_T blk;
    u32 prev_timestamp;
    struct aos_mbuf_st *mbuf;
#ifndef NX_DISABLE_PACKET_CHAIN
    NX_PACKET *curr_packet;
#endif
    struct alte_network_driver *net_dev = &alte_nic_devices;

    if (aic_get_reg_mode() == AT_REG_INSTRU_NET) {
        if (in_queue)
            nx_alte_reduce_statistics(nic_cfg, packet_ptr);
        nx_packet_transmit_release(packet_ptr);
        return AIC_ENOSYS;
    }

    if (!nic_cfg->nx_nic_activated) {
        if (in_queue)
            nx_alte_reduce_statistics(nic_cfg, packet_ptr);
        nx_packet_transmit_release(packet_ptr);
        return AIC_ENODEV;
    }

    if (!(net_dev->s_link_up_status & BIT(nic_cfg->netid << 1)) ||
        !(net_dev->s_link_up_status & BIT((nic_cfg->netid << 1) + 1))) {
        aicx_info(0x0ffe4b, "xd", "[NIC] alte aipc link not ready 0x%x active %d\r\n",
                  net_dev->s_link_up_status,
                  nic_cfg->nx_nic_activated);

        ret = aos_get_event(net_dev->send_event, ALTE_SEND_LINK_UP_EVENT,
                            AOS_OR_CLEAR, &actual_flags_ptr, timeout);
        if (ret)
            aicx_error(0x13b15a, "dx", "[NIC] get alte netid %d link up event fail 0x%x\r\n",
                       nic_cfg->netid, ret);

        aos_set_event(net_dev->send_event, ALTE_NETWORK_CHECK_PDP, AOS_OR);

        if (!(net_dev->s_link_up_status & BIT(nic_cfg->netid << 1)) ||
            !(net_dev->s_link_up_status & BIT((nic_cfg->netid << 1) + 1))) {
            aicx_info(0x004a79, "dxd", "[NIC] alte check netid %d aipc link status 0x%x active %d fail\r\n",
                      nic_cfg->netid, net_dev->s_link_up_status,
                      nic_cfg->nx_nic_activated);
            if (in_queue)
                nx_alte_reduce_statistics(nic_cfg, packet_ptr);
            nx_packet_transmit_release(packet_ptr);
            return AIC_ENOTCONN;
        }
    }

    if (is_high)
       link_id = (nic_cfg->netid << 1) + ALTE_HIGH;
    else
       link_id = (nic_cfg->netid << 1);

    aicx_debug(0x137d8d, "x", "alte send packet len 0x%x\n", packet_ptr->nx_packet_length);
    if (ablk_is_brr_blk(net_dev->aipc_send_handle,
                        packet_ptr->nx_packet_data_start, FALSE)) {
        aicx_debug(0x0b4a1b, "xx", "alte nic send blk directly 0x%x 0x%x\n",
                  (u32)packet_ptr, (u32)packet_ptr->nx_packet_data_start);
        share_mem = TRUE;
        goto SEND_BLK;
    }

    ret = ablk_get_block(net_dev->aipc_send_handle, sizeof(struct aos_mbuf_st) +
                         packet_ptr->nx_packet_length + ALTE_BLOCK_OFFSET + ALTE_BLOCK_TAIL_OFFSET,
                         &blk, timeout);
    if (ret != AIC_SUCCESS) {
        if (timeout)
            aicx_warn(0x035a21, "dddddddddxx", "[driver] get block fail %d len %d share %d heap %d nx %d netid %d timeout %d %d %d 0x%x 0x%x\r\n",
                      ret,
                      sizeof(struct aos_mbuf_st) + packet_ptr->nx_packet_length +
                      ALTE_BLOCK_OFFSET, nic_cfg->share_packet_queue_count,
                      nic_cfg->heap_packet_queue_count, nic_cfg->nx_packet_queue_count,
                      nic_cfg->netid, timeout,
                      nic_cfg->high_packet_queue_count, nic_cfg->normal_packet_queue_count,
                      nic_cfg->send_ack_packet_queue_head, nic_cfg->send_packet_queue_head);
        //_nx_packet_transmit_release(packet_ptr);
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
        if ((timeout > 200) &&
            ((aic_get_tick() - net_dev->last_trigger_fast_rebuild) > 3000)) {
             aos_set_event(net_dev->send_event, ALTE_FAST_REBUILD_EVENT, AOS_OR);
             net_dev->last_trigger_fast_rebuild = aic_get_tick();
        }
#endif
        return AIC_ENOMEM;
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    u32 offset = ALTE_BLOCK_OFFSET + sizeof(struct aos_mbuf_st);
    curr_packet = packet_ptr;
    while (curr_packet) {
       memcpy((void *)((u32)blk.addr + offset),
              curr_packet->nx_packet_prepend_ptr,
              curr_packet->nx_packet_append_ptr -
              curr_packet->nx_packet_prepend_ptr);
       offset += curr_packet->nx_packet_append_ptr -
              curr_packet->nx_packet_prepend_ptr;
       curr_packet = curr_packet->nx_packet_next;
    }

    if ((offset - ALTE_BLOCK_OFFSET - sizeof(struct aos_mbuf_st)) !=
        packet_ptr->nx_packet_length) {
        aicx_info(0x09d462, "dd", "[driver] chain packet err %d %d\n",
                  (offset - ALTE_BLOCK_OFFSET - sizeof(struct aos_mbuf_st)),
                  packet_ptr->nx_packet_length);
        if (in_queue)
            nx_alte_reduce_statistics(nic_cfg, packet_ptr);
        _nx_packet_transmit_release(packet_ptr);
        ablk_recycle_block(net_dev->aipc_send_handle, &blk);
        return AIC_EOVERFLOW;
    }
#else
    memcpy(blk.addr + ALTE_BLOCK_OFFSET + sizeof(struct aos_mbuf_st),
           packet_ptr->nx_packet_prepend_ptr,
           packet_ptr->nx_packet_length);
#endif

SEND_BLK:
    if (!share_mem) {
        mbuf = blk.addr;
        memset(mbuf, 0, sizeof(struct aos_mbuf_st));
        mbuf->data_len = packet_ptr->nx_packet_length;
        mbuf->data_offset = ALTE_BLOCK_OFFSET + sizeof(struct aos_mbuf_st);
        mbuf->data_ptr = blk.addr;
        aicx_debug(0x152dda, "", "[LTE_UL] alloc share mem\n");
    } else {
        mbuf = (struct aos_mbuf_st *)packet_ptr->nx_packet_data_start;
        memset(mbuf, 0, sizeof(struct aos_mbuf_st));
        mbuf->data_len = packet_ptr->nx_packet_length;
        mbuf->data_offset = packet_ptr->nx_packet_prepend_ptr -
            packet_ptr->nx_packet_data_start;
        mbuf->data_ptr = packet_ptr->nx_packet_data_start;
        blk.addr = packet_ptr->nx_packet_data_start;
        blk.size = packet_ptr->nx_packet_length + mbuf->data_offset + ALTE_BLOCK_TAIL_OFFSET;
        aicx_debug(0x072e84, "", "[LTE_UL] reuse share mem\n");
    }

#ifndef AIC_DISABLE_NET_CONNECT_CHECK
    if (net_dev->network_access_status && net_dev->work_mode != AIC_D2D_DATA_MODE &&
        !aic_net_get_trans_params(packet_ptr, &protocol, &ip_header_len,
                                  NULL, NULL)) {
        switch (protocol) {
        case NX_PROTOCOL_TCP:
            net_dev->tcp_monitor_cnt++;
            net_dev->flight_tcp_cnt++;
            break;
        case NX_PROTOCOL_ICMP:
        case NX_PROTOCOL_ICMPV6:
            if ((protocol == NX_PROTOCOL_ICMP &&
                (*(packet_ptr->nx_packet_prepend_ptr + ip_header_len) ==
                 NX_ICMP_ECHO_REQUEST_TYPE ||
                 *(packet_ptr->nx_packet_prepend_ptr + ip_header_len) ==
                 NX_ICMP_ECHO_REPLY_TYPE)) ||
               (protocol == NX_PROTOCOL_ICMPV6 &&
                (*(packet_ptr->nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER)) ==
                NX_ICMPV6_ECHO_REPLY_TYPE ||
                 *(packet_ptr->nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER)) ==
                NX_ICMPV6_ECHO_REQUEST_TYPE))) {
                net_dev->icmp_monitor_cnt++;
                net_dev->flight_icmp_cnt++;
            }
            break;
        default:
            net_dev->others_monitor_cnt++;
            break;
        }

        if (net_dev->tcp_monitor_cnt > ALTE_MAX_TCP_SEND_CNT ||
            net_dev->icmp_monitor_cnt > ALTE_MAX_ICMP_SEND_CNT ||
            net_dev->others_monitor_cnt > ALTE_MAX_OTHERS_SEND_CNT) {
            aicx_info(0x1a72b7, "ddddd", "[NIC] tcp %d icmp %d others %d prev time %d new time %d\n",
                      net_dev->tcp_monitor_cnt, net_dev->icmp_monitor_cnt,
                      net_dev->others_monitor_cnt, (u32)net_dev->monitor_timestamp,
                      (u32)aic_get_tick());
            if ((aic_get_tick() > net_dev->monitor_timestamp) &&
                ((aic_get_tick() - net_dev->monitor_timestamp) > 2000))
                aos_set_event(net_dev->send_event, ALTE_PING_EVENT,
                              AOS_OR);
        }
    }
#endif

    aic_identify_packet_pri(packet_ptr, mbuf);

    aicx_debug(0x1af8f2, "xxxxxdx", "[LTE_UL] send blk addr 0x%x 0x%x offset 0x%x val 0x%x proto 0x%x link_id %d packet_ptr 0x%x\r\n",
               blk.addr, mbuf->data_ptr, mbuf->data_offset,
               *(u32 *)(mbuf->data_ptr + mbuf->data_offset),
               mbuf->user_num, link_id, packet_ptr);

    if (net_dev->enhance_timer_enable) {
        aicx_info(0x17b179, "dd", "[NIC] Enhance start idle time %d, pri %d\n",
                  net_dev->enhance_idle_time, mbuf->ext);
        aos_change_timer(net_dev->enhance_timer,
                         net_dev->enhance_idle_time, AOS_NO_PERIOD);
        net_dev->enhance_send_ok = AOS_FALSE;
        net_dev->enhance_stage = AIC_NETWORK_ENHANCE_IDLE_TIME_STAGE;
        mbuf->ext = ALTE_ENHANCE_PACKET_PRIORITY;
    }

    if (mbuf->user_num == NX_PROTOCOL_UDP &&
        nx_bsd_check_udp_socket_need_ack((mbuf->ext1)))
        mbuf->ext = ALTE_ENHANCE_PACKET_PRIORITY;

    prev_timestamp = (u32)aic_get_tick();
    ret = ablk_send_block(net_dev->aipc_send_handle, link_id, &blk, timeout);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x11bea0, "dd", "[LTE_UL] send blk fail %d %d\n", link_id, ret);
        if (!share_mem)
            ablk_recycle_block(net_dev->aipc_send_handle, &blk);

        return AIC_ENOMEM;
    }

    if (aic_get_tick() - prev_timestamp > 1000)
        aicx_warn(0x14f39d, "d", "[LTE_UL] send data wait too long %d\r\n",
                  (u32)aic_get_tick() - prev_timestamp);

#ifdef CONFIG_NET_PCAP_DUMP
    aic_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr,
                          packet_ptr->nx_packet_length,
                          AIC_PDP_NIC_FLAG);
#endif

    if (in_queue)
        nx_alte_reduce_statistics(nic_cfg, packet_ptr);

    if (!share_mem)
        _nx_packet_transmit_release(packet_ptr);
    else
        aic_free_packet_only(packet_ptr);

#ifdef CONFIG_D2D
    BRR_MEM_INFO_T info;
    u8 at_str[32];
    u32 used = 0;
    if (net_dev->enable_d2d_fc) {
        ret = ablk_req_mem_info(net_dev->aipc_send_handle, 128, &info);
        if (!ret) {
            used = (u32)(((float)(info.total - info.free_cnt) / (float)info.total) * 100);
            if (used >= 80) {
                memset(at_str, 0, sizeof(at_str));
                snprintf(at_str, 32, "+DDATAFC: %d,%d%%",
                         (link_id >> 1) + 1, used);
                aic_transmit_at_cmd_urc(aic_get_d2d_urc_com_id(), at_str, strlen(at_str));
            }
        }
    }
#endif

    return AIC_SUCCESS;
}

#ifndef AIC_DISABLE_NET_CONNECT_CHECK
static void nx_alte_zero_flight_params()
{
    alte_nic_devices.flight_icmp_cnt = 0;
    alte_nic_devices.flight_tcp_cnt = 0;
    alte_nic_devices.flight_timestamp = aic_get_tick();
}

static void nx_alte_flight_check()
{
    if ((alte_nic_devices.flight_icmp_cnt > ALTE_MAX_ICMP_FLIGHT_CNT ||
         alte_nic_devices.flight_tcp_cnt > ALTE_MAX_TCP_FLIGHT_CNT) &&
        (aic_get_tick() - alte_nic_devices.flight_timestamp) > ALTE_FLIGHT_TIMEOUT) {
        aicx_info(0x0b4212, "ddd", "[NIC] flight_icmp_cnt %d flight_tcp_cnt %d flight timeout %d\r\n",
                  alte_nic_devices.flight_icmp_cnt, alte_nic_devices.flight_tcp_cnt,
                  (u32)(aic_get_tick() - alte_nic_devices.flight_timestamp));
        if (alte_nic_devices.network_abnormal_notify)
            alte_nic_devices.network_abnormal_notify(0);
        aos_set_event(alte_nic_devices.send_event, ALTE_NETWORK_NOTIFY_FLIGHT,
                      AOS_OR);
        nx_alte_zero_flight_params();
    }
}

static int nx_alte_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, u32 retry_cnt,
                        u32 timeout, u32 fail_wait)
{
    int i, ret;
    u8 send_buf[32];
    NX_PACKET *packet_ptr = NULL;

    if (!ip_ptr || !ip_address)
        return 0;

    for (i = 0; i < sizeof(send_buf); i++)
        send_buf[i] = 'a' + 1;

    send_buf[30] = 0xFF;
    send_buf[31] = 0xFF;

    for (i = 0; i < retry_cnt; i++) {
        ret = nxd_icmp_ping(ip_ptr, ip_address,
                            send_buf, sizeof(send_buf), &packet_ptr,
                            timeout);
        if ((ret != NX_SUCCESS) || (packet_ptr == NX_NULL)) {
            aicx_info(0x171f2a, "xxddd", "[NIC] network ping fail 0x%x 0x%x %d %d %d\n",
                      ret, (u32)packet_ptr, retry_cnt, timeout, fail_wait);

            if (packet_ptr)
                nx_packet_release(packet_ptr);

            if ((ret != NX_NO_RESPONSE) && fail_wait)
                aos_sleep(fail_wait);
        } else {
            nx_packet_release(packet_ptr);
            aicx_info(0x1546d9, "", "[NIC] network access successfully\n");
            nx_alte_zero_flight_params();

            return 0;
        }
    }

    return -1;
}

static void nx_alte_ping_task(void *p)
{
    AMSG_T msg;
    u8 pdp_type;
    NXD_ADDRESS dst_address;
    int i, fail_cnt = 0, cid;
    u32 actual_flags_ptr, ret;
    struct aic_network_ip_instance *aic_ip_ptr;
    struct alte_network_driver *nic_dev = p;

    while (1) {
        actual_flags_ptr = 0;
        aos_get_event(nic_dev->send_event,
#ifndef CONFIG_NET_VIRT_NIC
                      ALTE_NETWORK_ACCESS_EVENT | ALTE_PING_EVENT |
#endif
                      ALTE_FAST_REBUILD_EVENT | ALTE_NETWORK_NOTIFY_RELEASE |
                      ALTE_NETWORK_NOTIFY_FLIGHT | ALTE_NETWORK_CHECK_PDP |
                      ALTE_NETWORK_SEND_ONLY,
                      AOS_OR_CLEAR, &actual_flags_ptr, AOS_WAIT_FOREVER);

        if (aic_get_reg_mode() == AT_REG_INSTRU_NET)
            continue;

        if (actual_flags_ptr & ALTE_NETWORK_NOTIFY_FLIGHT) {
            ret = aic_send_cmd_and_wait("AT+CFUN=0", NULL);
            aicx_info(0x0db0cb, "d", "[NIC] send AT+CFUN=0 ret %d\r\n", ret);
            ret = aic_send_cmd_and_wait("AT+CFUN=1", NULL);
            aicx_info(0x060cfc, "d", "[NIC] send AT+CFUN=1 ret %d\r\n", ret);
            nx_alte_zero_flight_params();
        }

        aic_ip_ptr = aic_get_default_ip_instance();

        cid = 0xFF;
        for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
            if (nic_dev->alte_nic_devs[i].nx_nic_activated) {
                cid = nic_dev->alte_nic_devs[i].netid + 1;
                break;
            }
        }

        if (cid < 1 || cid > ANET_MAX_PDP_CID || !aic_ip_ptr ||
            !aic_ip_ptr->ip_ptr) {
            aicx_info(0x0a36be, "", "[NIC] don't have active pdp\r\n");
            continue;
        }

        for (i = 0; i < ALTE_GET_PDP_TYPE_MAX_CNT; i++) {
            pdp_type = aic_get_pdp_ip_type(cid);
            if (pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
                pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
                pdp_type == ANET_DEFAULT_IP_TYPE_V4V6)
                break;
            else
                aos_sleep(3000);
        }

        if (pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6) {
            dst_address.nxd_ip_version = NX_IP_VERSION_V4;
            dst_address.nxd_ip_address.v4 = IP_ADDRESS(8, 8, 8, 8);
        } else if (pdp_type == ANET_DEFAULT_IP_TYPE_V6) {
            dst_address.nxd_ip_version = NX_IP_VERSION_V6;
            dst_address.nxd_ip_address.v6[0] = 0x20014860;
            dst_address.nxd_ip_address.v6[1] = 0x48600000;
            dst_address.nxd_ip_address.v6[2] = 0x0;
            dst_address.nxd_ip_address.v6[3] = 0x8844;
            if (actual_flags_ptr & ALTE_NETWORK_ACCESS_EVENT)
                aos_sleep(3000);
        } else {
            aicx_info(0x18a47f, "x", "[NIC] get pdp type fail 0x%x\r\n", pdp_type);
            continue;
        }

        aicx_info(0x17fb57, "ddddxdd", "[NIC] connectivity check tcp %d icmp %d others %d status %d actual_flags_ptr 0x%x cid %d pdp_type %d\n",
                  nic_dev->tcp_monitor_cnt, nic_dev->icmp_monitor_cnt,
                  nic_dev->others_monitor_cnt, nic_dev->network_access_status,
                  actual_flags_ptr, cid, pdp_type);

        if (actual_flags_ptr & (~ALTE_NETWORK_SEND_ONLY)) {
            nic_dev->icmp_monitor_cnt = 0;
            nic_dev->tcp_monitor_cnt = 0;
            nic_dev->others_monitor_cnt = 0;
        }

#ifdef CONFIG_NET_VIRT_NIC
        if (nic_dev->work_mode != AIC_LTE_AT_SOCKET_MODE) {
            actual_flags_ptr &= (~ALTE_NETWORK_ACCESS_EVENT);
            actual_flags_ptr &= (~ALTE_PING_EVENT);
        }
#endif

        if (actual_flags_ptr & ALTE_NETWORK_SEND_ONLY) {
            aicx_info(0x05f387, "", "[NIC] start ping trigger\r\n");

            nx_alte_ping(aic_ip_ptr->ip_ptr, &dst_address, 1, 0, 0);
        }

        if ((actual_flags_ptr & ALTE_NETWORK_ACCESS_EVENT) &&
            !nic_dev->network_access_status) {
            nic_dev->network_access_retry_cnt++;
            if (!nx_alte_ping(aic_ip_ptr->ip_ptr, &dst_address,
                              ALTE_NETWORK_ACCESS_MAX_CNT,
                              3 * ALTE_CONNECTIVITY_CHECK_TIMEOUT, 2000))
                nic_dev->network_access_status = TRUE;
        } else if (actual_flags_ptr & ALTE_FAST_REBUILD_EVENT) {
            amsg_fill_msg(&msg, AMOD_EL2, ALTE_NOTIFY_L2_REBUILD,
                          ALTE_NOTIFY_L2_REBUILD);
            ret = amsg_send_msg(AIC_SYS_CP, &msg, 0);
            nic_dev->notify_rebuild_cnt++;
            aicx_info(0x198b42, "d", "[NIC] notify modem fast rebuild %d\n", ret);
            nx_alte_flight_check();
            if (alte_nic_devices.network_abnormal_notify)
                alte_nic_devices.network_abnormal_notify(1);

        } else if (nic_dev->pdp_active_flag &&
                   (actual_flags_ptr & ALTE_PING_EVENT) &&
                   !nic_dev->network_abnormal_notify) {
            aicx_info(0x13446e, "", "[NIC] start ping connectivity check\r\n");

            if (nx_alte_ping(aic_ip_ptr->ip_ptr, &dst_address,
                             ALTE_CONNECTIVITY_CHECK_CNT,
                             ALTE_CONNECTIVITY_CHECK_TIMEOUT, 0)) {
                amsg_fill_msg(&msg, AMOD_EL2, ALTE_NOTIFY_L2_REBUILD,
                              ALTE_NOTIFY_L2_REBUILD);
                ret = amsg_send_msg(AIC_SYS_CP, &msg, 0);
                nic_dev->notify_rebuild_cnt++;
                aicx_info(0x15075d, "", "[NIC] notify modem rebuild\r\n");
                nx_alte_flight_check();
            } else {
                ret = AIC_SUCCESS;
            }

            if (!ret) {
                fail_cnt = 0;
                nic_dev->icmp_monitor_cnt = 0;
                nic_dev->tcp_monitor_cnt = 0;
                nic_dev->others_monitor_cnt = 0;
            }
        }

        if (actual_flags_ptr & ALTE_NETWORK_CHECK_PDP) {
            aic_check_all_pdp_state();
        }

        if (actual_flags_ptr & ALTE_NETWORK_NOTIFY_RELEASE) {
            aos_sleep(1000);
            amsg_fill_msg(&msg, AMOD_EL2, AIC_NOTIFY_MODEM_RELEASE,
                          AIC_NOTIFY_MODEM_RELEASE);
            ret = amsg_send_msg(AIC_SYS_CP, &msg, 0);
            if (ret)
                aicx_warn(0x036185, "x", "[KEEPALIVE] delay notify modem release failed 0x%x\r\n", ret);
            else
                aicx_warn(0x1a7383, "", "[KEEPALIVE] delay notify modem release success\r\n");
        }

        aos_get_event(nic_dev->send_event,
                      ALTE_PING_EVENT | ALTE_FAST_REBUILD_EVENT, AOS_OR_CLEAR,
                      &actual_flags_ptr, 0);
    }
}
#endif

static void nx_alte_send_task(void *p)
{
    int i, ret;
    u32 old_priority;
    u32 actual_flags_ptr;
    AOS_INTERRUPT_DECLARE
    NX_PACKET *packet_ptr;
    u8 high_mem, normal_mem, link_id;
    struct alte_network_driver *nic_dev = p;
    struct alte_network_driver_cfg *nic_cfg;

    while (1) {
        aos_get_event(nic_dev->send_event, ALTE_SEND_EVENT, AOS_OR_CLEAR,
                      &actual_flags_ptr, AOS_WAIT_FOREVER);

        ret = aos_get_mutex(nic_dev->send_mutex, AOS_WAIT_FOREVER);
        if (ret != AOS_SUCCESS) {
            aicx_warn(0x045a30, "x", "[NIC] get send mutex fail 0x%x\n", ret);
            continue;
        }

        for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
            nic_cfg = &nic_dev->alte_nic_devs[i];
            high_mem = TRUE;
            normal_mem = TRUE;
            for ( ;; ) {
                if (nic_cfg->send_ack_packet_queue_head) {
                    AOS_INTERRUPT_DISABLE
                    link_id = (nic_cfg->netid << 1);
                    packet_ptr = nic_cfg->send_ack_packet_queue_head;
                    if (!packet_ptr->nx_packet_queue_next) {
                        nic_cfg->send_ack_packet_queue_head = NULL;
                        nic_cfg->send_ack_packet_queue_tail = NULL;
                    } else {
                        nic_cfg->send_ack_packet_queue_head = packet_ptr->nx_packet_queue_next;
                    }

                    nic_cfg->high_packet_queue_count--;
                    AOS_INTERRUPT_RESTORE
                    ret = _nx_alte_driver_send(packet_ptr, nic_cfg, TRUE, 0, TRUE);
                    if (ret == AIC_ENOMEM &&
                        (!nic_cfg->send_packet_queue_head || !normal_mem)) {
                        ret = _nx_alte_driver_send(packet_ptr, nic_cfg, TRUE, 500, TRUE);
                        if (ret == AIC_ENOMEM) {
                            nx_alte_reduce_statistics(nic_cfg, packet_ptr);
                            nx_packet_transmit_release(packet_ptr);
                        }
                    } else if (ret == AIC_ENOMEM &&
                               nic_cfg->send_packet_queue_head &&
                               ablk_get_send_left_size(nic_dev->aipc_send_handle,
                                                       link_id)) {
                        aicx_info(0x0bf9d7, "dx", "[NIC] send normal queue %d 0x%x",
                                  nic_cfg->high_packet_queue_count,
                                  nic_cfg->send_packet_queue_head);
                        AOS_INTERRUPT_DISABLE
                        packet_ptr->nx_packet_queue_next = nic_cfg->send_ack_packet_queue_head;
                        if (!nic_cfg->send_ack_packet_queue_head)
                            nic_cfg->send_ack_packet_queue_tail = packet_ptr;
                        nic_cfg->send_ack_packet_queue_head = packet_ptr;
                        nic_cfg->high_packet_queue_count++;
                        AOS_INTERRUPT_RESTORE
                        high_mem = FALSE;
                        goto SEND_NORMAL;
                    } else if (ret) {
                        if (ret == AIC_ENOMEM) {
                            nx_alte_reduce_statistics(nic_cfg, packet_ptr);
                            nx_packet_transmit_release(packet_ptr);
                        } else {
                            aicx_info(0x15013d, "xdd", "[NIC] send high packet fail 0x%x %d %d\n",
                                      ret, ablk_get_send_left_size(nic_dev->aipc_send_handle,
                                                                   (nic_cfg->netid << 1) + ALTE_HIGH),
                                      ablk_get_send_left_size(nic_dev->aipc_send_handle,
                                                              (nic_cfg->netid << 1)));
                        }
                    }

                    high_mem = TRUE;
                    continue;
                }

SEND_NORMAL:
                if (nic_cfg->send_packet_queue_head) {
                    AOS_INTERRUPT_DISABLE
                    link_id = (nic_cfg->netid << 1) + ALTE_HIGH;
                    packet_ptr = nic_cfg->send_packet_queue_head;
                    if (!packet_ptr->nx_packet_queue_next) {
                        nic_cfg->send_packet_queue_head = NULL;
                        nic_cfg->send_packet_queue_tail = NULL;
                    } else {
                        nic_cfg->send_packet_queue_head = packet_ptr->nx_packet_queue_next;
                    }

                    nic_cfg->normal_packet_queue_count--;
                    AOS_INTERRUPT_RESTORE
                    ret = _nx_alte_driver_send(packet_ptr, nic_cfg, FALSE, 0, TRUE);
                    if (ret == AIC_ENOMEM &&
                        (!nic_cfg->send_ack_packet_queue_head || !high_mem)) {
                        ret = _nx_alte_driver_send(packet_ptr, nic_cfg, FALSE,
                                                   500, TRUE);
                        if (ret == AIC_ENOMEM) {
                            nx_packet_transmit_release(packet_ptr);
                            nx_alte_reduce_statistics(nic_cfg, packet_ptr);
                        }
                    } else if (ret == AIC_ENOMEM &&
                               nic_cfg->send_ack_packet_queue_head &&
                               ablk_get_send_left_size(nic_dev->aipc_send_handle,
                                                       link_id)) {
                        aicx_info(0x0f8c68, "dx", "[NIC] send high queue %d 0x%x",
                                  nic_cfg->normal_packet_queue_count,
                                  nic_cfg->send_ack_packet_queue_head);
                        AOS_INTERRUPT_DISABLE
                        packet_ptr->nx_packet_queue_next = nic_cfg->send_packet_queue_head;
                        if (!nic_cfg->send_packet_queue_head)
                            nic_cfg->send_packet_queue_tail = packet_ptr;
                        nic_cfg->send_packet_queue_head = packet_ptr;
                        nic_cfg->normal_packet_queue_count++;
                        AOS_INTERRUPT_RESTORE
                        normal_mem = FALSE;
                        continue;
                    } else if (ret) {
                        if (ret == AIC_ENOMEM) {
                            nx_packet_transmit_release(packet_ptr);
                            nx_alte_reduce_statistics(nic_cfg, packet_ptr);
                        } else {
                            aicx_info(0x004f59, "xdd", "[NIC] send normal packet fail 0x%x %d %d\n",
                                      ret, ablk_get_send_left_size(nic_dev->aipc_send_handle,
                                                                   (nic_cfg->netid << 1) + ALTE_HIGH),
                                      ablk_get_send_left_size(nic_dev->aipc_send_handle,
                                                              (nic_cfg->netid << 1)));
                        }
                    }

                    normal_mem = TRUE;
                }

                if ((!nic_cfg->send_packet_queue_head &&
                     !nic_cfg->send_ack_packet_queue_head) ||
                    (!ablk_get_send_left_size(nic_dev->aipc_send_handle,
                                              (nic_cfg->netid << 1) + ALTE_HIGH) &&
                     !ablk_get_send_left_size(nic_dev->aipc_send_handle,
                                              (nic_cfg->netid << 1)))) {
                    aos_change_thread_pri(alte_nic_devices.send_thread_id,
                                          ALTE_TASK_PRIORITY, &old_priority);
                    break;
                }
            }
        }

        aos_put_mutex(nic_dev->send_mutex);
    }
}

static inline void nx_alte_nic_activate(struct alte_network_driver_cfg *nic_cfg)
{
    if (nic_cfg->nx_nic_interface_ptr) {
        if (nic_cfg->nx_nic_interface_ptr->nx_interface_link_up == NX_TRUE)
            nic_cfg->nx_nic_activated = NX_TRUE;
    } else {
        nic_cfg->nx_nic_activated = NX_FALSE;
    }
#if 0
    int ret;

    ret = ablk_create_link(alte_nic_devices.aipc_send_handle, nic_cfg->netid << 1,
                           16, 16);
    aicx_info(0x09d727, "xx", "ablk create link 0x%x 0x%x\n", nic_cfg->netid << 1, ret);
    ret = ablk_create_link(alte_nic_devices.aipc_send_handle,
                           (nic_cfg->netid << 1) + ALTE_HIGH,
                           16, 16);
    aicx_info(0x09d727, "xx", "ablk create link 0x%x 0x%x\n", (nic_cfg->netid << 1) + ALTE_HIGH, ret);
#endif
}

static inline void nx_alte_nic_deactivate(struct alte_network_driver_cfg *
                                          nic_cfg)
{
    nic_cfg->nx_nic_activated = NX_FALSE;
}

/**
 * _nx_alte_driver_attach - Bind the NIC to the specified IP instance.
 *
 * @nx_interface_ptr: The address interface structure.
 * @nx_ip_driver: The structure of the network card driver in NETX DUO.
 * @nic: Network card information.
 *
 * Bind the NIC to the specified IP instance.
 */
static inline void _nx_alte_driver_attach(NX_INTERFACE *nx_interface_ptr,
                                          NX_IP_DRIVER *nx_ip_driver,
                                          struct alte_network_driver_cfg *
                                          nic_cfg)
{
    int i;
    u8 netid;
    char *name;
    u8 byte_1 ,byte_2;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE

    for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
        if (!nic_cfg[i].nx_nic_interface_ptr)
            break;
    }

    if (i == ALTE_NIC_MAX_INSTANCES) {
        aicx_info(0x08a865, "", "[NIC] alte nic cfg is full\n");
        nx_ip_driver->nx_ip_driver_status = NX_NO_MORE_ENTRIES;
        return;
    } else {
        nic_cfg[i].nx_nic_ip_instance = nx_ip_driver->nx_ip_driver_ptr;
        nic_cfg[i].nx_nic_interface_ptr = nx_interface_ptr;
        nx_interface_ptr->nx_interface_additional_link_info =
            (void *)&nic_cfg[i];
    }

    nic_cfg = &nic_cfg[i];
    AOS_INTERRUPT_RESTORE

    if (!strcmp(nx_interface_ptr->nx_interface_name , "PRI")) {
        netid = 0;
    } else {
        name = nx_interface_ptr->nx_interface_name;
        byte_1 = nx_interface_ptr->nx_interface_name[strlen(name) - 2];
        byte_2 = nx_interface_ptr->nx_interface_name[strlen(name) - 1];
        aicx_debug(0x172d7b, "sdd", "[driver] %s %d %d\n", name, byte_1, byte_2);

        if (byte_1 >= '0' && byte_1 <= '9')
            netid = (byte_1 - 0x30) * 10;
        else
            netid = 0;

        if (byte_2 >= '0' && byte_2 <= '9') {
            netid += byte_2 - 0x30;
        } else {
            netid = 0xFF;
        }
    }

    if (netid >= ALTE_NIC_MAX_INSTANCES) {
        aicx_error(0x05f1d8, "sdd", "%s interface name is wrong %d %d\n",
                   nx_interface_ptr->nx_interface_name, byte_1, byte_2);
        return;
    }

    aicx_debug(0x08ce86, "sd", "[driver] %s netid %d\n",
               nx_interface_ptr->nx_interface_name, netid);
    nic_cfg->netid = netid;
    nic_cfg->s_set = BIT(netid << 1) | BIT((netid << 1) + ALTE_HIGH);
}

/**
 * _nx_alte_driver_init - Init network card driver.
 *
 * @nx_interface_ptr: The address interface structure.
 * @nx_ip_driver: The structure of the network card driver in NETX DUO.
 *
 * Init the parameters of the network card driver.
 */
static inline void _nx_alte_driver_init(NX_INTERFACE *nx_interface_ptr,
                                        NX_IP_DRIVER *nx_ip_driver)
{
    nx_interface_ptr->nx_interface_valid = NX_TRUE;
    nx_interface_ptr->nx_interface_address_mapping_needed = NX_FALSE;
    nx_interface_ptr->nx_interface_ip_mtu_size = ALTE_NX_ETHERNET_MTU -
        ALTE_NX_ETHERNET_SIZE;

    nx_interface_ptr->nx_interface_link_up = NX_FALSE;

    nx_interface_ptr->nx_interface_physical_address_msw = 0;
    nx_interface_ptr->nx_interface_physical_address_lsw = 0;
}

/**
 * _nx_alte_driver_enable - Enable network card driver.
 *
 * @nx_interface_ptr: The address interface structure.
 * @nx_ip_driver: The structure of the network card driver in NETX DUO.
 * @nic: Network card information.
 *
 * Set the network card driver to the enable state and request the resources.
 */
static void _nx_alte_driver_enable(NX_INTERFACE *nx_interface_ptr,
                                   NX_IP_DRIVER *nx_ip_driver,
                                   struct alte_network_driver_cfg *nic)
{
    struct alte_network_driver *net_dev = &alte_nic_devices;
    int link;

    nx_interface_ptr->nx_interface_link_up = NX_TRUE;

    for (link = 0; link < ablk_get_link_cnt(net_dev->aipc_send_handle, FALSE); link++) {
        if (ablk_get_a_link_state(net_dev->aipc_send_handle, link, AIC_FALSE) ==
            ALINK_STATE_LINKED)
            nx_alte_link_up_callback(link, ALINK_STATE_LINKED);
    }

    nx_alte_nic_activate(nic);
    nx_alte_create_thread();
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
    net_dev->monitor_timestamp = aic_get_tick();
#endif
}

static void _nx_alte_driver_release_packet(NX_PACKET *queue_head)
{
    NX_PACKET *packet_ptr, *next_packet_ptr;

    if (queue_head) {
        packet_ptr = queue_head;
        do {
            next_packet_ptr = packet_ptr->nx_packet_queue_next;
            if (!packet_ptr->nx_packet_reserved ||
                (packet_ptr->nx_packet_reserved & AIC_PACKET_DYNAMIC_MEM)) {
                alte_nic_devices.current_send_cache_size -=
                    (packet_ptr->nx_packet_data_end -
                     packet_ptr->nx_packet_data_start);
                if ((int)alte_nic_devices.current_send_cache_size < 0)
                    alte_nic_devices.current_send_cache_size = 0;
            }
            _nx_packet_transmit_release(packet_ptr);
            packet_ptr = next_packet_ptr;
        } while (packet_ptr);
    }
}

/**
 * _nx_alte_driver_disable - Disable network card driver.
 *
 * @nx_interface_ptr: The address interface structure.
 * @nx_ip_driver: The structure of the network card driver in NETX DUO.
 * @nic: Network card information.
 *
 * Set the network card driver to the disable state and release the resources it
 * holds.
 */
static void _nx_alte_driver_disable(NX_INTERFACE *nx_interface_ptr,
                                    NX_IP_DRIVER *nx_ip_driver,
                                    struct alte_network_driver_cfg *nic)
{
    AOS_INTERRUPT_DECLARE
    int ret;
    NX_IP *ip_ptr;
    struct alte_network_driver *net_dev = &alte_nic_devices;

    AOS_INTERRUPT_DISABLE
    nx_alte_nic_deactivate(nic);
    nx_interface_ptr->nx_interface_link_up = NX_FALSE;
    net_dev->thread_cnt--;
    AOS_INTERRUPT_RESTORE

    if (nic->nx_nic_ip_instance->ipv6_forward_release_packet)
        nic->nx_nic_ip_instance->ipv6_forward_release_packet(nx_interface_ptr->nx_interface_index);

    ret = aos_get_mutex(net_dev->send_mutex, AOS_WAIT_FOREVER);
    if (ret != AOS_SUCCESS)
        aicx_warn(0x0c6105, "x", "[NIC] get send mutex fail 0x%x when disable\n", ret);

    _nx_alte_driver_release_packet(nic->send_packet_queue_head);
    _nx_alte_driver_release_packet(nic->send_ack_packet_queue_head);

    if (ret == AOS_SUCCESS)
        aos_put_mutex(net_dev->send_mutex);

    AOS_INTERRUPT_DISABLE
    ip_ptr = nic->nx_nic_ip_instance;
    memset(nic, 0, sizeof(struct alte_network_driver_cfg));
    nic->nx_nic_ip_instance = ip_ptr;
    AOS_INTERRUPT_RESTORE

    nx_alte_destroy_thread();
}

/**
 * _nx_alte_driver_uninit - Set to uninitialized state.
 *
 * @nx_interface_ptr: The address interface structure.
 * @nx_ip_driver: The structure of the network card driver in NETX DUO.
 *
 */
static inline void _nx_alte_driver_uninit(NX_INTERFACE *nx_interface_ptr,
                                          NX_IP_DRIVER *nx_ip_driver)
{
    nx_interface_ptr->nx_interface_valid = NX_FALSE;
}

/**
 * _nx_alte_driver_unhandle - Handle commands not recognized by the current
 *                            driver.
 *
 * @nx_ip_driver: The structure of the network card driver in NETX DUO.
 *
 */
static inline void _nx_alte_driver_unhandle(NX_IP_DRIVER *nx_ip_driver)
{
    nx_ip_driver->nx_ip_driver_status = NX_UNHANDLED_COMMAND;

    if (nx_ip_driver->nx_ip_driver_return_ptr)
        *(nx_ip_driver->nx_ip_driver_return_ptr) = (ulong)0;
}

static void alte_pdp_state_change_cb(u8 pdp_cid, u8 pdp_state)
{
    u8 ori_active;
    struct alte_network_driver *nic_dev = &alte_nic_devices;

    if (pdp_cid > ANET_MAX_PDP_CID) {
        aicx_warn(0x083202, "d", "[NIC] pdp cid %d is invalid\r\n", pdp_cid);
        return;
    }

    aicx_info(0x1cc285, "ddx", "[NIC] cid %d pdp state %d active flag 0x%x\r\n",
              pdp_cid, pdp_state, nic_dev->pdp_active_flag);
    if (pdp_state == ANET_PDP_ACTIVATED) {
#ifdef ANET_ENABLE_DONGLE_NETWORK
        AMSG_T msg;

        if (!nic_dev->dongle_notify_flag) {
            nic_dev->dongle_notify_flag = TRUE;
            aicx_info(0x10a54b, "", "[NIC] notify modem dongle mode\n");
            amsg_fill_msg(&msg, AMOD_EL2, AIC_NOTIFY_MODEM_DONGLE,
                          AIC_NOTIFY_MODEM_DONGLE);
            amsg_send_msg(AIC_SYS_CP, &msg, 0);
        }
#endif
        ori_active = nic_dev->pdp_active_flag;
        nic_dev->pdp_active_flag |= BIT(pdp_cid - 1);
        if (!ori_active) {
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
            nic_dev->flight_timestamp = aic_get_tick();
#endif
#ifndef CONFIG_NET_VIRT_NIC
            if (!nic_dev->network_abnormal_notify)
                aos_set_event(nic_dev->send_event, ALTE_NETWORK_ACCESS_EVENT,
                              AOS_OR);
#endif
        }
    } else if (pdp_state == ANET_PDP_DEACTIVATED) {
        nic_dev->pdp_active_flag &= ~BIT(pdp_cid - 1);
        if (!nic_dev->pdp_active_flag) {
            nic_dev->network_access_retry_cnt = 0;
            nic_dev->network_access_status = 0;
        }
    } else {
        aicx_warn(0x1468a5, "d", "[NIC] pdp state %d is invalid\r\n", pdp_state);
    }
}

void nx_alte_notify_modem_release()
{
    aicx_info(0x02a822, "", "[NIC] set ALTE_NETWORK_NOTIFY_RELEASE event\r\n");
    aos_set_event(alte_nic_devices.send_event, ALTE_NETWORK_NOTIFY_RELEASE,
                  AOS_OR);
}

int nx_alte_check_cached_share_mem(void)
{
    int i;

    for (i = 0 ; i < ALTE_NIC_MAX_INSTANCES; i++) {
        if (alte_nic_devices.alte_nic_devs[i].nx_nic_activated &&
            alte_nic_devices.alte_nic_devs[i].heap_packet_queue_count)
            return AIC_TRUE;
    }

    return AIC_FALSE;
}

void *nx_alte_alloc_block(u32 len, int timeout)
{
    int ret;
    ABLK_T blk = {0};

    if (alte_nic_devices.aipc_send_handle) {
        ret = ablk_get_block(alte_nic_devices.aipc_send_handle,
                             len + ALTE_BLOCK_TAIL_OFFSET, &blk, timeout);
        if (ret && timeout) {
            aicx_error(0x049508, "xdd", "alloc aipc block fail 0x%x len %d timeout %d\n",
                       ret, len, timeout);
            return NULL;
        }
    } else {
        ret = AIC_EPERM;
        aicx_info(0x1a4521, "", "[DRIVER] alte send handle is null\r\n");
    }

    return blk.addr;
}

/**
 * nx_alte_release_block - Release AIPC share memory.
 *
 * @addr: AIPC share memory first address.
 *
 * Because build NX_PACKET mechanism is currently used, AIPC interface is
 * required to release its memory.
 */
int nx_alte_release_block(u8 *addr, u32 size)
{
    int ret = AIC_EACCES;
    ABLK_T blk;

    blk.addr = addr;
    blk.size = size;
    if (alte_nic_devices.aipc_recv_handle && alte_nic_devices.aipc_send_handle) {
        if (ablk_is_brr_blk(alte_nic_devices.aipc_recv_handle, addr, TRUE)) {
            ablk_release_block(alte_nic_devices.aipc_recv_handle, &blk);
            ret = AIC_SUCCESS;
        } else if (ablk_is_brr_blk(alte_nic_devices.aipc_send_handle,
                                   addr, FALSE)) {
            ablk_recycle_block(alte_nic_devices.aipc_send_handle, &blk);
            ret = AIC_SUCCESS;
        } else {
            aicx_error(0x01b861, "p", "can't support this addr %p\n", addr);
            ret = AIC_EINVAL;
        }
    } else {
           ret = AIC_EPERM;
           aicx_info(0x0fe0c3, "xx", "[DRIVER] alte send or recv handle is null 0x%x 0x%x\r\n",
                     alte_nic_devices.aipc_recv_handle,
                     alte_nic_devices.aipc_send_handle);
    }

    return ret;
}

/**
 * nx_alte_is_brr_block - Check whether the address is share memory.
 *
 * @addr: First address.
 */
bool nx_alte_is_brr_block(u8 *addr)
{
    int i;
    bool flag = FALSE;
    struct alte_network_driver_cfg *nic_cfg =
        &alte_nic_devices.alte_nic_devs[0];

    for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
        if (nic_cfg[i].nx_nic_activated) {
            flag = ablk_is_brr_blk(alte_nic_devices.aipc_recv_handle,
                                   addr, TRUE);
            break;
        }
    }

    return flag;
}

static int nx_alte_check_cached(NX_PACKET *packet_ptr,
                                struct alte_network_driver_cfg *nic_cfg,
                                bool is_high)
{
    u8 protocol;
    u32 total_heap_size = 0, used_heap_size = 0, free_heap_size = 0, ret;

    if (packet_ptr->nx_packet_reserved & AIC_PACKET_SHARE_MEM)
        return AIC_TRUE;

    ret = aos_get_heap_usage(&total_heap_size, &used_heap_size);
    if (ret) {
        total_heap_size = 0;
        used_heap_size = 0;
        free_heap_size = 0;
    } else {
        free_heap_size = total_heap_size - used_heap_size;
    }

    if (free_heap_size && free_heap_size < ALTE_HEAP_LOW_WATER)
        aicx_warn(0x12e3b6, "ddd", "[NIC] free heap is low water %d total %d used %d\n",
                  free_heap_size, total_heap_size, used_heap_size);

    aic_net_get_trans_params(packet_ptr, &protocol, NULL, NULL, NULL);
    if (protocol == NX_PROTOCOL_ICMP || protocol == NX_PROTOCOL_ICMPV6)
        return AIC_TRUE;

    if ((alte_nic_devices.current_send_cache_size >
         alte_nic_devices.max_send_cache_size) ||
        (is_high && nic_cfg->high_packet_queue_count > ALTE_MAX_HIGH_QUEUE_SIZE) ||
        (!is_high && nic_cfg->normal_packet_queue_count > ALTE_MAX_NORMAL_QUEUE_SIZE) ||
        (free_heap_size && free_heap_size < ALTE_HEAP_LOW_WATER && is_high &&
         nic_cfg->high_packet_queue_count > ALTE_LOW_HIGH_QUEUE_SIZE) ||
        (free_heap_size && free_heap_size < ALTE_HEAP_LOW_WATER && !is_high &&
         nic_cfg->normal_packet_queue_count > ALTE_LOW_NORMAL_QUEUE_SIZE))
        return AIC_FALSE;

    return AIC_TRUE;
}

/**
 * nx_alte_nic_driver_entry - NIC driver entry function.
 *
 * @nx_ip_driver: NIC driver structure.
 *
 * Handle the enable, disable, attach, detach, packet sending of the network
 * card, and get parameters such as network card sending and receiving packets.
 */
void nx_alte_nic_driver_entry(NX_IP_DRIVER *nx_ip_driver)
{
    int ret;
    bool is_high;
    u32 old_priority;
    NX_PACKET *packet;
    AOS_INTERRUPT_DECLARE
    NX_INTERFACE *nx_interface_ptr;
    u8 normal_flag = 0, high_flag = 0;
    struct alte_network_driver_cfg *nic_cfg =
        &alte_nic_devices.alte_nic_devs[0];

    nx_ip_driver->nx_ip_driver_status = NX_SUCCESS;
    packet = nx_ip_driver->nx_ip_driver_packet;
    if (nx_ip_driver->nx_ip_driver_interface) {
        nx_interface_ptr = nx_ip_driver->nx_ip_driver_interface;
    } else if (packet &&
               packet->nx_packet_address.nx_packet_interface_ptr &&
               packet->nx_packet_ip_version == NX_IP_VERSION_V6) {
        struct NXD_IPV6_ADDRESS_STRUCT *ipv6_address_ptr;
        ipv6_address_ptr = packet->nx_packet_address.nx_packet_ipv6_address_ptr;
        nx_interface_ptr = ipv6_address_ptr->nxd_ipv6_address_attached;
    } else {
        aicx_error(0x1c05b3, "", "[driver] alte interface ptr is null\n");
        if (nx_ip_driver->nx_ip_driver_command == NX_LINK_PACKET_SEND ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_PACKET_BROADCAST ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_ARP_SEND ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_ARP_RESPONSE_SEND ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_RARP_SEND ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_PACKET_DIRECT_SEND) {
            if (nx_ip_driver->nx_ip_driver_packet) {
                _nx_packet_transmit_release(nx_ip_driver->nx_ip_driver_packet);
            }
        }

        nx_ip_driver->nx_ip_driver_status =  NX_NOT_SUCCESSFUL;

        return;
    }

    if (nx_ip_driver->nx_ip_driver_command == NX_LINK_INTERFACE_ATTACH) {
        _nx_alte_driver_attach(nx_interface_ptr, nx_ip_driver, nic_cfg);
        return;
    }

    nic_cfg = nx_interface_ptr->nx_interface_additional_link_info;
    if (!nic_cfg) {
        aicx_info(0x1d77d2, "", "This IP instance maybe not attached\n");
        if (nx_ip_driver->nx_ip_driver_command == NX_LINK_PACKET_SEND ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_PACKET_BROADCAST ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_ARP_SEND ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_ARP_RESPONSE_SEND ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_RARP_SEND ||
            nx_ip_driver->nx_ip_driver_command == NX_LINK_PACKET_DIRECT_SEND) {
            if (nx_ip_driver->nx_ip_driver_packet) {
                _nx_packet_transmit_release(nx_ip_driver->nx_ip_driver_packet);
            }
        }

        nx_ip_driver->nx_ip_driver_status =  NX_NOT_SUCCESSFUL;

        return;
    }

    switch (nx_ip_driver->nx_ip_driver_command) {
    case NX_LINK_INITIALIZE:
        _nx_alte_driver_init(nx_interface_ptr, nx_ip_driver);
        break;
    case NX_LINK_ENABLE:
        aicx_info(0x1228da, "s", "%s enable\n", nx_interface_ptr->nx_interface_name);
        _nx_alte_driver_enable(nx_interface_ptr, nx_ip_driver, nic_cfg);
        break;
    case NX_LINK_DISABLE:
    case NX_LINK_INTERFACE_DETACH:
        aicx_info(0x1d5b22, "s", "[ALTE] interface %s disable\r\n",
                  nx_interface_ptr->nx_interface_name);
        nx_interface_ptr->nx_interface_additional_link_info = NULL;
        if (nx_interface_ptr->nx_interface_link_up)
            _nx_alte_driver_disable(nx_interface_ptr, nx_ip_driver, nic_cfg);
        nic_cfg->nx_nic_interface_ptr = NULL;
        break;
    case NX_LINK_PACKET_SEND:
    case NX_LINK_PACKET_BROADCAST:
    case NX_LINK_ARP_SEND:
    case NX_LINK_ARP_RESPONSE_SEND:
    case NX_LINK_RARP_SEND:
    case NX_LINK_PACKET_DIRECT_SEND:
        AOS_INTERRUPT_DISABLE
        if (nic_cfg->nx_nic_interface_ptr) {
            nic_cfg->nx_nic_interface_ptr->nx_interface_packets_sent++;
            nic_cfg->nx_nic_interface_ptr->nx_interface_bytes_sent +=
                nx_ip_driver->nx_ip_driver_packet->nx_packet_length;
        }

        if (!nic_cfg->nx_nic_interface_ptr->nx_ipv6_rtr_solicitation_max) {
            nic_cfg->nx_nic_interface_ptr->nx_ipv6_stateless_address_autoconfig_status =
                NX_STATELESS_ADDRESS_AUTOCONFIG_DISABLED;
            _nxd_ipv6_stateless_address_autoconfig_enable(nic_cfg->nx_nic_ip_instance,
                                                          nic_cfg->nx_nic_interface_ptr->nx_interface_index);
        }

        is_high = aic_identify_packet_pri(nx_ip_driver->nx_ip_driver_packet, NULL);
        if (nx_alte_check_cached(nx_ip_driver->nx_ip_driver_packet, nic_cfg, is_high)) {
            if (nic_cfg->nx_nic_activated) {
                AIC_ASSERT(!nx_ip_driver->nx_ip_driver_packet->nx_packet_queue_next);
                if (is_high) {
                    AOS_INTERRUPT_RESTORE
                    if (!nic_cfg->send_ack_packet_queue_head) {
                        ret = _nx_alte_driver_send(nx_ip_driver->nx_ip_driver_packet,
                                                   nic_cfg, TRUE, 0, FALSE);
                        if (ret != AIC_ENOMEM)
                            return;
                    }
                    aicx_warn(0x0d6da1, "", "[NIC] send high packet directly fail\n");
                    AOS_INTERRUPT_DISABLE
                    if (!nic_cfg->send_ack_packet_queue_head) {
                        nic_cfg->send_ack_packet_queue_head =
                            nx_ip_driver->nx_ip_driver_packet;
                        nic_cfg->send_ack_packet_queue_tail =
                            nx_ip_driver->nx_ip_driver_packet;
                        high_flag = AIC_TRUE;
                    } else {
                        nic_cfg->send_ack_packet_queue_tail->nx_packet_queue_next =
                            nx_ip_driver->nx_ip_driver_packet;
                        nic_cfg->send_ack_packet_queue_tail =
                            nx_ip_driver->nx_ip_driver_packet;
                    }

                    nic_cfg->high_packet_queue_count++;
                } else {
                    if (!nic_cfg->send_packet_queue_head) {
                        nic_cfg->send_packet_queue_head =
                            nx_ip_driver->nx_ip_driver_packet;
                        nic_cfg->send_packet_queue_tail =
                            nx_ip_driver->nx_ip_driver_packet;
                        normal_flag = AIC_TRUE;
                    } else {
                        nic_cfg->send_packet_queue_tail->nx_packet_queue_next =
                            nx_ip_driver->nx_ip_driver_packet;
                        nic_cfg->send_packet_queue_tail =
                            nx_ip_driver->nx_ip_driver_packet;
                    }

                    nic_cfg->normal_packet_queue_count++;
                }

                nic_cfg->packet_queue_count++;
                if (nx_ip_driver->nx_ip_driver_packet->nx_packet_reserved &
                    AIC_PACKET_SHARE_MEM) {
                    nic_cfg->share_packet_queue_count++;
                } else if (nx_ip_driver->nx_ip_driver_packet->nx_packet_reserved &
                           AIC_PACKET_DYNAMIC_MEM) {
                    nic_cfg->heap_packet_queue_count++;
                    alte_nic_devices.current_send_cache_size +=
                        (nx_ip_driver->nx_ip_driver_packet->nx_packet_data_end -
                         nx_ip_driver->nx_ip_driver_packet->nx_packet_data_start);
                } else {
                    nic_cfg->nx_packet_queue_count++;
                    alte_nic_devices.current_send_cache_size +=
                        (nx_ip_driver->nx_ip_driver_packet->nx_packet_data_end -
                         nx_ip_driver->nx_ip_driver_packet->nx_packet_data_start);
                }
                AOS_INTERRUPT_RESTORE

                if (high_flag || nic_cfg->heap_packet_queue_count ||
                    (nic_cfg->packet_queue_count > 20)) {
                    aos_change_thread_pri(alte_nic_devices.send_thread_id,
                                          ALTE_TASK_HIGH_PRIORITY,
                                          &old_priority);
                    normal_flag = TRUE;
                }

                if (normal_flag || high_flag)
                    aos_set_event(alte_nic_devices.send_event, ALTE_SEND_EVENT,
                                  AOS_OR);

                normal_flag = 0;
                high_flag = 0;
            } else {
                AOS_INTERRUPT_RESTORE
                _nx_packet_transmit_release(nx_ip_driver->nx_ip_driver_packet);
            }
        } else {
            AOS_INTERRUPT_RESTORE
            aicx_warn(0x1dc0b1, "dddddddxxdd", "netid %d psq too big %d share %d heap %d nx %d %d %d 0x%x 0x%x size %d, cache size %d\r\n",
                      nic_cfg->netid, nic_cfg->packet_queue_count,
                      nic_cfg->share_packet_queue_count,
                      nic_cfg->heap_packet_queue_count,
                      nic_cfg->nx_packet_queue_count,
                      nic_cfg->high_packet_queue_count,
                      nic_cfg->normal_packet_queue_count,
                      nic_cfg->send_ack_packet_queue_head,
                      nic_cfg->send_packet_queue_head,
                      nx_ip_driver->nx_ip_driver_packet->nx_packet_length,
                      alte_nic_devices.current_send_cache_size);
            _nx_packet_transmit_release(nx_ip_driver->nx_ip_driver_packet);
        }

        break;
    case NX_LINK_UNINITIALIZE:
        _nx_alte_driver_uninit(nx_interface_ptr, nx_ip_driver);
        break;
    case NX_LINK_MULTICAST_JOIN:
    case NX_LINK_MULTICAST_LEAVE:
        break;
    case NX_LINK_GET_ALLOC_ERRORS:
    case NX_LINK_GET_STATUS:
    case NX_LINK_GET_RX_COUNT:
    case NX_LINK_GET_TX_COUNT:
    case NX_LINK_GET_SPEED:
    case NX_LINK_GET_DUPLEX_TYPE:
    case NX_LINK_USER_COMMAND:
    default:
        _nx_alte_driver_unhandle(nx_ip_driver);
        break;
    }

    return;
}

#ifdef CONFIG_PLAT_CONSOLE
static int aic_alte_test_cmd(int argc, char **argv)
{
    int i;
    struct alte_network_driver *nic_dev = &alte_nic_devices;

    for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
        if (alte_nic_devices.alte_nic_devs[i].nx_nic_activated) {
            CONS_OUT("%s nx pqc %d, netid %d s_set 0x%x\r\n",
                     alte_nic_devices.alte_nic_devs[i].nx_nic_interface_ptr->nx_interface_name,
                     alte_nic_devices.alte_nic_devs[i].packet_queue_count,
                     alte_nic_devices.alte_nic_devs[i].netid,
                     alte_nic_devices.alte_nic_devs[i].s_set);
        }
    }

    CONS_OUT("max size %d current size %d link 0x%x %d\r\n",
             nic_dev->max_send_cache_size,
             nic_dev->current_send_cache_size,
             nic_dev->s_link_up_status, nic_dev->disable_copy);

    return 0;
}
CONSOLE_EXPORT_CMD(alte_test, aic_alte_test_cmd, alte test);

#endif

void aic_network_setup_enhance_low_power(u32 enable, u32 idle_time,
                                         u32 retry_time)
{
    struct alte_network_driver *dev = &alte_nic_devices;

    dev->enhance_timer_enable = enable;
    dev->enhance_idle_time = idle_time * 1000;
    dev->enhance_retry_time = retry_time * 1000;
    dev->enhance_send_ok = AIC_FALSE;
    dev->enhance_stage = AIC_NETWORK_ENHANCE_IDLE_TIME_STAGE;

    aos_deactivate_timer(dev->enhance_timer);
}

static int aic_network_check_tcp_unack(void)
{
    AOS_INTERRUPT_DECLARE
    NX_TCP_SOCKET *head_socket_ptr, *tcp_socket_ptr;
    struct aic_network_ip_instance *aic_ip;

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr)
        return AIC_FALSE;

    AOS_INTERRUPT_DISABLE
    head_socket_ptr = aic_ip->ip_ptr->nx_ip_tcp_created_sockets_ptr;
    tcp_socket_ptr = aic_ip->ip_ptr->nx_ip_tcp_created_sockets_ptr;
    while (tcp_socket_ptr) {
        if (tcp_socket_ptr->nx_tcp_socket_tx_outstanding_bytes) {
            aicx_info(0x119e95, "ddd", "[NIC] socket port %d %d have tx outstanding bytes %d\n",
                      tcp_socket_ptr->nx_tcp_socket_port,
                      tcp_socket_ptr->nx_tcp_socket_connect_port,
                      tcp_socket_ptr->nx_tcp_socket_tx_outstanding_bytes);
            AOS_INTERRUPT_RESTORE
            return AIC_TRUE;
        }

        tcp_socket_ptr = tcp_socket_ptr->nx_tcp_socket_created_next;
        if (tcp_socket_ptr == head_socket_ptr)
            break;
    }

    AOS_INTERRUPT_RESTORE

    return AIC_FALSE;
}

static void aic_network_enhance_timer_callback(void *pri)
{
    int ret;
    AMSG_T msg;
    struct alte_network_driver *dev = pri;

    aicx_info(0x010642, "dd", "[NIC] Enhance timer send_ok %d stage %d\n",
              dev->enhance_send_ok, dev->enhance_stage);
    if (dev->enhance_timer_enable) {
        if ((dev->enhance_send_ok && !aic_network_check_tcp_unack()) ||
            dev->enhance_stage == AIC_NETWORK_ENHANCE_RETRY_TIME_STAGE) {
            amsg_fill_msg(&msg, AMOD_EL2, AIC_NOTIFY_MODEM_RELEASE,
                          AIC_NOTIFY_MODEM_RELEASE);
            ret = amsg_send_msg(AIC_SYS_CP, &msg, 0);
            if (ret)
                aicx_warn(0x1a0124, "x", "[NIC] Enhance notify modem release failed 0x%x\r\n",
                          ret);
            else
                aicx_warn(0x00b05b, "", "[NIC] Enhance notify modem release success\r\n");
        } else {
            if (dev->enhance_stage == AIC_NETWORK_ENHANCE_IDLE_TIME_STAGE) {
                dev->enhance_stage = AIC_NETWORK_ENHANCE_RETRY_TIME_STAGE;
                if (dev->enhance_retry_time > dev->enhance_idle_time)
                    aos_change_timer(dev->enhance_timer,
                                     dev->enhance_retry_time - dev->enhance_idle_time,
                                     AOS_NO_PERIOD);
            }
        }
    }
}
#if 0
static void aic_network_cfg_tc_by_duplex_mode(void)
{
    int i;
    struct alte_network_driver *dev = &alte_nic_devices;

    for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
        if (dev->alte_nic_devs[i].nx_nic_activated) {
            aicx_info(0x06b6e9, "dd", "[NIC] cfg tc netid %d duplex mode %d\n",
                      dev->alte_nic_devs[i].netid, dev->modem_duplex_mode);
            if (dev->modem_duplex_mode == AIC_MODEM_FDD)
                aic_network_traffic_control_enable(dev->alte_nic_devs[i].netid + 1,
                                                   10020000, 0);
            else if (dev->modem_duplex_mode == AIC_MODEM_TDD)
                aic_network_traffic_control_enable(dev->alte_nic_devs[i].netid + 1,
                                                   8020000, 0);
        }
    }
}
#endif

#ifdef CONFIG_D2D

void aic_cfg_d2d_send_timeout(u32 timeout)
{
    alte_nic_devices.d2d_send_timeout = timeout;
}

int aic_lte_nic_send(NX_PACKET *packet_ptr)
{
    int i, ret;
    u32 timeout;
    u8 tmp_str[8];
    struct alte_network_driver *dev = &alte_nic_devices;

    for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
        if (packet_ptr->nx_packet_ip_interface ==
            dev->alte_nic_devs[i].nx_nic_interface_ptr)
            break;
    }

    if (i >= ALTE_NIC_MAX_INSTANCES) {
        nx_packet_transmit_release(packet_ptr);
        aicx_warn(0x1422fe, "", "[NIC] find interface fail when hw send\n");
        return ENXIO;
    }

    memset(tmp_str, 0, 8);
    memcpy(tmp_str, packet_ptr->nx_packet_prepend_ptr, 6);
    if (strcmp("RANDOM", tmp_str))
        timeout = alte_nic_devices.d2d_send_timeout;
    else
        timeout = 2000;

    ret = _nx_alte_driver_send(packet_ptr, &dev->alte_nic_devs[i], 0, timeout,
                               FALSE);
    if (ret == AIC_ENOMEM)
        nx_packet_transmit_release(packet_ptr);

    return ret;
}
#endif

int aic_network_msg_handle(AMSG_T *amsg, void *p)
{
    int socket_id;
    u16 src_port, length;
    struct alte_network_driver *dev = p;

    aicx_info(0x0f849d, "dddx", "[NIC] msg ty %d id %d p1 %d p2 0x%x\n",
              amsg->ty, amsg->id, amsg->p1, amsg->p2);

#ifdef CONFIG_D2D
    extern void aic_d2d_set_bw_mode(u8 mode);
#endif

    switch (amsg->p1) {
    case AIC_NETWORK_MODEM_SEND_OK:
        if (dev->enhance_timer_enable)
            dev->enhance_send_ok = AIC_TRUE;
        length = (u16)(amsg->p2 >> 16);
        src_port = (u16)(amsg->p2);
        socket_id = nx_bsd_find_udp_socket_by_port(src_port);
        if (socket_id != -1)
            nx_bsd_notify_udp_socket_ack(socket_id, length);
        break;
    case AIC_NETWORK_MODEM_SEND_FAIL:
        dev->enhance_send_ok = AIC_FALSE;
        break;
    case AIC_NETWORK_DUPLEX_MODE:
        dev->modem_duplex_mode = amsg->p2;
#if 0
        if (amsg->p2 == AIC_MODEM_TDD || amsg->p2 == AIC_MODEM_FDD) {
            aic_network_cfg_tc_by_duplex_mode();
        } else {
            aicx_info(0x09675e, "x", "[NIC] get modem duplex mode fail 0x%x\n", amsg->p2);
        }
#endif
        break;
    case AIC_NETWORK_IMS_FLUSH:
        aos_set_event(dev->send_event, ALTE_NETWORK_SEND_ONLY, AOS_OR);
        break;
#ifdef CONFIG_D2D
    case AIC_NETWORK_D2D_BW:
        aic_d2d_set_bw_mode(amsg->p2);
        break;
#endif
    default:
        break;
    }

    return AIC_SUCCESS;
}

void aic_network_register_abnormal_notify(void (*cb)(u32))
{
    alte_nic_devices.network_abnormal_notify = cb;
}

void aic_network_trigger_rebuild(void)
{
    AMSG_T msg;

    amsg_fill_msg(&msg, AMOD_EL2, ALTE_NOTIFY_L2_REBUILD,
                  ALTE_NOTIFY_L2_REBUILD);
    amsg_send_msg(AIC_SYS_CP, &msg, 0);
}

#ifdef CONFIG_D2D
void aic_set_d2d_flow_ctrl_report(u32 enable)
{
    struct alte_network_driver *dev = &alte_nic_devices;

    dev->enable_d2d_fc = enable;
}
#endif

void aic_set_lte_nic_work_mode(u32 mode)
{
    int i;
    struct alte_network_driver *dev = &alte_nic_devices;

    dev->work_mode = mode;

    if (mode == AIC_LTE_AT_SOCKET_MODE) {
        for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
            if (dev->alte_nic_devs[i].nx_nic_interface_ptr &&
                dev->alte_nic_devs[i].nx_nic_interface_ptr->nx_interface_valid) {
                dev->alte_nic_devs[i].nx_nic_interface_ptr->nx_ipv6_rtr_solicitation_count =
                    dev->alte_nic_devs[i].nx_nic_interface_ptr->nx_ipv6_rtr_solicitation_max;
            }
        }

        if (dev->pdp_active_flag) {
            if (!dev->network_abnormal_notify) {
                aos_set_event(dev->send_event, ALTE_NETWORK_ACCESS_EVENT,
                              AOS_OR);
            }
        }
    }
}

void aic_net_set_disable_copy()
{
    struct alte_network_driver *dev = &alte_nic_devices;

    dev->disable_copy = TRUE;
}

#ifdef CONFIG_NX_NAT
int nx_alte_check_cache_threshold(void)
{
    u8 i;
    u32 heap_packet_queue_count = 0;
    struct alte_network_driver_cfg *nic_cfg;
    u32 total_heap_size = 0, used_heap_size = 0, free_heap_size = 0, ret, cache_limit, queue_limit;

    extern UINT nx_nat_ul_win;

    for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
        nic_cfg = &alte_nic_devices.alte_nic_devs[i];
        heap_packet_queue_count += nic_cfg->heap_packet_queue_count;
    }

    ret = aos_get_heap_usage(&total_heap_size, &used_heap_size);
    if (ret) {
        total_heap_size = 0;
        used_heap_size = 0;
        free_heap_size = 0;
    } else {
        free_heap_size = total_heap_size - used_heap_size;
    }

    if (free_heap_size && free_heap_size < ALTE_HEAP_LOW_WATER) {
        aicx_warn(0x17650f, "ddd", "[NAT] free heap is low water %d total %d used %d max win 4096\n",
                  free_heap_size, total_heap_size, used_heap_size);
        return 4096;
    }

    cache_limit = (alte_nic_devices.max_send_cache_size / 10) * 8;
    queue_limit = (ALTE_MAX_NORMAL_QUEUE_SIZE / 10) * 8;
    if ((alte_nic_devices.current_send_cache_size > cache_limit) ||
        (heap_packet_queue_count > queue_limit)) {
        aicx_warn(0x030e93, "ddddd", "[NAT] curr cache %d max cache %d heap count %d max win 0 cache_limit %d queue_limit %d\n",
                  alte_nic_devices.current_send_cache_size,
                  alte_nic_devices.max_send_cache_size,
                  heap_packet_queue_count, cache_limit, queue_limit);
        return 0;
    }

    cache_limit = (alte_nic_devices.max_send_cache_size / 10) * 7;
    queue_limit = (ALTE_MAX_NORMAL_QUEUE_SIZE / 10) * 7;
    if ((alte_nic_devices.current_send_cache_size > cache_limit) ||
        (heap_packet_queue_count > queue_limit)) {
        aicx_debug(0x131f30, "ddddd", "[NAT] curr cache %d max cache %d heap count %d cache_limit %d queue_limit %d\n",
                  alte_nic_devices.current_send_cache_size,
                  alte_nic_devices.max_send_cache_size,
                  heap_packet_queue_count, cache_limit, queue_limit);
        return 2048;
    }

    cache_limit = (alte_nic_devices.max_send_cache_size / 10) * 6;
    queue_limit = (ALTE_MAX_NORMAL_QUEUE_SIZE / 10) * 6;
    if ((alte_nic_devices.current_send_cache_size > cache_limit) ||
        (heap_packet_queue_count > queue_limit)) {
        aicx_debug(0x079760, "ddddd", "[NAT] curr cache %d max cache %d heap count %d max win 4096 cache_limit %d queue_limit %d\n",
                  alte_nic_devices.current_send_cache_size,
                  alte_nic_devices.max_send_cache_size,
                  heap_packet_queue_count, cache_limit, queue_limit);
        return 4096;
    }

    cache_limit = (alte_nic_devices.max_send_cache_size / 10) * 4;
    queue_limit = (ALTE_MAX_NORMAL_QUEUE_SIZE / 10) * 4;
    if ((alte_nic_devices.current_send_cache_size > cache_limit) ||
        (heap_packet_queue_count > queue_limit)) {
        aicx_debug(0x1b5ced, "ddddd", "[NAT] curr cache %d max cache %d heap count %d max win 8192 cache_limit %d queue_limit %d\n",
                  alte_nic_devices.current_send_cache_size,
                  alte_nic_devices.max_send_cache_size,
                  heap_packet_queue_count, cache_limit, queue_limit);
        return 8192;
    }

    cache_limit = (alte_nic_devices.max_send_cache_size / 10) * 2;
    queue_limit = (ALTE_MAX_NORMAL_QUEUE_SIZE / 10) * 2;
    if ((alte_nic_devices.current_send_cache_size > cache_limit) ||
        (heap_packet_queue_count > queue_limit)) {
        aicx_debug(0x184031, "ddddd", "[NAT] curr cache %d max cache %d heap count %d max win 16384 cache_limit %d queue_limit %d\n",
                  alte_nic_devices.current_send_cache_size,
                  alte_nic_devices.max_send_cache_size,
                  heap_packet_queue_count, cache_limit, queue_limit);
        return 16384;
    }

    return nx_nat_ul_win;
}
#endif
/**
 * nx_alte_nic_init - Initialize the configuration information of the network
 *                    card.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int nx_alte_nic_init(void)
{
    int status= AIC_SUCCESS;

    memset(&alte_nic_devices, 0, sizeof(alte_nic_devices));

    alte_nic_devices.send_event = aos_create_event("alte send event");
    if (alte_nic_devices.send_event == AOS_NULL) {
        aicx_error(0x01602c, "", "[LTE] create alte event fail\n");

        return AIC_EPERM;
    }

    alte_nic_devices.aipc_recv_handle = ablk_request_brr(AIC_SYS_CP,
                                                         AIPC_BRR_IP);
    if (!alte_nic_devices.aipc_recv_handle) {
        aicx_error(0x1e72f9, "", "[driver] request recv brr fail\n");
        aos_delete_event(alte_nic_devices.send_event);
        return AIC_EPERM;
    }

    alte_nic_devices.aipc_send_handle = ablk_request_brr(AIC_SYS_CP,
                                                         AIPC_BRR_IP);
    if (!alte_nic_devices.aipc_send_handle) {
        aicx_error(0x011251, "", "[driver] request send brr fail\n");
        aos_delete_event(alte_nic_devices.send_event);
        return AIC_EPERM;
    }

    alte_nic_devices.s_set = (1 << (ALTE_NIC_MAX_INSTANCES * 2)) - 1;
    ablk_listen_a_blk(alte_nic_devices.aipc_send_handle, nx_alte_link_up_callback);

    alte_nic_devices.send_thread_id =
        aos_create_thread("alte-send", nx_alte_send_task,
                          (void *)&alte_nic_devices, SIZE_1K, 0, NULL,
                          ALTE_TASK_PRIORITY,
                          AOS_AUTO_START);
    if (alte_nic_devices.send_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x0c076b, "", "[driver] create lte send task fail\n");
        aos_delete_event(alte_nic_devices.send_event);

        return AIC_EPERM;
    }

    alte_nic_devices.recv_thread_id =
        aos_create_thread("alte-recv", nx_alte_recv_task,
                          (void *)&alte_nic_devices, SIZE_2K, 0, NULL,
                          ALTE_RECV_TASK_PRIORITY,
                          AOS_AUTO_START);
    if (alte_nic_devices.recv_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x063e03, "", "[driver] create lte recv task fail\n");
        aos_delete_event(alte_nic_devices.send_event);
        aos_delete_thread(alte_nic_devices.send_thread_id);

        return AIC_EPERM;
    }

#ifndef AIC_DISABLE_NET_CONNECT_CHECK
    alte_nic_devices.ping_thread_id =
        aos_create_thread("alte-ping", nx_alte_ping_task,
                          (void *)&alte_nic_devices, SIZE_1K + 512, 0, NULL,
                          10, AOS_AUTO_START);
    if (alte_nic_devices.ping_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x028ec7, "", "[driver] create lte ping task fail\n");
        aos_delete_event(alte_nic_devices.send_event);
        aos_delete_thread(alte_nic_devices.send_thread_id);
        aos_delete_thread(alte_nic_devices.recv_thread_id);

        return AIC_EPERM;
    }
#endif

    alte_nic_devices.send_mutex = aos_create_mutex("alte-send");
    if (alte_nic_devices.send_mutex == AOS_NULL) {
        aicx_error(0x15f648, "", "[driver] create send mutex fail\n");
        aos_delete_event(alte_nic_devices.send_event);
        aos_delete_thread(alte_nic_devices.send_thread_id);
        aos_delete_thread(alte_nic_devices.recv_thread_id);
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
        aos_delete_thread(alte_nic_devices.ping_thread_id);
#endif
        return AIC_EPERM;
    }

    if (aic_register_pdp_state_change_cb(alte_pdp_state_change_cb))
        aicx_warn(0x00079b, "", "[NIC] register pdp state cb fail\r\n");

    alte_nic_devices.enhance_timer = aos_create_timer("network enhance timer",
                                                      aic_network_enhance_timer_callback,
                                                      &alte_nic_devices, 10000,
                                                      AOS_NO_ACTIVATE,
                                                      AOS_NO_PERIOD);

    amsg_register_mod_handler(AIC_SYS_CP, AMOD_NET, aic_network_msg_handle,
                              &alte_nic_devices);

    alte_nic_devices.max_send_cache_size = aos_get_heap_size() / 10;
    alte_nic_devices.max_send_cache_size += (alte_nic_devices.max_send_cache_size / 2);
#ifdef CONFIG_D2D
    alte_nic_devices.d2d_send_timeout = 30000;
#endif
    alte_nic_devices.disable_copy = NX_FALSE;

    return status;
}

DIS_CHARGE_APP_P2_INIT_EXPORT(nx_alte_nic_init);

