
#include "ui.h"
#include "alv_app_mgr.h"
#include "lv_port_indev_keypad.h"
#include "app_log.h"
#include "obj_batch_deal.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI


enum {
    OBJ_LABEL_1_PHONEBOOK,

    //OBJ_LABEL_NAME,
    //OBJ_LABEL_NUM,
    OBJ_LABEL_1_CONTENT,
    OBJ_LABEL_1_OPTION,
    OBJ_LABEL_1_BACK,
    OBJ_LABEL_1_EDIT,
    OBJ_LABEL_1_MAX,
};

enum OBJ_LABEL_H {
    //	OBJ_LABEL_PHONEBOOK,
    OBJ_LABEL_2_OPTION,
    OBJ_LABEL_2_BACK,
    OBJ_LABEL_2_MAX,
};

enum OBJ_ENTRY_H {
    OBJ_ENTRY_PHONE,
    OBJ_ENTRY_SIM,
    OBJ_ENTRY_MAX,
};

typedef struct ui_screen_phonebook_details_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_1_MAX];
    ui_screen_phonebook_enter_param_t enter_param;
} ui_screen_phonebook_details_obj_t;

typedef struct ui_screen_phonebook_storge_format_obj_h {
    lv_obj_t *      obj_label_title;
    lv_obj_t *      obj_label[OBJ_LABEL_2_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[OBJ_ENTRY_MAX];
    uint32_t        sim_status;
    ui_screen_phonebook_enter_param_t enter_param;

} ui_screen_phonebook_storge_format_obj_t;

extern lv_obj_t * g_ta1;
extern lv_obj_t * g_pinyin_ime;
extern lv_obj_t * g_kb;
extern lv_obj_t * ui_screenpinyin;
extern void ui_Screenpinyin_screen_init(void);
extern ui_screen_phonebook_enter_param_t * get_ui_ScreenPhonebookDetails_enter_param(ui_screen_phonebook_details_obj_t * details_obj);
extern ui_screen_phonebook_enter_param_t * get_ui_ScreenPhonebookstorge_format_enter_param(ui_screen_phonebook_storge_format_obj_t * storge_format_obj);

/**********************
 *   STATIC FUNCTIONS
 **********************/
uint32_t get_setting_childId(uint32_t child_cnt)
{
    /*Modify manually*/
    lv_obj_t *      ui_setting_child = NULL;
    uint32_t        child_id;

    for (child_id = 0; child_id < child_cnt; child_id++) {
        ui_setting_child    = lv_obj_get_child(ui_PnlSettings,child_id);

        if (lv_obj_has_state(ui_setting_child,LV_STATE_FOCUSED)) {
            break;
            }

        }

    if (child_id >= child_cnt) {
        child_id            = 0;
        }

    //__inf("get_setting_childId:%d,child_cnt:%d\r\n",child_id,child_cnt);
    return child_id;
}



/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void phonebook_fp_app_ui_event_actions(lv_event_t * e)
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

                phonebooklist_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void phonebook_fp_details_ui_event_actions(lv_event_t * e)
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


                if (key == LV_KEY_UP) {

                    }
                else if (key == LV_KEY_DOWN) {

                    }
                else if (key == APP_KEY_VALUE_TOP_LEFT) {
                    alv_am_win_enter(NULL,"option",
                        (void *) get_ui_ScreenPhonebookDetails_enter_param(obj->user_data));
                    }
                else if (key == APP_KEY_VALUE_TOP_RIGHT) {
                    alv_am_win_exit_current((void *) get_ui_ScreenPhonebookDetails_enter_param(obj->user_data));
                    }
                else if (key == LV_KEY_ENTER) {
                    alv_am_win_enter(NULL,"edit",
                        (void *) get_ui_ScreenPhonebookDetails_enter_param(obj->user_data));
                    }
                else if (key == LV_KEY_HOME) {
                    ui_ScreenPhonebookDetailsDialNumber(obj);

                    }
                else if (key == LV_KEY_END) {
                    alv_am_app_exit_action_all();

                    }
                }
            break;

        default:
            break;
        }


}



void phonebook_fp_edit_ui_event_actions(lv_event_t * e)
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
                    phonebook_edit_pinyin_create(obj);

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

#if 1
                phonebook_edit_key_event(key,obj);

