/***************************************************************************
 * Copyright (c) 2024 Corechips Corporation
 *
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 *
 * SPDX-License-Identifier: MIT
 **************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   bk7238 NIC                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x085ce4
#define LOG_MODULE    AMOD_VENDOR

#include "nx_api.h"
#include <ssp.h>
#include "nx_api.h"
#include "aic_size.h"
#include "aic_errno.h"
#include <aic_init.h>
#include <aic_type.h>
#include <gpio_inc.h>
//#include "aic_net_gmac_mgr.h"
#include <eipu.h>
#include <stdio.h>
#include <aic_softap_mgr.h>
#include <aic_network_pcap.h>
#include <time.h>
#include <aic_net_api.h>
#include <aic_packet_management.h>
#include <aic_net_share_common.h>
#include "spi_protocol.h"
#ifdef CONFIG_NET_PCAP_DUMP
#include <aic_network_pcap.h>
#endif
#include <dvfs_inc.h>

#define BK7238_TX_MUTEX    "BK7238_TX_MUTEX"

#define NX_LINK_MTU        1514

#define NX_ETHERNET_IP     0x0800
#define NX_ETHERNET_ARP    0x0806
#define NX_ETHERNET_RARP   0x8035
#define NX_ETHERNET_IPV6   0x86DD
#define NX_ETHERNET_SIZE   14

/* Define driver prototypes.  */

VOID bk7238_network_driver(NX_IP_DRIVER *driver_req_ptr);
void bk7238_network_driver_output(NX_PACKET *packet_ptr,
                                  UINT interface_instance_id);
void bk7238_network_driver_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                   UINT interface_instance_id);
int bk7238_init(UINT interface_instance_id);
VOID bk7238_network_driver_entry(NX_IP_DRIVER *driver_req_ptr);

#define NX_MAX_BK7238_INTERFACES             4
#define NX_BK7238_DRIVER_MAX_MCAST_ADDRESSES 3
typedef struct MAC_ADDRESS_STRUCT
{
    ULONG nx_mac_address_msw;
    ULONG nx_mac_address_lsw;
} MAC_ADDRESS;


/* Define an application-specific data structure that holds internal
   data (such as the state information) of a device driver.

   The example below applies to the simulated RAM driver.
   User shall replace its content with information related to
   the actual driver being used. */
typedef struct bk7238_network_driver_instance_type
{
    u32            ssp_handle;
    NX_PACKET_POOL *bk7238_pool;
    volatile UINT  bExit;
    UINT           bk7238_network_driver_in_use;

    UINT           bk7238_network_driver_id;

    NX_INTERFACE   *bk7238_driver_interface_ptr;

    NX_IP          *bk7238_driver_ip_ptr;

    MAC_ADDRESS    bk7238_driver_mac_address;

    MAC_ADDRESS    bk7238_driver_mcast_address[NX_BK7238_DRIVER_MAX_MCAST_ADDRESSES];
} bk7238_network_driver_instance_type;


/* In this example, there are four instances of the simulated RAM driver.
   Therefore an array of four driver instances are created to keep track of
   the interface information of each driver. */
static bk7238_network_driver_instance_type bk7238_driver[NX_MAX_BK7238_INTERFACES];
static aos_mutex_ptr bk7238_tx_mutex_handle = AOS_NULL;
static aos_event_ptr bk7238_interrupt_event = AOS_NULL;

int bk7238_ssp_init(u32 *handle)
{
    struct ssp_cfg cfg;
    int ret;

    memset(&cfg, 0x0, sizeof(cfg));
    cfg.spi_mode = SPI_MODE_0;
    cfg.cs = 0;
    cfg.work_clk = 50000000;

    ret = ssp_device_register(handle, "ssp_dev0",
                              "ssp0", &cfg);
    alog_info("BK7238_ssp_init:%d\n", ret);
    return ret;
}

