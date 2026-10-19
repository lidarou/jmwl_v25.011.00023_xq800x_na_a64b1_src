/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Aic USB Semi Tether Component.
 *    This is the management of semi network share via USB network interfaces.
.*    It is no need to activate PDP in this mode,for we only check the data
 *    packets transmiting between upper devices and the model device.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0551bf
#define LOG_MODULE    AMOD_NET

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_cmd.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <nx_ip.h>
#include <nx_api.h>
#include <nxd_bsd.h>
#include "aic_at_cmd_handle.h"
#include "aic_general_at_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_at_parse_utils.h"
#include "aic_net_api.h"
#include "aic_net_pdp_mgr.h"
#include "aic_net_usb_semi_tether.h"
#include "aic_net_share_common.h"
#include "aic_usb_net_share_mgr.h"
#ifdef CONFIG_USBX_NETWORK
#include "ux_network_driver.h"
#endif

#ifdef CONFIG_USB_SERVICE
#include <usb_service.h>
#endif

#ifdef CONFIG_NX_DHCP
#include "nxd_dhcp_client.h"
#include "nxd_dhcp_server.h"
#endif

#ifdef CONFIG_USB_NET_SHARE
#include "aic_usb_share_at_handle.h"
#endif

/* Define internal functions. */
static s32 aic_usb_semi_tether_init(void);
static s32 usb_semi_tether_start(USB_SEMI_TETHER_INFO *semi_tether_ptr,
                                 u8 tether_mode);
static USB_SEMI_TETHER_INFO *get_idle_usb_semi_tether(void);
static s32 usb_semi_tether_config_lan(USB_SEMI_TETHER_INFO *semi_tether_ptr);
static void config_usb_semi_lan_ipv6(USB_TETHER_LAN_NET *lan_net, u8 intf_id);
static s32 create_semi_tether_ip_instance(void);
static void clear_semi_tether_info(USB_SEMI_TETHER_INFO *semi_tether_net);
static s32 usb_semi_tether_stop(USB_SEMI_TETHER_INFO *semi_tether_net);
static u32 config_semi_tether_interface(USB_SEMI_TETHER_INFO *semi_tether_info,
                                        u32 *interface_index);
static u32 set_ipv6_addr_to_intf(USB_SEMI_TETHER_INFO *semi_tether_ptr,
                                 u32 interf_index);
static u32 set_semi_tether_dhcp_paras(USB_SEMI_TETHER_INFO *semi_tether_ptr);
static s32 create_usb_semi_tether_lan_net(USB_SEMI_TETHER_INFO *semi_tether_ptr);
static s32 start_semi_tether_dhcp_server(USB_SEMI_TETHER_INFO *semi_tether_info);
static s32 start_dhcpv6_for_semi_tether(USB_SEMI_TETHER_INFO *semi_tether_ptr);
static u32 get_lan_ifc_index(u8 *ifc_name);
static s32 semi_tether_config_for_peer(USB_SEMI_TETHER_INFO *semi_tether_net);
static s32 config_usb_semi_lan_routing(USB_SEMI_TETHER_INFO *semi_tether_info);
static s32 attach_usb_semi_lan_intf(USB_SEMI_TETHER_INFO *semi_tether_info,
                                    u32 *ifc_index);
static USB_SEMI_TETHER_INFO *get_usb_semi_tether_info(u8 *intf_name);
static u32 set_semi_tether_dhcpv6_paras(USB_SEMI_TETHER_INFO *semi_tether_net);
static u32 handle_semitether_exe_cmd(AT_REQ_RESP *at_req_resp_ptr);
static u32 handle_semitether_query_cmd(AT_REQ_RESP *at_req_resp_ptr);

/* Define global vars. */
struct aic_network_ip_instance usb_semi_tether_ip = {0};

/* Define local vars. */
static USB_SEMI_TETHER_INFO usb_semi_tether_list[ANET_USB_SEMI_TETHER_CNT];
static struct aic_packet_pool semi_tether_pkt_pool = {0};

static AT_CMD_HANDLE semi_tether_at_handles[] =
{
    {"AT^SEMITETHER", AIC_NULL, handle_semitether_query_cmd,
     AIC_NULL, handle_semitether_exe_cmd},

    {AIC_NULL},
};

/* Define external vars. */
extern u8 current_usb_mode;
extern u8 old_usb_mode;
#ifdef CONFIG_NX_DHCP
extern NX_DHCP_SERVER *dhcp_server;
extern u8 *server_dhcp_stack;
extern NX_DHCPV6_SERVER *dhcpv6_server;
extern u8 *dhcpv6_server_stack;
#endif

/**
 * Handle AT^SEMITETHER cmd to start or stop USB semi share.The cmd format:
 * AT^SEMITETHER=<state>,<usbInterface>
 * For example:
 * Start USB semi net share: AT^SEMITETHER=1,"rndis0"
 * Stop USB semi share:
 * AT^SEMITETHER=0,"rndis0"
 */
