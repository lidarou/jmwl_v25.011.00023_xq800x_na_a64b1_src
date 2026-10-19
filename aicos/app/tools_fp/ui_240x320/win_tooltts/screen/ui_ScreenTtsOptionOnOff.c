
#include "../ui.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "stdio.h"
#include "alv_app_mgr.h"
#include "app_log.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI




enum OBJ_LABEL_H {
    OBJ_LABEL_OK,
    OBJ_LABEL_BACK,
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

};


enum {
    TTS_OPTIONS_ON_OFF_OPEN,
    TTS_OPTIONS_ON_OFF_CLOSE,
    TTS_OPTIONS_ON_OFF_MAX,
};


static uint8_t * option_text[TTS_OPTIONS_ON_OFF_MAX] =
{
    "打开",
    "关闭",
};


typedef struct ui_screen_tts_option_on_off_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[TTS_OPTIONS_ON_OFF_MAX];

    ui_screen_tts_option_on_off_enter_param_t enter_param;
} ui_screen_tts_option_on_off_obj_t;


lv_obj_t * ui_ScreenTtsOptionOnOff_screen_init(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenTtsOptionAllOnOff = NULL;
    lv_obj_t *      label;

    ui_screen_tts_option_on_off_obj_t * p_screen_param;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_tts_option_on_off_obj_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_tts_option_on_off_obj_t));

    if (enter_param)
        memcpy(&p_screen_param->enter_param,enter_param,sizeof(ui_screen_tts_option_on_off_enter_param_t));

    ui_ScreenTtsOptionAllOnOff = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenTtsOptionAllOnOff,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenTtsOptionAllOnOff,get_lv_style_font_common(),0);

    label               = lv_label_create(ui_ScreenTtsOptionAllOnOff);
    lv_label_set_long_mode(label,LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_anim_speed(label,10,0);
    lv_obj_set_style_text_align(label,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_x(label,0);
    lv_obj_set_y(label,0);
    lv_obj_set_width(label,240);
    lv_obj_set_height(label,LV_SIZE_CONTENT);
    lv_label_set_text(label,p_screen_param->enter_param.title);

    obj_batch_label_create(ui_ScreenTtsOptionAllOnOff,&p_screen_param->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);


    p_screen_param->obj_list = lv_obj_create(ui_ScreenTtsOptionAllOnOff);
    lv_obj_set_width(p_screen_param->obj_list,240);
    lv_obj_set_height(p_screen_param->obj_list,228);
    lv_obj_set_x(p_screen_param->obj_list,0);
    lv_obj_set_y(p_screen_param->obj_list,45);
    lv_obj_set_align(p_screen_param->obj_list,LV_ALIGN_TOP_LEFT);
    lv_obj_add_flag(p_screen_param->obj_list,LV_OBJ_FLAG_SCROLL_WITH_ARROW); /// Flags
    lv_obj_set_scrollbar_mode(p_screen_param->obj_list,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(p_screen_param->obj_list,LV_DIR_VER);
    lv_obj_set_style_radius(p_screen_param->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    //lv_obj_set_style_bg_color(ui_PnlSettings, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
    //lv_obj_set_style_bg_opa(ui_PnlSettings, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(p_screen_param->obj_list,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(p_screen_param->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_screen_param->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_screen_param->obj_list,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    //obj_batch_label_create(p_screen_param->obj_list,&p_screen_param->obj_entry[0],NULL,obj_batch_entry_param_array,OBJ_ENTRY_MAX);
    for (int i = 0; i < TTS_OPTIONS_ON_OFF_MAX; i++) {

        p_screen_param->obj_entry[i] = lv_label_create(p_screen_param->obj_list);
        lv_label_set_long_mode(p_screen_param->obj_entry[i],LV_LABEL_LONG_CLIP);
        lv_obj_set_style_anim_speed(p_screen_param->obj_entry[i],10,0);
        lv_obj_set_x(p_screen_param->obj_entry[i],-2);
        lv_obj_set_y(p_screen_param->obj_entry[i],45 * i);
        lv_obj_set_width(p_screen_param->obj_entry[i],240); /// 1
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

    if (app_fp_play_tts_get_on_off(p_screen_param->enter_param.type)) {
        lv_obj_add_state(p_screen_param->obj_entry[0],LV_STATE_FOCUSED);
        lv_label_set_long_mode(p_screen_param->obj_entry[0],LV_LABEL_LONG_SCROLL);
        }
    else {
        lv_obj_add_state(p_screen_param->obj_entry[1],LV_STATE_FOCUSED);
        lv_label_set_long_mode(p_screen_param->obj_entry[1],LV_LABEL_LONG_SCROLL);

        }

    lv_obj_set_user_data(ui_ScreenTtsOptionAllOnOff,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenTtsOptionAllOnOff,fp_tts_option_on_off_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenTtsOptionAllOnOff);

    lv_obj_set_user_data(win,ui_ScreenTtsOptionAllOnOff);

    return ui_ScreenTtsOptionAllOnOff;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_tts_option_on_off_obj_t * p_screen_param;

    p_screen_param      = (ui_screen_tts_option_on_off_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenTtsOptionOnOff_screen_uninit(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenTtsOptionAllOnOff = NULL;

    ui_ScreenTtsOptionAllOnOff = lv_obj_get_user_data(win);

    if (ui_ScreenTtsOptionAllOnOff) {
        lv_obj_del_delayed_callback(ui_ScreenTtsOptionAllOnOff,200,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}



lv_obj_t * get_tts_option_on_off_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_tts_option_on_off_obj_t * p_screen_param = lv_obj_get_user_data(ui_Screen);

    lv_obj_t *      list = p_screen_param->obj_list;

    return list;
}


void ui_ScreenTtsOptionOnOff_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenTtsOptionAllOnOff = NULL;

    ui_ScreenTtsOptionAllOnOff = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_ScreenTtsOptionAllOnOff);

    {
        lv_obj_t *      list = get_tts_option_on_off_list_obj(ui_ScreenTtsOptionAllOnOff);

        ;
        uint32_t        child_cnt = lv_obj_get_child_cnt(list);
        uint32_t        child_id = get_list_child_focused_id(list,child_cnt);
        lv_obj_t *      get_focus_obj = NULL;

        get_focus_obj       = lv_obj_get_child(list,child_id);


        if (get_focus_obj) {
            app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));
            }
        }
}


static void remind_timer_callback_cb(screen_remind_param_t * p_param)
{

    alv_am_win_exit_current(NULL);

}


void tts_option_on_off_remind_screen(lv_obj_t * ui)
{
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;

    screen_remind_param.str = "完成";

    ui_screen_remind_param_init(&screen_remind_param);

    screen_remind_param.tiemr_delay = 1000;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_cb;
    screen_remind_param.user_data = ui;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);

}


void tts_option_on_off_key_event(uint32_t key,lv_obj_t * ui_ScreenTtsOptionAllOnOff)
{
    ui_screen_tts_option_on_off_obj_t * p_screen_param = lv_obj_get_user_data(ui_ScreenTtsOptionAllOnOff);
    lv_obj_t *      list = get_tts_option_on_off_list_obj(ui_ScreenTtsOptionAllOnOff);

    ;
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

                lv_label_set_long_mode(pre_focus_obj,LV_LABEL_LONG_CLIP);
                lv_label_set_long_mode(get_focus_obj,LV_LABEL_LONG_SCROLL);
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

                lv_label_set_long_mode(pre_focus_obj,LV_LABEL_LONG_CLIP);
                lv_label_set_long_mode(get_focus_obj,LV_LABEL_LONG_SCROLL);
                app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));
                }
            break;

        case APP_KEY_VALUE_TOP_LEFT:
        case LV_KEY_ENTER:
            {
                uint32_t        index;

                get_focus_obj       = lv_obj_get_child(list,child_id);
                index               = (uint32_t) lv_obj_get_user_data(get_focus_obj);

                switch (index)
                {
                    case TTS_OPTIONS_ON_OFF_OPEN:
                        tts_option_on_off_remind_screen(ui_ScreenTtsOptionAllOnOff);
                        app_fp_play_tts_enable_on_off(p_screen_param->enter_param.type);
                        break;

                    case TTS_OPTIONS_ON_OFF_CLOSE:
                        tts_option_on_off_remind_screen(ui_ScreenTtsOptionAllOnOff);
                        app_fp_play_tts_disable_on_off(p_screen_param->enter_param.type);
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
            {
                alv_am_app_exit_action_all();
                }
            break;
        }


}






