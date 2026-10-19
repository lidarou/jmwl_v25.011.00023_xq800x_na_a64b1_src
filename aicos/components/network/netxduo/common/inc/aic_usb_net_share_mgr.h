/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the macros,structs and apis for managing USB
 *    network share.
 */

#ifndef AIC_USB_NETWORK_SHARE_H
#define AIC_USB_NETWORK_SHARE_H

#include <aic_size.h>
#ifdef CONFIG_NX_DHCP
#include <nxd_dhcpv6_server.h>
#endif
#include <aic_net_pdp_mgr.h>
#ifdef CONFIG_NX_NAT
#include <nx_nat.h>
#endif

#define ANET_USB_NET_MAX_NUM 1

#define ANET_USB_LAN_NET_ID_BASE 20

#define ANET_START_USB_NET_TETHER 1
#define ANET_STOP_USB_NET_TETHER 0

#define ANET_USB_PDP_DEACT_INVOKED 1
#define ANET_USB_PDP_DEACT_NOT_INVOKED 0

#define ANET_NOT_SENT_PDP_DEACT 0
#define ANET_SENT_PDP_DEACT 1

#define ANET_MAX_COUNT_WAIT_PDP 120

#define ANET_USB_LAN_IPV4_ADDR IP_ADDRESS(192, 168, 0, 1)
#define ANET_LAN_IPV4_ADDR_LIST_SIZE 8

#define ANET_LAN_IPV6_BASE_GAP 15
#define ANET_LAN_IPV6_ADDR_LIST_SIZE 5

#define ANET_USB_LAN_MTU_SIZE 1500

#define ANET_SHARE_AUTO_RECON_ENABLED 1
#define ANET_SHARE_AUTO_RECON_DISABLED 0

#define ANET_USB_NET_WAIT_PDP_TIME 12000

/* Wait no more than 2800ms for checking intf is actually rndis. */
#define ANET_WAIT_RNDIS_READY_TIME 2800

#define ANET_DIAL_INTF_RNDIS_MODE 0
#define ANET_DIAL_INTF_ECM_MODE 1

/* Define structs. */
typedef struct USB_TETHER_LAN_NET_STRUCT
{
    u16 lan_net_id;                /* Net id of LAN net */
    u16 ipv4_addr_pool_size;       /* Number of ipv4 addresses assign to peer*/
    ulong lan_ipv4_address;        /* IPv4 address of LAN net */
    ulong lan_ipv4_subnet_mask;    /* IPv4 subnet mask of LAN net */
    ulong lan_ipv4_dns1_address;   /* IPv4 DNS server address of LAN net */
    ulong lan_ipv4_dns2_address;   /* IPv4 DNS server address of LAN net */
    ulong ipv4_addr_pool_start;    /* Start of ipv4 addresses assign to peer*/
    ulong ipv4_addr_pool_end;      /* End of ipv4 addresses assign to peer*/

    u16 ipv6_addr_pool_size;       /* Number of ipv6 addresses assign to peer*/
    ulong local_ipv6_addr[4];      /* local IPv6 address of LAN net */
    ulong lan_global_ipv6_addr[4];     /* global IPv6 address of LAN net */
    ulong lan_ipv6_dns1_address[4];/* IPv6 DNS server address of LAN net */
    ulong lan_ipv6_dns2_address[4];/* IPv6 DNS server address of LAN net */
    ulong ipv6_addr_pool_start[4]; /* Start of ipv6 addresses assign to peer */
    ulong ipv6_addr_pool_end[4];   /* End of ipv6 addresses assign to peer */
    u32 global_ipv6_addr_index;
    u32 local_ipv6_addr_index;
} USB_TETHER_LAN_NET;