u32 handle_semitether_exe_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 paras_count;
    ulong *paras_arr;
    u8 usb_intf[ANET_MAX_LEN_INTERF_NAME] = {'\0'};
    s32 status = AIC_SUCCESS;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u8 enable_tether = 0xff;
    u16 usb_intf_up = AIC_FALSE;
    USB_SEMI_TETHER_INFO *usb_semi_tether = AIC_NULL;

    paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT^SEMITETHER=", &paras_count);
    /* The parameters count must be greater than 2. */
    if (!paras_arr || paras_count < 2) {
        aicx_error(0x0d00d1, "", "paras of AT^SEMITETHER error\n");
        goto PARA_ERROR;
    }

    enable_tether = atoi((u8 *)paras_arr[0]);
    if (enable_tether != ANET_USB_SEMI_TETHER_START &&
        enable_tether != ANET_USB_SEMI_TETHER_STOP) {
        goto PARA_ERROR;
    }

    /* Check if the usb interf name is valid. */
    if (!aic_is_valid_usb_intf((u8 *)paras_arr[1])) {
        goto PARA_ERROR;
    }
    strncpy(usb_intf, (u8 *)paras_arr[1], (ANET_MAX_LEN_INTERF_NAME - 1));

    /* Start or stop usb semi network share. */
    usb_semi_tether = get_usb_semi_tether_info(usb_intf);
    if (usb_semi_tether) {
        if (enable_tether == ANET_USB_SEMI_TETHER_START) {
            aicx_error(0x1834b1, "s", "interf %s is enaged for usb semi tether\n", usb_intf);
            goto OPERA_ERROR;
        } else if (enable_tether == ANET_USB_SEMI_TETHER_STOP) {
            usb_semi_tether_stop(usb_semi_tether);
        }
    } else {
        if (enable_tether == ANET_USB_SEMI_TETHER_START) {
            usb_semi_tether = get_idle_usb_semi_tether();
            if (!usb_semi_tether) {
                aicx_error(0x1b3e2d, "", "no idle usb semi tether\n");
                goto INTERNAL_ERROR;
            }
            usb_semi_tether->tether_enabled = enable_tether;
            usb_semi_tether->tether_mode = ANET_SHARE_MODE_ROUTING;
            strncpy(usb_semi_tether->net_interf_name, usb_intf,
                    (ANET_MAX_LEN_INTERF_NAME - 1));

            status = usb_semi_tether_start(usb_semi_tether,
                                           ANET_SHARE_MODE_ROUTING);
            if (status) {
                aicx_error(0x105cd3, "x", "start usb semi tether error 0x%x\n", status);
                goto INTERNAL_ERROR;
            }

            aic_wait_usb_intf_up(usb_intf, &usb_intf_up);
            if (!usb_intf_up) {
                aicx_error(0x175837, "", "error,can't make intf up for semi tether\n");
                status = ANET_ERROR_USB_INTF_DOWN;
                goto INTERNAL_ERROR;
            }

#ifdef CONFIG_NX_DHCP
            status = semi_tether_config_for_peer(usb_semi_tether);
            if (status) {
                aicx_error(0x0d088e, "x", "config for peer error for semi tether 0x%x\n", status);
                goto INTERNAL_ERROR;
            }
#endif

        } else if (enable_tether == ANET_USB_SEMI_TETHER_STOP) {
            aicx_info(0x016bed, "s", "interf %s was not semi tethered\n", usb_intf);
        }
    }

    aic_free_paras_mem(paras_arr, paras_count);
    strcpy(result_line, AT_RESULT_OK);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    return status;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    status = AT_ERROR_INVALID_PARAS;
    snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_PARAS);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    return status;

OPERA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    status = ANET_ERROR_INTF_ENGAGED;
    snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", ANET_ERROR_INTF_ENGAGED);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    return status;

INTERNAL_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    status = ANET_ERROR_USB_INTF_DOWN;
    usb_semi_tether_stop(usb_semi_tether);

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", ANET_ERROR_USB_INTF_DOWN);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    return status;
}

/**
 * Handle cmd AT^SEMITETHER? The reply is as follows:
 * ^SEMITETHER: 1,"rndis0"
 * OK
 */
u32 handle_semitether_query_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 i;
    USB_SEMI_TETHER_INFO *semi_tether_net;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    for (i = 0; i < ANET_USB_SEMI_TETHER_CNT; i++) {
        semi_tether_net = &usb_semi_tether_list[i];
        if (semi_tether_net->lan_net_id != 0) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "^SEMITETHER: 1,\"%s\"",
                     semi_tether_net->net_interf_name);

            line_pointers[0] = (ulong)&result_line[0];
            aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
            aos_memset(result_line, '\0', sizeof(result_line));
        }
    }

    strcpy(result_line, AT_RESULT_OK);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

s32 usb_semi_tether_start(USB_SEMI_TETHER_INFO *semi_tether_ptr, u8 tether_mode)
{
    s32 ret = AIC_SUCCESS;

    if (tether_mode != ANET_SHARE_MODE_ROUTING) {
        return ANET_ERROR_USB_SEMI_TETHER_MODE;
    }

    semi_tether_ptr->tether_mode = tether_mode;
    semi_tether_ptr->net_interf_id = atoi(semi_tether_ptr->net_interf_name);
    semi_tether_ptr->lan_net_id = ANET_USB_SEMI_LAN_NET_ID_BASE +
                                  semi_tether_ptr->net_interf_id;

    semi_tether_ptr->lan_net_ip = (NX_IP *)usb_semi_tether_ip.ip_ptr;
    ret = usb_semi_tether_config_lan(semi_tether_ptr);
    if (ret) {
        aicx_error(0x064ca9, "", "config usb semi tether lan error\n");
        return ret;
    }

    return AIC_SUCCESS;
}

s32 usb_semi_tether_config_lan(USB_SEMI_TETHER_INFO *semi_tether_ptr)
{
    s32 status;
    u32 lan_intf_index = NX_MAX_PHYSICAL_INTERFACES;

    status = config_usb_semi_lan_routing(semi_tether_ptr);
    if (status)
        return status;

    status = create_semi_tether_ip_instance();
    if (status)
        return status;

    semi_tether_ptr->lan_net_ip = usb_semi_tether_ip.ip_ptr;
    status = config_semi_tether_interface(semi_tether_ptr, &lan_intf_index);
    if (status || lan_intf_index >= NX_MAX_PHYSICAL_INTERFACES) {
        return status;
    }

    return AIC_SUCCESS;
}

