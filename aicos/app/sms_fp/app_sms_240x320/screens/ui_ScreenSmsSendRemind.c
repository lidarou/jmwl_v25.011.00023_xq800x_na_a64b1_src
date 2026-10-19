
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
#include "alv_app_mgr.h"
#include <stdio.h>
#include "app_msg.h"
#include "tele_data_msg.h"
#include "tts_app_player.h"
#include "tele_data_task.h"

typedef struct ui_screenSmsSendRemind_obj_h {
    lv_obj_t *      label_title;
    lv_obj_t *      label_count;
    lv_obj_t *      bar;
    uint32_t        send_count;
    uint32_t        send_cur;
    sms_send_remind_ui_enter_param_t enter_param;
} ui_screenSmsSendRemind_obj_t;

typedef struct {
    uint32_t        send_index;
    uint32_t        send_status;
    void *          user_data;
} sms_send_param_t;

void sms_send_remind_key_event(uint32_t key,lv_obj_t * ui_Screen);

static int32_t sms_screen_send_comlete_exit(void * user_data,uint32_t size)
{

    char            name[32];

    memset((u8 *) name,0,32);

    alv_am_win_get_cur_winname(name);

    if (strcmp(name,"sms_send_remind")) {
        return -1;
        }

    alv_am_win_exit_action(NULL,"write",NULL);
    alv_am_win_exit_action(NULL,"write_option",NULL);
    alv_am_win_exit_action(NULL,"send_option",NULL);
    alv_am_win_exit_action(NULL,"select_phone_num",NULL);
    lv_obj_del_now_custom();
    alv_am_win_exit_current(NULL);

    return 0;
}

static int32_t sms_send_comlete_action(void * user_data,uint32_t size)
{
    sms_send_param_t * p_sms_send_param;
    lv_obj_t *      ui_Screen;
    ui_screenSmsSendRemind_obj_t * p_screen_param;
    uint32_t        per;
    uint8_t         str_buf[50];

    char            name[32];

    memset((u8 *) name,0,32);

    alv_am_win_get_cur_winname(name);

    if (strcmp(name,"sms_send_remind")) {
        return -1;
        }

    p_sms_send_param    = user_data;

    if (p_sms_send_param == NULL)
        return -1;

    ui_Screen           = p_sms_send_param->user_data;
    p_screen_param      = lv_obj_get_user_data(ui_Screen);


    p_screen_param->send_cur = p_sms_send_param->send_index;

    memset(str_buf,0,50);
    sprintf(str_buf,"%d/%d",(int)(p_screen_param->send_cur),(int)(p_screen_param->send_count));
    lv_label_set_text(p_screen_param->label_count,str_buf);

    if (p_screen_param->send_count)
        per = p_screen_param->send_cur * 100 / p_screen_param->send_count;
    else 
        per = 100;

    lv_bar_set_value(p_screen_param->bar,per,LV_ANIM_OFF);

    if (p_sms_send_param->send_status) {

        int32_t         ret = 0;

        ret                 = app_msg_send_callback(sms_screen_send_comlete_exit,NULL,0);

        if (0 != ret) {
            app_loge("[%s] app_msg_send_callback error = %d. \n",__func__,ret);
            }

        }

    return 0;
}



int32_t sms_send_complete(uint32_t send_index,uint32_t send_status,void * user_data)
{
    sms_send_param_t sms_send_param;
    int32_t         ret = 0;

    sms_send_param.send_index = send_index;
    sms_send_param.send_status = send_status;
    sms_send_param.user_data = user_data;

    ret                 = app_msg_send_callback(sms_send_comlete_action,&sms_send_param,sizeof(sms_send_param));

    if (0 != ret) {
        app_loge("[%s] app_msg_send_callback error = %d. \n",__func__,ret);
        }

    return ret;
}




static void sms_fp_send_remind_ui_event_actions(lv_event_t * e)
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
                sms_send_remind_key_event(key,obj);

                }
            break;

        default:
            break;
        }


}



