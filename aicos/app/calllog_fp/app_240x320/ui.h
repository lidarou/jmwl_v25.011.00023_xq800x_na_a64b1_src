

#ifndef _SQUARELINE_CALLLOGLIST_UI_H
#define _SQUARELINE_CALLLOGLIST_UI_H

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


    LV_IMG_DECLARE(ui_img_call_contact_head_png);   // assets\call\contact_head.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_calling_png); // assets\call\phone_icon_calling.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_handup_png); // assets\call\phone_icon_handup.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_in_png);  // assets\call\phone_icon_in.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_miss_png); // assets\call\phone_icon_miss.png
    LV_IMG_DECLARE(ui_img_call_phone_icon_out_png); // assets\call\phone_icon_out.png







#define     NAME_TEXT_COLOR         lv_color_hex(0x000000)		
#define     NAME_TEXT_BG_COLOR      lv_color_hex(0xffffff)		
#define     NAME_TEXT_COLOR_FOCUS   lv_color_hex(0xffffff)			
#define     NAME_TEXT_BG_COLOR_FOCUS lv_color_hex(0xFF)	

#define     APP_KEY_VALUE_TOP_LEFT  LV_KEY_ESC
#define     APP_KEY_VALUE_TOP_RIGHT LV_KEY_DEL




    typedef enum {
        calllog_folder_dialed,
        calllog_folder_missed,
        calllog_folder_reveiced,
        calllog_folder_all,
        } calllog_folder_enum_t;


    typedef struct _calllog_list_enter_param_h_ {
        calllog_folder_enum_t folder_type;
        } calllog_list_enter_param_t;


    typedef struct _calllog_option_enter_param_h_ {
        uint32_t        file_tell_index;
        calllog_folder_enum_t folder_type;
        } calllog_option_enter_param_t;


    typedef struct _calllog_del_file_enter_param_h_ {
        uint32_t        file_tell_index;
        calllog_folder_enum_t folder_type;
        } calllog_del_file_enter_param_t;


    lv_obj_t * ui_ScreenCalllogMenu_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenCalllogMenu_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenCalllogDialCalls_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenCalllogDialCalls_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenCalllogDialCalls_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenCalllogDel_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenCalllogDel_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenDialedCallsOption_screen_init(lv_obj_t * win,calllog_option_enter_param_t * p_enter_param);
    void ui_ScreenDialedCallsOption_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenCalllogDelFile_screen_init(lv_obj_t * win,calllog_del_file_enter_param_t * p_enter_param);
    void ui_ScreenCalllogDelFile_screen_uninit(lv_obj_t * win,calllog_del_file_enter_param_t * p_enter_param);
    void calllog_menu_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void calllog_missed_calls_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void missed_calls_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void calllog_dial_calls_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void calllog_recevied_calls_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void dialed_calls_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void received_calls_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void calllog_del_file_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void calllog_del_key_event(uint32_t key,lv_obj_t * ui_Screen);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif

