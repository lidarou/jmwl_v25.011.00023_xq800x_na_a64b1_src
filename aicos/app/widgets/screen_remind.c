
#include "obj_batch_deal.h"
#include "aic_osa.h"
#include <aic_type.h>
#include "aic_srv_tele.h"
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aic_list.h>
#include "screen_remind.h"
#include "status_fp.h"
#include "tele_sms_data.h"
#include "calllog_data.h"
#include "tts_app_player.h"


enum {
    OBJ_IMG_BG,
    OBJ_IMG_SIM,
    OBJ_IMG_SIGNAL,

    //	OBJ_IMG_MUTE,
    OBJ_IMG_BAT,
    OBJ_IMG_MAX,
};

LV_IMG_DECLARE(ui_img_bg_png);
LV_IMG_DECLARE(ui_img_signal_2g_0_png);
LV_IMG_DECLARE(ui_img_silence_png);
LV_IMG_DECLARE(ui_img_bat_0_png);
LV_IMG_DECLARE(ui_img_sim_sim_1_png);
LV_IMG_DECLARE(ui_img_idle_message_png); // assets\idle\message.png
LV_IMG_DECLARE(ui_img_idle_miss_png); // assets\idle\miss.png
LV_IMG_DECLARE(ui_img_clock_png);

extern lv_img_dsc_t * ui_imgset_bat[];
extern lv_img_dsc_t * ui_imgset_bat_charge[];
extern lv_img_dsc_t * ui_imgset_sim[];


static obj_batch_img_param_array_t obj_batch_img_param_array[OBJ_IMG_MAX] =
{
    {
        (void *) &ui_img_bg_png,0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_LEFT,0,LV_OBJ_FLAG_SCROLLABLE
        },
    {
        (void *) &ui_img_sim_sim_1_png,8,16,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_LEFT,0,
        LV_OBJ_FLAG_SCROLLABLE
        },
    {
        (void *) &ui_img_signal_2g_0_png,20,7,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_LEFT,0,
        LV_OBJ_FLAG_SCROLLABLE
        },

    //{&ui_img_silence_png,		164,	10,	LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_LEFT,0,LV_OBJ_FLAG_SCROLLABLE},
    {
        (void *) &ui_img_bat_0_png,194,11,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_LEFT,0,
        LV_OBJ_FLAG_SCROLLABLE
        },
};


typedef struct ui_screen_remind_obj_h {
    lv_obj_t *      obj_img[OBJ_IMG_MAX];
    lv_obj_t *      obj_bg;
    lv_obj_t *      obj_label;
    lv_obj_t *      obj_line;
    lv_obj_t *      obj_img_mute;
    lv_obj_t *      obj_img_message;
    lv_obj_t *      obj_img_missed;
    lv_style_t      style_label;
    lv_style_t      style_line;
    screen_remind_param_t param;
    lv_timer_t *    remind_timer;

    lv_timer_t *    handle_timer_bat;
    uint8_t         bat_img_count;

} ui_screen_remind_obj_t;

void remindlist_timer_start(lv_obj_t * ui_ScreenPhonebooRemind);
void ui_screen_remind_uninit(lv_obj_t * ui_Screen);

extern const lv_img_dsc_t * screen_idle1_get_radio_signal();


void screen_remind_ui_event_actions(lv_event_t * e)
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
                    remindlist_timer_start(obj);

                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    ui_screen_remind_obj_t * p_screen_param_obj;

                    p_screen_param_obj  = (ui_screen_remind_obj_t *) lv_obj_get_user_data(obj);
                    lv_group_remove_obj(obj);

                    if (p_screen_param_obj->param.self_destory_flag)
                        ui_screen_remind_uninit(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;
                ui_screen_remind_obj_t * p_screen_param;

                p_screen_param      = lv_obj_get_user_data(obj);

                switch (key)
                {
                    case LV_KEY_DEL:
                    case LV_KEY_END:
                        if (p_screen_param->param.key_exit_enable) {
                            if (p_screen_param->param.fun)
                                p_screen_param->param.fun(&p_screen_param->param);
                            }

                        break;

                    default:
                        break;
                    }
                }
            break;

        default:
            break;
        }


}


void screen_remind_bat_timer_stop(lv_obj_t * ui_Screen)
{
    ui_screen_remind_obj_t * p_screen_param;

    p_screen_param      = (ui_screen_remind_obj_t *) lv_obj_get_user_data(ui_Screen);

    if (p_screen_param->handle_timer_bat != NULL) {
        lv_timer_del(p_screen_param->handle_timer_bat);
        p_screen_param->handle_timer_bat = NULL;
        }


}



static void screen_remind_bat_timer_event_cb(struct _lv_timer_t *timer)
{
    lv_obj_t *      ui_Screen;
    ui_screen_remind_obj_t * p_screen_param;

    ui_Screen           = timer->user_data;
    p_screen_param      = (ui_screen_remind_obj_t *) lv_obj_get_user_data(ui_Screen);

    if (p_screen_param->bat_img_count >= 10)
        p_screen_param->bat_img_count = 0;

    lv_img_set_src(p_screen_param->obj_img[OBJ_IMG_BAT],
        (const void *) ui_imgset_bat_charge[p_screen_param->bat_img_count]);

    p_screen_param->bat_img_count++;


}


void screen_remind_bat_timer_start(lv_obj_t * ui_Screen)
{
    ui_screen_remind_obj_t * p_screen_param;

    p_screen_param      = (ui_screen_remind_obj_t *) lv_obj_get_user_data(ui_Screen);


    if (p_screen_param->handle_timer_bat == NULL) {
        p_screen_param->bat_img_count = 0;
        p_screen_param->handle_timer_bat = lv_timer_create(screen_remind_bat_timer_event_cb,200,ui_Screen);
        }

}


extern uint8_t is_alarm_on(void);
extern uint8_t is_in_silence_theme(void);


void ui_screenremind_create_remind_img(lv_obj_t * parent)
{
    lv_obj_t *      obj_img;
    ui_screen_remind_obj_t * p_screen_param;
    lv_coord_t      pos_x,pos_y;

    p_screen_param      = (ui_screen_remind_obj_t *) lv_obj_get_user_data(parent);

    pos_x               = 164;
    pos_y               = 10;

    if (p_screen_param->obj_img_mute) {
        lv_obj_del(p_screen_param->obj_img_mute);
        p_screen_param->obj_img_mute = NULL;
        }

    if (p_screen_param->obj_img_message) {
        lv_obj_del(p_screen_param->obj_img_message);
        p_screen_param->obj_img_message = NULL;
        }

    if (p_screen_param->obj_img_missed) {
        lv_obj_del(p_screen_param->obj_img_missed);
        p_screen_param->obj_img_missed = NULL;
        }

#ifdef CONFIG_APP_SETTINGS_FP

    if (is_in_silence_theme() == 1) {
        obj_img             = lv_img_create(parent);

        lv_img_set_src(obj_img,&ui_img_silence_png);
        lv_obj_set_x(obj_img,pos_x);
        lv_obj_set_y(obj_img,pos_y);
        lv_obj_set_width(obj_img,LV_SIZE_CONTENT);
        lv_obj_set_height(obj_img,LV_SIZE_CONTENT);
        p_screen_param->obj_img_mute = obj_img;


        pos_x               = pos_x - 21 - 3;
        }

#endif

#ifdef CONFIG_APP_SMS_FP

    if (tele_sms_rev_unread_count_get()) {
        obj_img             = lv_img_create(parent);

        lv_img_set_src(obj_img,&ui_img_idle_message_png);
        lv_obj_set_x(obj_img,pos_x);
        lv_obj_set_y(obj_img,pos_y);
        lv_obj_set_width(obj_img,LV_SIZE_CONTENT);
        lv_obj_set_height(obj_img,LV_SIZE_CONTENT);

        p_screen_param->obj_img_message = obj_img;


        pos_x               = pos_x - 21 - 3;
        }

#endif

#ifdef CONFIG_APP_CALL_FP

    if (calllg_missed_unread_count_get()) {
        obj_img             = lv_img_create(parent);

        lv_img_set_src(obj_img,&ui_img_idle_miss_png);
        lv_obj_set_x(obj_img,pos_x);
        lv_obj_set_y(obj_img,pos_y);
        lv_obj_set_width(obj_img,LV_SIZE_CONTENT);
        lv_obj_set_height(obj_img,LV_SIZE_CONTENT);
        p_screen_param->obj_img_missed = obj_img;

        pos_x               = pos_x - 21 - 3;
        }

#endif

#ifdef CONFIG_APP_TOOLS_FP

    if (is_alarm_on() == 1) {
        obj_img             = lv_img_create(parent);

        lv_img_set_src(obj_img,&ui_img_clock_png);
        lv_obj_set_x(obj_img,pos_x);
        lv_obj_set_y(obj_img,pos_y);
        lv_obj_set_width(obj_img,LV_SIZE_CONTENT);
        lv_obj_set_height(obj_img,LV_SIZE_CONTENT);
        p_screen_param->obj_img_mute = obj_img;


        pos_x               = pos_x - 21 - 3;
        }

#endif
}


