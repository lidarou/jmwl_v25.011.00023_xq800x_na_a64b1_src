/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** AIC Component                                                            */
/**                                                                          */
/**   Netword driver direct forward                                          */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e350c
#define LOG_MODULE    AMOD_NET

#include <aic_cmd.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>

#include "nx_ip.h"
#include "nx_api.h"
#include "nx_packet.h"
#include "nxd_bsd.h"
#include "nx_ipv4.h"
#include "nx_ipv6.h"
#include "nx_tcp.h"
#include "nx_udp.h"

#include "timer_inc.h"

//#include "aic_network_pcap.h"
#include "aic_net_api.h"
#include "aic_net_direct_forward.h"

static struct aic_net_direct_forward_dev aic_drv_forward_dev[NX_MAX_IP_INTERFACES] = {0};

static struct aic_net_fragment_forward *
aic_get_valid_frag_cfg(struct aic_net_direct_forward_dev *forward_dev)
{
    int i;

    for (i = 0; i < AIC_NET_MAX_FRAGS; i++) {
        if (!forward_dev->frag_arrays[i].in_use) {
            forward_dev->frag_arrays[i].in_use = TRUE;
            return &forward_dev->frag_arrays[i];
        }

        if ((aic_get_timestamp() - forward_dev->frag_arrays[i].init_timestamp) >
            NX_IPV4_MAX_REASSEMBLY_TIME * 1000000) {
            memset(&forward_dev->frag_arrays[i], 0,
                   sizeof(struct aic_net_fragment_forward));
            forward_dev->frag_arrays[i].in_use = TRUE;
            return &forward_dev->frag_arrays[i];
        }
    }

    aicx_info(0x118f38, "", "[DIRECT] find valid frag cfg fail\r\n");
    return NULL;
}

static struct aic_net_fragment_forward *
aic_check_frag_param(struct aic_net_direct_forward_dev *forward_dev,
                     NX_IPV4_HEADER *head)
{
    int i;

    for (i = 0; i < AIC_NET_MAX_FRAGS; i++) {
        if (!forward_dev->frag_arrays[i].in_use)
            continue;

        if ((forward_dev->frag_arrays[i].ip_id ==
            (head->nx_ip_header_word_1 >> NX_SHIFT_BY_16)) &&
            (((head->nx_ip_header_word_2 & NX_IP_PROTOCOL_MASK) >>
             NX_SHIFT_BY_16) == forward_dev->frag_arrays[i].proto) &&
            (head->nx_ip_header_source_ip ==
             forward_dev->frag_arrays[i].src_addr) &&
            (head->nx_ip_header_destination_ip ==
             forward_dev->frag_arrays[i].dst_addr))
            return &forward_dev->frag_arrays[i];
    }

    aicx_info(0x19cdc8, "", "[DIRECT] find frag param fail\r\n");
    return NULL;
}

static struct aic_net_fragment_forward *
aic_net_process_frag_packet(struct aic_net_direct_forward_dev *forward_dev,
                            NX_PACKET *packet_ptr, u16 *frag_flag,
                            u16 *frag_offset)
{
    NX_IPV4_HEADER *ipv4_header;
    struct aic_net_fragment_forward *frag_cfg = NULL;

    ipv4_header = (NX_IPV4_HEADER *)packet_ptr->nx_packet_prepend_ptr;
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_word_1);
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_word_2);
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_source_ip);
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_destination_ip);

    *frag_flag = (ipv4_header->nx_ip_header_word_1 >> 13) & 0x7;
    *frag_offset = ipv4_header->nx_ip_header_word_1 & 0x1FFF;

    if (*frag_offset) {
        frag_cfg = aic_check_frag_param(forward_dev, ipv4_header);
    } else if ((*frag_flag & BIT(0)) && !*frag_offset) {
        frag_cfg = aic_get_valid_frag_cfg(forward_dev);
        if (frag_cfg) {
            frag_cfg->in_use = TRUE;
            frag_cfg->init_timestamp = aic_get_timestamp();
            frag_cfg->ip_id = ipv4_header->nx_ip_header_word_1 >> NX_SHIFT_BY_16;
            frag_cfg->proto = ((ipv4_header->nx_ip_header_word_2 &
                                NX_IP_PROTOCOL_MASK) >> NX_SHIFT_BY_16);
            frag_cfg->src_addr = ipv4_header->nx_ip_header_source_ip;
            frag_cfg->dst_addr = ipv4_header->nx_ip_header_destination_ip;
            aicx_debug(0x0b341e, "xd", "[DIRECT] get frag cfg successfully 0x%x %d\r\n",
                       *frag_flag, *frag_offset);
        }
    }

    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_word_1);
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_word_2);
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_source_ip);
    NX_CHANGE_ULONG_ENDIAN(ipv4_header->nx_ip_header_destination_ip);

    return frag_cfg;
}

