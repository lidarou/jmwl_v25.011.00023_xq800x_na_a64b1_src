
#include "../ui.h"
#include "app_log.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "alv_app_mgr.h"

#define __inf                   APP_LOGI

#define UI_SMS_INSERT_STR_MAX   42

typedef struct ui_screen_sms_write_obj_h {
    lv_obj_t *      label_back;
    lv_obj_t *      label_input_format;
    lv_obj_t *      label_remain_num;
    lv_style_t      style_remain_num;
    lv_obj_t *      obj_textarea;
    pinyin_param_t * pinyin;

    uint8_t         str_remain_num[5];
    uint32_t        wirte_count;

    sms_write_ui_enter_param_t ui_enter_param;

    uint8_t         insert_str[UI_SMS_INSERT_STR_MAX];
    uint8_t         number[TS_SMS_NUM_MAX_LEN + 1];
} ui_screen_sms_write_t;

#define UI_SMS_WIRTE_COUNT_MAX  500


lv_obj_t * ui_ScreenSmsWrite_screen_init(lv_obj_t * win,void * param)
{
    lv_obj_t *      ui_ScreenSmsWrite = NULL;
    sms_write_ui_enter_param_t * p_param;

    ui_screen_sms_write_t * p_screen_param;
    lv_obj_t *      obj;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_sms_write_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_sms_write_t));

    p_param             = NULL;

    if (param) {
        p_param             = param;

        if (p_param->p_number)
            strncpy(p_screen_param->number,p_param->p_number,TS_SMS_NUM_MAX_LEN);
        }

    ui_ScreenSmsWrite   = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenSmsWrite,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenSmsWrite,(lv_style_t *) get_lv_style_font_common(),0);


    obj                 = lv_label_create(ui_ScreenSmsWrite);
    lv_obj_set_width(obj,100);
    lv_obj_set_height(obj,20);
    lv_obj_set_x(obj,0);
    lv_obj_set_y(obj,0);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);
    lv_label_set_text(obj,"拼音");
    p_screen_param->label_input_format = obj;

    obj                 = lv_label_create(ui_ScreenSmsWrite);
    lv_obj_set_width(obj,60);
    lv_obj_set_height(obj,20);
    lv_obj_set_x(obj,180);
    lv_obj_set_y(obj,0);
    sprintf(p_screen_param->str_remain_num,"%d",UI_SMS_WIRTE_COUNT_MAX);
    lv_label_set_text(obj,p_screen_param->str_remain_num);

    p_screen_param->label_remain_num = obj;

    lv_style_init(&p_screen_param->style_remain_num);
    lv_style_set_text_font(&p_screen_param->style_remain_num,(const lv_font_t *) get_ft_info_font_custom(20));
    lv_style_set_text_align(&p_screen_param->style_remain_num,LV_TEXT_ALIGN_RIGHT);
    lv_style_set_text_color(&p_screen_param->style_remain_num,lv_color_hex(0xFF));
    lv_obj_add_style(p_screen_param->label_remain_num,&p_screen_param->style_remain_num,0);


    obj                 = lv_label_create(ui_ScreenSmsWrite);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"选项");

    obj                 = lv_label_create(ui_ScreenSmsWrite);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,162);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"返回");
    p_screen_param->label_back = obj;


    obj                 = lv_textarea_create(ui_ScreenSmsWrite);
    lv_obj_set_x(obj,-1);
    lv_obj_set_y(obj,50);
    lv_obj_set_width(obj,242);                      /// 1
    lv_obj_set_height(obj,220);                     /// 1

    //lv_textarea_set_one_line(obj, true);
    lv_obj_set_style_pad_left(obj,1,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_border_width(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_textarea_set_max_length(obj,UI_SMS_WIRTE_COUNT_MAX);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLLABLE);

    if ((p_param) && (p_param->p_content))
        lv_textarea_add_text(obj,p_param->p_content);

    lv_obj_add_state(obj,LV_STATE_FOCUSED);
    p_screen_param->obj_textarea = obj;


    p_screen_param->pinyin = NULL;

    lv_obj_set_user_data(ui_ScreenSmsWrite,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenSmsWrite,fp_sms_Write_ui_event_actions,LV_EVENT_ALL,ui_ScreenSmsWrite);

    lv_obj_set_user_data(win,ui_ScreenSmsWrite);

    return ui_ScreenSmsWrite;
}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_sms_write_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_write_t *) lv_obj_get_user_data(a->var);
    ime_pinyin_keyboard_destroy(p_screen_param->pinyin);
    lv_style_reset(&p_screen_param->style_remain_num);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenSmsWrite_screen_uninit(lv_obj_t * win,void * param)
{
    lv_obj_t *      ui_ScreenSmsWrite = NULL;

    ui_ScreenSmsWrite   = lv_obj_get_user_data(win);

    if (ui_ScreenSmsWrite) {
        lv_obj_del_delayed_callback(ui_ScreenSmsWrite,300,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}


void ui_ScreenSmsWrite_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsWrite;
    ui_screen_sms_write_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    ui_ScreenSmsWrite   = lv_obj_get_user_data(win);
    p_screen_param      = (ui_screen_sms_write_t *) lv_obj_get_user_data(ui_ScreenSmsWrite);

    if (p_screen_param->ui_enter_param.clear_content_flag) {
        p_screen_param->ui_enter_param.clear_content_flag = 0;
        lv_textarea_set_text(p_screen_param->obj_textarea,"a");
        lv_textarea_del_char(p_screen_param->obj_textarea);

        }
    else {
        if (strlen(p_screen_param->insert_str))
            lv_textarea_add_text(p_screen_param->obj_textarea,p_screen_param->insert_str);

        memset(p_screen_param->insert_str,0,UI_SMS_INSERT_STR_MAX);
        }

    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));
    sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_WIRTE_COUNT_MAX - count));
    lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

    if (p_screen_param->wirte_count != count) {
        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_WIRTE_COUNT_MAX - count));
        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

        if (p_screen_param->wirte_count == 0)
            lv_label_set_text(p_screen_param->label_back,"清除");
        else if (count == 0)
            lv_label_set_text(p_screen_param->label_back,"返回");

        p_screen_param->wirte_count = count;
        }

    lv_disp_load_scr(ui_ScreenSmsWrite);

}


