/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the pmic watchdog phy driver, it is pmic watchdog timer
 *    r2p0 phy driver.
*/

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "pmic_wdt_reg.h"
#include "../pmic_wdt_phy.h"

#define WDT_UNLOCK_MAGIC 0x31415926
#define WDT_CLK         32768

static u32 wdt_int_status;

static void pmic_wdt_unlock(void)
{
    pmic_writel(WDT_REG_WDT_LOCK, WDT_UNLOCK_MAGIC);
}

static void pmic_wdt_lock(void)
{
    pmic_writel(WDT_REG_WDT_LOCK, 0x0);
}

void pmic_wdt_kick(void)
{
    pmic_wdt_unlock();
    pmic_writel(WDT_REG_WDT_UPDATE, BIT_WDT_WDT_UPDATE);
    pmic_wdt_lock();
}

void pmic_wdt_mode_config(PMIC_WDT_MODE mode)
{
    u32 reg;

    reg = pmic_readl(WDT_REG_WDT_INT_EN);
    reg &= ~(BIT_WDT_WDT_RST_INT_EN | BIT_WDT_WDT_IRQ_INT_EN);
    switch(mode){
        case PMIC_WDT_NONE_MODE:
            break;
        case PMIC_WDT_IRQ_MODE_ONLY:
            reg |= BIT_WDT_WDT_IRQ_INT_EN;
            break;
        case PMIC_WDT_RST_MODE_ONLY:
            reg |= BIT_WDT_WDT_RST_INT_EN;
            break;
        case PMIC_WDT_IRQ_RST_HYBRID_MODE:
        default:
            reg |= (BIT_WDT_WDT_RST_INT_EN | BIT_WDT_WDT_IRQ_INT_EN);
            break;
    }

    pmic_wdt_unlock();
    pmic_writel(WDT_REG_WDT_INT_EN, reg);
    pmic_wdt_lock();
}

void pmic_wdt_clear_int_status(PMIC_WDT_MODE mode)
{
    u32 reg = 0;

    switch(mode){
        case PMIC_WDT_IRQ_MODE_ONLY:
            reg |= BIT_WDT_WDT_IRQ_INT_CLR;
            break;
        case PMIC_WDT_RST_MODE_ONLY:
            reg |= BIT_WDT_WDT_RST_INT_CLR;
            break;
        case PMIC_WDT_IRQ_RST_HYBRID_MODE:
        default:
            reg |= (BIT_WDT_WDT_RST_INT_CLR | BIT_WDT_WDT_IRQ_INT_CLR);
            break;
    }

    pmic_wdt_unlock();
    pmic_writel(WDT_REG_WDT_INT_CLR, reg);
    pmic_wdt_lock();
}

void pmic_wdt_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms)
{
    pmic_wdt_unlock();
    pmic_writel(WDT_REG_WDT_IRQ_CNT,
                BIT_WDT_WDT_IRQ_CNT(((rst_timeout_ms - irq_timeout_ms) * WDT_CLK) / 1000));
    pmic_writel(WDT_REG_WDT_RST_CNT,
                BIT_WDT_WDT_RST_CNT((rst_timeout_ms * WDT_CLK) / 1000));
    pmic_wdt_lock();
}

u32 pmic_wdt_get_int_status(void)
{
    return wdt_int_status;
}

u32 pmic_wdt_get_actual_cnt(void)
{
    return pmic_readl(WDT_REG_WDT_DEBUG_STATUS);;
}

void pmic_wdt_start(void)
{
    pmic_wdt_unlock();
    pmic_writel(WDT_REG_WDT_GLB_CTRL, BIT_WDT_WDT_EN);
    pmic_wdt_lock();
}

void pmic_wdt_stop(void)
{
    pmic_wdt_unlock();
    pmic_writel(WDT_REG_WDT_GLB_CTRL, 0);
    pmic_wdt_lock();
}

u32 pmic_wdt_cnt_to_ms(u32 cnt)
{
    return (cnt * 1000) / WDT_CLK;
}

struct pmic_wdt_ops pmic_wdt_phy = {
    .kick = pmic_wdt_kick,
    .mode_config = pmic_wdt_mode_config,
    .timeout_set = pmic_wdt_set_timeout,
    .start = pmic_wdt_start,
    .stop = pmic_wdt_stop,
};

int pmic_wdt_phy_probe(void)
{
    pmic_phy_ops_register(&pmic_wdt_phy);

    return 0;
}
early_init(pmic_wdt_phy_probe);
