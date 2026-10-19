
#include "../ui.h"
#include "obj_batch_deal.h"
#include "aic_osa.h"
#include <aic_type.h>
#include "aic_srv_tele.h"
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aic_list.h>
#include "screen_remind.h"
#include <stdio.h>
#include "alv_app_mgr.h"
#include "phonebook.h"
#include "tts_app_player.h"
#include "app_msg.h"
#include "backlight.h"

enum {
    OBJ_LABEL_OK,
    OBJ_LABEL_BACK,
    OBJ_LABEL_COPY,
    OBJ_LABEL_REMIND,
    OBJ_LABEL_MAX,
};

static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"确定"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },
    {
        0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"复制"
        },
    {
        0,50,240,280,0,NULL
        },
};



typedef struct ui_screenidle1_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    screen_remind_param_t screen_remind_param;
    phonebook_enter_param_remind_copy_t enter_param;
    uint8_t         buf[30];
} param_screenRemindCopy_obj_t;

void phonebook_copying_remind_key_event(uint32_t key,lv_obj_t * ui_Screen);

lv_obj_t * ui_ScreenPhonebookRemindCopy_screen_init(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_ScreenPhonebookRemindCopy;

    param_screenRemindCopy_obj_t * p_screen_param;
    uint16_t        copy_count;

    p_screen_param      = lv_mem_alloc(sizeof(param_screenRemindCopy_obj_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(phonebook_enter_param_remind_copy_t));

    if (p_param) {
        screen_remind_param_t * p_screen_remind_param = p_param;

        memcpy(&p_screen_param->screen_remind_param,p_screen_remind_param,sizeof(screen_remind_param_t));

        if (p_screen_remind_param->user_data)
            memcpy(&p_screen_param->enter_param,p_screen_remind_param->user_data,
                sizeof(phonebook_enter_param_remind_copy_t));
        }

    ui_ScreenPhonebookRemindCopy = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebookRemindCopy,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebookRemindCopy,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebookRemindCopy,&p_screen_param->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);

    if (p_screen_param->enter_param.copy_del) {
        lv_label_set_text(p_screen_param->obj_label[OBJ_LABEL_COPY],"删除");

        }

    copy_count          = 0;

    for (int i = 0; i < p_screen_param->enter_param.state_count; i++) {
        if (p_screen_param->enter_param.p_state[i])
            copy_count++;
        }

    sprintf(p_screen_param->buf,"%d个联系人?",copy_count);

    lv_label_set_text(p_screen_param->obj_label[OBJ_LABEL_REMIND],p_screen_param->buf);



    lv_obj_set_user_data(ui_ScreenPhonebookRemindCopy,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenPhonebookRemindCopy,phonebook_fp_remindcopy_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebookRemindCopy);

    lv_obj_set_user_data(win,ui_ScreenPhonebookRemindCopy);


    return ui_ScreenPhonebookRemindCopy;
}





static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    param_screenRemindCopy_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (param_screenRemindCopy_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}

void ui_ScreenPhonebookRemindCopy_screen_uninit(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_ScreenPhonebookRemindCopy;

    ui_ScreenPhonebookRemindCopy = lv_obj_get_user_data(win);

    if (ui_ScreenPhonebookRemindCopy) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebookRemindCopy,100,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}






void ui_ScreenPhonebookRemindCopy_screen_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_ScreenPhonebookRemindCopy = NULL;
    param_screenRemindCopy_obj_t * p_screen_param;

    ui_ScreenPhonebookRemindCopy = lv_obj_get_user_data(win);


    p_screen_param      = lv_obj_get_user_data(ui_ScreenPhonebookRemindCopy);

    lv_disp_load_scr(ui_ScreenPhonebookRemindCopy);
    {
        uint8_t         str_buf[50];

        memset(str_buf,0,50);
        strcpy(str_buf,lv_label_get_text(p_screen_param->obj_label[OBJ_LABEL_COPY]));
        strcat(str_buf,lv_label_get_text(p_screen_param->obj_label[OBJ_LABEL_REMIND]));
        app_fp_play_tts_menu(str_buf);
        }
}

