/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** NetX Component                                                           */
/**                                                                          */
/**   IPv6 forward                                                           */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  COMPONENT DEFINITION                                   RELEASE           */
/*                                                                           */
/*    aic_ipv6_forward.h                                     PORTABLE C      */
/*                                                           6.1.9           */
/*  AUTHOR                                                                   */
/*                                                                           */
/*    Qingsheng Li, AIC-Micro Corporation                                    */
/*                                                                           */
/*  DESCRIPTION                                                              */
/*                                                                           */
/*    This file defines the NetX Internet Protocol component,                */
/*    including all data types and external references.  It is assumed       */
/*    that nx_api.h and nx_port.h have already been included.                */
/*                                                                           */
/*  RELEASE HISTORY                                                          */
/*                                                                           */
/*    DATE              NAME                      DESCRIPTION                */
/*                                                                           */
/*  7-7-2022         Qingsheng Li               Initial Version 6.0          */
/*                                                                           */
/*****************************************************************************/

#ifndef __AIC_IPV6_FORWARD_H__
#define __AIC_IPV6_FORWARD_H__

#include <aic_osa.h>
#include <aic_list.h>
#include <aic_errno.h>

#include "aic_net_api.h"

#define AIC_IPV6_INBOUND  0
#define AIC_IPV6_OUTBOUND 1

#define AIC_IPV6_MAX_FORWARD_TABLE 256

struct aic_net_ipv6_forward_table {
    struct list_head list;

    u64 timestamp;

    u32 local_ipv6_addr[4];
    u32 external_ipv6_addr[4];
    u8 protocol;
    u16 local_port;
    u16 external_port;

    u16 inbound_win_scal;
    u16 outbound_win_scal;
};

struct aic_network_ipv6_router {
    u32 interface_id;
    NX_PACKET *ra_packet_ptr;

    u32 duid_time;

    u32 max_table_num;
    u32 used_table_num;
    struct list_head forward_list;
};

void aic_ipv6_forward_table_timeout(void);
int aic_ipv6_packet_router(NX_PACKET *packet_ptr, u8 is_unicast);
void aic_ipv6_forward_release_packet(u32 interface_index);
void aic_enable_ipv6_packet_route(struct aic_network_ip_instance *aic_ip_instance);
void aic_disable_ipv6_packet_route(struct aic_network_ip_instance *aic_ip_instance);
#ifdef CONFIG_NET_SHARE_COMMON
int aic_ipv6_send_invalid_ra(u32 interface_index);
int aic_network_ipv6_send_ra(NX_IP *ip_ptr, u32 interface_index);
#endif

#endif

