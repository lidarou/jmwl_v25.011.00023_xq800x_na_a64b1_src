/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the macros,structs and apis for managing USB
 *    semi network share.
 */

#ifndef AIC_NET_USB_SEMI_TETHER_H
#define AIC_NET_USB_SEMI_TETHER_H

#include <aic_net_pdp_mgr.h>
#include <aic_usb_net_share_mgr.h>

#define ANET_USB_SEMI_TETHER_CNT 3

#define ANET_USB_SEMI_TETHER_START 1
#define ANET_USB_SEMI_TETHER_STOP 0

#define ANET_USB_SEMI_LAN_NET_ID_BASE 30

#define ANET_USB_SEMI_LAN_IPV4_ADDR IP_ADDRESS(192, 168, 10, 1)

#define ANET_SEMI_TETHER_LOCAL_IPV6 "fe80::8:1"
#define ANET_SEMI_TETHER_GLOBAL_IPV6 "2409:2001::8:1"

/* Define structs. */
typedef struct USB_SEMI_TETHER_INFO_STRUCT
{
    u8 tether_enabled;
    u8 tether_mode;
    u16 lan_net_id;          /* Net id of LAN net */

    u32 lan_intf_index;      /* The interface index attached to IP instance */
#ifdef CONFIG_NX_DHCP
    u8 dhcp_configured;
    u8 dhcpv6_started;
#endif
    u8 net_interf_id;       /* USB interface id,the range is 0~9 */
    u8 net_interf_name[ANET_MAX_LEN_INTERF_NAME];

    USB_TETHER_LAN_NET *lan_net_ptr;
    NX_IP *lan_net_ip;
} USB_SEMI_TETHER_INFO;

/* Define global functions. */
struct aic_network_ip_instance *aic_get_semi_tether_ip_instance(void);

#endif  /* AIC_NET_USB_SEMI_TETHER_H */
