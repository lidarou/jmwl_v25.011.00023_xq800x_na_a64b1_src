
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x032a9f
#define LOG_MODULE    AMOD_NET
#include <aic_osa.h>
#include <aic_init.h>

#include "aic_ims_nic.h"
#include "nx_api.h"
#include "nxd_bsd.h"
#include "nx_port.h"
#include "nx_ipsec.h"
#include "aic_net_api.h"

#include <string.h>
#include <stdlib.h>

//#define IPSEC_AES_MD5
#define IPSEC_NONE_MD5

#define IPSEC_DEMO_STACK_SIZE   2048
#define IPSEC_SERVER_PORT       721
#define IPSEC_CLIENT_PORT       821

//#define CLIENT_MESSAGE "UDP Ipsec Socket testing\n"
#define CLIENT_MESSAGE_LEN      2500

static aos_thread_id udp_thread_server;
static aos_thread_id udp_thread_client;
static NX_PACKET_POOL bsd_pool;
struct aic_packet_pool aims_big_packet_pool;
static NX_IP bsd_ip;

//ULONG thread_0_counter;
static ULONG nx_thread_2_counter;
static ULONG error_counter;
static char ipsec_stack_2[IPSEC_DEMO_STACK_SIZE];
static char ipsec_stack_3[IPSEC_DEMO_STACK_SIZE];
static char ipsec_pool[16384];
static char client_message[CLIENT_MESSAGE_LEN];
static char buffer[CLIENT_MESSAGE_LEN];
static char echo_msg[CLIENT_MESSAGE_LEN];

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

extern void _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

static void thread_server_entry(void *thread_input)
{
    INT status, addrlen1;
    INT sock_udp_2;
    struct sockaddr_in echoServAddr;
    struct sockaddr_in fromAddr;
    NXD_ADDRESS dst_addr;
    NX_IPSEC_SA *sa_1 = NULL, *sa_2 = NULL;

    NX_PARAMETER_NOT_USED(thread_input);
    dst_addr.nxd_ip_version = NX_IP_VERSION_V4;
    dst_addr.nxd_ip_address.v4 = htonl(IP_ADDRESS(1, 2, 3, 4));

    sock_udp_2 = anet_socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_udp_2 == -1) {
        aicx_info(0x0c0a8b, "", "[NET] Error: BSD UDP Server socket create\n");
        return;
    }
    aicx_info(0x009aef, "d", "[NET] sock_udp_2 %d\n", sock_udp_2);

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = PF_INET;
    echoServAddr.sin_addr.s_addr = htonl(IP_ADDRESS(1,2,3,4));
    echoServAddr.sin_port = htons(IPSEC_SERVER_PORT);

    status = anet_bind(sock_udp_2, (struct sockaddr *)&echoServAddr,
                       sizeof(echoServAddr));
    if (status < 0) {
        aicx_info(0x19f826, "", "[NET] Error: BSP UDP Server Socket Bind\n");
        return;
    }

#ifdef IPSEC_AES_MD5
    status = _nx_ipsec_create_ingress_sa(&bsd_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_AES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         encrypt_key_cbc_128_encrypt,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 1,
                                         0, &dst_addr, IPSEC_SERVER_PORT,
                                         IPSEC_CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_ip, &sa_2,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_AES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         encrypt_key_cbc_128_encrypt,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 2,
                                         0, &dst_addr, IPSEC_CLIENT_PORT,
                                         IPSEC_SERVER_PORT);

#elif defined(IPSEC_NONE_MD5)
    status = _nx_ipsec_create_ingress_sa(&bsd_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_NULL,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         NULL,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 1,
                                         0, &dst_addr, IPSEC_SERVER_PORT,
                                         IPSEC_CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_ip, &sa_2,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_NULL,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                         NULL,
                                         NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                         authentication_key_1,
                                         sizeof(authentication_key_1) << 3, 2,
                                         0, &dst_addr, IPSEC_CLIENT_PORT,
                                         IPSEC_SERVER_PORT);
