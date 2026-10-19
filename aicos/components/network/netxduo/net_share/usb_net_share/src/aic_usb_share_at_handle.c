/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to Network Share
 *  This is the handler of AT commands relating to usb network share.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x134049
#define LOG_MODULE    AMOD_NET

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <nx_api.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <connmgr_api.h>
#include <aic_md_id.h>
#include <aic_net_api.h>
#include <aic_ipv6_forward.h>
#include "aic_at_cmd_handle.h"
#include "aic_general_at_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_at_parse_utils.h"
#include "aic_at_interact_cp.h"
#include "aic_net_pdp_mgr.h"
#include "aic_net_share_common.h"
#include "aic_usb_share_at_handle.h"
#include "aic_usb_net_share_mgr.h"

#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
#include "aic_net_share_port_mgr.h"
#endif

#ifdef CONFIG_USBX_NETWORK
#include "ux_network_driver.h"
#endif

#ifdef CONFIG_USB_SERVICE
#include <usb_service.h>
#include <rndis.h>
#include <cdc_ecm.h>
#endif

#ifdef CONFIG_MISC_SUPPORT
#include <aic_md_api.h>
#endif

#ifdef CONFIG_CUSTOMER_AT
CUSTOMER_USB_NET_CFG customer_usb_net_cfg;
u8 usb_dial_pdp_cid = ANET_DEFAULT_PDP_CID;
#endif
/* Define global vars. */
#ifdef CONFIG_CM_USB_DIAL_AT
CUSTOMER_M_DIAL_CFG m_dial_cfg;
#endif

AT_CMD_HANDLE usb_share_at_handles[] =
{
#ifdef CONFIG_GX_USB_DIAL_AT
    {"AT^USBTETHER", AIC_NULL, aic_handle_usbtether_query_cmd,
     AIC_NULL, aic_handle_usbtether_exe_cmd},
#endif

#ifdef CONFIG_CUSTOMER_AT
    {"AT+DIALMODE", aic_handle_dialmode_test_cmd, aic_handle_dialmode_query_cmd,
     aic_handle_dialmode_set_cmd, AIC_NULL},

    {"AT$MYCONFIG", aic_handle_myconfig_test_cmd, aic_handle_myconfig_query_cmd,
     aic_handle_myconfig_set_cmd, AIC_NULL},

    {"AT+USBNETIP", aic_handle_usbnetip_test_cmd, aic_handle_usbnetip_query_cmd,
     aic_handle_usbnetip_set_cmd, AIC_NULL},
#endif

#ifdef CONFIG_CM_USB_DIAL_AT
    {AT_DIALUP_CFG, aic_handle_mdialupcfg_test_cmd, aic_handle_mdialupcfg_query_cmd,
     aic_handle_mdialupcfg_set_cmd, AIC_NULL},

    {AT_DIALUP, aic_handle_mdialup_test_cmd, aic_handle_mdialup_query_cmd,
     aic_handle_mdialup_set_cmd, AIC_NULL},
#endif

    {AIC_NULL},
};

/* Define external vars. */
extern aos_mutex_ptr usb_net_mutex;
extern USB_TETHER_NET_INFO usb_tether_net_info_list[ANET_USB_NET_MAX_NUM];
extern REG_SERVICE_INFO reg_service_info;
extern aos_event_ptr event_net_share;
extern REG_SERVICE_INFO reg_service_info;
#ifdef CONFIG_USB_SERVICE
extern u8 current_usb_mode;
extern u8 old_usb_mode;
#endif
extern struct aic_network_ip_instance alte_ip;
extern INIT_NET_CFG init_net_cfg;

#ifdef CONFIG_GX_USB_DIAL_AT
/* Check if the same usb tether already started. */
static boolean same_usb_tether_started(u8 pdp_cid, u8 *usb_interf, u8 tether_mode)
{
    boolean ret = AIC_FALSE;
    u8 i, len1, len2, len;
    USB_TETHER_NET_INFO *usb_net;

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    /* Check the ucom channel and cid. */
    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net = &usb_tether_net_info_list[i];
        if (pdp_cid == usb_net->wan_net_id &&
            tether_mode == usb_net->usb_tether_mode) {
            len1 = strlen(usb_interf);
            len2 = strlen(usb_net->usb_interfce);
            len = len1 <= len2 ? len1 : len2;
            if (len != 0 && strncasecmp(usb_interf, usb_net->usb_interfce, len) == 0) {
                ret = AIC_TRUE;
                break;
            }
        }
    }
    aos_put_mutex(usb_net_mutex);

    return ret;
}

/** Handle AT^USBTETHER cmd to start or stop USB share.The cmd format:
 * AT^USBTETHER=<state>,<cid>,<usbInterface>,<shareMode>
 * For example:
 * Start USB net share: AT^USBTETHER=1,1,"rndis0",1,"cmnet"
 * Stop USB share(the third parameter is optional):
 * AT^USBTETHER=0,1 or AT^USBTETHER=0,1,"rndis0"
 */
