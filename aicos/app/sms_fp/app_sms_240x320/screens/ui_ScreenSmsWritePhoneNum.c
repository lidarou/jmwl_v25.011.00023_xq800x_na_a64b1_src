
#include "../ui.h"
#include "app_log.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "tele_sms_data.h"
#include <stdio.h>
#include "alv_app_mgr.h"
#include "app_msg.h"
#include "settings_fp_flow.h"

#define __inf                   APP_LOGI


typedef struct ui_screen_sms_write_phonenum_obj_h {
    lv_obj_t *      label_find;
    lv_obj_t *      label_input_format;
    lv_obj_t *      label_remain_num;
    lv_style_t      style_remain_num;
    lv_obj_t *      obj_textarea;
    pinyin_param_t * pinyin;

    uint8_t         str_remain_num[5];
    uint32_t        wirte_count;
    uint8_t         remind_str[30];
    uint8_t name[PHONEBOOK_NAME_LEN];
    uint8_t         number[TS_SMS_NUM_MAX_LEN + 1];
    sms_write_ui_enter_param_t * ui_enter_param;
} ui_screen_sms_write_phonenum_t;


#define UI_SMS_WIRTE_PHONE_NUM_COUNT_MAX 20


lv_obj_t * ui_ScreenSmsWritePhoneNum_screen_init(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsWritePhoneNum = NULL;

    ui_screen_sms_write_phonenum_t * p_screen_param;
    lv_obj_t *      obj;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_sms_write_phonenum_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_sms_write_phonenum_t));


    ui_ScreenSmsWritePhoneNum = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenSmsWritePhoneNum,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenSmsWritePhoneNum,(lv_style_t *) get_lv_style_font_common(),0);


    obj                 = lv_label_create(ui_ScreenSmsWritePhoneNum);
    lv_obj_set_width(obj,100);
    lv_obj_set_height(obj,20);
    lv_obj_set_x(obj,0);
    lv_obj_set_y(obj,0);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);
    lv_label_set_text(obj,"123");
    p_screen_param->label_input_format = obj;

    obj                 = lv_label_create(ui_ScreenSmsWritePhoneNum);
    lv_obj_set_width(obj,140);
    lv_obj_set_height(obj,40);
    lv_obj_set_x(obj,120);
    lv_obj_set_y(obj,0);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);

    //sprintf(p_screen_param->str_remain_num,"%d",UI_SMS_WIRTE_PHONE_NUM_COUNT_MAX);
    //lv_label_set_text(obj, p_screen_param->str_remain_num);
    p_screen_param->label_remain_num = obj;

    lv_style_init(&p_screen_param->style_remain_num);
    lv_style_set_text_font(&p_screen_param->style_remain_num,(lv_font_t *) get_ft_info_font_common());
    lv_style_set_text_align(&p_screen_param->style_remain_num,LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_color(&p_screen_param->style_remain_num,lv_color_hex(0xFF));
    lv_obj_add_style(p_screen_param->label_remain_num,&p_screen_param->style_remain_num,0);


    obj                 = lv_label_create(ui_ScreenSmsWritePhoneNum);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"返回");

    obj                 = lv_label_create(ui_ScreenSmsWritePhoneNum);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,162);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"清除");

    obj                 = lv_label_create(ui_ScreenSmsWritePhoneNum);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,100);
    lv_obj_set_y(obj,290);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);
    lv_label_set_text(obj,"查找");
    p_screen_param->label_find = obj;


    obj                 = lv_label_create(ui_ScreenSmsWritePhoneNum);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,50);
    lv_label_set_text(obj,"电话号码");


    obj                 = lv_textarea_create(ui_ScreenSmsWritePhoneNum);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,100);
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
    lv_textarea_set_max_byte(obj,UI_SMS_WIRTE_PHONE_NUM_COUNT_MAX);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLLABLE);

    //lv_textarea_add_text(obj,p_screen_param->number);
    lv_obj_add_state(obj,LV_STATE_FOCUSED);
    p_screen_param->obj_textarea = obj;


    p_screen_param->ui_enter_param = enter_param;

    strncpy(p_screen_param->number,p_screen_param->ui_enter_param->p_number,TS_SMS_NUM_MAX_LEN);



    p_screen_param->pinyin = NULL;

    lv_obj_set_user_data(ui_ScreenSmsWritePhoneNum,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenSmsWritePhoneNum,fp_sms_write_phone_num_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenSmsWritePhoneNum);

    lv_obj_set_user_data(win,ui_ScreenSmsWritePhoneNum);


    return ui_ScreenSmsWritePhoneNum;
}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_sms_write_phonenum_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_write_phonenum_t *) lv_obj_get_user_data(a->var);
    ime_pinyin_keyboard_destroy(p_screen_param->pinyin);
    lv_style_reset(&p_screen_param->style_remain_num);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenSmsWritePhoneNum_screen_uninit(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsWritePhoneNum = NULL;

    ui_ScreenSmsWritePhoneNum = lv_obj_get_user_data(win);

    if (ui_ScreenSmsWritePhoneNum) {
        lv_obj_del_delayed_callback(ui_ScreenSmsWritePhoneNum,500,lv_obj_del_screen_ready_cb);
        tele_sms_data_set_send_fun_callback(0,0);
        }

    lv_obj_set_user_data(win,NULL);
}


