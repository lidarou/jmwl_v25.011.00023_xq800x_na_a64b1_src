/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

#ifndef __INCLUDE_I2C_H__
#define __INCLUDE_I2C_H__

#include <i2c_core.h>

struct i2c_client {
    u8 addr;
    u32 index;
    void *data;
};

int i2c_transfer(struct i2c_client *client, struct i2c_msg *msgs, int num);
int i2c_master_recv(struct i2c_client *client, u8 *buf, int num);
int i2c_master_send(struct i2c_client *client, u8 *buf, int num);
int i2c_master_send_then_recv(struct i2c_client *client, u8*addr,
                                u16 addr_len, u8 *buf, int num);

int i2c_open(struct i2c_client *client);
void i2c_close(struct i2c_client *client);

#endif /* __INCLUDE_I2C_H__ */