u32 aic_handle_usbtether_exe_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 error_num = 0, paras_count, usb_intf_up = NX_FALSE;
    ulong *paras_arr;
    u8 usb_intf[16] = {'\0'};
    u8 apn_name[ANET_APN_NAME_MAX_LEN] = {'\0'};
    u8 share_mode = ANET_SHARE_MODE_ROUTING, cid = 0xff;
    u8 enable_usb_tether;
    u32 status = AIC_SUCCESS;
    boolean to_deact_pdp = AIC_FALSE;
    USB_TETHER_NET_INFO *usb_net = AIC_NULL;
    u32 serial_port_id;

    if (!aic_is_usb_plug_in()) {
        aic_reply_error_resp(at_req_resp_ptr, ANET_ERROR_USB_PLUG_OUT);
        return ANET_ERROR_USB_PLUG_OUT;
    }

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT^USBTETHER=", &paras_count);
    if (!paras_arr)
        goto PARA_ERROR;

    enable_usb_tether = atoi((u8 *)paras_arr[0]);
    if (enable_usb_tether != ANET_START_USB_NET_TETHER &&
        enable_usb_tether != ANET_STOP_USB_NET_TETHER) {
        error_num++;
        goto PARA_ERROR;
    }

    /** The parameters count must be greater than 1. */
    if ((enable_usb_tether == ANET_START_USB_NET_TETHER && paras_count < 3) ||
        (enable_usb_tether == ANET_STOP_USB_NET_TETHER  && paras_count < 2))
        goto PARA_ERROR;

    /*if (enable_usb_tether == ANET_START_USB_NET_TETHER && !aic_is_usb_in_net_mode()) {
        alog_error("%s:set usb to network mode first\n", __func__);
        goto PARA_ERROR;
    }*/

    if (paras_arr[1] && strlen((u8 *)paras_arr[1]) > 0) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1],
                                       strlen((u8 *)paras_arr[1])))
            goto PARA_ERROR;

        cid = atoi((u8 *)paras_arr[1]);
    } else {
        cid = ANET_DEFAULT_PDP_CID;
    }
    if (!aic_is_valid_pdp_cid(cid)) {
        error_num++;
        goto PARA_ERROR;
    }

    if (paras_count >= 3) {
        if (!aic_is_valid_usb_intf((u8 *)paras_arr[2])) {
            aicx_error(0x169a1f, "s", "error usb interface name--%s\n", (u8 *)paras_arr[2]);
            error_num ++;
            goto PARA_ERROR;
        }
        strncpy(usb_intf, (u8 *)paras_arr[2], (sizeof(usb_intf) - 1));

        if (paras_count >= 4) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[3],
                                           strlen((u8 *)paras_arr[3])))
                goto PARA_ERROR;

            share_mode = atoi((u8 *)paras_arr[3]);
            if (share_mode != ANET_SHARE_MODE_FORWARDING &&
                share_mode != ANET_SHARE_MODE_ROUTING) {
                error_num++;
                goto PARA_ERROR;
            }

            if (paras_count >= 5) {
                strncpy(apn_name, (u8 *)paras_arr[4],
                        (ANET_APN_NAME_MAX_LEN - 1));
            }
        }

        if (enable_usb_tether == ANET_START_USB_NET_TETHER &&
            same_usb_tether_started(cid, usb_intf, share_mode)) {
            aic_free_paras_mem(paras_arr, paras_count);
            if (!aic_is_ps_registered())
                aic_attach_packet_domain();
            else if (!aic_is_pdp_activated(cid))
                aic_enable_data_connection(cid, AIC_NULL, apn_name);

            aic_reply_ok_resp(at_req_resp_ptr);
            return AIC_SUCCESS;
        }
    }

    aic_free_paras_mem(paras_arr, paras_count);

    serial_port_id = at_req_resp_ptr->serial_port_id;
    /* Start or stop usb network share. */
    if (error_num == 0) {
        if (enable_usb_tether == ANET_START_USB_NET_TETHER) {
            if (cid == ANET_DEFAULT_PDP_CID &&
                share_mode == ANET_SHARE_MODE_FORWARDING) {
#ifdef CONFIG_NET_DRIVER_DIRECT_FORWARD
                status = aic_init_net_port_hashtable(ANET_NETSHARE_TYPE_USB,
                                                     share_mode);
                if (status) {
                    aicx_error(0x1b2d0d, "", "init usb hashtable error\n");
                    goto INTERNAL_ERROR;
                }
#endif
            }

            aic_wait_usb_intf_up(usb_intf, &usb_intf_up);
            if (!usb_intf_up) {
                aicx_error(0x162301, "", "error,can't make usb intf up\n");
                status = ANET_ERROR_USB_INTF_DOWN;
                goto INTERNAL_ERROR;
            }

            /* USB intf maybe change due to self adaptation. */
            if (usb_service_get_enum_mode() == RNDIS)
                strcpy(usb_intf, ANET_USB_INTF_NAME_RNDIS);
            if (usb_service_get_enum_mode() == ECM)
                strcpy(usb_intf, ANET_USB_INTF_NAME_ECM);

            status = aic_usbtether_start(serial_port_id, usb_intf,
                                         share_mode, cid,
                                         AIC_NULL, apn_name);
            if (!status) {
                if (!aic_is_pdp_activated(cid)) {
                    /* Send attach cmd for LTE.PDP will be activated after attach. */
                    if (!aic_is_ps_registered())
                        aic_attach_packet_domain();
                    else
                        aic_enable_data_connection(cid, AIC_NULL, apn_name);

                    if (share_mode == ANET_SHARE_MODE_FORWARDING) {
                        usb_net = aic_get_usb_net_by_cid(cid);
                        if (usb_net)
                            usb_net->forward_mode_start_time = aos_get_tickcount();
                    }
                }
            } else
                goto INTERNAL_ERROR;
        } else if (enable_usb_tether == ANET_STOP_USB_NET_TETHER) {
            usb_net = aic_get_usb_net_by_cid(cid);
            if (usb_net) {
                if (!usb_net->sent_pdp_deact && aic_is_pdp_activated(cid) &&
                    cid != ANET_DEFAULT_PDP_CID)
                    to_deact_pdp = AIC_TRUE;

                aic_usbtether_stop(usb_net, AIC_TRUE, to_deact_pdp);
            }
        }
    }

    /* Wait a while for USB restarting. */
    aos_sleep(2000);
    aic_reply_ok_resp(at_req_resp_ptr);

    return status;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    status = AT_ERROR_INVALID_PARAS;
    aic_reply_error_resp(at_req_resp_ptr, status);

    return status;

INTERNAL_ERROR:
    aic_reply_error_resp(at_req_resp_ptr, status);

    return status;
}

/**
 * Handle cmd AT^USBTETHER? The reply is as follows:
 * ^USBTETHER:1,1,"rndis",1,"IPV4V6","cmnet"
 * OK
 */
u32 aic_handle_usbtether_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 i;
    USB_TETHER_NET_INFO *usb_net;
    u8 tether_mode[8] = {'\0'};
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net = &usb_tether_net_info_list[i];
        if (aic_is_valid_pdp_cid(usb_net->wan_net_id)) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                    "^USBTETHER: 1,%d,", usb_net->wan_net_id);
            strcat(result_line, "\"");
            strcat(result_line, usb_net->usb_interfce);
            strcat(result_line, "\",");
            snprintf(tether_mode, sizeof(tether_mode), "%d,",
                     usb_net->usb_tether_mode);
            strcat(result_line, tether_mode);
            strcat(result_line, "\"");
            strncat(result_line, usb_net->apn,
                    (AT_MAX_AP_RESP_LENGTH - strlen(result_line) - 2));
            strcat(result_line, "\"");

            line_pointers[0] = (ulong)&result_line[0];
            aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
            aos_memset(result_line, '\0', sizeof(result_line));
        }
    }
    aos_put_mutex(usb_net_mutex);

    strcpy(result_line, AT_RESULT_OK);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}
#endif

void aic_usb_net_wait_pdp(u8 cid, USB_TETHER_NET_INFO *usb_net_ptr)
{
    WAN_NET *wan_net_ptr;
    u32 waiting_interval = 200, wait_total_time = 0;
    u16 wait_count = 0;

    if ((aic_is_flight_mode() || !aic_is_sim_card_plugged()) &&
        !aic_is_ps_registered()) {
        aicx_info(0x112757, "", "usb share no need to wait pdp act\n");
        return;
    }

    wan_net_ptr = aic_get_wan_net_per_pdp(cid);
    while (!(wan_net_ptr && wan_net_ptr->wan_ip_instance)) {
        if (!wan_net_ptr)
            wan_net_ptr = aic_get_wan_net_per_pdp(cid);

        aos_sleep(waiting_interval);
        wait_count++;
        wait_total_time += waiting_interval;
        if (wait_count == 10)
            waiting_interval = 500;

        if (wait_count > 10 && (wait_count % 2 == 0))
            aicx_info(0x00c579, "d", "wait %d times for PDP activation\n", wait_count);

        /* If the activating or attach cmd has received error,then wait 15s. */
        if ((!usb_net_ptr->waiting_pdp_act &&
            wait_total_time >= 1000) || wait_total_time >= 2000)
            return;
    }
}

