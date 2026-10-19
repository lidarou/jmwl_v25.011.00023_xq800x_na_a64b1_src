/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** AIC Component                                                            */
/**                                                                          */
/**   ifconfig                                                               */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07d38f
#define LOG_MODULE    AMOD_NET

#include <aic_cmd.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_net_api.h>
#ifndef WIN32
#include <aic_cmd_api.h>
#endif

#include "nx_ip.h"
#include "nx_api.h"
#include "nx_packet.h"

static int aic_network_ifconfig_cmd(int argc, char **argv)
{
    NX_IP *ip_ptr;
    int i,j;
    NX_INTERFACE *interface_ptr;
    struct NXD_IPV6_ADDRESS_STRUCT *ipv6_address_list_head;
    struct aic_network_ip_instance *aic_ip_ptr;

    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr) {
        aicx_info(0x08fedd, "", "[AT] ip instance not ready when set ctcc mode\n");
        return AIC_ECANCELED;
    }

    ip_ptr = _nx_ip_created_ptr;

    while (ip_ptr) {
        CONS_OUT("\nIP instance name: %s\tgateway address %d.%d.%d.%d ctcc mode %d\n",
                 ip_ptr->nx_ip_name,
                 (ip_ptr->nx_ip_gateway_address >> 24) & 0xFF,
                 (ip_ptr->nx_ip_gateway_address >> 16) & 0xFF,
                 (ip_ptr->nx_ip_gateway_address >> 8) & 0xFF,
                 (ip_ptr->nx_ip_gateway_address) & 0xFF,
                 aic_ip_ptr->ctcc_auth_mode);
        if (ip_ptr->nx_ip_gateway_interface)
            CONS_OUT("gateway interface: %s\n\n",
                     ip_ptr->nx_ip_gateway_interface->nx_interface_name);

        for (i = 0; i < NX_MAX_IP_INTERFACES; i++) {
            interface_ptr = &ip_ptr->nx_ip_interface[i];
            if (interface_ptr->nx_interface_link_up) {
                ipv6_address_list_head =
                    interface_ptr->nxd_interface_ipv6_address_list_head;
                CONS_OUT("\tindex %d   interface name %s\n",
                         interface_ptr->nx_interface_index,
                         interface_ptr->nx_interface_name);
                CONS_OUT("\tip address: %d.%d.%d.%d\n",
                         (interface_ptr->nx_interface_ip_address >> 24) & 0xFF,
                         (interface_ptr->nx_interface_ip_address >> 16) & 0xFF,
                         (interface_ptr->nx_interface_ip_address >> 8) & 0xFF,
                         (interface_ptr->nx_interface_ip_address) & 0xFF);
                CONS_OUT("\tip network mask: %d.%d.%d.%d\n",
                         (interface_ptr->nx_interface_ip_network_mask >> 24) & 0xFF,
                         (interface_ptr->nx_interface_ip_network_mask >> 16) & 0xFF,
                         (interface_ptr->nx_interface_ip_network_mask >> 8) & 0xFF,
                         (interface_ptr->nx_interface_ip_network_mask) & 0xFF);
                CONS_OUT("\tip network: %d.%d.%d.%d\n",
                         (interface_ptr->nx_interface_ip_network >> 24) & 0xFF,
                         (interface_ptr->nx_interface_ip_network >> 16) & 0xFF,
                         (interface_ptr->nx_interface_ip_network >> 8) & 0xFF,
                         (interface_ptr->nx_interface_ip_network) & 0xFF);
#ifdef CONFIG_NET_TRAFFIC_CONTROL
                CONS_OUT("\tip tc: [SEND]: %d %d %d  [RECV]: %d %d %d\n",
                         interface_ptr->nx_interface_send_current_speed,
                         interface_ptr->nx_interface_send_max_speed,
                         interface_ptr->nx_interface_send_max_burst_speed,
                         interface_ptr->nx_interface_recv_current_speed,
                         interface_ptr->nx_interface_recv_max_speed,
                         interface_ptr->nx_interface_recv_max_burst_speed);
#endif

                for (j = 0; ipv6_address_list_head; j++) {
                    CONS_OUT("\tipv6 address_%d: status 0x%x %x:%x:%x:%x:%x:%x:%x:%x/%d\n",
                             j, ipv6_address_list_head->nxd_ipv6_address_state,
                             ipv6_address_list_head->nxd_ipv6_address[0] >> 16,
                             ipv6_address_list_head->nxd_ipv6_address[0] & 0xFFFF,
                             ipv6_address_list_head->nxd_ipv6_address[1] >> 16,
                             ipv6_address_list_head->nxd_ipv6_address[1] & 0xFFFF,
                             ipv6_address_list_head->nxd_ipv6_address[2] >> 16,
                             ipv6_address_list_head->nxd_ipv6_address[2] & 0xFFFF,
                             ipv6_address_list_head->nxd_ipv6_address[3] >> 16,
                             ipv6_address_list_head->nxd_ipv6_address[3] & 0xFFFF,
                             ipv6_address_list_head->nxd_ipv6_address_prefix_length);

                    ipv6_address_list_head =
                        ipv6_address_list_head->nxd_ipv6_address_next;
                }
                CONS_OUT("\n\n");
            }
        }

        ip_ptr = ip_ptr->nx_ip_created_next;
        if (ip_ptr == _nx_ip_created_ptr)
            break;
    }

    return 0;
}
#ifndef WIN32
CONSOLE_EXPORT_CMD(ifconfig, aic_network_ifconfig_cmd, aic network ifconfig);
#endif