void ui_phonebook_copy_ok_remind_screen(lv_obj_t * ui_Screen)
{
    param_screenRemindCopy_obj_t * p_screen_param_obj;
    screen_remind_param_t screen_remind_param;

    p_screen_param_obj  = (param_screenRemindCopy_obj_t *) lv_obj_get_user_data(ui_Screen);
    ui_screen_remind_param_init(&screen_remind_param);

#if 0

    if (p_screen_param_obj->enter_param.copy_del == 0) {
        screen_remind_param.str = "已复制";
        }
    else {
        screen_remind_param.str = "已删除";
        }

    screen_remind_param.tiemr_delay = 1500;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_phonebook_copy;
    screen_remind_param.user_data = p_screen_param_obj->screen_remind_param.user_data;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);
#endif

    if (p_screen_param_obj->enter_param.copy_del == 0) { //phonebook_data_copy_storge(p_screen_param_obj->enter_param.format,p_screen_param_obj->enter_param.p_state,p_screen_param_obj->enter_param.state_count);

        //phone_data_msg_send_copy(p_screen_param_obj->enter_param.format,p_screen_param_obj->enter_param.p_state,p_screen_param_obj->enter_param.state_count);
        //ui_screen = ui_ScreenPhonebookRemindCopying_screen_init(&p_screen_param_obj->enter_param);
        //lv_disp_load_scr(ui_screen);
        alv_am_win_jump_action(NULL,"copying",(void *) &p_screen_param_obj->enter_param);

        }
    else {
        alv_am_win_jump_action(NULL,"copying",(void *) &p_screen_param_obj->enter_param);
        }
}


void ui_phonebook_copy_exit_remind_screen(lv_obj_t * ui_Screen)
{
#if 0
    param_screenRemindCopy_obj_t * p_screen_param_obj;
    lv_obj_t *      ui_screen;

    p_screen_param_obj  = (param_screenRemindCopy_obj_t *) lv_obj_get_user_data(ui_Screen);

    ui_screen           = p_screen_param_obj->enter_param.ui_screen;

    if (ui_screen)
        lv_disp_load_scr(ui_screen);

#else

    alv_am_win_exit_current(NULL);
#endif

}


void phonebook_copy_remind_key_event(uint32_t key,lv_obj_t * ui_Screen)
{

    switch (key)
    {
        case LV_KEY_ENTER:
        case APP_KEY_VALUE_TOP_LEFT:
            ui_phonebook_copy_ok_remind_screen(ui_Screen);
            break;

        case APP_KEY_VALUE_TOP_RIGHT:
            ui_phonebook_copy_exit_remind_screen(ui_Screen);
            break;

        default:
            break;
        }
}

typedef struct ui_screenRemindCopying_obj_h {
    lv_obj_t *      label_title;
    lv_obj_t *      label_count;
    lv_obj_t *      bar;
    uint32_t        copy_status;
    uint32_t        copy_count;
    uint32_t        copy_cur;
    phonebook_enter_param_remind_copy_t enter_param;
} param_screenRemindCopying_obj_t;

typedef struct {
    uint32_t        copy_count;
    uint32_t        copy_status;
    void *          user_data;
} phonebook_copy_param_t;

