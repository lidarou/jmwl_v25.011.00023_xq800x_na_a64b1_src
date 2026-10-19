/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __I2C_CFG_H__
#define __I2C_CFG_H__

#include <reg_base.h>

#define I2C_ADDR_10LEN  0
#define I2C_ADDR_7LEN   1
#define I2C_SLAVE  0
#define I2C_MASTER   1

#define NORMAL_I2C_START    1
#define I2C_NUMBER 5
struct aic_i2c {
    u32 index;
    u32 base;
    u32 apb_base;
    u32 eb;
    u32 source_clk;
    u32 irq;
    u32 freq;
    u32 mode;
    u32 div_a;
    u32 addr_len;
    u32 acken;
    u32 polling;
    u8 *xfer_buf;
    u32 xfer_len;
    u16 rw;
    int ret;
};

struct aic_i2c aic_i2c[I2C_NUMBER] = {
    [1] = {
        .index = 1,
        .base = REG_BASE_TOP_I2CC0,
        .apb_base = REG_BASE_TOP_GLB,
        .eb = 0x4,
        .source_clk = 24000000,
        .freq = 400000,
        .div_a = 0,
        .addr_len = I2C_ADDR_7LEN,
        .acken = 1,
        .polling = 0,
        .mode = I2C_MASTER,
    } ,
    [2] = {
        .index = 2,
        .base = REG_BASE_TOP_I2CC1,
        .apb_base = REG_BASE_TOP_GLB,
        .eb = 0x8,
        .source_clk = 24000000,
        .freq = 400000,
        .div_a = 0,
        .addr_len = I2C_ADDR_7LEN,
        .acken = 1,
        .polling = 0,
        .mode = I2C_MASTER,
    },
    [3] = {
        .index = 3,
        .base = REG_BASE_AP_I2C0,
        .apb_base = REG_BASE_AP_GLB,
        .eb = 0x24,
        .source_clk = 24000000,
        .freq = 400000,
        .div_a = 0,
        .addr_len = I2C_ADDR_7LEN,
        .acken = 1,
        .polling = 0,
        .mode = I2C_MASTER,
    },
    [4] = {
        .index = 4,
        .base = REG_BASE_AP_I2C1,
        .apb_base = REG_BASE_AP_GLB,
        .eb = 0x28,
        .source_clk = 24000000,
        .freq = 400000,
        .div_a = 0,
        .addr_len = I2C_ADDR_7LEN,
        .acken = 1,
        .polling = 0,
        .mode = I2C_MASTER,
    },
};
struct i2c_adapter aic_adap[I2C_NUMBER];

#endif /* __I2C_CFG_H__ */