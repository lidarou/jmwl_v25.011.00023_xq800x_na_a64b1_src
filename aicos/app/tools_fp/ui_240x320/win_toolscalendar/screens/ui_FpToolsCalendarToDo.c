
#include "../ui.h"
#include "app_log.h"
#include "obj_batch_deal.h"

//#include "screen_remind.h"
#include "aic_srv_tele.h"
#include "aic_md_id.h"
#include "alv_app_mgr.h"
#include <stdio.h>
#include "aic_md_api.h"

#define __inf                   APP_LOGI

#define UI_TODO_INSERT_STR_MAX  42

typedef struct ui_screen_todo_write_obj_h {
    lv_obj_t *      label_back;
    lv_obj_t *      label_input_format;
    lv_obj_t *      label_remain_num;
    lv_style_t      style_remain_num;
    lv_obj_t *      obj_textarea;
    pinyin_param_t * pinyin;

    uint8_t         str_remain_num[5];
    uint32_t        wirte_count;

    todo_write_ui_enter_param_t ui_enter_param;

    uint8_t         insert_str[UI_TODO_INSERT_STR_MAX];
} ui_screen_todo_write_t;

#define UI_TODO_WIRTE_COUNT_MAX 45
extern void tools_calendar_todo_timer_start(void);


lv_obj_t * ui_FpToolsCalendarTodo_screen_init(lv_obj_t * win,void * param)
{
    lv_obj_t *      ui_FpToolsCalendarToDo = NULL;

    //	todo_write_ui_enter_param_t *p_param;
    ui_screen_todo_write_t * p_screen_param;
    lv_obj_t *      obj;

    p_screen_param      = lv_mem_alloc(sizeof(ui_screen_todo_write_t));

    if (p_screen_param == NULL)
        return 0;

    memset(p_screen_param,0,sizeof(ui_screen_todo_write_t));

    //	p_param = NULL;
    ui_FpToolsCalendarToDo = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_FpToolsCalendarToDo,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_FpToolsCalendarToDo,(lv_style_t *) get_lv_style_font_common(),0);


    obj                 = lv_label_create(ui_FpToolsCalendarToDo);
    lv_obj_set_width(obj,100);
    lv_obj_set_height(obj,20);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,0);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);
    lv_label_set_text(obj,"拼音");
    p_screen_param->label_input_format = obj;

    obj                 = lv_label_create(ui_FpToolsCalendarToDo);
    lv_obj_set_width(obj,48);
    lv_obj_set_height(obj,40);
    lv_obj_set_x(obj,192);
    lv_obj_set_y(obj,0);
    sprintf(p_screen_param->str_remain_num,"%d",UI_TODO_WIRTE_COUNT_MAX);
    lv_label_set_text(obj,p_screen_param->str_remain_num);
    p_screen_param->label_remain_num = obj;


    lv_style_init(&p_screen_param->style_remain_num);
    lv_style_set_text_font(&p_screen_param->style_remain_num,(lv_font_t *) get_ft_info_font_common());
    lv_style_set_text_align(&p_screen_param->style_remain_num,LV_TEXT_ALIGN_RIGHT);
    lv_style_set_text_color(&p_screen_param->style_remain_num,lv_color_hex(0xFF));
    lv_obj_add_style(p_screen_param->label_remain_num,&p_screen_param->style_remain_num,0);

    obj                 = lv_label_create(ui_FpToolsCalendarToDo);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,24);
    lv_label_set_text(obj,"备忘录内容:");

    obj                 = lv_label_create(ui_FpToolsCalendarToDo);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,1);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"确定");

    obj                 = lv_label_create(ui_FpToolsCalendarToDo);
    lv_obj_set_width(obj,LV_SIZE_CONTENT);
    lv_obj_set_height(obj,LV_SIZE_CONTENT);
    lv_obj_set_x(obj,162);
    lv_obj_set_y(obj,277);
    lv_label_set_text(obj,"返回");
    p_screen_param->label_back = obj;


    obj                 = lv_textarea_create(ui_FpToolsCalendarToDo);
    lv_obj_set_x(obj,-1);
    lv_obj_set_y(obj,64);
    lv_obj_set_width(obj,242);                      /// 1
    lv_obj_set_height(obj,180);                     /// 1

    //lv_textarea_set_one_line(obj, true);
    lv_obj_set_style_pad_left(obj,1,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_color(obj,lv_color_hex(0xfffff),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(obj,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(obj,lv_color_white(),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);

    //lv_textarea_set_max_length(obj,UI_TODO_WIRTE_COUNT_MAX);
    lv_textarea_set_max_byte(obj,UI_TODO_WIRTE_COUNT_MAX);

    lv_textarea_set_one_line(obj,false);

    if (param) {
        lv_textarea_add_text(obj,param);
        }
    else {
        ui_FpToolsCalendarTodoSelected = 0xff;      //0xff表示新增一条备忘录，保存位置需要去找；不是0xff表示是编辑一条备忘录，完了保存在相同位置
        memset(calendar_todo_content,0,50);
        }

    lv_obj_add_state(obj,LV_STATE_FOCUSED);
    p_screen_param->obj_textarea = obj;


    p_screen_param->pinyin = NULL;

    lv_obj_set_user_data(ui_FpToolsCalendarToDo,p_screen_param);

    lv_obj_add_event_cb(ui_FpToolsCalendarToDo,tools_calendar_app_todo_ui_event_actions,LV_EVENT_ALL,
        ui_FpToolsCalendarToDo);

    lv_obj_set_user_data(win,ui_FpToolsCalendarToDo);

    return ui_FpToolsCalendarToDo;
}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_todo_write_t * p_screen_param;

    p_screen_param      = (ui_screen_todo_write_t *) lv_obj_get_user_data(a->var);
    ime_pinyin_keyboard_destroy(p_screen_param->pinyin);
    lv_style_reset(&p_screen_param->style_remain_num);
    lv_mem_free(p_screen_param);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_FpToolsCalendarTodo_screen_uninit(lv_obj_t * win,void * param)
{
    lv_obj_t *      ui_FpToolsCalendarToDo = NULL;

    ui_FpToolsCalendarToDo = lv_obj_get_user_data(win);

    if (ui_FpToolsCalendarToDo) {
        lv_obj_del_delayed_callback(ui_FpToolsCalendarToDo,300,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(win,NULL);
}


void ui_FpToolsCalendarTodo_screen_focus(lv_obj_t * win,void * enter_param)
{
    lv_obj_t *      ui_FpToolsCalendarToDo;
    ui_screen_todo_write_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    ui_FpToolsCalendarToDo = lv_obj_get_user_data(win);
    p_screen_param      = (ui_screen_todo_write_t *) lv_obj_get_user_data(ui_FpToolsCalendarToDo);

    if (strlen(p_screen_param->insert_str))
        lv_textarea_add_text(p_screen_param->obj_textarea,p_screen_param->insert_str);

    memset(p_screen_param->insert_str,0,UI_TODO_INSERT_STR_MAX);
    label               = lv_textarea_get_label(p_screen_param->obj_textarea);

    //	count = _lv_txt_get_encoded_length(lv_label_get_text(label));
    count               = _lv_txt_encoded_get_byte_id(lv_label_get_text(label),45);
    sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_TODO_WIRTE_COUNT_MAX - count));
    lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

    if (p_screen_param->wirte_count != count) {
        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_TODO_WIRTE_COUNT_MAX - count));
        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

        if (p_screen_param->wirte_count == 0)
            lv_label_set_text(p_screen_param->label_back,"清除");
        else if (count == 0)
            lv_label_set_text(p_screen_param->label_back,"返回");

        p_screen_param->wirte_count = count;
        }

    lv_disp_load_scr(ui_FpToolsCalendarToDo);

}


