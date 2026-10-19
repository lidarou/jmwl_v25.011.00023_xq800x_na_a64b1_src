
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c998f
#define LOG_MODULE    AMOD_NET
#include "lwip/sockets.h"
#include "lwip/tcpip.h"

#include <aic_ims_nic.h>
#include "aic_init.h"
#include "aic_osa.h"

#include <string.h>
#include <stdlib.h>
#include "aic_network_pcap.h"


#define DEMO_STACK_SIZE 2048
#define SERVER_PORT_1   87

#define CLIENT_PORT_1   77

#define UDP_SERVER_RCV_BUFFER_SIZE 1700

static aos_thread_id udp_thread_server;
static aos_thread_id udp_thread_client;
static struct netif udp_test_netif;

static ULONG udp_error_counter;

static CHAR udp_server_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static CHAR udp_send_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static CHAR udp_client_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];

static VOID udp_thread_server_entry(void *thread_input)
{
    INT status, addr_len;
    ip_addr_t ip_address;
    struct aic_network_ip_instance *aic_ip;
    UINT iface_index, address_index;
    INT sock_udp_server_1, i;
    struct sockaddr_in6 server_addr_1, client_addr_1;

    sock_udp_server_1 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_server_1 == -1) {
        aicx_info(0x04d281, "", "[NET] ipv udp Server 1 socket create fail\n");
        return;
    }

    aicx_info(0x14ddea, "u", "[NET] ipv6 udp Server socket 1 created %u\n",
              (ULONG)sock_udp_server_1);

    memset(&server_addr_1, 0, sizeof(server_addr_1));
    server_addr_1.sin6_addr.un.u32_addr[0] = htonl(0x20010db8u);
    server_addr_1.sin6_addr.un.u32_addr[1] = htonl(0xf101u);
    server_addr_1.sin6_addr.un.u32_addr[2] = htonl(0x0u);
    server_addr_1.sin6_addr.un.u32_addr[3] = htonl(0x0101u);
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
        if (status < 0) {
            if (errno == EAGAIN) {
                aicx_info(0x07d564, "", "[NET] ipv6 udp no error received, try again\n");
                continue;
            } else if (errno == ENOTCONN) {
                aicx_info(0x1a9529, "", "[NET] ipv6 udp connect is broken, closesocket socket.\n");
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
        aicx_info(0x171338, "du", "[NET] ipv6 udp server recv %d %u bytes\n",
                  sock_udp_server_1, (ULONG)status);

        memset(udp_send_buffer, 'b', UDP_SERVER_RCV_BUFFER_SIZE);
        status = sendto(sock_udp_server_1, udp_send_buffer,
                        strlen(udp_send_buffer), 0,
                        (struct sockaddr *)&client_addr_1,
                        sizeof(client_addr_1));
        if (status < 0)
            aicx_info(0x06f16a, "d", "[NET] ipv6 udp server sending to client fail s:%d\n",
                      sock_udp_server_1);
        else
            aicx_info(0x020fc7, "dd", "[NET] udp server sending to client:%d: bytes %d\n",
                      sock_udp_server_1, status);

        aos_sleep(1000);
    }

    status = closesocket(sock_udp_server_1);
    if (status >= 0)
        aicx_info(0x095fd8, "d", "[NET] ipv6 udp socket 1 closesocket conn to client on %d\n",
                  sock_udp_server_1);
    else
        aicx_info(0x1d0285, "d", "[NET] ipv6 udp socket 1 closesocket conn to client on %d fail\n",
                  sock_udp_server_1);
}

