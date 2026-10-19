
#include "lvgl.h"

#include "alv_app_mgr.h"

#include "app_log.h"
#include "backlight.h"

#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "app_call_list.h"
#include "flow/call_fp_flow.h"
#include "calllog_data.h"
#include "screen_remind.h"
#include "fp_playservice.h"
#include "app_log.h"
#include "obj_batch_deal.h"

#define __inf                   APP_LOGI

extern uint32_t ui_ScreenCalllogDialCalls_callog_change();

/* dialer win obj */
static lv_obj_t * g_call_dialer_win = NULL;

/* incoming win obj */
static lv_obj_t * g_call_incoming_win = NULL;

static lv_obj_t * g_call_on_the_phone_win = NULL;

static uint8_t  g_call_key_hangup;

static ui_redial_param_t auto_redial_para =
{
    0
};


static ui_sos_param_t auto_sos_para =
{
    0
};


void call_app_key_huangup_add()
{
    g_call_key_hangup++;
}


void call_app_key_huangup_reset()
{
    g_call_key_hangup   = 0;
}


uint8_t call_app_key_huangup_get()
{
    return g_call_key_hangup;
}


static void call_app_active(lv_obj_t * app,void * p_param)
{
    ui_call_enter_param_t * p_enter_param = p_param;

    ps_data_msg_send_call_stop_other_voice();

    switch (p_enter_param->call_origin)
    {
        case CALL_ORIGIN_DIALDER:
            alv_am_app_set_first_win(app,g_call_dialer_win);
            break;

        case CALL_ORIGIN_GLOBAL_INCOMING:
            alv_am_app_set_first_win(app,g_call_incoming_win);
            break;

        case CALL_ORIGIN_CALLLOG:
            break;

        case CALL_ORIGIN_CONTACT:
            alv_am_app_set_first_win(app,g_call_on_the_phone_win);
            break;
        }

    call_app_key_huangup_reset();

}


static void call_app_deactive_resume_other_app_action(void * p)
{
    call_fp_active_resume_other_app_action();
    set_fp_call_relesed_now_status(0);
}


static void call_app_deactive(lv_obj_t * app,void * p_param)
{

    lv_obj_del_delayed_callback(NULL,400,(lv_anim_ready_cb_t) call_app_deactive_resume_other_app_action);

}





static void call_incoming_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenIncoming_screen_init(win,p_param);

}


static void call_incoming_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenIncoming_screen_uninit(win,p_param);
}


static void call_incoming_fp_win_focus(lv_obj_t * win,void * p_param)
{

    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);

}


static void call_incoming_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



static void call_on_the_phone_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenOnThePhone_screen_init(win,p_param);

}


static void call_on_the_phone_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenOnThePhone_screen_uninit(win,p_param);
}


static void call_on_the_phone_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenOnThePhone_screen_focus(win,p_param);

}


static void call_on_the_phone_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



static void call_dial_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenDial_screen_init(win,p_param);

}


static void call_dial_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenDial_screen_uninit(win,p_param);
}


static void call_dial_fp_win_focus(lv_obj_t * win,void * p_param)
{

    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);

}


static void call_dial_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}




static void call_hangup_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenHangup_screen_init(win,p_param);

}


static void call_hangup_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenHangup_screen_uninit(win,p_param);
}


static void call_hangup_fp_win_focus(lv_obj_t * win,void * p_param)
{

    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);

}


static void call_hangup_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



static void call_phone_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenOnThePhoneOption_screen_init(win,p_param);

}


static void call_phone_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenOnThePhoneOption_screen_uninit(win,p_param);
}


static void call_phone_option_fp_win_focus(lv_obj_t * win,void * p_param)
{

    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);

}


static void call_phone_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}




