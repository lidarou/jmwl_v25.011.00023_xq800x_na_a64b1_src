/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __GPIO_PMIC_BOARD_CFG_H__
#define __GPIO_PMIC_BOARD_CFG_H__

#include <pmic_auto_reg.h>
#include <aic_gpio_pin.h>
#include <irq.h>
#include <pmic_interrupt.h>

struct aic_gpio_pmic_board_cfg {
    u32 index;
    u32 base;
    u32 glb_base;
    u32 glb_bit;
    u32 irq_no;
    u32 pin_cnt;
};

static struct aic_gpio_pmic_board_cfg gpio_pmic_bd_cfg[GPIO_PMIC_MAX] = {
    {
        .index = GPIO_PMIC_AON,
        .base = PMIC_BASE_GPIO_RTC,
        .glb_base = REG_GLB_UPS_CKM_EN_GLB_CLK_UPS_GPIO_RF,
        .glb_bit = BIT_GLB_UPS_CKM_EN_GLB_CLK_UPS_GPIO_RF,
        .irq_no = PMIC_IRQ_GP202_GPIO_UPS,
        .pin_cnt = 4,
    },
    {
        .index = GPIO_PMIC_PD,
        .base = PMIC_BASE_GPIO_PD,
        .glb_base = REG_GLB_PD_CKM_EN_GLB_CLK_PD_GPIO_RF,
        .glb_bit = BIT_GLB_PD_CKM_EN_GLB_CLK_PD_GPIO_RF,
        .irq_no = PMIC_IRQ_GPIO_PD,
        .pin_cnt = 1,
    }
};

#endif /* __GPIO_PMIC_BOARD_CFG_H__ */

