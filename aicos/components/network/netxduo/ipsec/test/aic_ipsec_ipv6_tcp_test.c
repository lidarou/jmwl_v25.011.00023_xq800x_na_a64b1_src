
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ac955
#define LOG_MODULE    AMOD_NET
#include <aic_osa.h>
#include <aic_init.h>

#include "nx_api.h"
#include "nxd_bsd.h"
#include "nx_ipsec.h"

#include <string.h>
#include <stdlib.h>

#define IPSEC_3DES_SHA1

#define DEMO_STACK_SIZE (16 * 1024)
#define SERVER_PORT 87
#define CLIENT_PORT 77
#define TCP_SERVER_RCV_BUFFER_SIZE 100

static aos_thread_id tcp_thread_server;
static aos_thread_id tcp_thread_client;
static NX_PACKET_POOL bsd_tcp_pool;
static NX_IP bsd_tcp_ip;

static INT max_fd;

static ULONG tcp_error_counter;
static fd_set master_list, read_ready;

static UCHAR tcp_packet_pool[16384];
static UCHAR tcp_ip_instance_pool[2048];
static UCHAR tcp_bsd_pool[2048];

static CHAR tcp_server_recv_buffer[TCP_SERVER_RCV_BUFFER_SIZE];
static CHAR tcp_client_recv_buffer[100];

static VOID tcp_thread_server_entry(void *thread_input);
static VOID tcp_thread_client_entry(void *thread_input);
VOID _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);
#ifdef IPSEC_AES_MD5
static UCHAR encrypt_key_cbc_128_encrypt[] = {
    0x02, 0xCF, 0x1E, 0x1D, 0x97, 0x96, 0xAD, 0x33,
    0x95, 0xD8, 0x12, 0x67, 0x57, 0xB6, 0xFC, 0x7C,
};
static UCHAR authentication_key_1[] = {
    0xBF, 0x8D, 0x54, 0x47, 0x0B, 0x5F, 0x36, 0x5F,
    0x14, 0xF8, 0x4C, 0x0E, 0x04, 0x6D, 0xE4, 0x05,
    0x3D, 0xE7, 0xA0, 0x5D, 0xB8, 0x47, 0xCD, 0x2C,
    0x98, 0x44, 0x63, 0x20, 0x82, 0xF5, 0x88, 0x60,
    0x2E, 0xA3, 0x88, 0x5D, 0xEF, 0x9C, 0xA2, 0x1B,
    0x8B, 0x46, 0xAA, 0x7C, 0xB9, 0x58, 0x89, 0x38,
};
#elif defined(IPSEC_NONE_MD5)
static UCHAR authentication_key_1[] = {
    0xBF, 0x8D, 0x54, 0x47, 0x0B, 0x5F, 0x36, 0x5F,
    0x14, 0xF8, 0x4C, 0x0E, 0x04, 0x6D, 0xE4, 0x05,
    0x3D, 0xE7, 0xA0, 0x5D, 0xB8, 0x47, 0xCD, 0x2C,
    0x98, 0x44, 0x63, 0x20, 0x82, 0xF5, 0x88, 0x60,
    0x2E, 0xA3, 0x88, 0x5D, 0xEF, 0x9C, 0xA2, 0x1B,
    0x8B, 0x46, 0xAA, 0x7C, 0xB9, 0x58, 0x89, 0x38,
};
#else
static UCHAR encrypt_key_3des_cbc_encrypt[] = {
    0x8f, 0x4f, 0x7a, 0xab, 0x25, 0x04, 0x37, 0x20,
    0xf4, 0xfb, 0xae, 0x01, 0xae, 0xdf, 0x07, 0x1c,
    0x68, 0xa2, 0x83, 0x68, 0x9b, 0x08, 0xad, 0x20,
};

