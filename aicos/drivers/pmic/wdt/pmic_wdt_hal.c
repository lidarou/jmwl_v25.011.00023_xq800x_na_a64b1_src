/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of pmic watchdog HAL module
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x141595
#define LOG_MODULE    AMOD_PMIC

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pmic_irq.h>
#include "pmic_wdt_phy.h"

#define PMIC_WDT_KICK_TIMER_NAME "pmic_wdt_period_kick"

aos_timer_ptr   pmic_wdt_kick_timer;
struct pmic_wdt_ops *pmic_wdt = NULL;

void pmic_watchdog_ping(void *param)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    pmic_wdt->kick();
    AIC_INT_RESTORE
}

void pmic_watchdog_start(void)
{
    AIC_INT_SAVE_AREA

#ifdef CONFIG_PMIC_WATCHDOG_INDEPENDENT_THREAD
    aos_activate_timer(pmic_wdt_kick_timer);
#endif

    AIC_INT_DISABLE
    if (pmic_wdt->start)
        pmic_wdt->start();
    AIC_INT_RESTORE;
}

void pmic_watchdog_stop(void)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    if (pmic_wdt->stop)
        pmic_wdt->stop();

#ifdef CONFIG_WATCHDOG_INDEPENDENT_THREAD
    aos_deactivate_timer(pmic_wdt_kick_timer);
#endif

    AIC_INT_RESTORE;
}

void pmic_watchdog_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    if (pmic_wdt->timeout_set)
        pmic_wdt->timeout_set(irq_timeout_ms, rst_timeout_ms);
    AIC_INT_RESTORE;
}

void pmic_watchdog_mode_config(PMIC_WDT_MODE mode)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    if (pmic_wdt->mode_config)
        pmic_wdt->mode_config(mode);
    AIC_INT_RESTORE;
}

void pmic_watchdog_irq_handler(void *param)
{
    if (pmic_wdt->stop)
        pmic_wdt->stop();
    /*call system dump & system reset later.*/

    AICX_ASSERT_IFX(0, 0x1a4e08, "", "pmic watchdog timeout!!!");
}

void pmic_phy_ops_register(struct pmic_wdt_ops *pmic_wdt_phy_ops)
{
    pmic_wdt = pmic_wdt_phy_ops;
}

void pmic_wdt_reset(u32 time_ms)
{
    if (pmic_wdt->mode_config)
        pmic_wdt->mode_config(PMIC_WDT_RST_MODE_ONLY);

    pmic_wdt->timeout_set(time_ms, time_ms);

    if (pmic_wdt->start)
        pmic_wdt->start();

    if (pmic_wdt->kick)
        pmic_wdt->kick();
}

int pmic_watchdog_init(void)
{
    if (pmic_wdt->init)
        pmic_wdt->init();

    register_pmic_irq(PMIC_IRQ_WDT, pmic_watchdog_irq_handler, NULL);

    /*requst timer thread for watchdog ping*/
#ifdef CONFIG_PMIC_WATCHDOG_INDEPENDENT_THREAD
    if (pmic_wdt->mode_config)
        pmic_wdt->mode_config(PMIC_WDT_IRQ_RST_HYBRID_MODE);

    pmic_wdt->timeout_set(12000, 15000);
    pmic_wdt_kick_timer = aos_create_timer(PMIC_WDT_KICK_TIMER_NAME,
                                         pmic_wdt->kick, NULL,
                                         3000,
                                         AOS_AUTO_ACTIVATE, AOS_BE_PERIOD);
    AIC_ASSERT(pmic_wdt_kick_timer);

    if (pmic_wdt->start)
        pmic_wdt->start();

    if (pmic_wdt->kick)
        pmic_wdt->kick();
#endif

    return 0;
}
AIC_DRV_L1_INIT_EXPORT(pmic_watchdog_init);
