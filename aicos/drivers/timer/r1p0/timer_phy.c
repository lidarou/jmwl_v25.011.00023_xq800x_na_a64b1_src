/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of timer module, it is timer
 *    r1p0 phy driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13d39e
#define LOG_MODULE    AMOD_TIMER

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_io.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <core_op.h>
#include <slp_inc.h>
#include <timer_cfg.h>
#include <timer_inc.h>

#include "../timer_phy.h"

#define TMR_VER             0x0
#define TMR_CTRL            0x4
#define TMR_CNT_UPDATE      0x8
#define TMR_INT_STATUS      0xC
#define TMR_INT_CLR         0x10
#define TMR_SET_CNT_H       0x14
#define TMR_SET_CNT_L       0x18
#define TMR_VAL_CNT_H       0x1C
#define TMR_VAL_CNT_L       0x20
#define TMR_UPDATE_ST       0x24

#define FREERUN_CNT_L       (REG_BASE_AON_SYSTMR + 0x0)
#define FREERUN_CNT_H       (REG_BASE_AON_SYSTMR + 0x4)
#define SYSTMR_CNT_L        (REG_BASE_AON_SYSTMR + 0x8)
#define SYSTMR_CNT_H        (REG_BASE_AON_SYSTMR + 0xC)

/* TMR_CTRL */
#define TMR_CTRL_EN         BIT_0
#define TMR_CTRL_IRQ_EN     BIT_1
#define TMR_CTRL_PERIOD_EN  BIT_2

/* TMR_CNT_UPDATE */
#define TMR_CNT_CLR_BIT     BIT_1
#define TMR_CNT_SET_BIT     BIT_0

/* TMR_INT_CLR */
#define TMR_INT_CLR_BIT     BIT_0

#define TMR_HP_VAL_MAX      0xFFFFFFFF

/*
 * aic_frtmr_get - use for us delay scenarios
 * Note that!!!
 * The return usec should be multiple of 30.518us.
 */
u64 aic_frtmr_get(void)
{
    u64 val;

    val = AIC_READ32(FREERUN_CNT_H);
    if (val)
        val = val << 32;
    val += AIC_READ32(FREERUN_CNT_L);

    val = val * 1000 *1000 / TMR_32K_FREQ;

    return val;
}

/*
 * aic_get_frtmr_cycle - get freerun timer cycle counter
 */
u64 aic_get_frtmr_cycle(void)
{
    u64 val;

    val = AIC_READ32(FREERUN_CNT_H);
    if (val)
        val = val << 32;
    val += AIC_READ32(FREERUN_CNT_L);

    return val;
}

u64 aic_systmr_get(void)
{
    u64 val;

    val = AIC_READ32(SYSTMR_CNT_H);
    if (val)
        val = val << 32;
    val += AIC_READ32(SYSTMR_CNT_L);
    /*
     * Why need do this?!
     * Beacuse hw implement ms counter by freerun counter,
     * but only divide 32 not 32.768, it causes the counter not precisely.
     * so need tu cali it, the function is cnt * 32 * 1000 / 32768.
     */
    val = val * 1000 / 1024;

    return val;
}

/*
 * aic_tmr_phy_get_cnt
 *
 * Get current timer event counter value.
 * the value should be us precision.
 */
u32 aic_tmr_phy_get_cnt(u32 index)
{
    u32 base = aic_timer[index].base;
    u32 cnt, val;

    cnt = AIC_READ32(base + TMR_VAL_CNT_L);
    val = cnt * 1000 / aic_timer[index].freq;

    return val;
}

/*
 * aic_tmr_phy_set_cnt
 *
 * Set current timer counter value.
 */
void aic_tmr_phy_set_cnt(u32 index, u32 cnt)
{
    u32 base = aic_timer[index].base;

    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    AIC_WRITE32(base + TMR_SET_CNT_L, cnt);
    AIC_WRITE32(base + TMR_CNT_UPDATE, TMR_CNT_SET_BIT);
    /* timer bug in sleep mode, time update not complete */
    /*
     * Comment out here, move this code to sleep mode when chip suspend.
     * Because it causes 90us for checking the complete status.
     */
    //while(AIC_READ32(base + TMR_UPDATE_ST));
    AIC_INT_RESTORE
}

