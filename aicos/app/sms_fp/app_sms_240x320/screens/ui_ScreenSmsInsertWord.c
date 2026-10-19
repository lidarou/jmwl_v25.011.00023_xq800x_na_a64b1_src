
#include "../ui.h"
#include "app_log.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "tele_sms_data.h"
#include <stdio.h>
#include "alv_app_mgr.h"

#define __inf                   APP_LOGI


typedef struct ui_screen_sms_insert_word_obj_h {
    lv_obj_t *      label_input_format;
    lv_obj_t *      label_remain_num;
    lv_style_t      style_remain_num;
    lv_obj_t *      obj_textarea;
    pinyin_param_t * pinyin;

    uint8_t         str_remain_num[5];
    uint32_t        wirte_count;
    uint8_t         remind_str[30];
    sms_write_ui_enter_param_t * ui_enter_param;
} ui_screen_sms_insert_word_t;


#define UI_SMS_INTERT_WORD_COUNT_MAX 16


lv_obj_t * ui_ScreenSmsInsertWord_screen_init(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsInsertWord = NULL;

    ui_screen_sms_insert_word_t * p_screen_param;
    lv_obj_t *      obj;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_sms_insert_word_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_sms_insert_word_t));
    p_screen_param->label_remain_num = enter_param;


    ui_ScreenSmsInsertWord = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenSmsInsertWord,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenSmsInsertWord,(lv_style_t *) get_lv_style_font_common(),0);


    obj                 = lv_label_create(ui_ScreenSmsInsertWord);
    lv_obj_set_width(obj,100);
    lv_obj_set_height(obj,20);
    lv_obj_set_x(obj,0);
    lv_obj_set_y(obj,0);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);
    lv_label_set_text(obj,"拼音");
    p_screen_param->label_input_format = obj;

    obj                 = lv_label_create(ui_ScreenSmsInsertWord);
    lv_obj_set_width(obj,140);
    lv_obj_set_height(obj,40);
    lv_obj_set_x(obj,120);
    lv_obj_set_y(obj,0);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);

    //sprintf(p_screen_param->str_remain_num,"%d",UI_SMS_INTERT_WORD_COUNT_MAX);
    //lv_label_set_text(obj, p_screen_param->str_remain_num);
    p_screen_param->label_remain_num = obj;

    lv_style_init(&p_screen_param->style_remain_num);
    lv_style_set_text_font(&p_screen_param->style_remain_num,(lv_font_t *) get_ft_info_font_common());
    lv_style_set_text_align(&p_screen_param->style_remain_num,LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_color(&p_screen_param->style_remain_num,lv_color_hex(0xFF));
    lv_obj_add_style(p_screen_param->label_remain_num,&p_screen_param->style_remain_num,0);


    obj                 = lv_label_create(ui_ScreenSmsInsertWord);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"返回");

    obj                 = lv_label_create(ui_ScreenSmsInsertWord);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,162);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"清除");

    obj                 = lv_label_create(ui_ScreenSmsInsertWord);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,100);
    lv_obj_set_y(obj,290);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);
    lv_label_set_text(obj,"确定");



    obj                 = lv_textarea_create(ui_ScreenSmsInsertWord);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,50);
    lv_obj_set_width(obj,LV_PCT(100) - 2);          /// 1
    lv_obj_set_height(obj,170);                     /// 1

    //lv_textarea_set_one_line(obj, true);
    lv_obj_set_style_pad_left(obj,1,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_border_width(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_textarea_set_max_byte(obj,UI_SMS_INTERT_WORD_COUNT_MAX);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLLABLE);

    //lv_textarea_add_text(obj,p_screen_param->number);
    lv_obj_add_state(obj,LV_STATE_FOCUSED);
    p_screen_param->obj_textarea = obj;


    p_screen_param->ui_enter_param = enter_param;

    p_screen_param->pinyin = NULL;

    lv_obj_set_user_data(ui_ScreenSmsInsertWord,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenSmsInsertWord,fp_sms_insert_word_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenSmsInsertWord);

    lv_obj_set_user_data(win,ui_ScreenSmsInsertWord);


    return ui_ScreenSmsInsertWord;
}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_sms_insert_word_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_insert_word_t *) lv_obj_get_user_data(a->var);
    ime_pinyin_keyboard_destroy(p_screen_param->pinyin);
    lv_style_reset(&p_screen_param->style_remain_num);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenSmsInsertWord_screen_uninit(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsInsertWord = NULL;

    ui_ScreenSmsInsertWord = lv_obj_get_user_data(win);

    if (ui_ScreenSmsInsertWord) {
        lv_obj_del_delayed_callback(ui_ScreenSmsInsertWord,500,lv_obj_del_screen_ready_cb);
        tele_sms_data_set_send_fun_callback(0,0);
        }

    lv_obj_set_user_data(win,NULL);
}


