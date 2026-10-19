/*****************************************************************************/
/*                                                                           */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is use for external packet operate interface.
 */

#ifndef __DLOAD_PACKET_H__
#define __DLOAD_PACKET_H__
#include "download/dload_cmd.h"

int setup_packet(packet_body *pkt_src, packet_body *pkt_dest);
void send_packet(packet_body *packet_ptr);
void send_ack_packet(CMD_TYPE pkt_type);
int hdlc_pack(char *dest,  char *src, int size);
int hdlc_unpack(char *dest, char *src, int size, unsigned short *crc16_value, int *ori_offset);
int hdlc_unpack_obo(char *dest, char *src, unsigned short *crc16_value, int *ori_offset);
int non_escape_unpack(char *dest, char *src, int size, unsigned short *crc16_value, int *ori_offset);
int non_escape_unpack_obo(char *dest, char *src, unsigned short *crc16_value, int *ori_offset);
int non_escape_setup_packet(packet_body *pkt_src, packet_body *pkt_dest);

#endif /*__DLOAD_PACKET_H__*/
