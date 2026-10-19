
#include "lvgl.h"

#include "alv_app_mgr.h"
#include "backlight.h"

#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "app_remind_list.h"
#include "app_log.h"
#include "tts_app_player.h"
#include "obj_batch_deal.h"

#define __inf                   APP_LOGI



#define APP_REMIND_UI_WIN_STACK_COUNT 2




typedef enum {
    REMIND_APP_WIN_idle_news = 1,
    REMIND_APP_WIN_low_power = 2,
} remind_app_win_enum_t;


typedef union {
    ui_screen_news_enter_param_t news;
} remind_win_enter_param_t;


typedef struct {
    remind_win_enter_param_t win_enter_param;
    remind_app_win_enum_t win_type;
} remind_app_enter_param_t;


typedef struct {
    lv_obj_t *      ui;
    lv_obj_t * (*init) (void * p_param);
    void(*uninit) (void * p_param);
    remind_app_win_enum_t win_type;
} remind_screen_param_t;


typedef struct {
    remind_screen_param_t ui_screen_stack[APP_REMIND_UI_WIN_STACK_COUNT];

    uint8_t         ui_screen_stack_count;

    uint8_t         sms_count;
    uint8_t         call_count;
} app_remind_param_t;


static app_remind_param_t app_remind_param;



extern lv_obj_t * ui_ScreenRemindNews_screen_init(void * p_enter_param);
extern void ui_ScreenRemindNews_screen_uninit(lv_obj_t * ui_ScreenRemindNews);
extern lv_obj_t * ui_FpLowPower_screen_init(void * p_enter_param);
extern void ui_FpLowPower_screen_uninit(lv_obj_t * ui_ScreenRemindNews);


void remind_fp_app_active(lv_obj_t * app,void * p_param)
{
    remind_app_enter_param_t * p_enter_param = p_param;

    remind_screen_param_t * p_remind_screen_param;

    if (app_remind_param.ui_screen_stack_count >= APP_REMIND_UI_WIN_STACK_COUNT) {
        __inf("error app_remind_param.ui_screen_stack_index:%d",app_remind_param.ui_screen_stack_count);
        return;
        }

    p_remind_screen_param = &app_remind_param.ui_screen_stack[app_remind_param.ui_screen_stack_count];

    switch (p_enter_param->win_type)
    {
        case REMIND_APP_WIN_idle_news:
            p_remind_screen_param->init = ui_ScreenRemindNews_screen_init;
            p_remind_screen_param->uninit = (void *) ui_ScreenRemindNews_screen_uninit;
            p_remind_screen_param->win_type = p_enter_param->win_type;
            break;

        case REMIND_APP_WIN_low_power:
            p_remind_screen_param->init = ui_FpLowPower_screen_init;
            p_remind_screen_param->uninit = (void *) ui_FpLowPower_screen_uninit;
            p_remind_screen_param->win_type = p_enter_param->win_type;
            break;

        default:
            p_remind_screen_param->init = ui_ScreenRemindNews_screen_init;
            p_remind_screen_param->uninit = (void *) ui_ScreenRemindNews_screen_uninit;
            p_remind_screen_param->win_type = REMIND_APP_WIN_idle_news;
            break;
        }

    p_remind_screen_param->ui = p_remind_screen_param->init(p_param);
    app_remind_param.ui_screen_stack_count++;


}


void remind_fp_app_deacactive(lv_obj_t * app,void * p_param)
{
    remind_screen_param_t * p_remind_screen_param;

    for (uint8_t i = 0; i < app_remind_param.ui_screen_stack_count; i++) {
        p_remind_screen_param = &app_remind_param.ui_screen_stack[i];
        p_remind_screen_param->uninit(p_remind_screen_param->ui);
        }

    app_remind_param.ui_screen_stack_count = 0;
    memset(app_remind_param.ui_screen_stack,0,sizeof(remind_screen_param_t) *APP_REMIND_UI_WIN_STACK_COUNT);

}


void remind_fp_app_focus(lv_obj_t * app,void * p_param)
{

    remind_screen_param_t * p_remind_screen_param;

    p_remind_screen_param = &app_remind_param.ui_screen_stack[app_remind_param.ui_screen_stack_count - 1];

    lv_disp_load_scr(p_remind_screen_param->ui);

}


void remind_fp_app_defocus(lv_obj_t * app,void * p_param)
{


}


void remind_fp_win_active_focus(void * p_param)
{
    remind_app_enter_param_t * p_enter_param = p_param;

    remind_screen_param_t * p_remind_screen_param;

    remind_screen_param_t remind_screen_param_tmp;

    memset(&remind_screen_param_tmp,0,sizeof(remind_screen_param_t));

    for (uint8_t i = 0; i < app_remind_param.ui_screen_stack_count; i++) {
        p_remind_screen_param = &app_remind_param.ui_screen_stack[i];

        if (p_remind_screen_param->win_type == p_enter_param->win_type) {
            memcpy(&remind_screen_param_tmp,p_remind_screen_param,sizeof(remind_screen_param_t));

            if (app_remind_param.ui_screen_stack_count > i + 1) {
                memcpy(&app_remind_param.ui_screen_stack[i],&app_remind_param.ui_screen_stack[i + 1],
                    sizeof(remind_screen_param_t) * (app_remind_param.ui_screen_stack_count - i - 1));

                }

            app_remind_param.ui_screen_stack_count--;
            memset(&app_remind_param.ui_screen_stack[app_remind_param.ui_screen_stack_count],0,
                sizeof(remind_screen_param_t));
            break;
            }

        }

    if (app_remind_param.ui_screen_stack_count >= APP_REMIND_UI_WIN_STACK_COUNT) {
        __inf("error app_remind_param.ui_screen_stack_index:%d",app_remind_param.ui_screen_stack_count);
        return;
        }

    p_remind_screen_param = &app_remind_param.ui_screen_stack[app_remind_param.ui_screen_stack_count];

    switch (p_enter_param->win_type)
    {
        case REMIND_APP_WIN_idle_news:
            p_remind_screen_param->init = ui_ScreenRemindNews_screen_init;
            p_remind_screen_param->uninit = (void *) ui_ScreenRemindNews_screen_uninit;
            p_remind_screen_param->win_type = p_enter_param->win_type;
            break;

        case REMIND_APP_WIN_low_power:
            p_remind_screen_param->init = ui_FpLowPower_screen_init;
            p_remind_screen_param->uninit = (void *) ui_FpLowPower_screen_uninit;
            p_remind_screen_param->win_type = p_enter_param->win_type;
            break;

        default:
            p_remind_screen_param->init = ui_ScreenRemindNews_screen_init;
            p_remind_screen_param->uninit = (void *) ui_ScreenRemindNews_screen_uninit;
            p_remind_screen_param->win_type = REMIND_APP_WIN_idle_news;
            break;
        }

    p_remind_screen_param->ui = p_remind_screen_param->init(p_param);
    app_remind_param.ui_screen_stack_count++;

    lv_disp_load_scr(p_remind_screen_param->ui);

    if (remind_screen_param_tmp.ui)
        remind_screen_param_tmp.uninit(remind_screen_param_tmp.ui);

}


void remind_fp_win_exit_current_ui()
{

    remind_screen_param_t * p_remind_screen_param;

    if (app_remind_param.ui_screen_stack_count > 1) {
        p_remind_screen_param = &app_remind_param.ui_screen_stack[app_remind_param.ui_screen_stack_count - 1];

        p_remind_screen_param->uninit(p_remind_screen_param->ui);
        app_remind_param.ui_screen_stack_count--;
        memset(&app_remind_param.ui_screen_stack[app_remind_param.ui_screen_stack_count],0,
            sizeof(remind_screen_param_t));

        p_remind_screen_param = &app_remind_param.ui_screen_stack[app_remind_param.ui_screen_stack_count - 1];


        lv_disp_load_scr(p_remind_screen_param->ui);
        }
    else {
        alv_am_app_exit(NULL,REMIND_FP_APP_NAME,NULL);
        }

}



void remind_fp_init(void)
{

    /* Create APP for idle */
    lv_obj_t *      app_remind = alv_am_app_create(lv_scr_act(),REMIND_FP_APP_NAME);

    alv_am_app_set_active(app_remind,remind_fp_app_active);
    alv_am_app_set_deactive(app_remind,remind_fp_app_deacactive);
    alv_am_app_set_focus(app_remind,remind_fp_app_focus);
    alv_am_app_set_defocus(app_remind,remind_fp_app_defocus);


    memset(&app_remind_param,0,sizeof(app_remind_param));
}


void remind_fp_reset_sms_count()
{
    app_remind_param.sms_count = 0;
}


void remind_fp_reset_call_count()
{
    app_remind_param.call_count = 0;
}


uint8_t remind_fp_get_sms_count()
{
    return app_remind_param.sms_count;
}


uint8_t remind_fp_get_call_count()
{
    return app_remind_param.call_count;
}


static int32_t win_news_show_action(void * p_param,uint32_t size)
{
    remind_app_enter_param_t * p_enter_param;

    p_enter_param       = p_param;

    if (alv_am_app_is_exist(REMIND_FP_APP_NAME)) {
        remind_fp_win_active_focus(p_param);
        }
    else {
        if (alv_am_app_is_exist("call")) {
            alv_am_app_enter_defocus_action(NULL,REMIND_FP_APP_NAME,p_enter_param);
            }
        else {
            alv_am_app_enter_action(NULL,REMIND_FP_APP_NAME,p_enter_param);
            }
        }

    return 0;
}



/*news_type:
            0:短信
            1：未接来电

*/
int32_t win_news_show(uint8_t news_type,uint32_t count)
{
    remind_app_enter_param_t * p_enter_param;

    if (news_type == 0)
        app_remind_param.sms_count = count;
    else if (news_type == 1)
        app_remind_param.call_count = count;

    p_enter_param       = aic_malloc(sizeof(remind_app_enter_param_t));

    p_enter_param->win_enter_param.news.type = news_type;

    p_enter_param->win_enter_param.news.count = count;

    if (news_type <= 1)
        p_enter_param->win_type = REMIND_APP_WIN_idle_news;
    else 
        p_enter_param->win_type = REMIND_APP_WIN_low_power;

    app_msg_send_callback(win_news_show_action,p_enter_param,sizeof(remind_app_enter_param_t));

    return 0;
}


int32_t box_bat_remind_show(uint8_t news_type,uint32_t count)
{
    uint8_t *       str;

    switch (news_type)
    {
        case low_power_shutdown:
            str = "电池已耗尽,将自动关机";
            create_comp_msgbox_remind(str);
            app_fp_play_tts_low_bat(str);
            break;

        case low_power_warning:
            str = "电池电量低,请充电";
            create_comp_msgbox_remind(str);
            app_fp_play_tts_low_bat(str);
            break;

        case usb_plug_in:
            str = "充电器已插入";
            create_comp_msgbox_remind(str);
            app_fp_play_tts_charge(str);
            break;

        case usb_plug_out:
            str = "充电器已拔出";
            create_comp_msgbox_remind(str);
            app_fp_play_tts_charge(str);
            break;

        case charge_full:
            str = "电池已充满";
            create_comp_msgbox_remind(str);
            app_fp_play_tts_low_bat(str);
            break;
        }

    return 0;
}




