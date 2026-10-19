/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __IP4_ADDR_H__
#define __IP4_ADDR_H__

// #include "opt.h"
// #include "def.h"
#include "aic_type.h"
#include "aic_net_pdp_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This is the aligned version of ip_addr_t,
   used as local variable, on the stack, etc. */
struct ip_addr {
  u32 addr;
};

/** ip_addr_t uses a struct for convenience only, so that the same defines can
 * operate both on ip_addr_t as well as on ip_addr_p_t. */
typedef struct ip_addr ip_addr_t;

#if 0
/* This is the packed version of ip_addr_t,
   used in network headers that are itself packed */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct ip_addr_packed {
  PACK_STRUCT_FIELD(u32 addr);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "epstruct.h"
#endif

/** ip_addr_t uses a struct for convenience only, so that the same defines can
 * operate both on ip_addr_t as well as on ip_addr_p_t. */
typedef struct ip_addr_packed ip_addr_p_t;

/*
 * struct ipaddr2 is used in the definition of the ARP packet format in
 * order to support compilers that don't have structure packing.
 */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct ip_addr2 {
  PACK_STRUCT_FIELD(u16 addrw[2]);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "epstruct.h"
#endif
#endif
/* Forward declaration to not include netif.h */
// struct netif;

extern const ip_addr_t ip_addr_any;
extern const ip_addr_t ip_addr_broadcast;

/** IP_ADDR_ can be used as a fixed IP address
 *  for the wildcard and the broadcast address
 */
#define IP_ADDR_ANY         ((ip_addr_t *)&ip_addr_any)
#define IP_ADDR_BROADCAST   ((ip_addr_t *)&ip_addr_broadcast)

/** 255.255.255.255 */
#define IPADDR_NONE         ((u32)0xffffffffUL)
/** 127.0.0.1 */
#define IPADDR_LOOPBACK     ((u32)0x7f000001UL)
/** 0.0.0.0 */
#define IPADDR_ANY          ((u32)0x00000000UL)
/** 255.255.255.255 */
#define IPADDR_BROADCAST    ((u32)0xffffffffUL)

/* Definitions of the bits in an Internet address integer.

   On subnets, host and network parts are found according to
   the subnet mask, not these masks.  */
#define IP_CLASSA(a)        ((((u32)(a)) & 0x80000000UL) == 0)
#define IP_CLASSA_NET       0xff000000
#define IP_CLASSA_NSHIFT    24
#define IP_CLASSA_HOST      (0xffffffff & ~IP_CLASSA_NET)
#define IP_CLASSA_MAX       128

#define IP_CLASSB(a)        ((((u32)(a)) & 0xc0000000UL) == 0x80000000UL)
#define IP_CLASSB_NET       0xffff0000
#define IP_CLASSB_NSHIFT    16
#define IP_CLASSB_HOST      (0xffffffff & ~IP_CLASSB_NET)
#define IP_CLASSB_MAX       65536

#define IP_CLASSC(a)        ((((u32)(a)) & 0xe0000000UL) == 0xc0000000UL)
#define IP_CLASSC_NET       0xffffff00
#define IP_CLASSC_NSHIFT    8
#define IP_CLASSC_HOST      (0xffffffff & ~IP_CLASSC_NET)

#define IP_CLASSD(a)        (((u32)(a) & 0xf0000000UL) == 0xe0000000UL)
#define IP_CLASSD_NET       0xf0000000          /* These ones aren't really */
#define IP_CLASSD_NSHIFT    28                  /*   net and host fields, but */
#define IP_CLASSD_HOST      0x0fffffff          /*   routing needn't know. */
#define IP_MULTICAST(a)     IP_CLASSD(a)

#define IP_EXPERIMENTAL(a)  (((u32)(a) & 0xf0000000UL) == 0xf0000000UL)
#define IP_BADCLASS(a)      (((u32)(a) & 0xf0000000UL) == 0xf0000000UL)

#define IP_LOOPBACKNET      127                 /* official! */


#if BYTE_ORDER == BIG_ENDIAN
/** Set an IP address given by the four byte-parts */
#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((u32)((a) & 0xff) << 24) | \
                         ((u32)((b) & 0xff) << 16) | \
                         ((u32)((c) & 0xff) << 8)  | \
                          (u32)((d) & 0xff)
#else
/** Set an IP address given by the four byte-parts.
    Little-endian version that prevents the use of htonl. */
#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((u32)((d) & 0xff) << 24) | \
                         ((u32)((c) & 0xff) << 16) | \
                         ((u32)((b) & 0xff) << 8)  | \
                          (u32)((a) & 0xff)
#endif

/** MEMCPY-like copying of IP addresses where addresses are known to be
 * 16-bit-aligned if the port is correctly configured (so a port could define
 * this to copying 2 u16's) - no NULL-pointer-checking needed. */
#ifndef IPADDR2_COPY
#define IPADDR2_COPY(dest, src) memcpy(dest, src, sizeof(ip_addr_t))
#endif

/*tag-201605251531, begin*/	
/** check src IP address - local srcaddr check */
#define ip_srcaddr_islocal(src)     (IS_LOCAL_IP_ADDR(src.addr))
/*tag-201605251531, end*/	
#define IS_LOCAL_IP_ADDR(addr)     ip4_addr_islocal_u32(addr)
// #define ip_srcaddr_islocal(src)     ip4_addr_islocal_u32(src.addr)
u8 ip4_addr_islocal_u32(u32 addr);

/** Copy IP address - faster than ip_addr_set: no NULL check */
#define ip_addr_copy(dest, src) ((dest).addr = (src).addr)
/** Safely copy one IP address to another (src may be NULL) */
#define ip_addr_set(dest, src) ((dest)->addr = \
                                    ((src) == NULL ? 0 : \
                                    (src)->addr))
/** Set complete address to zero */
#define ip_addr_set_zero(ipaddr)      ((ipaddr)->addr = 0)
/** Set address to IPADDR_ANY (no need for htonl()) */
#define ip_addr_set_any(ipaddr)       ((ipaddr)->addr = IPADDR_ANY)
/** Set address to loopback address */
#define ip_addr_set_loopback(ipaddr)  ((ipaddr)->addr = PP_HTONL(IPADDR_LOOPBACK))
/** Check if an address is in the loopback region */
#define ip_addr_isloopback(ipaddr)    (((ipaddr)->addr & PP_HTONL(IP_CLASSA_NET)) == PP_HTONL(((u32)IP_LOOPBACKNET) << 24))
/** Safely copy one IP address to another and change byte order
 * from host- to network-order. */
#define ip_addr_set_hton(dest, src) ((dest)->addr = \
                               ((src) == NULL ? 0:\
                               htonl((src)->addr)))
