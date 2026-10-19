
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x096c9a
#define LOG_MODULE    AMOD_DRV
#include "aic_osa.h"
#include "aic_init.h"
#include "nx_api.h"
#include "nxd_bsd.h"
#include <string.h>
#include <stdlib.h>

#include <aic_size.h>
#include <aic_net_api.h>
#include <nx_alte_driver.h>
#include <aic_net_pdp_mgr.h>

#define DEMO_STACK_SIZE SIZE_2K
#define SERVER_PORT 85
#define CLIENT_PORT 75
#define TCP_SERVER_RCV_BUFFER_SIZE 100

#define IP_LINK_DRIVER nx_alte_nic_driver_entry

static aos_thread_id tcp_thread_server;
static aos_thread_id tcp_thread_client;

static fd_set master_list, read_ready;

static CHAR tcp_server_recv_buffer[TCP_SERVER_RCV_BUFFER_SIZE];
static CHAR tcp_client_recv_buffer[100];

static VOID tcp_thread_server_entry(void *thread_input)
{
    struct aic_network_ip_instance *aic_ip;
    struct sockaddr_in server_addr, client_addr;
    INT status, sock, sock_tcp_server, client_len, i, is_set;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Designed for testing code */
    aic_ip = aic_get_pdp_ip_instance();
    if (!aic_ip->ip_ptr)
        aos_sleep(100);

    if (!aic_ip->bsd_task_stack)
        aic_init_bsd(aic_ip);

    status = nx_ip_interface_attach(aic_ip->ip_ptr, "IPv4 TCP Interface1",
                                    IP_ADDRESS(1, 2, 3, 4), 0xFFFFFF00UL,
                                    IP_LINK_DRIVER);
    if (status)
        aicx_info(0x1bef9d, "", "[NET] IPv4 TCP Error in attach IP Instance!\n");
    /* End */

    sock_tcp_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_tcp_server == -1) {
        aicx_info(0x0cb37f, "", "[NET] IPV4 TCP BSD TCP Server socket create fail\n");
        return;
    }

    aicx_info(0x176480, "u", "[NET] BSD IPv4 TCP Server socket created %u\n",
              (ULONG)sock_tcp_server);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(IP_ADDRESS(1, 2, 3, 4));
    server_addr.sin_port = htons(SERVER_PORT);

    status = bind(sock_tcp_server, (struct sockaddr *)&server_addr,
                  sizeof(server_addr));
    if (status < 0) {
        aicx_info(0x18d44d, "", "[NET] IPv4 TCP server socket bind fail\n");
        return;
    }

    FD_ZERO(&master_list);
    FD_ZERO(&read_ready);
    FD_SET(sock_tcp_server, &master_list);

    status = listen(sock_tcp_server, 5);
    if (status < 0) {
        aicx_info(0x181707, "", "[NET] IPv4 TCP server socket listen fail\n");
        return;
    } else {
        aicx_info(0x18d566, "", "[NET] IPv4 TCP server listen complete\n");
    }

    aicx_info(0x053ac8, "d", "[NET] IPv4 TCP Now accepting client connections %d\n",
              FD_SETSIZE);

    while (1) {
        read_ready = master_list;
        aos_sleep(20);
        status = select(FD_SETSIZE, &read_ready, 0, 0, 0);
        if (status == ERROR || status == 0) {
            aicx_info(0x19c4c4, "x", "[NET] IPv4 TCP select err 0x%x try again\n", status);
            continue;
        }

        is_set = FD_ISSET(sock_tcp_server, &read_ready);
        if (is_set) {
            client_len = sizeof(client_addr);
            sock = accept(sock_tcp_server, (struct sockaddr *)&client_addr,
                          &client_len);
            FD_SET(sock, &master_list);

            if (sock > FD_SETSIZE) {
                aicx_info(0x085a34, "d", "[NET] IPv4 TCP new connection %d\n", sock);
            }

            continue;
        }

        for (i = 0; i < FD_SETSIZE; i++) {
            if (((i + NX_BSD_SOCKFD_START) != sock_tcp_server) &&
                (FD_ISSET(i + NX_BSD_SOCKFD_START, &master_list)) &&
                (FD_ISSET(i + NX_BSD_SOCKFD_START, &read_ready))) {
                    while (1) {
                        status = recv(i + NX_BSD_SOCKFD_START,
                                      (VOID *)tcp_server_recv_buffer,
                                      TCP_SERVER_RCV_BUFFER_SIZE, 0);
                        if (status == ERROR) {
                            if (errno == EAGAIN) {
                                aicx_info(0x02df53, "", "[NET] IPv4 TCP noerr, try again \n");
                                continue;
                            } else if (errno == ENOTCONN) {
                                aicx_info(0x114cb7, "", "[NET] IPv4 TCP connect broken.\n");
                                break;
                            } else {
                                aicx_info(0x0ac64f, "dx", "[NET] IPv4 TCP err c %d recv 0x%x\n",
                                          i, errno);
                                break;
                            }
                        }

                        if (status == TCP_SERVER_RCV_BUFFER_SIZE)
                            status--;

                        tcp_server_recv_buffer[status] = 0;
                        aicx_info(0x044d52, "dus", "[NET] IPv4 tcp server recv %d %u bytes %s\n",
                                  i + NX_BSD_SOCKFD_START, (ULONG)status,
                                  tcp_server_recv_buffer);

                        status = send(i + NX_BSD_SOCKFD_START,
                                      "IPV4 TCP Hello\n",
                                      sizeof("IPV4 TCP Hello\n"), 0);
                        if (status == ERROR)
                            aicx_info(0x1c994f, "d", "[NET] IPv4 tcp server send fail s:%d\n",
                                      i + NX_BSD_SOCKFD_START);
                        aos_sleep(1000);
                    }

                    status = soc_close(i + NX_BSD_SOCKFD_START);
                    if (status != ERROR)
                        aicx_info(0x058ad6, "d", "[NET] IPv4 TCP socket close client on %d\n",
                                  i + NX_BSD_SOCKFD_START);
                    else
                        aicx_info(0x1e4c37, "d", "[NET] IPv4 TCP socket close on %d fail\n",
                                  i + NX_BSD_SOCKFD_START);
            }
        }
    }
}

