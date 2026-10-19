
#include <stdio.h>
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"

#include "app_log.h"
#include "screen_remind.h"
#include "alv_app_mgr.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI




enum {
    OBJ_LABEL_TITLE_NAME,
    OBJ_LABEL_PINYIN_FORMAT,

    //OBJ_LABEL_NAME,
    OBJ_LABEL_TITLE_NUM,

    //OBJ_LABEL_NUM,
    OBJ_LABEL_BACK,
    OBJ_LABEL_CLEAN,
    OBJ_LABEL_EDIT,
    OBJ_LABEL_MAX,
};


static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        1,2,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"姓名"
        },
    {
        162,12,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,NULL
        },

    //{1, 	52,		LV_PCT(100),		LV_SIZE_CONTENT,	0,					NULL},
    {
        1,102,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"电话号码:"
        },

    //{1, 	152,	LV_PCT(100),		LV_SIZE_CONTENT,	0,					NULL},
    {
        1,275,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },
    {
        162,275,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"清除"
        },
    {
        100,290,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"存储"
        },

};


enum {
    OBJ_TEXTAREA_NAME,
    OBJ_TEXTAREA_NUM,
    OBJ_TEXTAREA_MAX,
};


typedef struct ui_screen_phonebook_edit_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    lv_obj_t *      obj_textarea[OBJ_TEXTAREA_MAX];
    ui_screen_phonebook_enter_param_t enter_param;

    pinyin_param_t * pinyin;

} ui_screen_phonebook_edit_obj_t;


extern int32_t chset_uft8_to_pinyin_str(uint8_t * dst,int32_t dst_size,uint8_t * src);

void set_phonebook_edit_name_fucous(lv_obj_t * ui_ScreenPhonebookedit);


lv_obj_t * ui_ScreenPhonebookedit_screen_init(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookedit = NULL;

    phonebook_param_t phonebook_param;

    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = lv_mem_alloc(sizeof(ui_screen_phonebook_edit_obj_t));

    if (p_screen_param_obj == NULL)
        return 0;

    memset(p_screen_param_obj,0,sizeof(ui_screen_phonebook_edit_obj_t));
    memset(&phonebook_param,0,sizeof(phonebook_param));


    ui_ScreenPhonebookedit = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebookedit,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebookedit,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebookedit,&p_screen_param_obj->obj_label[0],NULL,
        obj_batch_label_param_array,
        OBJ_LABEL_MAX);
    lv_obj_add_style(p_screen_param_obj->obj_label[OBJ_LABEL_EDIT],(lv_style_t *) get_lv_style_font_custom(20),0);
    lv_obj_add_style(p_screen_param_obj->obj_label[OBJ_LABEL_PINYIN_FORMAT],
        (lv_style_t *) get_lv_style_font_custom(20),0);

    if (enter_param != NULL) {
        memcpy(&p_screen_param_obj->enter_param,enter_param,sizeof(ui_screen_phonebook_enter_param_t));
        memcpy(&phonebook_param,&p_screen_param_obj->enter_param.phonebook_param,sizeof(phonebook_param_t));
        }
    else {

        }