lv_obj_t * ui_screen_remind_init(screen_remind_param_t * param)
{
    lv_obj_t *      ui_Screen;
    lv_img_dsc_t *  img_dsc;

    ui_screen_remind_obj_t * p_param_screenRemind;

    p_param_screenRemind = lv_mem_alloc(sizeof(ui_screen_remind_obj_t));

    if (p_param_screenRemind == NULL)
        return 0;

    memset(p_param_screenRemind,0,sizeof(ui_screen_remind_obj_t));

    ui_Screen           = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen,LV_OBJ_FLAG_SCROLLABLE); /// Flags

    //lv_obj_add_style(ui_Screen, get_lv_style_font_common(), 0);
    lv_obj_set_style_radius(ui_Screen,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_Screen,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_Screen,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Screen,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Screen,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Screen,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    obj_batch_img_create(ui_Screen,(lv_obj_t * *) &p_param_screenRemind->obj_img,obj_batch_img_param_array,
        OBJ_IMG_MAX);


    img_dsc             = (lv_img_dsc_t *) screen_idle1_get_radio_signal();
    lv_img_set_src(p_param_screenRemind->obj_img[OBJ_IMG_SIGNAL],img_dsc);
    {
        uint8_t         card_id;

        if (0 == aic_srv_tele_sim_get_current_card(&card_id))
            lv_img_set_src(p_param_screenRemind->obj_img[OBJ_IMG_SIM],ui_imgset_sim[card_id]);
        }


    p_param_screenRemind->obj_bg = lv_obj_create(ui_Screen);
    lv_obj_set_x(p_param_screenRemind->obj_bg,0);
    lv_obj_set_y(p_param_screenRemind->obj_bg,40);
    lv_obj_set_width(p_param_screenRemind->obj_bg,240); /// 1
    lv_obj_set_height(p_param_screenRemind->obj_bg,280); /// 1
    lv_obj_clear_flag(p_param_screenRemind->obj_bg,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(p_param_screenRemind->obj_bg,lv_color_hex(0xffffff),LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(p_param_screenRemind->obj_bg,LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(p_param_screenRemind->obj_bg,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_param_screenRemind->obj_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t *      obj_txt_bg = lv_obj_create(p_param_screenRemind->obj_bg);

    lv_obj_set_x(obj_txt_bg,10);
    lv_obj_set_y(obj_txt_bg,10);
    lv_obj_set_width(obj_txt_bg,220);               /// 1
    lv_obj_set_height(obj_txt_bg,250);              /// 1
    lv_obj_clear_flag(obj_txt_bg,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj_txt_bg,lv_color_hex(0xffffff),LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(obj_txt_bg,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj_txt_bg,LV_OPA_100,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj_txt_bg,0,LV_PART_MAIN | LV_STATE_DEFAULT);


    p_param_screenRemind->obj_label = lv_label_create(obj_txt_bg);

    //lv_obj_set_x(p_param_screenRemind->obj_label, 10);
    //lv_obj_set_y(p_param_screenRemind->obj_label, 80);
    lv_obj_set_width(p_param_screenRemind->obj_label,220); /// 1

    //lv_obj_set_height(p_param_screenRemind->obj_label, 160);	 /// 1		
    lv_obj_set_style_border_side(p_param_screenRemind->obj_label,LV_BORDER_SIDE_NONE,
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(p_param_screenRemind->obj_label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(p_param_screenRemind->obj_label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(p_param_screenRemind->obj_label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(p_param_screenRemind->obj_label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(p_param_screenRemind->obj_label,LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(p_param_screenRemind->obj_label,LV_TEXT_ALIGN_CENTER,0);


    memcpy(&p_param_screenRemind->param,param,sizeof(screen_remind_param_t));


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
            228,268
            },
        {
            8,268
            },
        {
            8,10
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

    if (1 == get_bat_chg_status()) {
        screen_remind_bat_timer_start(ui_Screen);

        lv_img_set_src(p_param_screenRemind->obj_img[OBJ_IMG_BAT],(const void *) ui_imgset_bat_charge[0]);
        }
    else {
        uint8_t         bat_capacity = get_bat_capacity();

        bat_capacity        = (bat_capacity + 5) / 10;

        lv_img_set_src(p_param_screenRemind->obj_img[OBJ_IMG_BAT],(const void *) ui_imgset_bat[bat_capacity]);
        }



    lv_obj_add_event_cb(ui_Screen,screen_remind_ui_event_actions,LV_EVENT_ALL,ui_Screen);

    ui_screenremind_create_remind_img(ui_Screen);


    return ui_Screen;
}



static void phonebook_remind_timer_stop(lv_obj_t * ui_ScreenPhonebooRemind)
{
    ui_screen_remind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_remind_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebooRemind);

    if (p_screen_param_obj->remind_timer != NULL) {
        lv_timer_del(p_screen_param_obj->remind_timer);
        p_screen_param_obj->remind_timer = NULL;
        }

}

#ifdef CONFIG_AUDIO_TTS
extern uint8_t return_tts_play_status(void);

#endif

static void remind_timer_event_cb(struct _lv_timer_t *timer)
{
    screen_remind_param_t * p_param;

    p_param             = timer->user_data;
    lv_timer_pause(timer);

#ifdef CONFIG_AUDIO_TTS

    if (return_tts_play_status()) {
        lv_timer_set_period(timer,100);
        lv_timer_resume(timer);
        return;
        }

#endif

    if (p_param->fun)
        p_param->fun(p_param);


}


void remindlist_timer_start(lv_obj_t * ui_ScreenPhonebooRemind)
{
    ui_screen_remind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_remind_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebooRemind);
    uint32_t        delays;

    delays              = p_screen_param_obj->param.tiemr_delay;

    if (delays == 0)
        delays = 1500;

    if (p_screen_param_obj->remind_timer == NULL) {
        p_screen_param_obj->remind_timer = lv_timer_create(remind_timer_event_cb,delays,&p_screen_param_obj->param);

        //lv_timer_set_repeat_count(p_screen_param_obj->phonebook_remind_timer, 1);
        }

    app_fp_play_tts_menu(lv_label_get_text(p_screen_param_obj->obj_label));

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_remind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_remind_obj_t *) lv_obj_get_user_data(a->var);
    phonebook_remind_timer_stop(a->var);
    screen_remind_bat_timer_stop(a->var);
    lv_style_reset(&p_screen_param_obj->style_label);
    lv_style_reset(&p_screen_param_obj->style_line);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_screen_remind_uninit(lv_obj_t * ui_Screen)
{
    if (ui_Screen) {
        app_tts_stop();
        lv_obj_del_delayed_callback(ui_Screen,200,lv_obj_del_screen_ready_cb);
        }

    ui_Screen           = NULL;
}



void ui_screen_remind_param_init(screen_remind_param_t * param)
{
    //param->str 			= NULL;	
    param->tiemr_delay  = 1000;

    //param->fun 			= NULL;
    //param->user_data	= NULL;
    param->self_destory_flag = 1;
    param->key_exit_enable = 1;
}


