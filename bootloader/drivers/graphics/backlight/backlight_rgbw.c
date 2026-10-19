/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the kpled operate api.
*/
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>

int aic_bl_brightness_set(u32 brt_perc)
{
    int ret;
    u32 perc, reg;

    if (brt_perc) {
        perc = (brt_perc * 0x1F) / 100;
        reg = BIT_PMU_PD_REG1_LED2R_18_EB | BIT_PMU_PD_REG1_LED2G_18_EB |BIT_PMU_PD_REG1_LED2B_18_EB |
              BIT_PMU_PD_REG1_LED2W_18_EB | BIT_PMU_PD_REG1_LED2R_18(perc) |BIT_PMU_PD_REG1_LED2G_18(perc) |
              BIT_PMU_PD_REG1_LED2B_18(perc) | BIT_PMU_PD_REG1_LED2W_18(perc);
        ret = pmic_writel(REG_PMU_PD_LED_REG1, reg);
        ret = pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG07, BIT_PMU_RTC_LED2R_EN_18_REG);
        ret |= pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG08, BIT_PMU_RTC_LED2G_EN_18_REG);
        ret |= pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG09, BIT_PMU_RTC_LED2B_EN_18_REG);
        ret |= pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG10, BIT_PMU_RTC_LED2W_EN_18_REG);
    } else {
        ret = pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG07, 0);
        ret |= pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG08, 0);
        ret |= pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG09, 0);
        ret |= pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG10, 0);
    }

    return ret;
}