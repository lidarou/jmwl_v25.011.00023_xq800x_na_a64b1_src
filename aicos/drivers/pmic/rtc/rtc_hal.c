/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of pmic rtc module, it is pmic rtc
 *    common hal driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0abff8
#define LOG_MODULE    AMOD_RTC

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_boot_mode.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aipc_api.h>
#include <core_op.h>
#include <irq.h>
#include <pmic_irq.h>
#include <timer_inc.h>
#include <sysoff.h>
#include <timer_inc.h>

#include <rtc.h>
#include "rtc_interface.h"
#include "slp_inc.h"

#ifdef CONFIG_SUBSYS_AP
RTC_ALARM_CB    g_alarm_cb;
void            *g_alarm_param;
RTC_CB          g_rtc_chg_cb;
void            *g_rtc_chg_param;
#endif

addr_t rtc_sh_ptr;
u64 *p_rtc_ms, *p_tick_us, g_rtc_cali_base;

#ifdef CONFIG_SUBSYS_AP
void gx_rtc_get_time(struct rtc_time *tm)
{
    u64 delta_tick, cur_us, cur_rtc, ref_us, ref_rtc, slp_cnt, slp_tm = 0;
    struct rtc_time rtc_tm;

    if (p_tick_us && p_rtc_ms) {
        slp_cnt = slp_get_deepcnt() - g_rtc_cali_base;
        slp_tm = aic_tmr_slp_cali_tmr(slp_cnt);
        ref_us = *p_tick_us;
        ref_rtc = *p_rtc_ms;
    } else {
        ref_us = 0;
        ref_rtc = 0;
    }

    cur_us = aic_get_boot_time() + slp_tm;

    if (cur_us < ref_us) {
        gx_rtc_pmic_phy_read_time(&rtc_tm);
        rtc_day_to_tm(rtc_tm.tm_day, &rtc_tm);

        *p_rtc_ms = rtc_tm_to_time64(&rtc_tm) * 1000;
        *p_tick_us = aic_get_boot_time();
        g_rtc_cali_base = slp_get_deepcnt();
        cur_rtc = *p_rtc_ms;
    } else {
        delta_tick = (cur_us - ref_us)/1000;
        cur_rtc = ref_rtc + delta_tick;
    }

    rtc_ms_to_tm(cur_rtc, tm);
    tm->tm_usec = (cur_us - ref_us)%1000;
}

void gx_rtc_set_time(struct rtc_time *tm)
{
    u32 days;

    if (rtc_valid_tm(tm))
        return;

    /* update rtc share data for subsys sync time */
    if (p_rtc_ms && p_tick_us) {
        *p_rtc_ms = rtc_tm_to_time64(tm) * 1000;
        *p_tick_us = aic_get_boot_time();
        g_rtc_cali_base = slp_get_deepcnt();
        aicx_info(0x022d0d, "lldlld", "RTC set revert time %lld %lld\n", *p_tick_us, *p_rtc_ms);
        aicx_info(0x157294, "dddddd", "RTC set systime %d %d %d %d:%d:%d\n", tm->tm_year, tm->tm_mon,
            tm->tm_day, tm->tm_hour, tm->tm_min, tm->tm_sec);
    }

    days = rtc_tm_to_days(tm);

    tm->tm_day = days;

    gx_rtc_pmic_phy_set_time(tm);

    if (g_rtc_chg_cb != NULL) {
        g_rtc_chg_cb(g_rtc_chg_param);
    }
}

/*
 * gx_rtc_time_change_request
 * Request rtc time change callback.
 */
void gx_rtc_time_change_request(void *cb, void *param)
{
    g_rtc_chg_cb = (RTC_CB)cb;
    g_rtc_chg_param = param;
}

/*
 * gx_rtc_time_change_unrequest
 * Unrequest rtc time change callback.
 */
void gx_rtc_time_change_unrequest(void)
{
    g_rtc_chg_cb = NULL;
    g_rtc_chg_param = NULL;
}

void gx_rtc_get_alarm(struct rtc_time *ra)
{
    gx_rtc_pmic_phy_read_alarm(ra);

    rtc_day_to_tm(ra->tm_day, ra);
}