static void app_call_ui_fp_init(void)
{

    /* Create APP for calllog */
    lv_obj_t *      app_call = alv_am_app_create(lv_scr_act(),CALL_FP_APP_NAME);

    alv_am_app_set_active(app_call,call_app_active);
    alv_am_app_set_deactive(app_call,call_app_deactive);


    lv_obj_t *      win_incoming = alv_am_win_create(app_call,"incoming");

    alv_am_win_set_active(win_incoming,call_incoming_fp_win_active);
    alv_am_win_set_deactive(win_incoming,call_incoming_fp_win_deacactive);
    alv_am_win_set_focus(win_incoming,call_incoming_fp_win_focus);
    alv_am_win_set_defocus(win_incoming,call_incoming_fp_win_defocus);
    alv_am_app_set_first_win(app_call,win_incoming);
    g_call_incoming_win = win_incoming;


    lv_obj_t *      win_on_the_phone = alv_am_win_create(app_call,"on_the_phone");

    alv_am_win_set_active(win_on_the_phone,call_on_the_phone_fp_win_active);
    alv_am_win_set_deactive(win_on_the_phone,call_on_the_phone_fp_win_deacactive);
    alv_am_win_set_focus(win_on_the_phone,call_on_the_phone_fp_win_focus);
    alv_am_win_set_defocus(win_on_the_phone,call_on_the_phone_fp_win_defocus);
    g_call_on_the_phone_win = win_on_the_phone;


    lv_obj_t *      win_dial = alv_am_win_create(app_call,"dial");

    alv_am_win_set_active(win_dial,call_dial_fp_win_active);
    alv_am_win_set_deactive(win_dial,call_dial_fp_win_deacactive);
    alv_am_win_set_focus(win_dial,call_dial_fp_win_focus);
    alv_am_win_set_defocus(win_dial,call_dial_fp_win_defocus);
    g_call_dialer_win   = win_dial;


    lv_obj_t *      win_hangup = alv_am_win_create(app_call,"hangup");

    alv_am_win_set_active(win_hangup,call_hangup_fp_win_active);
    alv_am_win_set_deactive(win_hangup,call_hangup_fp_win_deacactive);
    alv_am_win_set_focus(win_hangup,call_hangup_fp_win_focus);
    alv_am_win_set_defocus(win_hangup,call_hangup_fp_win_defocus);


    lv_obj_t *      win_phone_option = alv_am_win_create(app_call,"phone_option");

    alv_am_win_set_active(win_phone_option,call_phone_option_fp_win_active);
    alv_am_win_set_deactive(win_phone_option,call_phone_option_fp_win_deacactive);
    alv_am_win_set_focus(win_phone_option,call_phone_option_fp_win_focus);
    alv_am_win_set_defocus(win_phone_option,call_phone_option_fp_win_defocus);


    call_app_key_huangup_reset();
}


static int32_t win_incoming_show_action(void * p_param,uint32_t size)
{
    ui_call_enter_param_t * p_incoming_info = p_param;

    if (false == alv_am_app_is_exist("call")) {
        p_incoming_info->call_origin = CALL_ORIGIN_GLOBAL_INCOMING;
        alv_am_app_enter_action(NULL,"call",p_incoming_info);
        alv_am_win_exit_action_by_app("tools","tool_alarm_doing",NULL);
        }
    else {
        alv_am_win_jump_action(NULL,"incoming",p_incoming_info);
        }

    return 0;
}


static int32_t win_connected_show_action(void * p_param,uint32_t size)
{
    ui_call_enter_param_t * p_connected_info = p_param;

    if (false == alv_am_app_is_exist("call")) {
        p_connected_info->call_origin = CALL_ORIGIN_CONTACT;
        alv_am_app_enter_action(NULL,"call",p_connected_info);
        }
    else {
        alv_am_win_jump_action(NULL,"on_the_phone",p_connected_info);
        }

    return 0;
}


static int32_t handle_dialer_result(int32_t result);


static void auto_redial_timer_event(lv_timer_t * t)
{
    if (false == alv_am_app_is_exist("call")) {
        ui_call_enter_param_t * p_dial_info;

        p_dial_info         = get_ui_call_enter_param();

        if (auto_redial_para.phone_num == NULL)
            return;

        //__inf("call_dial_number:%s",number);
        strncpy(p_dial_info->phone_num,auto_redial_para.phone_num,TELE_PHONE_NUMBER_LEN_MAX);

        if (auto_redial_para.contact_name)
            strncpy(p_dial_info->contact_name,auto_redial_para.contact_name,CONTACT_NAME_LEN_MAX);

        if (auto_redial_para.redial_timer) {
            lv_timer_del(auto_redial_para.redial_timer);
            auto_redial_para.redial_timer = NULL;
            }

        call_fp_flow_dial(auto_redial_para.phone_num,handle_dialer_result);
        }
    else { // 自动重拨遇到手机还处于电话界面时，延迟一分钟再重拨

        if (auto_redial_para.redial_timer == NULL)
            auto_redial_para.redial_timer = lv_timer_create(auto_redial_timer_event,5 * 1000,NULL);
        else {
            lv_timer_reset(auto_redial_para.redial_timer);
            lv_timer_resume(auto_redial_para.redial_timer);
            }
        }

}


