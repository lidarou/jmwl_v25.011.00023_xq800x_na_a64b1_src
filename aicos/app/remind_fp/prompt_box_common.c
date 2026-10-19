
#include "lvgl.h"

#include "alv_app_mgr.h"
#include "backlight.h"

#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "prompt_box_common.h"
#include "app_log.h"
#include "obj_batch_deal.h"

#if 1
#define __inf                   APP_LOGI

#else

#define __inf
#endif



typedef struct {
    lv_obj_t *      promptbox;
    lv_timer_t *    timer;
    lv_obj_t *      p_last_obj;
} comp_prompt_timer_info_t;


static comp_prompt_timer_info_t * g_comp_prompt_timer_info = NULL;


/**********************
 *   STATIC VARIABLES
 **********************/
static void delete_prompt_box_with_timer(comp_prompt_timer_info_t * p_prompt_timer_info)
{
    if (p_prompt_timer_info == NULL)
        return;

    lv_timer_del(p_prompt_timer_info->timer);
    p_prompt_timer_info->timer = NULL;
    lv_obj_set_user_data(p_prompt_timer_info->promptbox,NULL);
    lv_obj_del(p_prompt_timer_info->promptbox);
    lv_obj_t *      prev_scr = lv_obj_get_screen(p_prompt_timer_info->p_last_obj);

    if (lv_scr_act() == prev_scr) {
        lv_group_focus_obj(p_prompt_timer_info->p_last_obj);
        }
    else {
        LV_LOG_WARN("the act screen is changed ");

        //lv_obj_set_user_data(prev_scr, p_prompt_timer_info->p_last_obj);
        }

    lv_mem_free(p_prompt_timer_info);
    __inf("delete msgbox!");

    g_comp_prompt_timer_info = NULL;
}


static void prompt_box_common_timer_handle(lv_timer_t * t)
{
    delete_prompt_box_with_timer(t->user_data);

}


static void prompt_box_common_timer_destroy(lv_obj_t * p_promptbox)
{
    comp_prompt_timer_info_t * p_prompt_timer_info = NULL;

    p_prompt_timer_info = lv_obj_get_user_data(p_promptbox);

    if (p_prompt_timer_info == NULL)
        return;

    if (p_prompt_timer_info->timer)
        lv_timer_del(p_prompt_timer_info->timer);

    lv_mem_free(p_prompt_timer_info);
    g_comp_prompt_timer_info = NULL;
    lv_obj_set_user_data(p_promptbox,NULL);
    __inf("prompt_box_common_timer_destroy");
}


static void prompt_box_common_event_cb(lv_event_t * e)
{
    lv_obj_t *      obj = e->user_data;

    if (obj) {
        prompt_box_common_timer_destroy(obj);
        }

    __inf("prompt_box_common_event_cb");
}


static void prompt_box_common_key_event_cb(lv_event_t * e)
{
    lv_obj_t *      obj = e->user_data;

    lv_indev_wait_release(lv_indev_get_act());

    if (obj) {
        if (lv_obj_get_user_data(obj))
            delete_prompt_box_with_timer(lv_obj_get_user_data(obj));
        else 
            lv_obj_del(obj);
        }
}


int32_t prompt_box_common_create(lv_obj_t * parent,prompt_box_common_info_t * p_prompt_box_common_info)
{
    lv_obj_t *      p_promptbox,*label;
    lv_obj_t *      p_last_obj = NULL;
    comp_prompt_timer_info_t * p_prompt_timer_info = NULL;

    if (g_comp_prompt_timer_info) {
        delete_prompt_box_with_timer(g_comp_prompt_timer_info);
        }

    p_promptbox         = lv_obj_create(parent);
    lv_obj_clear_flag(p_promptbox,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(p_promptbox,(lv_style_t *) get_lv_style_font_common(),0);
    lv_obj_set_size(p_promptbox,lv_pct(80),lv_pct(50));
    lv_obj_center(p_promptbox);
    lv_obj_set_style_bg_color(p_promptbox,lv_color_hex(0xFFFFFF),LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(p_promptbox,255,LV_PART_MAIN | LV_STATE_DEFAULT);


    label               = lv_label_create(p_promptbox);
    lv_label_set_text(label,p_prompt_box_common_info->content);
    lv_obj_set_width(label,lv_pct(100));            /// 1
    lv_obj_set_style_border_side(label,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(label,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(label,LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(label,LV_TEXT_ALIGN_CENTER,0);

    p_last_obj          = lv_group_get_focused(lv_group_get_default());
    lv_group_add_obj(lv_group_get_default(),p_promptbox);
    lv_group_focus_obj(p_promptbox);

    lv_obj_add_event_cb(p_promptbox,prompt_box_common_key_event_cb,LV_EVENT_KEY,p_promptbox);
    lv_obj_add_event_cb(p_promptbox,prompt_box_common_event_cb,LV_EVENT_DELETE,p_promptbox);


    if (0 != p_prompt_box_common_info->timeout) {
        /*Create timer and save related infomation*/
        p_prompt_timer_info = (comp_prompt_timer_info_t *) lv_mem_alloc(sizeof(comp_prompt_timer_info_t));
        memset(p_prompt_timer_info,0,sizeof(comp_prompt_timer_info_t));
        p_prompt_timer_info->promptbox = p_promptbox;
        p_prompt_timer_info->p_last_obj = p_last_obj;
        p_prompt_timer_info->timer = lv_timer_create(prompt_box_common_timer_handle,
            p_prompt_box_common_info->timeout,p_prompt_timer_info);
        lv_obj_set_user_data(p_promptbox,p_prompt_timer_info);
        g_comp_prompt_timer_info = p_prompt_timer_info;

        }



    return 0;
}


