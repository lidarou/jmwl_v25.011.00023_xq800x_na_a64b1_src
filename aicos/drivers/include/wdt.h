/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines watchdog driver api.
 *    The watchdog api will be called by aicos user functions.
 */

#ifndef __WDT_H__
#define __WDT_H__

typedef enum {
    AIC_WDT_IRQ_MODE_ONLY = 0,
    AIC_WDT_RST_MODE_ONLY,
    AIC_WDT_IRQ_RST_HYBRID_MODE,
    AIC_WDT_NONE_MODE
} AIC_WDT_MODE;

/**
 * aic_watchdog_enable - integration watchdog api, use call aic_watchdog_enable start watchdog
 * then aic_watchdog_ping feeder watchdog.
 * @mode: watchdog work mode.
 * @irq_timeout_ms: stop feeder watchdog more than irq_timeout_ms, will generate irq to cpu.
 * @rst_timeout_ms: stop feeder watchdog more than irq_timeout_ms, will generate hardware reset signal to reset whole system.
 * non return.
 */
#define aic_watchdog_enable(mode, irq_timeout_ms, rst_timeout_ms) do { \
        aic_watchdog_mode_config(mode); \
        aic_watchdog_set_timeout(irq_timeout_ms, rst_timeout_ms);\
        aic_watchdog_start();\
    } while(0)

/**
 * aic_watchdog_ping - feeder watchdog in disable interrupt context.
 * @param: NULL, aos timer callback param.
 * non return.
 */
void aic_watchdog_ping(void *param);

/**
 * aic_watchdog_kick - feeder watchdog, interrupt enable allow preemption.
 * non return.
 */
void aic_watchdog_kick(void);

/**
 * aic_watchdog_start - watchdog start to work.
 * non return.
 */
void aic_watchdog_start(void);

/**
 * aic_watchdog_start - watchdog stop working.
 * non return.
 */
void aic_watchdog_stop(void);

/**
 * aic_watchdog_mode_config - config watchdog work mode.
 * @mode:watchdog work mode.
 * non return.
 */
void aic_watchdog_mode_config(AIC_WDT_MODE mode);

/**
 * aic_watchdog_set_timeout - watchdog timeout parameters config.
 * @irq_timeout_ms: stop feeder watchdog more than irq_timeout_ms, will generate irq to cpu.
 * @rst_timeout_ms: stop feeder watchdog more than irq_timeout_ms, will generate hardware reset signal to reset whole system.
 * non return.
 */
void aic_watchdog_set_timeout(u32 irq_timeout_ms, u32 rst_timeout_ms);

/**
 * wdt_reset - reset system after time_ms, used by system module, user should sysoff api.
 * @time_ms: system reset take effect time.
 * non return.
 */
void wdt_reset(u32 time_ms);

/**
 * aic_watchdog_cnt_get - get current watchdog timer countdown count.
 * return current watchdog timer countdown count.
 */
u32 aic_watchdog_cnt_get(void);

#endif /* __WDT_H__ */
