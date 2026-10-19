
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09a268
#define LOG_MODULE    AMOD_NET
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
#define SERVER_PORT 86
#define CLIENT_PORT 76
#define TCP_SERVER_RCV_BUFFER_SIZE 100

static aos_thread_id tcp_thread_server;
static aos_thread_id tcp_thread_client;

static INT max_fd;

static fd_set master_list, read_ready;

static CHAR tcp_server_recv_buffer[TCP_SERVER_RCV_BUFFER_SIZE];
static CHAR tcp_client_recv_buffer[TCP_SERVER_RCV_BUFFER_SIZE];

static VOID tcp_thread_server_entry(void *thread_input)
{
    NXD_ADDRESS ip_address;
    UINT iface_index, address_index;
    struct aic_network_ip_instance *aic_ip;
    struct sockaddr_in6 server_addr, client_addr;
    INT status, sock, sock_tcp_server, client_len, i, is_set;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Designed for testing code */
    aic_ip = aic_get_pdp_ip_instance();
    if (!aic_ip->ip_ptr)
        aos_sleep(100);

    if (!aic_ip->bsd_task_stack)
        aic_init_bsd(aic_ip);

    status = nx_ip_interface_attach(aic_ip->ip_ptr, "IPv4 UDP Interface1",
                                    IP_ADDRESS(1, 2, 3, 6), 0xFFFFFF00UL,
                                    IP_LINK_DRIVER);
    if (status)
        aicx_info(0x1118fb, "", "[NET] IPv4 UDP Error in attach IP Instance!\n");

    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        if (aic_ip->ip_ptr->nx_ip_interface[i].nx_interface_ip_address ==
            IP_ADDRESS(1, 2, 3, 6))
            break;
    }

    if (i >= NX_MAX_PHYSICAL_INTERFACES) {
        aicx_info(0x014731, "x", "[NET] find IP 0x%x interface fail\r\n", IP_ADDRESS(1, 2, 3, 6));
        return;
    }

    iface_index = i;
    status = (INT)nxd_ipv6_address_set(aic_ip->ip_ptr, iface_index, NX_NULL,
                                       10, &address_index);
    if (status) {
        aicx_info(0x0ecd91, "x", "[NET] ipv6 tcp primary addr set fail 0x%x\n", status);
        return;
    }

    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 0x102;

    status = (INT)nxd_ipv6_address_set(aic_ip->ip_ptr, iface_index, &ip_address,
                                       64, &address_index);
    if (status) {
        aicx_info(0x05aaba, "x", "[NET] ipv6 tcp host addr set fail 0x%x\n", status);
        return;
    }
    /* End */

    aos_sleep(3000);

    sock_tcp_server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sock_tcp_server == -1) {
        aicx_info(0x176eab, "", "[NET] BSD ipv6 tcp Server socket create fail\n");
        return;
    }

    aicx_info(0x140ffa, "u", "[NET] BSD ipv6 tcp Server socket created %u\n",
              (ULONG)sock_tcp_server);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    server_addr.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    server_addr.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    server_addr.sin6_addr._S6_un._S6_u32[3] = htonl(0x0102u);
    server_addr.sin6_port = htons(SERVER_PORT);
    server_addr.sin6_family = AF_INET6;

    status = bind(sock_tcp_server, (struct sockaddr *)&server_addr,
                  sizeof(server_addr));
    if (status < 0) {
        aicx_info(0x01e3b2, "", "[NET] ipv6 tcp server socket bind fail\n");
        return;
    }

    FD_ZERO(&master_list);
    FD_ZERO(&read_ready);
    FD_SET(sock_tcp_server, &master_list);
    max_fd = sock_tcp_server;

    status = listen(sock_tcp_server, 5);
    if (status < 0) {
        aicx_info(0x0ad21f, "", "[NET] ipv6 tcp server socket listen fail\n");
        return;
    } else {
        aicx_info(0x1dff09, "", "[NET] ipv6 tcp server listen complete\n");
    }

    aicx_info(0x110f0a, "d", "[NET] ipv6 tcp Now accepting client connections %d\n", max_fd);

    while (1) {
        read_ready = master_list;
        aos_sleep(20);
        status = select(max_fd + 1, &read_ready, 0, 0, 0);
        if (status == ERROR || status == 0) {
            aicx_info(0x1368bf, "x", "[NET] ipv6 tcp select err 0x%x try again\n", status);
            continue;
        }

        is_set = FD_ISSET(sock_tcp_server, &read_ready);
        if (is_set) {
            client_len = sizeof(client_addr);
            sock = accept(sock_tcp_server, (struct sockaddr *)&client_addr,
                          &client_len);
            FD_SET(sock, &master_list);

            if (sock > max_fd) {
                aicx_info(0x097891, "d", "[NET] ipv6 tcp new connection %d\n", sock);
                max_fd = sock;
            }

            continue;
        }

        for (i = 0; i < (max_fd + 1); i++) {
            if (((i + NX_BSD_SOCKFD_START) != sock_tcp_server) &&
                (FD_ISSET(i + NX_BSD_SOCKFD_START, &master_list)) &&
                (FD_ISSET(i + NX_BSD_SOCKFD_START, &read_ready))) {
                while (1) {
                    status = recv(i + NX_BSD_SOCKFD_START,
                                  (VOID *)tcp_server_recv_buffer,
                                  TCP_SERVER_RCV_BUFFER_SIZE, 0);
                    if (status == ERROR) {
                        if (errno == EAGAIN) {
                            aicx_info(0x17f774, "", "[NET] ipv6 tcp no err recv, again\n");
                            continue;
                        } else if (errno == ENOTCONN) {
                            aicx_info(0x0374eb, "", "[NET] ipv6 tcp connection is broken.\n");
                            break;
                        } else {
                            aicx_info(0x0ed1b9, "dx", "[NET] ipv6 tcp client err %d 0x%x\n",
                                       i, errno);
                            break;
                        }
                    }

                    if (status == TCP_SERVER_RCV_BUFFER_SIZE)
                        status--;

                    tcp_server_recv_buffer[status] = 0;
                    aicx_info(0x0ba480, "dus", "[NET] ipv6 tcp server recv %d %u bytes: %s\n",
                              i + NX_BSD_SOCKFD_START, (ULONG)status,
                              tcp_server_recv_buffer);

                    status = send(i + NX_BSD_SOCKFD_START, "ipv6 tcp Hello\n",
                                  sizeof("ipv6 tcp Hello\n"), 0);
                    if (status == ERROR)
                        aicx_info(0x1d3580, "d", "[NET] ipv6 tcp server send fail :%d\n",
                                  i + NX_BSD_SOCKFD_START);
                        aos_sleep(1000);
                }

                status = closesocket(i + NX_BSD_SOCKFD_START);
                if (status != ERROR)
                    aicx_info(0x09a36a, "d", "[NET] ipv6 tcp socket close client on %d\n",
                              i + NX_BSD_SOCKFD_START);
                else
                    aicx_info(0x1b44bc, "d", "[NET] ipv6 tcp socket close client on %d fail\n",
                              i + NX_BSD_SOCKFD_START);
            }
        }
    }
}

