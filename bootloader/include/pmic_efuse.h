/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/
#ifndef __PMIC_EFUSE_H__
#define __PMIC_EFUSE_H__
#include <aic_type.h>
int aic_pmic_efuse_block_write(int blk, u32 blk_val);
int aic_pmic_efuse_block_read(int blk, u32 *blk_val);
#endif /*__PMIC_EFUSE_H__*/