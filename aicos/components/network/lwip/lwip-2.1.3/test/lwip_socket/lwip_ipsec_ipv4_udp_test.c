
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19606a
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

#define IPSEC_3DES_MD5
#define DEMO_STACK_SIZE (4 * 1024)
#define SERVER_PORT 87
#define CLIENT_PORT 77
#define UDP_SERVER_RCV_BUFFER_SIZE 1600

static aos_thread_id udp_thread_server;
static aos_thread_id udp_thread_client;
static struct netif udp_test_netif;

static u32 udp_error_counter;

static u8 udp_server_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static u8 udp_client_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];

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

static const ip4_addr_t test_local_ip = {0x7f000001};
static const ip4_addr_t test_remote_ip = {0x7f000001};
static const ip4_addr_t test_netmask = {0xFFFFFF00};

static u32 local_ip = 0x100007f;
static u32 remote_ip = 0x100007f;
static u32 netmask = 0x0;


static spd_entry *sp_s_1 = NULL, *sp_s_2 =NULL;
static spd_entry *sp_c_1 = NULL, *sp_c_2 =NULL;
static sad_entry sa1, sa2;
extern db_set_netif* db;

static VOID udp_thread_server_entry(void *thread_input)
{
    ULONG actual_status;
    ip_addr_t dst_addr;
    sad_entry *sa_1 = NULL, *sa_2 = NULL;
    struct sockaddr_in server_addr, fromAddr;
    INT status, sock_udp_server, addrlen1;

    dst_addr.type = IPV4;
    dst_addr.u_addr.ip4.addr = htonl(test_local_ip.addr);

    sock_udp_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_server == -1) {
        aicx_info(0x042b9c, "", "[NET] BSD UDP Server socket create fail\n");
        return;
    }

    aicx_info(0x15a684, "u", "[NET] BSD UDP Server socket created %u\n",
              (ULONG)sock_udp_server);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(test_local_ip.addr);
    server_addr.sin_port = htons(SERVER_PORT);

    status = bind(sock_udp_server, (struct sockaddr *)&server_addr,
                  sizeof(server_addr));
    if (status < 0) {
        aicx_info(0x12ab9e, "", "[NET] server socket bind fail\n");
        return;
    }

    sp_s_1 = ipsec_spd_add(&local_ip, NULL, &remote_ip, NULL, IPSEC_PROTO_UDP,
                           IPSEC_HTONS(SERVER_PORT), IPSEC_HTONS(CLIENT_PORT),
                           POLICY_APPLY, &db->outbound_spd, IPV4);
    sp_s_2 = ipsec_spd_add(&remote_ip, NULL, &local_ip, NULL, IPSEC_PROTO_UDP,
                           IPSEC_HTONS(CLIENT_PORT), IPSEC_HTONS(SERVER_PORT),
                           POLICY_APPLY, &db->inbound_spd, IPV4);
    sa_1 = ipsec_sad_add(&sa1, &db->outbound_sad);
    sa_2 = ipsec_sad_add(&sa2, &db->inbound_sad);
    ipsec_spd_add_sa(sp_s_1, sa_1);
    ipsec_spd_add_sa(sp_s_2, sa_2);

    if (status) {
        aicx_info(0x048206, "x", "[NET] Error in UDP create_ingress_sa status = 0x%x\n!",
                  status);
        return;
    }

    aicx_info(0x02ccbf, "xx", "[SERVER] sa_1 0x%x sa_2 0x%x\n", sa_1, sa_2);

    while (1) {
        addrlen1 = sizeof(struct sockaddr_in);

        status = recvfrom(sock_udp_server, (void *)udp_server_recv_buffer, 100, 0,
                          (struct sockaddr *)&fromAddr, &addrlen1);
        if (status < 0) {
            aicx_info(0x15eb1b, "d", "[NET] Error: Server Socket receive errno %d\n", errno);
        } else {
            aicx_info(0x16ade8, "d", "[NET] Server recv from Client: %d\n", status);

            status = sendto(sock_udp_server, udp_server_recv_buffer, status, 0,
                            (struct sockaddr *)&fromAddr, sizeof(fromAddr));

            if (status < 0)
                aicx_info(0x1cfd21, "", "[NET] Error: BSD Server Socket echo\n");
        }

        aos_sleep(1000);
    }
}

