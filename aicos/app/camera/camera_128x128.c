/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of photo browser app
 */

/*********************
 *      INCLUDES
 *********************/
#include "camera.h"
#include "camera_flow.h"
#include "camera_service.h"
#include "aic_osa_ex.h"
#include "alv_app_mgr.h"
#include "../app_comp/camera_preview/comp_camera_preview.h"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
lv_obj_t *g_camera_preview_scr;

lv_obj_t *capture_btn = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void camera_capture_evt_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED) {
        flow_camera_capture_evt_cb(e);
    }
}

static void camera_exit_gesture_evt_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_RIGHT) {
        alv_am_app_exit(NULL, "camera",NULL);
        lv_indev_wait_release(lv_indev_get_act());
    }
}

static void camera_app_enter(lv_obj_t *app, void *p_param)
{
    g_camera_preview_scr = comp_camera_preview_create(lv_scr_act(),
                                                    lv_disp_get_hor_res(0),
                                                    lv_disp_get_ver_res(0),
                                                    2);
    if (!g_camera_preview_scr) {
        app_loge("%s: preview scr create failed\n", __func__);
        return;
    }

    /* create capture button */
    capture_btn = lv_btn_create(g_camera_preview_scr);
    if (!capture_btn) {
        app_loge("%s: capture btn create failed\n", __func__);
        comp_camera_preview_destroy(g_camera_preview_scr);
        return;
    }

    lv_obj_set_width(capture_btn, 30);
    lv_obj_set_height(capture_btn, 30);
    lv_obj_set_x(capture_btn, 0);
    lv_obj_set_y(capture_btn, -10);
    lv_obj_set_align(capture_btn, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(capture_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(capture_btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(capture_btn, 25, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(capture_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(capture_btn, lv_color_white(),
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(capture_btn, 255,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(capture_btn, 5,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(capture_btn, 0,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(capture_btn, camera_capture_evt_cb,
                        LV_EVENT_ALL, NULL);

    lv_obj_add_event_cb(lv_scr_act(), camera_exit_gesture_evt_cb,
                        LV_EVENT_GESTURE, NULL);

    flow_camera_enter(0);
}

static void camera_app_leave(lv_obj_t *app, void *p_param)
{
    if (g_camera_preview_scr) {
        comp_camera_preview_destroy(g_camera_preview_scr);
        g_camera_preview_scr = NULL;
    }

    flow_camera_leave(0);
}

static void camera_app_focus(lv_obj_t *app, void *p_param)
{
}

static void camera_app_defocus(lv_obj_t *app, void *p_param)
{
}

/**********************
 *   PUBLIC FUNCTIONS
 **********************/
/**
 * app_camera_init() - register camera app
 * @param: pointer to user data for new framework, can be 0.
 * @arg2: none.
 * Return: pointer to camera app object.
 */
void app_camera_init(void *param)
{
    lv_obj_t *app = alv_am_app_create(NULL, "camera");
    if (!app) {
        app_loge("%s: app create failed\n", __func__);
        return;
    }

    alv_am_app_set_active(app, camera_app_enter);
    alv_am_app_set_deactive(app, camera_app_leave);
    alv_am_app_set_focus(app, camera_app_focus);
    alv_am_app_set_defocus(app, camera_app_defocus);
}

/**
 * app_camera_deinit() - unregister camera app
 * @param: pointer to user data for new framework, can be 0.
 * @arg2: none.
 * Return: pointer to camera app object.
 */
void app_camera_deinit(void *param)
{
    flow_camera_leave(0);
}