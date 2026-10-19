
#include "obj_batch_deal.h"
#include "aic_osa.h"
#include <aic_type.h>
#include "aic_srv_tele.h"
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aic_list.h>
#include "idle_lockscreen_remind.h"
#include "app_log.h"
#include "alv_app_mgr.h"

#define __inf                   APP_LOGI




typedef struct ui_screen_remind_obj_h {
    //	lv_obj_t *obj_img[OBJ_IMG_MAX];
    lv_obj_t *      obj_bg;
    lv_obj_t *      obj_label;
    lv_obj_t *      obj_line;
    lv_style_t      style_label;
    lv_style_t      style_line;
    idle_lock_screen_remind_param_t param;
    lv_timer_t *    remind_timer;

} ui_idle_lock_screen_remind_obj_t;


void idle_lock_remindlist_timer_start(lv_obj_t * ui_ScreenIdleLockRemind);
void ui_idle_lock_screen_remind_uninit(lv_obj_t * ui_Screen);


static void remind_timer_event_cb(struct _lv_timer_t *timer)
{
    idle_lock_screen_remind_param_t * p_param;

    p_param             = timer->user_data;
    lv_timer_pause(timer);

    if (p_param->fun)
        p_param->fun(p_param);


}


void remind_key_event_state(uint32_t key,uint32_t code,lv_obj_t * ui_Screen)
{
    ui_idle_lock_screen_remind_obj_t * p_screen_param;
    uint32_t        flag;



    p_screen_param      = (ui_idle_lock_screen_remind_obj_t *) lv_obj_get_user_data(ui_Screen);

    key_update_pressed(key,code);
    flag                = key_get_pressed_flag(key);

    APP_LOGI("remind_key_event_state key= %d,flag=%d,flag1=%d,key=%d,\n",p_screen_param->param.key,flag,
        (1 << LV_EVENT_RELEASED),key);

    switch (key)
    {
        case LV_KEY_DEL:
            {
                if ((p_screen_param->param.key == LV_KEY_ESC) && ((1 << LV_EVENT_RELEASED) &flag)) {
                    alv_am_win_jump_action(NULL,"watch",NULL);
                    p_screen_param->param.key = 0x80;

                    if (p_screen_param->remind_timer) {
                        lv_timer_pause(p_screen_param->remind_timer);
                        remind_timer_event_cb(p_screen_param->remind_timer);
                        }
                    }

                }
            break;

        default:
            {
                }
            break;
        }

    if (code == LV_EVENT_RELEASED)
        key_update_release(key,LV_EVENT_RELEASED);
}


void idle_lock_screen_remind_ui_event_actions(lv_event_t * e)
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
                    idle_lock_remindlist_timer_start(obj);

                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    ui_idle_lock_screen_remind_obj_t * p_screen_param_obj;

                    p_screen_param_obj  = (ui_idle_lock_screen_remind_obj_t *) lv_obj_get_user_data(obj);
                    lv_group_remove_obj(obj);

                    if (p_screen_param_obj->param.self_destory_flag)
                        ui_idle_lock_screen_remind_uninit(obj);
                    }
                }
            break;

        case LV_EVENT_PRESSED:
        case LV_EVENT_LONG_PRESSED:
        case LV_EVENT_LONG_PRESSED_REPEAT:
        case LV_EVENT_RELEASED:
            {
                lv_indev_t *    indev_act;
                uint32_t        key;

                indev_act           = lv_event_get_param(e);
                key                 = indev_act->proc.types.keypad.last_key;

                remind_key_event_state(key,event_code,e->user_data);

                }

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



