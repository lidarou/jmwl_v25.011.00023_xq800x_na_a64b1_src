/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1e3985
#define LOG_MODULE    AMOD_DRV

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <stdio.h>

#include "aic_osa.h"
#include "nx_api.h"
#include "nxd_bsd.h"
#include "nx_port.h"

#include <string.h>
#include <stdlib.h>

#include "nx_alte_driver.h"
#include <aic_net_api.h>
#define IP_LINK_DRIVER nx_alte_nic_driver_entry

#define NIC_DEMO_STACK_SIZE   2048
#define NIC_SERVER_PORT_1     720

#define CLIENT_MESSAGE_0 "IPv4 UDP nic Socket testing 0\n"

static aos_thread_id thread_server;
static aos_thread_id thread_client;

static void thread_server_entry(void *thread_input)
{
    int sock_udp_1;
    s8 buffer1[64];
    int status, addrlen1;
    struct aic_network_ip_instance *aic_ip;
    struct sockaddr_in fromAddr1, echoServAddr1;

    NX_PARAMETER_NOT_USED(thread_input);
    /* Designed for testing code */
    aic_ip = aic_get_pdp_ip_instance();
    if (!aic_ip->ip_ptr)
        aos_sleep(100);

    if (!aic_ip->bsd_task_stack)
        aic_init_bsd(aic_ip);

    status = nx_ip_interface_attach(aic_ip->ip_ptr, "IPv4 UDP Interface1",
                                    IP_ADDRESS(1, 2, 3, 5), 0xFFFFFF00UL,
                                    IP_LINK_DRIVER);
    if (status)
        aicx_info(0x1118fb, "", "[NET] IPv4 UDP Error in attach IP Instance!\n");
    /* End */

    sock_udp_1 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_1 == -1) {
        aicx_info(0x19aa09, "", "[NET] Error: BSD IPv4 UDP Server socket 1 create\n");
        return;
    }

    aicx_info(0x1e4096, "d", "[NET] IPv4 UDP server sock_udp 1 %d\n", sock_udp_1);

    memset(&echoServAddr1, 0, sizeof(echoServAddr1));
    echoServAddr1.sin_family = PF_INET;
    echoServAddr1.sin_addr.s_addr = htonl(IP_ADDRESS(1,2,3,5));
    echoServAddr1.sin_port = htons(NIC_SERVER_PORT_1);

    status = bind(sock_udp_1, (struct sockaddr *)&echoServAddr1,
                  sizeof(echoServAddr1));
    if (status < 0) {
        aicx_info(0x0ab0e4, "d", "[NET] Error: BSP IPv4 UDP 1 Server Socket Bind %d\n",
                  status);
        return;
    }

    while (1) {
        addrlen1 = sizeof(struct sockaddr_in);

        status = recvfrom(sock_udp_1, (void *)buffer1, 64, 0,
                          (struct sockaddr *)&fromAddr1, &addrlen1);
        if (status == ERROR) {
            if (errno == EAGAIN) {
                aicx_info(0x139a62, "", "[NET] IPv4 udp socket 1 no err, try again\n");
                continue;
            }

            aicx_info(0x08a552, "x", "[NET] Err: IPv4 UDP BSD Server Socket 1 receive 0x%x\n",
                      tx_thread_identify()->bsd_errno);
        } else {
            aicx_info(0x1284c6, "s", "[NET] Server recv from 1 Client: %s\n", buffer1);

            status = sendto(sock_udp_1, buffer1, (INT)(status + 1), 0,
                            (struct sockaddr *)&fromAddr1, sizeof(fromAddr1));
            if (status == ERROR)
                aicx_info(0x198a9f, "", "[NET] Error: IPv4 UDP BSD Server Socket 1 echo\n");
        }

        aos_sleep(1000);
    }
}

static void thread_client_entry(void *thread_input)
{
    int status, addr_len;
    int sock_udp_1;
    struct sockaddr_in dest_addr_1, client_addr_1, from_addr_1;
    char echo_msg_1[64];

    NX_PARAMETER_NOT_USED(thread_input);
    aos_sleep(200);

    while (1) {
        sock_udp_1 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_1 == ERROR) {
            aicx_info(0x1a8a66, "", "[NET] Error: BSD IPv4 UDP 1 Client socket create\n");
            return;
        }

        aicx_info(0x18813a, "d", "[NET] IPv4 UDP client sock_udp 1 %d\n", sock_udp_1);

        memset(&client_addr_1, 0, sizeof(client_addr_1));
        client_addr_1.sin_family = PF_INET;
        client_addr_1.sin_addr.s_addr = htonl(IP_ADDRESS(1, 2, 3, 5));

        memset(&dest_addr_1, 0, sizeof(dest_addr_1));
        dest_addr_1.sin_addr.s_addr = htonl(IP_ADDRESS(1, 2, 3, 5));
        dest_addr_1.sin_port = htons(NIC_SERVER_PORT_1);
        dest_addr_1.sin_family = PF_INET;

        status = sendto(sock_udp_1, CLIENT_MESSAGE_0,
                        (int)(sizeof(CLIENT_MESSAGE_0)), 0,
                        (struct sockaddr *)&dest_addr_1, sizeof(dest_addr_1));
        if (status == ERROR) {
            aicx_info(0x19663f, "", "[NET] Error: IPv4 UDP BSD client socket 1 send\n");
        } else {
            addr_len = sizeof(struct sockaddr_in);
            status = recvfrom(sock_udp_1, (void *)echo_msg_1, 64, 0,
                              (struct sockaddr *)&from_addr_1, &addr_len);

            if (status == ERROR) {
                aicx_info(0x048fd6, "", "[NET] Error: IPv4 UDP BSD client socket 1 recv\n");
            } else {
                aicx_info(0x0ffcbe, "us", "[NET] IPv4 UDP 1 CLient %u recv: %s\n",
                          (UINT)sock_udp_1, echo_msg_1);
            }
        }

        status = soc_close(sock_udp_1);
        if (status == ERROR)
            aicx_info(0x189316, "", "[NET] Error:IPv4 UDP BSD client socket close 1\n");

        aos_sleep(1000);
    }
}

int aic_nic_ipv4_udp_test_init(void)
{
    thread_server = aos_create_thread("nic_ipv4_udp_test_server",
                                      thread_server_entry,
                                      NULL, NIC_DEMO_STACK_SIZE, 0, NULL, 20,
                                      AOS_AUTO_START);
    if (thread_client == AOS_INVALID_THREAD_ID)
    {
        aicx_info(0x10b493, "", "[NET] IPv4 UDP Error in NIC_test_server\n");
    }

    thread_client = aos_create_thread("nic_ipv4_udp_test_client",
                                      thread_client_entry,
                                      NULL, NIC_DEMO_STACK_SIZE, 0, NULL, 30,
                                      AOS_AUTO_START);
    if (thread_client == AOS_INVALID_THREAD_ID)
    {
        aicx_info(0x1528f3, "", "[NET] IPv4 UDP Error in NIC_test_client\n");
    }

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv4_udp_test_init);

