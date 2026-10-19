/*****************************************************************************/
/*                                                                           */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is download packet operate fucntions.Such as hdlc pack/unpack,
 *    hdlc escape, ack packet send...
 */

#include "download/dload_packet.h"
#include "download/dload_kit.h"
#include "aic_errno.h"
static packet_body operate_packet;

static void send_packet_to_pc(const void *buf, int len)
{
    len = cur_dl_chan->write(cur_dl_chan->data, (uint8_t *)buf, len);
}

int setup_packet(packet_body *pkt_src, packet_body *pkt_dest)
{
    unsigned short crc, size;
    int write_len;  /*orginal length*/
    int send_len;   /*length after encode*/
    int i;
    uint8_t curval;
    uint8_t *des_ptr = NULL;
    uint8_t *src_ptr = NULL;

    size = pkt_src->size;
    write_len = size + sizeof(unsigned short) + PKT_HEADER_SIZE;
    src_ptr = (uint8_t *)pkt_src;

    pkt_src->size = EndianConv_16(pkt_src->size);
    pkt_src->type = EndianConv_16(pkt_src->type);

    /*src CRC calculation*/
    crc = crc_16_l_calc(
              (unsigned char *)pkt_src,
              size + PKT_HEADER_SIZE);
    crc = EndianConv_16(crc);

    pkt_src->content[size] = (crc >> 8) & 0xFF;
    pkt_src->content[size+1] = (crc) & 0xFF;

    /*
     *des data preparation
     */

    des_ptr = (uint8_t *)pkt_dest;
    /*head flag*/
    *(des_ptr++) = HDLC_FLAG;
    send_len = 1;

    /*middle part process*/
    for (i = 0; i < write_len; i++) {
        curval = *(src_ptr + i);

        if ((curval == HDLC_FLAG) || (curval == HDLC_ESCAPE)) {
            *(des_ptr++) = HDLC_ESCAPE;
            *(des_ptr++) = ~HDLC_ESCAPE_MASK & curval;
            send_len++;
        } else {
            *(des_ptr++) = curval;
        }
        send_len++;
    }

    /*end flag*/
    *(des_ptr++) = HDLC_FLAG;
    send_len++;

    return send_len;
}

void send_packet(packet_body *packet_ptr)
{
    packet_body *tmp_packet_ptr;
    int send_len;

    tmp_packet_ptr = &operate_packet;
    memset(tmp_packet_ptr, 0, sizeof(packet_body));

#ifdef HDLC
    send_len = setup_packet(packet_ptr, tmp_packet_ptr);
#else
    send_len = non_escape_setup_packet(packet_ptr, tmp_packet_ptr);
#endif
    send_packet_to_pc((char *)tmp_packet_ptr, send_len);
}

void send_ack_packet(CMD_TYPE pkt_type)
{
    packet_body *tmp_packet_ptr = NULL;
    unsigned long ack_packet_src[8];
    unsigned long ack_packet_dst[8];
    packet_body *packet_ptr;
    int send_len;

    packet_ptr = (packet_body *) ack_packet_src;

    packet_ptr->type = pkt_type;
    packet_ptr->size = 0;

    tmp_packet_ptr = (packet_body *)ack_packet_dst;

#ifdef HDLC
    send_len = setup_packet(packet_ptr, tmp_packet_ptr);
#else
    send_len = non_escape_setup_packet(packet_ptr, tmp_packet_ptr);
#endif

    send_packet_to_pc((char *)(tmp_packet_ptr), send_len);
}

int hdlc_pack(char *dest,  char *src, int size)
{
    int translated_size = 0;
    int i;

    dest[translated_size++] = 0x7E;

    for (i = 0; i < size; i++) {
        if (src[i] == 0x7E) {
            dest[translated_size++] = 0x7D;
            dest[translated_size++] = 0x5E;
        } else if (src[i] == 0x7D) {
            dest[translated_size++] = 0x7D;
            dest[translated_size++] = 0x5D;
        } else {
            dest[translated_size++] = src[i];
        }
    }

    dest[translated_size++] = 0x7E;

    return translated_size;
}

int hdlc_unpack(char *dest, char *src, int size, unsigned short *crc16_value, int *ori_offset)
{
    int translated_size = 0;
    unsigned short crc16;
    int flag = 0;
    int i;

    for (i = 0; i < size; i++) {
        switch (flag) {
        case REP_PACKET_TYPE_0:
            if (src[i] == 0x7e)
                flag = REP_PACKET_TYPE_1;
            break;
        case REP_PACKET_TYPE_1:
            if (src[i] != 0x7e) {
                flag = REP_PACKET_TYPE_2;
                dest[translated_size++] = src[i];
            }
            break;
        case REP_PACKET_TYPE_2:
            if (src[i] == 0x7E) {
                crc16 = crc_16_l_calc((unsigned char *)dest, (unsigned int)(translated_size - 2));
                *crc16_value = crc16;
                *ori_offset = translated_size - 2;
                return translated_size;
            } else if (src[i - 1] == 0x7D) {
                if (src[i] == 0x5E)
                    dest[translated_size - 1] = 0x7E;
                else if (src[i] == 0x5D)
                    dest[translated_size - 1] = 0x7D;
            } else {
                dest[translated_size++] = src[i];
            }
            break;
        }
    }
    return translated_size;
}

