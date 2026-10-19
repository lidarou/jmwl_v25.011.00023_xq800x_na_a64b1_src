
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0c3209
#define LOG_MODULE    AMOD_NET
#include "ipsec/ipsec.h"
#include "ipsec/config.h"
#include "ipsec/ipsecdev.h"
#include <sockets.h>
#include "lwip/tcpip.h"

#include <aic_ims_nic.h>
#include "aic_init.h"
#include "aic_osa.h"

#include <string.h>
#include <stdlib.h>
#include "aic_network_pcap.h"

#define IPSEC_AES_MD5
#define DEMO_STACK_SIZE (4 * 1024)
#define SERVER_PORT 87
#define CLIENT_PORT 77
#define TCP_SERVER_RCV_BUFFER_SIZE 100

static aos_thread_id tcp_thread_server;
static aos_thread_id tcp_thread_client;
//static struct netif tcp_test_netif;

static int max_fd;

static u32 tcp_error_counter;
static fd_set master_list, read_ready;

static u8 tcp_server_recv_buffer[TCP_SERVER_RCV_BUFFER_SIZE];
static u8 tcp_client_recv_buffer[100];

static void tcp_thread_server_entry(void *thread_input);
static void tcp_thread_client_entry(void *thread_input);

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
//static const ip4_addr_t test_netmask = {0xFFFFFF00};

static u32 local_ip = 0x100007f;
static u32 remote_ip = 0x100007f;
static u32 netmask = 0x0;


//static spd_entry *sp_s_1 = NULL, *sp_s_2 = NULL;
//static spd_entry *sp_c_1 = NULL, *sp_c_2 = NULL;
//static sad_entry sa1, sa2;
extern db_set_netif* db;

void config_channel_paras(struct ipsec_channel *channel, u32 *src,
                    u32 *src_net, u32 *dest, u32 *dest_net,
                    u16 src_port, u16 dest_port, u32 spi_c, u32 spi_s,
                    u8 protocol, u8 mode, u32 seq,  u8 enc_alg, u8 *enckey,
                    u16 enckey_size, u8 auth_alg, u8 *authkey, u16 authkey_size,
                    u8 type)
{
    if (!channel)
        return;

    memset(channel, 0, sizeof(struct ipsec_channel));
    aicx_info(0x0095a8, "p", "[tcpip] config channel %p paras", channel);
    channel->src = src;
    channel->src_net = src_net;
    channel->dest = dest;
    channel->dest_net = dest_net;
    channel->src_port = src_port;
    channel->dest_port = dest_port;
    channel->spi_c = spi_c;
    channel->spi_s = spi_s;
    channel->protocol = IPSEC_PROTO_TCP;
    channel->mode = IPSEC_TRANSPORT;
    channel->seq = seq;
    channel->enc_alg = enc_alg;
    channel->enckey = enckey;
    channel->enckey_size = enckey_size;
    channel->auth_alg = auth_alg;
    channel->authkey = authkey;
    channel->authkey_size = authkey_size;
    channel->type = type;
}

static void tcp_thread_server_entry(void *thread_input)
{
    ip_addr_t dst_addr;
    sad_entry *sa_1 = NULL, *sa_2 = NULL;
    struct sockaddr_in server_addr, client_addr;
    int status, sock, sock_tcp_server, client_len, i, is_set;
    struct ipsec_channel channel;

    dst_addr.type = IPV4;
    dst_addr.u_addr.ip4.addr = htonl(test_local_ip.addr);

    sock_tcp_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_tcp_server == -1) {
        aicx_info(0x037b37, "", "[NET] BSD TCP Server socket create fail\n");
        return;
    }

    aicx_info(0x1ca13c, "u", "[NET] BSD TCP Server socket created %u\n",
              (u32)sock_tcp_server);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(test_local_ip.addr);
    server_addr.sin_port = htons(SERVER_PORT);

    status = bind(sock_tcp_server, (struct sockaddr *)&server_addr,
                  sizeof(server_addr));
    if (status < 0) {
        aicx_info(0x12ab9e, "", "[NET] server socket bind fail\n");
        return;
    }

#if 1
#ifdef IPSEC_AES_MD5
        config_channel_paras(&channel, &local_ip, &netmask, &remote_ip, &netmask,
                             SERVER_PORT, CLIENT_PORT, 1, 2,
                             IPSEC_PROTO_ESP, IPSEC_TRANSPORT, 0, IPSEC_AES_128_CBC,
                             encrypt_key_cbc_128_encrypt,
                             sizeof(encrypt_key_cbc_128_encrypt), IPSEC_HMAC_MD5,
                             authentication_key_1, sizeof(authentication_key_1), IPV4);