extern uint8_t get_sos_number(uint8_t number,char * buffer);


void stop_sos_timer(void)
{
    if (auto_sos_para.sos_timer) {
        lv_timer_del(auto_sos_para.sos_timer);
        auto_sos_para.sos_timer = NULL;
        }

    //memset(&auto_sos_para, 0, sizeof(ui_sos_param_t));
}


static void auto_sos_timer_event(lv_timer_t * t)
{

    if (false == alv_am_app_is_exist("call")) {
        ui_call_enter_param_t * p_dial_info;

        p_dial_info         = get_ui_call_enter_param();
        uint8_t         i;

        if (auto_sos_para.sos_number >= 3) {
            if (auto_sos_para.sos_timer) {
                lv_timer_del(auto_sos_para.sos_timer);
                }

            memset(&auto_sos_para,0,sizeof(ui_sos_param_t));
            return;
            }

        for (i = auto_sos_para.sos_number; i < 3; i++) {
            if (get_sos_number(auto_sos_para.sos_number,auto_sos_para.phone_num) == 0) {
                auto_sos_para.sos_number += 1;
                continue;
                }
            else {
                auto_sos_para.sos_number += 1;
                break;
                }
            }

        if (i >= 3) {
            auto_sos_para.sos_start = 0;
            auto_sos_para.sos_number = 0;

            if (auto_sos_para.sos_timer) {
                lv_timer_del(auto_sos_para.sos_timer);
                }

            memset(&auto_sos_para,0,sizeof(ui_sos_param_t));
            return;
            }

        if (auto_sos_para.phone_num == NULL) {
            auto_sos_para.sos_start = 0;
            auto_sos_para.sos_number = 0;

            if (auto_sos_para.sos_timer) {
                lv_timer_del(auto_sos_para.sos_timer);
                }

            memset(&auto_sos_para,0,sizeof(ui_sos_param_t));
            return;
            }

        strncpy(p_dial_info->phone_num,auto_sos_para.phone_num,TELE_PHONE_NUMBER_LEN_MAX);

        if (auto_sos_para.sos_timer) {
            lv_timer_del(auto_sos_para.sos_timer);
            auto_sos_para.sos_timer = NULL;
            }

        aos_memset((u8 *) p_dial_info->contact_name,0,CONTACT_NAME_LEN_MAX);
        alv_am_app_exit_action_all();

        call_fp_flow_dial(p_dial_info->phone_num,handle_dialer_result);
        }
    else { // sos手机还处于电话界面时，延迟一分钟再重拨

        if (auto_sos_para.sos_timer == NULL)
            auto_sos_para.sos_timer = lv_timer_create(auto_sos_timer_event,5 * 1000,NULL);
        else {
            lv_timer_reset(auto_sos_para.sos_timer);
            lv_timer_resume(auto_sos_para.sos_timer);
            }
        }

}


void start_sos_call(void)
{
    //call_flow_dial(auto_sos_para.phone_num, handle_dialer_result);
    auto_sos_para.sos_start = 1;
    auto_sos_para.sos_number = 0;
    aos_memset((u8 *) auto_sos_para.phone_num,0,TELE_PHONE_NUMBER_LEN_MAX);

    if (auto_sos_para.sos_timer) {
        lv_timer_reset(auto_sos_para.sos_timer);
        lv_timer_resume(auto_sos_para.sos_timer);
        }
    else {
        auto_sos_para.sos_timer = lv_timer_create(auto_sos_timer_event,2 * 1000,NULL);
        }
}


