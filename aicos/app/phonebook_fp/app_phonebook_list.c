
#include "lvgl.h"

#include "alv_app_mgr.h"

#include "app_log.h"
#include "backlight.h"


#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "app_phonebook_list.h"
#include "app_log.h"
#include "phonebook.h"
#include "call_fp_flow.h"

#define __inf                   APP_LOGI

extern void ui_ScreenPhonebookList_screen_focus(lv_obj_t * win,void * p_enter_param);
extern void ui_ScreenPhonebookedit_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param);
extern lv_obj_t * ui_ScreenPhonebooklist_option_screen_init(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebooklist_option_screen_focus(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern lv_obj_t * ui_ScreenPhonebookstorge_format_screen_init(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebookstorge_format_screen_focus(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebookstorge_format_screen_uninit(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern lv_obj_t * ui_ScreenPhonebookcopy_screen_init(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebookcopy_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param);
extern lv_obj_t * ui_ScreenPhonebookCopySelect_screen_init(lv_obj_t * win,
    ui_screen_phonebook_copy_param_t * copy_param);
extern void ui_ScreenPhonebookCopySelect_screen_uninit(lv_obj_t * win,ui_screen_phonebook_copy_param_t * copy_param);
extern void ui_ScreenPhonebookCopySelect_screen_focus(lv_obj_t * win,ui_screen_phonebook_copy_param_t * copy_param);
extern lv_obj_t * ui_ScreenPhonebooksetup_screen_init(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebooksetup_screen_uninit(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebooksetup_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param);
extern lv_obj_t * ui_ScreenPhonebookstorge_infromation_screen_init(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebookstorge_infromation_screen_uninit(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebookstorge_infromation_screen_focus(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern lv_obj_t * ui_ScreenPhonebooksetup_stroge_screen_init(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebooksetup_stroge_screen_uninit(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebooksetup_stroge_screen_focus(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebookoption_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param);
extern void ui_ScreenPhonebookOptionCopy_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenPhonebookOptionDel_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenPhonebookFind_screen_focus(lv_obj_t * win,void * p_enter_param);
extern void ui_ScreenPhonebookRemindCopy_screen_focus(lv_obj_t * win,void * p_param);
extern lv_obj_t * ui_ScreenPhonebookRemindCopying_screen_init(lv_obj_t * win,void * p_param);
extern void ui_ScreenPhonebookRemindCopying_screen_uninit(lv_obj_t * win,void * p_param);
extern void ui_ScreenPhonebookRemindCopying_screen_focus(lv_obj_t * win,void * p_param);

void phonebook_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookList_screen_focus(win,p_param);

}


void phonebook_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void phonebook_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookList_screen_init(win,p_param);

}


void phonebook_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookList_screen_uninit(win,p_param);

}



void details_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookDetails_screen_init(win,p_param);

}


void details_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookDetails_screen_uninit(win,p_param);

}


void details_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);
}


void details_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void edit_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookedit_screen_init(win,p_param);

}


void edit_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookedit_screen_uninit(win,p_param);

}


void edit_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookedit_screen_focus(win,p_param);

}


void edit_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void list_options_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebooklist_option_screen_init(win,p_param);

}


void list_options_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebooklist_option_screen_uninit(win,p_param);

}


void list_options_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebooklist_option_screen_focus(win,p_param);

}


void list_options_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void remind_fp_win_focus(lv_obj_t * win,void * p_param)
{

    //lv_obj_t *ui_screen = ui_ScreenPhonebooklistRemind_screen_init(p_param);
    // lv_disp_load_scr(ui_screen);
}


void remind_fp_win_defocus(lv_obj_t * win,void * p_param)
{

    //ui_ScreenPhonebooklistRemind_screen_uninit();
}


void storge_format_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookstorge_format_screen_init(win,p_param);

}


void storge_format_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookstorge_format_screen_uninit(win,p_param);

}


void storge_format_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookstorge_format_screen_focus(win,p_param);

}


void storge_format_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void copy_format_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookcopy_screen_init(win,p_param);

}


void copy_format_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookcopy_screen_uninit(win,p_param);

}


void copy_format_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookcopy_screen_focus(win,p_param);
}


