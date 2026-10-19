/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __CAMERA_BOARD_CFG_H__
#define __CAMERA_BOARD_CFG_H__

#include <aic_type.h>
#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <aic_gpio_pin.h>
#include <irq_chip.h>
#if defined(CONFIG_PMIC)
#include <pmu_user.h>
#endif

//#define CAMERA_SENSOR_OUTPUT_640x480

/* sensor output data format */
//#define CAMERA_SENSOR_OUTPUT_Y

#define CLK_CSI_24M_EN        CLK_CAM_MCLK_EB

struct aic_camera_cfg {
    IRQn_Type irq;
    GPIO_PIN_ID gpio_pwdn;
    GPIO_PIN_ID gpio_rst;
    void *cs_sensor_ops;
#if defined(CONFIG_PMIC)
    struct pmu_user *vddcamio1v8;
    struct pmu_user *vddcama2v8;
#endif
};

#endif /* __CAMERA_BOARD_CFG_H__ */

