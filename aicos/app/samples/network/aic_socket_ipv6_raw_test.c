
/* Usage: Open CONFIG_NET_SOCKET_IPV6_RAW_TEST */
#include <aic_osa_ex.h>
#include <aic_init.h>
#include <string.h>
#include <stdlib.h>
#include <aic_net_api.h>

#define DEMO_STACK_SIZE 2048
#define SERVER_PORT_1   87

#define CLIENT_PORT_1   77

#define RAW_SERVER_RCV_BUFFER_SIZE 100

#define SERVER_TASK_PRIORITY    20
#define CLIENT_TASK_PRIORITY    30

static aos_thread_id raw_thread_server;
static aos_thread_id raw_thread_client;

static ULONG raw_error_counter;

static CHAR raw_server_recv_buffer[RAW_SERVER_RCV_BUFFER_SIZE];
static CHAR raw_send_buffer[RAW_SERVER_RCV_BUFFER_SIZE];

static VOID raw_thread_server_entry(void *thread_input)
{
    u8 ttl, tos;
    INT status, addr_len, offset = 4;
    struct aic_network_ip_instance *aic_ip;
    INT sock_raw_server_1;
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

    sock_raw_server_1 = socket(AF_INET6, SOCK_RAW, 0x99);
    if (sock_raw_server_1 == -1) {
        app_logi("[NET] ipv6 BSD raw Server 1 socket create fail\n");
        return;
    }

    app_logi("[NET] ipv6 BSD raw Server socket 1 created %u\n",
              (ULONG)sock_raw_server_1);

    memset(&server_addr_1, 0, sizeof(server_addr_1));
    server_addr_1.sin6_addr._S6_un._S6_u32[0] = htonl(0x0u);
    server_addr_1.sin6_addr._S6_un._S6_u32[1] = htonl(0x0u);
    server_addr_1.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    server_addr_1.sin6_addr._S6_un._S6_u32[3] = htonl(0x1u);
    server_addr_1.sin6_family = AF_INET6;

    status = bind(sock_raw_server_1, (struct sockaddr *)&server_addr_1,
                  sizeof(server_addr_1));
    if (status < 0) {
        app_logi("[NET] ipv6 raw server socket 1 bind fail\n");
        return;
    }

    ttl = 64;
    tos = 32;
    status = setsockopt(sock_raw_server_1, IPPROTO_IP, IP_TOS, &tos, 1);
    if (status)
        app_logi("[NET] server set IP TOS fail\n");
    status = setsockopt(sock_raw_server_1, IPPROTO_IP, IP_TTL, &ttl, 1);
    if (status)
        app_logi("[NET] server set IP TTL fail\n");
    status = setsockopt(sock_raw_server_1, IPPROTO_RAW, IPV6_CHECKSUM, &offset, sizeof(offset));
    if (status)
        app_logi("[NET] ipv6 raw server set IPV6_CHECKSUM fail\n");

    while (1) {
        addr_len = sizeof(struct sockaddr_in6);
        status = recvfrom(sock_raw_server_1, (VOID *)raw_server_recv_buffer,
                          RAW_SERVER_RCV_BUFFER_SIZE, 0,
                          (struct sockaddr *)&client_addr_1, &addr_len);
        if (status == ERROR) {
            if (errno == EAGAIN) {
                app_logi("[NET] ipv6 raw no error received, try again\n");
                continue;
            } else if (errno == ENOTCONN) {
                app_logi("[NET] ipv6 raw connect is broken, close socket.\n");
                break;
            } else {
                app_logi("[NET] ipv6 raw err on client sock recv data 0x%x\n",
                          errno);
                break;
            }
        }

        if (status == RAW_SERVER_RCV_BUFFER_SIZE)
            status--;

        raw_server_recv_buffer[status] = 0;
        app_logi("[NET] ipv6 raw server recv %d %u bytes: %s\n",
                  sock_raw_server_1, (ULONG)status, raw_server_recv_buffer);
    }

    status = soc_close(sock_raw_server_1);
    if (status != ERROR)
        app_logi("[NET] ipv6 raw socket 1 close conn to client on %d\n",
                  sock_raw_server_1);
    else
        app_logi("[NET] ipv6 raw socket 1 close conn to client on %d fail\n",
                  sock_raw_server_1);
}

