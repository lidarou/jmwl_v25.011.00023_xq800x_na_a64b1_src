/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_LWIPOPTS_H
#define LWIP_HDR_LWIPOPTS_H

#define LWIP_NO_UNISTD_H 1
#define LWIP_NO_LIMITS_H 1

#include <errno.h>
#include <fcntl.h>
#include <ioctl.h>

typedef unsigned int sys_prot_t;

#ifdef CONFIG_SUBSYS_AP
#define LWIP_NO_CTYPE_H 1
#endif

// lwIP debug options, comment the ones you don't want
#define LWIP_DEBUG 0
#if LWIP_DEBUG
#define ETHARP_DEBUG                    LWIP_DBG_OFF
#define NETIF_DEBUG                     LWIP_DBG_OFF
#define PBUF_DEBUG                      LWIP_DBG_OFF
#define API_LIB_DEBUG                   LWIP_DBG_OFF
#define API_MSG_DEBUG                   LWIP_DBG_OFF
#define SOCKETS_DEBUG                   LWIP_DBG_OFF
#define ICMP_DEBUG                      LWIP_DBG_OFF
#define IGMP_DEBUG                      LWIP_DBG_OFF
#define INET_DEBUG                      LWIP_DBG_OFF
#define IP_DEBUG                        LWIP_DBG_OFF
#define DRIVERIF_DEBUG                  LWIP_DBG_OFF
#define IP_REASS_DEBUG                  LWIP_DBG_OFF
#define RAW_DEBUG                       LWIP_DBG_OFF
#define MEM_DEBUG                       LWIP_DBG_OFF
#define MEMP_DEBUG                      LWIP_DBG_OFF
#define SYS_DEBUG                       LWIP_DBG_OFF
#define TIMERS_DEBUG                    LWIP_DBG_OFF
#define TCP_DEBUG                       LWIP_DBG_OFF
#define TCP_ERR_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
#define TCP_FR_DEBUG                    LWIP_DBG_OFF
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF
#define TCP_WND_DEBUG                   LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
#define TCP_RST_DEBUG                   LWIP_DBG_OFF
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF
#define TCP_SACK_DEBUG                  LWIP_DBG_OFF
#define TCP_TLP_DEBUG                   LWIP_DBG_OFF
#define UDP_DEBUG                       LWIP_DBG_OFF
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#define SLIP_DEBUG                      LWIP_DBG_OFF
#define DHCP_DEBUG                      LWIP_DBG_OFF
#define AUTOIP_DEBUG                    LWIP_DBG_OFF
#define DNS_DEBUG                       LWIP_DBG_OFF
#define TFTP_DEBUG                      LWIP_DBG_OFF
#define SYS_ARCH_DEBUG                  LWIP_DBG_OFF
#define SNTP_DEBUG                      LWIP_DBG_OFF
#define IP6_DEBUG                       LWIP_DBG_OFF
#define DHCP6_DEBUG                     LWIP_DBG_OFF
#define DRV_STS_DEBUG                   LWIP_DBG_OFF
#endif
#define IPSEC_DEBUG                     LWIP_DBG_OFF
#define IPSEC_LOG_MESSAGE_SIZE          128

#define MEM_USE_POOLS                   0
#define EIPS                            1
#define DUMMY_LOOPBACK_SA_SP            0

//#define LWIP_PROVIDE_ERRNO
//#define LWIP_ERRNO_STDINCLUDE

// Options only in new opt.h
#define LWIP_SOCKET_SELECT              1
#define LWIP_SOCKET_POLL                0

#define LWIP_TIMEVAL_PRIVATE    0

// Options in old opt.h that differs from new opt.h
#define MEM_ALIGNMENT                   __SIZEOF_POINTER__
#define MEMP_NUM_NETDB                  8
#define IP_REASS_MAXAGE                 3
#define IP_SOF_BROADCAST                1
#define IP_SOF_BROADCAST_RECV           1
#define LWIP_MULTICAST_PING             0
//#define LWIP_DHCP_AUTOIP_COOP_TRIES     64
#define TCP_LISTEN_BACKLOG              1
#define TCP_DEFAULT_LISTEN_BACKLOG      16

#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   7

#define LWIP_SOCKET_OFFSET              5120
#define LWIP_NETIF_REMOVE_CALLBACK      1
#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_TX_SINGLE_PBUF       1
#define LWIP_NETCONN_FULLDUPLEX         0 // Caution
#define LWIP_COMPAT_SOCKETS             1
#define LWIP_POSIX_SOCKETS_IO_NAMES     0
#define LWIP_TCP_KEEPALIVE              1
#define RECV_BUFSIZE_DEFAULT            65535
#define SO_REUSE_RXTOALL                1

#define LWIP_CHECKSUM_ON_COPY           1
#define LWIP_IPV6                       1
#define LWIP_IPV6_NUM_ADDRESSES         5
#define LWIP_ND6_NUM_PREFIXES           10
#define LWIP_IPV6_DHCP6                 0
#define LWIP_IPV6_DHCP6_STATEFUL        0
#define LWIP_IPV6_MLD                   0

