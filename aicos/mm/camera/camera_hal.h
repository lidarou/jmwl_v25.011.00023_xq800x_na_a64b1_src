/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __CAMERA_HAL_H__
#define __CAMERA_HAL_H__

#include <aic_type.h>
#include <aic_gpio_pin.h>
#include <irq.h>
#include <camera_fb.h>

#include <camera_cfg/camera_cfg.h>

#include "camera_sensor.h"
#include "format_convert.h"

typedef enum {
    CAM_STREAM_ON = 0,
    CAM_STREAM_OFF,
    CAM_CAPTURE,
    CAM_SET_OUTPUT_SIZE,
    CAM_SET_DATA_FORMAT,
} camera_cmd_e;

typedef enum {
    CAM_STREAM_PREVIEW_INFO = 0,
    CAM_STREAM_CAPTURE_INFO,
#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
    CAM_STREAM_VIDEO_INFO,
#endif

} stream_info_id_e;

typedef enum {
    CAM_IN_CLOSE = 0,
    CAM_IN_OPEN,
    CAM_IN_PREVIEW,
    CAM_IN_CAPTURE,
#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
    CAM_IN_VIDEO,
#endif
} camera_state_e;

struct stream_info {
    u8 fb_num;
    enum ROT_MODE rot_mode;
    bool need_binning;
    u8 bytes_per_pixel;
    camera_data_fmt_e fmt;
    yuv_plane_num_e plane_num;
    /* sensor output size */
    int src_width;
    int src_height;
    /* unit is byte */
    int src_img_size;

    /* image size info */
    int width;
    int height;
    /* unit is byte */
    u32 img_size;
};

struct camera_info {
    camera_state_e cam_state;
    volatile u32 user_cap_addr;
    struct stream_info cam_stream_info;
    /* camera fb event */
    aos_event_ptr camera_fb_event_ptr;
};

extern struct camera_info g_camera_info;

#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
struct camera_video_queue_item {
    u32 raw_img_addr;
    u32 raw_img_size;
    u16 raw_img_width;
    u16 raw_img_height;
    camera_data_fmt_e data_fmt;
};

extern aos_queue_ptr camera_video_queue_ptr;
#endif

static inline boolean camera_is_opened(void)
{
    if (g_camera_info.cam_state != CAM_IN_CLOSE)
        return AIC_TRUE;
    else
        return AIC_FALSE;
}

int camera_open(camera_id_e id);
void camera_close(camera_id_e id);
int camera_preview(camera_id_e id, yuv_plane_num_e plane_num,
                   camera_data_fmt_e data_fmt,
                   int width, int height);
int camera_capture_prepare(camera_id_e id, yuv_plane_num_e plane_num,
                           camera_data_fmt_e data_fmt,
                           int width, int height);
int camera_capture(camera_id_e id, u32 user_addr);
#if defined(CONFIG_CAMERA_VIDEO_SUPPORT)
int camera_video_mode(camera_id_e id, int width, int height, camera_data_fmt_e fmt,
                      yuv_plane_num_e plane_num, u8 bytes_per_pixel);
int camera_video(camera_id_e id, int width, int height);
#endif
int camera_set_fps(camera_id_e id, u8 fps);
u8 camera_get_fps(camera_id_e id);
int camera_ioctl(camera_id_e id, camera_cmd_e cmd, void *input, void *output);
int camera_get_sensor_output_format(camera_id_e id, camera_data_fmt_e *fmt);

typedef void (*preview_refresh_cb)(u32 frame_addr, int width, int height, camera_data_fmt_e format);
int camera_register(preview_refresh_cb refresh);

struct stream_info *camera_get_stream_info(stream_info_id_e id);

#endif