#else

                //__inf("phonebook_fp_edit_ui_event_actions LV_EVENT_KEY:%d\r\n",key);
                if (get_ime_pinyin_keyboard_active_state()) {
                    phonebook_edit_pinyin_key_event(key,obj);
                    }
                else {
                    if (key == LV_KEY_UP) {
                        if (is_phonebook_edit_name_fucous(obj))
                            set_phonebook_edit_num_fucous(obj);
                        else 
                            set_phonebook_edit_name_fucous(obj);
                        }
                    else if (key == LV_KEY_DOWN) {
                        if (is_phonebook_edit_name_fucous(obj))
                            set_phonebook_edit_num_fucous(obj);
                        else 
                            set_phonebook_edit_name_fucous(obj);
                        }
                    else if (key == APP_KEY_VALUE_TOP_LEFT) {
                        set_phonebook_edit_fucous_cursor_left(obj);
                        }
                    else if (key == APP_KEY_VALUE_TOP_RIGHT) {
                        //alv_am_win_exit_current(NULL);					
                        set_phonebook_edit_fucous_cursor_right(obj);
                        }
                    else if (key == LV_KEY_ENTER) {

                        }
                    else {
                        phonebook_edit_pinyin_key_event(key,obj);
                        }
                    }

#endif
                }
            break;

        default:
            break;
        }


}





void phonebook_fp_list_option_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                phonebook_list_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}




void phonebook_fp_remind_ui_event_actions(lv_event_t * e)
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
                    phonebook_remind_timer_start(obj);

                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    ui_ScreenPhonebookRemind_screen_uninit();
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                //uint32_t key = lv_event_get_key(e);
                //lv_obj_t *obj = e->user_data;
                //alv_am_app_exit_action(NULL,"phonebook",NULL);				
                //alv_am_app_enter_action(NULL,"home_fp",NULL);
                }
            break;

        default:
            break;
        }


}


void phonebook_fp_storge_format_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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
                lv_obj_t *      list = (lv_obj_t *) get_phonebook_storge_format_list_obj(obj);
                uint32_t        child_cnt = lv_obj_get_child_cnt(list);
                uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
                lv_obj_t *      get_focus_obj = NULL;
                lv_obj_t *      pre_focus_obj = NULL;


                if (key == LV_KEY_UP) {
                    //__inf("LV_KEY_UP,child_id:%d,child_cnt:%d\r\n",child_id,child_cnt);
                    if (child_cnt <= 1)
                        break;

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
                else if (key == LV_KEY_DOWN) {
                    if (child_cnt <= 1)
                        break;

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
                else if (key == APP_KEY_VALUE_TOP_RIGHT) {
                    alv_am_win_exit_current(NULL);  //((void*)get_ui_ScreenPhonebookstorge_format_enter_param(obj->user_data));
                    }
                else if ((key == APP_KEY_VALUE_TOP_LEFT) || (key == LV_KEY_ENTER)) {
                    ui_ScreenPhonebookstorge_format_screen_key_enter(obj,child_id);
                    }
                else if (key == LV_KEY_END) {
                    alv_am_app_exit_action_all();

                    }
                }
            break;

        default:
            break;
        }


}


void phonebook_fp_copy_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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
                lv_obj_t *      list = (lv_obj_t *) get_phonebook_copy_list_obj(obj);

                ;
                uint32_t        child_cnt = lv_obj_get_child_cnt(list);
                uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
                lv_obj_t *      get_focus_obj = NULL;
                lv_obj_t *      pre_focus_obj = NULL;


                if (key == LV_KEY_UP) {
                    //__inf("LV_KEY_UP,child_id:%d,child_cnt:%d\r\n",child_id,child_cnt);
                    if (child_cnt <= 1)
                        break;

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
                else if (key == LV_KEY_DOWN) {
                    if (child_cnt <= 1)
                        break;

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
                else if (key == APP_KEY_VALUE_TOP_RIGHT) {
                    alv_am_win_exit_current((void *) NULL);
                    }
                else if ((key == APP_KEY_VALUE_TOP_LEFT) || (key == LV_KEY_ENTER)) {
                    ui_ScreenPhonebookCopy_screen_key_enter(obj,child_id);
                    }
                else if (key == LV_KEY_END) {
                    alv_am_app_exit_action_all();

                    }
                }
            break;

        default:
            break;
        }


}


