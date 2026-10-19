/*****************************************************************************/
/*                                                                           */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __TIMER_H__
#define __TIMER_H__

#include <aic_type.h>

/*
 * aic_udelay - use for us delay scenarios
 * Note that!!!
 * This usec should be multiple of 30.518us.
 */
void aic_udelay(u32 usec);
void aic_mdelay(u32 msec);
void aic_delay(u32 sec);
u32 aic_get_tick(void);
u32 aic_get_timestamp(void);
void aic_tmr_init(void);

#endif /* __TIMER_H__ */