static int32_t phonebook_screen_copying_comlete_exit(void * user_data,uint32_t size)
{

    ui_screen_phonebook_enter_param_t * p_enter_param;
    char            name[32];

    memset((u8 *) name,0,32);

    alv_am_win_get_cur_winname(name);

    if (strcmp(name,"copying")) {
        return - 1;
        }

    p_enter_param       = NULL;

    alv_am_win_exit_action(NULL,"pb",p_enter_param);
    alv_am_win_exit_action(NULL,"storge_format",p_enter_param);
    alv_am_win_exit_action(NULL,"list_option",p_enter_param);
    alv_am_win_exit_action(NULL,"edit",p_enter_param);
    alv_am_win_exit_action(NULL,"copy",p_enter_param);
    alv_am_win_exit_action(NULL,"copy_select",p_enter_param);
    alv_am_win_exit_action(NULL,"copy_remind",p_enter_param);
    lv_obj_del_now_custom();
    alv_am_win_jump_action(NULL,"pb",NULL);

    return 0;
}

static int32_t phonebook_copy_comlete_action(void * user_data,uint32_t size)
{
    phonebook_copy_param_t * p_phonebook_copy_param;
    lv_obj_t *      ui_Screen;
    param_screenRemindCopying_obj_t * p_screen_param;
    uint32_t        per;
    uint8_t         str_buf[50];

    char            name[32];

    memset((u8 *) name,0,32);

    p_phonebook_copy_param = user_data;


    //__inf("phonebook_copy_comlete_action");
    if (p_phonebook_copy_param == NULL)
        return -1;

    ui_Screen           = p_phonebook_copy_param->user_data;


    alv_am_win_get_cur_winname(name);

    //__inf("phonebook_copy_comlete_action cur_win:%s",name);
    if (strcmp(name,"copying")) {
        lv_obj_t *      app,*win;

        app                 = get_app_by_name("phonebook");

        if (app == NULL) {
            app_loge("phonebook_copy_comlete_action app==NUL");
            return -1;
            }

        win                 = get_win_by_name(app,"copying");

        if (win == NULL) {
            app_loge("phonebook_copy_comlete_action win==NUL");
            return -1;
            }

        if ((ui_Screen != NULL) && (ui_Screen == lv_obj_get_user_data(win))) {
            p_screen_param      = lv_obj_get_user_data(ui_Screen);
            p_screen_param->copy_cur = p_phonebook_copy_param->copy_count;
            p_screen_param->copy_status = p_phonebook_copy_param->copy_status;
            }
        else {

            app_loge("phonebook_copy_comlete_action ui_Screen:0x%x,0x%x",ui_Screen,lv_obj_get_user_data(win));
            }

        return 0;
        }


    p_screen_param      = lv_obj_get_user_data(ui_Screen);


    p_screen_param->copy_cur = p_phonebook_copy_param->copy_count;

    memset(str_buf,0,50);
    sprintf(str_buf,"%d/%d",(int) (p_screen_param->copy_cur),(int) (p_screen_param->copy_count));
    lv_label_set_text(p_screen_param->label_count,str_buf);

    if (p_screen_param->copy_count)
        per = p_screen_param->copy_cur * 100 / p_screen_param->copy_count;
    else 
        per = 100;

    lv_bar_set_value(p_screen_param->bar,per,LV_ANIM_OFF);

    if (p_phonebook_copy_param->copy_status) {

        int32_t         ret = 0;

        ret                 = app_msg_send_callback(phonebook_screen_copying_comlete_exit,NULL,0);

        if (0 != ret) {
            app_loge("[%s] app_msg_send_callback error = %d. \n",__func__,ret);
            }

        }

    return 0;
}



int32_t phonebook_copy_complete(uint32_t copy_count,uint32_t copy_status,void * user_data)
{
    phonebook_copy_param_t phonebook_copy_param;
    int32_t         ret = 0;
    char            name[32];

    memset((u8 *) name,0,32);

    alv_am_win_get_cur_winname(name);

    if ((copy_status == 0) && (strcmp(name,"copying")))
        return ret;

    phonebook_copy_param.copy_count = copy_count;
    phonebook_copy_param.copy_status = copy_status;
    phonebook_copy_param.user_data = user_data;

    ret                 = app_msg_send_callback(phonebook_copy_comlete_action,&phonebook_copy_param,
        sizeof(phonebook_copy_param));

    if (0 != ret) {
        app_loge("[%s] app_msg_send_callback error = %d. \n",__func__,ret);
        }

    return ret;
}




