/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09277c
#define LOG_MODULE    AMOD_PMIC_I2C

#include <aic_errno.h>
#include <aic_init.h>
#include <pmic_i2c.h>

#define PMIC_DEV_ADDR                       0x70
#define PMIC_REG_ADDR_LEN                   2
#define PMIC_I2C_SEND_BYTE_MAX              16

struct pmic_i2c_client pi2c_client;

int pmic_writeb(u16  addr, u8 value)
{
    u8 *addr_ptr;
    u8 buf[3];
    int ret;

    if (!pi2c_client.status)
        return AIC_ENXIO;

    addr_ptr = (u8 *)&addr;
    buf[0] = addr_ptr[0];
    buf[1] = addr_ptr[1];
    buf[2] = value;
    ret = pmic_i2c_send(&pi2c_client, buf, 3);
    if (ret !=3)
        return ret;

    return 0;
}

u8 pmic_readb(u16 addr)
{
    u8 buf;
    int ret;

    if (!pi2c_client.status)
        return AIC_ENXIO;

    ret = pmic_i2c_send_then_read(&pi2c_client, (u8 *)&addr, PMIC_REG_ADDR_LEN, &buf, 1);
    if (ret != 1)
        return 0;
    return buf;
}

int pmic_burst_write(u8 *buf, int num)
{
    int ret;

    if (!pi2c_client.status)
        return AIC_ENXIO;

    if (num > PMIC_I2C_SEND_BYTE_MAX)
        return AIC_EINVAL;

    ret = pmic_i2c_send(&pi2c_client, buf, num);
    if (ret != num)
        return ret;

    return 0;
}

int pmic_burst_read(u16 addr, u8 *buf, int num)
{
    int ret;

    if (!pi2c_client.status)
        return AIC_ENXIO;

    if (num > PMIC_I2C_SEND_BYTE_MAX)
        return AIC_EINVAL;

    ret = pmic_i2c_send_then_read(&pi2c_client, (u8 *)&addr, PMIC_REG_ADDR_LEN, buf, num);
    if (ret != num)
        return ret;

    return 0;
}

void pmic_writeb_async(u16  addr, u8 value)
{
    u8 *addr_ptr;
    u8 buf[3];

    if (!pi2c_client.status)
        return;

    addr_ptr = (u8 *)&addr;
    buf[0] = addr_ptr[0];
    buf[1] = addr_ptr[1];
    buf[2] = value;
    pmic_i2c_send_async(&pi2c_client, buf, 3);
}

void pmic_writel_async(u16 addr, u32 val)
{
    u8 buf[6];

    if (!pi2c_client.status)
        return;

    memcpy(buf, &addr, 2);
    memcpy(&buf[2], &val, 4);
    pmic_i2c_send_async(&pi2c_client, buf, 6);
}

int pmic_polling_async_status(void)
{
    if (!pi2c_client.status)
        return AIC_ENXIO;
    return pmic_i2c_transfer_async_status(&pi2c_client); 
}

int pmic_writel(u16 addr, u32 val)
{
    u8 buf[6];

    if (!pi2c_client.status)
        return AIC_ENXIO;

    memcpy(buf, &addr, 2);
    memcpy(&buf[2], &val, 4);
    return pmic_burst_write(buf, 6);
}

u32 pmic_readl(u16 addr)
{
    u32 val;

    if (!pi2c_client.status)
        return AIC_ENXIO;

    pmic_burst_read(addr, (u8 *)&val, 4);

    return val;
}

void pmic_set_client_addr(u8 addr)
{
    pi2c_client.addr = addr;
}

static int pmic_init_client(void)
{
    pi2c_client.addr = PMIC_DEV_ADDR;
    pi2c_client.index = 0;
    pi2c_client.status = 1;

    return pmic_i2c_open(&pi2c_client);
}

static int pmic_init(void)
{
    return pmic_init_client();
}

AIC_DRV_P1_INIT_EXPORT(pmic_init);
