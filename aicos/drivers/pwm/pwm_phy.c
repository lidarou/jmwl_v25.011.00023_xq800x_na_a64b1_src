/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of pwm module.
 *   It includes all the pwm common driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x141234
#define LOG_MODULE    AMOD_DRV

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_osa.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pwm_cfg.h>
#include <pwm.h>
#include "pwm_reg.h"
#include "pwm_phy.h"

#define PWM_MOD_MAX 0xFFFF
#define PWM_MOD_0XFF 0xFF

static struct aic_pwm_cfg *get_pwm_cfg(char *name)
{
    int i;

    for ( i = 0; i < PWM_NUM; i++) {
        if (!strcmp(aic_pwm[i].name, name))
            return &aic_pwm[i];
    }

    return NULL;
}
char *get_pwm_name(int index)
{
    return aic_pwm[index].name;
}

static void aic_pwm_phy_config(u32 base, int level, int prescale)
{
    AIC_WRITE32(base + REG_PWM_PWM_PREDIV, prescale);
    AIC_WRITE32(base + REG_PWM_PWM_MOD, PWM_MOD_0XFF);
    AIC_WRITE32(base + REG_PWM_PWM_DUTY, level);
    AIC_WRITE32(base + REG_PWM_PWM_PATTERN, BIT_PWM_PWM_PATTERN(0xFFFFFFFF));
    AIC_WRITE32(base + REG_PWM_PWM_GLB_CTRL, BIT_PWM_PWM_EN);
}

static void aic_pwm_config_update(u32 base)
{
    AIC_WRITE32(base + REG_PWM_PWM_UPDATE, BIT_PWM_PWM_UPDATE);
}

static inline u64 div64_u64(u64 dividend, u64 divisor)
{
    return dividend / divisor;
}

static int __aic_pwm_enable(int chan_idx, bool eb)
{
    AIC_WRITE32(aic_pwm[chan_idx].eb_reg, BIT_PWM_PWM_EN);

    if (eb) {
        AIC_WRITE32(aic_pwm[chan_idx].base + REG_PWM_PWM_GLB_CTRL, BIT_PWM_PWM_EN);
    } else {
        AIC_WRITE32(aic_pwm[chan_idx].base + REG_PWM_PWM_GLB_CTRL, ~BIT_PWM_PWM_EN);
        AIC_WRITE32(aic_pwm[chan_idx].eb_reg, 0);
    }

    return 0;
}

int aic_pwm_enabled(int chan_idx, bool eb)
{
    return __aic_pwm_enable(chan_idx, eb);
}

int pwm_num_get(void)
{
    return PWM_NUM;
}

int aic_pwm_config(char *pwm_name, u32 duty_ns, u32 period_ns)
{
    int level, aic_pwm_scale;
    u64 clk_rate, div, val;
    struct aic_pwm_cfg * pwm_cfg;

    pwm_cfg= get_pwm_cfg(pwm_name);
    if (!pwm_cfg)
        return AIC_EXDEV;

    level = duty_ns * (PWM_MOD_0XFF + 1) / period_ns;

    if (level > 0)
        __aic_pwm_enable(pwm_cfg->index, true);
    else
        goto out;

    clk_rate = PWM_CLK;
    div = 1000000000;
    div = div * (PWM_MOD_0XFF + 1);
    val = clk_rate * period_ns;
    aic_pwm_scale = div64_u64(val, div) - 1;
    if (aic_pwm_scale < 0)
        aic_pwm_scale = 0;

    aic_pwm_phy_config(pwm_cfg->base, level - 1, aic_pwm_scale);
    aic_pwm_config_update(pwm_cfg->base);

out:
    if(level == 0)
        __aic_pwm_enable(pwm_cfg->index, false);

    return 0;
}
