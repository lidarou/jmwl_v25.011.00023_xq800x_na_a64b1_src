/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Usb Network Share Management Component.
 *  This is the management of usb network share. After the dial commands
 *  are received and pdp activated, usb net,lan net and ip addresses
 *  would be configured. Also, DHCP and DHCPV6 Server would be stated
 *  to assign ip addresses for peer.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15b665
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

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
#include "aic_net_share_port_mgr.h"
#endif

#ifdef CONFIG_NET_ALTE_DRIVER
#include <nx_alte_driver.h>
#endif

#ifdef CONFIG_USBX_NETWORK
#include "ux_network_driver.h"
#endif

#ifdef CONFIG_USB_SERVICE
#include <usb_service.h>
#include <rndis.h>
#include <cdc_ecm.h>
#endif

#ifdef CONFIG_NX_NAT
#include <nx_nat.h>
#endif

#ifdef CONFIG_NX_DHCP
#include "nxd_dhcp_client.h"
#include "nxd_dhcp_server.h"
#endif

#ifdef CONFIG_PPP_DIAL
#include "aic_ppp_dial_mgr.h"
#endif

/* Define internal vars. */
static u16 usb_lan_net_id = ANET_USB_LAN_NET_ID_BASE;
#ifdef ANET_ENABLE_USB_DONGLE
static u8 dg_intf_mode = ANET_DIAL_INTF_RNDIS_MODE;
#endif

/* Define internal functions. */
#ifdef CONFIG_NX_DHCP
static u32 prepare_for_config_peer(USB_TETHER_NET_INFO *usb_net);
static u32 set_dhcp_server_paras(USB_TETHER_NET_INFO *usb_net);
#ifdef ENABLE_DHCPV6_SERVER
static u32 set_dhcpv6_server_paras(USB_TETHER_NET_INFO *usb_net);
static u32 start_dhcpv6_for_usb(USB_TETHER_NET_INFO *usb_net_ptr);
#endif
#endif

static s32 usbtether_config_lan(u16 usbnet_index, u16 share_mode);
static u8 get_usb_net_index(u32 ucom_id, u8 *usb_intf, u8 cid);
static s32 create_usbtether_lan_net(u16 usbnet_index);
static s32 config_usb_lan_forwarding(u16 usbnet_index);
static s32 config_usb_lan_routing(u16 usbnet_index);
static void config_usb_lan_ipv6(USB_TETHER_LAN_NET *lan_net, WAN_NET *wan_net);
static void config_usb_lan_ipv4_forwarding(u16 usb_net_index);
static u8 get_usb_net_index_unused(void);
static u32 attach_usb_lan_interface(u16 usb_net_index, u32 *ifc_index);
static s32 set_usb_lan_interface(u16 usb_net_index, u32 *interface_index);
static void clear_usb_net_info(USB_TETHER_NET_INFO *usb_net);
static s32 config_usb_lan(u8 pdp_cid, u8 usb_net_index);
static void set_usb_intf_ipv6_addrs(u16 usb_net_index, u32 lan_interf_index);
static s32 usbtether_pdp_activated(u8 usb_net_index, u8 pdp_cid, u8 *err_index);
static ulong get_proper_suffix_value(ulong base_value);
#ifdef CONFIG_USBX_NETWORK
static void usb_intf_state_callback(u32 intf_id, u32 intf_state);
#endif
#ifdef CONFIG_NX_NAT
static u32 config_usb_interf_nat(u16 usb_net_index);
static void usb_nat_cache_full_callback(NX_NAT_DEVICE *nat_ptr);
#endif

/*Define global vars. */
aos_mutex_ptr usb_net_mutex;
USB_TETHER_NET_INFO usb_tether_net_info_list[ANET_USB_NET_MAX_NUM];

/* Define external vars. */
extern aos_mutex_ptr pdp_info_mutex;
extern aos_mutex_ptr wan_net_mutex;
extern struct aic_network_ip_instance alte_ip;
extern struct aic_packet_pool anet_packet_pool;
extern NET_PDP_INFO net_pdp_info_list[ANET_PDP_MAX_NUM];
extern WAN_NET wan_net_list[ANET_PDP_MAX_NUM];
extern aos_event_ptr event_net_share;
extern AT_CMD_HANDLE usb_share_at_handles[];
#ifdef CONFIG_USB_SERVICE
extern u8 current_usb_mode;
extern u8 old_usb_mode;
#endif
#ifdef CONFIG_NX_DHCP
#ifdef ENABLE_DHCPV6_SERVER
extern NX_DHCPV6_SERVER *dhcpv6_server;
extern u8 *dhcpv6_server_stack;
#endif
extern NX_DHCP_SERVER *dhcp_server;
extern u8 *server_dhcp_stack;
extern u8 dhcp_server_name[20];
extern aos_mutex_ptr dhcp_server_mutex;
#endif
#ifdef CONFIG_NX_NAT
extern NX_NAT_DEVICE *nat_dev_list[NX_MAX_PHYSICAL_INTERFACES];
#endif
#ifdef CONFIG_CUSTOMER_AT
extern CUSTOMER_USB_NET_CFG customer_usb_net_cfg;
#endif
#ifdef CONFIG_CM_USB_DIAL_AT
extern CUSTOMER_M_DIAL_CFG m_dial_cfg;
#endif

u32 aic_usbtether_init(void)
{
    u8 i;

    aic_add_at_cmd_handle(&usb_share_at_handles[0]);
    usb_net_mutex = aos_create_mutex("usb_net_mutex");
    AIC_ASSERT(usb_net_mutex);

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        aos_memset(&usb_tether_net_info_list[i], 0, sizeof(USB_TETHER_NET_INFO));
        usb_tether_net_info_list[i].ucom_channel_id = 0xffff;
        usb_tether_net_info_list[i].lan_intf_index = 0xff;
        usb_tether_net_info_list[i].wan_intf_index = 0xff;
        usb_tether_net_info_list[i].usb_interf_id = 0xff;
        aos_memset(usb_tether_net_info_list[i].usb_interfce, '\0',
                   sizeof(usb_tether_net_info_list[i].usb_interfce));
    }

#ifdef CONFIG_CM_USB_DIAL_AT
    aic_read_init_mdial_cfg();
#endif

#ifdef CONFIG_USBX_NETWORK
    aic_usb_register_link_notify_callback(usb_intf_state_callback);
#endif

    return AIC_SUCCESS;
}

boolean aic_is_usb_plug_in(void)
{
#ifdef CONFIG_USB_SERVICE
    /* basic mode can't use usb. */
    if (aic_boot_mode_is_basic())
        return AIC_FALSE;

    return usb_get_connect_status();
#else
    return AIC_FALSE;
#endif
}

boolean aic_usb_tether_already_started(void)
{
    u8 i;

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        if (aic_is_valid_pdp_cid(usb_tether_net_info_list[i].wan_net_id)) {
            aos_put_mutex(usb_net_mutex);
            return AIC_TRUE;
        }
    }

    aos_put_mutex(usb_net_mutex);
    return AIC_FALSE;
}

#ifdef CONFIG_USBX_NETWORK
/** Be informed about the usb inteface state is up or down.
 * Especially unplug USB when usb network share is on.
 */
void usb_intf_state_callback(u32 intf_id, u32 intf_state)
{
    u8 usb_net_index;
    USB_TETHER_NET_INFO *usb_net_ptr = AIC_NULL;

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    usb_net_index = aic_get_usb_net_by_intf_id(intf_id);
    aos_put_mutex(usb_net_mutex);

    if (usb_net_index < ANET_USB_NET_MAX_NUM) {
        usb_net_ptr = &usb_tether_net_info_list[usb_net_index];
        usb_net_ptr->usb_interf_state = intf_state;
        aicx_info(0x1ca48b, "dd", "usb intf id %d set to state %d\n", intf_id, intf_state);

        if (intf_state == AIC_USB_LINK_DOWN)
            aos_set_event(event_net_share, NET_EVENT_STOP_USB_TETHER, AOS_OR);
    } else {
        /* USB intf can be down due to exception, and usb tether is stopped.
         * Then it may be up again due to usb reset,so retry usb tether. */
        if (intf_state == AIC_USB_LINK_UP)
            aos_set_event(event_net_share, NET_EVENT_USB_TETHER_RETRY, AOS_OR);
    }
}
#endif

static boolean usb_net_interf_switched(u8 *new_intf_str)
{
    u8 i;
    USB_TETHER_NET_INFO *usb_net_ptr;

    if (!new_intf_str)
        return AIC_FALSE;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net_ptr = &usb_tether_net_info_list[i];
        if (strlen(usb_net_ptr->usb_interfce) > 0) {
            if (strncasecmp(usb_net_ptr->usb_interfce, new_intf_str,
                strlen("ecm")) != 0)
                return AIC_TRUE;
        }
    }

    return AIC_FALSE;
}

#ifdef ANET_ENABLE_USB_DONGLE
/* Wait up to 2800ms for checking if receive rndis query info from peer. */
boolean aic_wait_usb_rndis_ready(void)
{
    u32 wait_time = 0;

    while (wait_time < ANET_WAIT_RNDIS_READY_TIME) {
        if (usb_tether_net_info_list[0].rndis_ready == 1)
            return AIC_TRUE;
        else {
            aos_sleep(100);
            wait_time += 100;
        }
    }

    return usb_tether_net_info_list[0].rndis_ready == 1;
}

void aic_usb_intf_ready_notified(u32 is_ready)
{
    u8 i;

    if (is_ready != 0)
        return;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_tether_net_info_list[i].rndis_ready = 1;
        aicx_info(0x015225, "", "usb intf rndis is ready\n");
    }
}
#endif

/** It is called after the AT cmd below is received from serial ports.
 * AT^USBTETHER=1,1,"rndis","tethering"
 */
s32 aic_usbtether_start(u32 ucom_channel_id, u8 *usb_intf, u8 share_mode,
                        u8 cid, u8 *pdp_type, u8 *apn)
{
    u8 i;
    u8 usb_net_index;
    s32 status = AIC_SUCCESS;
    USB_TETHER_NET_INFO *usb_net_ptr;
    boolean to_deact_pdp = AIC_FALSE;

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    /* Check if already started. Or just check usb interface instead? */
    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        if (strncasecmp(usb_intf, usb_tether_net_info_list[i].usb_interfce,
                        strlen(usb_intf)) == 0) {
            aos_put_mutex(usb_net_mutex);
            return ANET_ERROR_INTF_ENGAGED;
        }
    }

    if (usb_net_interf_switched(usb_intf)) {
        aos_put_mutex(usb_net_mutex);
        for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
            usb_net_ptr = &usb_tether_net_info_list[i];
            if (strlen(usb_net_ptr->usb_interfce) > 0) {
                if (!usb_net_ptr->sent_pdp_deact && aic_is_pdp_activated(cid) &&
                    cid != ANET_DEFAULT_PDP_CID)
                    to_deact_pdp = AIC_TRUE;

                aicx_info(0x1e274a, "s", "switch usb intf,stop %s\n", usb_net_ptr->usb_interfce);
                aic_usbtether_stop(usb_net_ptr, AIC_FALSE, to_deact_pdp);
            }
        }
        aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    }

    usb_net_index = get_usb_net_index_unused();
    if (usb_net_index >= ANET_USB_NET_MAX_NUM) {
        aos_put_mutex(usb_net_mutex);
        return ANET_ERROR_USB_NET_OVERFLOW;
    }

    usb_net_ptr = &usb_tether_net_info_list[usb_net_index];
    usb_net_ptr->wan_net_id = cid;
    usb_net_ptr->lan_net_id = usb_lan_net_id++;
    usb_net_ptr->ucom_channel_id = ucom_channel_id;
    strncpy(usb_net_ptr->usb_interfce, usb_intf,
            (sizeof(usb_net_ptr->usb_interfce) - 1));
    usb_net_ptr->usb_interf_id = atoi(usb_intf);
#ifdef CONFIG_USBX_NETWORK
    usb_net_ptr->usb_interf_state = AIC_USB_LINK_UP;
#endif
    usb_net_ptr->usb_tether_mode = share_mode;

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
    usb_net_ptr->port_range_start = ANET_USBSHARE_PORT_START;
    usb_net_ptr->port_range_end = ANET_USBSHARE_PORT_END;
#endif

    if (pdp_type) {
        strncpy(usb_net_ptr->pdp_type, pdp_type, (ANET_PDP_TYPE_MAX_LEN - 1));
    }
    if (apn) {
        strncpy(usb_net_ptr->apn, apn, (ANET_APN_NAME_MAX_LEN - 1));
    }

    usb_net_ptr->lan_net_ip = (NX_IP *)alte_ip.ip_ptr;

    /* Maybe PDP has already activated. */
    if (aic_is_pdp_activated(cid)) {
        status = usbtether_pdp_activated(usb_net_index, cid, AIC_NULL);
        if (status) {
            aicx_error(0x04d06d, "x", "config lan error,stop 0x%x\n", status);
            aos_put_mutex(usb_net_mutex);
            aic_usbtether_stop(usb_net_ptr, AIC_FALSE, AIC_FALSE);
            return status;
        }
    } else {
        usb_net_ptr->waiting_pdp_act = ANET_WAIT_PDP_ACTIVATE;
        if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_ROUTING)
            status = config_usb_lan(cid, usb_net_index);
    }

    aos_put_mutex(usb_net_mutex);

    return AIC_SUCCESS;
}

/* Use default pdp and rndis0. */
s32 aic_auto_start_usb_tether(u8 tether_mode, u8 *intf_name)
{
    u16 usb_intf_up = NX_FALSE;
    u8 cid = ANET_DEFAULT_PDP_CID;
    u32 status = AIC_SUCCESS;
    USB_TETHER_NET_INFO *usb_net = AIC_NULL;
    u32 serial_port_id = AT_NO_SERIAL_PORT_ID;
    u8 usb_intf[ANET_MAX_LEN_INTERF_NAME] = {0};

    strncpy(usb_intf, intf_name, (ANET_MAX_LEN_INTERF_NAME - 1));

#ifdef ANET_ENABLE_USB_DONGLE
TRY_ONE_INTF:
#endif
    aicx_info(0x13022a, "s", "auto usb tether for intf %s\n", usb_intf);
    aic_wait_usb_intf_up(usb_intf, &usb_intf_up);
    if (!usb_intf_up) {
        aicx_error(0x162301, "", "error,can't make usb intf up\n");
        return ANET_ERROR_USB_INTF_DOWN;
    }

#ifdef ANET_ENABLE_USB_DONGLE
    if (strcasestr(usb_intf, "rndis")) {
        if (!aic_wait_usb_rndis_ready()) {
            usb_intf_up = NX_FALSE;
            aicx_info(0x0e4025, "", "intf is not rndis,retry ecm\n");
            aos_memset(usb_intf, 0, ANET_MAX_LEN_INTERF_NAME);
            strncpy(usb_intf, ANET_USB_INTF_NAME_ECM, (ANET_MAX_LEN_INTERF_NAME - 1));
            goto TRY_ONE_INTF;
        }
    }
#endif

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
    if (cid == ANET_DEFAULT_PDP_CID && tether_mode == ANET_SHARE_MODE_FORWARDING) {
        status = aic_init_net_port_hashtable(ANET_NETSHARE_TYPE_USB, tether_mode);
        if (status) {
            aicx_error(0x0f54e7, "", "init usb net hashtable error\n");
            return ANET_ERROR_HASH_NOT_INIT;
        }
    }
#endif

    /* USB intf maybe change due to self adaptation. */
    if (usb_service_get_enum_mode() == RNDIS)
        strcpy(usb_intf, ANET_USB_INTF_NAME_RNDIS);
    if (usb_service_get_enum_mode() == ECM)
        strcpy(usb_intf, ANET_USB_INTF_NAME_ECM);

    status = aic_usbtether_start(serial_port_id, usb_intf, tether_mode,
                                 cid, AIC_NULL, AIC_NULL);
    if (status)
        return status;

    usb_net = aic_get_usb_net_by_cid(cid);
    if (!usb_net)
        return ANET_ERROR_NULL_USB_NET;
    else {
        usb_net->auto_started = 1;
        if (!aic_is_pdp_activated(cid)) {
            /* Send attach cmd for LTE.PDP will be activated after attach. */
            if (!aic_is_ps_registered())
                aic_attach_packet_domain();
            else
                aic_enable_data_connection(cid, AIC_NULL, AIC_NULL);

            if (tether_mode == ANET_SHARE_MODE_FORWARDING)
                usb_net->forward_mode_start_time = aos_get_tickcount();
        }
    }

    return AIC_SUCCESS;
}

/** It is called when PDP is activated,because maybe the USB net share
 * is waiting for PDP activation.
 */
s32 aic_notify_usbtether_pdp_activated(u8 cid)
{
    s32 status = AIC_SUCCESS;
    /* The usb net index which is configured error. */
    u8 error_index = 0xff;
    USB_TETHER_NET_INFO *usb_net;
    boolean to_deact_pdp = AIC_FALSE;

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    status = usbtether_pdp_activated(0xff, cid, &error_index);
    aos_put_mutex(usb_net_mutex);
    if (status) {
        if (error_index < ANET_USB_NET_MAX_NUM) {
            aicx_error(0x0e391f, "", "usbtether config error,stop\n");
            usb_net = &usb_tether_net_info_list[error_index];
            if (!usb_net->sent_pdp_deact && aic_is_pdp_activated(cid))
                to_deact_pdp = AIC_TRUE;

            aic_usbtether_stop(usb_net, AIC_FALSE, to_deact_pdp);
        }
    }

    return status;
}

/* Config the usb lan net after PDP is activated. */
s32 usbtether_pdp_activated(u8 usb_net_index, u8 pdp_cid, u8 *err_index)
{
    u32 status = AIC_SUCCESS;
    u8 i, index = 0xff;
    USB_TETHER_NET_INFO *usb_net;
    WAN_NET *wan_net;
    USB_TETHER_LAN_NET *lan_net;
    int waiting_time = 0;

    if (usb_net_index < ANET_USB_NET_MAX_NUM) {
        index = usb_net_index;
    } else {
        for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
            if (usb_tether_net_info_list[i].wan_net_id == pdp_cid) {
                index = i;
                break;
            }
        }
    }

    if (index == 0xff)
        return AIC_SUCCESS;

    usb_net = &usb_tether_net_info_list[index];
    usb_net->sent_pdp_deact = ANET_NOT_SENT_PDP_DEACT;
    /* Disconnect usb intf first,in case PC doesn't try to get ip due to timeout. */
    if (usb_net->usb_tether_mode == ANET_SHARE_MODE_FORWARDING &&
        usb_net->pdp_deact_invoked != ANET_USB_PDP_DEACT_INVOKED &&
        !aic_usb_intf_single_get_state(usb_net->usb_interfce)) {
        if (usb_net->forward_mode_start_time > 0) {
            waiting_time = aos_get_tickcount() - usb_net->forward_mode_start_time;
            usb_net->forward_mode_start_time = 0;
        }

        if (waiting_time > ANET_USB_NET_WAIT_PDP_TIME) {
            /* Disconnect usb intf first in case PC timeout. */
            aic_disconnect_usb_intf(usb_net->usb_interfce);
            aos_sleep(10);
        }
    }

    if (usb_net->pdp_deact_invoked == ANET_USB_PDP_DEACT_INVOKED) {
        usb_net->pdp_deact_invoked = ANET_USB_PDP_DEACT_NOT_INVOKED;
        status = config_usb_lan(pdp_cid, index);
    } else {
        wan_net = aic_get_wan_net_per_pdp(pdp_cid);
        lan_net = usb_net->lan_net_ptr;
        if (wan_net && lan_net) {
            if (usb_net->usb_tether_mode == ANET_SHARE_MODE_FORWARDING &&
                wan_net->wan_ipv4_address != lan_net->ipv4_addr_pool_start &&
                lan_net->ipv4_addr_pool_start != 0) {
                aicx_info(0x040075, "", "wan net for usb share is old\n");
                return AIC_SUCCESS;
            }
        }

        /* Maybe should config usb tether when it is started. */
        status = config_usb_lan(pdp_cid, index);
    }

    if (!status && aic_usb_intf_single_get_state(usb_net->usb_interfce)) {
        aic_connect_usb_intf(usb_net->usb_interfce);
    }

    if (status && err_index) {
        *err_index = index;
    }

    return status;
}

s32 config_usb_lan(u8 pdp_cid, u8 usb_net_index)
{
    u8 share_mode;
    s32 ret = AIC_SUCCESS;
    WAN_NET *wan_net;
    USB_TETHER_NET_INFO *usb_net_ptr;

    usb_net_ptr = &usb_tether_net_info_list[usb_net_index];
    /* Config wan_net_ptr first. */
    if (!usb_net_ptr->wan_net_ptr) {
        wan_net = aic_get_wan_net_per_pdp(pdp_cid);
        if (wan_net) {
            usb_net_ptr->wan_net_ptr = wan_net;
            usb_net_ptr->wan_intf_index = wan_net->wan_ifc_index;
        }
    }

    usb_net_ptr->lan_net_ip = (NX_IP *)alte_ip.ip_ptr;
    share_mode = usb_net_ptr->usb_tether_mode;
    ret = usbtether_config_lan(usb_net_index, share_mode);
    if (ret) {
        return ret;
    }

#ifdef CONFIG_NX_DHCP
    prepare_for_config_peer(usb_net_ptr);
#endif

    return ret;
}

/* aic_set_pdp_deactivated should be called first when PDP is deactivated.*/
u32 aic_usbtether_pdp_deactivated(u16 pdp_cid)
{
    USB_TETHER_NET_INFO *usb_net_ptr;
    u8 i;

    /* Clear wan net and lan net relating to the pdp. */
    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net_ptr = &usb_tether_net_info_list[i];
        if (usb_net_ptr->wan_net_id == pdp_cid) {
            usb_net_ptr->pdp_deact_invoked = ANET_USB_PDP_DEACT_INVOKED;
            usbtether_deact_invoked(usb_net_ptr);
        }
    }

    return AIC_SUCCESS;
}

void aic_notify_usb_tether_ipv6_change(u8 pdp_cid)
{
    USB_TETHER_NET_INFO *usb_net_ptr;
    USB_TETHER_LAN_NET *usb_lan_net;
    u8 i;
    u32 status;

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        /** wan_net_ptr is not NULL,that is to say the usb lan net
         * has already been configured. So not configure again.
         */
        usb_net_ptr = &usb_tether_net_info_list[i];
        if (pdp_cid != usb_net_ptr->wan_net_id) {
            continue;
        }

        usb_lan_net = usb_net_ptr->lan_net_ptr;
        if (usb_lan_net && aic_is_valid_ipv6_addr(usb_lan_net->lan_global_ipv6_addr)) {
            continue;
        } else {
            status = config_usb_lan(pdp_cid, i);
            if (status) {
                aicx_error(0x10e6cd, "x", "config ipv6 for usb lan error 0x%x\n", status);
            }
        }
    }

    aos_put_mutex(usb_net_mutex);
}

u8 get_usb_net_index(u32 ucom_id, u8 *usb_intf, u8 cid)
{
    u8 i, index = ANET_USB_NET_MAX_NUM;

    /* Maybe can't input usb interface. */
    if (!usb_intf || strlen(usb_intf) == 0) {
        for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
            if (usb_tether_net_info_list[i].wan_net_id == cid &&
                ucom_id == usb_tether_net_info_list[i].ucom_channel_id) {
                index = i;
            }
        }
    } else {
        for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
            if (strncasecmp(usb_intf, usb_tether_net_info_list[i].usb_interfce,
                strlen(usb_intf)) == 0 &&
                ucom_id == usb_tether_net_info_list[i].ucom_channel_id &&
                usb_tether_net_info_list[i].wan_net_id == cid) {
                index = i;
            }
        }
    }

    return index;
}

u8 aic_get_active_usb_net_by_cid(u8 pdp_cid)
{
    u8 i, index = ANET_USB_NET_MAX_NUM;
    WAN_NET *wan_net;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        wan_net = usb_tether_net_info_list[i].wan_net_ptr;
        if (wan_net && wan_net->pdp_cid == pdp_cid) {
            index = i;
            break;
        }
    }

    return index;
}

/* Get the usb net info using the pdp_cid,the pdp may be activated or not. */
USB_TETHER_NET_INFO *aic_get_usb_net_per_cid_com(u8 pdp_cid, u32 serial_port,
                                                 u8 *usb_net_index)
{
    u8 i;
    USB_TETHER_NET_INFO *usb_net;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net = &usb_tether_net_info_list[i];
        if (usb_net->wan_net_id == pdp_cid &&
            serial_port == usb_net->ucom_channel_id) {
            *usb_net_index = i;
            return usb_net;
        }
    }

    return AIC_NULL;
}

USB_TETHER_NET_INFO *aic_get_usb_net_by_cid(u8 pdp_cid)
{
    u8 i;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        if (usb_tether_net_info_list[i].wan_net_id == pdp_cid)
            return &usb_tether_net_info_list[i];
    }

    return AIC_NULL;
}

USB_TETHER_NET_INFO *aic_get_usb_net_by_wan_ifc(u32 ifc_index)
{
    u8 i;
    WAN_NET *wan_net;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        wan_net = usb_tether_net_info_list[i].wan_net_ptr;
        if (wan_net && wan_net->wan_ifc_index == ifc_index) {
            return &usb_tether_net_info_list[i];
        }
    }

    return AIC_NULL;
}

USB_TETHER_NET_INFO *aic_get_usb_net_by_lan_ifc(u16 usb_ifc_index)
{
    u8 i;
    USB_TETHER_NET_INFO *usb_net_ptr;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net_ptr = &usb_tether_net_info_list[i];
        if (usb_net_ptr->lan_intf_index == usb_ifc_index) {
            return usb_net_ptr;
        }
    }

    return AIC_NULL;
}

USB_TETHER_NET_INFO *aic_get_usb_net_waiting_pdp(u8 pdp_cid)
{
    u8 i;
    USB_TETHER_NET_INFO *usb_net_ptr;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net_ptr = &usb_tether_net_info_list[i];
        if (usb_net_ptr->wan_net_id == pdp_cid &&
            usb_net_ptr->waiting_pdp_act == ANET_WAIT_PDP_ACTIVATE) {
            return usb_net_ptr;
        }
    }

    return AIC_NULL;
}

s32 usbtether_config_lan(u16 usbnet_index, u16 share_mode)
{
    s32 status = AIC_SUCCESS;
    u32 lan_intf_index = NX_MAX_PHYSICAL_INTERFACES;

    if (share_mode == ANET_SHARE_MODE_FORWARDING) {
        status = config_usb_lan_forwarding(usbnet_index);
    } else if (share_mode == ANET_SHARE_MODE_ROUTING){
        status = config_usb_lan_routing(usbnet_index);
    }
    if (status) {
        return status;
    }

    status = set_usb_lan_interface(usbnet_index, &lan_intf_index);
    if (status || lan_intf_index >= NX_MAX_PHYSICAL_INTERFACES) {
        //aos_free((u8 *)usb_tether_net_info_list[usbnet_index].lan_net_ptr);
        return status;
    }

    return status;
}

#ifdef CONFIG_NX_DHCP
static s32 start_dhcp_server_for_usb(USB_TETHER_NET_INFO *usb_net)
{
    u32 status = AIC_SUCCESS;

    if (!usb_net || !usb_net->lan_net_ptr) {
        return ANET_ERROR_NULL_USB_NET;
    }

    status = set_dhcp_server_paras(usb_net);
    if (status) {
        nx_dhcp_server_delete(dhcp_server);
        aicx_error(0x1bbe62, "x", "set dhcp paras error 0x%x\n", status);
        goto CONFIG_ERROR;
    }

    status = nx_dhcp_server_start(dhcp_server);
    if (status) {
        nx_dhcp_server_delete(dhcp_server);
        aicx_error(0x187ab4, "x", "start DHCP server error 0x%x\n", status);
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

/* Config DHCP Server and DHCPV6 Server to assign ip addrs for peer. */
u32 prepare_for_config_peer(USB_TETHER_NET_INFO *usb_net)
{
#ifdef ENABLE_DHCPV6_SERVER
    WAN_NET *wan_net;
#endif
    USB_TETHER_LAN_NET *lan_net;
    u32 status = AIC_SUCCESS;

    if (!usb_net || !usb_net->lan_net_ptr)
        return ANET_ERROR_NULL_USB_NET;

    /* Start DHCP Server. */
    lan_net = usb_net->lan_net_ptr;
    if (usb_net->dhcp_configured != ANET_DHCP_CONFIGURED && lan_net->lan_ipv4_address) {
        if (!dhcp_server_mutex) {
            dhcp_server_mutex = aos_create_mutex("dhcp_server_mutex");
            AOS_ASSERT(dhcp_server_mutex);
        }

        aos_get_mutex(dhcp_server_mutex, AOS_WAIT_FOREVER);
        if (dhcp_server && dhcp_server->nx_dhcp_started) {
            status = set_dhcp_server_paras(usb_net);
        } else {
            status = aic_init_dhcp_server();
            if (!status)
                status = start_dhcp_server_for_usb(usb_net);
        }
        aos_put_mutex(dhcp_server_mutex);

        if (!status)
            usb_net->dhcp_configured = ANET_DHCP_CONFIGURED;
    }

#ifdef ENABLE_DHCPV6_SERVER
    /** Start DHCPv6 Server if has got global ipv6 addr.
     * Maybe there is no global ipv6 addr now.
     * Then DHCPv6 server would be statrted when we are notified
     * by ipv6 auto config complete.
     */
    wan_net = usb_net->wan_net_ptr;
    if (!usb_net->dhcpv6_started && wan_net &&
        aic_is_valid_ipv6_addr(wan_net->wan_ipv6_address1)) {
        if (dhcpv6_server) {
            aicx_info(0x014f1c, "", "DHCPv6 Server enaged\n");
        } else {
            status = start_dhcpv6_for_usb(usb_net);
            if (!status) {
                usb_net->dhcpv6_started = ANET_DHCPV6_STARTED;
            } else {
                aicx_error(0x0ed9ed, "x", "usbtether start DHCPv6 error--0x%x\n", status);
                usb_net->dhcpv6_started = ANET_DHCPV6_NOT_STARTED;
            }
        }
    }
#endif

    return status;
}

#ifdef ENABLE_DHCPV6_SERVER
u32 start_dhcpv6_for_usb(USB_TETHER_NET_INFO *usb_net_ptr)
{
    u32 status;

    status = aic_init_dhchv6_server();
    if (status)
        goto DHCPV6_ERROR;

    status = set_dhcpv6_server_paras(usb_net_ptr);
    if (status) {
        nx_dhcpv6_server_delete(dhcpv6_server);
        aicx_error(0x12bd6e, "x", "usbtether set dhcpv6 paras error 0x%x\n", status);
        goto DHCPV6_ERROR;
    }

    status = nx_dhcpv6_server_start(dhcpv6_server);
    if (status) {
        nx_dhcpv6_server_delete(dhcpv6_server);
        aicx_error(0x1d846f, "x", "usbtether start dhcpv6 error 0x%x\n", status);
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

u32 set_dhcpv6_server_paras(USB_TETHER_NET_INFO *usb_net)
{
    u32 status, addresses_added, ifc_index, i;
    NXD_ADDRESS dns_ipv6_address;
    NXD_ADDRESS start_ipv6_address;
    NXD_ADDRESS end_ipv6_address;
    NXD_ADDRESS lan_net_ipv6_address;
    USB_TETHER_LAN_NET *lan_net;
    WAN_NET *wan_net;
    NX_INTERFACE *net_intf_ptr;
    aos_systime systime;
    s32 local_tz, tmp_tz;

    ifc_index = usb_net->lan_intf_index;
    lan_net = usb_net->lan_net_ptr;
    wan_net = usb_net->wan_net_ptr;

    lan_net_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6;
    for (i = 0; i < 4; i++) {
        lan_net_ipv6_address.nxd_ip_address.v6[i] =
            lan_net->lan_global_ipv6_addr[i];
    }

    //aic_get_ipv6_addr_index(&alte_ip, &lan_net_ipv6_address, &addr_index);
    if (lan_net->global_ipv6_addr_index < NX_MAX_IPV6_ADDRESSES) {
        /** Set the network interface and global ipv6 address index,
         * if there are more than 1 global ipv6 addresses.
         */
        status = nx_dhcpv6_server_interface_set(dhcpv6_server,
                                                ifc_index,
                                                lan_net->global_ipv6_addr_index);
        if (status)
            aicx_error(0x142ad6, "x", "set addr for dhcpv6 error 0x%x\n", status);
    }

    /* Set up the DNS IPv6 server address. */
    dns_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    /* Copy the dns addr for wan_net. */
    for (i = 0; i < 4; i++) {
        dns_ipv6_address.nxd_ip_address.v6[i] =
            usb_net->wan_net_ptr->wan_ipv6_dns1_address[i];
    }
    status = nx_dhcpv6_create_dns_address(dhcpv6_server,
                                          &dns_ipv6_address);
    if (status) {
        aicx_error(0x0303f0, "x", "create dns for dhcpv6 error 0x%x\n", status);
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
    systime.day = 25;
    systime.hour = 7;
    systime.minute = 6;
    systime.second = 0;
#endif
    /* Actually,get system time this way. */
    aos_get_systime(&systime);
    local_tz = aic_get_current_time_zone();
    aicx_info(0x10b93c, "d", "set dhcpv6:local tz is %d\n", local_tz);
    tmp_tz = local_tz;
    local_tz = local_tz - (2 * tmp_tz);
    /* Ajust the time to time with time zone 0. */
    aic_adjust_time_with_time_zone(&systime, local_tz * 4);
    systime.year -= 30;
    usb_net->dhcpv6_duid = aic_mktime_to_seconds(&systime);
    net_intf_ptr = &alte_ip.ip_ptr->nx_ip_interface[ifc_index];

    status = nx_dhcpv6_set_server_duid(
             dhcpv6_server,
             NX_DHCPV6_SERVER_DUID_TYPE, NX_DHCPV6_SERVER_HW_TYPE,
             net_intf_ptr->nx_interface_physical_address_msw,
             net_intf_ptr->nx_interface_physical_address_lsw,
             usb_net->dhcpv6_duid);

    if (status) {
        aicx_error(0x129798, "x", "set duid error 0x%x\n", status);
    }

    /* Set the IPv6 start address. */
    start_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    for (i = 0; i < 4; i++) {
        start_ipv6_address.nxd_ip_address.v6[i] =
            lan_net->ipv6_addr_pool_start[i];
    }

    /* Set the IPv6 end address. */
    end_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    for (i = 0; i < 4; i++) {
        end_ipv6_address.nxd_ip_address.v6[i] =
            lan_net->ipv6_addr_pool_end[i];
    }

    /* Set the IPv6 address range. */
    status = nx_dhcpv6_create_ip_address_range(dhcpv6_server,
                                               &start_ipv6_address,
                                               &end_ipv6_address,
                                               &addresses_added);

    if (status)
        aicx_error(0x008604, "x", "create ip range for dhcpv6 error 0x%x\n", status);

    return status;
}
#endif

u32 set_dhcp_server_paras(USB_TETHER_NET_INFO *usb_net)
{
    u32 status = AIC_SUCCESS, addresses_added;
    USB_TETHER_LAN_NET *lan_net;

    if (!usb_net || !dhcp_server) {
        return ANET_ERROR_NULL_USB_NET;
    }

    lan_net = usb_net->lan_net_ptr;
    /* Load the assignable DHCP IP addresses for the interface. */
    status = nx_dhcp_create_server_ip_address_list(
             dhcp_server,
             usb_net->lan_intf_index,
             usb_net->lan_net_ptr->ipv4_addr_pool_start,
             usb_net->lan_net_ptr->ipv4_addr_pool_end,
             &addresses_added);

    /* Check for errors creating the list. */
    if (status)
        return status;

    /* Set the interface network parameters. */
    status = nx_dhcp_set_interface_network_parameters(
             dhcp_server,
             usb_net->lan_intf_index,
             usb_net->lan_net_ptr->lan_ipv4_subnet_mask,
             usb_net->lan_net_ptr->lan_ipv4_address,
             usb_net->lan_net_ptr->lan_ipv4_dns1_address);

    return status;
}
#endif

void clear_usb_net_info(USB_TETHER_NET_INFO *usb_net)
{
    if (!usb_net)
        return;

    aos_memset((u8 *)usb_net, 0, sizeof(USB_TETHER_NET_INFO));
    usb_net->lan_intf_index = 0xff;
    usb_net->wan_intf_index = 0xff;
    usb_net->ucom_channel_id = 0xffff;
    usb_net->usb_interf_id = 0xff;
    aos_memset(usb_net->usb_interfce, '\0', sizeof(usb_net->usb_interfce));
}

void config_usb_lan_ipv4_forwarding(u16 usb_net_index)
{
    USB_TETHER_LAN_NET *lan_net;
    WAN_NET *wan_net;
    u8 last_byte;

    wan_net = usb_tether_net_info_list[usb_net_index].wan_net_ptr;
    lan_net = usb_tether_net_info_list[usb_net_index].lan_net_ptr;

    if (wan_net && wan_net->wan_ipv4_address) {
        /* This is a fake addr in ndis mode. */
        last_byte = (u8)(wan_net->wan_ipv4_address & 0xff);
        if (last_byte < 254)
            lan_net->lan_ipv4_address = wan_net->wan_ipv4_address + 1;
        else
            lan_net->lan_ipv4_address = wan_net->wan_ipv4_address - 1;

        /* In ndis and tethering mode, the subnet mask are both 0xffffff00. */
        lan_net->lan_ipv4_subnet_mask = ANET_LAN_NET_MASK_ROUTING;
        lan_net->ipv4_addr_pool_size = 1;
        lan_net->ipv4_addr_pool_start = wan_net->wan_ipv4_address;
        lan_net->ipv4_addr_pool_end = wan_net->wan_ipv4_address;
        lan_net->lan_ipv4_dns1_address = wan_net->wan_ipv4_dns1_address;
        lan_net->lan_ipv4_dns2_address = wan_net->wan_ipv4_dns2_address;
    }

    /* If PDP has no ipv4 addr, then don't assign ipv4 addr to uper devices. */
    #if 0
    else if(lan_net->lan_ipv4_address == 0 && !wan_net->wan_ipv4_address) {
        /* If no ipv4 addr, then no dns addr. */
        lan_net->lan_ipv4_address = ANET_USB_LAN_IPV4_ADDR + usb_net_index;
        lan_net->lan_ipv4_subnet_mask = ANET_LAN_NET_MASK_ROUTING;
        lan_net->ipv4_addr_pool_size = ANET_LAN_IPV4_ADDR_LIST_SIZE;
        lan_net->ipv4_addr_pool_start = ANET_USB_LAN_IPV4_ADDR +
            (usb_net_index + 2) * ANET_LAN_IPV4_ADDR_LIST_SIZE;
        lan_net->ipv4_addr_pool_end = lan_net->ipv4_addr_pool_start +
            ANET_LAN_IPV4_ADDR_LIST_SIZE;
    }
    #endif
}

void config_usb_lan_ipv6(USB_TETHER_LAN_NET *lan_net, WAN_NET *wan_net)
{
    u8 i;

    if (!wan_net)
        return;

    if (!aic_is_valid_ipv6_addr(lan_net->local_ipv6_addr)) {
        /* The ipv6 addr suffix of lan is random. */
        for (i = 0; i < 3; i++) {
            lan_net->local_ipv6_addr[i] = wan_net->wan_local_ipv6_addr[i];
        }
        /* Set the last ulong to a certain value different from the wan net. */
        lan_net->local_ipv6_addr[3] =
            wan_net->wan_local_ipv6_addr[3] == 0x10 ? 0x20 : 0x10;
        /* IPv6 dns addrs of wan and lan are the same. */
        for (i = 0; i < 4; i++) {
            lan_net->lan_ipv6_dns1_address[i] = wan_net->wan_ipv6_dns1_address[i];
            lan_net->lan_ipv6_dns2_address[i] = wan_net->wan_ipv6_dns2_address[i];
        }
    }

    /** If the wan net has got the global ipv6 addr,
     * then assign a global addr to lan net.
     */
    if (aic_is_valid_ipv6_addr(wan_net->wan_ipv6_address1)) {
        for (i = 0; i < 3; i++) {
            lan_net->lan_global_ipv6_addr[i] = wan_net->wan_ipv6_address1[i];
        }
        /** Get a proper suffix value to make sure the addrs in the pool is
         * continuous and the end addr is greater than the start addr.
         */
        lan_net->lan_global_ipv6_addr[3] =
            get_proper_suffix_value(wan_net->wan_ipv6_address1[3]);

        /* Config ipv6 addr pool start. */
        for (i = 0; i < 3; i++) {
            lan_net->ipv6_addr_pool_start[i] = lan_net->lan_global_ipv6_addr[i];
        }
        lan_net->ipv6_addr_pool_start[3] =
            lan_net->lan_global_ipv6_addr[3] + ANET_LAN_IPV6_BASE_GAP;

        /* Config ipv6 addr pool end. */
        for (i = 0; i < 3; i++) {
            lan_net->ipv6_addr_pool_end[i] = lan_net->lan_global_ipv6_addr[i];
        }
        lan_net->ipv6_addr_pool_end[3] =
            lan_net->ipv6_addr_pool_start[3] + ANET_LAN_IPV6_ADDR_LIST_SIZE;

        lan_net->ipv6_addr_pool_size = ANET_LAN_IPV6_ADDR_LIST_SIZE;
    }
}

ulong get_proper_suffix_value(ulong base_value)
{
    u8 reserved_gap = ANET_LAN_IPV6_BASE_GAP * 2 + ANET_LAN_IPV6_ADDR_LIST_SIZE;
    ulong top_value;

    top_value = 0xffffffff - reserved_gap;
    if (base_value <= reserved_gap || base_value >= top_value) {
        return reserved_gap;
    } else {
        return base_value + 1;
    }
}

s32 create_usbtether_lan_net(u16 usbnet_index)
{
    USB_TETHER_LAN_NET *lan_net;

    usb_tether_net_info_list[usbnet_index].lan_net_ptr =
        (USB_TETHER_LAN_NET *)aos_zalloc_relaxed(sizeof(USB_TETHER_LAN_NET));

    lan_net = usb_tether_net_info_list[usbnet_index].lan_net_ptr;
    if (lan_net) {
        lan_net->lan_net_id = usb_tether_net_info_list[usbnet_index].lan_net_id;
        lan_net->global_ipv6_addr_index = 0xffff;
        lan_net->local_ipv6_addr_index = 0xffff;
    } else {
        aicx_error(0x0861e0, "", "error,malloc usb lan net fail\n");
        return AIC_ENOMEM;
    }

    return AIC_SUCCESS;
}

s32 config_usb_lan_forwarding(u16 usbnet_index)
{
    USB_TETHER_LAN_NET *lan_net;
    WAN_NET *wan_net;
    u32 ret;

    wan_net = usb_tether_net_info_list[usbnet_index].wan_net_ptr;
    lan_net = usb_tether_net_info_list[usbnet_index].lan_net_ptr;

    if (wan_net) {
        /* Check if lan net has malloced. */
        if (!lan_net) {
            ret = create_usbtether_lan_net(usbnet_index);
            if (ret) {
                return ret;
            }
        }

        lan_net = usb_tether_net_info_list[usbnet_index].lan_net_ptr;
        /** If the pdp has a ipv4 address, then generate a fake address
         * for lan in ndis mode; If not, then follow tethering mode.
         */
        config_usb_lan_ipv4_forwarding(usbnet_index);
        config_usb_lan_ipv6(lan_net, wan_net);
    }

    return AIC_SUCCESS;
}

#ifdef CONFIG_CUSTOMER_AT
static void config_usb_dial_private_lan_ip(USB_TETHER_LAN_NET *lan_net)
{
    u8 ip_num[4] = {0};
    u8 dhcp_ip_start, dhcp_ip_end, local_ip_last_byte = 0, count;

    if (!lan_net)
        return;

    ip_num[0] = 192;
    ip_num[1] = 168;
    if (customer_usb_net_cfg.third_byte_set)
        ip_num[2] = customer_usb_net_cfg.lan_ip_third_byte;
    else
        ip_num[2] = 0;

    if (customer_usb_net_cfg.dhcp_start_set) {
        dhcp_ip_start = customer_usb_net_cfg.dhcp_ip_start;
        ip_num[3] = dhcp_ip_start;
        lan_net->ipv4_addr_pool_start =
            IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);

        if (dhcp_ip_start > 1)
            local_ip_last_byte = 1;

        if (customer_usb_net_cfg.dhcp_end_set) {
            dhcp_ip_end = customer_usb_net_cfg.dhcp_ip_end;
            ip_num[3] = dhcp_ip_end;
            lan_net->ipv4_addr_pool_end =
                IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);
            lan_net->ipv4_addr_pool_size = dhcp_ip_end - dhcp_ip_start + 1;
            if (local_ip_last_byte == 0)
                local_ip_last_byte =
                    dhcp_ip_end < 254 ? (dhcp_ip_end + 1) : (dhcp_ip_end - 1);

            lan_net->lan_ipv4_address =
                IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], local_ip_last_byte);
        } else {
            count = 0xff - dhcp_ip_start;
            if (count >= ANET_LAN_IPV4_ADDR_LIST_SIZE)
                dhcp_ip_end = dhcp_ip_start + ANET_LAN_IPV4_ADDR_LIST_SIZE - 1;
            else
                dhcp_ip_end = 0xff;

            lan_net->ipv4_addr_pool_end =
                IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], dhcp_ip_end);
            lan_net->ipv4_addr_pool_size = dhcp_ip_end - dhcp_ip_start + 1;

            if (local_ip_last_byte == 0)
                local_ip_last_byte = dhcp_ip_start +
                                     (2 * ANET_LAN_IPV4_ADDR_LIST_SIZE);

            lan_net->lan_ipv4_address =
                IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], local_ip_last_byte);
        }
    } else {
        ip_num[3] = 1;
        lan_net->lan_ipv4_address =
            IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);
        lan_net->ipv4_addr_pool_start = lan_net->lan_ipv4_address +
                                        (2 * ANET_LAN_IPV4_ADDR_LIST_SIZE);
        lan_net->ipv4_addr_pool_end = lan_net->ipv4_addr_pool_start +
                                      ANET_LAN_IPV4_ADDR_LIST_SIZE - 1;
    }

    if (lan_net->ipv4_addr_pool_size == 0)
        lan_net->ipv4_addr_pool_size = ANET_LAN_IPV4_ADDR_LIST_SIZE;

}
#endif

