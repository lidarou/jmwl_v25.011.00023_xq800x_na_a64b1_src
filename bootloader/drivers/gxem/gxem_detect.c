/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the guixin engineering mode detect interface.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include "charger.h"
#include "fmd/fmd.h"
#include "aic_errno.h"
#include "serial/serial.h"
#include "console/console.h"
#include "platform/platform.h"
#include "platform/gxem_detect.h"
#include "timer.h"
#include "usb/cdc_class.h"
#include "initcall.h"
#include "platform/aic_cmdline.h"
#include "boot_mode.h"
#include "bcd.h"
#include "platform/aic_checksum.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
/*
 * gxem detect device define
 */
typedef enum {
    GXEM_DEV_NULL = 0,
    GXEM_DEV_USB,
    GXEM_DEV_UART,
    GXEM_DEV_FACTORY,
    GXEM_DEV_KEY,
    GXEM_DEV_MAX
} GXEM_DEV_E;

/* gxem receive data time */
#define GXEM_WAIT_TIME          5000

/* gxem data protocol define */
#define GXEM_PROTO_MAGIC        0x7E5A5A7E
#define GXEM_PROTO_SYS          0x0 /*ap */
#define GXEM_PROTO_ERRNO        0x0
#define GXEM_PROTO_CMD          0x1
#define GXEM_PROTO_SUBCMD       0x0
#define GXEM_PROTO_TAIL         0xED
#define GXEM_PROTO_LEN          sizeof(struct gxem_proto)

#define GXEM_MASK               0xFF

#define GXEM_DATA_MAX_SIZE      64

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
/**
 * gxem protocol struct define
 *  MAGIC(4byte) SN(4byte) sys(1byte) errno(1byte) cmd(1byte) subcmd(2byte) len(2byte) TAIL(1byte)
 *  0x7E5A5A7E   0x0       0x1        0x0          0x1        0x0           0x0        0xED
 * @magic: protocol magic.
 * @sn: protocol sn.
 * @sys: protocol sys.
 * @errno: protocol errno.
 * @cmd: protocol cmd.
 * @subcmd: protocol sub cmd.
 * @length: protocol length.
 * @tail: protocol tail.
 */
#pragma pack(push,1)
struct gxem_proto {
    u32     magic;
    u32     sn;
    u8      sys;
    u8      errno;
    u8      cmd;
    u16     subcmd;
    u16     length;
    u8      tail;
};
#pragma pack(pop)

/**
 * factory gxem information.
 *
 * @timeout: detect timeout.
 * @serial_com: serial com
 * @mode: boot mode.
 */
typedef struct gxem_info {
    u32         timeout;
    u16         serial_com;
    u16         mode;
} GXEM_INFO_S;

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
static u8 s_gxem_device = 0;
static u8 s_gxem_mode = 0;

/* gxem detect uart port */
#ifndef GXEM_UART_COM
static struct gxem_info s_gxem_info = {
    .serial_com = 3,
    .timeout = GXEM_WAIT_TIME,
};
#else
static struct gxem_info s_gxem_info = {
    .serial_com = GXEM_UART_COM,
    .timeout = GXEM_WAIT_TIME,
};
#endif

__attribute__((aligned(32))) char s_gxem_buf[GXEM_DATA_MAX_SIZE];

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * gxem_chk - check gxem data.
 *
 * Parameter:
 * @buf: gxem buf
 * @len: buf length
 *
 * Return:
 *    gxem mode.
 */
u8 gxem_chk(char *buf, int len)
{
    struct gxem_proto *proto;
    u8 mode = 0;

    if (len < GXEM_PROTO_LEN)
        return 0;

    proto = (struct gxem_proto *)buf;
    log_print("chk: recv proto: %x %x %x %x %x %x %x %x.\n",
              proto->magic, proto->sn, proto->sys, proto->errno,
              proto->cmd, proto->subcmd, proto->length, proto->tail);

    if ((proto->magic == GXEM_PROTO_MAGIC) &&
        (proto->sys == GXEM_PROTO_SYS) &&
        (proto->errno == GXEM_PROTO_ERRNO) &&
        (proto->cmd == GXEM_PROTO_CMD) &&
        (proto->subcmd != GXEM_PROTO_SUBCMD) &&
        (proto->tail == GXEM_PROTO_TAIL)) {
        mode =  proto->subcmd;
    }

    return mode;
}

#ifdef GXEM_UART_COM
/**
 * gxem_uart_request - send request by uart.
 *
 * Parameter:
 * @auart: uart com.
 *
 * Return:
 *
 */
void gxem_uart_request(u32 *auart)
{
    struct gxem_proto proto;

    proto.magic = GXEM_PROTO_MAGIC;
    proto.sn = 0x0;
    proto.sys = GXEM_PROTO_SYS;
    proto.errno = GXEM_PROTO_ERRNO;
    proto.cmd = GXEM_PROTO_CMD;
    proto.subcmd = GXEM_PROTO_SUBCMD;
    proto.length = 0;
    proto.tail = GXEM_PROTO_TAIL;

    aic_uart_write(auart, (u8 *)&proto, GXEM_PROTO_LEN);
}

/**
 * gxem_uart_detect - gxem detect by uart.
 *
 * Parameter:
 *
 * Return:
 *    AIC_SUCCESS is success, other fail.
 */
