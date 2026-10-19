

#ifndef _SQUARELINE_CALL_UI_H
#define _SQUARELINE_CALL_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "ui_events.h"
#include "flow/call_fp_flow.h"



    LV_IMG_DECLARE(ui_img_call_contact_head_png);   // assets\call\contact_head.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_calling_png); // assets\call\phone_icon_calling.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_handup_png); // assets\call\phone_icon_handup.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_in_png);  // assets\call\phone_icon_in.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_miss_png); // assets\call\phone_icon_miss.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_out_png); // assets\call\phone_icon_out.png

    LV_IMG_DECLARE(ui_img_phonebooklist_sim_1_png);
    LV_IMG_DECLARE(ui_img_phonebooklist_sim_2_png);


#define     NAME_TEXT_COLOR         lv_color_hex(0x000000)		
#define     NAME_TEXT_BG_COLOR      lv_color_hex(0xffffff)		
#define     NAME_TEXT_COLOR_FOCUS   lv_color_hex(0xffffff)			
#define     NAME_TEXT_BG_COLOR_FOCUS lv_color_hex(0xFF)	

#define     APP_KEY_VALUE_TOP_LEFT  LV_KEY_ESC
#define     APP_KEY_VALUE_TOP_RIGHT LV_KEY_DEL

#define     CALL_MVOL_LEVEL_MAX     9
#define     CALL_MVOL_LEVEL_MIN     1

    void on_the_phone_record_start_stop(lv_obj_t * ui_Screen);
    lv_obj_t * ui_ScreenIncoming_screen_init(lv_obj_t * win,ui_call_enter_param_t * p_enter);
    void ui_ScreenIncoming_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenOnThePhone_screen_init(lv_obj_t * win,ui_call_enter_param_t * p_enter);
    void ui_ScreenOnThePhone_screen_uninit(lv_obj_t * win,void * p);
    void ui_ScreenOnThePhone_screen_focus(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenDial_screen_init(lv_obj_t * win,ui_call_enter_param_t * p_enter);
    void ui_ScreenDial_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenHangup_screen_init(lv_obj_t * win,ui_call_enter_param_t * p_enter);
    void ui_ScreenHangup_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenOnThePhoneOption_screen_init(lv_obj_t * win,void * p_enter_param);
    void ui_ScreenOnThePhoneOption_screen_uninit(lv_obj_t * win,void * p);
    void incoming_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void on_the_phone_timer_start(lv_obj_t * ui_Screen);
    void on_the_phone_timer_stop(lv_obj_t * ui_Screen);
    void on_the_phone_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void dial_timer_start(lv_obj_t * ui_Screen);
    void dial_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void hangup_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void on_the_phone_option_key_event(uint32_t key,lv_obj_t * ui_Screen);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif

