/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <aic_errno.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_io.h>

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
        log_print("pmic i2c slave slave case3 read fail\n");
        return AIC_EIO;
    }
    ret = pmic_writeb((TEST_REG_ADDR + 1), glb_val);
    if (ret) {
        log_print("pmic i2c slave case3 write fail\n");
        return AIC_EIO;
    }
    ret = pmic_burst_read(TEST_REG_ADDR, tmp_data, 2);
    if (ret) {
        log_print("pmic i2c slave slave case3 read fail\n");
        return AIC_EIO;
    }

    if (tmp_data[1] == test_data[1])
        return AIC_SUCCESS;

    log_print("pmic i2c slave glb 1byte fail 0x8000 = 0x%x\n", glb_val);
    return AIC_EIO;
}

u32 aic_get_timestamp(void);
int pmic_i2c_slave_case2_test(u32 data)
{
    u8 glb_val = 0x55;
    u32 time1, time2, time3, time4;
    int ret;

    time1 = aic_get_timestamp();
    ret = pmic_writel(TEST_REG_ADDR, glb_val);
    time2 = aic_get_timestamp();
    if (ret) {
        log_print("pmic i2c slave case2 write fail\n");
        return AIC_EIO;
    }

    time3 = aic_get_timestamp();
    glb_val = pmic_readb(TEST_REG_ADDR);
    time4 = aic_get_timestamp();

    log_print("time1 = 0x%x,  time2 = 0x%x, total write cost = 0x%x\n",
            time1, time2, time2 - time1);
    log_print("time3 = 0x%x, time4 = 0x%x, total read cost = 0x%x\n",
            time3, time4, time4 - time3);
    if (glb_val == 0x55)
      return AIC_SUCCESS;

    log_print("pmic i2c slave glb 1byte fail addr 0x%x = 0x%x\n", TEST_REG_ADDR, glb_val);
    return AIC_EIO;
}

int pmic_i2c_slave_case1_test(u32 data)
{
    u32 chip_id;

    chip_id = pmic_readl(0x8000);
    chip_id = chip_id >> 16;
    if (chip_id == PMIC_CHIPID)
      return AIC_SUCCESS;

    log_print("pmic i2c slave fail 0x8000 = 0x%x\n", chip_id);
    return AIC_EIO;
}

int pmic_i2c_slave_case7_test(void)
{
    int ret, count = 0;
    u32 data = 0;

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
        log_print("pmic i2c slave success count = %d\n", count);
    }

    log_print("pmic i2c slave fail count = %d\n", count);
    while(1);
    return 0;
}

driver_init(pmic_i2c_slave_case7_test);
