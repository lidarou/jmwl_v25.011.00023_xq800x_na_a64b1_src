/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines sys dump Interface.
 */

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_math.h>
#include <aic_size.h>
#include <assert.h>
#include <initcall.h>
#include <platform/aic_cmdline.h>
#include <stdlib.h>
#include <stdio.h>

#include "boot_mode.h"
#include "serial/serial.h"
#include "sys_dump.h"
#include "sys_dump/sys_dump_cfg.h"
#include "timer.h"
#include "usb/cdc_class.h"

#define SYS_DUMP_MAGIC_NUM  (0x44554D50)
#define SYS_DUMP_TIMEOUT    (-1)

#define SYS_DUMP_BLOCK_SIZE (SIZE_64K - (DIAG_HEAD_LEN + DIAG_TAIL_LEN))

#define DIAG_HEAD_MAGIC     0x7E5A5A7E
#define DIAG_HEAD_SYS       0x0 /* ap */
#define DIAG_HEAD_ERRNO     0x0
#define DIAG_TAIL           0xED
#define DIAG_HEAD_LEN       sizeof(struct diag_cmd_head)
#define DIAG_TAIL_LEN       1

#define SYS_DUMP_CMD            (0x8)
#define SYS_DUMP_SUBCMD_NOTIFY  (0x1)
#define SYS_DUMP_SUBCMD_ACK     (0x2)
#define SYS_DUMP_SUBCMD_INFO    'a'
#define SYS_DUMP_SUBCMD_DUMP    'd'
#define SYS_DUMP_SUBCMD_CPLT    'f'
#define SYS_DUMP_SUBCMD_RESET   'r'

/* io device type define. */
enum {
    AIC_IODEV_NONE = 0,
    AIC_IODEV_UART,
    AIC_IODEV_USB,
    AIC_IODEV_FILE,
    AIC_IODEV_CNT
};

/**
 * magic(4B)   sn(4B) sys(1B) errno(1B) cmd(1B) subcmd(2B) len(2B) tail(1B)
 * 0x7E5A5A7E   0x0    0x1     0x0       0x1      0x0        0x0     0xED
 * @magic:  diag magic.
 * @sn:     diag sn.
 * @sys:    diag sys.
 * @errno:  diag errno.
 * @cmd:    diag cmd.
 * @subcmd: diag sub cmd.
 * @length: diag length.
 */
struct __attribute__((packed)) diag_cmd_head {
    u32 magic;
    u32 sn;
    u8  sys;
    u8  errno;
    u8  cmd;
    u16 subcmd;
    u16 length;
};

struct sys_dump_chn {
    void *data;
    u32 (*write)(void *data, u8 *buf, u32 len);
    u32 (*read)(void *data, u8 *buf, u32 len, u32 timeout);
    void (*baudrate_set)(void *data, u32 baudrate);
};

static __attribute__((aligned(32))) u8 sys_dump_buf[512];

static struct sys_dump_chn *cur_sys_dump_chn;

static u32 sys_dump_uart_read(void *data, u8 *buf, u32 len, u32 timeout)
{
    struct diag_cmd_head *diag_cmd = (struct diag_cmd_head *)sys_dump_buf;
    u32 head_len = sizeof(struct diag_cmd_head);
    int i = 0;

    while (i < (head_len + 1)) {
        while (aic_is_uart_recv_fifo_empty(data)) {}

        buf[i] = aic_uart_getchar(data);

        /*
         * May read some invalid data befor recevied a diag cmd,
         * so until read 0x7e start parse.
         */
        if (buf[0] != 0x7e)
            continue;
        i++;
    }

    if (diag_cmd->length != 0) {
        while (i < (head_len + diag_cmd->length + 1)) {
            while (aic_is_uart_recv_fifo_empty(data)) {}

            buf[i] = aic_uart_getchar(data);
            i++;
        }
    }

    return i;
}

static u32 sys_dump_uart_write(void *data, u8 *buf, u32 len)
{
    aic_uart_write(data, buf, len);

    return 0;
}

static void sys_dump_uart_baudrate_set(void *data, u32 baudrate)
{
    while (!aic_is_uart_send_fifo_empty(data));

    aic_mdelay(2);

    aic_uart_baud_set(data, baudrate);
}

static struct sys_dump_chn sys_dump_uart = {
    .write = sys_dump_uart_write,
    .read = sys_dump_uart_read,
    .baudrate_set = sys_dump_uart_baudrate_set,
};

