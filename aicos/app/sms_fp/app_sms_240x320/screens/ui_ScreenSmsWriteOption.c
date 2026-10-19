
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"
#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "stdio.h"
#include "tele_sms_data.h"
#include "alv_app_mgr.h"
#include "app_log.h"
#include "tts_app_player.h"
#include "settings_fp_flow.h"

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
        0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_MID,"信息"
        },
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"选择"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },

};


enum {
    SMS_WITER_SEND,
    SMS_WITER_SEND_OPTION,
    SMS_WITER_INSERT_OPTION,
    SMS_WITER_STORGE_INF,
    SMS_WITER_CLEAR,
    SMS_WITER_EXIT,
    SMS_WITER_MAX,
};

static uint8_t * option_text[SMS_WITER_MAX] =
{
    "发送",

    //"发送选项",
    "发送至多人",
    "插入选项",
    "存储信息",
    "清除文字",
    "退出编辑器",
};


typedef struct ui_screen_sms_write_option_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_list;
    lv_obj_t *      obj_entry[SMS_WITER_MAX];

    sms_write_ui_enter_param_t * ui_enter_param;

} ui_screen_sms_write_option_obj_t;

lv_obj_t * ui_ScreenSmsWriteOption_screen_init(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsWriteOption = NULL;

    ui_screen_sms_write_option_obj_t * p_screen_param;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_sms_write_option_obj_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_sms_write_option_obj_t));

    ui_ScreenSmsWriteOption = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenSmsWriteOption,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenSmsWriteOption,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenSmsWriteOption,&p_screen_param->obj_label[0],NULL,obj_batch_label_param_array,
        OBJ_LABEL_MAX);

    p_screen_param->obj_list = lv_obj_create(ui_ScreenSmsWriteOption);
    lv_obj_set_width(p_screen_param->obj_list,240);
    lv_obj_set_height(p_screen_param->obj_list,228);
    lv_obj_set_x(p_screen_param->obj_list,0);
    lv_obj_set_y(p_screen_param->obj_list,50);
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
    for (int i = 0; i < SMS_WITER_MAX; i++) {

        p_screen_param->obj_entry[i] = lv_label_create(p_screen_param->obj_list);

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

    lv_obj_add_state(p_screen_param->obj_entry[0],LV_STATE_FOCUSED);

    p_screen_param->ui_enter_param = enter_param;

    lv_obj_set_user_data(ui_ScreenSmsWriteOption,p_screen_param);

    lv_obj_add_event_cb(ui_ScreenSmsWriteOption,fp_sms_write_option_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenSmsWriteOption);

    lv_obj_set_user_data(win,ui_ScreenSmsWriteOption);

    return ui_ScreenSmsWriteOption;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_sms_write_option_obj_t * p_screen_param;

    p_screen_param      = (ui_screen_sms_write_option_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenSmsWriteOption_screen_uninit(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsWriteOption = NULL;

    ui_ScreenSmsWriteOption = lv_obj_get_user_data(win);

    if (ui_ScreenSmsWriteOption) {
        lv_obj_del_delayed_callback(ui_ScreenSmsWriteOption,500,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}



lv_obj_t * get_sms_write_option_list_obj(lv_obj_t * ui_Screen)
{
    ui_screen_sms_write_option_obj_t * p_screen_param = lv_obj_get_user_data(ui_Screen);

    lv_obj_t *      list = p_screen_param->obj_list;

    return list;
}


void ui_ScreenSmsWriteOption_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_ScreenSmsWriteOption = NULL;

    ui_ScreenSmsWriteOption = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_ScreenSmsWriteOption);

    {
        lv_obj_t *      list = (lv_obj_t *) get_sms_write_option_list_obj(ui_ScreenSmsWriteOption);

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


void sms_write_option_storge_remind_screen(lv_obj_t * ui_Screen);

static int32_t sms_send_check_sim_status()
{
    int32_t         ret;

    ret                 = 1;

    if (0 == get_tele_phonebook_sim_status()) {
        create_comp_msgbox_remind_play_tts_menu("请插入SIM卡");
        ret                 = 0;
        }
    else if (settings_fp_flow_get_flightmode_status()) {
        create_comp_msgbox_remind_play_tts_menu("请关闭飞行模式");
        ret                 = 0;
        }

    return ret;
}

void sms_write_option_key_event(uint32_t key,lv_obj_t * ui_ScreenSmsWriteOption)
{
    ui_screen_sms_write_option_obj_t * p_screen_param =
         (ui_screen_sms_write_option_obj_t *) lv_obj_get_user_data(ui_ScreenSmsWriteOption);
    lv_obj_t *      list = (lv_obj_t *) get_sms_write_option_list_obj(ui_ScreenSmsWriteOption);
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

        app_fp_play_tts_menu(lv_label_get_text(get_focus_obj));
        }
    else if ((key == APP_KEY_VALUE_TOP_LEFT) || (key == LV_KEY_ENTER)) {
        uint32_t        index;

        get_focus_obj       = lv_obj_get_child(list,child_id);
        index               = (uint32_t) lv_obj_get_user_data(get_focus_obj);

        switch (index)
        {
            case SMS_WITER_SEND:
                if (0 == sms_send_check_sim_status())
                    break;

                alv_am_win_enter(NULL,"write_num",(void *) p_screen_param->ui_enter_param);
                break;

            case SMS_WITER_SEND_OPTION:
                if (0 == sms_send_check_sim_status())
                    break;

                //alv_am_win_enter(NULL,"send_option",(void*)p_screen_param->ui_enter_param);                   
                alv_am_win_enter(NULL,"select_phone_num",(void *) p_screen_param->ui_enter_param);
                break;

            case SMS_WITER_INSERT_OPTION:
                alv_am_win_enter(NULL,"insert_option",(void *) p_screen_param->ui_enter_param);
                break;

            case SMS_WITER_STORGE_INF:
                sms_write_option_storge_remind_screen(ui_ScreenSmsWriteOption);
                break;

            case SMS_WITER_CLEAR:
                p_screen_param->ui_enter_param->clear_content_flag = 1;
                alv_am_win_exit_current((void *) NULL);
                break;

            case SMS_WITER_EXIT:
                alv_am_win_exit_action(NULL,"write",NULL);
                alv_am_win_exit_current((void *) NULL);
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


static void remind_timer_callback_storge_back_ui(screen_remind_param_t * p_param)
{

    alv_am_win_exit_action(NULL,"write_option",NULL);

}


static void sms_write_option_storge_draft(lv_obj_t * ui_Screen)
{
    ui_screen_sms_write_option_obj_t * p_screen_param;
    tele_sms_storge_draft_param_t * p_draft;
    uint8_t *       p_content;
    uint32_t        write_len;
    uint8_t         cur_card_id = TS_CARD_ID_MAX;

    p_screen_param      = (ui_screen_sms_write_option_obj_t *) lv_obj_get_user_data(ui_Screen);
    p_content           = p_screen_param->ui_enter_param->p_content;
    write_len           = sizeof(tele_sms_storge_send_param_t) +strlen(p_content);

    p_draft             = aic_zalloc(write_len);
    aic_srv_tele_sim_get_current_card(&cur_card_id);
    p_draft->card_id    = cur_card_id;

    strncpy(p_draft->number,p_content,TS_SMS_NUM_MAX_LEN);
    memcpy(&p_draft->text_start,p_content,strlen(p_content));

    tele_sms_data_draft_write_file(p_draft);

    aic_free(p_draft);
}


void sms_write_option_storge_remind_screen(lv_obj_t * ui_Screen)
{
    ui_screen_sms_write_option_obj_t * p_screen_param;
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;

    p_screen_param      = (ui_screen_sms_write_option_obj_t *) lv_obj_get_user_data(ui_Screen);


    ui_screen_remind_param_init(&screen_remind_param);
    screen_remind_param.str = "已存储";


    screen_remind_param.tiemr_delay = 1500;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_storge_back_ui;
    screen_remind_param.user_data = ui_Screen;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);


    sms_write_option_storge_draft(ui_Screen);

}