USB_SEMI_TETHER_INFO *get_idle_usb_semi_tether(void)
{
    u8 i;

    for (i = 0; i < ANET_USB_SEMI_TETHER_CNT; i++) {
        if (usb_semi_tether_list[i].tether_enabled ==
            ANET_USB_SEMI_TETHER_STOP) {
            return &usb_semi_tether_list[i];
        }
    }

    return AIC_NULL;
}

s32 create_semi_tether_ip_instance(void)
{
    u32 status;

    if (!semi_tether_pkt_pool.packet_pool) {
        semi_tether_pkt_pool.per_packet_size = ANET_PACKET_SIZE;
        semi_tether_pkt_pool.packet_pool_size = ANET_PACKET_POOL_SIZE;
        semi_tether_pkt_pool.packet_pool_name = "semi_tether_pkt_pool";
        status = aic_create_packet_pool(&semi_tether_pkt_pool);

        if (status)
            return status;
    }

    usb_semi_tether_ip.ip_name = "SEMI_TETHER_IP_INS";
    usb_semi_tether_ip.default_net_mask = 0xffffff00UL;
    usb_semi_tether_ip.pri = 6;
    usb_semi_tether_ip.task_stack_size = ANET_IP_TASK_STACK_SIZE;
    usb_semi_tether_ip.bsd_task_stack_size = SIZE_2K;

#ifdef CONFIG_USBX_NETWORK
    usb_semi_tether_ip.if_type = AIC_USB_NETWORK_LINK;
#endif
    usb_semi_tether_ip.packet_pool_name = "semi_tether_packet_pool";
    usb_semi_tether_ip.packet_pool_size = ANET_PACKET_POOL_SIZE;
    usb_semi_tether_ip.per_packet_size = ANET_PACKET_SIZE;
    usb_semi_tether_ip.packet_pool = semi_tether_pkt_pool.packet_pool;

    status = aic_create_ip_instance(&usb_semi_tether_ip);
    if (status) {
        aicx_error(0x118703, "d", "create ip instance for usb semi tether error %d\n", status);
        return status;
    }

    /* Enable ICMP. */
    status = nx_icmp_enable((NX_IP *)usb_semi_tether_ip.ip_ptr);
    if (status) {
        aicx_error(0x0a99fa, "d", "enable icmp error for usb semi tether %d\n", status);
    }

    /* Enable the IPv6 service. */
    status = nxd_ipv6_enable((NX_IP *)usb_semi_tether_ip.ip_ptr);
    if (status) {
        aicx_error(0x1705ed, "d", "enable ipv6 error for usb semi tether %d\n", status);
     }

    /* Enable ICMPV6. */
    status = nxd_icmp_enable((NX_IP *)usb_semi_tether_ip.ip_ptr);
    if (status) {
        aicx_error(0x092ec3, "d", "enable icmpv6 error for usb semi tether %d\n", status);
    }

    status = nx_udp_enable((NX_IP *)usb_semi_tether_ip.ip_ptr);
    if (status) {
        aicx_error(0x0366ac, "d", "enable udp error for usb semi tether %d\n", status);
    }

    status = nx_tcp_enable((NX_IP *)usb_semi_tether_ip.ip_ptr);
    if (status) {
        aicx_error(0x1de4cd, "d", "enable tcp error for usb semi tether %d\n", status);
    }

    status = nx_ip_fragment_enable((NX_IP *)usb_semi_tether_ip.ip_ptr);
    if (status) {
        aicx_error(0x037376, "d", "enable fragment error for usb semi tether %d\n", status);
    }

    if (!usb_semi_tether_ip.arp_cache_mem) {
        usb_semi_tether_ip.arp_cache_mem_size = SIZE_1K;
        status = aic_enable_arp(&usb_semi_tether_ip);

        if (status) {
            if (status == NX_ALREADY_ENABLED) {
                status = AIC_SUCCESS;
            } else {
                aicx_error(0x0c0f23, "d", "enable arp error for usb semi tether %d\n", status);
            }
        }
    }

    return AIC_SUCCESS;
}

u32 config_semi_tether_interface(USB_SEMI_TETHER_INFO *semi_tether_info,
                                 u32 *interface_index)
{
    u32 status = AIC_SUCCESS;
    ulong ipv4_addr;
    NX_IP *ip_ptr;
    NX_INTERFACE *nx_intf_ptr;
    USB_TETHER_LAN_NET *lan_net;

    lan_net = semi_tether_info->lan_net_ptr;
    ip_ptr = (NX_IP *)usb_semi_tether_ip.ip_ptr;

    /** The inteface index of the first interface is fixed to 0.
     * Other interfaces are not bound to a fixed index.
     */
    if (semi_tether_info->net_interf_id == 0) {
        if (semi_tether_info->lan_intf_index < NX_MAX_PHYSICAL_INTERFACES) {
            *interface_index = semi_tether_info->lan_intf_index;
            goto SET_IPV6_ADDR;
        }

        *interface_index = ANET_IFC_INDEX_DEFAULT;
        nx_intf_ptr = &ip_ptr->nx_ip_interface[ANET_IFC_INDEX_DEFAULT];

        if (lan_net && lan_net->lan_ipv4_address) {
            ipv4_addr = lan_net->lan_ipv4_address;
            usb_semi_tether_ip.default_ip_address = (u32)ipv4_addr;

            if (!nx_intf_ptr->nx_interface_valid) {
#ifdef CONFIG_USBX_NETWORK
                status = nx_ip_interface_attach(ip_ptr,
                            semi_tether_info->net_interf_name,
                            ipv4_addr, 0xffffff00UL,
                            _ux_network_driver_entry);
#endif
            } else {
                nx_intf_ptr->nx_interface_name =
                    semi_tether_info->net_interf_name;
                status = nx_ip_interface_address_set(ip_ptr,
                                                     ANET_IFC_INDEX_DEFAULT,
                                                     ipv4_addr, 0xffffff00UL);
            }
            if (status) {
                aicx_error(0x192ae7, "x", "semi tether:attach or set addr error 0x%x\n", status);
                return status;
            }

            usb_semi_tether_ip.ip_ptr->nx_ip_gateway_address =
                lan_net->lan_ipv4_address;
            usb_semi_tether_ip.ip_ptr->nx_ip_gateway_interface = nx_intf_ptr;
        }
    } else {
        status = attach_usb_semi_lan_intf(semi_tether_info, interface_index);
        if (status) {
            aicx_error(0x0cb722, "x", "semi tether:attach interface error 0x%x\n", status);
            return status;
        }
    }
    semi_tether_info->lan_intf_index = *interface_index;

SET_IPV6_ADDR:
    /* Set ipv6 link local and global addrs to lan ip instance. */
    set_ipv6_addr_to_intf(semi_tether_info, *interface_index);

    status = nx_ip_interface_mtu_set((NX_IP *)usb_semi_tether_ip.ip_ptr,
                                     *interface_index,
                                     ANET_PDP_MTU_SIZE);
    if (status) {
        aicx_error(0x17a3a1, "x", "semi tether:set mtu error 0x%x\n", status);
        return status;
    }

    return AIC_SUCCESS;
}

