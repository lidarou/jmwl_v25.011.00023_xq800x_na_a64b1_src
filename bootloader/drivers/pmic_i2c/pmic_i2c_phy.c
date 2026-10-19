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
#include <pmic_i2c/pmic_i2c_cfg.h>
#include <pmic_i2c_core.h>

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
#define I2C_P_READ     0x30
#define I2C_FSM_CLR    0x34
#define I2C_INTERVAL   0x38

/*SR*/
#define I2C_BA          BIT_8
#define I2C_TX_DV       BIT_6
#define I2C_RX_DV       BIT_5

#define I2C_RX_OVF        BIT_5
#define I2C_TO            BIT_3
#define I2C_NACK          BIT_2
#define I2C_DATA          BIT_1
#define I2C_COMP          BIT_0

#define I2C_FIFO_DEPTH  8
#define I2C_FIFO_ROOM  (I2C_FIFO_DEPTH - 2)

#define I2C_CPU(s)  (((s) >> 12) & 0x1)
#define I2C_FSM(s)  (((s) >> 9) & 0x3)
#define I2C_PMIC_READ_STRUCT(l, s)      (((l) << 8) | (s))
#define FREQ_DIV1(s, a)     (((s) / 22) / ((a) + 1))
#define FREQ_DIV2(s, f)     ((s) / (f))

#define I2C_SEND_INTERVAL(d, s)     (((d) * (s)) / 1000000)

#define I2C_WRITE   0
#define I2C_READ    1

#define PI2C_EVENT_FLAG       BIT_0

#define PI2C_PRINTF(...)

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

void aic_pmic_i2c_clk_enable(struct pmic_i2c *pi2c)
{
    writel_relaxed((void *)(pi2c->eb), 1);
}

void aic_pmic_i2c_clk_disable(struct pmic_i2c *pi2c)
{
    writel_relaxed((void *)(pi2c->eb), 0);
}

void aic_pmic_i2c_error_report(u32 status)
{
    if (status & I2C_RX_OVF)
        log_print("occur rx fifo overflow\n");

    if (status & I2C_TO)
       log_print("occur transfer timeout\n");

    if (status & I2C_NACK)
        log_print("no ack\n");
}

void aic_pmic_i2c_polling_status(struct pmic_i2c *pi2c)
{
    u32 status;

    while (1) {
        status = readl_relaxed((void *)(pi2c->base + I2C_ISR));
        writel_relaxed((void *)(pi2c->base + I2C_ISR), status);
        pi2c->ret = 0;
        if (status & I2C_NACK) {
            aic_pmic_i2c_error_report(status);
            pi2c->ret = -(status & (I2C_RX_OVF|I2C_TO | I2C_NACK));
            break;
        }

        if (status & I2C_COMP)
            break;
    }
}

void aic_pmic_i2c_polling_wait_read(struct pmic_i2c *pi2c)
{
    u32 status, int_status;

    pi2c->ret = 0;
    while (1) {
        status = readl_relaxed((void *)(pi2c->base + I2C_SR));
        int_status = readl_relaxed((void *)(pi2c->base + I2C_ISR));
        PI2C_PRINTF("aic_pmic_i2c_irq status = 0x%x\n", status);

        if (int_status & I2C_NACK) {
            aic_pmic_i2c_error_report(int_status);
            pi2c->ret = -(int_status & (I2C_RX_OVF|I2C_TO | I2C_NACK));
            writel_relaxed((void *)(pi2c->base + I2C_ISR), int_status);
            return;
        }
        if ((I2C_CPU(status) != pi2c->cpu) || (!I2C_FSM(status)))
            break;
    }

    aic_pmic_i2c_polling_status(pi2c);
}

static void aic_pmic_i2c_receive_data(struct pmic_i2c *pi2c,
                                    u8 *buf, u32 len)
{
    int i;

    for (i = 0; i < len; i++)
       buf[i] = readb_relaxed((void *)(pi2c->base + I2C_DR));

    PI2C_PRINTF("read: \n");
    for (i = 0; i < len; i++)
       PI2C_PRINTF("0x%x \n", buf[i]);
    PI2C_PRINTF("\nr end: \n");
}

static void aic_pmic_i2c_send_data(struct pmic_i2c *pi2c,
                                    u8 *buf, u32 len)
{
    int i;

    PI2C_PRINTF("send: \n");
    for (i = 0; i < len; i++) {
        PI2C_PRINTF("0x%x \n", buf[i]);
        writeb_relaxed((void *)(pi2c->base + I2C_DR), buf[i]);
    }
    PI2C_PRINTF("\nend: \n");
}

static void aic_pmic_i2c_send_stop(struct pmic_i2c *pi2c, u32 stop)
{
    union i2c_ctrl ctrl;

    ctrl.cr = readl_relaxed((void *)(pi2c->base + I2C_CR));
    ctrl.cr_bitmap.HOLD = !stop;
    writel_relaxed((void *)(pi2c->base + I2C_CR), ctrl.cr);
}

static void aic_pmic_i2c_rw_set(struct pmic_i2c *pi2c, u32 rw)
{
    union i2c_ctrl ctrl;

    ctrl.cr = readl_relaxed((void *)(pi2c->base + I2C_CR));
    ctrl.cr_bitmap.RW = rw;
    writel_relaxed((void *)(pi2c->base + I2C_CR), ctrl.cr);
}

void aic_pmic_i2c_clear_fifo(struct pmic_i2c *pi2c)
{
     union i2c_ctrl ctrl;

     ctrl.cr = readl_relaxed((void *)(pi2c->base + I2C_CR));
     ctrl.cr_bitmap.CLRFIFO = 1;
     writel_relaxed((void *)(pi2c->base + I2C_CR), ctrl.cr);
}

