/*
 * embedded IPsec
 * Copyright (c) 2003 Niklaus Schild and Christian Scheurer, HTI Biel/Bienne
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

/** @file esp.c
 *  @brief This module contains the Encapsulating Security Payload code
 *
 *  @author  Niklaus Schild <n.schild@gmx.ch> <BR>
 *
 *  <B>OUTLINE:</B>
 *
 *  <B>IMPLEMENTATION:</B>
 * All functions work in-place (i.g. mainipulate directly the original
 * packet without copying any data). For the encapsulation routine,
 * the caller must ensure that space for the new IP and ESP header are
 * available in front of the packet:
 *
 *  <pre>
 *                              | pointer to packet header
 *     ________________________\/________________________________________________
 *    |          |       |      |                             | padd       | ev. |
 *    | Ethernet | newIP | ESP  |   original (inner) packet   | next-proto | ICV |
 *    |__________|_______|______|_____________________________|____________|_____|
 *    |                         |                             |                  |
 *    |<-room for new headers-->|                             |<-   room tail  ->|
 *  </pre>
 *
 * This document is part of <EM>embedded IPsec<BR>
 * Copyright (c) 2003 Niklaus Schild and Christian Scheurer, HTI Biel/Bienne<BR>
 * All rights reserved.<BR>
 * This file contains code from the OpenSSL Project<BR>
 * portions Copyright (c) 1998-2003 OpenSSL (www.openssl.org)
 *</EM><HR>
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18958e
#define LOG_MODULE    AMOD_NET

#include "lwip/opt.h"

#if defined(EIPS) && EIPS == 1

#include <string.h>
#include <assert.h>

#include "ipsec/ipsec.h"
#include "ipsec/util.h"
#include "lwip/debug.h"
#include "ipsec/types.h"

#include "ipsec/sa.h"
#include "ipsec/esp.h"
#include "ipsec/crypto.h"

#include "ipsec/util.h"
#include "lwip/ip.h"

#if 0
__u32 ipsec_esp_bitmap     = 0;        /**< save session state to detect replays - must be 32 bits.
                                        *   Note: must be initialized with zero (0x00000000) when
                                        *         a new SA is established! */
__u32 ipsec_esp_lastSeq    = 0;        /**< save session state to detect replays
                                        *   Note: must be initialized with zero (0x00000000) when
                                        *         a new SA is established! */
static unsigned char     auth_in[8];   /* AES_GCM output, unused */
static unsigned char     auth_tag[16]; /* AES_GCM output, unused */
#endif
#define PAD_BLOCK_SIZE 16

#if 0
static int ipsec_memcpy_ip_header(char *dest_ptr, const char *src_ptr,
                                  u32 copy_count)
{
    int i, remainder_length, times;

    remainder_length = copy_count % 8;
    times = copy_count / 8;
    for (i = times; i > 0; i--) {
        aos_memcpy(dest_ptr - 8, src_ptr + remainder_length + ((i - 1) * 8), 8);
        dest_ptr = dest_ptr - 8;
    }

    if (remainder_length)
        aos_memcpy(dest_ptr - remainder_length, src_ptr, remainder_length);

    return 0;
}
#endif

/**
 * Returns the number of padding needed for a certain ESP packet size
 *
 * @param    len        the length of the packet
 * @return    the length of padding needed
 */
static __u8 ipsec_esp_get_padding(int len)
{
    int padding;

    for(padding = 0; padding < PAD_BLOCK_SIZE; padding++)
        if(((len+padding) % PAD_BLOCK_SIZE) == 0)
            break ;
    return (__u8)padding;
}

/**
 * Decapsulates an IP packet containing an ESP header.
 *
 * @param    packet     pointer to the ESP header
 * @param    offset     pointer to the offset which is passed back
 * @param    len        pointer to the length of the decapsulated packet
 * @param    sa         pointer to the SA
 * @return   IPSEC_STATUS_SUCCESS     if the packet could be decapsulated properly
 * @return   IPSEC_STATUS_FAILURE     if the SA's authentication algorithm was invalid or if ICV comparison failed
 * @return   IPSEC_STATUS_BAD_PACKET  if the decryption gave back a strange packet
 */
