/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Aic Management of Tranport Layer Ports
 *    This is the management of transport layer ports and directing
 *    upstream and downstream packets for network share.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17c7dc
#define LOG_MODULE    AMOD_NET

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <nx_api.h>
#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_api.h>
#include "aic_at_cmd_handle.h"
#include "aic_net_share_common.h"
#include "aic_net_share_port_mgr.h"
#include "aic_at_resp_handle.h"
#include "aic_usb_net_share_mgr.h"
#include "aic_net_pdp_mgr.h"
#include "aic_ppp_dial_mgr.h"
#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
#include <aic_ipv6_forward.h>
#endif

#ifdef CONFIG_NET_SEMI_TETHER
#include "aic_net_usb_semi_tether.h"
#endif

/* Define internal vars. */
static aos_mutex_ptr ppp_dial_hash_mutex;
static u16 next_ppp_dial_src_port = ANET_PPP_DIAL_PORT_START;
hashtable_t *ppp_dial_hashtable = AIC_NULL;
hashtable_t *usb_net_hashtable = AIC_NULL;
static aos_mutex_ptr usb_net_hash_mutex;
static u16 next_usb_net_src_port = ANET_USBSHARE_PORT_START;
static aos_timer_ptr net_port_timer;
#if 0
static hashtable_t *usbshare_tether_hashtable[ANET_USB_TETHER_MAX_CLIENTS];
static u16 usbshare_port_section =
    (ANET_USBSHARE_PORT_END - (ANET_USBSHARE_PORT_START - 1)) /
    ANET_USB_TETHER_MAX_CLIENTS;
static hashtable_t usbshare_ndis_down_hashtable;
#endif

/* Define global vars. */

/* Define internal functions. */
static void ppp_dial_hash_element_exist(void *existing, void *to_insert);
static void usb_net_hash_element_exist(void *existing, void *to_insert);
static void create_net_port_timer(void);
static void handle_net_port_timeout(void *input);
static u16 get_ppp_dial_idle_port(void);
static u16 get_usb_net_idle_port(void);
static PORT_CONVERT *find_port_convert_element(hashtable_t *hashtable_ptr,
                                               u16 original_port);
static void free_hash_item(void *element);
static void delete_hashtable(hashtable_t *port_hash);
static u8 get_netshare_type_mode(USB_TETHER_NET_INFO *usb_share_net);
static boolean is_port_enaged(hashtable_t *hashtable, u16 port_to_use);
static PORT_CONVERT *get_port_convert_element(hashtable_t *hashtable,
                                              u16 port_converted);
static void set_usb_down_pkt_direct(USB_TETHER_NET_INFO *usb_share_net,
                                    NET_DOWN_PKT_DIRECT* down_pkt_direct);
static void set_ppp_down_pkt_direct(NET_DOWN_PKT_DIRECT* down_pkt_direct);
static void set_usb_down_direct_no_port(USB_TETHER_NET_INFO *usb_share_net,
                                        NET_DOWN_PKT_DIRECT* down_pkt_direct);
static void down_direct_no_port_unreused(WAN_NET *wan_net,
                                         NET_DOWN_PKT_DIRECT *down_pkt_direct);
static void down_direct_no_port_reused(WAN_NET *wan_net_ptr,
                                       NET_DOWN_PKT_DIRECT *down_pkt_direct);
static u32 down_direct_reused_with_port(u16 original_dst_port,
                                        WAN_NET *wan_net_ptr,
                                        NET_DOWN_PKT_DIRECT *down_pkt_direct);
static void down_direct_unreused_with_port(u16 original_dst_port,
            WAN_NET *wan_net, NET_DOWN_PKT_DIRECT *down_pkt_direct);
static void set_intern_down_pkt_direct(WAN_NET *wan_net,
                                       NET_DOWN_PKT_DIRECT *down_pkt_direct);
static boolean is_net_port_timer_used(u8 net_share_type);

/* Define external vars. */
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];
extern struct aic_network_ip_instance alte_ip;
extern PPP_DIAL_NET_INFO ppp_dial_net_info;
extern USB_TETHER_NET_INFO usb_tether_net_info_list[ANET_USB_NET_MAX_NUM];
extern aos_event_ptr event_net_share;

