/*****************************************************************************/
/*                                                                           */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is use for external supply download kit interface.
 */

#ifndef __DLOAD_KIT_H__
#define __DLOAD_KIT_H__
#include <stddef.h>
#include <stdint.h>
#include "download/download.h"
#include "platform/platform.h"

#define CRC_16_POLYNOMIAL 0x1021
#define CRC_16_L_POLYNOMIAL 0x8000
#define CRC_16_L_SEED 0x80
#define CRC_16_L_OK 0x00

unsigned short EndianConv_16(unsigned short val);
unsigned int EndianConv_32(unsigned int val);
unsigned short get_hdlc_packet_crc16(uint8_t *src, int size);
unsigned int crc_16_l_calc(unsigned char *buf_ptr, unsigned int len);
unsigned short checksum_calc (const unsigned short *src, int len);

#endif /*__DLOAD_KIT_H__*/