void bk7238_init_mac_addr(bk7238_network_driver_instance_type *dr_ins)
{
    u8 buffer[18];
    u8 mac[6];
    u8 a, b;
    int i;

    get_mac(dr_ins->ssp_handle, buffer, 18);

    for (i = 0; i < 18; i += 3) {
        if (buffer[i] >= 'a')
            a = buffer[i] - 'a' + 10;
        else if (buffer[i] >= 'A')
            a = buffer[i] - 'A' + 10;
        else
            a = buffer[i] - 0x30;

        if (buffer[i + 1] >= 'a')
            b = buffer[i + 1] - 'a' + 10;
        else if (buffer[i + 1] >= 'A')
            b = buffer[i + 1] - 'A' + 10;
        else
            b = buffer[i + 1] - 0x30;
        mac[i / 3] = (a << 4) | b;
    }
    alog_info("[BK7238] mac:%x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2],
              mac[3], mac[4], mac[5]);
    dr_ins->bk7238_driver_mac_address.nx_mac_address_msw = ((u32)mac[0] << 8) | mac[1];
    dr_ins->bk7238_driver_mac_address.nx_mac_address_lsw = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) |
        ((u32)mac[4] << 8) | mac[5];
}

static int bk7238_rx(u32 handle, u8 *pbuf, u16 *plength, u16 *next_len, int first)
{
    int ret;

    *plength = 0;
    if (!*next_len)
        ret = receive_next_pkt(handle, pbuf, 1536, next_len);
    else
        ret = receive_next_pkt(handle, pbuf, *next_len, next_len);
    if (ret <= 0) {
        if (ret != -4)
            alog_warn("[BK7238] recv pkt fail %d\n", ret);
        return ret;
    }

    alog_debug("[BK7238] recv pkt length %d\n", ret);
    *plength = ret;
    return NX_SUCCESS;
}

static void bk7238_interrupt_callback(u32 index, u32 value, u32 eipu_pol)
{
    aos_set_event(bk7238_interrupt_event, 0x01, AOS_OR);
}

static void bk7238_loop_thread(void *instance_id)
{
    bk7238_network_driver_instance_type *dr_ins =
        (bk7238_network_driver_instance_type*)&bk7238_driver[(u32)instance_id];
    u32 handle = dr_ins->ssp_handle;
    NX_PACKET *packet_ptr = AOS_NULL;
    u32 event, status, first = TRUE;
    u16 rxlength, next_len = 0;
    uint8_t buffer[64] = {0};

    aic_dvfs_boost(TRUE);
    aic_gpio_input(14, GPIO_INPUT_NOINT, NULL, NULL);
    bk7238_ssp_init(&dr_ins->ssp_handle);

    handle = dr_ins->ssp_handle;

    bk7238_init_mac_addr(dr_ins);

    set_ap(handle, "aic_bk", "");
    get_ip(handle, buffer, 64);
    status = aic_config_softap_intf("AIC_BK_AP", bk7238_network_driver_entry);
    if (status != NX_SUCCESS) {
        alog_info("config softap intf %s fail\n", "AIC_BK_AP");
        return;
    }

    while (dr_ins->bExit != true) {
        aos_get_event(bk7238_interrupt_event, 0x01, AOS_OR_CLEAR, &event,
                      AOS_WAIT_FOREVER);
        first = TRUE;
        next_len = 0;
        do {
            if (packet_ptr == AOS_NULL) {
                status = aic_network_alloc_usb_packet(&packet_ptr, 1536);
                if (status != NX_SUCCESS) {
                    aos_sleep(1);
                    continue;
                }
            }

            if (next_len > 1536)
                next_len = 1536;

            bk7238_rx(handle, packet_ptr->nx_packet_prepend_ptr, &rxlength, &next_len, first);
            if (next_len)
                first = FALSE;
            else
                first = TRUE;
            if (rxlength > 20) {
                packet_ptr->nx_packet_append_ptr += rxlength;
                packet_ptr->nx_packet_length = rxlength;

                bk7238_network_driver_receive(dr_ins->bk7238_driver_ip_ptr, packet_ptr,
                                              (UINT)instance_id);
                packet_ptr = AOS_NULL;
            } else {
                break;
            }
        } while (next_len);
        next_len = 0;
        EIPU_RegCallback_Ex(7, EIPU_DBNC, bk7238_interrupt_callback,
                            HIGH_LEVEL, 0x00, NOSEC_MODE, SLEEP_WK_MODE);
    };
}

