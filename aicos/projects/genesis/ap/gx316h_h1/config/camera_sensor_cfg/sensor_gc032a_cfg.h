#ifndef __SENSOR_GC032A_CFG_H__
#define __SENSOR_GC032A_CFG_H__

#include <i2c.h>

#include <camera_cfg/camera_cfg.h>

#define GC032A_I2C_ADDR               0x21

struct i2c_client gc032a_client = {
    .addr = GC032A_I2C_ADDR,
    .index = 1,
};

#define CAMERA_SENSOR_OUTPUT_640x480

/* line number define */
//#define CAMERA_OUTPUT_1LINE
#define CAMERA_OUTPUT_2LINE
//#define CAMERA_OUTPUT_4LINE

/* output data format define */
//#define CAMERA_OUTPUT_MTK
#define CAMERA_OUTPUT_Z

/* data sample mode define */
#define CAMERA_DDR_MODE
//#define CAMERA_SDR_MODE

#if defined(CONFIG_PMIC)
/* camera sensor voltage supply */
#define CAMERA_SENSOR_IO_USER_NAME    "cam_gc032a_1v8"
#define CAMERA_SENSOR_IO_UNIT_NAME    "VDDCAMIO1V8"
#define CAMERA_SENSOR_AVDD_USER_NAME  "cam_gc032a_2v8"
#define CAMERA_SENSOR_AVDD_UNIT_NAME  "AVDDCAM2V8"
#endif

struct aic_camera_cfg camera_sensor_gc032a_cfg = {
    .irq = IRQ_CSI,
    .gpio_pwdn = 2,
    .gpio_rst = 1,
    .cs_sensor_ops = AIC_NULL,
};

#endif