void phonebook_fp_copy_select_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                phonebook_copy_select_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void phonebook_fp_remindcopy_ui_event_actions(lv_event_t * e)
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

                    //ui_ScreenPhonebookRemindCopy_screen_uninit();
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                //alv_am_app_exit_action(NULL,"phonebook",NULL);				
                //alv_am_app_enter_action(NULL,"home_fp",NULL);
                phonebook_copy_remind_key_event(key,obj);

                }
            break;

        default:
            break;
        }


}



void phonebook_fp_setup_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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
                lv_obj_t *      list = (lv_obj_t *) get_phonebook_setup_list_obj(obj);

                ;
                uint32_t        child_cnt = lv_obj_get_child_cnt(list);
                uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
                lv_obj_t *      get_focus_obj = NULL;
                lv_obj_t *      pre_focus_obj = NULL;


                if (key == LV_KEY_UP) {
                    //__inf("LV_KEY_UP,child_id:%d,child_cnt:%d\r\n",child_id,child_cnt);
                    if (child_cnt <= 1)
                        break;

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
                else if (key == LV_KEY_DOWN) {
                    if (child_cnt <= 1)
                        break;

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
                else if (key == APP_KEY_VALUE_TOP_RIGHT) {
                    alv_am_win_exit_current((void *) NULL);
                    }
                else if ((key == APP_KEY_VALUE_TOP_LEFT) || (key == LV_KEY_ENTER)) {
                    if (child_id == 0)
                        alv_am_win_enter(NULL,"storge_inf",(void *) NULL);
                    else if (child_id == 1)
                        alv_am_win_enter(NULL,"set_storge",(void *) NULL);

                    }
                else if (key == LV_KEY_END) {
                    alv_am_app_exit_action_all();

                    }
                }
            break;

        default:
            break;
        }


}


void phonebook_fp_storge_infromation_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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
                lv_obj_t *      list = (lv_obj_t *) get_phonebook_storge_infromation__obj(obj);
                uint32_t        child_cnt = lv_obj_get_child_cnt(list);
                lv_obj_t *      get_end_obj = lv_obj_get_child(list,child_cnt - 1);

                if (key == LV_KEY_UP) {
                    if (child_cnt < 6)
                        return;

                    if (lv_obj_get_scroll_y(list))
                        lv_obj_scroll_to_y(list,0,LV_ANIM_OFF);
                    else 
                        lv_obj_scroll_to_view(get_end_obj,LV_ANIM_OFF);
                    }
                else if (key == LV_KEY_DOWN) {
                    if (child_cnt < 6)
                        return;

                    if (lv_obj_get_scroll_y(list))
                        lv_obj_scroll_to_y(list,0,LV_ANIM_OFF);
                    else 
                        lv_obj_scroll_to_view(get_end_obj,LV_ANIM_OFF);
                    }
                else if (key == APP_KEY_VALUE_TOP_LEFT) {
                    }
                else if (key == APP_KEY_VALUE_TOP_RIGHT) {
                    alv_am_win_exit_current((void *) NULL);
                    }
                else if (key == LV_KEY_ENTER) {
                    //alv_am_win_enter(NULL,"copy_select",(void*)&copy_param); 		
                    }
                else if (key == LV_KEY_END) {
                    alv_am_app_exit_action_all();

                    }
                }
            break;

        default:
            break;
        }


}


void phonebook_fp_setup_stroge_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                phonebook_setup_stroge_key_event(key,obj);
                }
            break;

        default:
            break;
        }
}



void phonebook_fp_option_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                phonebook_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_phonebook_option_copy_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                phonebook_option_copy_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}




void fp_phonebook_option_del_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                phonebook_option_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}



void phonebook_fp_find_app_ui_event_actions(lv_event_t * e)
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
                    phonebook_find_pinyin_create(obj);
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

                phonebookfind_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


























void fp_blacklist_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                blacklist_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}





void fp_blacklist_option_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                blacklist_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}




void fp_blacklist_add_num_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    blacklist_add_num_pinyin_create(obj);
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

                blacklist_add_num_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}


void fp_blacklist_del_num_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                blacklist_del_num_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}


void fp_blacklist_find_app_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                blacklist_find_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}



void fp_blacklist_add_num_remind_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

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

                blacklist_add_num_remind_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}






