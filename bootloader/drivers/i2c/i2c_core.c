/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <console/console.h>
#include <initcall.h>
#include <i2c.h>
#include <platform/platform.h>

struct i2c_core i2c_core;

int i2c_transfer(struct i2c_client *client, struct i2c_msg *msgs, int num)
{
    struct i2c_adapter *adap = (struct i2c_adapter *)client->data;

    return adap->master_xfer(adap->data, msgs, num);
}

int i2c_master_recv(struct i2c_client *client, u8 *buf, int num)
{
    int ret;
    struct i2c_msg msg = {
        .addr = client->addr,
        .flags = I2C_M_RD,
        .len = num,
        .buf = buf,
    };

    ret = i2c_transfer(client, &msg, 1);

    return (ret == 1) ? num : ret;
}

int i2c_master_send(struct i2c_client *client, u8 *buf, int num)
{
    int ret;
    struct i2c_msg msg = {
        .addr = client->addr,
        .flags = 0,
        .len = num,
        .buf = buf,
    };

    ret = i2c_transfer(client, &msg, 1);

    return (ret == 1) ? num : ret;
}

int i2c_master_send_then_recv(struct i2c_client *client, u8*addr,
                                u16 addr_len, u8 *buf, int num)
{
    int ret;
    struct i2c_msg msg[2] = {
       {
        .addr = client->addr,
        .flags = 0,
        .len = addr_len,
        .buf = addr,
       }, {
        .addr = client->addr,
        .flags = I2C_M_RD,
        .len = num,
        .buf = buf,
       }
    };

    ret =i2c_transfer(client, &msg[0], 2);
    return (ret == 2) ? num : ret;
}

void i2c_adapter_register(struct i2c_adapter *adap, u32 num)
{
    i2c_core.adap = adap;
    i2c_core.num = num;
}

int i2c_open(struct i2c_client *client)
{
    if (client->index > i2c_core.num)
        return AIC_EIO;

    client->data = &i2c_core.adap[client->index];
    i2c_core.adap[client->index].count++;

    return AIC_SUCCESS;
}

void i2c_close(struct i2c_client *client)
{
    client->data = NULL;
    i2c_core.adap[client->index].count--;
}
