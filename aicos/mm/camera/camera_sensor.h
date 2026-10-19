/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __CAMERA_SENSOR_H__
#define __CAMERA_SENSOR_H__

#include <aic_type.h>
#include <aic_osa.h>
#include <camera_cfg/camera_cfg.h>

#define CAMERA_SENSOR_OUTPUT_WIDTH    240
#define CAMERA_SENSOR_OUTPUT_HEIGHT   320

typedef enum {
    CAMERA_SENSOR_OUTPUT_MODE_Z = 0,
    CAMERA_SENSOR_OUTPUT_MODE_M,
} camera_sensor_output_mode_e;

typedef enum {
    CAMERA_SENSOR_SAMPLE_SDR = 0,
    CAMERA_SENSOR_SAMPLE_DDR,
} camera_sensor_sample_mode_e;

typedef enum {
    CAMERA_SENSOR_DATA_1_LINE = 1,
    CAMERA_SENSOR_DATA_2_LINE,
    CAMERA_SENSOR_DATA_4_LINE,
} camera_sensor_data_line_num_e;

typedef enum {
    CAMERA_FRONT = 0,
    CAMERA_BACK,
} camera_id_e;

typedef enum {
    YUV422 = 0,
    YUV420,
    RAW8,
    RGB565,
    Y,
} camera_data_fmt_e;

typedef enum {
    YUV_ONE_PLANE = 0,
    YUV_TWO_PLANE,
} yuv_plane_num_e;

struct camera_spi_sensor_ops {
    int (*initial_setting)(camera_data_fmt_e data_fmt, u32 width, u32 height);
    int (*cs_open)(void);
    void (*cs_close)(void);
    int (*cs_start_xfer)(void);
    void (*cs_stop_xfer)(void);
    int (*cs_set_clock)(u32 rate);
    int (*cs_set_output_size)(u32 width, u32 height);
    int (*cs_set_output_format)(u8 fmt);
    camera_data_fmt_e (*cs_get_output_format)(void);
};

struct camera_spi_sensor_ops *camera_get_spi_sensor_ops(camera_id_e id);
void camera_sensor_register(camera_id_e id, struct aic_camera_cfg *cs_cfg);

#endif

