/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** NetX DUO Component                                                       */
/**                                                                          */
/**   NAT64                                                                  */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x114879
#define LOG_MODULE    AMOD_NET

#include <nx_ip.h>
#include <nx_api.h>
#include <nx_ipv6.h>
#include <nxd_bsd.h>

#include <timer_inc.h>

#include <aic_cmd.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_nat64.h>
#include <aic_net_api.h>
#include <aic_network_pcap.h>

#define ICMP_ECHO           8
#define ICMP_ECHOREPLY      0

#define ICMPV6_ECHO_REQUEST 128
#define ICMPV6_ECHO_REPLY   129

struct aic_nat64_table {
    u16 ip_id;

    u16 ori_local_port;
    u16 new_local_port;
    u16 ori_external_port;
    u16 new_external_port;

    u8 ori_nexthdr;
    u8 new_nexthdr;

    u32 ori_local_ip;
    u32 ori_external_ip;

    struct in6_addr new_local_ip;
    struct in6_addr new_external_ip;

    u64 timestamp;
    struct aic_nat64_device *nat64_dev;

    struct list_head list;
};

#define AIC_NAT64_INBOUND  0
#define AIC_NAT64_OUTBOUND 1

static u32 aic_nat64_dev_mask = 0;
static aos_mutex_ptr aic_nat64_mutex;
static struct list_head aic_nat64_dev_list;

static void aic_nat64_config_icmp(NX_PACKET *packet_ptr, u16 type, u16 id)
{
    struct icmphdr *icmp_hdr;

    icmp_hdr = (struct icmphdr *)packet_ptr->nx_packet_prepend_ptr;
    icmp_hdr->type = type & 0xFF;
    icmp_hdr->code = type >> 8;
    icmp_hdr->un.echo.id = id;
    icmp_hdr->checksum = 0;
}

static void aic_nat64_config_tcp(NX_PACKET *packet_ptr, u16 src_port,
                                 u16 dst_port)
{
    struct tcphdr *tcp_hdr;

    tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;

    if (tcp_hdr->source != src_port) {
        aic_checksum_adjust((u8 *)&tcp_hdr->check, (u8 *)&tcp_hdr->source, 2,
                            (u8 *)&src_port, 2);
        tcp_hdr->source = src_port;
    }

    if (tcp_hdr->dest != dst_port) {
        aic_checksum_adjust((u8 *)&tcp_hdr->check, (u8 *)&tcp_hdr->dest, 2,
                            (u8 *)&dst_port, 2);
        tcp_hdr->dest = dst_port;
    }
}

static void aic_nat64_config_udp(NX_PACKET *packet_ptr, u16 src_port,
                                 u16 dst_port)
{
    struct udphdr *udp_hdr;

    udp_hdr = (struct udphdr *)packet_ptr->nx_packet_prepend_ptr;

    if (udp_hdr->check && (udp_hdr->source != src_port)) {
        aic_checksum_adjust((u8 *)&udp_hdr->check, (u8 *)&udp_hdr->source, 2,
                            (u8 *)&src_port, 2);
        udp_hdr->source = src_port;
    }

    if (udp_hdr->check && (udp_hdr->dest != dst_port)) {
        aic_checksum_adjust((u8 *)&udp_hdr->check, (u8 *)&udp_hdr->dest, 2,
                            (u8 *)&dst_port, 2);
        udp_hdr->dest = dst_port;
    }
}

static void aic_nat64_trans_ip4(NX_PACKET *packet_ptr,
                                struct aic_nat64_table *nat64_table)
{
    u16 checksum;
    int payload_len;
    struct udphdr *udp_hdr;
    struct tcphdr *tcp_hdr;
    struct icmp6hdr *icmp6_hdr;
    struct iphdr *ip_header_ptr;
    struct ipv6hdr ipv6_header_ptr;

    memset(&ipv6_header_ptr, 0, sizeof(ipv6_header_ptr));
    ip_header_ptr = (struct iphdr *)packet_ptr->nx_packet_ip_header;
    payload_len = packet_ptr->nx_packet_length;
    ipv6_header_ptr.version = 6;
    ipv6_header_ptr.priority = ip_header_ptr->tos >> 4;
    ipv6_header_ptr.flow_lbl[0] = ip_header_ptr->tos << 4;
    ipv6_header_ptr.flow_lbl[1] = 0;
    ipv6_header_ptr.flow_lbl[2] = 0;
    ipv6_header_ptr.payload_len = htons(payload_len);
    ipv6_header_ptr.nexthdr = nat64_table->new_nexthdr;
    ipv6_header_ptr.hop_limit = ip_header_ptr->ttl;
    memcpy(ipv6_header_ptr.saddr.s6_addr,
           nat64_table->nat64_dev->local_ipv6_prefix,
           nat64_table->nat64_dev->local_ipv6_prefix_len >> 3);
    ipv6_header_ptr.saddr.s6_addr32[3] = ip_header_ptr->saddr;
    memcpy(ipv6_header_ptr.daddr.s6_addr,
           nat64_table->nat64_dev->external_ipv6_prefix,
           nat64_table->nat64_dev->external_ipv6_prefix_len >> 3);
    ipv6_header_ptr.daddr.s6_addr32[3] = ip_header_ptr->daddr;
    packet_ptr->nx_packet_ip_version = NX_IP_VERSION_V6;

