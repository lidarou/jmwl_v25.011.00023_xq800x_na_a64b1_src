

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



    LV_IMG_DECLARE(ui_img_multi_document_png);      // assets\multi\document.png
    LV_IMG_DECLARE(ui_img_multi_file_png);          // assets\multi\file.png
    LV_IMG_DECLARE(ui_img_multi_record_png);        // assets\multi\record.png
    LV_IMG_DECLARE(ui_img_multi_recording_png);     // assets\multi\recording.png
    LV_IMG_DECLARE(ui_img_multi_ff_png);            // assets\multi\ff.png
    LV_IMG_DECLARE(ui_img_multi_fr_png);            // assets\multi\fr.png
    LV_IMG_DECLARE(ui_img_multi_last_png);          // assets\multi\last.png
    LV_IMG_DECLARE(ui_img_multi_music_bg_png);      // assets\multi\music_bg.png
    LV_IMG_DECLARE(ui_img_multi_next_png);          // assets\multi\next.png
    LV_IMG_DECLARE(ui_img_multi_play_png);          // assets\multi\play.png
    LV_IMG_DECLARE(ui_img_multi_stop_png);          // assets\multi\stop.png
    LV_IMG_DECLARE(ui_img_multi_playmode_loop_png); // assets\multi\playmode_loop.png
    LV_IMG_DECLARE(ui_img_multi_playmode_order_png); // assets\multi\playmode_order.png
    LV_IMG_DECLARE(ui_img_multi_playmode_random_png); // assets\multi\playmode_random.png
    LV_IMG_DECLARE(ui_img_multi_playmode_single_png); // assets\multi\playmode_single.png
    LV_IMG_DECLARE(ui_img_multi_vol_1_png);         // assets\multi\vol_1.png
    LV_IMG_DECLARE(ui_img_multi_vol_10_png);        // assets\multi\vol_10.png
    LV_IMG_DECLARE(ui_img_multi_vol_2_png);         // assets\multi\vol_2.png
    LV_IMG_DECLARE(ui_img_multi_vol_3_png);         // assets\multi\vol_3.png
    LV_IMG_DECLARE(ui_img_multi_vol_4_png);         // assets\multi\vol_4.png
    LV_IMG_DECLARE(ui_img_multi_vol_5_png);         // assets\multi\vol_5.png
    LV_IMG_DECLARE(ui_img_multi_vol_6_png);         // assets\multi\vol_6.png
    LV_IMG_DECLARE(ui_img_multi_vol_7_png);         // assets\multi\vol_7.png
    LV_IMG_DECLARE(ui_img_multi_vol_8_png);         // assets\multi\vol_8.png
    LV_IMG_DECLARE(ui_img_multi_vol_9_png);         // assets\multi\vol_9.png








#define     NAME_TEXT_COLOR         lv_color_hex(0x000000)		
#define     NAME_TEXT_BG_COLOR      lv_color_hex(0xffffff)		
#define     NAME_TEXT_COLOR_FOCUS   lv_color_hex(0xffffff)			
#define     NAME_TEXT_BG_COLOR_FOCUS lv_color_hex(0xFF)	


#define     APP_KEY_VALUE_TOP_LEFT  LV_KEY_ESC
#define     APP_KEY_VALUE_TOP_RIGHT LV_KEY_DEL


#define     MUSIC_MVOL_LEVEL_MAX    9
#define     MUSIC_MVOL_LEVEL_MIN    0

    typedef struct __screen_record_play_enter_param_h_ {
        uint32_t        d_off;
        } screen_record_play_enter_param_t;


    typedef struct __screen_file_browser_play_enter_param_h_ {
        uint8_t *       path;
        } screen_file_browser_play_enter_param_t;


    lv_obj_t * ui_ScreenMultimediaMenu_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenMultimediaMenu_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenFileBrowser_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenFileBrowser_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenFileBrowser_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenMuiscPlay_screen_init(lv_obj_t * win,void * param);
    void ui_ScreenMuiscPlay_screen_uninit(lv_obj_t * win,void * param);
    void ui_ScreenMuiscPlay_screen_focus(lv_obj_t * win,void * param);
    void ui_ScreenMuiscPlay_screen_defocus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenMuiscOption_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenMuiscOption_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenMuiscSetup_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenMuiscSetup_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenMuiscList_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenMuiscList_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenRecord_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenRecord_screen_uninit(lv_obj_t * win,void * p);
    void ui_ScreenRecord_screen_focus(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenRecordOption_screen_init(lv_obj_t * win,void * p_ui_detail);
    void ui_ScreenRecordOption_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenRecordList_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenRecordList_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenRecordList_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenRecordPlay_screen_init(lv_obj_t * win,void * param);
    void ui_ScreenRecordPlay_screen_uninit(lv_obj_t * win,void * param);
    void ui_ScreenRecordPlay_screen_focus(lv_obj_t * win,void * param);
    void ui_ScreenRecordPlay_screen_defocus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenFileBrowserPlay_screen_init(lv_obj_t * win,void * param);
    void ui_ScreenFileBrowserPlay_screen_uninit(lv_obj_t * win,void * param);
    void ui_ScreenFileBrowserPlay_screen_focus(lv_obj_t * win,void * param);
    void ui_ScreenFileBrowserPlay_screen_defocus(lv_obj_t * win,void * param);

    void multimedia_menu_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void file_browser_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void muisc_play_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void muisc_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void muisc_setup_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void muisc_list_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void record_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void record_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void record_rev_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void record_play_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void file_browser_play_key_event(uint32_t key,lv_obj_t * ui_Screen);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif

