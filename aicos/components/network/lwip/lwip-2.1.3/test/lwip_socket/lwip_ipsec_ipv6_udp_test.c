
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x111f41
#define LOG_MODULE    AMOD_NET
#include "ipsec/ipsec.h"
#include "ipsec/config.h"
#include <sockets.h>
#include "lwip/tcpip.h"

#include <aic_ims_nic.h>
#include "aic_init.h"
#include "aic_osa.h"

#include <string.h>
#include <stdlib.h>
#include "aic_network_pcap.h"

//#define IPSEC_AES_MD5
//#define IPSEC_NONE_MD5

#define DEMO_STACK_SIZE (8 * 1024)
#define SERVER_PORT 87
#define CLIENT_PORT 77
#define UDP_SERVER_RCV_BUFFER_SIZE 4000
#define UDP_CLIENT_RCV_BUFFER_SIZE 4000


static aos_thread_id udp_thread_server;
static aos_thread_id udp_thread_client;
static struct netif udp_test_netif;

static u32 udp_error_counter;

static s8 udp_server_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static s8 udp_send_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static s8 udp_client_recv_buffer[UDP_CLIENT_RCV_BUFFER_SIZE];

static void udp_thread_server_entry(void *thread_input);
static void udp_thread_client_entry(void *thread_input);

#ifdef IPSEC_AES_MD5
static u8 encrypt_key_cbc_128_encrypt[] = {
    0x02, 0xCF, 0x1E, 0x1D, 0x97, 0x96, 0xAD, 0x33,
    0x95, 0xD8, 0x12, 0x67, 0x57, 0xB6, 0xFC, 0x7C,
};
static u8 authentication_key_1[] = {
    0xBF, 0x8D, 0x54, 0x47, 0x0B, 0x5F, 0x36, 0x5F,
    0x14, 0xF8, 0x4C, 0x0E, 0x04, 0x6D, 0xE4, 0x05,
    0x3D, 0xE7, 0xA0, 0x5D, 0xB8, 0x47, 0xCD, 0x2C,
    0x98, 0x44, 0x63, 0x20, 0x82, 0xF5, 0x88, 0x60,
    0x2E, 0xA3, 0x88, 0x5D, 0xEF, 0x9C, 0xA2, 0x1B,
    0x8B, 0x46, 0xAA, 0x7C, 0xB9, 0x58, 0x89, 0x38,
};
#elif defined(IPSEC_NONE_MD5)
static u8 authentication_key_1[] = {
    0xBF, 0x8D, 0x54, 0x47, 0x0B, 0x5F, 0x36, 0x5F,
    0x14, 0xF8, 0x4C, 0x0E, 0x04, 0x6D, 0xE4, 0x05,
    0x3D, 0xE7, 0xA0, 0x5D, 0xB8, 0x47, 0xCD, 0x2C,
    0x98, 0x44, 0x63, 0x20, 0x82, 0xF5, 0x88, 0x60,
    0x2E, 0xA3, 0x88, 0x5D, 0xEF, 0x9C, 0xA2, 0x1B,
    0x8B, 0x46, 0xAA, 0x7C, 0xB9, 0x58, 0x89, 0x38,
};
#else
static u8 encrypt_key_3des_cbc_encrypt[] = {
    0x8f, 0x4f, 0x7a, 0xab, 0x25, 0x04, 0x37, 0x20,
    0xf4, 0xfb, 0xae, 0x01, 0xae, 0xdf, 0x07, 0x1c,
    0x68, 0xa2, 0x83, 0x68, 0x9b, 0x08, 0xad, 0x20,
};