/* Config usb lan net in routing mode. */
s32 config_usb_lan_routing(u16 usbnet_index)
{
    USB_TETHER_LAN_NET *lan_net;
    WAN_NET *wan_net;
    u32 ret;
    u8 ip_num[4] = {0};
    USB_TETHER_NET_INFO *usb_net;

    usb_net = &usb_tether_net_info_list[usbnet_index];
    wan_net = usb_net->wan_net_ptr;
    lan_net = usb_net->lan_net_ptr;

    /* Set paras of LAN NET  Check if lan net has malloced. */
    if (!lan_net) {
        ret = create_usbtether_lan_net(usbnet_index);
        if (ret) {
            return ret;
        }
    }

    lan_net = usb_net->lan_net_ptr;
    lan_net->lan_ipv4_subnet_mask = ANET_LAN_NET_MASK_ROUTING;

#ifdef CONFIG_CUSTOMER_AT
    if (aic_is_usb_dial_ip_set()) {
        config_usb_dial_private_lan_ip(lan_net);
        goto CONFIG_DNS_SERVER;
    }
#endif

    lan_net->lan_ipv4_address = ANET_USB_LAN_IPV4_ADDR + usbnet_index;
    lan_net->ipv4_addr_pool_size = ANET_LAN_IPV4_ADDR_LIST_SIZE;
    lan_net->ipv4_addr_pool_start = ANET_USB_LAN_IPV4_ADDR +
        (usbnet_index + 2) * ANET_LAN_IPV4_ADDR_LIST_SIZE;
    lan_net->ipv4_addr_pool_end = lan_net->ipv4_addr_pool_start +
                                  ANET_LAN_IPV4_ADDR_LIST_SIZE - 1;

#ifdef CONFIG_CUSTOMER_AT
CONFIG_DNS_SERVER:
#endif
    if (wan_net) {
        lan_net->lan_ipv4_dns1_address = wan_net->wan_ipv4_dns1_address;
        lan_net->lan_ipv4_dns2_address = wan_net->wan_ipv4_dns2_address;
    } else {
        /* Maybe pdp is not activated now. */
        get_numeric_ipv4_addr(ip_num, ANET_DEFAULT_IPV4_DNS_SERVER1);
        lan_net->lan_ipv4_dns1_address =
            IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);

        get_numeric_ipv4_addr(ip_num, ANET_DEFAULT_IPV4_DNS_SERVER2);
        lan_net->lan_ipv4_dns2_address =
                IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);
    }

    config_usb_lan_ipv6(lan_net, wan_net);

    return AIC_SUCCESS;
}