void aic_pmic_i2c_clear_fsm(struct pmic_i2c *pi2c)
{
     writel_relaxed((void *)(pi2c->base + I2C_FSM_CLR), 1);
     writel_relaxed((void *)(pi2c->base + I2C_FSM_CLR), 0);
}

void aic_pmic_i2c_start(struct pmic_i2c  *pi2c, u16 addr)
{
    writel_relaxed((void *)(pi2c->base + I2C_AR), addr);
}

void aic_pmic_clear_read_cfg(struct pmic_i2c  *pi2c)
{
    writel_relaxed((void *)(pi2c->base + I2C_P_READ), 0);
}

static int aic_pmic_i2c_handle_r_msg(struct pmic_i2c  *pi2c,
                                struct i2c_msg *msg)
{
    PI2C_PRINTF("aic_pmic_i2c read start\n");
    aic_pmic_i2c_rw_set(pi2c, I2C_WRITE);
    aic_pmic_i2c_send_data(pi2c, msg[0].buf, msg[0].len);
    writel_relaxed((void *)(pi2c->base + I2C_P_READ),
            I2C_PMIC_READ_STRUCT(msg[1].len, 1));
    writel_relaxed((void *)(pi2c->base + I2C_TSR), msg->len);
    PI2C_PRINTF("aic_pmic_i2c read set start\n");
    aic_pmic_i2c_start(pi2c, msg->addr);
    aic_pmic_i2c_polling_wait_read(pi2c);
    if (!pi2c->ret)
        aic_pmic_i2c_receive_data(pi2c, msg[1].buf, msg[1].len);

    aic_pmic_clear_read_cfg(pi2c);
    PI2C_PRINTF("aic_pmic_i2c read end ret =%d\n", pi2c->ret);
    return pi2c->ret;
}

static int aic_pmic_i2c_handle_w_msg(struct pmic_i2c  *pi2c,
                                struct i2c_msg *msg)
{
    PI2C_PRINTF("aic_pmic_i2c write start\n");
    aic_pmic_i2c_rw_set(pi2c, I2C_WRITE);
    aic_pmic_i2c_send_data(pi2c, msg->buf, msg->len);
    aic_pmic_i2c_start(pi2c, msg->addr);

    aic_pmic_i2c_polling_status(pi2c);
    PI2C_PRINTF("aic_pmic_i2c write end ret =%d\n", pi2c->ret);
    return pi2c->ret;
}

int aic_pmic_i2c_master_xfer(void *dev, struct i2c_msg *msgs, int num)
{
    struct pmic_i2c *pi2c = (struct pmic_i2c *)dev;
    int ret;

    aic_pmic_i2c_clk_enable(pi2c);
    aic_pmic_i2c_clear_fsm(pi2c);
    aic_pmic_i2c_clear_fifo(pi2c);
    if (num != 1) {
        aic_pmic_i2c_send_stop(pi2c, 0);
        ret = aic_pmic_i2c_handle_r_msg(pi2c, msgs);
    } else {
        ret = aic_pmic_i2c_handle_w_msg(pi2c, msgs);
    }
    aic_pmic_i2c_clk_disable(pi2c);

    return ret < 0 ?  ret : num;
}

void aic_pmic_i2c_set_clk(struct pmic_i2c *pi2c, u32 freq)
{
   u32 pre_sclk = FREQ_DIV1(pi2c->source_clk, pi2c->div_a);
   u32 div_b = FREQ_DIV2(pre_sclk, freq) - 1;
   union i2c_ctrl ctrl;

   ctrl.cr = readl_relaxed((void *)(pi2c->base + I2C_CR));
   ctrl.cr_bitmap.DIV_A = pi2c->div_a;
   ctrl.cr_bitmap.DIV_B = div_b;
   writel_relaxed((void *)(pi2c->base + I2C_CR), ctrl.cr);
}

void aic_pmic_i2c_hw_init(struct pmic_i2c *pi2c)
{
    union i2c_ctrl ctrl;

    ctrl.cr = 0;
    ctrl.cr_bitmap.MS = pi2c->mode;
    ctrl.cr_bitmap.NEA = pi2c->addr_len;
    ctrl.cr_bitmap.ACKEN = pi2c->acken;
    writel_relaxed((void *)(pi2c->base + I2C_CR), ctrl.cr);
    writel_relaxed((void *)(pi2c->base + I2C_TOR), pi2c->timeout);
    writel_relaxed((void *)(pi2c->base + I2C_INTERVAL),
        I2C_SEND_INTERVAL(pi2c->mdelay, pi2c->source_clk));
    aic_pmic_i2c_clear_fifo(pi2c);
    aic_pmic_i2c_set_clk(pi2c, pi2c->freq);
}

void aic_pmic_i2c_apb_reset(struct pmic_i2c *pi2c)
{
    writel_relaxed((void *)(pi2c->rst), 1);
    writel_relaxed((void *)(pi2c->rst), 0);
}

int aic_pmic_i2c_init(void)
{
    u32 i2c_max = sizeof(aic_pmic_i2c) / sizeof(*aic_pmic_i2c);
    u32 i;

    for (i = 0; i < i2c_max; i++) {
        aic_pmic_i2c_clk_enable(&aic_pmic_i2c[i]);
        aic_pmic_i2c_hw_init(&aic_pmic_i2c[i]);
        aic_pmic_i2c_clk_disable(&aic_pmic_i2c[i]);
        pi2c_adap[i].data = &aic_pmic_i2c[i];
        pi2c_adap[i].master_xfer = aic_pmic_i2c_master_xfer;
    }
    pmic_i2c_adapter_register(pi2c_adap, i2c_max);

    return AIC_SUCCESS;
}
core_init(aic_pmic_i2c_init);