#else
    status = _nx_ipsec_create_ingress_sa(&bsd_ip, &sa_1,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                         encrypt_key_3des_cbc_encrypt,
                                         sizeof(encrypt_key_3des_cbc_encrypt) <<
                                         3, authentication_sha1_key_96,
                                         sizeof(authentication_sha1_key_96) <<
                                         3, 1, 0, &dst_addr, IPSEC_SERVER_PORT,
                                         IPSEC_CLIENT_PORT);
    status = _nx_ipsec_create_ingress_sa(&bsd_ip, &sa_2,
                                         NX_IPSEC_TRANSPORT_MODE,
                                         NX_IPSEC_ESP_PACKET,
                                         NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                         NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                         encrypt_key_3des_cbc_encrypt,
                                         sizeof(encrypt_key_3des_cbc_encrypt) <<
                                         3, authentication_sha1_key_96,
                                         sizeof(authentication_sha1_key_96) <<
                                         3, 2, 0, &dst_addr, IPSEC_CLIENT_PORT,
                                         IPSEC_SERVER_PORT);
#endif

    while (1) {
        addrlen1 = sizeof(struct sockaddr_in);

        status = anet_recvfrom(sock_udp_2, (void *)buffer, CLIENT_MESSAGE_LEN, 0,
                               (struct sockaddr *)&fromAddr, &addrlen1);
        if (status == NX_SOC_ERROR) {
            aicx_info(0x18f3eb, "", "[NET] Error: BSD Server Socket receive\n");
        } else {
            aicx_info(0x16ade8, "d", "[NET] Server recv from Client: %d\n", status);

            status = anet_sendto(sock_udp_2, buffer, status, 0,
                                 (struct sockaddr *)&fromAddr, sizeof(fromAddr));

            if (status == NX_SOC_ERROR)
                aicx_info(0x1cfd21, "", "[NET] Error: BSD Server Socket echo\n");
        }

        aos_sleep(1000);
    }
}

