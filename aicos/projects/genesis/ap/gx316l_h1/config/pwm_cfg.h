/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PWM_CFG_H__
#define __PWM_CFG_H__

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_reg_base.h>
#include <irq_chip.h>

#define PWM_CLK 26000000
#define PWM_NUM 8
struct aic_pwm_cfg {
    char *name;
    u32 index;
    u32 base;
    u32 eb_reg;
    u32 eb_bit;
};

static struct aic_pwm_cfg aic_pwm[PWM_NUM] = {
    {
        .name = "pwm0",
        .index = 0,
        .base = REG_BASE_TOP_PWM0,
        .eb_reg = REG_TOP_GLB_PWM0_EB,
        .eb_bit = BIT_TOP_GLB_PWM0_EB,
    },
    {
        .name = "pwm1",
        .index = 1,
        .base = REG_BASE_TOP_PWM1,
        .eb_reg = REG_TOP_GLB_PWM1_EB,
        .eb_bit = BIT_TOP_GLB_PWM1_EB,
    },
    {
        .name = "pwm2",
        .index = 2,
        .base = REG_BASE_TOP_PWM2,
        .eb_reg = REG_TOP_GLB_PWM2_EB,
        .eb_bit = BIT_TOP_GLB_PWM2_EB,
    },
    {
        .name = "pwm3",
        .index = 3,
        .base = REG_BASE_TOP_PWM3,
        .eb_reg = REG_TOP_GLB_PWM3_EB,
        .eb_bit = BIT_TOP_GLB_PWM3_EB,
    },
    {
        .name = "pwm4",
        .index = 4,
        .base = REG_BASE_TOP_PWM4,
        .eb_reg = REG_TOP_GLB_PWM4_EB,
        .eb_bit = BIT_TOP_GLB_PWM4_EB,
    },
    {
        .name = "pwm5",
        .index = 5,
        .base = REG_BASE_TOP_PWM5,
        .eb_reg = REG_TOP_GLB_PWM5_EB,
        .eb_bit = BIT_TOP_GLB_PWM5_EB,
    },
    {
        .name = "pwm6",
        .index = 6,
        .base = REG_BASE_TOP_PWM6,
        .eb_reg = REG_TOP_GLB_PWM6_EB,
        .eb_bit = BIT_TOP_GLB_PWM6_EB,
    },
    {
        .name = "pwm7",
        .index = 7,
        .base = REG_BASE_TOP_PWM7,
        .eb_reg = REG_TOP_GLB_PWM7_EB,
        .eb_bit = BIT_TOP_GLB_PWM7_EB,
    },
};

#endif /* __PWM_CFG_H__ */
