/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <console/console.h>
#include <initcall.h>
#include <platform/platform.h>
#include <i2c.h>
#include <i2c/i2c_cfg.h>

#define I2C_CR         0x00
#define I2C_SR         0x04
#define I2C_AR         0x08
#define I2C_DR         0x0c
#define I2C_ISR        0x10
#define I2C_TSR        0x14
#define I2C_SMPR       0x18
#define I2C_TOR        0x1c
#define I2C_IMR        0x20
#define I2C_IER        0x24
#define I2C_IDR        0x28
#define I2C_GFCR       0x2c

#define I2C_HOLD        BIT_4
/*SR*/
#define I2C_BA          BIT_8
#define I2C_TX_DV       BIT_6
#define I2C_RX_DV       BIT_5

#define I2C_RX_OVF        BIT_5
#define I2C_RX_TO         BIT_3
#define I2C_NACK          BIT_2
#define I2C_DATA          BIT_1
#define I2C_COMP          BIT_0

#define I2C_FIFO_DEPTH      8
#define I2C_FIFO_ROOM       (I2C_FIFO_DEPTH - 2)

#define FREQ_DIV1(s, a)     (((s) / 22) / ((a) + 1))
#define FREQ_DIV2(s, f)     ((s) / (f))

#define I2C_WRITE   0
#define I2C_READ    1
#define I2C_EVENT_FLAG       BIT_0


union i2c_ctrl {
    u32 cr;
    struct{
        u32 RW : 1;
        u32 MS : 1;
        u32 NEA : 1;
        u32 ACKEN : 1;
        u32 HOLD : 1;
        u32 SLVMON : 1;
        u32 CLRFIFO : 1;
        u32 RSV : 1;
        u32 DIV_B : 6;
        u32 DIV_A : 2;
    } cr_bitmap;
};

void aic_i2c_clk_enable(struct aic_i2c *ai2c)
{
    if (!ai2c->apb_base)
        return;
    writel_relaxed((void *)(ai2c->apb_base + ai2c->eb), 1);
}

void aic_i2c_clk_disable(struct aic_i2c *ai2c)
{
    if (!ai2c->apb_base)
        return;
    writel_relaxed((void *)(ai2c->apb_base + ai2c->eb), 0);
}

void aic_i2c_error_report(u32 status)
{
    if (status & I2C_RX_OVF)
        log_print("occur rx fifo overflow\n");

    if (status & I2C_NACK)
        log_print("no ack\n");
}

static void aic_i2c_receive_data(struct aic_i2c *ai2c,
                                    u8 *buf, u32 len)
{
    int i;

    for (i = 0; i < len; i++)
       buf[i] = readb_relaxed((void *)(ai2c->base + I2C_DR));
}

static void aic_i2c_send_data(struct aic_i2c *ai2c,
                                    u8 *buf, u32 len)
{
    int i;

    for (i = 0; i < len; i++)
        writeb_relaxed((void *)(ai2c->base + I2C_DR), buf[i]);
}

static void aic_i2c_transfer_hold(struct aic_i2c *ai2c, u32 hold)
{
    union i2c_ctrl ctrl;

    ctrl.cr = readl_relaxed((void *)(ai2c->base + I2C_CR));
    ctrl.cr_bitmap.HOLD = !hold;
    writel_relaxed((void *)(ai2c->base + I2C_CR), ctrl.cr);
}

static void aic_i2c_rw_set(struct aic_i2c *ai2c, u32 rw)
{
    union i2c_ctrl ctrl;

    ctrl.cr = readl_relaxed((void *)(ai2c->base + I2C_CR));
    ctrl.cr_bitmap.RW = rw;
    writel_relaxed((void *)(ai2c->base + I2C_CR), ctrl.cr);
}

static int aic_i2c_check_data_status(struct aic_i2c *ai2c)
{
    u32 status;

    while (1) {
        status = readl_relaxed((void *)(ai2c->base + I2C_ISR));
        writel_relaxed((void *)(ai2c->base + I2C_ISR), status);
        if (status & (I2C_RX_OVF | I2C_NACK)) {
            aic_i2c_error_report(status);
            return AIC_EIO;
        }

        if (status & I2C_DATA)
            return AIC_SUCCESS;
    }
}

static int aic_i2c_check_complete_status(struct aic_i2c *ai2c)
{
    u32 status;

    while (1) {
        status = readl_relaxed((void *)(ai2c->base + I2C_ISR));
        writel_relaxed((void *)(ai2c->base + I2C_ISR), status);
        if (status & (I2C_RX_OVF | I2C_NACK)) {
            aic_i2c_error_report(status);
            return AIC_EIO;
        }

        if (status & I2C_COMP)
            return AIC_SUCCESS;
    }
}

static int aic_i2c_polling_read_data(struct aic_i2c *ai2c,
                                      struct i2c_msg *msg)
{
    u8 *buf = msg->buf;
    u16 len = msg->len;
    u32 data_len;
    int ret;