int gxem_uart_detect(void)
{
    u32 start_tick, cur_tick, rttrig, *auart;
    int count = 0, i;
    u8 mode;

    auart = aic_uart_port_open(s_gxem_info.serial_com);
    if (!auart)
        return AIC_EINVAL;

    aic_uart_rx_trigger_level_register(auart, GXEM_PROTO_LEN);
    gxem_uart_request(auart);

    memset(s_gxem_buf, 0x0, sizeof(s_gxem_buf));

    start_tick = aic_get_tick();
    do {
        rttrig = aic_is_uart_rx_fifo_triger(auart);
        if (rttrig) {
            count = 0;
            do {
                s_gxem_buf[count++] = aic_uart_getchar(auart);
            } while(count < GXEM_PROTO_LEN);

            break;
        }

        cur_tick = aic_get_tick();
    } while ((cur_tick - start_tick) < s_gxem_info.timeout);

    log_print("uart(%d): recv data count(%d): ", s_gxem_info.serial_com, count);
    for (i = 0; i < count; i++)
        log_print("%x ", s_gxem_buf[i]);
    log_print("\n");

    if (!count) {
        log_print("uart: gxem detect timeout(%d)!\n", s_gxem_info.timeout);
        return AIC_EIO;
    }

    mode = gxem_chk(s_gxem_buf, GXEM_PROTO_LEN);
    if (!mode) {
        log_print("uart: gxem detect fail!\n");
        return AIC_ESRCH;
    }

    s_gxem_device = GXEM_DEV_UART;
    s_gxem_mode = mode;

    return AIC_SUCCESS;
}
#endif

/**
 * gxem_usb_recv - receive gxem data by usb.
 *
 * Parameter:
 * @buf: receive buf.
 * @len: receive buf length.
 *
 * Return:
 *  @count: receive count.
 */
int gxem_usb_recv(char *buf, int len)
{
    int count, i;

    count = usb_read((uint8_t *)buf, len, s_gxem_info.timeout);

    log_print("usb: recv data count(%d): ", count);
    for (i = 0; i < count; i++)
        log_print("%x ", buf[i]);
    log_print("\n");

    return count;
}

/**
 * gxem_usb_reply - reply gxem data by usb.
 *
 * Parameter:
 * @buf: reply buf.
 * @len: reply buf length.
 *
 * Return:
 *  @count: receive count.
 */
int gxem_usb_reply(char *buf, int len)
{
    usb_write((uint8_t *)buf, len, s_gxem_info.timeout);

    return AIC_SUCCESS;
}

/**
 * gxem_usb_detect - gxem detect by usb.
 *
 * Parameter:
 *
 * Return:
 *    AIC_SUCCESS is success, other fail.
 */
int gxem_usb_detect(void)
{
    int ret, count;
    u8 mode;

    memset(s_gxem_buf, 0x0, sizeof(s_gxem_buf));

    count = gxem_usb_recv(s_gxem_buf, GXEM_DATA_MAX_SIZE);
    if (!count) {
        log_print("usb: receive timeout(%d)!\n", s_gxem_info.timeout);
        return AIC_EAGAIN;
    }

    mode = gxem_chk(s_gxem_buf, GXEM_PROTO_LEN);
    if (!mode) {
        log_print("usb: receive mode(%d) error!\n", mode);
        return AIC_EINVAL;
    }

    s_gxem_device = GXEM_DEV_USB;
    s_gxem_mode = mode;

    ret = gxem_usb_reply(s_gxem_buf, count);

    return ret;
}

/**
 * gxem_detect - gxem detect.
 *
 * Parameter:
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int gxem_detect(void)
{
    char *cmd_buf = s_gxem_buf;
    boolean usb_detect = false;
    u32 boot_mode, bcd_mode;
    int ret;

    boot_mode = get_boot_mode();
    if (boot_mode == NORMAL_BOOT) {
        usb_detect = false;
    } else if (boot_mode == CHARGE_BOOT) {
#ifdef CONFIG_BCD
        bcd_mode = bcd_detect();
        if ((bcd_mode == AIC_BC_TYPE_SDP) || (bcd_mode == AIC_BC_TYPE_CDP))
            usb_detect = true;
#else
       usb_detect = true;
#endif
    } else {
        return 0;
    }

    do {
        if (usb_detect) {
            ret = gxem_usb_detect();
        } else {
#ifdef GXEM_UART_COM
            ret = gxem_uart_detect();
#else
            ret = AIC_ENODEV;
#endif
        }
    } while(0);

    if(ret == AIC_SUCCESS) {
        snprintf(cmd_buf, GXEM_DATA_MAX_SIZE, "%d,%d",
                 s_gxem_mode, s_gxem_device);

        log_print("receive cmd gxem = %s!\n", cmd_buf);
        aic_cmdline_add_key(cl_key_gxem, cmd_buf);
        charger_force_powerdown();
    }

    return ret;
}

/**
 * gxem_detect_init - gxem init.
 *
 * Parameter:
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int gxem_detect_init(void)
{
    platform_boot_mode_enterpoint_register(gxem_detect, GXEM_BOOT);

    return AIC_SUCCESS;
}

driver_init(gxem_detect_init);

