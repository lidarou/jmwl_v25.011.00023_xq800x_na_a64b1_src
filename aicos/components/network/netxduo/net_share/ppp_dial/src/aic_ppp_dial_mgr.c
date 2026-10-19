/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Aic PPP Dial Management Component.
 *    This is the management of ppp dial,which is one kind of network share.
 *    After the dial commands are received and pdp is activated,ppp dial net,
 *    lan net and wan net would be configured. Also,PPP would be started
 *    to assign ip addresses,dns addresses for peer.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c3d29
#define LOG_MODULE    AMOD_NET

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <tx_api.h>
#include <nx_api.h>
#include <nxd_bsd.h>
#include <nx_ppp.h>
#include <aic_size.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <connmgr_api.h>
#include <aic_ipv6_forward.h>
#include "aic_ppp_dial_mgr.h"
#include "aic_net_api.h"
#include "aic_at_cmd_handle.h"
#include "aic_net_share_common.h"
#include "aic_at_parse_utils.h"

#ifdef CONFIG_NET_ALTE_DRIVER
#include <nx_alte_driver.h>
#endif

#ifdef CONFIG_NET_PPP_CODEC
#include <aic_ppp_codec_core.h>
#endif

#ifdef CONFIG_USB_NET_SHARE
#include "aic_usb_net_share_mgr.h"
#endif

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
#include "aic_net_share_port_mgr.h"
#endif

/* Define internal vars. */
#ifdef CONFIG_NX_DHCP
#ifdef ENABLE_DHCPV6_SERVER
static ulong ppp_dhcpv6_duid = 0;
#endif
#endif

/* Define internal functions. */
static void non_ppp_frame_handler(NX_PACKET *packet_ptr);
static u32 create_and_start_ppp(void);
static u32 config_and_start_ppp(void);
#ifdef CONFIG_NX_DHCP
#ifdef ENABLE_DHCPV6_SERVER
static u32 start_dhcpv6_for_ppp(void);
static u32 set_ppp_dhcpv6_paras(void);
#endif
#endif
static u32 ppp_dial_config_lan(u8 share_mode);
static s32 create_ppp_lan_net(void);
static u32 config_ppp_lan_forwarding(void);
static u32 config_ppp_lan_routing(void);
//static u32 set_ppp_lan_interface(u32 *interface_index);
static void ppp_link_up_callback(NX_PPP *ppp_ptr);
static void ppp_link_down_callback(NX_PPP *ppp_ptr);
//static void dhcpv6_server_thread_task(void *thread_input);
static void set_ppp_intf_ipv6_addrs(u32 lan_interf_index);
//static u32 pap_verify_login(char *name, char *password);
static u32 get_ppp_intef_index(u8 *ifc_name);
static void set_ppp_lan_mac_addr(u32 intf_index);
static void extract_ipv6cp_interface_id(u8 *intf_id_ptr, ulong *ipv6_addr);
#ifdef CONFIG_NX_NAT
static u32 config_ppp_interf_nat(u32 interf_index);
static void delete_ppp_interf_nat(u32 interf_index);
static void ppp_nat_cache_full_callback(NX_NAT_DEVICE *nat_ptr);
#endif

/* Define global vars. */
aos_mutex_ptr ppp_dial_net_mutex;
PPP_DIAL_NET_INFO ppp_dial_net_info;
struct aic_network_ppp aic_ppp_server;
AIC_PPP_ECHO_CFG aic_ppp_echo_cfg;

/* Define external vars. */
extern aos_mutex_ptr pdp_info_mutex;
extern aos_mutex_ptr wan_net_mutex;
extern struct aic_network_ip_instance alte_ip;
extern struct aic_packet_pool anet_packet_pool;
extern NET_PDP_INFO net_pdp_info_list[ANET_PDP_MAX_NUM];
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];
extern aos_event_ptr event_net_share;
extern AT_CMD_HANDLE ppp_dial_at_handles[];
#ifdef CONFIG_NX_NAT
extern NX_NAT_DEVICE *nat_dev_list[NX_MAX_PHYSICAL_INTERFACES];
#endif

#ifdef CONFIG_NX_DHCP
extern NX_DHCPV6_SERVER *dhcpv6_server;
extern u8 *dhcpv6_server_stack;
#endif

static void aic_ppp_set_net_share_event(ulong event)
{
    aos_set_event(event_net_share, event, AOS_OR);
}

static void aic_ppp_reset_echo_retry_times(NX_PPP *ppp_ptr)
{
    if (ppp_ptr)
        ppp_ptr->nx_ppp_echo_retry_times = aic_ppp_echo_cfg.retry_times;
}

void aic_ppp_dial_init(void)
{
    aic_add_at_cmd_handle(&ppp_dial_at_handles[0]);
    ppp_dial_net_mutex = aos_create_mutex("ppp_dial_net_mutex");
    AIC_ASSERT(ppp_dial_net_mutex);

    aos_memset(&ppp_dial_net_info, 0, sizeof(PPP_DIAL_NET_INFO));
    ppp_dial_net_info.serial_id = 0xffff;
    ppp_dial_net_info.lan_intf_index = 0xff;
    ppp_dial_net_info.wan_interf_index = 0xff;
    aos_memset(ppp_dial_net_info.pdp_type, '\0',
               sizeof(ppp_dial_net_info.pdp_type));
    aos_memset(ppp_dial_net_info.apn, '\0',
               sizeof(ppp_dial_net_info.apn));
    strcpy(ppp_dial_net_info.interf_name, "ppp0");

    aic_ppp_echo_cfg.echo_enabled = NX_PPP_ECHO_REQUEST_ENABLED;
    aic_ppp_echo_cfg.echo_interval = NX_PPP_ECHO_REQUEST_INTERVAL;
    aic_ppp_echo_cfg.retry_times = NX_PPP_ECHO_REQUEST_RETRY_TIMES;
    aic_ppp_echo_cfg.timeout_one_shot = NX_PPP_ECHO_TIMEOUT_ONE_SHOT;
}

