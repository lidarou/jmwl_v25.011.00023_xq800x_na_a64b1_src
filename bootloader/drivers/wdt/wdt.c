/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of wdt reset module
*/
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aon_glb.h>
#include <top_glb.h>
#include <console/console.h>
#include <efuse.h>
#include <initcall.h>
#include <reg_base.h>
#include <stdlib.h>
#include <timer.h>
#include <wdt.h>
#include <wdt_cfg.h>

#define WDT_REG_WDT_IP_INFO                                                     (0x0)
#define WDT_REG_WDT_GLB_CTRL                                                    (0x4)
#define WDT_REG_WDT_UPDATE                                                      (0x8)
#define WDT_REG_WDT_INT_EN                                                      (0xc)
#define WDT_REG_WDT_INT_STATUS_RAW                                              (0x10)
#define WDT_REG_WDT_INT_STATUS                                                  (0x14)
#define WDT_REG_WDT_INT_CLR                                                     (0x18)
#define WDT_REG_WDT_IRQ_CNT                                                     (0x1c)
#define WDT_REG_WDT_RST_CNT                                                     (0x20)
#define WDT_REG_WDT_LOCK                                                        (0x24)
#define WDT_REG_WDT_DEBUG_STATUS                                                (0x28)

#define WDT_REG_WDT_GLB_CTRL_SET                                                (0x1004)
#define WDT_REG_WDT_UPDATE_SET                                                  (0x1008)
#define WDT_REG_WDT_INT_EN_SET                                                  (0x100c)
#define WDT_REG_WDT_INT_CLR_SET                                                 (0x1018)
#define WDT_REG_WDT_IRQ_CNT_SET                                                 (0x101c)
#define WDT_REG_WDT_RST_CNT_SET                                                 (0x1020)
#define WDT_REG_WDT_LOCK_SET                                                    (0x1024)
#define WDT_REG_WDT_GLB_CTRL_CLR                                                (0x2004)
#define WDT_REG_WDT_UPDATE_CLR                                                  (0x2008)
#define WDT_REG_WDT_INT_EN_CLR                                                  (0x200c)
#define WDT_REG_WDT_INT_CLR_CLR                                                 (0x2018)
#define WDT_REG_WDT_IRQ_CNT_CLR                                                 (0x201c)
#define WDT_REG_WDT_RST_CNT_CLR                                                 (0x2020)
#define WDT_REG_WDT_LOCK_CLR                                                    (0x2024)

/*WDT_REG_WDT_IP_INFO*/
#define BIT_WDT_IP_MAIN_VERSION(x)                                              (((x) & 0xff) << 8)
#define BIT_WDT_IP_SUB_VERSION(x)                                               (((x) & 0xff))

/*WDT_REG_WDT_GLB_CTRL*/
#define BIT_WDT_WDT_EN                                                          BIT(0)

/*WDT_REG_WDT_UPDATE*/
#define BIT_WDT_WDT_UPDATE                                                      BIT(0)

/*WDT_REG_WDT_INT_EN*/
#define BIT_WDT_WDT_RST_INT_EN                                                  BIT(1)
#define BIT_WDT_WDT_IRQ_INT_EN                                                  BIT(0)

/*WDT_REG_WDT_INT_STATUS_RAW*/
#define BIT_WDT_WDT_RST_INT_STATUS_RAW                                          BIT(1)
#define BIT_WDT_WDT_IRQ_INT_STATUS_RAW                                          BIT(0)

/*WDT_REG_WDT_INT_STATUS*/
#define BIT_WDT_WDT_RST_INT_STATUS                                              BIT(1)
#define BIT_WDT_WDT_IRQ_INT_STATUS                                              BIT(0)

/*WDT_REG_WDT_INT_CLR*/
#define BIT_WDT_WDT_RST_INT_CLR                                                 BIT(1)
#define BIT_WDT_WDT_IRQ_INT_CLR                                                 BIT(0)

