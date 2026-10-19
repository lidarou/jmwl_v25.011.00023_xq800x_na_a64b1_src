

#ifndef _SQUARELINE_CALLLOGLIST_UI_H
#define _SQUARELINE_CALLLOGLIST_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "ui_events.h"






#define     NAME_TEXT_COLOR         lv_color_hex(0x000000)		
#define     NAME_TEXT_BG_COLOR      lv_color_hex(0xffffff)		
#define     NAME_TEXT_COLOR_FOCUS   lv_color_hex(0xffffff)			
#define     NAME_TEXT_BG_COLOR_FOCUS lv_color_hex(0xFF)	

#define     APP_KEY_VALUE_TOP_LEFT  LV_KEY_ESC
#define     APP_KEY_VALUE_TOP_RIGHT LV_KEY_DEL


    typedef enum {
        news_type_sms,
        news_type_call,
        low_power_warning,
        low_power_shutdown,
        usb_plug_in,
        usb_plug_out,
        charge_full,
        } enum_news_type_t;


    typedef struct UI_SCREEN_NEWS_ENTER_PARAM_H {
        enum_news_type_t type;
        uint8_t         count;
        } ui_screen_news_enter_param_t;


    void remind_news_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void ui_ScreenRemindNews_screen_loard(lv_obj_t * ui_ScreenRemindNews);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif

