/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** NetX DUO Component                                                       */
/**                                                                          */
/**   IPv6 router                                                            */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a3181
#define LOG_MODULE    AMOD_NET

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <timer_inc.h>

#include "nx_api.h"
#include "nx_packet.h"
#include "nx_ipv6.h"
#include "nx_tcp.h"
#include "nx_udp.h"
#include "nx_icmpv6.h"
#include "nx_nd_cache.h"

#include "aic_bitmap.h"
#include "aic_net_api.h"
#include "aic_ipv6_forward.h"
#include "aic_net_pdp_mgr.h"

#ifdef CONFIG_NET_SHARE_COMMON
#include "aic_net_share_common.h"
#endif

static u32 aic_ipv6_dl_win = 262144;
static u32 aic_ipv6_ul_win = 131072;
static int ipv6_enable_cnt = 0;
static aos_mutex_ptr *ipv6_forward_mutex = NULL;
static aos_mutex_ptr *ipv6_forward_table_mutex = NULL;

static struct aic_network_ipv6_router aic_ipv6_route_dev[NX_MAX_IP_INTERFACES] = {0};

static int aic_net_ipv6_filter_ack_packet(struct tcphdr *tcp_hdr, u32 len)
{
    u32 tcp_header_len;

    tcp_header_len = tcp_hdr->doff * 4;

    aicx_debug(0x1c5085, "dddddd", "[IPV6_FORWORD] tcp_header_len %d len %d %d %d %d %d\n",
               tcp_header_len, len, tcp_hdr->ack, tcp_hdr->syn, tcp_hdr->rst, tcp_hdr->fin);

    if ((tcp_header_len == len) && tcp_hdr->ack && !tcp_hdr->syn &&
        !tcp_hdr->rst && !tcp_hdr->fin)
        return NX_TRUE;

    return NX_FALSE;
}

static struct aic_net_ipv6_forward_table *aic_ipv6_create_table(struct aic_network_ipv6_router *ipv6_dev,
                                                                NX_PACKET *packet_ptr,
                                                                u32 packet_type)
{
    u8 protocol;
    s64 delta_time = 120000;
    struct tcphdr *tcp_hdr;
    struct ipv6hdr *ipv6_hdr;
    struct list_head *list, *pos;
    struct aic_net_ipv6_forward_table *table;

REMOVE_AGAIN:
    if (ipv6_dev->used_table_num > ipv6_dev->max_table_num) {
        list_for_each_safe(list, pos, &ipv6_dev->forward_list) {
            table = list_entry(list, struct aic_net_ipv6_forward_table, list);
            if ((aic_get_tick() - table->timestamp) > delta_time) {
                list_del(&table->list);
                aos_free(table);
                ipv6_dev->used_table_num--;
            }
        }
    }

    if (ipv6_dev->used_table_num > ipv6_dev->max_table_num) {
        delta_time -= 30000;
        if (delta_time > 0) {
            goto REMOVE_AGAIN;
        } else {
            aicx_warn(0x13389a, "", "[IPV6_FORWARD] outbound table is full\n");
            return NULL;
        }
    }

    table = (struct aic_net_ipv6_forward_table *)aos_zalloc(sizeof(struct aic_net_ipv6_forward_table));
    if (!table) {
        return NULL;
    }

    table->timestamp = aic_get_tick();
    if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6 &&
        packet_ptr->nx_packet_ip_header) {
        ipv6_hdr = (struct ipv6hdr *)packet_ptr->nx_packet_ip_header;
        protocol = ipv6_hdr->nexthdr;
        if (packet_type == AIC_IPV6_INBOUND) {
            memcpy(table->local_ipv6_addr, ipv6_hdr->daddr.s6_addr, 16);
            memcpy(table->external_ipv6_addr, ipv6_hdr->saddr.s6_addr, 16);
        } else {
            memcpy(table->local_ipv6_addr, ipv6_hdr->saddr.s6_addr, 16);
            memcpy(table->external_ipv6_addr, ipv6_hdr->daddr.s6_addr, 16);
        }
    } else {
        aos_free(table);
        return NULL;
    }

    switch (protocol) {
    case IPPROTO_TCP:
        tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;

        table->protocol = IPPROTO_TCP;
        if (packet_type == AIC_IPV6_INBOUND) {
            table->local_port = tcp_hdr->dest;
            table->external_port = tcp_hdr->source;
        } else {
            table->local_port = tcp_hdr->source;
            table->external_port = tcp_hdr->dest;
        }
        break;
    default:
        aos_free(table);
        return NULL;
    }

    ipv6_dev->used_table_num++;

    list_add(&table->list, &ipv6_dev->forward_list);

    return table;
}

static struct aic_net_ipv6_forward_table *aic_ipv6_find_forward_table(struct aic_network_ipv6_router *ipv6_dev,
                                                                      NX_PACKET *packet_ptr,
                                                                      u8 packet_type)
{
    int ret;
    u8 next_hdr;
    struct tcphdr *tcp_hdr;
    struct ipv6hdr *ipv6_hdr;
    struct in6_addr local_addr;
    struct list_head *list, *pos;
    u16 local_port, external_port;
    struct in6_addr external_addr;
    struct aic_net_ipv6_forward_table *ipv6_table = NULL;

    if ((packet_type != AIC_IPV6_INBOUND) &&
        (packet_type != AIC_IPV6_OUTBOUND)) {
        return NULL;
    }

    ret = aos_get_mutex(ipv6_forward_table_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        return NULL;
    }

    list_for_each_safe(list, pos, &ipv6_dev->forward_list) {
        ipv6_table = list_entry(list, struct aic_net_ipv6_forward_table, list);
        ipv6_hdr = (struct ipv6hdr *)packet_ptr->nx_packet_ip_header;
        next_hdr = ipv6_hdr->nexthdr;
        switch (next_hdr) {
        case IPPROTO_TCP:
            tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;
            if (packet_type == AIC_IPV6_INBOUND) {
                local_port = tcp_hdr->dest;
                external_port = tcp_hdr->source;
                memcpy(local_addr.s6_addr, ipv6_hdr->daddr.s6_addr, 16);
                memcpy(external_addr.s6_addr, ipv6_hdr->saddr.s6_addr, 16);
            } else {
                local_port = tcp_hdr->source;
                external_port = tcp_hdr->dest;
                memcpy(local_addr.s6_addr, ipv6_hdr->saddr.s6_addr, 16);
                memcpy(external_addr.s6_addr, ipv6_hdr->daddr.s6_addr, 16);
            }
            break;
        default:
            aos_put_mutex(ipv6_forward_table_mutex);
            return NULL;
        }

        if ((ipv6_table->protocol == next_hdr) &&
            (ipv6_table->local_port == local_port) &&
            (ipv6_table->external_port == external_port) &&
            !memcmp((u8 *)&ipv6_table->local_ipv6_addr,
                    (u8 *)&local_addr, 16) &&
            !memcmp((u8 *)&ipv6_table->external_ipv6_addr,
                    (u8 *)&external_addr, 16)) {
            ipv6_table->timestamp = aic_get_tick();
            break;
        }
    }

    if (list != &ipv6_dev->forward_list) {
        aos_put_mutex(ipv6_forward_table_mutex);
        return ipv6_table;
    }

    ipv6_table = aic_ipv6_create_table(ipv6_dev, packet_ptr, packet_type);
    aos_put_mutex(ipv6_forward_table_mutex);

    return ipv6_table;
}

static void aic_ipv6_header_change_endian(NX_IPV6_HEADER *ip_header_ptr)
{
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr->nx_ip_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr->nx_ip_header_word_1);
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_header_ptr->nx_ip_header_destination_ip);
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_header_ptr->nx_ip_header_source_ip);
}

