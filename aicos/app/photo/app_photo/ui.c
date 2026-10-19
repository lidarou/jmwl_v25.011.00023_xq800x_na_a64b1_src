// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.3.4
// PROJECT: photo

#include "ui.h"
#include "../../util/ui_helpers.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_photo_main;
void ui_event_photo_delete_btn(lv_event_t * e);
lv_obj_t * ui_photo_delete_btn;
lv_obj_t * ui_photo_delete_label;
void ui_event_photo_preview_btn(lv_event_t * e);
lv_obj_t * ui_photo_preview_btn;
lv_obj_t * ui_photo_preview_label;
void ui_event_photo_return_btn(lv_event_t * e);
lv_obj_t * ui_photo_return_btn;
lv_obj_t * ui_photo_return_label;
lv_obj_t * ui_photo_preview;
void ui_event_photo_preview_delete_btn(lv_event_t * e);
lv_obj_t * ui_photo_preview_delete_btn;
lv_obj_t * ui_photo_preview_delete_label;
void ui_event_photo_preview_play_btn(lv_event_t * e);
lv_obj_t * ui_photo_preview_play_btn;
lv_obj_t * ui_photo_preview_play_label;
void ui_event_photo_preview_return_btn(lv_event_t * e);
lv_obj_t * ui_photo_preview_return_btn;
lv_obj_t * ui_photo_preview_return_label;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=0
    #error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_photo_delete_btn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    // lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        photo_delete_btn_event(e);
    }
}
void ui_event_photo_preview_btn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    // lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(ui_photo_preview, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0);
        photo_preview_btn_event(e);
    }
}
void ui_event_photo_return_btn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    // lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        photo_return_btn_event(e);
    }
}
void ui_event_photo_preview_delete_btn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    // lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        photo_preview_delete_btn_event(e);
    }
}
void ui_event_photo_preview_play_btn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    // lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        photo_play_btn_event(e);
    }
}
void ui_event_photo_preview_return_btn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    // lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED) {
        photo_preview_return_btn_event(e);
    }
}

///////////////////// SCREENS ////////////////////
void ui_photo_main_screen_init(void)
{
    ui_photo_main = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_photo_main, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_photo_delete_btn = lv_btn_create(ui_photo_main);
    lv_obj_set_width(ui_photo_delete_btn, 80);
    lv_obj_set_height(ui_photo_delete_btn, 50);
    lv_obj_set_align(ui_photo_delete_btn, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_add_flag(ui_photo_delete_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_photo_delete_btn, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_photo_delete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_photo_delete_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_photo_delete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_photo_delete_btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_photo_delete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_delete_label = lv_label_create(ui_photo_delete_btn);
    lv_obj_set_width(ui_photo_delete_label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_photo_delete_label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_photo_delete_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_photo_delete_label, "Delete");
    lv_obj_set_style_text_color(ui_photo_delete_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_photo_delete_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_preview_btn = lv_btn_create(ui_photo_main);
    lv_obj_set_width(ui_photo_preview_btn, 80);
    lv_obj_set_height(ui_photo_preview_btn, 50);
    lv_obj_set_align(ui_photo_preview_btn, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(ui_photo_preview_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_photo_preview_btn, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_photo_preview_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_photo_preview_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_photo_preview_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_photo_preview_btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_photo_preview_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_preview_label = lv_label_create(ui_photo_preview_btn);
    lv_obj_set_width(ui_photo_preview_label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_photo_preview_label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_photo_preview_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_photo_preview_label, "Preview");
    lv_obj_set_style_text_color(ui_photo_preview_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_photo_preview_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_return_btn = lv_btn_create(ui_photo_main);
    lv_obj_set_width(ui_photo_return_btn, 80);
    lv_obj_set_height(ui_photo_return_btn, 50);
    lv_obj_set_align(ui_photo_return_btn, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_add_flag(ui_photo_return_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_photo_return_btn, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_photo_return_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_photo_return_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_photo_return_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_photo_return_btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_photo_return_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_photo_return_btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_photo_return_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_return_label = lv_label_create(ui_photo_return_btn);
    lv_obj_set_width(ui_photo_return_label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_photo_return_label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_photo_return_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_photo_return_label, "Return");
    lv_obj_set_style_text_color(ui_photo_return_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_photo_return_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_photo_delete_btn, ui_event_photo_delete_btn, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_photo_preview_btn, ui_event_photo_preview_btn, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_photo_return_btn, ui_event_photo_return_btn, LV_EVENT_ALL, NULL);

}
void ui_photo_preview_screen_init(void)
{
    ui_photo_preview = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_photo_preview, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_photo_preview_delete_btn = lv_btn_create(ui_photo_preview);
    lv_obj_set_width(ui_photo_preview_delete_btn, 80);
    lv_obj_set_height(ui_photo_preview_delete_btn, 50);
    lv_obj_set_align(ui_photo_preview_delete_btn, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_add_flag(ui_photo_preview_delete_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_photo_preview_delete_btn, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_photo_preview_delete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_photo_preview_delete_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_photo_preview_delete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_photo_preview_delete_btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_photo_preview_delete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_preview_delete_label = lv_label_create(ui_photo_preview_delete_btn);
    lv_obj_set_width(ui_photo_preview_delete_label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_photo_preview_delete_label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_photo_preview_delete_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_photo_preview_delete_label, "Delete");
    lv_obj_set_style_text_color(ui_photo_preview_delete_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_photo_preview_delete_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_preview_play_btn = lv_btn_create(ui_photo_preview);
    lv_obj_set_width(ui_photo_preview_play_btn, 80);
    lv_obj_set_height(ui_photo_preview_play_btn, 50);
    lv_obj_set_align(ui_photo_preview_play_btn, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(ui_photo_preview_play_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_photo_preview_play_btn, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_photo_preview_play_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_photo_preview_play_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_photo_preview_play_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_photo_preview_play_btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_photo_preview_play_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_preview_play_label = lv_label_create(ui_photo_preview_play_btn);
    lv_obj_set_width(ui_photo_preview_play_label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_photo_preview_play_label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_photo_preview_play_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_photo_preview_play_label, "Play");
    lv_obj_set_style_text_color(ui_photo_preview_play_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_photo_preview_play_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_preview_return_btn = lv_btn_create(ui_photo_preview);
    lv_obj_set_width(ui_photo_preview_return_btn, 80);
    lv_obj_set_height(ui_photo_preview_return_btn, 50);
    lv_obj_set_align(ui_photo_preview_return_btn, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_add_flag(ui_photo_preview_return_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_photo_preview_return_btn, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_photo_preview_return_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_photo_preview_return_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_photo_preview_return_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_photo_preview_return_btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_photo_preview_return_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_photo_preview_return_label = lv_label_create(ui_photo_preview_return_btn);
    lv_obj_set_width(ui_photo_preview_return_label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_photo_preview_return_label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_photo_preview_return_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_photo_preview_return_label, "Return");
    lv_obj_set_style_text_color(ui_photo_preview_return_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_photo_preview_return_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_photo_preview_delete_btn, ui_event_photo_preview_delete_btn, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_photo_preview_play_btn, ui_event_photo_preview_play_btn, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_photo_preview_return_btn, ui_event_photo_preview_return_btn, LV_EVENT_ALL, NULL);

}

void ui_photo_main_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_photo_main_screen_init();
    ui_photo_preview_screen_init();
    lv_disp_load_scr(ui_photo_main);
}
