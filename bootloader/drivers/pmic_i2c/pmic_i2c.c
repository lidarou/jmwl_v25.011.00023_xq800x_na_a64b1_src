/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <aic_errno.h>
#include <console/console.h>
#include <initcall.h>
#include <platform/platform.h>
#include <pmic_i2c_core.h>
#include <pmic_i2c.h>

#define PI2C_PRINTF    log_print

#define PMIC_DEV_ADDR                       0x70
#define PMIC_REG_ADDR_LEN                   2
#define PMIC_I2C_SEND_BYTE_MAX              16

struct pmic_i2c_client pi2c_client;

int pmic_writeb(u16  addr, u8 value)
{
    u8 *addr_ptr;
    u8 buf[3];
    int ret;

    addr_ptr = (u8 *)&addr;
    buf[0] = addr_ptr[0];
    buf[1] = addr_ptr[1];
    buf[2] = value;
    ret = pmic_i2c_send(&pi2c_client, buf, 3);
    if (ret !=3) {
        PI2C_PRINTF("pmic_i2c_write failed, addr=0x%x, value=0x%x, ret=%d\n",
                   addr, value, ret);
        return ret;
    }

    return 0;
}

u8 pmic_readb(u16 addr)
{
    u8 buf;
    int ret;

    ret = pmic_i2c_send_then_read(&pi2c_client, (u8 *)&addr,
            PMIC_REG_ADDR_LEN, &buf, 1);
    if (ret != 1) {
        PI2C_PRINTF("pmic_i2c_read failed, addr=0x%x, ret=%d\n", addr, ret);
        return 0;
    }
    return buf;
}

int pmic_burst_write(u8 *buf, int num)
{
    int ret;

    if (num > PMIC_I2C_SEND_BYTE_MAX)
        return AIC_EINVAL;

    ret = pmic_i2c_send(&pi2c_client, buf, num);
    if (ret != num) {
        PI2C_PRINTF("pmic_burst_write failed, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

int pmic_burst_read(u16 addr, u8 *buf, int num)
{
    int ret;

    if (num > PMIC_I2C_SEND_BYTE_MAX)
        return AIC_EINVAL;

    ret = pmic_i2c_send_then_read(&pi2c_client, (u8 *)&addr, PMIC_REG_ADDR_LEN, buf, num);
    if (ret != num) {
        PI2C_PRINTF("pmic_i2c_read failed, reg = 0x%x, ret = %d\n", buf, ret);
        return ret;
    }
    return 0;
}

int pmic_writel(u16 addr, u32 val)
{
    u8 buf[6];

    memcpy(buf, &addr, 2);
    memcpy(&buf[2], &val, 4);
    return pmic_burst_write(buf, 6);
}

u32 pmic_readl(u16 addr)
{
    u32 val;

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

    return pmic_i2c_open(&pi2c_client);
}

int pmic_init(void)
{
    return pmic_init_client();
}
core_init(pmic_init);