u32 aic_init_net_port_hashtable(u8 netshare_type, u8 share_mode)
{
    boolean is_inited = AIC_FALSE;

    if (netshare_type == ANET_NETSHARE_TYPE_PPP) {
        ppp_dial_hashtable = (hashtable_t *)aos_zalloc(sizeof(hashtable_t));
        AIC_ASSERT(ppp_dial_hashtable);
        is_inited = hashtable_init(ppp_dial_hashtable,
                                   ANET_PORT_HASH_SIZE,
                                   NULL, NULL,
                                   ppp_dial_hash_element_exist);

        if (is_inited) {
            ppp_dial_hash_mutex = aos_create_mutex("ppp_dial_hash_mutex");
            AIC_ASSERT(ppp_dial_hash_mutex);
            create_net_port_timer();
        }
    } else if (netshare_type == ANET_NETSHARE_TYPE_USB) {
        if (share_mode == ANET_SHARE_MODE_FORWARDING) {
            usb_net_hashtable =
                (hashtable_t *)aos_zalloc(sizeof(hashtable_t));
            AIC_ASSERT(usb_net_hashtable);
            is_inited = hashtable_init(usb_net_hashtable,
                                       ANET_PORT_HASH_SIZE,
                                       NULL, NULL,
                                       usb_net_hash_element_exist);

        } /*else if (share_mode == ANET_SHARE_MODE_ROUTING) {
            for (i = 0; i < ANET_USB_TETHER_MAX_CLIENTS; i++) {
                usbshare_tether_hashtable[i] =
                    (hashtable_t *)aos_zalloc(sizeof(hashtable_t));
                AIC_ASSERT(usbshare_tether_hashtable[i]);
                is_inited = hashtable_init(usbshare_tether_hashtable[i],
                                           ANET_PORT_HASH_SIZE,
                                           NULL, NULL,
                                           usb_net_hash_element_exist);

                if (is_inited) {
                    next_usb_tether_src_port[i] = ANET_USBSHARE_PORT_START +
                                                  usbshare_port_section * i;
                }
            }
        }*/

        if (is_inited) {
            usb_net_hash_mutex = aos_create_mutex("usb_net_hash_mutex");
            AIC_ASSERT(usb_net_hash_mutex);
            create_net_port_timer();
        }
    }

    if(!is_inited) {
        return ANET_ERROR_HASH_NOT_INIT;
    }

    return AIC_SUCCESS;
}

void create_net_port_timer(void)
{
    if (!aos_is_timer_created(net_port_timer)) {
        net_port_timer = aos_create_timer("net_port_timer",
                                          handle_net_port_timeout,
                                          AIC_NULL,
                                          ANET_PORT_TIMER_EXPIRE_TIME,
                                          AOS_AUTO_ACTIVATE,
                                          AOS_BE_PERIOD);
        AIC_ASSERT(net_port_timer);
    }
}

void aic_delete_port_hashtable(u8 netshare_type, u8 share_mode)
{
    if (netshare_type == ANET_NETSHARE_TYPE_PPP) {
        if (ppp_dial_hash_mutex) {
            aos_get_mutex(ppp_dial_hash_mutex, AOS_WAIT_FOREVER);
            delete_hashtable(ppp_dial_hashtable);
            ppp_dial_hashtable = AIC_NULL;
            aos_put_mutex(ppp_dial_hash_mutex);

            aos_delete_mutex(ppp_dial_hash_mutex);
            ppp_dial_hash_mutex = AIC_NULL;
        }

        if (aos_is_timer_created(net_port_timer) &&
            !is_net_port_timer_used(ANET_NETSHARE_TYPE_USB)) {
            aicx_info(0x01ee37, "", "ppp dial stopped,delete net port timer\n");
            aos_delete_timer(net_port_timer);
        }
    } else if (netshare_type == ANET_NETSHARE_TYPE_USB) {
        if (usb_net_hash_mutex) {
            aos_get_mutex(usb_net_hash_mutex, AOS_WAIT_FOREVER);
            if (share_mode == ANET_SHARE_MODE_FORWARDING) {
                delete_hashtable(usb_net_hashtable);
                usb_net_hashtable = AIC_NULL;
            }
            aos_put_mutex(usb_net_hash_mutex);

            aos_delete_mutex(usb_net_hash_mutex);
            usb_net_hash_mutex = AIC_NULL;
        }

        if (aos_is_timer_created(net_port_timer) &&
            !is_net_port_timer_used(ANET_NETSHARE_TYPE_PPP)) {
            aicx_info(0x1e35fc, "", "delete net port timer\n");
            aos_delete_timer(net_port_timer);
        }
    }
}

void delete_hashtable(hashtable_t *port_hash)
{
    if (port_hash) {
        /* Free all items. */
        hashtable_flush(port_hash, free_hash_item);
        /* Free memory for table. */
        aos_free((u8 *)port_hash);
    }
}

void free_hash_item(void *element)
{
    //alog_info("%s: free element 0x%x\n", __func__, element);
    aos_free((PORT_CONVERT *)element);
}

/* Check if the net_port_timer is still used by certain net share. */
boolean is_net_port_timer_used(u8 net_share_type)
{
    boolean is_used = AIC_FALSE;
    u8 i = 0;
    USB_TETHER_NET_INFO *usb_net_ptr;

    if (net_share_type == ANET_NETSHARE_TYPE_PPP) {
        if (ppp_dial_net_info.wan_net_id == ANET_DEFAULT_PDP_CID &&
            ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING)
            is_used = AIC_TRUE;
    } else if (net_share_type == ANET_NETSHARE_TYPE_USB) {
        for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
            usb_net_ptr = &usb_tether_net_info_list[i];
            if (usb_net_ptr->wan_net_id == ANET_DEFAULT_PDP_CID &&
                usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
                is_used = AIC_TRUE;
                break;
            }
        }
    }

    return is_used;
}

/* Remove the port convertions too old in the hashtable. */
void aic_remove_aging_ports(hashtable_t *hashtable_ptr)
{
    ulong current_time, elapsed_time;
    collection_item_t *current_element, *tmp;
    u16 i;

    current_time = aos_get_tickcount();

    for (i = 0; i < hashtable_ptr->ctrl.size; i++) {
        current_element = hashtable_ptr->table[i]; // get column head
        while (current_element != AIC_NULL) {
            tmp = current_element->next;
            elapsed_time = current_time -
                ((PORT_CONVERT *)current_element)->latest_sys_time;

            if (elapsed_time >= ANET_PORT_AGING_TIME) {
                hashtable_remove(hashtable_ptr, current_element,
                                 free_hash_item);
            }
            current_element = tmp;
        }
    }
}

