
#include "lvgl.h"

#include "alv_app_mgr.h"

#include "app_log.h"
#include "backlight.h"


#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "app_audio.h"
#include "home_fp.h"
#include "screen_remind.h"
#include "phonebook.h"


static uint8_t  g_cur_win;

/**
 * home_fp_init() - the init of home fp app.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
typedef void(*home_app_win_init_t) (void);
typedef void(*home_app_win_enter_sub_t) (void);


typedef struct _HOME_APP_WIN_MENU_PARAM_H_ {
    lv_obj_t * *    win_p;
    home_app_win_init_t init_fun_p;
    home_app_win_init_t uninit_fun_p;
    char *          enter_subapp_name;
    home_app_win_enter_sub_t enter_sub_fun;
} home_app_win_menu_param_t;


void remind_timer_callback_phonebooklist(screen_remind_param_t * p_param)
{
    ui_ScreenPhonebook_screen_init();
    lv_disp_load_scr(ui_ScreenPhonebook);

}


extern void settings_ui_init(void);


void ui_ScreenPhonebook_screen_enter_sub_fun()
{
    if (phonebook_data_is_init()) {
        alv_am_app_enter_action(NULL,"phonebook",NULL);
        }
    else {
        screen_remind_param_t screen_remind_param;
        lv_obj_t *      ui_screen;

        ui_screen_remind_param_init(&screen_remind_param);

        screen_remind_param.str = "联系人未准备好";
        screen_remind_param.tiemr_delay = 1700;
        screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_phonebooklist;

        ui_screen           = ui_screen_remind_init(&screen_remind_param);

        lv_disp_load_scr(ui_screen);
        }

}


home_app_win_menu_param_t home_app_win_menu_param[] =
{
    {
        &ui_ScreenPhonebook,ui_ScreenPhonebook_screen_init,ui_ScreenPhonebook_screen_uninit,"phonebook",
        ui_ScreenPhonebook_screen_enter_sub_fun
        },
    {
        &ui_ScreenMessage,ui_ScreenMessage_screen_init,ui_ScreenMessage_screen_uninit,"sms",NULL
        },
    {
        &ui_ScreenCalllog,ui_ScreenCalllog_screen_init,ui_ScreenCalllog_screen_uninit,"calllog",NULL
        },
    {

        &ui_ScreenMultimedia,ui_ScreenMultimedia_screen_init,ui_ScreenMultimedia_screen_uninit,"multmedia" /*"
                            musicplayer"*/
        ,NULL
        },
    {
        &ui_ScreenToolkit,ui_ScreenToolkit_screen_init,ui_ScreenToolkit_screen_uninit,"tools",NULL
        },
    {
        &ui_ScreenSetting,ui_ScreenSetting_screen_init,ui_ScreenSetting_screen_uninit,"settings",NULL
        },
};


void home_fp_app_focus(lv_obj_t * app,void * p_param)
{

}


void home_fp_app_defocus(lv_obj_t * app,void * p_param)
{


}


void home_fp_app_active(lv_obj_t * app,void * p_param)
{
    g_cur_win           = 0;
}


void home_fp_app_deacactive(lv_obj_t * app,void * p_param)
{
}


void home_fp_win_focus(lv_obj_t * win,void * p_param)
{
    home_app_win_menu_param[g_cur_win].init_fun_p();

    lv_disp_load_scr(*home_app_win_menu_param[g_cur_win].win_p);

}


void home_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    home_app_win_menu_param[g_cur_win].uninit_fun_p();
}


void home_fp_win_active(lv_obj_t * win,void * p_param)
{

}


void home_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
}




void home_fp_init(void)
{

    /* Create APP for home */
    lv_obj_t *      app_home = alv_am_app_create(lv_scr_act(),HOME_FP_APP_NAME);

    alv_am_app_set_active(app_home,home_fp_app_active);
    alv_am_app_set_deactive(app_home,home_fp_app_deacactive);
    alv_am_app_set_focus(app_home,home_fp_app_focus);
    alv_am_app_set_defocus(app_home,home_fp_app_defocus);

    lv_obj_t *      win_fisrt = alv_am_win_create(app_home,"home");

    alv_am_win_set_active(win_fisrt,home_fp_win_active);
    alv_am_win_set_deactive(win_fisrt,home_fp_win_deacactive);
    alv_am_win_set_focus(win_fisrt,home_fp_win_focus);
    alv_am_win_set_defocus(win_fisrt,home_fp_win_defocus);
    alv_am_app_set_first_win(app_home,win_fisrt);

    ui_ScreenPhonebook  = NULL;
    ui_ScreenCalllog    = NULL;
    ui_ScreenMessage    = NULL;
    ui_ScreenMultimedia = NULL;
    ui_ScreenToolkit    = NULL;
    ui_ScreenSetting    = NULL;

}


