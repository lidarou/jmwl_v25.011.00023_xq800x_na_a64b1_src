/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0cfbc9
#define LOG_MODULE    AMOD_PMIC_I2C

#include <aic_errno.h>
#include <aic_init.h>
#include <pmic_io.h>
#include <tx_api.h>

#define PI2C_PRINTF(...)

#ifdef CONFIG_SUBSYS_CP
#define TEST_REG_ADDR 0x204
#else
#define TEST_REG_ADDR 0x14
#endif

#define PMIC_CHIPID     0x201

int pmic_i2c_slave_case3_test(u32 data)
{
    u8 glb_val =0x44;
    u8 test_data[2];
    u8 tmp_data[2];
    int ret;

    ret = pmic_burst_read(TEST_REG_ADDR, test_data, 2);
    if (ret) {
        aicx_error(0x0287cd, "", "pmic i2c slave slave case3 read fail\n");
        return AIC_EIO;
    }
    ret = pmic_writeb((TEST_REG_ADDR + 1), glb_val);
    if (ret) {
        aicx_error(0x1a8ed5, "", "pmic i2c slave case3 write fail\n");
        return AIC_EIO;
    }
    ret = pmic_burst_read(TEST_REG_ADDR, tmp_data, 2);
    if (ret) {
        aicx_error(0x0287cd, "", "pmic i2c slave slave case3 read fail\n");
        return AIC_EIO;
    }

    if (tmp_data[1] == test_data[1])
        return AIC_SUCCESS;

    aicx_error(0x175882, "x", "pmic i2c slave glb 1byte fail 0x8000 = 0x%x\n", glb_val);
    return AIC_EIO;
}

u32 aic_get_timestamp(void);
int pmic_i2c_slave_case2_test(u32 data)
{
    u8 glb_val = 0x55;
    u32 time1, time2, time3, time4;
    char *thread_name = (char *)data;
    int ret;

    time1 = aic_get_timestamp();
    ret = pmic_writel(TEST_REG_ADDR, glb_val);
    time2 = aic_get_timestamp();
    if (ret) {
        aicx_error(0x0ce2fc, "", "pmic i2c slave case2 write fail\n");
        return AIC_EIO;
    }

    time3 = aic_get_timestamp();
    glb_val = pmic_readb(TEST_REG_ADDR);
    time4 = aic_get_timestamp();

    aicx_error(0x15ac9a, "sxxx", "%s: time1 = 0x%x,  time2 = 0x%x, total write cost = 0x%x\n",
            thread_name, time1, time2, time2 - time1);
    aicx_error(0x073637, "sxxx", "%s: time3 = 0x%x, time4 = 0x%x, total read cost = 0x%x\n",
              thread_name, time3, time4, time4 - time3);
    if (glb_val == 0x55)
      return AIC_SUCCESS;

    aicx_error(0x07fcbe, "xx", "pmic i2c slave glb 1byte fail addr 0x%x = 0x%x\n", TEST_REG_ADDR, glb_val);
    return AIC_EIO;
}

int pmic_i2c_slave_case1_test(u32 data)
{
    u32 chip_id;

    chip_id = pmic_readl(0x8000);
    chip_id = chip_id >> 16;
    if (chip_id == PMIC_CHIPID)
      return AIC_SUCCESS;

    aicx_error(0x17b5b4, "x", "pmic i2c slave fail 0x8000 = 0x%x\n", chip_id);
    return AIC_EIO;
}

void pmic_i2c_slave_case7_test(uint32_t data)
{
    int ret, count = 0;
    char *thread_name = (char *)data;

    while(1) {
        count++;
        ret = pmic_i2c_slave_case1_test(data);
        if (ret)
            break;
        pmic_i2c_slave_case2_test(data);
        if (ret)
            break;
        pmic_i2c_slave_case3_test(data);
        if (ret)
            break;
        aicx_error(0x13e708, "sd", "%s: pmic i2c slave success count = %d\n",
                thread_name, count);
    }

    aicx_error(0x0cab7b, "sd", "%s: pmic i2c slave fail count = %d\n", thread_name, count);
    while(1);
}

TX_THREAD pi2c_self_test[2];
char pi2c_selftest_stack[2][1024];

int pmic_i2c_self_test_init(void)
{
    uint32_t status;

    /* Create the usbx_cdc_acm_thread_entry thread. */
    status = tx_thread_create(&pi2c_self_test[0],
                        "pmic i2c self test0",
            pmic_i2c_slave_case7_test, (uint32_t)"pmic i2c self test0",
            pi2c_selftest_stack[0],
            1024, 20, 20,
            TX_NO_TIME_SLICE,
            TX_AUTO_START);
    if (status != AIC_SUCCESS) {
        aicx_error(0x017c49, "x", "pmic_i2c_self_test0_init ret =0x%x.\r\n", status);
        return AIC_EIO;
    }

    /* Create the usbx_cdc_acm_thread_entry thread. */
    status = tx_thread_create(&pi2c_self_test[1],
                        "pmic i2c self test1",
            pmic_i2c_slave_case7_test, (uint32_t)"pmic i2c self test1",
            pi2c_selftest_stack[1],
            1024, 19, 19,
            TX_NO_TIME_SLICE,
            TX_AUTO_START);
    if (status != AIC_SUCCESS) {
        aicx_error(0x0c0fcc, "x", "pmic_i2c_self_test1_init ret =0x%x.\r\n", status);
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

AIC_COMP_P1_INIT_EXPORT(pmic_i2c_self_test_init);
