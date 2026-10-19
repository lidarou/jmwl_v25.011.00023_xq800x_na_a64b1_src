/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  pwm backlight control interface.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f746b
#define LOG_MODULE    AMOD_GPAPH

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <display.h>
#include <pmu_user.h>
#include <pwm.h>

#define BL_PWM_CHAN 0X1
#define BL_PWM_MAX_CYCLE 1000//ns
static u32 cur_bl_perc;

int aic_bl_brightness_set(u32 brt_perc)
{
    u32 duty;
    int ret;

    if (brt_perc == 0) {
        ret = pwm_enabled(0, FALSE);
        broadcast_backlight_switch(AIC_BL_TURN_OFF);
        goto out;
    }

    duty = brt_perc * BL_PWM_MAX_CYCLE / 100;

    ret = pwm_set(BL_PWM_CHAN, duty, BL_PWM_MAX_CYCLE);
    if (ret)
        goto out;

    cur_bl_perc = brt_perc;

    broadcast_backlight_switch(AIC_BL_TURN_ON);

out:
    return ret;
}

int aic_bl_brightness_get(u32* brt_perc)
{
    *brt_perc = cur_bl_perc;

    return 0;
}
