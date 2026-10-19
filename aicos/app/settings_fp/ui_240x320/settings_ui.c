

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

#include "win_settingsmenu/ui.h"
#include "win_settingsfamily/ui.h"
#include "win_settingssos/ui.h"
#include "win_settingsdisplay/ui.h"
#include "win_settingsphone/ui.h"
#include "win_settingsnet/ui.h"
#include "win_settingssecurity/ui.h"
#include "win_settingsreset/ui.h"
#include "win_settingstheme/ui.h"
#include "win_settingsdate/ui.h"
#include "win_settingscomm/ui.h"
#include "win_settingsota/ui.h"

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
    lv_obj_t *      app_settings = alv_am_app_create(lv_scr_act(),"settings");

    lv_obj_t *      win_settings_menu = alv_am_win_create(app_settings,"settings_menu"); // 设置菜单本身

    alv_am_win_set_active(win_settings_menu,settings_win_active);
    alv_am_win_set_deactive(win_settings_menu,settings_win_deactive);
    alv_am_win_set_focus(win_settings_menu,settings_win_focus);
    alv_am_win_set_defocus(win_settings_menu,settings_win_defocus);
    alv_am_app_set_first_win(app_settings,win_settings_menu);

    //////////////////////////////////////////////////////////////////////////////////////////////////////	
    lv_obj_t *      win_settings_family = alv_am_win_create(app_settings,"settings_family"); // 亲情号

    alv_am_win_set_active(win_settings_family,settings_family_win_active);
    alv_am_win_set_deactive(win_settings_family,settings_family_win_deactive);
    alv_am_win_set_focus(win_settings_family,settings_family_win_focus);
    alv_am_win_set_defocus(win_settings_family,settings_family_win_defocus);
    lv_obj_t *      win_settings_family_item = alv_am_win_create(app_settings,"setting_family_item"); // 亲情号

    alv_am_win_set_active(win_settings_family_item,settings_family_item_win_active);
    alv_am_win_set_deactive(win_settings_family_item,settings_family_item_win_deactive);
    alv_am_win_set_focus(win_settings_family_item,settings_family_item_win_focus);
    alv_am_win_set_defocus(win_settings_family_item,settings_family_item_win_defocus);
    lv_obj_t *      win_settings_family_num = alv_am_win_create(app_settings,"setting_family_num"); // 亲情号

    alv_am_win_set_active(win_settings_family_num,settings_family_num_win_active);
    alv_am_win_set_deactive(win_settings_family_num,settings_family_num_win_deactive);
    alv_am_win_set_focus(win_settings_family_num,settings_family_num_win_focus);
    alv_am_win_set_defocus(win_settings_family_num,settings_family_num_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_sos = alv_am_win_create(app_settings,"settings_sos"); // SOS

    alv_am_win_set_active(win_settings_sos,settings_sos_win_active);
    alv_am_win_set_deactive(win_settings_sos,settings_sos_win_deactive);
    alv_am_win_set_focus(win_settings_sos,settings_sos_win_focus);
    alv_am_win_set_defocus(win_settings_sos,settings_sos_win_defocus);
    lv_obj_t *      win_settings_sos_item = alv_am_win_create(app_settings,"setting_sos_item"); // SOS

    alv_am_win_set_active(win_settings_sos_item,settings_sos_item_win_active);
    alv_am_win_set_deactive(win_settings_sos_item,settings_sos_item_win_deactive);
    alv_am_win_set_focus(win_settings_sos_item,settings_sos_item_win_focus);
    alv_am_win_set_defocus(win_settings_sos_item,settings_sos_item_win_defocus);
    lv_obj_t *      win_settings_sos_num = alv_am_win_create(app_settings,"setting_sos_num"); // SOS

    alv_am_win_set_active(win_settings_sos_num,settings_sos_num_win_active);
    alv_am_win_set_deactive(win_settings_sos_num,settings_sos_num_win_deactive);
    alv_am_win_set_focus(win_settings_sos_num,settings_sos_num_win_focus);
    alv_am_win_set_defocus(win_settings_sos_num,settings_sos_num_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_display = alv_am_win_create(app_settings,"settings_disp"); // 显示设置

    alv_am_win_set_active(win_settings_display,settings_display_win_active);
    alv_am_win_set_deactive(win_settings_display,settings_display_win_deactive);
    alv_am_win_set_focus(win_settings_display,settings_display_win_focus);
    alv_am_win_set_defocus(win_settings_display,settings_display_win_defocus);
    lv_obj_t *      win_settings_display_item = alv_am_win_create(app_settings,"setting_disp_item"); // 

    alv_am_win_set_active(win_settings_display_item,settings_display_item_win_active);
    alv_am_win_set_deactive(win_settings_display_item,settings_display_item_win_deactive);
    alv_am_win_set_focus(win_settings_display_item,settings_display_item_win_focus);
    alv_am_win_set_defocus(win_settings_display_item,settings_display_item_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_phone = alv_am_win_create(app_settings,"settings_phone"); // 电话设置

    alv_am_win_set_active(win_settings_phone,settings_phone_win_active);
    alv_am_win_set_deactive(win_settings_phone,settings_phone_win_deactive);
    alv_am_win_set_focus(win_settings_phone,settings_phone_win_focus);
    alv_am_win_set_defocus(win_settings_phone,settings_phone_win_defocus);
    lv_obj_t *      win_settings_phone_lock = alv_am_win_create(app_settings,"setting_phone_lock"); // 电话设置

    alv_am_win_set_active(win_settings_phone_lock,settings_phone_lock_win_active);
    alv_am_win_set_deactive(win_settings_phone_lock,settings_phone_lock_win_deactive);
    alv_am_win_set_focus(win_settings_phone_lock,settings_phone_lock_win_focus);
    alv_am_win_set_defocus(win_settings_phone_lock,settings_phone_lock_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_net = alv_am_win_create(app_settings,"settings_net"); // 网络设置

    alv_am_win_set_active(win_settings_net,settings_net_win_active);
    alv_am_win_set_deactive(win_settings_net,settings_net_win_deactive);
    alv_am_win_set_focus(win_settings_net,settings_net_win_focus);
    alv_am_win_set_defocus(win_settings_net,settings_net_win_defocus);
    lv_obj_t *      win_settings_net_flight = alv_am_win_create(app_settings,"setting_net_flight"); // 网络设置

    alv_am_win_set_active(win_settings_net_flight,settings_net_flight_win_active);
    alv_am_win_set_deactive(win_settings_net_flight,settings_net_flight_win_deactive);
    alv_am_win_set_focus(win_settings_net_flight,settings_net_flight_win_focus);
    alv_am_win_set_defocus(win_settings_net_flight,settings_net_flight_win_defocus);
    lv_obj_t *      win_settings_net_sim = alv_am_win_create(app_settings,"setting_net_sim"); // 电话设置

    alv_am_win_set_active(win_settings_net_sim,settings_net_sim_win_active);
    alv_am_win_set_deactive(win_settings_net_sim,settings_net_sim_win_deactive);
    alv_am_win_set_focus(win_settings_net_sim,settings_net_sim_win_focus);
    alv_am_win_set_defocus(win_settings_net_sim,settings_net_sim_win_defocus);
    lv_obj_t *      win_settings_flightmode_result = alv_am_win_create(app_settings,"set_flight_result"); // 飞行模式

    alv_am_win_set_active(win_settings_flightmode_result,settings_flightmode_result_win_active);
    alv_am_win_set_deactive(win_settings_flightmode_result,settings_flightmode_result_win_deactive);
    alv_am_win_set_focus(win_settings_flightmode_result,settings_flightmode_result_win_focus);
    alv_am_win_set_defocus(win_settings_flightmode_result,settings_flightmode_result_win_defocus);
    lv_obj_t *      win_settings_simswitch_result = alv_am_win_create(app_settings,"set_sim_switch"); // SIM卡切换

    alv_am_win_set_active(win_settings_simswitch_result,settings_simswitch_result_win_active);
    alv_am_win_set_deactive(win_settings_simswitch_result,settings_simswitch_result_win_deactive);
    alv_am_win_set_focus(win_settings_simswitch_result,settings_simswitch_result_win_focus);
    alv_am_win_set_defocus(win_settings_simswitch_result,settings_simswitch_result_win_defocus);

    /////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_security = alv_am_win_create(app_settings,"settings_sec"); // 安全设置

    alv_am_win_set_active(win_settings_security,settings_security_win_active);
    alv_am_win_set_deactive(win_settings_security,settings_security_win_deactive);
    alv_am_win_set_focus(win_settings_security,settings_security_win_focus);
    alv_am_win_set_defocus(win_settings_security,settings_security_win_defocus);
    lv_obj_t *      win_settings_security_pass = alv_am_win_create(app_settings,"setting_sec_pass"); // 

    alv_am_win_set_active(win_settings_security_pass,settings_security_pass_win_active);
    alv_am_win_set_deactive(win_settings_security_pass,settings_security_pass_win_deactive);
    alv_am_win_set_focus(win_settings_security_pass,settings_security_pass_win_focus);
    alv_am_win_set_defocus(win_settings_security_pass,settings_security_pass_win_defocus);
    lv_obj_t *      win_settings_security_enable = alv_am_win_create(app_settings,"setting_sec_en"); // 

    alv_am_win_set_active(win_settings_security_enable,settings_security_enable_win_active);
    alv_am_win_set_deactive(win_settings_security_enable,settings_security_enable_win_deactive);
    alv_am_win_set_focus(win_settings_security_enable,settings_security_enable_win_focus);
    alv_am_win_set_defocus(win_settings_security_enable,settings_security_enable_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_reset = alv_am_win_create(app_settings,"settings_reset"); // 恢复出厂设置

    alv_am_win_set_active(win_settings_reset,settings_reset_win_active);
    alv_am_win_set_deactive(win_settings_reset,settings_reset_win_deactive);
    alv_am_win_set_focus(win_settings_reset,settings_reset_win_focus);
    alv_am_win_set_defocus(win_settings_reset,settings_reset_win_defocus);
    lv_obj_t *      win_settings_reset_result = alv_am_win_create(app_settings,"set_reset_result"); // 恢复出厂设置

    alv_am_win_set_active(win_settings_reset_result,settings_reset_result_win_active);
    alv_am_win_set_deactive(win_settings_reset_result,settings_reset_result_win_deactive);
    alv_am_win_set_focus(win_settings_reset_result,settings_reset_result_win_focus);
    alv_am_win_set_defocus(win_settings_reset_result,settings_reset_result_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_theme = alv_am_win_create(app_settings,"settings_theme"); // 情景模式

    alv_am_win_set_active(win_settings_theme,settings_theme_win_active);
    alv_am_win_set_deactive(win_settings_theme,settings_theme_win_deactive);
    alv_am_win_set_focus(win_settings_theme,settings_theme_win_focus);
    alv_am_win_set_defocus(win_settings_theme,settings_theme_win_defocus);
    lv_obj_t *      win_settings_theme_item = alv_am_win_create(app_settings,"set_theme_item"); // 情景模式

    alv_am_win_set_active(win_settings_theme_item,settings_theme_item_win_active);
    alv_am_win_set_deactive(win_settings_theme_item,settings_theme_item_win_deactive);
    alv_am_win_set_focus(win_settings_theme_item,settings_theme_item_win_focus);
    alv_am_win_set_defocus(win_settings_theme_item,settings_theme_item_win_defocus);
    lv_obj_t *      win_settings_theme_item_ring = alv_am_win_create(app_settings,"set_theme_ring"); // 情景模式

    alv_am_win_set_active(win_settings_theme_item_ring,settings_theme_item_ring_win_active);
    alv_am_win_set_deactive(win_settings_theme_item_ring,settings_theme_item_ring_win_deactive);
    alv_am_win_set_focus(win_settings_theme_item_ring,settings_theme_item_ring_win_focus);
    alv_am_win_set_defocus(win_settings_theme_item_ring,settings_theme_item_ring_win_defocus);
    lv_obj_t *      win_settings_theme_power = alv_am_win_create(app_settings,"set_theme_power"); // 情景模式

    alv_am_win_set_active(win_settings_theme_power,settings_theme_item_power_win_active);
    alv_am_win_set_deactive(win_settings_theme_power,settings_theme_item_power_win_deactive);
    alv_am_win_set_focus(win_settings_theme_power,settings_theme_item_power_win_focus);
    alv_am_win_set_defocus(win_settings_theme_power,settings_theme_item_power_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_date = alv_am_win_create(app_settings,"settings_date"); // 日期时间设置

    alv_am_win_set_active(win_settings_date,settings_date_win_active);
    alv_am_win_set_deactive(win_settings_date,settings_date_win_deactive);
    alv_am_win_set_focus(win_settings_date,settings_date_win_focus);
    alv_am_win_set_defocus(win_settings_date,settings_date_win_defocus);
    lv_obj_t *      win_settings_time_item = alv_am_win_create(app_settings,"set_time_item"); // 日期时间设置

    alv_am_win_set_active(win_settings_time_item,settings_date_time_item_win_active);
    alv_am_win_set_deactive(win_settings_time_item,settings_date_time_item_win_deactive);
    alv_am_win_set_focus(win_settings_time_item,settings_date_time_item_win_focus);
    alv_am_win_set_defocus(win_settings_time_item,settings_date_time_item_win_defocus);
    lv_obj_t *      win_settings_time_ampm = alv_am_win_create(app_settings,"set_time_ampm"); // 日期时间设置

    alv_am_win_set_active(win_settings_time_ampm,settings_date_time_ampm_win_active);
    alv_am_win_set_deactive(win_settings_time_ampm,settings_date_time_ampm_win_deactive);
    alv_am_win_set_focus(win_settings_time_ampm,settings_date_time_ampm_win_focus);
    alv_am_win_set_defocus(win_settings_time_ampm,settings_date_time_ampm_win_defocus);
    lv_obj_t *      win_settings_time_value = alv_am_win_create(app_settings,"set_time_value"); // 日期时间设置

    alv_am_win_set_active(win_settings_time_value,settings_date_time_value_win_active);
    alv_am_win_set_deactive(win_settings_time_value,settings_date_time_value_win_deactive);
    alv_am_win_set_focus(win_settings_time_value,settings_date_time_value_win_focus);
    alv_am_win_set_defocus(win_settings_time_value,settings_date_time_value_win_defocus);
    lv_obj_t *      win_settings_date_value = alv_am_win_create(app_settings,"set_date_value"); // 日期时间设置

    alv_am_win_set_active(win_settings_date_value,settings_date_date_value_win_active);
    alv_am_win_set_deactive(win_settings_date_value,settings_date_date_value_win_deactive);
    alv_am_win_set_focus(win_settings_date_value,settings_date_date_value_win_focus);
    alv_am_win_set_defocus(win_settings_date_value,settings_date_date_value_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_comm = alv_am_win_create(app_settings,"settings_comm"); // 

    alv_am_win_set_active(win_settings_comm,settings_comm_win_active);
    alv_am_win_set_deactive(win_settings_comm,settings_comm_win_deactive);
    alv_am_win_set_focus(win_settings_comm,settings_comm_win_focus);
    alv_am_win_set_defocus(win_settings_comm,settings_comm_win_defocus);
    lv_obj_t *      win_settings_comm_redial = alv_am_win_create(app_settings,"set_comm_redial"); // 

    alv_am_win_set_active(win_settings_comm_redial,settings_comm_redial_win_active);
    alv_am_win_set_deactive(win_settings_comm_redial,settings_comm_redial_win_deactive);
    alv_am_win_set_focus(win_settings_comm_redial,settings_comm_redial_win_focus);
    alv_am_win_set_defocus(win_settings_comm_redial,settings_comm_redial_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_t *      win_settings_ota = alv_am_win_create(app_settings,"settings_ota"); // OTA

    alv_am_win_set_active(win_settings_ota,settings_ota_win_active);
    alv_am_win_set_deactive(win_settings_ota,settings_ota_win_deactive);
    alv_am_win_set_focus(win_settings_ota,settings_ota_win_focus);
    alv_am_win_set_defocus(win_settings_ota,settings_ota_win_defocus);
    lv_obj_t *      win_settings_ota_result = alv_am_win_create(app_settings,"set_ota_result"); // ota

    alv_am_win_set_active(win_settings_ota_result,settings_ota_result_win_active);
    alv_am_win_set_deactive(win_settings_ota_result,settings_ota_result_win_deactive);
    alv_am_win_set_focus(win_settings_ota_result,settings_ota_result_win_focus);
    alv_am_win_set_defocus(win_settings_ota_result,settings_ota_result_win_defocus);
}


