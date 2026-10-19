
#include "lvgl.h"

#include "alv_app_mgr.h"

#include "app_log.h"
#include "backlight.h"


#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "app_calllog_list.h"
#include "app_log.h"
#include "calllog_data.h"

#define __inf                   APP_LOGI

extern void ui_ScreenCalllogMenu_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenCalllogDialCalls_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenCalllogDel_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenDialedCallsOption_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenCalllogDelFile_screen_focus(lv_obj_t * win,calllog_del_file_enter_param_t * p_enter_param);


void calllog_menu_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogMenu_screen_init(win,p_param);

}


void calllog_menu_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogMenu_screen_uninit(win,p_param);
}


void calllog_menu_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenCalllogMenu_screen_focus(win,p_param);
}


void calllog_menu_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


#if 0


void calllog_missed_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogMissedCalls_screen_active(win,p_param);

}


void calllog_missed_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogMissedCalls_screen_deactive(win,p_param);
}


void calllog_missed_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenCalllogMissedCalls_win_focus(win,p_param);

}


void calllog_missed_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


#endif


void calllog_dial_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDialCalls_screen_active(win,p_param);

}


void calllog_dial_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDialCalls_screen_deactive(win,p_param);
}


void calllog_dial_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDialCalls_win_focus(win,p_param);
}


void calllog_dial_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDialCalls_win_defocus(win,p_param);
}


#if 0


void calllog_receive_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogReceviedCalls_screen_active(win,p_param);

}


void calllog_receive_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogReceviedCalls_screen_deactive(win,p_param);
}


void calllog_receive_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogReceviedCalls_win_focus(win,p_param);
}


void calllog_receive_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


#endif


void calllog_del_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDel_screen_init(win,p_param);

}


void calllog_del_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDel_screen_uninit(win,p_param);
}


void calllog_del_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDel_screen_focus(win,p_param);
}


void calllog_del_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



#if 0


void calllog_missed_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenMissedCallsOption_screen_init(win,p_param);

}


void calllog_missed_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenMissedCallsOption_screen_uninit(win,p_param);
}


void calllog_missed_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);
}


void calllog_missed_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


#endif


void calllog_dialed_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenDialedCallsOption_screen_init(win,p_param);

}


void calllog_dialed_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenDialedCallsOption_screen_uninit(win,p_param);
}


void calllog_dialed_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenDialedCallsOption_screen_focus(win,p_param);
}


void calllog_dialed_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



#if 0


void calllog_received_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenReceivedCallsOption_screen_init(win,p_param);

}


void calllog_received_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenReceivedCallsOption_screen_uninit(win,p_param);
}


void calllog_received_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);
}


void calllog_received_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


#endif


void calllog_def_file_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDelFile_screen_init(win,p_param);

}


void calllog_def_file_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDelFile_screen_uninit(win,p_param);
}


void calllog_def_file_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenCalllogDelFile_screen_focus(win,p_param);
}


void calllog_def_file_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}




typedef enum {
    CALLLOG_APP_FIRST_WIN_dial = 1,
} calllog_app_frist_win_enum_t;


typedef union {
    calllog_list_enter_param_t dial;
} calllog_win_enter_param_t;


typedef struct {
    calllog_win_enter_param_t win_enter_param;
    calllog_app_frist_win_enum_t first_win;
} calllog_app_enter_param_t;


void calllog_fp_app_active(lv_obj_t * app,void * p_param)
{
    calllog_app_enter_param_t * p_enter_param = p_param;
    lv_obj_t *      frist_win;

    if (p_enter_param == NULL) {
        frist_win           = get_win_by_name(app,"menu");
        alv_am_app_set_first_win(app,frist_win);
        return;
        }

    switch (p_enter_param->first_win)
    {
        case CALLLOG_APP_FIRST_WIN_dial:
            frist_win = get_win_by_name(app,"dial");
            alv_am_app_set_first_win(app,frist_win);
            break;

        default:
            frist_win = get_win_by_name(app,"menu");
            alv_am_app_set_first_win(app,frist_win);
            break;
        }

}


void calllog_fp_app_deactive(lv_obj_t * app,void * p_param)
{

}