void aic_tmr_phy_int_clr(u32 index)
{
    u32 base = aic_timer[index].base;

    AIC_WRITE32(base + TMR_INT_CLR, TMR_INT_CLR_BIT);
}

static void aic_tmr_phy_hp_init(void)
{
    struct aic_timer_cfg *cfg = &aic_timer[HP_TIMER];

    AIC_WRITE32(cfg->glb_base, cfg->glb_bit);
    AIC_DSB();
    AIC_WRITE32(cfg->base + TMR_CTRL, TMR_CTRL_EN | TMR_CTRL_PERIOD_EN);
    AIC_WRITE32(cfg->base + TMR_CNT_UPDATE, TMR_CNT_CLR_BIT);
    AIC_WRITE32(cfg->base + TMR_INT_CLR, TMR_INT_CLR_BIT);
    AIC_WRITE32(cfg->base + TMR_SET_CNT_H, 0xFFFFFFFF);
    AIC_WRITE32(cfg->base + TMR_SET_CNT_L, 0xFFFFFFFF);
    AIC_WRITE32(cfg->base + TMR_CNT_UPDATE, TMR_CNT_SET_BIT);
}

/*
 * Timer suspend function for timer chip bug, it needs to check the
 * timer update status to make sure the timer count is updated.
 */
void aic_tmr_phy_suspend(void *handle)
{
    struct aic_timer_cfg *cfg = (struct aic_timer_cfg *)handle;
    /* timer bug in sleep mode, time update not complete */
    while(AIC_READ32(cfg->base + TMR_UPDATE_ST));

    /* disable hp timer eb */
    cfg = &aic_timer[HP_TIMER];
    AIC_WRITE32(cfg->glb_base, 0);
}

void aic_tmr_phy_resume(void *handle)
{
    struct aic_timer_cfg *cfg = (struct aic_timer_cfg *)handle;
    /* HP timer resume */
    if (AIC_READ32(cfg->glb_base))
        return ;

    /* set hp timer glb and init default count */
    aic_tmr_phy_hp_init();
}

void aic_tmr_phy_deinit(u32 index)
{
    u32 base = aic_timer[index].base;

    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

    /* fix timer update not complete bug */
    while(AIC_READ32(base + TMR_UPDATE_ST));

    AIC_WRITE32(base + TMR_CNT_UPDATE, TMR_CNT_CLR_BIT);
    AIC_WRITE32(base + TMR_INT_CLR, TMR_INT_CLR_BIT);

    /* fix timer update not complete bug */
    while(AIC_READ32(base + TMR_UPDATE_ST));
    AIC_INT_RESTORE
}

void aic_tmr_phy_init(u32 index)
{
    u32 base = aic_timer[index].base;
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

    AIC_WRITE32(base + TMR_CTRL, TMR_CTRL_EN | TMR_CTRL_IRQ_EN);
    AIC_WRITE32(base + TMR_CNT_UPDATE, TMR_CNT_CLR_BIT);
    AIC_WRITE32(base + TMR_INT_CLR, TMR_INT_CLR_BIT);
    AIC_INT_RESTORE

    /*
     * aon timers need to suspend to fix timer bug.
     * reguest sleep hook for hp timer resume
     */
    slp_register_sleep_hook_ex(aic_tmr_phy_suspend, &aic_timer[SCHEDULE_TIMER],
                               aic_tmr_phy_resume, &aic_timer[HP_TIMER],
                               LV_HIGH);
}

void aic_systmr_init(void)
{
    AIC_WRITE32(REG_AON_GLB_SYSTMR_EB, BIT_AON_GLB_SYSTMR_EB);

    aic_tmr_phy_hp_init();
}