static UCHAR authentication_sha1_key_96[] = {
    0x39, 0x56, 0x7E, 0x74, 0x08, 0x5C, 0x7B, 0x4C,
    0x94, 0xBC, 0xED, 0x43, 0x1A, 0x18, 0xF1, 0x09,
    0xBB, 0xAC, 0xEF, 0x73, 0x47, 0x1C, 0x0E, 0x00,
    0xB7, 0xFA, 0x0B, 0x1C, 0x4F, 0x97, 0x95, 0x76,
    0xB5, 0xF1, 0x02, 0x5A, 0xD5, 0xAD, 0xAA, 0x12,
    0x46, 0xA6, 0x97, 0x4C, 0xEB, 0x6B, 0xEC, 0x30,
    0xCF, 0x5C, 0xA1, 0x4F, 0x65, 0x23, 0x1F, 0x31,
    0xF2, 0x43, 0x38, 0x70, 0x7A, 0xD3, 0x5B, 0x7C,
};
#endif
static VOID tcp_thread_server_entry(void *thread_input)
{
    ULONG actual_status;
    NXD_ADDRESS ip_address;
    UINT iface_index, address_index;
    NX_IPSEC_SA *sa_1 = NULL, *sa_2 = NULL;
    struct sockaddr_in6 server_addr, client_addr;
    INT status, sock, sock_tcp_server, client_len, i, is_set;
    NXD_ADDRESS dst_addr;

    dst_addr.nxd_ip_version = NX_IP_VERSION_V6;
    dst_addr.nxd_ip_address.v6[0] = htonl(0x20010db8u);
    dst_addr.nxd_ip_address.v6[1] = htonl(0xf101u);
    dst_addr.nxd_ip_address.v6[2] = htonl(0x0u);
    dst_addr.nxd_ip_address.v6[3] = htonl(0x0101u);

    NX_PARAMETER_NOT_USED(thread_input);

    status = (INT)nx_ip_status_check(&bsd_tcp_ip, NX_IP_INITIALIZE_DONE,
                                     &actual_status, NX_IP_PERIODIC_RATE);
    if (status != NX_SUCCESS) {
        aicx_info(0x11b725, "", "[NET] ip status check fail!\n");
        return;
    }

    status = (INT)nxd_ipv6_enable(&bsd_tcp_ip);
    if (status != NX_SUCCESS) {
        aicx_info(0x1db7cb, "x", "[NET] enable ipv6 fail 0x%x\n", status);
        return;
    }

    iface_index = 0;
    status = (INT)nxd_ipv6_address_set(&bsd_tcp_ip, iface_index, NX_NULL,
                                       10, &address_index);
    if (status) {
        aicx_info(0x057978, "x", "[NET] ipv6 primary addr set fail 0x%x\n", status);
        return;
    }

    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 0x101;

    status = (INT)nxd_ipv6_address_set(&bsd_tcp_ip, iface_index, &ip_address,
                                       64, &address_index);
    if (status) {
        aicx_info(0x068cf7, "x", "[NET] ipv6 host addr set fail 0x%x\n", status);
        return;
    }

    aos_sleep(5 * NX_IP_PERIODIC_RATE);

    sock_tcp_server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sock_tcp_server == -1) {
        aicx_info(0x037b37, "", "[NET] BSD TCP Server socket create fail\n");
        return;
    }

    aicx_info(0x1ca13c, "u", "[NET] BSD TCP Server socket created %u\n",
              (ULONG)sock_tcp_server);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    server_addr.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    server_addr.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    server_addr.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    server_addr.sin6_port = htons(SERVER_PORT);
    server_addr.sin6_family = AF_INET6;

    status = bind(sock_tcp_server, (struct sockaddr *)&server_addr,
                  sizeof(server_addr));
    if (status < 0) {
        aicx_info(0x1be98e, "", "[NET] ipv6 server socket bind fail\n");
        return;
    }

