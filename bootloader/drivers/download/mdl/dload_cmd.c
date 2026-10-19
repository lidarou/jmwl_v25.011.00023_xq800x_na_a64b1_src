/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file include download command callback handler.
 */
#include <aic_errno.h>
#include <stdlib.h>
#include "download/dload_cmd.h"
#include "download/dload_kit.h"
#include "download/dload_packet.h"
#include "console/console.h"
#include "initcall.h"
#include "platform/boot.h"

typedef int (*cmd_operate) (packet_body *);

static DLOAD_CONDITION dload_file;

extern void dcache_clean_all(void);
cmd_operate op_table[PCK_CMD_TYPE_MAX - PCK_CMD_TYPE_MIN] = {0, 0};

#define CMD_INDEX(cmd)                ((cmd) - PCK_CMD_TYPE_MIN)
#define CMD_HANDLER(cmd)               (op_table[CMD_INDEX(cmd)])

/*data channel connect packet callback*/
int packet_connect(struct packet_body *pkt)
{
    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_start(struct packet_body *pkt)
{
    unsigned int *data = (unsigned int *)(pkt->content);
    unsigned int start_addr = *data;
    unsigned int file_size  = *(data + 1);

#ifdef HDLC
    start_addr = EndianConv_32(start_addr);//start_addr = 0x80004000;//EndianConv_32(start_addr);
    file_size  = EndianConv_32(file_size);
#endif

    log_print("start_addr:0x%x file_size:0x%x\n", start_addr, file_size);
    platform.next_img = (uint8_t *)(start_addr - IMAGE_HEADER_SIZE);

    /*add psram size check later, void access violation, will complete later*/
    dload_file.start_address = (unsigned int)platform.next_img;//security header offset
    dload_file.total_size = file_size;
    dload_file.recv_size = 0;
    dload_file.next_address = dload_file.start_address;

    /*memset/erase psram/norflash before write packet, will complete later*/
    memset((void *)start_addr, 0, file_size);

    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_change_baud(struct packet_body *pkt)
{
    uint32_t *data = (uint32_t *)pkt->content;
    uint32_t baudrate;

    baudrate = *data;

    log_print("dload uart baudrate set:%d\n", baudrate);
    if(cur_dl_chan->baudrate_set) {
        send_ack_packet(PCK_REP_ACK);
        cur_dl_chan->baudrate_set(cur_dl_chan->data, baudrate);
    } else {
        log_print("current channel don't support change baudrate!\n");
        send_ack_packet(PCK_REP_INVALID_CMD);
    }

    return 0;
}

/*middle packet callback*/
int packet_middle(struct packet_body *pkt)
{
    unsigned short data_len = pkt->size;

    memcpy((void *)dload_file.next_address, (const void *)(pkt->content), data_len);

    dload_file.next_address += data_len;
    dload_file.recv_size += data_len;

    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_end(struct packet_body *pkt)
{
    struct secure_boot *sec_boot = &platform.secure_boot;
    int ret;

    if (sec_boot->secure_boot_enable) {
        ret = sec_boot->secboot_authorization(&platform);
        if (ret) {
            log_print("secboot authorization fail!\n");
            send_ack_packet(PCK_REP_EEROR_CRC);
            return ret;
        }
    } else {
        platform.next_img = platform.next_img + IMAGE_HEADER_SIZE;
    }

    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_exec(struct packet_body *pkt)
{
    /*set PC to target binary start address, will complete later*/
    send_ack_packet(PCK_REP_ACK);
    /* Clean data cache before jump to fdl */
    dcache_clean_all();

    boot_platform_quit((struct boot_arm_vector_table *)platform.next_img);

    return 0;
}

void dload_cmd_register (CMD_TYPE cmd, cmd_operate proc)
{
    CMD_HANDLER(cmd) = proc;
}

void dl_cmd_operate(CMD_TYPE cmd,packet_body *packet)
{
    if (!CMD_HANDLER(cmd)) {
        send_ack_packet(PCK_REP_INVALID_CMD);
        return;
    }

    CMD_HANDLER(cmd)(packet);
}

static int dload_cmd_init(void)
{
    dload_cmd_register(PCK_CMD_CONNECT, packet_connect);
    dload_cmd_register(PCK_CMD_START_DATA, packet_start );
    dload_cmd_register(PCK_CMD_MIDST_DATA, packet_middle);
    dload_cmd_register(PCK_CMD_END_DATA, packet_end);
    dload_cmd_register(PCK_CMD_EXEC_DATA, packet_exec);
    dload_cmd_register(PCK_CMD_CHANGE_BAUD, packet_change_baud);

    return 0;
}

driver_init(dload_cmd_init);
