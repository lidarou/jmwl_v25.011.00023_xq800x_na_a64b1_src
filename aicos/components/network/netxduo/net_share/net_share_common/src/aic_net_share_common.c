/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Variables and Functions for All Network Share
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x06f00c
#define LOG_MODULE    AMOD_NET

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <nx_api.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <connmgr_api.h>
#include <aic_net_api.h>
#include "aic_at_process_common.h"
#include "aic_at_urc_handle.h"
#include "aic_at_parse_utils.h"
#include "aic_at_interact_cp.h"
#include "aic_general_at_handle.h"
#include "aic_net_share_common.h"
#ifdef CONFIG_PPP_DIAL
#include "aic_ppp_dial_mgr.h"
#endif

#ifdef CONFIG_USB_NET_SHARE
#include "aic_usb_share_at_handle.h"
#include "aic_usb_net_share_mgr.h"
#endif

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
#include "aic_net_share_port_mgr.h"
#endif

#ifdef CONFIG_NX_DHCP
#include "nxd_dhcpv6_server.h"
#endif

#ifdef NET_AT_SNTP
#include "aic_at_sntp.h"
#endif

#ifdef CONFIG_NET_PPP_CODEC
#include <aic_ppp_codec_core.h>
#endif

#ifdef CONFIG_NET_SEMI_TETHER
#include "aic_net_usb_semi_tether.h"
#endif

#ifdef CONFIG_USB_SERVICE
#include <usb_service.h>
#endif

#ifdef CONFIG_NX_DHCP
#include "nxd_dhcp_server.h"
#endif

#ifdef CONFIG_SOFTAP_SHARE
#include "aic_softap_mgr.h"
#endif

/* Define global vars. */
aos_event_ptr event_net_share;
#ifdef CONFIG_NX_DHCP
NX_DHCP_SERVER *dhcp_server;
u8 *server_dhcp_stack;
u8 dhcp_server_name[20] = {'\0'};
aos_mutex_ptr dhcp_server_mutex;
#ifdef ENABLE_DHCPV6_SERVER
NX_DHCPV6_SERVER *dhcpv6_server;
u8 *dhcpv6_server_stack;
static u8 dhcpv6_server_name[20] = {'\0'};
#endif
#endif
#ifdef CONFIG_NX_NAT
NX_NAT_DEVICE *nat_dev_list[NX_MAX_PHYSICAL_INTERFACES];
#endif

/* Define internal vars. */
static aos_thread_id thread_net_share;

/* Define internal functions. */
static u32 init_net_share_thread(void);
static void net_share_handle_entry(void *thread_input);
#ifdef CONFIG_USB_SERVICE
static void usb_plug_state_callback(u32 usb_plug_state);
#endif

extern struct aic_network_ip_instance alte_ip;
/* Define external vars. */
extern struct aic_packet_pool anet_packet_pool;
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
extern hashtable_t *ppp_dial_hashtable;
extern hashtable_t *usb_net_hashtable;
#endif
#ifdef CONFIG_PPP_DIAL
extern PPP_DIAL_NET_INFO ppp_dial_net_info;
extern struct aic_network_ppp aic_ppp_server;
#endif
#ifdef CONFIG_USB_NET_SHARE
extern USB_TETHER_NET_INFO usb_tether_net_info_list[ANET_USB_NET_MAX_NUM];
#endif
#ifdef CONFIG_USB_SERVICE
extern u8 current_usb_mode;
#endif
#ifdef CONFIG_SOFTAP_SHARE
extern SOFTAP_NET_INFO softap_net_info;
#endif
extern INIT_NET_CFG init_net_cfg;

#if CONFIG_CM_USB_DIAL_AT && CONFIG_USB_NET_SHARE
extern CUSTOMER_M_DIAL_CFG m_dial_cfg;
#endif

