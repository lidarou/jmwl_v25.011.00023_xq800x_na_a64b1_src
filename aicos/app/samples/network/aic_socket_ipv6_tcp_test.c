
/* Usage: Open CONFIG_NET_SOCKET_IPV6_TCP_TEST */

/* Auto gen by tool, don't change it. */
#include <aic_osa_ex.h>
#include <aic_init.h>
#include <string.h>
#include <stdlib.h>

#include <aic_net_api.h>

#define DEMO_STACK_SIZE 2048
#define SERVER_PORT 86
#define CLIENT_PORT 76
#define TCP_SERVER_RCV_BUFFER_SIZE 100

#define SERVER_TASK_PRIORITY    20
#define CLIENT_TASK_PRIORITY    30

static aos_thread_id tcp_thread_server;
static aos_thread_id tcp_thread_client;

static INT max_fd;

static fd_set master_list, read_ready;

static CHAR tcp_server_recv_buffer[TCP_SERVER_RCV_BUFFER_SIZE];
static CHAR tcp_client_recv_buffer[TCP_SERVER_RCV_BUFFER_SIZE];

static VOID tcp_thread_server_entry(void *thread_input)
{
    u8 ttl = 64, tos = 32;
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
    /* End */


    /* Wait for IPv6 stack to finish DAD process. */
    aos_sleep(3000);

    sock_tcp_server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sock_tcp_server == -1) {
        app_logi("[NET] BSD ipv6 tcp Server socket create fail\n");
        return;
    }

    app_logi("[NET] BSD ipv6 tcp Server socket created %u\n",
              (ULONG)sock_tcp_server);
    status = setsockopt(sock_tcp_server, IPPROTO_IP, IP_TOS, &tos, 1);
    if (status)
        app_logi("[NET] ipv6 tcp server set IP TOS fail\n");
    status = setsockopt(sock_tcp_server, IPPROTO_IP, IP_TTL, &ttl, 1);
    if (status)
        app_logi("[NET] ipv6 tcp server set IP TTL fail\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_addr._S6_un._S6_u32[0] = htonl(0x0);
    server_addr.sin6_addr._S6_un._S6_u32[1] = htonl(0x0);
    server_addr.sin6_addr._S6_un._S6_u32[2] = htonl(0x0);
    server_addr.sin6_addr._S6_un._S6_u32[3] = htonl(0x1u);
    server_addr.sin6_port = htons(SERVER_PORT);
    server_addr.sin6_family = AF_INET6;

    /* Bind this server socket */
    status = bind(sock_tcp_server, (struct sockaddr *)&server_addr,
                  sizeof(server_addr));
    if (status < 0) {
        app_logi("[NET] ipv6 tcp server socket bind fail\n");
        return;
    }

    FD_ZERO(&master_list);
    FD_ZERO(&read_ready);
    FD_SET(sock_tcp_server, &master_list);
    max_fd = sock_tcp_server;

    /* Now listen for any client connections for this server socket */
    status = listen(sock_tcp_server, 5);
    if (status < 0) {
        app_logi("[NET] ipv6 tcp server socket listen fail\n");
        return;
    } else {
        app_logi("[NET] ipv6 tcp server listen complete\n");
    }

    app_logi("[NET] ipv6 tcp Now accepting client connections %d\n", max_fd);

    while (1) {
        read_ready = master_list;
        aos_sleep(20);

        /* Let the underlying TCP stack determine the timeout. */
        status = select(max_fd + 1, &read_ready, 0, 0, 0);
        if (status == ERROR || status == 0) {
            app_logi("[NET] ipv6 tcp select err 0x%x try again\n", status);
            continue;
        }

        /* Detected a connection request. */
        is_set = FD_ISSET(sock_tcp_server, &read_ready);
        if (is_set) {
            client_len = sizeof(client_addr);
            sock = accept(sock_tcp_server, (struct sockaddr *)&client_addr,
                          &client_len);
            /* Add this new connection to our master list */
            FD_SET(sock, &master_list);

            if (sock > max_fd) {
                app_logi("[NET] ipv6 tcp new connection %d\n", sock);
                max_fd = sock;
            }

            continue;
        }

        /* Check the set of 'ready' sockets, e.g connected to remote host and waiting for
           notice of packets received. */
        for (i = 0; i < (max_fd + 1); i++) {
            if (((i + AIC_BSD_SOCKFD_START) != sock_tcp_server) &&
                (FD_ISSET(i + AIC_BSD_SOCKFD_START, &master_list)) &&
                (FD_ISSET(i + AIC_BSD_SOCKFD_START, &read_ready))) {
                while (1) {
                    status = recv(i + AIC_BSD_SOCKFD_START,
                                  (VOID *)tcp_server_recv_buffer,
                                  TCP_SERVER_RCV_BUFFER_SIZE, 0);
                    if (status == ERROR) {
                        if (errno == EAGAIN) {
                            /* This is a blocking socket. If no data is received, but the connection is still good,
                               the EAGAIN error is set. If it was a non blocking socket, the EWOULDBLOCK socket
                               error is set. */
                            app_logi("[NET] ipv6 tcp no err recv, again\n");
                            continue;
                        } else if (errno == ENOTCONN) {
                            /* If the socket connection is terminated, the socket error will be ENOTCONN. */
                            app_logi("[NET] ipv6 tcp connection is broken.\n");
                            break;
                        } else {
                            /* Another error has occurred...probably an internal error of some kind
                               so best to terminate the connection. */
                            app_logi("[NET] ipv6 tcp client err %d 0x%x\n",
                                       i, errno);
                            break;
                        }
                    }

                    /* recv returns with message.  Make sure Server_Rcv_Buffer is NULL-terminated. */
                    if (status == TCP_SERVER_RCV_BUFFER_SIZE)
                        status--;

                    tcp_server_recv_buffer[status] = 0;
                    app_logi("[NET] ipv6 tcp server recv %d %u bytes: %s\n",
                              i + AIC_BSD_SOCKFD_START, (ULONG)status,
                              tcp_server_recv_buffer);

                    status = send(i + AIC_BSD_SOCKFD_START, "ipv6 tcp Hello\n",
                                  sizeof("ipv6 tcp Hello\n"), 0);
                    if (status == ERROR)
                        app_logi("[NET] ipv6 tcp server send fail :%d\n",
                                  i + AIC_BSD_SOCKFD_START);
                        aos_sleep(1000);
                }

                /* Close this socket */
                status = soc_close(i + AIC_BSD_SOCKFD_START);
                if (status != ERROR)
                    app_logi("[NET] ipv6 tcp socket close client on %d\n",
                              i + AIC_BSD_SOCKFD_START);
                else
                    app_logi("[NET] ipv6 tcp socket close client on %d fail\n",
                              i + AIC_BSD_SOCKFD_START);
            }
        }
    }
}