/** IPv4 only: set the IP address given as an u32 */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
/** IPv4 only: get the IP address as an u32 */
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)

/** Get the network address by combining host address with netmask */
#define ip_addr_get_network(target, host, netmask) ((target)->addr = ((host)->addr) & ((netmask)->addr))

/**
 * Determine if two address are on the same network.
 *
 * @arg addr1 IP address 1
 * @arg addr2 IP address 2
 * @arg mask network identifier mask
 * @return !0 if the network identifiers of both address match
 */
#define ip_addr_netcmp(addr1, addr2, mask) (((addr1)->addr & \
                                              (mask)->addr) == \
                                             ((addr2)->addr & \
                                              (mask)->addr))
#define ip_addr_cmp(addr1, addr2)  ((addr1)->addr == (addr2)->addr)
#define ip_addr_eq(addr1, addr2)   ((addr1)->addr == (addr2)->addr)
#define ip_addr_val_isany(addrval) (addrval == IPADDR_ANY)
#define ip_addr_isany(addr1) ((addr1) == NULL || (addr1)->addr == IPADDR_ANY)

#define ip_addr_isbroadcast(ipaddr, netif) ip4_addr_isbroadcast_u32((ipaddr)->addr, (netif))
u8 ip4_addr_isbroadcast_u32(u32 addr, const WAN_NET *netif);

#define ip_addr_netmask_valid(netmask) ip4_addr_netmask_valid((netmask)->addr)
u8 ip4_addr_netmask_valid(u32 netmask);

#define ip_addr_val_ismulticast(addrval) ((addrval & PP_HTONL(0xf0000000UL)) == PP_HTONL(0xe0000000UL))
#define ip_addr_ismulticast(addr1) (((addr1)->addr & PP_HTONL(0xf0000000UL)) == PP_HTONL(0xe0000000UL))

#define ip_addr_islinklocal(addr1) (((addr1)->addr & PP_HTONL(0xffff0000UL)) == PP_HTONL(0xa9fe0000UL))

#define ip_addr_debug_print(debug, cat3, ipaddr)                               \
    alog_debug("%u.%u.%u.%u", ipaddr != NULL ? ip4_addr1_16(ipaddr) : 0,         \
               ipaddr != NULL ? ip4_addr2_16(ipaddr) : 0,                        \
               ipaddr != NULL ? ip4_addr3_16(ipaddr) : 0,                        \
               ipaddr != NULL ? ip4_addr4_16(ipaddr) : 0)

/* Get one byte from the 4-byte address */
#define ip4_addr1(ipaddr) (((u8*)(ipaddr))[0])
#define ip4_addr2(ipaddr) (((u8*)(ipaddr))[1])
#define ip4_addr3(ipaddr) (((u8*)(ipaddr))[2])
#define ip4_addr4(ipaddr) (((u8*)(ipaddr))[3])
/* These are cast to u16, with the intent that they are often arguments
 * to printf using the U16_F format from cc.h. */
#define ip4_addr1_16(ipaddr) ((u16)ip4_addr1(ipaddr))
#define ip4_addr2_16(ipaddr) ((u16)ip4_addr2(ipaddr))
#define ip4_addr3_16(ipaddr) ((u16)ip4_addr3(ipaddr))
#define ip4_addr4_16(ipaddr) ((u16)ip4_addr4(ipaddr))

#define IP4ADDR_STRLEN_MAX  16

/** For backwards compatibility */
// #define ip_ntoa(ipaddr)  ipaddr_ntoa(ipaddr)
#define ip_ntoa(ipaddr)  inet_ntoa(ipaddr)
#define ipaddr_ntoa_r(ipaddr,buf,buflen)  inet_ntop(AF_INET,ipaddr,buf,buflen)
#define ipaddr_aton(addr_str,ipaddr)  inet_pton(AF_INET,addr_str,ipaddr)

u32 ipaddr_addr(const char *cp);
// int ipaddr_aton(const char *cp, ip_addr_t *addr);
/** returns ptr to static buffer; not reentrant! */
// char *ipaddr_ntoa(const ip_addr_t *addr);
// char *ipaddr_ntoa_r(const ip_addr_t *addr, char *buf, int buflen);
char *macaddr_ntoa(u8 *macaddr);

ip_addr_t *ip_netif_get_local_ip(WAN_NET *netif);

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_IP_ADDR_H__ */
