/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0514ef
#define LOG_MODULE    AMOD_I2C

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <i2c.h>

struct i2c_core i2c_core;

int i2c_transfer(struct i2c_client *client, struct i2c_msg *msgs, int num)
{
    struct i2c_adapter *adap = (struct i2c_adapter *)client->data;
    int ret;

    ret = aos_get_mutex(adap->mutex, AOS_WAIT_FOREVER);
    if (ret)
        return AIC_EINTR;

    ret = adap->master_xfer(adap->data, msgs, num);
    aos_put_mutex(adap->mutex);
    return ret;
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
    int i;

    i2c_core.adap = adap;
    i2c_core.num = num;
    for (i = 1; i < num; i++) {
        adap[i].mutex = aos_create_mutex(adap[i].name);
        AIC_ASSERT(adap[i].mutex);
    }
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