/* Init the network share relating modules. */
s32 aic_net_share_init(void)
{
    init_net_share_thread();
#ifdef CONFIG_USB_NET_SHARE
    aic_usbtether_init();
#endif

#ifdef CONFIG_PPP_DIAL
    extern int aic_ppp_codec_init(void);

    aic_ppp_codec_init();
    aic_ppp_dial_init();
#endif

#ifdef CONFIG_USB_SERVICE
    /* basic mode can't use usb. */
    if (!aic_boot_mode_is_basic()) {
        usb_service_plug_event_listen_register(usb_plug_state_callback);
#ifdef ANET_ENABLE_USB_DONGLE
        usb_service_network_status_listen_register(aic_usb_intf_ready_notified);
#endif
    }
#endif

#ifdef CONFIG_SOFTAP_SHARE
    aic_softap_init();
#endif

#if defined CONFIG_USB_NET_SHARE || defined CONFIG_SOFTAP_SHARE
    strncpy(dhcp_server_name, "DHCP_SERVER", (sizeof(dhcp_server_name) - 1));
#ifdef ENABLE_DHCPV6_SERVER
    strncpy(dhcpv6_server_name, "DHCPV6_SERVER", (sizeof(dhcpv6_server_name) - 1));
#endif
    dhcp_server_mutex = aos_create_mutex("dhcp_server_mutex");
    AOS_ASSERT(dhcp_server_mutex);
#endif

    return AIC_SUCCESS;
}

void aic_net_set_usb_other_mode_event(void)
{
    aos_set_event(event_net_share, NET_EVENT_USB_TETHER_OTHER_MODE, AOS_OR);
}

#ifdef CONFIG_USB_SERVICE
void usb_plug_state_callback(u32 plug_state)
{
    aicx_info(0x1dc13d, "d", "usb plug state update %d\n", plug_state);

#ifdef CONFIG_USB_NET_SHARE
    if ((plug_state == USB_PLUG_DEFAULT_IN || plug_state == USB_PLUG_IN) &&
        !aic_usb_tether_already_started()) {
        aos_set_event(event_net_share, NET_EVENT_AUTO_USB_TETHER, AOS_OR);
    }
#endif
    if (plug_state == USB_PLUG_DEFAULT_OUT || plug_state == USB_PLUG_OUT) {
        current_usb_mode = LOG_AT_DIAG;

#ifdef CONFIG_PPP_DIAL
        if (ppp_dial_net_info.serial_id == AT_USB_AT_PORT_ID)
            aos_set_event(event_net_share, NET_EVENT_STOP_PPP_DIAL, AOS_OR);
#endif
    }
}
#endif

void aic_net_set_usb_dial_event(void)
{
    aos_set_event(event_net_share, NET_EVENT_AUTO_USB_TETHER, AOS_OR);
}

#ifdef CONFIG_NX_DHCP
u32 aic_init_dhcp_server(void)
{
    u32 status;

    if (!dhcp_server) {
        dhcp_server = aos_zalloc(sizeof(NX_DHCP_SERVER));
        if (!dhcp_server) {
            aicx_error(0x08f829, "", "malloc dhcp server fail\n");
            return AIC_ENOMEM;
        }
    }

    if (!server_dhcp_stack) {
        server_dhcp_stack = aos_zalloc(DHCP_SERVER_STACK_SIZE);
        if (!server_dhcp_stack) {
            aicx_error(0x057d70, "", "malloc dhcp server stack fail\n");
            aos_free(dhcp_server);
            dhcp_server = AIC_NULL;
            return AIC_ENOMEM;
        }
    }

    /* Create the DHCP Server. */
    status = nx_dhcp_server_create(
             dhcp_server, (NX_IP *)alte_ip.ip_ptr,
             (u8 *)server_dhcp_stack,
             DHCP_SERVER_STACK_SIZE, dhcp_server_name,
             (NX_PACKET_POOL *)anet_packet_pool.packet_pool);
    if (status) {
        aicx_error(0x02bf00, "x", "create dhcp server error 0x%x\n", status);
        goto INIT_ERROR;
    }

    return AIC_SUCCESS;

INIT_ERROR:
    aos_free(dhcp_server);
    dhcp_server = AIC_NULL;
    aos_free(server_dhcp_stack);
    server_dhcp_stack = AIC_NULL;

    return status;
}