void ui_ScreenSmsWritePhoneNum_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsWritePhoneNum;
    ui_screen_sms_write_phonenum_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    ui_ScreenSmsWritePhoneNum = lv_obj_get_user_data(win);
    p_screen_param      = (ui_screen_sms_write_phonenum_t *) lv_obj_get_user_data(ui_ScreenSmsWritePhoneNum);

    if (strlen(p_screen_param->number)) {
        lv_textarea_set_text(p_screen_param->obj_textarea,"a");
        lv_textarea_del_char(p_screen_param->obj_textarea);
        lv_textarea_add_text(p_screen_param->obj_textarea,p_screen_param->number);
        memset(p_screen_param->number,0,TS_SMS_NUM_MAX_LEN);
        }

    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));
    sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_WIRTE_PHONE_NUM_COUNT_MAX - count));
    lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

    if (count == 0)
        lv_label_set_text(p_screen_param->label_find,"查找");
    else 
        lv_label_set_text(p_screen_param->label_find,"确定");

    p_screen_param->wirte_count = count;


    lv_disp_load_scr(ui_ScreenSmsWritePhoneNum);

}


void sms_write_phonenum_pinyin_create(lv_obj_t * ui_Screen)
{
    ui_screen_sms_write_phonenum_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_write_phonenum_t *) lv_obj_get_user_data(ui_Screen);

    if (p_screen_param->pinyin)
        return;

    p_screen_param->pinyin = ime_pinyin_keyboard_create();

    if (p_screen_param->pinyin) {
        p_screen_param->pinyin->ta_user = p_screen_param->obj_textarea;
        ime_pinyin_ui_set_fortmat(p_screen_param->pinyin,format_num);
        }
}


static void remind_timer_callback_sms_send_result(screen_remind_param_t * p_param)
{

    alv_am_win_exit_action(NULL,"write",NULL);
    alv_am_win_exit_action(NULL,"write_option",NULL);
    alv_am_win_exit_action(NULL,"write_num",NULL);
}


typedef struct {
    int32_t         result;
    void *          user_data;
} sms_send_result_t;


static int32_t app_msg_sms_send_result_callback(void * p_param,uint32_t size)
{
    screen_remind_param_t screen_remind_param;
    sms_send_result_t * p_sms_send_result;
    lv_obj_t *      ui_screen;

    ui_screen_remind_param_init(&screen_remind_param);

    p_sms_send_result   = p_param;

    if (p_sms_send_result->result == 0)
        screen_remind_param.str = "短信已发送";
    else 
        screen_remind_param.str = "短信发送失败,请重新尝试";

    screen_remind_param.tiemr_delay = 1500;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_sms_send_result;
    screen_remind_param.user_data = NULL;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);

    return 0;
}



static int32_t sms_send_func_callback_result(int32_t send_result,void * user_data)
{
    sms_send_result_t sms_send_result;

    sms_send_result.result = send_result;
    sms_send_result.user_data = user_data;

    app_msg_send_callback(app_msg_sms_send_result_callback,&sms_send_result,sizeof(sms_send_result_t));
    return 0;
}


static void sms_send_no_sim_remind_screen()
{
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;

    screen_remind_param.str = "请插入SIM卡";

    ui_screen_remind_param_init(&screen_remind_param);

    screen_remind_param.tiemr_delay = 1500;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_sms_send_result;
    screen_remind_param.user_data = lv_scr_act();

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);

}


