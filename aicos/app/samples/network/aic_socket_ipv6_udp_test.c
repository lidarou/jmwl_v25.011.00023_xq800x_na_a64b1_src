
/* Usage: Open CONFIG_NET_SOCKET_IPV6_UDP_TEST */
#include <aic_osa_ex.h>
#include <aic_init.h>
#include <string.h>
#include <stdlib.h>
#include <aic_net_api.h>

#define DEMO_STACK_SIZE 2048
#define SERVER_PORT_1   87

#define CLIENT_PORT_1   77

#define UDP_SERVER_RCV_BUFFER_SIZE 100

#define SERVER_TASK_PRIORITY    20
#define CLIENT_TASK_PRIORITY    30

static aos_thread_id udp_thread_server;
static aos_thread_id udp_thread_client;

static ULONG udp_error_counter;

static CHAR udp_server_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static CHAR udp_send_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static CHAR udp_client_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];

static VOID udp_thread_server_entry(void *thread_input)
{
    u8 ttl, tos;
    INT status, addr_len;
    struct aic_network_ip_instance *aic_ip;
    INT sock_udp_server_1;
    struct sockaddr_in6 server_addr_1, client_addr_1;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Designed for testing code */
    aic_ip = aic_get_pdp_ip_instance();
    if (!aic_ip->ip_ptr)
        aos_sleep(100);

    if (!aic_ip->bsd_task_stack)
        aic_init_bsd(aic_ip);

    /* Wait for IPv6 stack to finish DAD process. */
    aos_sleep(3000);

    sock_udp_server_1 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_server_1 == -1) {
        app_logi("[NET] ipv6 udp BSD udp Server 1 socket create fail\n");
        return;
    }

    app_logi("[NET] ipv6 udp BSD udp Server socket 1 created %u\n",
              (ULONG)sock_udp_server_1);

    memset(&server_addr_1, 0, sizeof(server_addr_1));
    server_addr_1.sin6_addr._S6_un._S6_u32[0] = htonl(0x0u);
    server_addr_1.sin6_addr._S6_un._S6_u32[1] = htonl(0x0u);
    server_addr_1.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    server_addr_1.sin6_addr._S6_un._S6_u32[3] = htonl(0x1u);
    server_addr_1.sin6_port = htons(SERVER_PORT_1);
    server_addr_1.sin6_family = AF_INET6;

    status = bind(sock_udp_server_1, (struct sockaddr *)&server_addr_1,
                  sizeof(server_addr_1));
    if (status < 0) {
        app_logi("[NET] ipv6 udp server socket 1 bind fail\n");
        return;
    }

    ttl = 64;
    tos = 32;
    status = setsockopt(sock_udp_server_1, IPPROTO_IP, IP_TOS, &tos, 1);
    if (status)
        app_logi("[NET] server set IP TOS fail\n");
    status = setsockopt(sock_udp_server_1, IPPROTO_IP, IP_TTL, &ttl, 1);
    if (status)
        app_logi("[NET] server set IP TTL fail\n");

    while (1) {
        addr_len = sizeof(struct sockaddr_in6);
        status = recvfrom(sock_udp_server_1, (VOID *)udp_server_recv_buffer,
                          UDP_SERVER_RCV_BUFFER_SIZE, 0,
                          (struct sockaddr *)&client_addr_1, &addr_len);
        if (status == ERROR) {
            if (errno == EAGAIN) {
                app_logi("[NET] ipv6 udp no error received, try again\n");
                continue;
            } else if (errno == ENOTCONN) {
                app_logi("[NET] ipv6 udp connect is broken, close socket.\n");
                break;
            } else {
                app_logi("[NET] ipv6 udp err on client sock recv data 0x%x\n",
                          errno);
                break;
            }
        }

        if (status == UDP_SERVER_RCV_BUFFER_SIZE)
            status--;

        udp_server_recv_buffer[status] = 0;
        app_logi("[NET] ipv6 udp server recv %d %u bytes: %s\n",
                  sock_udp_server_1, (ULONG)status, udp_server_recv_buffer);

        snprintf(udp_send_buffer, UDP_SERVER_RCV_BUFFER_SIZE,
                 "IPV6 UDP 1 hello world 0x%x\n", NX_RAND());
        status = sendto(sock_udp_server_1, udp_send_buffer,
                        strlen(udp_send_buffer), 0,
                        (struct sockaddr *)&client_addr_1,
                        sizeof(client_addr_1));
        if (status == ERROR)
            app_logi("[NET] ipv6 udp server sending to client fail s:%d\n",
                      sock_udp_server_1);
        else
            app_logi("[NET] udp server sending to client:%d: %s\n",
                      sock_udp_server_1, udp_send_buffer);

        aos_sleep(1000);
    }

    status = soc_close(sock_udp_server_1);
    if (status != ERROR)
        app_logi("[NET] ipv6 udp socket 1 close conn to client on %d\n",
                  sock_udp_server_1);
    else
        app_logi("[NET] ipv6 udp socket 1 close conn to client on %d fail\n",
                  sock_udp_server_1);
}

