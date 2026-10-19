/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** NetX Component                                                           */
/**                                                                          */
/**   NAT64                                                                  */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  COMPONENT DEFINITION                                   RELEASE           */
/*                                                                           */
/*    aic_nat64.h                                            PORTABLE C      */
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
/*  10-18-2023         Qingsheng Li               Initial Version 6.0        */
/*                                                                           */
/*****************************************************************************/

#ifndef __AIC_NAT64_H__
#define __AIC_NAT64_H__

#include <aic_list.h>

struct aic_nat64_device {
    u32 id;

    u32 ipv4_addr;
    u32 ipv4_netmask;

    u8 local_ipv6_prefix_len;
    u32 local_ipv6_prefix[4];
    u8 external_ipv6_prefix_len;
    u32 external_ipv6_prefix[4];

    u32 max_table_num;
    u32 used_table_num;

    struct list_head list;
    struct list_head table_list;
};

int aic_nat64_enable(u32 interface_id, u32 ipv4_addr, u32 ipv4_mask,
                     u8 local_prefix_len, u8 *local_prefix,
                     u8 external_prefix_len, u8 *external_prefix,
                     u32 max_table_num);
int aic_nat64_disable(u32 interface_id);

#endif