void handle_net_port_timeout(void *input)
{
    aos_set_event(event_net_share, NET_EVENT_PORT_TIMEOUT, AOS_OR);
}

void ppp_dial_hash_element_exist(void *existing, void *to_insert)
{
    aicx_info(0x0a2fe7, "", "the ppp dial hash item already exist\n");

    //change the value to insert again?
}

/* The direct for upstream packets with no ports. */
u32 aic_ppp_dial_up_direct_no_port(NET_UP_PKT_DIRECT *ppp_up_direct)
{
    u8 wan_index;

    aos_memset((u8 *)ppp_up_direct, 0, sizeof(NET_UP_PKT_DIRECT));
    if (ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_ROUTING) {
        ppp_up_direct->to_nat = ANET_PKT_NEED_NAT;
        return AIC_SUCCESS;
    }

    wan_index = aic_get_wan_net_by_cid(ppp_dial_net_info.wan_net_id);
    if (wan_index != ANET_PDP_MAX_NUM) {
        ppp_up_direct->intf_index = wan_net_list[wan_index].wan_ifc_index;
    } else {
        aicx_error(0x047390, "d", "no wan net for ppp dial up direct %d\n",
                   ppp_dial_net_info.wan_net_id);
        return ANET_ERROR_INVALID_WAN_INDEX;
    }

    ppp_up_direct->port_converted = 0;
    ppp_up_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
    if (ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
        ppp_up_direct->to_nat = ANET_PKT_NO_NAT;
    }

    return AIC_SUCCESS;
}

/** The port convertion and upstream packet direct.
 * The transport layer source port range is 1025~10000 for ppp dial.
 */
u32 aic_ppp_dial_up_direct_with_port(u16 original_src_port,
                                 NET_UP_PKT_DIRECT *ppp_up_direct)
{
    u16 new_src_port = 0;
    PORT_CONVERT *element_found, *element_inserted;
    PORT_CONVERT *up_insert;
    u8 wan_index;

    aos_memset((u8 *)ppp_up_direct, 0, sizeof(NET_UP_PKT_DIRECT));
    if (ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_ROUTING) {
        ppp_up_direct->to_nat = ANET_PKT_NEED_NAT;
        ppp_up_direct->port_converted = original_src_port;
        return AIC_SUCCESS;
    }

    wan_index = aic_get_wan_net_by_cid(ppp_dial_net_info.wan_net_id);
    if (wan_index != ANET_PDP_MAX_NUM) {
        ppp_up_direct->intf_index = wan_net_list[wan_index].wan_ifc_index;
    } else {
        aicx_error(0x02c41b, "d", "no wan net for ppp dial up direct with port %d\n",
                   ppp_dial_net_info.wan_net_id);
        return ANET_ERROR_INVALID_WAN_INDEX;
    }

    /* Pdp not cid 1 and reserved ports needn't to be converted. */
    if (ppp_dial_net_info.wan_net_id != ANET_DEFAULT_PDP_CID ||
        original_src_port <= ANET_RESERVED_PORT_END) {
        new_src_port = original_src_port;
        goto NEXT_CONFIG;
    }

    if (!ppp_dial_hashtable ||
        !ppp_dial_hashtable->ctrl.initialized) {
        aicx_error(0x0256aa, "", "ppp hashtable is empty\n");
        return ANET_ERROR_HASH_NOT_INIT;
    }

    aos_get_mutex(ppp_dial_hash_mutex, AOS_WAIT_FOREVER);
    /* First,try to get from hashtable. */
    element_found = find_port_convert_element(ppp_dial_hashtable,
                                              original_src_port);

    /** If there is no item in the hash table,then assign a new port,
     * and add the item to the upstream and downstream hash table.
     */
    if (!element_found) {
        new_src_port = get_ppp_dial_idle_port();

        up_insert = (PORT_CONVERT *)aos_malloc(sizeof(PORT_CONVERT));
        AIC_ASSERT(up_insert);
        up_insert->previous_port = original_src_port;
        up_insert->converted_port = new_src_port;
        up_insert->latest_sys_time = aos_get_tickcount();
        element_inserted = hashtable_insert(ppp_dial_hashtable, up_insert,
                                            &up_insert->element_clue);

        if (!element_inserted) {
            aicx_error(0x10e7f5, "", "insert ppp dial hashtable error\n");
        }
    } else {
        /* Update the time when it is latest used. */
        element_found->latest_sys_time = aos_get_tickcount();
        new_src_port = element_found->converted_port;
    }
    aos_put_mutex(ppp_dial_hash_mutex);

NEXT_CONFIG:
    //alog_info("%s:original port=%d,new port=%d\n",
              //__func__, original_src_port, new_src_port);

    ppp_up_direct->port_converted = new_src_port;
    ppp_up_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
    if (ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
        if (new_src_port == ANET_DHCPV6_CLIENT_UDP_PORT) {
            ppp_up_direct->to_nat = ANET_PKT_NEED_NAT;
        } else {
            ppp_up_direct->to_nat = ANET_PKT_NO_NAT;
        }
    }

    return AIC_SUCCESS;
}

