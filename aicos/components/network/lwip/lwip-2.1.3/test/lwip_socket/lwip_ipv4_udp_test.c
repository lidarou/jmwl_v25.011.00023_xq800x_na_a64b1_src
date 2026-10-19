/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b9a04
#define LOG_MODULE    AMOD_NET

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <stdio.h>

#include "aic_osa.h"

#include <string.h>
#include <stdlib.h>

#include "lwip/ip.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/stats.h"
#include "lwip/pbuf.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip_addr.h"

#include <aic_ims_nic.h>

#define NIC_DEMO_STACK_SIZE   2048
#define NIC_SERVER_PORT_1     720

#define CLIENT_MESSAGE_0 "IPv4 UDP nic Socket testing 0\n"
static struct netif udp_test_netif;

static aos_thread_id thread_server;
static aos_thread_id thread_client;
const static ip4_addr_t test_local_ip = {0xC0A80001};
const static ip4_addr_t test_remote_ip = {0xC0A80001};

static s8 buffer1[2048];
static s8 echo_msg_1[2048];

static void thread_server_entry(void *thread_input)
{
    int sock_udp_1;
    int status, addrlen1;
    struct sockaddr_in fromAddr1, echoServAddr1;

    ip4_addr_t test_local_ip = {0x0100A8C0};
    ip4_addr_t test_local_gw = {0x0000A8C0};
    ip4_addr_t test_netmask = {0x00FFFFFF};

    IP4_ADDR(&test_local_ip, 192,168,0,1);
    IP4_ADDR(&test_netmask, 255,255,255,0);
    IP4_ADDR(&test_local_gw, 192,168,0,254);

    aos_sleep(2000);
    netif_add(&udp_test_netif, &test_local_ip, &test_netmask, &test_local_gw,
              NULL, lwip_ims_init_netif, NULL);

    netif_set_default(&udp_test_netif);
    netif_set_up(&udp_test_netif);

    aicx_info(0x09f85b, "xxx", "ip 0x%x gw 0x%x mask 0x%x\r\n",udp_test_netif.ip_addr.u_addr.ip4.addr,
              udp_test_netif.gw.u_addr.ip4.addr, udp_test_netif.netmask.u_addr.ip4.addr);

    sock_udp_1 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_1 == -1) {
        aicx_info(0x19aa09, "", "[NET] Error: BSD IPv4 UDP Server socket 1 create\n");
        return;
    }

    aicx_info(0x1e4096, "d", "[NET] IPv4 UDP server sock_udp 1 %d\n", sock_udp_1);
    memset(&echoServAddr1, 0, sizeof(echoServAddr1));
    echoServAddr1.sin_family = PF_INET;
    echoServAddr1.sin_addr.s_addr = htonl(test_remote_ip.addr);
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

        status = recvfrom(sock_udp_1, (void *)buffer1, sizeof(buffer1), 0,
                          (struct sockaddr *)&fromAddr1, (socklen_t *)&addrlen1);
        if (status == -1) {
            if (errno == EAGAIN) {
                aicx_info(0x139a62, "", "[NET] IPv4 udp socket 1 no err, try again\n");
                continue;
            }
            aicx_info(0x08a552, "x", "[NET] Err: IPv4 UDP BSD Server Socket 1 receive 0x%x\n",
                      errno);
        } else {
            aicx_info(0x16ae7c, "d", "[NET] Server recv from 1 Client: %d\n", status);

            status = sendto(sock_udp_1, buffer1, sizeof(buffer1), 0,
                            (struct sockaddr *)&fromAddr1, sizeof(fromAddr1));
            if (status == -1)
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

    aos_sleep(3000);

    while (1) {
        sock_udp_1 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_1 == -1) {
            aicx_info(0x1a8a66, "", "[NET] Error: BSD IPv4 UDP 1 Client socket create\n");
            return;
        }
        aicx_info(0x18813a, "d", "[NET] IPv4 UDP client sock_udp 1 %d\n", sock_udp_1);

        memset(&client_addr_1, 0, sizeof(client_addr_1));
        client_addr_1.sin_family = PF_INET;
        client_addr_1.sin_port = htons(40000);
        client_addr_1.sin_addr.s_addr = htonl(test_local_ip.addr);

        status = bind(sock_udp_1, (struct sockaddr *)&client_addr_1,
                      sizeof(client_addr_1));

        memset(&dest_addr_1, 0, sizeof(dest_addr_1));
        dest_addr_1.sin_addr.s_addr = htonl(test_remote_ip.addr);
        dest_addr_1.sin_port = htons(NIC_SERVER_PORT_1);
        dest_addr_1.sin_family = PF_INET;

        status = sendto(sock_udp_1, echo_msg_1, (int)(sizeof(echo_msg_1)), 0,
                        (struct sockaddr *)&dest_addr_1, sizeof(dest_addr_1));
        if (status == -1) {
            aicx_info(0x19663f, "", "[NET] Error: IPv4 UDP BSD client socket 1 send\n");
        } else {
            addr_len = sizeof(struct sockaddr_in);
            status = recvfrom(sock_udp_1, (void *)echo_msg_1,
                              (int)(sizeof(echo_msg_1)), 0,
                              (struct sockaddr *)&from_addr_1,
                              (socklen_t *)&addr_len);

            if (status == -1) {
                aicx_info(0x048fd6, "", "[NET] Error: IPv4 UDP BSD client socket 1 recv\n");
            } else {
                aicx_info(0x09b794, "ud", "[NET] IPv4 UDP 1 CLient %u recv: %d\n",
                          (UINT)sock_udp_1, status);
            }
        }

        status = closesocket(sock_udp_1);
        if (status == -1)
            aicx_info(0x055d15, "", "[NET] Error:IPv4 UDP BSD client socket closesocket 1\n");

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
        aicx_info(0x10b493, "", "[NET] IPv4 UDP Error in NIC_test_server\n");

    thread_client = aos_create_thread("nic_ipv4_udp_test_client",
                                      thread_client_entry,
                                      NULL, NIC_DEMO_STACK_SIZE, 0, NULL, 30,
                                      AOS_AUTO_START);
    if (thread_client == AOS_INVALID_THREAD_ID)
        aicx_info(0x1528f3, "", "[NET] IPv4 UDP Error in NIC_test_client\n");

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv4_udp_test_init);

