/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/******************************************************************************/
/**                                                                           */
/** demo_netx_nat.c                                                           */
/**                                                                           */
/**   This is a small demo of NAT (Network Address Translation) on the        */
/**   high-performance NetX TCP/IP stack.  This demo relies on ThreadX,       */
/**   NetX and NAT APIs to perform network address translation for IP         */
/**   packets traveling between private and external networks.this demo       */
/**   concentrates on the ICMP ping operation.                                */
/**                                                                           */
/******************************************************************************/
/******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x151ad0
#define LOG_MODULE    AMOD_NET

#include <aic_osa.h>
#include <aic_init.h>
#include <aic_net_api.h>

#include "nx_api.h"
#include "nx_nat.h"
#include "nx_api.h"
#include "nxd_bsd.h"

#if defined NX_NAT_ENABLE && defined __PRODUCT_NETXDUO__ && \
        (NX_MAX_PHYSICAL_INTERFACES >= 2)

#define     DEMO_STACK_SIZE         2048
#define     DEMO_DATA               "ABCDEFGHIJKLMNOPQRSTUVWXYZ "

/**
 * Set NetX IP packet pool packet size. This should be less than the Maximum
 * Transmit Unit (MTU) of the driver (allow enough room for the Ethernet header
 * plus padding bytes for frame alignment).
 */
#define NX_NAT_PACKET_SIZE                          1536

/* Set the size of the NAT IP packet pool. */
#define NX_NAT_PACKET_POOL_SIZE                     (NX_NAT_PACKET_SIZE * 10)

/* Set NetX IP helper thread stack size. */
#define NX_NAT_IP_THREAD_STACK_SIZE                 2048

/* Set the server IP thread priority */
#define NX_NAT_IP_THREAD_PRIORITY                   2

/* Set ARP cache size of a NAT ip instance. */
#define NX_NAT_ARP_CACHE_SIZE                       1024

/* Set NAT entries memory size. */
#define NX_NAT_ENTRY_CACHE_SIZE                     1024

/**
 * Define NAT IP addresses, local host private IP addresses and external host IP
 * address.
 */
#define NX_NAT_LOCAL_IPADR              (IP_ADDRESS(192, 168, 2, 1))
#define NX_NAT_LOCAL_HOST1              (IP_ADDRESS(192, 168, 2, 3))
#define NX_NAT_LOCAL_HOST2              (IP_ADDRESS(192, 168, 2, 10))
#define NX_NAT_LOCAL_GATEWAY            (IP_ADDRESS(192, 168, 2, 1))
#define NX_NAT_LOCAL_NETMASK            (IP_ADDRESS(255, 255, 255, 0))
#define NX_NAT_EXTERNAL_IPADR           (IP_ADDRESS(192, 168, 0, 10))
#define NX_NAT_EXTERNAL_HOST            (IP_ADDRESS(192, 168, 0, 100))
#define NX_NAT_EXTERNAL_GATEWAY         (IP_ADDRESS(192, 168, 0, 1))
#define NX_NAT_EXTERNAL_NETMASK         (IP_ADDRESS(255, 255, 255, 0))

/* Create NAT structures for preloading NAT tables with static
   entries for local server hosts. */
static NX_NAT_TRANSLATION_ENTRY server_inbound_entry_icmp;

static aos_thread_id ntest_0;
static struct aic_network_ip_instance nat_ip, local_ip, external_ip;
static NX_NAT_DEVICE nat_server;
static struct aic_packet_pool nat_packet_pool;
static u32 error_counter = 0;
/* Define thread prototypes. */
//extern void _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