#if 0
    lv_label_set_text(p_screen_param_obj->obj_label[OBJ_LABEL_NAME],phonebook_param.name);
    lv_label_set_text(p_screen_param_obj->obj_label[OBJ_LABEL_NUM],phonebook_param.number);

    lv_obj_set_style_text_color(p_screen_param_obj->obj_label[OBJ_LABEL_NAME],PHONEBOOK_NAME_TEXT_COLOR_FOCUS,
        LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_text_opa(p_screen_param_obj->obj_label[OBJ_LABEL_NAME],LV_OPA_COVER,
        LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_color(p_screen_param_obj->obj_label[OBJ_LABEL_NAME],PHONEBOOK_NAME_TEXT_BG_COLOR_FOCUS,
        LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(p_screen_param_obj->obj_label[OBJ_LABEL_NAME],LV_OPA_COVER,
        LV_PART_MAIN | LV_STATE_FOCUSED);

    lv_obj_set_style_text_color(p_screen_param_obj->obj_label[OBJ_LABEL_NUM],PHONEBOOK_NAME_TEXT_COLOR_FOCUS,
        LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_text_opa(p_screen_param_obj->obj_label[OBJ_LABEL_NUM],LV_OPA_COVER,
        LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_color(p_screen_param_obj->obj_label[OBJ_LABEL_NUM],PHONEBOOK_NAME_TEXT_BG_COLOR_FOCUS,
        LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(p_screen_param_obj->obj_label[OBJ_LABEL_NUM],LV_OPA_COVER,
        LV_PART_MAIN | LV_STATE_FOCUSED);
#endif

    p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME] = lv_textarea_create(ui_ScreenPhonebookedit);
    lv_obj_clear_flag(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_x(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],0);
    lv_obj_set_y(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],52);
    lv_obj_set_width(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],LV_PCT(100)); /// 1
    lv_obj_set_height(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],40); /// 1
    lv_textarea_set_one_line(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],true);
    lv_obj_set_style_bg_color(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],PHONEBOOK_NAME_TEXT_BG_COLOR_FOCUS,
        LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_pad_left(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],1,
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],0,
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],0,
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_border_width(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],0,
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_textarea_set_max_byte(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],PHONEBOOK_NAME_LEN);
    lv_textarea_add_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],phonebook_param.name);


    p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM] = lv_textarea_create(ui_ScreenPhonebookedit);
    lv_obj_clear_flag(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_x(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],0);
    lv_obj_set_y(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],152);
    lv_obj_set_width(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],LV_PCT(100)); /// 1
    lv_obj_set_height(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],40); /// 1

    lv_textarea_set_one_line(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],true);
    lv_obj_set_style_bg_color(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],PHONEBOOK_NAME_TEXT_BG_COLOR_FOCUS,
        LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_pad_left(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],1,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],0,
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],0,
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_border_width(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],0,
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_textarea_set_max_byte(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],PHONEBOOK_NUMBER_LEN);
    lv_textarea_add_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM],phonebook_param.number);


    lv_obj_set_user_data(ui_ScreenPhonebookedit,p_screen_param_obj);

    lv_obj_add_event_cb(ui_ScreenPhonebookedit,phonebook_fp_edit_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebookedit);


    lv_obj_set_user_data(win,ui_ScreenPhonebookedit);



    p_screen_param_obj->pinyin = NULL;
    set_phonebook_edit_name_fucous(ui_ScreenPhonebookedit);

    return ui_ScreenPhonebookedit;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(a->var);
    ime_pinyin_keyboard_destroy(p_screen_param_obj->pinyin);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebookedit_screen_uninit(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookedit = NULL;


    ui_ScreenPhonebookedit = lv_obj_get_user_data(win);

    if (ui_ScreenPhonebookedit) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebookedit,100,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}


void ui_ScreenPhonebookedit_screen_focus(lv_obj_t * win,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookedit = NULL;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;
    uint8_t         str_buf[100];

    ui_ScreenPhonebookedit = lv_obj_get_user_data(win);
    p_screen_param_obj  = lv_obj_get_user_data(ui_ScreenPhonebookedit);

    if (lv_obj_has_state(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],LV_STATE_FOCUSED)) {
        sprintf(str_buf,"%s%s","姓名",lv_textarea_get_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME]));
        }
    else {
        sprintf(str_buf,"%s%s","电话号码",
            lv_textarea_get_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM]));
        }

    lv_disp_load_scr(ui_ScreenPhonebookedit);

    app_fp_play_tts_menu(str_buf);

}


uint32_t is_phonebook_edit_name_fucous(lv_obj_t * ui_ScreenPhonebookedit)
{
    lv_obj_t *      edit_name;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    edit_name           = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME];

    return lv_obj_has_state(edit_name,LV_STATE_FOCUSED);
}