static void remind_timer_callback_sms_write_phone_num(screen_remind_param_t * p_param)
{
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;

    ui_screen_remind_param_init(&screen_remind_param);

    screen_remind_param.str = "短信发送失败,请重新尝试";

    screen_remind_param.tiemr_delay = 1500;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_sms_send_result;
    screen_remind_param.user_data = NULL;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);

}


static void sms_write_phone_num_remind_screen(lv_obj_t * ui_Screen)
{
    ui_screen_sms_write_phonenum_t * p_screen_param;
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;
    char *          text,*number;
    uint8_t         cur_card_id = TS_CARD_ID_MAX;

    ui_screen_remind_param_init(&screen_remind_param);
    p_screen_param      = (ui_screen_sms_write_phonenum_t *) lv_obj_get_user_data(ui_Screen);

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    sprintf(p_screen_param->remind_str,"卡%d正在发送短信",(int) (1 + cur_card_id));

    screen_remind_param.str = p_screen_param->remind_str;


    screen_remind_param.tiemr_delay = 10000;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_sms_write_phone_num;
    screen_remind_param.user_data = ui_Screen;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);

    text                = p_screen_param->ui_enter_param->p_content;
    number              = (char *) lv_textarea_get_text(p_screen_param->obj_textarea);

    tele_sms_data_set_send_fun_callback(sms_send_func_callback_result,NULL);
    tele_sms_data_send(text, number, p_screen_param->name,false);
    //tele_sms_data_send(text, "15302717405", NULL,false);
}






void sms_write_phonenum_key_event(uint32_t key,lv_obj_t * ui_Screen)
{
    ui_screen_sms_write_phonenum_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    p_screen_param      = (ui_screen_sms_write_phonenum_t *) lv_obj_get_user_data(ui_Screen);

    if (get_ime_pinyin_keyboard_active_state(p_screen_param->pinyin)) {

        ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
        label               = lv_textarea_get_label(p_screen_param->obj_textarea);
        count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

        if (p_screen_param->wirte_count != count) {
            sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_WIRTE_PHONE_NUM_COUNT_MAX - count));
            lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

            if (p_screen_param->wirte_count == 0)
                lv_label_set_text(p_screen_param->label_find,"确定");
            else if (count == 0)
                lv_label_set_text(p_screen_param->label_find,"查找");


            p_screen_param->wirte_count = count;
            memset(p_screen_param->name,0,PHONEBOOK_NAME_LEN);
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
                    memset(p_screen_param->name,0,PHONEBOOK_NAME_LEN);

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_get_encoded_length(lv_label_get_text(label));

                    if (p_screen_param->wirte_count != count) {
                        sprintf(p_screen_param->str_remain_num,"%d",
                            (int) (UI_SMS_WIRTE_PHONE_NUM_COUNT_MAX - count));
                        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                        if (p_screen_param->wirte_count == 0)
                            lv_label_set_text(p_screen_param->label_find,"确定");
                        else if (count == 0)
                            lv_label_set_text(p_screen_param->label_find,"查找");

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
                        sms_write_phone_num_find_ui_enter_param_t enter_param;

                        enter_param.p_number = p_screen_param->number;
                        enter_param.p_name	 = p_screen_param->name;
                        alv_am_win_enter(NULL,"write_num_find",(void *) &enter_param);
                        }
                    else {
                        settings_sim_exist_status_e sim_exist_status = SIM_EXIST_STATUS_MAX;

                        sim_exist_status    = settings_flow_get_sim_exist_status();

                        if (sim_exist_status == SIM_EXIST_STATUS_NO || sim_exist_status == SIM_EXIST_STATUS_MAX) // no sim
                            sms_send_no_sim_remind_screen();
                        else 
                            sms_write_phone_num_remind_screen(ui_Screen);
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
                    sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_SMS_WIRTE_PHONE_NUM_COUNT_MAX - count));
                    lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                    if (p_screen_param->wirte_count == 0)
                        lv_label_set_text(p_screen_param->label_find,"确定");
                    else if (count == 0)
                        lv_label_set_text(p_screen_param->label_find,"查找");


                    p_screen_param->wirte_count = count;
                    memset(p_screen_param->name,0,PHONEBOOK_NAME_LEN);
                    }

                break;
            }
        }


}




