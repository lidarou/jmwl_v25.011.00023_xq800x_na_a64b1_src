#ifndef __SENSOR_GC032A_DEFAULT_CFG_H__
#define __SENSOR_GC032A_DEFAULT_CFG_H__

#include <i2c.h>
#include <aic_gpio_pin.h>

#include "camera_cfg.h"

#define GC032A_I2C_ADDR               0x21

struct i2c_client gc032a_default_client = {
    .addr = GC032A_I2C_ADDR,
    .index = 1,
};

/* line number define */
//#define CAMERA_OUTPUT_1LINE
#define CAMERA_OUTPUT_2LINE
//#define CAMERA_OUTPUT_4LINE

/* output data format define */
#define CAMERA_OUTPUT_MTK
//#define CAMERA_OUTPUT_Z

/* data sample mode define */
#define CAMERA_DDR_MODE
//#define CAMERA_SDR_MODE

#if defined(CONFIG_PMIC)
/* camera sensor voltage supply */
#define CAMERA_SENSOR_IO_USER_NAME    "cam_gc032a_1v8"
#define CAMERA_SENSOR_IO_UNIT_NAME    "VDDMP1V88"
#define CAMERA_SENSOR_AVDD_USER_NAME  "cam_gc032a_2v8"
#define CAMERA_SENSOR_AVDD_UNIT_NAME  "VDDTCXO"
#endif

#ifdef CONFIG_PMIC_GPIO
//#define USING_PMIC_GPIO
#endif

struct aic_camera_cfg camera_sensor_gc032a_default_cfg = {
    .irq = IRQ_CSI,
#ifdef USING_PMIC_GPIO
    .gpio_pwdn = GPIO_PMIC_3,
    .gpio_rst = GPIO_PMIC_3,
#else
    .gpio_pwdn = 0,
    .gpio_rst = 1,
#endif
    .cs_sensor_ops = AIC_NULL,
};

#endif