static void thread_client_entry(void *thread_input)
{
    int status, sock_udp_1;
    int addr_len;
    struct sockaddr_in dest_addr, client_addr, from_addr;
    NX_IPSEC_SA *sa_1 = NULL, *sa_2 = NULL;
    NXD_ADDRESS dst_addr;
    struct sockaddr_in local_addr;

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(IP_ADDRESS(1, 2, 3, 4));
    local_addr.sin_port = htons(IPSEC_CLIENT_PORT);

    dst_addr.nxd_ip_version = NX_IP_VERSION_V4;
    dst_addr.nxd_ip_address.v4 = htonl(IP_ADDRESS(1, 2, 3, 4));

    NX_PARAMETER_NOT_USED(thread_input);

    aos_sleep(10);
    nx_thread_2_counter = 0;
#ifdef IPSEC_AES_MD5
    status = _nx_ipsec_create_egress_sa(&bsd_ip, &sa_1,
                                        NX_IPSEC_TRANSPORT_MODE,
                                        NX_IPSEC_ESP_PACKET,
                                        NX_CRYPTO_ENCRYPTION_AES_CBC,
                                        NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                        encrypt_key_cbc_128_encrypt,
                                        NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                        authentication_key_1,
                                        sizeof(authentication_key_1) << 3, 1,
                                        0, &dst_addr, IPSEC_CLIENT_PORT,
                                        IPSEC_SERVER_PORT);
    status = _nx_ipsec_create_egress_sa(&bsd_ip, &sa_2,
                                        NX_IPSEC_TRANSPORT_MODE,
                                        NX_IPSEC_ESP_PACKET,
                                        NX_CRYPTO_ENCRYPTION_AES_CBC,
                                        NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                        encrypt_key_cbc_128_encrypt,
                                        NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                        authentication_key_1,
                                        sizeof(authentication_key_1) << 3, 2,
                                        0, &dst_addr, IPSEC_SERVER_PORT,
                                        IPSEC_CLIENT_PORT);

#elif defined(IPSEC_NONE_MD5)
    status = _nx_ipsec_create_egress_sa(&bsd_ip, &sa_1,
                                        NX_IPSEC_TRANSPORT_MODE,
                                        NX_IPSEC_ESP_PACKET,
                                        NX_CRYPTO_ENCRYPTION_NULL,
                                        NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                        NULL,
                                        NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                        authentication_key_1,
                                        sizeof(authentication_key_1) << 3, 1,
                                        0, &dst_addr, IPSEC_SERVER_PORT,
                                        IPSEC_CLIENT_PORT);
    status = _nx_ipsec_create_egress_sa(&bsd_ip, &sa_2,
                                        NX_IPSEC_TRANSPORT_MODE,
                                        NX_IPSEC_ESP_PACKET,
                                        NX_CRYPTO_ENCRYPTION_NULL,
                                        NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96,
                                        NULL,
                                        NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,
                                        authentication_key_1,
                                        sizeof(authentication_key_1) << 3, 2,
                                        0, &dst_addr, IPSEC_CLIENT_PORT,
                                        IPSEC_SERVER_PORT);
#else
    status = _nx_ipsec_create_egress_sa(&bsd_ip, &sa_1,
                                        NX_IPSEC_TRANSPORT_MODE,
                                        NX_IPSEC_ESP_PACKET,
                                        NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                        NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                        encrypt_key_3des_cbc_encrypt,
                                        sizeof(encrypt_key_3des_cbc_encrypt) <<
                                        3, authentication_sha1_key_96,
                                        sizeof(authentication_sha1_key_96) << 3,
                                        1, 0, &dst_addr, IPSEC_SERVER_PORT,
                                        IPSEC_CLIENT_PORT);
    status = _nx_ipsec_create_egress_sa(&bsd_ip, &sa_2,
                                        NX_IPSEC_TRANSPORT_MODE,
                                        NX_IPSEC_ESP_PACKET,
                                        NX_CRYPTO_ENCRYPTION_3DES_CBC,
                                        NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96,
                                        encrypt_key_3des_cbc_encrypt,
                                        sizeof(encrypt_key_3des_cbc_encrypt) <<
                                        3, authentication_sha1_key_96,
                                        sizeof(authentication_sha1_key_96) << 3,
                                        2, 0, &dst_addr, IPSEC_CLIENT_PORT,
                                        IPSEC_SERVER_PORT);
#endif

    while (1) {
        sock_udp_1 = anet_socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_udp_1 == ERROR) {
            aicx_info(0x0e0583, "", "[NET] Error: BSD UDP Client socket create\n");
            return;
        }
        aicx_info(0x068058, "d", "[NET] sock_udp_1 %d\n", sock_udp_1);
        status = anet_bind(sock_udp_1, (struct sockaddr *)&local_addr,
                           sizeof(struct sockaddr_in));
        if (status != NX_SUCCESS) {
            aicx_info(0x170a10, "x", "ipsec tcp bind fail 0x%x\n", status);
            return;
        }

        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = PF_INET;
        client_addr.sin_addr.s_addr = htonl(IP_ADDRESS(1, 2, 3, 4));

        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_addr.s_addr = htonl(IP_ADDRESS(1, 2, 3, 4));
        dest_addr.sin_port = htons(IPSEC_SERVER_PORT);
        dest_addr.sin_family = PF_INET;

        memset(client_message, 'a', CLIENT_MESSAGE_LEN);
        aicx_info(0x116fcb, "ud", "[NET] BSD client socket %u sending test message size: %d\n",
                  (ULONG)sock_udp_1, CLIENT_MESSAGE_LEN);
        status = anet_sendto(sock_udp_1, client_message,
                             CLIENT_MESSAGE_LEN, 0,
                             (struct sockaddr *)&dest_addr, sizeof(dest_addr));

        if (status == NX_SOC_ERROR) {
            aicx_info(0x16850b, "", "[NET] Error: BSD client socket send\n");
        } else {
            addr_len = sizeof(struct sockaddr_in);
            status = anet_recvfrom(sock_udp_1, (void *)echo_msg, 64, 0,
                                   (struct sockaddr *)&from_addr, &addr_len);

            if (status == NX_SOC_ERROR) {
                aicx_info(0x0aeda3, "", "[NET] Error: BSD client socket echo receivev\n");
            } else {
                aicx_info(0x1665a8, "ud", "[NET] CLient %u recv: %d\n", (UINT)sock_udp_1,
                          status);
            }
        }

        status = soc_close(sock_udp_1);
        if (status == NX_SOC_ERROR)
            aicx_info(0x17db34, "", "[NET] Error: BSD client socket close\n");

        nx_thread_2_counter++;

        aos_sleep(1000);
    }
}