#elif defined(IPSEC_NONE_MD5)
        config_channel_paras(&channel, &local_ip, &netmask, &remote_ip, &netmask,
                             SERVER_PORT, CLIENT_PORT, 1, 2, IPSEC_PROTO_ESP,
                             IPSEC_TRANSPORT, 0, IPSEC_NULL, NULL, 0, IPSEC_HMAC_MD5,
                             authentication_key_1, sizeof(authentication_key_1), IPV4);
#else
        config_channel_paras(&channel, &local_ip, &netmask, &remote_ip, &netmask,
                             SERVER_PORT, CLIENT_PORT, 1, 2, IPSEC_PROTO_ESP,
                             IPSEC_TRANSPORT, 0, IPSEC_3DES,
                             encrypt_key_3des_cbc_encrypt,
                             sizeof(encrypt_key_3des_cbc_encrypt), IPSEC_HMAC_SHA1,
                             authentication_sha1_key_96,
                             sizeof(authentication_sha1_key_96), IPV4);
#endif
        status = CreateIpsecChannel(&channel);
#else
        sp_s_1 = ipsec_spd_add(&local_ip, &netmask, &remote_ip, &netmask, IPSEC_PROTO_TCP,
                               IPSEC_HTONS(SERVER_PORT), IPSEC_HTONS(CLIENT_PORT),
                               POLICY_APPLY, &db->outbound_spd, IPV4);
        sp_s_2 = ipsec_spd_add(&remote_ip, &netmask, &local_ip, &netmask, IPSEC_PROTO_TCP,
                               IPSEC_HTONS(CLIENT_PORT), IPSEC_HTONS(SERVER_PORT),
                               POLICY_APPLY, &db->inbound_spd, IPV4);
        sa_1 = ipsec_sad_add(&sa1, &db->outbound_sad);
        sa_2 = ipsec_sad_add(&sa2, &db->inbound_sad);
        ipsec_spd_add_sa(sp_s_1, sa_1);
        ipsec_spd_add_sa(sp_s_2, sa_2);
#endif

    if (status) {
        aicx_info(0x0e1399, "x", "[NET] Error in TCP create_ingress_sa status = 0x%x\n!",
                  status);
        return;
    }

    aicx_info(0x02ccbf, "xx", "[SERVER] sa_1 0x%x sa_2 0x%x\n", sa_1, sa_2);

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
        if (status <= 0) {
            aicx_info(0x1be7d0, "x", "[NET] select err 0x%x try again\n", status);
            continue;
        }

        is_set = FD_ISSET(sock_tcp_server, &read_ready);
        if (is_set) {
            client_len = sizeof(client_addr);
            sock = accept(sock_tcp_server, (struct sockaddr *)&client_addr,
                          (socklen_t *)&client_len);
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
            if ((i != sock_tcp_server) && (FD_ISSET(i, &master_list)) &&
                (FD_ISSET(i, &read_ready))) {
                    while (1) {
                        status = recv(i, (void *)tcp_server_recv_buffer,
                                      TCP_SERVER_RCV_BUFFER_SIZE, 0);
                        if (status < 0) {
                            if (errno == EAGAIN) {
                                aicx_info(0x05092d, "", "[NET] no error, try again later\n");
                                continue;
                            } else if (errno == ENOTCONN) {
                                aicx_info(0x0db8c0, "", "[NET] connection is broken.\n");
                                break;
                            } else {
                                aicx_info(0x036468, "dx", "[NET] err on c %d recv. 0x%x\n",
                                          i, errno);
                                break;
                            }
                        }

                        if (status == TCP_SERVER_RCV_BUFFER_SIZE)
                            status--;

                        tcp_server_recv_buffer[status] = 0;
                        aicx_info(0x1b0ea2, "dus", "[NET] tcp server recv %d %u bytes: %s\n",
                                  i, (u32)status, tcp_server_recv_buffer);

                        status = send(i, "IPv4 TCP Hello World\n",
                                      sizeof("IPv4 TCP Hello World\n"), 0);
                        if (status < 0)
                            aicx_info(0x0487cb, "d", "[NET] tcp server send to c fail s:%d\n", i);
                        else
                            aicx_info(0x162399, "d", "[NET] tcp server send msg to c:%d\n", i);

                        aos_sleep(1000);
                    }

                    status = closesocket(i);
                    if (status >= 0)
                        aicx_info(0x09b905, "d", "[NET] socket closesocket conn to client on %d\n", i);
                    else
                        aicx_info(0x0791dd, "d", "[NET] socket closesocket on %d fail\n", i);
            }
        }
    }
}