static VOID udp_thread_client_entry(void *thread_input)
{
    INT status, length;
    INT sock_udp_client_1;
    struct sockaddr_in6 echo_serv_addr6_1, local_addr6_1;

    aos_sleep(2000);

    memset(&local_addr6_1, 0, sizeof(local_addr6_1));
    local_addr6_1.sin6_addr.un.u32_addr[0] = htonl(0x20010db8u);
    local_addr6_1.sin6_addr.un.u32_addr[1] = htonl(0xf101u);
    local_addr6_1.sin6_addr.un.u32_addr[2] = htonl(0x0u);
    local_addr6_1.sin6_addr.un.u32_addr[3] = htonl(0x0101u);
    local_addr6_1.sin6_port = htons(CLIENT_PORT_1);
    local_addr6_1.sin6_family = AF_INET6;

    memset(&echo_serv_addr6_1, 0, sizeof(echo_serv_addr6_1));
    echo_serv_addr6_1.sin6_addr.un.u32_addr[0] = htonl(0x20010db8u);
    echo_serv_addr6_1.sin6_addr.un.u32_addr[1] = htonl(0xf101u);
    echo_serv_addr6_1.sin6_addr.un.u32_addr[2] = htonl(0x0u);
    echo_serv_addr6_1.sin6_addr.un.u32_addr[3] = htonl(0x0101u);
    echo_serv_addr6_1.sin6_port = htons(SERVER_PORT_1);
    echo_serv_addr6_1.sin6_family = AF_INET6;

    while (1) {
        sock_udp_client_1 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_client_1 == -1) {
            aicx_info(0x1b692e, "", "[NET] ipv6 udp client socket 1 create fail!\n");
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
        memset(udp_send_buffer, 'a', UDP_SERVER_RCV_BUFFER_SIZE);
        status = sendto(sock_udp_client_1, udp_send_buffer,
                        strlen(udp_send_buffer), 0,
                        (struct sockaddr *)&echo_serv_addr6_1,
                        sizeof(echo_serv_addr6_1));
        if (status < 0)
            aicx_info(0x10e44f, "d", "[NET] ipv6 udp client socket 1 %d send fail!\n",
                      sock_udp_client_1);
        else
            aicx_info(0x01f9a1, "dd", "[NET] udp client socket 1 %d send: bytes %d!\n",
                      sock_udp_client_1, status);

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
            aicx_info(0x15a225, "dd", "[NET] ipv6 udp client 1 %d recv %d bytes\n",
                      sock_udp_client_1, status);
        }

        status = closesocket(sock_udp_client_1);
        if (status >= 0)
            aicx_info(0x01210a, "d", "[NET] ipv6 udp client socket 1 %d closesocketd!\n",
                      sock_udp_client_1);
        else
            aicx_info(0x1d31d3, "d", "[NET] ipv6 udp client socket 1 %d closesocket fail!\n",
                      sock_udp_client_1);

        aos_sleep(1000);
    }
}

int aic_nic_ipv6_udp_test_init(void)
{
    ip4_addr_t test_local_ip = {0x0100A8C0};
    ip4_addr_t test_local_gw = {0x0000A8C0};
    ip4_addr_t test_netmask = {0x00FFFFFF};

    IP4_ADDR(&test_local_ip, 192,168,0,1);
    IP4_ADDR(&test_netmask, 255,255,255,0);
    IP4_ADDR(&test_local_gw, 192,168,0,254);

    aos_sleep(2000);
    netif_add(&udp_test_netif, &test_local_ip, &test_netmask, &test_local_gw,
              NULL, lwip_ims_init_netif, NULL);

#if LWIP_IPV6
    IP_ADDR6_HOST(udp_test_netif.ip6_addr, 0x20010db8, 0x0000f101, 0, 0x101);
    udp_test_netif.ip6_addr_state[0] = IP6_ADDR_VALID;
#endif /* LWIP_IPV6 */

    netif_set_default(&udp_test_netif);
    netif_set_up(&udp_test_netif);

    tcpip_init(NULL, NULL);
#if LWIP_NETCONN_SEM_PER_THREAD
    lwip_socket_thread_init();
#endif

    aic_network_parse_cp_at("cpcapon 1500");

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

    return 0;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv6_udp_test_init);

