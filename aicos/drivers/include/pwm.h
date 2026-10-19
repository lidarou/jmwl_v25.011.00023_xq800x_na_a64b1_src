/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of pwm module.
 *   It includes all the pwm api.
*/
#ifndef __PWM_H__
#define __PWM_H__
int pwm_config(char *pwm_name, u32 duty_ns, u32 period_ns);
int pwm_set(int chan_idx, u32 duty_perc_ns, u32 period_ns);
int pwm_enabled(int chan_idx, bool eb);
int pwm_is_enabled(int chan_idx);

#endif /* __PWM_H__ */
