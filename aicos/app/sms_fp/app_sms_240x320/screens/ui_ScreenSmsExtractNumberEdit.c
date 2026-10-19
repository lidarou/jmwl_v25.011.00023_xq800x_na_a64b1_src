
#include "../ui.h"
#include "app_log.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "alv_app_mgr.h"

#define __inf                   APP_LOGI


typedef struct ui_screen_sms_extract_num_edit_obj_h {
    lv_obj_t *      label_back;
    lv_obj_t *      obj_textarea;
    pinyin_param_t * pinyin;

    uint32_t        wirte_count;
    uint8_t         number[TS_SMS_NUM_MAX_LEN + 1];
} ui_screen_sms_extract_num_edit_t;


#define UI_SMS_EXTRACT_NUM_EDIT_COUNT_MAX 20


lv_obj_t * ui_ScreenSmsExtractNumEdit_screen_init(lv_obj_t * win,void * p_number)
{
    lv_obj_t *      ui_ScreenSmsExtractNumEdit = NULL;

    ui_screen_sms_extract_num_edit_t * p_screen_param;
    lv_obj_t *      obj;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_sms_extract_num_edit_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_sms_extract_num_edit_t));

    if (p_number)
        strncpy(p_screen_param->number,p_number,UI_SMS_EXTRACT_NUM_EDIT_COUNT_MAX);


    ui_ScreenSmsExtractNumEdit = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenSmsExtractNumEdit,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenSmsExtractNumEdit,(lv_style_t *) get_lv_style_font_common(),0);




    obj                 = lv_label_create(ui_ScreenSmsExtractNumEdit);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"选项");

    obj                 = lv_label_create(ui_ScreenSmsExtractNumEdit);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,162);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"清除");
    p_screen_param->label_back = obj;




    obj                 = lv_textarea_create(ui_ScreenSmsExtractNumEdit);
    lv_obj_set_x(obj,0);
    lv_obj_set_y(obj,0);
    lv_obj_set_width(obj,240);                      /// 1
    lv_obj_set_height(obj,280);                     /// 1

    //lv_textarea_set_one_line(obj, true);
    lv_obj_set_style_pad_left(obj,1,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_border_width(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_textarea_set_max_byte(obj,UI_SMS_EXTRACT_NUM_EDIT_COUNT_MAX);

    //lv_textarea_add_text(obj,p_screen_param->number);
    lv_obj_add_state(obj,LV_STATE_FOCUSED);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font52_common(),0);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLLABLE);


    p_screen_param->obj_textarea = obj;



    p_screen_param->pinyin = NULL;

    lv_obj_set_user_data(ui_ScreenSmsExtractNumEdit,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenSmsExtractNumEdit,fp_sms_extract_num_edit_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenSmsExtractNumEdit);

    lv_obj_set_user_data(win,ui_ScreenSmsExtractNumEdit);


    return ui_ScreenSmsExtractNumEdit;
}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_sms_extract_num_edit_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_extract_num_edit_t *) lv_obj_get_user_data(a->var);
    ime_pinyin_keyboard_destroy(p_screen_param->pinyin);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenSmsExtractNumEdit_screen_uninit(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsExtractNumEdit = NULL;

    ui_ScreenSmsExtractNumEdit = lv_obj_get_user_data(win);

    if (ui_ScreenSmsExtractNumEdit) {
        lv_obj_del_delayed_callback(ui_ScreenSmsExtractNumEdit,500,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}


void ui_ScreenSmsExtractNumEdit_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsExtractNumEdit;
    ui_screen_sms_extract_num_edit_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    ui_ScreenSmsExtractNumEdit = lv_obj_get_user_data(win);
    p_screen_param      = (ui_screen_sms_extract_num_edit_t *) lv_obj_get_user_data(ui_ScreenSmsExtractNumEdit);

    if (strlen(p_screen_param->number)) {
        lv_textarea_set_text(p_screen_param->obj_textarea,"a");
        lv_textarea_del_char(p_screen_param->obj_textarea);
        lv_textarea_add_text(p_screen_param->obj_textarea,p_screen_param->number);
        memset(p_screen_param->number,0,TS_SMS_NUM_MAX_LEN);
        }

    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

    if (count == 0)
        lv_label_set_text(p_screen_param->label_back,"返回");
    else 
        lv_label_set_text(p_screen_param->label_back,"清除");

    p_screen_param->wirte_count = count;


    lv_disp_load_scr(ui_ScreenSmsExtractNumEdit);

}


void sms_extract_num_edit_pinyin_create(lv_obj_t * ui_Screen)
{
    ui_screen_sms_extract_num_edit_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_extract_num_edit_t *) lv_obj_get_user_data(ui_Screen);

    if (p_screen_param->pinyin)
        return;

    p_screen_param->pinyin = ime_pinyin_keyboard_create();

    if (p_screen_param->pinyin) {
        p_screen_param->pinyin->ta_user = p_screen_param->obj_textarea;

        //p_screen_param->pinyin->label_format = p_screen_param->label_input_format;
        ime_pinyin_ui_set_fortmat(p_screen_param->pinyin,format_num);
        }
}




void sms_extract_num_edit_key_event(uint32_t key,lv_obj_t * ui_Screen)
{
    ui_screen_sms_extract_num_edit_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    p_screen_param      = (ui_screen_sms_extract_num_edit_t *) lv_obj_get_user_data(ui_Screen);

    if (get_ime_pinyin_keyboard_active_state(p_screen_param->pinyin)) {

        ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
        label               = lv_textarea_get_label(p_screen_param->obj_textarea);
        count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

        if (p_screen_param->wirte_count != count) {
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
            case APP_KEY_VALUE_TOP_LEFT:
                label = lv_textarea_get_label(p_screen_param->obj_textarea);
                alv_am_win_enter(NULL,"extract_num_option",lv_label_get_text(label));
                break;

            case APP_KEY_VALUE_TOP_RIGHT:
                if (p_screen_param->wirte_count) {
                    lv_textarea_del_char(p_screen_param->obj_textarea);

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

                    if (p_screen_param->wirte_count != count) {

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

            case LV_KEY_ENTER:
                {
                    if (p_screen_param->wirte_count == 0) {

                        }
                    else {
                        }
                    }
                break;

            case LV_KEY_END:
                alv_am_app_exit_action_all();
                break;

            default:
                ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
                label = lv_textarea_get_label(p_screen_param->obj_textarea);
                count = _lv_txt_get_encoded_length(lv_label_get_text(label));

                if (p_screen_param->wirte_count != count) {

                    if (p_screen_param->wirte_count == 0)
                        lv_label_set_text(p_screen_param->label_back,"清除");
                    else if (count == 0)
                        lv_label_set_text(p_screen_param->label_back,"返回");


                    p_screen_param->wirte_count = count;
                    }

                break;
            }
        }


}