    switch (nat64_table->new_nexthdr) {
    case IPPROTO_TCP:
        tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;
        aic_checksum_reduce((unsigned char*)(&tcp_hdr->check), (unsigned char *)&ip_header_ptr->saddr, 8);
        aic_checksum_add((unsigned char*)(&tcp_hdr->check), (unsigned char *)&ipv6_header_ptr.saddr, 32);
        break;
    case IPPROTO_UDP:
        udp_hdr = (struct udphdr *)packet_ptr->nx_packet_prepend_ptr;
        if (udp_hdr->check) {
            aic_checksum_reduce((unsigned char*)(&udp_hdr->check), (unsigned char *)&ip_header_ptr->saddr, 8);
            aic_checksum_add((unsigned char*)(&udp_hdr->check), (unsigned char *)&ipv6_header_ptr.saddr, 32);
        } else {
            _nx_ipv6_address_change_endian(ipv6_header_ptr.saddr.s6_addr32);
            _nx_ipv6_address_change_endian(ipv6_header_ptr.daddr.s6_addr32);
            /* nx_packet_ip_version must be is NX_IP_VERSION_V6 */
            checksum = _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_UDP,
                                               (UINT)packet_ptr->nx_packet_length,
                                               ipv6_header_ptr.saddr.s6_addr32,
                                               ipv6_header_ptr.daddr.s6_addr32);
            checksum = (USHORT)(~checksum);
            NX_CHANGE_USHORT_ENDIAN(checksum);
            udp_hdr->check = checksum;
            _nx_ipv6_address_change_endian(ipv6_header_ptr.saddr.s6_addr32);
            _nx_ipv6_address_change_endian(ipv6_header_ptr.daddr.s6_addr32);
        }
        break;
    case IPPROTO_ICMPV6:
        icmp6_hdr = (struct icmp6hdr *)packet_ptr->nx_packet_prepend_ptr;
        _nx_ipv6_address_change_endian(ipv6_header_ptr.saddr.s6_addr32);
        _nx_ipv6_address_change_endian(ipv6_header_ptr.daddr.s6_addr32);
        /* nx_packet_ip_version must be is NX_IP_VERSION_V6 */
        checksum = _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_ICMPV6,
                                           (UINT)packet_ptr->nx_packet_length,
                                           ipv6_header_ptr.saddr.s6_addr32,
                                           ipv6_header_ptr.daddr.s6_addr32);
        checksum = (USHORT)(~checksum);
        NX_CHANGE_USHORT_ENDIAN(checksum);
        icmp6_hdr->icmp6_cksum = checksum;
        _nx_ipv6_address_change_endian(ipv6_header_ptr.saddr.s6_addr32);
        _nx_ipv6_address_change_endian(ipv6_header_ptr.daddr.s6_addr32);
        break;
    default:
        break;
    }

    memcpy(packet_ptr->nx_packet_prepend_ptr - sizeof(struct ipv6hdr),
           &ipv6_header_ptr, sizeof(struct ipv6hdr));
    packet_ptr->nx_packet_ip_header = packet_ptr->nx_packet_prepend_ptr - sizeof(struct ipv6hdr);
    packet_ptr->nx_packet_ip_header_length = sizeof(struct ipv6hdr);
}

static void aic_nat64_trans_ip6(NX_PACKET *packet_ptr,
                                struct aic_nat64_table *nat64_table)
{
    u32 val;
    u16 checksum;
    int payload_len;
    struct iphdr ip_hdr;
    struct iphdr *ipv4_hdr;
    struct udphdr *udp_hdr;
    struct tcphdr *tcp_hdr;
    struct ipv6hdr *ipv6_hdr;
    struct icmphdr *icmp_hdr;

    ipv6_hdr = (struct ipv6hdr *)packet_ptr->nx_packet_ip_header;
    payload_len = packet_ptr->nx_packet_length;
    packet_ptr->nx_packet_ip_version = NX_IP_VERSION_V4;

    memset(&ip_hdr, 0, sizeof(ip_hdr));
    ip_hdr.version = 4;
    ip_hdr.ihl = sizeof(struct iphdr) / 4;
    ip_hdr.tos = (ipv6_hdr->priority << 4) | (ipv6_hdr->flow_lbl[0] >> 4);
    ip_hdr.tot_len = htons(payload_len + sizeof(struct iphdr));
    ip_hdr.id = htons(++(nat64_table->ip_id));
    ip_hdr.frag_off = 0;
    ip_hdr.ttl = ipv6_hdr->hop_limit;
    ip_hdr.protocol = nat64_table->ori_nexthdr;
    if (nat64_table->nat64_dev->local_ipv6_prefix_len == 32)
        ip_hdr.saddr = ipv6_hdr->saddr.s6_addr32[1];
    else if (nat64_table->nat64_dev->local_ipv6_prefix_len == 96)
        ip_hdr.saddr = ipv6_hdr->saddr.s6_addr32[3];
    else
        aicx_info(0x11c5d6, "d", "[NAT64] ipv6 local prefix length %d not support\n",
                  nat64_table->nat64_dev->local_ipv6_prefix_len);

    if (nat64_table->nat64_dev->external_ipv6_prefix_len == 32)
        ip_hdr.daddr = ipv6_hdr->daddr.s6_addr32[1];
    else if (nat64_table->nat64_dev->external_ipv6_prefix_len == 96)
        ip_hdr.daddr = ipv6_hdr->daddr.s6_addr32[3];
    else
        aicx_info(0x17e85f, "d", "[NAT64] ipv6 external prefix length %d not support\n",
                  nat64_table->nat64_dev->external_ipv6_prefix_len);

    switch (nat64_table->ori_nexthdr) {
    case IPPROTO_TCP:
        tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;
        aic_checksum_reduce((unsigned char*)(&tcp_hdr->check),
                            (unsigned char *)&ipv6_hdr->saddr, 32);
        aic_checksum_add((unsigned char*)(&tcp_hdr->check),
                         (unsigned char *)&ip_hdr.saddr, 8);
        break;
    case IPPROTO_UDP:
        udp_hdr = (struct udphdr *)packet_ptr->nx_packet_prepend_ptr;
        if (udp_hdr->check) {
            aic_checksum_reduce((unsigned char*)(&udp_hdr->check),
                                (unsigned char *)&ipv6_hdr->saddr, 32);
            aic_checksum_add((unsigned char*)(&udp_hdr->check),
                             (unsigned char *)&ip_hdr.saddr, 8);
        }
        break;
    case IPPROTO_ICMP:
        icmp_hdr = (struct icmphdr *)packet_ptr->nx_packet_prepend_ptr;
        NX_CHANGE_ULONG_ENDIAN(ip_hdr.saddr);
        NX_CHANGE_ULONG_ENDIAN(ip_hdr.daddr);
        /* nx_packet_ip_version must be is NX_IP_VERSION_V6 */
        checksum = _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_ICMP,
                                           (UINT)packet_ptr->nx_packet_length,
                                           0, 0);
        checksum = (USHORT)(~checksum);
        NX_CHANGE_USHORT_ENDIAN(checksum);
        icmp_hdr->checksum = checksum;
        NX_CHANGE_ULONG_ENDIAN(ip_hdr.saddr);
        NX_CHANGE_ULONG_ENDIAN(ip_hdr.daddr);
        break;
    default:
        aicx_warn(0x149469, "d", "[NAT64] ori nexthdr %d not support shwn translate ipv6\n",
                  nat64_table->ori_nexthdr);
        break;
    }

    memcpy(packet_ptr->nx_packet_prepend_ptr - sizeof(struct iphdr), &ip_hdr,
           sizeof(struct iphdr));
    packet_ptr->nx_packet_prepend_ptr -= sizeof(struct iphdr);
    packet_ptr->nx_packet_length += sizeof(struct iphdr);
    packet_ptr->nx_packet_ip_header = packet_ptr->nx_packet_prepend_ptr;

    checksum = _nx_ip_checksum_compute(packet_ptr, NX_IP_VERSION_V4,
                                       (UINT)20, NULL, NULL);
    val = (ULONG)(~checksum);
    val = val & NX_LOWER_16_MASK;

    /* Convert to network byte order. */
    NX_CHANGE_ULONG_ENDIAN(val);

    /* Now store the checksum in the IP header.  */
    ipv4_hdr = (struct iphdr *)packet_ptr->nx_packet_prepend_ptr;
    ipv4_hdr->check = (u16)(val >> 16);
}