int bk7238_init( UINT interface_instance_id)
{
    int status = NX_SUCCESS;

    bk7238_tx_mutex_handle = aos_create_mutex(BK7238_TX_MUTEX);
    bk7238_interrupt_event = aos_create_event("bk7238_inter_event");

    EIPU_RegCallback_Ex(7, EIPU_DBNC, bk7238_interrupt_callback,
                        HIGH_LEVEL, 0x00, NOSEC_MODE, SLEEP_WK_MODE);

    if (aos_create_thread("bk7238rx", bk7238_loop_thread,
                          (void *)interface_instance_id, SIZE_2K, 0, NULL, 10,
                          AOS_AUTO_START) == AOS_INVALID_THREAD_ID) {
        return AIC_EPERM;
    }

    return status;
}

VOID bk7238_network_driver_entry(NX_IP_DRIVER *driver_req_ptr)
{
    UINT          i = 0;
    NX_IP        *ip_ptr;
    NX_PACKET    *packet_ptr;
    NX_INTERFACE *interface_ptr;
    UINT          interface_index;
    USHORT        ether_type;
    ULONG        *ethernet_frame_ptr;

    /* Setup the IP pointer from the driver request.  */
    ip_ptr =  driver_req_ptr->nx_ip_driver_ptr;

    /* Default to successful return.  */
    driver_req_ptr->nx_ip_driver_status =  NX_SUCCESS;

    /* Setup interface pointer.  */
    interface_ptr = driver_req_ptr->nx_ip_driver_interface;

    /* Obtain the index number of the network interface. */
    interface_index = interface_ptr->nx_interface_index;
    if (driver_req_ptr->nx_ip_driver_command != NX_LINK_INTERFACE_ATTACH) {
        for (i = 0; i < NX_MAX_BK7238_INTERFACES; i++) {
            if (bk7238_driver[i].bk7238_network_driver_in_use == 0)
                continue;

            if (bk7238_driver[i].bk7238_driver_ip_ptr != ip_ptr)
                continue;

            if (bk7238_driver[i].bk7238_driver_interface_ptr == interface_ptr)
                break;
        }

        if (i == NX_MAX_BK7238_INTERFACES) {
            driver_req_ptr->nx_ip_driver_status =  NX_INVALID_INTERFACE;
            return;
        }
    }

    switch (driver_req_ptr->nx_ip_driver_command) {
    case NX_LINK_INTERFACE_ATTACH:
        /* Find an available driver instance to attach the interface. */
        for (i = 0; i < NX_MAX_BK7238_INTERFACES; i++)
            if (bk7238_driver[i].bk7238_network_driver_in_use == 0)
                break;

        /* An available entry is found. */
        if (i < NX_MAX_BK7238_INTERFACES) {
            bk7238_driver[i].bk7238_pool = aic_get_pdp_ip_instance()->packet_pool;
            /* Set the IN USE flag.*/
            bk7238_driver[i].bk7238_network_driver_in_use  = 1;

            bk7238_driver[i].bk7238_network_driver_id = i;

            /* Record the interface attached to the IP instance. */
            bk7238_driver[i].bk7238_driver_interface_ptr = interface_ptr;

            /* Record the IP instance. */
            bk7238_driver[i].bk7238_driver_ip_ptr = ip_ptr;
        } else {
            driver_req_ptr->nx_ip_driver_status =  NX_INVALID_INTERFACE;
        }

        break;
    case NX_LINK_INTERFACE_DETACH:
        bk7238_driver[i].bk7238_network_driver_in_use = NX_FALSE;
        break;

    case NX_LINK_INITIALIZE:
        /* Once the Ethernet controller is initialized, the driver needs to
           configure the NetX Interface Control block, as outlined below. */

        /* The nx_interface_ip_mtu_size should be the MTU for the IP payload.
           For regular Ethernet, the IP MTU is 1500. */
        nx_ip_interface_mtu_set(ip_ptr, interface_index, (NX_LINK_MTU - NX_ETHERNET_SIZE));

        /* Set the physical address (MAC address) of this IP instance.  */
        /* For this simulated RAM driver, the MAC address is constructed by
           incrementing a base lsw value, to simulate multiple nodes on the
           ethernet.  */
        nx_ip_interface_physical_address_set(ip_ptr, interface_index,
                                             bk7238_driver[i].bk7238_driver_mac_address.nx_mac_address_msw,
                                             bk7238_driver[i].bk7238_driver_mac_address.nx_mac_address_lsw, NX_FALSE);

        /* Indicate to the IP software that IP to physical mapping is required.  */
        nx_ip_interface_address_mapping_configure(ip_ptr, interface_index,
                                                  NX_TRUE);
        break;
    case NX_LINK_UNINITIALIZE:
        bk7238_driver[i].bk7238_network_driver_in_use = NX_FALSE;
        break;
    case NX_LINK_ENABLE:
        interface_ptr->nx_interface_link_up = NX_TRUE;
        break;
    case NX_LINK_DISABLE:
        interface_ptr->nx_interface_link_up = NX_FALSE;
        break;
    case NX_LINK_PACKET_SEND:
    case NX_LINK_PACKET_BROADCAST:
    case NX_LINK_ARP_SEND:
    case NX_LINK_ARP_RESPONSE_SEND:
    case NX_LINK_RARP_SEND:
        packet_ptr =  driver_req_ptr->nx_ip_driver_packet;

        if (driver_req_ptr ->nx_ip_driver_command == NX_LINK_ARP_SEND)
            ether_type = NX_ETHERNET_ARP;
        else if (driver_req_ptr ->nx_ip_driver_command == NX_LINK_ARP_RESPONSE_SEND)
            ether_type = NX_ETHERNET_ARP;
        else if (driver_req_ptr ->nx_ip_driver_command == NX_LINK_RARP_SEND)
            ether_type = NX_ETHERNET_RARP;
        else if (packet_ptr ->nx_packet_ip_version == NX_IP_VERSION_V4)
            ether_type = NX_ETHERNET_IP;
        else
            ether_type = NX_ETHERNET_IPV6;

        packet_ptr->nx_packet_prepend_ptr = packet_ptr ->nx_packet_prepend_ptr - NX_ETHERNET_SIZE;

        packet_ptr->nx_packet_length = packet_ptr ->nx_packet_length + NX_ETHERNET_SIZE;

        ethernet_frame_ptr = (ULONG *)(packet_ptr ->nx_packet_prepend_ptr - 2);

        /* Build the ethernet frame.  */
        *ethernet_frame_ptr       = driver_req_ptr ->nx_ip_driver_physical_address_msw;
        *(ethernet_frame_ptr + 1) = driver_req_ptr ->nx_ip_driver_physical_address_lsw;
        *(ethernet_frame_ptr + 2) = (interface_ptr ->nx_interface_physical_address_msw << 16) |
            (interface_ptr ->nx_interface_physical_address_lsw >> 16);
        *(ethernet_frame_ptr + 3) = (interface_ptr ->nx_interface_physical_address_lsw << 16) | ether_type;

        NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr));
        NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 1));
        NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 2));
        NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 3));