u32 set_ipv6_addr_to_intf(USB_SEMI_TETHER_INFO *semi_tether_ptr,
                          u32 interf_index)
{
    NXD_ADDRESS ipv6_addr = {0};
    u32 status = AIC_SUCCESS;
    u32 local_addr_index = 0xff, global_addr_index = 0xff;
    u8 i;
    USB_TETHER_LAN_NET *lan_net;

    lan_net = semi_tether_ptr->lan_net_ptr;
    if (!lan_net)
        return ANET_ERROR_LAN_NET_NOT_INIT;

    ipv6_addr.nxd_ip_version = NX_IP_VERSION_V6;
    for (i = 0; i < 4; i++) {
        ipv6_addr.nxd_ip_address.v6[i] = lan_net->local_ipv6_addr[i];
    }
    status = nxd_ipv6_address_set(usb_semi_tether_ip.ip_ptr, interf_index,
                                  &ipv6_addr, 10, &local_addr_index);

    if (status) {
        aicx_error(0x0fbbfc, "d", "semi tether::set local ipv6 addr error-- %d\n", status);
        return status;
    }

    ipv6_addr.nxd_ip_version = NX_IP_VERSION_V6;
    for (i = 0; i < 4; i++) {
        ipv6_addr.nxd_ip_address.v6[i] = lan_net->lan_global_ipv6_addr[i];
    }
    status = nxd_ipv6_address_set(usb_semi_tether_ip.ip_ptr, interf_index,
                                  &ipv6_addr, 64, &global_addr_index);

    if (status) {
        aicx_error(0x0f58c0, "d", "semi tether:set global ipv6 addr error-- %d\n", status);
        return status;
    }

    return AIC_SUCCESS;
}

s32 attach_usb_semi_lan_intf(USB_SEMI_TETHER_INFO *semi_tether_info,
                             u32 *ifc_index)
{
    s32 status = AIC_SUCCESS;
    USB_TETHER_LAN_NET *lan_net;

    lan_net = semi_tether_info->lan_net_ptr;

    if (semi_tether_info->lan_intf_index == 0xff) {
#ifdef CONFIG_USBX_NETWORK
        /* Assign interface index in _nx_ip_interface_attach. */
        status = nx_ip_interface_attach((NX_IP *)usb_semi_tether_ip.ip_ptr,
                                        semi_tether_info->net_interf_name,
                                        lan_net->lan_ipv4_address,
                                        lan_net->lan_ipv4_subnet_mask,
                                        _ux_network_driver_entry);

        if (status) {
            aicx_error(0x01a636, "x", "semi tether:attach error--0x%x\n", status);
            *ifc_index = NX_MAX_PHYSICAL_INTERFACES;
            return status;
        }
#endif

        /* Get interface index via interface name set above. */
        *ifc_index = get_lan_ifc_index(semi_tether_info->net_interf_name);
        if (*ifc_index == NX_MAX_PHYSICAL_INTERFACES) {
            status = ANET_ERROR_INVALID_IFC_INDEX;
            return status;
        }
    } else {
        *ifc_index = semi_tether_info->lan_intf_index;
    }

    return status;
}

/* Config lan net of usb semi tether in routing mode. */
s32 config_usb_semi_lan_routing(USB_SEMI_TETHER_INFO *semi_tether_info)
{
    USB_TETHER_LAN_NET *lan_net;
    u32 ret;
    u8 interf_id;
    u8 ip_num[4] = {0};

    /* Check if lan net has malloced. */
    if (!semi_tether_info->lan_net_ptr) {
        ret = create_usb_semi_tether_lan_net(semi_tether_info);
        if (ret)
            return ret;
    }

    interf_id = semi_tether_info->net_interf_id;

    lan_net = semi_tether_info->lan_net_ptr;
    if (lan_net->lan_ipv4_address == 0) {
        lan_net->lan_ipv4_address = ANET_USB_SEMI_LAN_IPV4_ADDR + interf_id;
        lan_net->lan_ipv4_subnet_mask = ANET_LAN_NET_MASK_ROUTING;

        get_numeric_ipv4_addr(ip_num, ANET_DEFAULT_IPV4_DNS_SERVER1);
        lan_net->lan_ipv4_dns1_address =
            IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);

        get_numeric_ipv4_addr(ip_num, ANET_DEFAULT_IPV4_DNS_SERVER2);
        lan_net->lan_ipv4_dns2_address =
                IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);

        lan_net->ipv4_addr_pool_size = ANET_LAN_IPV4_ADDR_LIST_SIZE;
        lan_net->ipv4_addr_pool_start = ANET_USB_SEMI_LAN_IPV4_ADDR +
            (interf_id + 2) * ANET_LAN_IPV4_ADDR_LIST_SIZE;
        lan_net->ipv4_addr_pool_end = lan_net->ipv4_addr_pool_start +
                                      ANET_LAN_IPV4_ADDR_LIST_SIZE - 1;
    }

    config_usb_semi_lan_ipv6(lan_net, interf_id);

    return AIC_SUCCESS;
}

