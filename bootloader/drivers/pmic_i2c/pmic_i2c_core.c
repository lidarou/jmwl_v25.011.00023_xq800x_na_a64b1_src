/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <aic_errno.h>
#include <aic_io.h>
#include <pmic_i2c.h>

struct pmic_i2c_core pi2c_core;

int pmic_i2c_transfer(struct pmic_i2c_client *client, struct i2c_msg *msgs, int num)
{
    struct pmic_i2c_adapter *adap = (struct pmic_i2c_adapter *)client->data;

    return adap->master_xfer(adap->data, msgs, num);
}

int pmic_i2c_send(struct pmic_i2c_client *client, u8 *buf, int num)
{
    int ret;
    struct i2c_msg msg = {
        .addr = client->addr,
        .flags = 0,
        .len = num,
        .buf = buf,
    };

    ret = pmic_i2c_transfer(client, &msg, 1);

    return (ret == 1) ? num : ret;
}

int pmic_i2c_send_then_read(struct pmic_i2c_client *client, u8 *reg,
                              int reg_len, u8 *buf, int num)
{
    int ret;
    struct i2c_msg msg[2] = {
       {
        .addr = client->addr,
        .flags = 0,
        .len = reg_len,
        .buf = reg,
       }, {
        .addr = client->addr,
        .flags = I2C_M_RD,
        .len = num,
        .buf = buf,
       }
    };

    ret = pmic_i2c_transfer(client, &msg[0], 2);
    return (ret == 2) ? num : ret;
}


void pmic_i2c_adapter_register(struct pmic_i2c_adapter *adap, u32 num)
{
    pi2c_core.adap = adap;
    pi2c_core.num = num;
}

int pmic_i2c_open(struct pmic_i2c_client *client)
{
    if (client->index > pi2c_core.num)
        return -1;

    client->data = &pi2c_core.adap[client->index];
    pi2c_core.adap[client->index].count++;

    return AIC_SUCCESS;
}

void pmic_i2c_close(struct pmic_i2c_client *client)
{
    client->data = NULL;
    pi2c_core.adap[client->index].count--;
}
