/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

#ifndef __INCLUDE_PMIC_I2C_CORE_H__
#define __INCLUDE_PMIC_I2C_CORE_H__

struct i2c_msg {
    u16 addr;
    u16 flags;
#define I2C_M_RD    0x0001
#define I2C_M_TEN   0x0010
#define I2C_M_INT   0x0800
#define I2C_M_STOP  0x8000
    u16 len;
    u8 *buf;
};

struct pmic_i2c_client {
    u8 addr;
    u32 index;
    void *data;
};

struct pmic_i2c_adapter {
    u8 *name;
    void *data;
    u32 count;
    int (*master_xfer)(void *, struct i2c_msg *, int);
    int (*set_clk)(void *, u32 clk);
};

struct pmic_i2c_core {
    struct pmic_i2c_adapter *adap;
    u32 num;
};

void pmic_i2c_adapter_register(struct pmic_i2c_adapter *adap, u32 num);
extern int pmic_i2c_open(struct pmic_i2c_client *client);
extern void pmic_i2c_close(struct pmic_i2c_client *client);

#endif /* __INCLUDE_PMIC_I2C_CORE_H__ */
