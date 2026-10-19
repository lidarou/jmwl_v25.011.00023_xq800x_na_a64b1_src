/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of pwm phy.
 *   It includes all the pwm phy function api.
*/
#ifndef __PWM_PHY_H__
#define __PWM_PHY_H__

int aic_pwm_config(char *pwm_name, u32 duty_ns, u32 period_ns);
char *get_pwm_name(int index);
int aic_pwm_enabled(int chan_idx, bool eb);
int pwm_num_get(void);

#endif /*__PWM_PHY_H__*/
