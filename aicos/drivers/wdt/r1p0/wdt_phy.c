/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the watchdog software driver, it is watchdog timer
 *    r1p0 phy driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1496ab
#define LOG_MODULE    AMOD_DRV

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <wdt_cfg.h>
#include "../wdt_phy.h"
#include "wdt_reg.h"

#define WDT_UNLOCK_MAGIC 0x31415926
#define WDT_CLK         32768

static u32 wdt_int_status;

struct wdt_reg_val
{
    u32 wdt_reg;
    u32 wdt_reg_reset_value;
};

static void aic_wdt_unlock(void)
{
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_LOCK, WDT_UNLOCK_MAGIC);
}

static void aic_wdt_lock(void)
{
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_LOCK, 0x0);
}

void aic_wdt_kick(void)
{
    aic_wdt_unlock();
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_UPDATE, BIT_WDT_WDT_UPDATE);
    aic_wdt_lock();
}

#ifdef CONFIG_WDT_RETENTION
void aic_wdt_retention_cfg(AIC_WDT_MODE mode)
{
    u32 reg, val = 0;

    reg = AIC_READ32(WDT_RETENTION_REG);
    reg &= WDT_RETENTION_EN_BIT;

    if (mode == AIC_WDT_IRQ_RST_HYBRID_MODE)
        val = WDT_RETENTION_IRQ_BIT | WDT_RETENTION_RST_BIT;
    else if (mode == AIC_WDT_IRQ_MODE_ONLY)
        val = WDT_RETENTION_IRQ_BIT;
    else if (mode == AIC_WDT_RST_MODE_ONLY)
        val = WDT_RETENTION_RST_BIT;

    reg = val | WDT_RETENTION_IRQ_CNT_2S(2) | WDT_RETENTION_RST_CNT_2S(6);

    AIC_WRITE32(WDT_RETENTION_REG, reg);
}

void aic_wdt_retention_enable(bool enabled)
{
    u32 reg;

    reg = AIC_READ32(WDT_RETENTION_REG);

    if (enabled)
        reg |= WDT_RETENTION_EN_BIT;
    else
        reg &= ~WDT_RETENTION_EN_BIT;

    AIC_WRITE32(WDT_RETENTION_REG, reg);
}
#endif

void aic_wdt_mode_config(AIC_WDT_MODE mode)
{
    u32 reg;

    reg = AIC_READ32(WDT_REB_BASE + WDT_REG_WDT_INT_EN);
    reg &= ~(BIT_WDT_WDT_RST_INT_EN | BIT_WDT_WDT_IRQ_INT_EN);
    switch(mode){
        case AIC_WDT_NONE_MODE:
            break;
        case AIC_WDT_IRQ_MODE_ONLY:
            reg |= BIT_WDT_WDT_IRQ_INT_EN;
            break;
        case AIC_WDT_RST_MODE_ONLY:
            reg |= BIT_WDT_WDT_RST_INT_EN;
            break;
        case AIC_WDT_IRQ_RST_HYBRID_MODE:
        default:
            reg |= (BIT_WDT_WDT_RST_INT_EN | BIT_WDT_WDT_IRQ_INT_EN);
            break;
    }

    aic_wdt_unlock();
#ifdef CONFIG_WDT_RETENTION
    aic_wdt_retention_cfg(mode);
#endif
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_INT_EN, reg);
    aic_wdt_lock();
}

void aic_wdt_clear_int_status(AIC_WDT_MODE mode)
{
    u32 reg = 0;

    switch(mode){
        case AIC_WDT_IRQ_MODE_ONLY:
            reg |= BIT_WDT_WDT_IRQ_INT_CLR;
            break;
        case AIC_WDT_RST_MODE_ONLY:
            reg |= BIT_WDT_WDT_RST_INT_CLR;
            break;
        case AIC_WDT_IRQ_RST_HYBRID_MODE:
        default:
            reg |= (BIT_WDT_WDT_RST_INT_CLR | BIT_WDT_WDT_IRQ_INT_CLR);
            break;
    }

    aic_wdt_unlock();
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_INT_CLR, reg);
    aic_wdt_lock();
}

void aic_wdt_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms)
{
    aic_wdt_unlock();
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_IRQ_CNT,
                BIT_WDT_WDT_IRQ_CNT(((rst_timeout_ms - irq_timeout_ms) * WDT_CLK) / 1000));
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_RST_CNT,
                BIT_WDT_WDT_RST_CNT((rst_timeout_ms * WDT_CLK) / 1000));
    aic_wdt_lock();
}

u32 aic_wdt_get_int_status(void)
{
    return wdt_int_status;
}

u32 aic_wdt_get_actual_cnt(void)
{
    return AIC_READ32(WDT_REB_BASE + WDT_REG_WDT_DEBUG_STATUS);;
}

void aic_wdt_start(void)
{
    u32 reg;

    reg = AIC_READ32(WDT_REB_BASE + WDT_REG_WDT_GLB_CTRL);
    aic_wdt_unlock();
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_GLB_CTRL, reg | BIT_WDT_WDT_EN);
    aic_wdt_lock();
#ifdef CONFIG_WDT_RETENTION
    aic_wdt_retention_enable(true);
#endif
}

void aic_wdt_stop(void)
{
    u32 reg;

#ifdef CONFIG_WDT_RETENTION
    aic_wdt_retention_enable(false);
#endif
    reg = AIC_READ32(WDT_REB_BASE + WDT_REG_WDT_GLB_CTRL);
    aic_wdt_unlock();
    AIC_WRITE32(WDT_REB_BASE + WDT_REG_WDT_GLB_CTRL, reg & ~BIT_WDT_WDT_EN);
    aic_wdt_lock();
}

u32 aic_wdt_cnt_to_ms(u32 cnt)
{
    return (cnt * 1000) / WDT_CLK;
}

void aic_wdt_enable(void)
{
    AIC_WRITE32(REG_AON_GLB_WDT_INT_REQ_ENABLE, 0xF);
    AIC_WRITE32(AON_WDT_EB, BIT_AON_GLB_WDT0_EB);
}

void aic_wdt_reset(void)
{
    AIC_WRITE32(REG_GLB_SW_RST_WDT, BIT_GLB_SW_RST_GLB_CLK_AON_WDT);
    AIC_WRITE32(REG_GLB_SW_RST_WDT, 0);
}

void aic_wdt_disable(void)
{
    AIC_WRITE32(AON_WDT_EB, ~BIT_AON_GLB_WDT0_EB);
}

u32 aic_wdt_cnt_get(void)
{
    return AIC_READ32(WDT_REB_BASE + WDT_REG_WDT_DEBUG_STATUS);
}
