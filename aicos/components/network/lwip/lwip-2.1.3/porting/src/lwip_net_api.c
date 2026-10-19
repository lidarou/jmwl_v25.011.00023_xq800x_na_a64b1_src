
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0645a6
#define LOG_MODULE    AMOD_NET
#include <string.h>
#include "aic_osa.h"

#include "lwip/sockets.h"
#include "ipsec/ipsecdev.h"
#include "lwip_net_api.h"
#include "aic_ims_nic.h"

static LWIP_IMS_NET ims_net_list[IMS_PDP_NUM];
struct netif lwip_ims_netif;

static void aic_ipv6_glb_addr_dad_success_notify(struct netif *netif,
                                                 ip6_addr_t *ip6_addr)
{
    u8 ipv6_str[48] = {'\0'};

    inet_ntop(AF_INET6, ip6_addr, ipv6_str, 48);

    aicx_info(0x167bc2, "s", "[AIC_NET] notify ipv6 str: %s\n", ipv6_str);
    if (netif->ip6_prefix_notify)
        netif->ip6_prefix_notify(0, ipv6_str);
}

int aic_socket_register_establish_notify(int sockID,
                                         void (*tcp_establish_notify)(int s, void *pdata),
                                         void *pdata)
{
    aicx_info(0x0c787d, "d", "[SOCKET] register %d establish notify\n", sockID);

    return lwip_socket_register_establish_notify(sockID, tcp_establish_notify,
                                                 pdata);
}

int aic_socket_unregister_establish_notify(int sockID)
{
    aicx_info(0x0b972c, "d", "[SOCKET] unregister %d establish notify\n", sockID);

    return lwip_socket_unregister_establish_notify(sockID);
}

int aic_socket_register_receive_notify(int sockID,
                                       void (*tcp_recv_notify)(int s, void *pdata),
                                       void *pdata)
{
    aicx_info(0x1893ec, "d", "[SOCKET] register %d recv notify\n", sockID);

    return lwip_socket_register_receive_notify(sockID, tcp_recv_notify, pdata);
}

int aic_socket_unregister_receive_notify(int sockID)
{
    aicx_info(0x0713db, "d", "[SOCKET] unregister %d recv notify\n", sockID);

    return lwip_socket_unregister_receive_notify(sockID);
}

int aic_socket_register_tcp_disconnect_notify(int sockID,
                                              void (*tcp_disconnect_notify)(int s, void *pdata, int is_active),
                                              void *pdata)
{
    aicx_info(0x1ae923, "d", "[SOCKET] register %d disconnect notify\n", sockID);

    return lwip_socket_register_tcp_disconnect_notify(sockID,
                                                      tcp_disconnect_notify,
                                                      pdata);
}

int aic_socket_unregister_tcp_disconnect_notify(int sockID)
{
    aicx_info(0x1ae923, "d", "[SOCKET] register %d disconnect notify\n", sockID);

    return lwip_socket_unregister_tcp_disconnect_notify(sockID);
}

/* Get ipv4 and ipv6 addr from ip_addr,save them to ipv4_addr and ipv6_addr. */
void get_ims_ip_addr_str(u8 *ip_addr, u8 *ipv4_addr, u8 *ipv6_addr)
{
    u8 *tmp_ptr;
    char *save_ptr;

    if (!ip_addr || !ipv4_addr || !ipv6_addr) {
        return;
    }

    tmp_ptr = strchr(ip_addr, ',');
    /* Only ipv4 addr or ipv6 addr. */
    if (!tmp_ptr) {
        if (!strchr(ip_addr, ':')) {
            strncpy(ipv4_addr, ip_addr, (IMS_IPV4_ADDR_LEN - 1));
        } else {
            strncpy(ipv6_addr, ip_addr, (IMS_IPV6_ADDR_LEN - 1));
        }
    } else {
        tmp_ptr = strtok_r(ip_addr, ",", &save_ptr);
        while (tmp_ptr) {
            if (!strchr(tmp_ptr, ':')) {
                strncpy(ipv4_addr, tmp_ptr, (IMS_IPV4_ADDR_LEN - 1));
            } else {
                strncpy(ipv6_addr, tmp_ptr, (IMS_IPV6_ADDR_LEN - 1));
            }

            tmp_ptr = strtok_r(AIC_NULL, ",", &save_ptr);
        }
    }
}