ipsec_status ipsec_esp_decapsulate(struct pbuf *p, int *offset, int *len, sad_entry *sa) {
    int ret_val = IPSEC_STATUS_NOT_INITIALIZED;            /* by default, the return value is undefined */
    __u8                  ip_header_len;
    int                   local_len;
    int                   payload_offset;
    int                   payload_len;
    ipsec_ip_header       *new_ip_packet;
    ipsec_ip6_header      *new_ip6_packet;
    esp_packet            *esp_header;
    char                  cbc_iv[IPSEC_ESP_IV_SIZE];
    __u8                  iv_size = 0;
    unsigned char         digest[IPSEC_AUTH_ICV];
    __u8                  *end_ptr;
    int                   trailer_len = 2; /* at least two bytes: padLen + nextProtocol */
    __u8                  next_proto;
    unsigned char         *packet = p->payload;

    LWIP_DEBUGF(IPSEC_DEBUG,
                ("ipsec_esp_decapsulate: packet=%p, *offset=%d, *len=%d sa=%p",
                (void *)packet, *offset, *len, (void *)sa));

    if (IP_HDR_GET_VERSION(packet) == 6) {
        ip_header_len = sizeof(ipsec_ip6_header);
        esp_header = (esp_packet*)(packet + ip_header_len);
        payload_offset = ip_header_len + IPSEC_ESP_SPI_SIZE + IPSEC_ESP_SEQ_SIZE;
        //payload_len = ipsec_ntohs(((ipsec_ip6_header *)packet)->len) - IPSEC_ESP_HDR_SIZE;
        payload_len = p->len - ip_header_len - IPSEC_ESP_HDR_SIZE;
    } else {
        ip_header_len = (((ipsec_ip_header *)packet)->v_hl & 0x0f) * 4;
        esp_header = (esp_packet*)(packet + ip_header_len);
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: esp_header %p", esp_header));
        payload_offset = ip_header_len + IPSEC_ESP_SPI_SIZE + IPSEC_ESP_SEQ_SIZE;
        payload_len = ipsec_ntohs(((ipsec_ip_header *)packet)->len) - ip_header_len - IPSEC_ESP_HDR_SIZE;
    }
    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: payload_len %d", payload_len));
    if(sa->auth_alg != 0) {
        /* preliminary anti-replay check (without updating the global sequence number window)     */
        /* This check prevents useless ICV calculation if the Sequence Number is obviously wrong  */
#if 1
        ret_val = ipsec_check_replay_window(ipsec_ntohl(esp_header->sequence),
                                            sa->ipsec_esp_lastSeq,
                                            sa->ipsec_esp_bitmap);
        if(ret_val != IPSEC_AUDIT_SUCCESS) {
            LWIP_DEBUGF(IPSEC_DEBUG,
                ("ipsec_esp_decapsulate:packet rejected by anti-replay check (lastSeq=%u, seq=%u, window size=%d)",
                 sa->ipsec_esp_lastSeq, ipsec_ntohl(esp_header->sequence),
                 IPSEC_SEQ_MAX_WINDOW));
            return (enum ipsec_status_list)ret_val;
        }
#else
        if ((htonl(esp_header->sequence) < sa->sequence_number) &&
            !((sa->sequence_number == 1) && (htonl(esp_header->sequence) == 1))) {
            LWIP_DEBUGF(IPSEC_DEBUG,
                ("ipsec_esp_decapsulate: seq err packet seq 0x%x sa seq 0x%x!\n",
                 htonl(esp_header->sequence), sa->sequence_number));

            return IPSEC_STATUS_BAD_PACKET;
        }
#endif
        /* recalcualte ICV */
        switch(sa->auth_alg) {
        case IPSEC_HMAC_SHA1:
            hmac_sha1(sa->authkey, sa->authkey_size, (unsigned char *)esp_header,
                      (payload_len - IPSEC_AUTH_ICV + IPSEC_ESP_HDR_SIZE),
                      (unsigned char *)&digest, IPSEC_AUTH_ICV);
            break;
        case IPSEC_HMAC_MD5:
            hmac_md5(sa->authkey, sa->authkey_size, (unsigned char *)esp_header,
                     (payload_len - IPSEC_AUTH_ICV + IPSEC_ESP_HDR_SIZE),
                     (unsigned char *)&digest, IPSEC_AUTH_ICV);
            break;
        default:
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: unknown HASH algorithm for this ESP"));
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: return = %d", IPSEC_STATUS_FAILURE));
            return IPSEC_STATUS_FAILURE;
        }

        /* compare ICV */
        if(memcmp(((char*)esp_header) + IPSEC_ESP_HDR_SIZE + payload_len - IPSEC_AUTH_ICV,
                  digest, IPSEC_AUTH_ICV) != 0) {
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: ESP ICV does not match"));
            LWIP_ERROR("ipsec_esp_decapsulate: return = -2",
                       0, return IPSEC_STATUS_FAILURE);
            //return IPSEC_STATUS_FAILURE;
        }

        /* reduce payload by ICV */
        payload_len -= IPSEC_AUTH_ICV;
#if 1
        /* post-ICV calculationn anti-replay check (this call will update the global sequence number window) */
        ret_val = ipsec_update_replay_window(ipsec_ntohl(esp_header->sequence),
                                             (__u32 *)&sa->ipsec_esp_lastSeq,
                                             (__u32 *)&sa->ipsec_esp_bitmap);
        if(ret_val != IPSEC_AUDIT_SUCCESS) {
            LWIP_DEBUGF(IPSEC_DEBUG,
                ("ipsec_esp_decapsulate: packet rejected by anti-replay update (lastSeq=%u, seq=%u, window size=%d)",
                sa->ipsec_esp_lastSeq, ipsec_ntohl(esp_header->sequence),
                IPSEC_SEQ_MAX_WINDOW));
            return (enum ipsec_status_list)ret_val;
        }
#else
        sa->sequence_number++;
#endif
        trailer_len += IPSEC_AUTH_ICV;
    }

    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: enc %d", sa->enc_alg));
    /* decapsulate the packet according the SA */
    switch (sa->enc_alg) {
    case IPSEC_AES_128_CBC:
        iv_size = IPSEC_AES_IV_SIZE;
        memcpy(cbc_iv, ((char*)esp_header) + IPSEC_ESP_HDR_SIZE, iv_size);
        aes_128_cbc_decrypt(sa, sa->enckey, (u8 *)cbc_iv,
                            (packet + payload_offset + iv_size), payload_len - iv_size,
                            (packet + payload_offset + iv_size), payload_len - iv_size);
        break;
    case IPSEC_3DES:
        iv_size = IPSEC_3DES_IV_SIZE;
        memcpy(cbc_iv, ((char*)esp_header) + IPSEC_ESP_HDR_SIZE, iv_size);
        des_ede3_cbc_decrypt(sa, sa->enckey, (u8 *)(packet + payload_offset),
                             (packet + payload_offset + iv_size), payload_len - iv_size,
                             (packet + payload_offset + iv_size), payload_len - iv_size);
        break;
    case IPSEC_NULL:
        break;
    default:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: unknown DECRYPT algorithm for this ESP"));
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: return = %d", IPSEC_STATUS_FAILURE));
        return IPSEC_STATUS_FAILURE;
    }

    /* now that everything's decrypted, shift everything to the right
    to overwrite the trailer and ICV. It's the caller's responsibility to call
    pbuf_remove_header to get rid of the ESP space overhead */
    end_ptr = ((__u8*)packet + payload_offset) + payload_len;
    next_proto = *(end_ptr - 1);
    trailer_len += *(end_ptr - 2); /* add pad size */

    *offset = payload_offset + IPSEC_ESP_IV_SIZE;

    if (sa->mode == IPSEC_TUNNEL) {
        if (sa->protocol_type == IPV4){
            new_ip_packet = (ipsec_ip_header*)(packet + *offset);
            local_len = ipsec_ntohs(new_ip_packet->len);
        } else {
            new_ip6_packet = (ipsec_ip6_header*)(packet + *offset);
            local_len = ipsec_ntohs(new_ip6_packet->len);
        }
        if ((local_len < IPSEC_MIN_IPHDR_SIZE) || (local_len > IPSEC_MTU)) {
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: decapsulated strange packet")) ;
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: return = %d", IPSEC_STATUS_BAD_PACKET) );
            return IPSEC_STATUS_BAD_PACKET;
        }
        *len = local_len;
    } else {
        if (sa->protocol_type == IPV4){
            /* set layer 4 protocol */
            ((ipsec_ip_header*)packet)->protocol = next_proto;
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: next_proto %d", next_proto));

            /* overwrite old ESP header and IV */
            new_ip_packet = (ipsec_ip_header*)(packet + IPSEC_ESP_HDR_SIZE + iv_size);
            memmove(new_ip_packet, packet, IPSEC_MIN_IPHDR_SIZE);
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: proto %d", new_ip_packet->protocol));

            /* len is same as old len minus all of the ESP overhead */
            *len = ipsec_ntohs(new_ip_packet->len) - IPSEC_ESP_HDR_SIZE - iv_size - trailer_len;
            new_ip_packet->len = ipsec_htons(*len);

            /* recalculate ip checksum */
            new_ip_packet->chksum = 0;
            new_ip_packet->chksum = ipsec_ip_chksum(new_ip_packet, IPSEC_MIN_IPHDR_SIZE);

            *len += trailer_len;
        } else {
            ((ipsec_ip6_header*)packet)->nexthdr = next_proto;
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: next_proto %d", next_proto));
            new_ip6_packet = (ipsec_ip6_header*)(packet + IPSEC_ESP_HDR_SIZE + iv_size);
            memmove(new_ip6_packet, packet, IPSEC_MIN_IP6HDR_SIZE);
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: proto %d", new_ip6_packet->nexthdr));
            /* len is same as old len minus all of the ESP overhead */
            *len = p->len - IPSEC_MIN_IP6HDR_SIZE - IPSEC_ESP_HDR_SIZE - iv_size - trailer_len;
            new_ip6_packet->len = ipsec_htons(*len);

            *len = *len + IPSEC_MIN_IP6HDR_SIZE + trailer_len;
        }
    }

    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: p->len %d len %d", p->len, *len));
    if (pbuf_remove_header(p, p->len - *len) != 0) {
        LWIP_DEBUGF(IPSEC_DEBUG,
            ("ipsecdev_input: failed to remove IPSec header"));
        pbuf_free(p);
    }
    p->len -= trailer_len;
    p->tot_len -= trailer_len;

    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_decapsulate: return = %d", IPSEC_STATUS_SUCCESS));
    return IPSEC_STATUS_SUCCESS;
 }

