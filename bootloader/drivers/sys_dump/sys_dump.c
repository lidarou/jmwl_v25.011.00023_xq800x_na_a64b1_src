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

#include "boot_mode.h"
#include "serial/serial.h"
#include "sys_dump.h"
#include "sys_dump/sys_dump_cfg.h"
#include "timer.h"
#include "usb/cdc_class.h"

#define SYS_DUMP_USE_USB
#define SYS_DUMP_MAGIC_NUM  (0x44554D50)
#define SYS_DUMP_TIMEOUT    (-1)
#define SYS_DUMP_KEY        "SYSDUMP"

struct sys_dump_chn {
    void *data;
    u32 (*write)(void *data, u8 *buf, u32 len);
    u32 (*read)(void *data, u8 *buf, u32 len);
    void (*baudrate_set)(void *data, u32 baudrate);
};

static u8 __attribute__((aligned(32))) sys_dump_buf[64];
static struct sys_dump_chn *cur_sys_dump_chn;

#ifdef SYS_DUMP_USE_UART
static u32 sys_dump_uart_read(void *data, u8 *buf, u32 len)
{
    u32 recv_cnt = 0;

    while (!aic_is_uart_recv_fifo_timeout(data))
        recv_cnt += aic_uart_read(data, buf + recv_cnt);

    aic_uart_clr_status(data);
    return recv_cnt;
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

static void sys_dump_uart_chn_init(void)
{
    sys_dump_uart.data = aic_uart_port_open(0);
    cur_sys_dump_chn = &sys_dump_uart;
}
#elif defined(SYS_DUMP_USE_USB)
#define SYS_DUMP_USB_BLOCK_SIZE  (SIZE_64K)

static u32 sys_dump_usb_read(void *data, u8 *buf, u32 len)
{
    return usb_read(buf, len, SYS_DUMP_TIMEOUT);
}

static u32 sys_dump_usb_write(void *data, u8 *buf, u32 len)
{
    u32 i, block_num, remainder_len;
    u8 *start = buf;
    int ret;

    block_num = len / SYS_DUMP_USB_BLOCK_SIZE;
    remainder_len = len % SYS_DUMP_USB_BLOCK_SIZE;

    for (i = 0; i < block_num; i++) {
        ret = usb_write(start, SYS_DUMP_USB_BLOCK_SIZE, SYS_DUMP_TIMEOUT);
        ASSERT_INFO(ret != 0, "sys dump usb write failed!!!\n");
        start += SYS_DUMP_USB_BLOCK_SIZE;
    }

    if (remainder_len != 0) {
        ret = usb_write(start, remainder_len, SYS_DUMP_TIMEOUT);
        ASSERT_INFO(ret != 0, "sys dump usb write failed!!!\n");
    }

    return 0;
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
#endif

static void sys_dump_recv(u8 *buf, u32 length)
{
    cur_sys_dump_chn->read(cur_sys_dump_chn->data, buf, length);
}

static void sys_dump_word(u32 val)
{
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, (u8 *)&val, 4);
}

static void sys_dump_data(u8 *address, u32 length)
{
    cur_sys_dump_chn->write(cur_sys_dump_chn->data, address, length);
}

static void sys_dump_mem_area(u32 address, u32 length)
{
    sys_dump_data((u8 *)address, length);
}

static void sys_dump_reg_area(u32 address, u32 length)
{
    u32 start, end = address + length;

    for (start = address; start < end; start += 4)
        sys_dump_word(AIC_READ32(start));
}

int sys_dump_handle(void)
{
    struct sys_dump_head head;
    const struct sys_dump_cfg_info *cfg = sys_dump_get_cfg();
    int i, cfg_num = sys_dump_get_cfg_num();
    u32 offset;

    /* get boot mode */
    if (get_boot_mode() != SYSDUMP_BOOT)
        return 0;

#ifdef SYS_DUMP_USE_UART
    sys_dump_uart_chn_init();
#elif defined(SYS_DUMP_USE_USB)
    sys_dump_usb_chn_init();
#endif

    /* send sys dump notify to tool */
    while (1) {
        log_print("wait for sys dump\n");

        /* wait for tool ack */
        while (1) {
            memset(sys_dump_buf, 0x0, sizeof(sys_dump_buf));
            sys_dump_recv(sys_dump_buf, sizeof(SYS_DUMP_KEY));
            if (strncmp((const char *)sys_dump_buf, SYS_DUMP_KEY,
                        (sizeof(SYS_DUMP_KEY) - 1)) == 0)
                break;
        }

        log_print("sys dump start ...\n");

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

        log_print("sys dump complete!\n");
    }

    return 0;
}

early_init_late(sys_dump_handle);
