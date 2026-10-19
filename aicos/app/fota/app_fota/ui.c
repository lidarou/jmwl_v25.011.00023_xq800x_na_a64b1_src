// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.2.0
// PROJECT: SquareLine_Project

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t *ui_fota_main_scrn;
lv_obj_t *ui_fota_main_top_panel;
void ui_event_fota_main_back_label(lv_event_t *e);
void ui_event_fota_dl_back_label(lv_event_t *e);
lv_obj_t *ui_fota_main_back_label;
lv_obj_t *ui_fota_main_title_label;
lv_obj_t *ui_fota_main_time_label;
lv_obj_t *ui_fota_main_check_panel;
lv_obj_t *ui_fota_main_auto_switch;
lv_obj_t *ui_fota_main_auto_label;
lv_obj_t *ui_fota_main_version_panel;
lv_obj_t *ui_fota_main_ver_label;
lv_obj_t *ui_fota_main_new_ver_label;
void ui_event_start_button(lv_event_t *e);
lv_obj_t *ui_fota_main_start_button;
lv_obj_t *ui_fota_main_start_label;
lv_obj_t *ui_fota_dl_scrn;
lv_obj_t *ui_fota_dl_top_panel;
lv_obj_t *ui_fota_dl_back_label;
lv_obj_t *ui_fota_dl_title_label;
lv_obj_t *ui_fota_dl_time_label;
lv_obj_t *ui_fota_dl_status_panel;
lv_obj_t *ui_fota_dl_status_label;
lv_obj_t *ui_fota_dl_spin_panel;
lv_obj_t *ui_fota_dl_proc_spinner;
void ui_event_stop_button(lv_event_t *e);
lv_obj_t *ui_fota_dl_stop_button;
lv_obj_t *ui_fota_dl_stop_label;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_fota_main_back_label(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  if (event_code == LV_EVENT_CLICKED) {
    app_fota_on_back_clicked(e);
  }
}
void ui_event_fota_dl_back_label(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  if (event_code == LV_EVENT_CLICKED) {
    app_fota_on_back_clicked(e);
  }
}
void ui_event_start_button(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  if (event_code == LV_EVENT_CLICKED) {
    app_fota_on_start_clicked(e);
  }
}
void ui_event_stop_button(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  if (event_code == LV_EVENT_CLICKED) {
    app_fota_on_stop_clicked(e);
  }
}