s32 set_usb_lan_interface(u16 usb_net_index, u32 *interface_index)
{
    u32 status = AIC_SUCCESS;
    USB_TETHER_NET_INFO *usb_net;

    usb_net = &usb_tether_net_info_list[usb_net_index];
    if (usb_net->lan_intf_index == 0xff) {
        status = attach_usb_lan_interface(usb_net_index, interface_index);
        if (!status) {
            usb_net->lan_intf_index = *interface_index;
            /* Modify the mtu size. */
            nx_ip_interface_mtu_set((NX_IP *)alte_ip.ip_ptr,
                                    usb_net->lan_intf_index,
                                    ANET_USB_LAN_MTU_SIZE);
        } else {
            return status;
        }
    } else {
        *interface_index = usb_net->lan_intf_index;
        status = nx_ip_interface_address_set((NX_IP *)alte_ip.ip_ptr,
                                             usb_net->lan_intf_index,
                                             usb_net->lan_net_ptr->lan_ipv4_address,
                                             ANET_LAN_NET_MASK_ROUTING);
        if (status)
            aicx_error(0x08b329, "x", "set_usb_lan_interface:set usb intf ip error 0x%x\n", status);
    }

    /* Set ipv6 link local and global addrs to lan ip instance. */
    set_usb_intf_ipv6_addrs(usb_net_index, usb_net->lan_intf_index);

#ifdef CONFIG_NX_NAT
    if (
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
        usb_net->usb_tether_mode == ANET_SHARE_MODE_ROUTING &&
#endif
        usb_net->nat_entry_created == ANET_NAT_ENTRY_NOT_CREATED &&
        usb_net->wan_intf_index < NX_MAX_PHYSICAL_INTERFACES) {
#ifdef CONFIG_CUSTOMER_AT
        if (aic_is_usb_dial_enabled())
#else
#ifdef CONFIG_CM_USB_DIAL_AT
        if (aic_is_mdialup_connect_enabled())
#endif
#endif
            config_usb_interf_nat(usb_net_index);
    }
#endif

#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
#ifdef CONFIG_CUSTOMER_AT
            if (aic_is_usb_dial_enabled())
#else
#ifdef CONFIG_CM_USB_DIAL_AT
            if (aic_is_mdialup_connect_enabled())
#endif
#endif
                aic_enable_ipv6_packet_route(&alte_ip);
#endif

#ifdef CONFIG_CUSTOMER_AT
    if (aic_is_usb_dial_enabled() && !aic_usb_intf_single_get_state(usb_net->usb_interfce)) {
        aicx_info(0x060e5a, "", "send RA for usb net dial\n");
        aic_network_ipv6_send_ra((NX_IP *)alte_ip.ip_ptr, usb_net->lan_intf_index);
    }
#endif

#ifdef CONFIG_CM_USB_DIAL_AT
        if (aic_is_mdialup_connect_enabled() &&
            !aic_usb_intf_single_get_state(usb_net->usb_interfce)) {
            aicx_info(0x00a4fa, "", "send RA for usb dialup\n");
            aic_network_ipv6_send_ra((NX_IP *)alte_ip.ip_ptr, usb_net->lan_intf_index);
        }
#endif

    return status;
}

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
void aic_usb_ipv6_dad_success(NX_IP *ip_ptr, u32 interf_index, u32 address_index)
{
    USB_TETHER_NET_INFO *usb_net;

    usb_net = aic_get_usb_net_by_lan_ifc(interf_index);
    if (!usb_net)
        return;

    /* Check if DAD of the ipv6 global addr succeeds. */
    if (address_index == usb_net->lan_net_ptr->global_ipv6_addr_index) {
        aicx_info(0x0a507e, "", "DAD success for usb net global ipv6 addr\n");

#ifdef CONFIG_CUSTOMER_AT
        if (aic_is_usb_dial_enabled() && !aic_usb_intf_single_get_state(usb_net->usb_interfce))
#else
#ifdef CONFIG_CM_USB_DIAL_AT
        if (aic_is_mdialup_connect_enabled() && !aic_usb_intf_single_get_state(usb_net->usb_interfce))
#endif
#endif
            aic_network_ipv6_send_ra(ip_ptr, interf_index);
    }
}
#endif

