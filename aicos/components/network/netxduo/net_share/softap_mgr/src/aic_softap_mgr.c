/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Softap Management Component.
 *  This is the management of softap. Config the softap interface,
 *  start DHCP Server,process several clients connecting and disconnecting etc.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x116721
#define LOG_MODULE    AMOD_NET

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <nx_api.h>
#include <nxd_bsd.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_api.h>
#include <aic_ipv6_forward.h>
#include "aic_at_cmd_handle.h"
#include "aic_general_at_handle.h"
#include "aic_net_share_common.h"
#include "aic_at_parse_utils.h"
#include "aic_usb_share_at_handle.h"
#include "aic_usb_net_share_mgr.h"
#include "aic_softap_mgr.h"

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
#include "aic_net_share_port_mgr.h"
#endif

#ifdef CONFIG_NET_ALTE_DRIVER
#include <nx_alte_driver.h>
#endif

#ifdef CONFIG_NX_NAT
#include <nx_nat.h>
#endif

#ifdef CONFIG_NX_DHCP
#include "nxd_dhcp_server.h"
#endif

#ifdef CONFIG_PPP_DIAL
#include "aic_ppp_dial_mgr.h"
#endif

aos_mutex_ptr softap_net_mutex;
SOFTAP_NET_INFO softap_net_info;

/* Define external vars. */
extern struct aic_network_ip_instance alte_ip;
#ifdef CONFIG_NX_NAT
extern NX_NAT_DEVICE *nat_dev_list[NX_MAX_PHYSICAL_INTERFACES];
#endif
#ifdef CONFIG_NX_DHCP
extern NX_DHCP_SERVER *dhcp_server;
extern u8 *server_dhcp_stack;
extern u8 dhcp_server_name[20];
extern aos_mutex_ptr dhcp_server_mutex;
#endif

static void reset_softap_net_info(void)
{
    aos_memset((u8 *)&softap_net_info, 0, sizeof(SOFTAP_NET_INFO));
    softap_net_info.lan_intf_index = ANET_INVALID_INTF_INDEX;
    softap_net_info.wan_intf_index = ANET_INVALID_INTF_INDEX;
}

void aic_softap_init(void)
{
    softap_net_mutex = aos_create_mutex("softap_net_mutex");
    AOS_ASSERT(softap_net_mutex);

    reset_softap_net_info();
}

static SOFTAP_CLIENT_INFO *get_idle_softap_client_info(void)
{
    u8 i;
    SOFTAP_LAN_NET *softap_lan_net;

    softap_lan_net = softap_net_info.lan_net_ptr;
    if (!softap_lan_net)
        return AIC_NULL;

    for (i = 0; i < ANET_SOFTAP_CLIENTS_NUM; i++) {
        if (softap_lan_net->clients_info[i].ipv4_addr == 0)
            return &softap_lan_net->clients_info[i];
    }

    return AIC_NULL;
}

static SOFTAP_CLIENT_INFO *get_softap_client_info_by_ip(ulong ip_addr)
{
    u8 i;
    SOFTAP_LAN_NET *softap_lan_net;

    softap_lan_net = softap_net_info.lan_net_ptr;
    if (!softap_lan_net)
        return AIC_NULL;

    for (i = 0; i < ANET_SOFTAP_CLIENTS_NUM; i++) {
        if (softap_lan_net->clients_info[i].ipv4_addr == ip_addr)
            return &softap_lan_net->clients_info[i];
    }

    return AIC_NULL;
}

#ifdef CONFIG_NX_NAT
static void softap_nat_cache_full_callback(NX_NAT_DEVICE *nat_ptr)
{
    aicx_info(0x0eee40, "d", "softap nat cache full, interf index %d\n",
              nat_ptr->nx_nat_global_interface_index);
}