void ui_ScreenSmsWrite_screen_defocus(lv_obj_t * win,void * enter_param)
{

}


void sms_write_pinyin_create(lv_obj_t * ui_Screen)
{
    ui_screen_sms_write_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_write_t *) lv_obj_get_user_data(ui_Screen);

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



void sms_write_key_event(uint32_t key,lv_obj_t * ui_Screen)
{
    ui_screen_sms_write_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    p_screen_param      = (ui_screen_sms_write_t *) lv_obj_get_user_data(ui_Screen);

    if (get_ime_pinyin_keyboard_active_state(p_screen_param->pinyin)) {

        ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
        label               = lv_textarea_get_label(p_screen_param->obj_textarea);
        count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

        if (p_screen_param->wirte_count != count) {
            sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_WIRTE_COUNT_MAX - count));
            lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

            if (p_screen_param->wirte_count == 0)
                lv_label_set_text(p_screen_param->label_back,"清除");
            else if (count == 0)
                lv_label_set_text(p_screen_param->label_back,"返回");

            p_screen_param->wirte_count = count;
            }
        }
    else {
        switch (key)
        {
            case LV_KEY_ENTER:
            case APP_KEY_VALUE_TOP_LEFT:
                p_screen_param->ui_enter_param.p_content =
                     (uint8_t *) lv_textarea_get_text(p_screen_param->obj_textarea);
                p_screen_param->ui_enter_param.p_insert_str = p_screen_param->insert_str;
                p_screen_param->ui_enter_param.p_number = p_screen_param->number;
                alv_am_win_enter(NULL,"write_option",(void *) &p_screen_param->ui_enter_param);
                break;

            case APP_KEY_VALUE_TOP_RIGHT:
                if (p_screen_param->wirte_count) {
                    lv_textarea_del_char(p_screen_param->obj_textarea);

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

                    if (p_screen_param->wirte_count != count) {
                        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_WIRTE_COUNT_MAX - count));
                        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                        if (p_screen_param->wirte_count == 0)
                            lv_label_set_text(p_screen_param->label_back,"清除");
                        else if (count == 0)
                            lv_label_set_text(p_screen_param->label_back,"返回");

                        p_screen_param->wirte_count = count;
                        }
                    }
                else {
                    alv_am_win_exit_current(NULL);
                    }

                break;

            case LV_KEY_END:
                alv_am_app_exit_action_all();
                break;

            default:
                {
                    ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

                    if (p_screen_param->wirte_count != count) {
                        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_WIRTE_COUNT_MAX - count));
                        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                        if (p_screen_param->wirte_count == 0)
                            lv_label_set_text(p_screen_param->label_back,"清除");
                        else if (count == 0)
                            lv_label_set_text(p_screen_param->label_back,"返回");

                        p_screen_param->wirte_count = count;
                        }

                    }
                break;
            }
        }

    if (get_ime_pinyin_keyboard_active_state(p_screen_param->pinyin)) {
        if (140 != lv_obj_get_width(p_screen_param->obj_textarea)) {
            uint32_t        cursor_pos;

            lv_obj_set_height(p_screen_param->obj_textarea,140);
            lv_obj_invalidate(lv_scr_act());
            cursor_pos          = lv_textarea_get_cursor_pos(p_screen_param->obj_textarea);
            lv_textarea_set_cursor_pos(p_screen_param->obj_textarea,cursor_pos + 1);
            lv_textarea_set_cursor_pos(p_screen_param->obj_textarea,cursor_pos);
            }
        }
    else {
        if (220 != lv_obj_get_width(p_screen_param->obj_textarea)) {
            uint32_t        cursor_pos;

            lv_obj_set_height(p_screen_param->obj_textarea,220);
            lv_obj_invalidate(lv_scr_act());

            cursor_pos          = lv_textarea_get_cursor_pos(p_screen_param->obj_textarea);
            lv_textarea_set_cursor_pos(p_screen_param->obj_textarea,cursor_pos + 1);
            lv_textarea_set_cursor_pos(p_screen_param->obj_textarea,cursor_pos);
            }
        }

}



