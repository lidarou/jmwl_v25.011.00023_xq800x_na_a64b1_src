/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines pmic watchdog driver api.
 *    The pmic watchdog api will be called by aicos user functions.
 */

#ifndef __WDT_H__
#define __WDT_H__

typedef enum {
    AIC_WDT_IRQ_MODE_ONLY = 0,
    AIC_WDT_RST_MODE_ONLY,
    AIC_WDT_IRQ_RST_HYBRID_MODE,
    AIC_WDT_NONE_MODE
} AIC_WDT_MODE;

void aic_wdt_kick(void);
void aic_wdt_mode_config(AIC_WDT_MODE mode);
void aic_wdt_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms);
void aic_wdt_start(void);
void aic_wdt_stop(void);
void aic_wdt_enable(void);
void aic_wdt_disable(void);
void aic_wdt_reset(void);
void wdt_reset(u32 time_ms);

#endif /* __WDT_H__ */
