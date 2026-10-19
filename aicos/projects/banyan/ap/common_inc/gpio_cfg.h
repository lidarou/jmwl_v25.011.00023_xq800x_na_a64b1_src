/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __GPIO_BOARD_CFG_H__
#define __GPIO_BOARD_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <aic_gpio_pin.h>
#include <irq.h>

#define GPIO_PULSE_DEF_INDEX        GPIO_17

struct aic_gpio_board_cfg {
    u32 index;
    u32 base;
    u32 glb_base;
    u32 glb_bit;
    u32 bp_oe_en;
    u32 bp_oe_out;
    u32 bp_ie_en;
    u32 bp_ie_in;
    u32 irq_no;
    u32 pin_cnt;
};

static struct aic_gpio_board_cfg gpio_bd_cfg[] = {
    {
        .index = 0,
        .base = REG_BASE_TOP_GPIO,
        .glb_base = REG_CKM_EN_GLB_TOP_TOP_GPIO_EB,
        .glb_bit = BIT_CKM_EN_GLB_TOP_TOP_GPIO_EB,
        .irq_no = IRQ_TOP_GPIO,
        .pin_cnt = 32,
    },
};

#endif /* __GPIO_BOARD_CFG_H__ */

