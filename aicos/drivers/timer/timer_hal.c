/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of timer module, it is timer
 *    common hal driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1243bd
#define LOG_MODULE    AMOD_TIMER

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <irq.h>
#include <irq_chip.h>
#include <timer_cfg.h>
#include <timer_inc.h>
#include <timer_tx_inc.h>

#include "timer_phy.h"

#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#include <tx_port.h>
#include <tx_thread.h>
#include <tx_timer.h>

VOID _tx_timer_interrupt(void);
#elif defined(CONFIG_RTOS_LITEOS_M)
#include "los_tick.h"
#endif

/*
 * aic_get_ms
 * Get the system tick count, it is system boot time or
 * onwall time, and it is 1ms per tick.
 * Notice!!!
 * The return value is 64 bit.
 */
u64 __TIMER_CODE aic_get_ms(void)
{
    return aic_systmr_get();
}

/*
 * aic_get_us
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 64 bit, and its unit is us.
 */
u64 __TIMER_CODE aic_get_us(void)
{
    return aic_frtmr_get();
}

/*
 * aic_get_ns
 * Get the ns count
 * Notice!!!
 * The return value is 64 bit.
 */
u64 __TIMER_CODE aic_get_ns(void)
{
    return aic_tmr_phy_hp_get_ns();
}

/*
 * aic_get_tmr_raw_step
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 64 bit, cycle counter is 30.5176us per step.
 */
u64 __TIMER_CODE aic_get_tmr_raw_step(void)
{
    return aic_get_frtmr_cycle();
}

/*
 * aic_get_tick
 * Get the system tick count, it is system boot time or
 * onwall time, and it is 1ms per tick.
 * Notice!!!
 * The return value is 64 bit.
 */
u64 __TIMER_CODE aic_get_tick(void)
{
    return aic_systmr_get();
}

/*
 * aic_get_timestamp
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 32 bit, and its unit is us.
 */
u32 __TIMER_CODE aic_get_timestamp(void)
{
    return (u32)aic_frtmr_get();
}

/*
 * aic_get_system_cycle
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 64 bit.
 */
u64 __TIMER_CODE aic_get_system_cycle(void)
{
    return aic_get_frtmr_cycle();
}

/*
 * aic_get_boot_time
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 64 bit, unit is us.
 */
u64 __TIMER_CODE aic_get_boot_time(void)
{
    return aic_get_frtmr_cycle() * 1000000 / TMR_32K_FREQ;
}

/*
 * aic_ndelay - use for ns delay scenarios
 * Note that!!!
 * This nsec should be multiple of 38.46ns.
 */
void __TIMER_CODE aic_ndelay(u32 nsec)
{
    u32 cnt_old, cnt_cur, delta_cnt, delay_cnt;

    cnt_old = aic_tmr_phy_hp_get_raw();
    delay_cnt = (u32)TMR_HP_NS_PER_CNT(nsec);
    do {
        cnt_cur = aic_tmr_phy_hp_get_raw();
        if (cnt_old >= cnt_cur)
            delta_cnt = cnt_old - cnt_cur;
        else
            delta_cnt = 0xFFFFFFFF - cnt_cur + cnt_old;
    } while (delta_cnt < delay_cnt);
}

/*
 * aic_udelay - use for us delay scenarios
 * Note that!!!
 * This usec should be multiple of 30.518us.
 */
void __TIMER_CODE aic_udelay(u32 usec)
{
    u64 cnt_old, cnt_cur, delay_us;

    cnt_old = aic_frtmr_get();
    do {
        cnt_cur = aic_frtmr_get();
        delay_us = cnt_cur - cnt_old;
    } while (delay_us < usec);
}

/*
 * aic_udelay - use for ms delay scenarios
 */
void __TIMER_CODE aic_mdelay(u32 msec)
{
    u64 cnt_old, cnt_cur;

    /*
     * system tick has 1ms error, if msec less than 1s,
     * use udelay will be better.
     */
    if (msec <= 1000) {
        aic_udelay(msec * 1000);
    } else {
        cnt_old = aic_systmr_get();
        do {
            cnt_cur = aic_systmr_get();
        } while ((cnt_cur - cnt_old) < msec);
    }
}

