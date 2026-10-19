/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of camera service.
 */

/*********************
 *      INCLUDES
 *********************/
#include "camera_service.h"
#include "aic_osa.h"
#include "camera_hal.h"
#include "camera_jpg_generator.h"
#include "format_convert.h"
#include "aic_osa_ex.h"
#include "display_cfg.h"
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

/*********************
 *      DEFINES
 *********************/
#define CAM_IMG_WIDTH LCD_PANEL_WIDTH
#define CAM_IMG_HEIGHT LCD_PANEL_HEIGHT


#define CAM_IMG_SAVING_THX_PRIO 20
#define CAM_IMG_SAVING_THX_STACK_SIZE 3000

/**********************
 *      TYPEDEFS
 **********************/
struct camera_config {
    camera_id_e id;
    camera_data_fmt_e fmt;
    yuv_plane_num_e plane;
    cam_capture_mode_e mode;
    u32 width;
    u32 height;
    int time;
};

struct camera_image_info {
    char *path;
    u8 *data;
    u32 size;
    camera_notify_callback cb;
};

struct camera_service {
    struct camera_image_info info;
    cam_state_e state;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/* default config for camera */
static struct camera_config g_cam_cfg = {
    CAMERA_FRONT, YUV422, YUV_ONE_PLANE, CAM_MODE_SINGLE,
    CAM_IMG_WIDTH, CAM_IMG_HEIGHT, 0};

/* instance of camera service for service framework */
static struct camera_service g_cam_srv;

/* ID of image saving thread */
static aos_thread_id g_cam_thx_id = AOS_INVALID_THREAD_ID;
/* ID of time-lapse shooting timer */
static aos_timer_ptr g_cam_timelapse = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void camera_service_init(void *param)
{
    struct camera_service *srv = &g_cam_srv;
    struct camera_config *cfg = &g_cam_cfg;

    if (srv->state == CAM_OPEN) {
        app_logi("%s: camera already open\n", __func__);
        return;
    }

    if (camera_open(cfg->id)) {
        app_loge("%s: camera open failed\n", __func__);
        return;
    }

    srv->state = CAM_OPEN;
}

static void camera_service_destroy(bool gracefully)
{
    struct camera_service *srv = &g_cam_srv;
    struct camera_config *cfg = &g_cam_cfg;

    if (srv->state != CAM_CLOSE) {
        camera_close(cfg->id);
        srv->state = CAM_CLOSE;
    }

    srv->info.path = 0;
    srv->info.data = 0;
    srv->info.size = 0;
    srv->info.cb = 0;

    cfg->mode = CAM_MODE_SINGLE;
    cfg->time = 0;
}

static inline void camera_image_name_generate(char *path, char *name)
{
    DIR *dir = NULL;
    struct dirent *dir_ent = NULL;
    char path_no_letter[CAM_IMG_DIR_LEN];
    int index, new_index = 0;
    int path_len = strlen(path);

    memcpy(path_no_letter, path, path_len);
    path_no_letter[path_len - 1] = '\0';

    if ((dir = opendir(path_no_letter)) == 0) {
        app_loge("%s: open %s failed\n", __func__, path_no_letter);
        return;
    }

    do {
        if ((dir_ent = readdir(dir)) == 0)
            break;

        if (dir_ent->d_type == DT_REG) {
            /* regular file */
            char num[4];
            if (strncmp(dir_ent->d_name, "IMG_", 4) != 0)
                continue;
            strncpy(num, dir_ent->d_name + 4, 3);
            num[3] = '\0';

            index = atoi(num);
            if (index == new_index) {
                new_index = index + 1;
                if (new_index > 999)
                    new_index = 0;
            }
        }
    } while (1);

    closedir(dir);

    sprintf(name, "IMG_%03d.jpg", new_index);

    app_logi("%s: generated name:%s\n", __func__, name);
}

static void camera_img_saving(struct camera_image_info *info)
{
    struct camera_config *cfg = &g_cam_cfg;
    struct cam_image_attr attr = {0};
    char *path = info->path;
    char path_name[CAM_IMG_PATH_NAME_LEN];
    u8 *img_data = info->data;
    int image_num = 0;

    attr.width = cfg->width;
    attr.height = cfg->height;

    /* TODO: quality should be debugged */
    if (cfg->mode == CAM_MODE_CONTINUOUS)
        image_num = cfg->time;
    else
        image_num = 1;

    app_logi("%s: image saving ... %d, %d, %p, %s\n", __func__,
             image_num, info->size, img_data, path);

    strcpy(path_name, path);
    for (int i = 0; i < image_num; i++) {
        camera_image_name_generate(path, path_name + strlen(path));
        app_logi("%s: path: %s\n", __func__, path);
        if (camera_write_jpeg_yuv422(path_name, &attr, img_data))
            app_loge("%s: image save failed\n", __func__);
        img_data = img_data + info->size / image_num;
    }

    if (info->cb)
        info->cb(info->data, info->size);
}

static void camera_imgsaving_thread_entry(void *data)
{
    camera_img_saving((struct camera_image_info *)data);
}

static void camera_time_lapse_shot_func(void *data)
{
    struct camera_service *srv = &g_cam_srv;
    struct camera_config *cfg = &g_cam_cfg;
    u8 *img_data = data;

    if (srv->state != CAM_SHOOTING) {
        app_loge("%s: state %d error\n", __func__, srv->state);
        return;
    }

    if (cfg->mode != CAM_MODE_COUNTDOWN) {
        app_loge("%s: mode %d error\n", __func__, cfg->mode);
        return;
    }

    if (srv->info.cb)
        srv->info.cb(img_data, srv->info.size);

    aos_delete_timer(g_cam_timelapse);
}

/**********************
 *   PUBLIC FUNCTIONS
 **********************/

/**
 * app_srv_camera_start() - start the camera service.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_start(void)
{
    camera_service_init(0);
}

/**
 * app_srv_camera_stop() - stop the camera service.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_stop(void)
{
    camera_service_destroy(0);
}

/**
 * app_srv_camera_capture_preview() - take camera into preview mode.
 * @arg1: none.
 * @arg2: none.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_camera_capture_preview(void)
{
    struct camera_service *srv = &g_cam_srv;
    struct camera_config *cfg = &g_cam_cfg;

    app_logd("===> state:%d\n", srv->state);

    if (srv->state != CAM_OPEN) {
        app_logi("%s: camera service not init\n", __func__);
        return -1;
    }

    app_logd("===> id %d, plane:%d, fmt:%d, width:%d, height:%d\n",
             cfg->id, cfg->plane, cfg->fmt, cfg->width, cfg->height);

    if (camera_preview(cfg->id, cfg->plane, cfg->fmt, cfg->width, 
                       cfg->height)) {
        app_loge("camera preview failed\n");
        return -1;
    }

    srv->state = CAM_PREVIEW;

    return 0;
}

/**
 * app_srv_camera_capture_prepare() - prepare the capture according to
 *                                    the currently set mode.
 * @img_size: the image size will be return by shooting.
 * @arg2: none.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_camera_capture_prepare(u32 *img_size)
{
    struct stream_info *capture_info = NULL;
    struct camera_service *srv = &g_cam_srv;
    struct camera_config *cfg = &g_cam_cfg;

    if (srv->state != CAM_PREVIEW) {
        app_loge("%s: camera not open %d\n", __func__, srv->state);
        return -1;
    }

    if (!img_size) {
        app_loge("%s: invalid param\n", __func__);
        return -1;
    }

    if (camera_capture_prepare(cfg->id, cfg->plane, cfg->fmt,
                               cfg->width, cfg->height)) {
        app_loge("%s: camera prepare failed\n", __func__);
        return -1;
    }

    capture_info = camera_get_stream_info(cfg->id);
    if (!capture_info) {
        app_loge("%s: camera get info failed\n", __func__);
        return -1;
    }

    switch (cfg->mode) {
    case CAM_MODE_SINGLE:
        *img_size = capture_info->img_size;
        break;
    case CAM_MODE_CONTINUOUS:
        *img_size = capture_info->img_size * cfg->time;
        break;
    case CAM_MODE_COUNTDOWN:
        *img_size = capture_info->img_size;
        break;
    default:
        return -1;
    }

    srv->info.size = *img_size;
    srv->state = CAM_PREPARE;

    return 0;
}

/**
 * app_srv_camera_shooting_start() - start shooting
 * @data: data buffer for storing captured image data .
 * @size: size of data buffer input.
 * @cb: callback of time-lapse shooting to notify finishing capture.
 * Return: 0 means success, -1 means failure, 1 means time lapse.
 */
int app_srv_camera_shooting_start(u8 *data, u32 size, camera_notify_callback cb)
{
    struct camera_service *srv = &g_cam_srv;
    struct camera_config *cfg = &g_cam_cfg;
    u8 *img_data = data;

    if (srv->state != CAM_PREPARE) {
        app_loge("%s: state %d error\n", __func__, srv->state);
        return -1;
    }

    app_logi("%s: size:%d, info size:%d, mode:%d\n", __func__,
             size, srv->info.size, cfg->mode);

    if (!data || size < srv->info.size) {
        app_loge("%s: invalid param\n", __func__);
        return -1;
    }

    srv->state = CAM_SHOOTING;
    srv->info.data = img_data;
    srv->info.cb = cb;

    switch (cfg->mode) {
    case CAM_MODE_SINGLE:
        camera_capture(cfg->id, (u32)img_data);
        break;
    case CAM_MODE_CONTINUOUS:
        app_logi("%s:time:%d, size: %d\n", __func__, cfg->time, cfg->time);
        for (int i = 0; i < cfg->time; i++) {
            camera_capture(cfg->id, (u32)img_data);
            img_data = img_data + size / cfg->time;
        }
        break;
    case CAM_MODE_COUNTDOWN:
        if (cb) {
            /* start a auto-timer to caputre */
            g_cam_timelapse = aos_create_timer(
                "camera timelapse timer", camera_time_lapse_shot_func, data,
                cfg->time * 1000, AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);
            if (!g_cam_timelapse) {
                app_loge("%s: timer create failed\n", __func__);
                return -1;
            }
            return 1;
        } else {
            camera_capture(cfg->id, (u32)img_data);
        }
        break;
    default:
        return -1;
    }

    return 0;
}

/**
 * app_srv_camera_shooting_stop() - stop shooting, generally used
 *                                  for time-lapse shooting.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_shooting_stop(void)
{
    struct camera_service *srv = &g_cam_srv;
    struct camera_config *cfg = &g_cam_cfg;
    app_logi("%s: enter \n", __func__);

    if (srv->state ==
            CAM_SHOOTING && cfg->mode == CAM_MODE_COUNTDOWN) {
        /* del the time-lapse for low memory */
        aos_delete_timer(g_cam_timelapse);
    }

    if (srv->state == CAM_SHOOTING || srv->state == CAM_IMG_SAVING) {
        srv->info.path = 0;
        srv->info.data = 0;
        srv->info.size = 0;
        srv->info.cb = 0;
        srv->state = CAM_OPEN;
    }
    else {
        app_logi("%s: state: %d \n", __func__, srv->state);
    }
}