static void tcp_thread_client_entry(void *thread_input)
{
    sad_entry *sa_1 = NULL, *sa_2 = NULL;
    int status, sock_tcp_client, length;
    struct sockaddr_in echo_serv_addr, local_addr, remote_addr;
    ip_addr_t dst_addr;
    struct ipsec_channel channel;

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

    while (1) {
        sock_tcp_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock_tcp_client == -1) {
            aicx_info(0x0fb311, "", "[NET] BSD TCP client socket create fail!\n");
            return;
        }

        aicx_info(0x0bcd2b, "u", "[NET] TCP client socket created ok %u\n",
                  (u32)sock_tcp_client);

        status = bind(sock_tcp_client, (struct sockaddr *)&local_addr,
                      sizeof(struct sockaddr_in));
        if (status < 0) {
            aicx_info(0x170a10, "x", "ipsec tcp bind fail 0x%x\n", status);
            return;
        }

#if 1
#ifdef IPSEC_AES_MD5
        config_channel_paras(&channel, &local_ip, &netmask, &remote_ip, &netmask,
                             CLIENT_PORT, SERVER_PORT, 2, 1,
                             IPSEC_PROTO_ESP, IPSEC_TRANSPORT, 0, IPSEC_AES_128_CBC,
                             encrypt_key_cbc_128_encrypt,
                             sizeof(encrypt_key_cbc_128_encrypt), IPSEC_HMAC_MD5,
                             authentication_key_1, sizeof(authentication_key_1), IPV4);
#elif defined(IPSEC_NONE_MD5)
        config_channel_paras(&channel, &local_ip, &netmask, &remote_ip, &netmask,
                             CLIENT_PORT, SERVER_PORT, 2, 1, IPSEC_PROTO_ESP,
                             IPSEC_TRANSPORT, 0, IPSEC_NULL, NULL, 0, IPSEC_HMAC_MD5,
                             authentication_key_1, sizeof(authentication_key_1), IPV4);
#else
        config_channel_paras(&channel, &local_ip, &netmask, &remote_ip, &netmask,
                             CLIENT_PORT, SERVER_PORT, 2, 1, IPSEC_PROTO_ESP,
                             IPSEC_TRANSPORT, 0, IPSEC_3DES,
                             encrypt_key_3des_cbc_encrypt,
                             sizeof(encrypt_key_3des_cbc_encrypt), IPSEC_HMAC_SHA1,
                             authentication_sha1_key_96,
                             sizeof(authentication_sha1_key_96), IPV4);
#endif
        status = CreateIpsecChannel(&channel);
#else
        sp_c_1 = ipsec_spd_add(&local_ip, &netmask, &remote_ip, &netmask, IPSEC_PROTO_TCP,
                               IPSEC_HTONS(CLIENT_PORT), IPSEC_HTONS(SERVER_PORT),
                               POLICY_APPLY, &db->outbound_spd, IPV4);
        sp_c_2 = ipsec_spd_add(&remote_ip, &netmask, &local_ip, &netmask, IPSEC_PROTO_TCP,
                               IPSEC_HTONS(SERVER_PORT), IPSEC_HTONS(CLIENT_PORT),
                               POLICY_APPLY, &db->inbound_spd, IPV4);
        sa_1 = ipsec_sad_add(&sa2, &db->outbound_sad);
        sa_2 = ipsec_sad_add(&sa1, &db->inbound_sad);
        status = ipsec_spd_add_sa(sp_c_1, sa_1);
        status = ipsec_spd_add_sa(sp_c_2, sa_2);
        if (status) {
            aicx_info(0x1a089a, "x", "[NET] Error in TCP create_egress_sa status = 0x%x\n!",
                      status);
            return;
        }
#endif

        aicx_info(0x143ad6, "xx", "[CLIENT] sa_1 0x%x sa_2 0x%x\n", sa_1, sa_2);

        status = connect(sock_tcp_client, (struct sockaddr *)&echo_serv_addr,
                         sizeof(echo_serv_addr));
        if (status < 0) {
            aicx_info(0x1c00c4, "", "[NET] BSD TCP client socket connect fail!\n");
            status = closesocket(sock_tcp_client);
            return;
        }

        aicx_info(0x1bbc8d, "d", "[NET] TCP client socket %d connected\n", sock_tcp_client);

        length = sizeof(struct sockaddr_in);
        status = getsockname(sock_tcp_client, (struct sockaddr *)&local_addr,
                             (socklen_t *)&length);
        aicx_info(0x09d777, "ux", "[NET] TCP client port %u client 0x%x\n",
                  (u32)local_addr.sin_port,
                  (u32)local_addr.sin_addr.s_addr);

        length = sizeof(struct sockaddr_in);
        status = getpeername(sock_tcp_client, (struct sockaddr *)&remote_addr,
                             (socklen_t *)&length);

        aicx_info(0x0e02be, "ux", "[NET] TCP remote port %u remote 0x%x\n",
                  (u32)remote_addr.sin_port,
                  (u32)remote_addr.sin_addr.s_addr);
        while (1) {
            aicx_info(0x02b41c, "dd", "[NET] client sock %d sending packet to server %d\n",
                      sock_tcp_client, sizeof("Hello"));

            status = send(sock_tcp_client, "IPv4 TCP Hello World\n",
                          (sizeof("IPv4 TCP Hello World\n")), 0);
            if (status < 0)
                aicx_info(0x03f106, "d", "[NET] tcp client socket %d send fail!\n",
                          sock_tcp_client);
            else
                aicx_info(0x0abee3, "d", "[NET] tcp client socket %d send hello!\n",
                          sock_tcp_client);

READ_AGAIN:
            status = recv(sock_tcp_client, (void *)tcp_client_recv_buffer,
                          100, 0);
            if (status < 0) {
                if (errno == EAGAIN) {
                    aos_sleep(500);
                    goto READ_AGAIN;
                }

                aicx_info(0x17a225, "dxx", "[NET] connect error on recv socket %d 0x%x 0x%x\n",
                          sock_tcp_client, status, errno);
                break;
            } else {
                aicx_info(0x0aeac9, "dds", "[NET] client %d recv %d bytes: %s\n",
                          sock_tcp_client, status, tcp_client_recv_buffer);
            }

            aos_sleep(1000);
        }

        status = closesocket(sock_tcp_client);
        if (status >= 0)
            aicx_info(0x1a0d59, "d", "[NET] client socket %d closesocketd!\n", sock_tcp_client);
        else
            aicx_info(0x1b045c, "d", "[NET] client socket %d closesocket fail!\n", sock_tcp_client);

#if 1
        DeleteIpsecChannel(&channel);
#else
        ipsec_sad_del(sa_1, &db->outbound_sad);
        ipsec_sad_del(sa_2, &db->inbound_sad);
        ipsec_spd_del(sp_c_1, &db->outbound_spd);
        ipsec_spd_del(sp_c_2, &db->inbound_spd);
#endif
    }
}