#ifdef ENABLE_DHCPV6_SERVER
u32 aic_init_dhchv6_server(void)
{
    u32 status = AIC_SUCCESS;
    struct aic_network_ip_instance *pdp_ip_instance;

    pdp_ip_instance = aic_get_pdp_ip_instance();

    if (!dhcpv6_server) {
        dhcpv6_server = aos_zalloc_relaxed(sizeof(NX_DHCPV6_SERVER));
        if (!dhcpv6_server) {
            aicx_error(0x0cfce2, "", "malloc dhcpv6 server error\n");
            return AT_ERROR_MALLOC_FAIL;
        }

        if (!dhcpv6_server_stack) {
            dhcpv6_server_stack = aos_zalloc_relaxed(DHCPV6_SERVER_STACK_SIZE);
            if (!dhcpv6_server_stack) {
                aicx_error(0x095599, "", "malloc dhcpv6 server stack fail\n");
                aos_free(dhcpv6_server);
                dhcpv6_server = AIC_NULL;
                return AT_ERROR_MALLOC_FAIL;
            }
        }

        /* Create the DHCPv6 Server. */
        status = nx_dhcpv6_server_create(
                    dhcpv6_server,
                    (NX_IP *)pdp_ip_instance->ip_ptr,
                    dhcpv6_server_name,
                    (NX_PACKET_POOL *)anet_packet_pool.packet_pool,
                    dhcpv6_server_stack,
                    DHCPV6_SERVER_STACK_SIZE,
                    NX_NULL, NX_NULL);

        if (status)
            aicx_error(0x091f2e, "x", "create dhcpv6 error 0x%x\n", status);
    }

    return status;
}
#endif
#endif

u32 init_net_share_thread(void)
{
    event_net_share = aos_create_event("event_net_share");
    AIC_ASSERT(event_net_share);

    thread_net_share = aos_create_thread("thread_net_share",
                                         net_share_handle_entry,
                                         NULL, SIZE_2K,
                                         0, NULL, 13,
                                         AOS_AUTO_START);
    if (thread_net_share == AOS_INVALID_THREAD_ID) {
        return AOS_INVALID_THREAD_ID;
    }

    return AIC_SUCCESS;
}

s32 aic_network_find_interface_type(NX_INTERFACE *interface_ptr)
{
    u32 interf_index;
    WAN_NET *wan_net_ptr;
#ifdef CONFIG_USB_NET_SHARE
    USB_TETHER_NET_INFO *usb_net_ptr;
#endif
    u8 i;
#ifdef CONFIG_NET_SEMI_TETHER
    struct aic_network_ip_instance *semi_tether_ip;
#endif

    interf_index = interface_ptr->nx_interface_index;

#ifdef CONFIG_USB_NET_SHARE
    usb_net_ptr = aic_get_usb_net_by_lan_ifc(interf_index);
    if (usb_net_ptr) {
        if (strcasestr(interface_ptr->nx_interface_name, "rndis")) {
            return AIC_USB_RNDIS_INTERFACE;
        } else if (strcasestr(interface_ptr->nx_interface_name, "ecm")) {
            return AIC_USB_ECM_INTERFACE;
        } else if (strcasestr(interface_ptr->nx_interface_name, "ncm")) {
            return AIC_USB_NCM_INTERFACE;
        }
    }
#endif

#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.lan_intf_index == interf_index)
        return AIC_PPP_INTERFACE;
#endif

    wan_net_ptr = aic_get_wan_net_by_ifc_index(interf_index);
    if (wan_net_ptr && alte_ip.ip_ptr) {
        for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
            if (interface_ptr == &(alte_ip.ip_ptr->nx_ip_interface[i]))
                return AIC_PDP_INTERFACE;
        }
    }

#ifdef CONFIG_NET_SEMI_TETHER
    semi_tether_ip = aic_get_semi_tether_ip_instance();
    if (semi_tether_ip->ip_ptr) {
        for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
            if (interface_ptr == &(semi_tether_ip->ip_ptr->nx_ip_interface[i]))
                return AIC_USB_SEMI_TETHER_INTERFACE;
        }
    }
