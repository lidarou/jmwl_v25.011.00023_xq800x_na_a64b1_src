
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"

#include "app_log.h"
#include "tts_app_player.h"
#include "alv_app_mgr.h"

#define __inf                   APP_LOGI




enum OBJ_LABEL_H {
    //	OBJ_LABEL_PHONEBOOK,
    OBJ_LABEL_OPTION,
    OBJ_LABEL_BACK,
    OBJ_LABEL_MAX,
};

static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    //{0,		0,		LV_SIZE_CONTENT,	240,				0,		"选择联系人存储"},
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"选择"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },

};


enum OBJ_ENTRY_H {
    OBJ_ENTRY_PHONE,
    OBJ_ENTRY_SIM,
    OBJ_ENTRY_MAX,
};

static obj_batch_label_param_array_t obj_batch_entry_param_array[OBJ_ENTRY_MAX] =
{
    {
        -2,45 * 0,240,45,0,"手机"
        },
    {
        -2,45 * 1,240,45,0,NULL
        },

};


typedef struct ui_screen_phonebook_storge_format_obj_h {
    lv_obj_t *      obj_label_title;
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[OBJ_ENTRY_MAX];
    uint32_t        sim_status;
    ui_screen_phonebook_enter_param_t enter_param;

} ui_screen_phonebook_storge_format_obj_t;

lv_obj_t * ui_ScreenPhonebookstorge_format_screen_init(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookstorge_format = NULL;

    ui_screen_phonebook_storge_format_obj_t * p_screen_param_obj;

    p_screen_param_obj  = lv_mem_alloc(sizeof(ui_screen_phonebook_storge_format_obj_t));

    if (p_screen_param_obj == NULL)
        return 0;

    memset(p_screen_param_obj,0,sizeof(ui_screen_phonebook_storge_format_obj_t));

    ui_ScreenPhonebookstorge_format = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebookstorge_format,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebookstorge_format,(lv_style_t *) get_lv_style_font_common(),0);


    p_screen_param_obj->obj_label_title = lv_label_create(ui_ScreenPhonebookstorge_format);
    lv_label_set_long_mode(p_screen_param_obj->obj_label_title,LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_anim_speed(p_screen_param_obj->obj_label_title,10,0);
    lv_obj_set_x(p_screen_param_obj->obj_label_title,0);
    lv_obj_set_y(p_screen_param_obj->obj_label_title,0);
    lv_obj_set_width(p_screen_param_obj->obj_label_title,240); /// 1
    lv_obj_set_height(p_screen_param_obj->obj_label_title,40); /// 1

    lv_label_set_text(p_screen_param_obj->obj_label_title,"选择联系人存储");



    obj_batch_label_create(ui_ScreenPhonebookstorge_format,&p_screen_param_obj->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);




    if (enter_param != NULL) {
        memcpy(&p_screen_param_obj->enter_param,enter_param,sizeof(ui_screen_phonebook_enter_param_t));
        }


    p_screen_param_obj->obj_list = lv_obj_create(ui_ScreenPhonebookstorge_format);
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
    lv_obj_clear_flag(p_screen_param_obj->obj_list,LV_OBJ_FLAG_SCROLLABLE); /// Flags

    obj_batch_label_create(p_screen_param_obj->obj_list,&p_screen_param_obj->obj_entry[0],NULL,
        obj_batch_entry_param_array,OBJ_ENTRY_MAX);

    for (int i = 0; i < OBJ_ENTRY_MAX; i++) {
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
        }

    lv_obj_add_state(p_screen_param_obj->obj_entry[0],LV_STATE_FOCUSED);

    p_screen_param_obj->sim_status = get_tele_phonebook_sim_status();

    switch (p_screen_param_obj->sim_status)
    {
        case 1:
            lv_label_set_text(p_screen_param_obj->obj_entry[OBJ_ENTRY_SIM],"SIM1");
            break;

        case 2:
            lv_label_set_text(p_screen_param_obj->obj_entry[OBJ_ENTRY_SIM],"SIM2");
            break;

        default:
            lv_obj_del(p_screen_param_obj->obj_entry[OBJ_ENTRY_SIM]);
            p_screen_param_obj->obj_entry[OBJ_ENTRY_SIM] = 0;
            break;
        }

    lv_obj_add_state(p_screen_param_obj->obj_entry[OBJ_ENTRY_PHONE],LV_STATE_FOCUSED);



    lv_obj_set_user_data(ui_ScreenPhonebookstorge_format,p_screen_param_obj);

    lv_obj_add_event_cb(ui_ScreenPhonebookstorge_format,phonebook_fp_storge_format_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebookstorge_format);

    lv_obj_set_user_data(win,ui_ScreenPhonebookstorge_format);

    return ui_ScreenPhonebookstorge_format;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_phonebook_storge_format_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_storge_format_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebookstorge_format_screen_uninit(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookstorge_format = NULL;

    ui_ScreenPhonebookstorge_format = lv_obj_get_user_data(win);

    if (ui_ScreenPhonebookstorge_format) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebookstorge_format,200,lv_obj_del_screen_ready_cb);
        }

    ui_ScreenPhonebookstorge_format = NULL;
    lv_obj_set_user_data(win,NULL);
}




