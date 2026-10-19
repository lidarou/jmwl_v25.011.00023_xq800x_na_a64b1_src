// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.2.0
// PROJECT: filebrowser

#ifndef _FILEBROWSER_UI_H
#define _FILEBROWSER_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern lv_obj_t *ui_fb_device_panel;
extern lv_obj_t *ui_fb_device_title_panel;
extern lv_obj_t *ui_fb_device_title_label;
extern lv_obj_t *ui_fb_device_list_panel;
extern lv_obj_t *ui_fb_folder_panel;
extern lv_obj_t *ui_fb_folder_title_panel;
extern lv_obj_t *ui_fb_folder_title_label;
extern lv_obj_t *ui_fb_folder_list_panel;
extern lv_obj_t *ui_fb_folder_swap_panel;
extern lv_obj_t *ui_fb_folder_swap_title_panel;
extern lv_obj_t *ui_fb_folder_swap_title_label;
extern lv_obj_t *ui_fb_folder_swap_list_panel;

LV_IMG_DECLARE(ui_img_folder_png);       // assets\folder.png
LV_IMG_DECLARE(ui_img_file_png);         // assets\file.png
LV_IMG_DECLARE(ui_img_folder_48x48_png); // assets\folder_48x48.png
LV_IMG_DECLARE(ui_img_file_48x48_png);   // assets\file_48x48.png

void ui_fb_device_list_screen_init(void);
void ui_fb_folder_screen_init(void);
void ui_fb_folder_swap_screen_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
