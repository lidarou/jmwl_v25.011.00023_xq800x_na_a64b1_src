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
#define AIC_THIS_FILE    0x080715
#define LOG_MODULE    AMOD_RTC

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <core_op.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <timer_inc.h>

#include <rtc.h>
#include "rtc_reg.h"

static u32 rtc_base;

u32 gx_rtc_pmic_phy_version(void)
{
    return pmic_readl(rtc_base + REG_RTC_IP_INFO);
}

u32 gx_rtc_pmic_phy_get_int_status(void)
{
    return pmic_readl(rtc_base + REG_RTC_INT_STS);
}

void gx_rtc_pmic_phy_int_clr(void)
{
    pmic_writel(rtc_base + REG_RTC_INT_CLR, BIT_RTC_INTR_CLR(0x1FF));
}

void gx_rtc_pmic_phy_set_int_mode(RTC_INT_MODE mod)
{
    pmic_writel(rtc_base + REG_RTC_INT_EN, mod);
}

void gx_rtc_pmic_phy_read_time(struct rtc_time *tm)
{
    volatile u32 val, val2;

    do {
        val = pmic_readl(rtc_base + REG_RTC_RD_TIME);
        val2 = pmic_readl(rtc_base + REG_RTC_RD_TIME);
    } while (!val && (val != val2));

    tm->tm_sec = BIT_RTC_SECOND(val);
    tm->tm_min = BIT_RTC_MINUTE(val) >> BIT_RTC_MINUTE_OFFSET;
    tm->tm_hour = BIT_RTC_HOUR(val) >> BIT_RTC_HOUR_OFFSET;
    tm->tm_day = pmic_readl(rtc_base + REG_RTC_RD_DAY);
}

void gx_rtc_pmic_phy_set_time(struct rtc_time *tm)
{
    u32 val;

    pmic_writel(rtc_base + REG_RTC_EN_CTRL, BIT_RTC_CNT_EN);

    val = tm->tm_sec |
            tm->tm_min << BIT_RTC_CFG_MINUTE_OFFSET |
            tm->tm_hour << BIT_RTC_CFG_HOUR_OFFSET;
    pmic_writel(rtc_base + REG_RTC_CFG_TIME, val);
    pmic_writel(rtc_base + REG_RTC_CFG_DAY, tm->tm_day);
    pmic_writel(rtc_base + REG_RTC_UPDATE, BIT_RTC_TIME_UPDATE);
    /* it should delay 700us, because rtc work in 4KHz */
    aic_udelay(700);
}

void gx_rtc_pmic_phy_alarm_enable(BOOL enable)
{
    if (enable)
        pmic_writel(rtc_base + REG_RTC_ALARM1_EN, 1);
    else
        pmic_writel(rtc_base + REG_RTC_ALARM1_EN, 0);
}

void gx_rtc_pmic_phy_read_alarm(struct rtc_time *ta)
{
    u32 val;

    if (pmic_readl(rtc_base + REG_RTC_ALARM1_EN)) {
        val = pmic_readl(rtc_base + REG_RTC_TIME_ALARM1);
        ta->tm_sec = BIT_RTC_ALARM1_SECOND(val);
        ta->tm_min = BIT_RTC_ALARM1_MINUTE(val) >> BIT_RTC_ALARM1_MINUTE_OFFSET;
        ta->tm_hour = BIT_RTC_ALARM1_HOUR(val) >> BIT_RTC_ALARM1_HOUR_OFFSET;
        ta->tm_day = pmic_readl(rtc_base + REG_RTC_DAY_ALARM1);
    } else {
        AIC_ASSERT("RTC alarm is disable!\n");
    }
}

void gx_rtc_pmic_phy_set_alarm(struct rtc_time *ta)
{
    u32 val;

    pmic_writel(rtc_base + REG_RTC_ALARM1_EN, BIT_RTC_ALARM1_EN);

    val = ta->tm_sec |
            ta->tm_min << BIT_RTC_ALARM1_MINUTE_OFFSET |
            ta->tm_hour << BIT_RTC_ALARM1_HOUR_OFFSET;
    pmic_writel(rtc_base + REG_RTC_TIME_ALARM1, val);
    pmic_writel(rtc_base + REG_RTC_DAY_ALARM1, ta->tm_day);
    pmic_writel(rtc_base + REG_RTC_ALARM1_CFG_DONE, BIT_RTC_ALARM1_CFG_DONE);
}

void gx_rtc_pmic_phy_del_alarm(void)
{
    pmic_writel(rtc_base + REG_RTC_ALARM1_EN, 0);
    pmic_writel(rtc_base + REG_RTC_TIME_ALARM1, 0);
    pmic_writel(rtc_base + REG_RTC_DAY_ALARM1, 0);
    pmic_writel(rtc_base + REG_RTC_ALARM1_CFG_DONE, BIT_RTC_ALARM1_CFG_DONE);
}

void gx_rtc_pmic_phy_init(void)
{
    pmic_writel(REG_GLB_UPS_CKM_EN_GLB_CLK_UPS_RTC_RF, 1);
    rtc_base = PMIC_BASE_RTC;
    pmic_writel(rtc_base + REG_RTC_EN_CTRL, BIT_RTC_CNT_EN);
    pmic_writel(rtc_base + REG_RTC_GLB_CTRL, 0x7F);
    /* enable RTC alarm1 */
    gx_rtc_pmic_phy_alarm_enable(TRUE);
    gx_rtc_pmic_phy_set_int_mode(RTC_ALARM1_INT);
}

