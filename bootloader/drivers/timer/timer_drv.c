/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of timer module, it is timer
 *    common hal driver.
*/

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aon_glb.h>
#include <initcall.h>
#include <reg_base.h>
#include <timer_cfg.h>
#include "platform/platform.h"

#define FREERUN_CNT_L       (REG_BASE_AON_SYSTMR + 0x0)
#define FREERUN_CNT_H       (REG_BASE_AON_SYSTMR + 0x4)
#define SYSTMR_CNT_L        (REG_BASE_AON_SYSTMR + 0x8)
#define SYSTMR_CNT_H        (REG_BASE_AON_SYSTMR + 0xC)

u32 aic_get_tick(void)
{
    return AIC_READ32(SYSTMR_CNT_L);
}

u32 aic_get_timestamp(void)
{
    u32 val;

    val = AIC_READ32(FREERUN_CNT_L);
    /* 30us per step */
    val = val * 30;

    return val;
}

void aic_delay(u32 sec)
{
    u32 cnt_old, cnt_cur, delay_ms;

    delay_ms = sec * 1000;
    cnt_old = aic_get_tick();
    do {
        cnt_cur = aic_get_tick();
    } while ((cnt_cur - cnt_old) < delay_ms);
}

void aic_mdelay(u32 msec)
{
    u32 cnt_old, cnt_cur;

    cnt_old = aic_get_tick();
    do {
        cnt_cur = aic_get_tick();
    } while ((cnt_cur - cnt_old) < msec);
}

/*
 * aic_udelay - use for us delay scenarios
 * Note that!!!
 * This usec should be multiple of 30.518us.
 */
void aic_udelay(u32 usec)
{
    u32 cnt_old, cnt_cur, delay_us;

    cnt_old = aic_get_timestamp();
    do {
        cnt_cur = aic_get_timestamp();
        delay_us = cnt_cur - cnt_old;
    } while (delay_us < usec);
}

int aic_tmr_init(void)
{
    /* set systmr glb bit */
    AIC_WRITE32(REG_AON_GLB_SYSTMR_EB, BIT_AON_GLB_SYSTMR_EB);

    platform_tick_register(aic_get_tick);

    return AIC_SUCCESS;
}

early_init(aic_tmr_init);
