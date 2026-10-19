/*****************************************************************************/
/*                                                                           */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is download kit functions.Such as little-end and big-end conversion,
 *    hdlc packet crc caculate..
 */

#include "download/dload_kit.h"
#include "console/console.h"

unsigned short EndianConv_16(unsigned short val)
{
    /*little endian*/
    return (val >> 8 | val << 8);
}

unsigned int EndianConv_32(unsigned int val)
{
    unsigned int tmp = 0;

    tmp = (val >> 24 | val << 24);
    tmp |= ((val >> 8) & 0x0000FF00);
    tmp |= ((val << 8) & 0x00FF0000);
    return tmp;
}

unsigned short get_hdlc_packet_crc16(uint8_t *src, int size)
{
    unsigned short crc16 = 0;

    crc16 = (src[size - 2] << 8) | src[size - 1];
    return crc16;
}

unsigned int crc_16_l_calc(unsigned char *buf_ptr, unsigned int len)
{
    unsigned short crc = 0;
    unsigned int i;

    while (len-- != 0) {
        for (i = CRC_16_L_SEED; i != 0; i = i >> 1) {
            if ((crc & CRC_16_L_POLYNOMIAL) != 0) {
                crc = crc << 1;
                crc = crc ^ CRC_16_POLYNOMIAL;
            } else {
                crc = crc << 1;
            }

            if ((*buf_ptr & i) != 0)
                crc = crc ^ CRC_16_POLYNOMIAL;
        }

        buf_ptr++;
    }

    return (crc);
}

unsigned short checksum_calc (const unsigned short *src, int len)
{
    unsigned int sum = 0;

    while (len > 3)
    {
        sum += *src++;
        sum += *src++;
        len -= 4;
    }

    switch (len&0x03)
    {
        case 2:
            sum += *src++;
            break;
        case 3:
            sum += *src++;
            sum += * ( (unsigned char *) src);
            break;
        case 1:
            sum += * ( (unsigned char *) src);
            break;
        default:
            break;
    }

    sum = (sum >> 16) + (sum & 0x0FFFF);
    sum += (sum >> 16);
    return (~sum);
}