#endif

    return -1;
}

/* pdp_active_mode is PDP_ONE_TO_ONE_MODE or PDP_ONE_TO_MULTI_MODE. */
NX_INTERFACE *aic_network_match_interface(NX_INTERFACE *interface_ptr,
                                          u8 *pdp_active_mode)
{
    u32 interf_index;
    WAN_NET *wan_net_ptr;
    u8 pdp_cid, used_cnt = 0;
#if defined  CONFIG_USB_NET_SHARE || defined CONFIG_PPP_DIAL || defined CONFIG_SOFTAP_SHARE
    u32 wan_intf_index;
#endif
#ifdef CONFIG_USB_NET_SHARE
    USB_TETHER_NET_INFO *usb_net_ptr;
    u32 usb_intf_index;
    u8 usb_net_index;
#endif
#ifdef CONFIG_PPP_DIAL
    u32 ppp_intf_index;
#endif

    interf_index = interface_ptr->nx_interface_index;
#ifdef CONFIG_SOFTAP_SHARE
    if (interf_index == softap_net_info.lan_intf_index && softap_net_info.wan_net_ptr) {
        wan_intf_index = softap_net_info.wan_net_ptr->wan_ifc_index;
        aicx_info(0x0b092e, "dd", "softap lan intf index %d,match wan intf index %d\n",
                  interf_index, wan_intf_index);
        if (aic_is_valid_intf_index(wan_intf_index) && alte_ip.ip_ptr)
            return &(alte_ip.ip_ptr->nx_ip_interface[wan_intf_index]);
        else
            return AIC_NULL;
    }
#endif

#ifdef CONFIG_USB_NET_SHARE
    usb_net_ptr = aic_get_usb_net_by_lan_ifc(interf_index);
    /* USB net share is active. */
    if (usb_net_ptr && usb_net_ptr->wan_net_ptr) {
        wan_intf_index = usb_net_ptr->wan_net_ptr->wan_ifc_index;
        if (wan_intf_index < NX_MAX_PHYSICAL_INTERFACES && alte_ip.ip_ptr)
            return &(alte_ip.ip_ptr->nx_ip_interface[wan_intf_index]);
        else
            return AIC_NULL;
    }
#endif

#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.lan_intf_index == interf_index &&
        ppp_dial_net_info.wan_net_ptr) {
        wan_intf_index = ppp_dial_net_info.wan_net_ptr->wan_ifc_index;
        if (wan_intf_index < NX_MAX_PHYSICAL_INTERFACES && alte_ip.ip_ptr)
            return &(alte_ip.ip_ptr->nx_ip_interface[wan_intf_index]);
        else
            return AIC_NULL;
    }
#endif

    wan_net_ptr = aic_get_wan_net_by_ifc_index(interf_index);
    if (!wan_net_ptr) {
        return AIC_NULL;
    }

    pdp_cid = wan_net_ptr->wan_net_id;
#ifdef CONFIG_USB_NET_SHARE
    usb_net_index = aic_get_active_usb_net_by_cid(pdp_cid);
    if (usb_net_index < ANET_USB_NET_MAX_NUM)
        used_cnt++;
#endif

#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.wan_net_id == pdp_cid)
        used_cnt++;
#endif

#ifdef CONFIG_SOFTAP_SHARE
    if (softap_net_info.wan_intf_index == interf_index)
        used_cnt++;
#endif

    if (used_cnt >= 2) {
        *pdp_active_mode = PDP_ONE_TO_MULTI_MODE;
        return AIC_NULL;
    }

#ifdef CONFIG_SOFTAP_SHARE
    if (softap_net_info.wan_intf_index == interf_index) {
        if (softap_net_info.lan_intf_index < NX_MAX_PHYSICAL_INTERFACES && alte_ip.ip_ptr)
            return &(alte_ip.ip_ptr->nx_ip_interface[softap_net_info.lan_intf_index]);
        else
            return AIC_NULL;
    }