#ifdef CONFIG_NX_NAT
u32 config_usb_interf_nat(u16 usb_net_index)
{
    u32 wan_intf_index, ret;
    ulong peer_addr;
    USB_TETHER_NET_INFO *usb_net;
    NX_NAT_DEVICE *nat_dev_ptr;

    usb_net = &usb_tether_net_info_list[usb_net_index];
    wan_intf_index = usb_net->wan_net_ptr->wan_ifc_index;

    if (!nat_dev_list[wan_intf_index]) {
        nat_dev_list[wan_intf_index] =
            (NX_NAT_DEVICE *)aos_zalloc(sizeof(NX_NAT_DEVICE));
        AOS_ASSERT(nat_dev_list[wan_intf_index]);
    }
    nat_dev_ptr = nat_dev_list[wan_intf_index];

    if (nat_dev_ptr->nx_nat_id != NX_NAT_ID) {
#ifdef NX_NAT_CACHE_SIZE
        alte_ip.nat_cache_mem_size = NX_NAT_CACHE_SIZE;
#else
        alte_ip.nat_cache_mem_size = SIZE_8K + SIZE_4K;
#endif
        ret = aic_nat_create(nat_dev_ptr, &alte_ip, wan_intf_index);
        if (ret)
            return ret;

        nx_nat_cache_notify_set(wan_intf_index, usb_nat_cache_full_callback);
        nx_nat_enable(wan_intf_index);
    }

    peer_addr = usb_net->lan_net_ptr->ipv4_addr_pool_start;
    ret = nx_nat_inbound_entry_create(wan_intf_index,
                                      &usb_net->icmp_nat_entry,
                                      peer_addr,
                                      0, 0, NX_PROTOCOL_ICMP);
    if (ret)
        aicx_error(0x008f00, "x", "create ICMP entry fail 0x%x\n", ret);

    ret = nx_nat_inbound_entry_create(wan_intf_index,
                                      &usb_net->tcp_nat_entry,
                                      peer_addr,
                                      0, 0, NX_PROTOCOL_TCP);
    if (ret)
        aicx_error(0x0f0f68, "x", "create TCP entry fail 0x%x\n", ret);

    ret = nx_nat_inbound_entry_create(wan_intf_index,
                                      &usb_net->udp_nat_entry,
                                      peer_addr,
                                      0, 0, NX_PROTOCOL_UDP);
    if (ret)
        aicx_error(0x19e59c, "x", "create UDP entry fail 0x%x\n", ret);

    usb_net->nat_entry_created = ANET_NAT_ENTRY_CREATED;

    return ret;
}

void usb_nat_cache_full_callback(NX_NAT_DEVICE *nat_ptr)
{
    aicx_info(0x107013, "d", "usb nat cache full, interf index %d\n",
              nat_ptr->nx_nat_global_interface_index);
}

void aic_delete_usb_interf_nat(USB_TETHER_NET_INFO *usb_net)
{
    u32 wan_intf_index;
    NX_NAT_DEVICE *nat_dev_ptr;

    wan_intf_index = usb_net->wan_intf_index;
    if (usb_net->nat_entry_created == ANET_NAT_ENTRY_NOT_CREATED ||
        wan_intf_index >= NX_MAX_PHYSICAL_INTERFACES)
        return;

    nx_nat_inbound_entry_delete(wan_intf_index, &usb_net->icmp_nat_entry);
    nx_nat_inbound_entry_delete(wan_intf_index, &usb_net->tcp_nat_entry);
    nx_nat_inbound_entry_delete(wan_intf_index, &usb_net->udp_nat_entry);

    if (aic_ppp_dial_use_same_nat(wan_intf_index) || aic_softap_use_same_nat(wan_intf_index)) {
        usb_net->nat_entry_created = ANET_NAT_ENTRY_NOT_CREATED;
        return;
    }

    nat_dev_ptr = nat_dev_list[wan_intf_index];
    if (nat_dev_ptr) {
        aic_nat_delete(nat_dev_ptr, &alte_ip);
        aos_free(nat_dev_ptr);
        nat_dev_list[wan_intf_index] = AIC_NULL;
    }

    usb_net->nat_entry_created = ANET_NAT_ENTRY_NOT_CREATED;
}
#endif

void set_usb_intf_ipv6_addrs(u16 usb_net_index, u32 lan_interf_index)
{
    u32 global_addr_index = 0xff, local_addr_index = 0xff;
    NXD_ADDRESS global_ipv6_addr, local_ipv6_addr;
    USB_TETHER_LAN_NET *lan_net;
    USB_TETHER_NET_INFO *usb_net;
    u8 i;
    u32 status;

    if (lan_interf_index >= NX_MAX_PHYSICAL_INTERFACES) {
        return;
    }

    usb_net = &usb_tether_net_info_list[usb_net_index];
    lan_net = usb_net->lan_net_ptr;
    if (!lan_net)
        return;

    if (lan_net->local_ipv6_addr_index >= NX_MAX_IPV6_ADDRESSES &&
        aic_is_valid_ipv6_addr(lan_net->local_ipv6_addr)) {
        /* Prefix length of link local addr is 10. */
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
            aicx_error(0x1c0861, "", "set local ipv6 addr for usb intf error\n");
        }
    }

    if (lan_net->global_ipv6_addr_index >= NX_MAX_IPV6_ADDRESSES &&
        aic_is_valid_ipv6_addr(lan_net->lan_global_ipv6_addr)) {
        /* Set global ipv6 addr. */
        global_ipv6_addr.nxd_ip_version = NX_IP_VERSION_V6;
        for (i = 0; i < 4; i++) {
            global_ipv6_addr.nxd_ip_address.v6[i] =
                lan_net->lan_global_ipv6_addr[i];
        }
        status = nxd_ipv6_address_set((NX_IP *)alte_ip.ip_ptr,
                                      lan_interf_index,
                                      &global_ipv6_addr, 64,
                                      &global_addr_index);
        if (!status)
            lan_net->global_ipv6_addr_index = global_addr_index;
    }
}

u32 attach_usb_lan_interface(u16 usb_net_index, u32 *ifc_index)
{
    u8 ifc_name[ANET_MAX_LEN_INTERF_NAME] = {'\0'};
    u32 status;
    USB_TETHER_LAN_NET *lan_net;
    USB_TETHER_NET_INFO *usb_net_ptr;

    usb_net_ptr = &usb_tether_net_info_list[usb_net_index];
    strncpy(ifc_name, usb_net_ptr->usb_interfce, (sizeof(ifc_name) - 1));
    lan_net = usb_net_ptr->lan_net_ptr;
    if (!lan_net)
        return AIC_SUCCESS;

#ifdef CONFIG_USBX_NETWORK
    /* Assign interface index in _nx_ip_interface_attach. */
    status = nx_ip_interface_attach((NX_IP *)alte_ip.ip_ptr,
                                    usb_net_ptr->usb_interfce,
                                    lan_net->lan_ipv4_address,
                                    lan_net->lan_ipv4_subnet_mask,
                                    _ux_network_driver_entry);

    if (status) {
        aicx_error(0x000c4f, "x", "usb lan interf attach error--0x%x\n", status);
        *ifc_index = NX_MAX_PHYSICAL_INTERFACES;
        return status;
    }
#endif

    /* Get interface index via interface name set above. */
    *ifc_index = aic_get_lan_ifc_index(ifc_name);
    if (*ifc_index == NX_MAX_PHYSICAL_INTERFACES) {
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
                aicx_error(0x0c2e1f, "", "enable arp for usb lan error\n");
            }
        }
    }

    return AIC_SUCCESS;
}

/** Receive one of the following AT commands to stop usb net share.
 * Paramerter 0 is disabling usb net share,1 is pdp cid.
 * 1)AT^USBTETHER=0,1[,"rndis"]
 * 2)AT+CGACT=0,cid
 */