boolean aic_is_valid_usb_intf(u8 *usb_intf_name)
{
    u8 len;

    if (!usb_intf_name)
        return AIC_FALSE;

    if (!(strcasestr(usb_intf_name, "rndis") ||
        strcasestr(usb_intf_name, "ecm")))
        return AIC_FALSE;

    len = strlen(usb_intf_name);
    if (!(*(usb_intf_name + len - 1) >= '0' &&
        *(usb_intf_name + len - 1) <= '9'))
        return AIC_FALSE;

    return AIC_TRUE;
}

/* Set usb interface up and wait until it is up. */
void aic_wait_usb_intf_up(u8 *usb_intf, u16 *usb_intf_up)
{
#ifdef CONFIG_USB_SERVICE
    u16 intf_up = NX_FALSE;
    u8 intf_id;
    u8 usb_mode = RNDIS;
    s32 ret;
#endif
#ifdef CONFIG_USBX_NETWORK
    u8 wait_count = 0;
    u16 wait_interval = 200;
#ifdef ANET_USB_INTF_SELF_ADAPTIVE
    u8 retry_count = 0;
#else
    u8 retry_count = 1;
#endif
#endif

    if (!usb_intf || !usb_intf_up)
        return;

    /* basic mode can't support usb. */
    if (aic_boot_mode_is_basic()) {
        *usb_intf_up = NX_FALSE;
        return;
    }

#ifdef CONFIG_USB_SERVICE
    intf_id = atoi(usb_intf);
    if (strcasestr(usb_intf, "rndis")) {
        usb_mode = RNDIS;
    } else if (strcasestr(usb_intf, "ecm")) {
        usb_mode = ECM;
    }

    //retry another intface if fail to make it up
    while (retry_count < 2) {
#ifdef ANET_USB_INTF_SELF_ADAPTIVE
        /* Don't retry any more in gxem mode. */
        if (aic_boot_mode_is_gxem() && retry_count == 1)
            break;
#endif
        ret = usb_service_enum_manage(usb_mode);
        if (!ret) {
            if (usb_mode != current_usb_mode) {
                old_usb_mode = current_usb_mode;
                current_usb_mode = usb_mode;
            }
        } else {
            aicx_error(0x18adce, "x", "set usb work mode error 0x%x\n", ret);
            *usb_intf_up = intf_up;
            return;
        }

#ifdef CONFIG_USBX_NETWORK
        intf_up = _ux_network_get_link_up_status(intf_id);
        while(!intf_up && wait_count < ANET_WAIT_USB_INTF_COUNT) {
            aos_sleep(wait_interval);
            wait_count++;
            intf_up = _ux_network_get_link_up_status(intf_id);
        }

        if (intf_up) {
            aicx_info(0x10a7c8, "", "usb intf is up now\n");
            break;
        } else {
            retry_count++;
            wait_count = 0;
#ifdef ANET_USB_INTF_SELF_ADAPTIVE
            /* Don't retry any more in gxem mode. */
            if (aic_boot_mode_is_gxem())
                break;

            if (usb_mode == RNDIS)
                usb_mode = ECM;
            else if (usb_mode == ECM)
                usb_mode = RNDIS;
#endif
        }
#endif
    }

    *usb_intf_up = intf_up;
#endif
}

/* Set usb interface down and wait until it is down. */
void aic_wait_usb_intf_down(u8 *usb_intf, u8 usb_mode, u16 *usb_intf_down)
{
    u16 intf_up = NX_TRUE;
    u8 intf_id;
#ifdef CONFIG_USB_SERVICE
    s32 ret;
#endif
#ifdef CONFIG_USBX_NETWORK
    u8 wait_count = 0;
    u16 wait_interval = 200;
#endif

    if (!usb_intf || !usb_intf_down)
        return;

#ifdef CONFIG_USB_SERVICE
    ret = usb_service_enum_manage(usb_mode);
    if (!ret) {
        if (usb_mode != current_usb_mode) {
            old_usb_mode = current_usb_mode;
            current_usb_mode = usb_mode;
        }
    } else {
        aicx_error(0x059950, "x", "down usb intf error 0x%x\n", ret);
        *usb_intf_down = !intf_up;
        return;
    }
#endif

    intf_id = atoi(usb_intf);
#ifdef CONFIG_USBX_NETWORK
    intf_up = _ux_network_get_link_up_status(intf_id);

    while(intf_up && wait_count < ANET_WAIT_USB_INTF_COUNT) {
        aos_sleep(wait_interval);
        wait_count++;
        intf_up = _ux_network_get_link_up_status(intf_id);
    }
#endif
    *usb_intf_down = !intf_up;
}

void aic_connect_usb_intf(u8 *intf_name)
{
    aicx_info(0x0159df, "", "connect usb intf\n");
    if (!intf_name)
        usb_rndis_single_connect();
    else {
        if (strcasestr(intf_name, "rndis"))
            usb_rndis_single_connect();
        else if (strcasestr(intf_name, "ecm"))
            usb_cdc_ecm_single_connect();
    }
}

void aic_disconnect_usb_intf(u8 *intf_name)
{
    u32 i, index = -1;

    aicx_info(0x0c2569, "", "disconnect usb intf\n");

    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        if (!alte_ip.ip_ptr->nx_ip_interface[i].nx_interface_name || !intf_name)
            continue;

        if (strcmp(intf_name, alte_ip.ip_ptr->nx_ip_interface[i].nx_interface_name) == 0) {
            break;
        }
    }

    if (i >= NX_MAX_PHYSICAL_INTERFACES)
        aicx_info(0x0e7b6e, "", "[NET] find interface index fail when usb disconnect\n");
    else
        index = i;

    if (!intf_name) {
        usb_rndis_single_disconnect();
    } else {
        if (strcasestr(intf_name, "rndis")) {
            usb_rndis_single_disconnect();
        } else if (strcasestr(intf_name, "ecm")) {
            usb_cdc_ecm_single_disconnect();
        }
    }

    if (index == -1)
        return;

    aicx_info(0x06344f, "", "[NET] start clear arp and nd cache when usb disconnect\n");
#ifndef NX_DISABLE_IPV4
#ifndef CONFIG_DISABLE_IPV4_ARP
    extern UINT _nx_arp_interface_entries_delete(NX_IP *ip_ptr, UINT index);
    /* Remove ARP entries associated with  this interface from ARP table. */
    _nx_arp_interface_entries_delete(alte_ip.ip_ptr, index);