/*
 * aic_udelay - use for seconds delay scenarios
 */
void __TIMER_CODE aic_delay(u32 sec)
{
    u64 cnt_old, cnt_cur, delay_ms;

    delay_ms = (u64)sec * 1000;
    cnt_old = aic_systmr_get();
    do {
        cnt_cur = aic_systmr_get();
    } while ((cnt_cur - cnt_old) < delay_ms);
}

/*
 * aic_tmr_slp_cali_tmr - calculate timer sleep time for glitch free clock.
 * The return value unit is us.
 */
u64 aic_tmr_slp_cali_tmr(u64 slp_cnt)
{
#ifdef TMR_SLP_CALI_CNT
    return slp_cnt * TMR_SLP_CALI_CNT * 1000000 / TMR_32K_FREQ;
#else
    return 0;
#endif
}

#if defined(CONFIG_DBG_CPPC_SAMP) && defined(CONFIG_CPU_CORTEX_R5)
void aic_tmr_samp_cfg(u32 us)
{
    aic_tmr_phy_hp_fiq_cfg(us);
}

ISR_RET_E aic_tmr_frq_handler(void *param)
{
    aic_tmr_phy_int_clr(HP_TIMER);

    return NONE_WORKER;
}
#endif

ISR_RET_E aic_tmr_irq_handler(void *param)
{
    struct aic_timer_cfg *cfg = param;
    u32 i = cfg->index;

    aic_tmr_phy_int_clr(i);

#if defined(CONFIG_RTOS_THREADX)
#ifndef CONFIG_TIMER_TX_NO_TICK_IRQ
    /* set period tick count */
    i = cfg->freq / TX_TIMER_TICKS_PER_SECOND;

    aic_tmr_phy_set_cnt(SCHEDULE_TIMER, i);
#endif /* CONFIG_TIMER_TX_NO_TICK_IRQ */
    _tx_timer_interrupt();
#elif defined(CONFIG_RTOS_LITEOS_M)
    OsTickHandler();
#endif /* CONFIG_RTOS_THREADX || CONFIG_RTOS_LITEOS_M */

    return NONE_WORKER;
}

int aic_tmr_init(void)
{
    u32 timer_max = sizeof(aic_timer) / sizeof(*aic_timer);
    u32 i;

    for (i = 0; i < timer_max; i++) {
        register_irq(aic_timer[i].irq_no, aic_tmr_irq_handler,
                    &aic_timer[i], NULL, NULL);

        if (i == SCHEDULE_TIMER) {
            /* tmr set glb */
            if (aic_timer[i].glb_base)
                AIC_WRITE32(aic_timer[i].glb_base, aic_timer[i].glb_bit);
            /* tmr phy int */
            aic_tmr_phy_init(i);
        }
    }

#if defined(CONFIG_DBG_CPPC_SAMP) && defined(CONFIG_CPU_CORTEX_R5)
    register_frq(aic_timer[HP_TIMER].irq_no, aic_tmr_frq_handler, NULL);
#endif
#if defined(CONFIG_RTOS_THREADX)
#ifndef CONFIG_TIMER_TX_NO_TICK_IRQ
    /* set period tick count */
    i = aic_timer[SCHEDULE_TIMER].freq / TX_TIMER_TICKS_PER_SECOND;

    aic_tmr_phy_set_cnt(SCHEDULE_TIMER, i);
#endif
#endif

    return AIC_SUCCESS;
}

AIC_EARLY_INIT_EXPORT(aic_tmr_init);

void aic_tmr_ealy_init(void)
{
    aic_systmr_init();
}

#ifdef CONFIG_TIMER_TX_NO_TICK_IRQ
/**
 * The following api is used for threadx no systick functions.
 * No systick function will use AON timer0(32KHz) for timer events.
 */