static void aic_net_checksum_adjust(u8 *checksum, u8 *old_data,
                                    int old_data_length, u8 *new_data,
                                    int new_data_length)
{
    long x, old_value, new_value;
    int i, j;

#ifdef NX_LITTLE_ENDIAN
    i = 1;
    j = 0;
#else
    i = 0;
    j = 1;
#endif

    /* Checksum Adjustment, RFC 3022, Section 4.2, Page 9.  */

    /* Get the old checksum.  */
    x = checksum[i] * 256 + checksum[j];
    x = ~x & 0xFFFF;

    /* Update the checksum for old data.  */
    while (old_data_length) {
        /* Get the old data.  */
        old_value = old_data[i] * 256 + old_data[j];

        /* Update the old data pointer.  */
        old_data += 2;

        /* Update the checksum.  */
        x -= old_value & 0xFFFF;

        /* Check the value.  */
        if (x <= 0) {
            /* Update the value.  */
            x--;
            x &= 0xFFFF;
        }

        /* Update the old data length.  */
        old_data_length -= 2;
    }

    /* Update the checksum for new data.  */
    while (new_data_length) {
        /* Get the new data.  */
        new_value = new_data[i] * 256 + new_data[j];

        /* Update the new data pointer.  */
        new_data += 2;

        /* Update the checksum.  */
        x += new_value & 0xFFFF;

        /* Check the value.  */
        if (x & 0x10000) {
            /* Update the value.  */
            x++;
            x &= 0xFFFF;
        }

        /* Update the new data length.  */
        new_data_length -= 2;
    }

    /* Update the value.  */
    x = ~x & 0xFFFF;

    /* Update the checksum.  */
    checksum[i] = (UCHAR)(x / 256);
    checksum[j] = (UCHAR)(x & 0xFF);

    /* Return.  */
    return;
}

static int aic_net_update_transport_port(NX_PACKET *packet_ptr,
                                         u8 is_src_port, u16 port)
{
    int ret;
    u16 checksum, src_port, dst_port;
    u8 protocol = 0;
    u32 ip_header_len = 0;
    NX_TCP_HEADER *tcp_header_ptr;
    NX_UDP_HEADER *udp_header_ptr;

    ret = aic_net_get_trans_params(packet_ptr, &protocol, &ip_header_len,
                                   NULL, NULL);
    if (ret)
        return ret;

    switch (protocol) {
    case NX_PROTOCOL_TCP:
        tcp_header_ptr =
            (NX_TCP_HEADER *)(packet_ptr->nx_packet_ip_header +
                              ip_header_len);
        checksum = htons(*(u16 *)((u8 *)tcp_header_ptr + 16));

        if (is_src_port) {
            src_port = htons(*(u16 *)tcp_header_ptr);
            aic_net_checksum_adjust((u8 *)&checksum, (u8 *)&src_port, 2,
                                    (u8 *)&port, 2);
            *((u16 *)tcp_header_ptr) = htons(port);
        } else {
            dst_port = htons(*(u16 *)((u8 *)tcp_header_ptr + 2));
            aic_net_checksum_adjust((u8 *)&checksum, (u8 *)&dst_port, 2,
                                    (u8 *)&port, 2);
            *((u16 *)tcp_header_ptr + 1) = htons(port);
        }

        *(u16 *)((u8 *)tcp_header_ptr + 16) = htons(checksum);
        break;
    case NX_PROTOCOL_UDP:
        udp_header_ptr =
            (NX_UDP_HEADER *)(packet_ptr->nx_packet_ip_header +
                              ip_header_len);
        checksum = htons(*(u16 *)((u8 *)udp_header_ptr + 6));

        if (is_src_port) {
            src_port = htons(*(u16 *)udp_header_ptr);
            aic_net_checksum_adjust((u8 *)&checksum, (u8 *)&src_port, 2,
                                    (u8 *)&port, 2);
            *((u16 *)udp_header_ptr) = htons(port);
        } else {
            dst_port = htons(*(u16 *)((u8 *)udp_header_ptr + 2));
            aic_net_checksum_adjust((u8 *)&checksum, (u8 *)&dst_port, 2,
                                    (u8 *)&port, 2);
            *((u16 *)udp_header_ptr + 1) = htons(port);
        }

        *(u16 *)((u8 *)udp_header_ptr + 6) = htons(checksum);
        break;
    default:
        break;
    }

    return AIC_SUCCESS;
}