#endif

#ifdef CONFIG_USB_NET_SHARE
    if (usb_net_index < ANET_USB_NET_MAX_NUM) {
        usb_intf_index = usb_tether_net_info_list[usb_net_index].lan_intf_index;
        if (usb_intf_index < NX_MAX_PHYSICAL_INTERFACES && alte_ip.ip_ptr)
            return &(alte_ip.ip_ptr->nx_ip_interface[usb_intf_index]);
        else
            return AIC_NULL;
    }
#endif

#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.wan_net_id == pdp_cid) {
        ppp_intf_index = ppp_dial_net_info.lan_intf_index;
        if (ppp_intf_index < NX_MAX_PHYSICAL_INTERFACES && alte_ip.ip_ptr)
            return &(alte_ip.ip_ptr->nx_ip_interface[ppp_intf_index]);
        else
            return AIC_NULL;
    }
#endif

    return AIC_NULL;
}

/** The input para is the NX_INTERFACE of wan net,the output is the
 * interface index of multi network share.
 */
u32 aic_network_get_pdp_peer_interface(NX_INTERFACE *interface_ptr)
{
    u32 pdp_interf_index;
    WAN_NET *wan_net_ptr;
    u32 multi_share_index = 0;
    u8 pdp_cid;
#ifdef CONFIG_USB_NET_SHARE
    u32 usb_intf_index;
    u8 usb_net_index;
#endif
#ifdef CONFIG_PPP_DIAL
    u32 ppp_intf_index;
#endif

    pdp_interf_index = interface_ptr->nx_interface_index;
    wan_net_ptr = aic_get_wan_net_by_ifc_index(pdp_interf_index);
    if (!wan_net_ptr) {
        return multi_share_index;
    }

    pdp_cid = wan_net_ptr->wan_net_id;
#ifdef CONFIG_USB_NET_SHARE
    usb_net_index = aic_get_active_usb_net_by_cid(pdp_cid);
    if (usb_net_index < ANET_USB_NET_MAX_NUM) {
        usb_intf_index = usb_tether_net_info_list[usb_net_index].lan_intf_index;
        if (aic_is_valid_intf_index(usb_intf_index))
            multi_share_index |= (u32)1 << usb_intf_index;
    }
#endif

#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.wan_net_id == pdp_cid) {
        ppp_intf_index = ppp_dial_net_info.lan_intf_index;
        if (aic_is_valid_intf_index(ppp_intf_index))
            multi_share_index |= (u32)1 << ppp_intf_index;
    }
#endif

#ifdef CONFIG_SOFTAP_SHARE
    if (softap_net_info.wan_intf_index == wan_net_ptr->wan_ifc_index) {
        if (aic_is_valid_intf_index(softap_net_info.lan_intf_index))
            multi_share_index |= (u32)1 << softap_net_info.lan_intf_index;
    }
#endif

    return multi_share_index;
}

u32 aic_get_wan_intf_index(NX_INTERFACE *nx_interface_ptr)
{
    u32 ifc_index = NX_MAX_PHYSICAL_INTERFACES;
#ifdef CONFIG_USB_NET_SHARE
    u8 i;
#endif

    if (!nx_interface_ptr ||
        nx_interface_ptr->nx_interface_index >= NX_MAX_PHYSICAL_INTERFACES)
        return NX_MAX_PHYSICAL_INTERFACES;

    ifc_index = nx_interface_ptr->nx_interface_index;

#ifdef CONFIG_SOFTAP_SHARE
    if (ifc_index == softap_net_info.lan_intf_index) {
        if (softap_net_info.wan_net_ptr)
            return softap_net_info.wan_net_ptr->wan_ifc_index;
    }
#endif

#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.lan_intf_index == ifc_index) {
        if (ppp_dial_net_info.wan_net_ptr)
            return ppp_dial_net_info.wan_net_ptr->wan_ifc_index;
    }
#endif

