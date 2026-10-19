/*****************************************************************************/
/*                                                                           */
/*     Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** Network Component                                                        */
/**                                                                          */
/**   Network traffic control                                                */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07b8d1
#define LOG_MODULE    AMOD_NET

#include <aic_bitmap.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aipc_api.h>
#include <aic_cmd.h>
#include <aic_osa.h>
#include <aos_mem.h>

#include <nx_api.h>
#include <nx_tcp.h>
#include <nx_packet.h>
#include <nx_ipv4.h>
#include <nx_ipv6.h>
#include <nx_icmpv6.h>

#include <aic_net_api.h>
#include <aic_packet_management.h>

#include "aic_net_tc.h"

#define AIC_TC_SPEED_TIMES   ((1000 / ((NX_IP_PERIODIC_RATE + (NX_IP_FAST_TIMER_RATE - 1)) / NX_IP_FAST_TIMER_RATE)) * 8)

static void aic_process_cached_recv_packet(NX_INTERFACE *interface_ptr)
{
    NX_PACKET *packet_ptr;
    AOS_INTERRUPT_DECLARE
    struct aic_network_ip_instance *aic_ip;

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_error(0x0b984a, "", "[TC] ip instance not ready\n");
        return;
    }

    if (!interface_ptr) {
        aicx_error(0x01b3e4, "", "[TC] interface is null when process recv\n");
        return;
    }

    if (interface_ptr->tc_recv_packet_queue_count)
        aicx_info(0x00efd4, "sd", "[TC] process %s cached %d recv packet",
                  interface_ptr->nx_interface_name,
                  interface_ptr->tc_recv_packet_queue_count);

    while (interface_ptr->tc_recv_packet_queue_head) {
        AOS_INTERRUPT_DISABLE
        packet_ptr = interface_ptr->tc_recv_packet_queue_head;
        interface_ptr->tc_recv_packet_queue_head = packet_ptr->nx_packet_queue_next;
        packet_ptr->nx_packet_queue_next = NULL;
        if (!interface_ptr->tc_recv_packet_queue_head)
            interface_ptr->tc_recv_packet_queue_tail = NULL;
        interface_ptr->tc_recv_packet_queue_count--;
        AOS_INTERRUPT_RESTORE

        interface_ptr->nx_interface_recv_current_speed += packet_ptr->nx_packet_length;

        _nx_ip_packet_receive(aic_ip->ip_ptr, packet_ptr);
    }
}

static void aic_process_cached_send_packet(NX_INTERFACE *interface_ptr)
{
    NX_PACKET *packet_ptr;
    AOS_INTERRUPT_DECLARE
    struct aic_network_ip_instance *aic_ip;

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_error(0x0b984a, "", "[TC] ip instance not ready\n");
        return;
    }

    if (!interface_ptr) {
        aicx_error(0x01b3e4, "", "[TC] interface is null when process recv\n");
        return;
    }

    if (interface_ptr->tc_send_packet_queue_count)
        aicx_info(0x1e3834, "sd", "[TC] process %s cached %d send packet",
                  interface_ptr->nx_interface_name,
                  interface_ptr->tc_send_packet_queue_count);

    while (interface_ptr->tc_send_packet_queue_head) {
        AOS_INTERRUPT_DISABLE
        packet_ptr = interface_ptr->tc_send_packet_queue_head;
        interface_ptr->tc_send_packet_queue_head = packet_ptr->nx_packet_queue_next;
        packet_ptr->nx_packet_queue_next = NULL;
        if (!interface_ptr->tc_send_packet_queue_head)
            interface_ptr->tc_send_packet_queue_tail = NULL;
        interface_ptr->tc_send_packet_queue_count--;
        AOS_INTERRUPT_RESTORE

        interface_ptr->nx_interface_send_current_speed += packet_ptr->nx_packet_length;

        if (interface_ptr->nx_interface_tc_send)
            interface_ptr->nx_interface_tc_send(interface_ptr, packet_ptr);
        else
            _nx_packet_transmit_release(packet_ptr);
    }
}

static void aic_tc_timer_callback(NX_INTERFACE *interface_ptr)
{
    int i;

    for (i = 0; i < ALTE_NIC_MAX_INSTANCES; i++) {
        if (interface_ptr->nx_interface_tc_enable) {
            interface_ptr->nx_interface_send_current_speed = 0;
            interface_ptr->nx_interface_recv_current_speed = 0;
            aic_process_cached_recv_packet(interface_ptr);
            aic_process_cached_send_packet(interface_ptr);
        }
    }
}

