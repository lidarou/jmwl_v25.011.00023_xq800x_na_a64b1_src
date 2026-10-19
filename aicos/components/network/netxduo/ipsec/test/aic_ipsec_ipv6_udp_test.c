
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x167e25
#define LOG_MODULE    AMOD_NET
#include <aic_osa.h>
#include <aic_init.h>

#include "aic_ims_nic.h"
#include "nx_api.h"
#include "nxd_bsd.h"
#include "nx_ipsec.h"
#include <aic_net_api.h>

#include <string.h>
#include <stdlib.h>

//#define IPSEC_AES_MD5
//#define IPSEC_NONE_MD5

#define DEMO_STACK_SIZE (16 * 1024)
#define SERVER_PORT 87
#define CLIENT_PORT 77
#define UDP_SERVER_RCV_BUFFER_SIZE 1400
#define UDP_CLIENT_RCV_BUFFER_SIZE 1400


static aos_thread_id udp_thread_server;
static aos_thread_id udp_thread_client;
static NX_PACKET_POOL bsd_udp_pool;
struct aic_packet_pool aims_big_packet_pool;
static NX_IP bsd_udp_ip;

static ULONG udp_error_counter;

static UCHAR udp_packet_pool[16384];
static UCHAR udp_ip_instance_pool[2048];
static UCHAR udp_bsd_pool[2048];

static CHAR udp_server_recv_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static CHAR udp_send_buffer[UDP_SERVER_RCV_BUFFER_SIZE];
static CHAR udp_client_recv_buffer[UDP_CLIENT_RCV_BUFFER_SIZE];

static VOID udp_thread_server_entry(void *thread_input);
static VOID udp_thread_client_entry(void *thread_input);
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

static VOID udp_thread_server_entry(void *thread_input)
{
    ULONG actual_status;
    NXD_ADDRESS ip_address;
    UINT iface_index, address_index;
    struct sockaddr_in6 server_addr, client_addr;
    INT status, sock_udp_server, addr_len;
    NXD_ADDRESS dst_addr;
    NX_IPSEC_SA *sa_1 = NULL, *sa_2 = NULL;

    dst_addr.nxd_ip_version = NX_IP_VERSION_V6;
    dst_addr.nxd_ip_address.v6[0] = htonl(0x20010db8u);
    dst_addr.nxd_ip_address.v6[1] = htonl(0xf101u);
    dst_addr.nxd_ip_address.v6[2] = htonl(0x0u);
    dst_addr.nxd_ip_address.v6[3] = htonl(0x0101u);

    NX_PARAMETER_NOT_USED(thread_input);

    status = (INT)nx_ip_status_check(&bsd_udp_ip, NX_IP_INITIALIZE_DONE,
                                     &actual_status, NX_IP_PERIODIC_RATE);
    if (status != NX_SUCCESS) {
        aicx_info(0x11b725, "", "[NET] ip status check fail!\n");
        return;
    }

    status = (INT)nxd_ipv6_enable(&bsd_udp_ip);
    if (status != NX_SUCCESS) {
        aicx_info(0x1db7cb, "x", "[NET] enable ipv6 fail 0x%x\n", status);
        return;
    }

    iface_index = 0;
    status = (INT)nxd_ipv6_address_set(&bsd_udp_ip, iface_index, NX_NULL, 10,
                                       &address_index);
    if (status) {
        aicx_info(0x057978, "x", "[NET] ipv6 primary addr set fail 0x%x\n", status);
        return;
    }

    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 0x101;

    status = (INT)nxd_ipv6_address_set(&bsd_udp_ip, iface_index, &ip_address,
                                       64, &address_index);
    if (status) {
        aicx_info(0x068cf7, "x", "[NET] ipv6 host addr set fail 0x%x\n", status);
        return;
    }

    aos_sleep(5 * NX_IP_PERIODIC_RATE);

    sock_udp_server = anet_socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_server == -1) {
        aicx_info(0x08aa63, "d", "[NET] BSD udp Server socket create fail, errno %d\n", errno);
        return;
    }

    aicx_info(0x11bb85, "u", "[NET] BSD udp Server socket created %u\n",
               (ULONG)sock_udp_server);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    server_addr.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    server_addr.sin6_addr._S6_un._S6_u32[2] = htonl(0x0u);
    server_addr.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    server_addr.sin6_port = htons(SERVER_PORT);
    server_addr.sin6_family = AF_INET6;

    status = anet_bind(sock_udp_server, (struct sockaddr *)&server_addr,
                       sizeof(server_addr));
    if (status < 0) {
        aicx_info(0x1be98e, "", "[NET] ipv6 server socket bind fail\n");
        return;
    }