/* It is called after ATD(T)*99***cid# is received from serial ports. */
u32 aic_ppp_dial_start(u32 serial_channel_id, u8 cid)
{
    u32 status = AIC_SUCCESS;

    if (ppp_dial_net_info.dial_started == ANET_PPP_DIAL_STARTED) {
        aicx_info(0x04c229, "", "ppp dial already started\n");
        return status;
    }

    aos_get_mutex(ppp_dial_net_mutex, AOS_WAIT_FOREVER);
    ppp_dial_net_info.dial_started = ANET_PPP_DIAL_STARTED;
    ppp_dial_net_info.serial_id = serial_channel_id;
    ppp_dial_net_info.wan_net_id = cid;
    ppp_dial_net_info.lan_net_id = ANET_PPP_LAN_NET_ID;
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
    ppp_dial_net_info.port_range_start = ANET_PPP_DIAL_PORT_START;
    ppp_dial_net_info.port_range_end = ANET_PPP_DIAL_PORT_END;
#endif

    /* Set share mode to ndis for default. */
    if (ppp_dial_net_info.ppp_dial_mode == 0)
        ppp_dial_net_info.ppp_dial_mode = ANET_SHARE_MODE_FORWARDING;
    aos_put_mutex(ppp_dial_net_mutex);

    /* Send attach cmd for LTE.PDP will be activated after attach. */
    if (!aic_is_ps_registered())
        aic_attach_packet_domain();
    else
        aic_enable_data_connection(cid, ppp_dial_net_info.pdp_type,
                                   ppp_dial_net_info.apn);

    if (cid == ANET_DEFAULT_PDP_CID &&
        ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
        status = aic_init_net_port_hashtable(ANET_NETSHARE_TYPE_PPP,
                                             ppp_dial_net_info.ppp_dial_mode);
        if (status) {
            aicx_error(0x06b261, "", "init hashtable for ppp dial error\n");
            return status;
        }
#endif
    }

    return status;
}

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
void aic_ppp_ipv6_dad_success(NX_IP *ip_ptr, u32 interf_index, u32 addr_index)
{
    if (interf_index != ppp_dial_net_info.lan_intf_index ||
        addr_index != ppp_dial_net_info.lan_net_ptr->global_ipv6_addr_index)
        return;

    aicx_info(0x09706c, "", "DAD success for ppp global ipv6 addr\n");
    aic_network_ipv6_send_ra(ip_ptr, interf_index);
}
#endif

/** aic_set_pdp_activated should be called first when PDP is activated,
 * then call usb_tether_pdp_activated.
 */
u32 aic_ppp_dial_pdp_activated(u8 pdp_cid)
{
    u8 share_mode;
    u32 status = AIC_SUCCESS;
    u8 wan_index;
    ulong ipv4_addr;
    WAN_NET *wan_net;
    PPP_DIAL_LAN_NET *lan_net;
    u32 lan_intf_index = NX_MAX_PHYSICAL_INTERFACES;

    if (ppp_dial_net_info.dial_started != ANET_PPP_DIAL_STARTED ||
        ppp_dial_net_info.wan_net_id != pdp_cid) {
        return status;
    }

    ppp_dial_net_info.lan_net_ip = (NX_IP *)alte_ip.ip_ptr;
    wan_net = ppp_dial_net_info.wan_net_ptr;
    lan_net = ppp_dial_net_info.lan_net_ptr;
    if (wan_net && lan_net && aic_is_valid_ipv6_addr(lan_net->lan_global_ipv6_addr)) {
        return AIC_SUCCESS;
    }

    aos_get_mutex(ppp_dial_net_mutex, AOS_WAIT_FOREVER);

    /* Config wan net first. */
    if (!wan_net) {
        wan_index = aic_get_wan_net_by_cid(pdp_cid);
        if (wan_index != ANET_PDP_MAX_NUM) {
            ppp_dial_net_info.wan_net_ptr = &wan_net_list[wan_index];
            ppp_dial_net_info.wan_interf_index =
                wan_net_list[wan_index].wan_ifc_index;
        }
    }

    share_mode = ppp_dial_net_info.ppp_dial_mode;
    status = ppp_dial_config_lan(share_mode);
    lan_net = ppp_dial_net_info.lan_net_ptr;
    aos_put_mutex(ppp_dial_net_mutex);
    if (status) {
        aicx_error(0x142c83, "", "config lan for ppp dial error\n");
        return status;
    }

    status = create_and_start_ppp();
    if (status) {
        aicx_error(0x023e09, "x", "create and start ppp error 0x%x\n", status);
        return status;
    }

    if (ppp_dial_net_info.lan_intf_index == 0xff) {
        ipv4_addr = ppp_dial_net_info.lan_net_ptr->lan_ipv4_address;
        //alog_info("%s:ipv4_addr is 0x%x\n", __func__, ipv4_addr);
#ifdef CONFIG_NX_PPP
        status = nx_ip_interface_attach((NX_IP *)alte_ip.ip_ptr, 
                                        ppp_dial_net_info.interf_name,
                                        ipv4_addr, 0xffffff00UL,
                                        nx_ppp_driver);
        if (status) {
            aicx_error(0x01b173, "x", "attach ppp interf error--0x%x\n", status);
            return status;
        }
#endif

        lan_intf_index = get_ppp_intef_index(ppp_dial_net_info.interf_name);
        if (lan_intf_index == NX_MAX_PHYSICAL_INTERFACES) {
            aicx_error(0x171f6e, "", "ERROR interface index for ppp\n");
            return ANET_ERROR_INVALID_IFC_INDEX;
        }
        ppp_dial_net_info.lan_intf_index = lan_intf_index;
        /* Modify the mtu size. */
        status = nx_ip_interface_mtu_set((NX_IP *)alte_ip.ip_ptr,
                                          lan_intf_index,
                                          ANET_PPP_LAN_MTU_SIZE);
        if (status) {
            aicx_error(0x07555a, "x", "set MTU error for ppp 0x%x\n", status);
        }
    }

    set_ppp_lan_mac_addr(ppp_dial_net_info.lan_intf_index);
    set_ppp_intf_ipv6_addrs(ppp_dial_net_info.lan_intf_index);

#ifdef CONFIG_NX_NAT
    if (
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
        ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_ROUTING &&
#endif
        !ppp_dial_net_info.nat_entry_created) {
        config_ppp_interf_nat(ppp_dial_net_info.wan_net_ptr->wan_ifc_index);
    }
#endif

#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
    aicx_info(0x065146, "", "ppp dial enable ipv6 packet route\n");
    aic_enable_ipv6_packet_route(&alte_ip);
#endif

#ifdef CONFIG_NX_DHCP
#ifdef ENABLE_DHCPV6_SERVER
    /** Start DHCPv6 Server if has got global ipv6 addr.
     * Maybe there is no global ipv6 addr now,then DHCPv6 server
     * would be statrted when we are notified by ipv6 auto config complete.
     */
    if (lan_net && aic_is_valid_ipv6_addr(lan_net->lan_global_ipv6_addr)) {
        if (!dhcpv6_server) {
            status = start_dhcpv6_for_ppp();
            if (!status) {
                ppp_dial_net_info.dhcpv6_started = ANET_DHCPV6_STARTED;
            } else {
                aicx_error(0x06b9df, "x", "start DHCPv6 for ppp error--0x%x\n", status);
                ppp_dial_net_info.dhcpv6_started = ANET_DHCPV6_NOT_STARTED;
            }
        }
    }
#endif
#endif

    return status;
}

