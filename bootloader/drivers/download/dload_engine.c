/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file include download state machine enter fucntion.
 */

#include <stdlib.h>
#include "download/dload_engine.h"
#include "download/dload_kit.h"
#include "download/dload_packet.h"
#include "console/console.h"
#ifdef CONFIG_DL
#include <fmd.h>
#include <flash_mem_config.h>
#include <psram_mem_config.h>
#endif

static const char reply_sync[] = "mdl v1.0";
__attribute__((aligned(32))) uint8_t
original_recv_buffer[HDLC_MAX_FRAME_LENGTH];
__attribute__((aligned(32))) uint8_t
 opr_buf[HDLC_MAX_FRAME_LENGTH];

extern u32 psram_get_dev_size(void);

void dload_recv_engine(void)
{

    uint16_t crc16_value, crc16_recv;
    struct packet_body *pkt;
    uint32_t dl_type;
    int ori_offset;
    uint8_t ch;
    int len, cnt = 0;

    /* wait host send hdlc handshake char*/
    while (1) {
        ch = cur_dl_chan->get_char_blocking(cur_dl_chan->data);
        if (ch == 0x7E) {
            cnt++;
            if(cnt < 3)
                continue;
#ifdef CONFIG_DL
            if (fmd_get_total_size(0) < FLASH0_TOTAL_SIZE) {
                send_ack_packet(PCK_PHONE_FLASH_DENSITY_NOT_ENOUGH);
                continue;
            }

            if (psram_get_dev_size() < PSRAM_REGION_SIZE) {
                send_ack_packet(PCK_PHONE_PSRAM_DENSITY_NOT_ENOUGH);
                continue;
            }
#endif
            pkt = (struct packet_body *)opr_buf;
            pkt->type = PCK_REP_VER;
            pkt->size = strlen(reply_sync);
            memcpy(pkt->content, reply_sync, sizeof(reply_sync) - 1);
            send_packet(pkt);
            break;
        }
    }

    dl_type = platform_get_dl_type();
    while (1) {
        memset(original_recv_buffer, 0, HDLC_MAX_FRAME_LENGTH * sizeof(uint8_t));
        memset(opr_buf, 0, HDLC_MAX_FRAME_LENGTH * sizeof(uint8_t));

#ifndef CONFIG_UNPACK_OBO
        if ((dl_type == DL_USB) || (dl_type == DL_SPI) || (dl_type == DL_UART0 )||(dl_type == DL_UART1)) {
            len = cur_dl_chan->read(cur_dl_chan->data, original_recv_buffer, HDLC_MAX_FRAME_LENGTH);
            if (!len)
                continue;
#ifdef HDLC
            len = hdlc_unpack((char *)opr_buf, (char *)original_recv_buffer, len, &crc16_value, &ori_offset);
#else
            len = non_escape_unpack((char *)opr_buf, (char *)original_recv_buffer, len, &crc16_value, &ori_offset);
#endif

#else
        if (dl_type == DL_USB) {
            len = cur_dl_chan->read(cur_dl_chan->data, original_recv_buffer, HDLC_MAX_FRAME_LENGTH);
            if (!len)
                continue;
#ifdef HDLC
            len = hdlc_unpack((char *)opr_buf, (char *)original_recv_buffer, len, &crc16_value, &ori_offset);
#else
            len = non_escape_unpack((char *)opr_buf, (char *)original_recv_buffer, len, &crc16_value, &ori_offset);
#endif
        } else if ((dl_type == DL_UART0 )||(dl_type == DL_UART1 || (dl_type == DL_SPI))) {
#ifdef HDLC
            len = hdlc_unpack_obo((char *)opr_buf, (char *)original_recv_buffer, &crc16_value, &ori_offset);
#else
            len = non_escape_unpack_obo((char *)opr_buf, (char *)original_recv_buffer, &crc16_value, &ori_offset);
#endif

#endif
        } else {
            log_print("Unknow download channel type!\n");
            return;
        }

        if (len <= 0)
            continue;

        crc16_recv = get_hdlc_packet_crc16(opr_buf, len);
#ifndef HDLC
        crc16_recv = EndianConv_16(crc16_recv);
#endif
        if (crc16_value != crc16_recv) {
            log_print("pkt crc val:0x%x crc16_recv:0x%x crc no match, retransmission!!!\n", crc16_value, crc16_recv);
            send_ack_packet(PCK_REP_EEROR_CRC);
            continue;
        }

        pkt = (struct packet_body *)opr_buf;
#ifdef HDLC
        pkt->size = EndianConv_16(pkt->size);
        pkt->type = EndianConv_16(pkt->type);
#endif
        dl_cmd_operate(pkt->type, pkt);
    }
}