static void aic_nat64_convert_ipv6_to_ipv4(NX_PACKET *packet_ptr,
                                           struct aic_nat64_table *table)
{
    u16 ori_local_port;
    struct icmp6hdr *icmpv6_hdr;

    switch (table->ori_nexthdr) {
    case IPPROTO_UDP:
        aic_nat64_config_udp(packet_ptr, table->ori_external_port,
                             table->ori_local_port);
        break;
    case IPPROTO_TCP:
        aic_nat64_config_tcp(packet_ptr, table->ori_external_port,
                             table->ori_local_port);
        break;
    case IPPROTO_ICMP:
    case IPPROTO_ICMPV6:
        icmpv6_hdr = (struct icmp6hdr *)packet_ptr->nx_packet_prepend_ptr;
        switch(icmpv6_hdr->icmp6_type) {
        case ICMPV6_ECHO_REQUEST:
            ori_local_port = (u16)ICMP_ECHO |
                ((u16)table->ori_local_port & 0xFF00);
            aic_nat64_config_icmp(packet_ptr, ori_local_port,
                                  table->ori_external_port);
            break;
        case ICMPV6_ECHO_REPLY:
            ori_local_port = (u16)ICMP_ECHOREPLY |
                ((u16)table->ori_local_port & 0xFF00);
            aic_nat64_config_icmp(packet_ptr, ori_local_port,
                                  table->ori_external_port);
            break;
        default:
            aicx_warn(0x1ae3f0, "d", "[NAT64] not support %d icmpv6 type when convert\n",
                      icmpv6_hdr->icmp6_type);
        }
    }

    aic_nat64_trans_ip6(packet_ptr, table);
}

static void aic_nat64_convert_ipv4_to_ipv6(NX_PACKET *packet_ptr,
                                           struct aic_nat64_table *table)
{
    switch (table->new_nexthdr) {
    case IPPROTO_UDP:
        aic_nat64_config_udp(packet_ptr, table->new_local_port,
                             table->new_external_port);
        break;
    case IPPROTO_TCP:
        aic_nat64_config_tcp(packet_ptr, table->new_local_port,
                             table->new_external_port);
        break;
    case IPPROTO_ICMP:
    case IPPROTO_ICMPV6:
        aic_nat64_config_icmp(packet_ptr, table->new_local_port,
                              table->new_external_port);
        break;
    }

    aic_nat64_trans_ip4(packet_ptr, table);
}

static struct aic_nat64_device *aic_find_nat64_dev_by_id(u32 dev_id)
{
    u32 hit = 0;
    struct aic_nat64_device *pos = NULL;
    struct aic_nat64_device *nat64_dev = NULL;

    if (list_empty(&aic_nat64_dev_list)) {
        aicx_warn(0x1270b2, "", "[NAT64] dev list id empty\n");
        return NULL;
    }

    list_for_each_entry_safe(nat64_dev, pos, &aic_nat64_dev_list,
                             list, struct aic_nat64_device) {
        if (nat64_dev->id == dev_id) {
            hit = TRUE;
            break;
        }
    }

    if (!hit) {
        aicx_warn(0x04055e, "d", "[NAT64] dev_id %d not found\n", dev_id);
        return NULL;
    }

    return nat64_dev;
}

static struct aic_nat64_table *aic_nat64_create_outbound_table(u32 dev_id,
                                                               NX_PACKET *packet_ptr)
{
    u8 protocol;
    struct iphdr *ip_hdr;
    struct udphdr *udp_hdr;
    struct tcphdr *tcp_hdr;
    s64 delta_time = 120000;
    struct icmphdr *icmp_hdr;
    struct list_head *list, *pos;
    struct aic_nat64_table *table;
    struct aic_nat64_device *nat64_dev;

    nat64_dev = aic_find_nat64_dev_by_id(dev_id);
    if (!nat64_dev) {
        aicx_error(0x1dcbca, "d", "[NAT64] cat find nat64 dev %d\n", dev_id);
        return NULL;
    }

REMOVE_AGAIN:
    if (nat64_dev->used_table_num > nat64_dev->max_table_num) {
        list_for_each_safe(list, pos, &nat64_dev->table_list) {
            table = list_entry(list, struct aic_nat64_table, list);
            if ((aic_get_tick() - table->timestamp) > delta_time) {
                list_del(&table->list);
                aos_free(table);
                nat64_dev->used_table_num--;
            }
        }
    }

    if (nat64_dev->used_table_num > nat64_dev->max_table_num) {
        delta_time -= 30000;
        if (delta_time > 0) {
            goto REMOVE_AGAIN;
        } else {
            aicx_warn(0x182314, "", "[NAT64] outbound table is full\n");
            return NULL;
        }
    }

    table = (struct aic_nat64_table *)aos_zalloc(sizeof(struct aic_nat64_table));
    if (!table) {
        aicx_error(0x09370f, "", "[NAT64] alloc nat64 table fail\n");
        return NULL;
    }

    table->timestamp = aic_get_tick();
    if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4 &&
        packet_ptr->nx_packet_ip_header) {
        ip_hdr = (struct iphdr *)packet_ptr->nx_packet_ip_header;
        protocol = ip_hdr->protocol;
        table->ori_local_ip = ip_hdr->saddr;
        table->ori_external_ip = ip_hdr->daddr;
        memcpy(table->new_local_ip.s6_addr, nat64_dev->local_ipv6_prefix,
               nat64_dev->local_ipv6_prefix_len >> 3);
        table->new_local_ip.s6_addr32[3] = ip_hdr->saddr;
        memcpy(table->new_external_ip.s6_addr,
               nat64_dev->external_ipv6_prefix,
               nat64_dev->external_ipv6_prefix_len >> 3);
        table->new_external_ip.s6_addr32[3] = ip_hdr->daddr;
    } else {
        aicx_error(0x09e3dd, "dx", "[NAT64] packet is wrong %d 0x%x\n",
                   packet_ptr->nx_packet_ip_version,
                   packet_ptr->nx_packet_ip_header);
        aos_free(table);
        return NULL;
    }

    switch (protocol) {
    case IPPROTO_TCP:
        tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;

        table->new_nexthdr = IPPROTO_TCP;
        table->ori_nexthdr = IPPROTO_TCP;
        table->ori_local_port = tcp_hdr->source;
        table->ori_external_port = tcp_hdr->dest;
        table->new_local_port = tcp_hdr->source;
        table->new_external_port = tcp_hdr->dest;
        table->nat64_dev = nat64_dev;
        break;
    case IPPROTO_UDP:
        udp_hdr = (struct udphdr *)packet_ptr->nx_packet_prepend_ptr;

        table->new_nexthdr = IPPROTO_UDP;
        table->ori_nexthdr = IPPROTO_UDP;
        table->ori_local_port = udp_hdr->source;
        table->ori_external_port = udp_hdr->dest;
        table->new_local_port = udp_hdr->source;
        table->new_external_port = udp_hdr->dest;
        table->nat64_dev = nat64_dev;
        break;
    case IPPROTO_ICMP:
        icmp_hdr = (struct icmphdr *)packet_ptr->nx_packet_prepend_ptr;

        table->new_nexthdr = IPPROTO_ICMPV6;
        table->ori_nexthdr = IPPROTO_ICMP;
        switch(icmp_hdr->type) {
        case ICMP_ECHO:
            table->new_local_port = (u16)ICMPV6_ECHO_REQUEST | ((u16)icmp_hdr->code << 8);
            break;
        case ICMP_ECHOREPLY:
            table->new_local_port = (u16)ICMPV6_ECHO_REPLY | ((u16)icmp_hdr->code << 8);
            break;
        default:
            aicx_warn(0x007e48, "d", "[NAT64] not support %d icmp type\n", icmp_hdr->type);
            aos_free(table);
            return NULL;
        }

        table->ori_local_port = (u16)icmp_hdr->type | ((u16)icmp_hdr->code << 8);
        table->ori_external_port = icmp_hdr->un.echo.id;
        table->new_external_port = icmp_hdr->un.echo.id;
        table->nat64_dev = nat64_dev;
        break;
    default:
        aicx_warn(0x1423b3, "d", "[NAT64] protocol %d not support when create outbound table\n",
                  protocol);
        aos_free(table);
        return NULL;
    }

    nat64_dev->used_table_num++;
    aicx_info(0x14a426, "dd", "[NAT64] dev %d add outbound nat64 table %d\n", nat64_dev->id,
              nat64_dev->used_table_num);
    list_add(&table->list, &nat64_dev->table_list);
    return table;
}

