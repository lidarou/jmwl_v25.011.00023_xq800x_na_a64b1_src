/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of pwm HAL module
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0142aa
#define LOG_MODULE    AMOD_DRV

#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <pwm.h>
#include "pwm_phy.h"
#include <aic_errno.h>

aos_mutex_ptr aic_pwm_mutex = AOS_NULL;

int pwm_config(char *pwm_name, u32 duty_ns, u32 period_ns)
{
    int ret;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_pwm_mutex, AOS_WAIT_FOREVER);
        if(ret) {
            aicx_error(0x05657b, "", "pwm config get lock fail!\n");
            return ret;
        }
    }

    ret = aic_pwm_config(pwm_name, duty_ns, period_ns);
    if (ret)
        aicx_error(0x136700, "", "pwm config fail!\n");

    if (aos_is_user_thread_context())
        aos_put_mutex(aic_pwm_mutex);

    return ret;
}

int pwm_set(int chan_idx, u32 duty_perc_ns, u32 period_ns)
{
    char *pwm_name;

    pwm_name = get_pwm_name(chan_idx);
    if (!pwm_name) {
        aicx_warn(0x0bcbdf, "s", "%s get pwm name failed!\n", __func__);
        return AIC_ENOENT;
    }

    return pwm_config(pwm_name, duty_perc_ns, period_ns);
}

int pwm_enabled(int chan_idx, bool eb)
{
    int ret;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_pwm_mutex, AOS_WAIT_FOREVER);
        if(ret) {
            aicx_error(0x05657b, "", "pwm config get lock fail!\n");
            return ret;
        }
    }

    ret = aic_pwm_enabled(chan_idx, eb);

    if (aos_is_user_thread_context())
        aos_put_mutex(aic_pwm_mutex);

    return ret;
}

int pwm_init(void)
{
    aic_pwm_mutex = aos_create_mutex("pwm_hal");
    AIC_ASSERT(aic_pwm_mutex);
    return 0;
}
AIC_NORMAL_DRV_P1_INIT_EXPORT(pwm_init);