lv_obj_t * get_phonebook_storge_format_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_phonebook_storge_format_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_storge_format_obj_t *) lv_obj_get_user_data (ui_Screen);

    lv_obj_t *      list = p_screen_param_obj->obj_list;

    return list;
}


void ui_ScreenPhonebookstorge_format_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{

    lv_obj_t *      ui_ScreenPhonebookstorge_format = NULL;

    ui_ScreenPhonebookstorge_format = lv_obj_get_user_data(win);

    lv_obj_t *      list = get_phonebook_storge_format_list_obj(ui_ScreenPhonebookstorge_format);
    uint32_t        child_cnt = lv_obj_get_child_cnt(list);
    uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
    lv_obj_t *      get_focus_obj = NULL;

    get_focus_obj       = lv_obj_get_child(list,child_id);

    lv_disp_load_scr(ui_ScreenPhonebookstorge_format);

    app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));


}


ui_screen_phonebook_enter_param_t * get_ui_ScreenPhonebookstorge_format_enter_param(ui_screen_phonebook_storge_format_obj_t * storge_format_obj)
{
    if (storge_format_obj != NULL)
        return & storge_format_obj->enter_param;

    return 0;
}




void ui_ScreenPhonebookstorge_format_screen_key_enter(lv_obj_t * ui_Screen,uint8_t focus_id)
{
    phonebook_head_t phonebook_head_flash;
    phonebook_head_t phonebook_head_sim;

    ui_screen_phonebook_enter_param_t enter_param,*p_enter_param;

    p_enter_param       = get_ui_ScreenPhonebookstorge_format_enter_param(ui_Screen->user_data);


    if (focus_id) {
        get_phonebook_head_sim(&phonebook_head_sim);

        if (phonebook_head_sim.used >= phonebook_head_sim.count) {
            uint8_t *       str;

            if (1 == get_tele_phonebook_sim_status())
                str = "SIM1存储已满";
            else if (2 == get_tele_phonebook_sim_status())
                str = "SIM2存储已满";
            else 
                return;

            create_comp_msgbox_remind_play_tts_menu(str);
            //app_fp_play_tts_menu(str);
            return;
            }
        }
    else {


        get_phonebook_head_local(&phonebook_head_flash);

        if (phonebook_head_flash.used >= phonebook_head_flash.count) {
            const uint8_t * str = "手机存储已满";

            create_comp_msgbox_remind_play_tts_menu((uint8_t *)str);
            //app_fp_play_tts_menu((uint8_t *)str);
            return;
            }
        }


    memcpy(&enter_param,p_enter_param,sizeof(enter_param));

    if (focus_id == 0)
        enter_param.phonebook_param.strage_format = 0;
    else 
        enter_param.phonebook_param.strage_format = get_tele_phonebook_sim_status();

    enter_param.p_cur_node = NULL;

    alv_am_win_enter(NULL,"edit",(void *) &enter_param);

}