static VOID raw_thread_client_entry(void *thread_input)
{
    u8 ttl, tos;
    INT status, length, offset = 4;
    INT sock_raw_client_1;
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
    echo_serv_addr6_1.sin6_family = AF_INET6;

    ttl = 64;
    tos = 32;

    while (1) {
        sock_raw_client_1 = socket(AF_INET6, SOCK_RAW, 0x99);
        if (sock_raw_client_1 == -1) {
            app_logi("[NET] ipv6 raw BSD raw client socket 1 create fail!\n");
            return;
        }

        status = setsockopt(sock_raw_client_1, IPPROTO_IP, IP_TOS, &tos, 1);
        if (status)
            app_logi("[NET] ipv6 raw client set IP TOS fail\n");
        status = setsockopt(sock_raw_client_1, IPPROTO_IP, IP_TTL, &ttl, 1);
        if (status)
            app_logi("[NET] ipv6 raw client set IP TTL fail\n");
        status = setsockopt(sock_raw_client_1, IPPROTO_RAW, IPV6_CHECKSUM, &offset, sizeof(offset));
        if (status)
            app_logi("[NET] ipv6 raw client set IPV6_CHECKSUM fail\n");

        status = bind(sock_raw_client_1, (struct sockaddr *)&local_addr6_1,
                      sizeof(local_addr6_1));
        if (status < 0) {
            app_logi("[NET] ipv6 raw client socket 1 bind fail\n");
            return;
        }

        app_logi("[NET] ipv6 raw client socket 1 created ok %u\n",
                  (ULONG)sock_raw_client_1);

        length = sizeof(struct sockaddr_in6);
        snprintf(raw_send_buffer, RAW_SERVER_RCV_BUFFER_SIZE,
                 "IPV6 raw 1 hello world 0x%x\n", NX_RAND());

        raw_send_buffer[0] = 128;
        raw_send_buffer[1] = 0;
        raw_send_buffer[5] = 1;
        raw_send_buffer[7] = 45;
        status = sendto(sock_raw_client_1, raw_send_buffer,
                        40, 0,
                        (struct sockaddr *)&echo_serv_addr6_1,
                        sizeof(echo_serv_addr6_1));
        if (status == ERROR)
            app_logi("[NET] ipv6 raw client socket 1 %d send fail!\n",
                      sock_raw_client_1);
        else
            app_logi("[NET] raw client socket 1 %d send: %s!\n",
                      sock_raw_client_1, raw_send_buffer);

        aos_sleep(1000);
        soc_close(sock_raw_client_1);
    }
}

int aic_nic_ipv6_raw_test_init(void)
{
    raw_thread_server = aos_create_thread("nic ipv6 raw test server",
                                          raw_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL,
                                          SERVER_TASK_PRIORITY,
                                          AOS_AUTO_START);
    if (raw_thread_server == AOS_INVALID_THREAD_ID)
    {
        raw_error_counter++;
        app_logi("[NET] Error in create nic ipv6 raw test server\n");
    }

    raw_thread_client = aos_create_thread("nic ipv6 raw test client",
                                          raw_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL,
                                          CLIENT_TASK_PRIORITY,
                                          AOS_AUTO_START);
    if (raw_thread_client == AOS_INVALID_THREAD_ID)
    {
        raw_error_counter++;
        app_logi("[NET] Error in create nic ipv6 raw test client\n");
    }

    return NX_SUCCESS;
}
AIC_APP_P2_INIT_EXPORT(aic_nic_ipv6_raw_test_init);


