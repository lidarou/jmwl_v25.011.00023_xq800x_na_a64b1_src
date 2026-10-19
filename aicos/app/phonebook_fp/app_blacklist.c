
#include "lvgl.h"

#include "alv_app_mgr.h"

#include "app_log.h"
#include "backlight.h"


#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "blacklist.h"
#include "app_blacklist.h"
#include "app_log.h"

#define __inf                   APP_LOGI


extern lv_obj_t * ui_ScreenBlacklist_screen_init(lv_obj_t * win,void * p);
extern void ui_ScreenBlacklist_screen_uninit(lv_obj_t * win,void * p);
extern lv_obj_t * ui_ScreenBlacklistOption_screen_init(lv_obj_t * win,
    ui_screen_blacklist_enter_param_t * p_enter_param);
extern void ui_ScreenBlacklistOption_screen_uninit(lv_obj_t * win,void * p);
extern lv_obj_t * ui_ScreenBlacklistAddNum_screen_init(lv_obj_t * win,
    ui_screen_blacklist_enter_param_t * p_enter_param);
extern void ui_ScreenBlacklistAddNum_screen_uninit(lv_obj_t * win,void * param);
extern void ui_ScreenBlacklistAddNum_screen_focus(lv_obj_t * win,void * enter_param);
extern void ui_ScreenBlacklistAddNum_screen_defocus(lv_obj_t * win,void * enter_param);
extern lv_obj_t * ui_ScreenBlacklistDelNum_screen_init(lv_obj_t * win,
    ui_screen_blacklist_enter_param_t * p_enter_param);
extern void ui_ScreenBlacklistDelNum_screen_uninit(lv_obj_t * win,ui_screen_blacklist_enter_param_t * p_enter_param);
extern void ui_ScreenBlacklistFind_screen_init(lv_obj_t * win,
    ui_screen_blacklist_find_num_enter_param_t * p_enter_param);
extern void ui_ScreenBlacklistFind_screen_uninit(lv_obj_t * win,void * p_enter_param);
extern lv_obj_t * ui_ScreenBlacklistAddNumRemind_screen_init(lv_obj_t * win,
    ui_screen_blacklist_add_num_enter_param_t * p_enter_param);
extern void ui_ScreenBlacklistAddNumRemind_screen_uninit(lv_obj_t * win,
    ui_screen_blacklist_add_num_enter_param_t * p_enter_param);
extern void ui_ScreenBlacklist_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenBlacklistOption_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenBlacklistDelNum_screen_focus(lv_obj_t * win,ui_screen_blacklist_enter_param_t * p_enter_param);
extern void ui_ScreenBlacklistFind_screen_focus(lv_obj_t * win,void * p_enter_param);
extern void ui_ScreenBlacklistAddNumRemind_screen_focus(lv_obj_t * win,
    ui_screen_blacklist_add_num_enter_param_t * p_enter_param);


void blacklist_list_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklist_screen_init(win,p_param);

}


void blacklist_list_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklist_screen_uninit(win,p_param);
}


void blacklist_list_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenBlacklist_screen_focus(win,p_param);

}


void blacklist_list_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void blacklist_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistOption_screen_init(win,p_param);

}


void blacklist_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistOption_screen_uninit(win,p_param);
}


void blacklist_option_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenBlacklistOption_screen_focus(win,p_param);

}


void blacklist_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}




void blacklist_add_num_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistAddNum_screen_init(win,p_param);

}


void blacklist_add_num_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistAddNum_screen_uninit(win,p_param);
}


void blacklist_add_num_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenBlacklistAddNum_screen_focus(win,p_param);

}


void blacklist_add_num_fp_win_defocus(lv_obj_t * win,void * p_param)
{

    ui_ScreenBlacklistAddNum_screen_defocus(win,p_param);
}



void blacklist_del_num_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistDelNum_screen_init(win,p_param);

}


void blacklist_del_num_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistDelNum_screen_uninit(win,p_param);
}


void blacklist_del_num_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistDelNum_screen_focus(win,p_param);

}


void blacklist_del_num_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}






void blacklist_find_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistFind_screen_init(win,p_param);

}


void blacklist_find_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistFind_screen_uninit(win,p_param);
}


void blacklist_find_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistFind_screen_focus(win,p_param);

}


void blacklist_find_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void blacklist_add_remind_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistAddNumRemind_screen_init(win,p_param);

}


void blacklist_add_remind_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistAddNumRemind_screen_uninit(win,p_param);
}


void blacklist_add_remind_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenBlacklistAddNumRemind_screen_focus(win,p_param);

}


void blacklist_add_remind_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}








typedef enum {
    BLACKLIST_APP_FIRST_WIN_add_num = 1,
    BLACKLIST_APP_FIRST_WIN_del_num,
} blacklist_app_frist_win_enum_t;


typedef union {
    ui_screen_blacklist_add_num_enter_param_t add_num;
    ui_screen_blacklist_enter_param_t del_num;
} blacklist_win_enter_param_t;


typedef struct {
    blacklist_win_enter_param_t win_enter_param;
    blacklist_app_frist_win_enum_t first_win;
} blacklist_app_enter_param_t;


