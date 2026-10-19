
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x109f29
#define LOG_MODULE    AMOD_NET
#include "aic_osa.h"
#include "aic_init.h"
#include <string.h>
#include <stdlib.h>
#include <aic_size.h>

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

#define DEMO_STACK_SIZE SIZE_2K
#define SERVER_PORT 85
#define CLIENT_PORT 75
#define TCP_SERVER_RCV_BUFFER_SIZE 100

static aos_thread_id tcp_thread_server;
static aos_thread_id tcp_thread_client;
static struct netif tcp_test_netif;

static fd_set master_list, read_ready;

static u8 tcp_server_recv_buffer[TCP_SERVER_RCV_BUFFER_SIZE];
static u8 tcp_client_recv_buffer[100];

const static ip4_addr_t test_local_ip = {0xC0A80001};
const static ip4_addr_t test_remote_ip = {0xC0A80001};

static void tcp_thread_server_entry(void *thread_input)
{
    struct sockaddr_in server_addr, client_addr;
    int status, sock, sock_tcp_server, client_len, i, is_set;
    ip4_addr_t test_local_ip = {0x0100A8C0};
    ip4_addr_t test_local_gw = {0x0000A8C0};
    ip4_addr_t test_netmask = {0x00FFFFFF};

    IP4_ADDR(&test_local_ip, 192,168,0,1);
    IP4_ADDR(&test_netmask, 255,255,255,0);
    IP4_ADDR(&test_local_gw, 192,168,0,254);

    aos_sleep(2000);
    netif_add(&tcp_test_netif, &test_local_ip, &test_netmask, &test_local_gw,
              NULL, lwip_ims_init_netif, NULL);

    netif_set_default(&tcp_test_netif);
    netif_set_up(&tcp_test_netif);

    aicx_info(0x09f85b, "xxx", "ip 0x%x gw 0x%x mask 0x%x\r\n",tcp_test_netif.ip_addr.u_addr.ip4.addr,
              tcp_test_netif.gw.u_addr.ip4.addr, tcp_test_netif.netmask.u_addr.ip4.addr);

    sock_tcp_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_tcp_server == -1) {
        aicx_info(0x0cb37f, "", "[NET] IPV4 TCP BSD TCP Server socket create fail\n");
        return;
    }

    aicx_info(0x176480, "u", "[NET] BSD IPv4 TCP Server socket created %u\n",
              (u32)sock_tcp_server);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(test_remote_ip.addr);
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
        if (status <= 0) {
            aicx_info(0x19c4c4, "x", "[NET] IPv4 TCP select err 0x%x try again\n", status);
            continue;
        }

        is_set = FD_ISSET(sock_tcp_server, &read_ready);
        if (is_set) {
            client_len = sizeof(client_addr);
            sock = accept(sock_tcp_server, (struct sockaddr *)&client_addr,
                          (socklen_t *)&client_len);
            FD_SET(sock, &master_list);

            if (sock > FD_SETSIZE) {
                aicx_info(0x085a34, "d", "[NET] IPv4 TCP new connection %d\n", sock);
            }

            continue;
        }

        for (i = 0; i < FD_SETSIZE; i++) {
            if ((i != sock_tcp_server) && (FD_ISSET(i, &master_list)) &&
                (FD_ISSET(i, &read_ready))) {
                    while (1) {
                        status = recv(i, (void *)tcp_server_recv_buffer,
                                      TCP_SERVER_RCV_BUFFER_SIZE, 0);
                        if (status < 0) {
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
                                  i, (u32)status, tcp_server_recv_buffer);

                        status = send(i, "IPV4 TCP Hello\n",
                                      sizeof("IPV4 TCP Hello\n"), 0);
                        if (status < 0)
                            aicx_info(0x1c994f, "d", "[NET] IPv4 tcp server send fail s:%d\n",
                                      i);
                        aos_sleep(1000);
                    }

                    status = closesocket(i);
                    if (status >= 0)
                        aicx_info(0x107fdc, "d", "[NET] IPv4 TCP socket closesocket client on %d\n",
                                  i);
                    else
                        aicx_info(0x1a7e56, "d", "[NET] IPv4 TCP socket closesocket on %d fail\n",
                                  i);
            }
        }
    }
}

static void tcp_thread_client_entry(void *thread_input)
{
    int status, sock_tcp_client, length;
    struct sockaddr_in echo_serv_addr, local_addr, remote_addr;

    aos_sleep(3000);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(test_local_ip.addr);
    local_addr.sin_port = htons(CLIENT_PORT);

    memset(&echo_serv_addr, 0, sizeof(echo_serv_addr));
    echo_serv_addr.sin_family = AF_INET;
    echo_serv_addr.sin_addr.s_addr = htonl(test_remote_ip.addr);
    echo_serv_addr.sin_port = htons(SERVER_PORT);

    while (1) {
        sock_tcp_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock_tcp_client == -1) {
            aicx_info(0x094f17, "", "[NET] BSD IPv4 TCP client socket create fail!\n");
            return;
        }

        aicx_info(0x19f4b3, "u", "[NET] IPv4 TCP client socket created ok %u\n",
                  (u32)sock_tcp_client);

        status = connect(sock_tcp_client, (struct sockaddr *)&echo_serv_addr,
                         sizeof(echo_serv_addr));
        if (status < 0) {
            aicx_info(0x048172, "xd", "[NET] IPv4 TCP client socket connect fail 0x%x errno %d!\n",
                      status, errno);
            status = closesocket(sock_tcp_client);
            return;
        }

        aicx_info(0x0c63d5, "d", "[NET] IPv4 TCP client socket %d connected\n",
                  sock_tcp_client);

        length = sizeof(struct sockaddr_in);
        status = getsockname(sock_tcp_client, (struct sockaddr *)&local_addr,
                             (socklen_t *)&length);
        aicx_info(0x125faf, "ux", "[NET] IPv4 TCP client port %u client 0x%x\n",
                  (u32)local_addr.sin_port,
                  (u32)local_addr.sin_addr.s_addr);

        length = sizeof(struct sockaddr_in);
        status = getpeername(sock_tcp_client, (struct sockaddr *)&remote_addr,
                             (socklen_t *)&length);
        aicx_info(0x0dcf8d, "ux", "[NET] IPv4 TCP remote port %u remote 0x%x\n",
                  (u32)remote_addr.sin_port,
                  (u32)remote_addr.sin_addr.s_addr);

        while (1) {
            status = send(sock_tcp_client, "IPV4 TCP Hello",
                          (sizeof("IPV4 TCP Hello")), 0);
            if (status < 0)
                aicx_info(0x103c4f, "d", "[NET] IPv4 TCP client socket %d send fail!\n",
                          sock_tcp_client);

READ_AGAIN:
            status = recv(sock_tcp_client, (void *)tcp_client_recv_buffer,
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

        status = closesocket(sock_tcp_client);
        if (status > 0)
            aicx_info(0x117a54, "d", "[NET] IPv4 TCP client socket %d closesocketd!\n",
                      sock_tcp_client);
    }
}

int aic_nic_ipv4_tcp_test_init(void)
{

    tcpip_init(NULL, NULL);

#if LWIP_NETCONN_SEM_PER_THREAD
    lwip_socket_thread_init();
#endif

    tcp_thread_server = aos_create_thread("nic_ipv4_tcp_test_server",
                                          tcp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 20,
                                          AOS_AUTO_START);

    tcp_thread_client = aos_create_thread("nic_ipv4_tcp_test_client",
                                          tcp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 30,
                                          AOS_AUTO_START);

    return 0;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv4_tcp_test_init);

