/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

#ifndef __INCLUDE_I2C_CORE_H__
#define __INCLUDE_I2C_CORE_H__

#include <aic_osa.h>

struct i2c_msg {
    u16 addr;
    u16 flags;
#define I2C_M_RD    0x0001
#define I2C_M_TEN   0x0010
#define I2C_M_STOP  0x8000
    u16 len;
    u8 *buf;
};

struct i2c_adapter {
    u8 *name;
    void *data;
    u32 count;
    int (*master_xfer)(void *, struct i2c_msg *, int);
    aos_mutex_ptr mutex;
};

struct i2c_core {
    struct i2c_adapter *adap;
    u32 num;
};

void i2c_adapter_register(struct i2c_adapter *adap, u32 num);

#endif /* __INCLUDE_I2C_CORE_H__ */
