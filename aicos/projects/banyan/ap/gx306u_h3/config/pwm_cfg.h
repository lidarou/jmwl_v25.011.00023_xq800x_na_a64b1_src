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
#define PWM_NUM 2
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
        .eb_reg = REG_CKM_EN_GLB_TOP_TOP_PWM0_EB,
        .eb_bit = BIT_CKM_EN_GLB_TOP_TOP_PWM0_EB,
    },
    {
        .name = "pwm1",
        .index = 1,
        .base = REG_BASE_TOP_PWM1,
        .eb_reg = REG_CKM_EN_GLB_TOP_TOP_PWM1_EB,
        .eb_bit = BIT_CKM_EN_GLB_TOP_TOP_PWM1_EB,
    },
};

#endif /* __PWM_CFG_H__ */