lv_obj_t * ui_ScreenSmsSendRemind_screen_init(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_Screen;
    lv_obj_t *      label;

    ui_screenSmsSendRemind_obj_t * p_param_obj;
    uint8_t         str_buf[50];

    p_param_obj         = lv_mem_alloc(sizeof(ui_screenSmsSendRemind_obj_t));

    if (p_param_obj == NULL)
        return 0;

    memset(p_param_obj,0,sizeof(ui_screenSmsSendRemind_obj_t));

    if (p_param) {
        memcpy(&p_param_obj->enter_param,p_param,sizeof(sms_send_remind_ui_enter_param_t));
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


    memset(str_buf,0,50);
    sprintf(str_buf,"正在发送...");
    lv_label_set_text(label,str_buf);

    p_param_obj->label_title = label;


    for (int i = 0; i < p_param_obj->enter_param.node_count; i++) {
        if (p_param_obj->enter_param.p_node[i])
            p_param_obj->send_count++;
        }



    label               = lv_label_create(ui_Screen);
    lv_obj_set_x(label,10);
    lv_obj_set_y(label,120);
    lv_obj_set_width(label,LV_SIZE_CONTENT);        /// 1
    lv_obj_set_height(label,LV_SIZE_CONTENT);       /// 1

    memset(str_buf,0,50);
    sprintf(str_buf,"0/%d",(int)(p_param_obj->send_count));
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

    lv_obj_add_event_cb(ui_Screen,sms_fp_send_remind_ui_event_actions,LV_EVENT_ALL,ui_Screen);

    lv_obj_set_user_data(win,ui_Screen);



    tele_data_msg_send_sms(p_param_obj->enter_param.p_content,p_param_obj->enter_param.p_node,
        p_param_obj->enter_param.node_count,sms_send_complete,ui_Screen);



    return ui_Screen;
}





static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screenSmsSendRemind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screenSmsSendRemind_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}

void ui_ScreenSmsSendRemind_screen_uninit(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_Screen = NULL;

    ui_Screen           = lv_obj_get_user_data(win);

    if (ui_Screen) {
        lv_obj_del_delayed_callback(ui_Screen,100,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}

void ui_ScreenSmsSendRemind_screen_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_Screen = NULL;
    uint8_t         str_buf[50];
    uint32_t        per;

    ui_screenSmsSendRemind_obj_t * p_screen_param;

    ui_Screen           = lv_obj_get_user_data(win);

    p_screen_param      = lv_obj_get_user_data(ui_Screen);

    memset(str_buf,0,50);
    sprintf(str_buf,"%d/%d",(int)(p_screen_param->send_cur),(int)(p_screen_param->send_count));
    lv_label_set_text(p_screen_param->label_count,str_buf);

    if (p_screen_param->send_count)
        per = p_screen_param->send_cur * 100 / p_screen_param->send_count;
    else 
        per = 100;

    lv_bar_set_value(p_screen_param->bar,per,LV_ANIM_OFF);


    lv_disp_load_scr(ui_Screen);

    app_fp_play_tts_menu(lv_label_get_text(p_screen_param->label_title));

    if (p_screen_param->send_cur >= p_screen_param->send_count) {

        int32_t         ret = 0;

        ret                 = app_msg_send_callback(sms_screen_send_comlete_exit,NULL,0);

        if (0 != ret) {
            app_loge("[%s] app_msg_send_callback error = %d. \n",__func__,ret);
            }

        }
}



void sms_send_remind_key_event(uint32_t key,lv_obj_t * ui_Screen)
{

    switch (key)
    {
        case LV_KEY_ENTER:
        case APP_KEY_VALUE_TOP_LEFT:
            break;

        case APP_KEY_VALUE_TOP_RIGHT:
            {

                tele_sms_send_data_stop();

                alv_am_win_exit_action(NULL,"write",NULL);
                alv_am_win_exit_action(NULL,"write_option",NULL);
                alv_am_win_exit_action(NULL,"send_option",NULL);
                alv_am_win_exit_action(NULL,"select_phone_num",NULL);
                lv_obj_del_now_custom();
                alv_am_win_exit_current(NULL);

                }
            break;

        default:
            break;
        }
}