LWIP_IMS_NET *get_ims_net_by_pdp_id(u8 pdp_id)
{
    u8 i;

    for (i = 0; i < IMS_PDP_NUM; i++) {
        if (ims_net_list[i].ims_pdp_cid == pdp_id) {
            return &ims_net_list[i];
        }
    }

    /* No ims net for the pdp id,then get an idle ims net. */
    for (i = 0; i < IMS_PDP_NUM; i++) {
        if (ims_net_list[i].ims_pdp_cid == 0) {
            return &ims_net_list[i];
        }
    }

    return NULL;
}

/**
 * lwip_ims_netif_create - Create IMS net interface.
 *
 * @ip_addr: ipv4 address. for example:0x7e000001
 * @ip6_addr: ipv6 address. for example:0xFe800000000000000000000000000001
 *
 */
netif_handle lwip_ims_netif_create(ip4_addr_t *ip_addr, ip6_addr_t *ip6_addr)
{
    s8_t chosen_idx;
    ip4_addr_t ip_netmask;
    ip6_addr_t local_ip6;
    int ret;

    IP4_ADDR(&ip_netmask, 255,255,255,0);
    if (ip_addr)
        netif_add(&lwip_ims_netif, ip_addr, &ip_netmask, ip_addr,
                  NULL, lwip_ims_init_netif, NULL);
    else
        netif_add(&lwip_ims_netif, NULL, NULL, NULL, NULL,
                  lwip_ims_init_netif, NULL);

#if LWIP_IPV6
    if (ip6_addr) {
        IP6_ADDR(&local_ip6, lwip_htonl(0xfe800000), 0x0, ip6_addr->addr[2],
                 ip6_addr->addr[3]);
        ret = netif_add_ip6_address(&lwip_ims_netif, &local_ip6, &chosen_idx);
        if (ret != ERR_OK)
            aicx_info(0x15e4a6, "", "[NETIF] add ipv6 address fail\r\n");
        else
            aicx_info(0x0de33f, "d", "[NETIF] add ipv6 address index %d success\r\n", chosen_idx);

        /* Set address state. */
#if LWIP_IPV6_DUP_DETECT_ATTEMPTS
        /* Will perform duplicate address detection (DAD). */
        netif_ip6_addr_set_state(&lwip_ims_netif, 0, IP6_ADDR_TENTATIVE);
#else
        /* Consider address valid. */
        netif_ip6_addr_set_state(&lwip_ims_netif, 0, IP6_ADDR_PREFERRED);
#endif /* LWIP_IPV6_AUTOCONFIG */

    } else {
        netif_create_ip6_linklocal_address(&lwip_ims_netif, 1);
    }
#endif /* LWIP_IPV6 */

    netif_set_default(&lwip_ims_netif);
    netif_set_up(&lwip_ims_netif);
    netif_set_ip6_autoconfig_enabled(&lwip_ims_netif, TRUE);

    return (netif_handle)(&lwip_ims_netif);
}

err_t lwip_ims_netif_delete(netif_handle handle)
{
    netif_remove((struct netif *)handle);
    return ERR_OK;
}

err_t lwip_ims_netif_set_ip(netif_handle handle, ip4_addr_t *ip_addr,
                            ip4_addr_t *ip_mask, ip4_addr_t *ip_gw)
{
    if (ip_addr)
        netif_set_ipaddr((struct netif *)handle, ip_addr);
    if (ip_mask)
        netif_set_netmask((struct netif *)handle, ip_mask);
    if (ip_gw)
        netif_set_gw((struct netif *)handle, ip_gw);

    return ERR_OK;
}

err_t lwip_ims_netif_set_ip6(netif_handle handle, ip6_addr_t *ip6_addr)
{
    s8_t chosen_idx;
    err_t err;

    if (!ip6_addr)
        return ERR_ARG;

    err = netif_add_ip6_address((struct netif *)handle, ip6_addr, &chosen_idx);
    if (err != ERR_OK)
        aicx_info(0x09871b, "", "[LWIP_IMS] add ipv6 address fail\r\n");
    else
        aicx_info(0x0dc19c, "d", "[LWIP_IMS] add ipv6 address index %d success\r\n", chosen_idx);

    return err;
}