static VOID tcp_thread_client_entry(void *thread_input)
{
    INT status, sock_tcp_client, length;
    struct sockaddr_in6 echo_serv_addr6, local_addr6, remote_addr6;

    NX_PARAMETER_NOT_USED(thread_input);

    aos_sleep(4000);

    memset(&local_addr6, 0, sizeof(local_addr6));
    local_addr6.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    local_addr6.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    local_addr6.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    local_addr6.sin6_addr._S6_un._S6_u32[3] = htonl(0x0102u);
    local_addr6.sin6_port = htons(CLIENT_PORT);
    local_addr6.sin6_family = AF_INET6;

    memset(&echo_serv_addr6, 0, sizeof(echo_serv_addr6));
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[3] = htonl(0x0102u);
    echo_serv_addr6.sin6_port = htons(SERVER_PORT);
    echo_serv_addr6.sin6_family = AF_INET6;

    while (1) {
        sock_tcp_client = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (sock_tcp_client == -1) {
            aicx_info(0x08d25d, "", "[NET] BSD ipv6 tcp client socket create fail!\n");
            return;
        }

        aicx_info(0x15ca69, "u", "[NET] ipv6 tcp client socket created ok %u\n",
                  (ULONG)sock_tcp_client);
        status = connect(sock_tcp_client, (struct sockaddr *)&echo_serv_addr6,
                         sizeof(echo_serv_addr6));
        if (status != OK) {
            aicx_info(0x064540, "x", "[NET] BSD ipv6 tcp client socket connect fail 0x%x!\n",
                      status);
            status = closesocket(sock_tcp_client);
            return;
        }

        aicx_info(0x1213c1, "d", "[NET] ipv6 tcp client socket %d connected\n",
                  sock_tcp_client);

        length = sizeof(struct sockaddr_in6);
        status = getsockname(sock_tcp_client, (struct sockaddr *)&local_addr6,
                             &length);
        aicx_info(0x124539, "uxxxx", "[NET] ipv6 tcp client port %u client 0x%x 0x%x 0x%x 0x%x\n",
                  (ULONG)local_addr6.sin6_port,
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[0],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[1],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[2],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[3]);
        length = sizeof(struct sockaddr_in6);
        status = getpeername(sock_tcp_client, (struct sockaddr *)&remote_addr6,
                             &length);
        aicx_info(0x1033dd, "uxxxx", "[NET] ipv6 tcp remote port %u remote 0x%x 0x%x 0x%x 0x%x\n",
                  (ULONG)remote_addr6.sin6_port,
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[0],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[1],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[2],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[3]);

        while (1) {
            status = send(sock_tcp_client, "ipv6 tcp Hello",
                          (sizeof("ipv6 tcp Hello")), 0);
            if (status == ERROR)
                aicx_info(0x0ada8f, "d", "[NET] ipv6 tcp client socket %d send fail!\n",
                          sock_tcp_client);
            else
                aicx_info(0x0b410d, "d", "[NET] ipv6 tcp client socket %d send hello!\n",
                          sock_tcp_client);
READ_AGAIN:
            status = recv(sock_tcp_client, (VOID *)tcp_client_recv_buffer,
                          100, 0);
            if (status < 0) {
                if (errno == EAGAIN)
                    goto READ_AGAIN;
                aicx_info(0x01464b, "dxx", "[NET] ipv6 tcp connect error socket %d 0x%x 0x%x\n",
                          sock_tcp_client, status, errno);
                break;
            } else {
                aicx_info(0x1597ea, "dds", "[NET] ipv6 tcp client %d recv %d bytes: %s\n",
                          sock_tcp_client, status, tcp_client_recv_buffer);
            }
            aos_sleep(1000);
        }

        status = closesocket(sock_tcp_client);
        if (status != ERROR)
            aicx_info(0x169d01, "d", "[NET] ipv6 tcp client socket %d closed!\n",
                      sock_tcp_client);
        else
            aicx_info(0x0e6359, "d", "[NET] ipv6 tcp client socket %d close fail!\n",
                      sock_tcp_client);
    }
}

int aic_nic_ipv6_tcp_test_init(void)
{
    tcp_thread_server = aos_create_thread("nic ipv6 tcp test server",
                                          tcp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 30,
                                          AOS_AUTO_START);
    if (tcp_thread_server == AOS_INVALID_THREAD_ID)
        aicx_info(0x09494d, "", "[NET] Error in create nic ipv6 tcp test server\n");

    tcp_thread_client = aos_create_thread("nic ipv6 tcp test client",
                                          tcp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 30,
                                          AOS_AUTO_START);
    if (tcp_thread_client == AOS_INVALID_THREAD_ID)
        aicx_info(0x15f42a, "", "[NET] Error in create nic ipv6 tcp test client\n");

    return NX_SUCCESS;
}

AIC_APP_NORMAL_P2_INIT_EXPORT(aic_nic_ipv6_tcp_test_init);

