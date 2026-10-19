
#include "../ui.h"
#include "obj_batch_deal.h"
#include "aic_osa.h"
#include <aic_type.h>
#include "aic_srv_tele.h"
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aic_list.h>
#include "screen_remind.h"


enum {
    OBJ_LABEL_OK,
    OBJ_LABEL_BACK,
    OBJ_LABEL_REMIND,
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
    {
        0,0,240,280,0,NULL
        },
};



typedef struct ui_screenidle1_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    screen_remind_param_t param;
    lv_timer_t *    phonebook_remind_timer;

} param_screenRemind_obj_t;


lv_obj_t *      ui_ScreenPhonebookRemind;


lv_obj_t * ui_ScreenPhonebookRemind_screen_init(void * param)
{
    param_screenRemind_obj_t * p_param_screenRemind;
    screen_remind_param_t * p_screen_remind_param;

    p_param_screenRemind = lv_mem_alloc(sizeof(param_screenRemind_obj_t));

    if (p_param_screenRemind == NULL)
        return 0;

    memset(p_param_screenRemind,0,sizeof(param_screenRemind_obj_t));

    ui_ScreenPhonebookRemind = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebookRemind,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebookRemind,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebookRemind,&p_param_screenRemind->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);


    memcpy(&p_param_screenRemind->param,param,sizeof(screen_remind_param_t));
    p_screen_remind_param = param;

    if (p_screen_remind_param->str) {
        lv_label_set_text((lv_obj_t *) p_param_screenRemind->obj_label,p_screen_remind_param->str);
        }


    lv_obj_set_user_data(ui_ScreenPhonebookRemind,p_param_screenRemind);

    lv_obj_add_event_cb(ui_ScreenPhonebookRemind,phonebook_fp_remind_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebookRemind);


    return ui_ScreenPhonebookRemind;
}



static void phonebook_remind_timer_stop(lv_obj_t * ui_ScreenPhonebooRemind)
{
    param_screenRemind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (param_screenRemind_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebooRemind);

    if (p_screen_param_obj->phonebook_remind_timer != NULL) {
        lv_timer_del(p_screen_param_obj->phonebook_remind_timer);
        p_screen_param_obj->phonebook_remind_timer = NULL;
        }

}


extern lv_obj_t * ui_ScreenPhonebook;


static void phonebook_remind_timer_event_cb(struct _lv_timer_t *timer)
{
    screen_remind_param_t * p_param;

    p_param             = timer->user_data;

    if (p_param->fun)
        p_param->fun(p_param->user_data);


}


void phonebook_remind_timer_start(lv_obj_t * ui_ScreenPhonebooRemind)
{
    param_screenRemind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (param_screenRemind_obj_t *) lv_obj_get_user_data(ui_ScreenPhonebooRemind);
    uint32_t        delays;

    delays              = p_screen_param_obj->param.tiemr_delay;

    if (delays == 0)
        delays = 1500;


    if (p_screen_param_obj->phonebook_remind_timer == NULL) {
        p_screen_param_obj->phonebook_remind_timer = lv_timer_create(phonebook_remind_timer_event_cb,delays,
            &p_screen_param_obj->param);

        //lv_timer_set_repeat_count(p_screen_param_obj->phonebook_remind_timer, 1);
        }

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    param_screenRemind_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (param_screenRemind_obj_t *) lv_obj_get_user_data(a->var);
    phonebook_remind_timer_stop(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebookRemind_screen_uninit(void)
{
    if (ui_ScreenPhonebookRemind) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebookRemind,200,lv_obj_del_screen_ready_cb);
        }

    ui_ScreenPhonebookRemind = NULL;
}