static struct aic_nat64_table *aic_nat64_create_inbound_table(u32 dev_id,
                                                              NX_PACKET *packet_ptr)
{
    u8 protocol;
    struct udphdr *udp_hdr;
    struct tcphdr *tcp_hdr;
    s64 delta_time = 240000;
    struct ipv6hdr *ipv6_hdr;
    struct icmp6hdr *icmpv6_hdr;
    struct list_head *list, *pos;
    struct aic_nat64_table *table;
    struct aic_nat64_device *nat64_dev;

    nat64_dev = aic_find_nat64_dev_by_id(dev_id);
    if (!nat64_dev) {
        aicx_error(0x1a47f0, "d", "[NAT64] can't find nat64 dev %d\n", dev_id);
        return NULL;
    }

REMOVE_AGAIN:
    if (nat64_dev->used_table_num > nat64_dev->max_table_num) {
        list_for_each_safe(list, pos, &nat64_dev->table_list) {
            table = list_entry(list, struct aic_nat64_table, list);
            if ((aic_get_tick() - table->timestamp) > delta_time) {
                list_del(&table->list);
                aos_free(table);
                nat64_dev->used_table_num--;
            }
        }
    }

    if (nat64_dev->used_table_num > nat64_dev->max_table_num) {
        delta_time -= 30000;
        if (delta_time) {
            goto REMOVE_AGAIN;
        } else {
            aicx_warn(0x182314, "", "[NAT64] outbound table is full\n");
            return NULL;
        }
    }

    table = (struct aic_nat64_table *)aos_zalloc(sizeof(struct aic_nat64_table));
    if (!table) {
        aicx_error(0x09370f, "", "[NAT64] alloc nat64 table fail\n");
        return NULL;
    }

    table->timestamp = aic_get_tick();

    if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6 &&
        packet_ptr->nx_packet_ip_header) {
        ipv6_hdr = (struct ipv6hdr *)packet_ptr->nx_packet_ip_header;
        protocol = ipv6_hdr->nexthdr;
        table->ori_local_ip = ipv6_hdr->daddr.s6_addr32[3];
        table->ori_external_ip = ipv6_hdr->saddr.s6_addr32[3];
        memcpy(table->new_local_ip.s6_addr, ipv6_hdr->daddr.s6_addr,
               16);
        memcpy(table->new_external_ip.s6_addr, ipv6_hdr->saddr.s6_addr,
               16);
    } else {
        aicx_error(0x09e3dd, "dx", "[NAT64] packet is wrong %d 0x%x\n",
                   packet_ptr->nx_packet_ip_version,
                   packet_ptr->nx_packet_ip_header);
        aos_free(table);
        return NULL;
    }

    switch (protocol) {
    case IPPROTO_TCP:
        tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;

        table->new_nexthdr = IPPROTO_TCP;
        table->ori_nexthdr = IPPROTO_TCP;
        table->ori_local_port = tcp_hdr->dest;
        table->ori_external_port = tcp_hdr->source;
        table->new_local_port = tcp_hdr->dest;
        table->new_external_port = tcp_hdr->source;
        table->nat64_dev = nat64_dev;
        break;
    case IPPROTO_UDP:
        udp_hdr = (struct udphdr *)packet_ptr->nx_packet_prepend_ptr;

        table->new_nexthdr = IPPROTO_UDP;
        table->ori_nexthdr = IPPROTO_UDP;
        table->ori_local_port = udp_hdr->dest;
        table->ori_external_port = udp_hdr->source;
        table->new_local_port = udp_hdr->dest;
        table->new_external_port = udp_hdr->source;
        table->nat64_dev = nat64_dev;
        break;
    case IPPROTO_ICMPV6:
        icmpv6_hdr = (struct icmp6hdr *)packet_ptr->nx_packet_prepend_ptr;

        table->new_nexthdr = IPPROTO_ICMPV6;
        table->ori_nexthdr = IPPROTO_ICMP;
        switch(icmpv6_hdr->icmp6_type) {
        case ICMPV6_ECHO_REQUEST:
            table->ori_local_port = (u16)ICMP_ECHOREPLY | ((u16)icmpv6_hdr->icmp6_code << 8);
            table->new_local_port = (u16)ICMPV6_ECHO_REPLY | ((u16)icmpv6_hdr->icmp6_code << 8);
            break;
        case ICMPV6_ECHO_REPLY:
            table->ori_local_port = (u16)ICMP_ECHO | ((u16)icmpv6_hdr->icmp6_code << 8);
            table->new_local_port = (u16)ICMPV6_ECHO_REQUEST | ((u16)icmpv6_hdr->icmp6_code << 8);
            break;
        default:
            aicx_warn(0x1c9bcb, "d", "[NAT64] not support %d icmpv6 type\n", icmpv6_hdr->icmp6_type);
            aos_free(table);
            return NULL;
        }

        table->ori_external_port = icmpv6_hdr->icmp6_dataun.u_echo.identifier;
        table->new_external_port = icmpv6_hdr->icmp6_dataun.u_echo.identifier;
        table->nat64_dev = nat64_dev;
        break;
    default:
        aicx_warn(0x132b54, "d", "[NAT64] next_hdr %d not support when create inbound table\n",
                  protocol);
        aos_free(table);
        return NULL;
    }

    nat64_dev->used_table_num++;
    aicx_info(0x04446e, "dd", "[NAT64] dev %d add inbound nat64 table %d\n", nat64_dev->id,
              nat64_dev->used_table_num);
    list_add(&table->list, &nat64_dev->table_list);
    return table;
}