static u8 authentication_sha1_key_96[] = {
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

#if 0
static u32 local_ip[4] = {0x0, 0x0, 0x0, 0x1000000};
static u32 remote_ip[4] = {0x0, 0x0, 0x0, 0x1000000};
static u32 netmask[4] = {0x0, 0x0, 0x0, 0x0};
#else
static u32 local_ip[4] = {0xb80d0120, 0x01f10000, 0x0, 0x1010000};
static u32 remote_ip[4] = {0xb80d0120, 0x01f10000, 0x0, 0x1010000};
static u32 netmask[4] = {0x0, 0x0, 0x0, 0x0};
#endif

static spd_entry *sp_s_1 = NULL, *sp_s_2 = NULL;
static spd_entry *sp_c_1 = NULL, *sp_c_2 = NULL;
static sad_entry sa1, sa2;
extern db_set_netif* db;

static void udp_thread_server_entry(void *thread_input)
{
    struct sockaddr_in6 server_addr, client_addr;
    int status, sock_udp_server, addr_len;
    ip_addr_t dst_addr;
    sad_entry *sa_1 = NULL, *sa_2 = NULL;

    dst_addr.type = IPV6;
    dst_addr.u_addr.ip6.addr[0] = htonl(0x20010db8);
    dst_addr.u_addr.ip6.addr[1] = htonl(0x0000f101);
    dst_addr.u_addr.ip6.addr[2] = htonl(0x00000000);
    dst_addr.u_addr.ip6.addr[3] = htonl(0x00000101);

    sock_udp_server = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_server == -1) {
        aicx_info(0x02128a, "d", "[NET udp Server socket create fail, errno %d\n", errno);
        return;
    }

    aicx_info(0x1432ec, "u", "[NET] udp Server socket created %u\n",
               (u32)sock_udp_server);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_addr.un.u32_addr[0] = htonl(0x20010db8);
    server_addr.sin6_addr.un.u32_addr[1] = htonl(0x0000f101);
    server_addr.sin6_addr.un.u32_addr[2] = htonl(0x00000000);
    server_addr.sin6_addr.un.u32_addr[3] = htonl(0x00000101);
    server_addr.sin6_port = htons(SERVER_PORT);
    server_addr.sin6_family = AF_INET6;

    status = bind(sock_udp_server, (struct sockaddr *)&server_addr,
                       sizeof(server_addr));
    if (status < 0) {
        aicx_info(0x1be98e, "", "[NET] ipv6 server socket bind fail\n");
        return;
    }

    sp_s_1 = ipsec_spd_add(local_ip, NULL, remote_ip, NULL, IPSEC_PROTO_UDP,
                           IPSEC_HTONS(SERVER_PORT), IPSEC_HTONS(CLIENT_PORT),
                           POLICY_APPLY, &db->outbound_spd, IPV6);
    sp_s_2 = ipsec_spd_add(remote_ip, NULL, local_ip, NULL, IPSEC_PROTO_UDP,
                           IPSEC_HTONS(CLIENT_PORT), IPSEC_HTONS(SERVER_PORT),
                           POLICY_APPLY, &db->inbound_spd, IPV6);
    sa_1 = ipsec_sad_add(&sa1, &db->outbound_sad);
    sa_2 = ipsec_sad_add(&sa2, &db->inbound_sad);
    ipsec_spd_add_sa(sp_s_1, sa_1);
    ipsec_spd_add_sa(sp_s_2, sa_2);


    while (1) {
        addr_len = sizeof(struct sockaddr_in6);
        status = recvfrom(sock_udp_server, (void *)udp_server_recv_buffer,
                          UDP_SERVER_RCV_BUFFER_SIZE, 0,
                          (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);
        if (status < 0) {
            if (errno == EAGAIN) {
                aicx_info(0x1baa83, "", "[NET] no error received, try again later\n");
                aos_sleep(500);
                continue;
            } else if (errno == ENOTCONN) {
                aicx_info(0x05d2d7, "", "[NET] connection is broken, close the socket.\n");
                break;
            } else {
                aicx_info(0x0fbf3b, "x", "[NET] error on client socket recv data 0x%x\n",
                          errno);
                break;
            }
        }

        if (status == UDP_SERVER_RCV_BUFFER_SIZE)
            status--;

        udp_server_recv_buffer[status] = 0;
        aicx_info(0x16f111, "dud", "[NET] udp server recv %d %u bytes: %d\n",
                  sock_udp_server, (u32)status, status);

        memset(udp_send_buffer, 'b', UDP_SERVER_RCV_BUFFER_SIZE);
        status = sendto(sock_udp_server, udp_send_buffer,
                             UDP_SERVER_RCV_BUFFER_SIZE, 0,
                             (struct sockaddr *)&client_addr,
                             sizeof(client_addr));
        if (status < 0)
            aicx_info(0x10ca4a, "d", "[NET] udp server sending to client fail s:%d\n",
                      sock_udp_server);
        else
            aicx_info(0x0b91ae, "dd", "[NET] udp server sending to client:%d: %d\n",
                      sock_udp_server, status);
        aos_sleep(1000);
    }

    status = closesocket(sock_udp_server);
    if (status >= 0)
        aicx_info(0x0aba03, "d", "[NET] socket close conn to client on %d\n",
                  sock_udp_server);
    else
        aicx_info(0x0b2735, "d", "[NET] socket close conn to client on %d fail\n",
                  sock_udp_server);
}

static void udp_thread_client_entry(void *thread_input)
{
    int status, sock_udp_client, length;
    struct sockaddr_in6 echo_serv_addr6, local_addr6;
    ip_addr_t dst_addr;
    sad_entry *sa_1 = NULL, *sa_2 = NULL;

    dst_addr.type = IPV6;
    dst_addr.u_addr.ip6.addr[0] = htonl(0x20010db8);
    dst_addr.u_addr.ip6.addr[1] = htonl(0x0000f101);
    dst_addr.u_addr.ip6.addr[2] = htonl(0x00000000);
    dst_addr.u_addr.ip6.addr[3] = htonl(0x00000101);

    memset(&local_addr6, 0, sizeof(local_addr6));
    local_addr6.sin6_addr.un.u32_addr[0] = htonl(0x20010db8);
    local_addr6.sin6_addr.un.u32_addr[1] = htonl(0x0000f101);
    local_addr6.sin6_addr.un.u32_addr[2] = htonl(0x00000000);
    local_addr6.sin6_addr.un.u32_addr[3] = htonl(0x00000101);
    local_addr6.sin6_port = htons(CLIENT_PORT);
    local_addr6.sin6_family = AF_INET6;

    memset(&echo_serv_addr6, 0, sizeof(echo_serv_addr6));
    echo_serv_addr6.sin6_addr.un.u32_addr[0] = htonl(0x20010db8);
    echo_serv_addr6.sin6_addr.un.u32_addr[1] = htonl(0x0000f101);
    echo_serv_addr6.sin6_addr.un.u32_addr[2] = htonl(0x00000000);
    echo_serv_addr6.sin6_addr.un.u32_addr[3] = htonl(0x00000101);
    echo_serv_addr6.sin6_port = htons(SERVER_PORT);
    echo_serv_addr6.sin6_family = AF_INET6;

    sp_c_1 = ipsec_spd_add(local_ip, NULL, remote_ip, NULL, IPSEC_PROTO_UDP,
                           IPSEC_HTONS(CLIENT_PORT), IPSEC_HTONS(SERVER_PORT),
                           POLICY_APPLY, &db->outbound_spd, IPV6);
    sp_c_2 = ipsec_spd_add(remote_ip, NULL, local_ip, NULL, IPSEC_PROTO_UDP,
                           IPSEC_HTONS(SERVER_PORT), IPSEC_HTONS(CLIENT_PORT),
                           POLICY_APPLY, &db->inbound_spd, IPV6);
    sa_1 = ipsec_sad_add(&sa2, &db->outbound_sad);
    sa_2 = ipsec_sad_add(&sa1, &db->inbound_sad);
    status = ipsec_spd_add_sa(sp_c_1, sa_1);
    status = ipsec_spd_add_sa(sp_c_2, sa_2);
    if (status) {
        aicx_info(0x1a089a, "x", "[NET] Error in TCP create_egress_sa status = 0x%x\n!",
                  status);
        return;
    }

    while (1) {
        sock_udp_client = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_client == -1) {
            aicx_info(0x0eca15, "", "[NET] udp client socket create fail!\n");
            return;
        }

        status = bind(sock_udp_client, (struct sockaddr *)&local_addr6,
                      sizeof(struct sockaddr_in6));
        if (status < 0) {
            aicx_info(0x17d7c5, "x", "[IPV6] ipsec udp bind fail 0x%x\n", status);
            return;
        }

        aicx_info(0x1d9f5e, "u", "[NET] udp client socket created ok %u\n",
                  (u32)sock_udp_client);

        length = sizeof(struct sockaddr_in6);
        //snprintf(udp_send_buffer, UDP_SERVER_RCV_BUFFER_SIZE,
                 //"hello world 0x%x\n", NX_RAND());

        memset(udp_send_buffer, 'a', UDP_SERVER_RCV_BUFFER_SIZE);
        status = sendto(sock_udp_client, udp_send_buffer,
                             UDP_SERVER_RCV_BUFFER_SIZE, 0,
                             (struct sockaddr *)&echo_serv_addr6,
                             sizeof(echo_serv_addr6));
        if (status < 0)
            aicx_info(0x0da511, "d", "[NET] udp client socket %d send fail!\n",
                      sock_udp_client);
        else
            aicx_info(0x1220bd, "dd", "[NET] udp client socket %d send: %d!\n",
                      sock_udp_client, status);

READ_AGAIN:
        status = recvfrom(sock_udp_client, (void *)udp_client_recv_buffer,
                               UDP_CLIENT_RCV_BUFFER_SIZE, 0,
                               (struct sockaddr *)&echo_serv_addr6, (socklen_t *)&length);
        if (status < 0) {
            if (errno == EAGAIN) {
                aos_sleep(500);
                goto READ_AGAIN;
            }
            aicx_info(0x068028, "dxx", "[NET] connection error on recv socket %d 0x%x 0x%x\n",
                      sock_udp_client, status, errno);
        } else {
            aicx_info(0x07a102, "ddd", "[NET] udp client %d recv %d bytes: %d\n",
                      sock_udp_client, status, status);
        }

        status = closesocket(sock_udp_client);
        if (status >= 0)
            aicx_info(0x0d5509, "d", "[NET] client socket %d closed!\n", sock_udp_client);
        else
            aicx_info(0x173836, "dd", "[NET] client socket %d close fail! errno %d\n", sock_udp_client, errno);

        aos_sleep(1000);
    }
}

