/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aic_size.h>
#include <i2c.h>
#include <timer_inc.h>
#include <tx_api.h>

#define PI2C_PRINTF    drv_logi

#define EEPROM_DEV_ADDR                       0x50
#define EEPROM_I2C_THRD_PRI                     9
#define EEPROM_I2C_TEST_STACK_SIZE            SIZE_1K

struct eeprom {
    struct i2c_client client;
    aos_thread_id thread;
};
struct eeprom eeprom_app;

char eeprom_stack[2048];

/**
* @addr: the bufffer stores the data to be sent.
* @value: THe value is the data to sent.
* return: 0: success, other: error
*/
int eeprom_write(u16 addr, u8 value)
{
    u8 *addr_ptr;
    u8 buf[3];
    int ret;

    addr_ptr = (u8 *)&addr;
    buf[0] = addr_ptr[0];
    buf[1] = addr_ptr[1];
    buf[2] = value;
    ret = i2c_master_send(&eeprom_app.client, buf, 3);
    if (ret !=3) {
        PI2C_PRINTF("eeprom_write failed, addr=0x%x, value=0x%x, ret=%d\n",
                   addr, value, ret);
        return ret;
    }

    return 0;
}

/**
* @buf: the bufffer stores the data to be sent.
* @num: write data number.
* return: 0: success, other: error
*/
int eeprom_write_burst(u8 *buf, u16 num)
{
    int ret;

    ret = i2c_master_send(&eeprom_app.client, buf, num);
    if (ret != num) {
        PI2C_PRINTF("eeprom_write_burst failed, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

/**
* @buf: the bufffer space for store data.
* @num: read data number.
* return: 0: success, other: error
*/

int eeprom_read_only(u8 *buf, int num)
{
    int ret;

    ret = i2c_master_recv(&eeprom_app.client, buf, num);
    if (ret != num) {
        PI2C_PRINTF("eeprom_read_only failed, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

/**
* @addr: the destination address to read.
* @buf: the bufffer space for store data.
* @num: read data number.
* return: 0: success, other: error
*/

int eeprom_read(u16 addr, u8 *buf, int num)
{
    int ret;

    ret = i2c_master_send_then_recv(&eeprom_app.client, (u8 *)&addr, 2,
                                buf, num);
    if (ret != num) {
        PI2C_PRINTF("eeprom_read failed, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

int eeprom_read16_write16(void)
{
    u8 val1[18] = {0x00, 0x02, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82,
        0x96, 0xa1, 0xb8, 0xc9, 0xd7, 0xe0, 0xf2, 0x05, 0x11};
    u8 val2[4] = {0x00, 0x10, 0x05, 0x11};
    u8 test_val[16];
    int i;

    if(eeprom_write_burst(val1, 16))
         return AIC_ENXIO;
    /* eeprom device need 6ms delay between two transfer */
    aic_udelay(6000);
    if (eeprom_write_burst(val2, 4))
        return AIC_ENXIO;

    /* eeprom device need 6ms delay between two transfer */
    aic_udelay(6000);
    eeprom_read(0x200, test_val, 16);
        PI2C_PRINTF("test_val:\n");
    for (i = 0; i < 16; i++)
       PI2C_PRINTF("0x%x,\n", test_val[i]);
    PI2C_PRINTF("\ntest_val end\n");

    if (memcmp(&val1[2], test_val, 16))
         return AIC_ENXIO;
    return 0;
}


int eeprom_read6_write6(void)
{
    u8 val[8] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71};
    u8 test_val[6];

    if(eeprom_write_burst(val, 8))
         return AIC_ENXIO;
    /* eeprom device need 6ms delay between two transfer */
    aic_udelay(6000);
    eeprom_read(0x2, test_val, 6);
    if (memcmp(&val[2], test_val, 6))
         return AIC_ENXIO;
    return 0;
}


int eeprom_read1_write1(void)
{
    u8 val= 0x55;
    u8 test_val;

    PI2C_PRINTF("eeprom_read1_write1 \n");
    if(eeprom_write(0, val))
         return AIC_ENXIO;
    /* eeprom device need 6ms delay between two transfer */
    aic_udelay(6000);
    eeprom_read(0, &test_val, 1);
    PI2C_PRINTF("test_val = 0x%x\n", test_val);
    if (val != test_val)
         return AIC_ENXIO;
    return 0;
}

void eeprom_i2c_test(void* data)
{
    int count = 0;

    while (1) {
        count++;
        PI2C_PRINTF("eeprom_i2c_test count = %d \n", count);
        if (eeprom_read1_write1()) {
            PI2C_PRINTF("eeprom_read1_write1 fail\n");
            aos_suspend_thread(eeprom_app.thread);
        }

        if (eeprom_read6_write6()) {
            PI2C_PRINTF("eeprom_read6_write6 fail\n");
            aos_suspend_thread(eeprom_app.thread);
        }

        if (eeprom_read16_write16()) {
            PI2C_PRINTF("eeprom_read16_write16 fail\n");
            aos_suspend_thread(eeprom_app.thread);
        }
        PI2C_PRINTF("eeprom_i2c_test \n");
        aos_sleep(1000);
    }
}

static int eeprom_i2c_init_client(void)
{
    int ret;

    eeprom_app.client.index = 3;
    eeprom_app.client.addr = EEPROM_DEV_ADDR;
    ret = i2c_open(&eeprom_app.client);
    if (ret)
        return ret;

    eeprom_app.thread = aos_create_thread(
                        "eeprom test",
                        eeprom_i2c_test, AIC_NULL,
                        EEPROM_I2C_TEST_STACK_SIZE,
                        0, AIC_NULL,
                        EEPROM_I2C_THRD_PRI,
                        AOS_AUTO_START);
    if (eeprom_app.thread == AOS_INVALID_THREAD_ID) {
        app_loge("eeprom_i2c_init_client fail ret =0x%x.\r\n", eeprom_app.thread);
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

static int eeprom_i2c_init(void)
{
    return eeprom_i2c_init_client();
}

AIC_NORMAL_COMP_P1_INIT_EXPORT(eeprom_i2c_init);