static VOID udp_thread_client_entry(void *thread_input)
{
    u8 ttl, tos;
    INT status, length;
    INT sock_udp_client_1;
    struct sockaddr_in6 echo_serv_addr6_1, local_addr6_1;

    NX_PARAMETER_NOT_USED(thread_input);

    aos_sleep(4000);

    memset(&local_addr6_1, 0, sizeof(local_addr6_1));
    local_addr6_1.sin6_addr._S6_un._S6_u32[0] = htonl(0x0u);
    local_addr6_1.sin6_addr._S6_un._S6_u32[1] = htonl(0x0u);
    local_addr6_1.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    local_addr6_1.sin6_addr._S6_un._S6_u32[3] = htonl(0x1u);
    local_addr6_1.sin6_port = htons(CLIENT_PORT_1);
    local_addr6_1.sin6_family = AF_INET6;

    memset(&echo_serv_addr6_1, 0, sizeof(echo_serv_addr6_1));
    echo_serv_addr6_1.sin6_addr._S6_un._S6_u32[0] = htonl(0x0u);
    echo_serv_addr6_1.sin6_addr._S6_un._S6_u32[1] = htonl(0x0u);
    echo_serv_addr6_1.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    echo_serv_addr6_1.sin6_addr._S6_un._S6_u32[3] = htonl(0x1u);
    echo_serv_addr6_1.sin6_port = htons(SERVER_PORT_1);
    echo_serv_addr6_1.sin6_family = AF_INET6;

    ttl = 64;
    tos = 32;

    while (1) {
        sock_udp_client_1 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_client_1 == -1) {
            app_logi("[NET] ipv6 udp BSD udp client socket 1 create fail!\n");
            return;
        }

        status = setsockopt(sock_udp_client_1, IPPROTO_IP, IP_TOS, &tos, 1);
        if (status)
            app_logi("[NET] ipv6 udp client set IP TOS fail\n");
        status = setsockopt(sock_udp_client_1, IPPROTO_IP, IP_TTL, &ttl, 1);
        if (status)
            app_logi("[NET] ipv6 udp client set IP TTL fail\n");

        status = bind(sock_udp_client_1, (struct sockaddr *)&local_addr6_1,
                      sizeof(local_addr6_1));
        if (status < 0) {
            app_logi("[NET] ipv6 udp client socket 1 bind fail\n");
            return;
        }

        app_logi("[NET] ipv6 udp client socket 1 created ok %u\n",
                  (ULONG)sock_udp_client_1);

        length = sizeof(struct sockaddr_in6);
        snprintf(udp_send_buffer, UDP_SERVER_RCV_BUFFER_SIZE,
                 "IPV6 UDP 1 hello world 0x%x\n", NX_RAND());

        status = sendto(sock_udp_client_1, udp_send_buffer,
                        strlen(udp_send_buffer), 0,
                        (struct sockaddr *)&echo_serv_addr6_1,
                        sizeof(echo_serv_addr6_1));
        if (status == ERROR)
            app_logi("[NET] ipv6 udp client socket 1 %d send fail!\n",
                      sock_udp_client_1);
        else
            app_logi("[NET] udp client socket 1 %d send: %s!\n",
                      sock_udp_client_1, udp_send_buffer);
READ_AGAIN_1:
        status = recvfrom(sock_udp_client_1, (VOID *)udp_client_recv_buffer,
                          100, 0, (struct sockaddr *)&echo_serv_addr6_1,
                          &length);
        if (status < 0) {
            if (errno == EAGAIN)
                goto READ_AGAIN_1;
            app_logi("[NET] ipv6 udp connect 1 err recv socket %d 0x%x 0x%x\n",
                      sock_udp_client_1, status, errno);
        } else {
            app_logi("[NET] ipv6 udp client 1 %d recv %d bytes: %s\n",
                      sock_udp_client_1, status, udp_client_recv_buffer);
        }

        status = soc_close(sock_udp_client_1);
        if (status != ERROR)
            app_logi("[NET] ipv6 udp client socket 1 %d closed!\n",
                      sock_udp_client_1);
        else
            app_logi("[NET] ipv6 udp client socket 1 %d close fail!\n",
                      sock_udp_client_1);

        aos_sleep(1000);
    }
}

int aic_nic_ipv6_udp_test_init(void)
{
    udp_thread_server = aos_create_thread("nic ipv6 udp test server",
                                          udp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL,
                                          SERVER_TASK_PRIORITY,
                                          AOS_AUTO_START);
    if (udp_thread_server == AOS_INVALID_THREAD_ID)
    {
        udp_error_counter++;
        app_logi("[NET] Error in create nic ipv6 udp test server\n");
    }

    udp_thread_client = aos_create_thread("nic ipv6 udp test client",
                                          udp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL,
                                          CLIENT_TASK_PRIORITY,
                                          AOS_AUTO_START);
    if (udp_thread_client == AOS_INVALID_THREAD_ID)
    {
        udp_error_counter++;
        app_logi("[NET] Error in create nic ipv6 udp test client\n");
    }

    return NX_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_nic_ipv6_udp_test_init);

