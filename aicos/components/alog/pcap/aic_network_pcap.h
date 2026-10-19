/**************************************************************************/
/*                                                                        */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    aic_network_pcap.h                                  PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Qingsheng Li, AIC MICRO Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains all the header and extern functions used by the  */
/*    pcap dump.                                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  01-22-2022     Qingsheng Li             Initial Version 1.0           */
/*                                                                        */
/**************************************************************************/

#ifndef _AIC_NETWORK_PCAP_H_
#define _AIC_NETWORK_PCAP_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef CONFIG_MCX_SUPPORT
#include <aic_mcx.h>
#endif
#include <aic_bitmap.h>
#include <aic_boot_mode.h>

#ifdef CONFIG_TCPIP_LWIP
#include <lwip/pbuf.h>
#endif

#ifndef __MCX_CODE
#define __MCX_CODE
#endif

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

int aic_network_pcap_update_time_do(u32 seconds, u32 milliseconds);
#define aic_network_pcap_update_time(seconds, milliseconds) do { \
        if (!aic_boot_mode_is_basic()) \
            aic_network_pcap_update_time_do(seconds, milliseconds); \
    } while (0)

int aic_network_cpcap_dump(u8 *packet_ptr, u32 packet_len);

#ifdef CONFIG_SUBSYS_AP

#define AIC_PDP_NIC_FLAG BIT(0)
#define AIC_USB_NIC_FLAG BIT(1)
#define AIC_PPP_NIC_FLAG BIT(2)

void aic_network_disable_pdp_nic_capdump();
void aic_network_disable_usb_nic_capdump();
void aic_network_disable_ppp_nic_capdump();

void aic_network_enable_pdp_nic_capdump();
void aic_network_enable_usb_nic_capdump();
void aic_network_enable_ppp_nic_capdump();

void aic_network_config_pcap_size(int len);
int aic_network_pcap_dump_do(u8 *packet_ptr, u32 packet_len, u8 type);

#define aic_network_pcap_dump(packet_ptr, packet_len, type) do { \
        if (!aic_boot_mode_is_basic()) \
            aic_network_pcap_dump_do(packet_ptr, packet_len, type); \
    } while (0)

int aic_network_data_dump(u8 *packet_ptr, u32 packet_len);

#endif

__MCX_CODE void aic_network_parse_cp_at(u8 *string);
__MCX_CODE void aic_network_cpcap_enable(void);
__MCX_CODE void aic_network_cpcap_disable(void);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
}
#endif

#endif