/** Set the MAC addr of ppp lan interface,which is used by DHCPV6.
 * The MAC addr is random.
 */
void set_ppp_lan_mac_addr(u32 intf_index)
{
    NX_INTERFACE *nx_interf_ptr;

    nx_interf_ptr = &alte_ip.ip_ptr->nx_ip_interface[intf_index];
    /* Msw is 2 bytes,lsw is 4 bytes. */
    nx_interf_ptr->nx_interface_physical_address_msw = (ulong)0x0102;
    nx_interf_ptr->nx_interface_physical_address_lsw = (ulong)0x03040506;
}

u32 get_ppp_intef_index(u8 *ifc_name)
{
    u32 index = NX_MAX_PHYSICAL_INTERFACES;
    u16 i;
    NX_INTERFACE *nx_interface;
    NX_IP *alte_ip_ptr;

    for(i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        alte_ip_ptr = (NX_IP *)alte_ip.ip_ptr;
        nx_interface = &(alte_ip_ptr->nx_ip_interface[i]);
        //alog_info("%s:interf name is %s\n", __func__,
                  //nx_interface->nx_interface_name);
        if (strncasecmp(ifc_name, nx_interface->nx_interface_name,
                        strlen(ifc_name)) == 0) {
            /* Find a valid index. */
            index = i;
            //alog_info("%s:get interf index--%d\n", __func__, index);
            break;
        }
    }

    return index;
}

#ifdef CONFIG_NX_NAT
u32 config_ppp_interf_nat(u32 interf_index)
{
    u32 ret;
    ulong peer_addr;
    NX_NAT_TRANSLATION_ENTRY *icmp_entry_ptr;
    NX_NAT_TRANSLATION_ENTRY *tcp_entry_ptr;
    NX_NAT_TRANSLATION_ENTRY *udp_entry_ptr;
    NX_NAT_DEVICE *nat_dev_ptr;

    if (!nat_dev_list[interf_index]) {
        nat_dev_list[interf_index] =
            (NX_NAT_DEVICE *)aos_zalloc(sizeof(NX_NAT_DEVICE));
        AOS_ASSERT(nat_dev_list[interf_index]);
    }
    nat_dev_ptr = nat_dev_list[interf_index];

    if (nat_dev_ptr->nx_nat_id != NX_NAT_ID) {
        alte_ip.nat_cache_mem_size = SIZE_8K + SIZE_4K;
        ret = aic_nat_create(nat_dev_ptr, &alte_ip, interf_index);
        if (ret) {
            aicx_error(0x1d1a03, "x", "create nat device for ppp fail 0x%x\n", ret);
            return ret;
        }
    }

    nx_nat_cache_notify_set(interf_index, ppp_nat_cache_full_callback);
    nx_nat_enable(interf_index);
    peer_addr = ppp_dial_net_info.lan_net_ptr->peer_ipv4_addr;
    icmp_entry_ptr = &ppp_dial_net_info.ppp_icmp_nat_entry;
    tcp_entry_ptr = &ppp_dial_net_info.ppp_tcp_nat_entry;
    udp_entry_ptr = &ppp_dial_net_info.ppp_udp_nat_entry;

    ret = nx_nat_inbound_entry_create(interf_index, icmp_entry_ptr,
                                      peer_addr, 0, 0, NX_PROTOCOL_ICMP);
    if (ret)
        aicx_error(0x059694, "x", "create ICMP entry fail for ppp dial 0x%x\n", ret);

    ret = nx_nat_inbound_entry_create(interf_index, tcp_entry_ptr,
                                      peer_addr, 0, 0, NX_PROTOCOL_TCP);
    if (ret)
        aicx_error(0x198d99, "x", "create TCP entry fail for ppp dial 0x%x\n", ret);

    ret = nx_nat_inbound_entry_create(interf_index, udp_entry_ptr,
                                      peer_addr, 0, 0, NX_PROTOCOL_UDP);
    if (ret)
        aicx_error(0x021caf, "x", "create UDP entry fail for ppp dial 0x%x\n", ret);

    ppp_dial_net_info.nat_entry_created = ANET_NAT_ENTRY_CREATED;

    return ret;
}

void ppp_nat_cache_full_callback(NX_NAT_DEVICE *nat_ptr)
{
    aicx_info(0x14be9f, "d", "ppp nat cache full, interf index %d\n",
              nat_ptr->nx_nat_global_interface_index);
}

void delete_ppp_interf_nat(u32 interf_index)
{
    NX_NAT_TRANSLATION_ENTRY *icmp_entry_ptr;
    NX_NAT_TRANSLATION_ENTRY *tcp_entry_ptr;
    NX_NAT_TRANSLATION_ENTRY *udp_entry_ptr;
    NX_NAT_DEVICE *nat_dev_ptr;

    if (ppp_dial_net_info.nat_entry_created == ANET_NAT_ENTRY_NOT_CREATED ||
        interf_index >= NX_MAX_PHYSICAL_INTERFACES)
        return;

    icmp_entry_ptr = &ppp_dial_net_info.ppp_icmp_nat_entry;
    tcp_entry_ptr = &ppp_dial_net_info.ppp_tcp_nat_entry;
    udp_entry_ptr = &ppp_dial_net_info.ppp_udp_nat_entry;

    nx_nat_inbound_entry_delete(interf_index, icmp_entry_ptr);
    nx_nat_inbound_entry_delete(interf_index, tcp_entry_ptr);
    nx_nat_inbound_entry_delete(interf_index, udp_entry_ptr);

    if (aic_usb_net_use_same_nat(interf_index) || aic_softap_use_same_nat(interf_index)) {
        ppp_dial_net_info.nat_entry_created = ANET_NAT_ENTRY_NOT_CREATED;
        return;
    }

    nat_dev_ptr = nat_dev_list[interf_index];
    /* No usb net share relating to the PDP. */
    if (nat_dev_ptr) {
        aic_nat_delete(nat_dev_ptr, &alte_ip);
        aos_free(nat_dev_ptr);
        nat_dev_list[interf_index] = AIC_NULL;
    }

    ppp_dial_net_info.nat_entry_created = ANET_NAT_ENTRY_NOT_CREATED;
}
#endif