static void aic_ipv6_adjust_win(struct aic_network_ipv6_router *ipv6_dev,
                                NX_PACKET *packet_ptr, u32 packet_type)
{
    int ret;
    u16 window;
    struct tcphdr *tcp_hdr;
    struct aic_net_ipv6_forward_table *table;
    u32 win_scal = 0, real_window, rwin_scale, option_words, max_win;

    if (packet_ptr->nx_packet_ip_version != NX_IP_VERSION_V6) {
        return;
    }

    if (packet_type != AIC_IPV6_INBOUND && packet_type != AIC_IPV6_OUTBOUND) {
        return;
    }

    aic_ipv6_header_change_endian((NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr);
    packet_ptr->nx_packet_ip_header = packet_ptr->nx_packet_prepend_ptr;
    packet_ptr->nx_packet_prepend_ptr += sizeof(struct ipv6hdr);
    packet_ptr->nx_packet_length -= sizeof(struct ipv6hdr);
    table = aic_ipv6_find_forward_table(ipv6_dev, packet_ptr, packet_type);
    if (!table) {
        packet_ptr->nx_packet_prepend_ptr -= sizeof(struct ipv6hdr);
        packet_ptr->nx_packet_length += sizeof(struct ipv6hdr);
        aic_ipv6_header_change_endian((NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr);
        return;
    }

    tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;
    option_words = tcp_hdr->doff - 5;
    if (tcp_hdr->syn) {
        ret = _nx_tcp_window_scaling_option_get((packet_ptr->nx_packet_prepend_ptr +
                                                 sizeof(struct tcphdr)),
                                                option_words * (u32)sizeof(u32), (ULONG *)&rwin_scale);

        if ((ret == NX_TRUE) && (rwin_scale != 0xFF)) {
            if (packet_type == AIC_IPV6_INBOUND)
                table->inbound_win_scal = rwin_scale;
            else
                table->outbound_win_scal = rwin_scale;
        }
    }

    if (aic_net_ipv6_filter_ack_packet((struct tcphdr *)packet_ptr->nx_packet_prepend_ptr,
                                       packet_ptr->nx_packet_length)) {

        if (packet_type == AIC_IPV6_INBOUND) {
            win_scal = table->inbound_win_scal;
            max_win = aic_ipv6_ul_win;
        } else {
            win_scal = table->outbound_win_scal;
            max_win = aic_ipv6_dl_win;
        }

        window = htons(tcp_hdr->window);
        real_window = window * (1 << win_scal);

        aicx_debug(0x065c4f, "ddd", "[IPV6_FORWARD] window %d real_window %d max_win %d\n",
                   window, real_window, max_win);
        if (real_window > max_win) {
            real_window = max_win / (1 << win_scal);
            if (real_window > 0xFFFF)
                real_window = 65535;

            real_window = htons(real_window);
            tcp_hdr->window = real_window;
            window = htons(window);
            /* Adjust the checksum for window.  */
            aic_checksum_adjust((u8 *)&tcp_hdr->check, (u8 *)&window,
                                sizeof(u16), (u8 *)&real_window, sizeof(u16));
            aicx_debug(0x1960a7, "dddd", "[IPV6_FORWARD] change window from %d to %d, win scal %d, max win %d\n",
                       htons(window), htons(real_window), 1 << win_scal, max_win);
        }
    }

    packet_ptr->nx_packet_prepend_ptr -= sizeof(struct ipv6hdr);
    packet_ptr->nx_packet_length += sizeof(struct ipv6hdr);
    aic_ipv6_header_change_endian((NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr);
}

/**
 * aic_ipv6_send_ra - Send router advertisement packet.
 *
 * @ip_ptr: Netxduo IP instance pointer.
 * @ra_packet_ptr: The original packet that store router advertisment message.
 * @dst_interface: output interface.
 *
 * Send router advertisement packet. The incoming packet will not be used for
 * sending, and a new packet will be re-applied internally for sending.
 */
static int aic_ipv6_send_ra(NX_IP *ip_ptr, NX_PACKET *ra_packet_ptr,
                            NX_INTERFACE *dst_interface)
{
    u16 checksum;
    NX_PACKET *packet_ptr;
    NX_IPV6_HEADER *ip_header_ptr;
    NX_ICMPV6_RA *icmpv6_ra_ptr;
    ulong src_address[4], dest_address[4];
    u32 protocol, payload_size, hop_limit = 255, status;

    if (!ra_packet_ptr) {
        aicx_error(0x0fcab5, "", "[IPV6_ROUTER] ra packet ptr is null\r\n");
        return AIC_EINVAL;
    }

    status = nx_packet_copy(ra_packet_ptr, &packet_ptr,
                            ra_packet_ptr->nx_packet_pool_owner,
                            AOS_NO_WAIT);
    if (status != NX_SUCCESS) {
        aicx_error(0x03ccd4, "x", "[IPV6_ROUTER] alloc packet fail 0x%x when send ra\n",
                   status);
        return status;
    }

    aicx_info(0x1baf8e, "s", "[IPV6_ROUTER] send ra packet to %s\n",
              dst_interface->nx_interface_name);
    ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr;
    protocol = (ip_header_ptr->nx_ip_header_word_1 & 0xFF00) >> 8;
    payload_size = packet_ptr->nx_packet_length - sizeof(NX_IPV6_HEADER);
    memcpy(dest_address, ip_header_ptr->nx_ip_header_destination_ip,
           sizeof(dest_address));

    packet_ptr->nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER);
    packet_ptr->nx_packet_length -= sizeof(NX_IPV6_HEADER);

    status = _nxd_ipv6_interface_find(ip_ptr, dest_address,
                                      &packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr,
                                      dst_interface);
    if (status != NX_SUCCESS) {
        aicx_info(0x0a50f3, "xxxxsx", "[IPV6_ROUTER] 0x%x 0x%x 0x%x 0x%x find interface %s fail 0x%x when send ra \n",
                  dest_address[0], dest_address[1], dest_address[2],
                  dest_address[3], dst_interface->nx_interface_name, status);

        aicx_error(0x1c7bf7, "", "[IPV6_ROUTER] no ipv6 addr to use\n");
        nx_packet_release(packet_ptr);
        return NX_INVALID_INTERFACE;
    }

    icmpv6_ra_ptr = (NX_ICMPV6_RA *)packet_ptr->nx_packet_prepend_ptr;
    icmpv6_ra_ptr->nx_icmpv6_ra_icmpv6_header.nx_icmpv6_header_checksum = 0;
    icmpv6_ra_ptr->nx_icmpv6_ra_flag = 0x40;
    memcpy(src_address, packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr->nxd_ipv6_address,
           sizeof(src_address));
    checksum = _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_ICMPV6,
                                       (UINT)packet_ptr->nx_packet_length,
                                       src_address, dest_address);
    checksum = (u16)(~checksum);

    NX_CHANGE_USHORT_ENDIAN(checksum);

    icmpv6_ra_ptr->nx_icmpv6_ra_icmpv6_header.nx_icmpv6_header_checksum =
        checksum;

    _nx_ipv6_packet_send(ip_ptr, packet_ptr, protocol, payload_size,
                         hop_limit, src_address, dest_address);

    return AIC_SUCCESS;
}

static int aic_ipv6_process_dhcpv6(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                   struct aic_network_ipv6_router *ipv6_router,
                                   u32 wan_index)
{
    u8 flag = FALSE, is_solicit = FALSE;
    u16 dhcpv6_client_len = 0, checksum;
    u32 port, ret, new_offset = 0, ori_offset = 0;
    NX_PACKET *new_packet_ptr = NULL;
    NX_UDP_HEADER *udp_header_ptr;
    NX_IPV6_HEADER *ip_header_ptr;
    NX_UDP_HEADER *new_udp_header_ptr;
    aos_systime systime;
    s32 local_tz, tmp_tz;
    NX_INTERFACE *interface_ptr;
    ulong dns_server[4];

    interface_ptr = packet_ptr->nx_packet_ip_interface;
    ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr;
    if ((ip_header_ptr->nx_ip_header_word_1 & 0x0000FF00) >> 8 == NX_PROTOCOL_UDP) {
        udp_header_ptr = (NX_UDP_HEADER *)(packet_ptr->nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER));
        port = udp_header_ptr->nx_udp_header_word_0;
        NX_CHANGE_ULONG_ENDIAN(port);
        if ((port >> 16) == 546 && (port & 0x0000FFFF) == 547)
            aicx_info(0x05c8fc, "", "[IPV6_FORWARD] recv dhcpv6 packet\n");
        else
            return EPERM;
    } else {
        return EPERM;
    }

    ret = nx_packet_allocate(packet_ptr->nx_packet_pool_owner, &new_packet_ptr,
                             NX_IPv6_PACKET, NX_NO_WAIT);
    if (ret)
        return ret;

    new_packet_ptr->nx_packet_ip_version = NX_IP_VERSION_V6;
    new_udp_header_ptr = (NX_UDP_HEADER *)new_packet_ptr->nx_packet_prepend_ptr;
    new_udp_header_ptr->nx_udp_header_word_0 = (udp_header_ptr->nx_udp_header_word_0 >> 16) |
        (udp_header_ptr->nx_udp_header_word_0 << 16);
    new_offset += sizeof(NX_UDP_HEADER);
    ori_offset += sizeof(NX_IPV6_HEADER) + sizeof(NX_UDP_HEADER);
    /* 1: Solicit 2: Advertise 3: Request 7: Reply 11: Information-request */
    if (*(packet_ptr->nx_packet_prepend_ptr + ori_offset) == 1) {
        *(new_packet_ptr->nx_packet_prepend_ptr + new_offset) = 2;
        is_solicit = TRUE;
    } else {
        *(new_packet_ptr->nx_packet_prepend_ptr + new_offset) = 7;
    }
    new_offset++;
    ori_offset++;

    /* Transaction ID */
    memcpy(new_packet_ptr->nx_packet_prepend_ptr + new_offset,
           packet_ptr->nx_packet_prepend_ptr + ori_offset, 3);
    new_offset += 3;
    ori_offset += 3;

    while ((packet_ptr->nx_packet_prepend_ptr + ori_offset) <
           packet_ptr->nx_packet_append_ptr) {
        if (*(packet_ptr->nx_packet_prepend_ptr + ori_offset + 1) == 1) {
            flag = TRUE;
            dhcpv6_client_len = htons(*(u16 *)(packet_ptr->nx_packet_prepend_ptr + ori_offset + 2)) + 4;
            break;
        } else {
            ori_offset += htons(*(u16 *)(packet_ptr->nx_packet_prepend_ptr + ori_offset + 2)) + 4;
        }
    }

    if (!flag) {
        aicx_info(0x050566, "", "[IPV6_FORWARD] find client identifier\n");
        nx_packet_release(new_packet_ptr);
        return EPERM;
    }

    /* Copy client indentifier */
    memcpy(new_packet_ptr->nx_packet_prepend_ptr + new_offset,
           packet_ptr->nx_packet_prepend_ptr + ori_offset, dhcpv6_client_len);
    new_offset += dhcpv6_client_len;

    u8 server_id[8] = {0x00, 0x02, 0x00, 0x0e, 0x00, 0x01, 0x00, 0x01};
    /* Config server indentifier */
    memcpy(new_packet_ptr->nx_packet_prepend_ptr + new_offset, server_id, 8);

    if (!ipv6_router->duid_time) {
        aos_get_systime(&systime);
        extern s32 aic_get_current_time_zone(void);
        local_tz = aic_get_current_time_zone();
        tmp_tz = local_tz;
        local_tz = local_tz - (2 * tmp_tz);
        /* Ajust the time to time with time zone 0. */
        extern void aic_adjust_time_with_time_zone(aos_systime *current_time, s32 time_zone);
        aic_adjust_time_with_time_zone(&systime, local_tz * 4);
        systime.year -= 30;
        ipv6_router->duid_time = aic_mktime_to_seconds(&systime);
        ipv6_router->duid_time = htonl(ipv6_router->duid_time);
    }

    memcpy(new_packet_ptr->nx_packet_prepend_ptr + new_offset + 8,
           &ipv6_router->duid_time, 4);

    *(new_packet_ptr->nx_packet_prepend_ptr + new_offset + 12) =
        (u8)(interface_ptr->nx_interface_physical_address_msw >> 8);
    *(new_packet_ptr->nx_packet_prepend_ptr + new_offset + 13) =
        (u8)interface_ptr->nx_interface_physical_address_msw;

    *(new_packet_ptr->nx_packet_prepend_ptr + new_offset + 14) =
        (u8)(interface_ptr->nx_interface_physical_address_lsw >> 24);
    *(new_packet_ptr->nx_packet_prepend_ptr + new_offset + 15) =
        (u8)(interface_ptr->nx_interface_physical_address_lsw >> 16);
    *(new_packet_ptr->nx_packet_prepend_ptr + new_offset + 16) =
        (u8)(interface_ptr->nx_interface_physical_address_lsw >> 8);
    *(new_packet_ptr->nx_packet_prepend_ptr + new_offset + 17) =
        (u8)(interface_ptr->nx_interface_physical_address_lsw);
    new_offset += 18;

    /* Add preference */
    if (is_solicit) {
        u8 preference[5] = {0x00, 0x07, 0x00, 0x01, 0x00};
        memcpy(new_packet_ptr->nx_packet_prepend_ptr + new_offset, preference, 5);
        new_offset += 5;
    }

    /* Add dns */
    u8 dns_cfg[4] = {0x00, 0x17, 0x00, 0x10};
    memcpy(new_packet_ptr->nx_packet_prepend_ptr + new_offset, dns_cfg, 4);
    new_offset += 4;

    ret = aic_get_ipv6_dns_server(dns_server, wan_index);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x11e44c, "xd", "[IPV6_ROUTER] dhchv6 get ipv6 dns server fail 0x%x if index is %d\n",
                   ret, wan_index);
        nx_packet_release(new_packet_ptr);
        return EPERM;
    }

    NX_CHANGE_ULONG_ENDIAN(dns_server[0]);
    NX_CHANGE_ULONG_ENDIAN(dns_server[1]);
    NX_CHANGE_ULONG_ENDIAN(dns_server[2]);
    NX_CHANGE_ULONG_ENDIAN(dns_server[3]);

    memcpy(new_packet_ptr->nx_packet_prepend_ptr + new_offset, (u8 *)dns_server, 16);
    new_offset += 16;

    new_packet_ptr->nx_packet_length = new_offset;
    new_packet_ptr->nx_packet_append_ptr += new_offset;

    ret = _nxd_ipv6_interface_find(ip_ptr, ip_header_ptr->nx_ip_header_source_ip,
                                   &new_packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr,
                                   interface_ptr);
    if (ret != NX_SUCCESS) {
        aicx_info(0x013295, "xxxxsx", "[IPV6_ROUTER] 0x%x 0x%x 0x%x 0x%x find interface %s fail 0x%x when process dhcpv6 \n",
                  ip_header_ptr->nx_ip_header_source_ip[0], ip_header_ptr->nx_ip_header_source_ip[1],
                  ip_header_ptr->nx_ip_header_source_ip[2], ip_header_ptr->nx_ip_header_source_ip[3],
                  interface_ptr->nx_interface_name, ret);
        nx_packet_release(new_packet_ptr);
        return ret;
    }

    new_udp_header_ptr->nx_udp_header_word_1 = htonl((u32)new_offset) >> 16;
    checksum = _nx_ip_checksum_compute(new_packet_ptr, NX_PROTOCOL_UDP,
                                       (UINT)new_packet_ptr->nx_packet_length,
                                       new_packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr->nxd_ipv6_address,
                                       ip_header_ptr->nx_ip_header_source_ip);
    checksum = (u16)(~checksum);
    NX_CHANGE_USHORT_ENDIAN(checksum);
    new_udp_header_ptr->nx_udp_header_word_1 |= ((u32)checksum << 16);
    _nx_ipv6_packet_send(ip_ptr, new_packet_ptr, NX_PROTOCOL_UDP, new_packet_ptr->nx_packet_length,
                         0x80, new_packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr->nxd_ipv6_address,
                         ip_header_ptr->nx_ip_header_source_ip);
    nx_packet_release(packet_ptr);

    return 0;
}