static int aic_net_up_link_direct_trans_packet(NET_UP_PKT_DIRECT *pkt_direct,
                                               NX_PACKET *packet_ptr)
{
    NX_IP_DRIVER driver_request;

    driver_request.nx_ip_driver_ptr = pkt_direct->ip_ptr;
    driver_request.nx_ip_driver_packet = packet_ptr;
    driver_request.nx_ip_driver_interface =
        &pkt_direct->ip_ptr->nx_ip_interface[pkt_direct->intf_index];
    driver_request.nx_ip_driver_return_ptr = (ULONG *)pkt_direct;
    driver_request.nx_ip_driver_command = NX_LINK_PACKET_SEND;

    if (!driver_request.nx_ip_driver_interface->nx_interface_link_up) {
        nx_packet_release(packet_ptr);
        return AIC_EACCES;
    }

    packet_ptr->nx_packet_ip_interface = driver_request.nx_ip_driver_interface;
    (driver_request.nx_ip_driver_interface->nx_interface_link_driver_entry)(&driver_request);
    if (driver_request.nx_ip_driver_status) {
        nx_packet_release(packet_ptr);
        return driver_request.nx_ip_driver_status;
    }

    return AIC_SUCCESS;
}

static int aic_net_down_link_direct_trans_packet(NET_DOWN_PKT_DIRECT *pkt_direct,
                                                 NX_PACKET *packet_ptr)
{
    NX_ARP *arp_ptr;
    AOS_INTERRUPT_DECLARE
    NX_IP_DRIVER driver_request;
    NX_IPV4_HEADER *ipv4_header;
    u32 index, network_mask, network, destination_ip;

    if (packet_ptr->nx_packet_ip_version != NX_IP_VERSION_V4) {
        aicx_error(0x0bd9c2, "x", "[DIRECT] can't support this ip version 0x%x\n",
                   packet_ptr->nx_packet_ip_version);
        nx_packet_release(packet_ptr);
        return AIC_EINVAL;
    }

    ipv4_header = (NX_IPV4_HEADER *)packet_ptr->nx_packet_ip_header;
    destination_ip = ipv4_header->nx_ip_header_destination_ip;
    NX_CHANGE_ULONG_ENDIAN(destination_ip);

    driver_request.nx_ip_driver_ptr = pkt_direct->ip_ptr;
    driver_request.nx_ip_driver_packet = packet_ptr;
    driver_request.nx_ip_driver_interface =
        &pkt_direct->ip_ptr->nx_ip_interface[pkt_direct->intf_index];
    driver_request.nx_ip_driver_return_ptr = (ULONG *)pkt_direct;
    driver_request.nx_ip_driver_command = NX_LINK_PACKET_DIRECT_SEND;

    if (!driver_request.nx_ip_driver_interface->nx_interface_link_up) {
        nx_packet_release(packet_ptr);
        return AIC_EACCES;
    }

    packet_ptr->nx_packet_ip_interface = driver_request.nx_ip_driver_interface;
    if (packet_ptr ->nx_packet_ip_interface->nx_interface_address_mapping_needed) {
        /* Get the network and network mask.*/
        network_mask = packet_ptr->nx_packet_ip_interface->nx_interface_ip_network_mask;
        network = packet_ptr->nx_packet_address.nx_packet_interface_ptr->nx_interface_ip_network;

        /* Determine if an IP limited or directed broadcast is requested. */
        if ((destination_ip == NX_IP_LIMITED_BROADCAST) ||
            (((destination_ip & network_mask) == network) &&
             ((destination_ip & ~network_mask) == ~network_mask))) {

            /* Build the driver request. */
            driver_request.nx_ip_driver_command = NX_LINK_PACKET_BROADCAST;
            driver_request.nx_ip_driver_physical_address_msw = 0xFFFFUL;
            driver_request.nx_ip_driver_physical_address_lsw = 0xFFFFFFFFUL;
        } else if ((destination_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE) {
            /* Yes, we have a class D multicast address. Derive the physical
               mapping from the class D address. */

            /* Determine if the group address has been joined in this IP instance.  */
            index =  0;
            while (index < NX_MAX_MULTICAST_GROUPS) {
                /* Determine if the destination address matches the requested address.  */
                if (pkt_direct->ip_ptr->nx_ipv4_multicast_entry[index].nx_ipv4_multicast_join_list ==
                    destination_ip) {
                    /* Yes, break the loop!  */
                    break;
                }

                /* Increment the join list index.  */
                index++;
            }

            /* Build the driver request. Derive the physical mapping from
               the class D address.  */
            driver_request.nx_ip_driver_physical_address_msw = NX_IP_MULTICAST_UPPER;
            driver_request.nx_ip_driver_physical_address_lsw =
                NX_IP_MULTICAST_LOWER | (destination_ip & NX_IP_MULTICAST_MASK);
        } else {
            /* Look into the ARP Routing Table to derive the physical address. */

            /* Calculate the hash index for the destination IP address.  */
            index =  (UINT)((destination_ip + (destination_ip >> 8)) & NX_ARP_TABLE_MASK);

            /* Determine if there is an entry for this IP address.  */
            arp_ptr =  pkt_direct->ip_ptr->nx_ip_arp_table[index];

            /* Loop to look for an ARP match.  */
            while (arp_ptr) {
                /* Determine if this arp entry matches the destination IP address. */
                if (arp_ptr->nx_arp_ip_address == destination_ip) {
                    /* Yes, we found a match.  Get out of the loop! */
                    break;
                }

                /* Move to the next active ARP entry. */
                arp_ptr = arp_ptr->nx_arp_active_next;

                /* Determine if we are at the end of the ARP list. */
                if (arp_ptr == pkt_direct->ip_ptr->nx_ip_arp_table[index]) {
                    /* Clear the ARP pointer.  */
                    arp_ptr =  NX_NULL;
                    break;
                }
            }

            /* Determine if we actually found a matching and effective ARP entry.  */
            if ((arp_ptr) &&
                (arp_ptr->nx_arp_physical_address_msw |
                 arp_ptr->nx_arp_physical_address_lsw)) {

                /* Disable interrupts temporarily.  */
                AOS_INTERRUPT_DISABLE

                /* Yes, we have a physical mapping. Copy the physical address into the driver
                   request structure. */
                driver_request.nx_ip_driver_physical_address_msw = arp_ptr->nx_arp_physical_address_msw;
                driver_request.nx_ip_driver_physical_address_lsw = arp_ptr->nx_arp_physical_address_lsw;

                /* Move this ARP entry to the head of the list.  */
                pkt_direct->ip_ptr->nx_ip_arp_table[index] = arp_ptr;
                /* Restore interrupts.  */
                AOS_INTERRUPT_RESTORE
            }
        }
    }

    (driver_request.nx_ip_driver_interface->nx_interface_link_driver_entry)(&driver_request);
    if (driver_request.nx_ip_driver_status) {
        nx_packet_release(packet_ptr);
        return driver_request.nx_ip_driver_status;
    }

    return AIC_SUCCESS;
}

void aic_net_down_link_update_dst_port(NX_PACKET *packet_ptr, u8 netid)
{
    u32 tmp;
    int ret;
    u8 proto = 0;
    s32 interface_type;
    NX_IPV4_HEADER *ipv4_header;
    u16 dst_port = 0, map_port, frag_flag, frag_offset;

    ipv4_header = (NX_IPV4_HEADER *)packet_ptr->nx_packet_prepend_ptr;
    tmp = ipv4_header->nx_ip_header_word_1;
    NX_CHANGE_ULONG_ENDIAN(tmp);
    frag_flag = (tmp >> 13) & 0x7;
    frag_offset = tmp & 0x1FFF;

    if (frag_offset)
        return;

    ret = aic_net_get_trans_params(packet_ptr, &proto, NULL, NULL, &dst_port);
    aicx_debug(0x15d795, "sdxxx", "[%s_DL]: ret %d proto: 0x%x dst_port 0x%x packet 0x%x\r\n",
               packet_ptr->nx_packet_ip_interface->nx_interface_name,
               ret, proto, dst_port, packet_ptr);
    if (proto == NX_PROTOCOL_TCP || proto == NX_PROTOCOL_UDP) {
        interface_type =
            aic_network_find_interface_type(packet_ptr->nx_packet_ip_interface);

        if (interface_type == AIC_PPP_INTERFACE) {
            ret = aic_get_ppp_dial_dst_port(dst_port, &map_port);
        } else if (interface_type == AIC_USB_RNDIS_INTERFACE ||
                   interface_type == AIC_USB_ECM_INTERFACE ||
                   interface_type == AIC_USB_NCM_INTERFACE) {
            ret = aic_get_usb_net_dst_port(dst_port, netid, &map_port);
        } else if (interface_type == AIC_USB_SEMI_TETHER_INTERFACE) {
            /* Not handle semi tether condition here. */
            return;
        } else {
            aicx_warn(0x0d233c, "sd", "[%s_DL] can't support this interface type %d\r\n",
                      packet_ptr->nx_packet_ip_interface->nx_interface_name,
                      interface_type);
            return;
        }

        if (ret) {
            aicx_error(0x1b3b0d, "sd", "[%s_DL] ppp get dst map port fail %d\r\n",
                       packet_ptr->nx_packet_ip_interface->nx_interface_name,
                       ret);
        } else {
            aicx_debug(0x0400d9, "sxd", "[%s_DL]: dst_port 0x%x map_port %d\r\n",
                       packet_ptr->nx_packet_ip_interface->nx_interface_name,
                       dst_port, map_port);
            if (dst_port != map_port)
                aic_net_update_transport_port(packet_ptr, FALSE, map_port);
        }
    }
}

int aic_net_down_link_direct_dispatch(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                      u8 netid)
{
    int ret;
    u8 proto = 0;
    NET_DOWN_PKT_DIRECT pkt_direct;
    struct aic_net_fragment_forward *frag_cfg;
    struct aic_net_direct_forward_dev *forward_dev;
    u16 src_port = 0, dst_port = 0, frag_flag, frag_offset;

    forward_dev = &aic_drv_forward_dev[packet_ptr->nx_packet_ip_interface->nx_interface_index];

    frag_cfg = aic_net_process_frag_packet(forward_dev, packet_ptr,
                                           &frag_flag, &frag_offset);

    if (frag_cfg && frag_cfg->down_frag_in_progress && frag_offset) {
        ret = 0;
        memcpy(&pkt_direct, &frag_cfg->down_direct, sizeof(pkt_direct));
    } else if (frag_cfg && frag_offset && !frag_cfg->down_frag_in_progress) {
        aicx_info(0x1e52f8, "", "[LTE_DL] there may be packet loss\n");
        nx_packet_release(packet_ptr);
        return DIRECT_FORWARD_CONSUMER;
    } else if (!frag_offset) {
        aic_net_get_trans_params(packet_ptr, &proto, NULL, &src_port,
                                 &dst_port);

        if (proto == NX_PROTOCOL_TCP || proto == NX_PROTOCOL_UDP)
          ret = aic_down_pkt_direct_with_port(dst_port, netid + 1,
                                              &pkt_direct);
        else
          ret = aic_down_pkt_direct_no_port(netid + 1, &pkt_direct);

        if (frag_cfg && frag_flag & BIT(0) && !frag_offset) {
            frag_cfg->down_frag_in_progress = TRUE;
            memcpy(&frag_cfg->down_direct, &pkt_direct, sizeof(pkt_direct));
        } else if (frag_cfg &&
                   ((frag_flag & BIT(1)) || (!frag_flag && !frag_offset)) &&
                   frag_cfg->down_frag_in_progress) {
            memset(frag_cfg, 0, sizeof(struct aic_net_fragment_forward));
        }
    } else {
        aicx_info(0x033338, "sxxd", "[%s_DL] frag cfg 0x%x, frag_flag 0x%x frag_offset %d there may be packet loss\n",
                  packet_ptr->nx_packet_ip_interface->nx_interface_name,
                  frag_cfg, frag_flag, frag_offset);
        nx_packet_release(packet_ptr);
        return DIRECT_FORWARD_CONSUMER;
    }

    if (frag_cfg && frag_offset && !(frag_flag & BIT(0)))
        memset(frag_cfg, 0, sizeof(struct aic_net_fragment_forward));

    aicx_debug(0x14a0ce, "xxxxxxxxd", "[LTE_DL] proto 0x%x dst_port 0x%x ip_ptr 0x%x intf 0x%x to_nat 0x%x packet 0x%x len 0x%x frag_flag 0x%x offset %d\r\n",
               proto, dst_port, pkt_direct.ip_ptr, pkt_direct.intf_index,
               pkt_direct.to_nat, packet_ptr, packet_ptr->nx_packet_length,
               frag_flag, frag_offset);
    if (ret) {
        aicx_error(0x1b43e2, "xd", "[LTE_DL] pkt direct fail 0x%x proto %d\r\n", ret, proto);
        nx_packet_release(packet_ptr);
        if (frag_cfg)
            memset(frag_cfg, 0, sizeof(struct aic_net_fragment_forward));
        return DIRECT_FORWARD_CONSUMER;
    } else {
        if (pkt_direct.to_nat == ANET_PKT_NO_NAT) {
            ret = aic_net_down_link_direct_trans_packet(&pkt_direct,
                                                        packet_ptr);
            if (ret)
                aicx_error(0x1c0782, "x", "[LTE_DL] link direct trans fail 0x%x\r\n", ret);

            return DIRECT_FORWARD_CONSUMER;
        } else if (pkt_direct.to_nat == ANET_PKT_MULTI_HOME) {
            NX_PACKET *packet_copy;

            ret = nx_packet_copy(packet_ptr, &packet_copy,
                                 ip_ptr->nx_ip_default_packet_pool,
                                 NX_NO_WAIT);
            if (!packet_copy) {
                aicx_error(0x050e90, "x", "[LTE_DL] alloc packet fail when multi home 0x%x\r\n",
                           ret);
            } else {
                ret = aic_net_down_link_direct_trans_packet(&pkt_direct,
                                                            packet_ptr);
                if (ret)
                    aicx_error(0x10ae08, "x", "[LTE_DL] direct trans fail 0x%x multi home\r\n",
                               ret);

                _nx_ip_packet_deferred_receive(ip_ptr, packet_copy);
                return DIRECT_FORWARD_CONSUMER;
            }
        }
    }

    return DIRECT_FORWARD_BYPASS;
}

int aic_net_up_link_direct_dispatch(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                    u8 netid)
{
    int ret;
    u8 proto;
    ULONG src_ip_addr;
    s32 interface_type;
    NET_UP_PKT_DIRECT up_direct;
    NX_INTERFACE *ori_interface;
    u16 src_port, frag_flag, frag_offset;
    struct aic_net_fragment_forward *frag_cfg;
    struct aic_net_direct_forward_dev *forward_dev;

    memcpy(&src_ip_addr, &packet_ptr->nx_packet_prepend_ptr[12], 4);
    NX_CHANGE_ULONG_ENDIAN(src_ip_addr);

    ret = aic_net_get_trans_params(packet_ptr, &proto, NULL, &src_port, NULL);
    aicx_debug(0x0de454, "sxxxxxx", "[%s_UL] proto 0x%x src_port 0x%x ret 0x%x packet buf 0x%x val 0x%x packet 0x%x\r\n",
               packet_ptr->nx_packet_ip_interface->nx_interface_name,
               proto, src_port, ret, packet_ptr->nx_packet_data_start,
               *(u32 *)packet_ptr->nx_packet_prepend_ptr, packet_ptr);
    if (!ret) {

        ori_interface = packet_ptr->nx_packet_ip_interface;

        forward_dev = &aic_drv_forward_dev[ori_interface->nx_interface_index];

        interface_type =
            aic_network_find_interface_type(ori_interface);

        if ((interface_type < AIC_PPP_INTERFACE) ||
            (interface_type > AIC_USB_SEMI_TETHER_INTERFACE)) {
            aicx_warn(0x16ed3f, "sd", "[%s_UL] can't support this interface type %d\r\n",
                      ori_interface->nx_interface_name, interface_type);
            return DIRECT_FORWARD_BYPASS;
        }

        if (interface_type == AIC_USB_SEMI_TETHER_INTERFACE)
            return DIRECT_FORWARD_BYPASS;

        frag_cfg = aic_net_process_frag_packet(forward_dev, packet_ptr,
                                               &frag_flag, &frag_offset);

        if (frag_cfg) {
            if (frag_offset && frag_cfg->up_frag_in_progress) {
                ret = 0;
                memcpy(&up_direct, &frag_cfg->up_direct, sizeof(up_direct));
                goto UL_DIRECT_SEND;
            } else if (((frag_flag & BIT(1)) || (!frag_flag && !frag_offset)) &&
                       frag_cfg->up_frag_in_progress) {
                memset(frag_cfg, 0, sizeof(struct aic_net_fragment_forward));
            } else if (frag_offset && !frag_cfg->up_frag_in_progress) {
                aicx_info(0x067b5a, "s", "[%s_UL] there may be packet loss\n",
                          packet_ptr->nx_packet_ip_interface->nx_interface_name);
                nx_packet_release(packet_ptr);
                return DIRECT_FORWARD_CONSUMER;
            }
        } else if (!frag_cfg && frag_offset) {
            aicx_info(0x1ac512, "s", "[%s_UL] no frag cfg, there may be packet loss\n",
                      packet_ptr->nx_packet_ip_interface->nx_interface_name);
            nx_packet_release(packet_ptr);
            return DIRECT_FORWARD_CONSUMER;
        }

        if ((proto == NX_PROTOCOL_TCP) || (proto == NX_PROTOCOL_UDP)) {
            if (interface_type == AIC_PPP_INTERFACE) {
                ret = aic_ppp_dial_up_direct_with_port(src_port, &up_direct);
            } else if (interface_type == AIC_USB_RNDIS_INTERFACE ||
                       interface_type == AIC_USB_ECM_INTERFACE ||
                       interface_type == AIC_USB_NCM_INTERFACE) {
                ret = aic_usb_net_up_direct_with_port(src_port, src_ip_addr,
                                                      netid, &up_direct);
            }

            aicx_debug(0x13e5d1, "sxxxxxxxd", "[%s_UL] ret 0x%x ip_ptr 0x%x intf_index 0x%x port_converted 0x%x to_nat 0x%x packet 0x%x frag_flag 0x%x frag_offset %d\r\n",
                       packet_ptr->nx_packet_ip_interface->nx_interface_name,
                       ret, up_direct.ip_ptr, up_direct.intf_index,
                       up_direct.port_converted, up_direct.to_nat,
                       packet_ptr, frag_flag, frag_offset);

            if (!ret) {
                if ((up_direct.port_converted != src_port) &&
                     up_direct.port_converted) {
                    aic_net_update_transport_port(packet_ptr, TRUE,
                                                  up_direct.port_converted);
                }
            } else {
                if (frag_cfg)
                    memset(frag_cfg, 0, sizeof(struct aic_net_fragment_forward));
                nx_packet_release(packet_ptr);
                return DIRECT_FORWARD_CONSUMER;
            }
        } else {
            if (interface_type == AIC_PPP_INTERFACE) {
                ret = aic_ppp_dial_up_direct_no_port(&up_direct);
            } else if (interface_type == AIC_USB_RNDIS_INTERFACE ||
                       interface_type == AIC_USB_ECM_INTERFACE ||
                       interface_type == AIC_USB_NCM_INTERFACE) {
                ret = aic_usb_net_up_direct_no_port(src_ip_addr, netid,
                                                 &up_direct);
            }

            aicx_debug(0x0ef4db, "sxxxxxxd", "[%s_UL] no port: ret 0x%x to_nat 0x%x 0x%x 0x%x packet 0x%x frag_flag 0x%x frag_offset %d\r\n",
                       packet_ptr->nx_packet_ip_interface->nx_interface_name,
                       ret, up_direct.to_nat, up_direct.ip_ptr,
                       up_direct.port_converted, packet_ptr, frag_flag,
                       frag_offset);

            if (ret) {
                if (frag_cfg)
                    memset(frag_cfg, 0, sizeof(struct aic_net_fragment_forward));
                nx_packet_release(packet_ptr);
                return DIRECT_FORWARD_CONSUMER;
            }
        }

UL_DIRECT_SEND:
        if ((frag_flag & BIT(0)) && !frag_offset) {
            if (frag_cfg) {
                frag_cfg->up_frag_in_progress = TRUE;
                memcpy(&frag_cfg->up_direct, &up_direct, sizeof(up_direct));
            }
        }

        if (frag_cfg && frag_offset && !(frag_flag & BIT(0)))
            memset(frag_cfg, 0, sizeof(struct aic_net_fragment_forward));

        if (!ret && (up_direct.to_nat == ANET_PKT_NO_NAT)) {
            aic_net_up_link_direct_trans_packet(&up_direct, packet_ptr);
            return DIRECT_FORWARD_CONSUMER;
        } else if (!ret && (up_direct.to_nat == ANET_PKT_MULTI_HOME)) {
            NX_PACKET *packet_copy;

            ret = nx_packet_copy(packet_ptr, &packet_copy,
                                 ip_ptr->nx_ip_default_packet_pool, NX_NO_WAIT);
            if (ret == NX_SUCCESS) {
                aic_net_up_link_direct_trans_packet(&up_direct, packet_ptr);
            }

            _nx_ip_packet_deferred_receive(ip_ptr, packet_copy);
            return DIRECT_FORWARD_CONSUMER;
        }
    } else {
        aicx_error(0x0168e4, "sx", "[%s_UL] get trans params fail 0x%x\n",
                   packet_ptr->nx_packet_ip_interface->nx_interface_name, ret);
        nx_packet_release(packet_ptr);
        return DIRECT_FORWARD_CONSUMER;
    }

    return DIRECT_FORWARD_BYPASS;
}

#ifdef CONFIG_PLAT_CONSOLE
static int aic_direct_forward_test_cmd(int argc, char **argv)
{
    int i, j;

    for (i = 0; i < NX_MAX_IP_INTERFACES; i++) {
        CONS_OUT("interface index %d:\r\n", i);
        for (j = 0; j < AIC_NET_MAX_FRAGS; j++)
            CONS_OUT("\tin_use: %d init_timestamp: %d now time: %d\r\n",
                     aic_drv_forward_dev[i].frag_arrays[j].in_use,
                     aic_drv_forward_dev[i].frag_arrays[j].init_timestamp,
                     aic_get_timestamp());
    }

    return 0;
}
CONSOLE_EXPORT_CMD(direct_forward, aic_direct_forward_test_cmd, direct forward);
#endif