/**
 * Encapsulates an IP packet into an ESP packet which will again be added to an IP packet.
 *
 * @param    packet        pointer to the IP packet
 * @param    offset        pointer to the offset which will point to the new encapsulated packet
 * @param    len           pointer to the length of the new encapsulated packet
 * @param    sa            pointer to the SA
 * @param    src_addr      source IP address of the outer IP header
 * @param    dest_addr     destination IP address of the outer IP header
 * @return   IPSEC_STATUS_SUCCESS        if the packet was properly encapsulated
 * @return   IPSEC_STATUS_TTL_EXPIRED    if the TTL expired
 * @return   IPSEC_STATUS_FAILURE        if the SA contained a bad authentication algorithm
 */
ipsec_status ipsec_esp_encapsulate(struct pbuf *p, struct pbuf *esp_p, int *offset,
                                   int *len, sad_entry *sa, __u32 *src_addr,
                                   __u32 *dest_addr)
{
    __u8                tos = 0;
    __u16               inner_len;
    __u16               payload_len;
    __u8                padd_len;
    __u8                *pos;
    __u8                padd;
    __u8                iv_size = 0;
    ipsec_ip_header     *new_ip_header = NULL;
    ipsec_ip6_header    *new_ip6_header = NULL;
    ipsec_esp_header    *new_esp_header = NULL;
    unsigned char       digest[IPSEC_AUTH_ICV];
    char                cbc_iv[IPSEC_ESP_IV_SIZE];
    __u8                shift_len;
    char                *ori_ptr = (char *)esp_p->payload;
    char                *esp_packet = (char *)esp_p->payload;
    char                *packet = (char *)p->payload;
    int                 space_overhead = 0;

    LWIP_DEBUGF(IPSEC_DEBUG,
        ("ipsec_esp_encapsulate: packet=%p, *offset=%d, *len=%d, sa=%p, src_addr=%u, dest_addr=%u",
        (void *)packet, *offset, *len, (void *)sa, *src_addr, *dest_addr));

    //aic_network_cpcap_dump((u8 *)p->payload, p->len);
    space_overhead = sa->protocol ==
                IPSEC_PROTO_AH ? IPSEC_MAX_AH_HLEN - IPSEC_MIN_IP6HDR_SIZE : IPSEC_MAX_ESP_HLEN - IPSEC_MIN_IP6HDR_SIZE;
    if (pbuf_add_header(esp_p, space_overhead) != 0) {
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: failed to add space for ESP data to pbuf"));
        return IPSEC_STATUS_FAILURE;
    }

    if (IP_HDR_GET_VERSION(packet) == 4) {
        /* save TOS from inner header */
        tos = ((ipsec_ip_header *)packet)->tos;

        if (((ipsec_ip_header *)packet)->ttl == 0) {
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: return = %d",
                        IPSEC_STATUS_TTL_EXPIRED));
            return IPSEC_STATUS_TTL_EXPIRED;
        }

        /* the "encryptable" portion of the packet */
        inner_len = ipsec_ntohs(((ipsec_ip_header *)packet)->len);
        if (sa->mode == IPSEC_TRANSPORT)
            inner_len -= IPSEC_MIN_IPHDR_SIZE; /* can't encrypt the orig IP header in TRANSPORT mode */
    } else {
        inner_len = ipsec_ntohs(((ipsec_ip6_header *)packet)->len) + IPSEC_MIN_IP6HDR_SIZE;
        if (sa->mode == IPSEC_TRANSPORT)
            inner_len -= IPSEC_MIN_IP6HDR_SIZE; /* can't encrypt the orig IP header in TRANSPORT mode */
    }

    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: inner_len %d", inner_len));
    switch (sa->enc_alg) {
    case IPSEC_3DES:
        iv_size = IPSEC_3DES_IV_SIZE;
        break;
    case IPSEC_AES_128_CBC:
        iv_size = IPSEC_AES_IV_SIZE;
        break;
    case IPSEC_NULL:
        iv_size = IPSEC_NULL_IV_SIZE;
        break;
    default:
        return IPSEC_STATUS_FAILURE;
    }

    /* compute the required padding */
    padd_len = ipsec_esp_get_padding(inner_len + 2);    /* +2 b/c padLength byte and nextHeader byte */

    /* shift the packet contents to the left to make room for the ESP trailer and (optional) ESP ICV */
    shift_len = padd_len + 2;
    if (sa->auth_alg != 0)
        shift_len += IPSEC_AUTH_ICV;

    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: shift_len %d", shift_len));
    /* set new packet header pointers */
    if (sa->mode == IPSEC_TUNNEL) {
        new_ip_header = (ipsec_ip_header*)(packet - iv_size - IPSEC_ESP_HDR_SIZE - IPSEC_MIN_IPHDR_SIZE);
        new_esp_header = (ipsec_esp_header*)(packet - iv_size - IPSEC_ESP_HDR_SIZE);
    } else if (sa->mode == IPSEC_TRANSPORT) {
        switch (sa->protocol_type) {
        case IPV4:
            memcpy(esp_packet - shift_len, packet, ipsec_ntohs(((ipsec_ip_header *)packet)->len));
            esp_packet = esp_packet - shift_len;
            new_ip_header = (ipsec_ip_header*)(esp_packet - iv_size - IPSEC_ESP_HDR_SIZE);
            new_esp_header = (ipsec_esp_header*)(esp_packet - iv_size - IPSEC_ESP_HDR_SIZE + IPSEC_MIN_IPHDR_SIZE);
            /* move orig ip header to the new location, making room for ESP HDR and IV */
            memcpy((unsigned char *)new_ip_header, packet, IPSEC_MIN_IPHDR_SIZE);
            /* packet should point at the data that's to be encrypted. In TRANSPORT
            mode, this is the transport layer data */
            esp_packet = esp_packet + IPSEC_MIN_IPHDR_SIZE;
            p->reserved = ipsec_ntohs(*((u16 *)esp_packet));
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: src port %d", p->reserved));
            break;
        case IPV6:
            memcpy(esp_packet - shift_len, packet,
                   ipsec_ntohs(((ipsec_ip6_header *)packet)->len) + IPSEC_MIN_IP6HDR_SIZE);
            esp_packet = esp_packet - shift_len;
            new_ip6_header = (ipsec_ip6_header*)(esp_packet - iv_size - IPSEC_ESP_HDR_SIZE);
            new_esp_header = (ipsec_esp_header*)(esp_packet - iv_size - IPSEC_ESP_HDR_SIZE + IPSEC_MIN_IP6HDR_SIZE);
            memcpy((unsigned char *)new_ip6_header, packet, IPSEC_MIN_IP6HDR_SIZE);
            esp_packet = esp_packet + IPSEC_MIN_IP6HDR_SIZE;
            p->reserved = ipsec_ntohs(*((u16 *)esp_packet));
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: src port %d", p->reserved));
            break;
        }
    } else {
        return IPSEC_STATUS_FAILURE;
    }

    /* add padding if needed */
    pos = ((unsigned char*)esp_packet) + inner_len;
    if (padd_len != 0) {
        padd = 1;
        while(padd <= padd_len)
            *pos++ = padd++;
    }

    /* append padding length and next protocol field to the payload */
    *pos++ = padd_len;

    /* in tunnel mode the next protocol field is always IP */
    if (sa->protocol_type == IPV4)
        *pos = sa->mode == IPSEC_TUNNEL ? 0x04 : new_ip_header->protocol;
    else
        *pos = sa->mode == IPSEC_TUNNEL ? 0x04 : new_ip6_header->nexthdr;

    payload_len = inner_len + IPSEC_ESP_HDR_SIZE + iv_size + padd_len + 2;

    ipsec_init_iv(sa, (__u32 *)cbc_iv, iv_size);
    /* insert IV in fron of packet */
    memcpy(((char*)esp_packet) - iv_size, cbc_iv, iv_size);
    /* decapsulate the packet according the SA */
    if(sa->enc_alg != 0) {
        switch(sa->enc_alg) {
        case IPSEC_AES_128_CBC:
            aes_128_cbc_encrypt(sa, sa->enckey, cbc_iv, (__u8 *)esp_packet,
                                inner_len + padd_len + 2, (__u8 *)esp_packet,
                                inner_len + padd_len + 2);
            break;
        case IPSEC_3DES:
            des_ede3_cbc_encrypt(sa, sa->enckey, cbc_iv, (__u8 *)esp_packet,
                                 inner_len + padd_len + 2, (__u8 *)esp_packet,
                                 inner_len + padd_len + 2);
            break;
        case IPSEC_NULL:
            break;
        default:
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: unknown ENCRYPT algorithm for this ESP"));
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: return = %d", IPSEC_STATUS_FAILURE));
            return IPSEC_STATUS_FAILURE;
        }
    }

    /* setup ESP header */
    new_esp_header->spi = ipsec_htonl(sa->spi);
    /** 1st packet needs to be sent out with squ = 1 */
    sa->sequence_number++;
    new_esp_header->sequence_number = ipsec_htonl(sa->sequence_number);

    /* calculate the ICV if needed */
    if(sa->auth_alg != 0) {
        /* recalcualte ICV */
        switch(sa->auth_alg) {
            case IPSEC_HMAC_SHA1:
                hmac_sha1(sa->authkey, sa->authkey_size,
                          (unsigned char *)new_esp_header, payload_len,
                          (unsigned char *)&digest, IPSEC_AUTH_ICV);
                break;
            case IPSEC_HMAC_MD5:
                hmac_md5(sa->authkey, sa->authkey_size,
                         (unsigned char *)new_esp_header, payload_len,
                         (unsigned char *)&digest, IPSEC_AUTH_ICV);
                break;
            default:
                LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: unknown HASH algorithm for this ESP"));
                LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: return = %d", IPSEC_STATUS_FAILURE));
                return IPSEC_STATUS_FAILURE;
        }

        /* set ICV */
        memcpy(((char*)new_esp_header) + payload_len, digest, IPSEC_AUTH_ICV);

        /* increase payload by ICV */
        payload_len += IPSEC_AUTH_ICV;
    }

    /* TUNNEL mode requires special handling since we're setting up a brand new
    IP header (as opposed to using the original one that was copied earlier) */
    if (sa->mode == IPSEC_TUNNEL) {
        if (sa->protocol_type == IPV4) {
            /* set ip header fields */
            new_ip_header->len = ipsec_htons(payload_len + IPSEC_MIN_IPHDR_SIZE);
            new_ip_header->protocol = IPSEC_PROTO_ESP;
            new_ip_header->v_hl = 0x45;
            new_ip_header->tos = tos;
            new_ip_header->id = 1000;    /**@todo id must be generated properly and incremented */
            new_ip_header->offset = 0;
            new_ip_header->ttl = 64;
            new_ip_header->chksum = 0;
            memcpy(&new_ip_header->src, &src_addr, sizeof(new_ip_header->src));
            memcpy(&new_ip_header->dest, &dest_addr, sizeof(new_ip_header->dest));
            /* set checksum */
            new_ip_header->chksum =
                ipsec_ip_chksum(new_ip_header, sizeof(ipsec_ip_header));
        } else {
            new_ip6_header->version = 0x60;
            memset(new_ip6_header->flow_lbl, 0, 3);
            new_ip6_header->len = ipsec_htons(payload_len);
            new_ip6_header->nexthdr = IPSEC_PROTO_ESP;
            new_ip6_header->hop_limit = 255;
            memcpy(&new_ip6_header->src, &src_addr, sizeof(new_ip6_header->src));
            memcpy(&new_ip6_header->dest, &dest_addr, sizeof(new_ip6_header->dest));
        }
    }else {
        if (sa->protocol_type == IPV4){
            new_ip_header->protocol = IPSEC_PROTO_ESP;
            new_ip_header->chksum = 0;
            new_ip_header->len = ipsec_htons(payload_len + IPSEC_MIN_IPHDR_SIZE);
            new_ip_header->chksum =
                ipsec_ip_chksum(new_ip_header, sizeof(ipsec_ip_header));
        } else {
            new_ip6_header->nexthdr = IPSEC_PROTO_ESP;
            new_ip6_header->len = ipsec_htons(payload_len);
        }
    }

    /* setup return values */
    if (sa->protocol_type == IPV4) {
        *offset = (char*)new_ip_header - (char*)ori_ptr;
        *len = payload_len + IPSEC_MIN_IPHDR_SIZE;
    } else {
        *offset = (char*)new_ip6_header - (char*)ori_ptr;
        *len = payload_len + IPSEC_MIN_IP6HDR_SIZE;
    }

    /* remove unused space (likely to occur in ESP mode due to variable padding.)*/
    space_overhead = (ori_ptr + *offset) - (char*)esp_p->payload;
    if (pbuf_remove_header(esp_p, space_overhead) != 0) {
        LWIP_DEBUGF(IPSEC_DEBUG | LWIP_DBG_TRACE,
            ("ipsec_esp_encapsulate: failed to remove unnecessary IPSec space"));
    }

    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_esp_encapsulate: return = %d", IPSEC_STATUS_SUCCESS));
    return IPSEC_STATUS_SUCCESS;
 }

#endif /* defined(EIPS) && EIPS == 1 */
