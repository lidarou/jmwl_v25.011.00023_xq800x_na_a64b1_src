/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the structs and apis for managing PDP info and
 *    WAN net.
 */

#ifndef AIC_NET_PDP_MGR_H
#define AIC_NET_PDP_MGR_H

#ifndef WIN32
#include <aic_at_process_common.h>
#endif
#include <aic_additional_apns.h>
#include <nx_api.h>
#ifdef CONFIG_NX_NAT
#include <nx_nat.h>
#endif

#define ANET_PDP_ACTIVATED 1
#define ANET_PDP_DEACTIVATED 0

#define ANET_PDP_IFC_NAME "anet_lte"

#define ANET_PDP_MTU_SIZE 1500

#define ANET_IFC_INDEX_DEFAULT 0

#define ANET_IPV4_BROADCAST_ADDR 0xffffffffUL
#define ANET_IPV4_DEFAULT_NET_MASK 0xffffff00UL

#define ANET_PDP_TO_ACTIVATE 1
#define ANET_PDP_TO_DEACTIVATE 0

#define ANET_DEFAULT_APN_TYPE "default"

#define ANET_PDP_TYPE_IPV4 "IP"
#define ANET_PDP_TYPE_IPV6 "IPV6"
#define ANET_PDP_TYPE_IPV4V6 "IPV4V6"

#define ANET_MAX_LEN_INTERF_NAME 20

#define ANET_MAX_PDP_STATE_CBS 5

#define ANET_DEFAULT_IPV4_DNS_SERVER1 "114.114.114.114"
#define ANET_DEFAULT_IPV4_DNS_SERVER2 "8.8.8.8"

#define ANET_DEFAULT_IPV6_DNS_SERVER1 "2400:3200::1"
#define ANET_DEFAULT_IPV6_DNS_SERVER2 "2400:da00::6666"

#define ANET_DEFAULT_IP_TYPE_NONE 0
#define ANET_DEFAULT_IP_TYPE_V4 1
#define ANET_DEFAULT_IP_TYPE_V6 2
#define ANET_DEFAULT_IP_TYPE_V4V6 3

#define ANET_NAT64_TABLE_NUM 500

#ifdef WIN32
#define ANET_APN_NAME_MAX_LEN 60

#define ANET_MAX_LEN_IPV4_ADDR 20
#define ANET_MAX_LEN_DEC_IPV6_ADDR 70
#define ANET_MAX_LEN_HEX_IPV6_ADDR 48
#endif

#define ANET_IPV6_FORMAT_DECIMAL 0
#define ANET_IPV6_FORMAT_HEX 1

#define ANET_APN_CNT_PER_IMSI 20
#define ANET_APN_CNT_USER_SET 20
#define ANET_FULL_IMSI_LEN 16
#define ANET_FULL_ICCID_LEN 24

#define AT_EVENT_TRY_ALL_APN 0x01

#define ANET_ERROR_PDP_MGR_BASE 100
#define ANET_ERROR_INVALID_PDP ANET_ERROR_PDP_MGR_BASE
#define ANET_ERROR_PDP_OVERFLOW ANET_ERROR_PDP_MGR_BASE + 1
#define ANET_ERROR_PDP_NOT_FOUND ANET_ERROR_PDP_MGR_BASE + 2
#define ANET_ERROR_INVALID_WAN_INDEX ANET_ERROR_PDP_MGR_BASE + 3
#define ANET_ERROR_WAN_NOT_FOUND ANET_ERROR_PDP_MGR_BASE + 4
#define ANET_ERROR_INVALID_IFC_INDEX ANET_ERROR_PDP_MGR_BASE + 5
#define ANET_ERROR_PDP_NOT_ACTIVATED ANET_ERROR_PDP_MGR_BASE + 6
#define ANET_ERROR_PDP_CBS_OVERFLOW ANET_ERROR_PDP_MGR_BASE + 7
#define ANET_ERROR_NO_IPV6_DNS_SERVER ANET_ERROR_PDP_MGR_BASE + 8
#define ANET_ERROR_NO_MISC_DATA ANET_ERROR_PDP_MGR_BASE + 9
#define ANET_ERROR_APN_NOT_FOUND ANET_ERROR_PDP_MGR_BASE + 10
#define ANET_ERROR_NO_IP_INSTANCE ANET_ERROR_PDP_MGR_BASE + 11
#define ANET_ERROR_INVALID_IP_TYPE ANET_ERROR_PDP_MGR_BASE + 12
#define ANET_ERROR_SET_APN_FAIL ANET_ERROR_PDP_MGR_BASE + 13
#define ANET_ERROR_GET_SIM_INFO_FAIL ANET_ERROR_PDP_MGR_BASE + 14

typedef struct NET_PDP_INFO_STRUCT
{
    u8 pdp_cid;                   /* PDP cid*/
    u8 pdp_activated;             /* PDP is activated or not */
    u32 pdp_ifc_index;
    u8 pdp_ipv4_address[ANET_MAX_LEN_IPV4_ADDR];       /* IPv4 address */
    u8 pdp_ipv4_dns1_address[ANET_MAX_LEN_IPV4_ADDR];  /* IPv4 DNS address */
    u8 pdp_ipv4_dns2_address[ANET_MAX_LEN_IPV4_ADDR];  /* IPv4 DNS address */

    u8 pdp_ipv6_suffix_hex[ANET_MAX_LEN_HEX_IPV6_ADDR];
    u8 pdp_ipv6_dns1_address[ANET_MAX_LEN_HEX_IPV6_ADDR]; /* IPv6 DNS server */
    u8 pdp_ipv6_dns2_address[ANET_MAX_LEN_HEX_IPV6_ADDR]; /* IPv6 DNS server */
} NET_PDP_INFO;

