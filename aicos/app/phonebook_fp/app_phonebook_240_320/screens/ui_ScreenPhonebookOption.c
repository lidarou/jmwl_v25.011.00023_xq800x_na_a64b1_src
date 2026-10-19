
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"
//#include "comp_msgbox.h"
#include "blacklist.h"
#include "app_blacklist.h"
#include "app_sms_list.h"
#include "app_log.h"
#include "app_call_list.h"
#include "alv_app_mgr.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI




enum OBJ_LABEL_H {
    OBJ_LABEL_PHONEBOOK,
    OBJ_LABEL_OPTION,
    OBJ_LABEL_BACK,
    OBJ_LABEL_MAX,
};


static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        0,0,240,LV_SIZE_CONTENT,0,NULL
        },
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"选择"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },

};


enum OBJ_ENTRY_H {
    OBJ_ENTRY_CALL,
    OBJ_ENTRY_SEND_MAESSGE,
    OBJ_ENTRY_SHARE_MESSAGE,
    OBJ_ENTRY_COPY,
    OBJ_ENTRY_COPY1,
    OBJ_ENTRY_DEL,
    OBJ_ENTRY_BLACKLIST,
    OBJ_ENTRY_MAX,
};


static obj_batch_label_param_array_t obj_batch_entry_param_array[OBJ_ENTRY_MAX] =
{
    {
        0,45 * 0,LV_PCT(100),45,0,"呼叫"
        },
    {
        0,45 * 1,LV_PCT(100),45,0,"发送信息"
        },
    {
        0,45 * 2,LV_PCT(100),45,0,"通过短信共享"
        },
    {
        0,45 * 3,LV_PCT(100),45,0,"复制"
        },
    {
        0,45 * 4,LV_PCT(100),45,0,"转存"
        },
    {
        0,45 * 5,LV_PCT(100),45,0,"删除"
        },
    {
        0,45 * 6,LV_PCT(100),45,0,"加至黑名单"
        },
};


typedef struct ui_screen_phonebook_option_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[OBJ_ENTRY_MAX];


    lv_style_t      style_title;

    ui_screen_phonebook_enter_param_t enter_param;

    void *          p_sms_enter_param;
    uint8_t *       p_share_data;

    void *          p_blacklist_enter_param;
    blacklist_data_t * p_blacklist_data;
    uint32_t        blacklist_param_index;

} ui_screen_phonebook_option_obj_t;


