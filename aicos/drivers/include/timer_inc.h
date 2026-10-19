/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines timer driver api.
 *    The timer api will be called by thx os functions.
 */

#ifndef __TIMER_INC_H__
#define __TIMER_INC_H__

#include <aic_type.h>
#include <stdlib.h>

/*
 * aic_get_ms
 * Get the system tick count, it is system boot time or
 * onwall time, and it is 1ms per tick.
 * Notice!!!
 * The return value is 64 bit.
 */
u64 aic_get_ms(void);
/*
 * aic_get_us
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 64 bit, and its unit is us.
 */
u64 aic_get_us(void);
/*
 * aic_get_ns
 * Get the ns count
 * Notice!!!
 * The return value is 64 bit.
 */
u64 aic_get_ns(void);
/*
 * aic_ndelay - use for ns delay scenarios
 * Note that!!!
 * This nsec should be multiple of 38.46ns.
 */
void aic_ndelay(u32 nsec);
/*
 * aic_get_tmr_raw_step
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 64 bit, cycle counter is 30.5176us per step.
 */
u64 aic_get_tmr_raw_step(void);
/*
 * aic_get_tick
 * Get the system tick count, it is system boot time or
 * onwall time, and it is 1ms per tick.
 * Notice!!!
 * The return value is 64 bit.
 */
u64 aic_get_tick(void);
/*
 * aic_get_timestamp
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 32 bit, and its unit is us.
 */
u32 aic_get_timestamp(void);
/*
 * aic_get_system_cycle
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 64 bit.
 */
u64 aic_get_system_cycle(void);
/*
 * aic_get_boot_time
 * Get the free run count, it is 32KHz clock, it will start
 * count at the boot time.
 * Notice!!!
 * The return value is 64 bit, unit is us.
 */
u64 aic_get_boot_time(void);
/*
 * aic_udelay - use for seconds delay scenarios
 */
void aic_delay(u32 sec);
/*
 * aic_udelay - use for ms delay scenarios
 */
void aic_mdelay(u32 msec);
/*
 * aic_udelay - use for us delay scenarios
 * Note that!!!
 * This usec should be multiple of 30.518us.
 */
void aic_udelay(u32 usec);
/*
 * aic_tmr_slp_cali_tmr - calculate timer sleep time
 * for glitch free clock.
 */
u64 aic_tmr_slp_cali_tmr(u64 slp_cnt);

void aic_tmr_samp_cfg(u32 us);

void aic_tmr_ealy_init(void);

#endif /* __TIMER_INC_H__ */

