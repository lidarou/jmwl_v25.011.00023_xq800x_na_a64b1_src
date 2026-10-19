/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1149d8
#define LOG_MODULE    AMOD_PMIC_I2C

#include <aic_errno.h>
#include <aic_init.h>
#include <pmic_io.h>
#include <tx_api.h>

#define PI2C_PRINTF(...)

typedef int (*PMIC_I2C_FUNC)(void);
#define PMIC_RW_BYTES(n)    pmic_i2c_case##n##_test

int pmic_i2c_case17_test(void)
{
    u8 val1[18] = {0x00, 0x02, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82,
        0x96, 0xa1, 0xb8, 0xc9, 0xd7, 0xe0, 0xf2, 0x05, 0x11};
    u8 val2[4] = {0x00, 0x10, 0x05, 0x11};
    u8 test_val[16];
    int i;

    if(pmic_burst_write(val1, 16))
         return AIC_ENXIO;

    if (pmic_burst_write(val2, 4))
        return AIC_ENXIO;

    pmic_burst_read(0x200, test_val, 16);
        PI2C_PRINTF("test_val:\n");
    for (i = 0; i < 16; i++)
       PI2C_PRINTF("0x%x,", test_val[i]);
    PI2C_PRINTF("\ntest_val end\n");

    if (memcmp(&val1[2], test_val, 16))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case16_test(void)
{
    u8 val1[17] = {0x00, 0x02, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82,
        0x96, 0xa1, 0xb8, 0xc9, 0xd7, 0xe0, 0xf2, 0xe2};
    u8 val2[3] = {0x00, 0x10, 0xe2};
    u8 test_val[15];
    int i;

    if (pmic_burst_write(val1, 16))
         return AIC_ENXIO;

    if (pmic_burst_write(val2, 3))
         return AIC_ENXIO;

    pmic_burst_read(0x200, test_val, 15);
    PI2C_PRINTF("test_val:\n");
    for (i = 0; i < 15; i++)
       PI2C_PRINTF("0x%x,", test_val[i]);
    PI2C_PRINTF("\ntest_val end\n");
    if (memcmp(&val1[2], test_val, 15))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case15_test(void)
{
    u8 val[16] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82,
        0x96, 0xa1, 0xb8, 0xc9, 0xd7, 0xe0, 0xf2};
    u8 test_val[14];

    if (pmic_burst_write(val, 16))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 14);
    if (memcmp(&val[2], test_val, 14))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case14_test(void)
{
    u8 val[15] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82,
        0x96, 0xa1, 0xb8, 0xc9, 0xd7, 0xe0};
    u8 test_val[13];

    if(pmic_burst_write(val, 15))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 13);
    if (memcmp(&val[2], test_val, 13))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case13_test(void)
{
    u8 val[14] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82,
        0x96, 0xa1, 0xb8, 0xc9, 0xd7};
    u8 test_val[12];

    if(pmic_burst_write(val, 14))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 12);
    if (memcmp(&val[2], test_val, 12))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case12_test(void)
{
    u8 val[13] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82,
        0x96, 0xa1, 0xb8, 0xc9};
    u8 test_val[11];

    if(pmic_burst_write(val, 13))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 11);
    if (memcmp(&val[2], test_val, 11))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case11_test(void)
{
    u8 val[12] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82, 0x96, 0xa1, 0xb8};
    u8 test_val[10];

    if(pmic_burst_write(val, 12))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 10);
    if (memcmp(&val[2], test_val, 10))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case10_test(void)
{
    u8 val[11] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82, 0x96, 0xa1};
    u8 test_val[9];

    if(pmic_burst_write(val, 11))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 9);
    if (memcmp(&val[2], test_val, 9))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case9_test(void)
{
    u8 val[10] = {0x2, 0x0, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82, 0x96};
    u8 test_val[8];

    if(pmic_burst_write(val, 10))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 8);
    if (memcmp(&val[2], test_val, 8))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case8_test(void)
{
    u8 val[9] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71, 0x82};
    u8 test_val[7];

    if(pmic_burst_write(val, 9))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 7);
    if (memcmp(&val[2], test_val, 7))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case7_test(void)
{
    u8 val[8] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64, 0x71};
    u8 test_val[6];

    if(pmic_burst_write(val, 8))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 6);
    if (memcmp(&val[2], test_val, 6))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case6_test(void)
{
    u8 val[7] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52, 0x64};
    u8 test_val[5];

    if(pmic_burst_write(val, 7))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 5);
    if (memcmp(&val[2], test_val, 5))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case5_test(void)
{
    u8 val[6] = {0x2, 0x00, 0x20, 0x33, 0x45, 0x52};
    u8 test_val[4];

    if(pmic_burst_write(val, 6))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 4);
    if (memcmp(&val[2], test_val, 4))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case4_test(void)
{
    u8 val[5] = {0x2, 0x00, 0x20, 0x33, 0x45};
    u8 test_val[3];

    if(pmic_burst_write(val, 5))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 3);
    if (memcmp(&val[2], test_val, 3))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case3_test(void)
{
    u8 val[4] = {0x2, 0x00, 0x20, 0x33};
    u8 test_val[2];

    if(pmic_burst_write(val, 4))
         return AIC_ENXIO;

    pmic_burst_read(0x2, test_val, 2);
    if (memcmp(&val[2], test_val, 2))
         return AIC_ENXIO;
    return 0;
}