#ifdef CONFIG_NET_PCAP_DUMP
        aic_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr + NX_ETHERNET_SIZE,
                              packet_ptr->nx_packet_length - NX_ETHERNET_SIZE,
                              AIC_USB_NIC_FLAG);
#endif
        bk7238_network_driver_output(packet_ptr, 0);
        break;
    case NX_LINK_MULTICAST_JOIN:
        break;
    case NX_LINK_MULTICAST_LEAVE:
        break;
    case NX_LINK_GET_STATUS:
        *(driver_req_ptr ->nx_ip_driver_return_ptr) =
            interface_ptr->nx_interface_link_up;
        break;
    case NX_LINK_GET_SPEED:
    case NX_LINK_GET_DUPLEX_TYPE:
    case NX_LINK_GET_ERROR_COUNT:
    case NX_LINK_GET_RX_COUNT:
    case NX_LINK_GET_TX_COUNT:
    case NX_LINK_GET_ALLOC_ERRORS:
    case NX_LINK_GET_INTERFACE_TYPE:
        /* Return the link's interface type in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr ->nx_ip_driver_return_ptr) = NX_INTERFACE_TYPE_UNKNOWN;
        break;

    case NX_LINK_DEFERRED_PROCESSING:
        break;

    case NX_LINK_SET_PHYSICAL_ADDRESS:
        for (i = 0; i < NX_MAX_BK7238_INTERFACES; i++)
            if (bk7238_driver[i].bk7238_driver_interface_ptr == interface_ptr)
                break;

        if (i < NX_MAX_BK7238_INTERFACES) {
            /* Set the physical address.  */
            bk7238_driver[i].bk7238_driver_mac_address.nx_mac_address_msw =
                driver_req_ptr ->nx_ip_driver_physical_address_msw;
            bk7238_driver[i].bk7238_driver_mac_address.nx_mac_address_lsw =
                driver_req_ptr ->nx_ip_driver_physical_address_lsw;
        } else {
            driver_req_ptr ->nx_ip_driver_status =  NX_INVALID_INTERFACE;
        }
        break;
    default:
        driver_req_ptr ->nx_ip_driver_status =  NX_UNHANDLED_COMMAND;
        break;
    }
}