static void phonebook_fp_remindcopying_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    //lv_obj_t * target = lv_event_get_target(e);
    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);

                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                //alv_am_app_exit_action(NULL,"phonebook",NULL);				
                //alv_am_app_enter_action(NULL,"home_fp",NULL);
                phonebook_copying_remind_key_event(key,obj);

                }
            break;

        default:
            break;
        }


}



lv_obj_t * ui_ScreenPhonebookRemindCopying_screen_init(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_Screen;
    lv_obj_t *      label;

    param_screenRemindCopying_obj_t * p_param_obj;
    uint8_t         str_buf[50];

    p_param_obj         = lv_mem_alloc(sizeof(param_screenRemindCopying_obj_t));

    if (p_param_obj == NULL)
        return 0;

    memset(p_param_obj,0,sizeof(param_screenRemindCopying_obj_t));

    if (p_param) {
        memcpy(&p_param_obj->enter_param,p_param,sizeof(phonebook_enter_param_remind_copy_t));
        }

    ui_Screen           = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_Screen,get_lv_style_font_common(),0);


    label               = lv_label_create(ui_Screen);
    lv_obj_set_x(label,162);
    lv_obj_set_y(label,277);
    lv_obj_set_width(label,LV_SIZE_CONTENT);        /// 1
    lv_obj_set_height(label,LV_SIZE_CONTENT);       /// 1
    lv_label_set_text(label,"取消");


    label               = lv_label_create(ui_Screen);
    lv_obj_set_x(label,0);
    lv_obj_set_y(label,0);
    lv_obj_set_width(label,240);                    /// 1
    lv_obj_set_height(label,LV_SIZE_CONTENT);       /// 1

    if (p_param_obj->enter_param.copy_del) {
        memset(str_buf,0,50);
        sprintf(str_buf,"正在删除...");
        lv_label_set_text(label,str_buf);

        }
    else {
        if (p_param_obj->enter_param.format) {
            memset(str_buf,0,50);
            sprintf(str_buf,"正在从SIM%d复制到手机...",(int) (get_tele_phonebook_sim_status()));
            lv_label_set_text(label,str_buf);

            }
        else {
            memset(str_buf,0,50);
            sprintf(str_buf,"正在从手机复制到SIM%d...",(int) (get_tele_phonebook_sim_status()));
            lv_label_set_text(label,str_buf);

            }

        }

    p_param_obj->label_title = label;


    for (int i = 0; i < p_param_obj->enter_param.state_count; i++) {
        if (p_param_obj->enter_param.p_state[i])
            p_param_obj->copy_count++;
        }



    label               = lv_label_create(ui_Screen);
    lv_obj_set_x(label,10);
    lv_obj_set_y(label,120);
    lv_obj_set_width(label,LV_SIZE_CONTENT);        /// 1
    lv_obj_set_height(label,LV_SIZE_CONTENT);       /// 1

    memset(str_buf,0,50);
    sprintf(str_buf,"0/%d",(int) (p_param_obj->copy_count));
    lv_label_set_text(label,str_buf);
    p_param_obj->label_count = label;


    p_param_obj->bar    = lv_bar_create(ui_Screen);
    lv_obj_set_x(p_param_obj->bar,10);
    lv_obj_set_y(p_param_obj->bar,170);
    lv_obj_set_width(p_param_obj->bar,220);         /// 1
    lv_obj_set_height(p_param_obj->bar,6);          /// 1
    lv_bar_set_range(p_param_obj->bar,0,100);
    lv_bar_set_value(p_param_obj->bar,0,LV_ANIM_OFF);

    lv_obj_set_user_data(ui_Screen,p_param_obj);

    lv_obj_add_event_cb(ui_Screen,phonebook_fp_remindcopying_ui_event_actions,LV_EVENT_ALL,ui_Screen);

    lv_obj_set_user_data(win,ui_Screen);


    if (p_param_obj->enter_param.copy_del) {
        phone_data_msg_send_del((phonebook_map_node_t * *) (p_param_obj->enter_param.p_state),
            p_param_obj->enter_param.state_count,
            phonebook_copy_complete,ui_Screen);
        }
    else {
        phone_data_msg_send_copy(p_param_obj->enter_param.format,
            (phonebook_map_node_t * *) (p_param_obj->enter_param.p_state),
            p_param_obj->enter_param.state_count,phonebook_copy_complete,ui_Screen);
        }


    backlight_turn_on_always();

    return ui_Screen;
}





