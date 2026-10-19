/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of watchdog HAL module
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x031ea7
#define LOG_MODULE    AMOD_PMIC

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#include "pmic_efuse_phy.h"
#include <pmic_efuse.h>

#define AIC_PMIC_EFUSE_MUTEX      "pmic_efuse"
aos_mutex_ptr aic_pmic_efuse_mutex = AOS_NULL;

int aic_pmic_efuse_block_write(int blk, u32 blk_val)
{
    int ret = 0;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_pmic_efuse_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    ret = aic_pmic_efuse_write(blk, blk_val);
    if (ret)
        aicx_error(0x01c798, "", "pmic efuse write data fail!!\n");

    if (aos_is_user_thread_context())
        aos_put_mutex(aic_pmic_efuse_mutex);

    return ret;
}

int aic_pmic_efuse_block_read(int blk, u32 *blk_val)
{
    u32 ret;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_pmic_efuse_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    ret = aic_pmic_efuse_read(blk, blk_val);
    if (ret)
        aicx_error(0x07d2b6, "", "pmic efuse read data fail!!\n");

    if (aos_is_user_thread_context())
        aos_put_mutex(aic_pmic_efuse_mutex);

    return ret;
}

int aic_pmic_efuse_init(void)
{
    aic_pmic_efuse_mutex = aos_create_mutex(AIC_PMIC_EFUSE_MUTEX);
    AIC_ASSERT(aic_pmic_efuse_mutex);

    return 0;
}
AIC_DRV_P1_INIT_EXPORT(aic_pmic_efuse_init);