#endif
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    extern UINT _nx_nd_cache_interface_entries_delete(NX_IP *ip_ptr, UINT index);
    /* Delete ND cache entries associated with this interface. */
    _nx_nd_cache_interface_entries_delete(alte_ip.ip_ptr, index);
#endif /* FEATURE_NX_IPV6 */
}

u32 aic_usb_intf_single_get_state(u8 *intf_name)
{
    if (!intf_name)
        return usb_rndis_single_get_state();
    else {
        if (strcasestr(intf_name, "rndis"))
            return usb_rndis_single_get_state();
        else if (strcasestr(intf_name, "ecm"))
            return usb_cdc_ecm_single_get_state();
    }

    return usb_rndis_single_get_state();
}

#if CONFIG_CUSTOMER_AT || CONFIG_CM_USB_DIAL_AT
static void connect_usb_dial_to_wan(USB_TETHER_NET_INFO *usb_net)
{
    if (!usb_net)
        return;

    if (aic_is_pdp_activated(usb_net->wan_net_id)) {
        aic_notify_usbtether_pdp_activated(usb_net->wan_net_id);
    } else {
        usb_net->waiting_pdp_act = ANET_WAIT_PDP_ACTIVATE;
    }
}
#endif

#ifdef CONFIG_CUSTOMER_AT
void aic_init_customer_net_cfg(NET_CFG *net_cfg)
{
    customer_usb_net_cfg.dial_enabled = net_cfg->cus_dial_mode;
    customer_usb_net_cfg.usb_intf_mode = net_cfg->cus_intf_mode;
    customer_usb_net_cfg.ip_mode = net_cfg->cus_ip_mode;
    customer_usb_net_cfg.third_byte_set = net_cfg->cus_third_byte_set;
    customer_usb_net_cfg.lan_ip_third_byte = net_cfg->cus_dhcp_third_byte;
    customer_usb_net_cfg.dhcp_start_set = net_cfg->cus_dhcp_start_set;
    customer_usb_net_cfg.dhcp_ip_start = net_cfg->cus_dhcp_ip_start;
    customer_usb_net_cfg.dhcp_ip_end = net_cfg->cus_dhcp_ip_end;
}

static void print_customer_usb_dial_cfg(void)
{
    aicx_info(0x09d483, "ddddd", "usb dial cfg:enabled=%d,intf mode=%d,ip mode=%d,ip third byte=%d,ip start=%d\n",
              customer_usb_net_cfg.dial_enabled,
              customer_usb_net_cfg.usb_intf_mode,
              customer_usb_net_cfg.ip_mode,
              customer_usb_net_cfg.lan_ip_third_byte,
              customer_usb_net_cfg.dhcp_ip_start);
}

u8 aic_get_cus_dial_ip_mode(void)
{
    if (customer_usb_net_cfg.ip_mode == ANET_USB_NET_PRIVATE_IP)
        return ANET_SHARE_MODE_ROUTING;
    else if (customer_usb_net_cfg.ip_mode == ANET_USB_NET_PUBLIC_IP)
        return ANET_SHARE_MODE_FORWARDING;
    else
        return ANET_SHARE_MODE_ROUTING;
}

/* Customer AT AT+DIALMODE=0/1 Switch on/off USB net share. */
u32 aic_handle_dialmode_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line0, "+DIALMODE: (0-1)", (AT_MAX_AP_RESP_LENGTH - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_dialmode_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+DIALMODE: %d", customer_usb_net_cfg.dial_enabled);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

static s32 usb_net_dial_start(u32 at_port_id)
{
    u8 share_mode, current_tether_mode;
    s32 ret = AIC_SUCCESS;
    u8 usb_intf[ANET_MAX_LEN_INTERF_NAME] = {'\0'};
    USB_TETHER_NET_INFO *usb_net;
    u16 usb_intf_up = NX_FALSE;

    if (customer_usb_net_cfg.usb_intf_mode == ANET_USB_INTF_RNDIS_MODE)
        strcpy(usb_intf, "rndis0");
    else if (customer_usb_net_cfg.usb_intf_mode == ANET_USB_INTF_ECM_MODE)
        strcpy(usb_intf, "ecm0");

    aic_wait_usb_intf_up(usb_intf, &usb_intf_up);
    if (!usb_intf_up) {
        aicx_error(0x16b87d, "s", "usb net dial:can't make intf up %s\n", usb_intf);
        return ANET_ERROR_USB_INTF_DOWN;
    }

    /* USB intf maybe change due to self adaptation. */
    if (usb_service_get_enum_mode() == RNDIS)
        strcpy(usb_intf, ANET_USB_INTF_NAME_RNDIS);
    if (usb_service_get_enum_mode() == ECM)
        strcpy(usb_intf, ANET_USB_INTF_NAME_ECM);

    print_customer_usb_dial_cfg();
    share_mode = aic_get_cus_dial_ip_mode();
    usb_net = aic_get_usb_net_by_cid(usb_dial_pdp_cid);
    if (usb_net) {
        current_tether_mode = usb_net->usb_tether_mode;
        if (share_mode != current_tether_mode) {
            aicx_error(0x0294f8, "d", "error,usb net of share mode %d still ongoing\n",
                      current_tether_mode);
            return ANET_ERROR_INTERNAL_ERROR;
        }

        connect_usb_dial_to_wan(usb_net);
    } else {
        ret = aic_usbtether_start(at_port_id, usb_intf, share_mode,
                                  usb_dial_pdp_cid, AIC_NULL, AIC_NULL);
        if (ret) {
            aicx_error(0x09cea5, "x", "usbtether start error 0x%x\n", ret);
            return ret;
        }
    }

    if (!aic_is_pdp_activated(usb_dial_pdp_cid)) {
        /* Send attach cmd for LTE.PDP will be activated after attach. */
        if (!aic_is_ps_registered())
            aic_attach_packet_domain();
        else
            aic_enable_data_connection(usb_dial_pdp_cid, AIC_NULL, AIC_NULL);
    }

    return ret;
}

static void save_cus_dial_intf_cfg(u8 dial_mode, u8 intf_mode)
{
    NET_CFG net_cfg;

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
    net_cfg.cus_dial_mode = dial_mode;
    net_cfg.cus_intf_mode = intf_mode;
    aic_save_config_to_nv(&net_cfg);
#endif
}

static void save_customer_usbnet_ip_cfg(CUSTOMER_USB_NET_CFG *net_ip_cfg)
{
    NET_CFG net_cfg;

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
    net_cfg.cus_ip_mode = net_ip_cfg->ip_mode;
    net_cfg.cus_third_byte_set = net_ip_cfg->third_byte_set;
    net_cfg.cus_dhcp_start_set = net_ip_cfg->dhcp_start_set;
    net_cfg.cus_dhcp_third_byte = net_ip_cfg->lan_ip_third_byte;
    net_cfg.cus_dhcp_ip_start = net_ip_cfg->dhcp_ip_start;
    net_cfg.cus_dhcp_ip_end = net_ip_cfg->dhcp_ip_end;
    aic_save_config_to_nv(&net_cfg);
#endif
}

u32 aic_handle_dialmode_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 dial_mode, i;
    USB_TETHER_NET_INFO *usb_net;
    s32 ret;
    u32 at_port_id;
    u8 usb_intf[ANET_MAX_LEN_INTERF_NAME] = {'\0'};

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+DIALMODE=", &paras_count);
    if (!paras_arr || paras_count != 1 || strlen((u8 *)paras_arr[0]) != 1)
        goto PARA_ERROR;

    /* The parameter must be 0 or 1. */
    if (!(*((u8 *)paras_arr[0]) == 0x30 || *((u8 *)paras_arr[0]) == 0x31))
        goto PARA_ERROR;

    dial_mode = atoi((u8 *)paras_arr[0]);
    aic_free_paras_mem(paras_arr, paras_count);
    customer_usb_net_cfg.dial_enabled = dial_mode;
    print_customer_usb_dial_cfg();
    save_cus_dial_intf_cfg(customer_usb_net_cfg.dial_enabled, customer_usb_net_cfg.usb_intf_mode);

    /* Start USB net share. */
    if (dial_mode == ANET_USB_DIAL_ENABLED) {
        /* If in public ip mode,then make intf down first,then up it. */
        if (customer_usb_net_cfg.usb_intf_mode == ANET_USB_INTF_RNDIS_MODE)
            strcpy(usb_intf, "rndis0");
        else if (customer_usb_net_cfg.usb_intf_mode == ANET_USB_INTF_ECM_MODE)
            strcpy(usb_intf, "ecm0");

        at_port_id = at_req_res_ptr->serial_port_id;
        ret = usb_net_dial_start(at_port_id);
        if (ret) {
            aicx_info(0x174c50, "", "usb_net_dial_start error\n");
            aic_reply_error_resp(at_req_res_ptr, ret);
            return ret;
        }
    } else {
        /* Stop usb net share. */
        for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
            usb_net = &usb_tether_net_info_list[i];
            if (usb_net->wan_net_id == usb_dial_pdp_cid) {
                aic_delete_usb_interf_nat(usb_net);
                if (!aic_usb_intf_single_get_state(usb_net->usb_interfce)) {
                    aicx_info(0x116ab9, "", "send invalid ra\n");
                    aic_ipv6_send_invalid_ra(usb_net->lan_intf_index);
                }

                aic_disable_ipv6_packet_route(&alte_ip);
                if (!aic_usb_intf_single_get_state(usb_net->usb_interfce))
                    aic_disconnect_usb_intf(usb_net->usb_interfce);
            }
        }
    }

    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