static NX_ICMPV6_OPTION_PREFIX *aic_ipv6_find_prefix(NX_PACKET *packet_ptr)
{
    u32 option_offset = 0;
    u8 option_len, option_type;
    NX_ICMPV6_OPTION_PREFIX *icmpv6_option_prefix = NULL;

    while (1) {
        if ((packet_ptr->nx_packet_prepend_ptr + sizeof(NX_ICMPV6_RA) +
             option_offset) < packet_ptr->nx_packet_append_ptr) {
            option_type = *(packet_ptr->nx_packet_prepend_ptr +
                            sizeof(NX_ICMPV6_RA) + option_offset);
            if (option_type == ICMPV6_OPTION_TYPE_PREFIX_INFO) {
                aicx_info(0x19605b, "", "[IPV6_ROUTER] find icmpv6 option prefix info\n");
                icmpv6_option_prefix =
                    (NX_ICMPV6_OPTION_PREFIX *)(packet_ptr->nx_packet_prepend_ptr +
                                                sizeof(NX_ICMPV6_RA) +
                                                option_offset);
                return icmpv6_option_prefix;
            }

            option_len = *(packet_ptr->nx_packet_prepend_ptr +
                           sizeof(NX_ICMPV6_RA) + option_offset + 1) * 8;
            if (option_len == 0) {
                aicx_error(0x05efae, "", "[IPV6_ROUTER] ra option is wrong\n");
                break;
            }

            option_offset += option_len;
        } else {
            aicx_error(0x1c28d4, "", "[IPV6_ROUTER] can't find enough ra option\n");
            break;
        }
    }

    return NULL;
}

