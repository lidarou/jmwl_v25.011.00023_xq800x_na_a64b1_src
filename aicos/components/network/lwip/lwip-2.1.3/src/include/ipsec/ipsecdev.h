#ifndef IPSECDEV_H
#define IPSECDEV_H

#include "lwip/opt.h"

#if defined(EIPS) && EIPS == 1

#include <lwip/netif.h>
#include <lwip/pbuf.h>

#include "ipsec/ipsec.h"
#include "lwip/debug.h"
#include "ipsec/config.h"

#include <string.h>

struct ipsec_channel{
    u32 *src;
    u32 *src_net;
    u32 *dest;
    u32 *dest_net;
    u16 src_port;
    u16 dest_port;
    u32 spi_c;
    u32 spi_s;
    u8  protocol;  //UDP or TCP proto
    u8  mode;      //tunnel or transport
    u32 seq;
    u8  enc_alg;
    u8  *enckey;
    u16 enckey_size;
    u8  auth_alg;
    u8  *authkey;
    u16 authkey_size;
    u8  type;      //IPv4 or IPv6
};

struct pbuf * ipsecdev_input(struct pbuf* p, struct netif *inp);
struct pbuf * ipsecdev_output(struct netif* netif, struct pbuf* p, const ip4_addr_t *ipaddr);
struct pbuf * ipsecdev_output_ip6(struct netif* netif, struct pbuf* p, const ip6_addr_t *ipaddr);
void ipsecdev_add(struct netif* netif);

err_t CreateIpsecChannel(struct ipsec_channel *channel);
err_t DeleteIpsecChannel(struct ipsec_channel *channel);

#endif /* defined(EIPS) && EIPS == 1 */

#endif /* IPSECDEV_H */