static void sys_dump_uart_chn_init(u8 dev_com, u32 baud)
{
#ifdef CONFIG_SYS_DUMP_INIT_UART_PIN
    AIC_WRITE32(DUMP_UART_TX_PIN, DUMP_UART_TX_VALUE);
    AIC_WRITE32(DUMP_UART_TX_PAD_PIN, DUMP_UART_TX_PAD_VALUE);
    AIC_WRITE32(DUMP_UART_RX_PIN, DUMP_UART_RX_VALUE);
    AIC_WRITE32(DUMP_UART_RX_PAD_PIN, DUMP_UART_RX_PAD_VALUE);
#endif

    sys_dump_uart.data = aic_uart_port_open(dev_com);
    cur_sys_dump_chn = &sys_dump_uart;

    cur_sys_dump_chn->baudrate_set(cur_sys_dump_chn->data, baud);
}

static u32 sys_dump_usb_read(void *data, u8 *buf, u32 len, u32 timeout)
{
    return usb_read(buf, len, timeout);
}

static u32 sys_dump_usb_write(void *data, u8 *buf, u32 len)
{
    return usb_write(buf, len, SYS_DUMP_TIMEOUT);
}

static struct sys_dump_chn sys_dump_usb = {
    .write = sys_dump_usb_write,
    .read = sys_dump_usb_read,
};

static void sys_dump_usb_chn_init(void)
{
    sys_dump_usb.data = NULL;
    cur_sys_dump_chn = &sys_dump_usb;
}

static u32 sys_dump_recv(u8 *buf, u32 length, u32 timeout)
{
    return cur_sys_dump_chn->read(cur_sys_dump_chn->data, buf, length, timeout);
}

static void sys_dump_word(u32 val)
{
    struct diag_cmd_head diag_cmd;
    u8 tail = DIAG_TAIL;

    /* send diag cmd head */
    diag_cmd.magic = DIAG_HEAD_MAGIC;
    diag_cmd.sn = 0;
    diag_cmd.sys = DIAG_HEAD_SYS;
    diag_cmd.errno = DIAG_HEAD_ERRNO;
    diag_cmd.cmd = SYS_DUMP_CMD;
    diag_cmd.subcmd = SYS_DUMP_SUBCMD_DUMP;
    diag_cmd.length = 4;
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&diag_cmd,
                            DIAG_HEAD_LEN);

    /* send data */
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&val, 4);

    /* send diag cmd tail */
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&tail, 1);
}

static void sys_dump_data(u8 *address, u32 length)
{
    struct diag_cmd_head diag_cmd;
    u8 tail = DIAG_TAIL;

    /* send diag cmd head */
    diag_cmd.magic = DIAG_HEAD_MAGIC;
    diag_cmd.sn = 0;
    diag_cmd.sys = DIAG_HEAD_SYS;
    diag_cmd.errno = DIAG_HEAD_ERRNO;
    diag_cmd.cmd = SYS_DUMP_CMD;
    diag_cmd.subcmd = SYS_DUMP_SUBCMD_DUMP;
    diag_cmd.length = length;
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&diag_cmd,
                            DIAG_HEAD_LEN);

    /* send data */
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, address, length);

    /* send diag cmd tail */
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&tail, 1);
}

static void sys_dump_mem_area(u32 address, u32 length)
{
    u32 i, block_num, remainder_len;
    u8 *start = (u8 *)address;

    block_num = length / SYS_DUMP_BLOCK_SIZE;
    remainder_len = length % SYS_DUMP_BLOCK_SIZE;

    for (i = 0; i < block_num; i++) {
        sys_dump_data(start, SYS_DUMP_BLOCK_SIZE);
        start += SYS_DUMP_BLOCK_SIZE;
    }

    if (remainder_len != 0) {
        sys_dump_data(start, remainder_len);
    }
}

static void sys_dump_reg_area(u32 address, u32 length)
{
    u32 start, end = address + length;

    for (start = address; start < end; start += 4)
        sys_dump_word(AIC_READ32(start));
}

#if 0
static void sys_dump_send_notify(void)
{
    struct diag_cmd_head diag_cmd;
    u8 tail = DIAG_TAIL;

    /* send diag cmd head */
    diag_cmd.magic = DIAG_HEAD_MAGIC;
    diag_cmd.sn = 0;
    diag_cmd.sys = DIAG_HEAD_SYS;
    diag_cmd.errno = DIAG_HEAD_ERRNO;
    diag_cmd.cmd = SYS_DUMP_CMD;
    diag_cmd.subcmd = SYS_DUMP_SUBCMD_NOTIFY;
    diag_cmd.length = 0;
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&diag_cmd,
                            DIAG_HEAD_LEN);

    /* send diag cmd tail */
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&tail, 1);
}
#endif

static void sys_dump_send_complete(void)
{
    struct diag_cmd_head diag_cmd;
    u8 tail = DIAG_TAIL;

    /* send diag cmd head */
    diag_cmd.magic = DIAG_HEAD_MAGIC;
    diag_cmd.sn = 0;
    diag_cmd.sys = DIAG_HEAD_SYS;
    diag_cmd.errno = DIAG_HEAD_ERRNO;
    diag_cmd.cmd = SYS_DUMP_CMD;
    diag_cmd.subcmd = SYS_DUMP_SUBCMD_CPLT;
    diag_cmd.length = 0;
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&diag_cmd,
                            DIAG_HEAD_LEN);

    /* send diag cmd tail */
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&tail, 1);
}

static void sys_dump_subcmd_info(void)
{
    u32 length;
    char *str, *info_addr;
    char *def_str = "lpr";
    struct diag_cmd_head diag_cmd;
    u8 tail = DIAG_TAIL;

    if (get_boot_mode() == SYSDUMP_BOOT) {
        /* get assert info address */
        info_addr = aic_cmdline_get_value(cl_key_asrt_info);
        str = (char *)(strtoul(info_addr, NULL, 16));
    } else {
        str = def_str;
    }
    length = strlen(str);

    /* send diag cmd head */
    diag_cmd.magic = DIAG_HEAD_MAGIC;
    diag_cmd.sn = 0;
    diag_cmd.sys = DIAG_HEAD_SYS;
    diag_cmd.errno = DIAG_HEAD_ERRNO;
    diag_cmd.cmd = SYS_DUMP_CMD;
    diag_cmd.subcmd = SYS_DUMP_SUBCMD_INFO;
    diag_cmd.length = length;
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&diag_cmd,
                            DIAG_HEAD_LEN);

    /* send data */
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)str, length);

    /* send diag cmd tail */
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&tail, 1);
}

static void sys_dump_subcmd_dump(void)
{
    struct sys_dump_head head;
    const struct sys_dump_cfg_info *cfg = sys_dump_get_cfg();
    int i, cfg_num = sys_dump_get_cfg_num();
    u32 offset;

    offset = 0;
    for (i = 0 ; i < cfg_num; i++) {
        head.magic = SYS_DUMP_MAGIC_NUM;
        head.type = cfg[i].type;
        head.address = cfg[i].address;
        head.length = cfg[i].length;

        sys_dump_data((void *)&head, sizeof(head));

        if (cfg[i].type == DUMP_MEM)
            sys_dump_mem_area(cfg[i].address, cfg[i].length);
        else if(cfg[i].type == DUMP_REG)
            sys_dump_reg_area(cfg[i].address, cfg[i].length);
        else
            ASSERT_INFO(0, "sys dump type not support!\n");
    }

    /* dump finished */
    sys_dump_send_complete();
}

static void sys_dump_subcmd_reset(void)
{
    bl_reset();
    while (1);
}

static void sys_dump_wait_for_cmd(void)
{
    struct diag_cmd_head *diag_cmd;
    u32 ret;

    while (1) {
        memset(sys_dump_buf, 0x0, sizeof(sys_dump_buf));
        ret = sys_dump_recv(sys_dump_buf, sizeof(sys_dump_buf), -1);
        if (!ret)
            continue;

        diag_cmd = (struct diag_cmd_head *)sys_dump_buf;

        if ((diag_cmd->magic == DIAG_HEAD_MAGIC) &&
            (diag_cmd->errno == DIAG_HEAD_ERRNO) &&
            (diag_cmd->cmd == SYS_DUMP_CMD)) {
            if (diag_cmd->subcmd == SYS_DUMP_SUBCMD_DUMP) {
                sys_dump_subcmd_info();
                sys_dump_subcmd_dump();
            } else if (diag_cmd->subcmd == SYS_DUMP_SUBCMD_INFO)
                sys_dump_subcmd_info();
            else if (diag_cmd->subcmd == SYS_DUMP_SUBCMD_RESET)
                sys_dump_subcmd_reset();
        }
    }
}

int sys_dump_handle(void)
{
    u8 dev_type, dev_com;
    u32 dev_baud;
    enum BOOT_MOD boot_mode;

    /* get boot mode */
    boot_mode = get_boot_mode();
    if ((boot_mode != SYSDUMP_BOOT) && (boot_mode != LP_SYSDUMP_BOOT))
        return 0;

    if (boot_mode == SYSDUMP_BOOT) {
        aic_cmdline_parse();

        /* get dump channel */
        dev_type = aic_cmdline_get_value_int(cl_key_dev_type);
        if (dev_type == AIC_IODEV_UART) {
            dev_com = aic_cmdline_get_value_int(cl_key_dev_com);
            dev_baud = aic_cmdline_get_value_int(cl_key_dev_baud);
            sys_dump_uart_chn_init(dev_com, dev_baud);
        } else if (dev_type == AIC_IODEV_USB) {
            sys_dump_usb_chn_init();
        } else {
            /* default use usb as dump channel */
            sys_dump_usb_chn_init();
        }

        if (dev_type == AIC_IODEV_UART) {
            sys_dump_subcmd_info();
            sys_dump_subcmd_dump();
        }
    } else {
        sys_dump_usb_chn_init();
    }

    /* wait for cmd */
    sys_dump_wait_for_cmd();

    return 0;
}
early_init_late(sys_dump_handle);