int hdlc_unpack_obo(char *dest, char *src, unsigned short *crc16_value, int *ori_offset)
{
    int translated_size = 0;
    unsigned short crc16;
    int rec_ch = 0;
    int flag = 0;
    int i = 0;

    while (1) {
        rec_ch = cur_dl_chan->get_char_blocking(cur_dl_chan->data);
        if (rec_ch == -1) {
            return rec_ch;
        }
        src[i] = rec_ch & 0xff;

        switch (flag) {
        case REP_PACKET_TYPE_0:
            if (src[i] == 0x7e)
                flag = REP_PACKET_TYPE_1;
            break;
        case REP_PACKET_TYPE_1:
            if (src[i] != 0x7e) {
                flag = REP_PACKET_TYPE_2;
                dest[translated_size++] = src[i];
            }
            break;
        case REP_PACKET_TYPE_2:
            if (src[i] == 0x7E) {
                crc16 = crc_16_l_calc((unsigned char *)dest, (unsigned int)(translated_size - 2));
                *crc16_value = crc16;
                *ori_offset = translated_size - 2;
                return translated_size;
            } else if (src[i - 1] == 0x7D) {
                if (src[i] == 0x5E)
                    dest[translated_size - 1] = 0x7E;
                else if (src[i] == 0x5D)
                    dest[translated_size - 1] = 0x7D;
            } else {
                dest[translated_size++] = src[i];
            }
            break;
        }
        i++;
    }
}

static const char non_escape_header[] = {0x9F, 0x8E, 0x6D, 0x5C};
#define NON_ESCAPE_HEADER_LEN (sizeof(non_escape_header) / sizeof(non_escape_header[0]))

int non_escape_unpack(char *dest, char *src, int size, unsigned short *crc16_value, int *ori_offset)
{
    unsigned short crc16, context_size;
    int translated_size = 0;

    context_size = *((unsigned short *)(src + NON_ESCAPE_HEADER_LEN + 2));
    translated_size = context_size + 4 + 2;

    if (memcmp(src, non_escape_header, NON_ESCAPE_HEADER_LEN))
        return AIC_EPERM;

    memcpy(dest, src + NON_ESCAPE_HEADER_LEN, translated_size);

    crc16 = checksum_calc((unsigned short *)dest, translated_size - 2);
    *crc16_value = crc16;
    *ori_offset = translated_size - 2;

    return translated_size;
}

int non_escape_unpack_obo(char *dest, char *src, unsigned short *crc16_value, int *ori_offset)
{
    unsigned short crc16, context_size = 0;
    int translated_size = 0;
    int i = 0, j = 0;
    int rec_ch = 0;
    int flag = 0;

    while(1) {
        rec_ch = cur_dl_chan->get_char_blocking(cur_dl_chan->data);
        if (rec_ch == -1) {
            return rec_ch;
        }
        src[i] = rec_ch & 0xff;
        if((src[i] != non_escape_header[0]) && (i == 0))
            continue;

        switch (flag) {
        case REP_PACKET_TYPE_0://non escape check
            j++;
            if (j == NON_ESCAPE_HEADER_LEN)
                if(!memcmp(&src[0], non_escape_header, NON_ESCAPE_HEADER_LEN)) {
                    flag = REP_PACKET_TYPE_2;
                    j = 0;
                }
            break;
        case REP_PACKET_TYPE_2://type & size check
            j++;
            dest[translated_size++] = src[i];
            if (j == PKT_HEADER_SIZE) {
                context_size = *(unsigned short *)(dest + 2);
                flag = REP_PACKET_TYPE_3;
                j = 0;
            }
            break;

        case REP_PACKET_TYPE_3:
            j++;
            dest[translated_size++] = src[i];
            if (j == (context_size + 2))
                flag = REP_PACKET_TYPE_4;
            break;
        case REP_PACKET_TYPE_4:
            crc16 = checksum_calc((unsigned short *)dest, (unsigned int)(translated_size - 2));//crc_16_l_calc((unsigned char *)dest, (unsigned int)(translated_size - 2));
            *crc16_value = crc16;
            *ori_offset = translated_size - 2;
            return translated_size;
            break;
        }
        i++;
    }
}

int non_escape_setup_packet(packet_body *pkt_src, packet_body *pkt_dest)
{
    uint8_t *des_ptr = NULL;
    uint8_t *src_ptr = NULL;
    uint16_t crc, size;
    int write_len;  /*orginal length*/
    int send_len;   /*length after encode*/

    size = pkt_src->size;
    write_len = size + sizeof(unsigned short) + PKT_HEADER_SIZE;
    src_ptr = (uint8_t *)pkt_src;

    //pkt_src->size = EndianConv_16(pkt_src->size);
    //pkt_src->type = EndianConv_16(pkt_src->type);

    /*src CRC calculation*/
    crc = checksum_calc(
              (unsigned short *)pkt_src,
              size + PKT_HEADER_SIZE);

    crc = EndianConv_16(crc);

    pkt_src->content[size] = (crc >> 8) & 0xFF;
    pkt_src->content[size+1] = (crc) & 0xFF;

    /*
     *des data preparation
     */

    des_ptr = (uint8_t *)pkt_dest;
    /*head flag*/
    memcpy(des_ptr, non_escape_header, NON_ESCAPE_HEADER_LEN);
    des_ptr += NON_ESCAPE_HEADER_LEN;
    send_len = NON_ESCAPE_HEADER_LEN;

    /*middle part process*/
    memcpy(des_ptr, src_ptr, write_len);
    des_ptr += write_len;
    send_len += write_len;

    /*end flag*/
    *(des_ptr++) = HDLC_FLAG;
    send_len++;

    return send_len;
}
