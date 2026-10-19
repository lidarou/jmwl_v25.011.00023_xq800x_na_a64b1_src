/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines pmic clock map.
 */

#ifndef __AIC_PMIC_CLK_MAP_H__
#define __AIC_PMIC_CLK_MAP_H__

/* glb pd clocks */
#define GLB_CLK_APB_MAIN               0
#define GLB_CLK_APB_PD                 1
#define GLB_CLK_APB_RTC                2
#define GLB_CLK_APB_IIC                3
#define GLB_CLK_APB_EFUSE              4
#define GLB_CLK_APB_ADC                5
#define GLB_CLK_PD_PMU_ANA             6
#define GLB_CLK_PD_ADC                 7
#define GLB_CLK_PD_EFUSE               8
#define GLB_CLK_PD_PWM                 9
#define GLB_CLK_PD_GPIO                10
#define GLB_CLK_PD_WDT                 11
#define GLB_CLK_PD_PMU_ANA_406K        12

/* glb rtc clocks */
#define GLB_CLK_RTC_XCO26M_32KLESS     0
#define GLB_CLK_RTC_CAL_26M            1
#define GLB_CLK_RTC_CAL_32K            2
#define GLB_CLK_RTC_RTC                3
#define GLB_CLK_RTC_PMU                4
#define GLB_CLK_UPS_GPIO               5
#define GLB_CLK_UPS_PMU                6
#define GLB_CLK_UPS_CNT                7

#endif /* __AIC_PMIC_CLK_MAP_H__ */