/*WDT_REG_WDT_IRQ_CNT*/
#define BIT_WDT_WDT_IRQ_CNT(x)                                                  (((x) & 0xffffffff))

/*WDT_REG_WDT_RST_CNT*/
#define BIT_WDT_WDT_RST_CNT(x)                                                  (((x) & 0xffffffff))

/*WDT_REG_WDT_LOCK*/
#define BIT_WDT_WDT_LOCK(x)                                                     (((x) & 0xffffffff))

/*WDT_REG_WDT_DEBUG_STATUS*/
#define BIT_WDT_WDT_ACTUAL_CNT(x)                                               (((x) & 0xffffffff))

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
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_LOCK, WDT_UNLOCK_MAGIC);
}

static void aic_wdt_lock(void)
{
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_LOCK, 0x0);
}

void aic_wdt_kick(void)
{
    aic_wdt_unlock();
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_UPDATE, BIT_WDT_WDT_UPDATE);
    aic_wdt_lock();
}

void aic_wdt_mode_config(AIC_WDT_MODE mode)
{
    u32 reg;

    reg = AIC_READ32(REG_BASE_AON_WDT0 + WDT_REG_WDT_INT_EN);
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
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_INT_EN, reg);
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
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_INT_CLR, reg);
    aic_wdt_lock();
}

void aic_wdt_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms)
{
    aic_wdt_unlock();
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_IRQ_CNT,
                BIT_WDT_WDT_IRQ_CNT(((rst_timeout_ms - irq_timeout_ms) * WDT_CLK) / 1000));
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_RST_CNT,
                BIT_WDT_WDT_RST_CNT((rst_timeout_ms * WDT_CLK) / 1000));
    aic_wdt_lock();
}

u32 aic_wdt_get_int_status(void)
{
    return wdt_int_status;
}

u32 aic_wdt_get_actual_cnt(void)
{
    return AIC_READ32(REG_BASE_AON_WDT0 + WDT_REG_WDT_DEBUG_STATUS);;
}

void aic_wdt_start(void)
{
    u32 reg;

    reg = AIC_READ32(REG_BASE_AON_WDT0 + WDT_REG_WDT_GLB_CTRL);
    aic_wdt_unlock();
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_GLB_CTRL, reg | BIT_WDT_WDT_EN);
    aic_wdt_lock();
}

void aic_wdt_stop(void)
{
    u32 reg;

    reg = AIC_READ32(REG_BASE_AON_WDT0 + WDT_REG_WDT_GLB_CTRL);
    aic_wdt_unlock();
    AIC_WRITE32(REG_BASE_AON_WDT0 + WDT_REG_WDT_GLB_CTRL, reg & ~BIT_WDT_WDT_EN);
    aic_wdt_lock();
}

u32 aic_wdt_cnt_to_ms(u32 cnt)
{
    return (cnt * 1000) / WDT_CLK;
}

void aic_wdt_enable(void)
{
    AIC_WRITE32(REG_AON_GLB_WDT_INT_REQ_ENABLE, 0xF);
    AIC_WRITE32(REG_AON_GLB_WDT0_EB, BIT_AON_GLB_WDT0_EB);
}

void aic_wdt_reset(void)
{
    AIC_WRITE32(REG_TOP_GLB_SW_RST_WDT0, BIT_TOP_GLB_SW_RST_GLB_CLK_AON_WDT0);
    AIC_WRITE32(REG_TOP_GLB_SW_RST_WDT0, 0);
}

void aic_wdt_disable(void)
{
    AIC_WRITE32(REG_AON_GLB_WDT0_EB, ~BIT_AON_GLB_WDT0_EB);
}

void wdt_reset(u32 time_ms)
{
    aic_wdt_reset();
    aic_wdt_enable();
    aic_wdt_mode_config(AIC_WDT_RST_MODE_ONLY);
    aic_wdt_set_timeout(time_ms, time_ms);
    aic_wdt_start();
    aic_wdt_kick();
    while(1)
        ;
}