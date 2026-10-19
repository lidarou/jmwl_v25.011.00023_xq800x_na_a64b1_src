
#include "../ui.h"
#include "app_log.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "blacklist.h"
#include "app_blacklist.h"
#include "alv_app_mgr.h"
#include <stdio.h>

#define __inf                   APP_LOGI

#define UI_SMS_INSERT_STR_MAX   42

typedef struct ui_screen_blacklist_add_num_obj_h {
    lv_obj_t *      label_ok;
    lv_obj_t *      label_back;
    lv_obj_t *      label_input_format;
    lv_obj_t *      label_remain_num;
    lv_style_t      style_remain_num;
    lv_obj_t *      obj_textarea;
    pinyin_param_t * pinyin;

    uint8_t         str_remain_num[5];
    uint32_t        wirte_count;

    ui_screen_blacklist_enter_param_t enter_param;

    uint8_t         insert_str[BLACKLIST_NUMBER_LEN + 2];
} ui_screen_blacklist_add_num_t;


#define UI_BLACKLIST_NUM_MAX    30


lv_obj_t * ui_ScreenBlacklistAddNum_screen_init(lv_obj_t * win,ui_screen_blacklist_enter_param_t * p_enter_param)
{
    lv_obj_t *      ui_ScreenBlacklistAddNum = NULL;

    ui_screen_blacklist_add_num_t * p_screen_param;
    lv_obj_t *      obj;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_blacklist_add_num_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_blacklist_add_num_t));


    if (p_enter_param)
        memcpy(&p_screen_param->enter_param,p_enter_param,sizeof(ui_screen_blacklist_enter_param_t));

    ui_ScreenBlacklistAddNum = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenBlacklistAddNum,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenBlacklistAddNum,(lv_style_t *) get_lv_style_font_common(),0);


    obj                 = lv_label_create(ui_ScreenBlacklistAddNum);
    lv_obj_set_width(obj,100);
    lv_obj_set_height(obj,20);
    lv_obj_set_x(obj,0);
    lv_obj_set_y(obj,0);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);
    lv_label_set_text(obj,"123");
    p_screen_param->label_input_format = obj;

    obj                 = lv_label_create(ui_ScreenBlacklistAddNum);
    lv_obj_set_width(obj,30);
    lv_obj_set_height(obj,40);
    lv_obj_set_x(obj,210);
    lv_obj_set_y(obj,0);
    //lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);
    sprintf(p_screen_param->str_remain_num,"%d",UI_BLACKLIST_NUM_MAX);
    lv_label_set_text(obj,p_screen_param->str_remain_num);
    p_screen_param->label_remain_num = obj;

    lv_style_init(&p_screen_param->style_remain_num);
    lv_style_set_text_font(&p_screen_param->style_remain_num,(lv_font_t *)get_ft_info_font_custom(20));
    lv_style_set_text_align(&p_screen_param->style_remain_num,LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_color(&p_screen_param->style_remain_num,lv_color_hex(0xFF));
    lv_obj_add_style(p_screen_param->label_remain_num,&p_screen_param->style_remain_num,0);

    obj                 = lv_label_create(ui_ScreenBlacklistAddNum);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,50);
    lv_label_set_text(obj,"电话号码");

    obj                 = lv_label_create(ui_ScreenBlacklistAddNum);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"查找");
    p_screen_param->label_ok = obj;

    obj                 = lv_label_create(ui_ScreenBlacklistAddNum);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,162);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"返回");
    p_screen_param->label_back = obj;


    obj                 = lv_textarea_create(ui_ScreenBlacklistAddNum);
    lv_obj_set_x(obj,0);
    lv_obj_set_y(obj,100);
    lv_obj_set_width(obj,LV_PCT(100));              /// 1
    lv_obj_set_height(obj,170);                     /// 1

    //lv_textarea_set_one_line(obj, true);
    lv_obj_set_style_pad_left(obj,1,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_border_width(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_textarea_set_max_byte(obj,UI_BLACKLIST_NUM_MAX);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_state(obj,LV_STATE_FOCUSED);
    p_screen_param->obj_textarea = obj;


    p_screen_param->pinyin = NULL;

    lv_obj_set_user_data(ui_ScreenBlacklistAddNum,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenBlacklistAddNum,fp_blacklist_add_num_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenBlacklistAddNum);

    lv_obj_set_user_data(win,ui_ScreenBlacklistAddNum);

    return ui_ScreenBlacklistAddNum;
}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_blacklist_add_num_t * p_screen_param;

    p_screen_param      = (ui_screen_blacklist_add_num_t *) lv_obj_get_user_data(a->var);
    ime_pinyin_keyboard_destroy(p_screen_param->pinyin);
    lv_style_reset(&p_screen_param->style_remain_num);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenBlacklistAddNum_screen_uninit(lv_obj_t * win,void * param)
{
    lv_obj_t *      ui_ScreenBlacklistAddNum = NULL;

    ui_ScreenBlacklistAddNum = lv_obj_get_user_data(win);

    if (ui_ScreenBlacklistAddNum) {
        lv_obj_del_delayed_callback(ui_ScreenBlacklistAddNum,300,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}


void ui_ScreenBlacklistAddNum_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenBlacklistAddNum;
    ui_screen_blacklist_add_num_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    ui_ScreenBlacklistAddNum = lv_obj_get_user_data(win);
    p_screen_param      = (ui_screen_blacklist_add_num_t *) lv_obj_get_user_data(ui_ScreenBlacklistAddNum);


    if (strlen(p_screen_param->insert_str)) {
        lv_textarea_add_text(p_screen_param->obj_textarea,p_screen_param->insert_str);
        memset(p_screen_param->insert_str,0,BLACKLIST_NUMBER_LEN + 2);
        }

    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

    if (p_screen_param->wirte_count != count) {
        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_BLACKLIST_NUM_MAX - count));
        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

        if (p_screen_param->wirte_count == 0) {
            lv_label_set_text(p_screen_param->label_ok,"确定");
            lv_label_set_text(p_screen_param->label_back,"清除");
            }
        else if (count == 0) {
            lv_label_set_text(p_screen_param->label_ok,"查找");
            lv_label_set_text(p_screen_param->label_back,"返回");
            }

        p_screen_param->wirte_count = count;
        }


    lv_disp_load_scr(ui_ScreenBlacklistAddNum);

}