#ifdef CONFIG_USB_NET_SHARE
    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        if (usb_tether_net_info_list[i].lan_intf_index == ifc_index) {
            if (usb_tether_net_info_list[i].wan_net_ptr)
                return usb_tether_net_info_list[i].wan_net_ptr->wan_ifc_index;
        }
    }
#endif

    return ifc_index;
}

/**
 * The thread is to refresh the port hashtables when the timer expires
 * every 10 seconds,also handle the event of stopping ppp dial etc.
 */
void net_share_handle_entry(void *thread_input)
{
    u32 general_events = 0;
#ifdef CONFIG_PPP_DIAL
    u32 com_id;
#ifdef CONFIG_CUSTOMER_AT
    u8 disconn_urc[30];
    u32 urc_com_id;
#endif
#endif
#ifdef CONFIG_USB_NET_SHARE
    USB_TETHER_NET_INFO *usb_net_ptr = AIC_NULL;
    u8 i, cid;
    boolean to_deact_pdp = AIC_FALSE;
#endif

    while (1) {
        aos_get_event(event_net_share, 0xFFFFFFFF, AOS_OR_CLEAR,
                      &general_events, AOS_WAIT_FOREVER);

        if (general_events & NET_EVENT_PORT_TIMEOUT) {
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
            if (!hashtable_is_empty(ppp_dial_hashtable))
                aic_remove_aging_ports(ppp_dial_hashtable);

            if (!hashtable_is_empty(usb_net_hashtable))
                aic_remove_aging_ports(usb_net_hashtable);
#endif
        }

#ifdef CONFIG_PPP_DIAL
        if (general_events & NET_EVENT_STOP_PPP_DIAL) {
#ifdef CONFIG_CUSTOMER_AT
            /* Transmit urc via the other AT com. */
            if (ppp_dial_net_info.serial_id == AT_USB_AT_PORT_ID &&
                conn_get_uart_at_conid != AIC_ENODEV)
                urc_com_id = conn_get_uart_at_conid;
            else if (ppp_dial_net_info.serial_id == conn_get_uart_at_conid &&
                     conn_get_usb_at_conid != AIC_ENODEV)
                urc_com_id = AT_USB_AT_PORT_ID;
            else
                urc_com_id = ppp_dial_net_info.serial_id;
#endif
            /* Stop ppp dial. */
            aic_ppp_dial_stop(ppp_dial_net_info.serial_id);
#ifdef CONFIG_CUSTOMER_AT
            if (urc_com_id != AT_NO_SERIAL_PORT_ID) {
                aos_memset(disconn_urc, 0, sizeof(disconn_urc));
                strcpy(disconn_urc, "+PPPD: DISCONNECTED");
                aic_transmit_at_cmd_urc(urc_com_id, disconn_urc, strlen(disconn_urc));
                aos_memset(disconn_urc, 0, sizeof(disconn_urc));
                strcpy(disconn_urc, "NO CARRIER");
                aic_transmit_at_cmd_urc(urc_com_id, disconn_urc, strlen(disconn_urc));
            }
#endif
        }

        if (general_events & NET_EVENT_PPP_DATA_MODE) {
            ppp_dial_net_info.dial_com_mode = CONN_PPP_MODE;
            if (ppp_dial_net_info.serial_id == AT_USB_AT_PORT_ID)
                com_id = conn_get_usb_at_conid;
            else
                com_id = ppp_dial_net_info.serial_id;

            conn_at_set_work_mode(com_id, CONN_PPP_MODE);
#ifdef CONFIG_NET_PPP_CODEC
            aic_ppp_codec_start_process(com_id);
#endif
        }
#endif

#ifdef CONFIG_USB_NET_SHARE
        if (general_events & NET_EVENT_STOP_USB_TETHER) {
#ifdef CONFIG_USBX_NETWORK
            for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
                usb_net_ptr = &usb_tether_net_info_list[i];
                cid = usb_net_ptr->wan_net_id;

                if (usb_net_ptr->lan_intf_index < NX_MAX_PHYSICAL_INTERFACES &&
                    (usb_net_ptr->usb_interf_state == AIC_USB_LINK_DOWN ||
                    (!aic_is_pdp_activated(cid) &&
                    usb_net_ptr->pdp_deact_invoked != ANET_USB_PDP_DEACT_INVOKED))) {
                    aicx_info(0x162fe1, "d", "stop usb tether of usb intf index %d\n",
                              usb_net_ptr->lan_intf_index);
                    if (!usb_net_ptr->sent_pdp_deact && aic_is_pdp_activated(cid) &&
                        cid != ANET_DEFAULT_PDP_CID)
                        to_deact_pdp = AIC_TRUE;

                    aic_usbtether_stop(usb_net_ptr, AIC_TRUE, to_deact_pdp);
                }
            }
#endif
        }

        if (general_events & NET_EVENT_AUTO_USB_TETHER) {
#ifdef CONFIG_MISC_SUPPORT
            aic_load_net_nv_cfg();
#else
            init_net_cfg.net_cfg.auto_start_usb_net = 0;
#endif

            if (aic_is_usb_plug_in() && !aic_usb_tether_already_started()) {
#if CONFIG_CM_USB_DIAL_AT || CONFIG_CUSTOMER_AT
#ifdef CONFIG_CUSTOMER_AT
                aic_init_customer_net_cfg(&(init_net_cfg.net_cfg));
                if (init_net_cfg.net_cfg.cus_intf_mode == 0)
                    aic_auto_start_usb_tether(aic_get_cus_dial_ip_mode(), ANET_USB_INTF_NAME_RNDIS);
                else if (init_net_cfg.net_cfg.cus_intf_mode == 1)
                    aic_auto_start_usb_tether(aic_get_cus_dial_ip_mode(), ANET_USB_INTF_NAME_ECM);
#else
#ifdef CONFIG_CM_USB_DIAL_AT
                aic_read_init_mdial_cfg();
                aic_auto_mdialup_start(m_dial_cfg.intf_mode);
#endif
#endif
#else
#ifdef ANET_ENABLE_USB_DONGLE
                aic_dg_auto_usb_dial(ANET_SHARE_MODE_ROUTING, aic_get_dg_usb_intf_mode());
#else
#ifdef ANET_ENABLE_AUTO_USB_DIAL
                aic_auto_start_usb_tether(ANET_SHARE_MODE_ROUTING, ANET_USB_INTF_NAME_RNDIS);
#else
                if (init_net_cfg.net_cfg.auto_start_usb_net == 1)
                    aic_auto_start_usb_tether(ANET_SHARE_MODE_ROUTING, ANET_USB_INTF_NAME_RNDIS);
#endif
#endif
#endif
            }
        }

#ifdef ANET_ENABLE_USB_DONGLE
        if (general_events & NET_EVENT_USB_TETHER_OTHER_MODE) {
            aicx_info(0x099828, "d", "usb dg try intf mode %d\n", aic_get_dg_usb_intf_mode());
            aic_dg_auto_usb_dial(ANET_SHARE_MODE_ROUTING, aic_get_dg_usb_intf_mode());
        }
#endif

        if (general_events & NET_EVENT_USB_TETHER_RETRY) {
            if (!aic_usb_tether_already_started() && aic_is_auto_start_usb_dial()) {
                if (usb_service_get_enum_mode() == RNDIS)
                    aic_auto_start_usb_tether(ANET_SHARE_MODE_ROUTING, ANET_USB_INTF_NAME_RNDIS);
                else if (usb_service_get_enum_mode() == ECM)
                    aic_auto_start_usb_tether(ANET_SHARE_MODE_ROUTING, ANET_USB_INTF_NAME_ECM);
            }
        }
#endif
    }
}

