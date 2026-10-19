/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the pmic charger interface.
*/
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>

void charger_force_powerdown(void)
{
   pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG04, BIT_PMU_RTC_PD_CHRG_18R_REG_EB | BIT_PMU_RTC_PD_CHRG_18R_REG);

   return 0;
}
