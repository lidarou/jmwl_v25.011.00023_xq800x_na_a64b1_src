
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"
#include <stdio.h>
#include "app_log.h"
#include "alv_app_mgr.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI




enum {
    OBJ_LABEL_CONTONS,
    OBJ_LABEL_STORGE,
    OBJ_LABEL_BACK,
    OBJ_LABEL_OPTION,
    OBJ_LABEL_MAX,
};


static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_MID,"显示联系人"
        },
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"存储"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },
    {
        100,290,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"选择"
        },

};


static const lv_img_dsc_t * ui_imgset_select[2] =
{
    &ui_img_phonebooklist_check_box_png,&ui_img_phonebooklist_check_box_pre_png
};


typedef struct ui_screen_phonebook_setup_stroge_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[2];
    uint8_t *       p_state;
    uint8_t         state_count;
} ui_screen_phonebook_setup_stroge_obj_t;


lv_obj_t * ui_ScreenPhonebooksetup_stroge_screen_init(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebooksetup_stroge = NULL;

    ui_screen_phonebook_setup_stroge_obj_t * p_screen_param_obj;
    uint8_t         str_buf[10];

    p_screen_param_obj  = lv_mem_alloc(sizeof(ui_screen_phonebook_setup_stroge_obj_t));

    if (p_screen_param_obj == NULL)
        return 0;

    p_screen_param_obj->state_count = 2;
    p_screen_param_obj->p_state = lv_mem_alloc(p_screen_param_obj->state_count);
    memset(p_screen_param_obj->p_state,0,p_screen_param_obj->state_count);



    ui_ScreenPhonebooksetup_stroge = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebooksetup_stroge,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebooksetup_stroge,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebooksetup_stroge,&p_screen_param_obj->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);
    lv_obj_add_style(p_screen_param_obj->obj_label[OBJ_LABEL_OPTION],(lv_style_t *) get_lv_style_font_custom(20),0);

    p_screen_param_obj->obj_list = lv_obj_create(ui_ScreenPhonebooksetup_stroge);
    lv_obj_set_width(p_screen_param_obj->obj_list,240);
    lv_obj_set_height(p_screen_param_obj->obj_list,228);
    lv_obj_set_x(p_screen_param_obj->obj_list,0);
    lv_obj_set_y(p_screen_param_obj->obj_list,50);
    lv_obj_set_align(p_screen_param_obj->obj_list,LV_ALIGN_TOP_LEFT);
    lv_obj_add_flag(p_screen_param_obj->obj_list,LV_OBJ_FLAG_SCROLL_WITH_ARROW); /// Flags
    lv_obj_set_scrollbar_mode(p_screen_param_obj->obj_list,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(p_screen_param_obj->obj_list,LV_DIR_VER);
    lv_obj_set_style_radius(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(p_screen_param_obj->obj_list,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_screen_param_obj->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(p_screen_param_obj->obj_list,LV_OBJ_FLAG_SCROLLABLE);


    lv_obj_t *      ui_LabelEntry;
    lv_obj_t *      ui_ImageIcon;
    uint8_t         display_mode;

    display_mode        = tele_phone_data_get_display_mode();

    __inf("get display_mode:%d",display_mode);

    for (int i = 0; i < 2; i++) {

        p_screen_param_obj->obj_entry[i] = lv_obj_create(p_screen_param_obj->obj_list);
        lv_obj_set_x(p_screen_param_obj->obj_entry[i],-2);
        lv_obj_set_y(p_screen_param_obj->obj_entry[i],45 * i);
        lv_obj_set_width(p_screen_param_obj->obj_entry[i],240); /// 1
        lv_obj_set_height(p_screen_param_obj->obj_entry[i],45); /// 1
        lv_obj_set_style_pad_left(p_screen_param_obj->obj_entry[i],0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(p_screen_param_obj->obj_entry[i],0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(p_screen_param_obj->obj_entry[i],0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(p_screen_param_obj->obj_entry[i],0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_side(p_screen_param_obj->obj_entry[i],LV_BORDER_SIDE_NONE,
            LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(p_screen_param_obj->obj_entry[i],0,LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(p_screen_param_obj->obj_entry[i],LV_OBJ_FLAG_SCROLLABLE);

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



        ui_ImageIcon        = lv_img_create(p_screen_param_obj->obj_entry[i]);

        if (display_mode & (1 << i)) {
            lv_img_set_src(ui_ImageIcon,ui_imgset_select[1]);
            p_screen_param_obj->p_state[i] = 1;
            }
        else {
            lv_img_set_src(ui_ImageIcon,ui_imgset_select[0]);
            p_screen_param_obj->p_state[i] = 0;
            }

        lv_obj_set_width(ui_ImageIcon,LV_SIZE_CONTENT); /// 1
        lv_obj_set_height(ui_ImageIcon,LV_SIZE_CONTENT); /// 1
        lv_obj_set_x(ui_ImageIcon,2);
        lv_obj_set_y(ui_ImageIcon,12);
        lv_obj_set_align(ui_ImageIcon,LV_ALIGN_TOP_LEFT);
        lv_obj_add_flag(ui_ImageIcon,LV_OBJ_FLAG_ADV_HITTEST); /// Flags
        lv_obj_clear_flag(ui_ImageIcon,LV_OBJ_FLAG_SCROLLABLE); /// Flags


        ui_LabelEntry       = lv_label_create(p_screen_param_obj->obj_entry[i]);
        lv_obj_set_width(ui_LabelEntry,170);
        lv_obj_set_height(ui_LabelEntry,45);
        lv_obj_set_x(ui_LabelEntry,24);
        lv_obj_set_y(ui_LabelEntry,0);
        lv_obj_set_align(ui_LabelEntry,LV_ALIGN_TOP_LEFT);

        if (i == 0) {
            lv_label_set_text(ui_LabelEntry,"手机");
            }
        else if (i == 1) {
            memset(str_buf,0,10);
            sprintf(str_buf,"SIM%d",(int) get_tele_phonebook_sim_status());
            lv_label_set_text(ui_LabelEntry,str_buf);
            }

        }



    lv_obj_add_state(p_screen_param_obj->obj_entry[0],LV_STATE_FOCUSED);


    lv_obj_set_user_data(ui_ScreenPhonebooksetup_stroge,p_screen_param_obj);

    lv_obj_add_event_cb(ui_ScreenPhonebooksetup_stroge,phonebook_fp_setup_stroge_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebooksetup_stroge);

    lv_obj_set_user_data(win,ui_ScreenPhonebooksetup_stroge);

    return ui_ScreenPhonebooksetup_stroge;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_phonebook_setup_stroge_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_setup_stroge_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj->p_state);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebooksetup_stroge_screen_uninit(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebooksetup_stroge = NULL;

    ui_ScreenPhonebooksetup_stroge = lv_obj_get_user_data(win);

    if (ui_ScreenPhonebooksetup_stroge) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebooksetup_stroge,200,lv_obj_del_screen_ready_cb);
        }

    ui_ScreenPhonebooksetup_stroge = NULL;

    lv_obj_set_user_data(win,NULL);
}


lv_obj_t * get_phonebook_setup_stroge_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_phonebook_setup_stroge_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_setup_stroge_obj_t *) lv_obj_get_user_data (ui_Screen);

    lv_obj_t *      list = p_screen_param_obj->obj_list;

    return list;
}


void ui_ScreenPhonebooksetup_stroge_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebooksetup_stroge = NULL;

    ui_ScreenPhonebooksetup_stroge = lv_obj_get_user_data(win);
    lv_obj_t *      list = get_phonebook_setup_stroge_list_obj(ui_ScreenPhonebooksetup_stroge);

    ;
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;
    lv_obj_t *      label;

    get_focus_obj       = lv_obj_get_child(list,child_id);
    label               = lv_obj_get_child(get_focus_obj,1);

    lv_disp_load_scr(ui_ScreenPhonebooksetup_stroge);

    app_fp_play_tts_menu(lv_label_get_text(label));

}


void phonebook_setup_stroge_key_event(uint32_t key,lv_obj_t * ui_Screen)
{
    ui_screen_phonebook_setup_stroge_obj_t * p_screen_param_obj;
    lv_obj_t *      list = get_phonebook_setup_stroge_list_obj(ui_Screen);
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;
    lv_obj_t *      pre_focus_obj = NULL;

    p_screen_param_obj  = (ui_screen_phonebook_setup_stroge_obj_t *) lv_obj_get_user_data(ui_Screen);
    lv_obj_t *      label;

    if (key == LV_KEY_UP) {
        //__inf("LV_KEY_UP,child_id:%d,child_cnt:%d\r\n",child_id,child_cnt);
        if (child_cnt <= 1)
            return;

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

        label               = lv_obj_get_child(get_focus_obj,1);
        app_fp_play_tts_menu(lv_label_get_text(label));
        }
    else if (key == LV_KEY_DOWN) {
        if (child_cnt <= 1)
            return;

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

        label               = lv_obj_get_child(get_focus_obj,1);
        app_fp_play_tts_menu(lv_label_get_text(label));
        }
    else if (key == APP_KEY_VALUE_TOP_LEFT) {
        uint8_t         display_mode;

        display_mode        = 0;

        for (uint32_t i = 0; i < child_cnt; i++) {
            if (p_screen_param_obj->p_state[i]) {
                display_mode        = display_mode | (1 << i);
                }
            }

        __inf("set display_mode:%d",display_mode);

        if (display_mode > 0) {
            tele_phone_data_set_display_mode(display_mode);

            alv_am_win_exit_action(NULL,"pb",NULL);
            alv_am_win_exit_action(NULL,"list_option",NULL);
            alv_am_win_exit_action(NULL,"setup",NULL);
            alv_am_win_jump_action(NULL,"pb",NULL);
            }
        }
    else if (key == APP_KEY_VALUE_TOP_RIGHT) {
        alv_am_win_exit_current(NULL);
        }
    else if (key == LV_KEY_ENTER) {
        lv_obj_t *      obj_img;

        get_focus_obj       = lv_obj_get_child(list,child_id);
        obj_img             = lv_obj_get_child(get_focus_obj,0);

        if (p_screen_param_obj->p_state[child_id]) {
            p_screen_param_obj->p_state[child_id] = 0;
            lv_img_set_src(obj_img,ui_imgset_select[0]);
            }
        else {
            p_screen_param_obj->p_state[child_id] = 1;
            lv_img_set_src(obj_img,ui_imgset_select[1]);
            }
        }
    else if (key == LV_KEY_END) {
        alv_am_app_exit_action_all();
        }

}