#ifdef IPSEC_AES_MD5
    status = _nx_ipsec_create_ingress_sa(&bsd_tcp_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_AES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         encrypt_key_cbc_128_encrypt,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 1,
                                         0, &dst_addr, SERVER_PORT, CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_tcp_ip, &sa_2,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_AES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         encrypt_key_cbc_128_encrypt,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 2,
                                         0, &dst_addr, CLIENT_PORT, SERVER_PORT);

#elif defined(IPSEC_NONE_MD5)
    status = _nx_ipsec_create_ingress_sa(&bsd_tcp_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_NULL,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         NULL,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 1,
                                         0, &dst_addr, SERVER_PORT, CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_tcp_ip, &sa_2,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_NULL,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         NULL,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 2,
                                         0, &dst_addr, CLIENT_PORT, SERVER_PORT);
#else
    status = _nx_ipsec_create_ingress_sa(&bsd_tcp_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                         encrypt_key_3des_cbc_encrypt,
                                         sizeof(encrypt_key_3des_cbc_encrypt) <<
                                         3, authentication_sha1_key_96,
                                         sizeof(authentication_sha1_key_96) <<
                                         3, 1, 0, &dst_addr, SERVER_PORT, CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_tcp_ip, &sa_2,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                         encrypt_key_3des_cbc_encrypt,
                                         sizeof(encrypt_key_3des_cbc_encrypt) <<
                                         3, authentication_sha1_key_96,
                                         sizeof(authentication_sha1_key_96) <<
                                         3, 2, 0, &dst_addr, CLIENT_PORT, SERVER_PORT);
#endif

    FD_ZERO(&master_list);
    FD_ZERO(&read_ready);
#ifdef WIN32
    ANET_FD_SET(sock_tcp_server, &master_list);
#else
    FD_SET(sock_tcp_server, &master_list);
#endif
    max_fd = sock_tcp_server;

    status = listen(sock_tcp_server, 5);
    if (status < 0) {
        aicx_info(0x1dd74c, "", "[NET] server socket listen fail\n");
        return;
    } else {
        aicx_info(0x1495fe, "", "[NET] server listen complete\n");
    }

    aicx_info(0x023af6, "d", "[NET] Now accepting client connections %d\n", max_fd);

    while (1) {
        read_ready = master_list;
        aos_sleep(20);
        status = select(max_fd + 1, &read_ready, 0, 0, 0);
        if (status == ERROR || status == 0) {
            aicx_info(0x1be7d0, "x", "[NET] select err 0x%x try again\n", status);
            continue;
        }

        is_set = FD_ISSET(sock_tcp_server, &read_ready);
        if (is_set) {
            client_len = sizeof(client_addr);
            sock = accept(sock_tcp_server, (struct sockaddr *)&client_addr,
                          &client_len);
#ifdef WIN32
            ANET_FD_SET(sock, &master_list);
#else
            FD_SET(sock, &master_list);
#endif

            if (sock > max_fd) {
                aicx_info(0x10abe3, "d", "[NET] new connection %d\n", sock);
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
                            aicx_info(0x034f9e, "", "[NET] no error recv, try again\n");
                            continue;
                        } else if (errno == ENOTCONN) {
                            aicx_info(0x0db8c0, "", "[NET] connection is broken.\n");
                            break;
                        } else {
                            aicx_info(0x14e565, "dx", "[NET] client sock err %d 0x%x\n",
                                       i, errno);
                            break;
                        }
                    }

                    if (status == TCP_SERVER_RCV_BUFFER_SIZE)
                        status--;

                    tcp_server_recv_buffer[status] = 0;
                    aicx_info(0x1b0ea2, "dus", "[NET] tcp server recv %d %u bytes: %s\n",
                              i + NX_BSD_SOCKFD_START, (ULONG)status,
                              tcp_server_recv_buffer);

                    status = send(i + NX_BSD_SOCKFD_START, "Hello\n",
                                  sizeof("Hello\n"), 0);
                    if (status == ERROR)
                        aicx_info(0x155d5e, "d", "[NET] tcp server send to client fail :%d\n",
                                  i + NX_BSD_SOCKFD_START);
                    else
                        aicx_info(0x0e5d3a, "d", "[NET] tcp server send to c:%d: hello\n",
                                  i + NX_BSD_SOCKFD_START);
                    aos_sleep(1000);
                }

                status = soc_close(i + NX_BSD_SOCKFD_START);
                if (status != ERROR)
                    aicx_info(0x0aba03, "d", "[NET] socket close conn to client on %d\n",
                              i + NX_BSD_SOCKFD_START);
                else
                    aicx_info(0x0b2735, "d", "[NET] socket close conn to client on %d fail\n",
                              i + NX_BSD_SOCKFD_START);
            }
        }
    }
}

static VOID tcp_thread_client_entry(void *thread_input)
{
    ULONG actual_status;
    INT status, sock_tcp_client, length;
    NX_IPSEC_SA *sa_1 = NULL, *sa_2 = NULL;
    struct sockaddr_in6 echo_serv_addr6, local_addr6, remote_addr6;
    NXD_ADDRESS dst_addr;

    dst_addr.nxd_ip_version = NX_IP_VERSION_V6;
    dst_addr.nxd_ip_address.v6[0] = htonl(0x20010db8u);
    dst_addr.nxd_ip_address.v6[1] = htonl(0xf101u);
    dst_addr.nxd_ip_address.v6[2] = htonl(0x0u);
    dst_addr.nxd_ip_address.v6[3] = htonl(0x0101u);

    NX_PARAMETER_NOT_USED(thread_input);

    status = (INT)nx_ip_status_check(&bsd_tcp_ip, NX_IP_INITIALIZE_DONE,
                                     &actual_status, NX_IP_PERIODIC_RATE);
    if (status != NX_SUCCESS)
        return;
    tx_thread_sleep(10 * NX_IP_PERIODIC_RATE);

    memset(&local_addr6, 0, sizeof(local_addr6));
    local_addr6.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    local_addr6.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    local_addr6.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    local_addr6.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    local_addr6.sin6_port = htons(CLIENT_PORT);
    local_addr6.sin6_family = AF_INET6;

    memset(&echo_serv_addr6, 0, sizeof(echo_serv_addr6));
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    echo_serv_addr6.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    echo_serv_addr6.sin6_port = htons(SERVER_PORT);
    echo_serv_addr6.sin6_family = AF_INET6;

    while (1) {
        sock_tcp_client = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (sock_tcp_client == -1) {
            aicx_info(0x0fb311, "", "[NET] BSD TCP client socket create fail!\n");
            return;
        }

        aicx_info(0x0bcd2b, "u", "[NET] TCP client socket created ok %u\n",
                  (ULONG)sock_tcp_client);
        status = bind(sock_tcp_client, (struct sockaddr *)&local_addr6,
                      sizeof(struct sockaddr_in6));
        if (status != NX_SUCCESS) {
            aicx_info(0x0a876a, "x", "[IPV6] ipsec tcp bind fail 0x%x\n", status);
            return;
        }
#ifdef IPSEC_AES_MD5
        status = _nx_ipsec_create_egress_sa(&bsd_tcp_ip, &sa_1,
                                            NX_IPSEC_TRANSPORT_MODE,
                                            NX_IPSEC_ESP_PACKET,
                                            NX_CRYPTO_ENCRYPTION_AES_CBC,
                                            NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                            encrypt_key_cbc_128_encrypt,
                                            NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                            authentication_key_1,
                                            sizeof(authentication_key_1) << 3, 1,
                                            0, &dst_addr, CLIENT_PORT,
                                            SERVER_PORT);
        status = _nx_ipsec_create_egress_sa(&bsd_tcp_ip, &sa_2,
                                            NX_IPSEC_TRANSPORT_MODE,
                                            NX_IPSEC_ESP_PACKET,
                                            NX_CRYPTO_ENCRYPTION_AES_CBC,
                                            NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                            encrypt_key_cbc_128_encrypt,
                                            NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                            authentication_key_1,
                                            sizeof(authentication_key_1) << 3, 2,
                                            0, &dst_addr, SERVER_PORT,
                                            CLIENT_PORT);

#elif defined(IPSEC_NONE_MD5)
        status = _nx_ipsec_create_egress_sa(&bsd_tcp_ip, &sa_1,
                                            NX_IPSEC_TRANSPORT_MODE,
                                            NX_IPSEC_ESP_PACKET,
                                            NX_CRYPTO_ENCRYPTION_NULL,
                                            NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                            NULL,
                                            NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                            authentication_key_1,
                                            sizeof(authentication_key_1) << 3, 1,
                                            0, &dst_addr, SERVER_PORT,
                                            CLIENT_PORT);
        status = _nx_ipsec_create_egress_sa(&bsd_tcp_ip, &sa_2,
                                            NX_IPSEC_TRANSPORT_MODE,
                                            NX_IPSEC_ESP_PACKET,
                                            NX_CRYPTO_ENCRYPTION_NULL,
                                            NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                            NULL,
                                            NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                            authentication_key_1,
                                            sizeof(authentication_key_1) << 3, 2,
                                            0, &dst_addr, CLIENT_PORT,
                                            SERVER_PORT);
#else
        status = _nx_ipsec_create_egress_sa(&bsd_tcp_ip, &sa_1,
                                            NX_IPSEC_TRANSPORT_MODE,
                                            NX_IPSEC_ESP_PACKET,
                                            NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                            NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                            encrypt_key_3des_cbc_encrypt,
                                            sizeof(encrypt_key_3des_cbc_encrypt) <<
                                            3, authentication_sha1_key_96,
                                            sizeof(authentication_sha1_key_96) << 3,
                                            1, 0, &dst_addr, SERVER_PORT,
                                            CLIENT_PORT);
        status = _nx_ipsec_create_egress_sa(&bsd_tcp_ip, &sa_2,
                                            NX_IPSEC_TRANSPORT_MODE,
                                            NX_IPSEC_ESP_PACKET,
                                            NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                            NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                            encrypt_key_3des_cbc_encrypt,
                                            sizeof(encrypt_key_3des_cbc_encrypt) <<
                                            3, authentication_sha1_key_96,
                                            sizeof(authentication_sha1_key_96) << 3,
                                            2, 0, &dst_addr, CLIENT_PORT,
                                            SERVER_PORT);
#endif

        status = connect(sock_tcp_client, (struct sockaddr *)&echo_serv_addr6,
                         sizeof(echo_serv_addr6));
        if (status != OK) {
        aicx_info(0x0272aa, "x", "[NET] BSD TCP client socket connect fail 0x%x!\n", status);
        status = soc_close(sock_tcp_client);
        return;
        }

        aicx_info(0x1bbc8d, "d", "[NET] TCP client socket %d connected\n", sock_tcp_client);

        length = sizeof(struct sockaddr_in6);
        status = getsockname(sock_tcp_client, (struct sockaddr *)&local_addr6,
                             &length);
        aicx_info(0x177c63, "uxxxx", "[NET] TCP client port %u client 0x%x 0x%x 0x%x 0x%x\n",
                  (ULONG)local_addr6.sin6_port,
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[0],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[1],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[2],
                  (UINT)local_addr6.sin6_addr._S6_un._S6_u32[3]);
        length = sizeof(struct sockaddr_in6);
        status = getpeername(sock_tcp_client, (struct sockaddr *)&remote_addr6,
                             &length);
        aicx_info(0x0052e5, "uxxxx", "[NET] TCP remote port %u remote 0x%x 0x%x 0x%x 0x%x\n",
                  (ULONG)remote_addr6.sin6_port,
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[0],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[1],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[2],
                  (UINT)remote_addr6.sin6_addr._S6_un._S6_u32[3]);

        while (1) {
            status = send(sock_tcp_client, "Hello", (sizeof("Hello")), 0);
            if (status == ERROR)
                aicx_info(0x03f106, "d", "[NET] tcp client socket %d send fail!\n",
                          sock_tcp_client);
            else
                aicx_info(0x0abee3, "d", "[NET] tcp client socket %d send hello!\n",
                          sock_tcp_client);
READ_AGAIN:
            status = recv(sock_tcp_client, (VOID *)tcp_client_recv_buffer,
                          100, 0);
            if (status < 0) {
                if (errno == EAGAIN)
                    goto READ_AGAIN;
                aicx_info(0x17a225, "dxx", "[NET] connect error on recv socket %d 0x%x 0x%x\n",
                          sock_tcp_client, status, errno);
                break;
            } else {
                aicx_info(0x0aeac9, "dds", "[NET] client %d recv %d bytes: %s\n",
                          sock_tcp_client, status, tcp_client_recv_buffer);
            }
            aos_sleep(1000);
        }

        status = soc_close(sock_tcp_client);
        if (status != ERROR)
            aicx_info(0x0d5509, "d", "[NET] client socket %d closed!\n", sock_tcp_client);
        else
            aicx_info(0x1c0d82, "d", "[NET] client socket %d close fail!\n", sock_tcp_client);
    }
}

int tx_ipv6_tcp_ipsec_application_define(void)
{
    UINT status;

    tcp_thread_server = aos_create_thread("ipsec tcp test server",
                                          tcp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 8,
                                          AOS_AUTO_START);
    if (tcp_thread_server == AOS_INVALID_THREAD_ID)
    {
        tcp_error_counter++;
        aicx_info(0x1bb190, "", "[NET] Error in create ipsec tcp test server\n");
    }

    tcp_thread_client = aos_create_thread("ipsec tcp test client",
                                          tcp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 8,
                                          AOS_AUTO_START);
    if (tcp_thread_client == AOS_INVALID_THREAD_ID)
    {
        tcp_error_counter++;
        aicx_info(0x03cc9f, "", "[NET] Error in create ipsec tcp test client\n");
    }

    nx_system_initialize();

    status = nx_packet_pool_create(&bsd_tcp_pool, "BSD TCP Ipsec Packet Pool",
                                   256, tcp_packet_pool, 16384);
    if (status) {
        tcp_error_counter++;
        aicx_info(0x188b1a, "", "[NET] Error in creating tcp packet pool!\n");
    }

    status = nx_ip_create(&bsd_tcp_ip, "BSD TCP Ipsec IP Instance",
                          IP_ADDRESS(1, 2, 3, 4),
                          0xFFFFFF00UL, &bsd_tcp_pool, _nx_ram_network_driver,
                          tcp_ip_instance_pool, 2048, 1);
    if (status) {
        tcp_error_counter++;
        aicx_info(0x0268b6, "", "[NET] Error in creating tcp IP Instance!\n");
    }

    status = nx_tcp_enable(&bsd_tcp_ip);
    if (status) {
        tcp_error_counter++;
        aicx_info(0x178b54, "", "[NET] Error in enable TCP!\n");
    }

    status = (UINT)bsd_initialize(&bsd_tcp_ip, &bsd_tcp_pool, tcp_bsd_pool,
                                  2048, 2);
    if (status) {
        tcp_error_counter++;
        aicx_info(0x007e89, "", "[NET] Error bsd_initialize!\n");
    }

    aicx_info(0x024543, "d", "[NET] IPsec tcp test error counter %d\n", tcp_error_counter);

    return AOS_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(tx_ipv6_tcp_ipsec_application_define);