/* Define the test threads. */
static void ntest_0_entry(void *thread_input)
{
    u32 status;
    NX_PACKET *my_packet;

    NX_PARAMETER_NOT_USED(thread_input);

    /***********************************/
    /*       Disable NAT feature       */
    /***********************************/
    /* Local Host ping External Host address. */
    status = nx_icmp_ping(local_ip.ip_ptr, NX_NAT_EXTERNAL_HOST, DEMO_DATA,
                          sizeof(DEMO_DATA), &my_packet, NX_IP_PERIODIC_RATE);
    if (status == NX_SUCCESS) {
        error_counter++;
        aicx_info(0x016518, "", "[NAT] local ip ping fail\n");
        return;
    }

    /* Check the NAT forwarded count. */
#ifndef NX_DISABLE_NAT_INFO
    if ((nat_server.forwarded_packets_received != 0) ||
        (nat_server.forwarded_packets_sent != 0) ||
        (nat_server.forwarded_packets_dropped != 0)) {
        error_counter++;
        aicx_info(0x153e90, "", "[NAT] local ip ping forward error\n");
        return;
    } else {
        aicx_info(0x168d01, "", "[NAT] local ip ping forward successfully\n");
    }
#endif

    /**
     * External Host ping NAT External address, NAT IP instance will
     * response the requet.
     */
    status = nx_icmp_ping(external_ip.ip_ptr, NX_NAT_EXTERNAL_IPADR, DEMO_DATA,
                          sizeof(DEMO_DATA), &my_packet, NX_IP_PERIODIC_RATE);
    if ((status != NX_SUCCESS) || (my_packet == NX_NULL) ||
        (my_packet->nx_packet_length != 28)) {
        error_counter++;
        aicx_info(0x19718c, "", "[NAT] external ip ping fail\n");
        return;
    }

    /* Check the NAT forwarded count. */
#ifndef NX_DISABLE_NAT_INFO
    if ((nat_server.forwarded_packets_received != 0) ||
        (nat_server.forwarded_packets_sent != 0) ||
        (nat_server.forwarded_packets_dropped != 0)) {
        error_counter++;
        aicx_info(0x1e73e7, "", "[NAT] extern ip ping forward error\n");
        return;
    } else {
        aicx_info(0x0ebbfa, "", "[NAT] extern ip ping forward successfully\n");
    }
#endif

    /* Enable the NAT service. */
    nx_nat_enable(&nat_server);

    /* Local Host ping External Host address. */
    status = nx_icmp_ping(local_ip.ip_ptr, NX_NAT_EXTERNAL_HOST, DEMO_DATA,
                          sizeof(DEMO_DATA), &my_packet, NX_IP_PERIODIC_RATE);
    if ((status != NX_SUCCESS) || (my_packet == NX_NULL) ||
        (my_packet->nx_packet_length != 28)) {
        error_counter++;
        aicx_info(0x1538b1, "", "[NAT_ENABLE] local ip ping error\n");
        return;
    }

    /* Check the NAT forwarded count. */
#ifndef NX_DISABLE_NAT_INFO
    if ((nat_server.forwarded_packets_received != 2) ||
        (nat_server.forwarded_packets_sent != 2) ||
        (nat_server.forwarded_packets_dropped != 0)) {
        error_counter++;
        aicx_info(0x149cb7, "", "[NAT_ENABLE] local ip ping forword error\n");
        return;
    } else {
        aicx_info(0x0f16f5, "", "[NAT_ENABLE] local ip ping forword successfully\n");
    }
#endif

    /**
     * External Host ping NAT External address, NAT IP instance will response
     * the requet.
     */
    status = nx_icmp_ping(external_ip.ip_ptr, NX_NAT_EXTERNAL_IPADR, DEMO_DATA,
                          sizeof(DEMO_DATA), &my_packet, NX_IP_PERIODIC_RATE);
    if ((status != NX_SUCCESS) || (my_packet == NX_NULL) ||
        (my_packet->nx_packet_length != 28)) {
        error_counter++;
        aicx_info(0x10c76e, "", "[NAT_ENABLE] extern ip ping error\n");
        return;
    }

    /**
     * Check the NAT forwarded count. NAT receive the ping request, but can
     * not forward this packet to local network.  discard it.
     */
#ifndef NX_DISABLE_NAT_INFO
    if ((nat_server.forwarded_packets_received != 3) ||
        (nat_server.forwarded_packets_sent != 2) ||
        (nat_server.forwarded_packets_dropped != 1)) {
        error_counter++;
        aicx_info(0x10bbd6, "", "[NAT_ENABLE] extern ip ping forward error\n");
        return;
    } else {
        aicx_info(0x022ee1, "", "[NAT_ENABLE] extern ip ping forward successfully\n");
    }
#endif

    /**********************************************/
    /*       Preload a inbound entry for ICMP     */
    /**********************************************/

    /* Calling NAT API to create a inbound entry. */
    status = nx_nat_inbound_entry_create(&nat_server,
                                         &server_inbound_entry_icmp,
                                         NX_NAT_LOCAL_HOST1, 0, 0,
                                         NX_PROTOCOL_ICMP);
    if (status != NX_SUCCESS) {
        error_counter++;
        aicx_info(0x03bb04, "", "[NAT_ENABLE] inbound entry create fail\n");
        return;
    }

    /**
     * External Host ping NAT External address, LOCAL HOST1 will response all
     * inbound icmp request from external network.
     */
    status = nx_icmp_ping(external_ip.ip_ptr, NX_NAT_EXTERNAL_IPADR, DEMO_DATA,
                          sizeof(DEMO_DATA), &my_packet, NX_IP_PERIODIC_RATE);

    if ((status != NX_SUCCESS) || (my_packet == NX_NULL) ||
        (my_packet->nx_packet_length != 28)) {
        error_counter++;
        aicx_info(0x1ce25b, "", "[NAT_ENABLE] inbound ping fail\n");
        return;
    }

    /* Check the NAT forwarded count. */
#ifndef NX_DISABLE_NAT_INFO
    if ((nat_server.forwarded_packets_received != 5) ||
        (nat_server.forwarded_packets_sent != 4) ||
        (nat_server.forwarded_packets_dropped != 1)) {
        error_counter++;
        aicx_info(0x17ce74, "", "[NAT_ENABLE] inbound ping forward fail\n");
        return;
    } else {
        aicx_info(0x1ad57f, "", "[NAT_ENABLE] inbound ping forward sucessfully\n");
    }
#endif
}