/** aic_set_pdp_deactivated should be called first when PDP is deactivated. */
u32 aic_ppp_dial_pdp_deactivated(u16 pdp_cid)
{
    u32 serial_port_id;

    aos_get_mutex(ppp_dial_net_mutex, AOS_WAIT_FOREVER);
    if (ppp_dial_net_info.wan_net_id != pdp_cid) {
        aos_put_mutex(ppp_dial_net_mutex);
        return AIC_SUCCESS;
    }

    if (ppp_dial_net_info.serial_id == AT_USB_AT_PORT_ID)
        serial_port_id = conn_get_usb_at_conid;
    else
        serial_port_id = ppp_dial_net_info.serial_id;

    aos_put_mutex(ppp_dial_net_mutex);

    nx_ppp_stop(aic_ppp_server.nx_ppp);

    /* Wait a while before enter at mode,for PPP will transmit
     * terminate frames to stop. */
    aicx_info(0x15019d, "", "wait 1s before enter PPP at mode\n");
    aos_sleep(1000);

    /* Make serial port enter at cmd mode. */
#ifdef CONFIG_NET_PPP_CODEC
    aic_ppp_codec_stop_process();
#endif
    conn_at_set_work_mode(serial_port_id, CONN_AT_MODE);
    ppp_dial_net_info.dial_com_mode = CONN_AT_MODE;

    nx_ip_interface_detach((NX_IP *)alte_ip.ip_ptr,
                           ppp_dial_net_info.lan_intf_index);

    aic_delete_ppp(&aic_ppp_server);
    aos_memset(&aic_ppp_server, 0, sizeof(aic_ppp_server));

    aos_get_mutex(ppp_dial_net_mutex, AOS_WAIT_FOREVER);

#ifdef CONFIG_NX_DHCP
#ifdef ENABLE_DHCPV6_SERVER
    if (dhcpv6_server && ppp_dial_net_info.dhcpv6_started) {
        nx_dhcpv6_server_delete(dhcpv6_server);
        aos_free(dhcpv6_server);
        dhcpv6_server = AIC_NULL;
        if (dhcpv6_server_stack) {
            aos_free(dhcpv6_server_stack);
            dhcpv6_server_stack = AIC_NULL;
        }
    }
#endif
#endif

    /* Clear lan net and ppp_dial_net_info. */
    if (ppp_dial_net_info.lan_net_ptr) {
        /* Free the memory of lan net. */
        aos_free((u8 *)ppp_dial_net_info.lan_net_ptr);
    }

    aic_reset_ppp_dial_net_info();
    aos_put_mutex(ppp_dial_net_mutex);

    return AIC_SUCCESS;
}

/* Configure the lan net for PPP dial. */
u32 ppp_dial_config_lan(u8 share_mode)
{
    u32 status = AIC_SUCCESS;

    if (!ppp_dial_net_info.dial_started) {
        return status;
    }

    if (share_mode == ANET_SHARE_MODE_FORWARDING) {
        status = config_ppp_lan_forwarding();
    } else if (share_mode == ANET_SHARE_MODE_ROUTING){
        status = config_ppp_lan_routing();
    }
    if (status) {
        aicx_error(0x1bb51e, "", "config ppp lan net error\n");
        return status;
    }

    return AIC_SUCCESS;
}

u32 create_and_start_ppp(void)
{
    u32 status;

    if (aic_ppp_server.nx_ppp) {
        aicx_info(0x092c23, "", "already created PPP\n");
        return AIC_SUCCESS;
    }

    aic_ppp_server.nx_ppp_name = "aic_ppp_server";
    aic_ppp_server.nx_ip_ptr = alte_ip.ip_ptr;
    aic_ppp_server.stack_size = SIZE_2K;
    aic_ppp_server.thread_priority = 10;
    aic_ppp_server.packet_pool_name = "ppp_packet_pool";
    /** Not sure about the size. Only LCP and NCP frames wil
     * consume the packet pool.
     */
    aic_ppp_server.packet_pool_size = ANET_PPP_CONFIG_FRAME_SIZE * 15;
    aic_ppp_server.per_packet_size = ANET_PPP_CONFIG_FRAME_SIZE;
    aic_ppp_server.ppp_non_ppp_packet_handler = non_ppp_frame_handler;
    aic_ppp_server.ppp_byte_send = AIC_NULL;

    status = aic_create_ppp(&aic_ppp_server);
    if (status) {
        aicx_error(0x0653cb, "x", "create PPP error--0x%x\n", status);
        return status;
    }

    aic_ppp_server.nx_ppp->nx_ppp_reset_echo_retry_times = aic_ppp_reset_echo_retry_times;
    aic_ppp_server.nx_ppp->nx_ppp_set_net_share_event = aic_ppp_set_net_share_event;
    status = config_and_start_ppp();
    if (status)
        aicx_error(0x14f73b, "x", "config PPP error --0x%x\n", status);

    return status;
}

void aic_ppp_load_echo_cfg(NX_PPP *ppp_ptr)
{
    if (ppp_ptr) {
        ppp_ptr->nx_ppp_echo_enabled = aic_ppp_echo_cfg.echo_enabled;
        ppp_ptr->nx_ppp_echo_timeout = aic_ppp_echo_cfg.timeout_one_shot;
        ppp_ptr->nx_ppp_echo_retry_times = aic_ppp_echo_cfg.retry_times;
        ppp_ptr->nx_ppp_echo_interval = aic_ppp_echo_cfg.echo_interval;
        ppp_ptr->nx_ppp_echo_waiting_time = 0;
        ppp_ptr->nx_ppp_next_echo_remain_time = 0;
    }
}

