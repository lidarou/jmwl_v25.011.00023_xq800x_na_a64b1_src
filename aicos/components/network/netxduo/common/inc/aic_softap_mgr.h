/**************************************************************************/
/*                                                                        */
/* Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the macros,structs and apis for managing softap.
 */

#ifndef AIC_SOFTAP_MGR_H
#define AIC_SOFTAP_MGR_H

#include <aic_size.h>
#include <aic_net_pdp_mgr.h>
#ifdef CONFIG_NX_NAT
#include <nx_nat.h>
#endif

#define ANET_SOFTAP_CLIENTS_NUM 10

#ifdef CONFIG_BK7238
#define ANET_SOFTAP_LAN_IPV4_ADDR IP_ADDRESS(192, 168, 0, 1)
#else
#define ANET_SOFTAP_LAN_IPV4_ADDR IP_ADDRESS(192, 168, 10, 1)
#endif
#define ANET_SOFTAP_IPV4_ADDR_LIST_SIZE ANET_SOFTAP_CLIENTS_NUM

#define ANET_SOFTAP_LAN_MTU_SIZE 1500

#define ANET_INVALID_INTF_INDEX 0xff

#define ANET_DEFAULT_DNS_SERVER1_V4 IP_ADDRESS(114, 114, 114, 114)
#define ANET_DEFAULT_DNS_SERVER2_V4 IP_ADDRESS(8, 8, 8, 8)

/* Define structs. */
typedef struct SOFTAP_CLIENT_INFO_STRUCT
{
    ulong ipv4_addr;
    u8 nat_entry_created;    /* If the NAT entry has created. */
    NX_NAT_TRANSLATION_ENTRY icmp_nat_entry;
    NX_NAT_TRANSLATION_ENTRY tcp_nat_entry;
    NX_NAT_TRANSLATION_ENTRY udp_nat_entry;
} SOFTAP_CLIENT_INFO;

typedef struct SOFTAP_LAN_NET_STRUCT
{
    u16 ipv4_addr_pool_size;       /* Number of ipv4 addresses assign to peer*/
    ulong lan_ipv4_address;        /* IPv4 address of LAN net */
    ulong lan_ipv4_address_user;   /* IPv4 address of LAN net that user cfg*/
    ulong lan_ipv4_subnet_mask;    /* IPv4 subnet mask of LAN net */
    ulong lan_ipv4_dns1_address;   /* IPv4 DNS server address of LAN net */
    ulong lan_ipv4_dns2_address;   /* IPv4 DNS server address of LAN net */
    ulong ipv4_addr_pool_start;    /* Start of ipv4 addresses assign to peer*/
    ulong ipv4_addr_pool_end;      /* End of ipv4 addresses assign to peer*/

    ulong local_ipv6_addr[4];      /* local IPv6 address of LAN net */
    ulong global_ipv6_addr[4];     /* global IPv6 address of LAN net */
    ulong ipv6_dns1_address[4];    /* IPv6 DNS server address of LAN net */
    ulong ipv6_dns2_address[4];    /* IPv6 DNS server address of LAN net */
    u32 global_ipv6_addr_index;
    u32 local_ipv6_addr_index;

    SOFTAP_CLIENT_INFO clients_info[ANET_SOFTAP_CLIENTS_NUM];
} SOFTAP_LAN_NET;

typedef struct SOFTAP_NET_INFO_STRUCT
{
    u32 lan_intf_index;      /* The interface index attached to IP instance */
    u32 wan_intf_index;      /* The interface index of the PDP interface  */
    u8 softap_interfce[ANET_MAX_LEN_INTERF_NAME];
    u8 wan_net_id;           /* Net id of WAN net */
    u8 lan_intf_state;
#ifdef CONFIG_NX_DHCP
    u8 dhcp_configured;
#endif

    SOFTAP_LAN_NET *lan_net_ptr;
    WAN_NET *wan_net_ptr;
    NX_IP *net_ip;
} SOFTAP_NET_INFO;

void aic_config_lan_ip(u32 ip_addr);

/* Define global functions. */
SOFTAP_NET_INFO *aic_get_softap_info_by_lan_ifc(u8 softap_intf_index);
void aic_softap_init(void);

#endif  /* AIC_USB_NETWORK_SHARE_H */
