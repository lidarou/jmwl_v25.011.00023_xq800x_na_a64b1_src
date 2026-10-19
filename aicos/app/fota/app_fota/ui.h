// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.2.0
// PROJECT: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern lv_obj_t * ui_fota_main_scrn;
extern lv_obj_t * ui_fota_main_top_panel;
void ui_event_fota_main_back_label(lv_event_t * e);
extern lv_obj_t * ui_fota_main_back_label;
extern lv_obj_t * ui_fota_main_title_label;
extern lv_obj_t * ui_fota_main_time_label;
extern lv_obj_t * ui_fota_main_check_panel;
extern lv_obj_t * ui_fota_main_auto_switch;
extern lv_obj_t * ui_fota_main_auto_label;
extern lv_obj_t * ui_fota_main_version_panel;
extern lv_obj_t * ui_fota_main_ver_label;
extern lv_obj_t * ui_fota_main_new_ver_label;
void ui_event_start_button(lv_event_t * e);
extern lv_obj_t * ui_fota_main_start_button;
extern lv_obj_t * ui_fota_main_start_label;
extern lv_obj_t * ui_fota_dl_scrn;
extern lv_obj_t * ui_fota_dl_top_panel;
extern lv_obj_t * ui_fota_dl_back_label;
extern lv_obj_t * ui_fota_dl_title_label;
extern lv_obj_t * ui_fota_dl_time_label;
extern lv_obj_t * ui_fota_dl_status_panel;
extern lv_obj_t * ui_fota_dl_status_label;
extern lv_obj_t * ui_fota_dl_spin_panel;
extern lv_obj_t * ui_fota_dl_proc_spinner;
void ui_event_stop_button(lv_event_t * e);
extern lv_obj_t * ui_fota_dl_stop_button;
extern lv_obj_t * ui_fota_dl_stop_label;

void app_fota_on_back_clicked(lv_event_t * e);
void app_fota_on_start_clicked(lv_event_t * e);
void app_fota_on_stop_clicked(lv_event_t * e);




void ui_fota_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