///////////////////// SCREENS ////////////////////
void ui_ScreenMain_screen_init(void) {
  ui_fota_main_scrn = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_fota_main_scrn, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_flex_flow(ui_fota_main_scrn, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(ui_fota_main_scrn, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_set_style_bg_color(ui_fota_main_scrn, lv_color_hex(0x282828),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_fota_main_scrn, 255,
                          LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_left(ui_fota_main_scrn, 0,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_right(ui_fota_main_scrn, 0,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_top(ui_fota_main_scrn, 0,
                           LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_bottom(ui_fota_main_scrn, 20,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_main_top_panel = lv_obj_create(ui_fota_main_scrn);
  lv_obj_set_height(ui_fota_main_top_panel, 41);
  lv_obj_set_width(ui_fota_main_top_panel, lv_pct(100));
  lv_obj_set_x(ui_fota_main_top_panel, 0);
  lv_obj_set_y(ui_fota_main_top_panel, -178);
  lv_obj_set_align(ui_fota_main_top_panel, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(ui_fota_main_top_panel, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(ui_fota_main_top_panel, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(ui_fota_main_top_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(ui_fota_main_top_panel, 0,
                          LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_fota_main_top_panel, lv_color_hex(0x424242),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_fota_main_top_panel, 255,
                          LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_main_back_label = lv_label_create(ui_fota_main_top_panel);
  lv_obj_set_width(ui_fota_main_back_label, 40);
  lv_obj_set_height(ui_fota_main_back_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_fota_main_back_label, 16);
  lv_obj_set_y(ui_fota_main_back_label, -19);
  lv_obj_set_align(ui_fota_main_back_label, LV_ALIGN_BOTTOM_LEFT);
  lv_label_set_long_mode(ui_fota_main_back_label,
                         LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_add_flag(ui_fota_main_back_label, LV_OBJ_FLAG_CLICKABLE);
  lv_label_set_text(ui_fota_main_back_label, "<");

  ui_fota_main_title_label = lv_label_create(ui_fota_main_top_panel);
  lv_obj_set_height(ui_fota_main_title_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_flex_grow(ui_fota_main_title_label, 1);
  lv_obj_set_align(ui_fota_main_title_label, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_fota_main_title_label, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_fota_main_title_label, "");
  lv_obj_set_style_text_align(ui_fota_main_title_label, LV_TEXT_ALIGN_CENTER,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_main_time_label = lv_label_create(ui_fota_main_top_panel);
  lv_obj_set_width(ui_fota_main_time_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_fota_main_time_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_fota_main_time_label, 16);
  lv_obj_set_y(ui_fota_main_time_label, -19);
  lv_obj_set_align(ui_fota_main_time_label, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_fota_main_time_label,
                         LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(ui_fota_main_time_label, "15:27");

  ui_fota_main_check_panel = lv_obj_create(ui_fota_main_scrn);
  lv_obj_set_height(ui_fota_main_check_panel, 50);
  lv_obj_set_width(ui_fota_main_check_panel, lv_pct(100));
  lv_obj_set_x(ui_fota_main_check_panel, 0);
  lv_obj_set_y(ui_fota_main_check_panel, -93);
  lv_obj_set_align(ui_fota_main_check_panel, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(ui_fota_main_check_panel, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(ui_fota_main_check_panel, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(ui_fota_main_check_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_color(ui_fota_main_check_panel, lv_color_hex(0x282828),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_fota_main_check_panel, 255,
                          LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_fota_main_check_panel,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_fota_main_check_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_main_auto_switch = lv_switch_create(ui_fota_main_check_panel);
  lv_obj_set_width(ui_fota_main_auto_switch, 50);
  lv_obj_set_height(ui_fota_main_auto_switch, 25);
  lv_obj_set_x(ui_fota_main_auto_switch, -47);
  lv_obj_set_y(ui_fota_main_auto_switch, -18);
  lv_obj_set_align(ui_fota_main_auto_switch, LV_ALIGN_CENTER);

  ui_fota_main_auto_label = lv_label_create(ui_fota_main_check_panel);
  lv_obj_set_width(ui_fota_main_auto_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_fota_main_auto_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_fota_main_auto_label, 16);
  lv_obj_set_y(ui_fota_main_auto_label, -19);
  lv_obj_set_align(ui_fota_main_auto_label, LV_ALIGN_BOTTOM_LEFT);
  lv_label_set_long_mode(ui_fota_main_auto_label,
                         LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(ui_fota_main_auto_label, "Auto Check update");

  ui_fota_main_version_panel = lv_obj_create(ui_fota_main_scrn);
  lv_obj_set_width(ui_fota_main_version_panel, lv_pct(100));
  lv_obj_set_flex_grow(ui_fota_main_version_panel, 1);
  lv_obj_set_x(ui_fota_main_version_panel, 1);
  lv_obj_set_y(ui_fota_main_version_panel, 0);
  lv_obj_set_align(ui_fota_main_version_panel, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(ui_fota_main_version_panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(ui_fota_main_version_panel, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(ui_fota_main_version_panel,
                    LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_color(ui_fota_main_version_panel, lv_color_hex(0x282828),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_fota_main_version_panel, 255,
                          LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_fota_main_version_panel,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_fota_main_version_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_main_ver_label = lv_label_create(ui_fota_main_version_panel);
  lv_obj_set_width(ui_fota_main_ver_label, lv_pct(100));
  lv_obj_set_flex_grow(ui_fota_main_ver_label, 1);
  lv_obj_set_x(ui_fota_main_ver_label, 16);
  lv_obj_set_y(ui_fota_main_ver_label, -19);
  lv_obj_set_align(ui_fota_main_ver_label, LV_ALIGN_BOTTOM_LEFT);
  lv_label_set_text(ui_fota_main_ver_label, "Current: W23.37.1");

  ui_fota_main_new_ver_label = lv_label_create(ui_fota_main_version_panel);
  lv_obj_set_width(ui_fota_main_new_ver_label, lv_pct(100));
  lv_obj_set_flex_grow(ui_fota_main_new_ver_label, 1);
  lv_obj_set_x(ui_fota_main_new_ver_label, 16);
  lv_obj_set_y(ui_fota_main_new_ver_label, -19);
  lv_obj_set_align(ui_fota_main_new_ver_label, LV_ALIGN_BOTTOM_LEFT);
  lv_label_set_text(ui_fota_main_new_ver_label, "New : W23.40.1");

  ui_fota_main_start_button = lv_btn_create(ui_fota_main_scrn);
  lv_obj_set_width(ui_fota_main_start_button, 120);
  lv_obj_set_height(ui_fota_main_start_button, 32);
  lv_obj_set_y(ui_fota_main_start_button, 125);
  lv_obj_set_x(ui_fota_main_start_button, lv_pct(0));
  lv_obj_set_align(ui_fota_main_start_button, LV_ALIGN_BOTTOM_MID);
  lv_obj_add_flag(ui_fota_main_start_button,
                  LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
  lv_obj_clear_flag(ui_fota_main_start_button,
                    LV_OBJ_FLAG_SCROLLABLE); /// Flags

  ui_fota_main_start_label = lv_label_create(ui_fota_main_start_button);
  lv_obj_set_width(ui_fota_main_start_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_fota_main_start_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_fota_main_start_label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_fota_main_start_label, "Update Now");
  lv_obj_set_style_border_color(ui_fota_main_start_label,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_fota_main_start_label, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_fota_main_back_label, ui_event_fota_main_back_label,
                      LV_EVENT_ALL, NULL);

  lv_obj_add_event_cb(ui_fota_main_start_button, ui_event_start_button,
                      LV_EVENT_ALL, NULL);
}
void ui_ScreenDownload_screen_init(void) {
  ui_fota_dl_scrn = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_fota_dl_scrn, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_flex_flow(ui_fota_dl_scrn, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(ui_fota_dl_scrn, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_set_style_bg_color(ui_fota_dl_scrn, lv_color_hex(0x282828),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_fota_dl_scrn, 255,
                          LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_left(ui_fota_dl_scrn, 0,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_right(ui_fota_dl_scrn, 0,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_top(ui_fota_dl_scrn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_bottom(ui_fota_dl_scrn, 20,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_dl_top_panel = lv_obj_create(ui_fota_dl_scrn);
  lv_obj_set_height(ui_fota_dl_top_panel, 41);
  lv_obj_set_width(ui_fota_dl_top_panel, lv_pct(100));
  lv_obj_set_x(ui_fota_dl_top_panel, 0);
  lv_obj_set_y(ui_fota_dl_top_panel, -178);
  lv_obj_set_align(ui_fota_dl_top_panel, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(ui_fota_dl_top_panel, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(ui_fota_dl_top_panel, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(ui_fota_dl_top_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(ui_fota_dl_top_panel, 0,
                          LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_fota_dl_top_panel, lv_color_hex(0x424242),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_fota_dl_top_panel, 255,
                          LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_dl_back_label = lv_label_create(ui_fota_dl_top_panel);
  lv_obj_set_width(ui_fota_dl_back_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_fota_dl_back_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_fota_dl_back_label, 16);
  lv_obj_set_y(ui_fota_dl_back_label, -19);
  lv_obj_set_align(ui_fota_dl_back_label, LV_ALIGN_BOTTOM_LEFT);
  lv_label_set_long_mode(ui_fota_dl_back_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_add_flag(ui_fota_dl_back_label, LV_OBJ_FLAG_CLICKABLE);
  lv_label_set_text(ui_fota_dl_back_label, "<");

  ui_fota_dl_title_label = lv_label_create(ui_fota_dl_top_panel);
  lv_obj_set_height(ui_fota_dl_title_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_flex_grow(ui_fota_dl_title_label, 1);
  lv_obj_set_align(ui_fota_dl_title_label, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_fota_dl_title_label, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_fota_dl_title_label, "");
  lv_obj_set_style_text_align(ui_fota_dl_title_label, LV_TEXT_ALIGN_CENTER,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_dl_time_label = lv_label_create(ui_fota_dl_top_panel);
  lv_obj_set_width(ui_fota_dl_time_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_fota_dl_time_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_fota_dl_time_label, 16);
  lv_obj_set_y(ui_fota_dl_time_label, -19);
  lv_obj_set_align(ui_fota_dl_time_label, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_fota_dl_time_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(ui_fota_dl_time_label, "15:27");

  ui_fota_dl_status_panel = lv_obj_create(ui_fota_dl_scrn);
  lv_obj_set_height(ui_fota_dl_status_panel, 50);
  lv_obj_set_width(ui_fota_dl_status_panel, lv_pct(100));
  lv_obj_set_x(ui_fota_dl_status_panel, 0);
  lv_obj_set_y(ui_fota_dl_status_panel, -93);
  lv_obj_set_align(ui_fota_dl_status_panel, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(ui_fota_dl_status_panel, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(ui_fota_dl_status_panel, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(ui_fota_dl_status_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_color(ui_fota_dl_status_panel, lv_color_hex(0x282828),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_fota_dl_status_panel, 255,
                          LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_fota_dl_status_panel, lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_fota_dl_status_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_dl_status_label = lv_label_create(ui_fota_dl_status_panel);
  lv_obj_set_width(ui_fota_dl_status_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_fota_dl_status_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_fota_dl_status_label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_fota_dl_status_label, "Downloading...");
  lv_obj_set_style_border_color(ui_fota_dl_status_label, lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_fota_dl_status_label, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_dl_spin_panel = lv_obj_create(ui_fota_dl_scrn);
  lv_obj_set_width(ui_fota_dl_spin_panel, lv_pct(100));
  lv_obj_set_flex_grow(ui_fota_dl_spin_panel, 1);
  lv_obj_set_x(ui_fota_dl_spin_panel, 1);
  lv_obj_set_y(ui_fota_dl_spin_panel, 0);
  lv_obj_set_align(ui_fota_dl_spin_panel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_fota_dl_spin_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_color(ui_fota_dl_spin_panel, lv_color_hex(0x282828),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_fota_dl_spin_panel, 255,
                          LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_fota_dl_spin_panel, lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_fota_dl_spin_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_fota_dl_proc_spinner = lv_spinner_create(ui_fota_dl_spin_panel, 1000, 90);
  lv_obj_set_width(ui_fota_dl_proc_spinner, 80);
  lv_obj_set_height(ui_fota_dl_proc_spinner, 80);
  lv_obj_set_align(ui_fota_dl_proc_spinner, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_fota_dl_proc_spinner, LV_OBJ_FLAG_CLICKABLE); /// Flags

  ui_fota_dl_stop_button = lv_btn_create(ui_fota_dl_scrn);
  lv_obj_set_width(ui_fota_dl_stop_button, 120);
  lv_obj_set_height(ui_fota_dl_stop_button, 32);
  lv_obj_set_y(ui_fota_dl_stop_button, 125);
  lv_obj_set_x(ui_fota_dl_stop_button, lv_pct(0));
  lv_obj_set_align(ui_fota_dl_stop_button, LV_ALIGN_BOTTOM_MID);
  lv_obj_add_flag(ui_fota_dl_stop_button,
                  LV_OBJ_FLAG_SCROLL_ON_FOCUS);                      /// Flags
  lv_obj_clear_flag(ui_fota_dl_stop_button, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  ui_fota_dl_stop_label = lv_label_create(ui_fota_dl_stop_button);
  lv_obj_set_width(ui_fota_dl_stop_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_fota_dl_stop_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_fota_dl_stop_label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_fota_dl_stop_label, "Stop");

  lv_obj_add_event_cb(ui_fota_dl_back_label, ui_event_fota_dl_back_label,
                      LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_fota_dl_stop_button, ui_event_stop_button,
                      LV_EVENT_ALL, NULL);
}

void ui_fota_init(void) {
  lv_disp_t *dispp = lv_disp_get_default();
  lv_theme_t *theme = lv_theme_default_init(
      dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
      true, LV_FONT_DEFAULT);
  lv_disp_set_theme(dispp, theme);
  ui_ScreenMain_screen_init();
  ui_ScreenDownload_screen_init();
  // lv_disp_load_scr(ui_fota_main_scrn);
}