/**
 * app_srv_camera_image_save() - save the image of camera capture
 * @path: path to save the image .
 * @data: image data.
 * @size: data size.
 * @cb: callback of finishing the image saving.
 * @sync: 1 means saving asynchronous.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_camera_image_save(char *path, u8 *data, u32 size,
                              camera_notify_callback cb, u8 async)
{
    struct camera_service *srv = &g_cam_srv;

    if (!path || !data || !size || !cb) {
        app_loge("%s: invalid param\n", __func__);
        return -1;
    }

    srv->info.path = path;
    srv->info.data = data;
    srv->info.size = size;
    srv->info.cb = cb;
    srv->state = CAM_IMG_SAVING;

    if (async) {
        /* start a thread to saving the image to FS */
        g_cam_thx_id = aos_create_thread_simply(
            "cam imgsaving thread", camera_imgsaving_thread_entry,
            &srv->info, CAM_IMG_SAVING_THX_STACK_SIZE, CAM_IMG_SAVING_THX_PRIO);
    } else {
        camera_img_saving(&srv->info);
    }

    return 0;
}

/**
 * app_srv_camera_state_get() - get current state of camera.
 * @arg1: none.
 * @arg2: none.
 * Return: return the state of camera.
 */
cam_state_e app_srv_camera_state_get(void)
{
    return g_cam_srv.state;
}

