

#ifndef _SQUARELINE_SMSLIST_UI_H
#define _SQUARELINE_SMSLIST_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "ui_events.h"


    LV_IMG_DECLARE(ui_img_1_png);
    LV_IMG_DECLARE(ui_img_2_png);
    LV_IMG_DECLARE(ui_img_3_png);
    LV_IMG_DECLARE(ui_img_4_png);
    LV_IMG_DECLARE(ui_img_5_png);


    LV_IMG_DECLARE(ui_img_phonebooklist_check_box_png);
    LV_IMG_DECLARE(ui_img_phonebooklist_check_box_pre_png);
    LV_IMG_DECLARE(ui_img_phonebooklist_phone_png);
    LV_IMG_DECLARE(ui_img_phonebooklist_sim_1_png);
    LV_IMG_DECLARE(ui_img_phonebooklist_sim_2_png);
    LV_IMG_DECLARE(ui_img_sms_message_png);
    LV_IMG_DECLARE(ui_img_sms_message_readed_png);


#define     APP_KEY_VALUE_TOP_LEFT  LV_KEY_ESC
#define     APP_KEY_VALUE_TOP_RIGHT LV_KEY_DEL



#define     NAME_TEXT_COLOR         lv_color_hex(0x000000)		
#define     NAME_TEXT_BG_COLOR      lv_color_hex(0xffffff)		
#define     NAME_TEXT_COLOR_FOCUS   lv_color_hex(0xffffff)			
#define     NAME_TEXT_BG_COLOR_FOCUS lv_color_hex(0xFF)	



    typedef struct UI_SCREEN_TTS_OPTION_ON_OFF_ENTER_PARAM_H {
        uint8_t         type;
        uint8_t *       title;
        } ui_screen_tts_option_on_off_enter_param_t;


    typedef struct UI_SCREEN_TTS_TIME_SETELT_ENTER_PARAM_H {
        uint8_t         type;                       //0:开始时间，1，结束时间
        } ui_screen_tts_time_select_enter_param_t;


    lv_obj_t * ui_ScreenTtsOption_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOption_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOption_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenTtsOptionOnOff_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionOnOff_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionOnOff_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenTtsOptionVol_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionVol_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionVol_screen_focus(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionVol_screen_defocus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenTtsOptionTimeAuto_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionTimeAuto_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionTimeAuto_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenTtsOptionTimeSet_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionTimeSet_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionTimeSet_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenTtsOptionTimeSelect_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionTimeSelect_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenTtsOptionTimeSelect_screen_focus(lv_obj_t * win,void * enter_param);
    void tts_option_on_off_remind_screen(lv_obj_t * ui);
    void tts_option_key_event(uint32_t key,lv_obj_t * ui_ScreenTtsOption);
    void tts_option_on_off_key_event(uint32_t key,lv_obj_t * ui_ScreenTtsOptionAllOnOff);
    void tts_option_vol_key_event(uint32_t key,lv_obj_t * ui_ScreenTtsOptionVol);
    void tts_option_time_auto_key_event(uint32_t key,lv_obj_t * ui_ScreenTtsOptionTimeAuto);
    void tts_option_time_set_key_event(uint32_t key,lv_obj_t * ui_ScreenTtsOptionTimeSet);
    void tts_option_time_select_key_event(uint32_t key,lv_obj_t * ui_ScreenTtsOptionTimeSelect);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif

