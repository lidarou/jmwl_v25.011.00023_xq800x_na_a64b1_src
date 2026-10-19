
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "stdio.h"
#include "tele_sms_data.h"

#ifdef CONFIG_APP_PHONEBOOK_FP
#include "app_phonebook_list.h"
#include "app_blacklist.h"
#endif

#include "alv_app_mgr.h"

#ifdef CONFIG_APP_SMS_FP
#include "app_sms_list.h"
#endif

#include "app_call_list.h"
#include "app_log.h"

#define __inf                   APP_LOGI




enum OBJ_LABEL_H {
    OBJ_LABEL_OPTION,
    OBJ_LABEL_BACK,
    OBJ_LABEL_MAX,
};


static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"选择"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },

};


enum {
    IDLE_NUM_CALL,
    IDLE_NUM_STORAGE,
    IDLE_NUM_SEND_SMS,
    IDLE_NUM_BLACKLIST,
    IDLE_NUM_MAX,
};


static uint8_t * option_text[IDLE_NUM_MAX] =
{
    "呼叫",
    "存储",
    "发送信息",
    "加至黑名单",
};


typedef struct ui_screen_idle_num_option_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[IDLE_NUM_MAX];

    idle_num_wirte_ui_enter_param_t ui_enter_param;

    void *          p_enter_phonebook_param;
    void *          p_enter_sms_param;
    void *          p_blacklist_enter_param;

} ui_screen_idle_num_option_obj_t;


lv_obj_t * ui_ScreenIdleNumOption_screen_init(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenIdleNumOption = NULL;
    lv_obj_t *      label;

    ui_screen_idle_num_option_obj_t * p_screen_param;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_idle_num_option_obj_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_idle_num_option_obj_t));

    if (enter_param) {
        memcpy(&p_screen_param->ui_enter_param,enter_param,sizeof(idle_num_wirte_ui_enter_param_t));
        }


    ui_ScreenIdleNumOption = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenIdleNumOption,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenIdleNumOption,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenIdleNumOption,&p_screen_param->obj_label[0],NULL,obj_batch_label_param_array,
        OBJ_LABEL_MAX);


    label               = lv_label_create(ui_ScreenIdleNumOption);
    lv_label_set_long_mode(label,LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_anim_speed(label,10,0);
    lv_obj_set_x(label,0);
    lv_obj_set_y(label,0);
    lv_obj_set_width(label,240);                    /// 1
    lv_obj_set_height(label,LV_SIZE_CONTENT);       /// 1

    if (enter_param)
        lv_label_set_text(label,p_screen_param->ui_enter_param.number);




    p_screen_param->obj_list = lv_obj_create(ui_ScreenIdleNumOption);
    lv_obj_set_width(p_screen_param->obj_list,240);
    lv_obj_set_height(p_screen_param->obj_list,228);
    lv_obj_set_x(p_screen_param->obj_list,0);
    lv_obj_set_y(p_screen_param->obj_list,50);
    lv_obj_set_align(p_screen_param->obj_list,LV_ALIGN_TOP_MID);
    lv_obj_add_flag(p_screen_param->obj_list,LV_OBJ_FLAG_SCROLL_WITH_ARROW); /// Flags
    lv_obj_set_scrollbar_mode(p_screen_param->obj_list,LV_SCROLLBAR_MODE_ON);
    lv_obj_set_scroll_dir(p_screen_param->obj_list,LV_DIR_VER);
    lv_obj_set_style_radius(p_screen_param->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    //lv_obj_set_style_bg_color(ui_PnlSettings, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
    //lv_obj_set_style_bg_opa(ui_PnlSettings, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(p_screen_param->obj_list,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(p_screen_param->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_screen_param->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_screen_param->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    //obj_batch_label_create(p_screen_param->obj_list,&p_screen_param->obj_entry[0],NULL,obj_batch_entry_param_array,OBJ_ENTRY_MAX);
    for (int i = 0; i < IDLE_NUM_MAX; i++) {

        p_screen_param->obj_entry[i] = lv_label_create(p_screen_param->obj_list);

        lv_obj_set_x(p_screen_param->obj_entry[i],0);
        lv_obj_set_y(p_screen_param->obj_entry[i],45 * i);
        lv_obj_set_width(p_screen_param->obj_entry[i],LV_PCT(100)); /// 1
        lv_obj_set_height(p_screen_param->obj_entry[i],45); /// 1

        lv_label_set_text(p_screen_param->obj_entry[i],option_text[i]);
        lv_obj_set_user_data(p_screen_param->obj_entry[i],(void *) i);

        lv_obj_set_style_bg_color(p_screen_param->obj_entry[i],NAME_TEXT_BG_COLOR,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(p_screen_param->obj_entry[i],LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_text_color(p_screen_param->obj_entry[i],NAME_TEXT_COLOR,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(p_screen_param->obj_entry[i],LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_text_color(p_screen_param->obj_entry[i],NAME_TEXT_COLOR_FOCUS,
            LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(p_screen_param->obj_entry[i],NAME_TEXT_BG_COLOR_FOCUS,
            LV_PART_MAIN | LV_STATE_FOCUSED);
        }

    lv_obj_add_state(p_screen_param->obj_entry[0],LV_STATE_FOCUSED);


    lv_obj_set_user_data(ui_ScreenIdleNumOption,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenIdleNumOption,fp_idle_num_option_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenIdleNumOption);

    lv_obj_set_user_data(win,ui_ScreenIdleNumOption);

    return ui_ScreenIdleNumOption;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_idle_num_option_obj_t * p_screen_param;

    p_screen_param      = (ui_screen_idle_num_option_obj_t *) lv_obj_get_user_data(a->var);

    if (p_screen_param->p_enter_phonebook_param) {
#ifdef CONFIG_APP_PHONEBOOK_FP
        destory_app_phonebook_enter_param(p_screen_param->p_enter_phonebook_param);
#endif
        }

    if (p_screen_param->p_enter_sms_param) {
#ifdef CONFIG_APP_SMS_FP
        destory_app_sms_enter_param(p_screen_param->p_enter_sms_param);
#endif
        }

    if (p_screen_param->p_blacklist_enter_param) {
#ifdef CONFIG_APP_PHONEBOOK_FP
        destory_app_blacklist_enter_param(p_screen_param->p_blacklist_enter_param,NULL);
#endif
        }


    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenIdleNumOption_screen_uninit(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenIdleNumOption = NULL;

    ui_ScreenIdleNumOption = lv_obj_get_user_data(win);

    if (ui_ScreenIdleNumOption) {
        lv_obj_del_delayed_callback(ui_ScreenIdleNumOption,100,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}



lv_obj_t * get_idle_num_option_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_idle_num_option_obj_t * p_screen_param =
         (ui_screen_idle_num_option_obj_t *) lv_obj_get_user_data(ui_Screen);

    lv_obj_t *      list = p_screen_param->obj_list;

    return list;
}


static void app_blacklist_win_exit_fun(void)
{
    alv_am_win_exit_action_by_app("idle_fp","idle_num_option",NULL);
    alv_am_win_exit_current(NULL);
}




void idle_num_option_key_event(uint32_t key,lv_obj_t * ui_ScreenIdleNumOption)
{
    ui_screen_idle_num_option_obj_t * p_screen_param =
         (ui_screen_idle_num_option_obj_t *) lv_obj_get_user_data(ui_ScreenIdleNumOption);

    lv_obj_t *      list = get_idle_num_option_list_obj(ui_ScreenIdleNumOption);
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;
    lv_obj_t *      pre_focus_obj = NULL;


    if (key == LV_KEY_UP) {
        //__inf("LV_KEY_UP,child_id:%d,child_cnt:%d\r\n",child_id,child_cnt);
        if (child_id != 0) {
            pre_focus_obj       = lv_obj_get_child(list,child_id);
            lv_obj_clear_state(pre_focus_obj,LV_STATE_FOCUSED);


            get_focus_obj       = lv_obj_get_child(list,child_id - 1);
            lv_obj_add_state(get_focus_obj,LV_STATE_FOCUSED);

            if ((child_id - 1) == 0) {
                lv_obj_scroll_to_y(list,0,LV_ANIM_OFF);
                }
            else {
                lv_obj_scroll_to_view(get_focus_obj,LV_ANIM_OFF);
                }
            }
        else {
            pre_focus_obj       = lv_obj_get_child(list,child_id);
            lv_obj_clear_state(pre_focus_obj,LV_STATE_FOCUSED);

            get_focus_obj       = lv_obj_get_child(list,child_cnt - 1);
            lv_obj_add_state(get_focus_obj,LV_STATE_FOCUSED);

            lv_obj_scroll_to_view(get_focus_obj,LV_ANIM_OFF);
            }

        }
    else if (key == LV_KEY_DOWN) {

        if (child_id != child_cnt - 1) {
            pre_focus_obj       = lv_obj_get_child(list,child_id);
            lv_obj_clear_state(pre_focus_obj,LV_STATE_FOCUSED);

            get_focus_obj       = lv_obj_get_child(list,child_id + 1);
            lv_obj_add_state(get_focus_obj,LV_STATE_FOCUSED);

            lv_obj_scroll_to_view(get_focus_obj,LV_ANIM_OFF);
            }
        else {
            pre_focus_obj       = lv_obj_get_child(list,child_id);
            lv_obj_clear_state(pre_focus_obj,LV_STATE_FOCUSED);

            get_focus_obj       = lv_obj_get_child(list,0);
            lv_obj_add_state(get_focus_obj,LV_STATE_FOCUSED);

            lv_obj_scroll_to_y(list,0,LV_ANIM_OFF);
            }
        }
    else if ((key == APP_KEY_VALUE_TOP_LEFT) || (key == LV_KEY_ENTER)) {
        uint32_t        index;

        get_focus_obj       = lv_obj_get_child(list,child_id);
        index               = (uint32_t) lv_obj_get_user_data(get_focus_obj);

        switch (index)
        {
            case IDLE_NUM_CALL:
#ifdef CONFIG_APP_CALL_FP
                {
                    call_dial_number(p_screen_param->ui_enter_param.number,NULL);

                    alv_am_win_exit_action_by_app("idle_fp","idle_num_write",NULL);
                    alv_am_win_exit_action_by_app("idle_fp","idle_num_option",NULL);
                    }
#endif

                break;

            case IDLE_NUM_STORAGE:
                {
#ifdef CONFIG_APP_PHONEBOOK_FP
                    void *          p_enter_param;

                    if (p_screen_param->p_enter_phonebook_param != NULL) {
                        destory_app_phonebook_enter_param(p_screen_param->p_enter_phonebook_param);
                        p_screen_param->p_enter_phonebook_param = NULL;
                        }

                    p_enter_param       = create_app_phonebook_enter_param_fisrt_write_win(NULL,
                        p_screen_param->ui_enter_param.number);
                    p_screen_param->p_enter_phonebook_param = p_enter_param;
                    alv_am_app_enter_action(NULL,"phonebook",p_enter_param);
#endif
                    }
                break;

            case IDLE_NUM_SEND_SMS:
                {
#ifdef CONFIG_APP_SMS_FP
                    void *          p_enter_param;

                    if (p_screen_param->p_enter_sms_param != NULL) {
                        destory_app_sms_enter_param(p_screen_param->p_enter_sms_param);
                        p_screen_param->p_enter_sms_param = NULL;
                        }

                    p_enter_param       = create_app_sms_enter_param_fisrt_write_win(NULL,
                        p_screen_param->ui_enter_param.number);
                    p_screen_param->p_enter_sms_param = p_enter_param;
                    alv_am_app_enter_action(NULL,"sms",p_enter_param);
#endif
                    }
                break;

            case IDLE_NUM_BLACKLIST:
                {
#ifdef CONFIG_APP_PHONEBOOK_FP
                    void *          p_blacklist_enter_param;

                    if (p_screen_param->p_blacklist_enter_param) {
                        destory_app_blacklist_enter_param(p_screen_param->p_blacklist_enter_param,NULL);
                        p_screen_param->p_blacklist_enter_param = NULL;
                        }

                    p_blacklist_enter_param =
                         create_app_blacklist_enter_param_fisrt_add_num(p_screen_param->ui_enter_param.number,
                        app_blacklist_win_exit_fun);

                    if (p_blacklist_enter_param) {
                        p_screen_param->p_blacklist_enter_param = p_blacklist_enter_param;
                        alv_am_app_enter_action(NULL,"blacklist",p_blacklist_enter_param);
                        }

#endif
                    }
                break;
            }
        }
    else if (key == APP_KEY_VALUE_TOP_RIGHT) {
        alv_am_win_exit_current((void *) NULL);
        }
    else if (key == LV_KEY_END) {
        alv_am_app_exit_action_all();
        }
}