#ifdef IPSEC_AES_MD5
    status = _nx_ipsec_create_ingress_sa(&bsd_udp_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_AES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         encrypt_key_cbc_128_encrypt,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 1,
                                         0, &dst_addr, SERVER_PORT,
                                         CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_udp_ip, &sa_2,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_AES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         encrypt_key_cbc_128_encrypt,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 2,
                                         0, &dst_addr, CLIENT_PORT,
                                         SERVER_PORT);

#elif defined(IPSEC_NONE_MD5)
    status = _nx_ipsec_create_ingress_sa(&bsd_udp_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_NULL,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         NULL,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 1,
                                         0, &dst_addr, SERVER_PORT, CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_udp_ip, &sa_2,
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
    status = _nx_ipsec_create_ingress_sa(&bsd_udp_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                         encrypt_key_3des_cbc_encrypt,
                                         sizeof(encrypt_key_3des_cbc_encrypt) <<
                                         3, authentication_sha1_key_96,
                                         sizeof(authentication_sha1_key_96) <<
                                         3, 1, 0, &dst_addr, SERVER_PORT,
                                         CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_udp_ip, &sa_2,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                         encrypt_key_3des_cbc_encrypt,
                                         sizeof(encrypt_key_3des_cbc_encrypt) <<
                                         3, authentication_sha1_key_96,
                                         sizeof(authentication_sha1_key_96) <<
                                         3, 2, 0, &dst_addr, CLIENT_PORT,
                                         SERVER_PORT);
#endif

    while (1) {
        addr_len = sizeof(struct sockaddr_in6);
        status = anet_recvfrom(sock_udp_server, (VOID *)udp_server_recv_buffer,
                               UDP_SERVER_RCV_BUFFER_SIZE, 0,
                               (struct sockaddr *)&client_addr, &addr_len);
        if (status == ERROR) {
            if (errno == EAGAIN) {
                aicx_info(0x1baa83, "", "[NET] no error received, try again later\n");
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
                  sock_udp_server, (ULONG)status, status);

        //snprintf(udp_send_buffer, UDP_SERVER_RCV_BUFFER_SIZE,
                 //"hello world 0x%x\n", NX_RAND());
        memset(udp_send_buffer, 'b', UDP_SERVER_RCV_BUFFER_SIZE);
        status = anet_sendto(sock_udp_server, udp_send_buffer,
                             UDP_SERVER_RCV_BUFFER_SIZE, 0,
                             (struct sockaddr *)&client_addr,
                             sizeof(client_addr));
        if (status == NX_SOC_ERROR)
            aicx_info(0x10ca4a, "d", "[NET] udp server sending to client fail s:%d\n",
                      sock_udp_server);
        else
            aicx_info(0x0b91ae, "dd", "[NET] udp server sending to client:%d: %d\n",
                      sock_udp_server, status);
        aos_sleep(1000);
    }

    status = soc_close(sock_udp_server);
    if (status != NX_SOC_ERROR)
        aicx_info(0x0aba03, "d", "[NET] socket close conn to client on %d\n",
                  sock_udp_server);
    else
        aicx_info(0x0b2735, "d", "[NET] socket close conn to client on %d fail\n",
                  sock_udp_server);
}

static VOID udp_thread_client_entry(void *thread_input)
{
    ULONG actual_status;
    INT status, sock_udp_client, length;
    struct sockaddr_in6 echo_serv_addr6, local_addr6;
    NXD_ADDRESS dst_addr;
    NX_IPSEC_SA *sa_1 = NULL, *sa_2 = NULL;

    dst_addr.nxd_ip_version = NX_IP_VERSION_V6;
    dst_addr.nxd_ip_address.v6[0] = htonl(0x20010db8u);
    dst_addr.nxd_ip_address.v6[1] = htonl(0xf101u);
    dst_addr.nxd_ip_address.v6[2] = htonl(0x0u);
    dst_addr.nxd_ip_address.v6[3] = htonl(0x0101u);

    NX_PARAMETER_NOT_USED(thread_input);

    status = (INT)nx_ip_status_check(&bsd_udp_ip, NX_IP_INITIALIZE_DONE,
                                     &actual_status, NX_IP_PERIODIC_RATE);
    if (status != NX_SUCCESS)
        return;
    aos_sleep(10 * NX_IP_PERIODIC_RATE);

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
#ifdef IPSEC_AES_MD5
    status = _nx_ipsec_create_egress_sa(&bsd_udp_ip, &sa_1,
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
    status = _nx_ipsec_create_egress_sa(&bsd_udp_ip, &sa_2,
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
    status = _nx_ipsec_create_egress_sa(&bsd_udp_ip, &sa_1,
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
    status = _nx_ipsec_create_egress_sa(&bsd_udp_ip, &sa_2,
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
    status = _nx_ipsec_create_egress_sa(&bsd_udp_ip, &sa_1,
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
    status = _nx_ipsec_create_egress_sa(&bsd_udp_ip, &sa_2,
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

    while (1) {
        sock_udp_client = anet_socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_client == -1) {
            aicx_info(0x12d618, "", "[NET] BSD udp client socket create fail!\n");
            return;
        }

        status = anet_bind(sock_udp_client, (struct sockaddr *)&local_addr6,
                      sizeof(struct sockaddr_in6));
        if (status != NX_SUCCESS) {
            aicx_info(0x17d7c5, "x", "[IPV6] ipsec udp bind fail 0x%x\n", status);
            return;
        }

        aicx_info(0x1d9f5e, "u", "[NET] udp client socket created ok %u\n",
                  (ULONG)sock_udp_client);

        length = sizeof(struct sockaddr_in6);
        //snprintf(udp_send_buffer, UDP_SERVER_RCV_BUFFER_SIZE,
                 //"hello world 0x%x\n", NX_RAND());

        memset(udp_send_buffer, 'a', UDP_SERVER_RCV_BUFFER_SIZE);
        status = anet_sendto(sock_udp_client, udp_send_buffer,
                             UDP_SERVER_RCV_BUFFER_SIZE, 0,
                             (struct sockaddr *)&echo_serv_addr6,
                             sizeof(echo_serv_addr6));
        if (status == ERROR)
            aicx_info(0x0da511, "d", "[NET] udp client socket %d send fail!\n",
                      sock_udp_client);
        else
            aicx_info(0x1220bd, "dd", "[NET] udp client socket %d send: %d!\n",
                      sock_udp_client, status);
READ_AGAIN:
        status = anet_recvfrom(sock_udp_client, (VOID *)udp_client_recv_buffer,
                               UDP_CLIENT_RCV_BUFFER_SIZE, 0,
                               (struct sockaddr *)&echo_serv_addr6, &length);
        if (status < 0) {
            if (errno == EAGAIN)
                goto READ_AGAIN;
            aicx_info(0x068028, "dxx", "[NET] connection error on recv socket %d 0x%x 0x%x\n",
                      sock_udp_client, status, errno);
        } else {
            aicx_info(0x07a102, "ddd", "[NET] udp client %d recv %d bytes: %d\n",
                      sock_udp_client, status, status);
        }

        status = soc_close(sock_udp_client);
        if (status != NX_SOC_ERROR)
            aicx_info(0x0d5509, "d", "[NET] client socket %d closed!\n", sock_udp_client);
        else
            aicx_info(0x173836, "dd", "[NET] client socket %d close fail! errno %d\n", sock_udp_client, errno);

        aos_sleep(1000);
    }
}

int tx_ipv6_udp_ipsec_application_define(void)
{
    UINT status;
    NXD_ADDRESS dst_addr;

    dst_addr.nxd_ip_version = NX_IP_VERSION_V6;
    dst_addr.nxd_ip_address.v6[0] = htonl(0x20010db8u);
    dst_addr.nxd_ip_address.v6[1] = htonl(0xf101u);
    dst_addr.nxd_ip_address.v6[2] = htonl(0x0u);
    dst_addr.nxd_ip_address.v6[3] = htonl(0x0101u);

    nx_system_initialize();
#ifdef WIN32
    aic_network_pcap_init();
    _nx_ipsec_crypto_library_init();
#endif

    status = nx_packet_pool_create(&bsd_udp_pool, "BSD udp Ipsec Packet Pool",
                                   1500, udp_packet_pool, 16384);
    if (status) {
        udp_error_counter++;
        aicx_info(0x1d7071, "", "[NET] Error in creating udp packet pool!\n");
    }

    status = nx_ip_create(&bsd_udp_ip, "BSD udp Ipsec IP Instance",
                          IP_ADDRESS(1, 2, 3, 4), 0xFFFFFF00UL,
                          &bsd_udp_pool, aic_ims_nic_driver_entry,//_nx_ram_network_driver,
                          udp_ip_instance_pool, 2048, 1);
    if (status) {
        udp_error_counter++;
        aicx_info(0x141c87, "", "[NET] Error in creating udp IP Instance!\n");
    }

    status = nx_udp_enable(&bsd_udp_ip);
    if (status) {
        udp_error_counter++;
        aicx_info(0x1e07c9, "", "[NET] Error in enable udp!\n");
    }

    status = nx_ip_fragment_enable(&bsd_udp_ip);
    if (status) {
        udp_error_counter++;
        aicx_info(0x15210a, "", "[NET] Error in enable ip fragment!\n");
    }

    status = (UINT)bsd_initialize(&bsd_udp_ip, &bsd_udp_pool, udp_bsd_pool,
                                  2048, 2);
    if (status) {
        udp_error_counter++;
        aicx_info(0x0d6ab6, "x", "[NET] Error bsd_initialize! 0x%x\n", status);
    }

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