static u32 create_nat_entry_for_softap_client(SOFTAP_CLIENT_INFO *client_ptr, u8 wan_intf_index)
{
    u32 ret = AIC_SUCCESS;
    ulong peer_addr;

    if (!aic_is_valid_intf_index(wan_intf_index) || !client_ptr)
        return ANET_ERROR_INVALID_WAN;

    peer_addr = client_ptr->ipv4_addr;
    ret = nx_nat_inbound_entry_create(wan_intf_index,
                                      &client_ptr->icmp_nat_entry,
                                      peer_addr,
                                      0, 0, NX_PROTOCOL_ICMP);
    if (ret)
        aicx_error(0x100e31, "x", "create softap ICMP entry error 0x%x\n", ret);

    ret = nx_nat_inbound_entry_create(wan_intf_index,
                                      &client_ptr->tcp_nat_entry,
                                      peer_addr,
                                      0, 0, NX_PROTOCOL_TCP);
    if (ret)
        aicx_error(0x086524, "x", "create softap TCP entry error 0x%x\n", ret);

    ret = nx_nat_inbound_entry_create(wan_intf_index,
                                      &client_ptr->udp_nat_entry,
                                      peer_addr,
                                      0, 0, NX_PROTOCOL_UDP);
    if (ret)
        aicx_error(0x1b9db1, "x", "create softap UDP entry error 0x%x\n", ret);

    if (!ret)
        client_ptr->nat_entry_created = 1;

    return ret;
}

static u32 create_nat_entry_for_all_clients(u8 wan_intf_index)
{
    u8 i;
    SOFTAP_LAN_NET *softap_lan_net;
    SOFTAP_CLIENT_INFO *client_ptr;

    softap_lan_net = softap_net_info.lan_net_ptr;
    if (!softap_lan_net || softap_net_info.wan_intf_index != wan_intf_index)
        return ANET_ERROR_INTERNAL_ERROR;

    for (i = 0; i < ANET_SOFTAP_CLIENTS_NUM; i++) {
        client_ptr = &softap_lan_net->clients_info[i];
        if (client_ptr->ipv4_addr != 0 && !client_ptr->nat_entry_created)
            create_nat_entry_for_softap_client(client_ptr, wan_intf_index);
    }

    return AIC_SUCCESS;
}

static u32 config_softap_intf_nat(u8 wan_intf_index)
{
    u32 ret = AIC_SUCCESS;
    NX_NAT_DEVICE *nat_dev_ptr;

    if (!aic_is_valid_intf_index(wan_intf_index))
        return ANET_ERROR_INVALID_WAN;

    nat_dev_ptr = nat_dev_list[wan_intf_index];
    if (nat_dev_ptr && nat_dev_ptr->nx_nat_id == NX_NAT_ID)
        return ret;

    if (!nat_dev_list[wan_intf_index]) {
        nat_dev_list[wan_intf_index] = (NX_NAT_DEVICE *)aos_zalloc(sizeof(NX_NAT_DEVICE));
        AOS_ASSERT(nat_dev_list[wan_intf_index]);
    }

    nat_dev_ptr = nat_dev_list[wan_intf_index];
    if (nat_dev_ptr->nx_nat_id != NX_NAT_ID) {
        alte_ip.nat_cache_mem_size = SIZE_8K + SIZE_4K;
        ret = aic_nat_create(nat_dev_ptr, &alte_ip, wan_intf_index);
        if (ret)
            return ret;

        nx_nat_cache_notify_set(wan_intf_index, softap_nat_cache_full_callback);
        nx_nat_enable(wan_intf_index);
    }

    return ret;
}

static void delete_softap_client_nat(SOFTAP_CLIENT_INFO *client_info_ptr)
{
    u32 wan_intf_index;

    if (!client_info_ptr || !client_info_ptr->nat_entry_created)
        return;

    wan_intf_index = softap_net_info.wan_intf_index;
    if (!aic_is_valid_intf_index(wan_intf_index))
        return;

    nx_nat_inbound_entry_delete(wan_intf_index, &client_info_ptr->icmp_nat_entry);
    nx_nat_inbound_entry_delete(wan_intf_index, &client_info_ptr->tcp_nat_entry);
    nx_nat_inbound_entry_delete(wan_intf_index, &client_info_ptr->udp_nat_entry);
    client_info_ptr->nat_entry_created = ANET_NAT_ENTRY_NOT_CREATED;
}