void blacklist_fp_app_active(lv_obj_t * app,void * p_param)
{
    blacklist_app_enter_param_t * p_enter_param = p_param;
    lv_obj_t *      frist_win;

    if (p_enter_param == NULL) {
        frist_win           = get_win_by_name(app,"list");
        alv_am_app_set_first_win(app,frist_win);
        return;
        }

    switch (p_enter_param->first_win)
    {
        case BLACKLIST_APP_FIRST_WIN_add_num:
            frist_win = get_win_by_name(app,"add_remind");
            alv_am_app_set_first_win(app,frist_win);
            break;

        case BLACKLIST_APP_FIRST_WIN_del_num:
            frist_win = get_win_by_name(app,"del_num");
            alv_am_app_set_first_win(app,frist_win);
            break;

        default:
            frist_win = get_win_by_name(app,"list");
            alv_am_app_set_first_win(app,frist_win);
            break;
        }

}


void blacklist_fp_app_deactive(lv_obj_t * app,void * p_param)
{

}









/*
list->option->add_num->find
            ->del_num



add_remind 其他app添加黑名单时效用此提醒界面

*/
void app_blacklist_fp_init(void)
{

    /* Create APP for Idle */
    lv_obj_t *      app_backlist = alv_am_app_create(lv_scr_act(),BACKLIST_FP_APP_NAME);

    alv_am_app_set_active(app_backlist,blacklist_fp_app_active);
    alv_am_app_set_deactive(app_backlist,blacklist_fp_app_deactive);

    lv_obj_t *      win_list = alv_am_win_create(app_backlist,"list");

    alv_am_win_set_active(win_list,blacklist_list_fp_win_active);
    alv_am_win_set_deactive(win_list,blacklist_list_fp_win_deacactive);
    alv_am_win_set_focus(win_list,blacklist_list_fp_win_focus);
    alv_am_win_set_defocus(win_list,blacklist_list_fp_win_defocus);
    alv_am_app_set_first_win(app_backlist,win_list);


    lv_obj_t *      win_option = alv_am_win_create(app_backlist,"option");

    alv_am_win_set_active(win_option,blacklist_option_fp_win_active);
    alv_am_win_set_deactive(win_option,blacklist_option_fp_win_deacactive);
    alv_am_win_set_focus(win_option,blacklist_option_fp_win_focus);
    alv_am_win_set_defocus(win_option,blacklist_option_fp_win_defocus);

    lv_obj_t *      win_add_num = alv_am_win_create(app_backlist,"add_num");

    alv_am_win_set_active(win_add_num,blacklist_add_num_fp_win_active);
    alv_am_win_set_deactive(win_add_num,blacklist_add_num_fp_win_deacactive);
    alv_am_win_set_focus(win_add_num,blacklist_add_num_fp_win_focus);
    alv_am_win_set_defocus(win_add_num,blacklist_add_num_fp_win_defocus);

    lv_obj_t *      win_del_num = alv_am_win_create(app_backlist,"del_num");

    alv_am_win_set_active(win_del_num,blacklist_del_num_fp_win_active);
    alv_am_win_set_deactive(win_del_num,blacklist_del_num_fp_win_deacactive);
    alv_am_win_set_focus(win_del_num,blacklist_del_num_fp_win_focus);
    alv_am_win_set_defocus(win_del_num,blacklist_del_num_fp_win_defocus);

    lv_obj_t *      win_find = alv_am_win_create(app_backlist,"find");

    alv_am_win_set_active(win_find,blacklist_find_fp_win_active);
    alv_am_win_set_deactive(win_find,blacklist_find_fp_win_deacactive);
    alv_am_win_set_focus(win_find,blacklist_find_fp_win_focus);
    alv_am_win_set_defocus(win_find,blacklist_find_fp_win_defocus);

    lv_obj_t *      win_add_remind = alv_am_win_create(app_backlist,"add_remind");

    alv_am_win_set_active(win_add_remind,blacklist_add_remind_fp_win_active);
    alv_am_win_set_deactive(win_add_remind,blacklist_add_remind_fp_win_deacactive);
    alv_am_win_set_focus(win_add_remind,blacklist_add_remind_fp_win_focus);
    alv_am_win_set_defocus(win_add_remind,blacklist_add_remind_fp_win_defocus);

}



void * create_app_blacklist_enter_param_fisrt_add_num(uint8_t * p_number,void * exit_fun)
{
    blacklist_app_enter_param_t * p_app_enter_param;

    p_app_enter_param   = lv_mem_alloc(sizeof(blacklist_app_enter_param_t));

    memset(p_app_enter_param,0,sizeof(blacklist_app_enter_param_t));

    p_app_enter_param->win_enter_param.add_num.p_number = p_number;
    p_app_enter_param->win_enter_param.add_num.exit_fun = exit_fun;

    p_app_enter_param->first_win = BLACKLIST_APP_FIRST_WIN_add_num;

    return p_app_enter_param;
}


void * create_app_blacklist_enter_param_fisrt_del_num(blacklist_data_t * p_blacklist_data,uint32_t focus_index,
    void * exit_fun)
{
    blacklist_app_enter_param_t * p_app_enter_param;

    p_app_enter_param   = lv_mem_alloc(sizeof(blacklist_app_enter_param_t));

    memset(p_app_enter_param,0,sizeof(blacklist_app_enter_param_t));

    p_app_enter_param->win_enter_param.del_num.p_blacklist_data = p_blacklist_data;
    p_app_enter_param->win_enter_param.del_num.focus_index = focus_index;
    p_app_enter_param->win_enter_param.del_num.exit_fun = exit_fun;

    p_app_enter_param->first_win = BLACKLIST_APP_FIRST_WIN_del_num;

    return p_app_enter_param;
}



void destory_app_blacklist_enter_param(void * p,void * exit_fun)
{
    lv_mem_free(p);
}