static struct aic_nat64_table *aic_nat64_find_table(struct aic_nat64_device *nat64_dev,
                                                    NX_PACKET *packet_ptr,
                                                    u8 packet_type)
{
    u8 next_hdr;
    struct iphdr *ip_hdr;
    u16 src_port, dst_port;
    struct udphdr *udp_hdr;
    struct tcphdr *tcp_hdr;
    struct ipv6hdr *ipv6_hdr;
    struct icmphdr *icmp_hdr;
    struct icmp6hdr *icmp6_hdr;
    struct list_head *list, *pos;
    struct aic_nat64_table *nat64_table = NULL;

    if ((packet_type != AIC_NAT64_INBOUND) &&
        (packet_type != AIC_NAT64_OUTBOUND)) {
        aicx_error(0x144bd9, "d", "[NAT64] packet type %d is error\n", packet_type);
        return NULL;
    }

    list_for_each_safe(list, pos, &nat64_dev->table_list) {
        nat64_table = list_entry(list, struct aic_nat64_table, list);
        if (packet_type == AIC_NAT64_OUTBOUND) {
            ip_hdr = (struct iphdr *)packet_ptr->nx_packet_ip_header;
            next_hdr = ip_hdr->protocol;
            switch (next_hdr) {
            case IPPROTO_TCP:
                tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;
                src_port = tcp_hdr->source;
                dst_port = tcp_hdr->dest;
                break;
            case IPPROTO_UDP:
                udp_hdr = (struct udphdr *)packet_ptr->nx_packet_prepend_ptr;
                src_port = udp_hdr->source;
                dst_port = udp_hdr->dest;
                break;
            case IPPROTO_ICMP:
                icmp_hdr = (struct icmphdr *)packet_ptr->nx_packet_prepend_ptr;
                src_port = (u16)icmp_hdr->type | ((u16)icmp_hdr->code << 8);
                dst_port = icmp_hdr->un.echo.id;
                break;
            default:
                aicx_error(0x13f852, "d", "[NAT64] protocol %d not support when find table\n",
                           next_hdr);
                return NULL;
            }

            if ((nat64_table->ori_nexthdr == next_hdr) &&
                (nat64_table->ori_external_port == dst_port) &&
                (nat64_table->ori_local_port == src_port) &&
                (nat64_table->ori_local_ip == ip_hdr->saddr) &&
                (nat64_table->ori_external_ip == ip_hdr->daddr)) {
                nat64_table->timestamp = aic_get_tick();
                break;
            }
        } else {
            ipv6_hdr = (struct ipv6hdr *)packet_ptr->nx_packet_ip_header;
            next_hdr = ipv6_hdr->nexthdr;
            switch (next_hdr) {
            case IPPROTO_TCP:
                tcp_hdr = (struct tcphdr *)packet_ptr->nx_packet_prepend_ptr;
                src_port = tcp_hdr->source;
                dst_port = tcp_hdr->dest;
                break;
            case IPPROTO_UDP:
                udp_hdr = (struct udphdr *)packet_ptr->nx_packet_prepend_ptr;
                src_port = udp_hdr->source;
                dst_port = udp_hdr->dest;
                break;
            case IPPROTO_ICMPV6:
                icmp6_hdr = (struct icmp6hdr *)packet_ptr->nx_packet_prepend_ptr;
                switch (icmp6_hdr->icmp6_type) {
                case ICMPV6_ECHO_REQUEST:
                    dst_port = (u16)ICMPV6_ECHO_REPLY |
                                    ((u16)icmp6_hdr->icmp6_code << 8);
                    break;
                case ICMPV6_ECHO_REPLY:
                    dst_port = (u16)ICMPV6_ECHO_REQUEST |
                                    ((u16)icmp6_hdr->icmp6_code << 8);
                    break;
                default:
                    aicx_error(0x1cec00, "d", "[NAT64] icmp6_type %d not support when find table\n",
                               icmp6_hdr->icmp6_type);
                    return NULL;
                }

                src_port = icmp6_hdr->icmp6_dataun.u_echo.identifier;
                break;
            default:
                aicx_error(0x09f7b0, "d", "[NAT64] nexthdr %d not support when find table\n",
                           next_hdr);
                return NULL;
            }

            if ((nat64_table->new_nexthdr == next_hdr) &&
                (nat64_table->new_external_port == src_port) &&
                (nat64_table->new_local_port == dst_port) &&
                !memcmp((u8 *)&nat64_table->new_external_ip,
                        (u8 *)&ipv6_hdr->saddr, 16) &&
                !memcmp((u8 *)&nat64_table->new_local_ip,
                        (u8 *)&ipv6_hdr->daddr, 16)) {
                nat64_table->timestamp = aic_get_tick();
                break;
            }
        }
    }

    if (list != &nat64_dev->table_list)
        return nat64_table;

    if (packet_type == AIC_NAT64_INBOUND)
        return aic_nat64_create_inbound_table(nat64_dev->id, packet_ptr);
    else
        return aic_nat64_create_outbound_table(nat64_dev->id, packet_ptr);
}

