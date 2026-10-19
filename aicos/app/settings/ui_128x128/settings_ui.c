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

#include "win_settingsmenu/ui.h"
#include "win_backlight/ui.h"
#include "win_ring/ui.h"
#include "win_soundmode/ui.h"
#include "win_notice/ui.h"
#include "win_network/ui.h"
#include "win_mobile_network/ui.h"
#include "win_system/ui.h"
#include "win_about/ui.h"
#include "win_version_info/ui.h"
#include "win_reset/ui.h"
#include "settings_factoryreset.h"


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * settings_ui_init() - the init of settings app wins.
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
    alv_am_app_set_first_win(app_settings, win_settings_menu);

    lv_obj_t *win_settings_backlight = alv_am_win_create(app_settings, "backlight");
    alv_am_win_set_active(win_settings_backlight, backlight_win_active);
    alv_am_win_set_deactive(win_settings_backlight, backlight_win_deactive);
    alv_am_win_set_focus(win_settings_backlight, backlight_win_focus);

    lv_obj_t *win_settings_ring = alv_am_win_create(app_settings, "ring");
    alv_am_win_set_active(win_settings_ring, ring_win_active);
    alv_am_win_set_deactive(win_settings_ring, ring_win_deactive);
    alv_am_win_set_focus(win_settings_ring, ring_win_focus);

    lv_obj_t *win_ring_soundmode = alv_am_win_create(app_settings, "soundmode");
    alv_am_win_set_active(win_ring_soundmode, soundmode_win_active);
    alv_am_win_set_deactive(win_ring_soundmode, soundmode_win_deactive);
    alv_am_win_set_focus(win_ring_soundmode, soundmode_win_focus);

    lv_obj_t *win_settings_notice = alv_am_win_create(app_settings, "notice");
    alv_am_win_set_active(win_settings_notice, notice_win_active);
    alv_am_win_set_deactive(win_settings_notice, notice_win_deactive);
    alv_am_win_set_focus(win_settings_notice, notice_win_focus);

    lv_obj_t *win_network_mobile = alv_am_win_create(app_settings, "mobile_network");
    alv_am_win_set_active(win_network_mobile, mobile_network_win_active);
    alv_am_win_set_deactive(win_network_mobile, mobile_network_win_deactive);
    alv_am_win_set_focus(win_network_mobile, mobile_network_win_focus);

    lv_obj_t *win_settings_network = alv_am_win_create(app_settings, "network");
    alv_am_win_set_active(win_settings_network, network_win_active);
    alv_am_win_set_deactive(win_settings_network, network_win_deactive);
    alv_am_win_set_focus(win_settings_network, network_win_focus);

    lv_obj_t *win_settings_system = alv_am_win_create(app_settings, "system");
    alv_am_win_set_active(win_settings_system, system_win_active);
    alv_am_win_set_deactive(win_settings_system, system_win_deactive);
    alv_am_win_set_focus(win_settings_system, system_win_focus);

    lv_obj_t *win_settings_about = alv_am_win_create(app_settings, "about");
    alv_am_win_set_active(win_settings_about, about_win_active);
    alv_am_win_set_deactive(win_settings_about, about_win_deactive);
    alv_am_win_set_focus(win_settings_about, about_win_focus);

    lv_obj_t *win_about_reset = alv_am_win_create(app_settings, "reset");
    alv_am_win_set_active(win_about_reset, reset_win_active);
    alv_am_win_set_deactive(win_about_reset, reset_win_deactive);
    alv_am_win_set_focus(win_about_reset, reset_win_focus);

    /*----- Sub Windows ------*/
    lv_obj_t *win_settings_factoryreset = alv_am_win_create(app_settings, "factoryreset");
    alv_am_win_set_active(win_settings_factoryreset, factoryreset_active);

    lv_obj_t *win_about_version_info = alv_am_win_create(app_settings, "version_info");
    alv_am_win_set_active(win_about_version_info, version_info_win_active);
    alv_am_win_set_deactive(win_about_version_info, version_info_win_deactive);
    alv_am_win_set_focus(win_about_version_info, version_info_win_focus);
}
