/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/* Usage: Open CONFIG_NET_SOCKET_IPV4_UDP_TEST */
#include <aic_errno.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <stdio.h>
#include <aic_osa_ex.h>
#include <string.h>
#include <stdlib.h>
#include <aic_net_api.h>

#define NIC_DEMO_STACK_SIZE   2048
#define NIC_SERVER_PORT_1     720

#define SERVER_TASK_PRIORITY    20
#define CLIENT_TASK_PRIORITY    30

#define CLIENT_MESSAGE_0 "IPv4 UDP nic Socket testing 0\n"

static aos_thread_id thread_server;
static aos_thread_id thread_client;

static void thread_server_entry(void *thread_input)
{
    u8 tos = 32, ttl = 32;
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

    /* End */

    /* Create a BSD UDP Server Socket */
    sock_udp_1 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    /* Check for any errors */
    if (sock_udp_1 == -1) {
        app_logi("[NET] Error: BSD IPv4 UDP Server socket 1 create\n");
        return;
    }

    app_logi("[NET] IPv4 UDP server sock_udp 1 %d\n", sock_udp_1);

    memset(&echoServAddr1, 0, sizeof(echoServAddr1));
    echoServAddr1.sin_family = PF_INET;
    echoServAddr1.sin_addr.s_addr = htonl(IP_ADDRESS(127, 0, 0, 1));
    echoServAddr1.sin_port = htons(NIC_SERVER_PORT_1);

    status = setsockopt(sock_udp_1, IPPROTO_IP, IP_TOS, &tos, 1);
    if (status)
        app_logi("[NET] ipv4 udp server set IP TOS fail\n");
    status = setsockopt(sock_udp_1, IPPROTO_IP, IP_TTL, &ttl, 1);
    if (status)
        app_logi("[NET] ipv4 udp server set IP TTL fail\n");

    /* Bind the UDP socket to the IP port. */
    status = bind(sock_udp_1, (struct sockaddr *)&echoServAddr1,
                  sizeof(echoServAddr1));
    if (status < 0) {
        app_logi("[NET] Error: BSP IPv4 UDP 1 Server Socket Bind %d\n",
                  status);
        return;
    }

    while (1) {
        addrlen1 = sizeof(struct sockaddr_in);

        /* Receive a UDP packet from a client. */
        status = recvfrom(sock_udp_1, (void *)buffer1, 64, 0,
                          (struct sockaddr *)&fromAddr1, &addrlen1);
        /* Check for any errors */
        if (status == ERROR) {
            if (errno == EAGAIN) {
                app_logi("[NET] IPv4 udp socket 1 no err, try again\n");
                continue;
            }

            app_logi("[NET] Err: IPv4 UDP BSD Server Socket 1 receive 0x%x\n",
                      errno);
        } else {
            app_logi("[NET] Server recv from 1 Client: %s\n", buffer1);
            /* Now echo the recieved data string to the same client */
            status = sendto(sock_udp_1, buffer1, (INT)(status + 1), 0,
                            (struct sockaddr *)&fromAddr1, sizeof(fromAddr1));
            if (status == ERROR)
                app_logi("[NET] Error: IPv4 UDP BSD Server Socket 1 echo\n");
        }

        aos_sleep(1000);
    }
}

static void thread_client_entry(void *thread_input)
{
    u8 ttl = 32, tos = 32;
    int status, addr_len;
    int sock_udp_1;
    struct sockaddr_in dest_addr_1;
    char echo_msg_1[64];

    NX_PARAMETER_NOT_USED(thread_input);
    aos_sleep(200);

    while (1) {
        /* Create a BSD UDP Client Socket */
        sock_udp_1 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_1 == ERROR) {
            app_logi("[NET] Error: BSD IPv4 UDP 1 Client socket create\n");
            return;
        }

        status = setsockopt(sock_udp_1, IPPROTO_IP, IP_TOS, &tos, 1);
        if (status)
            app_logi("[NET] ipv4 udp client set IP TOS fail\n");
        status = setsockopt(sock_udp_1, IPPROTO_IP, IP_TTL, &ttl, 1);
        if (status)
            app_logi("[NET] ipv4 udp client set IP TTL fail\n");

        app_logi("[NET] IPv4 UDP client sock_udp 1 %d\n", sock_udp_1);
        memset(&dest_addr_1, 0, sizeof(dest_addr_1));
        dest_addr_1.sin_addr.s_addr = htonl(IP_ADDRESS(127, 0, 0, 1));
        dest_addr_1.sin_port = htons(NIC_SERVER_PORT_1);
        dest_addr_1.sin_family = PF_INET;

        status = sendto(sock_udp_1, CLIENT_MESSAGE_0,
                        (int)(sizeof(CLIENT_MESSAGE_0)), 0,
                        (struct sockaddr *)&dest_addr_1, sizeof(dest_addr_1));
        if (status == ERROR) {
            app_logi("[NET] Error: IPv4 UDP BSD client socket 1 send\n");
        } else {
            addr_len = sizeof(struct sockaddr_in);
            /* Try to receive an echo from the server. */
READ_AGAIN_1:
            status = recvfrom(sock_udp_1, (void *)echo_msg_1, 64, 0,
                              (struct sockaddr *)&dest_addr_1, &addr_len);

            if (status == ERROR) {
                if (errno == EAGAIN)
                    goto READ_AGAIN_1;
                app_logi("[NET] Error: IPv4 UDP BSD client socket 1 recv\n");
            } else {
                app_logi("[NET] IPv4 UDP 1 CLient %u recv: %s\n",
                          (UINT)sock_udp_1, echo_msg_1);
            }
        }

        /* Done with this client socket */
        status = soc_close(sock_udp_1);
        if (status == ERROR)
            app_logi("[NET] Error:IPv4 UDP BSD client socket close 1\n");

        aos_sleep(1000);
    }
}

int aic_nic_ipv4_udp_test_init(void)
{
    thread_server = aos_create_thread("nic_ipv4_udp_test_server",
                                      thread_server_entry,
                                      NULL, NIC_DEMO_STACK_SIZE, 0, NULL,
                                      SERVER_TASK_PRIORITY,
                                      AOS_AUTO_START);
    if (thread_client == AOS_INVALID_THREAD_ID)
    {
        app_logi("[NET] IPv4 UDP Error in NIC_test_server\n");
    }

    thread_client = aos_create_thread("nic_ipv4_udp_test_client",
                                      thread_client_entry,
                                      NULL, NIC_DEMO_STACK_SIZE, 0, NULL,
                                      CLIENT_TASK_PRIORITY,
                                      AOS_AUTO_START);
    if (thread_client == AOS_INVALID_THREAD_ID)
    {
        app_logi("[NET] IPv4 UDP Error in NIC_test_client\n");
    }

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv4_udp_test_init);

