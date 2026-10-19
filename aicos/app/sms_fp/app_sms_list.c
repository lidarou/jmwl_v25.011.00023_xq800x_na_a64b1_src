
#include "lvgl.h"

#include "alv_app_mgr.h"

#include "app_log.h"
#include "backlight.h"


#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "app_sms_list.h"
#include "app_log.h"

#define __inf                   APP_LOGI

extern void ui_ScreenSmsMenu_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenSmsWriteOption_screen_focus(lv_obj_t * win,void * enter_param);
extern void ui_ScreenSmsWritePhoneNumFind_screen_focus(lv_obj_t * win,void * p_param);
extern void ui_ScreenSmsSendOptionFind_screen_focus(lv_obj_t * win,void * p_param);
extern void ui_ScreenSmsSelectPhoneNum_screen_focus(lv_obj_t * win,void * p_param);
extern void ui_ScreenSmsInsertOption_screen_focus(lv_obj_t * win,void * p_param);
extern void ui_ScreenSmsListDel_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenSmsDetailRevOption_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenSmsDetailRevOptionDel_screen_focus(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsDetailSendOption_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenSmsDetailSendOptionDel_screen_focus(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsDetailDraftOption_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenSmsDetailDraftOptionDel_screen_focus(lv_obj_t * win,
    sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsDelAllRead_screen_focus(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsDelRevAll_screen_focus(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsDelRevRead_screen_focus(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsDelDraftRead_screen_focus(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsDelSendRead_screen_focus(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsListDelRev_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenSmsDelSelect_screen_focus(lv_obj_t * win,sms_detail_ui_enter_param_t * p_enter_param);
extern void ui_ScreenSmsListDelSend_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenSmsListDelDraft_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenSmsExtractNumber_screen_focus(lv_obj_t * win,void * p_enter_param);
extern void ui_ScreenSmsExtractNumOption_screen_focus(lv_obj_t * win,void * p_param);
extern lv_obj_t * ui_ScreenSmsSendRemind_screen_init(lv_obj_t * win,void * p_param);
extern void ui_ScreenSmsSendRemind_screen_uninit(lv_obj_t * win,void * p_param);
extern void ui_ScreenSmsSendRemind_screen_focus(lv_obj_t * win,void * p_param);
extern void ui_ScreenSmsListRev_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenSmsListSend_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenSmsListDraft_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenSmsDetailRev_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenSmsDetailSend_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenSmsDetailDraft_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenSmsListDelRevSelect_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenSmsListDelDraftSelect_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenSmsListDelSendSelect_win_defocus(lv_obj_t * win,void * param);

void sms_menu_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsMenu_screen_init(win,p_param);

}


void sms_menu_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsMenu_screen_uninit(win,p_param);
}



void sms_menu_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsMenu_screen_focus(win,p_param);

}


void sms_menu_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void sms_write_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWrite_screen_init(win,p_param);

}


void sms_write_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWrite_screen_uninit(win,p_param);
}


void sms_write_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWrite_screen_focus(win,p_param);
}


void sms_write_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_write_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWriteOption_screen_init(win,p_param);

}


void sms_write_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWriteOption_screen_uninit(win,p_param);
}


void sms_write_option_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsWriteOption_screen_focus(win,p_param);

}


void sms_write_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_write_phone_num_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWritePhoneNum_screen_init(win,p_param);

}


void sms_write_phone_num_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWritePhoneNum_screen_uninit(win,p_param);
}


void sms_write_phone_num_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsWritePhoneNum_screen_focus(win,p_param);

}


void sms_write_phone_num_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_write_phone_num_find_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWritePhoneNumFind_screen_init(win,p_param);

}


void sms_write_phone_num_find_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsWritePhoneNumFind_screen_uninit(win,p_param);
}



void sms_write_phone_num_find_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsWritePhoneNumFind_screen_focus(win,p_param);
}


void sms_write_phone_num_find_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_send_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsSendOptionFind_screen_init(win,p_param);

}


void sms_send_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsSendOptionFind_screen_uninit(win,p_param);
}


void sms_send_option_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsSendOptionFind_screen_focus(win,p_param);
}


void sms_send_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_select_phone_num_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsSelectPhoneNum_screen_init(win,p_param);

}


void sms_select_phone_num_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsSelectPhoneNum_screen_uninit(win,p_param);
}


void sms_select_phone_num_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsSelectPhoneNum_screen_focus(win,p_param);

}


void sms_select_phone_num_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}

void sms_send_remind_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsSendRemind_screen_init(win,p_param);

}

void sms_send_remind_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsSendRemind_screen_uninit(win,p_param);
}

void sms_send_remind_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsSendRemind_screen_focus(win,p_param);


}

void sms_send_remind_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}