PORT_CONVERT *find_port_convert_element(hashtable_t *hashtable_ptr,
                                        u16 original_port)
{
    collection_item_t *current_element, *tmp;
    u32 i;

    if (hashtable_is_empty(hashtable_ptr)) {
        return AIC_NULL;
    }

    for (i = 0; i < hashtable_ptr->ctrl.size; i++) {
        current_element = hashtable_ptr->table[i]; // get column head
        while (current_element != AIC_NULL) {
            tmp = current_element->next;
            if (((PORT_CONVERT *)current_element)->previous_port == original_port) {
                return ((PORT_CONVERT *)current_element);
            }
            current_element = tmp;
        }
    }

    return AIC_NULL;
}

/* Find the idle port num to convert to for ppp dial. */
u16 get_ppp_dial_idle_port(void)
{
    //hashtable_t *ppp_hashtable = &ppp_dial_hashtable;
    u16 src_port_converted;

    while (is_port_enaged(ppp_dial_hashtable, next_ppp_dial_src_port)) {
        aicx_info(0x1d5bef, "d", "get ppp dial idle port,port %d is enaged\n",
                  next_ppp_dial_src_port);
        next_ppp_dial_src_port++;
        if (next_ppp_dial_src_port > ANET_PPP_DIAL_PORT_END) {
            next_ppp_dial_src_port = ANET_PPP_DIAL_PORT_START;
        }
    }

    src_port_converted = next_ppp_dial_src_port;
    next_ppp_dial_src_port++;
    if (next_ppp_dial_src_port > ANET_PPP_DIAL_PORT_END) {
        next_ppp_dial_src_port = ANET_PPP_DIAL_PORT_START;
    }

    return src_port_converted;
}

boolean is_port_enaged(hashtable_t *hashtable, u16 port_to_use)
{
    collection_item_t *current_element, *tmp;
    u32 i;

    if (!hashtable)
        return AIC_FALSE;

    for (i = 0; i < hashtable->ctrl.size; i++) {
        current_element = hashtable->table[i]; // get column head
        while (current_element != AIC_NULL) {
            tmp = current_element->next;
            if (((PORT_CONVERT *)current_element)->converted_port == port_to_use) {
                return AIC_TRUE;
            }
            current_element = tmp;
        }
    }

    return AIC_FALSE;
}

void usb_net_hash_element_exist(void *existing, void *to_insert)
{
    aicx_error(0x1ba357, "", "the usb net hash item already exist\n");

    //change the value to insert again?
}

/* The direct for upstream packets with no ports. */
u32 aic_usb_net_up_direct_no_port(ulong src_ipv4, u8 usb_intf_id,
                               NET_UP_PKT_DIRECT *up_direct)
{
    USB_TETHER_NET_INFO *usb_net_ptr;
    u8 usb_net_index, wan_index;

    aos_memset((u8 *)up_direct, 0, sizeof(NET_UP_PKT_DIRECT));

    usb_net_index = aic_get_usb_net_by_intf_id(usb_intf_id);
    if (usb_net_index == ANET_USB_NET_MAX_NUM) {
        return ANET_ERROR_NO_USB_NET;
    }
    usb_net_ptr = &usb_tether_net_info_list[usb_net_index];

    if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_ROUTING) {
        up_direct->to_nat = ANET_PKT_NEED_NAT;
        return AIC_SUCCESS;
    }

    wan_index = aic_get_wan_net_by_cid(usb_net_ptr->wan_net_id);
    if (wan_index != ANET_PDP_MAX_NUM) {
        up_direct->intf_index = wan_net_list[wan_index].wan_ifc_index;
    } else {
        aicx_error(0x054343, "d", "no wan net for usb net up direct with no port %d\n",
                   usb_net_ptr->wan_net_id);
        return ANET_ERROR_INVALID_WAN_INDEX;
    }

    up_direct->port_converted = 0;
    up_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
    if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
        up_direct->to_nat = ANET_PKT_NO_NAT;
    }

    return AIC_SUCCESS;
}

/** The port convertion and upstream packet direct.
 * The transport layer source port range is 10001~49151 for usb net share.
 */
u32 aic_usb_net_up_direct_with_port(u16 original_src_port, ulong src_ipv4,
                                 u8 usb_intf_id, NET_UP_PKT_DIRECT *up_direct)
{
    u16 new_src_port = 0;
    PORT_CONVERT *element_found = AIC_NULL;
    PORT_CONVERT *up_insert;
    USB_TETHER_NET_INFO *usb_net_ptr;
    u8 usb_net_index, wan_index;

    aos_memset((u8 *)up_direct, 0, sizeof(NET_UP_PKT_DIRECT));

    usb_net_index = aic_get_usb_net_by_intf_id(usb_intf_id);
    if (usb_net_index == ANET_USB_NET_MAX_NUM) {
        return ANET_ERROR_NO_USB_NET;
    }

    usb_net_ptr = &usb_tether_net_info_list[usb_net_index];
    if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_ROUTING) {
        up_direct->to_nat = ANET_PKT_NEED_NAT;
        return AIC_SUCCESS;
    }

    wan_index = aic_get_wan_net_by_cid(usb_net_ptr->wan_net_id);
    if (wan_index != ANET_PDP_MAX_NUM) {
        up_direct->intf_index = wan_net_list[wan_index].wan_ifc_index;
    } else {
        aicx_error(0x1285d2, "d", "no wan net for usb up direct with port %d\n",
                   usb_net_ptr->wan_net_id);
        return ANET_ERROR_INVALID_WAN_INDEX;
    }

    /* Pdp not cid 1 and reserved ports needn't to be converted. */
    if (usb_net_ptr->wan_net_id != ANET_DEFAULT_PDP_CID ||
        original_src_port <= ANET_RESERVED_PORT_END) {
        new_src_port = original_src_port;
        goto NEXT_CONFIG;
    }