/**
 * app_srv_camera_capture_mode_set() - set  a camera capture mode .
 * @mode: capture mode, see definition of cam_capture_mode_e.
 * @time: if countdown mode, time should be set; if continous,
 *        number of consecutive shots should be set.
 * Return: none.
 */
void app_srv_camera_capture_mode_set(cam_capture_mode_e mode, int time)
{
    g_cam_cfg.mode = mode;
    g_cam_cfg.time = time;
}

/**
 * app_srv_camera_capture_mode_get() - get the current camera capture mode.
 * @arg1: none.
 * @arg2: none.
 * Return: capture mode, see definition of cam_capture_mode_e.
 */
cam_capture_mode_e app_srv_camera_capture_mode_get(void)
{
    return g_cam_cfg.mode;
}

/**
 * app_srv_camera_frame_rate_set() - set the frame rate of camera.
 * @fps: frame rate to set for camera.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_frame_rate_set(u8 fps)
{
    camera_set_fps(g_cam_cfg.id, fps);
}

/**
 * app_srv_camera_frame_rate_get() - get the frame rate of camera.
 * @arg1: none.
 * @arg2: none.
 * Return: return the valid frame rate of current camera setting.
 */
u8 app_srv_camera_frame_rate_get(void)
{
    return camera_get_fps(g_cam_cfg.id);
}

/**
 * app_srv_camera_flashlight_switch() - main menu display.
 * @state: flash light state to switch, see definition of
 *         cam_flashlihgt_state_e.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_flashlight_switch(cam_flashlihgt_state_e state)
{
    /* hardware not support */
    return;
}