    writel_relaxed((void *)(ai2c->base + I2C_TSR), len);
    writel_relaxed((void *)(ai2c->base + I2C_AR), msg->addr);
    while (len) {
        if (len > I2C_FIFO_ROOM)
            ret = aic_i2c_check_data_status(ai2c);
        else
            ret = aic_i2c_check_complete_status(ai2c);
        if (ret)
            return ret;
        data_len = len > I2C_FIFO_ROOM ? I2C_FIFO_ROOM : len;
        aic_i2c_receive_data(ai2c, buf, data_len);
        buf = buf + data_len;
        len -= data_len;
    }

    return AIC_SUCCESS;
}

static int aic_i2c_polling_write_data(struct aic_i2c *ai2c,
                                        struct i2c_msg *msg)
{
    u8 *buf = msg->buf;
    u16 len = msg->len;
    u32 data_len;
    int ret;

    while (len) {
        data_len = len > I2C_FIFO_ROOM ? I2C_FIFO_ROOM : len;
        aic_i2c_send_data(ai2c, buf, data_len);
        writel_relaxed((void *)(ai2c->base + I2C_AR), msg->addr);
        if (len > I2C_FIFO_ROOM)
            ret = aic_i2c_check_data_status(ai2c);
        else
            ret = aic_i2c_check_complete_status(ai2c);
        if (ret)
            return ret;

        buf = buf + data_len;
        len -= data_len;
    }

    return AIC_SUCCESS;
}

void aic_i2c_clear_fifo(struct aic_i2c *ai2c)
{
     union i2c_ctrl ctrl;

     ctrl.cr = readl_relaxed((void *)(ai2c->base + I2C_CR));
     ctrl.cr_bitmap.CLRFIFO = 1;
     writel_relaxed((void *)(ai2c->base + I2C_CR), ctrl.cr);
}

static int aic_i2c_poll_handle_msg(struct aic_i2c *ai2c,
                                struct i2c_msg *msg, int is_last)
{
    int ret = 0;

    aic_i2c_clear_fifo(ai2c);
    aic_i2c_transfer_hold(ai2c, 0);
    if (msg->flags & I2C_M_RD) {
        aic_i2c_rw_set(ai2c, I2C_READ);
        ret = aic_i2c_polling_read_data(ai2c, msg);
    } else {
        aic_i2c_rw_set(ai2c, I2C_WRITE);
        ret = aic_i2c_polling_write_data(ai2c, msg);
    }

    if (is_last) {
        aic_i2c_transfer_hold(ai2c, 1);
        aic_i2c_check_complete_status(ai2c);
    }

    return ret;
}

int aic_i2c_master_xfer(void *dev, struct i2c_msg *msgs, int num)
{
    struct aic_i2c *ai2c = (struct aic_i2c *)dev;
    int i, ret;

    aic_i2c_clk_enable(ai2c);
    for (i = 0; i < num; i++) {
        ret = aic_i2c_poll_handle_msg(ai2c, &msgs[i], (i == (num - 1)));
        if (ret)
            break;
    }
    aic_i2c_clk_disable(ai2c);

    return ret ? ret : num;
}

void aic_i2c_set_clk(struct aic_i2c *ai2c, u32 freq)
{
   u32 pre_sclk = FREQ_DIV1(ai2c->source_clk, ai2c->div_a);
   u32 div_b = FREQ_DIV2(pre_sclk, freq) - 1;
   union i2c_ctrl ctrl;

   ctrl.cr = readl_relaxed((void *)(ai2c->base + I2C_CR));
   ctrl.cr_bitmap.DIV_A = ai2c->div_a;
   ctrl.cr_bitmap.DIV_B = div_b;
   writel_relaxed((void *)(ai2c->base + I2C_CR), ctrl.cr);
}

void aic_i2c_hw_init(struct aic_i2c *ai2c)
{
    union i2c_ctrl ctrl;

    ctrl.cr = 0;
    ctrl.cr_bitmap.MS = ai2c->mode;
    ctrl.cr_bitmap.NEA = ai2c->addr_len;
    ctrl.cr_bitmap.ACKEN = ai2c->acken;
    ctrl.cr_bitmap.MS = ai2c->mode;
    writel_relaxed((void *)(ai2c->base + I2C_CR), ctrl.cr);
    aic_i2c_clear_fifo(ai2c);
    aic_i2c_set_clk(ai2c, ai2c->freq);
}

int aic_i2c_init(void)
{
    u32 i2c_max = sizeof(aic_i2c) / sizeof(*aic_i2c);
    u32 i;

    for (i = NORMAL_I2C_START; i < i2c_max; i++) {
        aic_i2c_clk_enable(&aic_i2c[i]);
        aic_i2c_hw_init(&aic_i2c[i]);
        aic_i2c_clk_disable(&aic_i2c[i]);
        aic_adap[i].data = &aic_i2c[i];
        aic_adap[i].master_xfer = aic_i2c_master_xfer;
    }
    i2c_adapter_register(aic_adap, i2c_max);
    return AIC_SUCCESS;
}

service_init(aic_i2c_init);
