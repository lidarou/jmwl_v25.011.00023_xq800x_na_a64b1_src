/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of wdt module.
 *   It includes all the wdt phy function api.
*/
#ifndef __WDT_PHY_H__
#define __WDT_PHY_H__
#include <wdt.h>

void aic_wdt_kick(void);
void aic_wdt_mode_config(AIC_WDT_MODE mode);
void aic_wdt_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms);
void aic_wdt_start(void);
void aic_wdt_stop(void);
void aic_wdt_enable(void);
void aic_wdt_disable(void);
void aic_wdt_reset(void);
u32 aic_wdt_cnt_get(void);

#endif /* __WDT_PHY_H__ */