static int32_t win_released_show_action(void * p_param,uint32_t size)
{

    ui_call_enter_param_t * p_call_param = p_param;

    calllog_data_param_t calllog_data_param;

    //calllog_data_time_t cur_time;
    uint32_t        len,ret;

    if (alv_am_app_is_exist("call")) {
        set_fp_call_relesed_now_status(1);
        }

    ret                 = ui_ScreenCalllogDialCalls_callog_change();

    if (ret == 0)
        alv_am_app_exit_action(NULL,"call",NULL);


    memset(&calllog_data_param,0,sizeof(calllog_data_param));

    if (CALLLOG_DATA_NAME_LEN > CONTACT_NAME_LEN_MAX)
        len = CONTACT_NAME_LEN_MAX;
    else 
        len = CALLLOG_DATA_NAME_LEN;

    strncpy((char *) &calllog_data_param.name,p_call_param->contact_name,len);


    if (CALLLOG_DATA_LEN > TELE_PHONE_NUMBER_LEN_MAX)
        len = TELE_PHONE_NUMBER_LEN_MAX;
    else 
        len = CALLLOG_DATA_LEN;

    strncpy((char *) &calllog_data_param.number,p_call_param->phone_num,len);

    if (strlen(p_call_param->phone_num) == 0) {
        if (ret)
            alv_am_app_exit_action(NULL,"call",NULL);

        return - 1;
        }

    calllog_data_param.card_id = p_call_param->card_id;
    calllog_data_param.call_type = p_call_param->call_type;


    memcpy(&calllog_data_param.time,&p_call_param->time,sizeof(calllog_data_time_t));

    //__inf("win_released_show_action call_type:%d",p_call_param->call_type);
    //__inf("p_call_param->phone_num :%s",p_call_param->phone_num);
    //__inf("auto_redial_para.phone_num :%s",auto_redial_para.phone_num);
    //__inf(" p_call_param->end_call_type :%d,sos_start=%d,call_type=%d",p_call_param->end_call_type,auto_sos_para.sos_start,p_call_param->call_type);
    if (auto_sos_para.sos_start) {
        //__inf("sos_start p_call_param->end_call_type :%d",p_call_param->end_call_type);
        if ((p_call_param->end_call_type != 2) && (p_call_param->end_call_type != 6)) {
            if (auto_sos_para.sos_timer) {
                lv_timer_del(auto_sos_para.sos_timer);
                auto_sos_para.sos_timer = NULL;
                }

            memset(&auto_sos_para,0,sizeof(ui_sos_param_t));
            }
        else {
            if (auto_sos_para.sos_timer) {
                lv_timer_reset(auto_sos_para.sos_timer);
                lv_timer_resume(auto_sos_para.sos_timer);
                }
            else {
                auto_sos_para.sos_timer = lv_timer_create(auto_sos_timer_event,2 * 1000,NULL);
                }
            }

        p_call_param->end_call_type = 0;
        }
    else if (0 == strncmp(auto_redial_para.phone_num,p_call_param->phone_num,strlen(p_call_param->phone_num))) {
        //__inf("p_call_param->end_call_type :%d",p_call_param->end_call_type);
        if (p_call_param->end_call_type != 2) {
            if (auto_redial_para.redial_timer) {
                lv_timer_del(auto_redial_para.redial_timer);
                }

            memset(&auto_redial_para,0,sizeof(ui_redial_param_t));
            }
        else {
            auto_redial_para.redial_counter += 1;

            if (auto_redial_para.redial_counter >= 3) {
                if (auto_redial_para.redial_timer) {
                    lv_timer_del(auto_redial_para.redial_timer);
                    }

                memset(&auto_redial_para,0,sizeof(ui_redial_param_t));
                }
            else {
                if (auto_redial_para.redial_timer) {
                    lv_timer_reset(auto_redial_para.redial_timer);
                    lv_timer_resume(auto_redial_para.redial_timer);
                    }
                else {
                    auto_redial_para.redial_timer = lv_timer_create(auto_redial_timer_event,2 * 1000,NULL);
                    }
                }
            }

        p_call_param->end_call_type = 0;
        }

    switch (p_call_param->call_type)
    {
        case 0:
            {
                extern unsigned char get_auto_redial_status(void);

                if ((p_call_param->end_call_type == 2) && get_auto_redial_status()) {
                    auto_redial_para.redial_counter = 0;
                    strncpy(auto_redial_para.contact_name,p_call_param->contact_name,CONTACT_NAME_LEN_MAX);
                    strncpy(auto_redial_para.phone_num,p_call_param->phone_num,TELE_PHONE_NUMBER_LEN_MAX);

                    if (auto_redial_para.redial_timer == NULL)
                        auto_redial_para.redial_timer = lv_timer_create(auto_redial_timer_event,2 * 1000,NULL);
                    else {
                        lv_timer_reset(auto_redial_para.redial_timer);
                        lv_timer_resume(auto_redial_para.redial_timer);
                        }
                    }
                }

            //calllog_data_dialed_number_write(&calllog_data_param,&cur_time);
            //break;
        case 1:
            //calllog_data_missed_calls_write(&calllog_data_param,&cur_time);
            //break;
        case 2:
            //calllog_data_recevied_calls_write(&calllog_data_param,&cur_time);
            calllog_data_write_number(&calllog_data_param);
            break;

        default:
            break;
        }

    if (ret)
        alv_am_app_exit_action(NULL,"call",NULL);

    return 0;
}









