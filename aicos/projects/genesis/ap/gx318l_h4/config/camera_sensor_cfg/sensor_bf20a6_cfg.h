#ifndef __SENSOR_BF20A6_CFG_H__
#define __SENSOR_BF20A6_CFG_H__

#include <i2c.h>
#include <camera_cfg/camera_cfg.h>

#define BF20A6_I2C_ADDR               0x6e

struct i2c_client bf20a6_client = {
    .addr = BF20A6_I2C_ADDR,
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
#define CAMERA_SENSOR_IO_USER_NAME    "cam_bf20a6_1v8"
#define CAMERA_SENSOR_IO_UNIT_NAME    "VDDMP1V88"
#define CAMERA_SENSOR_AVDD_USER_NAME  "cam_bf20a6_2v8"
#define CAMERA_SENSOR_AVDD_UNIT_NAME  "DVDD3V"
#endif

struct aic_camera_cfg camera_sensor_bf20a6_cfg = {
    .irq = IRQ_CSI,
    .gpio_pwdn = 2,
    .gpio_rst = 2,
    .cs_sensor_ops = AIC_NULL,
};

#endif
