/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __TIMER_CFG_H__
#define __TIMER_CFG_H__

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_reg_base.h>
#include <irq_chip.h>

#define TMR_26M_FREQ            26000000
#define TMR_2M_FREQ             2000000
#define TMR_32K_FREQ            32768
#define TMR_SLP_CALI_CNT        0

#define TMR_HP_CNT_PER_US(x)    (((u64)(x) * 315) >> 13)
#define TMR_HP_CNT_PER_NS(x)    (((u64)(x) * 78769) >> 11)
#define TMR_HP_NS_PER_CNT(x)    (((u64)(x) * 213) >> 13)
#define TMR_HP_MAX_NS           (0x267627624FLL)

#define REG_AON_GLB_SYSTMR_EB   REG_CKM_EN_GLB_AON_AON_SYSTMR_CNT_EB
#define BIT_AON_GLB_SYSTMR_EB   BIT_CKM_EN_GLB_AON_AON_SYSTMR_CNT_EB

#define TMR_CNT_SHDW_REG        REG_TOP_GLB_TTC_ACTUAL_CNT_L_ND

enum TMR_MODE {
    SCHEDULE_TIMER,
    HP_TIMER,
    STAMP_TIMER,
};

struct aic_timer_cfg {
    u32 index;
    u32 base;
    u32 glb_base;
    u32 glb_bit;
    u32 irq_no;
    u32 freq;
};

static struct aic_timer_cfg aic_timer[2] = {
    {
        .index = 0,
        .base = REG_BASE_TOP_ND_TTC_SYNC,
        //.glb_base = REG_CKM_EN_GLB_MCU_MCU_TMR_EB,
        //.glb_bit = BIT_CKM_EN_GLB_MCU_MCU_TMR_EB,
        .irq_no = IRQ_ND_TIMER,
        .freq = TMR_32K_FREQ,
    },
    {
        .index = 1,
        .base = REG_BASE_BLK_MCU_TMR,
        .glb_base = REG_CKM_EN_GLB_MCU_MCU_TMR_EB,
        .glb_bit = BIT_CKM_EN_GLB_MCU_MCU_TMR_EB,
        .irq_no = IRQ_TMR,
        .freq = TMR_26M_FREQ,
   },
};

/*
 * aic_tmr_phy_hp_get_ns
 *
 * Get current timer event counter value.
 * the value should be ns precision.
 */
__attribute__( ( always_inline ) ) inline static u64 aic_tmr_phy_hp_get_ns(void)
{
    u32 cnt, base = aic_timer[HP_TIMER].base;
    u64 val;

    cnt = 0xFFFFFFFF - *(const volatile u32 *)(base + 0x20);

    val = TMR_HP_CNT_PER_NS(cnt);

    return val;
}

/*
 * aic_tmr_phy_hp_get_raw
 *
 * Get current timer event counter value.
 */
__attribute__( ( always_inline ) ) inline static u32 aic_tmr_phy_hp_get_raw(void)
{
    return *(const volatile u32 *)(REG_BASE_BLK_MCU_TMR + 0x20);
}

#endif /* __TIMER_CFG_H__ */
