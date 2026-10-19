/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of camera flow layer.
 */

/*********************
 *      INCLUDES
 *********************/
#include "camera_flow.h"
#include "camera_service.h"
#include "camera_jpg_generator.h"
#include "timer_inc.h"
#include "aic_osa_ex.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include "comp_camera_preview.h"

/*********************
 *      DEFINES
 *********************/
#define CONFIG_CAPTURE_COUNTDOWN_IN_FLOW 0
#define CAM_CAPTURE_DELAY 3

/**********************
 *      TYPEDEFS
 **********************/
struct app_camera_res {
    lv_obj_t *preview_scr;
    lv_obj_t *img;
};

struct app_camera_info {
    struct app_camera_res res;
    cam_capture_mode_e mode;
    int time;
};


/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static struct app_camera_info g_cam_info;
static lv_timer_t * g_display_timer = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/

extern lv_obj_t *g_camera_preview_scr;
extern lv_obj_t *capture_btn;

void display_timer(lv_timer_t * t)
{
    if (g_display_timer) {
        comp_camera_preview_resume();
        app_srv_camera_shooting_stop();

        if (app_srv_camera_capture_preview()) {
            app_loge("%s: enter preview failed\n", __func__);
        }
        /* show g_capture_btn */
        lv_obj_clear_flag(capture_btn,LV_OBJ_FLAG_HIDDEN);
        g_display_timer = NULL;
    }
    else {
        app_loge("%s: timer has been del\n", __func__);
    }
}

static void camera_image_display(u8 *data, u32 size)
{
    /* hide g_capture_btn */
    lv_obj_add_flag(capture_btn,LV_OBJ_FLAG_HIDDEN);

    comp_camera_preview_pause();
    g_display_timer = lv_timer_create(display_timer, CAM_CAPTURE_DELAY * 1000, NULL);/* timeout 时间 <50ms, camera 刷新率是 20fps */
    lv_timer_set_repeat_count(g_display_timer, 1);
}

static void camera_image_save_cb(void *data, u32 size)
{
    app_logi("%s: free data: %p", __func__, data);

    if (data)
        aic_free(data);
}

static void camera_shooting(camera_notify_callback cb)
{
    char path[CAM_IMG_PATH_NAME_LEN] = {CAM_IMG_DISK_LETTER};
    u8 *img_data = NULL;
    u32 img_size = 0;
    int ret = 0;

    if (app_srv_camera_capture_prepare(&img_size)) {
        app_loge("%s: camera prepare failed\n", __func__);
        return;
    }

    app_logi("%s: image size: %d\n", __func__, img_size);

    img_data = (u8 *)aic_malloc(img_size);
    if (!img_data) {
        app_loge("%s: no more memory!\n", __func__);
        return;
    }
    memset(img_data, 0, img_size);

    ret =
        app_srv_camera_shooting_start(img_data, img_size, cb);
    if (ret) {
        if (ret == -1)
            app_loge("%s: camera shot failed\n", __func__);
        app_logi("%s: camera time lapse mode\n", __func__);
        aic_free(img_data);
        goto END;
    }

    strcpy(path + strlen(path), CAM_IMG_DIR);

    ret = app_srv_camera_image_save(path, img_data, img_size,
                                    camera_image_save_cb, 1);
    if (ret) {
        app_loge("%s: camera shot failed\n", __func__);
        aic_free(img_data);
        goto END;
    }

    camera_image_display(img_data, img_size);
    return;

END:
    app_srv_camera_shooting_stop();

    if (app_srv_camera_capture_preview()) {
        app_loge("%s: enter preview failed\n", __func__);
    }
}

