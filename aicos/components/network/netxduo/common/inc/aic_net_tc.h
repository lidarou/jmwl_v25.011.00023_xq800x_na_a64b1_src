/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_NET_TC_H__
#define __AIC_NET_TC_H__

#include <aic_type.h>

#define AIC_ETHERNET_SIZE                    14

int aic_tc_cache_recv_packet(NX_INTERFACE *interface_ptr, NX_PACKET *packet_ptr);
int aic_tc_cache_send_packet(NX_INTERFACE *interface_ptr, NX_PACKET *packet_ptr);

int aic_network_tc_enable(u32 interface_index, u32 send_max_speed,
                          u32 recv_max_speed);
int aic_network_tc_disable(u32 interface_index);

#endif