int bsd_aic_ipsec_test_application_define(void)
{
    UINT status;
    NXD_ADDRESS dst_addr;

    dst_addr.nxd_ip_version = NX_IP_VERSION_V4;
    dst_addr.nxd_ip_address.v4 = htonl(IP_ADDRESS(1, 2, 3, 4));

    nx_system_initialize();
#ifdef WIN32
    aic_network_pcap_init();
    _nx_ipsec_crypto_library_init();
#endif

    status = nx_packet_pool_create(&bsd_pool, "AIC IPSEC Packet Pool", 1600,
                                   ipsec_pool, 16384);
    if (status)
    {
        error_counter++;
        aicx_info(0x1e2d64, "", "[NET] Error in creating BSD packet pool\n!");
    }

    status = nx_ip_create(&bsd_ip, "AIC IPSEC Instance 2",
                          IP_ADDRESS(1, 2, 3, 4),
                          0xFFFFFF00UL, &bsd_pool, aic_ims_nic_driver_entry,//_nx_ram_network_driver,
                          ipsec_stack_2, IPSEC_DEMO_STACK_SIZE, 1);
    if (status) {
        aicx_info(0x1615dc, "", "[NET] Error creating BSD IP instance\n");
        error_counter++;
    }

    status = nx_udp_enable(&bsd_ip);
    if (status)
    {
        error_counter++;
        aicx_info(0x089a70, "", "[NET] Error in nx_udp_enable\n!");
    }

    status = nx_ip_fragment_enable(&bsd_ip);
    if (status) {
        error_counter++;
        aicx_info(0x15210a, "", "[NET] Error in enable ip fragment!\n");
    }

    status = (UINT)bsd_initialize(&bsd_ip, &bsd_pool, ipsec_stack_3, 2048, 2);
    if (status)
    {
        error_counter++;
        aicx_info(0x0cb9d9, "x", "[NET] Error in bsd_init status = 0x%x\n!", status);
    }

    udp_thread_server = aos_create_thread("ipsec udp test server",
                                          thread_server_entry,
                                          NULL, IPSEC_DEMO_STACK_SIZE,
                                          0, NULL, 8,
                                          AOS_AUTO_START);
    if (udp_thread_server == AOS_INVALID_THREAD_ID)
    {
        error_counter++;
        aicx_info(0x060035, "", "[NET] Error in create ipsec udp test server\n");
    }

    udp_thread_client = aos_create_thread("ipsec udp test client",
                                          thread_client_entry,
                                          NULL, IPSEC_DEMO_STACK_SIZE,
                                          0, NULL, 8,
                                          AOS_AUTO_START);
    if (udp_thread_client == AOS_INVALID_THREAD_ID)
    {
        error_counter++;
        aicx_info(0x058035, "", "[NET] Error in create ipsec udp test client\n");
    }
    aicx_info(0x086eb6, "dd", "[NET]counter = %d message size %d\n", error_counter,
                        CLIENT_MESSAGE_LEN);

    return AOS_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(bsd_aic_ipsec_test_application_define);