boolean aic_is_usb_dial_enabled(void)
{
    return (customer_usb_net_cfg.dial_enabled == ANET_USB_DIAL_ENABLED);
}

u32 aic_handle_myconfig_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line0, "$MYCONFIG: \"USBNETMODE\",(0-1)",
            (AT_MAX_AP_RESP_LENGTH - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_myconfig_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "$MYCONFIG: \"USBNETMODE\",%d", customer_usb_net_cfg.usb_intf_mode);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

/* Customer AT AT$MYCONFIG="USBNETMODE",<0/1>
 * 0 -- rndis    1 -- ecm
 */
u32 aic_handle_myconfig_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 intf_mode;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT$MYCONFIG=", &paras_count);
    if (!paras_arr || paras_count != 2)
        goto PARA_ERROR;

    /* The second parameter must be 0 or 1. */
    if (!(*((u8 *)paras_arr[1]) == 0x30 || *((u8 *)paras_arr[1]) == 0x31))
        goto PARA_ERROR;

    intf_mode = atoi((u8 *)paras_arr[1]);
    aic_free_paras_mem(paras_arr, paras_count);

    customer_usb_net_cfg.usb_intf_mode = intf_mode;
    save_cus_dial_intf_cfg(customer_usb_net_cfg.dial_enabled, intf_mode);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

u8 aic_get_usb_intf_mode(void)
{
    return customer_usb_net_cfg.usb_intf_mode;
}

static boolean should_use_new_pdp_info(u8 ip_mode)
{
    USB_TETHER_NET_INFO *usb_net;
    WAN_NET *wan_net;

    usb_net = aic_get_usb_net_by_cid(usb_dial_pdp_cid);
    wan_net = aic_get_wan_net_per_pdp(usb_dial_pdp_cid);
    if (usb_net && wan_net && ip_mode == ANET_USB_NET_PUBLIC_IP) {
        if (usb_net->lan_net_ptr->ipv4_addr_pool_start != wan_net->wan_ipv4_address)
            return AIC_TRUE;
    }

    return AIC_FALSE;
}

/* Customer AT AT+USBNETIP=mode[,tpos,dhcps,dhcpe]
 * mode: 0 -- public IP; 1 -- private IP
 * tpos: the third position of gateway addr
 * dhcps: dhcp start value
 * dhcpe: dhcp end value
 */
u32 aic_handle_usbnetip_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    strncpy(result_line0, "+USBNETIP: (0-1)[,(0-255)[,(0-255)[,(1-254)]]]",
            (AT_MAX_AP_RESP_LENGTH - 1));
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_usbnetip_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+USBNETIP: %d", customer_usb_net_cfg.ip_mode);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_usbnetip_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 ip_mode, ip_third_byte, dhcp_start, dhcp_end;
    boolean ip_cfg_changed = AIC_FALSE, pdp_ip_changed = AIC_FALSE;
    s32 ret;
    USB_TETHER_NET_INFO *usb_net;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+USBNETIP=", &paras_count);
    if (!paras_arr || paras_count < 1)
        goto PARA_ERROR;

    /* The first parameter must be 0 or 1. */
    if (!(*((u8 *)paras_arr[0]) == 0x30 || *((u8 *)paras_arr[0]) == 0x31))
        goto PARA_ERROR;

    ip_mode = atoi((u8 *)paras_arr[0]);
    if (customer_usb_net_cfg.ip_mode != ip_mode) {
        customer_usb_net_cfg.ip_mode = ip_mode;
        ip_cfg_changed = AIC_TRUE;
    }

    if (paras_count >= 2) {
        if (strlen((u8 *)paras_arr[1]) > 0) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                goto PARA_ERROR;

            customer_usb_net_cfg.third_byte_set = ANET_SET_BY_USER;
            ip_third_byte = atoi((u8 *)paras_arr[1]);
            if (customer_usb_net_cfg.lan_ip_third_byte != ip_third_byte) {
                customer_usb_net_cfg.lan_ip_third_byte = ip_third_byte;
                ip_cfg_changed = AIC_TRUE;
            }
        }

        if (paras_count >= 3) {
            if (strlen((u8 *)paras_arr[2]) > 0) {
                if (!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
                    goto PARA_ERROR;

                customer_usb_net_cfg.dhcp_start_set = ANET_SET_BY_USER;
                dhcp_start = atoi((u8 *)paras_arr[2]);
                if (customer_usb_net_cfg.dhcp_ip_start != dhcp_start) {
                    customer_usb_net_cfg.dhcp_ip_start = dhcp_start;
                    ip_cfg_changed = AIC_TRUE;
                }
            }
        }

        if (paras_count >= 4) {
            if (strlen((u8 *)paras_arr[3]) > 0) {
                if (!aic_is_valid_integer_para((u8 *)paras_arr[3], strlen((u8 *)paras_arr[3])))
                    goto PARA_ERROR;

                customer_usb_net_cfg.dhcp_end_set = ANET_SET_BY_USER;
                dhcp_end = atoi((u8 *)paras_arr[3]);
                if (customer_usb_net_cfg.dhcp_ip_end != dhcp_end) {
                    customer_usb_net_cfg.dhcp_ip_end = dhcp_end;
                    ip_cfg_changed = AIC_TRUE;
                }
            }
        }
    }

    aic_free_paras_mem(paras_arr, paras_count);
    print_customer_usb_dial_cfg();
    save_customer_usbnet_ip_cfg(&customer_usb_net_cfg);

    /* Maybe pdp deactivated during usb dial of public ip mode. */
    pdp_ip_changed = should_use_new_pdp_info(ip_mode);
    if (ip_cfg_changed || pdp_ip_changed) {
        usb_net = aic_get_usb_net_by_cid(usb_dial_pdp_cid);
        if (usb_net) {
            aicx_info(0x05b76c, "", "disable current usb dial for netip or pdp change\n");
            aic_stop_all_usb_net(AIC_FALSE);
            /* Sleep a while to wait disconnect usb intf above. */
            aos_sleep(30);
        }

        ret = usb_net_dial_start(at_req_res_ptr->serial_port_id);
        if (ret) {
            aicx_error(0x1dba64, "x", "usb dial start error 0x%x\n", ret);
            aic_reply_error_resp(at_req_res_ptr, ret);
            return ret;
        }
    }

    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