void copy_format_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void copy_select_format_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookCopySelect_screen_init(win,p_param);

}


void copy_select_format_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookCopySelect_screen_uninit(win,p_param);

}


void copy_select_format_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookCopySelect_screen_focus(win,p_param);

}


void copy_select_format_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}

void copy_remind_format_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookRemindCopy_screen_init(win,p_param);

}

void copy_remind_format_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookRemindCopy_screen_uninit(win,p_param);

}

void copy_remind_format_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookRemindCopy_screen_focus(win,p_param);

}


void copy_remind_format_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}

void copying_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookRemindCopying_screen_init(win,p_param);

}

void copying_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookRemindCopying_screen_uninit(win,p_param);

}


void copying_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookRemindCopying_screen_focus(win,p_param);
}

void copying_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}

void setup_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebooksetup_screen_init(win,p_param);

}


void setup_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebooksetup_screen_uninit(win,p_param);

}


void setup_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebooksetup_screen_focus(win,p_param);

}


void setup_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void storge_information_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookstorge_infromation_screen_init(win,p_param);

}


void storge_information_fp_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookstorge_infromation_screen_uninit(win,p_param);

}


void storge_information_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookstorge_infromation_screen_focus(win,p_param);
}


void storge_information_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void storge_set_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebooksetup_stroge_screen_init(win,p_param);

}


void storge_set_fp_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebooksetup_stroge_screen_uninit(win,p_param);

}


void storge_set_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebooksetup_stroge_screen_focus(win,p_param);
}


void storge_set_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void option_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookoption_screen_init(win,(ui_screen_phonebook_enter_param_t *) p_param);

}


void option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookoption_screen_uninit(win,p_param);

}


void option_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookoption_screen_focus(win,p_param);

}


void option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void option_copy_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookOptionCopy_screen_init(win,p_param);

}


void option_copy_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookOptionCopy_screen_uninit(win,p_param);

}


void option_copy_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookOptionCopy_screen_focus(win,p_param);

}


void option_copy_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}





void option_del_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookOptionDel_screen_init(win,p_param);

}


void option_del_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookOptionDel_screen_uninit(win,p_param);

}


void option_del_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookOptionDel_screen_focus(win,p_param);

}


void option_del_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}






void pb_find_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookFind_screen_init(win,p_param);

}


void pb_find_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenPhonebookFind_screen_uninit(win,p_param);

}


void pb_find_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPhonebookFind_screen_focus(win,p_param);

}


void pb_find_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}










typedef enum {
    PHONEBOOK_APP_FIRST_WIN_storge_format = 1,
} phonebook_app_frist_win_enum_t;

typedef union {
    ui_screen_phonebook_enter_param_t phonebook;
} phonebook_win_enter_param_t;

typedef struct {
    phonebook_win_enter_param_t win_enter_param;
    phonebook_app_frist_win_enum_t first_win;
} phonebook_app_enter_param_t;

void phonebook_fp_app_active(lv_obj_t * app,void * p_param)
{
    phonebook_app_enter_param_t * p_enter_param = p_param;
    lv_obj_t *      frist_win;

    if (p_enter_param == NULL) {
        frist_win           = get_win_by_name(app,"pb");
        alv_am_app_set_first_win(app,frist_win);
        return;
        }

    switch (p_enter_param->first_win)
    {
        case PHONEBOOK_APP_FIRST_WIN_storge_format:
            frist_win = get_win_by_name(app,"storge_format");
            alv_am_app_set_first_win(app,frist_win);
            break;

        default:
            frist_win = get_win_by_name(app,"pb");
            alv_am_app_set_first_win(app,frist_win);
            break;
        }

}


void phonebook_fp_app_deactive(lv_obj_t * app,void * p_param)
{

}








