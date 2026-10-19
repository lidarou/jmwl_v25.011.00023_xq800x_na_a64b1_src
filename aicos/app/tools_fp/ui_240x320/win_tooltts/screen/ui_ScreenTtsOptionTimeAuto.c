
#include "../ui.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "stdio.h"
#include "tts_app_player.h"
#include "alv_app_mgr.h"
#include "app_log.h"

#define __inf                   APP_LOGI




enum OBJ_LABEL_H {
    OBJ_LABEL_TITLE,
    OBJ_LABEL_OK,
    OBJ_LABEL_BACK,
    OBJ_LABEL_MAX,
};


static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_MID,"整点报时"
        },
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"确定"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },

};


enum {
    TTS_OPTIONS_TIME_AUTO_CLOSE,
    TTS_OPTIONS_TIME_AUTO_DAY,
    TTS_OPTIONS_TIME_AUTO_NIGHT,
    TTS_OPTIONS_TIME_AUTO_ALL_DAY,
    TTS_OPTIONS_TIME_AUTO_CUSTOM,
    TTS_OPTIONS_TIME_AUTO_MAX
};


static uint8_t * option_text[TTS_OPTIONS_TIME_AUTO_MAX] =
{
    "关闭",
    "白天模式(8:00-18:00)",
    "夜晚模式(18:00-23:00)",
    "全天",
    "自定义时间",
};


typedef struct ui_screen_tts_option_time_auto_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[TTS_OPTIONS_TIME_AUTO_MAX];


} ui_screen_tts_option_time_auto_obj_t;


lv_obj_t * ui_ScreenTtsOptionTimeAuto_screen_init(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenTtsOptionTimeAuto = NULL;
    uint8_t         index;

    ui_screen_tts_option_time_auto_obj_t * p_screen_param;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_tts_option_time_auto_obj_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_tts_option_time_auto_obj_t));

    ui_ScreenTtsOptionTimeAuto = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenTtsOptionTimeAuto,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenTtsOptionTimeAuto,get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenTtsOptionTimeAuto,&p_screen_param->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);

    p_screen_param->obj_list = lv_obj_create(ui_ScreenTtsOptionTimeAuto);
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
    for (int i = 0; i < TTS_OPTIONS_TIME_AUTO_MAX; i++) {

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

    index               = app_fp_play_tts_get_time_auto_mode();
    lv_obj_add_state(p_screen_param->obj_entry[index],LV_STATE_FOCUSED);
    lv_label_set_long_mode(p_screen_param->obj_entry[index],LV_LABEL_LONG_SCROLL);

    lv_obj_set_user_data(ui_ScreenTtsOptionTimeAuto,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenTtsOptionTimeAuto,fp_tts_option_time_auto_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenTtsOptionTimeAuto);

    lv_obj_set_user_data(win,ui_ScreenTtsOptionTimeAuto);

    return ui_ScreenTtsOptionTimeAuto;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_tts_option_time_auto_obj_t * p_screen_param;

    p_screen_param      = (ui_screen_tts_option_time_auto_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenTtsOptionTimeAuto_screen_uninit(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenTtsOptionTimeAuto = NULL;

    ui_ScreenTtsOptionTimeAuto = lv_obj_get_user_data(win);

    if (ui_ScreenTtsOptionTimeAuto) {
        lv_obj_del_delayed_callback(ui_ScreenTtsOptionTimeAuto,200,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}



lv_obj_t * get_tts_option_time_auto_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_tts_option_time_auto_obj_t * p_screen_param = lv_obj_get_user_data(ui_Screen);

    lv_obj_t *      list = p_screen_param->obj_list;

    return list;
}


void ui_ScreenTtsOptionTimeAuto_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenTtsOptionTimeAuto = NULL;

    ui_ScreenTtsOptionTimeAuto = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_ScreenTtsOptionTimeAuto);

    {
        lv_obj_t *      list = get_tts_option_time_auto_list_obj(ui_ScreenTtsOptionTimeAuto);

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


void ui_ScreenTtsOptionTimeAuto_screen_defocus(lv_obj_t * win,void * enter_param)
{
}


void tts_option_time_auto_key_event(uint32_t key,lv_obj_t * ui_ScreenTtsOptionTimeAuto)
{
    lv_obj_t *      list = get_tts_option_time_auto_list_obj(ui_ScreenTtsOptionTimeAuto);

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
                    case TTS_OPTIONS_TIME_AUTO_CLOSE:
                        tts_option_on_off_remind_screen(ui_ScreenTtsOptionTimeAuto);
                        app_fp_play_tts_set_time_mode(0);
                        break;

                    case TTS_OPTIONS_TIME_AUTO_DAY:
                        tts_option_on_off_remind_screen(ui_ScreenTtsOptionTimeAuto);
                        app_fp_play_tts_set_time_mode(1);
                        break;

                    case TTS_OPTIONS_TIME_AUTO_NIGHT:
                        tts_option_on_off_remind_screen(ui_ScreenTtsOptionTimeAuto);
                        app_fp_play_tts_set_time_mode(2);
                        break;

                    case TTS_OPTIONS_TIME_AUTO_ALL_DAY:
                        tts_option_on_off_remind_screen(ui_ScreenTtsOptionTimeAuto);
                        app_fp_play_tts_set_time_mode(3);
                        break;

                    case TTS_OPTIONS_TIME_AUTO_CUSTOM:
                        alv_am_win_enter(NULL,"tools_tts_time_set",NULL);
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






