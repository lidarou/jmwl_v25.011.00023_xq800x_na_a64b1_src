/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is entry of notify_center app.
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "app_log.h"
#include "alv_app_mgr.h"
#include "statusbar.h"
#include "backlight.h"
#include "app_audio.h"

#ifdef CONFIG_RESOLUTION_128x128
#include "app_notify_center_128x128/ui.h"
#endif

static void notify_center_app_focus(lv_obj_t *app, void *p_param)
{
    lv_disp_load_scr(ui_Notify_Center);

    SHOW_STATUSBAR;

    backlight_config_t init_value = {0};
    backlight_get_config(&init_value);
    lv_slider_set_value(ui_Notify_Center_BL_Slider, init_value.brightness_on, LV_ANIM_OFF);

    uint8_t p_vol = 0;
    app_audio_get_master_volume(&p_vol);
    lv_slider_set_value(ui_Notify_Center_Volume_Slider, p_vol, LV_ANIM_OFF);

    APP_LOGI("[%s] the brightness_on is %d, the p_vol is %d",
                __func__, init_value.brightness_on, p_vol);
}
static void notify_center_app_active(lv_obj_t *app, void *p_param)
{
    notify_center_ui_init(app);
}
static void notify_center_app_deactive(lv_obj_t *app, void *p_param)
{
    HIDE_STATUSBAR;

    if(ui_Notify_Center){
        lv_obj_del(ui_Notify_Center);
    }

}
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * notify_center_init() - the init of notify_center app.
 * @arg1: void.
 * @arg2: none.
 * Return: none.
 */
void notify_center_init(void)
{
    lv_obj_t * app_notify_center = alv_am_app_create(lv_scr_act(), "notify center");

    alv_am_app_set_active(app_notify_center, notify_center_app_active);
    alv_am_app_set_deactive(app_notify_center, notify_center_app_deactive);
    alv_am_app_set_focus(app_notify_center, notify_center_app_focus);
}
