/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of UPS module.
 *   It includes all the UPS common function api.
 */
#ifndef __UPS_H__
#define __UPS_H__

/*
 * ups_enter - enter ups mode.
 * Before enter ups mode, it needs to set rc32K calibration
 * configuration to make sure that the ups rc32k is precisely.
 */
void ups_enter(void);

/*
 * gx_ups_cnt_clear - clear ups cnt.
 * It will recount from 0.
 */
void gx_ups_cnt_clear(void);

/*
 * gx_ups_wakeup_time - set ups wake up time.
 * It will clean ups count, and set wake up time,
 * the time unit is second.
 */
void gx_ups_wakeup_time(u32 sec);

/*
 * gx_ups_start_count - start ups count.
 */
void gx_ups_start_count(void);

/*
 * gx_ups_cfg_period_mode - Configue ups count as period mode.
 */
void gx_ups_cfg_period_mode(void);

void gx_ups_set_comp(u32 cnt);
u32 gx_ups_get_cnt(void);
u32 gx_ups_int_status(void);
void gx_ups_set_sw_reg(u32 val);
u32 gx_ups_get_sw_reg(void);

#endif

