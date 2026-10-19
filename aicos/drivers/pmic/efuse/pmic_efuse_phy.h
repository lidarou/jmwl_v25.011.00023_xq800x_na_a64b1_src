/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of efuse module.
 *   It includes all the efuse phy function api.
*/
#ifndef __PMIC_EFUSE_PHY_H__
#define __PMIC_EFUSE_PHY_H__
#include <aic_bitmap.h>

#define PMIC_EFUSE_WRITE_MAGIC       0X12EF
#define PMIC_EFUSE_WAIT_TIME_CNT     100

typedef enum {
    AIC_PMIC_EFUSE_READ_FINISH = 1,
    AIC_PMIC_EFUSE_WRITE_FINISH = 2,
    AIC_PMIC_EFUSE_MAGIC_ERROR = 4,
    AIC_PMIC_EFUSE_ALL = 0xF
} AIC_PMIC_EFUSE_INT;

typedef enum {
    AIC_PMIC_EFUSE_INT_S_READ_FINISH = BIT(0),
    AIC_PMIC_EFUSE_INT_S_WRITE_FINISH = BIT(1),
    AIC_PMIC_EFUSE_INT_S_MAGIC_ERROR  = BIT(2)
} AIC_PMIC_EFUSE_INT_MASK;

int aic_pmic_efuse_write(int blk, u32 data);
int aic_pmic_efuse_read(int blk,u32 *data);

#endif /* __PMIC_EFUSE_PHY_H__ */