lv_obj_t * ui_ScreenPhonebookoption_screen_init(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookoption = NULL;

    int             i,j;
    phonebook_param_t phonebook_param;

    ui_screen_phonebook_option_obj_t * p_screen_param_obj;

    p_screen_param_obj  = lv_mem_alloc(sizeof(ui_screen_phonebook_option_obj_t));

    if (p_screen_param_obj == NULL)
        return 0;

    memset(p_screen_param_obj,0,sizeof(ui_screen_phonebook_option_obj_t));

    if (enter_param != NULL) {
        memcpy(&p_screen_param_obj->enter_param,enter_param,sizeof(ui_screen_phonebook_enter_param_t));
        memcpy(&phonebook_param,&p_screen_param_obj->enter_param.phonebook_param,sizeof(phonebook_param_t));
        }

    ui_ScreenPhonebookoption = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebookoption,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebookoption,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebookoption,&p_screen_param_obj->obj_label[1],NULL,
        &obj_batch_label_param_array[1],OBJ_LABEL_MAX - 1);

    p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK] = lv_label_create(ui_ScreenPhonebookoption);
    lv_label_set_long_mode(p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK],LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_anim_speed(p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK],10,0);
    lv_obj_set_x(p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK],
        obj_batch_label_param_array[OBJ_LABEL_PHONEBOOK].x);
    lv_obj_set_y(p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK],
        obj_batch_label_param_array[OBJ_LABEL_PHONEBOOK].y);
    lv_obj_set_width(p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK],
        obj_batch_label_param_array[OBJ_LABEL_PHONEBOOK].witdh); /// 1
    lv_obj_set_height(p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK],
        obj_batch_label_param_array[OBJ_LABEL_PHONEBOOK].high); /// 1
    lv_label_set_text(p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK],phonebook_param.name);

    lv_style_init(&p_screen_param_obj->style_title);
    lv_style_set_text_font(&p_screen_param_obj->style_title,(lv_font_t *) get_ft_info_font_common());
    lv_style_set_text_align(&p_screen_param_obj->style_title,LV_TEXT_ALIGN_CENTER);
    lv_obj_add_style(p_screen_param_obj->obj_label[OBJ_LABEL_PHONEBOOK],&p_screen_param_obj->style_title,0);


    p_screen_param_obj->obj_list = lv_obj_create(ui_ScreenPhonebookoption);
    lv_obj_set_width(p_screen_param_obj->obj_list,240);
    lv_obj_set_height(p_screen_param_obj->obj_list,228);
    lv_obj_set_x(p_screen_param_obj->obj_list,0);
    lv_obj_set_y(p_screen_param_obj->obj_list,50);
    lv_obj_set_align(p_screen_param_obj->obj_list,LV_ALIGN_TOP_LEFT);
    lv_obj_add_flag(p_screen_param_obj->obj_list,LV_OBJ_FLAG_SCROLL_WITH_ARROW); /// Flags
    lv_obj_set_scrollbar_mode(p_screen_param_obj->obj_list,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(p_screen_param_obj->obj_list,LV_DIR_VER);
    lv_obj_set_style_radius(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    //lv_obj_set_style_bg_color(ui_PnlSettings, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
    //lv_obj_set_style_bg_opa(ui_PnlSettings, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(p_screen_param_obj->obj_list,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    //obj_batch_label_create(p_screen_param_obj->obj_list,&p_screen_param_obj->obj_entry[0],NULL,obj_batch_entry_param_array,OBJ_ENTRY_MAX);
    j                   = 0;

    for (i = 0; i < OBJ_ENTRY_MAX; i++) {
        if (0 == get_tele_phonebook_sim_status()) {
            if ((i == OBJ_ENTRY_COPY) || (i == OBJ_ENTRY_COPY1)) {
                p_screen_param_obj->obj_entry[i] = 0;
                continue;
                }
            }

        p_screen_param_obj->obj_entry[i] = lv_label_create(p_screen_param_obj->obj_list);

        lv_label_set_long_mode(p_screen_param_obj->obj_entry[i],LV_LABEL_LONG_CLIP);
        lv_obj_set_style_anim_speed(p_screen_param_obj->obj_entry[i],10,0);
        lv_obj_set_x(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].x);
        lv_obj_set_y(p_screen_param_obj->obj_entry[i],45 * j);
        lv_obj_set_width(p_screen_param_obj->obj_entry[i],240); /// 1
        lv_obj_set_height(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].high); /// 1

        if (obj_batch_entry_param_array[i].align)
            lv_obj_set_align(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].align);

        lv_label_set_text(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].text);


        lv_obj_set_style_bg_color(p_screen_param_obj->obj_entry[i],PHONEBOOK_NAME_TEXT_BG_COLOR,
            LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(p_screen_param_obj->obj_entry[i],LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_text_color(p_screen_param_obj->obj_entry[i],PHONEBOOK_NAME_TEXT_COLOR,
            LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(p_screen_param_obj->obj_entry[i],LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_text_color(p_screen_param_obj->obj_entry[i],PHONEBOOK_NAME_TEXT_COLOR_FOCUS,
            LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(p_screen_param_obj->obj_entry[i],PHONEBOOK_NAME_TEXT_BG_COLOR_FOCUS,
            LV_PART_MAIN | LV_STATE_FOCUSED);

        lv_obj_set_user_data(p_screen_param_obj->obj_entry[i],(void *) i);
        j++;
        }

    lv_obj_add_state(p_screen_param_obj->obj_entry[0],LV_STATE_FOCUSED);

    if (enter_param != NULL) {
        memcpy(&p_screen_param_obj->enter_param,enter_param,sizeof(ui_screen_phonebook_enter_param_t));
        }

#if 0
    int32_t         ret;

    ret                 = blacklist_have_this_number(&p_screen_param_obj->p_blacklist_data,
        &p_screen_param_obj->blacklist_param_index,enter_param->phonebook_param.number);

    if (ret) {
        lv_label_set_text(p_screen_param_obj->obj_entry[OBJ_ENTRY_BLACKLIST],"从黑名单列表删除");
        }

#endif



    lv_obj_set_user_data(ui_ScreenPhonebookoption,p_screen_param_obj);

    lv_obj_add_event_cb(ui_ScreenPhonebookoption,phonebook_fp_option_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebookoption);

    lv_obj_set_user_data(win,ui_ScreenPhonebookoption);


    return ui_ScreenPhonebookoption;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_phonebook_option_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_option_obj_t *) lv_obj_get_user_data(a->var);

#ifdef CONFIG_APP_SMS_FP

    if (p_screen_param_obj->p_sms_enter_param)
        destory_app_sms_enter_param(p_screen_param_obj->p_sms_enter_param);

#endif

    if (p_screen_param_obj->p_share_data)
        lv_mem_free(p_screen_param_obj->p_share_data);

    if (p_screen_param_obj->p_blacklist_enter_param)
        destory_app_blacklist_enter_param(p_screen_param_obj->p_blacklist_enter_param,NULL);

    if (p_screen_param_obj->p_blacklist_data)
        lv_mem_free(p_screen_param_obj->p_blacklist_data);

    lv_style_reset(&p_screen_param_obj->style_title);

    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebookoption_screen_uninit(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookoption = NULL;

    ui_ScreenPhonebookoption = lv_obj_get_user_data(win);

    if (ui_ScreenPhonebookoption) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebookoption,100,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}







lv_obj_t * get_phonebook_option_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_phonebook_option_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_option_obj_t *) lv_obj_get_user_data(ui_Screen);

    lv_obj_t *      list = p_screen_param_obj->obj_list;

    return list;
}


void ui_ScreenPhonebookoption_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookoption = NULL;

    ui_ScreenPhonebookoption = lv_obj_get_user_data(win);
    lv_obj_t *      list = get_phonebook_option_list_obj(ui_ScreenPhonebookoption);

    ;
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;

    get_focus_obj       = lv_obj_get_child(list,child_id);

    lv_disp_load_scr(ui_ScreenPhonebookoption);

    app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));


}


