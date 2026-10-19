/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x105413
#define LOG_MODULE    AMOD_PMIC_I2C

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <pmic_i2c.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

struct pmic_i2c_core pi2c_core;

int pmic_i2c_transfer_irq(struct pmic_i2c_client *client, struct i2c_msg *msgs, int num)
{
    struct pmic_i2c_adapter *adap = (struct pmic_i2c_adapter *)client->data;
    int ret;

#ifndef CONFIG_SUBSYS_CP
    if (__get_primask_value())
        AIC_ASSERT(0);
#endif
    ret = aos_get_mutex(adap->mutex, AOS_WAIT_FOREVER);
    if (ret)
        return AIC_EINTR;

    pi2c_core.status = PI2C_INT_TRANS;
    ret = adap->master_xfer(adap->data, msgs, num, PI2C_INT_XFER);
    pi2c_core.status = PI2C_IDLE;
    aos_put_mutex(adap->mutex);
    return ret;
}

int pmic_i2c_transfer_async(struct pmic_i2c_client *client, struct i2c_msg *msgs, int num)
{

    struct pmic_i2c_adapter *adap = (struct pmic_i2c_adapter *)client->data;
    int ret;

    ret = aos_get_mutex(adap->mutex, AOS_WAIT_FOREVER);
    if (ret)
        return AIC_EINTR;
    pi2c_core.status = PI2C_ASYNC_TRANS;
    ret = adap->master_xfer(adap->data, msgs, num, PI2C_ASYNC_XFER);

    return ret;
}

void pmic_i2c_send_async(struct pmic_i2c_client *client, u8 *buf, int num)
{
    struct i2c_msg msg = {
        .addr = client->addr,
        .flags = 0,
        .len = num,
        .buf = buf,
    };

    pmic_i2c_transfer_async(client, &msg, 1);
}

int pmic_i2c_transfer_async_status(struct pmic_i2c_client *client)
{
    struct pmic_i2c_adapter *adap = (struct pmic_i2c_adapter *)client->data;
    int ret;

    AIC_ASSERT(pi2c_core.status == PI2C_ASYNC_TRANS);

    ret = adap->async_pending(adap->data);
    pi2c_core.status = PI2C_IDLE;
    aos_put_mutex(adap->mutex);
    return ret;
}

int pmic_i2c_transfer_polling(struct pmic_i2c_client *client, struct i2c_msg *msgs, int num)
{
    struct pmic_i2c_adapter *adap = (struct pmic_i2c_adapter *)client->data;
    int ret;

    if (!aic_sys_is_assert())
        AIC_ASSERT(pi2c_core.status == PI2C_IDLE);
    pi2c_core.status = PI2C_POLLING_TRANS;
    ret = adap->master_xfer(adap->data, msgs, num, PI2C_POLLING_XFER);
    pi2c_core.status = PI2C_IDLE;

    return ret;
}

int pmic_i2c_transfer(struct pmic_i2c_client *client, struct i2c_msg *msgs, int num)
{
    if (!aos_is_user_thread_context())
       return pmic_i2c_transfer_polling(client, msgs, num);
    else
       return pmic_i2c_transfer_irq(client, msgs, num);
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
    int i;

    pi2c_core.adap = adap;
    pi2c_core.num = num;
    for (i = 0; i < num; i++) {
        adap[i].mutex = aos_create_mutex(adap[i].name);
        AIC_ASSERT(adap[i].mutex);
    }
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
