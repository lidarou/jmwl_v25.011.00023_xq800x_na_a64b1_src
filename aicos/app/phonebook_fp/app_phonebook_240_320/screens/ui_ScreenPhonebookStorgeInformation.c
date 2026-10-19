
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"
#include <stdio.h>
#include "app_log.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI




enum {
    OBJ_LABEL_STORGE,
    OBJ_LABEL_BACK,
    OBJ_LABEL_MAX,
};


static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_MID,"存储状态"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },

};


typedef struct ui_screen_phonebook_storge_infromation_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
} ui_screen_phonebook_storge_infromation_obj_t;


lv_obj_t * ui_ScreenPhonebookstorge_infromation_screen_init(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookstorge_infromation = NULL;

    lv_obj_t *      obj_label_tmp;
    uint8_t         str_buf[30];
    phonebook_head_t phonebook_head_flash;
    phonebook_head_t phonebook_head_sim;
    int             i;

    ui_screen_phonebook_storge_infromation_obj_t * p_screen_param_obj;

    p_screen_param_obj  = lv_mem_alloc(sizeof(ui_screen_phonebook_storge_infromation_obj_t));

    if (p_screen_param_obj == NULL)
        return 0;

    ui_ScreenPhonebookstorge_infromation = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebookstorge_infromation,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebookstorge_infromation,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebookstorge_infromation,&p_screen_param_obj->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);


    get_phonebook_head_local(&phonebook_head_flash);
    get_phonebook_head_sim(&phonebook_head_sim);

    p_screen_param_obj->obj_list = lv_obj_create(ui_ScreenPhonebookstorge_infromation);
    lv_obj_set_width(p_screen_param_obj->obj_list,240 + 5);
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

    //lv_obj_clear_flag(p_screen_param_obj->obj_list, LV_OBJ_FLAG_SCROLLABLE);
    i                   = 0;
    obj_label_tmp       = lv_label_create(p_screen_param_obj->obj_list);
    lv_obj_set_x(obj_label_tmp,0);
    lv_obj_set_y(obj_label_tmp,45 * i++);
    lv_obj_set_width(obj_label_tmp,LV_PCT(100));    /// 1
    lv_obj_set_height(obj_label_tmp,45);            /// 1
    lv_label_set_text(obj_label_tmp,"手机:");


    memset(str_buf,0,30);
    sprintf(str_buf,"%d可用",phonebook_head_flash.count);
    obj_label_tmp       = lv_label_create(p_screen_param_obj->obj_list);
    lv_obj_set_x(obj_label_tmp,0);
    lv_obj_set_y(obj_label_tmp,45 * i++);
    lv_obj_set_width(obj_label_tmp,LV_PCT(100));    /// 1
    lv_obj_set_height(obj_label_tmp,45);            /// 1
    lv_label_set_text(obj_label_tmp,str_buf);

    memset(str_buf,0,30);
    sprintf(str_buf,"%d已用",phonebook_head_flash.used);
    obj_label_tmp       = lv_label_create(p_screen_param_obj->obj_list);
    lv_obj_set_x(obj_label_tmp,0);
    lv_obj_set_y(obj_label_tmp,45 * i++);
    lv_obj_set_width(obj_label_tmp,LV_PCT(100));    /// 1
    lv_obj_set_height(obj_label_tmp,45);            /// 1
    lv_label_set_text(obj_label_tmp,str_buf);

    if (get_tele_phonebook_sim_status()) {
        obj_label_tmp       = lv_label_create(p_screen_param_obj->obj_list);
        lv_obj_set_x(obj_label_tmp,0);
        lv_obj_set_y(obj_label_tmp,45 * i++);
        lv_obj_set_width(obj_label_tmp,LV_PCT(100)); /// 1
        lv_obj_set_height(obj_label_tmp,45);        /// 1
        lv_label_set_text(obj_label_tmp,"--------");


        memset(str_buf,0,30);
        sprintf(str_buf,"SIM%d",(int) get_tele_phonebook_sim_status());
        obj_label_tmp       = lv_label_create(p_screen_param_obj->obj_list);
        lv_obj_set_x(obj_label_tmp,0);
        lv_obj_set_y(obj_label_tmp,45 * i++);
        lv_obj_set_width(obj_label_tmp,LV_PCT(100)); /// 1
        lv_obj_set_height(obj_label_tmp,45);        /// 1
        lv_label_set_text(obj_label_tmp,str_buf);


        memset(str_buf,0,30);
        sprintf(str_buf,"%d可用",phonebook_head_sim.count);
        obj_label_tmp       = lv_label_create(p_screen_param_obj->obj_list);
        lv_obj_set_x(obj_label_tmp,0);
        lv_obj_set_y(obj_label_tmp,45 * i++);
        lv_obj_set_width(obj_label_tmp,LV_PCT(100)); /// 1
        lv_obj_set_height(obj_label_tmp,45);        /// 1
        lv_label_set_text(obj_label_tmp,str_buf);

        memset(str_buf,0,30);
        sprintf(str_buf,"%d已用",phonebook_head_sim.used);
        obj_label_tmp       = lv_label_create(p_screen_param_obj->obj_list);
        lv_obj_set_x(obj_label_tmp,0);
        lv_obj_set_y(obj_label_tmp,45 * i++);
        lv_obj_set_width(obj_label_tmp,LV_PCT(100)); /// 1
        lv_obj_set_height(obj_label_tmp,45);        /// 1
        lv_label_set_text(obj_label_tmp,str_buf);
        }

    lv_obj_set_user_data(ui_ScreenPhonebookstorge_infromation,p_screen_param_obj);

    lv_obj_add_event_cb(ui_ScreenPhonebookstorge_infromation,phonebook_fp_storge_infromation_ui_event_actions,
        LV_EVENT_ALL,ui_ScreenPhonebookstorge_infromation);

    lv_obj_set_user_data(win,ui_ScreenPhonebookstorge_infromation);

    return ui_ScreenPhonebookstorge_infromation;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_phonebook_storge_infromation_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_storge_infromation_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebookstorge_infromation_screen_uninit(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookstorge_infromation = NULL;

    ui_ScreenPhonebookstorge_infromation = lv_obj_get_user_data(win);

    if (ui_ScreenPhonebookstorge_infromation) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebookstorge_infromation,500,lv_obj_del_screen_ready_cb);
        }

    ui_ScreenPhonebookstorge_infromation = NULL;

    lv_obj_set_user_data(win,NULL);
}




lv_obj_t * get_phonebook_storge_infromation__obj(lv_obj_t * ui_Screen)
{
    ui_screen_phonebook_storge_infromation_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_storge_infromation_obj_t *) lv_obj_get_user_data(ui_Screen);

    lv_obj_t *      list = p_screen_param_obj->obj_list;

    return list;
}


void ui_ScreenPhonebookstorge_infromation_screen_focus(lv_obj_t * win,
    ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookstorge_infromation = NULL;

    ui_ScreenPhonebookstorge_infromation = lv_obj_get_user_data(win);

    ui_screen_phonebook_storge_infromation_obj_t * p_screen_param_obj =
         lv_obj_get_user_data(ui_ScreenPhonebookstorge_infromation);

    lv_disp_load_scr(ui_ScreenPhonebookstorge_infromation);

    app_fp_play_tts_menu(lv_label_get_text(p_screen_param_obj->obj_label[OBJ_LABEL_STORGE]));

}