static VOID udp_thread_client_entry(void *thread_input)
{
    ULONG actual_status;
    sad_entry *sa_1 = NULL, *sa_2 = NULL;
    INT status, sock_udp_client, length, addr_len;
    struct sockaddr_in echo_serv_addr, local_addr, from_addr;
    ip_addr_t dst_addr;

    dst_addr.type = IPV4;
    dst_addr.u_addr.ip4.addr = htonl(test_local_ip.addr);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(test_local_ip.addr);
    local_addr.sin_port = htons(CLIENT_PORT);

    memset(&echo_serv_addr, 0, sizeof(echo_serv_addr));
    echo_serv_addr.sin_family = AF_INET;
    echo_serv_addr.sin_addr.s_addr = htonl(test_remote_ip.addr);
    echo_serv_addr.sin_port = htons(SERVER_PORT);

    sock_udp_client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_client == -1) {
        aicx_info(0x12d618, "", "[NET] BSD udp client socket create fail!\n");
        return;
    }

    aicx_info(0x1d9f5e, "u", "[NET] udp client socket created ok %u\n",
              (ULONG)sock_udp_client);

    status = bind(sock_udp_client, (struct sockaddr *)&local_addr,
                  sizeof(struct sockaddr_in));
    if (status < 0) {
        aicx_info(0x0b1bd9, "x", "ipsec udp bind fail 0x%x\n", status);
        return;
    }

    sp_c_1 = ipsec_spd_add(&local_ip, NULL, &remote_ip, NULL, IPSEC_PROTO_UDP,
                           IPSEC_HTONS(CLIENT_PORT), IPSEC_HTONS(SERVER_PORT),
                           POLICY_APPLY, &db->outbound_spd, IPV4);
    sp_c_2 = ipsec_spd_add(&remote_ip, NULL, &local_ip, NULL, IPSEC_PROTO_UDP,
                           IPSEC_HTONS(SERVER_PORT), IPSEC_HTONS(CLIENT_PORT),
                           POLICY_APPLY, &db->inbound_spd, IPV4);
    sa_1 = ipsec_sad_add(&sa2, &db->outbound_sad);
    sa_2 = ipsec_sad_add(&sa1, &db->inbound_sad);
    status = ipsec_spd_add_sa(sp_c_1, sa_1);
    status = ipsec_spd_add_sa(sp_c_2, sa_2);
    if (status) {
        aicx_info(0x066d6c, "x", "[NET] Error in udp create_egress_sa status = 0x%x\n!",
                  status);
        return;
    }

    aicx_info(0x143ad6, "xx", "[CLIENT] sa_1 0x%x sa_2 0x%x\n", sa_1, sa_2);

    while (1) {
        aicx_info(0x02b41c, "dd", "[NET] client sock %d sending packet to server %d\n",
                  sock_udp_client, sizeof("Hello"));

        memset(udp_client_recv_buffer, 'a', UDP_SERVER_RCV_BUFFER_SIZE);
        status = sendto(sock_udp_client, udp_client_recv_buffer,
                        UDP_SERVER_RCV_BUFFER_SIZE, 0,
                        (struct sockaddr *)&echo_serv_addr, sizeof(echo_serv_addr));
        if (status < 0)
            aicx_info(0x0da511, "d", "[NET] udp client socket %d send fail!\n",
                      sock_udp_client);
        else
            aicx_info(0x1496cb, "d", "[NET] udp client socket %d send hello!\n",
                      sock_udp_client);

READ_AGAIN:
        addr_len = sizeof(struct sockaddr_in);
        status = recvfrom(sock_udp_client, (void *)udp_client_recv_buffer, 100, 0,
                          (struct sockaddr *)&from_addr, &addr_len);
        if (status < 0) {
            if (errno == EAGAIN) {
                aos_sleep(1000);
                goto READ_AGAIN;
            }

            aicx_info(0x17a225, "dxx", "[NET] connect error on recv socket %d 0x%x 0x%x\n",
                      sock_udp_client, status, errno);
            break;
        } else {
            aicx_info(0x0aeac9, "dds", "[NET] client %d recv %d bytes: %s\n",
                      sock_udp_client, status, udp_client_recv_buffer);
        }

        aos_sleep(1000);
    }

    status = closesocket(sock_udp_client);
    if (status >= 0)
        aicx_info(0x1a0d59, "d", "[NET] client socket %d closesocketd!\n", sock_udp_client);
    else
        aicx_info(0x1b045c, "d", "[NET] client socket %d closesocket fail!\n", sock_udp_client);

    ipsec_sad_del(sa_1, &db->outbound_sad);
    ipsec_sad_del(sa_2, &db->inbound_sad);
    ipsec_spd_del(sp_c_1, &db->outbound_spd);
    ipsec_spd_del(sp_c_2, &db->inbound_spd);

}