void lwip_ims_netif_register_ip6_prefix_notify(netif_handle handle,
                                               void (*ip6_addr_notify)(u8 cid, u8 *ip6_addr))
{
    ((struct netif *)handle)->ip6_prefix_notify = ip6_addr_notify;
    ((struct netif *)handle)->ip6_glb_addr_dad_success_notify = aic_ipv6_glb_addr_dad_success_notify;
}

u32 set_ims_pdp_activated(u8 ims_pdp_cid, u8 *ip_addr, u32 addr_size)
{
    LWIP_IMS_NET *ims_net_ptr;
    ip6_addr_t ip6_addr;
    ip4_addr_t ip4_addr;

    ims_net_ptr = get_ims_net_by_pdp_id(ims_pdp_cid);
    if (!ims_net_ptr) {
        return 1;
    }

    ims_net_ptr->ims_pdp_cid = ims_pdp_cid;
    get_ims_ip_addr_str(ip_addr, ims_net_ptr->ipv4_addr, ims_net_ptr->ipv6_addr);
    if (strlen(ims_net_ptr->ipv4_addr) > 0) {
        inet_pton(AF_INET, ims_net_ptr->ipv4_addr, &ims_net_ptr->ipv4_addr_integer);
        ip4_addr.addr = ims_net_ptr->ipv4_addr_integer;
    }

    if (strlen(ims_net_ptr->ipv6_addr) > 0) {
        inet_pton(AF_INET6, ims_net_ptr->ipv6_addr, ims_net_ptr->local_ipv6_addr);
        aos_memcpy(ip6_addr.addr, ims_net_ptr->local_ipv6_addr, 16);
    }

    ims_net_ptr->netif = lwip_ims_netif_create(&ip4_addr, &ip6_addr);

    return 0;
}

void set_ims_pdp_deactivated(u8 ims_pdp_cid)
{
    LWIP_IMS_NET *ims_net_ptr;

    ims_net_ptr = get_ims_net_by_pdp_id(ims_pdp_cid);
    if (!ims_net_ptr) {
        return;
    }

    if (ims_net_ptr->netif) {
        lwip_ims_netif_delete(ims_net_ptr->netif);
    }

    aos_memset(ims_net_ptr, 0, sizeof(LWIP_IMS_NET));
}

u32 register_ipv6_prefix_notify(u8 ims_pdp_cid,
                                void (*ipv6_prefix_change_notify)(u8 cid, u8 *global_ipv6))
{
    LWIP_IMS_NET *ims_net_ptr;

    ims_net_ptr = get_ims_net_by_pdp_id(ims_pdp_cid);
    if (!ims_net_ptr) {
        return 1;
    }

    if (ims_net_ptr->netif) {
        lwip_ims_netif_register_ip6_prefix_notify(ims_net_ptr->netif,
                                                  ipv6_prefix_change_notify);
    } else {
        return 1;
    }

    return 0;
}

void config_channel_paras(struct ipsec_channel *channel, u32 *src,
                    u32 *src_net, u32 *dest, u32 *dest_net,
                    u16 src_port, u16 dest_port, u32 spi_c, u32 spi_s,
                    u32 seq,  u8 enc_alg, u8 *enckey,
                    u16 enckey_size, u8 auth_alg, u8 *authkey, u16 authkey_size,
                    u8 type)
{
    if (!channel)
        return;

    memset(channel, 0, sizeof(struct ipsec_channel));
    aicx_info(0x0095a8, "p", "[tcpip] config channel %p paras", channel);
    channel->src = src;
    channel->src_net = src_net;
    channel->dest = dest;
    channel->dest_net = dest_net;
    channel->src_port = src_port;
    channel->dest_port = dest_port;
    channel->spi_c = spi_c;
    channel->spi_s = spi_s;
    channel->protocol = IPSEC_PROTO_ESP;
    channel->mode = IPSEC_TRANSPORT;
    channel->seq = seq;
    channel->enc_alg = enc_alg;
    channel->enckey = enckey;
    channel->enckey_size = enckey_size;
    channel->auth_alg = auth_alg;
    channel->authkey = authkey;
    channel->authkey_size = authkey_size;
    channel->type = type;
}