boolean aic_is_usb_dial_ip_set(void)
{
    return (customer_usb_net_cfg.third_byte_set ||
            customer_usb_net_cfg.dhcp_start_set);
}

#endif

#ifdef CONFIG_CM_USB_DIAL_AT
/* Customer AT AT+MDIALUPCFG, USB net share.
 * <mode> 0--rndis   1--ecm
 * <auto> 0--off     1--on
 * <workmode> 0--private net mode    1--pubulic net mode
 */
u32 aic_handle_mdialupcfg_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[40] = {'\0'};
    u8 result_line1[40] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, sizeof(result_line0), RESP_DIALUP_CFG"\"mode\",(0,1)");
    snprintf(result_line1, sizeof(result_line1), RESP_DIALUP_CFG"\"auto\",(0,1)");
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

u32 aic_handle_mdialupcfg_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[40] = {'\0'};
    u8 result_line1[40] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    NET_CFG net_cfg;

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
#endif

    snprintf(result_line0, sizeof(result_line0), RESP_DIALUP_CFG"\"mode\",%d",
             net_cfg.m_intf_mode);
    snprintf(result_line1, sizeof(result_line1), RESP_DIALUP_CFG"\"auto\",%d",
             net_cfg.m_auto_flag);
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

static s32 save_m_dial_cfg_to_nv(u8 intf_mode, u8 auto_flag, u8 work_mode)
{
    NET_CFG net_cfg;
    s32 ret = AIC_SUCCESS;

    aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
    aic_read_net_config(&net_cfg);
    if (auto_flag != 0xff) {
        net_cfg.m_auto_flag = auto_flag;
        /* Auto start ps if auto usb dialup. */
        if (auto_flag == 1)
            net_cfg.auto_cfg_pdp_intf = auto_flag;
    }

    if (intf_mode != 0xff)
        net_cfg.m_intf_mode = intf_mode;

    if (work_mode != 0xff)
        net_cfg.m_work_mode = work_mode;

    ret = aic_save_config_to_nv(&net_cfg);
#endif

    return ret;
}

static u8 get_which_to_config(u8 *cfg_str)
{
    u8 config_flag = 0xff;

    if (!cfg_str)
        return config_flag;

    if (strncasecmp(cfg_str, "mode", 4) == 0 && strlen(cfg_str) == 4) {
        config_flag = ANET_DIAL_CONFIG_INTF_MODE;
    } else if (strncasecmp(cfg_str, "auto", 4) == 0 && strlen(cfg_str) == 4) {
        config_flag = ANET_DIAL_CONFIG_AUTO_FLAG;
    } else if (strncasecmp(cfg_str, "workmode", 8) == 0 && strlen(cfg_str) == 8) {
        config_flag = ANET_DIAL_CONFIG_WORK_MODE;
    }

    return config_flag;
}

/* Just save the config paras to nv,the config won't work until reset.
 * AT+MDIALUPCFG="mode"[,<mode>]  0--rndis; 1--ecm
 * AT+MDIALUPCFG="workmode"[,<workmode>]  0--private net mode;1--pubulic net mode
 * AT+MDIALUPCFG="auto"[,<auto_flag>]  0--off; 1--on
 * AT+MDIALUPCFG="auth"[,<user>,<password>]
 */
u32 aic_handle_mdialupcfg_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[40] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 config_flag, second_para;
    NET_CFG net_cfg;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             AT_DIALUP_CFG"=", &paras_count);
    if (!paras_arr || paras_count < 1 || paras_count > 2)
        goto PARA_ERROR;

    /* Not support authentication. */
    config_flag = get_which_to_config((u8 *)paras_arr[0]);
    if (config_flag == 0xff || config_flag == ANET_DIAL_CONFIG_WORK_MODE)
        goto PARA_ERROR;

    if (paras_count == 1) {
        aos_memset(&net_cfg, 0, sizeof(NET_CFG));
#ifdef CONFIG_MISC_SUPPORT
        aic_read_net_config(&net_cfg);
#endif
        switch (config_flag) {
            case ANET_DIAL_CONFIG_INTF_MODE:
                if (net_cfg.m_intf_mode == ANET_DIAL_INTF_RNDIS_MODE) {
#ifndef CONFIG_RNDIS_INSTANCE
                    net_cfg.m_intf_mode = ANET_DIAL_INTF_ECM_MODE;
#endif
                } else if (net_cfg.m_intf_mode == ANET_DIAL_INTF_ECM_MODE) {
#ifndef CONFIG_CDC_ECM_INSTANCE
                    net_cfg.m_intf_mode = ANET_DIAL_INTF_RNDIS_MODE;
#endif
                }

                snprintf(result_line0, sizeof(result_line0),
                         RESP_DIALUP_CFG"\"mode\",%d", net_cfg.m_intf_mode);
                break;

            case ANET_DIAL_CONFIG_AUTO_FLAG:
                snprintf(result_line0, sizeof(result_line0),
                         RESP_DIALUP_CFG"\"auto\",%d", net_cfg.m_auto_flag);
                break;
        }

        goto HANDLE_QUERY;
    } else if (paras_count == 2) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            goto PARA_ERROR;

        second_para = atoi((u8 *)paras_arr[1]);
        if ((second_para != 0 && second_para != 1)) {
            goto PARA_ERROR;
        }

        switch (config_flag) {
            case ANET_DIAL_CONFIG_INTF_MODE:
                if (second_para == ANET_DIAL_INTF_RNDIS_MODE) {
#ifndef CONFIG_RNDIS_INSTANCE
                    goto PARA_ERROR;
#endif
                } else if (second_para == ANET_DIAL_INTF_ECM_MODE) {
#ifndef CONFIG_CDC_ECM_INSTANCE
                    goto PARA_ERROR;
#endif
                }

                save_m_dial_cfg_to_nv(second_para, 0xff, 0xff);
                break;

            case ANET_DIAL_CONFIG_AUTO_FLAG:
                save_m_dial_cfg_to_nv(0xff, second_para, 0xff);
                break;
        }
    }

    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