static VOID tcp_thread_client_entry(void *thread_input)
{
    INT status, sock_tcp_client, length;
    struct sockaddr_in echo_serv_addr, local_addr, remote_addr;

    NX_PARAMETER_NOT_USED(thread_input);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(IP_ADDRESS(1, 2, 3, 4));
    local_addr.sin_port = htons(CLIENT_PORT);

    memset(&echo_serv_addr, 0, sizeof(echo_serv_addr));
    echo_serv_addr.sin_family = AF_INET;
    echo_serv_addr.sin_addr.s_addr = htonl(IP_ADDRESS(1, 2, 3, 4));
    echo_serv_addr.sin_port = htons(SERVER_PORT);

    while (1) {
        sock_tcp_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock_tcp_client == -1) {
            aicx_info(0x094f17, "", "[NET] BSD IPv4 TCP client socket create fail!\n");
            return;
        }

        aicx_info(0x19f4b3, "u", "[NET] IPv4 TCP client socket created ok %u\n",
                  (ULONG)sock_tcp_client);

        status = connect(sock_tcp_client, (struct sockaddr *)&echo_serv_addr,
                         sizeof(echo_serv_addr));
        if (status != OK) {
            aicx_info(0x025452, "x", "[NET] BSD IPv4 TCP client socket connect fail 0x%x!\n",
                      status);
            status = soc_close(sock_tcp_client);
            return;
        }

        aicx_info(0x0c63d5, "d", "[NET] IPv4 TCP client socket %d connected\n",
                  sock_tcp_client);

        length = sizeof(struct sockaddr_in);
        status = getsockname(sock_tcp_client, (struct sockaddr *)&local_addr,
                             &length);
        aicx_info(0x125faf, "ux", "[NET] IPv4 TCP client port %u client 0x%x\n",
                  (ULONG)local_addr.sin_port,
                  (UINT)local_addr.sin_addr.s_addr);
        length = sizeof(struct sockaddr_in);
        status = getpeername(sock_tcp_client, (struct sockaddr *)&remote_addr,
                             &length);
        aicx_info(0x0dcf8d, "ux", "[NET] IPv4 TCP remote port %u remote 0x%x\n",
                  (ULONG)remote_addr.sin_port,
                  (UINT)remote_addr.sin_addr.s_addr);

        while (1) {
            status = send(sock_tcp_client, "IPV4 TCP Hello",
                          (sizeof("IPV4 TCP Hello")), 0);
            if (status == ERROR)
                aicx_info(0x103c4f, "d", "[NET] IPv4 TCP client socket %d send fail!\n",
                          sock_tcp_client);
READ_AGAIN:
            status = recv(sock_tcp_client, (VOID *)tcp_client_recv_buffer,
                          100, 0);
            if (status < 0) {
                if (errno == EAGAIN)
                    goto READ_AGAIN;
                aicx_info(0x1848ea, "dxx", "[NET] IPv4 TCP error on recv socket %d 0x%x 0x%x\n",
                          sock_tcp_client, status, errno);
                break;
            } else {
                aicx_info(0x050461, "dds", "[NET] IPv4 TCP client %d recv %d bytes: %s\n",
                          sock_tcp_client, status, tcp_client_recv_buffer);
            }

            aos_sleep(1000);
        }

        status = soc_close(sock_tcp_client);
        if (status != ERROR)
            aicx_info(0x167dad, "d", "[NET] IPv4 TCP client socket %d closed!\n",
                      sock_tcp_client);
        else
            /* Auto gen by tool, don't change it. */
            aidx_info(AIC_THIS_FILE, 0x197b5c, "d",
                      sock_tcp_client);
    }
}

int aic_nic_ipv4_tcp_test_init(void)
{
    NXD_ADDRESS dst_addr;

    dst_addr.nxd_ip_version = NX_IP_VERSION_V4;
    dst_addr.nxd_ip_address.v4 = htonl(IP_ADDRESS(1, 2, 3, 4));

    tcp_thread_server = aos_create_thread("nic_ipv4_tcp_test_server",
                                          tcp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 20,
                                          AOS_AUTO_START);

    tcp_thread_client = aos_create_thread("nic_ipv4_tcp_test_client",
                                          tcp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 30,
                                          AOS_AUTO_START);

    return NX_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv4_tcp_test_init);