static int aic_ipv6_check_invalid_prefix(NX_PACKET *packet_ptr,
                                         u32 interface_index)
{
    int ret, need_invalid = 0;
    NX_ICMPV6_RA *icmpv6_ra_ptr;
    NX_INTERFACE *dst_interface;
    struct NXD_IPV6_ADDRESS_STRUCT *ipv6_addr_ptr;
    struct aic_network_ipv6_router *ipv6_router_dev;
    NX_ICMPV6_OPTION_PREFIX *icmpv6_option_prefix = NULL, *new_icmpv6_option_prefix = NULL;
    struct aic_network_ip_instance *aic_ip_instance;

    aic_ip_instance = aic_get_default_ip_instance();

    if (!packet_ptr || !aic_ip_instance || !aic_ip_instance->ip_ptr)
        return AIC_SUCCESS;

    if (!ipv6_forward_mutex) {
        return AIC_ENODEV;
    }

    ret = aos_get_mutex(ipv6_forward_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        return ret;
    }

    if (!aic_ipv6_route_dev[interface_index].ra_packet_ptr) {
        aicx_error(0x053a39, "", "[IPV6_FORWARD] ra packet is NULL when check invalid\n");
        aos_put_mutex(ipv6_forward_mutex);
        return AIC_SUCCESS;
    }

    ipv6_router_dev = &aic_ipv6_route_dev[interface_index];
    dst_interface = &aic_ip_instance->ip_ptr->nx_ip_interface[interface_index];
    ipv6_addr_ptr = dst_interface->nxd_interface_ipv6_address_list_head;
    packet_ptr->nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER);
    packet_ptr->nx_packet_length -= sizeof(NX_IPV6_HEADER);
    ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER);
    ipv6_router_dev->ra_packet_ptr->nx_packet_length -= sizeof(NX_IPV6_HEADER);
    icmpv6_ra_ptr = (NX_ICMPV6_RA *)ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr;

    new_icmpv6_option_prefix = aic_ipv6_find_prefix(packet_ptr);
    if (!new_icmpv6_option_prefix) {
        aicx_error(0x181b8b, "", "[IPV6_FORWARD] find new ra packet prefix fail\r\n");
        packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
        packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);
        ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
        ipv6_router_dev->ra_packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);
        aos_put_mutex(ipv6_forward_mutex);
        return AIC_SUCCESS;
    }

    icmpv6_option_prefix =
        aic_ipv6_find_prefix(ipv6_router_dev->ra_packet_ptr);
    if (!icmpv6_option_prefix) {
        aicx_error(0x0e7589, "", "[IPV6_FORWARD] find prev ra packet prefix fail\r\n");
        packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
        packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);
        ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
        ipv6_router_dev->ra_packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);
        aos_put_mutex(ipv6_forward_mutex);
        return AIC_SUCCESS;
    }

    if ((icmpv6_option_prefix->nx_icmpv6_option_prefix[0] !=
         new_icmpv6_option_prefix->nx_icmpv6_option_prefix[0]) ||
        (icmpv6_option_prefix->nx_icmpv6_option_prefix[1] !=
         new_icmpv6_option_prefix->nx_icmpv6_option_prefix[1]))
        need_invalid = TRUE;

    if (!need_invalid) {
        packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
        packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);
        ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
        ipv6_router_dev->ra_packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);
        aos_put_mutex(ipv6_forward_mutex);
        return AIC_SUCCESS;
    }

    aicx_info(0x0aa557, "", "[IPV6_FORWARD] previous prefix is invalid\r\n");

    icmpv6_ra_ptr->nx_icmpv6_ra_router_lifetime = 0;
    icmpv6_option_prefix->nx_icmpv6_option_prefix_preferred_lifetime = 0;
    icmpv6_option_prefix->nx_icmpv6_option_prefix_valid_lifetime = 0;

    packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
    packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);
    ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
    ipv6_router_dev->ra_packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);

    ret = aic_ipv6_send_ra(aic_ip_instance->ip_ptr, ipv6_router_dev->ra_packet_ptr,
                           dst_interface);
    if (ret != AIC_SUCCESS)
        aicx_warn(0x0ef020, "d", "[IPV6_ROUTER] send invalid ra packet fail %d !\n",
                  ret);

    if (ipv6_router_dev->ra_packet_ptr) {
        nx_packet_release(ipv6_router_dev->ra_packet_ptr);
        ipv6_router_dev->ra_packet_ptr = NULL;
    }

    aos_put_mutex(ipv6_forward_mutex);

    return ret;
}

int aic_ipv6_send_invalid_ra(u32 interface_index)
{
    int ret;
    NX_PACKET *packet_ptr;
    NX_ICMPV6_RA *icmpv6_ra_ptr;
    NX_INTERFACE *dst_interface;
    NX_ICMPV6_OPTION_PREFIX *icmpv6_option_prefix = NULL;
    struct aic_network_ip_instance *aic_ip_instance;
    struct aic_network_ipv6_router *ipv6_router_dev;

    if (!aic_ipv6_route_dev[interface_index].ra_packet_ptr) {
        aicx_info(0x0e1caf, "", "[IPV6_FORWARD] don't have ra when send invalid\n");
        return AIC_SUCCESS;
    }

    if (!ipv6_forward_mutex) {
        return AIC_ENODEV;
    }

    ipv6_router_dev = &aic_ipv6_route_dev[interface_index];
    if (nx_packet_copy(ipv6_router_dev->ra_packet_ptr, &packet_ptr,
                       ipv6_router_dev->ra_packet_ptr->nx_packet_pool_owner,
                       0)) {
        aicx_error(0x05c73c, "", "[IPV6_FORWARD] alloc packet fail when send invalid ra\n");
        return AIC_ENOMEM;
    }

    ret = aos_get_mutex(ipv6_forward_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        return ret;
    }

    aic_ip_instance = aic_get_default_ip_instance();

    dst_interface = &aic_ip_instance->ip_ptr->nx_ip_interface[interface_index];
    packet_ptr->nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER);
    packet_ptr->nx_packet_length -= sizeof(NX_IPV6_HEADER);
    icmpv6_ra_ptr = (NX_ICMPV6_RA *)packet_ptr->nx_packet_prepend_ptr;

    icmpv6_option_prefix = aic_ipv6_find_prefix(packet_ptr);
    if (!icmpv6_option_prefix) {
        aicx_error(0x0e7589, "", "[IPV6_FORWARD] find prev ra packet prefix fail\r\n");
        aos_put_mutex(ipv6_forward_mutex);
        return AIC_SUCCESS;
    }

    aicx_info(0x1254d7, "", "[IPV6_FORWARD] send invalid ra\r\n");

    icmpv6_ra_ptr->nx_icmpv6_ra_router_lifetime = 0;
    icmpv6_option_prefix->nx_icmpv6_option_prefix_preferred_lifetime = 0;
    icmpv6_option_prefix->nx_icmpv6_option_prefix_valid_lifetime = 0;

    packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
    packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);

    ret = aic_ipv6_send_ra(aic_ip_instance->ip_ptr, packet_ptr, dst_interface);
    if (ret != AIC_SUCCESS)
        aicx_warn(0x0ef020, "d", "[IPV6_ROUTER] send invalid ra packet fail %d !\n",
                  ret);

    nx_packet_release(packet_ptr);

    aos_put_mutex(ipv6_forward_mutex);

    return ret;
}

