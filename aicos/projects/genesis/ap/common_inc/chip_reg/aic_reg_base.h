/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip reg base files.
 */


#ifndef __AIC_REG_BASE_H__
#define __AIC_REG_BASE_H__

#include <reg_base.h>

#define REG_BASE_PMIC_START             0x0
#define REG_BASE_PMIC_END               0xFFFF

/* lzma redefine */
#define REG_BASE_LZMA                   (0x42070000)
#define REG_LZMA_EB                     (0x51000148)
#define REG_LZMA_RESET                  (void *)(0x42080004)
/* int definition */
#define BIT_DEC_ERR                     BIT(1)
#define BIT_DEC_DONE                    BIT(0)
/* chipid definition */
#define REG_CHIP_ID                     (0x400D015C)
#define REG_VERSION_ID                  REG_AON_GLB_VERSION_ID
#define CHIP_VERSION_AA                 0xA0000000
#define CHIP_VERSION_AB                 0xA0000001
#define CHIP_VERSION_AC                 0xA0000002
#define CHIP_VERSION_AD                 0xA0000003

#endif /* __AIC_REG_BASE_H__ */
