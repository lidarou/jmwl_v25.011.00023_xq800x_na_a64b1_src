
#include "../ui.h"
#include "app_log.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "alv_app_mgr.h"
#include "app_call_list.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI

#define UI_SMS_INSERT_STR_MAX   42

typedef struct ui_screen_idle_num_wirte_obj_h {
    lv_obj_t *      label_back;
    lv_obj_t *      obj_textarea;
    pinyin_param_t * pinyin;

    uint32_t        pre_count;

    idle_num_wirte_ui_enter_param_t ui_enter_param;

} ui_screen_idle_num_wirte_t;

lv_obj_t * ui_ScreenIdleNumWirte_screen_init(lv_obj_t * win,void * param)
{
    lv_obj_t *      ui_ScreenIdleNumWirte = NULL;

    ui_screen_idle_num_wirte_t * p_screen_param;
    lv_obj_t *      obj;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_idle_num_wirte_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_idle_num_wirte_t));

    if (param) {
        memcpy(&p_screen_param->ui_enter_param,param,sizeof(idle_num_wirte_ui_enter_param_t));
        }

    ui_ScreenIdleNumWirte = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenIdleNumWirte,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenIdleNumWirte,(lv_style_t *) get_lv_style_font_common(),0);



    obj                 = lv_label_create(ui_ScreenIdleNumWirte);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"选项");

    obj                 = lv_label_create(ui_ScreenIdleNumWirte);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,162);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"清除");
    p_screen_param->label_back = obj;


    obj                 = lv_textarea_create(ui_ScreenIdleNumWirte);
    lv_obj_set_x(obj,0);
    lv_obj_set_y(obj,0);
    lv_obj_set_width(obj,LV_PCT(100) - 2);          /// 1
    lv_obj_set_height(obj,270);                     /// 1

    //lv_textarea_set_one_line(obj, true);
    lv_obj_set_style_pad_left(obj,1,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_border_width(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_textarea_set_max_byte(obj,IDLE_NUM_WIRTE_MAX_LEN);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font52_common(),0);

    if (param)
        lv_textarea_add_text(obj,p_screen_param->ui_enter_param.number);

    lv_obj_add_state(obj,LV_STATE_FOCUSED);
    p_screen_param->obj_textarea = obj;


    p_screen_param->pinyin = NULL;

    lv_obj_set_user_data(ui_ScreenIdleNumWirte,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenIdleNumWirte,fp_idle_num_write_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenIdleNumWirte);

    lv_obj_set_user_data(win,ui_ScreenIdleNumWirte);

    return ui_ScreenIdleNumWirte;
}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_idle_num_wirte_t * p_screen_param;

    p_screen_param      = (ui_screen_idle_num_wirte_t *) lv_obj_get_user_data(a->var);
    ime_pinyin_keyboard_destroy(p_screen_param->pinyin);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenIdleNumWirte_screen_uninit(lv_obj_t * win,void * param)
{
    lv_obj_t *      ui_ScreenIdleNumWirte = NULL;

    ui_ScreenIdleNumWirte = lv_obj_get_user_data(win);

    if (ui_ScreenIdleNumWirte) {
        lv_obj_del_delayed_callback(ui_ScreenIdleNumWirte,300,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}


void ui_ScreenIdleNumWirte_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenIdleNumWirte;
    ui_screen_idle_num_wirte_t * p_screen_param;

    ui_ScreenIdleNumWirte = lv_obj_get_user_data(win);
    p_screen_param      = (ui_screen_idle_num_wirte_t *) lv_obj_get_user_data(ui_ScreenIdleNumWirte);


    lv_disp_load_scr(ui_ScreenIdleNumWirte);

    {
        lv_obj_t *      label;
        uint32_t        count;
        uint8_t         str_buf[4],*text;

        label               = lv_textarea_get_label(p_screen_param->obj_textarea);
        count               = _lv_txt_get_encoded_length(lv_label_get_text(label));
        text                = (uint8_t *) lv_textarea_get_text(p_screen_param->obj_textarea);

        memset(str_buf,0,4);

        if ((count > 0) && (p_screen_param->pre_count < count)) {
            p_screen_param->pre_count = count;
            str_buf[0]          = text[count - 1];
            app_fp_play_tts_key(str_buf);
            }
        }

}


void ui_ScreenIdleNumWirte_screen_defocus(lv_obj_t * win,void * enter_param)
{

}


void idle_num_wirte_pinyin_create(lv_obj_t * ui_Screen)
{
    ui_screen_idle_num_wirte_t * p_screen_param;

    p_screen_param      = (ui_screen_idle_num_wirte_t *) lv_obj_get_user_data(ui_Screen);

    if (p_screen_param->pinyin)
        return;

    p_screen_param->pinyin = ime_pinyin_keyboard_create();

    if (p_screen_param->pinyin) {
        p_screen_param->pinyin->ta_user = p_screen_param->obj_textarea;
        ime_pinyin_ui_set_fortmat(p_screen_param->pinyin,format_num);
        }
}

/*
int32_t idle_num_wirte_screen_exit_fun(void *p_param, uint32_t size)
{
    alv_am_win_exit_action_by_app("idle_fp", "idle_num_write", NULL);
}
*/
static void idle_num_wirte_screen_exit_fun(lv_anim_t * a)
{

    alv_am_win_exit_action_by_app("idle_fp","idle_num_write",NULL);
}

void idle_num_wirte_key_event(uint32_t key,lv_obj_t * ui_Screen)
{
    ui_screen_idle_num_wirte_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    p_screen_param      = (ui_screen_idle_num_wirte_t *) lv_obj_get_user_data(ui_Screen);

    if (get_ime_pinyin_keyboard_active_state(p_screen_param->pinyin)) {

        ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
        label               = lv_textarea_get_label(p_screen_param->obj_textarea);
        count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

        if (count == 0) {
            alv_am_win_exit_current(NULL);
            }
        else {
            uint8_t         str_buf[4],*text;

            text                = (uint8_t *) lv_textarea_get_text(p_screen_param->obj_textarea);

            memset(str_buf,0,4);

            if ((count > 0) && (p_screen_param->pre_count < count)) {
                p_screen_param->pre_count = count;
                str_buf[0]          = text[count - 1];
                app_fp_play_tts_key(str_buf);
                }
            }
        }
    else {
        switch (key)
        {
            case APP_KEY_VALUE_TOP_LEFT:
                {
                    uint8_t *       number;
                    idle_num_wirte_ui_enter_param_t enter_param;

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    number              = lv_label_get_text(label);
                    memset(&enter_param,0,sizeof(idle_num_wirte_ui_enter_param_t));
                    strncpy(enter_param.number,number,IDLE_NUM_WIRTE_MAX_LEN);
                    alv_am_win_enter(NULL,"idle_num_option",(void *) &enter_param);
                    }
                break;

            case APP_KEY_VALUE_TOP_RIGHT:
                label = lv_textarea_get_label(p_screen_param->obj_textarea);
                count = _lv_txt_get_encoded_length(lv_label_get_text(label));

                if (count == 0)
                    alv_am_win_exit_current(NULL);
                else {
                    lv_textarea_del_char(p_screen_param->obj_textarea);
                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));
                    p_screen_param->pre_count = count;

                    if (count == 0) {
                        lv_label_set_text(p_screen_param->label_back,"返回");
                        }
                    }

                break;

            case LV_KEY_HOME:
#ifdef CONFIG_APP_CALL_FP
                {
                    uint8_t *       number;
                    int32_t ret;

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    number              = lv_label_get_text(label);
                    ret = call_dial_number(number,NULL);

                    lv_obj_del_delayed_callback(NULL,200,idle_num_wirte_screen_exit_fun);
                    }
#endif

                break;

            case LV_KEY_END:
                alv_am_app_exit_action_all();
                break;

            default:
                ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
                {
                    uint8_t         str_buf[4],*text;

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));
                    text                = (uint8_t *) lv_textarea_get_text(p_screen_param->obj_textarea);

                    memset(str_buf,0,4);

                    if ((count > 0) && (p_screen_param->pre_count < count)) {
                        p_screen_param->pre_count = count;
                        str_buf[0]          = text[count - 1];
                        app_fp_play_tts_key(str_buf);
                        }
                    }
                break;
            }
        }

    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

    if (count) {
        lv_label_set_text(p_screen_param->label_back,"清除");
        }

}


