// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.2.0
// PROJECT: filebrowser

#include "ui.h"
#include "../filebrowser_flow.h"
#include <ui_helpers.h>

///////////////////// VARIABLES ////////////////////
lv_obj_t *ui_fb_device_panel = NULL;
lv_obj_t *ui_fb_device_title_panel = NULL;
lv_obj_t *ui_fb_device_title_label = NULL;
lv_obj_t *ui_fb_device_list_panel = NULL;
lv_obj_t *ui_fb_folder_panel = NULL;
lv_obj_t *ui_fb_folder_title_panel = NULL;
lv_obj_t *ui_fb_folder_title_label = NULL;
lv_obj_t *ui_fb_folder_list_panel = NULL;
lv_obj_t *ui_fb_folder_swap_panel = NULL;
lv_obj_t *ui_fb_folder_swap_title_panel = NULL;
lv_obj_t *ui_fb_folder_swap_title_label = NULL;
lv_obj_t *ui_fb_folder_swap_list_panel = NULL;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
static void ui_fb_clean_panel(lv_obj_t *panel)
{
    lv_obj_set_style_pad_row(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(panel, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_radius(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(panel, LV_BORDER_SIDE_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

///////////////////// SCREENS ////////////////////
void ui_fb_device_list_screen_init(void)
{
    ui_fb_device_panel = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_fb_device_panel,
                      LV_OBJ_FLAG_SCROLLABLE |
                          LV_OBJ_FLAG_GESTURE_BUBBLE); /// Flags
    lv_obj_set_flex_flow(ui_fb_device_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_fb_device_panel, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    ui_fb_clean_panel(ui_fb_device_panel);

    ui_fb_device_title_panel = lv_obj_create(ui_fb_device_panel);
    lv_obj_set_height(ui_fb_device_title_panel, 32);
    lv_obj_set_width(ui_fb_device_title_panel, lv_pct(100));
    lv_obj_set_flex_flow(ui_fb_device_title_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_fb_device_title_panel, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_fb_device_title_panel,
                      LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE |
                          LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC |
                          LV_OBJ_FLAG_SCROLL_MOMENTUM |
                          LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_style_radius(ui_fb_device_title_panel, 0,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fb_device_title_panel, 0,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_fb_device_title_panel, LV_BORDER_SIDE_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_fb_device_title_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_fb_device_title_panel, 0,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_fb_device_title_panel, 0,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_fb_device_title_panel, 0,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_fb_device_title_label = lv_label_create(ui_fb_device_title_panel);
    lv_obj_set_width(ui_fb_device_title_label, LV_SIZE_CONTENT);  /// 216
    lv_obj_set_height(ui_fb_device_title_label, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_fb_device_title_label, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_fb_device_title_label, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_fb_device_title_label, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_label_set_text(ui_fb_device_title_label, "File Browser");
    /* add left padding to the title */
    lv_obj_set_style_pad_left(ui_fb_device_title_label, 8, 0);

    ui_fb_device_list_panel = lv_obj_create(ui_fb_device_panel);
    lv_obj_set_width(ui_fb_device_list_panel, lv_pct(100));
    lv_obj_set_flex_grow(ui_fb_device_list_panel, 1);
    lv_obj_set_style_radius(ui_fb_device_list_panel, 0,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fb_device_list_panel, 0,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_fb_device_list_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_fb_device_list_panel, 0,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_fb_device_list_panel, 0,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_fb_device_list_panel, 0,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_fb_device_panel, ui_fb_event_device_gesture_back,
                        LV_EVENT_GESTURE, NULL);
}
void ui_fb_folder_screen_init(void)
{
    ui_fb_folder_panel = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_fb_folder_panel,
                      LV_OBJ_FLAG_SCROLLABLE |
                          LV_OBJ_FLAG_GESTURE_BUBBLE); /// Flags
    lv_obj_set_flex_flow(ui_fb_folder_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_fb_folder_panel, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    ui_fb_clean_panel(ui_fb_folder_panel);
    ui_fb_folder_title_panel = lv_obj_create(ui_fb_folder_panel);
    lv_obj_set_height(ui_fb_folder_title_panel, 32);
    lv_obj_set_width(ui_fb_folder_title_panel, lv_pct(100));
    lv_obj_set_flex_flow(ui_fb_folder_title_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_fb_folder_title_panel, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_fb_folder_title_panel,
                      LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE |
                          LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC |
                          LV_OBJ_FLAG_SCROLL_MOMENTUM |
                          LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_style_radius(ui_fb_folder_title_panel, 0,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fb_folder_title_panel, 0,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_fb_folder_title_panel, LV_BORDER_SIDE_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_fb_folder_title_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_fb_folder_title_panel, 0,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_fb_folder_title_panel, 0,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_fb_folder_title_panel, 0,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_fb_folder_title_label = lv_label_create(ui_fb_folder_title_panel);
    lv_obj_set_width(ui_fb_folder_title_label, LV_SIZE_CONTENT);  /// 216
    lv_obj_set_height(ui_fb_folder_title_label, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_fb_folder_title_label, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_fb_folder_title_label, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_fb_folder_title_label, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_label_set_text(ui_fb_folder_title_label, "File Browser");
    lv_label_set_long_mode(ui_fb_folder_title_label, LV_LABEL_LONG_SCROLL);
    /* add left padding to the title */
    lv_obj_set_style_pad_left(ui_fb_folder_title_label, 8, 0);
    ui_fb_folder_list_panel = lv_obj_create(ui_fb_folder_panel);
    lv_obj_set_width(ui_fb_folder_list_panel, lv_pct(100));
    lv_obj_set_flex_grow(ui_fb_folder_list_panel, 1);
    lv_obj_set_style_radius(ui_fb_folder_list_panel, 0,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fb_folder_list_panel, 0,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_fb_folder_list_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_fb_folder_list_panel, 0,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_fb_folder_list_panel, 0,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_fb_folder_list_panel, 0,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    /* make it possible to receive key event */
    lv_group_add_obj(lv_group_get_default(), ui_fb_folder_panel);
    lv_obj_add_event_cb(ui_fb_folder_panel, ui_fb_folder_event_handler,
                        LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(ui_fb_folder_list_panel, ui_fb_folder_event_handler,
                        LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fb_folder_panel, ui_fb_event_gesture_back,
                        LV_EVENT_GESTURE, NULL);
}
void ui_fb_folder_swap_screen_init(void)
{
    ui_fb_folder_swap_panel = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_fb_folder_swap_panel,
                      LV_OBJ_FLAG_SCROLLABLE |
                          LV_OBJ_FLAG_GESTURE_BUBBLE); /// Flags
    lv_obj_set_flex_flow(ui_fb_folder_swap_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_fb_folder_swap_panel, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    ui_fb_clean_panel(ui_fb_folder_swap_panel);
    ui_fb_folder_swap_title_panel = lv_obj_create(ui_fb_folder_swap_panel);
    lv_obj_set_height(ui_fb_folder_swap_title_panel, 32);
    lv_obj_set_width(ui_fb_folder_swap_title_panel, lv_pct(100));
    lv_obj_set_flex_flow(ui_fb_folder_swap_title_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_fb_folder_swap_title_panel, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_fb_folder_swap_title_panel,
                      LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE |
                          LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC |
                          LV_OBJ_FLAG_SCROLL_MOMENTUM |
                          LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_style_radius(ui_fb_folder_swap_title_panel, 0,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fb_folder_swap_title_panel, 0,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_fb_folder_swap_title_panel,
                                 LV_BORDER_SIDE_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_fb_folder_swap_title_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_fb_folder_swap_title_panel, 0,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_fb_folder_swap_title_panel, 0,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_fb_folder_swap_title_panel, 0,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_fb_folder_swap_title_label =
        lv_label_create(ui_fb_folder_swap_title_panel);
    lv_obj_set_width(ui_fb_folder_swap_title_label, LV_SIZE_CONTENT);  /// 216
    lv_obj_set_height(ui_fb_folder_swap_title_label, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_fb_folder_swap_title_label, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_fb_folder_swap_title_label, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_fb_folder_swap_title_label, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_label_set_text(ui_fb_folder_swap_title_label, "File Browser");
    lv_label_set_long_mode(ui_fb_folder_swap_title_label, LV_LABEL_LONG_SCROLL);
    /* add left padding to the title */
    lv_obj_set_style_pad_left(ui_fb_folder_swap_title_label, 8, 0);
    ui_fb_folder_swap_list_panel = lv_obj_create(ui_fb_folder_swap_panel);
    lv_obj_set_width(ui_fb_folder_swap_list_panel, lv_pct(100));
    lv_obj_set_flex_grow(ui_fb_folder_swap_list_panel, 1);
    lv_obj_set_style_radius(ui_fb_folder_swap_list_panel, 0,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fb_folder_swap_list_panel, 0,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_fb_folder_swap_list_panel, 0,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_fb_folder_swap_list_panel, 0,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_fb_folder_swap_list_panel, 0,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_fb_folder_swap_list_panel, 0,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    /* make it possible to receive key event */
    lv_group_add_obj(lv_group_get_default(), ui_fb_folder_swap_panel);
    lv_obj_add_event_cb(ui_fb_folder_swap_panel, ui_fb_folder_event_handler,
                        LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(ui_fb_folder_swap_list_panel,
                        ui_fb_folder_event_handler, LV_EVENT_LONG_PRESSED,
                        NULL);
    lv_obj_add_event_cb(ui_fb_folder_swap_panel, ui_fb_event_gesture_back,
                        LV_EVENT_GESTURE, NULL);
}
