/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip clock arrays file.
 */

#ifndef __AIC_PMIC_CLK_H__
#define __AIC_PMIC_CLK_H__

#include <aic_iot_clk.h>
#include "aic_pmic_clk_map.h"

static struct clk_gate pmic_clk_gate_array[] = {
    /* glb rtc clocks */
    {
        .reg = 0x690,
        .clk_id = GLB_CLK_RTC_XCO26M_32KLESS,
        .index = 0,
    },
    {
        .reg = 0x694,
        .clk_id = GLB_CLK_RTC_CAL_26M,
        .index = 0,
    },
    {
        .reg = 0x698,
        .clk_id = GLB_CLK_RTC_CAL_32K,
        .index = 0,
    },
    {
        .reg = 0x69c,
        .clk_id = GLB_CLK_RTC_RTC,
        .index = 0,
    },
    {
        .reg = 0x6a0,
        .clk_id = GLB_CLK_RTC_PMU,
        .index = 0,
    },
    {
        .reg = 0x6a4,
        .clk_id = GLB_CLK_UPS_GPIO,
        .index = 0,
    },
    {
        .reg = 0x6a8,
        .clk_id = GLB_CLK_UPS_PMU,
        .index = 0,
    },
    {
        .reg = 0x6ac,
        .clk_id = GLB_CLK_UPS_CNT,
        .index = 0,
    },
    /* glb pd clocks */
    {
        .reg = 0x8004,
        .clk_id = GLB_CLK_APB_MAIN,
        .index = 0,
    },
    {
        .reg = 0x8008,
        .clk_id = GLB_CLK_APB_PD,
        .index = 0,
    },
    {
        .reg = 0x800c,
        .clk_id = GLB_CLK_APB_RTC,
        .index = 0,
    },
    {
        .reg = 0x8010,
        .clk_id = GLB_CLK_APB_IIC,
        .index = 0,
    },
    {
        .reg = 0x8014,
        .clk_id = GLB_CLK_APB_EFUSE,
        .index = 0,
    },
    {
        .reg = 0x8018,
        .clk_id = GLB_CLK_APB_ADC,
        .index = 0,
    },
    {
        .reg = 0x801c,
        .clk_id = GLB_CLK_PD_PMU_ANA,
        .index = 0,
    },
    {
        .reg = 0x8020,
        .clk_id = GLB_CLK_PD_ADC,
        .index = 0,
    },
    {
        .reg = 0x8024,
        .clk_id = GLB_CLK_PD_EFUSE,
        .index = 0,
    },
    {
        .reg = 0x8028,
        .clk_id = GLB_CLK_PD_PWM,
        .index = 0,
    },
    {
        .reg = 0x802c,
        .clk_id = GLB_CLK_PD_GPIO,
        .index = 0,
    },
    {
        .reg = 0x8030,
        .clk_id = GLB_CLK_PD_WDT,
        .index = 0,
    },
    {
        .reg = 0x8034,
        .clk_id = GLB_CLK_PD_PMU_ANA_406K,
        .index = 0,
    },
};

#endif /* __AIC_PMIC_CLK_H__ */

