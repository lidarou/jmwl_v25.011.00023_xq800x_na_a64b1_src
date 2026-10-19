/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the macros,structs and apis for managing
 *    PPP dial,which is one kind of network share.
 */

#ifndef AIC_PPP_DIAL_MGR_H
#define AIC_PPP_DIAL_MGR_H

#ifdef CONFIG_NX_DHCP
#include <nxd_dhcpv6_server.h>
#endif
#include <aic_net_pdp_mgr.h>
#ifdef CONFIG_NX_NAT
#include <nx_nat.h>
#endif

/* Define macros. */
#define ANET_PPP_DIAL_NET_NUM 1

#define ANET_PPP_LAN_NET_ID 200

#define ANET_PPP_DIAL_STARTED 1
#define ANET_PPP_DIAL_STOPPED 0

/* For tethering mode. */
#define ANET_PPP_LAN_IPV4_ADDR IP_ADDRESS(192, 168, 2, 1)
#define ANET_PPP_PEER_IPV4_ADDR IP_ADDRESS(192, 168, 2, 10)

#define ANET_PPP_LAN_MTU_SIZE 1500

#define ANET_PPP_CONFIG_FRAME_SIZE 200

#define ANET_PPP_LAN_IPV6_GAP 5

/* Define structs. */
typedef struct PPP_DIAL_LAN_NET_STRUCT
{
    u16 lan_net_id;                /* Net id of LAN net */
    ulong lan_ipv4_address;        /* IPv4 address of LAN net */
    ulong lan_ipv4_subnet_mask;    /* IPv4 subnet mask of LAN net */
    ulong lan_ipv4_dns1_address;   /* IPv4 DNS server address of LAN net */
    ulong lan_ipv4_dns2_address;   /* IPv4 DNS server address of LAN net */
    ulong peer_ipv4_addr;          /* Ipv4 addresses assigned to peer */

    ulong local_ipv6_addr[4];      /* link local IPv6 address of LAN net */
    ulong lan_global_ipv6_addr[4]; /* global IPv6 address of LAN net */
    ulong lan_ipv6_dns1_address[4];/* IPv6 DNS server address of LAN net */
    ulong lan_ipv6_dns2_address[4];/* IPv6 DNS server address of LAN net */
    ulong peer_ipv6_addr[4];       /* IPv6 address assign to peer */
    u32 global_ipv6_addr_index;    /* Index of global ipv6 addr */
    u32 local_ipv6_addr_index;     /* Index of link local ipv6 addr */
} PPP_DIAL_LAN_NET;

typedef struct PPP_DIAL_NET_INFO_STRUCT
{
    u8 lan_net_id;           /* Net id of LAN net */
    u8 wan_net_id;           /* Net id of WAN net */
    u8 ppp_dial_mode;        /* Forwarding mode or routing mode */
    u8 dial_started;         /* If has received ATD*99 cmd */
    u8 dial_com_mode;        /* The com is CMD or DATA mode. */
    u8 waiting_pdp_act;      /* If waiting for PDP to activate. */
    u8 nat_entry_created;    /* If NAT entry has created */
    u8 dhcpv6_started;
    u32 lan_intf_index;      /* The interface index */
    u32 wan_interf_index;    /* The interface index of PDP */
    u32 port_range_start;    /* The beginning of port range */
    u32 port_range_end;      /* The end of port range */
    u32 serial_id;
    u8 interf_name[8];
    u8 pdp_type[ANET_PDP_TYPE_MAX_LEN];
    u8 apn[ANET_APN_NAME_MAX_LEN];

    PPP_DIAL_LAN_NET *lan_net_ptr;
    WAN_NET *wan_net_ptr;
    NX_IP *lan_net_ip;

    NX_NAT_DEVICE *nat_dev_ptr;
    NX_NAT_TRANSLATION_ENTRY ppp_icmp_nat_entry;
    NX_NAT_TRANSLATION_ENTRY ppp_tcp_nat_entry;
    NX_NAT_TRANSLATION_ENTRY ppp_udp_nat_entry;
} PPP_DIAL_NET_INFO;

/* Define public functions. */
void aic_ppp_dial_init(void);
u32 aic_ppp_dial_pdp_activated(u8 pdp_cid);
u32 aic_ppp_dial_pdp_deactivated(u16 pdp_cid);
//u16 aic_get_ppp_dial_net_by_cid(u16 pdp_cid);
u32 aic_ppp_dial_start(u32 serial_channel_id, u8 cid);
u32 aic_ppp_dial_stop(u32 serial_channel_id);
void aic_ppp_dial_stop_half_way(u8 pdp_cid);
void aic_reset_ppp_dial_net_info(void);
u8 aic_get_ppp_dial_cid(void);
boolean aic_is_ppp_dial_data_mode(u32 com_id);
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
void aic_ppp_ipv6_dad_success(NX_IP *ip_ptr, u32 interf_index, u32 addr_index);
#endif

#endif  /* AIC_PPP_DIAL_MGR_H */