static int aic_nat64_check_464_packet(NX_PACKET *packet_ptr)
{
    int ret;
    struct ipv6hdr *ipv6_hdr;
    struct aic_nat64_device *nat64_dev;

    ret = aos_get_mutex(aic_nat64_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        aicx_warn(0x068a04, "x", "[NAT64] get nat64 mutex fail 0x%x when check\n", ret);
        return FALSE;
    }

    if (packet_ptr->nx_packet_ip_version != NX_IP_VERSION_V6) {
        aos_put_mutex(aic_nat64_mutex);
        aicx_debug(0x08ec01, "d", "[NAT64] ip version %d not 464 packet\n",
                   packet_ptr->nx_packet_ip_version);
        return FALSE;
    }

    nat64_dev = aic_find_nat64_dev_by_id(packet_ptr->nx_packet_ip_interface->nat64_dev_id);
    if (!nat64_dev) {
        aos_put_mutex(aic_nat64_mutex);
        aicx_debug(0x09ce81, "d", "[NAT64] dev id %d not 464 packet\n",
                  packet_ptr->nx_packet_ip_interface->nat64_dev_id);
        return FALSE;
    }

    ipv6_hdr = (struct ipv6hdr *)packet_ptr->nx_packet_ip_header;
    if (htonl(ipv6_hdr->saddr.s6_addr32[0]) != nat64_dev->external_ipv6_prefix[0] ||
        htonl(ipv6_hdr->saddr.s6_addr32[1]) != nat64_dev->external_ipv6_prefix[1] ||
        htonl(ipv6_hdr->saddr.s6_addr32[2]) != nat64_dev->external_ipv6_prefix[2] ||
        htonl(ipv6_hdr->daddr.s6_addr32[0]) != nat64_dev->local_ipv6_prefix[0] ||
        htonl(ipv6_hdr->daddr.s6_addr32[1]) != nat64_dev->local_ipv6_prefix[1] ||
        htonl(ipv6_hdr->daddr.s6_addr32[2]) != nat64_dev->local_ipv6_prefix[2]) {
        aos_put_mutex(aic_nat64_mutex);
        aicx_debug(0x03e13b, "", "[NAT64] prefix is not equal not 464 packet\n");
        return FALSE;
    }

    aos_put_mutex(aic_nat64_mutex);

    aicx_debug(0x1de705, "", "[NAT64] this is 464 packet\n");
    return TRUE;
}

static void aic_nat64_process_inbound_packet(NX_PACKET *packet_ptr)
{
    int ret;
    struct ipv6hdr *ipv6_hdr;
    struct aic_nat64_table *table;
    struct aic_nat64_device *nat64_dev;

    if (packet_ptr->nx_packet_ip_version != NX_IP_VERSION_V6) {
        aicx_info(0x0093fe, "d", "[NAT64][INBOUND] packet version %d\n",
                  packet_ptr->nx_packet_ip_version);
        return;
    }

    ret = aos_get_mutex(aic_nat64_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        aicx_warn(0x02ab73, "x", "[NAT64] get nat64 mutex fail 0x%x when process inbound\n", ret);
        return;
    }

    nat64_dev = aic_find_nat64_dev_by_id(packet_ptr->nx_packet_ip_interface->nat64_dev_id);
    if (!nat64_dev) {
        aos_put_mutex(aic_nat64_mutex);
        aicx_info(0x10c9c6, "d", "[NAT64][INBOUND] dev id %d not exist\n",
                  packet_ptr->nx_packet_ip_interface->nat64_dev_id);
        return;
    }

    ipv6_hdr = (struct ipv6hdr *)packet_ptr->nx_packet_ip_header;
    if (ipv6_hdr->saddr.s6_addr32[0] != nat64_dev->external_ipv6_prefix[0] ||
        ipv6_hdr->saddr.s6_addr32[1] != nat64_dev->external_ipv6_prefix[1] ||
        ipv6_hdr->saddr.s6_addr32[2] != nat64_dev->external_ipv6_prefix[2] ||
        ipv6_hdr->daddr.s6_addr32[0] != nat64_dev->local_ipv6_prefix[0] ||
        ipv6_hdr->daddr.s6_addr32[1] != nat64_dev->local_ipv6_prefix[1] ||
        ipv6_hdr->daddr.s6_addr32[2] != nat64_dev->local_ipv6_prefix[2]) {
        aos_put_mutex(aic_nat64_mutex);
        alog_info("[NAT64][INBOUND] prefix check fail, id %d external 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                  nat64_dev->id, ipv6_hdr->saddr.s6_addr32[0], nat64_dev->external_ipv6_prefix[0],
                  ipv6_hdr->saddr.s6_addr32[1], nat64_dev->external_ipv6_prefix[1],
                  ipv6_hdr->saddr.s6_addr32[2], nat64_dev->external_ipv6_prefix[2]);
        alog_info("[NAT64][INBOUND] prefix check fail, local prefix 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                  ipv6_hdr->daddr.s6_addr32[0], nat64_dev->local_ipv6_prefix[0],
                  ipv6_hdr->daddr.s6_addr32[1], nat64_dev->local_ipv6_prefix[1],
                  ipv6_hdr->daddr.s6_addr32[2], nat64_dev->local_ipv6_prefix[2]);
        return;
    }

    table = aic_nat64_find_table(nat64_dev, packet_ptr, AIC_NAT64_INBOUND);
    if (!table) {
        aos_put_mutex(aic_nat64_mutex);
        aicx_info(0x0fd653, "", "[NAT64][INBOUND] find table fail\n");
        return;
    }

    aic_nat64_convert_ipv6_to_ipv4(packet_ptr, table);
    aos_put_mutex(aic_nat64_mutex);
}

static void aic_nat64_process_outbound_packet(NX_PACKET *packet_ptr)
{
    int ret;
    struct aic_nat64_table *table;
    struct aic_nat64_device *nat64_dev;
    struct NXD_IPV6_ADDRESS_STRUCT *ipv6_address_list_head;

    if (packet_ptr->nx_packet_ip_version != NX_IP_VERSION_V4)
        return;

    ipv6_address_list_head = packet_ptr->nx_packet_ip_interface->nxd_interface_ipv6_address_list_head;
    while (ipv6_address_list_head) {
        if ((ipv6_address_list_head->nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_VALID) &&
            (IPv6_Address_Type(ipv6_address_list_head->nxd_ipv6_address) & IPV6_ADDRESS_GLOBAL)) {
            break;
        } else {
            ipv6_address_list_head = ipv6_address_list_head->nxd_ipv6_address_next;
        }
    }

    if (!ipv6_address_list_head) {
        aicx_warn(0x05db16, "", "[NAT64] ipv6 addr not ready when process outbound\n");
        return;
    }

    packet_ptr->nx_packet_ip_header = packet_ptr->nx_packet_prepend_ptr;
    packet_ptr->nx_packet_prepend_ptr += sizeof(struct iphdr);
    packet_ptr->nx_packet_length -= sizeof(struct iphdr);

    ret = aos_get_mutex(aic_nat64_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        aicx_warn(0x13108c, "x", "[NAT64] get nat64 mutex fail 0x%x when process outbound\n", ret);
        return;
    }

    nat64_dev = aic_find_nat64_dev_by_id(packet_ptr->nx_packet_ip_interface->nat64_dev_id);
    if (!nat64_dev) {
        aos_put_mutex(aic_nat64_mutex);
        return;
    }

    table = aic_nat64_find_table(nat64_dev, packet_ptr, AIC_NAT64_OUTBOUND);
    if (!table) {
        aos_put_mutex(aic_nat64_mutex);
        return;
    }

    aic_nat64_convert_ipv4_to_ipv6(packet_ptr, table);
    aos_put_mutex(aic_nat64_mutex);

    packet_ptr->nx_packet_prepend_ptr -= sizeof(struct ipv6hdr);
    packet_ptr->nx_packet_length += sizeof(struct ipv6hdr);

    packet_ptr->nx_packet_address.nx_packet_ipv6_address_ptr = ipv6_address_list_head;
}

static int aic_nat64_create_device(u32 ipv4_addr, u32 ipv4_mask, u8 local_prefix_len,
                                   u8 *local_prefix, u8 external_prefix_len,
                                   u8 *external_prefix, u32 max_table_num)
{
    int i;
    struct list_head *list, *pos;
    struct aic_nat64_device *nat64_dev;

    list_for_each_safe(list, pos, &aic_nat64_dev_list) {
        nat64_dev = list_entry(list, struct aic_nat64_device, list);
        if (nat64_dev->ipv4_addr == ipv4_addr &&
            nat64_dev->ipv4_netmask == ipv4_mask &&
            !memcmp(nat64_dev->local_ipv6_prefix, local_prefix,
                    local_prefix_len >> 3) &&
            !memcmp(nat64_dev->external_ipv6_prefix, external_prefix,
                    external_prefix_len >> 3)) {
            aicx_info(0x1d66ed, "", "[NAT64] nat64 device has already created\n");
            return nat64_dev->id;
        }
    }

    for (i = 0 ; i < 32; i++) {
        if (!(aic_nat64_dev_mask & BIT(i)))
            break;
    }

    if (i >= 32) {
        aicx_error(0x0d802b, "", "[NAT64] nat64 device is full\n");
        return ENFILE;
    }

    aic_nat64_dev_mask |= BIT(i);
    nat64_dev = aos_zalloc(sizeof(struct aic_nat64_device));
    if (!nat64_dev) {
        aicx_error(0x020dfe, "", "[NAT64] alloc nat64 device fail\n");
        return ENOMEM;
    }

    nat64_dev->ipv4_addr = ipv4_addr;
    nat64_dev->ipv4_netmask = ipv4_mask;
    nat64_dev->local_ipv6_prefix_len = local_prefix_len;
    memcpy(nat64_dev->local_ipv6_prefix, local_prefix, local_prefix_len >> 3);
    nat64_dev->external_ipv6_prefix_len = external_prefix_len;
    memcpy(nat64_dev->external_ipv6_prefix, external_prefix,
           external_prefix_len >> 3);

    list_add(&nat64_dev->list, &aic_nat64_dev_list);
    list_head_init(&nat64_dev->table_list);

    nat64_dev->id = i;
    nat64_dev->max_table_num = max_table_num;
    nat64_dev->used_table_num = 0;

    alog_info("[NAT64] create device id %d\n", i);

    return i;
}

static void aic_nat64_remove_device(u32 id)
{
    struct list_head *list, *pos;
    struct aic_nat64_table *nat64_table = NULL;
    struct aic_nat64_device *nat64_dev = NULL;

    nat64_dev = aic_find_nat64_dev_by_id(id);
    if (!nat64_dev) {
        aicx_warn(0x0adef7, "d", "[NAT64] nat64 dev %d not exist\n", id);
        return;
    }

    list_del(&nat64_dev->list);

    if (!nat64_dev) {
        aicx_error(0x0cb944, "d", "[NAT64] remove nat64 dev %d fail\n", id);
        return;
    }

    aic_nat64_dev_mask &= ~BIT(id);
    alog_info("[NAT64] remove device id %d\n", id);
    list_for_each_safe(list, pos, &nat64_dev->table_list) {
        nat64_table = list_entry(list, struct aic_nat64_table, list);
        list_del(list);
        aos_free(nat64_table);
    }

    aos_free(nat64_dev);
}

static void aic_nat64_table_timeout(NX_INTERFACE *interface_ptr)
{
    struct list_head *list, *pos;
    struct aic_nat64_table *table;
    struct aic_nat64_device *nat64_dev = NULL;

    nat64_dev = aic_find_nat64_dev_by_id(interface_ptr->nat64_dev_id);
    if (!nat64_dev) {
        aicx_warn(0x0adef7, "d", "[NAT64] nat64 dev %d not exist\n",
                  interface_ptr->nat64_dev_id);
        return;
    }

    list_for_each_safe(list, pos, &nat64_dev->table_list) {
        table = list_entry(list, struct aic_nat64_table, list);
        if ((aic_get_tick() - table->timestamp) > 240000) {
            list_del(&table->list);
            aos_free(table);
            nat64_dev->used_table_num--;
        }
    }
}

static int aic_nat64_init(void)
{
    aic_nat64_mutex = aos_create_mutex("aic nat64 mutex");
    if (!aic_nat64_mutex)
        aicx_warn(0x057545, "", "[NAT64] create nat64 mutex fail");

    list_head_init(&aic_nat64_dev_list);
    return 0;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_nat64_init);

