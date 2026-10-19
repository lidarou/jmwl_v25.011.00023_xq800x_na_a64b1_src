
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"

#include "app_log.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI




enum {
    OBJ_LABEL_SETUP,
    OBJ_LABEL_OPTION,
    OBJ_LABEL_BACK,
    OBJ_LABEL_MAX,
};


static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_MID,"设置"
        },
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"选择"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },

};


typedef struct ui_screen_phonebook_setup_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[2];
} ui_screen_phonebook_setup_obj_t;


lv_obj_t * ui_ScreenPhonebooksetup_screen_init(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebooksetup = NULL;

    ui_screen_phonebook_setup_obj_t * p_screen_param_obj;

    p_screen_param_obj  = lv_mem_alloc(sizeof(ui_screen_phonebook_setup_obj_t));

    if (p_screen_param_obj == NULL)
        return 0;

    ui_ScreenPhonebooksetup = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebooksetup,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebooksetup,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebooksetup,&p_screen_param_obj->obj_label[0],NULL,
        obj_batch_label_param_array,
        OBJ_LABEL_MAX);

    p_screen_param_obj->obj_list = lv_obj_create(ui_ScreenPhonebooksetup);
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

    int             i   = 0;


    p_screen_param_obj->obj_entry[i] = lv_label_create(p_screen_param_obj->obj_list);

    lv_obj_set_x(p_screen_param_obj->obj_entry[i],-2);
    lv_obj_set_y(p_screen_param_obj->obj_entry[i],45 * i);
    lv_obj_set_width(p_screen_param_obj->obj_entry[i],240); /// 1
    lv_obj_set_height(p_screen_param_obj->obj_entry[i],45); /// 1


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

    lv_label_set_text(p_screen_param_obj->obj_entry[0],"存储状况");

    i                   = 1;

    if ((1 == get_tele_phonebook_sim_status()) || (2 == get_tele_phonebook_sim_status())) {
        p_screen_param_obj->obj_entry[i] = lv_label_create(p_screen_param_obj->obj_list);

        lv_obj_set_x(p_screen_param_obj->obj_entry[i],-2);
        lv_obj_set_y(p_screen_param_obj->obj_entry[i],45 * i);
        lv_obj_set_width(p_screen_param_obj->obj_entry[i],240); /// 1
        lv_obj_set_height(p_screen_param_obj->obj_entry[i],45); /// 1


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


        lv_label_set_text(p_screen_param_obj->obj_entry[1],"显示联系人");

        }

    lv_obj_add_state(p_screen_param_obj->obj_entry[0],LV_STATE_FOCUSED);


    lv_obj_set_user_data(ui_ScreenPhonebooksetup,p_screen_param_obj);

    lv_obj_add_event_cb(ui_ScreenPhonebooksetup,phonebook_fp_setup_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebooksetup);

    lv_obj_set_user_data(win,ui_ScreenPhonebooksetup);

    return ui_ScreenPhonebooksetup;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_phonebook_setup_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_setup_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebooksetup_screen_uninit(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebooksetup = NULL;

    ui_ScreenPhonebooksetup = lv_obj_get_user_data(win);

    if (ui_ScreenPhonebooksetup) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebooksetup,500,lv_obj_del_screen_ready_cb);
        }

    ui_ScreenPhonebooksetup = NULL;

    lv_obj_set_user_data(win,NULL);
}


lv_obj_t * get_phonebook_setup_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_phonebook_setup_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_setup_obj_t *) lv_obj_get_user_data(ui_Screen);

    lv_obj_t *      list = p_screen_param_obj->obj_list;

    return list;
}


void ui_ScreenPhonebooksetup_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebooksetup = NULL;

    ui_ScreenPhonebooksetup = lv_obj_get_user_data(win);
    lv_obj_t *      list = get_phonebook_setup_list_obj(ui_ScreenPhonebooksetup);

    ;
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;


    get_focus_obj       = lv_obj_get_child(list,child_id);

    lv_disp_load_scr(ui_ScreenPhonebooksetup);

    app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));

}