void ui_ScreenSmsInsertWord_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsInsertWord;
    ui_screen_sms_insert_word_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    ui_ScreenSmsInsertWord = lv_obj_get_user_data(win);
    p_screen_param      = (ui_screen_sms_insert_word_t *) lv_obj_get_user_data(ui_ScreenSmsInsertWord);

    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
    count               = _lv_txt_encoded_get_byte_id(lv_label_get_text(label),UI_SMS_INTERT_WORD_COUNT_MAX);
    sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_INTERT_WORD_COUNT_MAX - count));
    lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

    p_screen_param->wirte_count = count;

    lv_disp_load_scr(ui_ScreenSmsInsertWord);

}


void sms_insert_word_pinyin_create(lv_obj_t * ui_Screen)
{
    ui_screen_sms_insert_word_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_insert_word_t *) lv_obj_get_user_data(ui_Screen);

    if (p_screen_param->pinyin)
        return;

    p_screen_param->pinyin = ime_pinyin_keyboard_create();

    if (p_screen_param->pinyin) {
        p_screen_param->pinyin->ta_user = p_screen_param->obj_textarea;
        p_screen_param->pinyin->label_format = p_screen_param->label_input_format;
        ime_pinyin_ui_set_fortmat(p_screen_param->pinyin,format_pinyin);
        ime_pinyin_ui_set_fortmat_flag_auto(p_screen_param->pinyin,1);
        }
}



void sms_insert_word_key_event(uint32_t key,lv_obj_t * ui_Screen)
{
    ui_screen_sms_insert_word_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    p_screen_param      = (ui_screen_sms_insert_word_t *) lv_obj_get_user_data(ui_Screen);

    if (get_ime_pinyin_keyboard_active_state(p_screen_param->pinyin)) {

        ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
        label               = lv_textarea_get_label(p_screen_param->obj_textarea);
        count               = _lv_txt_encoded_get_byte_id(lv_label_get_text(label),UI_SMS_INTERT_WORD_COUNT_MAX);

        if (p_screen_param->wirte_count != count) {
            sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_INTERT_WORD_COUNT_MAX - count));
            lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

            p_screen_param->wirte_count = count;
            }
        }
    else {
        switch (key)
        {
            case APP_KEY_VALUE_TOP_LEFT:
                alv_am_win_exit_current(NULL);
                break;

            case APP_KEY_VALUE_TOP_RIGHT:
                if (p_screen_param->wirte_count) {
                    lv_textarea_del_char(p_screen_param->obj_textarea);

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_encoded_get_byte_id(lv_label_get_text(label),UI_SMS_INTERT_WORD_COUNT_MAX);

                    if (p_screen_param->wirte_count != count) {
                        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_INTERT_WORD_COUNT_MAX - count));
                        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                        p_screen_param->wirte_count = count;
                        }
                    }

                break;

            case LV_KEY_ENTER:
                {
                    char *          str;

                    str                 = (char *) lv_textarea_get_text(p_screen_param->obj_textarea);

                    if (strlen(str))
                        strncpy(p_screen_param->ui_enter_param->p_insert_str,str,40);


                    alv_am_win_exit_action(NULL,"write_option",NULL);
                    alv_am_win_exit_action(NULL,"insert_option",NULL);
                    alv_am_win_exit_action(NULL,"insert_word",NULL);
                    }
                break;

            case LV_KEY_END:
                alv_am_app_exit_action_all();
                break;

            default:
                ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
                label = lv_textarea_get_label(p_screen_param->obj_textarea);
                count = _lv_txt_encoded_get_byte_id(lv_label_get_text(label),UI_SMS_INTERT_WORD_COUNT_MAX);

                if (p_screen_param->wirte_count != count) {
                    sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_INTERT_WORD_COUNT_MAX - count));
                    lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                    p_screen_param->wirte_count = count;
                    }

                break;
            }
        }


}





