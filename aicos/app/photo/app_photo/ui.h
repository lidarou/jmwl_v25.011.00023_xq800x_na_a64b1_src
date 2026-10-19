// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.3.4
// PROJECT: photo

#ifndef _PHOTO_UI_H
#define _PHOTO_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern lv_obj_t * ui_photo_main;
void ui_event_photo_delete_btn(lv_event_t * e);
extern lv_obj_t * ui_photo_delete_btn;
extern lv_obj_t * ui_photo_delete_label;
void ui_event_photo_preview_btn(lv_event_t * e);
extern lv_obj_t * ui_photo_preview_btn;
extern lv_obj_t * ui_photo_preview_label;
void ui_event_photo_return_btn(lv_event_t * e);
extern lv_obj_t * ui_photo_return_btn;
extern lv_obj_t * ui_photo_return_label;
extern lv_obj_t * ui_photo_preview;
void ui_event_photo_preview_delete_btn(lv_event_t * e);
extern lv_obj_t * ui_photo_preview_delete_btn;
extern lv_obj_t * ui_photo_preview_delete_label;
void ui_event_photo_preview_play_btn(lv_event_t * e);
extern lv_obj_t * ui_photo_preview_play_btn;
extern lv_obj_t * ui_photo_preview_play_label;
void ui_event_photo_preview_return_btn(lv_event_t * e);
extern lv_obj_t * ui_photo_preview_return_btn;
extern lv_obj_t * ui_photo_preview_return_label;

void photo_delete_btn_event(lv_event_t * e);
void photo_preview_btn_event(lv_event_t * e);
void photo_return_btn_event(lv_event_t * e);
void photo_preview_delete_btn_event(lv_event_t * e);
void photo_play_btn_event(lv_event_t * e);
void photo_preview_return_btn_event(lv_event_t * e);





void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