static int32_t win_incoming_show(ui_call_enter_param_t * p_incoming_info)
{
    aos_systime     time;

    aos_get_systime(&time);

    p_incoming_info->time.year = time.year;
    p_incoming_info->time.month = time.month;
    p_incoming_info->time.day = time.day;
    p_incoming_info->time.hour = time.hour;
    p_incoming_info->time.minute = time.minute;
    p_incoming_info->time.second = time.second;

    app_msg_send_callback(win_incoming_show_action,p_incoming_info,sizeof(ui_call_enter_param_t));
    return 0;
}


static int32_t win_connected_show(ui_call_enter_param_t * p_connected_info)
{
    aos_systime     time;

    aos_get_systime(&time);

    p_connected_info->time.year = time.year;
    p_connected_info->time.month = time.month;
    p_connected_info->time.day = time.day;
    p_connected_info->time.hour = time.hour;
    p_connected_info->time.minute = time.minute;
    p_connected_info->time.second = time.second;
    app_msg_send_callback(win_connected_show_action,p_connected_info,sizeof(ui_call_enter_param_t));
    return 0;
}


static int32_t win_released_show(ui_call_enter_param_t * p_call_param)
{
    app_msg_send_callback(win_released_show_action,p_call_param,sizeof(ui_call_enter_param_t));
    return 0;
}


static int32_t ims_status_show(bool ims_on)
{
    return 0;

}


static const call_flow_t g_call_wins_flow =
{
    win_incoming_show,
    win_connected_show,
    win_released_show,
    ims_status_show,
};


void app_call_fp_init(void)
{
    app_call_ui_fp_init();
    call_fp_flow_register_ui_actions(&g_call_wins_flow);
    call_fp_flow_init();
}



static void remind_timer_callback_call_dial_fialed(screen_remind_param_t * p_param)
{

    lv_disp_load_scr(p_param->user_data);

}


static void call_dial_faild_remind_screen()
{
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;

    screen_remind_param.str = "拨打失败";

    ui_screen_remind_param_init(&screen_remind_param);

    screen_remind_param.tiemr_delay = 1500;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_call_dial_fialed;
    screen_remind_param.user_data = lv_scr_act();

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);

}




static int32_t handle_dialer_result_action(void * p_param,uint32_t size)
{
    int32_t         result,*p_result;


    p_result            = p_param;
    result              = *p_result;

    __inf("[%s],dialer result = %d",__func__,result);

    if (0 != result) {
        call_dial_faild_remind_screen();
        }
    else {
        ui_call_enter_param_t * p_dial_info;
        aos_systime     time;

        p_dial_info         = (ui_call_enter_param_t *) get_ui_call_enter_param();

        aos_get_systime(&time);

        p_dial_info->time.year = time.year;
        p_dial_info->time.month = time.month;
        p_dial_info->time.day = time.day;
        p_dial_info->time.hour = time.hour;
        p_dial_info->time.minute = time.minute;
        p_dial_info->time.second = time.second;


        if (false == alv_am_app_is_exist("call")) {
            p_dial_info->call_origin = CALL_ORIGIN_DIALDER;
            alv_am_app_enter_action(NULL,"call",p_dial_info);
            }
        else {
            alv_am_win_jump_action(NULL,"dial",p_dial_info);
            }
        }

    return 0;
}


static int32_t handle_dialer_result(int32_t result)
{
    app_msg_send_callback(handle_dialer_result_action,&result,sizeof(result));

    return 0;
}

int32_t call_dial_number(uint8_t * number,uint8_t * name)
{
    int32_t ret;

    ret = -1;

    ui_call_enter_param_t * p_dial_info;

    p_dial_info         = (ui_call_enter_param_t *) get_ui_call_enter_param();

    if (number == NULL)
        return ret;

    //__inf("call_dial_number:%s",number);
    strncpy(p_dial_info->phone_num,number,TELE_PHONE_NUMBER_LEN_MAX);

    if (name)
        strncpy(p_dial_info->contact_name,name,CONTACT_NAME_LEN_MAX);

    if (true == call_fp_flow_is_em_enter_code(number)) {
        __inf("[%s],enter em app",__func__);
        alv_am_app_enter(NULL,"em",NULL);
        return 0;
    }

    if (true == call_fp_flow_is_imei_enter_code(number)) {
        __inf("[%s],imei_enter",__func__);
        alv_am_win_jump_action(NULL,"fp_imei",NULL);
        return 0;
        }

    ret = call_fp_flow_dial(number,handle_dialer_result);
    return ret;
}