void set_phonebook_edit_name_fucous(lv_obj_t * ui_ScreenPhonebookedit)
{
    lv_obj_t *      edit_name;
    lv_obj_t *      edit_num;
    lv_obj_t *      label;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    edit_name           = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME];
    edit_num            = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM];

    lv_obj_add_state(edit_name,LV_STATE_FOCUSED);
    lv_obj_clear_state(edit_num,LV_STATE_FOCUSED);

    label               = p_screen_param_obj->obj_label[OBJ_LABEL_PINYIN_FORMAT];
    lv_label_set_text(label,"拼音");

    if (p_screen_param_obj->pinyin) {
        p_screen_param_obj->pinyin->ta_user = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME];

        //p_screen_param_obj->pinyin->label_format = label;
        ime_pinyin_ui_set_fortmat(p_screen_param_obj->pinyin,format_pinyin);
        ime_pinyin_ui_set_fortmat_flag_auto(p_screen_param_obj->pinyin,1);
        }

}


void set_phonebook_edit_num_fucous(lv_obj_t * ui_ScreenPhonebookedit)
{
    lv_obj_t *      edit_name;
    lv_obj_t *      edit_num;
    lv_obj_t *      label;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    edit_name           = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME];
    edit_num            = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM];

    lv_obj_add_state(edit_num,LV_STATE_FOCUSED);
    lv_obj_clear_state(edit_name,LV_STATE_FOCUSED);

    label               = p_screen_param_obj->obj_label[OBJ_LABEL_PINYIN_FORMAT];
    lv_label_set_text(label,"数字");


    if (p_screen_param_obj->pinyin) {
        p_screen_param_obj->pinyin->ta_user = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM];
        ime_pinyin_ui_set_fortmat(p_screen_param_obj->pinyin,format_num);
        ime_pinyin_ui_set_fortmat_flag_auto(p_screen_param_obj->pinyin,0);
        }

}


void set_phonebook_edit_fucous_cursor_right(lv_obj_t * ui_ScreenPhonebookedit)
{
    lv_obj_t *      edit_name;
    lv_obj_t *      edit_num;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    edit_name           = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME];
    edit_num            = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM];

    if (lv_obj_has_state(edit_name,LV_STATE_FOCUSED))
        lv_textarea_cursor_right(edit_name);
    else 
        lv_textarea_cursor_right(edit_num);
}


void set_phonebook_edit_fucous_cursor_left(lv_obj_t * ui_ScreenPhonebookedit)
{
    lv_obj_t *      edit_name;
    lv_obj_t *      edit_num;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    edit_name           = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME];
    edit_num            = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM];

    if (lv_obj_has_state(edit_name,LV_STATE_FOCUSED))
        lv_textarea_cursor_left(edit_name);
    else 
        lv_textarea_cursor_left(edit_num);
}


void del_phonebook_edit_fucous_text(lv_obj_t * ui_ScreenPhonebookedit)
{
    lv_obj_t *      edit_name;
    lv_obj_t *      edit_num;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    edit_name           = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME];
    edit_num            = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM];

    if (lv_obj_has_state(edit_name,LV_STATE_FOCUSED))
        lv_textarea_del_char(edit_name);
    else 
        lv_textarea_del_char(edit_num);
}


void phonebook_edit_pinyin_create(lv_obj_t * ui_ScreenPhonebookedit)
{
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);

    if (p_screen_param_obj->pinyin)
        return;

    p_screen_param_obj->pinyin = ime_pinyin_keyboard_create();

    if (p_screen_param_obj->pinyin) {
        p_screen_param_obj->pinyin->ta_user = p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME];
        p_screen_param_obj->pinyin->label_format = p_screen_param_obj->obj_label[OBJ_LABEL_PINYIN_FORMAT];
        ime_pinyin_ui_set_fortmat(p_screen_param_obj->pinyin,format_pinyin);
        ime_pinyin_ui_set_fortmat_flag_auto(p_screen_param_obj->pinyin,1);

        }


    lv_textarea_cursor_right(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME]);
    lv_textarea_cursor_right(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM]);
}