#ifdef CONFIG_USB_NET_SHARE
boolean aic_is_auto_start_usb_dial(void)
{
#if CONFIG_CM_USB_DIAL_AT || CONFIG_CUSTOMER_AT
    return AIC_TRUE;
#endif
#ifdef ANET_ENABLE_USB_DONGLE
    return AIC_TRUE;
#endif
#ifdef ANET_ENABLE_AUTO_USB_DIAL
    return AIC_TRUE;
#endif

    aic_load_net_nv_cfg();
    if (init_net_cfg.net_cfg.auto_start_usb_net == 1)
        return AIC_TRUE;

    return AIC_FALSE;
}
#endif

u8 aic_get_net_share_mode(u32 intf_index)
{
    u8 share_mode = 0xff;
#ifdef CONFIG_USB_NET_SHARE
    USB_TETHER_NET_INFO *usb_net_ptr;

    if (intf_index < NX_MAX_PHYSICAL_INTERFACES) {
        usb_net_ptr = aic_get_usb_net_by_lan_ifc(intf_index);
        if (usb_net_ptr)
            return usb_net_ptr->usb_tether_mode;
    }
#endif

#ifdef CONFIG_PPP_DIAL
    if (intf_index < NX_MAX_PHYSICAL_INTERFACES) {
        if (ppp_dial_net_info.lan_intf_index == intf_index)
            return ppp_dial_net_info.ppp_dial_mode;
    }
#endif

#ifdef CONFIG_SOFTAP_SHARE
    if (intf_index < NX_MAX_PHYSICAL_INTERFACES &&
        softap_net_info.lan_intf_index == intf_index)
            return ANET_SHARE_MODE_ROUTING;
#endif

    return share_mode;
}