static void delete_all_softap_client_nat(void)
{
    u8 i;
    SOFTAP_LAN_NET *lan_net = AIC_NULL;
    SOFTAP_CLIENT_INFO *client_info;
    NX_NAT_DEVICE *nat_dev_ptr;
    u32 wan_intf_index;

    wan_intf_index = softap_net_info.wan_intf_index;
    lan_net = softap_net_info.lan_net_ptr;
    if (!aic_is_valid_intf_index(wan_intf_index) || !lan_net)
        return;

    for (i = 0; i < ANET_SOFTAP_CLIENTS_NUM; i++) {
        client_info = &lan_net->clients_info[i];
        delete_softap_client_nat(client_info);
    }

    if (!aic_ppp_dial_use_same_nat(wan_intf_index) && !aic_usb_net_use_same_nat(wan_intf_index)) {
        nat_dev_ptr = nat_dev_list[wan_intf_index];
        if (nat_dev_ptr) {
            aic_nat_delete(nat_dev_ptr, &alte_ip);
            aos_free(nat_dev_ptr);
            nat_dev_list[wan_intf_index] = AIC_NULL;
        }
    }
}

#endif

static u32 config_dhcp_server_for_softap(SOFTAP_LAN_NET *lan_net_ptr)
{
    u32 status = AIC_SUCCESS, addresses_added;

    if (!dhcp_server)
        return ANET_ERROR_INTERNAL_ERROR;

    if (!lan_net_ptr)
        return ANET_ERROR_LAN_NET_NOT_INIT;

    /* Load the assignable DHCP IP addresses for the interface. */
    status = nx_dhcp_create_server_ip_address_list(
             dhcp_server,
             softap_net_info.lan_intf_index,
             lan_net_ptr->ipv4_addr_pool_start,
             lan_net_ptr->ipv4_addr_pool_end,
             &addresses_added);

    if (status) {
        aicx_error(0x015868, "x", "config dhcp ip addr list error 0x%x\n", status);
        return status;
    }

    /* Set the interface network parameters. */
    status = nx_dhcp_set_interface_network_parameters(
             dhcp_server,
             softap_net_info.lan_intf_index,
             lan_net_ptr->lan_ipv4_subnet_mask,
             lan_net_ptr->lan_ipv4_address,
             lan_net_ptr->lan_ipv4_dns1_address);
    if (status)
        aicx_error(0x0f89a5, "x", "config dhcp intf para error 0x%x\n", status);

    return status;
}

static u32 config_and_start_dhcp_for_softap(SOFTAP_LAN_NET *lan_net)
{
    u32 status;

    if (!lan_net)
        return ANET_ERROR_LAN_NET_NOT_INIT;

    status = config_dhcp_server_for_softap(lan_net);
    if (status) {
        nx_dhcp_server_delete(dhcp_server);
        aicx_error(0x148cc3, "x", "softap config dhcp error 0x%x\n", status);
        goto CONFIG_ERROR;
    }

    status = nx_dhcp_server_start(dhcp_server);
    if (status) {
        nx_dhcp_server_delete(dhcp_server);
        aicx_error(0x09ee49, "x", "softap start DHCP server error 0x%x\n", status);
        goto CONFIG_ERROR;
    }

    return AIC_SUCCESS;

CONFIG_ERROR:
    aos_free(dhcp_server);
    dhcp_server = AIC_NULL;
    aos_free(server_dhcp_stack);
    server_dhcp_stack = AIC_NULL;

    return status;
}

static boolean is_in_same_sub_net(SOFTAP_LAN_NET *softap_lan_net, ulong client_ip)
{
    if (!softap_lan_net)
        return AIC_FALSE;

    if ((softap_lan_net->lan_ipv4_address & softap_lan_net->lan_ipv4_subnet_mask) ==
        (client_ip & softap_lan_net->lan_ipv4_subnet_mask))
        return AIC_TRUE;

    return AIC_FALSE;
}

/** Be informed when new client is connected. */
static void new_dhcp_client_added_cb(ulong client_ip)
{
    u8 wan_intf_index;
    u32 ret = AIC_SUCCESS;
    SOFTAP_CLIENT_INFO *client_ptr = AIC_NULL;
    WAN_NET *wan_net;

    if (!aic_is_valid_intf_index(softap_net_info.lan_intf_index) ||
        !is_in_same_sub_net(softap_net_info.lan_net_ptr, client_ip))
        return;

    wan_net = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
    if (wan_net) {
        softap_net_info.wan_net_ptr = wan_net;
        softap_net_info.wan_intf_index = wan_net->wan_ifc_index;
    }

    aos_get_mutex(softap_net_mutex, AOS_WAIT_FOREVER);
    wan_intf_index = softap_net_info.wan_intf_index;
#ifdef CONFIG_NX_NAT
    ret = config_softap_intf_nat(wan_intf_index);
    if (!ret) {
        client_ptr = get_softap_client_info_by_ip(client_ip);
        if (client_ptr && client_ptr->nat_entry_created)
            return;

        if (!client_ptr) {
            client_ptr = get_idle_softap_client_info();
            if (!client_ptr) {
                aicx_error(0x05bad6, "x", "error,no idle client info for 0x%8x\n", client_ip);
                return;
            }
        }

        aicx_info(0x03b9a7, "x", "create nat entry for new client 0x%8x\n", client_ip);
        client_ptr->ipv4_addr = client_ip;
        create_nat_entry_for_softap_client(client_ptr, wan_intf_index);
    }
#endif

    aos_put_mutex(softap_net_mutex);
}

static void dhcp_client_discon_via_probe(NX_INTERFACE *nx_interface, ULONG ip_address)
{
    SOFTAP_CLIENT_INFO *client_info_ptr;

    if (!nx_interface || nx_interface->nx_interface_index != softap_net_info.lan_intf_index)
        return;

    aos_get_mutex(softap_net_mutex, AOS_WAIT_FOREVER);
    aicx_info(0x0e7972, "x", "probe result:client of ip 0x%8x disconnect\n", ip_address);
    client_info_ptr = get_softap_client_info_by_ip(ip_address);
    if (client_info_ptr) {
        delete_softap_client_nat(client_info_ptr);
        client_info_ptr->ipv4_addr = 0;
    }

    aos_put_mutex(softap_net_mutex);
    aic_clear_dhcp_client_by_ip(dhcp_server, ip_address);
}

void aic_handle_client_disassoc(ULONG ip_addr)
{
    SOFTAP_CLIENT_INFO *client_info_ptr;
    u32 ret;

    aos_get_mutex(softap_net_mutex, AOS_WAIT_FOREVER);
    client_info_ptr = get_softap_client_info_by_ip(ip_addr);
    if (client_info_ptr) {
        aicx_info(0x185b9f, "x", "client of ip 0x%8x disassoc\n", ip_addr);
        delete_softap_client_nat(client_info_ptr);
        client_info_ptr->ipv4_addr = 0;
    }

    aos_put_mutex(softap_net_mutex);
    aic_clear_dhcp_client_by_ip(dhcp_server, ip_addr);

    ret = nx_arp_entry_delete(alte_ip.ip_ptr, ip_addr);
    if (ret)
        aicx_error(0x05691e, "x", "delete ARP entry error 0x%x\n", ret);
}

static u32 start_dhcp_server_for_softap(u8 softap_intf_index)
{
    u32 ret = AIC_SUCCESS;

    if (!dhcp_server_mutex) {
        dhcp_server_mutex = aos_create_mutex("dhcp_server_mutex");
        AOS_ASSERT(dhcp_server_mutex);
    }

    aos_get_mutex(dhcp_server_mutex, AOS_WAIT_FOREVER);
    if (dhcp_server && dhcp_server->nx_dhcp_started) {
        ret = config_dhcp_server_for_softap(softap_net_info.lan_net_ptr);
    } else {
        ret = aic_init_dhcp_server();
        if (!ret)
            ret = config_and_start_dhcp_for_softap(softap_net_info.lan_net_ptr);
    }

    aos_put_mutex(dhcp_server_mutex);
    if (ret)
        aicx_error(0x1672ea, "x", "config dhcpserver for softap error 0x%x\n", ret);
    else {
        softap_net_info.dhcp_configured = ANET_DHCP_CONFIGURED;
        aic_register_dhcp_client_added(dhcp_server, new_dhcp_client_added_cb);
    }

    return ret;
}

