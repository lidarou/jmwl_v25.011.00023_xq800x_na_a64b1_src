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
#define NX_IPV6_STATELESS_AUTOCONFIG_CONTROL

//#define NX_IPSEC_ENABLE
#define NX_TCP_ENABLE_WINDOW_SCALING

#define NX_TCP_ENABLE_KEEPALIVE

#define NX_NAT_ENABLE

#define NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY

#define NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
#define NX_DNS_CACHE_ENABLE
#define NX_BSD_ENABLE_DNS

#define NX_DISABLE_ERROR_CHECKING
#define NX_SNTP_DISABLE_ERROR_CHECKING
#define NX_NAT_DISABLE_ERROR_CHECKING
#define NX_DISABLE_NAT_INFO

#define NX_PPP_DISABLE_CHAP

#define NX_DISABLE_IGMPV2
#define NX_DISABLE_TCP_INFO
#define NX_DISABLE_UDP_INFO

#define NX_DISABLE_IPV4_RARP

#define NX_DISABLE_IPV4_IGMP
#define NX_BSD_DISABLE_APPEND_IP_HEADER

#define NX_DISABLE_ICMPV4_ERROR_MESSAGE
#define NX_DISABLE_ICMPV6_ERROR_MESSAGE

#define NX_DISABLE_IPV6_DAD
//#define NX_DISABLE_LOOPBACK_INTERFACE

#define NX_DISBALE_IP_STATUS_CHECK
#define NX_DISABLE_TCP_ACK_SEGMENT_PACKET

#define NX_ENABLE_IP_RAW_PACKET_FILTER
#define NX_ENABLE_IP_RAW_PACKET_ALL_STACK

#define NX_NAT_CACHE_SIZE              32768
#define AIC_USB_LAN_NET_CHECK
#define AIC_USB_NIC_MAX_QUEUED_NUM     256
#define AIC_ENABLE_IP_SHARE_COPY
#define ANET_USB_INTF_SELF_ADAPTIVE
#define ANET_ENABLE_USB_DONGLE

#define NX_BSD_SOCKET_RAW_PROTOCOL_TABLE_SIZE 2
#define NX_BSD_MAX_SOCKETS                    2
#define ALTE_MAX_HIGH_QUEUE_SIZE              120
#define ALTE_MAX_NORMAL_QUEUE_SIZE            90

#define ANET_ENABLE_DONGLE_NETWORK

#define ALTE_MAX_ICMP_FLIGHT_CNT              5
#define ALTE_FLIGHT_TIMEOUT                   16000

#endif /* __NX_CONFIG_H__ */
