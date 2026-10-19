/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * pmic irq driver file.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x113d5f
#define LOG_MODULE    AMOD_PMIC

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "../bcd_core.h"
#include <bcd.h>

#define GP201_REG_PMU_RTC_PMU_INTER_RTC_REG1    0X8a4
#define GP201_BIT_PMU_RTC_CHG_INT_18R_REG       BIT(0)

int aic_chg_int_rise(void)
{
    int pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202) {
        return pmic_readl(0x104) & BIT_4;
    }

    return pmic_readl(GP201_REG_PMU_RTC_PMU_INTER_RTC_REG1) & GP201_BIT_PMU_RTC_CHG_INT_18R_REG;
}

#ifdef CONFIG_BCD_DUMMY

int aic_bcd_result(void)
{
    return AIC_BC_TYPE_DUMMY;
}

#else

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

static void aic_bcd_clear(void)
{
    pmic_writel(REG_PMU_PD_BCD_REG0, 0x0);
}

static int aic_dcd_result_check(void)
{
    int ret = 0, i;
    u32 val;

    for (i = 0; i < 5; i++) {
        aos_sleep(100);
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
    int pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202) {
        return AIC_BC_TYPE_DUMMY;
    }

    aic_bcd_detect_enabled(true);
    aic_dcd_detect_enabled(true);
    aos_sleep(AIC_BC_DETECT_DELAY);

    if (aic_dcd_result_check() < 0) {
        ret = AIC_BC_TYPE_NO_DCD;
        goto out;
    }

    aic_dcd_detect_enabled(false);
    aic_pd_detect_enabled(true);
    aos_sleep(AIC_BC_DETECT_DELAY);

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
    aos_sleep(AIC_BC_DETECT_DELAY);

    if (aic_sd_result_check())
        ret = AIC_BC_TYPE_DCP;
    else
        ret = AIC_BC_TYPE_CDP;

out:
    aic_bcd_clear();
    aic_bcd_detect_enabled(false);
    aic_dcd_detect_enabled(false);

    return ret;
}
#endif
