/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the macros,structs and apis used by all
 *    network share.
 */

#ifndef AIC_NET_SHARE_COMMON_H
#define AIC_NET_SHARE_COMMON_H

#include "nx_api.h"
#ifdef CONFIG_NX_NAT
#include <nx_nat.h>
#endif

#define DHCP_SERVER_STACK_SIZE SIZE_2K

#define ANET_DHCP_CONFIGURED 1
#define ANET_DHCP_NOT_CONFIGURED 0

#define ANET_DHCPV6_STARTED 1
#define ANET_DHCPV6_NOT_STARTED 0

#define DHCPV6_SERVER_STACK_SIZE SIZE_2K

#define ANET_NOT_WAIT_PDP_ACTIVATE 0
#define ANET_WAIT_PDP_ACTIVATE 1

#define ANET_SHARE_MODE_FORWARDING 1
#define ANET_SHARE_MODE_ROUTING 2

#define ANET_NAT_ENTRY_CREATED 1
#define ANET_NAT_ENTRY_NOT_CREATED 0

#define ANET_LAN_NET_MASK_ROUTING 0xffffff00UL
#define ANET_LAN_NET_MASK_FORWARDING 0xffffffffUL

#define ANET_NETSHARE_TYPE_PPP 1
#define ANET_NETSHARE_TYPE_USB 2

#define NET_EVENT_PORT_TIMEOUT 0x01
#define NET_EVENT_PPP_DATA_MODE 0x02
#define NET_EVENT_STOP_PPP_DIAL 0x04
#define NET_EVENT_RESTART_PPP 0x08
#define NET_EVENT_STOP_USB_TETHER 0x10
#define NET_EVENT_AUTO_USB_TETHER 0x20
#define NET_EVENT_USB_TETHER_RETRY 0x40
#define NET_EVENT_USB_TETHER_OTHER_MODE 0x80

#define ANET_USB_INTF_NAME_RNDIS "rndis0"
#define ANET_USB_INTF_NAME_ECM "ecm0"

#define ANET_ERROR_NET_SHARE_BASE 1100
/* Errors replied to the upper device. */
#define ANET_ERROR_INTF_ENGAGED ANET_ERROR_NET_SHARE_BASE
#define ANET_ERROR_PDP_ENGAGED ANET_ERROR_NET_SHARE_BASE + 1
#define ANET_ERROR_USB_INTF_DOWN ANET_ERROR_NET_SHARE_BASE + 2
#define ANET_ERROR_PDP_ACTIVATE_FAIL ANET_ERROR_NET_SHARE_BASE + 3
#define ANET_ERROR_INTERNAL_ERROR ANET_ERROR_NET_SHARE_BASE + 4
/* Errors for usb tether and ppp dial. */
#define ANET_ERROR_USB_NET_OVERFLOW ANET_ERROR_NET_SHARE_BASE + 5
#define ANET_ERROR_INVALID_WAN ANET_ERROR_NET_SHARE_BASE + 6
#define ANET_ERROR_INVALID_USB_NET_INDEX ANET_ERROR_NET_SHARE_BASE + 7
#define ANET_ERROR_NULL_USB_NET ANET_ERROR_NET_SHARE_BASE + 8
#define ANET_ERROR_PPP_NET_NOT_READY ANET_ERROR_NET_SHARE_BASE + 9
/* Errors for usb semi tether. */
#define ANET_ERROR_USB_SEMI_TETHER_OVERFLOW ANET_ERROR_NET_SHARE_BASE + 10
#define ANET_ERROR_USB_SEMI_TETHER_MODE ANET_ERROR_NET_SHARE_BASE + 11
#define ANET_ERROR_INVALID_USB_INTF ANET_ERROR_NET_SHARE_BASE + 12
#define ANET_ERROR_LAN_NET_NOT_INIT ANET_ERROR_NET_SHARE_BASE + 13
/* Errors for port managing. */
#define ANET_ERROR_HASH_NOT_INIT ANET_ERROR_NET_SHARE_BASE + 14
#define ANET_ERROR_PKT_NO_DIRECT ANET_ERROR_NET_SHARE_BASE + 15
#define ANET_ERROR_NO_USB_NET ANET_ERROR_NET_SHARE_BASE + 16
#define ANET_ERROR_SRC_PORT_ENGAGED ANET_ERROR_NET_SHARE_BASE + 17
#define ANET_ERROR_CLIENTS_OVERFLOW ANET_ERROR_NET_SHARE_BASE + 18
#define ANET_ERROR_NO_IDLE_PORT ANET_ERROR_NET_SHARE_BASE + 19
#define ANET_ERROR_NOT_MATCH_PPP_DIAL ANET_ERROR_NET_SHARE_BASE + 20
#define ANET_ERROR_NOT_MATCH_USB_NET ANET_ERROR_NET_SHARE_BASE + 21
#define ANET_ERROR_USB_PLUG_OUT ANET_ERROR_NET_SHARE_BASE + 22
#define ANET_ERROR_CLIENT_INFO_FULL ANET_ERROR_NET_SHARE_BASE + 23

#ifdef CONFIG_CM_USB_DIAL_AT
#define ANET_CM_ERROR_INVALID_PARA 4
#define ANET_CM_ERROR_NO_NET 100
#endif

#ifdef CONFIG_NX_DHCP
u32 aic_init_dhcp_server(void);
#ifdef ENABLE_DHCPV6_SERVER
u32 aic_init_dhchv6_server(void);
#endif
#endif

s32 aic_network_find_interface_type(NX_INTERFACE *interface_ptr);
NX_INTERFACE *aic_network_match_interface(NX_INTERFACE *interface_ptr,
                                          u8 *pdp_active_mode);
u32 aic_network_get_pdp_peer_interface(NX_INTERFACE *interface_ptr);
u32 aic_get_wan_intf_index(NX_INTERFACE *nx_interface_ptr);
u8 aic_get_net_share_mode(u32 intf_index);
void aic_act_net_share_waiting_pdp(void);
u32 aic_get_lan_ifc_index(u8 *ifc_name);
boolean aic_dhcp_server_still_used(void);
boolean aic_ppp_dial_use_same_nat(u32 wan_intf_index);
boolean aic_softap_use_same_nat(u32 wan_intf_index);
boolean aic_usb_net_use_same_nat(u32 wan_intf_index);
boolean aic_is_valid_intf_index(u8 intf_index);
#ifdef CONFIG_SOFTAP_SHARE
u32 aic_config_softap_intf(u8 *intf_name, VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *));
u32 aic_delete_softap_intf(void);
void aic_notify_softap_pdp_activated(void);
void aic_handle_client_disassoc(ULONG ip_addr);
#endif
#ifdef CONFIG_USB_NET_SHARE
boolean aic_is_auto_start_usb_dial(void);
#endif
void aic_net_set_usb_other_mode_event(void);

#endif /* AIC_NET_SHARE_COMMON_H */