int aic_tc_cache_recv_packet(NX_INTERFACE *interface_ptr, NX_PACKET *packet_ptr)
{
    AOS_INTERRUPT_DECLARE

    if (!interface_ptr->nx_interface_tc_enable)
        return AIC_FALSE;

    if (aic_identify_packet_pri(packet_ptr, NULL))
        return AIC_FALSE;

    if (interface_ptr->nx_interface_recv_current_speed >
        interface_ptr->nx_interface_recv_max_speed) {
        if (interface_ptr->nx_interface_recv_current_speed <
            interface_ptr->nx_interface_recv_max_burst_speed) {
            AOS_INTERRUPT_DISABLE
            if (interface_ptr->tc_recv_packet_queue_head) {
                interface_ptr->tc_recv_packet_queue_tail->nx_packet_queue_next =
                    packet_ptr;
                interface_ptr->tc_recv_packet_queue_tail = packet_ptr;
            } else {
                interface_ptr->tc_recv_packet_queue_head = packet_ptr;
                interface_ptr->tc_recv_packet_queue_tail = packet_ptr;
            }

            interface_ptr->tc_recv_packet_queue_count++;
            AOS_INTERRUPT_RESTORE
        } else {
            aicx_info(0x0d84a6, "d", "[TC] need drop packet when recv, len %d\n",
                      packet_ptr->nx_packet_length);
            _nx_packet_transmit_release(packet_ptr);
        }
    } else {
        interface_ptr->nx_interface_recv_current_speed +=
            packet_ptr->nx_packet_length;
        return AIC_FALSE;
    }

    interface_ptr->nx_interface_recv_current_speed +=
        packet_ptr->nx_packet_length;

    return AIC_TRUE;
}

int aic_tc_cache_send_packet(NX_INTERFACE *interface_ptr, NX_PACKET *packet_ptr)
{
    int ret;
    NX_PACKET *new_packet_ptr;
    AOS_INTERRUPT_DECLARE

    if (!interface_ptr->nx_interface_tc_enable)
        return AIC_FALSE;

    if (interface_ptr->nx_interface_address_mapping_needed) {
        packet_ptr->nx_packet_prepend_ptr += AIC_ETHERNET_SIZE;
        packet_ptr->nx_packet_length -= AIC_ETHERNET_SIZE;
    }

    if (aic_identify_packet_pri(packet_ptr, NULL)) {
        if (interface_ptr->nx_interface_address_mapping_needed) {
            packet_ptr->nx_packet_prepend_ptr -= AIC_ETHERNET_SIZE;
            packet_ptr->nx_packet_length += AIC_ETHERNET_SIZE;
        }

        return AIC_FALSE;
    }

    if (interface_ptr->nx_interface_address_mapping_needed) {
        packet_ptr->nx_packet_prepend_ptr -= AIC_ETHERNET_SIZE;
        packet_ptr->nx_packet_length += AIC_ETHERNET_SIZE;
    }

    if (interface_ptr->nx_interface_send_current_speed >
        interface_ptr->nx_interface_send_max_speed) {
        if (interface_ptr->nx_interface_send_current_speed <
            interface_ptr->nx_interface_send_max_burst_speed) {

            if (packet_ptr->nx_packet_reserved & AIC_PACKET_SHARE_MEM) {
                ret = nx_packet_copy(packet_ptr, &new_packet_ptr,
                                     packet_ptr->nx_packet_pool_owner,
                                     AOS_NO_WAIT);
                if (ret) {
                    _nx_packet_transmit_release(packet_ptr);
                    aicx_warn(0x0a0ab1, "d", "[TC] alloc fail when tc cache send packet %d\n",
                              ret);
                    return AIC_TRUE;
                }

                _nx_packet_transmit_release(packet_ptr);
                packet_ptr = new_packet_ptr;
            }

            AOS_INTERRUPT_DISABLE
            if (interface_ptr->tc_send_packet_queue_head) {
                interface_ptr->tc_send_packet_queue_tail->nx_packet_queue_next =
                    packet_ptr;
                interface_ptr->tc_send_packet_queue_tail = packet_ptr;
            } else {
                interface_ptr->tc_send_packet_queue_head = packet_ptr;
                interface_ptr->tc_send_packet_queue_tail = packet_ptr;
            }

            interface_ptr->tc_send_packet_queue_count++;
            AOS_INTERRUPT_RESTORE
        } else {
            aicx_info(0x08b4c0, "d", "[TC] need drop packet when send, len %d\n",
                      packet_ptr->nx_packet_length);
            _nx_packet_transmit_release(packet_ptr);
        }
    } else {
        interface_ptr->nx_interface_send_current_speed +=
            packet_ptr->nx_packet_length;
        return AIC_FALSE;
    }

    interface_ptr->nx_interface_send_current_speed +=
        packet_ptr->nx_packet_length;

    return AIC_TRUE;
}