HANDLE_QUERY:
    aic_free_paras_mem(paras_arr, paras_count);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, ANET_CM_ERROR_INVALID_PARA);

    return AIC_SUCCESS;
}

/* Test AT of AT+MDIALUP=<cid>,<connect>, start or stop USB net share. */
u32 aic_handle_mdialup_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[40] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, sizeof(result_line0), RESP_DIALUP"(1),(0,1)");
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

void aic_transmit_mdialup_connect_info(u32 at_port_id, boolean is_urc)
{
    USB_TETHER_NET_INFO *usb_net;
    u8 result_line[220] = {'\0'};
    u8 ipv4_addr[ANET_MAX_LEN_IPV4_ADDR] = {'\0'};
    u8 ipv6_addr[ANET_MAX_LEN_HEX_IPV6_ADDR] = {'\0'};
    ulong numeric_ipv4;
    ulong numeric_ipv6[4];
    u8 i;

    aos_get_mutex(usb_net_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
        usb_net = &usb_tether_net_info_list[i];
        if (aic_is_valid_pdp_cid(usb_net->wan_net_id)) {
            snprintf(result_line, sizeof(result_line), RESP_DIALUP"%d,%d,",
                     usb_net->wan_net_id, m_dial_cfg.connect_enabled);
            numeric_ipv4 = usb_net->lan_net_ptr->lan_ipv4_address;

            if (numeric_ipv4) {
                NX_CHANGE_ULONG_ENDIAN(numeric_ipv4);
                inet_ntop(AF_INET, &numeric_ipv4, ipv4_addr, ANET_MAX_LEN_IPV4_ADDR);
                strcat(result_line, "\"");
                strncat(result_line, ipv4_addr, strlen(ipv4_addr));
                strcat(result_line, "\",\"");
                strncat(result_line, ipv4_addr, strlen(ipv4_addr));
                strcat(result_line, "\"");
                /* Append dns server addrs. */
                numeric_ipv4 = usb_net->lan_net_ptr->lan_ipv4_dns1_address;
                if (numeric_ipv4) {
                    strcat(result_line, ",\"");
                    aos_memset(ipv4_addr, 0, ANET_MAX_LEN_IPV4_ADDR);
                    NX_CHANGE_ULONG_ENDIAN(numeric_ipv4);
                    inet_ntop(AF_INET, &numeric_ipv4, ipv4_addr, ANET_MAX_LEN_IPV4_ADDR);
                    strncat(result_line, ipv4_addr, strlen(ipv4_addr));
                    strcat(result_line, "\"");
                }

                numeric_ipv4 = usb_net->lan_net_ptr->lan_ipv4_dns2_address;
                if (numeric_ipv4) {
                    strcat(result_line, ",\"");
                    aos_memset(ipv4_addr, 0, ANET_MAX_LEN_IPV4_ADDR);
                    NX_CHANGE_ULONG_ENDIAN(numeric_ipv4);
                    inet_ntop(AF_INET, &numeric_ipv4, ipv4_addr, ANET_MAX_LEN_IPV4_ADDR);
                    strncat(result_line, ipv4_addr, strlen(ipv4_addr));
                    strcat(result_line, "\"");
                }

                if (is_urc) {
                    aic_transmit_at_cmd_urc(at_port_id, result_line, strlen(result_line));
                } else {
                    aic_transmit_at_result(at_port_id, result_line, strlen(result_line));
                }
            }

            if (aic_is_valid_ipv6_addr(usb_net->lan_net_ptr->lan_global_ipv6_addr)) {
                /*Assemble ipv6. */
                aos_memset(result_line, 0, sizeof(result_line));
                snprintf(result_line, sizeof(result_line), RESP_DIALUP"%d,%d,",
                         usb_net->wan_net_id, m_dial_cfg.connect_enabled);

                aos_memcpy((u8 *)numeric_ipv6,
                           (u8 *)usb_net->lan_net_ptr->lan_global_ipv6_addr, 16);
                aic_convert_numeric_ipv6_to_str(numeric_ipv6, ipv6_addr, ANET_MAX_LEN_HEX_IPV6_ADDR);
                strcat(result_line, "\"");
                strncat(result_line, ipv6_addr, strlen(ipv6_addr));
                strcat(result_line, "\",\"");
                strncat(result_line, ipv6_addr, strlen(ipv6_addr));
                strcat(result_line, "\"");

                aos_memcpy((u8 *)numeric_ipv6, (u8 *)usb_net->lan_net_ptr->lan_ipv6_dns1_address, 16);
                if (aic_is_valid_ipv6_addr(numeric_ipv6)) {
                    strcat(result_line, ",\"");
                    aic_convert_numeric_ipv6_to_str(numeric_ipv6, ipv6_addr, ANET_MAX_LEN_HEX_IPV6_ADDR);
                    strncat(result_line, ipv6_addr, strlen(ipv6_addr));
                    strcat(result_line, "\"");
                }

                aos_memcpy((u8 *)numeric_ipv6, (u8 *)usb_net->lan_net_ptr->lan_ipv6_dns2_address, 16);
                if (aic_is_valid_ipv6_addr(numeric_ipv6)) {
                    strcat(result_line, ",\"");
                    aic_convert_numeric_ipv6_to_str(numeric_ipv6, ipv6_addr, ANET_MAX_LEN_HEX_IPV6_ADDR);
                    strncat(result_line, ipv6_addr, strlen(ipv6_addr));
                    strcat(result_line, "\"");
                }

                if (is_urc) {
                    aic_transmit_at_cmd_urc(at_port_id, result_line, strlen(result_line));
                } else {
                    aic_transmit_at_result(at_port_id, result_line, strlen(result_line));
                }
            }
        }
    }

    aos_put_mutex(usb_net_mutex);
}

/* Customer AT AT+MDIALUP?
 * +MDIALUP:<cid>,<connect>,<ipv4>,<v4_gw>,<v4_dns1>,<v4_dns2>
 * +MDIALUP:<cid>,<connect>,<ipv6>,<v6_gw>,<v6_dns1>,<v6_dns2>
 */
