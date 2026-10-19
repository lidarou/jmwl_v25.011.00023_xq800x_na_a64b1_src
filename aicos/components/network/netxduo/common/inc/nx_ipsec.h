/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** NetX Component                                                           */
/**                                                                          */
/**   Internet Protocol Security(IPsec)                                      */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  COMPONENT DEFINITION                                   RELEASE           */
/*                                                                           */
/*    nx_ipsec.h                                             PORTABLE C      */
/*                                                           6.1.9           */
/*  AUTHOR                                                                   */
/*                                                                           */
/*    Qingsheng Li, AIC-Micro Corporation                                    */
/*                                                                           */
/*  DESCRIPTION                                                              */
/*                                                                           */
/*    This file defines the NetX Internet Protocol component,                */
/*    including all data types and external references.  It is assumed       */
/*    that nx_api.h and nx_port.h have already been included.                */
/*                                                                           */
/*  RELEASE HISTORY                                                          */
/*                                                                           */
/*    DATE              NAME                      DESCRIPTION                */
/*                                                                           */
/*  12-15-2021     Qingsheng Li               Initial Version 6.0            */
/*                                                                           */
/*****************************************************************************/

#ifndef NX_IPSEC_H
#define NX_IPSEC_H

#include "nx_api.h"
#include "nx_crypto.h"
#include "nx_crypto_3des.h"
#include "nx_crypto_aes.h"
#include "nx_crypto_hmac_md5.h"
#include "nx_crypto_hmac_sha1.h"

typedef enum IPSEC_MODE
{
    NX_IPSEC_TRANSPORT_MODE,
    NX_IPSEC_TUNNEL_MODE
} IPSEC_MODE;

typedef enum IPSEC_PROTOCOL
{
    NX_IPSEC_ESP_PACKET = NX_PROTOCOL_NEXT_HEADER_ENCAP_SECURITY,
    NX_IPSEC_AH_PACKET = NX_PROTOCOL_NEXT_HEADER_AUTHENTICATION,
} IPSEC_PROTOCOL;

#define NX_IPSEC_PKT_CONT 1

#define NX_IPSEC_TRAFFIC_BYPASS 2
#define NX_IPSEC_TRAFFIC_DROP 3
#define NX_IPSEC_TRAFFIC_PROTECT 4
#define NX_IPSEC_TRAFFIC_PENDING_IKEV2 5

#define NX_IPSEC_HW_PENDING 6

#define NX_IPSEC_REJECTED 7

#define NX_IPSEC_ESP_TRAIL_LEN 2

#define NX_IPSEC_CRYPTO_BLOCK_SIZE 16

#pragma pack(1)
/**
 * struct NX_IPSEC_ESP_HEADER_STRUCT - ESP Header.
 * @spi: Security Parameters Index.
 * @seq: Sequence Number.
 *
 * ESP Header.
 */
typedef struct NX_IPSEC_ESP_HEADER_STRUCT
{
    UINT spi;
    UINT seq;
} NX_IPSEC_ESP_HEADER;

/**
 * struct NX_IPSEC_ESP_TAIL_STRUCT - ESP Tail.
 * @padding_length: Pad Length.
 * @next_hdr: Next Header.
 *
 * ESP Tail.
 */
typedef struct NX_IPSEC_ESP_TAIL_STRUCT
{
    UCHAR padding_length;
    UCHAR next_hdr;
} NX_IPSEC_ESP_TAIL;
#pragma pack()

typedef struct NX_IPSEC_AUTHENTICATION_HEADER_STRUCT
{
    UCHAR nexthdr;
    UCHAR hdrlen;
    USHORT reserved;
    UINT spi;
    UINT seq;
} NX_IPSEC_AUTHENTICATION_HEADER;

/**
 * struct NX_IPSEC_SA_INTEGRITY_METHOD_STRUCT.
 * @key: key data for authenication algorithm.
 * @crypto_metadata: Metadata space required by authenication algorithm.
 * @output_length: ICV length.
 * @nx_crypto_key_size_in_bits: Key length in bit.
 * @nx_crypto_ICV_size_in_bits: ICV length in bit.
 * @nx_crypto_IV_size_in_bits: IV length in bit.
 * @nx_crypto_algorithm: Authenication algorithm.
 * @nx_crypto_block_size_in_bytes:
 *
 * Authenication algorithm related parameters.
 */
typedef struct NX_IPSEC_SA_INTEGRITY_METHOD_STRUCT
{
    void *key;
    void *crypto_metadata;

    UINT output_length;
    UINT nx_crypto_key_size_in_bits;
    UINT nx_crypto_IV_size_in_bits;
    UINT nx_crypto_ICV_size_in_bits;
    UINT nx_crypto_algorithm;
    UINT nx_crypto_block_size_in_bytes;
} NX_IPSEC_SA_INTEGRITY_METHOD;

