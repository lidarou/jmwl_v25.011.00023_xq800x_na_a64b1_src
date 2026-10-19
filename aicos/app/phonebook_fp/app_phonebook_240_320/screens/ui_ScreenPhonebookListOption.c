
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"
#include "alv_app_mgr.h"
#include "app_log.h"
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
        0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_MID,"电话本"
        },
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"选择"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },

};


enum OBJ_ENTRY_H {
    OBJ_ENTRY_ADD,
    OBJ_ENTRY_FIND,
    OBJ_ENTRY_COPY,
    OBJ_ENTRY_DEL,
    OBJ_ENTRY_BLACKLIST,
    OBJ_ENTRY_SET,
    OBJ_ENTRY_MAX,
};


static obj_batch_label_param_array_t obj_batch_entry_param_array[OBJ_ENTRY_MAX] =
{
    {
        -2,45 * 0,240,45,0,"增加联系人"
        },
    {
        -2,45 * 1,240,45,0,"查找"
        },
    {
        -2,45 * 2,240,45,0,"复制"
        },
    {
        -2,45 * 3,240,45,0,"删除"
        },
    {
        -2,45 * 4,240,45,0,"黑名单"
        },
    {
        -2,45 * 5,240,45,0,"设置"
        },
};


typedef struct ui_screen_phonebook_list_option_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[OBJ_ENTRY_MAX];


} ui_screen_phonebook_list_option_obj_t;


lv_obj_t * ui_ScreenPhonebooklist_option_screen_init(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    int             i,j;
    uint32_t        phonebook_count;
    lv_obj_t *      ui_ScreenPhonebooklist_option = NULL;

    ui_screen_phonebook_list_option_obj_t * p_screen_param_obj;

    phonebook_count     = get_phonebook_ui_used();

    p_screen_param_obj  = lv_mem_alloc(sizeof(ui_screen_phonebook_list_option_obj_t));

    if (p_screen_param_obj == NULL)
        return 0;


    ui_ScreenPhonebooklist_option = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebooklist_option,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebooklist_option,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebooklist_option,&p_screen_param_obj->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);

    p_screen_param_obj->obj_list = lv_obj_create(ui_ScreenPhonebooklist_option);
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

    if (phonebook_count == 0)
        lv_obj_clear_flag(p_screen_param_obj->obj_list,LV_OBJ_FLAG_SCROLLABLE); /// Flags

    //obj_batch_label_create(p_screen_param_obj->obj_list,&p_screen_param_obj->obj_entry[0],NULL,obj_batch_entry_param_array,OBJ_ENTRY_MAX);
    j                   = 0;

    for (i = 0; i < OBJ_ENTRY_MAX; i++) {
        if (0 == get_tele_phonebook_sim_status()) {
            if (i == OBJ_ENTRY_COPY) {
                p_screen_param_obj->obj_entry[i] = 0;
                continue;
                }
            }
        else if (phonebook_count == 0) {
            switch (i)
            {
                case OBJ_ENTRY_FIND:
                case OBJ_ENTRY_COPY:
                case OBJ_ENTRY_DEL:
                    p_screen_param_obj->obj_entry[i] = 0;
                    continue;
                }
            }

        p_screen_param_obj->obj_entry[i] = lv_label_create(p_screen_param_obj->obj_list);

        lv_obj_set_x(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].x);
        lv_obj_set_y(p_screen_param_obj->obj_entry[i],45 * j);
        lv_obj_set_width(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].witdh); /// 1
        lv_obj_set_height(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].high); /// 1

        if (obj_batch_entry_param_array[i].align)
            lv_obj_set_align(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].align);

        lv_label_set_text(p_screen_param_obj->obj_entry[i],obj_batch_entry_param_array[i].text);
        lv_obj_set_user_data(p_screen_param_obj->obj_entry[i],(void *) i);

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
        j++;
        }

    lv_obj_add_state(p_screen_param_obj->obj_entry[0],LV_STATE_FOCUSED);

    app_fp_play_tts_menu(lv_label_get_text(p_screen_param_obj->obj_entry[0]));


    lv_obj_set_user_data(ui_ScreenPhonebooklist_option,p_screen_param_obj);

    lv_obj_add_event_cb(ui_ScreenPhonebooklist_option,phonebook_fp_list_option_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebooklist_option);

    lv_obj_set_user_data(win,ui_ScreenPhonebooklist_option);

    return ui_ScreenPhonebooklist_option;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_phonebook_list_option_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_list_option_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebooklist_option_screen_uninit(lv_obj_t * win,void * p)
{
    lv_obj_t *      ui_ScreenPhonebooklist_option = NULL;

    ui_ScreenPhonebooklist_option = lv_obj_get_user_data(win);

    if (ui_ScreenPhonebooklist_option) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebooklist_option,500,lv_obj_del_screen_ready_cb);
        }

    ui_ScreenPhonebooklist_option = NULL;

    lv_obj_set_user_data(win,NULL);
}



lv_obj_t * get_phonebook_list_option_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_phonebook_list_option_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_list_option_obj_t *) lv_obj_get_user_data (ui_Screen);

    lv_obj_t *      list = p_screen_param_obj->obj_list;

    return list;
}


void ui_ScreenPhonebooklist_option_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebooklist_option = NULL;

    ui_ScreenPhonebooklist_option = lv_obj_get_user_data(win);
    lv_obj_t *      list = get_phonebook_list_option_list_obj(ui_ScreenPhonebooklist_option);

    ;
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;

    get_focus_obj       = lv_obj_get_child(list,child_id);


    lv_disp_load_scr(ui_ScreenPhonebooklist_option);

    app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));

}


void phonebook_list_option_key_event(uint32_t key,lv_obj_t * ui_ScreenPhonebooklist_option)
{
    lv_obj_t *      list = get_phonebook_list_option_list_obj(ui_ScreenPhonebooklist_option);
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;
    lv_obj_t *      pre_focus_obj = NULL;

    switch (key)
    {
        case LV_KEY_UP:
            {
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

                app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));
                }
            break;

        case LV_KEY_DOWN:
            {

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

                app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));
                }
            break;

        case APP_KEY_VALUE_TOP_LEFT:
        case LV_KEY_ENTER:
            {
                get_focus_obj       = lv_obj_get_child(list,child_id);
                child_id            = (uint32_t) lv_obj_get_user_data(get_focus_obj);

                switch (child_id)
                {
                    case OBJ_ENTRY_ADD:
                        {
                            alv_am_win_enter(NULL,"storge_format",NULL);
                            }
                        break;

                    case OBJ_ENTRY_FIND:
                        {
                            alv_am_win_enter(NULL,"pb_find",NULL);
                            }
                        break;

                    case OBJ_ENTRY_COPY:
                        {
                            alv_am_win_enter(NULL,"copy",NULL);
                            }
                        break;

                    case OBJ_ENTRY_DEL:
                        {
                            ui_screen_phonebook_copy_param_t copy_param;

                            copy_param.format   = 0;
                            copy_param.copy_del = 1;
                            alv_am_win_enter(NULL,"copy_select",&copy_param);
                            }
                        break;

                    case OBJ_ENTRY_BLACKLIST:
                        alv_am_app_enter_action(NULL,"blacklist",NULL);
                        break;

                    case OBJ_ENTRY_SET:
                        alv_am_win_enter(NULL,"setup",NULL);
                        break;
                    }
                }
            break;

        case APP_KEY_VALUE_TOP_RIGHT:
            {
                alv_am_win_exit_current((void *) NULL);
                }
            break;

        case LV_KEY_END:
            alv_am_app_exit_action_all();
            break;

        default:
            break;
        }
}