/**
 * aic_ipv6_add_ra_dns_option - Add dns option to the router advertisement
 *                              message.
 *
 * @packet_ptr: Router advertisement packet.
 * @icmpv6_ra_ptr: NX_ICMPV6_RA pointer.
 * @icmpv6_option_dns: NX_ICMPV6_OPTION_DNS pointer.
 * @src_address: Source IPv6 address.
 * @dest_address: Destnation IPv6 address.
 * @payload_size: Payload size.
 *
 * It will traverse whether there is a dns option in the packet,. If there is,
 * it will not be added; if not, it will be added and update the payload size
 * and checksum.
 */
static void aic_ipv6_add_ra_dns_option(NX_PACKET *packet_ptr,
                                       NX_ICMPV6_RA *icmpv6_ra_ptr,
                                       NX_ICMPV6_OPTION_DNS *icmpv6_option_dns,
                                       ulong *src_address, ulong *dest_address,
                                       u32 *payload_size)
{
    u16 checksum, life_time;
    u8 option_len, option_type;
    u32 option_offset = 0, status;

    life_time = icmpv6_ra_ptr->nx_icmpv6_ra_router_lifetime;
    NX_CHANGE_USHORT_ENDIAN(life_time);

    while (1) {
        if ((packet_ptr->nx_packet_prepend_ptr + sizeof(NX_ICMPV6_RA) +
             option_offset) < packet_ptr->nx_packet_append_ptr) {
            option_type = *(packet_ptr->nx_packet_prepend_ptr +
                            sizeof(NX_ICMPV6_RA) + option_offset);
            if (option_type == ICMPV6_OPTION_TYPE_DNS) {
                icmpv6_option_dns = NULL;
                break;
            }

            option_len = *(packet_ptr->nx_packet_prepend_ptr +
                           sizeof(NX_ICMPV6_RA) + option_offset + 1) *
                           8;
            option_offset += option_len;
        } else {
            break;
        }
    }

    if (icmpv6_option_dns) {
        status =
            aic_get_ipv6_dns_server(icmpv6_option_dns->nx_icmpv6_option_dns_servers,
                                    packet_ptr->nx_packet_ip_interface->nx_interface_index);
        if (status != AIC_SUCCESS) {
            aicx_error(0x06c3a1, "xs", "[IPV6_ROUTER] get ipv6 dns server fail 0x%x if is %s\n",
                       status,
                       packet_ptr->nx_packet_ip_interface->nx_interface_name);
            return;
        }

        icmpv6_ra_ptr->nx_icmpv6_ra_flag = 0x40;
        icmpv6_ra_ptr->nx_icmpv6_ra_icmpv6_header.nx_icmpv6_header_checksum = 0;
        icmpv6_option_dns->nx_icmpv6_option_dns_type = ICMPV6_OPTION_TYPE_DNS;
        icmpv6_option_dns->nx_icmpv6_option_dns_length = 3;
        icmpv6_option_dns->nx_icmpv6_option_dns_reserved = 0;
        icmpv6_option_dns->nx_icmpv6_option_dns_lifetime = life_time;
        NX_CHANGE_ULONG_ENDIAN(icmpv6_option_dns->nx_icmpv6_option_dns_lifetime);
        NX_CHANGE_ULONG_ENDIAN(icmpv6_option_dns->nx_icmpv6_option_dns_servers[0]);
        NX_CHANGE_ULONG_ENDIAN(icmpv6_option_dns->nx_icmpv6_option_dns_servers[1]);
        NX_CHANGE_ULONG_ENDIAN(icmpv6_option_dns->nx_icmpv6_option_dns_servers[2]);
        NX_CHANGE_ULONG_ENDIAN(icmpv6_option_dns->nx_icmpv6_option_dns_servers[3]);
        packet_ptr->nx_packet_length += sizeof(NX_ICMPV6_OPTION_DNS);
        packet_ptr->nx_packet_append_ptr += sizeof(NX_ICMPV6_OPTION_DNS);
        *payload_size += sizeof(NX_ICMPV6_OPTION_DNS);
        checksum = _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_ICMPV6,
                                           (UINT)packet_ptr->nx_packet_length,
                                           src_address, dest_address);

        checksum = (u16)(~checksum);

        NX_CHANGE_USHORT_ENDIAN(checksum);

        icmpv6_ra_ptr->nx_icmpv6_ra_icmpv6_header.nx_icmpv6_header_checksum =
            checksum;
    }
}

/**
 * aic_ipv6_packet_copy - Copy of data content and related parameters.
 *
 * @dst_packet_ptr: Destnation packet pointer.
 * @src_packet_ptr: Source packet ptr.
 *
 */
static void aic_ipv6_packet_copy(NX_PACKET *dst_packet_ptr,
                                 NX_PACKET *src_packet_ptr)
{
    memcpy(dst_packet_ptr->nx_packet_prepend_ptr,
           src_packet_ptr->nx_packet_prepend_ptr,
           src_packet_ptr->nx_packet_length);
    dst_packet_ptr->nx_packet_append_ptr =
        dst_packet_ptr->nx_packet_prepend_ptr +
        src_packet_ptr->nx_packet_length;
    dst_packet_ptr->nx_packet_ip_interface =
        src_packet_ptr->nx_packet_ip_interface;
    dst_packet_ptr->nx_packet_ip_version = NX_IP_VERSION_V6;
    dst_packet_ptr->nx_packet_length = src_packet_ptr->nx_packet_length;
    dst_packet_ptr->nx_packet_ip_header = dst_packet_ptr->nx_packet_prepend_ptr;
    dst_packet_ptr->nx_packet_ip_header_length = sizeof(NX_IPV6_HEADER);
}

/**
 * aic_ipv6_record_ra - Copy of data content and related parameters.
 *
 * @dst_packet_ptr: Destnation packet pointer.
 * @src_packet_ptr: Source packet ptr.
 *
 * Record the latest router advertisement packet locally.
 */
static int aic_ipv6_record_ra(NX_PACKET *packet_ptr)
{
    int ret;
    NX_ICMPV6_RA *icmpv6_ra_ptr;
    NX_IPV6_HEADER *ip_header_ptr;
    NX_INTERFACE *local_interface;
    ulong src_address[4], dest_address[4];
    u32 payload_size, status; /* addr_type */
    u8 next_header_type;
    NX_ICMPV6_OPTION_DNS *icmpv6_option_dns = NULL;
    struct aic_network_ipv6_router *ipv6_router_dev;

    if (!ipv6_forward_mutex) {
        return AIC_ENODEV;
    }

    ret = aos_get_mutex(ipv6_forward_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        return ret;
    }

    local_interface = packet_ptr->nx_packet_ip_interface;
    ipv6_router_dev = &aic_ipv6_route_dev[local_interface->nx_interface_index];
    ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr;
    next_header_type = (u8)((ip_header_ptr->nx_ip_header_word_1 >> 8) & 0xFF);
    payload_size = ip_header_ptr->nx_ip_header_word_1 >> 16;
    memcpy(src_address, ip_header_ptr->nx_ip_header_source_ip,
           sizeof(src_address));
#if 0
    addr_type = IPv6_Address_Type(dest_address);
    if (addr_type & IPV6_ADDRESS_UNICAST) {
        aicx_info(0x08eba1, "xxxxx", "[IPV6] don't need record unicast ipv6 addr, type 0x%x 0x%x 0x%x 0x%x 0x%x\r\n",
                  addr_type, dest_address[0], dest_address[1], dest_address[2],
                  dest_address[3]);
        aos_put_mutex(ipv6_forward_mutex);
        return AIC_SUCCESS;
    }
#endif
    if (next_header_type == NX_PROTOCOL_ICMPV6) {
        icmpv6_ra_ptr = (NX_ICMPV6_RA *)(packet_ptr->nx_packet_prepend_ptr +
                                         sizeof(NX_IPV6_HEADER));
        if (icmpv6_ra_ptr->nx_icmpv6_ra_icmpv6_header.nx_icmpv6_header_type ==
            NX_ICMPV6_ROUTER_ADVERTISEMENT_TYPE) {
            aicx_info(0x072da3, "x", "[IPV6_ROUTER] recv ra packet flag 0x%x\n",
                      icmpv6_ra_ptr->nx_icmpv6_ra_flag);

            if (!ipv6_router_dev->ra_packet_ptr) {
                status = nx_packet_allocate(packet_ptr->nx_packet_pool_owner,
                                            &ipv6_router_dev->ra_packet_ptr,
                                            NX_PHYSICAL_HEADER, 0);
                if (status != NX_SUCCESS) {
                    aicx_warn(0x1027f5, "x", "[IPV6_ROUTER] alloc ra packet fail 0x%x\n",
                              status);
                    aos_put_mutex(ipv6_forward_mutex);
                    return status;
                }
            }

            aic_ipv6_packet_copy(ipv6_router_dev->ra_packet_ptr,
                                 packet_ptr);
            ip_header_ptr =
                (NX_IPV6_HEADER *)ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr;

            ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr +=
                sizeof(NX_IPV6_HEADER);
            ipv6_router_dev->ra_packet_ptr->nx_packet_length -=
                sizeof(NX_IPV6_HEADER);

            icmpv6_ra_ptr =
                (NX_ICMPV6_RA *)ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr;
            icmpv6_option_dns =
                (NX_ICMPV6_OPTION_DNS *)(ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr +
                                         ipv6_router_dev->ra_packet_ptr->nx_packet_length);

            dest_address[0] = 0xff020000;
            dest_address[1] = 0x0;
            dest_address[2] = 0x0;
            dest_address[3] = 0x1;
            memcpy(ip_header_ptr->nx_ip_header_destination_ip, dest_address,
                   sizeof(dest_address));
            icmpv6_ra_ptr->nx_icmpv6_ra_flag = 0x40;
            aic_ipv6_add_ra_dns_option(ipv6_router_dev->ra_packet_ptr,
                                       icmpv6_ra_ptr, icmpv6_option_dns,
                                       src_address, dest_address,
                                       &payload_size);
            ip_header_ptr->nx_ip_header_word_1 &= 0x0000FFFF;
            ip_header_ptr->nx_ip_header_word_1 |= (payload_size << 16);
            ipv6_router_dev->ra_packet_ptr->nx_packet_prepend_ptr -=
                sizeof(NX_IPV6_HEADER);
            ipv6_router_dev->ra_packet_ptr->nx_packet_length +=
                sizeof(NX_IPV6_HEADER);
        }
    }

    aos_put_mutex(ipv6_forward_mutex);

    return AIC_SUCCESS;
}