void sms_insert_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsInsertOption_screen_init(win,p_param);

}


void sms_insert_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsInsertOption_screen_uninit(win,p_param);
}


void sms_insert_option_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsInsertOption_screen_focus(win,p_param);
}


void sms_insert_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void sms_insert_word_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsInsertWord_screen_init(win,p_param);

}


void sms_insert_word_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsInsertWord_screen_uninit(win,p_param);
}


void sms_insert_word_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsInsertWord_screen_focus(win,p_param);

}


void sms_insert_word_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_insert_number_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsInsertNum_screen_init(win,p_param);

}


void sms_insert_number_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsInsertNum_screen_uninit(win,p_param);
}


void sms_insert_number_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsInsertNum_screen_focus(win,p_param);

}


void sms_insert_number_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void sms_list_rev_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListRev_screen_active(win,p_param);

}


void sms_list_rev_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListRev_screen_deactive(win,p_param);
}


void sms_list_rev_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsListRev_win_focus(win,p_param);

}


void sms_list_rev_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListRev_win_defocus(win,p_param);
}


void sms_list_send_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListSend_screen_active(win,p_param);
}


void sms_list_send_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListSend_screen_deactive(win,p_param);
}


void sms_list_send_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsListSend_win_focus(win,p_param);

}


void sms_list_send_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListSend_win_defocus(win,p_param);
}


void sms_list_draft_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDraft_screen_active(win,p_param);
}


void sms_list_draft_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDraft_screen_deactive(win,p_param);
}


void sms_list_draft_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsListDraft_win_focus(win,p_param);

}


void sms_list_draft_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDraft_win_defocus(win,p_param);
}



void sms_list_del_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDel_screen_init(win,p_param);
}


void sms_list_del_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDel_screen_uninit(win,p_param);

}


void sms_list_del_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsListDel_screen_focus(win,p_param);
}


void sms_list_del_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}





void sms_detail_rev_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRev_screen_active(win,p_param);
}


void sms_detail_rev_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRev_screen_deactive(win,p_param);
}


void sms_detail_rev_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRev_win_focus(win,p_param);
}


void sms_detail_rev_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRev_win_defocus(win,p_param);
}


void sms_detail_rev_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRevOption_screen_init(win,p_param);
}


void sms_detail_rev_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRevOption_screen_uninit(win,p_param);
}


void sms_detail_rev_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRevOption_screen_focus(win,p_param);
}


void sms_detail_rev_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_detail_rev_option_del_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRevOptionDel_screen_init(win,p_param);
}


void sms_detail_rev_option_del_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRevOptionDel_screen_uninit(win,p_param);
}


void sms_detail_rev_option_del_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRevOptionDel_screen_focus(win,p_param);
}


void sms_detail_rev_option_del_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_detail_rev_option_inf_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen = (lv_obj_t *) ui_ScreenSmsDetailRevOptionInf_screen_init(win,p_param);

    if (ui_screen)
        lv_disp_load_scr(ui_screen);
}


void sms_detail_rev_option_inf_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailRevOptionInf_screen_uninit(win,p_param);
}



void sms_detail_send_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSend_screen_active(win,p_param);
}


void sms_detail_send_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSend_screen_deactive(win,p_param);
}


void sms_detail_send_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSend_win_focus(win,p_param);
}


void sms_detail_send_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSend_win_defocus(win,p_param);
}


void sms_detail_send_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSendOption_screen_init(win,p_param);
}


void sms_detail_send_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSendOption_screen_uninit(win,p_param);
}


void sms_detail_send_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSendOption_screen_focus(win,p_param);
}


void sms_detail_send_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_detail_send_option_del_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSendOptionDel_screen_init(win,p_param);
}


void sms_detail_send_option_del_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSendOptionDel_screen_uninit(win,p_param);
}


void sms_detail_send_option_del_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSendOptionDel_screen_focus(win,p_param);
}


void sms_detail_send_option_del_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void sms_detail_send_option_inf_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen = (lv_obj_t *) ui_ScreenSmsDetailSendOptionInf_screen_init(win,p_param);

    if (ui_screen)
        lv_disp_load_scr(ui_screen);
}


void sms_detail_send_option_inf_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailSendOptionInf_screen_uninit(win,p_param);
}






void sms_detail_draft_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraft_screen_active(win,p_param);
}


void sms_detail_draft_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraft_screen_deactive(win,p_param);
}


void sms_detail_draft_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraft_win_focus(win,p_param);
}


void sms_detail_draft_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraft_win_defocus(win,p_param);
}


void sms_detail_draft_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraftOption_screen_init(win,p_param);
}


void sms_detail_draft_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraftOption_screen_uninit(win,p_param);
}


void sms_detail_draft_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraftOption_screen_focus(win,p_param);
}