typedef struct USB_TETHER_NET_INFO_STRUCT
{
    u16 lan_net_id;          /* Net id of LAN net */
    u8 wan_net_id;           /* Net id of WAN net */
    u8 usb_tether_mode;      /* Forward mode or Routing mode */
    u8 pdp_deact_invoked;    /* PDP is deacted not due to stopping usb share */
    u8 sent_pdp_deact;       /* AT+CGACT=0,cid has been sent */
    u8 usb_interf_id;        /* USB interface id,the range is 0~9 */
    u8 usb_interf_state;     /* The state of USB interface is up or down */
    u32 lan_intf_index;      /* The interface index attached to IP instance */
    u32 wan_intf_index;      /* The interface index of the PDP interface  */
    u32 ucom_channel_id;     /* Id of ucom channel. */
    /* Name of usb interface,such as rndis0,ecm0. */
    u8 usb_interfce[ANET_MAX_LEN_INTERF_NAME];
    u8 pdp_type[ANET_PDP_TYPE_MAX_LEN];  /* Pdp type,such as IPV46. */
    u8 apn[ANET_APN_NAME_MAX_LEN];  /* Apn name */
    u32 port_range_start;    /* The beginning of port range. */
    u32 port_range_end;      /* The end of port range. */
#ifdef CONFIG_NX_DHCP
    u8 dhcp_configured;
    u8 dhcpv6_started;
    ulong dhcpv6_duid;
#endif
    u8 waiting_pdp_act;      /* If waiting for PDP to activate. */
    u8 nat_entry_created;    /* If the NAT entry has created. */
    u8 auto_started;         /* If the usb tether is auto started. */
    u8 rndis_ready;
    u32 forward_mode_start_time;

    USB_TETHER_LAN_NET *lan_net_ptr;
    WAN_NET *wan_net_ptr;

    NX_IP *lan_net_ip;
    //NX_NAT_DEVICE *usb_nat_dev_ptr;
    NX_NAT_TRANSLATION_ENTRY icmp_nat_entry;
    NX_NAT_TRANSLATION_ENTRY tcp_nat_entry;
    NX_NAT_TRANSLATION_ENTRY udp_nat_entry;
} USB_TETHER_NET_INFO;

/* Define global functions. */
u32 aic_usbtether_init(void);
u32 aic_usbtether_pdp_deactivated(u16 pdp_cid);
u8 aic_get_active_usb_net_by_cid(u8 pdp_cid);
USB_TETHER_NET_INFO *aic_get_usb_net_per_cid_com(u8 pdp_cid, u32 serial_port,
                                                 u8 *usb_net_index);
s32 aic_notify_usbtether_pdp_activated(u8 cid);
s32 aic_usbtether_start(u32 ucom_channel_id, u8 *usb_intf, u8 share_mode,
                        u8 cid, u8 *pdp_type, u8 *apn);
u32 aic_usbtether_stop(USB_TETHER_NET_INFO *usb_net_ptr, boolean down_usb_intf,
                       boolean to_deactivate_pdp);
u8 aic_get_usb_net_by_intf_id(u8 usb_intf_id);
u32 aic_usbtether_stop_half_way(u8 pdp_cid, u32 ucom_id);
USB_TETHER_NET_INFO *aic_get_usb_net_by_lan_ifc(u16 usb_ifc_index);
void aic_notify_usb_tether_ipv6_change(u8 pdp_cid);
USB_TETHER_NET_INFO *aic_get_usb_net_by_wan_ifc(u32 ifc_index);
//void aic_get_usb_intf_prefix(u8 *usb_intf);
USB_TETHER_NET_INFO *aic_get_usb_net_waiting_pdp(u8 pdp_cid);
s32 aic_auto_start_usb_tether(u8 tether_mode, u8 *intf_name);
void aic_usb_net_wait_pdp(u8 cid, USB_TETHER_NET_INFO *usb_net_ptr);
boolean aic_is_usb_plug_in(void);
USB_TETHER_NET_INFO *aic_get_usb_net_by_cid(u8 pdp_cid);
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
void aic_usb_ipv6_dad_success(NX_IP *ip_ptr, u32 interf_index, u32 address_index);
#endif
boolean aic_usb_tether_already_started(void);
void aic_stop_all_usb_net(boolean to_down_intf);
#ifdef CONFIG_NX_NAT
void aic_delete_usb_interf_nat(USB_TETHER_NET_INFO *usb_net);
#endif
boolean usbtether_all_stopped(void);
void usbtether_deact_invoked(USB_TETHER_NET_INFO *usb_net_ptr);
#ifdef ANET_ENABLE_USB_DONGLE
void aic_usb_intf_ready_notified(u32 is_ready);
void aic_set_dg_usb_intf_mode(u8 intf_mode);
u8 aic_get_dg_usb_intf_mode(void);
void aic_dg_auto_usb_dial(u8 tether_mode, u8 intf_mode);
#endif
void aic_usb_intf_update_xmit_queue_room_size(void);

#endif  /* AIC_USB_NETWORK_SHARE_H */