/*
void ui_FpToolsCalendarToDo_screen_defocus(lv_obj_t *win,void *enter_param)
{
    
}
*/
void todo_write_pinyin_create(lv_obj_t * ui_Screen)
{
    ui_screen_todo_write_t * p_screen_param;

    p_screen_param      = (ui_screen_todo_write_t *) lv_obj_get_user_data(ui_Screen);

    p_screen_param->pinyin = ime_pinyin_keyboard_create();

    if (p_screen_param->pinyin) {
        p_screen_param->pinyin->ta_user = p_screen_param->obj_textarea;
        p_screen_param->pinyin->label_format = p_screen_param->label_input_format;
        ime_pinyin_ui_set_fortmat(p_screen_param->pinyin,format_pinyin);
        ime_pinyin_ui_set_fortmat_flag_auto(p_screen_param->pinyin,1);
        }
}



void todo_write_key_event(uint32_t key,lv_obj_t * ui_Screen)
{
    ui_screen_todo_write_t * p_screen_param;
    lv_obj_t *      label;
    uint32_t        count;

    p_screen_param      = (ui_screen_todo_write_t *) lv_obj_get_user_data(ui_Screen);

    if (get_ime_pinyin_keyboard_active_state(p_screen_param->pinyin)) {

        ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
        label               = lv_textarea_get_label(p_screen_param->obj_textarea);

        //		count = _lv_txt_get_encoded_length(lv_label_get_text(label));
        count               = _lv_txt_encoded_get_byte_id(lv_label_get_text(label),45);

        if (p_screen_param->wirte_count != count) {
            sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_TODO_WIRTE_COUNT_MAX - count));
            lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

            if (p_screen_param->wirte_count == 0)
                lv_label_set_text(p_screen_param->label_back,"清除");
            else if (count == 0)
                lv_label_set_text(p_screen_param->label_back,"返回");

            p_screen_param->wirte_count = count;
            }
        }
    else {
        if ((key == LV_KEY_ENTER) &&
             ((true == lv_obj_has_flag(p_screen_param->pinyin->cand_panel,LV_OBJ_FLAG_HIDDEN))) &&
             (p_screen_param->pinyin->cand_panel1 == NULL)) {
            memset(calendar_todo_content,0,TODOLIST_CONTENT_LEN + 5);
            strcpy(calendar_todo_content,lv_textarea_get_text(p_screen_param->obj_textarea));

            if (ui_FpToolsCalendarTodoSelected == 0xff) // 新增加一条，看看超过10条没，有10条的话，提示删除
            {
                s32             ret;
                uint8_t         i   = 0;
                todolist_all_nvram_param_t todolist_all;

                ret                 = aic_md_read(MD_ID_TOOLS_TODO,sizeof(todolist_all_nvram_param_t),&todolist_all);

                if (ret) {
                    APP_LOGI("read net cfg error 0x%x\n",ret);
                    }

                for (i = 0; i < 10; i++) {
                    if (todolist_all.todolist[i].year < 1970 || todolist_all.todolist[i].year > 2037) {
                        break;
                        }
                    }

                if (i >= 10) //超过10条，提示删除
                {
                    alv_am_win_jump_action(NULL,"tool_cal_tododel",NULL);
                    }
                else 
                    alv_am_win_jump_action(NULL,"tool_cal_alarm",NULL);
                }
            else 
                // 编辑旧的备忘录
                alv_am_win_jump_action(NULL,"tool_cal_alarm",NULL);

            return;
            }

        switch (key)
        {
            case LV_KEY_DEL:
                if (p_screen_param->wirte_count) {
                    lv_textarea_del_char(p_screen_param->obj_textarea);

                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);

                    //					count = _lv_txt_get_encoded_length(lv_label_get_text(label));
                    count               = _lv_txt_encoded_get_byte_id(lv_label_get_text(label),45);

                    if (p_screen_param->wirte_count != count) {
                        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_TODO_WIRTE_COUNT_MAX - count));
                        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                        if (p_screen_param->wirte_count == 0)
                            lv_label_set_text(p_screen_param->label_back,"清除");
                        else if (count == 0)
                            lv_label_set_text(p_screen_param->label_back,"返回");

                        p_screen_param->wirte_count = count;
                        }
                    }
                else {
                    uint8_t         index = 1;      // 回退到设置的显示设置选项

                    alv_am_win_jump_action(NULL,"tool_cal_item",(void *) &index);
                    }

                break;

            case LV_KEY_ESC:
                memset(calendar_todo_content,0,TODOLIST_CONTENT_LEN + 5);
                strcpy(calendar_todo_content,lv_textarea_get_text(p_screen_param->obj_textarea));

                if (ui_FpToolsCalendarTodoSelected == 0xff) // 新增加一条，看看超过10条没，有10条的话，提示删除
                {
                    s32             ret;
                    uint8_t         i   = 0;
                    todolist_all_nvram_param_t todolist_all;

                    ret                 = aic_md_read(MD_ID_TOOLS_TODO,sizeof(todolist_all_nvram_param_t),
                        &todolist_all);

                    if (ret) {
                        APP_LOGI("read net cfg error 0x%x\n",ret);
                        }

                    for (i = 0; i < 10; i++) {
                        if (todolist_all.todolist[i].year < 1970 || todolist_all.todolist[i].year > 2037) {
                            break;
                            }
                        }

                    if (i >= 10) //超过10条，提示删除
                    {
                        alv_am_win_jump_action(NULL,"tool_cal_tododel",NULL);
                        }
                    else 
                        alv_am_win_jump_action(NULL,"tool_cal_alarm",NULL);
                    }
                else 
                    // 编辑旧的备忘录
                    alv_am_win_jump_action(NULL,"tool_cal_alarm",NULL);

                break;

            case LV_KEY_END:
                alv_am_app_exit_action_all();
                break;

            default:
                {
                    ime_pinyin_ui_event_actions(key,p_screen_param->pinyin);
                    label               = lv_textarea_get_label(p_screen_param->obj_textarea);
                    count               = _lv_txt_encoded_get_byte_id(lv_label_get_text(label),45);

                    //					count = _lv_txt_get_encoded_length(lv_label_get_text(label));
                    if (p_screen_param->wirte_count != count) {
                        sprintf(p_screen_param->str_remain_num,"%d",(int) (UI_TODO_WIRTE_COUNT_MAX - count));
                        lv_label_set_text(p_screen_param->label_remain_num,p_screen_param->str_remain_num);

                        if (p_screen_param->wirte_count == 0)
                            lv_label_set_text(p_screen_param->label_back,"清除");
                        else if (count == 0)
                            lv_label_set_text(p_screen_param->label_back,"返回");

                        p_screen_param->wirte_count = count;
                        }
                    }
                break;
            }
        }


}



