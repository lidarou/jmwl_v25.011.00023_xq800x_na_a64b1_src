/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PMIC_I2C_CFG_H__
#define __PMIC_I2C_CFG_H__

#include <top_glb.h>
#include <reg_base.h>
#include <aic_interrupt.h>
#include <pmic_i2c_core.h>

#define I2C_ADDR_10LEN  0
#define I2C_ADDR_7LEN   1
#define I2C_SLAVE  0
#define I2C_MASTER   1
#define PMIC_I2C_CP 0
#define PMIC_I2C_AP 1

struct pmic_i2c {
    u32 index;
    char* name;
    u32 base;
    u32 eb;
    u32 rst;
    u32 source_clk;
    u32 irq;
    u32 freq;
    u32 mode;
    u32 div_a;
    u32 addr_len;
    u32 acken;
    u32 polling;
    u32 cpu;
    /* SCL clock count */
    u32 timeout;
    u32 mdelay;
    u8 *xfer_buf;
    u32 xfer_len;
    volatile u32 complete;
    int ret;
    aos_event_ptr event;
};

struct pmic_i2c aic_pmic_i2c[1] = {
    {
        .index = 0,
        .name = "pmic i2c",
        .base = REG_BASE_TOP_I2C_PMIC,
        .eb = REG_TOP_GLB_I2CC_PMIC_EB_MCU,
        .rst = REG_TOP_GLB_SW_RST_TOP_I2C_PMIC,
        .source_clk = 125000000,
        .irq = IRQ_I2C_PMIC,
        .freq = 1000000, /* 1 MHZ */
        .div_a = 0,
        .addr_len = I2C_ADDR_7LEN,
        .acken = 1,
        .polling = 0,
        .timeout = 0xff,
        .mdelay = 0,
        .mode = I2C_MASTER,
        .cpu = PMIC_I2C_AP,
    },
};
struct pmic_i2c_adapter pi2c_adap[1];

#endif /* __PMIC_I2C_CFG_H__ */