uint32_t get_phonebook_option_list_child_id(lv_obj_t * list,uint32_t child_cnt)
{
    /*Modify manually*/
    lv_obj_t *      ui_entry_child = NULL;
    uint32_t        child_id;

    for (child_id = 0; child_id < child_cnt; child_id++) {
        ui_entry_child      = lv_obj_get_child(list,child_id);

        if (lv_obj_has_state(ui_entry_child,LV_STATE_FOCUSED)) {
            break;
            }

        }

    if (child_id >= child_cnt) {
        child_id            = 0;
        }

    //__inf("get_setting_childId:%d,child_cnt:%d\r\n",child_id,child_cnt);
    return child_id;
}


ui_screen_phonebook_enter_param_t * get_ui_ScreenPhonebookOption_enter_param(ui_screen_phonebook_option_obj_t * option_obj)
{
    if (option_obj != NULL)
        return & option_obj->enter_param;

    return 0;
}



void DialNumber(lv_obj_t * ui_ScreenPhonebookoption)
{
#ifdef CONFIG_APP_CALL_FP
    ui_screen_phonebook_option_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_option_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookoption);

    call_dial_number(p_screen_param_obj->enter_param.phonebook_param.number,
        p_screen_param_obj->enter_param.phonebook_param.name);

#endif
}


static void app_blacklist_win_exit_fun(void)
{
    alv_am_win_exit_action_by_app("phonebook","option",NULL);
    alv_am_win_exit_current(NULL);
}