// Options in old lwipopts.h
#define ARP_QUEUEING                    0
#define DEFAULT_ACCEPTMBOX_SIZE         32
#define DEFAULT_RAW_RECVMBOX_SIZE       128
#define DEFAULT_TCP_RECVMBOX_SIZE       128
#define DEFAULT_UDP_RECVMBOX_SIZE       128
#define ETHARP_SUPPORT_STATIC_ENTRIES   1
#define ETH_PAD_SIZE                    2
//#define IP_REASS_MAX_PBUFS              (((65535) / (IP_FRAG_MAX_MTU - 20 - 8) + 1) * MEMP_NUM_REASSDATA)
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_OFF
#define LWIP_DHCP                       0
#define LWIP_DNS                        0
#define LWIP_ARP                        0
#define LWIP_ETHERNET                   0
#define LWIP_HAVE_LOOPIF                0
#define LWIP_IGMP                       1
#define LWIP_NETIF_API                  1
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_LOOPBACK             0
#define LWIP_RAW                        0
#define LWIP_SO_RCVBUF                  1
#define LWIP_SO_RCVTIMEO                1
#define LWIP_SO_SNDTIMEO                1
#define LWIP_STATS_DISPLAY              1
#define MEM_LIBC_MALLOC                 1
//#define MEMP_NUM_ARP_QUEUE              (65535 * LWIP_CONFIG_NUM_SOCKETS / (IP_FRAG_MAX_MTU - 20 - 8))
/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF                 (65535 * 3 * LWIP_CONFIG_NUM_SOCKETS / (IP_FRAG_MAX_MTU - 20 - 8))
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN                LWIP_CONFIG_NUM_SOCKETS
/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF                   LWIP_CONFIG_NUM_SOCKETS*2
/* MEMP_NUM_RAW_PCB: the number of UDP protocol control blocks. One
   per active RAW "connection". */
#define MEMP_NUM_RAW_PCB                LWIP_CONFIG_NUM_SOCKETS
//#define MEMP_NUM_REASSDATA              (IP_REASS_MAX_MEM_SIZE / 65535)
/* MEMP_NUM_TCPIP_MSG_*: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#define MEMP_NUM_TCPIP_MSG_API          32
#define MEMP_NUM_TCPIP_MSG_INPKT        64
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB                LWIP_CONFIG_NUM_SOCKETS
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN         LWIP_CONFIG_NUM_SOCKETS
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG                (((TCP_SND_BUF * 3 / 2) + TCP_WND) * LWIP_CONFIG_NUM_SOCKETS / TCP_MSS)
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB                LWIP_CONFIG_NUM_SOCKETS
/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
//#define MEM_SIZE                        (1600) // (512*1024)
/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE               1550
/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE                  16
#define SO_REUSE                        1
#define TCPIP_MBOX_SIZE                 256
#define TCPIP_THREAD_PRIO               7
#define TCPIP_THREAD_STACKSIZE          0x800
/* ---------- TCP options ---------- */
/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX                      64
/* TCP Maximum segment size. */
#define TCP_MSS                         1400
/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF                     65535
/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN                (8 * TCP_SND_BUF) / TCP_MSS
#define TCP_TTL                         255
/* TCP receive window. */
#define TCP_WND                         32768
#define UDP_TTL                         255

// Options in old lwipopts.h but kept in Defaults with new opt.h
#define IP_FORWARD                      0
#define LWIP_DBG_TYPES_ON               LWIP_DBG_ON
#define LWIP_ICMP                       1
#define LWIP_NETCONN                    1
#define LWIP_SOCKET                     1
#define LWIP_STATS                      1
#define LWIP_TCP                        1
#define LWIP_UDP                        1
#define NO_SYS                          0
#define TCP_QUEUE_OOSEQ                 LWIP_TCP

#if defined LWIP_STATS_DISPLAY && !LWIP_STATS
#undef LWIP_STATS_DISPLAY
#define LWIP_STATS_DISPLAY              0
#endif

// Change some options for lwIP 2.1.2
#undef TCP_MAXRTX
#define TCP_MAXRTX                      12

#define MEMP_NUM_SYS_TIMEOUT            (LWIP_NUM_SYS_TIMEOUT_INTERNAL + (LWIP_IPV6 * LWIP_IPV6_DHCP6))

#undef DEFAULT_ACCEPTMBOX_SIZE
#define DEFAULT_ACCEPTMBOX_SIZE         LWIP_CONFIG_NUM_SOCKETS

#undef TCP_MSS
#define TCP_MSS                         (IP_FRAG_MAX_MTU - 20 - 20)

#undef IP_SOF_BROADCAST_RECV
#define IP_SOF_BROADCAST_RECV           0

/**
 * Defines whether to enable debugging for driver module.
 */
#ifndef DRIVERIF_DEBUG
#define DRIVERIF_DEBUG                  LWIP_DBG_OFF
#endif

// Options for old lwipopts.h
#define IP_FRAG_MAX_MTU                 1500
#define LWIP_CONFIG_NUM_SOCKETS         32
//#define IP_REASS_MAX_MEM_SIZE           (MEM_SIZE / 4)

// Options for enhancement code, same for old lwipopts.h
#define LWIP_ENABLE_NET_CAPABILITY      1
#define LWIP_ENABLE_CAP_NET_BROADCAST   0

#define MEMP_MEM_MALLOC                 1

// use PBUF_RAM instead of PBUF_POOL in udp_input
#define USE_PBUF_RAM_UDP_INPUT          1

#define LWIP_SOCKET_IOCTL               1
#define LWIP_SOCKET_FCNTL               1

#define LWIP_PBUF_CUSTOM_DATA           u32 reserved;

#define IP_PROTO_ESP                    50
#define IP_PROTO_AH                     51

#define IP6_NEXTH_ESP                   50
#define IP6_NEXTH_AH                    51
/** If set, the netif has IPSEC capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_IPSEC                0x80U
/** indicates this pbuf includes a IPSEC flag */
#define PBUF_FLAG_IPSEC                 0x40U

#define CONFIG_USE_LOS                  1

#endif /* LWIP_HDR_LWIPOPTS_H */