static int aic_netx_nat_test_application_define(void)
{
    int ret;

    /* Initialize the NetX system. */
    nx_system_initialize();

    /* Create the main thread. */
    ntest_0 = aos_create_thread("nat test thread", ntest_0_entry, NULL,
                                DEMO_STACK_SIZE, 0, NULL, 4,
                                AOS_AUTO_START);

    /* Create NAT packet pool. */
    nat_packet_pool.per_packet_size = NX_NAT_PACKET_SIZE;
    nat_packet_pool.packet_pool_size = NX_NAT_PACKET_POOL_SIZE;
    nat_packet_pool.packet_pool_name = "NAT Packet Pool";
    ret =  aic_create_packet_pool(&nat_packet_pool);
    /* Check status. */
    if (ret) {
        aicx_error(0x1acab3, "", "create NAT Packet Pool fail\n");
        return NX_NO_PACKET;
    }

    nat_ip.ip_name = "NAT IP Instance";
    nat_ip.default_ip_address = NX_NAT_EXTERNAL_IPADR;
    nat_ip.default_net_mask = NX_NAT_EXTERNAL_NETMASK;
    nat_ip.arp_cache_mem_size = NX_NAT_ARP_CACHE_SIZE;
    nat_ip.if_type = AIC_RAM_NETWORK_LINK;
    nat_ip.pri = NX_NAT_IP_THREAD_PRIORITY;
    nat_ip.task_stack_size = NX_NAT_IP_THREAD_STACK_SIZE;
    nat_ip.packet_pool = nat_packet_pool.packet_pool;
    ret = aic_create_ip_instance(&nat_ip);
    if (ret) {
        error_counter++;
        return NX_NOT_CREATED;
    }

    ret = aic_enable_arp(&nat_ip);
    if (ret) {
        error_counter++;
        return NX_NOT_CREATED;
    }

    /* Set the private interface(private network). */
    ret = nx_ip_interface_attach(nat_ip.ip_ptr, "Private Interface",
                                 NX_NAT_LOCAL_IPADR, NX_NAT_LOCAL_NETMASK,
                                 _nx_ram_network_driver);
    if (ret) {
        error_counter++;
        return NX_NOT_SUCCESSFUL;
    }

    local_ip.ip_name = "Local IP Instance";
    local_ip.default_ip_address = NX_NAT_LOCAL_HOST1;
    local_ip.default_net_mask = NX_NAT_LOCAL_NETMASK;
    local_ip.arp_cache_mem_size = NX_NAT_ARP_CACHE_SIZE;
    local_ip.if_type = AIC_RAM_NETWORK_LINK;
    local_ip.pri = NX_NAT_IP_THREAD_PRIORITY;
    local_ip.task_stack_size = NX_NAT_IP_THREAD_STACK_SIZE;
    local_ip.packet_pool = nat_packet_pool.packet_pool;
    ret = aic_create_ip_instance(&local_ip);
    if (ret) {
        error_counter++;
        return NX_NOT_CREATED;
    }

    ret = aic_enable_arp(&local_ip);
    if (ret) {
        error_counter++;
        return NX_NOT_CREATED;
    }

    external_ip.ip_name = "External IP Instance";
    external_ip.default_ip_address = NX_NAT_EXTERNAL_HOST;
    external_ip.default_net_mask = NX_NAT_EXTERNAL_NETMASK;
    external_ip.arp_cache_mem_size = NX_NAT_ARP_CACHE_SIZE;
    external_ip.if_type = AIC_RAM_NETWORK_LINK;
    external_ip.pri = NX_NAT_IP_THREAD_PRIORITY;
    external_ip.task_stack_size = NX_NAT_IP_THREAD_STACK_SIZE;
    external_ip.packet_pool = nat_packet_pool.packet_pool;

    ret = aic_create_ip_instance(&external_ip);
    if (ret) {
        error_counter++;
        return NX_NOT_CREATED;
    }

    ret = aic_enable_arp(&external_ip);
    if (ret) {
        error_counter++;
        return NX_NOT_CREATED;
    }

    /* Set the global network gateway for NAT IP instance. */
    ret = nx_ip_gateway_address_set(nat_ip.ip_ptr, NX_NAT_EXTERNAL_GATEWAY);
    if (ret) {
        error_counter++;
        return NX_NOT_SUCCESSFUL;
    }

    /* Set the global network gateway for Local IP instance. */
    ret = nx_ip_gateway_address_set(local_ip.ip_ptr, NX_NAT_LOCAL_GATEWAY);
    if (ret) {
        error_counter++;
        return NX_NOT_SUCCESSFUL;
    }

    /* Set the global network gateway for External IP instance. */
    ret = nx_ip_gateway_address_set(external_ip.ip_ptr, NX_NAT_EXTERNAL_GATEWAY);
    /* Check status. */
    if (ret)
    {
        error_counter++;
        return NX_NOT_SUCCESSFUL;
    }

    /* Enable ICMP. */
    nx_icmp_enable(nat_ip.ip_ptr);
    nx_icmp_enable(local_ip.ip_ptr);
    nx_icmp_enable(external_ip.ip_ptr);

    nat_ip.nat_cache_mem_size = NX_NAT_ENTRY_CACHE_SIZE;
    ret = aic_nat_create(&nat_server, &nat_ip, 0);
    /* Check status. */
    if (ret) {
        error_counter++;
        return NX_NOT_SUCCESSFUL;
    }

    return NX_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_netx_nat_test_application_define);

#endif