int tx_ipv6_udp_ipsec_application_define(void)
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
#ifdef IPSEC_AES_MD5
    ipsec_sa_config(&sa1, remote_ip, netmask, 1, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_AES_128_CBC,
                    encrypt_key_cbc_128_encrypt,
                    sizeof(encrypt_key_cbc_128_encrypt), IPSEC_HMAC_MD5,
                    authentication_key_1, sizeof(authentication_key_1), IPV6);
    ipsec_sa_config(&sa2, local_ip, netmask, 2, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_AES_128_CBC,
                    encrypt_key_cbc_128_encrypt,
                    sizeof(encrypt_key_cbc_128_encrypt), IPSEC_HMAC_MD5,
                    authentication_key_1, sizeof(authentication_key_1), IPV6);
#elif defined(IPSEC_NONE_MD5)
    ipsec_sa_config(&sa1, remote_ip, netmask, 1, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_NULL, NULL, 0, IPSEC_HMAC_MD5,
                    authentication_key_1, sizeof(authentication_key_1), IPV6);
    ipsec_sa_config(&sa2, local_ip, netmask, 2, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_NULL, NULL, 0, IPSEC_HMAC_MD5,
                    authentication_key_1, sizeof(authentication_key_1), IPV6);
#else
    ipsec_sa_config(&sa1, remote_ip, netmask, 1, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_3DES,
                    encrypt_key_3des_cbc_encrypt,
                    sizeof(encrypt_key_3des_cbc_encrypt), IPSEC_HMAC_SHA1,
                    authentication_sha1_key_96,
                    sizeof(authentication_sha1_key_96), IPV6);
    ipsec_sa_config(&sa2, local_ip, netmask, 2, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_3DES,
                    encrypt_key_3des_cbc_encrypt,
                    sizeof(encrypt_key_3des_cbc_encrypt), IPSEC_HMAC_SHA1,
                    authentication_sha1_key_96,
                    sizeof(authentication_sha1_key_96), IPV6);
#endif

    udp_thread_server = aos_create_thread("ipsec udp test server",
                                          udp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 8,
                                          AOS_AUTO_START);
    if (udp_thread_server == AOS_INVALID_THREAD_ID)
    {
        udp_error_counter++;
        aicx_info(0x060035, "", "[NET] Error in create ipsec udp test server\n");
    }

    udp_thread_client = aos_create_thread("ipsec udp test client",
                                          udp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE, 0, NULL, 8,
                                          AOS_AUTO_START);
    if (udp_thread_client == AOS_INVALID_THREAD_ID)
    {
        udp_error_counter++;
        aicx_info(0x058035, "", "[NET] Error in create ipsec udp test client\n");
    }

    aicx_info(0x0732af, "d", "[NET] IPsec udp test error counter %d\n", udp_error_counter);

    return AOS_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(tx_ipv6_udp_ipsec_application_define);

