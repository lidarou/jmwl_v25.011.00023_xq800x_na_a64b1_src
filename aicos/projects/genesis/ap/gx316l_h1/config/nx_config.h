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
#define NX_WEB_HTTPS_ENABLE

//#define NX_IPSEC_ENABLE
#define NX_TCP_ENABLE_WINDOW_SCALING
#define NX_IPV6_STATELESS_AUTOCONFIG_CONTROL

#define NX_TCP_ENABLE_KEEPALIVE

#define NX_NAT_ENABLE

#define NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY

#define NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
#define NX_DNS_CACHE_ENABLE
#define NX_BSD_ENABLE_DNS

/* only for mqtts */
#define NX_SECURE_ENABLE

//#define AIC_HTTP_REDIR
#define NX_DISABLE_ERROR_CHECKING
#define NX_NAT_DISABLE_ERROR_CHECKING

#define ANET_MAX_PACKET_NUM 128
#define NX_DISABLE_TCP_ACK_SEGMENT_PACKET

#endif /* __NX_CONFIG_H__ */