int pmic_i2c_case2_test(void)
{
    u8 val= 0x55;
    u8 test_val;

    if(pmic_write(0, val))
         return AIC_ENXIO;

    test_val = pmic_read(0);
    if (val != test_val)
         return AIC_ENXIO;
    return 0;
}

PMIC_I2C_FUNC pmic_i2c_test[] = {
    PMIC_RW_BYTES(2),
    PMIC_RW_BYTES(3),
    PMIC_RW_BYTES(4),
    PMIC_RW_BYTES(5),
    PMIC_RW_BYTES(6),
    PMIC_RW_BYTES(7),
    PMIC_RW_BYTES(8),
    PMIC_RW_BYTES(9),
    PMIC_RW_BYTES(10),
    PMIC_RW_BYTES(11),
    PMIC_RW_BYTES(12),
    PMIC_RW_BYTES(13),
    PMIC_RW_BYTES(14),
    PMIC_RW_BYTES(15),
    PMIC_RW_BYTES(16),
    PMIC_RW_BYTES(17),
};

TX_THREAD pi2c_self_test;
char pi2c_selftest_stack[2048];

void pmic_i2c_self_test(uint32_t data)
{
    u32 count = 0;
    int ret, i;

    while (1){
        count ++;
        for (i = 0; i < 16; i++) {
            ret = pmic_i2c_test[i]();
            if (ret) {
                aicx_info(0x046ac0, "ddd", "case25 test fail , i =%d, count =%d, ret = %d\n",
                            i, count, ret);
                while(1);
            }
        }
        aicx_info(0x18b620, "d", "pmic_i2c send 1byte ~ 17 bytes ok, count =%d\n", count);
    }
}

int pmic_i2c_self_test_init(void)
{
    uint32_t status;

    /* Create the usbx_cdc_acm_thread_entry thread. */
    status = tx_thread_create(&pi2c_self_test,
                            "pmic i2c self test",
            pmic_i2c_self_test, 0,
            pi2c_selftest_stack,
            1024, 20, 20, TX_NO_TIME_SLICE,
            TX_AUTO_START);
    if (status != AIC_SUCCESS) {
        aicx_error(0x02d880, "x", "pmic_i2c_self_test_init ret =0x%x.\r\n", status);
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

AIC_COMP_P1_INIT_EXPORT(pmic_i2c_self_test_init);