void sms_detail_draft_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_detail_draft_option_del_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraftOptionDel_screen_init(win,p_param);
}


void sms_detail_draft_option_del_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraftOptionDel_screen_uninit(win,p_param);
}


void sms_detail_draft_option_del_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDetailDraftOptionDel_screen_focus(win,p_param);
}


void sms_detail_draft_option_del_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void sms_del_all_read_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelAllRead_screen_init(win,p_param);
}


void sms_del_all_read_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelAllRead_screen_uninit(win,p_param);
}


void sms_del_all_read_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelAllRead_screen_focus(win,p_param);
}


void sms_del_all_read_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void sms_del_rev_all_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelRevAll_screen_init(win,p_param);
}


void sms_del_rev_all_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelRevAll_screen_uninit(win,p_param);
}


void sms_del_rev_all_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelRevAll_screen_focus(win,p_param);
}


void sms_del_rev_all_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}




void sms_del_rev_read_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelRevRead_screen_init(win,p_param);
}


void sms_del_rev_read_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelRevRead_screen_uninit(win,p_param);
}


void sms_del_rev_read_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelRevRead_screen_focus(win,p_param);
}


void sms_del_rev_read_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}



void sms_del_draft_read_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelDraftRead_screen_init(win,p_param);
}


void sms_del_draft_read_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelDraftRead_screen_uninit(win,p_param);
}


void sms_del_draft_read_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelDraftRead_screen_focus(win,p_param);
}


void sms_del_draft_read_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void sms_del_send_read_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelSendRead_screen_init(win,p_param);
}


void sms_del_send_read_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelSendRead_screen_uninit(win,p_param);
}


void sms_del_send_read_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelSendRead_screen_focus(win,p_param);
}


void sms_del_send_read_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}






void sms_list_del_rev_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelRev_screen_init(win,p_param);
}


void sms_list_del_rev_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelRev_screen_uninit(win,p_param);
}


void sms_list_del_rev_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelRev_screen_focus(win,p_param);
}


void sms_list_del_rev_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void sms_list_del_rev_sel_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelRevSelect_screen_active(win,p_param);

}


void sms_list_del_rev_sel_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelRevSelect_screen_deactive(win,p_param);
}


void sms_list_del_rev_sel_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsListDelRevSelect_win_focus(win,p_param);

}


void sms_list_del_rev_sel_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelRevSelect_win_defocus(win,p_param);
}


void sms_del_select_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelSelect_screen_init(win,p_param);
}


void sms_del_select_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelSelect_screen_uninit(win,p_param);
}


void sms_del_select_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsDelSelect_screen_focus(win,p_param);
}


void sms_del_select_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void sms_list_del_send_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelSend_screen_init(win,p_param);
}


void sms_list_del_send_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelSend_screen_uninit(win,p_param);
}


void sms_list_del_send_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelSend_screen_focus(win,p_param);
}


void sms_list_del_send_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}



void sms_list_del_draft_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelDraft_screen_init(win,p_param);
}


void sms_list_del_draft_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelDraft_screen_uninit(win,p_param);
}


void sms_list_del_draft_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelDraft_screen_focus(win,p_param);
}


void sms_list_del_draft_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}




void sms_list_del_draft_sel_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelDraftSelect_screen_active(win,p_param);

}


void sms_list_del_draft_sel_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelDraftSelect_screen_deactive(win,p_param);
}


void sms_list_del_draft_sel_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsListDelDraftSelect_win_focus(win,p_param);

}


void sms_list_del_draft_sel_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelDraftSelect_win_defocus(win,p_param);
}


void sms_list_del_send_sel_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelSendSelect_screen_active(win,p_param);

}


void sms_list_del_send_sel_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelSendSelect_screen_deactive(win,p_param);
}


void sms_list_del_send_sel_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenSmsListDelSendSelect_win_focus(win,p_param);

}


void sms_list_del_send_sel_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsListDelSendSelect_win_defocus(win,p_param);
}



void sms_extract_num_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumber_screen_init(win,p_param);
}


void sms_extract_num_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumber_screen_uninit(win,p_param);
}


void sms_extract_num_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumber_screen_focus(win,p_param);
}


void sms_extract_num_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}



void sms_extract_num_edit_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumEdit_screen_init(win,p_param);
}


void sms_extract_num_edit_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumEdit_screen_uninit(win,p_param);
}


void sms_extract_num_edit_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumEdit_screen_focus(win,p_param);
}


void sms_extract_num_edit_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void sms_extract_num_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumOption_screen_init(win,p_param);
}


void sms_extract_num_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumOption_screen_uninit(win,p_param);
}


void sms_extract_num_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenSmsExtractNumOption_screen_focus(win,p_param);
}


