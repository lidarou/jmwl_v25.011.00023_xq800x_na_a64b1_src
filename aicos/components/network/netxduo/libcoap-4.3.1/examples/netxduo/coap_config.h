/*
 * coap_config.h.netxduo -- NETXDUO configuration for libcoap
 *
 * Copyright (C) 2023-2024 Qingsheng Li <qingsheng.li@aicxtek.com> and others
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This file is part of the CoAP library libcoap.
 */

#ifndef COAP_CONFIG_H_
#define COAP_CONFIG_H_

#define WITH_NETXDUO 1

#ifndef COAP_CONSTRAINED_STACK
#define COAP_CONSTRAINED_STACK 1
#endif

#ifndef COAP_DISABLE_TCP
#define COAP_DISABLE_TCP 1
#endif

#define HAVE_TIME_H
#define _POSIX_TIMERS       1
#define HAVE_MALLOC
#define HAVE_SNPRINTF
#define COAP_SERVER_SUPPORT 0
#define COAP_CLIENT_SUPPORT 1

#define PACKAGE_NAME "libcoap"
#define PACKAGE_VERSION "4.3.1"
#define PACKAGE_STRING "libcoap 4.3.1"

/* it's just provided by libc. i hope we don't get too many of those, as
 * actually we'd need autotools again to find out what environment we're
 * building in */
#define HAVE_STRNLEN 1

#define HAVE_LIMITS_H

#define HAVE_PTHREAD_H

#define HAVE_PTHREAD_MUTEX_LOCK

#define NETSTACK_CONF_WITH_IPV6 1

#endif /* COAP_CONFIG_H_ */
