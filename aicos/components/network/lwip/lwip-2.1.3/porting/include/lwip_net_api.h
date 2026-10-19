/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LWIP_NET_API_H_
#define _LWIP_NET_API_H_

#include "lwip/arch.h"
#include <fcntl.h>
#include "lwip/sockets.h"
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IMS_PDP_NUM 1
#define IMS_IPV4_ADDR_LEN 20
#define IMS_IPV6_ADDR_LEN 48

typedef int netif_handle;

typedef struct ims_net{
    netif_handle netif;
    u8 ims_pdp_cid;
    u8 ipv4_addr[IMS_IPV4_ADDR_LEN];  /* IPv4 address string */
    ulong ipv4_addr_integer;               /* numeric IPv4 address */
    u8 ipv6_addr[IMS_IPV6_ADDR_LEN];  /* IPv6 address string */
    ulong local_ipv6_addr[4];              /* numeric local IPv6 address */
} LWIP_IMS_NET;

netif_handle lwip_ims_netif_create(ip4_addr_t *ip_addr, ip6_addr_t *ip6_addr);
err_t lwip_ims_netif_delete(netif_handle handle);
err_t lwip_ims_netif_set_ip(netif_handle handle, ip4_addr_t *ip_addr,
                            ip4_addr_t *ip_mask, ip4_addr_t *ip_gw);
err_t lwip_ims_netif_set_ip6(netif_handle handle, ip6_addr_t *ip6_addr);

#if 1
/* register/unregister async callback notify */
int lwip_socket_register_establish_notify(int sockID,
                                          void (*tcp_establish_notify)(int s, void *pdata),
                                          void *pdata);
int lwip_socket_unregister_establish_notify(int sockID);
int lwip_socket_register_receive_notify(int sockID,
                                        void (*tcp_recv_notify)(int s, void *pdata),
                                        void *pdata);
int lwip_socket_unregister_receive_notify(int sockID);
int lwip_socket_register_tcp_disconnect_notify(int sockID,
                                               void (*tcp_disconnect_notify)(int s, void *pdata, int is_active),
                                               void *pdata);
int lwip_socket_unregister_tcp_disconnect_notify(int sockID);

#define  anet_accept(s,addr,addrlen)                 lwip_accept(s,addr,addrlen)
#define  anet_bind(s,name,namelen)                   lwip_bind(s,name,namelen)
#define  anet_connect(s,name,namelen)                lwip_connect(s,name,namelen)
#define  anet_getpeername(s,name,namelen)            lwip_getpeername(s,name,namelen)
#define  anet_getsockname(s,name,namelen)            lwip_getsockname(s,name,namelen)
#define  anet_listen(s,backlog)                      lwip_listen(s,backlog)
#define  anet_recvfrom(s,mem,len,flags,from,fromlen) lwip_recvfrom(s,mem,len,flags,from,fromlen)
#define  anet_recv(s,mem,len,flags)                  lwip_recv(s,mem,len,flags)
#define  anet_sendto(s,dataptr,size,flags,to,tolen)  lwip_sendto(s,dataptr,size,flags,to,tolen)
#define  anet_send(s,dataptr,size,flags)             lwip_send(s,dataptr,size,flags)
#define  anet_select(maxfdp1,readset,writeset,exceptset,timeout) lwip_select(maxfdp1,readset,writeset,exceptset,timeout)
#define  anet_socket(domain,type,protocol)           lwip_socket(domain,type,protocol)
#define  anet_getsockopt(s,level,optname,opval,optlen) lwip_setsockopt(s,level,optname,opval,optlen)
#define  anet_setsockopt(s,level,optname,opval,optlen) lwip_getsockopt(s,level,optname,opval,optlen)
#define  ANET_FD_SET   FD_SET

int aic_socket_register_establish_notify(int sockID,
                                         void (*tcp_establish_notify)(int s, void *pdata),
                                         void *pdata);

int aic_socket_unregister_establish_notify(int sockID);

int aic_socket_register_receive_notify(int sockID,
                                       void (*tcp_recv_notify)(int s, void *pdata),
                                       void *pdata);

int aic_socket_unregister_receive_notify(int sockID);

int aic_socket_register_tcp_disconnect_notify(int sockID,
                                              void (*tcp_disconnect_notify)(int s, void *pdata, int is_active),
                                              void *pdata);

int aic_socket_unregister_tcp_disconnect_notify(int sockID);

u32 set_ims_pdp_activated(u8 ims_pdp_cid, u8 *ip_addr, u32 addr_size);
void set_ims_pdp_deactivated(u8 ims_pdp_cid);
u32 register_ipv6_prefix_notify(u8 ims_pdp_cid,
                                void (*ipv6_prefix_change_notify)(u8 cid, u8 *global_ipv6));
#endif
#ifdef __cplusplus
}
#endif

#endif /* _LWIP_PORTING_SOCKETS_H_ */
