
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x06e0c4
#define LOG_MODULE    AMOD_DRV
#include "aic_osa.h"
#include "aic_init.h"
#include "nx_api.h"
#include "nxd_bsd.h"
#include <string.h>
#include <stdlib.h>
#include <aic_net_api.h>

#include <nx_alte_driver.h>
#define IP_LINK_DRIVER nx_alte_nic_driver_entry

#define DEMO_STACK_SIZE 2048
#define SERVER_PORT_1   87

#define CLIENT_PORT_1   77

#define UDP_SERVER_RCV_BUFFER_SIZE 100

static aos_thread_id udp_thread_server;
static aos_thread_id udp_thread_client;

static ULONG udp_error_counter;

static CHAR udp_server_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static CHAR udp_send_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static CHAR udp_client_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];

static VOID udp_thread_server_entry(void *thread_input)
{
    INT status, addr_len;
    NXD_ADDRESS ip_address;
    struct aic_network_ip_instance *aic_ip;
    UINT iface_index, address_index;
    INT sock_udp_server_1, i;
    struct sockaddr_in6 server_addr_1, client_addr_1;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Designed for testing code */
    aic_ip = aic_get_pdp_ip_instance();
    if (!aic_ip->ip_ptr)
        aos_sleep(100);

    if (!aic_ip->bsd_task_stack)
        aic_init_bsd(aic_ip);

    status = nx_ip_interface_attach(aic_ip->ip_ptr, "IPv6 UDP Interface1",
                                    IP_ADDRESS(1, 2, 3, 7), 0xFFFFFF00UL,
                                    IP_LINK_DRIVER);
    if (status)
        aicx_info(0x09c306, "", "[NET] IPv6 UDP Error in attach IP Instance!\n");

    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        if (aic_ip->ip_ptr->nx_ip_interface[i].nx_interface_ip_address ==
            IP_ADDRESS(1, 2, 3, 7))
            break;
    }

    if (i >= NX_MAX_PHYSICAL_INTERFACES) {
        aicx_info(0x014731, "x", "[NET] find IP 0x%x interface fail\r\n", IP_ADDRESS(1, 2, 3, 7));
        return;
    }

    iface_index = i;
    status = (INT)nxd_ipv6_address_set(aic_ip->ip_ptr, iface_index, NX_NULL, 10,
                                       &address_index);
    if (status) {
        aicx_info(0x10ca0b, "x", "[NET] ipv6 udp ip 1 primary addr set fail 0x%x\n", status);
        return;
    }

    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 0x101;

    status = (INT)nxd_ipv6_address_set(aic_ip->ip_ptr, iface_index, &ip_address,
                                       64, &address_index);
    if (status) {
        aicx_info(0x19c7b7, "x", "[NET] ipv6 udp 1 host addr set fail 0x%x\n", status);
        return;
    }

    aos_sleep(3000);

    sock_udp_server_1 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_server_1 == -1) {
        aicx_info(0x1bc22d, "", "[NET] ipv6 udp BSD udp Server 1 socket create fail\n");
        return;
    }

    aicx_info(0x0b0d7c, "u", "[NET] ipv6 udp BSD udp Server socket 1 created %u\n",
              (ULONG)sock_udp_server_1);

    memset(&server_addr_1, 0, sizeof(server_addr_1));
    server_addr_1.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    server_addr_1.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    server_addr_1.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    server_addr_1.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    server_addr_1.sin6_port = htons(SERVER_PORT_1);
    server_addr_1.sin6_family = AF_INET6;

    status = bind(sock_udp_server_1, (struct sockaddr *)&server_addr_1,
                  sizeof(server_addr_1));
    if (status < 0) {
        aicx_info(0x109712, "", "[NET] ipv6 udp server socket 1 bind fail\n");
        return;
    }

    while (1) {
        addr_len = sizeof(struct sockaddr_in6);
        status = recvfrom(sock_udp_server_1, (VOID *)udp_server_recv_buffer,
                          UDP_SERVER_RCV_BUFFER_SIZE, 0,
                          (struct sockaddr *)&client_addr_1, &addr_len);
        if (status == ERROR) {
            if (errno == EAGAIN) {
                aicx_info(0x07d564, "", "[NET] ipv6 udp no error received, try again\n");
                continue;
            } else if (errno == ENOTCONN) {
                aicx_info(0x02aaef, "", "[NET] ipv6 udp connect is broken, close socket.\n");
                break;
            } else {
                aicx_info(0x116e4b, "x", "[NET] ipv6 udp err on client sock recv data 0x%x\n",
                          errno);
                break;
            }
        }

        if (status == UDP_SERVER_RCV_BUFFER_SIZE)
            status--;

        udp_server_recv_buffer[status] = 0;
        aicx_info(0x0d7c0e, "dus", "[NET] ipv6 udp server recv %d %u bytes: %s\n",
                  sock_udp_server_1, (ULONG)status, udp_server_recv_buffer);

        snprintf(udp_send_buffer, UDP_SERVER_RCV_BUFFER_SIZE,
                 "IPV6 UDP 1 hello world 0x%x\n", NX_RAND());
        status = sendto(sock_udp_server_1, udp_send_buffer,
                        strlen(udp_send_buffer), 0,
                        (struct sockaddr *)&client_addr_1,
                        sizeof(client_addr_1));
        if (status == ERROR)
            aicx_info(0x06f16a, "d", "[NET] ipv6 udp server sending to client fail s:%d\n",
                      sock_udp_server_1);
        else
            aicx_info(0x048b31, "ds", "[NET] udp server sending to client:%d: %s\n",
                      sock_udp_server_1, udp_send_buffer);

        aos_sleep(1000);
    }

    status = soc_close(sock_udp_server_1);
    if (status != ERROR)
        aicx_info(0x071e41, "d", "[NET] ipv6 udp socket 1 close conn to client on %d\n",
                  sock_udp_server_1);
    else
        aicx_info(0x156067, "d", "[NET] ipv6 udp socket 1 close conn to client on %d fail\n",
                  sock_udp_server_1);
}

static VOID udp_thread_client_entry(void *thread_input)
{
    INT status, length;
    INT sock_udp_client_1;
    struct sockaddr_in6 echo_serv_addr6_1, local_addr6_1;

    NX_PARAMETER_NOT_USED(thread_input);

    aos_sleep(4000);

    memset(&local_addr6_1, 0, sizeof(local_addr6_1));
    local_addr6_1.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    local_addr6_1.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    local_addr6_1.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    local_addr6_1.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    local_addr6_1.sin6_port = htons(CLIENT_PORT_1);
    local_addr6_1.sin6_family = AF_INET6;

    memset(&echo_serv_addr6_1, 0, sizeof(echo_serv_addr6_1));
    echo_serv_addr6_1.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    echo_serv_addr6_1.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    echo_serv_addr6_1.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    echo_serv_addr6_1.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    echo_serv_addr6_1.sin6_port = htons(SERVER_PORT_1);
    echo_serv_addr6_1.sin6_family = AF_INET6;

    while (1) {
        sock_udp_client_1 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_client_1 == -1) {
            aicx_info(0x0e2b47, "", "[NET] ipv6 udp BSD udp client socket 1 create fail!\n");
            return;
        }
        status = bind(sock_udp_client_1, (struct sockaddr *)&local_addr6_1,
                      sizeof(local_addr6_1));
        if (status < 0) {
            aicx_info(0x166cf5, "", "[NET] ipv6 udp client socket 1 bind fail\n");
            return;
        }

        aicx_info(0x16d851, "u", "[NET] ipv6 udp client socket 1 created ok %u\n",
                  (ULONG)sock_udp_client_1);

        length = sizeof(struct sockaddr_in6);
        snprintf(udp_send_buffer, UDP_SERVER_RCV_BUFFER_SIZE,
                 "IPV6 UDP 1 hello world 0x%x\n", NX_RAND());

        status = sendto(sock_udp_client_1, udp_send_buffer,
                        strlen(udp_send_buffer), 0,
                        (struct sockaddr *)&echo_serv_addr6_1,
                        sizeof(echo_serv_addr6_1));
        if (status == ERROR)
            aicx_info(0x10e44f, "d", "[NET] ipv6 udp client socket 1 %d send fail!\n",
                      sock_udp_client_1);
        else
            aicx_info(0x00c91a, "ds", "[NET] udp client socket 1 %d send: %s!\n",
                      sock_udp_client_1, udp_send_buffer);
READ_AGAIN_1:
        status = recvfrom(sock_udp_client_1, (VOID *)udp_client_recv_buffer,
                          100, 0, (struct sockaddr *)&echo_serv_addr6_1,
                          &length);
        if (status < 0) {
            if (errno == EAGAIN)
                goto READ_AGAIN_1;
            aicx_info(0x0511ad, "dxx", "[NET] ipv6 udp connect 1 err recv socket %d 0x%x 0x%x\n",
                      sock_udp_client_1, status, errno);
        } else {
            aicx_info(0x084883, "dds", "[NET] ipv6 udp client 1 %d recv %d bytes: %s\n",
                      sock_udp_client_1, status, udp_client_recv_buffer);
        }

        status = soc_close(sock_udp_client_1);
        if (status != ERROR)
            aicx_info(0x138f13, "d", "[NET] ipv6 udp client socket 1 %d closed!\n",
                      sock_udp_client_1);
        else
            aicx_info(0x18fffc, "d", "[NET] ipv6 udp client socket 1 %d close fail!\n",
                      sock_udp_client_1);

        aos_sleep(1000);
    }
}

int aic_nic_ipv6_udp_test_init(void)
{
    udp_thread_server = aos_create_thread("nic ipv6 udp test server",
                                          udp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 20,
                                          AOS_AUTO_START);
    if (udp_thread_server == AOS_INVALID_THREAD_ID)
    {
        udp_error_counter++;
        aicx_info(0x11fc25, "", "[NET] Error in create nic ipv6 udp test server\n");
    }

    udp_thread_client = aos_create_thread("nic ipv6 udp test client",
                                          udp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 30,
                                          AOS_AUTO_START);
    if (udp_thread_client == AOS_INVALID_THREAD_ID)
    {
        udp_error_counter++;
        aicx_info(0x07ec1d, "", "[NET] Error in create nic ipv6 udp test client\n");
    }

    return NX_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv6_udp_test_init);

