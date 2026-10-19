/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of pmic rtc module.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08799d
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_io.h>
#include <core_op.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <rtc.h>
#include <timer_inc.h>

#include "../ups_interface.h"

#define UPS_BASE_YEAR       2000

#define UPS_HOUR_OF_SEC     3600
#define UPS_DAY_OF_SEC      (UPS_HOUR_OF_SEC * 24)

/*  REG_GLB_UPS_UPS_CNT_RSVD_REG_FOR__SW  */
#define BIT_UPS_SW_DAY(x)                           ((x) & 0x7fff)
#define BIT_UPS_SW_DAY_OFFSET                       17
#define BIT_UPS_SW_HOUR(x)                          ((x) & 0x1f)
#define BIT_UPS_SW_HOUR_OFFSET                      12
#define BIT_UPS_SW_MINUTE(x)                        ((x) & 0x3f)
#define BIT_UPS_SW_MINUTE_OFFSET                    6
#define BIT_UPS_SW_SECOND(x)                        ((x) & 0x3f)

static int ups_base;

void gx_ups_phy_mode_enter(void)
{
    pmic_writel(ups_base + PMIC_BASE_PMU_UPS, 1);
}

void gx_ups_phy_mode_exit(void)
{
    pmic_writel(ups_base + PMIC_BASE_PMU_UPS, 0);
}

void gx_ups_phy_cnt_clr(void)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, 0);
    aic_mdelay(50);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, BIT_GLB_UPS_UPS_CNT_EN);
}

void gx_ups_phy_int_clr(void)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INTR_CLR, BIT_GLB_UPS_UPS_CNT_INTR_CLR);
}

u32 gx_ups_phy_get_int_status(void)
{
    return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT_STS);
}

void gx_ups_phy_start_count(void)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, BIT_GLB_UPS_UPS_CNT_EN);
}

void gx_ups_phy_set_comp(u32 cnt)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, BIT_GLB_UPS_UPS_CNT_EN);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INTR_EN, BIT_GLB_UPS_UPS_CNT_INTR_EN);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_COMPARE_THR, cnt);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_COMPARE_EN, BIT_GLB_UPS_UPS_CNT_COMPARE_EN);
}

static void gx_ups_phy_clr_comp(void)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_COMPARE_EN, 0);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_COMPARE_THR, 0);
}

u32 gx_ups_phy_get_cnt(void)
{
    return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT);
}

void gx_ups_phy_set_sw_reg(u32 val)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_RSVD_REG_FOR_SW, val);
}

u32 gx_ups_phy_get_sw_reg(void)
{
    return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT_RSVD_REG_FOR_SW);
}

void gx_ups_phy_set_period(void)
{
    pmic_writel(REG_GLB_UPS_UPS_CNT_COMPARE_MODE, 1);
}

void gx_ups_phy_to_rtc(void)
{
    struct rtc_time tm = {0};
    u32 val, sec_last, sec_curr, sec_offset;

    val = gx_ups_phy_get_cnt();
    if (!val)
        return;

    sec_offset = val / GX_UPS_CLK;
    sec_last = gx_ups_phy_get_sw_reg();
    if (!sec_last)
        return;

    sec_curr = sec_last + sec_offset;

    rtc_time64_to_tm(sec_curr, &tm);

    gx_rtc_set_time(&tm);

    gx_ups_phy_cnt_clr();
    gx_ups_phy_clr_comp();
}

void gx_ups_phy_from_rtc(void)
{
    struct rtc_time tm;
    u64 sec;

    gx_rtc_get_time(&tm);

    sec = rtc_tm_to_time64(&tm);
    if (sec >= 0xFFFFFFFF)
        AIC_ASSERT(0);

    gx_ups_phy_set_sw_reg((u32)sec);
}

void gx_ups_phy_init(void)
{
    ups_base = 0x0;
    pmic_writel(REG_GLB_UPS_CKM_EN_GLB_CLK_UPS_COUNTER_RF, BIT_GLB_UPS_CKM_EN_GLB_CLK_UPS_COUNTER_RF);
    //disable ups period mode
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_COMPARE_MODE, 0);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INTR_MASK, 0);
    /* enable ups cnt */
    //pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, BIT_GLB_UPS_UPS_CNT_EN);
    /* set ups int en */
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INTR_EN, BIT_GLB_UPS_UPS_CNT_INTR_EN);
}