/**
 * aic_network_tc_enable - Set traffic control parameters.
 *
 * @interface_index: Network interface index.
 * @send_max_speed: Maximum downlink rate, bps.
 * @recv_max_speed: Maximum uplink rate, bps.
 *
 * Use a periodic timer for traffic control.
 */
int aic_network_tc_enable(u32 interface_index, u32 send_max_speed,
                          u32 recv_max_speed)
{
    NX_INTERFACE *interface_ptr;
    struct aic_network_ip_instance *aic_ip;

    if (interface_index > NX_MAX_IP_INTERFACES) {
        aicx_error(0x0c6bee, "d", "[TC] interface_index %d is wrong when enable tc\n",
                   interface_index);
        return AIC_EINVAL;
    }

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_error(0x0b984a, "", "[TC] ip instance not ready\n");
        return AIC_EINVAL;
    }

    interface_ptr = &aic_ip->ip_ptr->nx_ip_interface[interface_index];

    interface_ptr->nx_interface_tc_enable = TRUE;
    interface_ptr->nx_interface_send_current_speed = 0;
    interface_ptr->nx_interface_recv_current_speed = 0;

    interface_ptr->nx_interface_send_max_speed =
        send_max_speed / AIC_TC_SPEED_TIMES;
    interface_ptr->nx_interface_recv_max_speed =
        recv_max_speed / AIC_TC_SPEED_TIMES;

    interface_ptr->nx_interface_send_max_burst_speed =
        interface_ptr->nx_interface_send_max_speed / 4 + interface_ptr->nx_interface_send_max_speed;
    interface_ptr->nx_interface_recv_max_burst_speed =
        interface_ptr->nx_interface_recv_max_speed / 3 + interface_ptr->nx_interface_recv_max_speed;

    interface_ptr->nx_interface_tc_callback = aic_tc_timer_callback;

    return AIC_SUCCESS;
}

int aic_network_tc_disable(u32 interface_index)
{
    u32 ori_priority, old_priority;
    NX_PACKET *packet_ptr, *next_packet_ptr;
    NX_INTERFACE *interface_ptr;
    struct aic_network_ip_instance *aic_ip;

    if (interface_index > NX_MAX_IP_INTERFACES) {
        aicx_error(0x0c6bee, "d", "[TC] interface_index %d is wrong when enable tc\n",
                   interface_index);
        return AIC_EINVAL;
    }

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_error(0x0b984a, "", "[TC] ip instance not ready\n");
        return AIC_EINVAL;
    }

    interface_ptr = &aic_ip->ip_ptr->nx_ip_interface[interface_index];

    interface_ptr->nx_interface_tc_enable = FALSE;
    interface_ptr->nx_interface_send_current_speed = 0;
    interface_ptr->nx_interface_recv_current_speed = 0;

    interface_ptr->nx_interface_send_max_speed = 0;
    interface_ptr->nx_interface_recv_max_speed = 0;

    interface_ptr->nx_interface_recv_max_burst_speed = 0;
    interface_ptr->nx_interface_send_max_burst_speed = 0;

    interface_ptr->nx_interface_tc_callback = NULL;
    //interface_ptr->nx_interface_tc_send = NULL;

    aos_change_thread_pri(aos_identify_thread(), 3, &old_priority);
    ori_priority = old_priority;
    if (interface_ptr->tc_send_packet_queue_head) {
        packet_ptr = interface_ptr->tc_send_packet_queue_head;
        do {
            next_packet_ptr = packet_ptr->nx_packet_queue_next;
            nx_packet_release(packet_ptr);
            packet_ptr = next_packet_ptr;
            interface_ptr->tc_send_packet_queue_count--;
        } while (packet_ptr);

        interface_ptr->tc_send_packet_queue_head = NULL;
    }

    if (interface_ptr->tc_recv_packet_queue_head) {
        packet_ptr = interface_ptr->tc_recv_packet_queue_head;
        do {
            next_packet_ptr = packet_ptr->nx_packet_queue_next;
            nx_packet_release(packet_ptr);
            packet_ptr = next_packet_ptr;
            interface_ptr->tc_recv_packet_queue_count--;
        } while (packet_ptr);

        interface_ptr->tc_recv_packet_queue_head = NULL;
    }

    if (interface_ptr->tc_recv_packet_queue_count ||
        interface_ptr->tc_send_packet_queue_count)
        AIC_ASSERT(FALSE);

    aos_change_thread_pri(aos_identify_thread(), ori_priority, &old_priority);

    return AIC_SUCCESS;
}

