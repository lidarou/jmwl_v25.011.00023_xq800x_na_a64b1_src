/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines NETXDUO configurations.
 */

#ifndef __NX_CONFIG_H__
#define __NX_CONFIG_H__

#define NX_ENABLE_EXTENDED_NOTIFY_SUPPORT

//#define NX_IPSEC_ENABLE
#define NX_TCP_ENABLE_WINDOW_SCALING
#define NX_IPV6_STATELESS_AUTOCONFIG_CONTROL
#define NX_TCP_ENABLE_KEEPALIVE
#define NX_TCP_KEEPALIVE_INITIAL        30
#define NX_TCP_KEEPALIVE_RETRY          30

#define NX_NAT_ENABLE
#define NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY

#define NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
#define NX_DNS_CACHE_ENABLE
#define NX_BSD_ENABLE_DNS

//#define AIC_HTTP_REDIR
#define NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_TCP_ACK_SEGMENT_PACKET

#define NX_NAT_CACHE_SIZE              32768
#define AIC_USB_LAN_NET_CHECK
#define AIC_USB_NIC_MAX_QUEUED_NUM     256
#define AIC_ENABLE_IP_SHARE_COPY
#define ANET_USB_INTF_SELF_ADAPTIVE
#define ANET_ENABLE_USB_DONGLE

#define ANET_ENABLE_DONGLE_NETWORK

#define ALTE_MAX_ICMP_FLIGHT_CNT       5
#define ALTE_FLIGHT_TIMEOUT            16000

#endif /* __NX_CONFIG_H__ */