u32 aic_handle_mdialup_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u32 port_id;

    port_id = at_req_res_ptr->serial_port_id;
    if (aic_is_mdialup_connect_enabled()) {
        if (aic_is_pdp_activated(ANET_DEFAULT_PDP_CID))
            aic_transmit_mdialup_connect_info(port_id, AIC_FALSE);
        else {
            aos_sleep(1000);
            if (aic_is_pdp_activated(ANET_DEFAULT_PDP_CID))
                aic_transmit_mdialup_connect_info(port_id, AIC_FALSE);
        }
    }

    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

void aic_read_init_mdial_cfg(void)
{
    if (!init_net_cfg.init_read_flag) {
        aos_memset(&init_net_cfg, 0, sizeof(INIT_NET_CFG));
        if (!aic_read_net_config(&init_net_cfg.net_cfg))
            init_net_cfg.init_read_flag = 1;
    }

    m_dial_cfg.intf_mode = init_net_cfg.net_cfg.m_intf_mode;
    m_dial_cfg.auto_flag = init_net_cfg.net_cfg.m_auto_flag;
    m_dial_cfg.work_mode = init_net_cfg.net_cfg.m_work_mode;
    m_dial_cfg.connect_enabled = m_dial_cfg.auto_flag;

    if (m_dial_cfg.intf_mode == ANET_DIAL_INTF_RNDIS_MODE) {
#ifndef CONFIG_RNDIS_INSTANCE
        m_dial_cfg.intf_mode = ANET_DIAL_INTF_ECM_MODE;
#endif
    } else if (m_dial_cfg.intf_mode == ANET_DIAL_INTF_ECM_MODE) {
#ifndef CONFIG_CDC_ECM_INSTANCE
        m_dial_cfg.intf_mode = ANET_DIAL_INTF_RNDIS_MODE;
#endif
    }
}

/* Customer AT AT+MDIALUP=<cid>,<connect>, start or stop USB net share.
 * <connect>: 1--start  0--stop
 */
u32 aic_handle_mdialup_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    ulong *paras_arr;
    u8 cid = 0, conn_action, i, same_action = 0;
    USB_TETHER_NET_INFO *usb_net = AIC_NULL;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             AT_DIALUP"=", &paras_count);
    if (!paras_arr || paras_count != 2)
        goto PARA_ERROR;

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])) ||
        !aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
        goto PARA_ERROR;

    cid = atoi((u8 *)paras_arr[0]);

    conn_action = atoi((u8 *)paras_arr[1]);
    if (cid != ANET_DEFAULT_PDP_CID || (conn_action != ANET_DIAL_CONNECT_ENABLED &&
        conn_action != ANET_DIAL_CONNECT_DISABLED))
        goto PARA_ERROR;

    aic_free_paras_mem(paras_arr, paras_count);
    if (conn_action == m_dial_cfg.connect_enabled) {
        same_action = 1;
        if (conn_action == ANET_DIAL_CONNECT_DISABLED) {
            aic_reply_ok_resp(at_req_res_ptr);
            return AIC_SUCCESS;
        }
    }

    m_dial_cfg.connect_enabled = conn_action;
    if (conn_action == ANET_DIAL_CONNECT_ENABLED) {
        usb_net = aic_get_usb_net_by_cid(cid);
        if (usb_net) {
            if (same_action && aic_is_pdp_activated(cid))
                goto MDIAL_OK;

            if (aic_is_flight_mode() || !aic_is_sim_card_plugged())
                goto ERROR_NO_NET;

            connect_usb_dial_to_wan(usb_net);
            if (!aic_is_pdp_activated(cid)) {
                m_dial_cfg.delay_report_info = AIC_TRUE;
                /* Send attach cmd for LTE.PDP will be activated after attach. */
                if (!aic_is_ps_registered())
                    aic_attach_packet_domain();
                else
                    aic_enable_data_connection(cid, AIC_NULL, AIC_NULL);
            }
        } else {
            aicx_error(0x024466, "", "mdialup:error,no usb net to connect\n");
            aic_reply_error_resp(at_req_res_ptr, ANET_ERROR_NO_USB_NET);
            return ANET_ERROR_NO_USB_NET;
        }
    } else {
        for (i = 0; i < ANET_USB_NET_MAX_NUM; i++) {
            usb_net = &usb_tether_net_info_list[i];
            if (usb_net->wan_net_id == cid) {
                aic_delete_usb_interf_nat(usb_net);
                aicx_info(0x0775fb, "", "mdialup: send invalid ra\n");
                if (!aic_usb_intf_single_get_state(usb_net->usb_interfce))
                    aic_ipv6_send_invalid_ra(usb_net->lan_intf_index);

                aic_disable_ipv6_packet_route(&alte_ip);
                if (!aic_usb_intf_single_get_state(usb_net->usb_interfce))
                    aic_disconnect_usb_intf(usb_net->usb_interfce);
            }
        }
    }

MDIAL_OK:
    aic_reply_ok_resp(at_req_res_ptr);
    if (conn_action == ANET_DIAL_CONNECT_ENABLED) {
        if (!m_dial_cfg.delay_report_info)
            aic_transmit_mdialup_connect_info(at_req_res_ptr->serial_port_id, AIC_TRUE);
        else {
            /* Wait one second for activating PDP. */
            aos_sleep(1000);
            /* Wait one more second for IPV6 RA. */
            if (!aic_is_pdp_activated(cid) || (aic_is_pdp_activated(cid) &&
                aic_get_pdp_ip_type(cid) == ANET_DEFAULT_IP_TYPE_V4V6))
                aos_sleep(1000);

            if (aic_is_pdp_activated(cid))
                aic_transmit_mdialup_connect_info(at_req_res_ptr->serial_port_id, AIC_TRUE);

            m_dial_cfg.delay_report_info = AIC_FALSE;
        }
    }

    return AIC_SUCCESS;

ERROR_NO_NET:
    aic_reply_error_resp(at_req_res_ptr, ANET_CM_ERROR_NO_NET);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    aic_reply_error_resp(at_req_res_ptr, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}

void aic_auto_mdialup_start(u8 intf_mode)
{
    u8 intf_name[ANET_MAX_LEN_INTERF_NAME] = {'\0'};

    if (intf_mode == ANET_DIAL_INTF_RNDIS_MODE)
        strcpy(intf_name, ANET_USB_INTF_NAME_RNDIS);
    else if (intf_mode == ANET_DIAL_INTF_ECM_MODE)
        strcpy(intf_name, ANET_USB_INTF_NAME_ECM);

    aic_auto_start_usb_tether(ANET_SHARE_MODE_ROUTING, intf_name);
}

boolean aic_is_mdialup_connect_enabled(void)
{
    return (m_dial_cfg.connect_enabled == ANET_DIAL_CONNECT_ENABLED);
}
#endif