lv_obj_t * ui_idle_lock_screen_remind_init(idle_lock_screen_remind_param_t * param)
{
    lv_obj_t *      ui_Screen;

    ui_idle_lock_screen_remind_obj_t * p_param_screenRemind;

    p_param_screenRemind = lv_mem_alloc(sizeof(ui_idle_lock_screen_remind_obj_t));

    if (p_param_screenRemind == NULL)
        return 0;

    memset(p_param_screenRemind,0,sizeof(ui_idle_lock_screen_remind_obj_t));

    ui_Screen           = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen,LV_OBJ_FLAG_SCROLLABLE); /// Flags

    //lv_obj_add_style(ui_Screen, get_lv_style_font_common(), 0);
    //	obj_batch_img_create(ui_Screen,&p_param_screenRemind->obj_img,obj_batch_img_param_array,OBJ_IMG_MAX);
    lv_obj_set_style_bg_color(ui_Screen,lv_color_hex(0xffffff),LV_PART_MAIN | LV_STATE_DEFAULT); //黑色背景

    p_param_screenRemind->obj_bg = lv_obj_create(ui_Screen);
    lv_obj_set_x(p_param_screenRemind->obj_bg,0);
    lv_obj_set_y(p_param_screenRemind->obj_bg,0);
    lv_obj_set_width(p_param_screenRemind->obj_bg,240); /// 1
    lv_obj_set_height(p_param_screenRemind->obj_bg,320); /// 1
    lv_obj_clear_flag(p_param_screenRemind->obj_bg,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(p_param_screenRemind->obj_bg,lv_color_hex(0xffffff),LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(p_param_screenRemind->obj_bg,LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(p_param_screenRemind->obj_bg,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t *      obj_txt_bg = lv_obj_create(p_param_screenRemind->obj_bg);

    lv_obj_set_x(obj_txt_bg,10);
    lv_obj_set_y(obj_txt_bg,10);
    lv_obj_set_width(obj_txt_bg,220);               /// 1
    lv_obj_set_height(obj_txt_bg,310);              /// 1
    lv_obj_clear_flag(obj_txt_bg,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj_txt_bg,lv_color_hex(0xffffff),LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj_txt_bg,LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(obj_txt_bg,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    p_param_screenRemind->obj_label = lv_label_create(ui_Screen);

    //lv_obj_set_x(p_param_screenRemind->obj_label, 10);
    //lv_obj_set_y(p_param_screenRemind->obj_label, 80);
    lv_obj_set_width(p_param_screenRemind->obj_label,220); /// 1

    //lv_obj_set_height(p_param_screenRemind->obj_label, 160);	 /// 1	
    lv_obj_set_style_pad_top(p_param_screenRemind->obj_label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_param_screenRemind->obj_label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(p_param_screenRemind->obj_label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(p_param_screenRemind->obj_label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(p_param_screenRemind->obj_label,LV_ALIGN_CENTER);


    memcpy(&p_param_screenRemind->param,param,sizeof(idle_lock_screen_remind_param_t));


    if (param->str) {
        lv_label_set_text(p_param_screenRemind->obj_label,param->str);
        }

    lv_style_init(&p_param_screenRemind->style_label);
    lv_style_set_text_font(&p_param_screenRemind->style_label,(lv_font_t *) get_ft_info_font_common());
    lv_style_set_text_align(&p_param_screenRemind->style_label,LV_TEXT_ALIGN_CENTER);
    lv_obj_add_style(p_param_screenRemind->obj_label,(lv_style_t *) &p_param_screenRemind->style_label,0);


    const static lv_point_t line_points[] = {
        {
            8,8
            },
        {
            228,8
            },
        {
            228,308
            },
        {
            8,308
            },
        {
            8,8
            }
        };

    p_param_screenRemind->obj_line = lv_line_create(p_param_screenRemind->obj_bg);

    lv_line_set_points(p_param_screenRemind->obj_line,line_points,5); // 设置点数组。line将连接这些点，按顺序画出直线

    lv_style_init(&p_param_screenRemind->style_line);

    // 下面3个样式是 line 的专有样式接口，类似于arc
    lv_style_set_line_width(&p_param_screenRemind->style_line,2);

    //lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
    // 使用这个样式能让画出来的线条看起来更平滑
    lv_style_set_line_rounded(&p_param_screenRemind->style_line,true);
    lv_obj_add_style(p_param_screenRemind->obj_line,(lv_style_t *) &p_param_screenRemind->style_line,0);



    lv_obj_set_user_data(ui_Screen,p_param_screenRemind);

    lv_obj_add_event_cb(ui_Screen,idle_lock_screen_remind_ui_event_actions,LV_EVENT_ALL,ui_Screen);


    return ui_Screen;
}



void idle_lock_remindlist_timer_start(lv_obj_t * ui_ScreenIdleLockRemind)
{
    ui_idle_lock_screen_remind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_idle_lock_screen_remind_obj_t *) lv_obj_get_user_data(ui_ScreenIdleLockRemind);
    uint32_t        delays;

    delays              = p_screen_param_obj->param.tiemr_delay;

    if (delays == 0)
        delays = 1500;

    if (p_screen_param_obj->remind_timer == NULL) {
        p_screen_param_obj->remind_timer = lv_timer_create(remind_timer_event_cb,delays,&p_screen_param_obj->param);

        //lv_timer_set_repeat_count(p_screen_param_obj->phonebook_remind_timer, 1);
        }

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_idle_lock_screen_remind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_idle_lock_screen_remind_obj_t *) lv_obj_get_user_data(a->var);
    lv_style_reset(&p_screen_param_obj->style_label);
    lv_style_reset(&p_screen_param_obj->style_line);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_idle_lock_screen_remind_uninit(lv_obj_t * ui_Screen)
{
    if (ui_Screen) {
        lv_obj_del_delayed_callback(ui_Screen,500,lv_obj_del_screen_ready_cb);
        }

    ui_Screen           = NULL;
}



void ui_idle_lock_screen_remind_param_init(idle_lock_screen_remind_param_t * param)
{
    //param->str 			= NULL;	
    param->tiemr_delay  = 1000;

    //param->fun 			= NULL;
    //param->user_data	= NULL;
    param->self_destory_flag = 1;

}