#ifdef CONFIG_TIMER_TX_NO_TICK_DBG
/*
 * tmr_dbg_info: define timer debug paramater info
 * @tmsp: debug timer stamp
 * @id: debug index number
 * @fn_name: it could be timer callback or thread name
 * @onwall: timer onwall
 */
struct tmr_dbg_info {
    UINT tmsp;
    TX_TMR_DGB_ID id;
    UINT fn_name;
    UINT onwall;
};
#ifndef TX_TIMER_DBG_BUF_SIZE
#define TX_TIMER_DBG_BUF_SIZE   256
#endif
struct tmr_dbg_info _tx_tmr_dbg[TX_TIMER_DBG_BUF_SIZE];
static UINT _tx_tmr_dbg_cnt;

/**
 * _tx_timer_dbg
 * Timer debug api, it will store timer actions and info into debug
 * buffer, include timer operations/timer therad/timer list and timer
 * event param.
 */
VOID _tx_timer_dbg(UINT id, TX_TIMER_INTERNAL *timer_ptr)
{
    TX_THREAD *thread_ptr;
    struct tmr_dbg_info *tmr_dbg_ptr;

    tmr_dbg_ptr = &_tx_tmr_dbg[_tx_tmr_dbg_cnt];
    tmr_dbg_ptr->id = id;
    tmr_dbg_ptr->tmsp = (UINT)aic_get_tick();

    if (timer_ptr) {
        thread_ptr = container_of(timer_ptr, TX_THREAD, tx_thread_timer);
        tmr_dbg_ptr->onwall = timer_ptr->tx_timer_internal_ticks_onwall;
        if (thread_ptr->tx_thread_id == TX_THREAD_ID)
            tmr_dbg_ptr->fn_name = (UINT)thread_ptr->tx_thread_name;
        else
            tmr_dbg_ptr->fn_name = (UINT)timer_ptr->tx_timer_internal_timeout_function;
    } else {
        tmr_dbg_ptr->onwall = 0;
        tmr_dbg_ptr->fn_name = 0;
    }

    if (++_tx_tmr_dbg_cnt == TX_TIMER_DBG_BUF_SIZE)
        _tx_tmr_dbg_cnt = 0;
}
#endif

/**
 * _tx_timer_get_remaining
 * Get the latest timer event remaining count, this count will
 * be set into aon timer0.
 */
ULONG _tx_timer_get_remaining(TX_TIMER_INTERNAL *timer_ptr)
{
    ULONG64 current = _tx_time_get();

    if (timer_ptr->tx_timer_internal_ticks_onwall > current)
        return (ULONG)(timer_ptr->tx_timer_internal_ticks_onwall - current);
    else
        return 0;
}

/**
 * _tx_timer_set_expire
 * Set the next expire timer event count.
 * The whole of this function follow is in irq disable env.
 */
VOID _tx_timer_set_expire(TX_TIMER_INTERNAL *timer_ptr)
{
    ULONG64 expire_val, tick_val, delta_val, ms;
    u32 cnt;

    TX_TIMER_DBG(TX_TMR_DBG_SET_EXP, timer_ptr);

    expire_val = timer_ptr->tx_timer_internal_ticks_onwall;
    tick_val = _tx_time_get();

    if (expire_val <= tick_val) {
        delta_val = 1;
    } else {
        delta_val = expire_val - tick_val;
    }

    ms = delta_val * 1000 / TX_TIMER_TICKS_PER_SECOND;

    cnt = (u32)(ms * aic_timer[SCHEDULE_TIMER].freq / 1000);

    aic_tmr_phy_set_cnt(SCHEDULE_TIMER, cnt);
}

VOID _tx_timer_clear_expire(void)
{
    TX_TIMER_DBG(TX_TMR_DBG_CLR_EXP, NULL);

    aic_tmr_phy_deinit(SCHEDULE_TIMER);
}

ULONG64 _tx_timer_get_tick(VOID)
{
    return (ULONG64)aic_get_tick();
}
#endif /* CONFIG_TIMER_TX_NO_TICK_IRQ */