void aic_act_net_share_waiting_pdp(void)
{
#ifdef CONFIG_USB_NET_SHARE
    u8 i;
    USB_TETHER_NET_INFO *usb_net_ptr;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net_ptr = &usb_tether_net_info_list[i];
        if (usb_net_ptr->waiting_pdp_act == ANET_WAIT_PDP_ACTIVATE) {
            aic_enable_data_connection(usb_net_ptr->wan_net_id,
                                       AIC_NULL, usb_net_ptr->apn);
        }
    }
#endif

#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.waiting_pdp_act)
        aic_enable_data_connection(ppp_dial_net_info.wan_net_id,
                                   ppp_dial_net_info.pdp_type,
                                   ppp_dial_net_info.apn);

#endif
}

u32 aic_get_lan_ifc_index(u8 *ifc_name)
{
    u32 index = NX_MAX_PHYSICAL_INTERFACES;
    u16 i;
    NX_IP *ip_ptr;
    NX_INTERFACE *nx_interface;

    ip_ptr = (NX_IP *)alte_ip.ip_ptr;

    for(i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        nx_interface = &(ip_ptr->nx_ip_interface[i]);
        if (nx_interface->nx_interface_valid && nx_interface->nx_interface_name) {
            if (strncasecmp(ifc_name, nx_interface->nx_interface_name,
                            strlen(ifc_name)) == 0) {
                /* Find a valid index. */
                index = i;
                break;
            }
        }
    }

    return index;
}

boolean aic_dhcp_server_still_used(void)
{
#ifdef CONFIG_USB_NET_SHARE
    if (!usbtether_all_stopped())
        return AIC_TRUE;
#endif

    return AIC_FALSE;
}

boolean aic_ppp_dial_use_same_nat(u32 wan_intf_index)
{
#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.dial_started == ANET_PPP_DIAL_STARTED &&
        (wan_intf_index == ppp_dial_net_info.wan_interf_index))
        return AIC_TRUE;
#endif

    return AIC_FALSE;
}

boolean aic_softap_use_same_nat(u32 wan_intf_index)
{
#ifdef CONFIG_SOFTAP_SHARE
    if (softap_net_info.wan_intf_index == wan_intf_index)
        return AIC_TRUE;
#endif

    return AIC_FALSE;
}

boolean aic_usb_net_use_same_nat(u32 wan_intf_index)
{
#ifdef CONFIG_USB_NET_SHARE
    if (aic_get_usb_net_by_wan_ifc(wan_intf_index))
        return AIC_TRUE;
#endif

    return AIC_FALSE;
}

boolean aic_is_valid_intf_index(u8 intf_index)
{
    return (intf_index < NX_MAX_PHYSICAL_INTERFACES);
}

/* Start the network share init function. */
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_net_share_init);