static u32 attach_softap_lan_interface(VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *))
{
    u32 status;
    SOFTAP_LAN_NET *lan_net;

    if (softap_net_info.lan_intf_index < NX_MAX_PHYSICAL_INTERFACES)
        return AIC_SUCCESS;

    lan_net = softap_net_info.lan_net_ptr;
    if (!lan_net)
        return ANET_ERROR_INVALID_IFC_INDEX;

    status = nx_ip_interface_attach((NX_IP *)alte_ip.ip_ptr,
                                    softap_net_info.softap_interfce,
                                    lan_net->lan_ipv4_address,
                                    lan_net->lan_ipv4_subnet_mask,
                                    ip_link_driver);

    if (status) {
        aicx_error(0x166de8, "x", "softap lan interf attach error--0x%x\n", status);
        return status;
    }

    /* Get interface index via interface name set above. */
    softap_net_info.lan_intf_index = aic_get_lan_ifc_index(softap_net_info.softap_interfce);
    if (softap_net_info.lan_intf_index == NX_MAX_PHYSICAL_INTERFACES) {
       status = ANET_ERROR_INVALID_IFC_INDEX;
       return status;
    }

    if (!alte_ip.arp_cache_mem) {
        alte_ip.arp_cache_mem_size = SIZE_1K;
        status = aic_enable_arp(&alte_ip);
        if (status) {
            if (status == NX_ALREADY_ENABLED) {
                status = AIC_SUCCESS;
            } else {
                aicx_error(0x02e610, "x", "enable arp error for softap 0x%x\n", status);
            }
        }
    }

    status = nx_ip_interface_mtu_set((NX_IP *)alte_ip.ip_ptr,
                                     softap_net_info.lan_intf_index,
                                     ANET_SOFTAP_LAN_MTU_SIZE);

    return status;
}

static s32 create_softap_lan_net(void)
{
    SOFTAP_LAN_NET *softap_lan_net;

    softap_net_info.lan_net_ptr = aos_zalloc(sizeof(SOFTAP_LAN_NET));
    softap_lan_net = softap_net_info.lan_net_ptr;

    if (softap_lan_net) {
        softap_lan_net->global_ipv6_addr_index = ANET_INVALID_INTF_INDEX;
        softap_lan_net->local_ipv6_addr_index = ANET_INVALID_INTF_INDEX;
    } else {
        aicx_error(0x049889, "", "error,malloc softap lan net fail\n");
        return AIC_ENOMEM;
    }

    return AIC_SUCCESS;
}

static u32 config_softap_lan_net_ipv4(VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *))
{
    u32 ret = AIC_SUCCESS;
    WAN_NET *wan_net = AIC_NULL;
    SOFTAP_LAN_NET *softap_lan_net;

    if (!softap_net_info.lan_net_ptr) {
        if (create_softap_lan_net())
            return ANET_ERROR_LAN_NET_NOT_INIT;
    }

    softap_lan_net = softap_net_info.lan_net_ptr;
    if (!softap_lan_net->lan_ipv4_address_user) {
        softap_lan_net->lan_ipv4_address = ANET_SOFTAP_LAN_IPV4_ADDR;
        softap_lan_net->ipv4_addr_pool_start = ANET_SOFTAP_LAN_IPV4_ADDR + 9;
    } else {
        softap_lan_net->lan_ipv4_address = softap_lan_net->lan_ipv4_address_user;
        softap_lan_net->ipv4_addr_pool_start = softap_lan_net->lan_ipv4_address_user + 9;
    }
    softap_lan_net->ipv4_addr_pool_end = softap_lan_net->ipv4_addr_pool_start +
                                         ANET_SOFTAP_IPV4_ADDR_LIST_SIZE -1;
    softap_lan_net->ipv4_addr_pool_size = ANET_SOFTAP_IPV4_ADDR_LIST_SIZE;
    softap_lan_net->lan_ipv4_subnet_mask = ANET_LAN_NET_MASK_ROUTING;

    wan_net = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
    if (wan_net) {
        softap_net_info.wan_net_ptr = wan_net;
        softap_net_info.wan_intf_index = wan_net->wan_ifc_index;
        softap_lan_net->lan_ipv4_dns1_address = wan_net->wan_ipv4_dns1_address;
        softap_lan_net->lan_ipv4_dns2_address = wan_net->wan_ipv4_dns2_address;
    } else {
        softap_lan_net->lan_ipv4_dns1_address = ANET_DEFAULT_DNS_SERVER1_V4;
        softap_lan_net->lan_ipv4_dns2_address = ANET_DEFAULT_DNS_SERVER2_V4;
    }

    ret = attach_softap_lan_interface(ip_link_driver);
    if (ret) {
        aicx_error(0x13fd81, "x", "attach softap intf error 0x%x\n", ret);
        return ret;
    }

#ifdef CONFIG_NX_DHCP
    if (aic_is_valid_intf_index(softap_net_info.lan_intf_index) &&
        softap_net_info.dhcp_configured == ANET_DHCP_NOT_CONFIGURED)
        ret = start_dhcp_server_for_softap(softap_net_info.lan_intf_index);
#endif

    return ret;
}