void gx_rtc_set_alarm(struct rtc_time *ra, RTC_ALARM_CB cb, void *param)
{
    u32 days;

    if (rtc_valid_tm(ra))
        return;

    g_alarm_cb = cb;
    g_alarm_param = param;

    days = rtc_tm_to_days(ra);

    ra->tm_day = days;

    gx_rtc_pmic_phy_set_alarm(ra);
}

void gx_rtc_alarm_enable(BOOL enable)
{
    gx_rtc_pmic_phy_alarm_enable(enable);
}

void gx_rtc_del_alarm(void)
{
    gx_rtc_pmic_phy_del_alarm();
}

#ifdef CONFIG_FP
signed char gx_rtc_alarm_param(void)//获取当前是哪个闹钟在等待中，包括闹钟和备忘录
{
    if((g_alarm_param == NULL) || (g_alarm_cb == NULL))
        return -1;
    else
        return *((signed char *)g_alarm_param);//闹钟为0--2；备忘录从3---12
}
#endif

void gx_rtc_set_int_mode(RTC_INT_MODE mod)
{
    gx_rtc_pmic_phy_set_int_mode(mod);
}

void gx_rtc_irq_handler(void *param)
{
    gx_rtc_pmic_phy_int_clr();

    if (aic_boot_mode_is_charge()) {
        sysoff_chgalrm_reset();
        while(1);
    }

    if (g_alarm_cb) {
        if (g_alarm_param)
            g_alarm_cb(g_alarm_param);
        else
            g_alarm_cb((void *)NULL);
    }
}
#else

void gx_rtc_get_time(struct rtc_time *tm)
{
    u64 delta_tick, cur_us, cur_rtc, ref_us, ref_rtc;

    if (p_tick_us)
        ref_us = *p_tick_us;
    else
        ref_us = 0;
    if (p_rtc_ms)
        ref_rtc = *p_rtc_ms;
    else
        ref_rtc = 0;
    cur_us = aic_get_boot_time();

    if (cur_us < ref_us)
        delta_tick = 0;
    else
        delta_tick = (cur_us - ref_us)/1000;

    cur_rtc = ref_rtc + delta_tick;

    rtc_ms_to_tm(cur_rtc, tm);
    tm->tm_usec = aic_get_timestamp()%1000;
}

static int gx_rtc_receive_handle(AMSG_T *msg, void *p)
{
    p_rtc_ms = (u64 *)msg->p2;
    AICX_ASSERT_IFX(p_rtc_ms != 0, 0x10ae37, "", "RTC: share memory address was wrong!\n");
    p_tick_us = p_rtc_ms + 1;

    return AIC_SUCCESS;
}
#endif

int gx_rtc_init(void)
{
    int ret;
#ifdef CONFIG_SUBSYS_AP
    struct rtc_time tm = {0};
    AMSG_T msg;

    gx_rtc_pmic_phy_init();

    register_pmic_irq(PMIC_IRQ_RTC, gx_rtc_irq_handler, NULL);

    rtc_sh_ptr = aipc_ctrl_alloc(AIC_SYS_CP, sizeof(u64) * 4);
    AICX_ASSERT_IFX(rtc_sh_ptr != 0, 0x1a3b96, "", "RTC: alloc share memory failed!\n");
    p_rtc_ms = (u64 *)(rtc_sh_ptr + 0x10);
    p_tick_us = p_rtc_ms + 1;

    /* get rtc time from register */
    gx_rtc_pmic_phy_read_time(&tm);
    rtc_day_to_tm(tm.tm_day, &tm);

    *p_rtc_ms = rtc_tm_to_time64(&tm) * 1000;
    *p_tick_us = aic_get_boot_time();

    amsg_fill_msg(&msg, AMOD_RTC, 0, (u32)p_rtc_ms);
    ret = amsg_send_msg(AIC_SYS_CP, &msg, AIPC_NO_WAIT);
    AICX_ASSERT_IFX(ret == AIC_SUCCESS, 0x1a1470, "d", "RTC: notify send fail(%d)\n", ret);
#else
    ret = amsg_register_mod_handler(AIC_SYS_AP, AMOD_RTC, gx_rtc_receive_handle, NULL);
    AICX_ASSERT_IFX(ret == AIC_SUCCESS, 0x186e4b, "d", "RTC: register amsg handler fail(%d)\n", ret);
#endif

    return 0;
}

AIC_DRV_L2_INIT_EXPORT(gx_rtc_init);

