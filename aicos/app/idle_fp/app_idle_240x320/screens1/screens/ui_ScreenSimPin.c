
#include <stdio.h>
#include "../ui.h"
#include "app_log.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "lv_port_indev_keypad.h"
#include "prompt_box_common.h"
#include "alv_app_mgr.h"
#include "app_msg.h"
#include "phonebook.h"

#define __inf                   APP_LOGI

typedef struct ui_screen_sim_pin_obj_h {
    lv_obj_t *      p_promptbox;
    lv_obj_t *      label_ramain_times;
    lv_obj_t *      label_ok;
    lv_obj_t *      label_del;
    lv_obj_t *      textarea;

    uint8_t password[10];
} ui_screen_sim_pin_obj_t;

typedef struct {

    /* the remain times */
    int32_t         remain_times;

    /* 0: pin is closed, 1: pin is open. */
    int32_t         status;
} ts_sim_pin_param_t;

static ts_sim_pin_param_t ts_sim_pin_param;


static int32_t get_sim_pin_remain_times_resp_callback(void * p_param,uint32_t size)
{
    ts_sim_get_pin_remain_times_resp_t * p_get_pin_remain_times_resp = NULL;

    if (NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n",__func__);
        return - 1;
        }

    p_get_pin_remain_times_resp = (ts_sim_get_pin_remain_times_resp_t *) p_param;
    app_logi("[%s]ret_value = %d, remain_times = %d.\r\n",
        __func__,
        p_get_pin_remain_times_resp->ret_value,
        p_get_pin_remain_times_resp->remain_times);


    if (p_get_pin_remain_times_resp->ret_value == 0)
        ts_sim_pin_param.remain_times = p_get_pin_remain_times_resp->remain_times;


    return 0;
}


int32_t app_msg_func_callback_enter_sim_pin_screen(void * p_param,uint32_t size)
{
    if (false == alv_am_app_is_exist("idle_fp")) {
        lv_obj_t *      frist_win;
        lv_obj_t *      idle_app;

        idle_app            = get_app_by_name("idle_fp");
        frist_win           = get_win_by_name(idle_app,"fp_sim_pin");
        alv_am_app_set_first_win(idle_app,frist_win);
        alv_am_app_enter_action(NULL,"idle_fp",NULL);
        }
    else {
        alv_am_win_enter(NULL,"fp_sim_pin",NULL);
        }

    return 0;
}

static int32_t get_sim_pin_lock_status_resp_callback(void * p_param,uint32_t size)
{
    ts_sim_get_pin_lock_status_resp_t * p_get_pin_lock_status_resp = NULL;

    if (NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n",__func__);
        return - 1;
        }

    p_get_pin_lock_status_resp = (ts_sim_get_pin_lock_status_resp_t *) p_param;
    app_logi("[%s]ret_value = %d, status = %d.\r\n",
        __func__,
        p_get_pin_lock_status_resp->ret_value,
        p_get_pin_lock_status_resp->status);

    if (p_get_pin_lock_status_resp->ret_value == 0) {
        ts_sim_pin_param.status = p_get_pin_lock_status_resp->status;

        if (ts_sim_pin_param.status)
            app_msg_send_callback(app_msg_func_callback_enter_sim_pin_screen,0,0);
        }

    return 0;
}

void get_sim_pin_remain_times(void)
{
    int             ret = -1;
    uint8_t         cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret                 = aic_srv_tele_sim_get_pin_remain_times(cur_card_id,get_sim_pin_remain_times_resp_callback);
    app_logi("[%s]get: ret = %d.",__func__,ret);
}

int32_t app_msg_func_callback_exit_sim_pin_screen(void * p_param,uint32_t size)
{
#if 0
    char            name[32];

    memset((u8 *) name,0,32);

    alv_am_win_get_cur_winname(name);

    if (strcmp(name,"fp_sim_pin")) {
        return;
        }

#elif 0
    lv_obj_t *      frist_win;
    lv_obj_t *      idle_app;

    idle_app            = get_app_by_name("idle_fp");
    frist_win           = get_win_by_name(idle_app,"watch");

    alv_am_win_jump_action(NULL,"watch",NULL);
    alv_am_app_set_first_win(idle_app,frist_win);
#elif 1
    alv_am_win_exit_action_by_app("idle_fp","fp_sim_pin",NULL);
#endif

    tele_phonebook_reset_init_steps();
    get_sim_pin_remain_times();
    return 0;
}