void aic_config_lan_ip(u32 ip_addr)
{
    softap_net_info.lan_net_ptr->lan_ipv4_address_user = ip_addr;
}

/** It is called after interface wlan0 is up. */
u32 aic_config_softap_intf(u8 *intf_name, VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *))
{
    s32 ret = AIC_SUCCESS;

    aos_get_mutex(softap_net_mutex, AOS_WAIT_FOREVER);
    softap_net_info.wan_net_id = ANET_DEFAULT_PDP_CID;
    strncpy(softap_net_info.softap_interfce, intf_name,
            (sizeof(softap_net_info.softap_interfce) - 1));
    softap_net_info.lan_intf_state = 1;
    softap_net_info.net_ip = (NX_IP *)alte_ip.ip_ptr;

    ret = config_softap_lan_net_ipv4(ip_link_driver);
    if (!ret) {
        /* Register cb of arp probe failing to get which client is disconnected*/
        aic_register_arp_age_notify(&alte_ip, dhcp_client_discon_via_probe);
    }
    aos_put_mutex(softap_net_mutex);

    return AIC_SUCCESS;
}

u32 aic_delete_softap_intf(void)
{
    u8 intf_index;

    aos_get_mutex(softap_net_mutex, AOS_WAIT_FOREVER);
    delete_all_softap_client_nat();

    intf_index = softap_net_info.lan_intf_index;
    if (aic_is_valid_intf_index(intf_index))
        nx_ip_interface_detach((NX_IP *)alte_ip.ip_ptr, intf_index);

    if (softap_net_info.lan_net_ptr)
        aos_free((u8 *)softap_net_info.lan_net_ptr);

    /* Clear the usb net info of the index. */
    reset_softap_net_info();

    /* If all usb net info are cleared,delete dhcp and dhcpv6 server. */
    if (!aic_dhcp_server_still_used()) {
#ifdef CONFIG_NX_DHCP
        if (dhcp_server) {
            nx_dhcp_server_delete(dhcp_server);
            aos_free(dhcp_server);
            dhcp_server = AIC_NULL;
            if (server_dhcp_stack) {
                aos_free(server_dhcp_stack);
                server_dhcp_stack = AIC_NULL;
            }
         }
#endif
    }

    aos_put_mutex(softap_net_mutex);

    return AIC_SUCCESS;
}

//u32 config_softap_lan_net_ipv6(void)
//{
//}

void aic_notify_softap_pdp_activated(void)
{
    WAN_NET *wan_net;

    /* Softap is switched on. */
    if (!aic_is_valid_intf_index(softap_net_info.lan_intf_index))
        return;

    wan_net = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
    if (wan_net) {
        softap_net_info.wan_net_ptr = wan_net;
        softap_net_info.wan_intf_index = wan_net->wan_ifc_index;
    }

    aos_get_mutex(softap_net_mutex, AOS_WAIT_FOREVER);
#ifdef CONFIG_NX_NAT
    if (!config_softap_intf_nat(wan_net->wan_ifc_index)) {
        create_nat_entry_for_all_clients(wan_net->wan_ifc_index);
    }
#endif
    aos_put_mutex(softap_net_mutex);
}

/* Deactivating of PDP is invoked. Some info is still kept. */
void aic_notify_softap_pdp_deact(void)
{
    if (!aic_is_valid_intf_index(softap_net_info.lan_intf_index))
        return;

    aos_get_mutex(softap_net_mutex, AOS_WAIT_FOREVER);
#ifdef CONFIG_NX_NAT
    delete_all_softap_client_nat();
#endif

    softap_net_info.wan_net_ptr = AIC_NULL;
    softap_net_info.wan_intf_index = ANET_INVALID_INTF_INDEX;
    aos_put_mutex(softap_net_mutex);
}