u32 config_and_start_ppp(void)
{
    WAN_NET *wan_net;
    PPP_DIAL_LAN_NET *ppp_lan_net;
    NX_PPP *nx_ppp_ptr;
    u8 local_interface_id[8] = {0};
    u8 peer_interface_id[8] = {0};
    u8 pdp_cid;
    u32 status;
    boolean has_ipv6 = AIC_FALSE;

    wan_net = ppp_dial_net_info.wan_net_ptr;
    ppp_lan_net = ppp_dial_net_info.lan_net_ptr;
    if (!wan_net || !ppp_lan_net) {
        aicx_error(0x00b17f, "", "error,no wan net or lan net for ppp dial\n");
        return ANET_ERROR_PPP_NET_NOT_READY;
    }

    nx_ppp_ptr = aic_ppp_server.nx_ppp;
    pdp_cid = ppp_dial_net_info.wan_net_id;

    /* Get ipv6 interface id. */
    if (aic_is_valid_ipv6_addr(ppp_lan_net->local_ipv6_addr)) {
        has_ipv6 = AIC_TRUE;
        extract_ipv6cp_interface_id(local_interface_id,
                                    ppp_lan_net->local_ipv6_addr);
        extract_ipv6cp_interface_id(peer_interface_id,
                                    ppp_lan_net->peer_ipv6_addr);
    }

    /* There is an ipv4 address. */
    if(ppp_lan_net->lan_ipv4_address != 0) {
        status = nx_ppp_ip_address_assign(nx_ppp_ptr,
                                          ppp_lan_net->lan_ipv4_address,
                                          ppp_lan_net->peer_ipv4_addr);
        nx_ppp_dns_address_set(nx_ppp_ptr,
                               ppp_lan_net->lan_ipv4_dns1_address);
        /* Use 8.8.8.8 as one dns server. */
        nx_ppp_secondary_dns_address_set(nx_ppp_ptr, IP_ADDRESS(8, 8, 8, 8));
        //nx_ppp_secondary_dns_address_set(nx_ppp_ptr,
                                         //ppp_lan_net->lan_ipv4_dns2_address);
        /* There is an ipv6 address. */
        if (has_ipv6) {
            _nx_ppp_ipv4v6_mode_set(nx_ppp_ptr, NX_PPP_IP_MODE_V4V6);

            status = nx_ppp_ipv6_interface_id_set(nx_ppp_ptr,
                                                  local_interface_id,
                                                  peer_interface_id);
        } else {
            _nx_ppp_ipv4v6_mode_set(nx_ppp_ptr, NX_PPP_IP_MODE_V4);
        }
    } else {
        _nx_ppp_ipv4v6_mode_set(nx_ppp_ptr, NX_PPP_IP_MODE_V6);
        status = nx_ppp_ipv6_interface_id_set(nx_ppp_ptr,
                                              local_interface_id,
                                              peer_interface_id);
    }

    aic_ppp_load_echo_cfg(nx_ppp_ptr);
    /* Register the link up/down callbacks. */
    nx_ppp_link_up_notify(nx_ppp_ptr, ppp_link_up_callback);
    /* Will ppp be down if received LCP or IPCP terminate? */
    nx_ppp_link_down_notify(nx_ppp_ptr, ppp_link_down_callback);

    nx_ppp_pap_enable(nx_ppp_ptr, AIC_NULL, AIC_NULL);
    /* Not enable CHAP,something is not clear now. */
    //nx_ppp_start(nx_ppp_ptr);

    return status;
}

void extract_ipv6cp_interface_id(u8 *intf_id_ptr, ulong *ipv6_addr)
{
    if (!intf_id_ptr || !ipv6_addr)
        return;

    intf_id_ptr[0] = (u8)((ipv6_addr[2] >> 24) & 0xffUL);
    intf_id_ptr[1] = (u8)((ipv6_addr[2] >> 16) & 0xffUL);
    intf_id_ptr[2] = (u8)((ipv6_addr[2] >> 8) & 0xffUL);
    intf_id_ptr[3] = (u8)(ipv6_addr[2] & 0xffUL);

    intf_id_ptr[4] = (u8)((ipv6_addr[3] >> 24) & 0xffUL);
    intf_id_ptr[5] = (u8)((ipv6_addr[3] >> 16) & 0xffUL);
    intf_id_ptr[6] = (u8)((ipv6_addr[3] >> 8) & 0xffUL);
    intf_id_ptr[7] = (u8)(ipv6_addr[3] & 0xffUL);
}

#if 0
u32 pap_verify_login(char *name, char *password)
{
    aicx_info(0x1dd28b, "sss", "%s:name is %s, password is %s, verify ok\n",
              __func__, name, password);
    return AIC_SUCCESS;
}
#endif

#ifdef CONFIG_NX_DHCP
#ifdef ENABLE_DHCPV6_SERVER
u32 start_dhcpv6_for_ppp(void)
{
    u32 status = AIC_SUCCESS;

    status = aic_init_dhchv6_server();
    if (status)
        goto DHCPV6_ERROR;

    status = set_ppp_dhcpv6_paras();
    if (status) {
        nx_dhcpv6_server_delete(dhcpv6_server);
        aicx_error(0x0cda8b, "x", "ppp dial set dhcpv6 paras error 0x%x\n", status);
        goto DHCPV6_ERROR;
    }

    status = nx_dhcpv6_server_start(dhcpv6_server);
    if (status) {
        nx_dhcpv6_server_delete(dhcpv6_server);
        aicx_error(0x14e307, "x", "start dhcpv6 error for ppp dial 0x%x\n", status);
        goto DHCPV6_ERROR;
    }

    return status;

DHCPV6_ERROR:
    if (dhcpv6_server) {
       aos_free(dhcpv6_server);
       dhcpv6_server = AIC_NULL;
    }

    if (dhcpv6_server_stack) {
       aos_free(dhcpv6_server_stack);
       dhcpv6_server_stack = AIC_NULL;
    }

    return status;
}