static VOID tcp_thread_client_entry(void *thread_input)
{
    u8 tos = 32, ttl = 64;
    INT status, sock_tcp_client, length;
    struct sockaddr_in6 echo_serv_addr6, local_addr6, remote_addr6;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Wait for IPv6 stack to finish DAD process. */
    aos_sleep(4000);

    memset(&echo_serv_addr6, 0, sizeof(echo_serv_addr6));
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[0] = htonl(0x0u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[1] = htonl(0x0u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[3] = htonl(0x1u);
    echo_serv_addr6.sin6_port = htons(SERVER_PORT);
    echo_serv_addr6.sin6_family = AF_INET6;

    while (1) {
        sock_tcp_client = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (sock_tcp_client == -1) {
            app_logi("[NET] BSD ipv6 tcp client socket create fail!\n");
            return;
        }

        status = setsockopt(sock_tcp_client, IPPROTO_IP, IP_TOS, &tos, 1);
        if (status)
            app_logi("[NET] ipv6 tcp client set IP TOS fail\n");
        status = setsockopt(sock_tcp_client, IPPROTO_IP, IP_TTL, &ttl, 1);
        if (status)
            app_logi("[NET] ipv6 tcp client set IP TTL fail\n");

        app_logi("[NET] ipv6 tcp client socket created ok %u\n",
                  (ULONG)sock_tcp_client);
        status = connect(sock_tcp_client, (struct sockaddr *)&echo_serv_addr6,
                         sizeof(echo_serv_addr6));
        if (status != OK) {
            app_logi("[NET] BSD ipv6 tcp client socket connect fail 0x%x!\n",
                      status);
            status = soc_close(sock_tcp_client);
            return;
        }

        app_logi("[NET] ipv6 tcp client socket %d connected\n",
                  sock_tcp_client);

        length = sizeof(struct sockaddr_in6);
        /* Get the socket's primary interface address and port */
        status = getsockname(sock_tcp_client, (struct sockaddr *)&local_addr6,
                             &length);
        app_logi("[NET] ipv6 tcp client port %u client 0x%x 0x%x 0x%x 0x%x\n",
                  (ULONG)local_addr6.sin6_port,
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[0],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[1],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[2],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[3]);
        length = sizeof(struct sockaddr_in6);
        /* Get the socket's remote address and port */
        status = getpeername(sock_tcp_client, (struct sockaddr *)&remote_addr6,
                             &length);
        app_logi("[NET] ipv6 tcp remote port %u remote 0x%x 0x%x 0x%x 0x%x\n",
                  (ULONG)remote_addr6.sin6_port,
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[0],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[1],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[2],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[3]);

        while (1) {
            status = send(sock_tcp_client, "ipv6 tcp Hello",
                          (sizeof("ipv6 tcp Hello")), 0);
            if (status == ERROR)
                app_logi("[NET] ipv6 tcp client socket %d send fail!\n",
                          sock_tcp_client);
            else
                app_logi("[NET] ipv6 tcp client socket %d send hello!\n",
                          sock_tcp_client);
READ_AGAIN:
            status = recv(sock_tcp_client, (VOID *)tcp_client_recv_buffer,
                          100, 0);
            if (status < 0) {
                if (errno == EAGAIN)
                    goto READ_AGAIN;
                app_logi("[NET] ipv6 tcp connect error socket %d 0x%x 0x%x\n",
                          sock_tcp_client, status, errno);
                break;
            } else {
                app_logi("[NET] ipv6 tcp client %d recv %d bytes: %s\n",
                          sock_tcp_client, status, tcp_client_recv_buffer);
            }
            aos_sleep(1000);
        }

        status = soc_close(sock_tcp_client);
        if (status != ERROR)
            app_logi("[NET] ipv6 tcp client socket %d closed!\n",
                      sock_tcp_client);
        else
            app_logi("[NET] ipv6 tcp client socket %d close fail!\n",
                      sock_tcp_client);
    }
}

int aic_nic_ipv6_tcp_test_init(void)
{
    tcp_thread_server = aos_create_thread("nic ipv6 tcp test server",
                                          tcp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL,
                                          SERVER_TASK_PRIORITY,
                                          AOS_AUTO_START);
    if (tcp_thread_server == AOS_INVALID_THREAD_ID)
        app_logi("[NET] Error in create nic ipv6 tcp test server\n");

    tcp_thread_client = aos_create_thread("nic ipv6 tcp test client",
                                          tcp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL,
                                          CLIENT_TASK_PRIORITY,
                                          AOS_AUTO_START);
    if (tcp_thread_client == AOS_INVALID_THREAD_ID)
        app_logi("[NET] Error in create nic ipv6 tcp test client\n");

    return NX_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv6_tcp_test_init);