void remind_timer_callback_phonebookedit_storge(screen_remind_param_t * p_param)
{
    int32_t         ret;
    lv_obj_t *      ui_ScreenPhonebookedit;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;
    ui_screen_phonebook_enter_param_t * p_enter_param;

    ui_ScreenPhonebookedit = p_param->user_data;
    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    p_enter_param       = &p_screen_param_obj->enter_param;

    //alv_am_win_enter(NULL,"pb",(void*)p_enter_param);	
    ret                 = alv_am_win_exit_action(NULL,"pb",p_enter_param); //堆栈中有pb退出返回0，否者返回-1
    alv_am_win_exit_action(NULL,"details",p_enter_param);
    alv_am_win_exit_action(NULL,"storge_format",p_enter_param);
    alv_am_win_exit_action(NULL,"list_option",p_enter_param);

    if (ret) {
        alv_am_win_exit_action_by_app("idle_fp","idle_num_option",NULL);
        alv_am_win_exit_action(NULL,"edit",p_enter_param);
        }
    else {
        alv_am_win_jump_action(NULL,"pb",NULL);
        }
}


void ui_phonebook_edit_remind_screen_storge(lv_obj_t * ui_ScreenPhonebookedit)
{
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    ui_screen_remind_param_init(&screen_remind_param);

    switch (p_screen_param_obj->enter_param.phonebook_param.strage_format)
    {
        case 1:
            screen_remind_param.str = "联系人已存入SIM1";
            break;

        case 2:
            screen_remind_param.str = "联系人已存入SIM2";
            break;

        default:
            screen_remind_param.str = "联系人已存入手机";
            break;
        }

    screen_remind_param.tiemr_delay = 1500;
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_phonebookedit_storge;
    screen_remind_param.user_data = ui_ScreenPhonebookedit;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);

}


void remind_timer_callback_phonebookedit_no_name(screen_remind_param_t * p_param)
{
    lv_obj_t *      ui_ScreenPhonebookedit;

    ui_ScreenPhonebookedit = p_param->user_data;

    lv_disp_load_scr(ui_ScreenPhonebookedit);

}


void ui_phonebook_edit_remind_screen_no_name(lv_obj_t * ui_ScreenPhonebookedit)
{
    screen_remind_param_t screen_remind_param;
    lv_obj_t *      ui_screen;


    ui_screen_remind_param_init(&screen_remind_param);
    screen_remind_param.str = "没有名字或号码";
    screen_remind_param.fun = (remind_timer_callback_fun_t) remind_timer_callback_phonebookedit_no_name;
    screen_remind_param.user_data = ui_ScreenPhonebookedit;

    ui_screen           = ui_screen_remind_init(&screen_remind_param);

    lv_disp_load_scr(ui_screen);

}




void phonebook_edit_pinyin_key_event(uint32_t key,lv_obj_t * ui_ScreenPhonebookedit)
{
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);

    ime_pinyin_ui_event_actions(key,p_screen_param_obj->pinyin);
}