/**
 * aic_ipv6_forward - Forward the packet to the destination interface.
 *
 * @ori_packet_ptr: Original packet pointer.
 * @dst_interface: Destination interface.
 * @is_unicast: Is it unicast.
 * @ip_ptr: IP instance pointer.
 *
 * Forward the packet to the destination interface, If it is multicast data, it
 * will apply for a new data packet internally to send.
 */
static int aic_ipv6_forward(NX_PACKET *ori_packet_ptr,
                            NX_INTERFACE *dst_interface,
                            u8 is_unicast,
                            NX_IP *ip_ptr)
{
    int ret;
    u16 checksum;
    u8 next_header_type;
    NX_ICMPV6_RA *icmpv6_ra_ptr;
    NXD_IPV6_ADDRESS *ipv6_addr;
    NX_PACKET *packet_ptr = NULL;
    NX_INTERFACE *local_interface;
    NX_IPV6_HEADER *ip_header_ptr;
    ND_CACHE_ENTRY *nd_cache_entry;
    ulong src_address[4], dest_address[4];
    NX_ICMPV6_HEADER *nx_icmpv6_ra_icmpv6_header;
    NX_ICMPV6_OPTION_DNS *icmpv6_option_dns = NULL;
    u32 src_addr_type, interface_type;
    u32 protocol, payload_size, hop_limit = 255, status;

    if (!dst_interface->nx_interface_link_up) {
        return AIC_ENODEV;
    }

    nx_icmpv6_ra_icmpv6_header =
        (NX_ICMPV6_HEADER *)(ori_packet_ptr->nx_packet_prepend_ptr +
                             sizeof(NX_IPV6_HEADER));
    ip_header_ptr = (NX_IPV6_HEADER *)ori_packet_ptr->nx_packet_prepend_ptr;
    next_header_type = (u8)((ip_header_ptr->nx_ip_header_word_1 >> 8) & 0xFF);

    if (!is_unicast ||
        (is_unicast && (next_header_type == NX_PROTOCOL_ICMPV6) &&
        (nx_icmpv6_ra_icmpv6_header->nx_icmpv6_header_type ==
         NX_ICMPV6_ROUTER_ADVERTISEMENT_TYPE))) {
        ret = nx_packet_copy(ori_packet_ptr, &packet_ptr,
                             ip_ptr->nx_ip_default_packet_pool,
                             AOS_NO_WAIT);
        if (!packet_ptr || (ret != NX_SUCCESS)) {
            aicx_error(0x198dc4, "x", "[IPV6_ROUTER] packet copy fail 0x%x\n", ret);
            return ret;
        }
    } else {
        packet_ptr = ori_packet_ptr;
    }

#ifdef NX_PACKET_HEADER_PAD
    packet_ptr->nx_packet_packet_pad[1] = *(u32 *)ori_packet_ptr->nx_packet_ip_header;
#endif

    ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr;
    nx_icmpv6_ra_icmpv6_header =
        (NX_ICMPV6_HEADER *)(packet_ptr->nx_packet_prepend_ptr +
                             sizeof(NX_IPV6_HEADER));

    src_addr_type = IPv6_Address_Type(ip_header_ptr->nx_ip_header_source_ip);
    next_header_type = (u8)((ip_header_ptr->nx_ip_header_word_1 >> 8) & 0xFF);

    local_interface = packet_ptr->nx_packet_ip_interface;
#ifdef CONFIG_NET_SHARE_COMMON
    interface_type =
        aic_network_find_interface_type(local_interface);
#endif

    if ((next_header_type != NX_PROTOCOL_ICMPV6) && is_unicast) {
        hop_limit = ip_header_ptr->nx_ip_header_word_1 & 0xFF;
        --hop_limit;
        if (!hop_limit) {
            aicx_debug(0x0c083f, "", "[IPV6_ROUTER] ipv6 hop limit is zero\n");
            if (ori_packet_ptr != packet_ptr)
                nx_packet_release(packet_ptr);
            return NX_INVALID_PACKET;
        }
    }

    protocol = (ip_header_ptr->nx_ip_header_word_1 & 0xFF00) >> 8;
    payload_size = ip_header_ptr->nx_ip_header_word_1 >> 16;
    memcpy(src_address, ip_header_ptr->nx_ip_header_source_ip,
           sizeof(src_address));
    memcpy(dest_address, ip_header_ptr->nx_ip_header_destination_ip,
           sizeof(dest_address));

    packet_ptr->nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER);
    packet_ptr->nx_packet_length -= sizeof(NX_IPV6_HEADER);

    if (next_header_type == NX_PROTOCOL_ICMPV6) {
        if ((src_addr_type & IPV6_ADDRESS_UNICAST) &&
            (interface_type != AIC_PDP_INTERFACE)) {
            if (_nx_nd_cache_find_entry(ip_ptr,
                                        ip_header_ptr->nx_ip_header_source_ip,
                                        &nd_cache_entry) != NX_SUCCESS) {
                status = _nxd_ipv6_interface_find(ip_ptr,
                                                  ip_header_ptr->nx_ip_header_source_ip,
                                                  &ipv6_addr, local_interface);
                if (status != NX_SUCCESS) {
                    aicx_info(0x095f71, "xxxxx", "[IPV6_ROUTER] 0x%x 0x%x 0x%x 0x%x find interface fail 0x%x when add nd cache\n",
                              ip_header_ptr->nx_ip_header_source_ip[0],
                              ip_header_ptr->nx_ip_header_source_ip[1],
                              ip_header_ptr->nx_ip_header_source_ip[2],
                              ip_header_ptr->nx_ip_header_source_ip[3], status);
                } else {
                    _nx_nd_cache_add(ip_ptr,
                                     ip_header_ptr->nx_ip_header_source_ip,
                                     local_interface,
                                     ori_packet_ptr->nx_packet_ip_header - 8, 0,
                                     ND_CACHE_STATE_REACHABLE, ipv6_addr,
                                     &nd_cache_entry);
                }
            }
        }

        if (nx_icmpv6_ra_icmpv6_header->nx_icmpv6_header_type ==
            NX_ICMPV6_ROUTER_ADVERTISEMENT_TYPE) {
            icmpv6_ra_ptr = (NX_ICMPV6_RA *)packet_ptr->nx_packet_prepend_ptr;
            icmpv6_option_dns =
                (NX_ICMPV6_OPTION_DNS *)(packet_ptr->nx_packet_prepend_ptr +
                                         packet_ptr->nx_packet_length);
            aic_ipv6_add_ra_dns_option(packet_ptr, icmpv6_ra_ptr,
                                       icmpv6_option_dns, src_address,
                                       dest_address, &payload_size);
         }
    }

    if (nx_icmpv6_ra_icmpv6_header->nx_icmpv6_header_type ==
        NX_ICMPV6_ROUTER_ADVERTISEMENT_TYPE &&
        next_header_type == NX_PROTOCOL_ICMPV6) {
        u32 interface_index = dst_interface->nx_interface_index;

        packet_ptr->nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);
        packet_ptr->nx_packet_length += sizeof(NX_IPV6_HEADER);

        ret = aic_ipv6_check_invalid_prefix(packet_ptr, interface_index);

        if (!ipv6_forward_mutex) {
            if (ori_packet_ptr != packet_ptr)
                nx_packet_release(packet_ptr);
            return AIC_ENODEV;
        }

        ret = aos_get_mutex(ipv6_forward_mutex, AOS_WAIT_FOREVER);
        if (ret) {
            if (ori_packet_ptr != packet_ptr)
                nx_packet_release(packet_ptr);
            return ret;
        }

        if (!aic_ipv6_route_dev[interface_index].ra_packet_ptr) {
            ret = nx_packet_copy(packet_ptr, &aic_ipv6_route_dev[interface_index].ra_packet_ptr,
                                 packet_ptr->nx_packet_pool_owner,
                                 AOS_NO_WAIT);
            if (ret)
                aicx_warn(0x0cea93, "x", "[IPV6_FORWARD] ra packet copy fail 0x%x \r\n", ret);
        }

        aos_put_mutex(ipv6_forward_mutex);
        packet_ptr->nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER);
        packet_ptr->nx_packet_length -= sizeof(NX_IPV6_HEADER);

    }