#if 0
    if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_ROUTING) {
        /* Maybe the peer hasn't got ip addr. */
        if (src_ipv4 >= usb_net_ptr->lan_net_ptr->ipv4_addr_pool_start &&
            src_ipv4 <= usb_net_ptr->lan_net_ptr->ipv4_addr_pool_end) {
            hashtable_index = src_ipv4 -
                              usb_net_ptr->lan_net_ptr->ipv4_addr_pool_start;
            if (hashtable_index >= ANET_USB_TETHER_MAX_CLIENTS) {
                aicx_error(0x0d27dc, "s", "%s:too many clients\n", __func__);
                return ANET_ERROR_CLIENTS_OVERFLOW;
            }
        } else {
            new_src_port = original_src_port;
            goto NEXT_CONFIG;
        }
    }
#endif

    if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
        if (!usb_net_hashtable ||
            !usb_net_hashtable->ctrl.initialized) {
            aicx_error(0x108d03, "", "usb net hash table not init\n");
            return ANET_ERROR_HASH_NOT_INIT;
        }
    }

    aos_get_mutex(usb_net_hash_mutex, AOS_WAIT_FOREVER);
    /* First,try to get from hash table. */
    if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
        element_found = find_port_convert_element(usb_net_hashtable,
                                                  original_src_port);
    }

    /** If there is no item in the hash table,then assign a new port,
     * and add the item to the upstream and downstream hash tables.
     */
    if (!element_found) {
        /* Find the idle port num to convert to. */
        if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
            new_src_port = get_usb_net_idle_port();
        }

        up_insert = (PORT_CONVERT *)aos_malloc(sizeof(PORT_CONVERT));
        AIC_ASSERT(up_insert);
        up_insert->previous_port = original_src_port;
        up_insert->converted_port = new_src_port;
        up_insert->latest_sys_time = aos_get_tickcount();

        if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
            if (!hashtable_insert(usb_net_hashtable,
                                  up_insert, &up_insert->element_clue)) {
                aicx_error(0x0e1e6a, "", "insert usb net hashtable error\n");
            }
        }
    } else {
        /* Update the time when it is latest used. */
        element_found->latest_sys_time = aos_get_tickcount();
        new_src_port = element_found->converted_port;
    }
    aos_put_mutex(usb_net_hash_mutex);

NEXT_CONFIG:
        //alog_info("%s:original port=%d,new=%d\n", __func__,
                  //original_src_port, new_src_port);
        up_direct->port_converted = new_src_port;
        up_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
        if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
            if (new_src_port == ANET_DHCP_CLIENT_UDP_PORT ||
                new_src_port == ANET_DHCPV6_CLIENT_UDP_PORT) {
                //alog_info("%s:DHCP packet to netx\n", __func__);
                up_direct->to_nat = ANET_PKT_NEED_NAT;
            } else {
                up_direct->to_nat = ANET_PKT_NO_NAT;
            }
        } else {
            up_direct->to_nat = ANET_PKT_NEED_NAT;
        }

    return AIC_SUCCESS;
}

u16 get_usb_net_idle_port(void)
{
    u16 src_port_converted;

    while (is_port_enaged(usb_net_hashtable, next_usb_net_src_port)) {
        next_usb_net_src_port++;
        if (next_usb_net_src_port > ANET_USBSHARE_PORT_END) {
            next_usb_net_src_port = ANET_USBSHARE_PORT_START;
        }
    }

    src_port_converted = next_usb_net_src_port;
    next_usb_net_src_port++;
    if (next_usb_net_src_port > ANET_USBSHARE_PORT_END) {
        next_usb_net_src_port = ANET_USBSHARE_PORT_START;
    }

    return src_port_converted;
}