/*
callog->menu->missed	->missed_option	->"app call"
                                        ->"app sms"
                                        ->"app calllog"
                                        ->def_file

            ->dial		->dialed_option	->"app call"
                                        ->"app sms"
                                        ->"app calllog"
                                        ->def_file

            ->receive	->received_option->"app call"
                                        ->"app sms"
                                        ->"app calllog"
                                        ->def_file

            ->del

*/
void app_calllog_fp_init(void)
{

    /* Create APP for calllog */
    lv_obj_t *      app_calllog = alv_am_app_create(lv_scr_act(),CALLLOG_FP_APP_NAME);

    alv_am_app_set_active(app_calllog,calllog_fp_app_active);
    alv_am_app_set_deactive(app_calllog,calllog_fp_app_deactive);

    lv_obj_t *      win_menu = alv_am_win_create(app_calllog,"menu");

    alv_am_win_set_active(win_menu,calllog_menu_fp_win_active);
    alv_am_win_set_deactive(win_menu,calllog_menu_fp_win_deacactive);
    alv_am_win_set_focus(win_menu,calllog_menu_fp_win_focus);
    alv_am_win_set_defocus(win_menu,calllog_menu_fp_win_defocus);
    alv_am_app_set_first_win(app_calllog,win_menu);

#if 0
    lv_obj_t *      win_missed = alv_am_win_create(app_calllog,"missed");

    alv_am_win_set_active(win_missed,calllog_missed_fp_win_active);
    alv_am_win_set_deactive(win_missed,calllog_missed_fp_win_deacactive);
    alv_am_win_set_focus(win_missed,calllog_missed_fp_win_focus);
    alv_am_win_set_defocus(win_missed,calllog_missed_fp_win_defocus);
#endif

    lv_obj_t *      win_dial = alv_am_win_create(app_calllog,"dial");

    alv_am_win_set_active(win_dial,calllog_dial_fp_win_active);
    alv_am_win_set_deactive(win_dial,calllog_dial_fp_win_deacactive);
    alv_am_win_set_focus(win_dial,calllog_dial_fp_win_focus);
    alv_am_win_set_defocus(win_dial,calllog_dial_fp_win_defocus);

#if 0
    lv_obj_t *      win_receive = alv_am_win_create(app_calllog,"receive");

    alv_am_win_set_active(win_receive,calllog_receive_fp_win_active);
    alv_am_win_set_deactive(win_receive,calllog_receive_fp_win_deacactive);
    alv_am_win_set_focus(win_receive,calllog_receive_fp_win_focus);
    alv_am_win_set_defocus(win_receive,calllog_receive_fp_win_defocus);
#endif

    lv_obj_t *      win_del = alv_am_win_create(app_calllog,"del");

    alv_am_win_set_active(win_del,calllog_del_fp_win_active);
    alv_am_win_set_deactive(win_del,calllog_del_fp_win_deacactive);
    alv_am_win_set_focus(win_del,calllog_del_fp_win_focus);
    alv_am_win_set_defocus(win_del,calllog_del_fp_win_defocus);


#if 0
    lv_obj_t *      win_missed_option = alv_am_win_create(app_calllog,"missed_option");

    alv_am_win_set_active(win_missed_option,calllog_missed_option_fp_win_active);
    alv_am_win_set_deactive(win_missed_option,calllog_missed_option_fp_win_deacactive);
    alv_am_win_set_focus(win_missed_option,calllog_missed_option_fp_win_focus);
    alv_am_win_set_defocus(win_missed_option,calllog_missed_option_fp_win_defocus);
#endif

    lv_obj_t *      win_dialed_option = alv_am_win_create(app_calllog,"dialed_option");

    alv_am_win_set_active(win_dialed_option,calllog_dialed_option_fp_win_active);
    alv_am_win_set_deactive(win_dialed_option,calllog_dialed_option_fp_win_deacactive);
    alv_am_win_set_focus(win_dialed_option,calllog_dialed_option_fp_win_focus);
    alv_am_win_set_defocus(win_dialed_option,calllog_dialed_option_fp_win_defocus);

#if 0
    lv_obj_t *      win_received_option = alv_am_win_create(app_calllog,"received_option");

    alv_am_win_set_active(win_received_option,calllog_received_option_fp_win_active);
    alv_am_win_set_deactive(win_received_option,calllog_received_option_fp_win_deacactive);
    alv_am_win_set_focus(win_received_option,calllog_received_option_fp_win_focus);
    alv_am_win_set_defocus(win_received_option,calllog_received_option_fp_win_defocus);
#endif

    lv_obj_t *      win_def_file = alv_am_win_create(app_calllog,"def_file");

    alv_am_win_set_active(win_def_file,calllog_def_file_option_fp_win_active);
    alv_am_win_set_deactive(win_def_file,calllog_def_file_option_fp_win_deacactive);
    alv_am_win_set_focus(win_def_file,calllog_def_file_option_fp_win_focus);
    alv_am_win_set_defocus(win_def_file,calllog_def_file_option_fp_win_defocus);

    calllog_data_init();

}




void * create_app_calllog_enter_param_fisrt_dial_win(uint8_t call_type)
{
    calllog_app_enter_param_t * p_app_enter_param;

    p_app_enter_param   = lv_mem_alloc(sizeof(calllog_app_enter_param_t));

    memset(p_app_enter_param,0,sizeof(calllog_app_enter_param_t));

    p_app_enter_param->win_enter_param.dial.folder_type = call_type;

    p_app_enter_param->first_win = CALLLOG_APP_FIRST_WIN_dial;

    return p_app_enter_param;
}


void destory_app_calllog_enter_param(void * p)
{
    lv_mem_free(p);
}