u32 get_lan_ifc_index(u8 *ifc_name)
{
    u8 i;
    NX_IP *ip_ptr;
    NX_INTERFACE *nx_interface;

    ip_ptr = (NX_IP *)usb_semi_tether_ip.ip_ptr;

    for(i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        nx_interface = &(ip_ptr->nx_ip_interface[i]);
        if (nx_interface->nx_interface_link_up &&
            nx_interface->nx_interface_name) {
            if (strncasecmp(ifc_name, nx_interface->nx_interface_name,
                            strlen(ifc_name)) == 0) {
                /* Find a valid index. */
                return i;
            }
        }
    }

    return NX_MAX_PHYSICAL_INTERFACES;
}

void config_usb_semi_lan_ipv6(USB_TETHER_LAN_NET *lan_net, u8 intf_id)
{
    u8 i;
    u8 local_ipv6_str[ANET_MAX_LEN_HEX_IPV6_ADDR] = {'\0'};
    u8 global_ipv6_str[ANET_MAX_LEN_HEX_IPV6_ADDR] = {'\0'};

    if (aic_is_valid_ipv6_addr(lan_net->lan_global_ipv6_addr)) {
        return;
    }

    strncpy(local_ipv6_str, ANET_SEMI_TETHER_LOCAL_IPV6,
            (ANET_MAX_LEN_HEX_IPV6_ADDR - 1));
    strncpy(global_ipv6_str, ANET_SEMI_TETHER_GLOBAL_IPV6,
            (ANET_MAX_LEN_HEX_IPV6_ADDR - 1));
    inet_pton(AF_INET6, local_ipv6_str, &lan_net->local_ipv6_addr[0]);
    inet_pton(AF_INET6, global_ipv6_str, &lan_net->lan_global_ipv6_addr[0]);

    /* Config IPv6 addrs and dns addrs. */
    inet_pton(AF_INET6, ANET_DEFAULT_IPV6_DNS_SERVER1,
              &lan_net->lan_ipv6_dns1_address[0]);
    inet_pton(AF_INET6, ANET_DEFAULT_IPV6_DNS_SERVER2,
              &lan_net->lan_ipv6_dns2_address[0]);

    for (i = 0; i < 4; i++) {
        NX_CHANGE_ULONG_ENDIAN(lan_net->local_ipv6_addr[i]);
        NX_CHANGE_ULONG_ENDIAN(lan_net->lan_global_ipv6_addr[i]);
        if (i == 3)
            lan_net->lan_global_ipv6_addr[3] += intf_id;

        NX_CHANGE_ULONG_ENDIAN(lan_net->lan_ipv6_dns1_address[i]);
        NX_CHANGE_ULONG_ENDIAN(lan_net->lan_ipv6_dns2_address[i]);
    }

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

struct aic_network_ip_instance *aic_get_semi_tether_ip_instance(void)
{
    return &usb_semi_tether_ip;
}

#ifdef CONFIG_NX_DHCP
/* Config DHCP Server and DHCPV6 Server to assign ip addrs for peer. */
s32 semi_tether_config_for_peer(USB_SEMI_TETHER_INFO *semi_tether_net)
{
    USB_TETHER_LAN_NET *lan_net;
    s32 status = AIC_SUCCESS;

    if (!semi_tether_net || !semi_tether_net->lan_net_ptr) {
        aicx_error(0x0eaca8, "", "invalid semi tether net\n");
        return ANET_ERROR_NULL_USB_NET;
    }

    /* Start DHCP Server. */
    lan_net = semi_tether_net->lan_net_ptr;
    if (semi_tether_net->dhcp_configured != ANET_DHCP_CONFIGURED &&
        lan_net->lan_ipv4_address) {
        if (dhcp_server && dhcp_server->nx_dhcp_started) {
            status = set_semi_tether_dhcp_paras(semi_tether_net);
        } else {
            status = start_semi_tether_dhcp_server(semi_tether_net);
        }
        semi_tether_net->dhcp_configured = ANET_DHCP_CONFIGURED;
    }

    if (!semi_tether_net->dhcpv6_started &&
        aic_is_valid_ipv6_addr(lan_net->lan_global_ipv6_addr)) {

        if (!dhcpv6_server) {
            status = start_dhcpv6_for_semi_tether(semi_tether_net);
            if (!status) {
                semi_tether_net->dhcpv6_started = ANET_DHCPV6_STARTED;
            } else {
                aicx_error(0x0da59e, "x", "semi tether:start DHCPv6 error--0x%x\n", status);
                semi_tether_net->dhcpv6_started = ANET_DHCPV6_NOT_STARTED;
                if (dhcpv6_server) {
                   aos_free(dhcpv6_server);
                   dhcpv6_server = AIC_NULL;
                }
                if (dhcpv6_server_stack) {
                   aos_free(dhcpv6_server_stack);
                   dhcpv6_server_stack = AIC_NULL;
                }
            }
        }
    }

    return status;
}

s32 init_semi_tether_dhchv6_server(void)
{
    s32 status = AIC_SUCCESS;
    struct aic_network_ip_instance *ip_instance;

    ip_instance = aic_get_semi_tether_ip_instance();

    if (!dhcpv6_server) {
        dhcpv6_server = aos_zalloc(sizeof(NX_DHCPV6_SERVER));
        if (!dhcpv6_server) {
            aicx_error(0x0df94f, "", "semi tether:malloc dhcpv6 server error\n");
            return AIC_ENOMEM;
        }

        if (!dhcpv6_server_stack) {
            dhcpv6_server_stack = aos_zalloc(DHCPV6_SERVER_STACK_SIZE);
            if (!dhcpv6_server_stack) {
                aicx_error(0x029dae, "", "semi tether:malloc dhcpv6 server stack fail\n");
                aos_free(dhcpv6_server);
                return AIC_ENOMEM;
            }
        }

        /* Create the DHCPv6 Server. */
        status = nx_dhcpv6_server_create(
                    dhcpv6_server,
                    (NX_IP *)ip_instance->ip_ptr,
                    "Anet_DHCPv6_Server",
                    (NX_PACKET_POOL *)semi_tether_pkt_pool.packet_pool,
                    dhcpv6_server_stack,
                    DHCPV6_SERVER_STACK_SIZE,
                    NX_NULL, NX_NULL);

        if (status)
            aicx_error(0x01bc99, "x", "semi tether:create dhcpv6 error 0x%x\n", status);
    }

    return status;
}

u32 set_semi_tether_dhcpv6_paras(USB_SEMI_TETHER_INFO *semi_tether_net)
{
    u32 status, addresses_added, ifc_index, addr_index, i;
    ulong duid_time;
    NXD_ADDRESS dns_ipv6_address;
    NXD_ADDRESS start_ipv6_address;
    NXD_ADDRESS end_ipv6_address;
    NXD_ADDRESS lan_ipv6_address;
    USB_TETHER_LAN_NET *lan_net;
    NX_INTERFACE *net_intf_ptr;
    aos_systime systime;

    ifc_index = semi_tether_net->lan_intf_index;
    lan_net = semi_tether_net->lan_net_ptr;

    lan_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6;
    for (i = 0; i < 4; i++) {
        lan_ipv6_address.nxd_ip_address.v6[i] =
            lan_net->lan_global_ipv6_addr[i];
    }

    aic_get_ipv6_addr_index(&usb_semi_tether_ip, &lan_ipv6_address, &addr_index);
    //alog_info("%s:ipv6 addr index %d\n", __func__, addr_index);

    if (addr_index < NX_MAX_IPV6_ADDRESSES) {
        status = nx_dhcpv6_server_interface_set(dhcpv6_server,
                                                ifc_index, addr_index);
        if (status) {
            aicx_error(0x0f3936, "x", "semi tether:set addr for dhcpv6 error 0x%x\n", status);
        }
    }

    /* Set up the DNS IPv6 server address. */
    dns_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    for (i = 0; i < 4; i++) {
        dns_ipv6_address.nxd_ip_address.v6[i] = lan_net->lan_ipv6_dns1_address[i];
    }
    status = nx_dhcpv6_create_dns_address(dhcpv6_server,
                                          &dns_ipv6_address);
    if (status) {
        aicx_error(0x1e1ba5, "x", "semi tether:create dhcpv6 dns error 0x%x\n", status);
    }

    //set systime to fixed values.
    systime.year = 2022 - 30;
    systime.month = 9;
    systime.day = 28;
    systime.hour = 10;
    systime.minute = 0;
    systime.second = 0;
    duid_time = aic_mktime_to_seconds(&systime);

    net_intf_ptr = &usb_semi_tether_ip.ip_ptr->nx_ip_interface[ifc_index];

    status = nx_dhcpv6_set_server_duid(
             dhcpv6_server,
             NX_DHCPV6_SERVER_DUID_TYPE, NX_DHCPV6_SERVER_HW_TYPE,
             net_intf_ptr->nx_interface_physical_address_msw,
             net_intf_ptr->nx_interface_physical_address_lsw,
             duid_time);
    if (status) {
        aicx_error(0x030715, "x", "semi tether:set duid error 0x%x\n", status);
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
    if (status) {
        aicx_error(0x148c21, "x", "semi tether:create ip range error 0x%x\n", status);
    }

    return status;
}

s32 start_dhcpv6_for_semi_tether(USB_SEMI_TETHER_INFO *semi_tether_ptr)
{
    s32 status;

    status = init_semi_tether_dhchv6_server();
    if (!status) {
        set_semi_tether_dhcpv6_paras(semi_tether_ptr);

        /* Start the NetX DHCPv6 server. */
        if (!status) {
            status = nx_dhcpv6_server_start(dhcpv6_server);
            if (status) {
                aicx_error(0x1863ff, "x", "semi tether:start dhcpv6 error 0x%x\n", status);
            }
        }
    }

    return status;
}

s32 start_semi_tether_dhcp_server(USB_SEMI_TETHER_INFO *semi_tether_info)
{
    u32 status;
    u8 dhcp_name[20] = {'\0'};

    if (!semi_tether_info || !semi_tether_info->lan_net_ptr) {
        return ANET_ERROR_NULL_USB_NET;
    }

    if (!dhcp_server) {
        dhcp_server = aos_zalloc(sizeof(NX_DHCP_SERVER));
        if (!dhcp_server) {
            aicx_error(0x0f0ca4, "", "semi tether:malloc dhcp server fail\n");
            return AIC_ENOMEM;
        }
    }

    if (!server_dhcp_stack) {
        server_dhcp_stack = aos_zalloc(DHCP_SERVER_STACK_SIZE);
        if (!server_dhcp_stack) {
            aicx_error(0x1aae05, "", "semi tether:malloc dhcp server stack fail\n");
            aos_free(dhcp_server);
            dhcp_server = AIC_NULL;
            return AIC_ENOMEM;
        }
    }

    strncpy(dhcp_name, "USB_DHCP_Server", (sizeof(dhcp_name) - 1));
    status = nx_dhcp_server_create(
             dhcp_server, (NX_IP *)usb_semi_tether_ip.ip_ptr,
             (u8 *)server_dhcp_stack,
             DHCP_SERVER_STACK_SIZE, dhcp_name,
             (NX_PACKET_POOL *)semi_tether_pkt_pool.packet_pool);
    if (status) {
        aicx_error(0x02e2cd, "", "semi tether:create dhcp server error\n");
        goto INIT_ERROR;
    }

    status = set_semi_tether_dhcp_paras(semi_tether_info);
    if (status) {
        aicx_error(0x0f6a5e, "x", "semi tether:set dhcp paras error 0x%x\n", status);
        goto INIT_ERROR;
    }

    status = nx_dhcp_server_start(dhcp_server);
    if (status) {
        aicx_error(0x09ba22, "x", "semi tether:start DHCP server error 0x%x\n", status);
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

u32 set_semi_tether_dhcp_paras(USB_SEMI_TETHER_INFO *semi_tether_ptr)
{
    u32 status, addresses_added;
    USB_TETHER_LAN_NET *lan_net;

    if (!semi_tether_ptr || !dhcp_server) {
        return ANET_ERROR_NULL_USB_NET;
    }

    lan_net = semi_tether_ptr->lan_net_ptr;
    /* Load the assignable DHCP IP addresses for the interface. */
    status = nx_dhcp_create_server_ip_address_list(
             dhcp_server,
             semi_tether_ptr->lan_intf_index,
             lan_net->ipv4_addr_pool_start,
             lan_net->ipv4_addr_pool_end,
             &addresses_added);

    if (status ||
        addresses_added != semi_tether_ptr->lan_net_ptr->ipv4_addr_pool_size) {
        aicx_error(0x1708f9, "x", "semi tether:set ip addr list error 0x%x\n", status);
        return status;
    }

    /* Set the interface network parameters. */
    status = nx_dhcp_set_interface_network_parameters(
             dhcp_server,
             semi_tether_ptr->lan_intf_index,
             lan_net->lan_ipv4_subnet_mask,
             lan_net->lan_ipv4_address,
             lan_net->lan_ipv4_dns1_address);

    if (status) {
        aicx_error(0x0f6a5e, "x", "semi tether:set dhcp paras error 0x%x\n", status);
        return status;
    }

    return AIC_SUCCESS;
}
#endif

s32 create_usb_semi_tether_lan_net(USB_SEMI_TETHER_INFO *semi_tether_ptr)
{
    USB_TETHER_LAN_NET *lan_net;

    semi_tether_ptr->lan_net_ptr =
        (USB_TETHER_LAN_NET *)aos_zalloc(sizeof(USB_TETHER_LAN_NET));
    lan_net = semi_tether_ptr->lan_net_ptr;

    if (lan_net) {
        lan_net->lan_net_id = semi_tether_ptr->lan_net_id;
        lan_net->global_ipv6_addr_index = 0xffff;
        lan_net->local_ipv6_addr_index = 0xffff;
    } else {
        aicx_error(0x081644, "", "malloc semi tether lan net fail\n");
        return AIC_ENOMEM;
    }

    return AIC_SUCCESS;
}

USB_SEMI_TETHER_INFO *get_usb_semi_tether_info(u8 *intf_name)
{
    u8 i;

    for (i = 0; i < ANET_USB_SEMI_TETHER_CNT; i++) {
        if (strncasecmp(usb_semi_tether_list[i].net_interf_name, intf_name,
            strlen(intf_name)) == 0) {
            return &usb_semi_tether_list[i];
        }
    }

    return AIC_NULL;
}

void clear_semi_tether_info(USB_SEMI_TETHER_INFO *semi_tether_net)
{
    if (!semi_tether_net)
        return;

    aos_memset((u8 *)semi_tether_net, 0, sizeof(USB_SEMI_TETHER_INFO));
    semi_tether_net->lan_intf_index = 0xff;
    aos_memset(semi_tether_net->net_interf_name, '\0',
               sizeof(semi_tether_net->net_interf_name));
}

boolean usb_semi_tether_all_stopped(void)
{
    u8 i;

    for (i = 0; i < ANET_USB_SEMI_TETHER_CNT; i++){
        if (usb_semi_tether_list[i].lan_net_id != 0) {
            return AIC_FALSE;
        }
    }

    return AIC_TRUE;
}

s32 usb_semi_tether_stop(USB_SEMI_TETHER_INFO *semi_tether_net)
{
    u16 ifc_index;
#ifdef CONFIG_USB_SERVICE
    u16 net_intf_down;
    u8 usb_intf[ANET_MAX_LEN_INTERF_NAME] = {'\0'};
#endif

    ifc_index = semi_tether_net->lan_intf_index;
    nx_ip_interface_detach((NX_IP *)usb_semi_tether_ip.ip_ptr, ifc_index);

    if (semi_tether_net->lan_net_ptr) {
        aos_free((u8 *)semi_tether_net->lan_net_ptr);
    }

#ifdef CONFIG_NX_DHCP
    if (dhcpv6_server && semi_tether_net->dhcpv6_started) {
        nx_dhcpv6_server_delete(dhcpv6_server);
        aos_free(dhcpv6_server);
        dhcpv6_server = AIC_NULL;
        if (dhcpv6_server_stack) {
            aos_free(dhcpv6_server_stack);
            dhcpv6_server_stack = AIC_NULL;
        }
    }
#endif

#ifdef CONFIG_USB_SERVICE
    strncpy(usb_intf, semi_tether_net->net_interf_name,
            (ANET_MAX_LEN_INTERF_NAME - 1));
#endif
    /* Clear the net info of the index. */
    clear_semi_tether_info(semi_tether_net);

    /** If all usb semi tether info are cleared, delete dhcp and dhcpv6 server,
     * and delete usb_semi_tether_ip.
     */
    if (usb_semi_tether_all_stopped()) {
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

#ifdef CONFIG_USB_SERVICE
        if (aic_is_usb_in_net_mode()) {
            aic_wait_usb_intf_down(usb_intf, old_usb_mode, &net_intf_down);

            if (!net_intf_down) {
                aicx_error(0x0c2aa1, "", "can't down usb intf for semi tether\n");
            }
        }
#endif

        aic_delete_ip_instance(&usb_semi_tether_ip);
    }

    return AIC_SUCCESS;
}

s32 aic_usb_semi_tether_init(void)
{
    u8 i;

    for (i = 0; i < ANET_USB_SEMI_TETHER_CNT; i++) {
        aos_memset(&usb_semi_tether_list[i], 0 ,sizeof(USB_SEMI_TETHER_INFO));
        usb_semi_tether_list[i].lan_intf_index = 0xff;
        usb_semi_tether_list[i].net_interf_id = 0xff;
        aos_memset(usb_semi_tether_list[i].net_interf_name, '\0',
                   sizeof(usb_semi_tether_list[i].net_interf_name));
    }

    aic_add_at_cmd_handle(&semi_tether_at_handles[0]);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_usb_semi_tether_init);

#if 0
/** usbtether start -i rndis0
 * usbtether stop
 */
static s32 aic_net_usb_tether_cmd(int argc, char **argv)
{
    s32 ret;
    u8 i, enable_tether = 0xff;
    u16 usb_intf_up = AIC_FALSE;
    USB_SEMI_TETHER_INFO *usb_semi_tether = AIC_NULL;

    //UNUSED_PARAM(argv);

    aicx_info(0x02db74, "sd", "%s:para count %d\n", __func__, argc);
    for (i = 0; i < argc; i++) {
        aicx_info(0x13e6f2, "sds", "%s:para %d -- %s\n", __func__, i, argv[i]);
        if (i == 0) {
            if (!strncmp(argv[i], "start", strlen("start"))) {
                enable_tether = ANET_USB_SEMI_TETHER_START;
            } else if (!strncmp(argv[i], "stop", strlen("stop"))) {
                enable_tether = ANET_USB_SEMI_TETHER_STOP;
            } else {
                aicx_info(0x1adf8e, "ss", "%s:error para -- %s\n", __func__, argv[i]);
                return AIC_EPFNOSUPPORT;
            }
            continue;
        }

        if (*(argv[i]) == '-' &&
            ((*(argv[i] + 1) >= 65) && (*(argv[i] + 1) <= 121))) {

            switch (*(argv[i] + 1)) {
            /* -i indicates the interface */
            case 'i':
                i++;
                if (i < argc) {
                    aicx_info(0x01e7af, "ss", "%s:interface is %s\n", __func__, argv[i]);

                    usb_semi_tether = get_usb_semi_tether_info(argv[i]);
                    if (usb_semi_tether) {
                        if (enable_tether == ANET_USB_SEMI_TETHER_START) {
                            aicx_info(0x0608d2, "ss", "%s:interf %s is enaged\n", __func__, argv[i]);
                            return ANET_ERROR_INTF_ENGAGED;
                        } else if (enable_tether == ANET_USB_SEMI_TETHER_STOP) {
                            usb_semi_tether_stop(usb_semi_tether);
                            break;
                        }
                    } else {
                        if (enable_tether == ANET_USB_SEMI_TETHER_START) {
                            usb_semi_tether = get_idle_usb_semi_tether();
                            if (!usb_semi_tether) {
                                aicx_info(0x06d916, "s", "%s:no idle usb semi tether\n", __func__);
                                return ANET_ERROR_USB_SEMI_TETHER_OVERFLOW;
                            }
                            usb_semi_tether->tether_enabled = enable_tether;
                            strncpy(usb_semi_tether->net_interf_name, argv[i],
                                    (ANET_MAX_LEN_INTERF_NAME - 1));
                        } else if (enable_tether == ANET_USB_SEMI_TETHER_STOP) {
                            aicx_info(0x0232a0, "ss", "%s:error,interf %s was not tethered\n",
                                     __func__, argv[i]);
                            return ANET_ERROR_INVALID_USB_INTF;
                        }
                    }
                }
                break;

            default:
                aicx_info(0x003a47, "ss", "%s:not support para %s\n", __func__, argv[i]);
                return AIC_EPFNOSUPPORT;
            }
        }
    }

    if (enable_tether == ANET_USB_SEMI_TETHER_START) {
        aic_wait_usb_intf_up(usb_semi_tether->net_interf_name, &usb_intf_up);
        if (!usb_intf_up) {
            aicx_info(0x15ba26, "s", "%s:error,can't make usb intf up\n", __func__);
            ret = ANET_ERROR_USB_INTF_DOWN;
            goto INTERNAL_ERROR;
        }

        ret = usb_semi_tether_start(usb_semi_tether,
                                    ANET_SHARE_MODE_ROUTING);
        if (ret) {
            aicx_info(0x16bc78, "sx", "%s:start error 0x%x\n", __func__, ret);
            goto INTERNAL_ERROR;
        }
    }

    return AIC_SUCCESS;

INTERNAL_ERROR:
    usb_semi_tether_stop(usb_semi_tether);

    return ret;
}

CONSOLE_EXPORT_CMD(usbtether, aic_net_usb_tether_cmd, usb net semi tether);
#endif