void phonebook_option_key_event(uint32_t key,lv_obj_t * ui_ScreenPhonebookoption)
{
    ui_screen_phonebook_option_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_option_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookoption);
    lv_obj_t *      list = get_phonebook_option_list_obj(ui_ScreenPhonebookoption);

    ;
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_phonebook_option_list_child_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;
    lv_obj_t *      pre_focus_obj = NULL;
    lv_obj_t *      label;


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

        label               = pre_focus_obj;        //lv_obj_get_child(pre_focus_obj, 1);
        lv_label_set_long_mode(label,LV_LABEL_LONG_CLIP);

        label               = get_focus_obj;        //lv_obj_get_child(get_focus_obj, 1);
        lv_label_set_long_mode(label,LV_LABEL_LONG_SCROLL);

        app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));
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

        label               = pre_focus_obj;        //lv_obj_get_child(pre_focus_obj, 1);
        lv_label_set_long_mode(label,LV_LABEL_LONG_CLIP);

        label               = get_focus_obj;        //lv_obj_get_child(get_focus_obj, 1);
        lv_label_set_long_mode(label,LV_LABEL_LONG_SCROLL);
        app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));
        }
    else if ((key == LV_KEY_ENTER) || (key == APP_KEY_VALUE_TOP_LEFT)) {
        uint32_t        id;

        get_focus_obj       = lv_obj_get_child(list,child_id);
        id                  = (uint32_t) lv_obj_get_user_data(get_focus_obj);

        switch (id)
        {
            case OBJ_ENTRY_CALL:
                {
                    DialNumber(ui_ScreenPhonebookoption);
                    }
                break;

            case OBJ_ENTRY_SEND_MAESSGE:
#ifdef CONFIG_APP_SMS_FP
                {
                    void *          p_sms_enter_param;

                    if (p_screen_param_obj->p_sms_enter_param) {
                        destory_app_sms_enter_param(p_screen_param_obj->p_sms_enter_param);
                        p_screen_param_obj->p_sms_enter_param = NULL;
                        }

                    p_sms_enter_param   = create_app_sms_enter_param_fisrt_write_win(NULL,
                        p_screen_param_obj->enter_param.phonebook_param.number);

                    if (p_sms_enter_param) {
                        p_screen_param_obj->p_sms_enter_param = p_sms_enter_param;
                        alv_am_app_enter_action(NULL,"sms",p_sms_enter_param);
                        }

                    }
#endif

                break;

            case OBJ_ENTRY_SHARE_MESSAGE:
#ifdef CONFIG_APP_SMS_FP
                {
                    void *          p_sms_enter_param;
                    uint32_t        share_data_size;

                    if (p_screen_param_obj->p_sms_enter_param) {
                        destory_app_sms_enter_param(p_screen_param_obj->p_sms_enter_param);
                        p_screen_param_obj->p_sms_enter_param = NULL;
                        }

                    if (p_screen_param_obj->p_share_data == NULL) {
                        share_data_size     =
                             strlen(p_screen_param_obj->enter_param.phonebook_param.name) +strlen(p_screen_param_obj->enter_param.phonebook_param.number) + 4;
                        p_screen_param_obj->p_share_data = lv_mem_alloc(share_data_size);

                        if (p_screen_param_obj->p_share_data) {
                            strcpy(p_screen_param_obj->p_share_data,
                                p_screen_param_obj->enter_param.phonebook_param.name);
                            strcat(p_screen_param_obj->p_share_data,"\n");
                            strcat(p_screen_param_obj->p_share_data,
                                p_screen_param_obj->enter_param.phonebook_param.number);
                            }
                        else {
                            break;
                            }
                        }

                    p_sms_enter_param   =
                         (void *) create_app_sms_enter_param_fisrt_write_win(p_screen_param_obj->p_share_data,NULL);

                    if (p_sms_enter_param) {
                        p_screen_param_obj->p_sms_enter_param = p_sms_enter_param;
                        alv_am_app_enter_action(NULL,"sms",p_sms_enter_param);
                        }

                    }
#endif

                break;

            case OBJ_ENTRY_COPY:
                {
                    ui_screen_phonebook_option_copy_param_t enter_param;

                    enter_param.p_phonebook_enter_param = &p_screen_param_obj->enter_param;
                    enter_param.copy_del = 0;
                    alv_am_win_enter(NULL,"option_copy",&enter_param);
                    }
                break;

            case OBJ_ENTRY_COPY1:
                {
                    ui_screen_phonebook_option_copy_param_t enter_param;

                    enter_param.p_phonebook_enter_param = &p_screen_param_obj->enter_param;
                    enter_param.copy_del = 2;
                    alv_am_win_enter(NULL,"option_copy",&enter_param);
                    }
                break;

            case OBJ_ENTRY_DEL:
                {
                    ui_screen_phonebook_option_copy_param_t enter_param;

                    enter_param.p_phonebook_enter_param = &p_screen_param_obj->enter_param;
                    enter_param.copy_del = 1;
                    alv_am_win_enter(NULL,"option_del",&enter_param);
                    }
                break;

            case OBJ_ENTRY_BLACKLIST:
                {
                    if (p_screen_param_obj->p_blacklist_data) {
                        void *          p_blacklist_enter_param;

                        if (p_screen_param_obj->p_blacklist_enter_param) {
                            destory_app_blacklist_enter_param(p_screen_param_obj->p_blacklist_enter_param,NULL);
                            p_screen_param_obj->p_blacklist_enter_param = NULL;
                            }

                        p_blacklist_enter_param =
                             (void *) create_app_blacklist_enter_param_fisrt_del_num(p_screen_param_obj->p_blacklist_data,
                            p_screen_param_obj->blacklist_param_index,app_blacklist_win_exit_fun);

                        if (p_blacklist_enter_param) {
                            p_screen_param_obj->p_blacklist_enter_param = p_blacklist_enter_param;
                            alv_am_app_enter_action(NULL,"blacklist",p_blacklist_enter_param);
                            }
                        }
                    else {
                        void *          p_blacklist_enter_param;

                        if (p_screen_param_obj->p_blacklist_enter_param) {
                            destory_app_blacklist_enter_param(p_screen_param_obj->p_blacklist_enter_param,NULL);
                            p_screen_param_obj->p_blacklist_enter_param = NULL;
                            }

                        p_blacklist_enter_param =
                             create_app_blacklist_enter_param_fisrt_add_num(p_screen_param_obj->enter_param.phonebook_param.number,
                            app_blacklist_win_exit_fun);

                        if (p_blacklist_enter_param) {
                            p_screen_param_obj->p_blacklist_enter_param = p_blacklist_enter_param;
                            alv_am_app_enter_action(NULL,"blacklist",p_blacklist_enter_param);
                            }
                        }



                    }
                break;
            }
        }
    else if (key == APP_KEY_VALUE_TOP_RIGHT) {
        alv_am_win_exit_current(NULL);
        }
    else if (key == LV_KEY_END) {
        alv_am_app_exit_action_all();
        }


}