/* Get the way to direct to for downstream packets without ports. */
u32 aic_down_pkt_direct_no_port(u8 net_id, NET_DOWN_PKT_DIRECT *down_pkt_direct)
{
    u8 wan_index;
    WAN_NET *wan_net_ptr;
    //u32 status = AIC_SUCCESS;
    //AOS_INTERRUPT_DECLARE

    aos_memset((u8 *)down_pkt_direct, 0, sizeof(NET_DOWN_PKT_DIRECT));
    //AOS_INTERRUPT_DISABLE
    wan_index = aic_get_wan_net_by_cid(net_id);
    if (wan_index == ANET_PDP_MAX_NUM) {
        //AOS_INTERRUPT_RESTORE
        return ANET_ERROR_INVALID_WAN_INDEX;
    }

    wan_net_ptr = &wan_net_list[wan_index];

    if (net_id == ANET_DEFAULT_PDP_CID) {
        down_direct_no_port_reused(wan_net_ptr, down_pkt_direct);
    } else {
        down_direct_no_port_unreused(wan_net_ptr, down_pkt_direct);
    }
    //AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}

/** Get the way to direct to and the dst port to convert to
 * for downstream packets.
 */
u32 aic_down_pkt_direct_with_port(u16 original_dst_port, u8 net_id,
                              NET_DOWN_PKT_DIRECT *down_pkt_direct)
{
    u8 wan_index;
    WAN_NET *wan_net_ptr;
    u32 status = AIC_SUCCESS;
    //AOS_INTERRUPT_DECLARE

    aos_memset((u8 *)down_pkt_direct, 0, sizeof(NET_DOWN_PKT_DIRECT));
    //AOS_INTERRUPT_DISABLE
    wan_index = aic_get_wan_net_by_cid(net_id);
    if (wan_index == ANET_PDP_MAX_NUM) {
        //AOS_INTERRUPT_RESTORE
        return ANET_ERROR_INVALID_WAN_INDEX;
    }

    wan_net_ptr = &wan_net_list[wan_index];

    if (net_id == ANET_DEFAULT_PDP_CID) {
        status = down_direct_reused_with_port(original_dst_port,
                                              wan_net_ptr,
                                              down_pkt_direct);
    } else {
        down_direct_unreused_with_port(original_dst_port, wan_net_ptr,
                                       down_pkt_direct);
    }
    //AOS_INTERRUPT_RESTORE

    /*alog_info("%s:original port=%d,to_nat=%d, intf index=%d,"
              "IP pointer=0x%x\n", __func__,
              original_dst_port, down_pkt_direct->to_nat,
              down_pkt_direct->intf_index,
              down_pkt_direct->ip_ptr);*/

    return status;
}

void set_usb_down_direct_no_port(USB_TETHER_NET_INFO *usb_net,
                                 NET_DOWN_PKT_DIRECT* down_pkt_direct)
{
    down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
    down_pkt_direct->intf_index = usb_net->lan_intf_index;

    if (usb_net->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
        down_pkt_direct->to_nat = ANET_PKT_NO_NAT;
    } else {
        down_pkt_direct->to_nat = ANET_PKT_NEED_NAT;
    }
}

/* Direct downstream packets with no ports for PDP non reused. */
void down_direct_no_port_unreused(WAN_NET *wan_net,
                                  NET_DOWN_PKT_DIRECT *down_pkt_direct)
{
    u8 usb_net_index;

    usb_net_index = aic_get_active_usb_net_by_cid(wan_net->wan_net_id);
    if (usb_net_index != ANET_USB_NET_MAX_NUM) {
        set_usb_down_direct_no_port(&usb_tether_net_info_list[usb_net_index],
                                    down_pkt_direct);
    } else if (wan_net->wan_net_id == ppp_dial_net_info.wan_net_id) {
        /* Check if ppp dial use the net id.*/
        set_ppp_down_pkt_direct(down_pkt_direct);
    } else {
        set_intern_down_pkt_direct(wan_net, down_pkt_direct);
    }
}

u8 get_netshare_type_mode(USB_TETHER_NET_INFO *usb_net_info)
{
    u8 net_type_mode = 0;

    if (ppp_dial_net_info.wan_net_id == ANET_DEFAULT_PDP_CID) {
        if (ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
            net_type_mode |= ANET_PPP_DIAL_FORWARDING;
        } else if (ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_ROUTING) {
            net_type_mode |= ANET_PPP_DIAL_ROUTING;
        }
    }

    if (usb_net_info) {
        if (usb_net_info->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
            net_type_mode |= ANET_USBSHARE_FORWARDING;
        } else if (usb_net_info->usb_tether_mode == ANET_SHARE_MODE_ROUTING) {
            net_type_mode |= ANET_USBSHARE_ROUTING;
        }
    }

    return net_type_mode;
}

/* Direct downstream packets without ports for PDP reused(default PDP). */
void down_direct_no_port_reused(WAN_NET *wan_net_ptr,
                                NET_DOWN_PKT_DIRECT *down_pkt_direct)
{
    USB_TETHER_NET_INFO *usb_net_info = AIC_NULL;
    u8 usb_net_index, net_type_mode;

    usb_net_index = aic_get_active_usb_net_by_cid(wan_net_ptr->wan_net_id);
    if (usb_net_index != ANET_USB_NET_MAX_NUM) {
        usb_net_info = &usb_tether_net_info_list[usb_net_index];
    }
    net_type_mode = get_netshare_type_mode(usb_net_info);

    /* Direct to usb share,ppp dial and internal sockets. */
    if (net_type_mode & ANET_PPP_DIAL_FORWARDING) {
        //down_pkt_direct->ip_ptr = (NX_IP *)ppp_dial_lan_ip.ip_ptr;
        down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
        down_pkt_direct->intf_index = ppp_dial_net_info.lan_intf_index;
        down_pkt_direct->to_nat = ANET_PKT_MULTI_HOME;
    } else if (net_type_mode & ANET_USBSHARE_FORWARDING) {
        //down_pkt_direct->ip_ptr = (NX_IP *)usb_lan_ip.ip_ptr;
        down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
        down_pkt_direct->intf_index = usb_net_info->lan_intf_index;
        down_pkt_direct->to_nat = ANET_PKT_MULTI_HOME;
    } else {
        down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
        down_pkt_direct->intf_index = wan_net_ptr->wan_ifc_index;
        /** If there is tethering net share or no net share,then packets
         * should be directed to NETX.
         */
        down_pkt_direct->to_nat = ANET_PKT_NEED_NAT;
    }
}

u32 aic_get_usb_net_dst_port(u16 original_dst_port, u8 usb_intf_id, u16 *actual_port)
{
    PORT_CONVERT *element_found = AIC_NULL;
    u8 usb_net_index;
    USB_TETHER_NET_INFO *usb_net_info;

    usb_net_index = aic_get_usb_net_by_intf_id(usb_intf_id);
    if (usb_net_index == ANET_USB_NET_MAX_NUM) {
        return ANET_ERROR_NO_USB_NET;
    }

    usb_net_info = &usb_tether_net_info_list[usb_net_index];
    /* If net id is not 1,then not convert dst port. */
    *actual_port = original_dst_port;
    if (usb_net_info->wan_net_id == ANET_DEFAULT_PDP_CID &&
        (original_dst_port > ANET_RESERVED_PORT_END)) {
        /* Try to get from usb net share hashtable. */
        if (usb_net_info->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
            aos_get_mutex(usb_net_hash_mutex, AOS_WAIT_FOREVER);
            if (!hashtable_is_empty(usb_net_hashtable))
                element_found = get_port_convert_element(
                                usb_net_hashtable,
                                original_dst_port);
            aos_put_mutex(usb_net_hash_mutex);
            if (element_found) {
                *actual_port = element_found->previous_port;
            }
        }
    }

    //alog_info("%s:net id=%d, original port=%d, converted port=%d\n",
              //__func__, net_id, original_dst_port, *actual_port);
    return AIC_SUCCESS;
}

u32 aic_get_ppp_dial_dst_port(u16 original_dst_port, u16 *actual_port)
{
    PORT_CONVERT *element_found = AIC_NULL;

    if (ppp_dial_net_info.dial_started != ANET_PPP_DIAL_STARTED) {
        aicx_error(0x1660dd, "", "get dst port:ppp dial not started\n");
        return ANET_ERROR_NOT_MATCH_PPP_DIAL;
    }

    *actual_port = original_dst_port;
    if (ppp_dial_net_info.wan_net_id == ANET_DEFAULT_PDP_CID &&
        (original_dst_port > ANET_RESERVED_PORT_END) &&
        ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
        if (ppp_dial_hash_mutex) {
            aos_get_mutex(ppp_dial_hash_mutex, AOS_WAIT_FOREVER);
            /* Try to get from ppp dial hashtable. */
            if (!hashtable_is_empty(ppp_dial_hashtable)) {
                element_found = get_port_convert_element(ppp_dial_hashtable,
                                                         original_dst_port);
                if (element_found) {
                    *actual_port = element_found->previous_port;
                }
            }
            aos_put_mutex(ppp_dial_hash_mutex);
        }
    }

    //alog_info("%s:net id=%d, original port=%d, converted port=%d\n",
              //__func__, net_id, original_dst_port, *actual_port);
    return AIC_SUCCESS;
}

u32 down_direct_reused_with_port(u16 original_dst_port, WAN_NET *wan_net_ptr,
                                 NET_DOWN_PKT_DIRECT *down_pkt_direct)
{
    u8 usb_net_index, net_type_mode;
    USB_TETHER_NET_INFO *usb_net_info = AIC_NULL;

    usb_net_index = aic_get_active_usb_net_by_cid(ANET_DEFAULT_PDP_CID);
    if (usb_net_index != ANET_USB_NET_MAX_NUM) {
        usb_net_info = &usb_tether_net_info_list[usb_net_index];
    }
    net_type_mode = get_netshare_type_mode(usb_net_info);

    /* Resreved port. */
    if (original_dst_port <= ANET_RESERVED_PORT_END) {
        //down_pkt_direct_reserved_port(usb_net_index, original_dst_port,
                                      //wan_net_ptr, down_pkt_direct);
        //down_pkt_direct->port_converted = original_dst_port;
        down_direct_no_port_reused(wan_net_ptr, down_pkt_direct);
        return AIC_SUCCESS;
        /* The packet is to internal socket. */
    } else if (original_dst_port >= ANET_INTERNAL_PORT_START &&
               original_dst_port <= ANET_INTERNAL_PORT_END) {
        set_intern_down_pkt_direct(wan_net_ptr, down_pkt_direct);
    } else if (original_dst_port >= ANET_PPP_DIAL_PORT_START &&
             original_dst_port <= ANET_PPP_DIAL_PORT_END) {
        if (ppp_dial_net_info.wan_net_id != wan_net_ptr->wan_net_id) {
            aicx_error(0x1e4029, "", "not match ppp dial\n");
            return ANET_ERROR_PKT_NO_DIRECT;
        }

        //set_ppp_down_pkt_direct(down_pkt_direct);
        down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
        down_pkt_direct->intf_index = ppp_dial_net_info.lan_intf_index;
        if (ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
            down_pkt_direct->to_nat = ANET_PKT_NO_NAT;
        }
    } else if (original_dst_port >= ANET_USBSHARE_PORT_START &&
               original_dst_port <= ANET_USBSHARE_PORT_END) {
        if (usb_net_index == ANET_USB_NET_MAX_NUM) {
            aicx_error(0x067530, "", "not match usb net\n");
            return ANET_ERROR_PKT_NO_DIRECT;
        }

        usb_net_info = &usb_tether_net_info_list[usb_net_index];
        //set_usb_down_pkt_direct(usb_share_net, down_pkt_direct);
        down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
        down_pkt_direct->intf_index = usb_net_info->lan_intf_index;
        if (usb_net_info->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
            down_pkt_direct->to_nat = ANET_PKT_NO_NAT;
        }
    } else if (original_dst_port >= NX_NAT_START_TCP_PORT &&
               original_dst_port <= NX_NAT_END_TCP_PORT) {
        if (!(net_type_mode & ANET_PPP_DIAL_ROUTING) &&
            !(net_type_mode & ANET_USBSHARE_ROUTING)) {
            aicx_error(0x0f0c44, "", "error,no net share of routing mode\n");
            return ANET_ERROR_PKT_NO_DIRECT;
        }

        down_pkt_direct->to_nat = ANET_PKT_NEED_NAT;
    }

    return AIC_SUCCESS;
}

/* Direct downstream packets of PDP non reused. */
void down_direct_unreused_with_port(u16 original_dst_port, WAN_NET *wan_net,
                                    NET_DOWN_PKT_DIRECT *down_pkt_direct)
{
    u8 usb_net_index;

    //down_pkt_direct->port_converted = original_dst_port;
    usb_net_index = aic_get_active_usb_net_by_cid(wan_net->wan_net_id);
    if (usb_net_index != ANET_USB_NET_MAX_NUM) {
        set_usb_down_pkt_direct(&usb_tether_net_info_list[usb_net_index],
                                down_pkt_direct);
    } else if (wan_net->wan_net_id == ppp_dial_net_info.wan_net_id) {
        /* Check if ppp dial use the net id.*/
        set_ppp_down_pkt_direct(down_pkt_direct);
    } else {
        set_intern_down_pkt_direct(wan_net, down_pkt_direct);
    }
}

void set_usb_down_pkt_direct(USB_TETHER_NET_INFO *usb_net_info,
                             NET_DOWN_PKT_DIRECT* down_pkt_direct)
{
    down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
    down_pkt_direct->intf_index = usb_net_info->lan_intf_index;

    if (usb_net_info->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
        down_pkt_direct->to_nat = ANET_PKT_NO_NAT;
    } else {
        down_pkt_direct->to_nat = ANET_PKT_NEED_NAT;
    }
}

void set_ppp_down_pkt_direct(NET_DOWN_PKT_DIRECT *down_pkt_direct)
{
    down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
    down_pkt_direct->intf_index = ppp_dial_net_info.lan_intf_index;

    if (ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
        down_pkt_direct->to_nat = ANET_PKT_NO_NAT;
    } else if(ppp_dial_net_info.ppp_dial_mode ==
              ANET_SHARE_MODE_ROUTING) {
        down_pkt_direct->to_nat = ANET_PKT_NEED_NAT;
    }
}

void set_intern_down_pkt_direct(WAN_NET *wan_net,
                                NET_DOWN_PKT_DIRECT *down_pkt_direct)
{
    down_pkt_direct->ip_ptr = (NX_IP *)alte_ip.ip_ptr;
    down_pkt_direct->intf_index = wan_net->wan_ifc_index;
    down_pkt_direct->to_nat = ANET_PKT_NEED_NAT;
}

/* Get the port convert element via the port converted. */
PORT_CONVERT *get_port_convert_element(hashtable_t *hashtable,
                                       u16 port_converted)
{
    collection_item_t *current_element, *tmp;
    u32 i;

    for (i = 0; i < hashtable->ctrl.size; i++) {
        current_element = hashtable->table[i];
        while (current_element != AIC_NULL) {
            tmp = current_element->next;
            if (((PORT_CONVERT *)current_element)->converted_port ==
                  port_converted) {
                return (PORT_CONVERT *)current_element;
            }
            current_element = tmp;
        }
    }

    return AIC_NULL;
}

/** The input interface index is wan interf,then return lan interf index of
 * related ndis network share. If the input interf index is the lan interf
 * index of the forwarding network share,then return the wan interf index.
 */
u32 aic_get_forwarding_matched_interf(u32 interf_index)
{
    u8 matched_intf_index = NX_MAX_PHYSICAL_INTERFACES;
    WAN_NET *wan_net_ptr;
    USB_TETHER_NET_INFO *usb_net_info;

    wan_net_ptr = aic_get_wan_net_by_ifc_index(interf_index);
    if (wan_net_ptr) {
        if (ppp_dial_net_info.wan_interf_index == interf_index &&
            ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING)
            return ppp_dial_net_info.lan_intf_index;

        usb_net_info = aic_get_usb_net_by_wan_ifc(interf_index);
        if (usb_net_info && usb_net_info->usb_tether_mode == ANET_SHARE_MODE_FORWARDING)
            return usb_net_info->lan_intf_index;
    } else {
        if (ppp_dial_net_info.lan_intf_index == interf_index &&
            ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING)
            return ppp_dial_net_info.wan_interf_index;

        usb_net_info = aic_get_usb_net_by_lan_ifc(interf_index);
        if (usb_net_info && usb_net_info->usb_tether_mode == ANET_SHARE_MODE_FORWARDING)
            return usb_net_info->wan_intf_index;
    }

    return matched_intf_index;
}

