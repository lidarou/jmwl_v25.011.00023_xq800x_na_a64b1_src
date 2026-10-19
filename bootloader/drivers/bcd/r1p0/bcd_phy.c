/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * pmic bcd driver file.
 *
 */

#include <aic_bitmap.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <bcd.h>
#include <io.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "pmu_pd.h"
#include "pmu_rtc.h"
#include "../bcd_core.h"
#include <timer.h>

int aic_chg_int_rise(void)
{
    return pmic_readl(REG_PMU_RTC_PMU_INTER_RTC_REG1) & BIT_PMU_RTC_CHG_INT_18R_REG;
}

#ifndef CONFIG_BCD_DUMMY
static void BCD_REG_AND(u16 offset, u32 bit_value)
{
    pmic_writel(offset, pmic_readl(offset) & bit_value);
}

static void BCD_REG_OR(u16 offset, u32 bit_value)
{
    pmic_writel(offset, pmic_readl(offset) | bit_value);
}

static void aic_bcd_detect_enabled(bool enabled)
{
    if (enabled)
        BCD_REG_OR(REG_PMU_PD_BCD_REG0, BIT_PMU_PD_EN_BCD_18_REG);
    else
        BCD_REG_AND(REG_PMU_PD_BCD_REG0, ~BIT_PMU_PD_EN_BCD_18_REG);
}

static void aic_dcd_detect_enabled(bool enabled)
{
    if (enabled)
        BCD_REG_OR(REG_PMU_PD_BCD_REG0, BIT_PMU_PD_EN_DCD_18_REG);
    else
        BCD_REG_AND(REG_PMU_PD_BCD_REG0, ~BIT_PMU_PD_EN_DCD_18_REG);
}

static void aic_pd_detect_enabled(bool enabled)
{
    if (enabled)
        BCD_REG_OR(REG_PMU_PD_BCD_REG0, BIT_PMU_PD_EN_PD_18_REG);
    else
        BCD_REG_AND(REG_PMU_PD_BCD_REG0, ~BIT_PMU_PD_EN_PD_18_REG);
}

static void aic_sd_detect_enabled(bool enabled)
{
    if (enabled)
        BCD_REG_OR(REG_PMU_PD_BCD_REG0, BIT_PMU_PD_EN_SD_18_REG);
    else
        BCD_REG_AND(REG_PMU_PD_BCD_REG0, ~BIT_PMU_PD_EN_SD_18_REG);
}

static int aic_dcd_result_check(void)
{
    int ret = 0, i;
    u32 val;

    for (i = 0; i < 5; i++) {
        aic_mdelay(100);
        val = pmic_readl(REG_PMU_PD_BCD_REG0);
        if (val & BIT_PMU_PD_DCD_DET_18_REG)
            break;
    }

    if (i == 5)
        ret = AIC_ETIME;

    return ret;
}

static int aic_ps2_result_check(void)
{
    return pmic_readl(REG_PMU_PD_BCD_REG0) & BIT_PMU_PD_PS2_DET_18_REG;
}

static int aic_pd_result_check(void)
{
    return pmic_readl(REG_PMU_PD_BCD_REG0) & BIT_PMU_PD_PD_DET_18_REG;
}

static int aic_sd_result_check(void)
{
    return pmic_readl(REG_PMU_PD_BCD_REG0) & BIT_PMU_PD_SD_DET_18_REG;
}

int aic_bcd_result(void)
{
    int ret;

    aic_bcd_detect_enabled(true);
    aic_dcd_detect_enabled(true);
    aic_mdelay(AIC_BC_DETECT_DELAY);

    if (aic_dcd_result_check() < 0) {
        ret = AIC_BC_TYPE_NO_DCD;
        goto out;
    }

    aic_dcd_detect_enabled(false);
    aic_pd_detect_enabled(true);
    aic_mdelay(AIC_BC_DETECT_DELAY);

    if (aic_ps2_result_check()) {
        ret = AIC_BC_TYPE_PS2;
        goto out;
    }

    if (!aic_pd_result_check()) {
        ret = AIC_BC_TYPE_SDP;
        goto out;
    }

    aic_pd_detect_enabled(false);
    aic_sd_detect_enabled(true);
    aic_mdelay(AIC_BC_DETECT_DELAY);

    if (aic_sd_result_check())
        ret = AIC_BC_TYPE_DCP;
    else
        ret = AIC_BC_TYPE_CDP;

    aic_sd_detect_enabled(false);
    aic_bcd_detect_enabled(false);
out:
    return ret;
}
#endif
