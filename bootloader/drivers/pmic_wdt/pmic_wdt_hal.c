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
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <console/console.h>
#include <initcall.h>
#include "pmic_wdt_phy.h"

struct pmic_wdt_ops *pmic_wdt = NULL;

void pmic_watchdog_ping(void *param)
{
    pmic_wdt->kick();
}

void pmic_watchdog_start(void)
{
    if (pmic_wdt->start)
        pmic_wdt->start();
}

void pmic_watchdog_stop(void)
{
    if (pmic_wdt->stop)
        pmic_wdt->stop();
}

void pmic_watchdog_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms)
{
    if (pmic_wdt->timeout_set)
        pmic_wdt->timeout_set(irq_timeout_ms, rst_timeout_ms);
}

void pmic_watchdog_mode_config(PMIC_WDT_MODE mode)
{
    if (pmic_wdt->mode_config)
        pmic_wdt->mode_config(mode);
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