void phonebook_edit_key_event(uint32_t key,lv_obj_t * ui_ScreenPhonebookedit)
{
    lv_obj_t *      obj = ui_ScreenPhonebookedit;
    ui_screen_phonebook_edit_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_edit_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebookedit);
    uint8_t         str_buf[100];

    //__inf("phonebook_fp_edit_ui_event_actions LV_EVENT_KEY:%d\r\n",key);
    if (get_ime_pinyin_keyboard_active_state(p_screen_param_obj->pinyin)) {
        phonebook_edit_pinyin_key_event(key,obj);
        }
    else {
        if (key == LV_KEY_UP) {
            if (is_phonebook_edit_name_fucous(obj))
                set_phonebook_edit_num_fucous(obj);
            else 
                set_phonebook_edit_name_fucous(obj);

            if (lv_obj_has_state(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],LV_STATE_FOCUSED)) {
                sprintf(str_buf,"%s%s","姓名",
                    lv_textarea_get_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME]));
                }
            else {
                sprintf(str_buf,"%s%s","电话号码",
                    lv_textarea_get_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM]));
                }

            app_fp_play_tts_menu(str_buf);
            }
        else if (key == LV_KEY_DOWN) {
            if (is_phonebook_edit_name_fucous(obj))
                set_phonebook_edit_num_fucous(obj);
            else 
                set_phonebook_edit_name_fucous(obj);

            if (lv_obj_has_state(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME],LV_STATE_FOCUSED)) {
                sprintf(str_buf,"%s%s","姓名",
                    lv_textarea_get_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME]));
                }
            else {
                sprintf(str_buf,"%s%s","电话号码",
                    lv_textarea_get_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM]));
                }

            app_fp_play_tts_menu(str_buf);
            }
        else if (key == APP_KEY_VALUE_TOP_LEFT) {
            //set_phonebook_edit_fucous_cursor_left(obj);			
            alv_am_win_exit_current((void *) &p_screen_param_obj->enter_param);
            }
        else if (key == APP_KEY_VALUE_TOP_RIGHT) {
            //set_phonebook_edit_fucous_cursor_right(obj);
            del_phonebook_edit_fucous_text(obj);
            }
        else if (key == LV_KEY_ENTER) {
            phonebook_param_t phonebook_param;
            uint8_t         len,storage_flag;
            const char *    text;

            storage_flag        = 0;
            memset(&phonebook_param,0,sizeof(phonebook_param_t));
            memcpy(&phonebook_param,&p_screen_param_obj->enter_param.phonebook_param,sizeof(phonebook_param_t));

            text                =
                 (const char *) lv_textarea_get_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NAME]);
            len                 = strlen(text);

            if (len > PHONEBOOK_NAME_LEN) {
                len                 = PHONEBOOK_NAME_LEN;
                }
            else if (len == 0) {
                ui_phonebook_edit_remind_screen_no_name(ui_ScreenPhonebookedit);
                return;
                }

            if (0 == strncmp(phonebook_param.name,text,PHONEBOOK_NAME_LEN))
                storage_flag++;

            strncpy(phonebook_param.name,text,PHONEBOOK_NAME_LEN);
            phonebook_param.name[len] = 0;
            __inf("phonebook_edit_key_event len:%d,name:%s",len,text);

            text                =
                 (const char *) lv_textarea_get_text(p_screen_param_obj->obj_textarea[OBJ_TEXTAREA_NUM]);
            len                 = strlen(text);

            if (len > PHONEBOOK_NUMBER_LEN) {
                len                 = PHONEBOOK_NUMBER_LEN;
                }
            else if (len == 0) {
                ui_phonebook_edit_remind_screen_no_name(ui_ScreenPhonebookedit);
                return;
                }

            if (0 == strncmp(phonebook_param.number,text,PHONEBOOK_NUMBER_LEN))
                storage_flag++;

            strncpy(phonebook_param.number,text,len);
            phonebook_param.number[len] = 0;
            __inf("phonebook_edit_key_event len:%d,num:%s",len,text);

            //			if(storage_flag==2)
            //				return ;
            memset(phonebook_param.name_pinyin,0,PHONEBOOK_PINYIN_LEN + 2);
            chset_uft8_to_pinyin_str(phonebook_param.name_pinyin,PHONEBOOK_PINYIN_LEN,phonebook_param.name);

            if (p_screen_param_obj->enter_param.p_cur_node)
                edit_phonebook_param_storge(p_screen_param_obj->enter_param.p_cur_node,&phonebook_param);
            else 
                add_phonebook_param_storge(&phonebook_param);

            memcpy(&p_screen_param_obj->enter_param.phonebook_param,&phonebook_param,sizeof(phonebook_param_t));

            ui_phonebook_edit_remind_screen_storge(ui_ScreenPhonebookedit);
            }
        else if (key == LV_KEY_END) {
            alv_am_app_exit_action_all();
            }
        else {
            phonebook_edit_pinyin_key_event(key,obj);
            }
        }
}