#ifdef CONFIG_NET_SHARE_COMMON
    else if (nx_icmpv6_ra_icmpv6_header->nx_icmpv6_header_type ==
             NX_ICMPV6_ROUTER_SOLICITATION_TYPE &&
             next_header_type == NX_PROTOCOL_ICMPV6) {
        aicx_info(0x00c1f2, "s", "[IPV6_FORWARD] recv rs from %s, auto reply ra\n",
                  local_interface->nx_interface_name);
        aic_network_ipv6_send_ra(ip_ptr, local_interface->nx_interface_index);
    }
#endif

    status = _nxd_ipv6_interface_find(ip_ptr, dest_address,
                                      &packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr,
                                      dst_interface);
    if ((status != NX_SUCCESS) ||
        (!status && (packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr->nxd_ipv6_address_attached == local_interface))) {
        aicx_info(0x0389c2, "ssxxxxx", "[IPV6_ROUTER] %s find IPV6 interface %s fail 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                  local_interface->nx_interface_name,
                  dst_interface->nx_interface_name, status, dest_address[0],
                  dest_address[1], dest_address[2], dest_address[3]);
        if (_nx_nd_cache_find_entry(ip_ptr, dest_address, &nd_cache_entry) !=
            NX_SUCCESS) {
            aicx_info(0x0f1ace, "sxxxx", "[IPV6_ROUTER] %s find IPV6 outgoing address fail 0x%x 0x%x 0x%x 0x%x\n",
                      local_interface->nx_interface_name,
                      dest_address[0], dest_address[1],
                      dest_address[2], dest_address[3]);
            if (ori_packet_ptr != packet_ptr)
                nx_packet_release(packet_ptr);
            return NX_INVALID_PARAMETERS;
        }

        packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr =
            nd_cache_entry->nx_nd_cache_outgoing_address;
    }

    if ((nx_icmpv6_ra_icmpv6_header->nx_icmpv6_header_type ==
         NX_ICMPV6_ROUTER_ADVERTISEMENT_TYPE) &&
        (next_header_type == NX_PROTOCOL_ICMPV6)) {
        nx_icmpv6_ra_icmpv6_header->nx_icmpv6_header_checksum = 0;
        memcpy(src_address, packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr->nxd_ipv6_address,
               sizeof(src_address));
        checksum = _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_ICMPV6,
                                           (UINT)packet_ptr->nx_packet_length,
                                           src_address, dest_address);
        checksum = (u16)(~checksum);

        NX_CHANGE_USHORT_ENDIAN(checksum);

        nx_icmpv6_ra_icmpv6_header->nx_icmpv6_header_checksum = checksum;
    }

    _nx_ipv6_packet_send(ip_ptr, packet_ptr, protocol, payload_size, hop_limit,
                         src_address, dest_address);

    if (is_unicast && (next_header_type == NX_PROTOCOL_ICMPV6) &&
        (nx_icmpv6_ra_icmpv6_header->nx_icmpv6_header_type ==
         NX_ICMPV6_ROUTER_ADVERTISEMENT_TYPE)) {
        nx_packet_release(ori_packet_ptr);
    }

    return AIC_SUCCESS;
}

void aic_ipv6_forward_table_timeout(void)
{
    int i, ret;
    struct list_head *list, *pos;
    struct aic_net_ipv6_forward_table *table;
    struct aic_network_ipv6_router *ipv6_router;

    ret = aos_get_mutex(ipv6_forward_table_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        return;
    }

    for (i = 0; i < NX_MAX_IP_INTERFACES; i++) {
        ipv6_router = &aic_ipv6_route_dev[i];
        list_for_each_safe(list, pos, &ipv6_router->forward_list) {
            table = list_entry(list, struct aic_net_ipv6_forward_table, list);
            if ((aic_get_tick() - table->timestamp) > 240000) {
                list_del(&table->list);
                aos_free(table);
                ipv6_router->used_table_num--;
            }
        }
    }
    aos_put_mutex(ipv6_forward_table_mutex);
}

/**
 * aic_ipv6_packet_router - Select a suitable destination interface to router.
 *
 * @packet_ptr: packet pointer.
 * @is_unicast: Is it unicast.
 *
 * Select a suitable destination interface to router, If the router
 * advertisement is received, ,it will be saved locally.
 */