static void camera_time_lapse_cb(void *data, u32 size)
{
    char path[CAM_IMG_PATH_NAME_LEN] = {CAM_IMG_DISK_LETTER};
    u8 *img_data = (u8 *)data;
    u32 img_size = size;
    int ret = 0;

    if (!img_data) {
        app_loge("%s: invalid param\n", __func__);
        return;
    }

    ret =
        app_srv_camera_shooting_start(img_data, img_size, 0);
    if (ret) {
        app_loge("%s: camera shot failed\n", __func__);
        aic_free(img_data);
        goto END;
    }

    strcpy(path + strlen(path), CAM_IMG_DIR);
    ret = app_srv_camera_image_save(path, img_data, img_size,
                                    camera_image_save_cb, 1);
    if (ret) {
        app_loge("%s: camera shot failed\n", __func__);
        aic_free(img_data);
        goto END;
    }

    camera_image_display(img_data, img_size);
    return;

END:
    app_srv_camera_shooting_stop();

    if (app_srv_camera_capture_preview()) {
        app_loge("%s: enter preview failed\n", __func__);
    }
}

#ifdef CONFIG_CAPTURE_COUNTDOWN_IN_FLOW
static void camera_capture_time_lapse_cb(lv_timer_t *timer)
{
    camera_shooting(0);
    lv_timer_del(timer);
}
#endif /* CONFIG_CAPTURE_COUNTDOWN_IN_FLOW */

/**********************
 *   PUBLIC FUNCTIONS
 **********************/

/**
 * flow_camera_enter() - enter camera app flow.
 * @param: pointer to user data, reserved.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_enter(void *param)
{
    app_logi("%s: camera flow enter\n", __func__);

    app_srv_camera_start();

    if (app_srv_camera_capture_preview()) {
        app_loge("%s: preview failed\n", __func__);
        return;
    }
}

/**
 * flow_camera_leave() - exit camera app flow.
 * @param: pointer to user data, reserved.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_leave(void *param)
{
    lv_obj_t *preview_scr = g_cam_info.res.preview_scr;
    app_logi("%s: enter \n", __func__);

    if (g_display_timer) {
        app_srv_camera_shooting_stop();
        lv_timer_del(g_display_timer);
        g_display_timer = NULL;
    }

    app_srv_camera_stop();

    if (preview_scr)
        lv_obj_del(preview_scr);

}

/**
 * flow_camera_capture_evt_cb() - event callback of capture key operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_capture_evt_cb(lv_event_t *e)
{
#ifdef CONFIG_CAPTURE_COUNTDOWN_IN_FLOW
    if (app_srv_camera_capture_mode_get() == CAM_MODE_COUNTDOWN) {
        lv_timer_t *timer = NULL;

        timer = lv_timer_create(camera_capture_time_lapse_cb,
                                3000, 0);
        if (!timer)
            app_loge("%s: lv timer create fail\n", __func__);
        return;
    }
#endif /* CONFIG_CAPTURE_COUNTDOWN_IN_FLOW */

    camera_shooting(camera_time_lapse_cb);
}

/**
 * flow_camera_capture_mode_select_evt_cb() - event callback of mode selection.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_capture_mode_select_evt_cb(lv_event_t *e)
{
    cam_capture_mode_e mode = CAM_MODE_SINGLE;

    /* TODO: get the mode button id; */

    app_srv_camera_capture_mode_set(mode, g_cam_info.time);
}

/**
 * flow_camera_capture_mode_option_evt_cb() - event callback of mode option
 * list.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_capture_mode_option_evt_cb(lv_event_t *e)
{
    cam_capture_mode_e mode = CAM_MODE_SINGLE;

    /* TODO: get the mode button id; */

    app_srv_camera_capture_mode_set(mode, g_cam_info.time);
}

/**
 * flow_camera_flashlight_switch_evt_cb - event callback for flash light switch.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_flashlight_switch_evt_cb(lv_event_t *e)
{
    cam_flashlihgt_state_e state = CAM_FLASHLIGHT_OFF;

    /* set the flashlight state according to choose */

    app_srv_camera_flashlight_switch(state);
}

/**
 * flow_camera_countdown_evt_cb - event callback for countdown operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_countdown_evt_cb(lv_event_t *e)
{
    /* TODO: get the time-lapse time */
}