int tx_ipv4_udp_ipsec_application_define(void)
{
    UINT status = 0;
    struct netif *netif;
#if LWIP_HAVE_LOOPIF == 0
    ip4_addr_t test_local_ip = {0x0100007f};
    ip4_addr_t test_local_gw = {0x0100007f};
    ip4_addr_t test_netmask = {0x00FFFFFF};

    IP4_ADDR(&test_local_ip, 127,0,0,1);
    IP4_ADDR(&test_netmask, 255,255,255,0);
    IP4_ADDR(&test_local_gw, 127,0,0,1);

    aos_sleep(2000);
    netif_add(&udp_test_netif, &test_local_ip, &test_netmask, &test_local_gw,
              NULL, lwip_ims_init_netif, NULL);

#if LWIP_IPV6
    IP_ADDR6_HOST(udp_test_netif.ip6_addr, 0, 0, 0, 0x01);
    udp_test_netif.ip6_addr_state[0] = IP6_ADDR_VALID;
#endif /* LWIP_IPV6 */

    netif_set_default(&udp_test_netif);
    netif_set_up(&udp_test_netif);
#endif

    tcpip_init(NULL, NULL);
#if LWIP_NETCONN_SEM_PER_THREAD
    lwip_socket_thread_init();
#endif

    aic_network_parse_cp_at("cpcapon 1500");

#ifdef IPSEC_AES_MD5
    ipsec_sa_config(&sa1, &remote_ip, &netmask, 1, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_AES_128_CBC,
                    encrypt_key_cbc_128_encrypt,
                    sizeof(encrypt_key_cbc_128_encrypt), IPSEC_HMAC_MD5,
                    authentication_key_1, sizeof(authentication_key_1), IPV4);
    ipsec_sa_config(&sa2, &local_ip, &netmask, 2, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_AES_128_CBC,
                    encrypt_key_cbc_128_encrypt,
                    sizeof(encrypt_key_cbc_128_encrypt), IPSEC_HMAC_MD5,
                    authentication_key_1, sizeof(authentication_key_1), IPV4);
#elif defined(IPSEC_NONE_MD5)
    ipsec_sa_config(&sa1, &remote_ip, &netmask, 1, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_NULL, NULL, 0, IPSEC_HMAC_MD5,
                    authentication_key_1, sizeof(authentication_key_1), IPV4);
    ipsec_sa_config(&sa2, &local_ip, &netmask, 2, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_NULL, NULL, 0, IPSEC_HMAC_MD5,
                    authentication_key_1, sizeof(authentication_key_1), IPV4);
#else
    ipsec_sa_config(&sa1, &remote_ip, &netmask, 1, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_3DES,
                    encrypt_key_3des_cbc_encrypt,
                    sizeof(encrypt_key_3des_cbc_encrypt), IPSEC_HMAC_SHA1,
                    authentication_sha1_key_96,
                    sizeof(authentication_sha1_key_96), IPV4);
    ipsec_sa_config(&sa2, &local_ip, &netmask, 2, IPSEC_PROTO_ESP,
                    IPSEC_TRANSPORT, 0, IPSEC_3DES,
                    encrypt_key_3des_cbc_encrypt,
                    sizeof(encrypt_key_3des_cbc_encrypt), IPSEC_HMAC_SHA1,
                    authentication_sha1_key_96,
                    sizeof(authentication_sha1_key_96), IPV4);
#endif

    udp_thread_server = aos_create_thread("ipsec udp test server",
                                          udp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE,
                                          0, NULL, 8,
                                          AOS_AUTO_START);
    if (udp_thread_server == AOS_INVALID_THREAD_ID)
    {
        udp_error_counter++;
        aicx_info(0x060035, "", "[NET] Error in create ipsec udp test server\n");
    }

    udp_thread_client = aos_create_thread("ipsec udp test client",
                                          udp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE,
                                          0, NULL, 8,
                                          AOS_AUTO_START);
    if (udp_thread_client == AOS_INVALID_THREAD_ID)
    {
        udp_error_counter++;
        aicx_info(0x058035, "", "[NET] Error in create ipsec udp test client\n");
    }

    aicx_info(0x0732af, "d", "[NET] IPsec udp test error counter %d\n", udp_error_counter);
    return status;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(tx_ipv4_udp_ipsec_application_define);

