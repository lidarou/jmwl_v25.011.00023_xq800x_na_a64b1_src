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

#ifndef __PMIC_WDT_H__
#define __PMIC_WDT_H__

typedef enum {
    PMIC_WDT_IRQ_MODE_ONLY = 0,
    PMIC_WDT_RST_MODE_ONLY,
    PMIC_WDT_IRQ_RST_HYBRID_MODE,
    PMIC_WDT_NONE_MODE
} PMIC_WDT_MODE;

void pmic_watchdog_ping(void *param);
void pmic_watchdog_start(void);
void pmic_watchdog_stop(void);
void pmic_watchdog_mode_config(PMIC_WDT_MODE mode);
void pmic_watchdog_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms);
void pmic_wdt_reset(u32 time_ms);

#endif /* __PMIC_WDT_H__ */