void idle_num_wirte_key_event_state(uint32_t key,uint32_t code,lv_obj_t * ui_Screen)
{
    ui_screen_idle_num_wirte_t * p_screen_param;

    p_screen_param      = (ui_screen_idle_num_wirte_t *) lv_obj_get_user_data(ui_Screen);

    key_update_pressed(key,code);


    switch (key)
    {
        case LV_KEY_DEL:
            {
                uint32_t        flag;


                flag                = key_get_pressed_flag(key);

                if ((1 << LV_EVENT_LONG_PRESSED_REPEAT) &flag) {
                    key_update_release(key,LV_EVENT_RELEASED);
                    }
                else if ((1 << LV_EVENT_LONG_PRESSED) &flag) {

                    lv_obj_t *      label = lv_textarea_get_label(p_screen_param->obj_textarea);
                    uint8_t         count = _lv_txt_get_encoded_length(lv_label_get_text(label));

                    while (count) {
                        lv_textarea_del_char(p_screen_param->obj_textarea);
                        count               -= 1;
                        }

                    p_screen_param->pre_count = count;
                    lv_label_set_text(p_screen_param->label_back,"返回");

                    key_update_release(key,LV_EVENT_RELEASED);
                    }
                else if ((1 << LV_EVENT_RELEASED) &flag) {
                    key_update_release(key,LV_EVENT_RELEASED);
                    }

                }
            break;
        }

    if (code == LV_EVENT_RELEASED)
        key_update_release(key,LV_EVENT_RELEASED);

}