int tx_ipv4_tcp_ipsec_application_define(void)
{
    u32 status = 0;

#if LWIP_HAVE_LOOPIF == 0
    netif_handle net_handle;
    ip4_addr_t test_local_ip = {0x0100007f};
    ip4_addr_t test_local_gw = {0x0100007f};
    ip4_addr_t test_netmask = {0x00FFFFFF};
    ip_addr_t ip6_addr;

    IP4_ADDR(&test_local_ip, 127,0,0,1);
    IP4_ADDR(&test_netmask, 255,255,255,0);
    IP4_ADDR(&test_local_gw, 127,0,0,1);
    IP_ADDR6_HOST(&ip6_addr, 0, 0, 0, 0x1);

    aos_sleep(2000);
    //netif_add(&tcp_test_netif, &test_local_ip, &test_netmask, &test_local_gw,
              //NULL, lwip_ims_init_netif, NULL);

    net_handle = lwip_ims_netif_create(&test_local_ip, (ip6_addr_t *)&ip6_addr);
#endif

    aic_network_parse_cp_at("cpcapon 1500");
#if 0
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
#endif

    tcp_thread_server = aos_create_thread("ipsec tcp test server",
                                          tcp_thread_server_entry,
                                          NULL, DEMO_STACK_SIZE,
                                          0, NULL, 8,
                                          AOS_AUTO_START);
    if (tcp_thread_server == AOS_INVALID_THREAD_ID)
    {
        tcp_error_counter++;
        aicx_info(0x1bb190, "", "[NET] Error in create ipsec tcp test server\n");
    }

    tcp_thread_client = aos_create_thread("ipsec tcp test client",
                                          tcp_thread_client_entry,
                                          NULL, DEMO_STACK_SIZE,
                                          0, NULL, 8,
                                          AOS_AUTO_START);
    if (tcp_thread_client == AOS_INVALID_THREAD_ID)
    {
        tcp_error_counter++;
        aicx_info(0x03cc9f, "", "[NET] Error in create ipsec tcp test client\n");
    }

    aicx_info(0x024543, "d", "[NET] IPsec tcp test error counter %d\n", tcp_error_counter);
    return status;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(tx_ipv4_tcp_ipsec_application_define);

