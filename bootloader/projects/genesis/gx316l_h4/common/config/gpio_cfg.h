/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __GPIO_BOARD_CFG_H__
#define __GPIO_BOARD_CFG_H__

#include <reg_base.h>
#include <aic_gpio_pin.h>
#include <aon_glb.h>
#include <top_glb.h>

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

static struct aic_gpio_board_cfg gpio_bd_cfg[GPIO_IP_MAX] = {
    {
        .index = GPIO_AON,
        .base = REG_BASE_AON_GPIO,
        .glb_base = REG_AON_GLB_GPIO_EB,
        .glb_bit = BIT_AON_GLB_GPIO_EB,
        .bp_oe_en = REG_AON_GLB_N_GPIO_BYPASS_OE_AON,
        .bp_oe_out = REG_AON_GLB_GPIO_BYPASS_OUT_AON,
        .bp_ie_en = REG_AON_GLB_N_GPIO_BYPASS_IE_AON,
        .bp_ie_in = REG_AON_GLB_GPIO_BYPASS_IN_AON,
        .pin_cnt = 16,
    },
    {
        .index = GPIO_TOP,
        .base = REG_BASE_TOP_GPIO,
        .glb_base = REG_TOP_GLB_GPIO_EB,
        .glb_bit = BIT_TOP_GLB_GPIO_EB,
        .bp_oe_en = REG_TOP_GLB_N_GPIO_BYPASS_OE_TOP,
        .bp_oe_out = REG_TOP_GLB_GPIO_BYPASS_OUT_TOP,
        .bp_ie_en = REG_TOP_GLB_N_GPIO_BYPASS_IE_TOP,
        .bp_ie_in = REG_TOP_GLB_GPIO_BYPASS_IN_TOP,
        .pin_cnt = 32,
    },
};

#endif /* __GPIO_BOARD_CFG_H__ */