/**
 * struct NX_IPSEC_SA_ENCRYPTION_METHOD_STRUCT.
 * @crypto_metadata: Metadata space required by authenication algorithm.
 * @nx_crypto_iv: Initialization Vector.
 * @nx_crypto_IV_size_in_bits: IV length in bit.
 * @nx_crypto_algorithm: Authenication algorithm.
 * @nx_crypto_block_size_in_bytes:
 *
 * Encryption algorithm related parameters.
 */
typedef struct NX_IPSEC_SA_ENCRYPTION_METHOD_STRUCT
{
    void *crypto_metadata;
    void *nx_crypto_iv;

    UINT nx_crypto_IV_size_in_bits;
    UINT nx_crypto_algorithm;
    UINT nx_crypto_block_size_in_bytes;
} NX_IPSEC_SA_ENCRYPTION_METHOD;

/**
 * struct NX_IPSEC_SA_STRUCT.
 * @spi: Security Parameters Index.
 * @seq: Sequence Number.
 * @nx_ipsec_selector_ptr: Unused.
 * @nx_ipsec_sa_head_encap_size: ESP Header Size.
 * @nx_ipsec_sa_mode: Work Mode, Tunnel/Transport.
 * @nx_ipsec_sa_protocol: Encapsulation Protocol.
 * @nx_ipsec_sa_encryption_method: Encryption Method.
 * @nx_ipsec_sa_integrity_method: Authentication Method.
 * @crypto_method: Encryption Method For Crypto Library.
 * @authentication_method: Authentication Method For Crypto Library.
 * @dest_addr: Destination address of IPv4 or IPv6.
 *
 * Store complete SA related parameters.
 */
typedef struct NX_IPSEC_SA_STRUCT
{
    UINT spi;
    UINT seq;
    UINT seq_mask;
    void *nx_ipsec_selector_ptr;
    UINT nx_ipsec_sa_head_encap_size;
    //UINT nx_crypto_algorithm;
    IPSEC_MODE nx_ipsec_sa_mode;
    IPSEC_PROTOCOL nx_ipsec_sa_protocol;
    NX_IPSEC_SA_ENCRYPTION_METHOD *nx_ipsec_sa_encryption_method;
    NX_IPSEC_SA_INTEGRITY_METHOD *nx_ipsec_sa_integrity_method;

    NX_CRYPTO_METHOD crypto_method;
    NX_CRYPTO_METHOD authentication_method;

    USHORT src_port;
    USHORT dst_port;
    NXD_ADDRESS dest_addr;
    struct NX_IPSEC_SA_STRUCT *sa_next, *sa_prev;
} NX_IPSEC_SA;

UINT _nx_ipsec_ip_output_packet_process(NX_IP *ip_ptr, NX_PACKET **nx_packet,
                                        ULONG protocol, ULONG payload_size,
                                        ULONG *size);

UINT _nx_ipsec_sa_ingress_lookup(NX_IP *ip_ptr, NXD_ADDRESS *src_addr,
                                 NXD_ADDRESS *dest_addr, UINT spi,
                                 UCHAR protocol, NX_PACKET *nx_packet,
                                 UCHAR *nx_packet_prepend_ptr);

UINT _nx_ipsec_sa_ingress_selector_check(UCHAR *nx_packet_prepend_ptr,
                                         UCHAR protocol,
                                         void *nx_ipsec_selector_ptr);

void _nx_ipsec_sa_lifetime_tick(NX_IP *ip_ptr);

void _nx_ipsec_hw_packet_process(NX_IP *ip_ptr);

UINT _nx_ipsec_create_egress_sa(NX_IP *ip_ptr, NX_IPSEC_SA **sa,
                                IPSEC_MODE mode, IPSEC_PROTOCOL proto,
                                UINT algo, UINT authentication,
                                UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                UCHAR *ik, NX_CRYPTO_KEY_SIZE ik_size_in_bits,
                                UINT spi, UCHAR protocol,
                                NXD_ADDRESS *dest_addr, USHORT src_port,
                                USHORT dst_port);

UINT _nx_ipsec_create_ingress_sa(NX_IP *ip_ptr, NX_IPSEC_SA **sa,
                                 IPSEC_MODE mode, IPSEC_PROTOCOL proto,
                                 UINT algo, UINT authentication, UCHAR *key,
                                 NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                 UCHAR *ik, NX_CRYPTO_KEY_SIZE ik_size_in_bits,
                                 UINT spi, UCHAR protocol,
                                 NXD_ADDRESS *dest_addr, USHORT src_port,
                                 USHORT dst_port);

void _nx_ipsec_release_sa(NX_IP *ip_ptr, NX_IPSEC_SA *sa, INT is_ingress);

#endif

