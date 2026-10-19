

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


    typedef struct _sms_write_ui_enter_param_h {
        uint8_t *       p_content;
        uint8_t *       p_insert_str;
        uint8_t *       p_number;
        uint8_t         clear_content_flag;
        } sms_write_ui_enter_param_t;

    typedef struct _sms_detail_ui_enter_param_h {
        uint32_t        file_index;
        } sms_detail_ui_enter_param_t;

    typedef struct _sms_write_phone_num_find_ui_enter_param_h {
        uint8_t *       p_number;
        uint8_t *       p_name;
        } sms_write_phone_num_find_ui_enter_param_t;

    typedef struct _sms_del_select_ui_enter_param_h {
        uint8_t *       path;
        uint32_t *      p_file_index;
        uint32_t        file_count;
        } sms_del_select_ui_enter_param_t;

    typedef struct _sms_send_remind_ui_enter_param_h {
        uint8_t *       p_content;
        void * *        p_node;
        uint16_t        node_count;
        uint16_t        select_count;
        } sms_send_remind_ui_enter_param_t;

    typedef struct _sms_extract_number_ui_enter_param_h {
        uint8_t *       p_context;
        uint8_t *       number;
        } sms_extract_number_ui_enter_param_t;

    sms_detail_ui_enter_param_t * get_ui_ScreenSmsDetailRev_enter_param(lv_obj_t * ui_Screen);
    uint8_t * get_ui_ScreenSmsDetailRev_number(lv_obj_t * ui_Screen);
    uint8_t * get_ui_ScreenSmsDetailRev_content(lv_obj_t * ui_Screen);
    void ui_screen_sms_extract_number_enter(lv_obj_t * ui_cur_screen,sms_extract_number_ui_enter_param_t * enter_param);
    void sms_del_send_read_file();
    void sms_del_draft_read_file();
    void sms_del_rev_read_file(const char * path);
    lv_obj_t * ui_ScreenSmsMenu_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenSmsMenu_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsWrite_screen_init(lv_obj_t * win,void * param);
    void ui_ScreenSmsWrite_screen_uninit(lv_obj_t * win,void * param);
    void ui_ScreenSmsWrite_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenSmsWriteOption_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenSmsWriteOption_screen_uninit(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenSmsWritePhoneNum_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenSmsWritePhoneNum_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenSmsWritePhoneNum_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenSmsWritePhoneNumFind_screen_init(lv_obj_t * win,void * p_param);
    void ui_ScreenSmsWritePhoneNumFind_screen_uninit(lv_obj_t * win,void * p_param);
    lv_obj_t * ui_ScreenSmsSendOptionFind_screen_init(lv_obj_t * win,void * p_param);
    void ui_ScreenSmsSendOptionFind_screen_uninit(lv_obj_t * win,void * p_param);
    lv_obj_t * ui_ScreenSmsSelectPhoneNum_screen_init(lv_obj_t * win,void * p_param);
    void ui_ScreenSmsSelectPhoneNum_screen_uninit(lv_obj_t * win,void * p_param);
    lv_obj_t * ui_ScreenSmsInsertOption_screen_init(lv_obj_t * win,void * p_param);
    void ui_ScreenSmsInsertOption_screen_uninit(lv_obj_t * win,void * p_param);
    lv_obj_t * ui_ScreenSmsInsertWord_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenSmsInsertWord_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenSmsInsertWord_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenSmsInsertNum_screen_init(lv_obj_t * win,void * enter_param);
    void ui_ScreenSmsInsertNum_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenSmsInsertNum_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenSmsListRev_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsListRev_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsListRev_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsListSend_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsListSend_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsListSend_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsListDraft_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsListDraft_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsListDraft_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsListDel_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenSmsListDel_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsDetailRev_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsDetailRev_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsDetailRev_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsDetailRevOption_screen_init(lv_obj_t * win,void * p_ui_detail);
    void ui_ScreenSmsDetailRevOption_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsDetailRevOptionDel_screen_init(lv_obj_t * win,
        sms_detail_ui_enter_param_t * p_enter_param);
    void ui_ScreenSmsDetailRevOptionDel_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsDetailRevOptionInf_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenSmsDetailRevOptionInf_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsDetailSend_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsDetailSend_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsDetailSend_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsDetailSendOption_screen_init(lv_obj_t * win,void * p_ui_detail);
    void ui_ScreenSmsDetailSendOption_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsDetailSendOptionDel_screen_init(lv_obj_t * win,
        sms_detail_ui_enter_param_t * p_enter_param);
    void ui_ScreenSmsDetailSendOptionDel_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsDetailSendOptionInf_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenSmsDetailSendOptionInf_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsDetailDraft_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsDetailDraft_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsDetailDraft_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsDetailDraftOption_screen_init(lv_obj_t * win,void * p_ui_detail);
    void ui_ScreenSmsDetailDraftOption_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsDetailDraftOptionDel_screen_init(lv_obj_t * win,
        sms_detail_ui_enter_param_t * p_enter_param);
    void ui_ScreenSmsDetailDraftOptionDel_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsDelAllRead_screen_init(lv_obj_t * win,void * p_enter_param);
    void ui_ScreenSmsDelAllRead_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsDelRevAll_screen_init(lv_obj_t * win,void * p_enter_param);
    void ui_ScreenSmsDelRevAll_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsDelRevRead_screen_init(lv_obj_t * win,void * p_enter_param);
    void ui_ScreenSmsDelRevRead_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsDelDraftRead_screen_init(lv_obj_t * win,void * p_enter_param);
    void ui_ScreenSmsDelDraftRead_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsDelSendRead_screen_init(lv_obj_t * win,void * p_enter_param);
    void ui_ScreenSmsDelSendRead_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsListDelRev_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenSmsListDelRev_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsListDelRevSelect_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsListDelRevSelect_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsListDelRevSelect_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsDelSelect_screen_init(lv_obj_t * win,void * p_enter_param);
    void ui_ScreenSmsDelSelect_screen_uninit(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
    lv_obj_t * ui_ScreenSmsListDelSend_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenSmsListDelSend_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsListDelDraft_screen_init(lv_obj_t * win,void * p);
    void ui_ScreenSmsListDelDraft_screen_uninit(lv_obj_t * win,void * p);
    lv_obj_t * ui_ScreenSmsListDelDraftSelect_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsListDelDraftSelect_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsListDelDraftSelect_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsListDelSendSelect_screen_active(lv_obj_t * win,void * param);
    void ui_ScreenSmsListDelSendSelect_screen_deactive(lv_obj_t * win,void * param);
    void ui_ScreenSmsListDelSendSelect_win_focus(lv_obj_t * win,void * param);
    lv_obj_t * ui_ScreenSmsExtractNumber_screen_init(lv_obj_t * win,sms_extract_number_ui_enter_param_t * enter_param);
    void ui_ScreenSmsExtractNumber_screen_uninit(lv_obj_t * win,void * p_enter_param);
    lv_obj_t * ui_ScreenSmsExtractNumEdit_screen_init(lv_obj_t * win,void * p_number);
    void ui_ScreenSmsExtractNumEdit_screen_uninit(lv_obj_t * win,void * enter_param);
    void ui_ScreenSmsExtractNumEdit_screen_focus(lv_obj_t * win,void * enter_param);
    lv_obj_t * ui_ScreenSmsExtractNumOption_screen_init(lv_obj_t * win,void * p_number);
    void ui_ScreenSmsExtractNumOption_screen_uninit(lv_obj_t * win,void * p_param);
    sms_detail_ui_enter_param_t * get_ui_ScreenSmsDetailDraft_enter_param(lv_obj_t * ui_Screen);
    uint8_t * get_ui_ScreenSmsDetailDraft_content(lv_obj_t * ui_Screen);
    uint8_t * get_ui_ScreenSmsDetailRev_filename(lv_obj_t * ui_Screen);
    uint8_t get_ui_ScreenSmsDetailRev_card_id(lv_obj_t * ui_Screen);
    uint8_t * get_ui_ScreenSmsDetailSend_number(lv_obj_t * ui_Screen);
    uint8_t * get_ui_ScreenSmsDetailSend_filename(lv_obj_t * ui_Screen);
    uint8_t get_ui_ScreenSmsDetailSend_card_id(lv_obj_t * ui_Screen);
    sms_detail_ui_enter_param_t * get_ui_ScreenSmsDetailSend_enter_param(lv_obj_t * ui_Screen);
    uint8_t * get_ui_ScreenSmsDetailSend_content(lv_obj_t * ui_Screen);
    void sms_send_select_phone_remind_screen(lv_obj_t * ui_Screen);
    void sms_menu_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_write_pinyin_create(lv_obj_t * ui_Screen);
    void sms_write_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_write_option_key_event(uint32_t key,lv_obj_t * ui_ScreenSmsWriteOption);
    void sms_write_phonenum_pinyin_create(lv_obj_t * ui_Screen);
    void sms_write_phonenum_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_rev_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_send_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_draft_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_rev_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_send_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_write_phone_num_find_select_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_send_option_select_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_select_phone_num_select_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_insert_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_insert_word_pinyin_create(lv_obj_t * ui_Screen);
    void sms_insert_word_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_insert_number_pinyin_create(lv_obj_t * ui_Screen);
    void sms_insert_number_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_rev_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_rev_option_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_rev_option_inf_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_send_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_send_option_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_send_option_inf_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_draft_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_draft_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_detail_draft_option_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_del_all_read_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_del_rev_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_del_rev_select_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_del_select_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_del_send_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_del_draft_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_del_draft_select_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_list_del_send_select_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_del_rev_all_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_del_rev_read_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_del_draft_read_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_del_send_read_del_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_extract_number_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_extract_num_edit_pinyin_create(lv_obj_t * ui_Screen);
    void sms_extract_num_edit_key_event(uint32_t key,lv_obj_t * ui_Screen);
    void sms_extract_num_option_key_event(uint32_t key,lv_obj_t * ui_Screen);
    int32_t sms_menu_remind_box_empty(uint8_t child_id);
    uint8_t * get_ui_ScreenSmsDetailRev_name(lv_obj_t * ui_Screen);
    uint8_t * get_ui_ScreenSmsDetailSend_name(lv_obj_t * ui_Screen);

#ifdef __cplusplus
} /*extern "C"*/

#endif

#endif