u32 set_ppp_dhcpv6_paras(void)
{
    u32 status, addresses_added, ifc_index, addr_index, i;
    //ulong duid_time;
    NXD_ADDRESS dns_ipv6_address;
    NXD_ADDRESS start_ipv6_address;
    NXD_ADDRESS end_ipv6_address;
    PPP_DIAL_LAN_NET *lan_net;
    WAN_NET *wan_net;
    NX_INTERFACE *net_intf_ptr;
    aos_systime systime;
    s32 local_tz, tmp_tz;

    ifc_index = ppp_dial_net_info.lan_intf_index;
    lan_net = ppp_dial_net_info.lan_net_ptr;
    wan_net = ppp_dial_net_info.wan_net_ptr;

    addr_index = lan_net->global_ipv6_addr_index;
    if (addr_index < NX_MAX_IPV6_ADDRESSES) {
        /** Set the network interface and global ipv6 address index,
         * if there are more than 1 global ipv6 addresses.
         */
        status = nx_dhcpv6_server_interface_set(dhcpv6_server,
                                                ifc_index, addr_index);
        if (status) {
            aicx_error(0x0223d7, "x", "set dhcpv6 interface for ppp dial error 0x%x\n", status);
        }
    }

    /* Set up the DNS IPv6 server address. */
    dns_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    /* Copy the dns addr from wan_net. */
    for (i = 0; i < 4; i++) {
        dns_ipv6_address.nxd_ip_address.v6[i] =
            lan_net->lan_ipv6_dns1_address[i];
    }
    /*alog_info("%s:set dns 0x%08x 0x%08x 0x%08x 0x%08x\n", __func__,
              lan_net->lan_ipv6_dns1_address[0],
              lan_net->lan_ipv6_dns1_address[1],
              lan_net->lan_ipv6_dns1_address[2],
              lan_net->lan_ipv6_dns1_address[3]);*/

    status = nx_dhcpv6_create_dns_address(dhcpv6_server,
                                          &dns_ipv6_address);
    if (status) {
        aicx_error(0x1470f2, "x", "create dhcpv6 dns for ppp dial error--0x%x\n", status);
    }

    /** Note: For DUID types that do not require time, the 'duid_time'
     * input can be left at zero.
     * The DUID_TYPE and HW_TYPE are configurable options
     * that are user defined in nx_dhcpv6_server.h.
     */
    //duid_time = SECONDS_SINCE_JAN_1_2000_MOD_32;
    /** The seconds converted by aic_mktime_to_seconds is 8 hours ahead.
     * So the systime should be set 8 hours backward. For example,
     * if the actual time is 22:00:00,then the systime should be set 14:00:00.
     */
#if 0
    //set systime to fixed values.
    systime.year = 2022 - 30;
    systime.month = 7;
    systime.day = 21;
    systime.hour = 16;
    systime.minute = 30;
    systime.second = 0;
#endif
    /* Actually,get system time this way. */
    if (ppp_dhcpv6_duid == 0) {
        aos_get_systime(&systime);
        local_tz = aic_get_current_time_zone();
        tmp_tz = local_tz;
        local_tz = local_tz - (2 * tmp_tz);
        /* Ajust the time to time with time zone 0. */
        aic_adjust_time_with_time_zone(&systime, local_tz * 4);
        systime.year -= 30;
        ppp_dhcpv6_duid = aic_mktime_to_seconds(&systime);
    }

    net_intf_ptr = &(alte_ip.ip_ptr->nx_ip_interface[ifc_index]);
    status = nx_dhcpv6_set_server_duid(
             dhcpv6_server,
             NX_DHCPV6_SERVER_DUID_TYPE, NX_DHCPV6_SERVER_HW_TYPE,
             net_intf_ptr->nx_interface_physical_address_msw,
             net_intf_ptr->nx_interface_physical_address_lsw,
             ppp_dhcpv6_duid);
    if (status) {
        aicx_error(0x0cfe35, "x", "set duid for ppp dial error 0x%x\n", status);
    }

    /* The IPv6 start address and the end address are the same. */
    start_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    end_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    for (i = 0; i < 4; i++) {
        start_ipv6_address.nxd_ip_address.v6[i] = lan_net->peer_ipv6_addr[i];
        end_ipv6_address.nxd_ip_address.v6[i] = lan_net->peer_ipv6_addr[i];
    }

    /* Set the IPv6 address range,only one IPv6 address. */
    status = nx_dhcpv6_create_ip_address_range(dhcpv6_server,
                                               &start_ipv6_address,
                                               &end_ipv6_address,
                                               &addresses_added);
    if (status) {
        aicx_error(0x1c5246, "x", "create dhcpv6 ip range for ppp dial error 0x%x\n", status);
    }

    return status;
}
#endif
#endif

#if 0
/* Define the DHCPV6 server thread. */
void dhcpv6_server_thread_task(void *thread_input)
{
    u32 status;

    /* Create the DHCPv6 Server. */
    status =  nx_dhcpv6_server_create(
        ppp_dial_dhcpv6_server,
        (NX_IP *)ppp_dial_lan_ip.ip_ptr,
        "PPP_Dial_DHCPv6_Server",
        (NX_PACKET_POOL *)anet_packet_pool.packet_pool,
        ppp_dial_dhcpv6_stack,
        DHCPV6_SERVER_STACK_SIZE,
        NX_NULL, NX_NULL);

    if (status) {
        aicx_error(0x134707, "", "dhcpv6_server_thread_task:create dhcpv6 server error\n");
        return;
    }

    set_ppp_dhcpv6_paras();

    /* Start the NetX DHCPv6 server. */
    status =  nx_dhcpv6_server_start(ppp_dial_dhcpv6_server);
    if (status) {
        aicx_error(0x0d7358, "", "dhcpv6_server_thread_task:start dhcpv6 error\n");
    }
}
#endif

/** The format of decimal_ipv6_suffix is like this:
 * 10.36.125.47.00.00.172.245
 * interface_id is the suffix,it has 8 bytes.
 */
u32 extract_pdp_ipv6_suffix(u8 *interface_id, u8 *decimal_ipv6_suffix)
{
    u8 *tmp_ptr, *interface_ptr;
    u8 suffix_str[ANET_MAX_LEN_DEC_IPV6_ADDR/2] = {'\0'};
    u8 decimal_number;
    char *save_ptr;

    interface_ptr = interface_id;
    strncpy(suffix_str, decimal_ipv6_suffix,
            (ANET_MAX_LEN_DEC_IPV6_ADDR/2 - 1));
    tmp_ptr = strtok_r(suffix_str, ".", &save_ptr);

    while (tmp_ptr) {
        decimal_number = atoi(tmp_ptr);
        //snprintf(interface_ptr, 3, "%02x", decimal_number);
        *interface_ptr = decimal_number;
        interface_ptr++;

        tmp_ptr = strtok_r(AIC_NULL, ".", &save_ptr);
    }

    return AIC_SUCCESS;
}

void ppp_link_up_callback(NX_PPP *ppp_ptr)
{
    aicx_info(0x03f41a, "", "ppp link up\n");
    aic_network_ipv6_send_ra(ppp_dial_net_info.lan_net_ip,
                             ppp_dial_net_info.lan_intf_index);
}

void ppp_link_down_callback(NX_PPP *ppp_ptr)
{
    aicx_info(0x0c89b7, "", "ppp link is down\n");
}

void aic_reset_ppp_dial_net_info(void)
{
    aos_memset(&ppp_dial_net_info, 0, sizeof(PPP_DIAL_NET_INFO));
    ppp_dial_net_info.serial_id = 0xffff;
    ppp_dial_net_info.lan_intf_index = 0xff;
    ppp_dial_net_info.wan_interf_index = 0xff;
    aos_memset(ppp_dial_net_info.pdp_type, '\0',
               sizeof(ppp_dial_net_info.pdp_type));
    aos_memset(ppp_dial_net_info.apn, '\0',
               sizeof(ppp_dial_net_info.apn));
    strcpy(ppp_dial_net_info.interf_name, "ppp0");
}

void config_ppp_lan_ipv6(PPP_DIAL_LAN_NET *lan_net, WAN_NET *wan_net)
{
    u16 i;

    //alog_info("%s: E\n", __func__);
    if (aic_is_valid_ipv6_addr(lan_net->lan_global_ipv6_addr)) {
        return;
    }

    /* The ipv6 addr suffix of lan is random. */
    if (aic_is_valid_ipv6_addr(wan_net->wan_local_ipv6_addr)) {
        for (i = 0; i < 3; i++) {
            lan_net->local_ipv6_addr[i] = wan_net->wan_local_ipv6_addr[i];
        }
        lan_net->local_ipv6_addr[3] = wan_net->wan_local_ipv6_addr[3] +
                                      ANET_PPP_LAN_IPV6_GAP;

        /* IPv6 dns addrs of wan and lan are the same. */
        for (i = 0; i < 4; i++) {
            lan_net->lan_ipv6_dns1_address[i] =
                wan_net->wan_ipv6_dns1_address[i];
            lan_net->lan_ipv6_dns2_address[i] =
                wan_net->wan_ipv6_dns2_address[i];
        }
    }

    /** If the wan net has got the global ipv6 addr,
     * then assign a global addr to lan net.
     */
    if (aic_is_valid_ipv6_addr(wan_net->wan_ipv6_address1)) {
        for (i = 0; i < 4; i++) {
            lan_net->lan_global_ipv6_addr[i] = wan_net->wan_ipv6_address1[i];
        }
        lan_net->lan_global_ipv6_addr[3] += ANET_PPP_LAN_IPV6_GAP;

        /* The ipv6 addr suffix of peer,peer should get prefix from RA. */
        for (i = 0; i < 4; i++) {
            lan_net->peer_ipv6_addr[i] = lan_net->lan_global_ipv6_addr[i];
        }
        lan_net->peer_ipv6_addr[3] += 1;
    }
}

s32 create_ppp_lan_net(void)
{
    PPP_DIAL_LAN_NET *lan_net;

    ppp_dial_net_info.lan_net_ptr =
        (PPP_DIAL_LAN_NET *)aos_zalloc(sizeof(PPP_DIAL_LAN_NET));

    lan_net = ppp_dial_net_info.lan_net_ptr;
    if (lan_net) {
        lan_net->lan_net_id = ANET_PPP_LAN_NET_ID;
        lan_net->global_ipv6_addr_index = 0xffff;
        lan_net->local_ipv6_addr_index = 0xffff;
    } else {
        aicx_error(0x12e967, "", "malloc ppp lan net fail\n");
        return AIC_ENOMEM;
    }

    return AIC_SUCCESS;
}

/* Configure the lan net of PPP dial in forwarding mode. */
u32 config_ppp_lan_forwarding(void)
{
    PPP_DIAL_LAN_NET *lan_net;
    WAN_NET *wan_net;
    u32 ret;

    wan_net = ppp_dial_net_info.wan_net_ptr;

    if (wan_net) {
        /* Check if lan net has malloced. */
        if (!ppp_dial_net_info.lan_net_ptr) {
            ret = create_ppp_lan_net();
            if (ret) {
                return ret;
            }
        }

        lan_net = ppp_dial_net_info.lan_net_ptr;
        /** If the pdp has a ipv4 address, then generate a fake address
         * for lan net in ndis mode.
         */
        if (lan_net->lan_ipv4_address == 0 && wan_net->wan_ipv4_address) {
            /* This is a fake addr in ndis mode. */
            lan_net->lan_ipv4_address = wan_net->wan_ipv4_address + 1;
            /* In ndis mode, the subnet mask is 0xffffffff. */
            lan_net->lan_ipv4_dns1_address = wan_net->wan_ipv4_dns1_address;
            lan_net->lan_ipv4_dns2_address = wan_net->wan_ipv4_dns2_address;
            lan_net->peer_ipv4_addr = wan_net->wan_ipv4_address;
            //alog_info("%s:local ip is %d,peer ip is %d\n", __func__,
                      //lan_net->lan_ipv4_address, lan_net->peer_ipv4_addr);
        }
        lan_net->lan_ipv4_subnet_mask = ANET_LAN_NET_MASK_FORWARDING;

        config_ppp_lan_ipv6(lan_net, wan_net);
    } else {
        aicx_error(0x1968d9, "", "no wan net for ppp forwarding dial\n");
        return ANET_ERROR_WAN_NOT_FOUND;
    }

    return AIC_SUCCESS;
}

/* Configure the lan net of PPP dial in tehtering mode. */
u32 config_ppp_lan_routing(void)
{
    PPP_DIAL_LAN_NET *lan_net;
    WAN_NET *wan_net;
    u32 ret;

    wan_net = ppp_dial_net_info.wan_net_ptr;

    /* Set paras of LAN NET */
    if (wan_net) {
        /* Check if lan net has malloced. */
        if (!ppp_dial_net_info.lan_net_ptr) {
            ret = create_ppp_lan_net();
            if (ret) {
                return ret;
            }
        }

        lan_net = ppp_dial_net_info.lan_net_ptr;
        if (lan_net->lan_ipv4_address == 0 && wan_net->wan_ipv4_address) {
            lan_net->lan_ipv4_address = ANET_PPP_LAN_IPV4_ADDR;
            lan_net->lan_ipv4_subnet_mask = ANET_LAN_NET_MASK_ROUTING;
            lan_net->lan_ipv4_dns1_address = wan_net->wan_ipv4_dns1_address;
            lan_net->lan_ipv4_dns2_address = wan_net->wan_ipv4_dns2_address;
            lan_net->peer_ipv4_addr = ANET_PPP_PEER_IPV4_ADDR;
        }

        config_ppp_lan_ipv6(lan_net, wan_net);
    } else {
        aicx_error(0x0d4f14, "", "no wan net for ppp routing dial\n");
        return ANET_ERROR_WAN_NOT_FOUND;
    }

    return AIC_SUCCESS;
}

/* Set ipv6 link local and global addrs to PPP lan ip instance. */
void set_ppp_intf_ipv6_addrs(u32 lan_interf_index)
{
    u32 global_addr_index, local_addr_index;
    NXD_ADDRESS global_ipv6_addr, local_ipv6_addr;
    PPP_DIAL_LAN_NET *lan_net;
    u8 i;
    u32 status;

    lan_net = ppp_dial_net_info.lan_net_ptr;
    if (lan_interf_index >= NX_MAX_PHYSICAL_INTERFACES || !lan_net) {
        return;
    }

    if (lan_net->local_ipv6_addr_index >= NX_MAX_IPV6_ADDRESSES &&
        aic_is_valid_ipv6_addr(lan_net->local_ipv6_addr)) {
        local_ipv6_addr.nxd_ip_version = NX_IP_VERSION_V6;
        for (i = 0; i < 4; i++) {
            local_ipv6_addr.nxd_ip_address.v6[i] =
                lan_net->local_ipv6_addr[i];
        }
        status = nxd_ipv6_address_set((NX_IP *)alte_ip.ip_ptr,
                                      lan_interf_index,
                                      &local_ipv6_addr, 10,
                                      &local_addr_index);
        if (!status) {
            lan_net->local_ipv6_addr_index = local_addr_index;
        } else {
            aicx_error(0x0e795e, "", "set local ipv6 addr for ppp error\n");
        }
    }

    if (lan_net->global_ipv6_addr_index >= NX_MAX_IPV6_ADDRESSES &&
        aic_is_valid_ipv6_addr(lan_net->lan_global_ipv6_addr)) {
        global_ipv6_addr.nxd_ip_version = NX_IP_VERSION_V6;
        for (i = 0; i < 4; i++) {
            global_ipv6_addr.nxd_ip_address.v6[i] =
                lan_net->lan_global_ipv6_addr[i];
        }
        status = nxd_ipv6_address_set((NX_IP *)alte_ip.ip_ptr,
                                      lan_interf_index,
                                      &global_ipv6_addr, 64,
                                      &global_addr_index);
        if (!status) {
            lan_net->global_ipv6_addr_index = global_addr_index;
        } else {
            aicx_error(0x084de0, "", "set global ipv6 addr for ppp error\n");
        }
    }
}

/** Receive one of the following AT commands to stop ppp dial.
 * 1)+++ and ATH
 * 2)+++
 */
u32 aic_ppp_dial_stop(u32 serial_channel_id)
{
    u8 pdp_cid;

    /* Not the same serial port,then don't handle. */
    if (serial_channel_id != ppp_dial_net_info.serial_id) {
        return AIC_SUCCESS;
    }

    if (ppp_dial_net_info.dial_started == ANET_PPP_DIAL_STARTED) {
        pdp_cid = ppp_dial_net_info.wan_net_id;

#ifdef CONFIG_NX_NAT
        if (
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
            ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_ROUTING &&
#endif
            ppp_dial_net_info.nat_entry_created == ANET_NAT_ENTRY_CREATED) {
            delete_ppp_interf_nat(ppp_dial_net_info.wan_interf_index);
        }
#endif
        ppp_dial_net_info.dial_started = ANET_PPP_DIAL_STOPPED;

        if (ppp_dial_net_info.wan_net_id == ANET_DEFAULT_PDP_CID &&
            ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
            aic_delete_port_hashtable(ANET_NETSHARE_TYPE_PPP,
                                  ppp_dial_net_info.ppp_dial_mode);
#endif
        }

        aic_ppp_dial_pdp_deactivated(ppp_dial_net_info.wan_net_id);

#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
        aicx_info(0x0ec425, "", "ppp dial disable ipv6 packet route\n");
        aic_disable_ipv6_packet_route(&alte_ip);
#endif

        /* If not the default PDP,deactivate it. */
        if (pdp_cid != ANET_DEFAULT_PDP_CID) {
            aic_send_cgact_exe_cmd(ANET_PDP_TO_DEACTIVATE, pdp_cid);
        }
    }

    return AIC_SUCCESS;
}

/* Stop PPP dial when it is starting,such as fail to activate PDP. */
void aic_ppp_dial_stop_half_way(u8 pdp_cid)
{
    if (pdp_cid == ANET_DEFAULT_PDP_CID &&
        ppp_dial_net_info.ppp_dial_mode == ANET_SHARE_MODE_FORWARDING) {
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
        aic_delete_port_hashtable(ANET_NETSHARE_TYPE_PPP,
                              ppp_dial_net_info.ppp_dial_mode);
#endif
    }

    aic_reset_ppp_dial_net_info();
}

/* The handler for receiving a non-ppp data packet. */
void non_ppp_frame_handler(NX_PACKET *packet_ptr)
{
    u32 len;

    aicx_info(0x14c01c, "", "non ppp frame\n");
    len = packet_ptr->nx_packet_length;
    *(packet_ptr->nx_packet_prepend_ptr + len) = '\0';
    if (strstr(packet_ptr->nx_packet_prepend_ptr, "+++"))
        aicx_info(0x05822c, "", "recv non ppp frame +++\n");

    if (strcasestr(packet_ptr->nx_packet_prepend_ptr, "ATH")) {
        aicx_info(0x1e840c, "", "recv non ppp frame ATH,stop ppp dial\n");
        aos_set_event(event_net_share, NET_EVENT_STOP_PPP_DIAL, AOS_OR);
    }

    nx_packet_release(packet_ptr);
}

u8 aic_get_ppp_dial_cid(void)
{
    return ppp_dial_net_info.wan_net_id;
}

boolean aic_is_ppp_dial_data_mode(u32 com_id)
{
    return (ppp_dial_net_info.serial_id == com_id &&
            ppp_dial_net_info.dial_com_mode == CONN_PPP_MODE);
}