static void lv_obj_del_screen_copying_ready_cb(lv_anim_t * a)
{
    param_screenRemindCopying_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (param_screenRemindCopying_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}

void ui_ScreenPhonebookRemindCopying_screen_uninit(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_Screen = NULL;

    ui_Screen           = lv_obj_get_user_data(win);

    if (ui_Screen) {
        lv_obj_del_delayed_callback(ui_Screen,100,lv_obj_del_screen_copying_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);

    backlight_exit_turn_on_always();
}

void ui_ScreenPhonebookRemindCopying_screen_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_Screen = NULL;
    uint8_t         str_buf[50];
    uint32_t        per;

    param_screenRemindCopying_obj_t * p_screen_param;

    ui_Screen           = lv_obj_get_user_data(win);

    p_screen_param      = lv_obj_get_user_data(ui_Screen);

    memset(str_buf,0,50);
    sprintf(str_buf,"%d/%d",(int) (p_screen_param->copy_cur),(int) (p_screen_param->copy_count));
    lv_label_set_text(p_screen_param->label_count,str_buf);

    if (p_screen_param->copy_count)
        per = p_screen_param->copy_cur * 100 / p_screen_param->copy_count;
    else 
        per = 100;

    lv_bar_set_value(p_screen_param->bar,per,LV_ANIM_OFF);


    lv_disp_load_scr(ui_Screen);

    app_fp_play_tts_menu(lv_label_get_text(p_screen_param->label_title));

    if ((p_screen_param->copy_cur == p_screen_param->copy_count) || (p_screen_param->copy_status)) {

        int32_t         ret = 0;

        ret                 = app_msg_send_callback(phonebook_screen_copying_comlete_exit,NULL,0);

        if (0 != ret) {
            app_loge("[%s] app_msg_send_callback error = %d. \n",__func__,ret);
            }

        }
}



void phonebook_copying_remind_key_event(uint32_t key,lv_obj_t * ui_Screen)
{

    switch (key)
    {
        case LV_KEY_ENTER:
        case APP_KEY_VALUE_TOP_LEFT:
            break;

        case APP_KEY_VALUE_TOP_RIGHT:
            {
                ui_screen_phonebook_enter_param_t * p_enter_param;

                p_enter_param       = NULL;

                tele_phonebook_copy_stop();

                alv_am_win_exit_action(NULL,"pb",p_enter_param);
                alv_am_win_exit_action(NULL,"storge_format",p_enter_param);
                alv_am_win_exit_action(NULL,"list_option",p_enter_param);
                alv_am_win_exit_action(NULL,"edit",p_enter_param);
                alv_am_win_exit_action(NULL,"copy",p_enter_param);
                alv_am_win_exit_action(NULL,"copy_select",p_enter_param);
                alv_am_win_exit_action(NULL,"copy_remind",p_enter_param);
                lv_obj_del_now_custom();
                alv_am_win_jump_action(NULL,"pb",NULL);

                }
            break;

        case LV_KEY_END:
        {
            tele_phonebook_copy_stop();
            alv_am_app_exit_action_all();
        }
        break;

        default:
            break;
        }
}

