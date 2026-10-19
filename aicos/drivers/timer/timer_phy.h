/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of timer module, it is timer
 *    phy driver header file.
*/

#ifndef _TIMER_PHY_H_
#define _TIMER_PHY_H_

#include <aic_reg_base.h>
#include <aic_type.h>

#define __TIMER_CODE __attribute__((section(".timer_code")))

u64 __TIMER_CODE aic_frtmr_get(void);
u64 __TIMER_CODE aic_systmr_get(void);
u64 aic_get_frtmr_cycle(void);
u32 aic_tmr_phy_get_cnt(u32 index);
void aic_tmr_phy_set_cnt(u32 index, u32 cnt);
void aic_tmr_phy_int_clr(u32 index);
void aic_tmr_phy_deinit(u32 index);
void aic_tmr_phy_init(u32 index);
void aic_systmr_init(void);
void aic_tmr_phy_hp_fiq_cfg(u32 us);

#endif //_TIMER_PHY_H_

