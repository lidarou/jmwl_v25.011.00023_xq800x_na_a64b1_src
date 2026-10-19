/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the macros,structs and apis for management of
 *    transport layer ports.
 */

#ifndef AIC_NET_SHARE_PORT_MGR_H
#define AIC_NET_SHARE_PORT_MGR_H

#include <aic_osa.h>
#include "hashtable.h"

#define ANET_RESERVED_PORT_START 0
#define ANET_RESERVED_PORT_END 1024

#define ANET_PPP_DIAL_PORT_START 10000
#define ANET_PPP_DIAL_PORT_END 19999

/** NX_NAT_START_TCP_PORT~NX_NAT_END_TCP_PORT(20000~30000) is used
 * for tethering net share,including ppp dial and usb net share.
 */

#define ANET_USBSHARE_PORT_START 30001
#define ANET_USBSHARE_PORT_END 49151

#define ANET_INTERNAL_PORT_START 49152 //0xc000
#define ANET_INTERNAL_PORT_END 65535

#define ANET_PKT_NO_NAT 1
#define ANET_PKT_NEED_NAT 2
#define ANET_PKT_MULTI_HOME 3

#define ANET_USB_TETHER_MAX_CLIENTS 2

/* 10000 ms, 10s */
#define ANET_PORT_TIMER_EXPIRE_TIME 10000

/* 90 seconds */
#define ANET_PORT_AGING_TIME 90000

#define ANET_PPP_DIAL_FORWARDING 1
#define ANET_PPP_DIAL_ROUTING 2
#define ANET_USBSHARE_FORWARDING 4
#define ANET_USBSHARE_ROUTING 8

#define ANET_DHCPV6_CLIENT_UDP_PORT 546
#define ANET_DHCP_CLIENT_UDP_PORT 68

/* Define structs. */
typedef struct
{
    collection_item_t element_clue;
    u16 previous_port;
    u16 converted_port;
    ulong latest_sys_time;
} PORT_CONVERT;

typedef struct
{
    NX_IP *ip_ptr;
    u32 intf_index;
    u16 port_converted;  /* The port num convert to. */
    u8 to_nat;           /* If need to NAT. */
} NET_UP_PKT_DIRECT;

typedef struct NET_DOWN_PKT_DIRECT_STRUCT
{
    NX_IP *ip_ptr;
    u32 intf_index;
    u8 to_nat;           /* If need to NAT or multi home. */
    //u16 port_converted;  /* The port num convert to. */
    //ulong ip_addr_converted;
} NET_DOWN_PKT_DIRECT;

/* Define global functions. */
u32 aic_init_net_port_hashtable(u8 netshare_type, u8 share_mode);
void aic_delete_port_hashtable(u8 netshare_type, u8 share_mode);
u32 aic_ppp_dial_up_direct_no_port(NET_UP_PKT_DIRECT *ppp_up_direct);
u32 aic_ppp_dial_up_direct_with_port(u16 original_src_port,
                                     NET_UP_PKT_DIRECT *ppp_up_direct);
u32 aic_usb_net_up_direct_no_port(ulong src_ipv4, u8 usb_intf_id,
                                  NET_UP_PKT_DIRECT *up_direct);
u32 aic_usb_net_up_direct_with_port(u16 original_src_port, ulong src_ipv4,
                                    u8 usb_intf_id, NET_UP_PKT_DIRECT *up_direct);
u32 aic_down_pkt_direct_no_port(u8 net_id, NET_DOWN_PKT_DIRECT *down_pkt_direct);
u32 aic_down_pkt_direct_with_port(u16 original_dst_port, u8 net_id,
                                  NET_DOWN_PKT_DIRECT *down_pkt_direct);
u32 aic_get_usb_net_dst_port(u16 original_dst_port, u8 usb_intf_id,
                             u16 *actual_port);
u32 aic_get_ppp_dial_dst_port(u16 original_dst_port, u16 *actual_port);
u32 aic_get_wan_intf_index(NX_INTERFACE *nx_interface_ptr);
u32 aic_get_forwarding_matched_interf(u32 interf_index);
void aic_remove_aging_ports(hashtable_t *hashtable_ptr);

#endif /* AIC_NET_SHARE_PORT_MGR_H */