void sms_extract_num_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


static void sms_app_active(lv_obj_t * app,void * p_param)
{
    sms_app_enter_param_t * p_enter_param = p_param;
    lv_obj_t *      frist_win;

    if (p_enter_param == NULL) {
        frist_win           = get_win_by_name(app,"menu");
        alv_am_app_set_first_win(app,frist_win);
        return;
        }

    switch (p_enter_param->first_win)
    {
        case SMS_APP_FIRST_WIN_WRITE:
            frist_win = get_win_by_name(app,"write");
            alv_am_app_set_first_win(app,frist_win);
            break;

        case SMS_APP_FIRST_WIN_REV:
            frist_win = get_win_by_name(app,"list_rev");
            alv_am_app_set_first_win(app,frist_win);
            break;

        default:
            frist_win = get_win_by_name(app,"menu");
            alv_am_app_set_first_win(app,frist_win);
            break;
        }

}


static void sms_app_deactive(lv_obj_t * app,void * p_param)
{



}






/*
menu->write		->write_option	->write_num			->write_num_find
                                ->send_option		->select_phone_num->sms_send_remind
                                ->insert_option		->insert_word
                                                    ->insert_number

                                                
    ->list_rev	->detail_rev	->detail_rev_option	->detail_rev_op_del
                                                    ->write
                                                    ->extract_num		->extract_num_edit -> extract_num_option
                                                    ->detail_rev_op_inf

                                                    
    ->list_send	->detail_send	->detail_send_option->detail_send_op_del
                                                    ->write
                                                    ->extract_num		->extract_num_edit -> extract_num_option
                                                    ->detail_send_op_inf


    ->list_draft->detail_draft	->detail_draft_option	->detail_draft_op_del
                                                        ->write
                                                        ->extract_num	->extract_num_edit -> extract_num_option
                                                        ->detail_send_op_inf

    ->list_del	->del_all_read
                ->list_del_rev	->list_del_rev_sel	->del_select
                                                    ->del_rev_read
                                                    ->del_rev_all
                                                    
                ->list_del_send	->list_del_send_sel	->del_select
                                                    ->del_send_read
                                            
                ->list_del_draft->list_del_draft_sel->del_select
                                                    ->del_draft_read
*/
void app_sms_fp_init(void)
{

    /* Create APP for Idle */
    lv_obj_t *      app_sms = alv_am_app_create(lv_scr_act(),SMS_FP_APP_NAME);

    alv_am_app_set_active(app_sms,sms_app_active);
    alv_am_app_set_deactive(app_sms,sms_app_deactive);


    lv_obj_t *      win_menu = alv_am_win_create(app_sms,"menu");

    alv_am_win_set_active(win_menu,sms_menu_fp_win_active);
    alv_am_win_set_deactive(win_menu,sms_menu_fp_win_deacactive);
    alv_am_win_set_focus(win_menu,sms_menu_fp_win_focus);
    alv_am_win_set_defocus(win_menu,sms_menu_fp_win_defocus);
    alv_am_app_set_first_win(app_sms,win_menu);

    lv_obj_t *      win_write = alv_am_win_create(app_sms,"write");

    alv_am_win_set_active(win_write,sms_write_fp_win_active);
    alv_am_win_set_deactive(win_write,sms_write_fp_win_deacactive);
    alv_am_win_set_focus(win_write,sms_write_fp_win_focus);
    alv_am_win_set_defocus(win_write,sms_write_fp_win_defocus);


    lv_obj_t *      win_write_option = alv_am_win_create(app_sms,"write_option");

    alv_am_win_set_active(win_write_option,sms_write_option_fp_win_active);
    alv_am_win_set_deactive(win_write_option,sms_write_option_fp_win_deacactive);
    alv_am_win_set_focus(win_write_option,sms_write_option_fp_win_focus);
    alv_am_win_set_defocus(win_write_option,sms_write_option_fp_win_defocus);


    lv_obj_t *      win_write_num = alv_am_win_create(app_sms,"write_num");

    alv_am_win_set_active(win_write_num,sms_write_phone_num_fp_win_active);
    alv_am_win_set_deactive(win_write_num,sms_write_phone_num_fp_win_deacactive);
    alv_am_win_set_focus(win_write_num,sms_write_phone_num_fp_win_focus);
    alv_am_win_set_defocus(win_write_num,sms_write_phone_num_fp_win_defocus);

    lv_obj_t *      win_write_num_find = alv_am_win_create(app_sms,"write_num_find");

    alv_am_win_set_active(win_write_num_find,sms_write_phone_num_find_fp_win_active);
    alv_am_win_set_deactive(win_write_num_find,sms_write_phone_num_find_fp_win_deacactive);
    alv_am_win_set_focus(win_write_num_find,sms_write_phone_num_find_fp_win_focus);
    alv_am_win_set_defocus(win_write_num_find,sms_write_phone_num_find_fp_win_defocus);


    lv_obj_t *      win_send_option = alv_am_win_create(app_sms,"send_option");

    alv_am_win_set_active(win_send_option,sms_send_option_fp_win_active);
    alv_am_win_set_deactive(win_send_option,sms_send_option_fp_win_deacactive);
    alv_am_win_set_focus(win_send_option,sms_send_option_fp_win_focus);
    alv_am_win_set_defocus(win_send_option,sms_send_option_fp_win_defocus);

    lv_obj_t *      win_select_phone_num = alv_am_win_create(app_sms,"select_phone_num");

    alv_am_win_set_active(win_select_phone_num,sms_select_phone_num_fp_win_active);
    alv_am_win_set_deactive(win_select_phone_num,sms_select_phone_num_fp_win_deacactive);
    alv_am_win_set_focus(win_select_phone_num,sms_select_phone_num_fp_win_focus);
    alv_am_win_set_defocus(win_select_phone_num,sms_select_phone_num_fp_win_defocus);

    lv_obj_t *      win_send_remind = alv_am_win_create(app_sms,"sms_send_remind");

    alv_am_win_set_active(win_send_remind,sms_send_remind_fp_win_active);
    alv_am_win_set_deactive(win_send_remind,sms_send_remind_fp_win_deacactive);
    alv_am_win_set_focus(win_send_remind,sms_send_remind_fp_win_focus);
    alv_am_win_set_defocus(win_send_remind,sms_send_remind_fp_win_defocus);

    lv_obj_t *      win_insert_option = alv_am_win_create(app_sms,"insert_option");

    alv_am_win_set_active(win_insert_option,sms_insert_option_fp_win_active);
    alv_am_win_set_deactive(win_insert_option,sms_insert_option_fp_win_deacactive);
    alv_am_win_set_focus(win_insert_option,sms_insert_option_fp_win_focus);
    alv_am_win_set_defocus(win_insert_option,sms_insert_option_fp_win_defocus);

    lv_obj_t *      win_insert_word = alv_am_win_create(app_sms,"insert_word");

    alv_am_win_set_active(win_insert_word,sms_insert_word_fp_win_active);
    alv_am_win_set_deactive(win_insert_word,sms_insert_word_fp_win_deacactive);
    alv_am_win_set_focus(win_insert_word,sms_insert_word_fp_win_focus);
    alv_am_win_set_defocus(win_insert_word,sms_insert_word_fp_win_defocus);


    lv_obj_t *      win_insert_num = alv_am_win_create(app_sms,"insert_number");

    alv_am_win_set_active(win_insert_num,sms_insert_number_fp_win_active);
    alv_am_win_set_deactive(win_insert_num,sms_insert_number_fp_win_deacactive);
    alv_am_win_set_focus(win_insert_num,sms_insert_number_fp_win_focus);
    alv_am_win_set_defocus(win_insert_num,sms_insert_number_fp_win_defocus);


    lv_obj_t *      win_list_rev = alv_am_win_create(app_sms,"list_rev");

    alv_am_win_set_active(win_list_rev,sms_list_rev_fp_win_active);
    alv_am_win_set_deactive(win_list_rev,sms_list_rev_fp_win_deacactive);
    alv_am_win_set_focus(win_list_rev,sms_list_rev_fp_win_focus);
    alv_am_win_set_defocus(win_list_rev,sms_list_rev_fp_win_defocus);

    lv_obj_t *      win_list_send = alv_am_win_create(app_sms,"list_send");

    alv_am_win_set_active(win_list_send,sms_list_send_fp_win_active);
    alv_am_win_set_deactive(win_list_send,sms_list_send_fp_win_deacactive);
    alv_am_win_set_focus(win_list_send,sms_list_send_fp_win_focus);
    alv_am_win_set_defocus(win_list_send,sms_list_send_fp_win_defocus);

    lv_obj_t *      win_list_draft = alv_am_win_create(app_sms,"list_draft");

    alv_am_win_set_active(win_list_draft,sms_list_draft_fp_win_active);
    alv_am_win_set_deactive(win_list_draft,sms_list_draft_fp_win_deacactive);
    alv_am_win_set_focus(win_list_draft,sms_list_draft_fp_win_focus);
    alv_am_win_set_defocus(win_list_draft,sms_list_draft_fp_win_defocus);

    lv_obj_t *      win_list_del = alv_am_win_create(app_sms,"list_del");

    alv_am_win_set_active(win_list_del,sms_list_del_fp_win_active);
    alv_am_win_set_deactive(win_list_del,sms_list_del_fp_win_deacactive);
    alv_am_win_set_focus(win_list_del,sms_list_del_fp_win_focus);
    alv_am_win_set_defocus(win_list_del,sms_list_del_fp_win_defocus);


    lv_obj_t *      win_detail_rev = alv_am_win_create(app_sms,"detail_rev");

    alv_am_win_set_active(win_detail_rev,sms_detail_rev_fp_win_active);
    alv_am_win_set_deactive(win_detail_rev,sms_detail_rev_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_rev,sms_detail_rev_fp_win_focus);
    alv_am_win_set_defocus(win_detail_rev,sms_detail_rev_fp_win_defocus);

    lv_obj_t *      win_detail_rev_option = alv_am_win_create(app_sms,"detail_rev_option");

    alv_am_win_set_active(win_detail_rev_option,sms_detail_rev_option_fp_win_active);
    alv_am_win_set_deactive(win_detail_rev_option,sms_detail_rev_option_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_rev_option,sms_detail_rev_option_fp_win_focus);
    alv_am_win_set_defocus(win_detail_rev_option,sms_detail_rev_option_fp_win_defocus);

    lv_obj_t *      win_detail_rev_option_del = alv_am_win_create(app_sms,"detail_rev_op_del");

    alv_am_win_set_active(win_detail_rev_option_del,sms_detail_rev_option_del_fp_win_active);
    alv_am_win_set_deactive(win_detail_rev_option_del,sms_detail_rev_option_del_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_rev_option_del,sms_detail_rev_option_del_fp_win_focus);
    alv_am_win_set_defocus(win_detail_rev_option_del,sms_detail_rev_option_del_fp_win_defocus);

    lv_obj_t *      win_detail_rev_option_inf = alv_am_win_create(app_sms,"detail_rev_op_inf");

    alv_am_win_set_focus(win_detail_rev_option_inf,sms_detail_rev_option_inf_fp_win_focus);
    alv_am_win_set_defocus(win_detail_rev_option_inf,sms_detail_rev_option_inf_fp_win_defocus);


    lv_obj_t *      win_detail_send = alv_am_win_create(app_sms,"detail_send");

    alv_am_win_set_active(win_detail_send,sms_detail_send_fp_win_active);
    alv_am_win_set_deactive(win_detail_send,sms_detail_send_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_send,sms_detail_send_fp_win_focus);
    alv_am_win_set_defocus(win_detail_send,sms_detail_send_fp_win_defocus);

    lv_obj_t *      win_detail_send_option = alv_am_win_create(app_sms,"detail_send_option");

    alv_am_win_set_active(win_detail_send_option,sms_detail_send_option_fp_win_active);
    alv_am_win_set_deactive(win_detail_send_option,sms_detail_send_option_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_send_option,sms_detail_send_option_fp_win_focus);
    alv_am_win_set_defocus(win_detail_send_option,sms_detail_send_option_fp_win_defocus);

    lv_obj_t *      win_detail_send_option_del = alv_am_win_create(app_sms,"detail_send_op_del");

    alv_am_win_set_active(win_detail_send_option_del,sms_detail_send_option_del_fp_win_active);
    alv_am_win_set_deactive(win_detail_send_option_del,sms_detail_send_option_del_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_send_option_del,sms_detail_send_option_del_fp_win_focus);
    alv_am_win_set_defocus(win_detail_send_option_del,sms_detail_send_option_del_fp_win_defocus);

    lv_obj_t *      win_detail_send_option_inf = alv_am_win_create(app_sms,"detail_send_op_inf");

    alv_am_win_set_focus(win_detail_send_option_inf,sms_detail_send_option_inf_fp_win_focus);
    alv_am_win_set_defocus(win_detail_send_option_inf,sms_detail_send_option_inf_fp_win_defocus);

    lv_obj_t *      win_detail_draft = alv_am_win_create(app_sms,"detail_draft");

    alv_am_win_set_active(win_detail_draft,sms_detail_draft_fp_win_active);
    alv_am_win_set_deactive(win_detail_draft,sms_detail_draft_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_draft,sms_detail_draft_fp_win_focus);
    alv_am_win_set_defocus(win_detail_draft,sms_detail_draft_fp_win_defocus);


    lv_obj_t *      win_detail_draft_option = alv_am_win_create(app_sms,"detail_draft_option");

    alv_am_win_set_active(win_detail_draft_option,sms_detail_draft_option_fp_win_active);
    alv_am_win_set_deactive(win_detail_draft_option,sms_detail_draft_option_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_draft_option,sms_detail_draft_option_fp_win_focus);
    alv_am_win_set_defocus(win_detail_draft_option,sms_detail_draft_option_fp_win_defocus);


    lv_obj_t *      win_detail_draft_option_del = alv_am_win_create(app_sms,"detail_draft_op_del");

    alv_am_win_set_active(win_detail_draft_option_del,sms_detail_draft_option_del_fp_win_active);
    alv_am_win_set_deactive(win_detail_draft_option_del,sms_detail_draft_option_del_fp_win_deacactive);
    alv_am_win_set_focus(win_detail_draft_option_del,sms_detail_draft_option_del_fp_win_focus);
    alv_am_win_set_defocus(win_detail_draft_option_del,sms_detail_draft_option_del_fp_win_defocus);


    lv_obj_t *      win_del_all_read = alv_am_win_create(app_sms,"del_all_read");

    alv_am_win_set_active(win_del_all_read,sms_del_all_read_fp_win_active);
    alv_am_win_set_deactive(win_del_all_read,sms_del_all_read_fp_win_deacactive);
    alv_am_win_set_focus(win_del_all_read,sms_del_all_read_fp_win_focus);
    alv_am_win_set_defocus(win_del_all_read,sms_del_all_read_fp_win_defocus);


    lv_obj_t *      win_del_rev_all = alv_am_win_create(app_sms,"del_rev_all");

    alv_am_win_set_active(win_del_rev_all,sms_del_rev_all_fp_win_active);
    alv_am_win_set_deactive(win_del_rev_all,sms_del_rev_all_fp_win_deacactive);
    alv_am_win_set_focus(win_del_rev_all,sms_del_rev_all_fp_win_focus);
    alv_am_win_set_defocus(win_del_rev_all,sms_del_rev_all_fp_win_defocus);

    lv_obj_t *      win_del_rev_read = alv_am_win_create(app_sms,"del_rev_read");

    alv_am_win_set_active(win_del_rev_read,sms_del_rev_read_fp_win_active);
    alv_am_win_set_deactive(win_del_rev_read,sms_del_rev_read_fp_win_deacactive);
    alv_am_win_set_focus(win_del_rev_read,sms_del_rev_read_fp_win_focus);
    alv_am_win_set_defocus(win_del_rev_read,sms_del_rev_read_fp_win_defocus);


    lv_obj_t *      win_del_draft_read = alv_am_win_create(app_sms,"del_draft_read");

    alv_am_win_set_active(win_del_draft_read,sms_del_draft_read_fp_win_active);
    alv_am_win_set_deactive(win_del_draft_read,sms_del_draft_read_fp_win_deacactive);
    alv_am_win_set_focus(win_del_draft_read,sms_del_draft_read_fp_win_focus);
    alv_am_win_set_defocus(win_del_draft_read,sms_del_draft_read_fp_win_defocus);


    lv_obj_t *      win_del_send_read = alv_am_win_create(app_sms,"del_send_read");

    alv_am_win_set_active(win_del_send_read,sms_del_send_read_fp_win_active);
    alv_am_win_set_deactive(win_del_send_read,sms_del_send_read_fp_win_deacactive);
    alv_am_win_set_focus(win_del_send_read,sms_del_send_read_fp_win_focus);
    alv_am_win_set_defocus(win_del_send_read,sms_del_send_read_fp_win_defocus);



    lv_obj_t *      win_list_del_rev = alv_am_win_create(app_sms,"list_del_rev");

    alv_am_win_set_active(win_list_del_rev,sms_list_del_rev_fp_win_active);
    alv_am_win_set_deactive(win_list_del_rev,sms_list_del_rev_fp_win_deacactive);
    alv_am_win_set_focus(win_list_del_rev,sms_list_del_rev_fp_win_focus);
    alv_am_win_set_defocus(win_list_del_rev,sms_list_del_rev_fp_win_defocus);

    lv_obj_t *      list_del_rev_sel = alv_am_win_create(app_sms,"list_del_rev_sel");

    alv_am_win_set_active(list_del_rev_sel,sms_list_del_rev_sel_fp_win_active);
    alv_am_win_set_deactive(list_del_rev_sel,sms_list_del_rev_sel_fp_win_deacactive);
    alv_am_win_set_focus(list_del_rev_sel,sms_list_del_rev_sel_fp_win_focus);
    alv_am_win_set_defocus(list_del_rev_sel,sms_list_del_rev_sel_fp_win_defocus);


    lv_obj_t *      win_del_select = alv_am_win_create(app_sms,"del_select");

    alv_am_win_set_active(win_del_select,sms_del_select_fp_win_active);
    alv_am_win_set_deactive(win_del_select,sms_del_select_fp_win_deacactive);
    alv_am_win_set_focus(win_del_select,sms_del_select_fp_win_focus);
    alv_am_win_set_defocus(win_del_select,sms_del_select_fp_win_defocus);



    lv_obj_t *      win_list_del_send = alv_am_win_create(app_sms,"list_del_send");

    alv_am_win_set_active(win_list_del_send,sms_list_del_send_fp_win_active);
    alv_am_win_set_deactive(win_list_del_send,sms_list_del_send_fp_win_deacactive);
    alv_am_win_set_focus(win_list_del_send,sms_list_del_send_fp_win_focus);
    alv_am_win_set_defocus(win_list_del_send,sms_list_del_send_fp_win_defocus);


    lv_obj_t *      win_list_del_draft = alv_am_win_create(app_sms,"list_del_draft");

    alv_am_win_set_active(win_list_del_draft,sms_list_del_draft_fp_win_active);
    alv_am_win_set_deactive(win_list_del_draft,sms_list_del_draft_fp_win_deacactive);
    alv_am_win_set_focus(win_list_del_draft,sms_list_del_draft_fp_win_focus);
    alv_am_win_set_defocus(win_list_del_draft,sms_list_del_draft_fp_win_defocus);


    lv_obj_t *      list_del_draft_sel = alv_am_win_create(app_sms,"list_del_draft_sel");

    alv_am_win_set_active(list_del_draft_sel,sms_list_del_draft_sel_fp_win_active);
    alv_am_win_set_deactive(list_del_draft_sel,sms_list_del_draft_sel_fp_win_deacactive);
    alv_am_win_set_focus(list_del_draft_sel,sms_list_del_draft_sel_fp_win_focus);
    alv_am_win_set_defocus(list_del_draft_sel,sms_list_del_draft_sel_fp_win_defocus);


    lv_obj_t *      list_del_send_sel = alv_am_win_create(app_sms,"list_del_send_sel");

    alv_am_win_set_active(list_del_send_sel,sms_list_del_send_sel_fp_win_active);
    alv_am_win_set_deactive(list_del_send_sel,sms_list_del_send_sel_fp_win_deacactive);
    alv_am_win_set_focus(list_del_send_sel,sms_list_del_send_sel_fp_win_focus);
    alv_am_win_set_defocus(list_del_send_sel,sms_list_del_send_sel_fp_win_defocus);



    lv_obj_t *      win_extract_num = alv_am_win_create(app_sms,"extract_num");

    alv_am_win_set_active(win_extract_num,sms_extract_num_fp_win_active);
    alv_am_win_set_deactive(win_extract_num,sms_extract_num_fp_win_deacactive);
    alv_am_win_set_focus(win_extract_num,sms_extract_num_fp_win_focus);
    alv_am_win_set_defocus(win_extract_num,sms_extract_num_fp_win_defocus);


    lv_obj_t *      win_extract_num_edit = alv_am_win_create(app_sms,"extract_num_edit");

    alv_am_win_set_active(win_extract_num_edit,sms_extract_num_edit_fp_win_active);
    alv_am_win_set_deactive(win_extract_num_edit,sms_extract_num_edit_fp_win_deacactive);
    alv_am_win_set_focus(win_extract_num_edit,sms_extract_num_edit_fp_win_focus);
    alv_am_win_set_defocus(win_extract_num_edit,sms_extract_num_edit_fp_win_defocus);



    lv_obj_t *      win_extract_num_option = alv_am_win_create(app_sms,"extract_num_option");

    alv_am_win_set_active(win_extract_num_option,sms_extract_num_option_fp_win_active);
    alv_am_win_set_deactive(win_extract_num_option,sms_extract_num_option_fp_win_deacactive);
    alv_am_win_set_focus(win_extract_num_option,sms_extract_num_option_fp_win_focus);
    alv_am_win_set_defocus(win_extract_num_option,sms_extract_num_option_fp_win_defocus);





}


void * create_app_sms_enter_param_fisrt_write_win(uint8_t * p_context,uint8_t * p_number)
{
    sms_app_enter_param_t * p_app_enter_param;

    p_app_enter_param   = lv_mem_alloc(sizeof(sms_app_enter_param_t));

    memset(p_app_enter_param,0,sizeof(sms_app_enter_param_t));

    p_app_enter_param->win_enter_param.write.p_content = p_context;
    p_app_enter_param->win_enter_param.write.p_number = p_number;

    p_app_enter_param->first_win = SMS_APP_FIRST_WIN_WRITE;

    return p_app_enter_param;
}


void destory_app_sms_enter_param(void * p)
{
    lv_mem_free(p);
}


void * create_app_sms_enter_param_fisrt_rev_win(void)
{
    sms_app_enter_param_t * p_app_enter_param;

    p_app_enter_param   = lv_mem_alloc(sizeof(sms_app_enter_param_t));

    memset(p_app_enter_param,0,sizeof(sms_app_enter_param_t));

    p_app_enter_param->first_win = SMS_APP_FIRST_WIN_REV;

    return p_app_enter_param;
}