int aic_nat64_enable(u32 interface_id, u32 ipv4_addr, u32 ipv4_mask,
                     u8 local_prefix_len, u8 *local_prefix,
                     u8 external_prefix_len, u8 *external_prefix,
                     u32 max_table_num)
{
    int ret;
    NX_INTERFACE *interface_ptr;
    struct aic_network_ip_instance *aic_ip;

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_error(0x08e702, "", "[NAT64] get ip instance fail\n");
        return ENODEV;
    }

    if (interface_id > NX_MAX_IP_INTERFACES) {
        aicx_error(0x169ef6, "d", "[NAT64] inteface id %d is wrong\n", interface_id);
        return EINVAL;
    }

    interface_ptr = &aic_ip->ip_ptr->nx_ip_interface[interface_id];
    if (interface_ptr->nat64_dev_id != -1) {
        alog_info("[NAT64] interface_id %d nat64 dev has already enable\n",
                  interface_id);
        return EBUSY;
    }

    nx_ip_interface_address_set(aic_ip->ip_ptr, interface_id, ipv4_addr,
                                ipv4_mask);
    ret = aos_get_mutex(aic_nat64_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        aicx_warn(0x07c0f2, "xd", "[NAT64] get nat64 mutex fail 0x%x when enable %d\n", ret, interface_id);
        return ENOLCK;
    }

    alog_info("[NAT64] enable local prefix %d 0x%x 0x%x 0x%x external_prefix %d 0x%x 0x%x 0x%x\n",
              local_prefix_len, *(u32 *)local_prefix, *((u32 *)local_prefix + 1),
              *((u32 *)local_prefix + 2), external_prefix_len,
              *(u32 *)external_prefix, *((u32 *)external_prefix + 1),
              *((u32 *)external_prefix + 2));

    interface_ptr->nat64_dev_id = aic_nat64_create_device(ipv4_addr, ipv4_mask,
                                                          local_prefix_len,
                                                          local_prefix,
                                                          external_prefix_len,
                                                          external_prefix,
                                                          max_table_num);
    if (interface_ptr->nat64_dev_id < 0) {
        aos_put_mutex(aic_nat64_mutex);
        aicx_error(0x09e22c, "", "[NAT64] create nat64 device fail\n");
        return interface_ptr->nat64_dev_id;
    }

    aos_put_mutex(aic_nat64_mutex);

    interface_ptr->nx_interface_nat64_check_464_packet =
        aic_nat64_check_464_packet;
    interface_ptr->nx_interface_nat64_inbound =
        aic_nat64_process_inbound_packet;
    interface_ptr->nx_interface_nat64_outbound =
        aic_nat64_process_outbound_packet;
    interface_ptr->nx_interface_nat64_table_timeout =
        aic_nat64_table_timeout;

    return 0;
}

int aic_nat64_disable(u32 interface_id)
{
    int ret;
    NX_INTERFACE *interface_ptr;
    struct aic_network_ip_instance *aic_ip;

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_error(0x08e702, "", "[NAT64] get ip instance fail\n");
        return ENODEV;
    }

    if (interface_id > NX_MAX_IP_INTERFACES) {
        aicx_error(0x169ef6, "d", "[NAT64] inteface id %d is wrong\n", interface_id);
        return EINVAL;
    }

    interface_ptr = &aic_ip->ip_ptr->nx_ip_interface[interface_id];
    if (interface_ptr->nat64_dev_id < 0)
        return interface_ptr->nat64_dev_id;

    ret = aos_get_mutex(aic_nat64_mutex, AOS_WAIT_FOREVER);
    if (ret) {
        aicx_warn(0x07c0f2, "xd", "[NAT64] get nat64 mutex fail 0x%x when enable %d\n", ret, interface_id);
        return ENOLCK;
    }

    aic_nat64_remove_device(interface_ptr->nat64_dev_id);
    aos_put_mutex(aic_nat64_mutex);

    interface_ptr->nx_interface_nat64_check_464_packet = NULL;
    interface_ptr->nx_interface_nat64_inbound = NULL;
    interface_ptr->nx_interface_nat64_outbound = NULL;
    interface_ptr->nx_interface_nat64_table_timeout = NULL;
    interface_ptr->nat64_dev_id = -1;

    return 0;
}

#ifdef CONFIG_PLAT_CONSOLE
unsigned char eth_ipv4_tcp_no_need_check[] = {
    0x45, 0x00,
    0x00, 0x3c, 0xa6, 0xd4, 0x00, 0x00, 0x80, 0x01, 0xaa, 0x47, 0xc0, 0xa8, 0x34, 0x53, 0xc0, 0xa8,
    0x34, 0x01, 0x08, 0x00, 0x4c, 0x99, 0x00, 0x01, 0x00, 0xc2, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69
};

static int aic_nat64_cmd(int argc, char **argv)
{
    u32 ret;
    NX_PACKET *packet_ptr;
    u32 ipv4_addr, ipv4_addr_mask;
    struct aic_network_ip_instance *aic_ip_instance;
    u8 ipv6_local_prefix[16] = {0x20, 0x01, 0x0d, 0xb8, 0xaa, 0xaa, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    u8 ipv6_external_prefix[16] = {0x20, 0x01, 0x0d, 0xb8, 0xbb, 0xbb, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    aic_ip_instance = aic_get_default_ip_instance();
    if (!aic_ip_instance || !aic_ip_instance->ip_ptr) {
        aicx_error(0x009446, "", "[NAT64] network maybe not active\n");
        return -1;
    }

    ret = nx_packet_allocate(aic_ip_instance->ip_ptr->nx_ip_default_packet_pool,
                             &packet_ptr, NX_PHYSICAL_HEADER, NX_NO_WAIT);
    if (ret || !packet_ptr) {
        aicx_error(0x1e0d34, "", "[NAT64] mem not enungh\n");
        return ret;
    }

    ret = nx_packet_data_append(packet_ptr, eth_ipv4_tcp_no_need_check,
                                sizeof(eth_ipv4_tcp_no_need_check),
                                aic_ip_instance->ip_ptr->nx_ip_default_packet_pool,
                                NX_NO_WAIT);
    aic_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr,
                          packet_ptr->nx_packet_length,
                          AIC_PDP_NIC_FLAG);
    packet_ptr->nx_packet_ip_interface = &aic_ip_instance->ip_ptr->nx_ip_interface[NX_LOOPBACK_INTERFACE];

    ipv4_addr = htonl(IP_ADDRESS(192, 168, 0, 6));
    ipv4_addr_mask = htonl(IP_ADDRESS(255, 255, 255, 0));
    aic_nat64_enable(NX_LOOPBACK_INTERFACE, ipv4_addr, ipv4_addr_mask, 96,
                     ipv6_local_prefix, 96, ipv6_external_prefix, 300);
    aic_nat64_process_outbound_packet(packet_ptr);
    aic_nat64_disable(NX_LOOPBACK_INTERFACE);

    aic_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr,
                          packet_ptr->nx_packet_length,
                          AIC_PDP_NIC_FLAG);

    aic_nat64_enable(NX_LOOPBACK_INTERFACE, ipv4_addr, ipv4_addr_mask, 96,
                     ipv6_external_prefix, 96, ipv6_local_prefix, 300);
    packet_ptr->nx_packet_prepend_ptr += sizeof(struct ipv6hdr);
    packet_ptr->nx_packet_length -= sizeof(struct ipv6hdr);
    aic_nat64_process_inbound_packet(packet_ptr);
    aic_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr,
                          packet_ptr->nx_packet_length,
                          AIC_PDP_NIC_FLAG);

    nx_packet_release(packet_ptr);
    return 0;
}
CONSOLE_EXPORT_CMD(nat64, aic_nat64_cmd, nat64);
#endif

