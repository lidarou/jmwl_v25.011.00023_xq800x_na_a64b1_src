/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d84a8
#define LOG_MODULE    AMOD_CAMERA

#include <camera_cfg/camera_cfg.h>

#include "camera_sensor.h"

#define CAMERA_SENSOR_ARRAY_MAX    2
struct aic_camera_cfg camera_cfg[CAMERA_SENSOR_ARRAY_MAX];

struct camera_spi_sensor_ops *camera_get_spi_sensor_ops(camera_id_e id)
{
    return camera_cfg[id].cs_sensor_ops;
}

void camera_sensor_register(camera_id_e id, struct aic_camera_cfg *cs_cfg)
{
    aos_memcpy(&camera_cfg[id], cs_cfg, sizeof(struct aic_camera_cfg));
}