/*
pb->details		->edit			
                ->option->option_copy
                        ->option_del
                
  ->list_option	->storge_format	->edit
                ->pb_find		->>edit
                ->copy			->copy_select
                ->copy_select
                ->setup			->set_storge
                ->storge_inf
*/
void app_phonebook_fp_init(void)
{

    /* Create APP for Idle */
    lv_obj_t *      app_phonebook = alv_am_app_create(lv_scr_act(),PHONEBOOK_FP_APP_NAME);

    alv_am_app_set_active(app_phonebook,phonebook_fp_app_active);
    alv_am_app_set_deactive(app_phonebook,phonebook_fp_app_deactive);

    lv_obj_t *      win_phonebook = alv_am_win_create(app_phonebook,"pb");

    alv_am_win_set_active(win_phonebook,phonebook_fp_win_active);
    alv_am_win_set_deactive(win_phonebook,phonebook_fp_win_deacactive);
    alv_am_win_set_focus(win_phonebook,phonebook_fp_win_focus);
    alv_am_win_set_defocus(win_phonebook,phonebook_fp_win_defocus);
    alv_am_app_set_first_win(app_phonebook,win_phonebook);


    lv_obj_t *      win_details = alv_am_win_create(app_phonebook,"details");

    alv_am_win_set_active(win_details,details_fp_win_active);
    alv_am_win_set_deactive(win_details,details_fp_win_deacactive);
    alv_am_win_set_focus(win_details,details_fp_win_focus);
    alv_am_win_set_defocus(win_details,details_fp_win_defocus);

    lv_obj_t *      win_edit = alv_am_win_create(app_phonebook,"edit");

    alv_am_win_set_active(win_edit,edit_fp_win_active);
    alv_am_win_set_deactive(win_edit,edit_fp_win_deacactive);
    alv_am_win_set_focus(win_edit,edit_fp_win_focus);
    alv_am_win_set_defocus(win_edit,edit_fp_win_defocus);

    lv_obj_t *      win_list_options = alv_am_win_create(app_phonebook,"list_option");

    alv_am_win_set_active(win_list_options,list_options_fp_win_active);
    alv_am_win_set_deactive(win_list_options,list_options_fp_win_deacactive);
    alv_am_win_set_focus(win_list_options,list_options_fp_win_focus);
    alv_am_win_set_defocus(win_list_options,list_options_fp_win_defocus);

    //lv_obj_t *win_remind= alv_am_win_create(app_phonebook, "remind");
    //alv_am_win_set_focus(win_remind, remind_fp_win_focus);
    //alv_am_win_set_defocus(win_remind, remind_fp_win_defocus);
    lv_obj_t *      win_storge_format = alv_am_win_create(app_phonebook,"storge_format");

    alv_am_win_set_active(win_storge_format,storge_format_fp_win_active);
    alv_am_win_set_deactive(win_storge_format,storge_format_fp_win_deacactive);
    alv_am_win_set_focus(win_storge_format,storge_format_fp_win_focus);
    alv_am_win_set_defocus(win_storge_format,storge_format_fp_win_defocus);

    lv_obj_t *      win_copy = alv_am_win_create(app_phonebook,"copy");

    alv_am_win_set_active(win_copy,copy_format_fp_win_active);
    alv_am_win_set_deactive(win_copy,copy_format_fp_win_deacactive);
    alv_am_win_set_focus(win_copy,copy_format_fp_win_focus);
    alv_am_win_set_defocus(win_copy,copy_format_fp_win_defocus);


    lv_obj_t *      win_copy_slect = alv_am_win_create(app_phonebook,"copy_select");

    alv_am_win_set_active(win_copy_slect,copy_select_format_fp_win_active);
    alv_am_win_set_deactive(win_copy_slect,copy_select_format_fp_win_deacactive);
    alv_am_win_set_focus(win_copy_slect,copy_select_format_fp_win_focus);
    alv_am_win_set_defocus(win_copy_slect,copy_select_format_fp_win_defocus);

    lv_obj_t *      win_copy_remind = alv_am_win_create(app_phonebook,"copy_remind");

    alv_am_win_set_active(win_copy_remind,copy_remind_format_fp_win_active);
    alv_am_win_set_deactive(win_copy_remind,copy_remind_format_fp_win_deacactive);
    alv_am_win_set_focus(win_copy_remind,copy_remind_format_fp_win_focus);
    alv_am_win_set_defocus(win_copy_remind,copy_remind_format_fp_win_defocus);

    lv_obj_t *      win_copying = alv_am_win_create(app_phonebook,"copying");

    alv_am_win_set_active(win_copying,copying_fp_win_active);
    alv_am_win_set_deactive(win_copying,copying_fp_win_deacactive);
    alv_am_win_set_focus(win_copying,copying_fp_win_focus);
    alv_am_win_set_defocus(win_copying,copying_fp_win_defocus);

    lv_obj_t *      win_setup = alv_am_win_create(app_phonebook,"setup");

    alv_am_win_set_active(win_setup,setup_fp_win_active);
    alv_am_win_set_deactive(win_setup,setup_fp_win_deacactive);
    alv_am_win_set_focus(win_setup,setup_fp_win_focus);
    alv_am_win_set_defocus(win_setup,setup_fp_win_defocus);


    lv_obj_t *      win_storge_inf = alv_am_win_create(app_phonebook,"storge_inf");

    alv_am_win_set_active(win_storge_inf,storge_information_fp_win_active);
    alv_am_win_set_deactive(win_storge_inf,storge_information_fp_win_deactive);
    alv_am_win_set_focus(win_storge_inf,storge_information_fp_win_focus);
    alv_am_win_set_defocus(win_storge_inf,storge_information_fp_win_defocus);


    lv_obj_t *      win_set_storge = alv_am_win_create(app_phonebook,"set_storge");

    alv_am_win_set_active(win_set_storge,storge_set_fp_win_active);
    alv_am_win_set_deactive(win_set_storge,storge_set_fp_win_deactive);
    alv_am_win_set_focus(win_set_storge,storge_set_fp_win_focus);
    alv_am_win_set_defocus(win_set_storge,storge_set_fp_win_defocus);

    lv_obj_t *      win_option = alv_am_win_create(app_phonebook,"option");

    alv_am_win_set_active(win_option,option_fp_win_active);
    alv_am_win_set_deactive(win_option,option_fp_win_deacactive);
    alv_am_win_set_focus(win_option,option_fp_win_focus);
    alv_am_win_set_defocus(win_option,option_fp_win_defocus);


    lv_obj_t *      win_option_copy = alv_am_win_create(app_phonebook,"option_copy");

    alv_am_win_set_active(win_option_copy,option_copy_fp_win_active);
    alv_am_win_set_deactive(win_option_copy,option_copy_fp_win_deacactive);
    alv_am_win_set_focus(win_option_copy,option_copy_fp_win_focus);
    alv_am_win_set_defocus(win_option_copy,option_copy_fp_win_defocus);

    lv_obj_t *      win_option_del = alv_am_win_create(app_phonebook,"option_del");

    alv_am_win_set_active(win_option_del,option_del_fp_win_active);
    alv_am_win_set_deactive(win_option_del,option_del_fp_win_deacactive);
    alv_am_win_set_focus(win_option_del,option_del_fp_win_focus);
    alv_am_win_set_defocus(win_option_del,option_del_fp_win_defocus);


    lv_obj_t *      win_pb_find = alv_am_win_create(app_phonebook,"pb_find");

    alv_am_win_set_active(win_pb_find,pb_find_fp_win_active);
    alv_am_win_set_deactive(win_pb_find,pb_find_fp_win_deacactive);
    alv_am_win_set_focus(win_pb_find,pb_find_fp_win_focus);
    alv_am_win_set_defocus(win_pb_find,pb_find_fp_win_defocus);

    call_fp_flow_call_status_init();
}


void * create_app_phonebook_enter_param_fisrt_write_win(uint8_t * name,uint8_t * p_number)
{
    phonebook_app_enter_param_t * p_app_enter_param;

    p_app_enter_param   = lv_mem_alloc(sizeof(phonebook_app_enter_param_t));

    memset(p_app_enter_param,0,sizeof(phonebook_app_enter_param_t));

    if (name)
        strncpy(p_app_enter_param->win_enter_param.phonebook.phonebook_param.name,name,PHONEBOOK_NAME_LEN);

    if (p_number)
        strncpy(p_app_enter_param->win_enter_param.phonebook.phonebook_param.number,p_number,PHONEBOOK_NUMBER_LEN);

    p_app_enter_param->first_win = PHONEBOOK_APP_FIRST_WIN_storge_format;

    return p_app_enter_param;
}


void destory_app_phonebook_enter_param(void * p)
{
    lv_mem_free(p);
}













