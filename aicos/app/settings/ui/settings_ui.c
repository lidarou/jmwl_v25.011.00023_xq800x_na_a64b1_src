/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/


/**
 *  DESCRIPTION
 *
 *    This file defines the apis of settings app ui.
 */

/*********************
 *      INCLUDES
 *********************/
#include "alv_app_mgr.h"
#include "statusbar.h"
#include "softkey.h"

#include "win_settingsmenu/ui.h"
#include "win_backlight/ui.h"
#include "win_ring_volume/ui.h"
#include "win_about/ui.h"
#include "win_version_info/ui.h"
#include "win_device_info/ui.h"
#include "win_phone/ui.h"
/*********************
 *      DEFINES
 *********************/


/**********************
 *   STATIC FUNCTIONS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * settings_ui_init() - the ui init of settings app.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void settings_ui_init(void)
{
    /* Create APP for Settings */
    lv_obj_t *app_settings = alv_am_app_create(lv_scr_act(), "settings");

    lv_obj_t *win_settings_menu = alv_am_win_create(app_settings, "settings_menu");
    alv_am_win_set_active(win_settings_menu, settings_win_active);
    alv_am_win_set_deactive(win_settings_menu, settings_win_deactive);
    alv_am_win_set_focus(win_settings_menu, settings_win_focus);
    alv_am_win_set_defocus(win_settings_menu, settings_win_defocus);
    alv_am_app_set_first_win(app_settings, win_settings_menu);

    lv_obj_t *win_settings_backlight = alv_am_win_create(app_settings, "backlight");
    alv_am_win_set_active(win_settings_backlight, backlight_win_active);
    alv_am_win_set_deactive(win_settings_backlight, backlight_win_deactive);
    alv_am_win_set_focus(win_settings_backlight, backlight_win_focus);
    alv_am_win_set_defocus(win_settings_backlight, backlight_win_defocus);

    lv_obj_t *win_settings_ring = alv_am_win_create(app_settings, "ring_volume");
    alv_am_win_set_active(win_settings_ring, ring_volume_win_active);
    alv_am_win_set_deactive(win_settings_ring, ring_volume_win_deactive);
    alv_am_win_set_focus(win_settings_ring, ring_volume_win_focus);
    alv_am_win_set_defocus(win_settings_ring, ring_volume_win_defocus);

    lv_obj_t *win_settings_phone = alv_am_win_create(app_settings, "phone");
    alv_am_win_set_active(win_settings_phone, phone_win_active);
    alv_am_win_set_deactive(win_settings_phone, phone_win_deactive);
    alv_am_win_set_focus(win_settings_phone, phone_win_focus);
    alv_am_win_set_defocus(win_settings_phone, phone_win_defocus);

    lv_obj_t *win_about_device_info = alv_am_win_create(app_settings, "device_info");
    alv_am_win_set_active(win_about_device_info, device_info_win_active);
    alv_am_win_set_deactive(win_about_device_info, device_info_win_deactive);
    alv_am_win_set_focus(win_about_device_info, device_info_win_focus);

    lv_obj_t *win_settings_about = alv_am_win_create(app_settings, "about");
    alv_am_win_set_active(win_settings_about, about_win_active);
    alv_am_win_set_deactive(win_settings_about, about_win_deactive);
    alv_am_win_set_focus(win_settings_about, about_win_focus);
    alv_am_win_set_defocus(win_settings_about, about_win_defocus);

    lv_obj_t *win_about_version_info = alv_am_win_create(app_settings, "version_info");
    alv_am_win_set_active(win_about_version_info, version_info_win_active);
    alv_am_win_set_deactive(win_about_version_info, version_info_win_deactive);
    alv_am_win_set_focus(win_about_version_info, version_info_win_focus);
}