u32 aic_usbtether_stop(USB_TETHER_NET_INFO *usb_net_ptr, boolean down_usb_intf,
                       boolean to_deactivate_pdp)
{
    u8 cid;
    u16 ifc_index;
#ifdef CONFIG_USB_SERVICE
    u16 net_intf_down;
    u8 usb_intf[ANET_MAX_LEN_INTERF_NAME] = {'\0'};
#endif

    if (!usb_net_ptr)
        return AIC_SUCCESS;

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    cid = usb_net_ptr->wan_net_id;
    if (to_deactivate_pdp)
        /* Deactivate the PDP first. */
        aic_send_cgact_exe_cmd(ANET_PDP_TO_DEACTIVATE, cid);

    if (cid == ANET_DEFAULT_PDP_CID &&
        usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
        aic_delete_port_hashtable(ANET_NETSHARE_TYPE_USB,
                                  usb_net_ptr->usb_tether_mode);
#endif
    }

    if (!aic_usb_intf_single_get_state(usb_net_ptr->usb_interfce))
        aic_disconnect_usb_intf(usb_net_ptr->usb_interfce);

#ifdef CONFIG_USB_SERVICE
        if (down_usb_intf && usb_net_ptr->usb_interf_state == AIC_USB_LINK_UP) {
            strncpy(usb_intf, usb_net_ptr->usb_interfce,
                    (ANET_MAX_LEN_INTERF_NAME - 1));
            aic_wait_usb_intf_down(usb_intf, old_usb_mode, &net_intf_down);
            if (!net_intf_down) {
                aicx_error(0x16904c, "", "error,can't make usb intf down\n");
            }
        }
#endif

#ifdef CONFIG_NX_NAT
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
    if (usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_ROUTING)
#endif
        aic_delete_usb_interf_nat(usb_net_ptr);
#endif

#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
    aic_disable_ipv6_packet_route(&alte_ip);
#endif

    ifc_index = usb_net_ptr->lan_intf_index;
    /** Detach lan net interface.The function can still be used to
     * detach the interface,even if the interface index is 0.
     */
    if (ifc_index < NX_MAX_PHYSICAL_INTERFACES )
        nx_ip_interface_detach((NX_IP *)alte_ip.ip_ptr, ifc_index);

    if (usb_net_ptr->lan_net_ptr)
        aos_free((u8 *)usb_net_ptr->lan_net_ptr);

#ifdef CONFIG_NX_DHCP
#ifdef ENABLE_DHCPV6_SERVER
    if (dhcpv6_server && usb_net_ptr->dhcpv6_started) {
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

    /* Clear the usb net info of the index. */
    clear_usb_net_info(usb_net_ptr);

    /* If all usb net info are cleared,delete dhcp and dhcpv6 server. */
    if (usbtether_all_stopped()) {
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
    aos_put_mutex(usb_net_mutex);

    return AIC_SUCCESS;
}

/* Stop USB net share when it is starting,such as fail to activate PDP. */
u32 aic_usbtether_stop_half_way(u8 pdp_cid, u32 ucom_id)
{
    u8 usbnet_index = ANET_USB_NET_MAX_NUM;
    USB_TETHER_NET_INFO *usb_net_ptr;

    usbnet_index = get_usb_net_index(ucom_id, AIC_NULL, pdp_cid);
    if (usbnet_index >= ANET_USB_NET_MAX_NUM) {
        return ANET_ERROR_INVALID_USB_NET_INDEX;
    }

    usb_net_ptr = &usb_tether_net_info_list[usbnet_index];
    if (pdp_cid == ANET_DEFAULT_PDP_CID &&
        usb_net_ptr->usb_tether_mode == ANET_SHARE_MODE_FORWARDING) {
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
        aic_delete_port_hashtable(ANET_NETSHARE_TYPE_USB,
                              usb_net_ptr->usb_tether_mode);
#endif
    }

    /* Clear the usb net info of the index. */
    clear_usb_net_info(usb_net_ptr);

    return AIC_SUCCESS;
}

/** Deactivating of PDP is invoked,but the usb net share may be auto
 * reconnected when PDP is activated again. So some info is still kept.
 */
void usbtether_deact_invoked(USB_TETHER_NET_INFO *usb_net_ptr)
{
    USB_TETHER_LAN_NET *lan_net;

    if (!usb_net_ptr)
        return;

    if (!aic_usb_intf_single_get_state(usb_net_ptr->usb_interfce)) {
        aic_ipv6_send_invalid_ra(usb_net_ptr->lan_intf_index);
    }

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    usb_net_ptr->sent_pdp_deact = ANET_SENT_PDP_DEACT;
    /** Clear lan net and delete ipv6 addrs of the interface. But not detach
     * the interface,so the ipv4 addr is still in use.
     */
    lan_net = usb_net_ptr->lan_net_ptr;
    if (lan_net) {
        if (lan_net->local_ipv6_addr_index < NX_MAX_IPV6_ADDRESSES)
            nxd_ipv6_address_delete((NX_IP *)alte_ip.ip_ptr,
                                    lan_net->local_ipv6_addr_index);

        if (lan_net->global_ipv6_addr_index < NX_MAX_IPV6_ADDRESSES)
            nxd_ipv6_address_delete((NX_IP *)alte_ip.ip_ptr,
                                    lan_net->global_ipv6_addr_index);

        aos_memset(lan_net, 0, sizeof(USB_TETHER_LAN_NET));
        lan_net->lan_net_id = usb_net_ptr->lan_net_id;
        lan_net->local_ipv6_addr_index = 0xffff;
        lan_net->global_ipv6_addr_index = 0xffff;
    }

#ifdef CONFIG_NX_NAT
    aic_delete_usb_interf_nat(usb_net_ptr);
#endif
    aic_disable_ipv6_packet_route(&alte_ip);
    if (!aic_usb_intf_single_get_state(usb_net_ptr->usb_interfce))
        aic_disconnect_usb_intf(usb_net_ptr->usb_interfce);

    usb_net_ptr->wan_net_ptr = AIC_NULL;
    usb_net_ptr->wan_intf_index = 0xff;

#ifdef CONFIG_NX_DHCP
    /** Set DHCP to not_configured,so DNS addrs will be reconfigured
     * when PDP is activated again.
     */
    usb_net_ptr->dhcp_configured = ANET_DHCP_NOT_CONFIGURED;
    if (dhcp_server) {
        nx_dhcp_server_delete(dhcp_server);
        aos_free(dhcp_server);
        dhcp_server = AIC_NULL;
        if (server_dhcp_stack) {
            aos_free(server_dhcp_stack);
            server_dhcp_stack = AIC_NULL;
        }
     }

#ifdef ENABLE_DHCPV6_SERVER
    /* DHCPv6 should be reinited. */
    if (usb_net_ptr->dhcpv6_started == ANET_DHCPV6_STARTED) {
        if (dhcpv6_server) {
            nx_dhcpv6_server_delete(dhcpv6_server);
            aos_free(dhcpv6_server);
            dhcpv6_server = AIC_NULL;
        }
        if (dhcpv6_server_stack) {
            aos_free(dhcpv6_server_stack);
            dhcpv6_server_stack = AIC_NULL;
        }
        usb_net_ptr->dhcpv6_started = ANET_DHCPV6_NOT_STARTED;
        usb_net_ptr->dhcpv6_duid = 0;
    }
#endif
#endif
    aos_put_mutex(usb_net_mutex);
}

boolean usbtether_all_stopped(void)
{
    u8 i;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++){
        if (usb_tether_net_info_list[i].wan_net_id != 0) {
            return FALSE;
        }
    }

    return TRUE;
}

u8 get_usb_net_index_unused(void)
{
    u8 index = ANET_USB_NET_MAX_NUM;
    u8 i;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        if (usb_tether_net_info_list[i].lan_net_id == 0 &&
            usb_tether_net_info_list[i].wan_net_id == 0) {
            index = i;
            break;
        }
    }

    return index;
}

u8 aic_get_usb_net_by_intf_id(u8 usb_intf_id)
{
    u8 i;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        if (usb_tether_net_info_list[i].usb_interf_id == usb_intf_id) {
            return i;
        }
    }

    return ANET_USB_NET_MAX_NUM;
}

void aic_stop_all_usb_net(boolean to_down_intf)
{
    USB_TETHER_NET_INFO *usb_net;
    u8 i;

    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net = &usb_tether_net_info_list[i];
        if (strlen(usb_net->usb_interfce) > 0) {
            aic_usbtether_stop(usb_net, to_down_intf, AIC_FALSE);
        }
    }
}

#ifdef ANET_ENABLE_USB_DONGLE
void aic_set_dg_usb_intf_mode(u8 intf_mode)
{
    dg_intf_mode = intf_mode;
}

u8 aic_get_dg_usb_intf_mode(void)
{
    return dg_intf_mode;
}

void aic_dg_auto_usb_dial(u8 tether_mode, u8 intf_mode)
{
    u8 intf_name[ANET_MAX_LEN_INTERF_NAME] = {0};

    if (intf_mode == ANET_DIAL_INTF_RNDIS_MODE)
        strcpy(intf_name, ANET_USB_INTF_NAME_RNDIS);
    else if (intf_mode == ANET_DIAL_INTF_ECM_MODE)
        strcpy(intf_name, ANET_USB_INTF_NAME_ECM);
    else
        strcpy(intf_name, ANET_USB_INTF_NAME_RNDIS);

    aic_auto_start_usb_tether(tether_mode, intf_name);
}
#endif

void aic_usb_intf_update_xmit_queue_room_size(void)
{
#ifndef AIC_USB_NIC_MAX_QUEUED_NUM
    usb_rndis_update_xmit_queue_room_size(ANET_MAX_PACKET_NUM / 4);
    usb_cdc_ecm_update_xmit_queue_room_size(ANET_MAX_PACKET_NUM / 4);
#else
    usb_rndis_update_xmit_queue_room_size(AIC_USB_NIC_MAX_QUEUED_NUM);
    usb_cdc_ecm_update_xmit_queue_room_size(AIC_USB_NIC_MAX_QUEUED_NUM);
#endif
}

