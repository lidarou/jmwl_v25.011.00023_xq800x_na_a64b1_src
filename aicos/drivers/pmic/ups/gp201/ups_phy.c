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
#define AIC_THIS_FILE    0x1942cc
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

static int gx_ups_phy_new_ver(void)
{
    u32 pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202) {
        return 1;
    } else {
        return 0;
    }
}

void gx_ups_phy_mode_exit(void)
{
    if (gx_ups_phy_new_ver()) {
        pmic_writel(ups_base + REG_PMU_UPS_PMU_UPS_CTRL_REG, 0);
    } else {
        pmic_writel(ups_base + REG_PMU_RTC_PMU_UPS_CTRL_REG, 0);
    }
}

void gx_ups_phy_mode_enter(void)
{
    if (gx_ups_phy_new_ver()) {
        pmic_writel(ups_base + REG_PMU_UPS_PMU_UPS_CTRL_REG, 1);
    } else {
        pmic_writel(ups_base + REG_PMU_RTC_PMU_UPS_CTRL_REG, 1);
        //pmic_writel(ups_base + REG_GLB_UPS_SHUTDOWN_TRIGGER, 1);
    }
}

void gx_ups_phy_cnt_clr(void)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, 0);
    aic_mdelay(50);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, BIT_GLB_UPS_UPS_CNT_EN);
}

void gx_ups_phy_int_clr(void)
{
    if (gx_ups_phy_new_ver()) {
        pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INTR_CLR, BIT_GLB_UPS_UPS_CNT_INTR_CLR);
    } else {
        pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INT_CLR, BIT_GLB_UPS_UPS_CNT_INTR_CLR);
    }
}

u32 gx_ups_phy_get_int_status(void)
{
    if (gx_ups_phy_new_ver()) {
        return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT_STS);
    } else {
        return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT_STS_RD);
    }
}

void gx_ups_phy_start_count(void)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, BIT_GLB_UPS_UPS_CNT_EN);
}

void gx_ups_phy_set_comp(u32 cnt)
{
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, BIT_GLB_UPS_UPS_CNT_EN);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INT_EN, BIT_GLB_UPS_UPS_CNT_INTR_EN);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_COMP_THR, cnt);
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_COMP_EN, BIT_GLB_UPS_UPS_CNT_COMPARE_EN);
}

u32 gx_ups_phy_get_cnt(void)
{
    if (gx_ups_phy_new_ver()) {
        return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT);
    } else {
        return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT_RD);
    }
}

void gx_ups_phy_set_sw_reg(u32 val)
{
    if (gx_ups_phy_new_ver()) {
        pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_RSVD_REG_FOR_SW, val);
    } else {
        pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_RSVD_REG_FOR__SW, val);
    }
}

u32 gx_ups_phy_get_sw_reg(void)
{
    if (gx_ups_phy_new_ver()) {
        return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT_RSVD_REG_FOR_SW);
    } else {
        return pmic_readl(ups_base + REG_GLB_UPS_UPS_CNT_RSVD_REG_FOR__SW);
    }
}

void gx_ups_phy_set_period(void)
{
    if (gx_ups_phy_new_ver()) {
        pmic_writel(REG_GLB_UPS_UPS_CNT_COMPARE_MODE, 1);
    }
}

void gx_ups_phy_to_rtc(void)
{
    struct rtc_time tm = {0};
    u32 val, day, sec_offset, ups_day, ups_hour, ups_min, ups_sec;

    val = gx_ups_phy_get_cnt();
    if (!val)
        return;

    sec_offset = val / GX_UPS_CLK;

    ups_day = sec_offset / UPS_DAY_OF_SEC;
    sec_offset %= UPS_DAY_OF_SEC;
    ups_hour = sec_offset / UPS_HOUR_OF_SEC;
    sec_offset %= UPS_HOUR_OF_SEC;
    ups_min = sec_offset / 60;
    ups_sec = sec_offset % 60;

    val = gx_ups_phy_get_sw_reg();
    if (!val)
        return;

    day = ups_day + BIT_UPS_SW_DAY(val >> BIT_UPS_SW_DAY_OFFSET);
    tm.tm_hour = ups_hour + BIT_UPS_SW_HOUR(val >> BIT_UPS_SW_HOUR_OFFSET);
    tm.tm_min = ups_min + BIT_UPS_SW_MINUTE(val >> BIT_UPS_SW_MINUTE_OFFSET);
    tm.tm_sec = ups_sec + BIT_UPS_SW_SECOND(val);

    rtc_day_to_tm(day, &tm);

    gx_rtc_set_time(&tm);

}

void gx_ups_phy_from_rtc(void)
{
    struct rtc_time tm;
    u32 val, day;

    gx_rtc_get_time(&tm);

    day = rtc_tm_to_days(&tm);

    val = BIT_UPS_SW_SECOND(tm.tm_sec);
    val |= BIT_UPS_SW_MINUTE(tm.tm_min) << BIT_UPS_SW_MINUTE_OFFSET;
    val |= BIT_UPS_SW_HOUR(tm.tm_hour) << BIT_UPS_SW_HOUR_OFFSET;
    val |= BIT_UPS_SW_DAY(day) << BIT_UPS_SW_DAY_OFFSET;

    gx_ups_phy_set_sw_reg(val);
}

void gx_ups_phy_init(void)
{
    if (gx_ups_phy_new_ver()) {
        ups_base = -0x10;
        pmic_writel(REG_GLB_UPS_CKM_EN_GLB_CLK_UPS_COUNTER_RF, BIT_GLB_RTC_CKM_EN_GLB_CLK_UPS_COUNTER_RF);
        //disable ups period mode
        pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_COMPARE_MODE, 0);
        pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INTR_MASK, 0);
    } else {
        ups_base = 0;
        /* set ups glb eb */
        pmic_writel(REG_GLB_RTC_CKM_EN_CLK_UPS_CNT, BIT_GLB_RTC_CKM_EN_GLB_CLK_UPS_COUNTER_RF);
    }
    /* enable ups cnt */
    //pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_EN, BIT_GLB_UPS_UPS_CNT_EN);
    /* set ups int en */
    pmic_writel(ups_base + REG_GLB_UPS_UPS_CNT_INT_EN, BIT_GLB_UPS_UPS_CNT_INTR_EN);
}