void ui_ScreenBlacklistAddNum_screen_defocus(lv_obj_t * win,void * enter_param)
{

}


void blacklist_add_num_pinyin_create(lv_obj_t * ui_Screen)
{
    ui_screen_blacklist_add_num_t * p_screen_param;

    p_screen_param      = (ui_screen_blacklist_add_num_t *) lv_obj_get_user_data(ui_Screen);

    if (p_screen_param->pinyin)
        return;

    p_screen_param->pinyin = ime_pinyin_keyboard_create();

    if (p_screen_param->pinyin) {
        p_screen_param->pinyin->ta_user = p_screen_param->obj_textarea;
        p_screen_param->pinyin->label_format = p_screen_param->label_input_format;
        ime_pinyin_ui_set_fortmat(p_screen_param->pinyin,format_num);
        }
}


static void remind_timer_callback_ui_return(screen_remind_param_t * p_param)
{

    alv_am_win_exit_action(NULL,"option",NULL);

    alv_am_win_exit_action(NULL,"list",NULL);
    alv_am_win_jump_action(NULL,"list",NULL);


}


static void blacklist_add_num_to_flash(lv_obj_t * ui_Screen)
{
    lv_obj_t *      label;
    blacklist_param_t blacklist_param;
    ui_screen_blacklist_add_num_t * p_screen_param;
    char *          num;

    p_screen_param      = (ui_screen_blacklist_add_num_t *) lv_obj_get_user_data(ui_Screen);
    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
    num                 = lv_label_get_text(label);

    memset(&blacklist_param,0,sizeof(blacklist_param));
    strncpy(blacklist_param.number,num,BLACKLIST_NUMBER_LEN);

    blacklist_add_flash_data(&blacklist_param,sizeof(blacklist_param));
}


static void blacklist_add_num_remind_screen(lv_obj_t * ui_Screen)
{
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;

    ui_screen_remind_param_init(&screen_remind_param);
    screen_remind_param.str = "号码已加入列表";

    screen_remind_param.tiemr_delay = 1500;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_ui_return;
    screen_remind_param.user_data = ui_Screen;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);


    blacklist_add_num_to_flash(ui_Screen);

}




void blacklist_add_num_key_event(uint32_t key,lv_obj_t * ui_Screen)
{
    ui_screen_blacklist_add_num_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    p_screen_param      = (ui_screen_blacklist_add_num_t *) lv_obj_get_user_data(ui_Screen);

    if (get_ime_pinyin_keyboard_active_state(p_screen_param->pinyin)) {

        ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
        label               = lv_textarea_get_label(p_screen_param->obj_textarea);
        count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

        if (p_screen_param->wirte_count != count) {
            sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_BLACKLIST_NUM_MAX - count));
            lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

            if (p_screen_param->wirte_count == 0) {
                lv_label_set_text(p_screen_param->label_ok,"确定");
                lv_label_set_text(p_screen_param->label_back,"清除");
                }
            else if (count == 0) {
                lv_label_set_text(p_screen_param->label_ok,"查找");
                lv_label_set_text(p_screen_param->label_back,"返回");
                }

            p_screen_param->wirte_count = count;
            }
        }
    else {
        switch (key)
        {
            case LV_KEY_ENTER:
            case APP_KEY_VALUE_TOP_LEFT:
                if (p_screen_param->wirte_count) {
                    blacklist_add_num_remind_screen(ui_Screen);
                    }
                else {
                    ui_screen_blacklist_find_num_enter_param_t enter_param;

                    enter_param.p_number = p_screen_param->insert_str;
                    enter_param.number_size = BLACKLIST_NUMBER_LEN;
                    alv_am_win_enter(NULL,"find",&enter_param);
                    }

                break;

            case APP_KEY_VALUE_TOP_RIGHT:
                if (p_screen_param->wirte_count) {
                    lv_textarea_del_char(p_screen_param->obj_textarea);

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

                    if (p_screen_param->wirte_count != count) {
                        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_BLACKLIST_NUM_MAX - count));
                        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                        if (p_screen_param->wirte_count == 0) {
                            lv_label_set_text(p_screen_param->label_ok,"确定");
                            lv_label_set_text(p_screen_param->label_back,"清除");
                            }
                        else if (count == 0) {
                            lv_label_set_text(p_screen_param->label_ok,"查找");
                            lv_label_set_text(p_screen_param->label_back,"返回");
                            }

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
                ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
                label = lv_textarea_get_label(p_screen_param->obj_textarea);
                count = _lv_txt_get_encoded_length(lv_label_get_text(label));

                if (p_screen_param->wirte_count != count) {
                    sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_BLACKLIST_NUM_MAX - count));
                    lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                    if (p_screen_param->wirte_count == 0) {
                        lv_label_set_text(p_screen_param->label_ok,"确定");
                        lv_label_set_text(p_screen_param->label_back,"清除");
                        }
                    else if (count == 0) {
                        lv_label_set_text(p_screen_param->label_ok,"查找");
                        lv_label_set_text(p_screen_param->label_back,"返回");
                        }

                    p_screen_param->wirte_count = count;
                    }

                break;
            }
        }


}



