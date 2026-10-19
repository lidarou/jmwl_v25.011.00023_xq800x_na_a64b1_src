/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the efuse demo for user
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a0a83
#define LOG_MODULE    AMOD_PMIC

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <pmic_efuse.h>

#define AIC_EFUSE_TEST_DATA     0x7F6F5F4F

int pmic_efuse_demo_init(void)
{
    int i, ret;
    u32 data;
    /* efuse demo, never ever run in real-chip*/
#if 0
    for (i = 0; i < 2; i++) {
        ret = aic_pmic_efuse_block_write(i, AIC_EFUSE_TEST_DATA);
        if(ret)
            aicx_error(0x01c798, "", "pmic efuse write data fail!!\n");

        ret = aic_pmic_efuse_block_read(i, &data);
        if(ret)
            aicx_error(0x01c798, "", "pmic efuse write data fail!!\n");

        if(data != AIC_EFUSE_TEST_DATA)
            aicx_error(0x0dd9ae, "x", "pmic efuse demo write/read fail 0x%x!!\n",data);
    }
#endif
    return 0;
}
//AIC_NORMAL_APP_L1_INIT_EXPORT(pmic_efuse_demo_init);
