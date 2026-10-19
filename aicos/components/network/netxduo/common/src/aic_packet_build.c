/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Build Packet Management (Packet)                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f86cc
#define LOG_MODULE    AMOD_NET

#include <aic_errno.h>
#include <nx_api.h>
#include <nx_packet.h>

#include "aic_packet_management.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    aic_packet_build                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Qingsheng Li, AIC Micro Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function build a packet from the specified memory.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    data                                  specified memory              */
/*    len                                   data length                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_PACKET                             The constructed packet        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  03-07-2022     Qingsheng Li               Initial Version 6.9         */
/*                                                                        */
/**************************************************************************/
NX_PACKET *aic_packet_build(u8 *start, u32 len, u32 offset,
                            void *def_pool, u8 mem_type)
{
    NX_PACKET *packet_ptr;

    packet_ptr = aic_alloc_packet();
    if (!packet_ptr) {
        aicx_error(0x1d0a4e, "", "alloc nx packet fail\n");
        return NULL;
    }

    packet_ptr->nx_packet_reserved |= mem_type;
    packet_ptr->nx_packet_pool_owner = def_pool;

    packet_ptr->nx_packet_data_start = start;
    packet_ptr->nx_packet_data_end = start + len;
    /* Setup various fields for this packet. */
    packet_ptr->nx_packet_queue_next = NX_NULL;
#ifndef NX_DISABLE_PACKET_CHAIN
    packet_ptr->nx_packet_next = NX_NULL;
    packet_ptr->nx_packet_last = NX_NULL;
#endif
    packet_ptr->nx_packet_length = 0;
    packet_ptr->nx_packet_prepend_ptr =
        packet_ptr->nx_packet_data_start + offset;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr;
    packet_ptr->nx_packet_address.nx_packet_interface_ptr = NX_NULL;
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    packet_ptr->nx_packet_interface_capability_flag = 0;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    /* Set the TCP queue to the value that indicates it has been allocated. */
    /* lint -e{923} suppress cast of ULONG to pointer. */
    packet_ptr->nx_packet_union_next.nx_packet_tcp_queue_next =
        (NX_PACKET *)NX_PACKET_ALLOCATED;

#ifdef FEATURE_NX_IPV6

    /* Clear the option state. */
    packet_ptr->nx_packet_option_state = 0;
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE

    /* Clear the ipsec state. */
    packet_ptr->nx_packet_ipsec_state = 0;
    packet_ptr->nx_packet_ipsec_sa_ptr = NX_NULL;
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
    /* Initialize the IP version field */
    packet_ptr->nx_packet_ip_version = NX_IP_VERSION_V4;
#endif

    /* Initialize the IP identification flag. */
    packet_ptr->nx_packet_identical_copy = NX_FALSE;

    /* Initialize the IP header length. */
    packet_ptr->nx_packet_ip_header_length = 0;

#ifdef NX_ENABLE_THREAD
    packet_ptr->nx_packet_type = 0;
#endif

    return packet_ptr;
}