typedef struct WAN_NET_STRUCT
{
    u8 wan_net_id;                  /* Net id of WAN net */
    u8 pdp_cid;                     /* PDP cid*/
    u8 pdp_activated;               /* PDP is activated or not */
    u8 nat64_ipv4_index;
    u32 wan_ifc_index;              /* Interface index.*/
    u8 wan_ifc_name[ANET_MAX_LEN_INTERF_NAME];
    u8 apn_name[ANET_APN_NAME_MAX_LEN];
    ulong wan_ipv4_address;          /* IPv4 address of WAN net */
    ulong wan_ipv4_dns1_address;     /* IPv4 DNS address of WAN net */
    ulong wan_ipv4_dns2_address;     /* IPv4 DNS address of WAN net */
    ulong wan_ipv4_net_mask;
    ulong wan_ipv4_gateway;

    ulong wan_local_ipv6_addr[4];    /* local IPv6 address of WAN net */
    ulong wan_ipv6_address1[4];      /* global IPv6 address1 of WAN net */
    ulong wan_ipv6_address2[4];      /* global IPv6 address2 of WAN net */
    ulong wan_ipv6_dns1_address[4];  /* IPv6 DNS server address of WAN net */
    ulong wan_ipv6_dns2_address[4];  /* IPv6 DNS server address of WAN net */

    NX_IP *wan_ip_instance;          /* NX_IP instance of WAN net */
} WAN_NET;

typedef struct APN_STRUCT
{
    u8 *imsi;    /* imsi */
    u8 *apn;    /* apn */
    u8 *userid;    /* userid */
    u8 *password;    /* password */
    u8 auth_prot;    /* auth protocol */
    u8 *iccid;    /* iccid */
} APN;

typedef struct ONE_APN_PER_IMSI_STRUCT
{
    u8 apn_tried;
    CARRIER_APN apn_info;
} ONE_APN_PER_IMSI;

typedef struct ALL_APNS_PER_IMSI_STRUCT
{
    u8 imsi[ANET_FULL_IMSI_LEN];
    ONE_APN_PER_IMSI all_apns[ANET_APN_CNT_PER_IMSI];
} ALL_APNS_PER_IMSI;

typedef void (*aic_pdp_state_change_cb)(u8 pdp_cid, u8 pdp_state);

/* Define global functions. */
s32 aic_pdp_mgr_init(void);
u32 aic_set_pdp_deactivated(u16 pdp_cid);
boolean aic_is_pdp_activated(u16 cid);
u8 aic_get_wan_net_by_cid(u8 cid);
WAN_NET *aic_get_wan_net_by_ifc_index(u32 ifc_index);
boolean aic_is_valid_ipv4_addr(char *ipv4_address);
boolean aic_is_valid_ipv6_addr(ulong *ipv6_addr);
WAN_NET *aic_get_wan_net_per_pdp(u8 pdp_cid);
void aic_send_cmd_cgdcont(u8 cid, u8 *pdp_type, u8 *apn_name);
struct aic_network_ip_instance *aic_get_pdp_ip_instance(void);
s32 aic_disable_data_connection(u8 pdp_cid);
u32 aic_enable_data_connection(u8 pdp_cid, u8 *pdp_type, u8 *apn);
u32 aic_set_pdp_activated(u16 pdp_cid, u8 *ipv4_addr, u8 *ipv4_dns1,
                          u8 *ipv4_dns2, u8 *ipv6_suffix,
                          u8 *ipv6_dns1, u8 *ipv6_dns2, u8 ipv6_format);
u32 aic_get_ipv6_dns_server(ulong *dns_server_ptr, u32 wan_intf_index);
boolean aic_is_default_pdp_activated(void);
u32 aic_register_pdp_state_change_cb(aic_pdp_state_change_cb cb);
void aic_unregister_pdp_state_change_cb(aic_pdp_state_change_cb cb);
void aic_notify_pdp_state_change(u8 pdp_cid, u8 pdp_state);
u32 aic_send_cgact_exe_cmd(u8 is_to_deact, u8 pdp_cid);
void get_numeric_ipv4_addr(u8 *ip_num, u8 *ipv4_str);
WAN_NET *aic_get_activated_default_wan(void);
boolean aic_is_valid_pdp_type(u8 *pdp_type_str);
void aic_handle_pdp_deact(u8 cid, boolean is_flight_mode);
u32 aic_send_cgcontrdp_set_cmd(u8 cid);
u8 aic_get_pdp_ip_type(u8 cid);
void aic_attach_packet_domain(void);
void aic_deact_all_pdp(boolean is_flight_mode);
void aic_check_all_pdp_state(void);
WAN_NET *aic_get_all_wan_net(void);
u8 aic_get_max_wan_net_num(void);
u8 aic_get_pdp_intf_index(u8 pdp_cid);
NET_PDP_INFO *aic_get_pdp_info_by_cid(u8 cid);
u32 aic_set_apn_by_user(APN *apn_ptr);
boolean aic_need_try_saved_apns(void);

#endif /* AIC_NETWORK_PDP_MGR_H */