void bk7238_network_driver_output(NX_PACKET *packet_ptr, UINT instance_id)
{
    bk7238_network_driver_instance_type *dr_ins =
        (bk7238_network_driver_instance_type *)&bk7238_driver[instance_id];
    u32 handle = dr_ins->ssp_handle;

    aos_get_mutex(bk7238_tx_mutex_handle, AOS_WAIT_FOREVER);
    send_net_pkt(handle, packet_ptr->nx_packet_prepend_ptr,
                 packet_ptr->nx_packet_length);
    aos_put_mutex(bk7238_tx_mutex_handle);

    packet_ptr ->nx_packet_prepend_ptr =  packet_ptr ->nx_packet_prepend_ptr + NX_ETHERNET_SIZE;
    packet_ptr ->nx_packet_length =  packet_ptr ->nx_packet_length - NX_ETHERNET_SIZE;
    nx_packet_transmit_release(packet_ptr);
}

void bk7238_network_driver_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                   UINT instance_id)
{
    UINT packet_type;

#ifdef CONFIG_NET_PCAP_DUMP
    aic_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr + NX_ETHERNET_SIZE,
                          packet_ptr->nx_packet_length - NX_ETHERNET_SIZE,
                          AIC_USB_NIC_FLAG);
#endif
    packet_type = (((UINT)(*(packet_ptr ->nx_packet_prepend_ptr + 12))) << 8) |
        ((UINT)(*(packet_ptr ->nx_packet_prepend_ptr + 13)));

    alog_debug("[MAC] packet type 0x%x\n", packet_type);
    packet_ptr ->nx_packet_address.nx_packet_interface_ptr =
        bk7238_driver[instance_id].bk7238_driver_interface_ptr;
    if ((packet_type == NX_ETHERNET_IP) || (packet_type == NX_ETHERNET_IPV6)) {
        packet_ptr->nx_packet_prepend_ptr =
            packet_ptr->nx_packet_prepend_ptr + NX_ETHERNET_SIZE;

        packet_ptr->nx_packet_length = packet_ptr->nx_packet_length - NX_ETHERNET_SIZE;
        packet_ptr->nx_packet_ip_header = packet_ptr->nx_packet_prepend_ptr;

        _nx_ip_packet_receive(ip_ptr, packet_ptr);
    }
#ifndef NX_DISABLE_IPV4
#ifndef NX_DISABLE_IPV4_ARP
    else if (packet_type == NX_ETHERNET_ARP)
    {
        packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr + NX_ETHERNET_SIZE;
        packet_ptr->nx_packet_length = packet_ptr->nx_packet_length - NX_ETHERNET_SIZE;
        _nx_arp_packet_deferred_receive(ip_ptr, packet_ptr);
    }
#endif
#ifndef NX_DISABLE_IPV4_RARP
    else if (packet_type == NX_ETHERNET_RARP)
    {
        packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr + NX_ETHERNET_SIZE;
        packet_ptr->nx_packet_length =  packet_ptr->nx_packet_length - NX_ETHERNET_SIZE;
        _nx_rarp_packet_deferred_receive(ip_ptr, packet_ptr);
    }
#endif
#endif
    else
    {
        nx_packet_release(packet_ptr);
    }
}

static int bk7238_network_init(void)
{
    init_spi_buffers();

    bk7238_init(0);

    return 0;
}
DIS_CHARGE_APP_L2_INIT_EXPORT(bk7238_network_init);