int aic_ipv6_packet_router(NX_PACKET *packet_ptr, u8 is_unicast)
{
    NX_INTERFACE *local_interface;
    NX_IPV6_HEADER *ip_header_ptr;
    ND_CACHE_ENTRY *nd_cache_ptr = NULL;
    NX_INTERFACE *dst_interface = AIC_NULL;
    int i, ret = NX_INVALID_PACKET, index;
    u8 pdp_active_mode = PDP_ONE_TO_ONE_MODE;
    struct aic_network_ip_instance *aic_ip_instance;
    u32 addr_type, interface_type = AIC_NET_INTERFACE_CNT;

    ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr;
    addr_type = IPv6_Address_Type(ip_header_ptr->nx_ip_header_destination_ip);
    if (addr_type & IPV6_ADDRESS_LOOPBACK) {
        return NX_IP_ADDRESS_ERROR;
    }

    aic_ip_instance = aic_get_default_ip_instance();
    if (!aic_ip_instance || !aic_ip_instance->ip_ptr) {
        return AIC_ENODEV;
    }

    local_interface = packet_ptr->nx_packet_ip_interface;
    if (!local_interface || !local_interface->nx_interface_valid) {
        return AIC_ENODEV;
    }

#ifdef CONFIG_NET_SHARE_COMMON
    interface_type =
        aic_network_find_interface_type(local_interface);
    dst_interface =
        aic_network_match_interface(local_interface, &pdp_active_mode);
#endif

    if (interface_type == AIC_PDP_INTERFACE) {
        aic_ipv6_record_ra(packet_ptr);
    } else if (dst_interface) {
        ret = aic_ipv6_process_dhcpv6(aic_ip_instance->ip_ptr, packet_ptr,
                                      &aic_ipv6_route_dev[local_interface->nx_interface_index],
                                      dst_interface->nx_interface_index);
        if (!ret)
            return ret;
    }

    if (!ipv6_enable_cnt)
        return AIC_ENODEV;

    if (addr_type & IPV6_ADDRESS_LINKLOCAL) {
        for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++) {
            nd_cache_ptr = &aic_ip_instance->ip_ptr->nx_ipv6_nd_cache[i];
            if (CHECK_IPV6_ADDRESSES_SAME(ip_header_ptr->nx_ip_header_destination_ip,
                                          nd_cache_ptr->nx_nd_cache_dest_ip)) {
                break;
            }
        }

        if (i >= NX_IPV6_NEIGHBOR_CACHE_SIZE) {
            return NX_IP_ADDRESS_ERROR;
        }
    }

    if (!dst_interface && (pdp_active_mode == PDP_ONE_TO_ONE_MODE))
        return AIC_EACCES;

    switch (interface_type) {
    case AIC_PDP_INTERFACE:
        aic_ipv6_adjust_win(&aic_ipv6_route_dev[local_interface->nx_interface_index],
                            packet_ptr, AIC_IPV6_INBOUND);
        if (pdp_active_mode == PDP_ONE_TO_ONE_MODE) {
            ret = aic_ipv6_forward(packet_ptr, dst_interface, is_unicast,
                                   aic_ip_instance->ip_ptr);
            if (ret != AIC_SUCCESS) {
                return ret;
            }
        } else if ((pdp_active_mode == PDP_ONE_TO_MULTI_MODE) && !is_unicast) {
#ifdef CONFIG_NET_SHARE_COMMON
            index = aic_network_get_pdp_peer_interface(local_interface);
#endif
            for (i = 0; i < NX_MAX_IP_INTERFACES; i++) {
                if (!(index & BIT(i)))
                    continue;

                dst_interface = &aic_ip_instance->ip_ptr->nx_ip_interface[i];
                ret = aic_ipv6_forward(packet_ptr, dst_interface, is_unicast,
                                       aic_ip_instance->ip_ptr);
                if (ret != AIC_SUCCESS) {
                    return ret;
                }
            }
        } else if ((pdp_active_mode == PDP_ONE_TO_MULTI_MODE) && is_unicast) {
            if (!nd_cache_ptr) {
                for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++) {
                    nd_cache_ptr = &aic_ip_instance->ip_ptr->nx_ipv6_nd_cache[i];
                    if ((nd_cache_ptr->nx_nd_cache_nd_status != ND_CACHE_STATE_INVALID) &&
                        (nd_cache_ptr->nx_nd_cache_nd_status != ND_CACHE_STATE_INCOMPLETE) &&
                        CHECK_IPV6_ADDRESSES_SAME(ip_header_ptr->nx_ip_header_destination_ip,
                                                  nd_cache_ptr->nx_nd_cache_dest_ip)) {
                        break;
                    }
                }

                if (i >= NX_IPV6_NEIGHBOR_CACHE_SIZE)
                    nd_cache_ptr  = NULL;
            }

            if (!nd_cache_ptr) {
                aicx_warn(0x04af66, "", "[IPV6_ROUTER] can't find dst interface by nd cache table\n");
                return NX_NOT_FOUND;
            }

            ret = aic_ipv6_forward(packet_ptr,
                                   nd_cache_ptr->nx_nd_cache_interface_ptr,
                                   is_unicast,
                                   aic_ip_instance->ip_ptr);
            if (ret != AIC_SUCCESS) {
                aicx_warn(0x0ceaa0, "xx", "[IPV6_ROUTER] packet forward fail mode 0x%x index 0x%x!\n",
                          pdp_active_mode, i);
                return ret;
            }
        }
        break;
    case AIC_PPP_INTERFACE:
    case AIC_USB_RNDIS_INTERFACE:
    case AIC_USB_ECM_INTERFACE:
        if (!dst_interface) {
            return AIC_ENODEV;
        }

        aic_ipv6_adjust_win(&aic_ipv6_route_dev[dst_interface->nx_interface_index],
                            packet_ptr, AIC_IPV6_OUTBOUND);
        ret = aic_ipv6_forward(packet_ptr, dst_interface, is_unicast,
                               aic_ip_instance->ip_ptr);
        break;
    default:
        break;
    }

    return ret;
}

#ifdef CONFIG_NET_SHARE_COMMON
int aic_network_ipv6_send_ra(NX_IP *ip_ptr,
                             u32 interface_index)
{
    int ret;
    u32 wan_index;
    NX_INTERFACE *dst_interface;

    if (!ip_ptr || interface_index >= NX_MAX_IP_INTERFACES) {
        return AIC_EINVAL;
    }

    dst_interface = &ip_ptr->nx_ip_interface[interface_index];
    wan_index = aic_get_wan_intf_index(dst_interface);
    if (!aic_ipv6_route_dev[wan_index].ra_packet_ptr) {
        aicx_error(0x05b52d, "d", "[IPV6_FORWARD] %d ra packet ptr is null\r\n", wan_index);
        return AIC_EAGAIN;
    }

    if (aic_ipv6_route_dev[interface_index].ra_packet_ptr) {
        aic_ipv6_check_invalid_prefix(aic_ipv6_route_dev[wan_index].ra_packet_ptr,
                                      interface_index);
    }

    if (!aic_ipv6_route_dev[interface_index].ra_packet_ptr) {
        ret = nx_packet_copy(aic_ipv6_route_dev[wan_index].ra_packet_ptr,
                             &aic_ipv6_route_dev[interface_index].ra_packet_ptr,
                             aic_ipv6_route_dev[wan_index].ra_packet_ptr->nx_packet_pool_owner,
                             AOS_NO_WAIT);
        if (ret)
            aicx_warn(0x09e1dc, "x", "[IPV6_FORWARD] ra packet copy to lan fail 0x%x \r\n",
                      ret);
    }

    return aic_ipv6_send_ra(ip_ptr, aic_ipv6_route_dev[wan_index].ra_packet_ptr,
                            dst_interface);
}
#endif

/**
 * aic_enable_ipv6_packet_route - Enable IPv6 packet router.
 *
 * @aic_ip_instance: AIC ip instance pointer.
 */
void aic_enable_ipv6_packet_route(struct aic_network_ip_instance *
                                  aic_ip_instance)
{
    AOS_INTERRUPT_DECLARE

    if (!aic_ip_instance || !aic_ip_instance->ip_ptr)
        return;

    AOS_INTERRUPT_DISABLE
    ipv6_enable_cnt++;
    AOS_INTERRUPT_RESTORE
}

/**
 * aic_disable_ipv6_packet_route - Disable IPv6 packet router.
 *
 * @aic_ip_instance: AIC ip instance pointer.
 */
void aic_disable_ipv6_packet_route(struct aic_network_ip_instance *
                                   aic_ip_instance)
{
    AOS_INTERRUPT_DECLARE

    if (!aic_ip_instance || !aic_ip_instance->ip_ptr)
        return;

    AOS_INTERRUPT_DISABLE
    if (ipv6_enable_cnt > 0)
        ipv6_enable_cnt--;
    AOS_INTERRUPT_RESTORE
}

void aic_ipv6_forward_release_packet(u32 interface_index)
{
    int ret;

    if (!ipv6_forward_mutex) {
        return;
    }

    ret = aos_get_mutex(ipv6_forward_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        return;
    }

    if (aic_ipv6_route_dev[interface_index].ra_packet_ptr) {
        nx_packet_release(aic_ipv6_route_dev[interface_index].ra_packet_ptr);
        aic_ipv6_route_dev[interface_index].ra_packet_ptr = NULL;
    }

    aic_ipv6_route_dev[interface_index].duid_time = 0;

    aos_put_mutex(ipv6_forward_mutex);
}

int aic_ipv6_forward_init(void)
{
    int i;
    struct aic_network_ipv6_router *ipv6_dev;

    memset(aic_ipv6_route_dev, 0, sizeof(aic_ipv6_route_dev));
    ipv6_forward_mutex = aos_create_mutex("ipv6 forward mutex");
    if (!ipv6_forward_mutex) {
        return AIC_ENOMEM;
    }

    ipv6_forward_table_mutex = aos_create_mutex("ipv6 forward table mutex");
    if (!ipv6_forward_table_mutex) {
        return AIC_ENOMEM;
    }

    for (i = 0; i < NX_MAX_IP_INTERFACES; i++) {
        ipv6_dev = &aic_ipv6_route_dev[i];
        list_head_init(&ipv6_dev->forward_list);
        ipv6_dev->interface_id = i;
        ipv6_dev->max_table_num = AIC_IPV6_MAX_FORWARD_TABLE;
    }

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_ipv6_forward_init);

