/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of slp module.
*/


#ifndef __SYSOFF_MANAGE_H__
#define __SYSOFF_MANAGE_H__

#include <aic_errno.h>
#include <tx_api.h>
#include <aic_bitmap.h>


/*REG_GLB_UPS_PMIC_SW_REGISTER*/
//software defined
#define PMIC_SW_REG_DUMP_FLAG           BIT(0)
#define PMIC_SW_REG_APCRASH             BIT(1)
#define PMIC_SW_REG_CPCRASH             BIT(2)
#define PMIC_SW_REG_ADSPCRASH           BIT(1)|BIT(2)
#define PMIC_SW_REG_APWDG               BIT(4)
#define PMIC_SW_REG_ADSPWDG             BIT(5)
#define PMIC_SW_REG_CPWDG               BIT(4)|BIT(5)
#define PMIC_SW_REG_BASIC               BIT(6)
#define PMIC_SW_REG_WARMRST             BIT(7)
#define PMIC_SW_REG_SHUTDOWN            BIT(8)
#define PMIC_SW_REG_AUTODL              BIT(9)
#define PMIC_SW_REG_CHG_ALARM           BIT(10)
#define PMIC_SW_REG_GXEM                BIT(11)
#define PMIC_SW_REG_DPSHUTDW            BIT(12)
#define PMIC_SW_REG_BL_PHASE            BIT(13)
#define PMIC_SW_REG_MASK_1              BIT(14)
#define PMIC_SW_REG_MASK_2              BIT(15)
#define PMIC_SW_REG_BOOT_MODE_MASK      (0xFFE)
#define PMIC_SW_MASK                    (0xC000)//Bit14/Bit15
#define PMIC_SW_REG_DUMPMODE_FLAG_MASK  (PMIC_SW_REG_DUMP_FLAG)

#define PMIC_SW_MASK_SHIFT_BIT 14


/*REG_GLB_UPS_PWR_ON_RST_FLAG*/
#define BIT_GLB_UPS_PWR_ON_FLAG_MASK  (0x1ff)

#endif /* __SYSOFF_MANAGE_H__ */