void home_app_win_next(lv_obj_t * cur_win)
{
    int32_t         win_count;
    int32_t         index,index_pre;

    if (cur_win == NULL)
        return;

    win_count           = sizeof(home_app_win_menu_param) / sizeof(home_app_win_menu_param[0]);

    for (index = 0; index < win_count; index++) {
        if (*home_app_win_menu_param[index].win_p == cur_win)
            break;
        }

    index_pre           = index;
    index++;

    if (index >= win_count)
        index = 0;

    _ui_screen_change(home_app_win_menu_param[index].win_p,LV_SCR_LOAD_ANIM_FADE_ON,10,0,
        home_app_win_menu_param[index].init_fun_p);

    g_cur_win           = index;

    //if(index_pre<win_count)
    //	home_app_win_uninit_fun_p[index_pre]();
}


void home_app_win_pre(lv_obj_t * cur_win)
{
    int32_t         win_count;
    int32_t         index,index_pre;

    if (cur_win == NULL)
        return;

    win_count           = sizeof(home_app_win_menu_param) / sizeof(home_app_win_menu_param[0]);

    for (index = 0; index < win_count; index++) {
        if (*home_app_win_menu_param[index].win_p == cur_win)
            break;
        }

    index_pre           = index;

    if (index > 0)
        index--;
    else 
        index = win_count - 1;

    _ui_screen_change(home_app_win_menu_param[index].win_p,LV_SCR_LOAD_ANIM_FADE_ON,10,0,
        home_app_win_menu_param[index].init_fun_p);

    g_cur_win           = index;

    //if(index_pre<win_count)
    //	home_app_win_uninit_fun_p[index_pre]();
}


void home_app_win_uninit(lv_obj_t * cur_win)
{
    int32_t         win_count;
    int32_t         index,index_pre;

    if (cur_win == NULL)
        return;

    win_count           = sizeof(home_app_win_menu_param) / sizeof(home_app_win_menu_param[0]);

    for (index = 0; index < win_count; index++) {
        if (*home_app_win_menu_param[index].win_p == cur_win)
            break;
        }

    index_pre           = index;

    //index++;
    //if(index>=win_count)
    //	index = 0;
    if (index_pre < win_count)
        home_app_win_menu_param[index_pre].uninit_fun_p();
}


void home_app_win_enter_subapp(lv_obj_t * cur_win)
{
    {
        int32_t         win_count;
        int32_t         index;

        if (cur_win == NULL)
            return;

        win_count           = sizeof(home_app_win_menu_param) / sizeof(home_app_win_menu_param[0]);

        for (index = 0; index < win_count; index++) {
            if (*home_app_win_menu_param[index].win_p == cur_win)
                break;
            }

        APP_LOGI("home_app_win_enter_subapp:index:%d,home_app_win_menu_param[index].enter_subapp_name:%s",index,
            home_app_win_menu_param[index].enter_subapp_name);

        if (index >= win_count)
            return;

        if (home_app_win_menu_param[index].enter_sub_fun)
            home_app_win_menu_param[index].enter_sub_fun();
        else if (home_app_win_menu_param[index].enter_subapp_name)
            alv_am_app_enter_action(NULL,home_app_win_menu_param[index].enter_subapp_name,NULL);


        }
}


void home_app_win_key_action(uint32_t key,void * param)
{
    switch (key)
    {
        case LV_KEY_UP:
        case LV_KEY_RIGHT:
            home_app_win_next(param);
            break;

        case LV_KEY_DOWN:
        case LV_KEY_LEFT:
            home_app_win_pre(param);
            break;

        case LV_KEY_ENTER:
        case LV_KEY_ESC:
            home_app_win_enter_subapp(param);
            break;

        case LV_KEY_DEL:
            alv_am_app_exit_action(NULL,"home_fp",NULL);
            break;

        case LV_KEY_END:
            alv_am_app_exit_action_all();
            break;
        }
}


