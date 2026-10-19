/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the structs and apis for managing IMS net.
 */

#ifndef AIC_IMS_NET_MGR_H
#define AIC_IMS_NET_MGR_H

#ifdef CONFIG_IMS_EMRG_SUPPORT
#define ANET_IMS_PDP_NUM 2
#else
#define ANET_IMS_PDP_NUM 1
#endif

#define ANET_DEFAULT_IMS_PDP_CID 5

#define ANET_IMS_IFC_INDEX_DEFAULT 0

#define ANET_IMS_PDP_ACTIVATED 1
#define ANET_IMS_PDP_DEACTIVATED 0

#define ANET_IMS_IPV4_ADDR_LEN 20
#define ANET_IMS_IPV6_ADDR_LEN 48

#define ANET_IMS_INTERF_NAME_LEN 16

#define ANET_IMS_PACKET_SIZE 1600
#define ANET_IMS_PACKET_POOL_SIZE ANET_IMS_PACKET_SIZE * 64

#define ANET_IMS_PDP_MTU_SIZE 1500

#define ANET_IMS_IP_TASK_STACK_SIZE SIZE_2K

#define ANET_IMS_DEFAULT_NET_MASK 0xffffff00UL

#define ANET_IMS_INTF_NAME_START "anet_ims"

#define ANET_IMS_NET_ERROR_BASE 0x100 + 500
#define ANET_IMS_IFC_INDEX_ERROR ANET_IMS_NET_ERROR_BASE + 1
#define ANET_ERROR_NO_IMS_NET ANET_IMS_NET_ERROR_BASE + 2
#define ANET_ERROR_IMS_INTF_CONFIG ANET_IMS_NET_ERROR_BASE + 3

typedef struct IMS_NET_STRUCT
{
    u8 ims_pdp_cid;                        /* IMS pdp cid */
    u8 ims_activated;                      /* IMS pdp is activated or not */
    u32 ifc_index;                         /* Interface index in IP instance */
    u8 ifc_name[ANET_IMS_INTERF_NAME_LEN]; /* Interface name */
    u8 ipv4_addr[ANET_IMS_IPV4_ADDR_LEN];  /* IPv4 address string */
    ulong ipv4_addr_integer;               /* numeric IPv4 address */
    ulong ipv4_net_mask;                   /* IPv4 network mask */
    ulong ipv4_gateway;                    /* IPv4 gateway */
    u8 ipv6_addr[ANET_IMS_IPV6_ADDR_LEN];  /* IPv6 address string */
    ulong local_ipv6_addr[4];              /* numeric local IPv6 address */
    ulong ipv6_addr1_integer[4];           /* numeric global IPv6 address1 */
    ulong ipv6_addr2_integer[4];           /* numeric global IPv6 address2 */

    NX_IP *ims_ip_instance;                /* NX_IP instance of IMS bearer */
#ifdef CONFIG_IMS_EMRG_SUPPORT
    void (*ipv6_prefix_notify_handler)(u8 cid,u8 *global_ipv6);
#else
    void (*ipv6_prefix_notify_handler)(u8 *global_ipv6);
#endif
} IMS_NET;

/* Define global functions. */
s32 aic_ims_net_init(void);
u32 set_ims_pdp_activated(u8 ims_pdp_cid, u8 *ip_addr, u32 addr_size);
void set_ims_pdp_deactivated(u8 ims_pdp_cid);
NX_IP *get_ims_nx_ip(void);
#ifdef CONFIG_IMS_EMRG_SUPPORT
u32 register_ipv6_prefix_notify(u8 ims_pdp_cid,
    void (*ipv6_prefix_change_notify)(u8 cid, u8 *global_ipv6));
#else
u32 register_ipv6_prefix_notify(u8 ims_pdp_cid,
    void (*ipv6_prefix_change_notify)(u8 *global_ipv6));
#endif
char *strtok_r(char *str, const char *sep, char **state);

#endif /* AIC_IMS_NET_MGR_H */