int32_t app_msg_func_callback_update_sim_pin_screen(void * p_param,uint32_t size)
{
    ui_screen_sim_pin_obj_t * p_screen_param;
    lv_obj_t *      screen;
    lv_obj_t *      win;
    lv_obj_t *      idle_app;
    lv_obj_t *      label;
    uint8_t         str_buf[100];
    uint32_t        count;

    idle_app            = get_app_by_name("idle_fp");
    win                 = get_win_by_name(idle_app,"fp_sim_pin");
    screen              = lv_obj_get_user_data(win);

    if (screen == NULL)
        return 0;

    p_screen_param      = lv_obj_get_user_data(screen);

    if (p_screen_param) {
        memset(str_buf,0,100);
        sprintf(str_buf,"您还有%d次机会",(int)ts_sim_pin_param.remain_times);
        lv_label_set_text(p_screen_param->label_ramain_times,str_buf);

        label               = lv_textarea_get_label(p_screen_param->textarea);
        count               = strlen(lv_label_get_text(label));

        for (; count > 0; count--) {
            lv_textarea_del_char(p_screen_param->textarea);
            }

        prompt_box_common_info_t box_common_info = {
            0
            };
        box_common_info.timeout = 2000;
        box_common_info.content = "PIN码错误";
        prompt_box_common_create(lv_scr_act(),&box_common_info);

        lv_obj_set_style_text_opa(p_screen_param->label_del, LV_OPA_40, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(p_screen_param->label_ok, LV_OPA_40, LV_PART_MAIN | LV_STATE_DEFAULT);
        }

    return 0;
}

static int32_t unlock_sim_pin_resp_callback(void * p_param,uint32_t size)
{
    ts_sim_unlock_pin_resp_t * p_unlock_pin_resp = NULL;

    if (NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n",__func__);
        return - 1;
        }

    p_unlock_pin_resp   = (ts_sim_unlock_pin_resp_t *) p_param;
    app_logi("[%s]ret_value = %d, remain_times = %d.\r\n",
        __func__,
        p_unlock_pin_resp->ret_value,
        p_unlock_pin_resp->remain_times);

    if (p_unlock_pin_resp->ret_value) {
        //ts_sim_pin_param.remain_times = p_unlock_pin_resp->remain_times;
        if (ts_sim_pin_param.remain_times > 0)
            ts_sim_pin_param.remain_times--;

        app_msg_send_callback(app_msg_func_callback_update_sim_pin_screen,0,0);

        }
    else {
        app_msg_send_callback(app_msg_func_callback_exit_sim_pin_screen,0,0);
        }

    return 0;
}

void get_sim_pin_lock_status(void)
{
    int             ret = -1;
    uint8_t         cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret                 = aic_srv_tele_sim_get_pin_lock_status(cur_card_id,get_sim_pin_lock_status_resp_callback);
    app_logi("[%s]get: ret = %d.",__func__,ret);
}

void unlock_sim_pin(char * p_pin)
{
    int             ret = -1;
    uint8_t         cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret                 = aic_srv_tele_sim_unlock_pin(cur_card_id,p_pin,unlock_sim_pin_resp_callback);
    app_logi("[%s]get: ret = %d.p_pin:%s",__func__,ret,p_pin);
}








void ts_sim_pin_param_init()
{
    int32_t         ret;
    uint32_t        sim_status;

    ts_sim_pin_param.remain_times = 3;
    ts_sim_pin_param.status = 0;

    app_logi("[%s]get:", __func__);
    ret                 = aic_srv_tele_sim_get_exist_status(&sim_status);

    if ((ret != 0) || (sim_status == 0))
        return;

    get_sim_pin_lock_status();
    get_sim_pin_remain_times();
}

int32_t get_sim_pin_param_status()
{
    return ts_sim_pin_param.status;
}

int32_t get_sim_pin_param_remain_times()
{
    return ts_sim_pin_param.remain_times;
}








lv_obj_t * ui_ScreenSimPinscreen_init(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSimPin = NULL;

    ui_screen_sim_pin_obj_t * p_screen_param;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_sim_pin_obj_t));

    if (p_screen_param == NULL)
        return 0;


    memset(p_screen_param,0,sizeof(ui_screen_sim_pin_obj_t));

    ui_ScreenSimPin     = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenSimPin,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenSimPin,get_lv_style_font_common(),0);
    lv_obj_set_style_bg_color(ui_ScreenSimPin,lv_color_hex(0xBEBEBE),LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ScreenSimPin,255,LV_PART_MAIN | LV_STATE_DEFAULT);



    {
        lv_obj_t *      p_promptbox,*label,*textarea;
        uint8_t         str_buf[100];
        uint8_t         cur_card = TS_CARD_ID_MAX;

        p_promptbox         = lv_obj_create(ui_ScreenSimPin);
        lv_obj_clear_flag(p_promptbox,LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_style(p_promptbox,get_lv_style_font_common(),0);
        lv_obj_set_pos(p_promptbox,10,30);
        lv_obj_set_size(p_promptbox,220,260);
        lv_obj_center(p_promptbox);
        lv_obj_set_style_bg_color(p_promptbox,lv_color_hex(0xFFFFFF),LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(p_promptbox,255,LV_PART_MAIN | LV_STATE_DEFAULT);
        p_screen_param->p_promptbox = p_promptbox;

        memset(str_buf,0,100);
        aic_srv_tele_sim_get_current_card(&cur_card);
        sprintf(str_buf,"输入卡%d的PIN(4-8)",cur_card + 1);
        label               = lv_label_create(p_promptbox);
        lv_label_set_text(label,str_buf);
        lv_obj_set_pos(label,0,0);
        lv_obj_set_size(label,lv_pct(100),LV_SIZE_CONTENT);
        lv_obj_set_style_border_side(label,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(label,LV_TEXT_ALIGN_CENTER,0);

        memset(str_buf,0,100);
        sprintf(str_buf,"您还有%d次机会",(int)ts_sim_pin_param.remain_times);
        label               = lv_label_create(p_promptbox);
        lv_label_set_text(label,str_buf);
        lv_obj_set_pos(label,0,90);
        lv_obj_set_size(label,lv_pct(100),LV_SIZE_CONTENT);
        lv_obj_add_style(label,get_lv_style_font_custom(20),0);
        lv_obj_set_style_border_side(label,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(label,LV_TEXT_ALIGN_CENTER,0);

        p_screen_param->label_ramain_times = label;


        textarea            = lv_textarea_create(p_promptbox);
        lv_obj_set_x(textarea,0);
        lv_obj_set_y(textarea,130);
        lv_obj_set_width(textarea,lv_pct(100));
        lv_obj_set_height(textarea,50);
        lv_textarea_set_one_line(textarea,true);
        lv_obj_set_style_pad_left(textarea,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(textarea,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(textarea,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(textarea,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(textarea,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(textarea,LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_set_style_border_width(textarea,0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_textarea_set_max_length(textarea,8);
        lv_obj_clear_flag(textarea,LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_state(textarea,LV_STATE_FOCUSED);
        lv_textarea_set_password_mode(textarea,true);

        p_screen_param->textarea = textarea;

        label               = lv_label_create(p_promptbox);
        lv_obj_set_width(label,LV_SIZE_CONTENT);
        lv_obj_set_height(label,LV_SIZE_CONTENT);
        lv_obj_set_x(label,0);
        lv_obj_set_y(label,200);
        lv_label_set_text(label,"确认");
        lv_obj_set_style_text_opa(label,LV_OPA_40,LV_PART_MAIN | LV_STATE_DEFAULT);

        p_screen_param->label_ok = label;


        label               = lv_label_create(p_promptbox);
        lv_obj_set_width(label,LV_SIZE_CONTENT);
        lv_obj_set_height(label,LV_SIZE_CONTENT);
        lv_obj_set_x(label,112);
        lv_obj_set_y(label,200);
        lv_label_set_text(label,"删除");
        lv_obj_set_style_text_opa(label,LV_OPA_40,LV_PART_MAIN | LV_STATE_DEFAULT);

        p_screen_param->label_del = label;


        }





    lv_obj_set_user_data(ui_ScreenSimPin,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenSimPin,fp_sim_pin_ui_event_actions,LV_EVENT_ALL,ui_ScreenSimPin);

    lv_obj_set_user_data(win,ui_ScreenSimPin);

    return ui_ScreenSimPin;

}

static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_sim_pin_obj_t * p_screen_param;

    p_screen_param      = (ui_screen_sim_pin_obj_t *) lv_obj_get_user_data(a->var);

    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}

void ui_ScreenSimPinscreen_uninit(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSimPin = NULL;

    ui_ScreenSimPin     = lv_obj_get_user_data(win);

    if (ui_ScreenSimPin) {
        lv_obj_del_delayed_callback(ui_ScreenSimPin,100,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}

void screen_sim_pin_focus_event(uint32_t event_code,lv_obj_t * ui_ScreenSimPin)
{
    ui_screen_sim_pin_obj_t * p_screen_param = lv_obj_get_user_data(ui_ScreenSimPin);

    switch (event_code)
    {
        case LV_EVENT_DEFOCUSED:
            lv_obj_set_style_bg_color(p_screen_param->p_promptbox,lv_color_hex(0xCECECE),
                LV_PART_MAIN | LV_STATE_DEFAULT);
            break;

        case LV_EVENT_FOCUSED:
            lv_obj_set_style_bg_color(p_screen_param->p_promptbox,lv_color_hex(0xFFFFFF),
                LV_PART_MAIN | LV_STATE_DEFAULT);
        }
}





void screen_sim_pin_key_event(uint32_t key,lv_obj_t * ui_ScreenSimPin)
{
    ui_screen_sim_pin_obj_t * p_screen_param = lv_obj_get_user_data(ui_ScreenSimPin);
    char *          password;
    uint32_t        count;

    switch (key)
    {
        case LV_KEY_UP:
        case LV_KEY_DOWN:
            break;

        case LV_KEY_ENTER:
        case APP_KEY_VALUE_TOP_LEFT:
            password = (char *)lv_textarea_get_text(p_screen_param->textarea);

            if (password == NULL)
                break;

            count = strlen(password);

            if(count>=4){
                memset(p_screen_param->password,0,10);
                strcpy(p_screen_param->password,password);
                unlock_sim_pin(p_screen_param->password);
                for(;count>0;count--)
                {
                    lv_textarea_del_char(p_screen_param->textarea);
                }
                lv_obj_set_style_text_opa(p_screen_param->label_del, LV_OPA_40, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(p_screen_param->label_ok, LV_OPA_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
            break;

        case APP_KEY_VALUE_TOP_RIGHT:
            {
                lv_textarea_del_char(p_screen_param->textarea);
                password            = (char *)lv_textarea_get_text(p_screen_param->textarea);

                if (password == NULL) {
                    lv_obj_set_style_text_opa(p_screen_param->label_del,LV_OPA_40,LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(p_screen_param->label_ok,LV_OPA_40,LV_PART_MAIN | LV_STATE_DEFAULT);
                    break;
                    }

                count               = strlen(password);

                if (count == 0)
                    lv_obj_set_style_text_opa(p_screen_param->label_del,LV_OPA_40,LV_PART_MAIN | LV_STATE_DEFAULT);

                if (count < 4)
                    lv_obj_set_style_text_opa(p_screen_param->label_ok,LV_OPA_40,LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            break;

        case LV_KEY_END:
            break;

        case LV_KEY_9:
        case LV_KEY_8:
        case LV_KEY_7:
        case LV_KEY_6:
        case LV_KEY_5:
        case LV_KEY_4:
        case LV_KEY_3:
        case LV_KEY_2:
        case LV_KEY_1:
        case LV_KEY_0:
            {
                uint32_t        c;

                if (ts_sim_pin_param.remain_times == 0)
                    break;

                if ((key >= LV_KEY_0) && (key <= LV_KEY_9)) {
                    c                   = '0' + key - LV_KEY_0;
                    lv_textarea_add_char(p_screen_param->textarea,c);
                    }

                password            = (char *)lv_textarea_get_text(p_screen_param->textarea);

                if (password == NULL)
                    break;

                count               = strlen(password);

                if (count > 0)
                    lv_obj_set_style_text_opa(p_screen_param->label_del,LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);

                if (count >= 4)
                    lv_obj_set_style_text_opa(p_screen_param->label_ok,LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);

                }
            break;
        }

}



int32_t app_msg_func_callback_enable_sim_pin_screen_failed(void * p_param,uint32_t size)
{
#if 0
    prompt_box_common_info_t box_common_info = {
        0
        };
    box_common_info.timeout = 0;
    box_common_info.content = "PIN码错误";
    prompt_box_common_create(lv_scr_act(),&box_common_info);

#else

    create_comp_msgbox_remind("PIN码错误");
#endif

    get_sim_pin_remain_times();
    return 0;
}

int32_t app_msg_func_callback_enable_sim_pin_screen_sucessed(void * p_param,uint32_t size)
{
#if 0
    prompt_box_common_info_t box_common_info = {
        0
        };
    box_common_info.timeout = 0;

    if (ts_sim_pin_param.status)
        box_common_info.content = "PIN码开启成功";
    else 
        box_common_info.content = "PIN码关闭成功";

    prompt_box_common_create(lv_scr_act(),&box_common_info);

#else

    if (ts_sim_pin_param.status)
        create_comp_msgbox_remind("PIN码开启成功");
    else 
        create_comp_msgbox_remind("PIN码关闭成功");

#endif

    ts_sim_pin_param.remain_times = 3;
    get_sim_pin_remain_times();

    return 0;
}

static int32_t get_sim_pin_lock_status_resp_callback1(void * p_param,uint32_t size)
{
    ts_sim_get_pin_lock_status_resp_t * p_get_pin_lock_status_resp = NULL;

    if (NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n",__func__);
        return - 1;
        }

    p_get_pin_lock_status_resp = (ts_sim_get_pin_lock_status_resp_t *) p_param;
    app_logi("[%s]ret_value = %d, status = %d.\r\n",
        __func__,
        p_get_pin_lock_status_resp->ret_value,
        p_get_pin_lock_status_resp->status);

    if (p_get_pin_lock_status_resp->ret_value == 0) {
        ts_sim_pin_param.status = p_get_pin_lock_status_resp->status;
        app_msg_send_callback(app_msg_func_callback_enable_sim_pin_screen_sucessed,0,0);
        }



    return 0;
}


int32_t app_msg_func_callback_enable_sim_pin_sucessed(void * p_param,uint32_t size)
{
    int             ret = -1;
    uint8_t         cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret                 = aic_srv_tele_sim_get_pin_lock_status(cur_card_id,get_sim_pin_lock_status_resp_callback1);
    app_logi("[%s]get: ret = %d.",__func__,ret);

    return 0;
}


static int32_t set_sim_pin_lock_status_resp_callback(void * p_param,uint32_t size)
{
    ts_sim_set_pin_lock_status_resp_t * p_set_pin_lock_status_resp = NULL;

    if (NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n",__func__);
        return - 1;
        }

    p_set_pin_lock_status_resp = (ts_sim_set_pin_lock_status_resp_t *) (p_param);
    app_logi("[%s]ret_value = %d, remain_times = %d.\r\n",
        __func__,
        p_set_pin_lock_status_resp->ret_value,
        p_set_pin_lock_status_resp->remain_times);

    if (p_set_pin_lock_status_resp->ret_value) {
        if (ts_sim_pin_param.remain_times > 0)
            ts_sim_pin_param.remain_times--;

        app_msg_send_callback(app_msg_func_callback_enable_sim_pin_screen_failed,0,0);

        }
    else {

        app_msg_send_callback(app_msg_func_callback_enable_sim_pin_sucessed,0,0);
        }

    return 0;
}



void set_sim_pin_lock_status(bool is_enable,uint8_t * p_pin)
{
    int             ret = -1;
    ts_sim_pin_lock_status_t lock_status = {
        0
        };
    uint8_t         cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    lock_status.is_enable = is_enable;
    memcpy(lock_status.pin_pwd,p_pin,strlen(p_pin));
    ret                 = aic_srv_tele_sim_set_pin_lock_status(cur_card_id,&lock_status,
        set_sim_pin_lock_status_resp_callback);
    app_logi("[%s]set true: ret = %d.",__func__,ret);
}






















































int32_t app_msg_func_callback_change_sim_pin_screen_failed(void * p_param,uint32_t size)
{
    create_comp_msgbox_remind("修改PIN码错误");

    get_sim_pin_remain_times();
    return 0;
}


int32_t app_msg_func_callback_change_sim_pin_screen_sucessed(void * p_param,uint32_t size)
{
    create_comp_msgbox_remind("修改PIN码成功");

    get_sim_pin_remain_times();

    return 0;
}


static int32_t change_sim_pin_resp_callback(void * p_param,uint32_t size)
{
    ts_sim_change_pin_resp_t * p_change_pin_resp = NULL;

    if (NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n",__func__);
        return - 1;
        }

    p_change_pin_resp   = (ts_sim_change_pin_resp_t *) p_param;
    app_logi("[%s]ret_value = %d, remain_times = %d.\r\n",
        __func__,
        p_change_pin_resp->ret_value,
        p_change_pin_resp->remain_times);

    if (p_change_pin_resp->ret_value) {
        if (ts_sim_pin_param.remain_times > 0)
            ts_sim_pin_param.remain_times--;

        app_msg_send_callback(app_msg_func_callback_change_sim_pin_screen_failed,0,0);

        }
    else {

        app_msg_send_callback(app_msg_func_callback_change_sim_pin_screen_sucessed,0,0);
        }

    return 0;
}





void change_sim_pin(char * p_new_pin,char * p_old_pin)
{
    int             ret = -1;
    uint8_t         cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret                 = aic_srv_tele_sim_change_pin(cur_card_id,p_new_pin,p_old_pin,change_sim_pin_resp_callback);
    app_logi("[%s]get: ret = %d.",__func__,ret);
}











