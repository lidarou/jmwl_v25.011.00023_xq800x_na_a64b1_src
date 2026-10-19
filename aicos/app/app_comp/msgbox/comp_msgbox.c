/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file comp_msgbox.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "comp_msgbox.h"

/*********************
 *      DEFINES
 *********************/
#define COMP_MSGBOX_WIDTH (80)
#define COMP_MSGBOX_HEIGHT (44)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    char *btns[3];
    lv_event_cb_t _btn_left_callback;
    lv_event_cb_t _btn_right_callback;
}comp_msg_btnsinfo_t;
typedef struct {
    lv_obj_t *msgbox;
    lv_timer_t *timer;
    lv_obj_t *p_last_obj;
}comp_msg_timer_info_t;

/**********************
 *   STATIC VARIABLES
 **********************/
static comp_msg_timer_info_t *g_msg_timer_info = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void delete_msgbox_with_timer(void)
{
    lv_timer_del(g_msg_timer_info->timer);
    lv_msgbox_close(g_msg_timer_info->msgbox);
    lv_obj_t * prev_scr = lv_obj_get_screen(g_msg_timer_info->p_last_obj);
    if(lv_scr_act() == prev_scr) {
        lv_group_focus_obj(g_msg_timer_info->p_last_obj);
    } else {
        LV_LOG_WARN("the act screen is changed ");
        lv_obj_set_user_data(prev_scr, g_msg_timer_info->p_last_obj);
    }
    lv_mem_free(g_msg_timer_info);
    g_msg_timer_info = NULL;
    LV_LOG_INFO("delete msgbox!");
}
static void comp_msg_btns_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_current_target(e);
    comp_msg_btnsinfo_t * msg_btnsinfo = (comp_msg_btnsinfo_t * )lv_obj_get_user_data(target);

    if(code == LV_EVENT_KEY && lv_event_get_key(e) == LV_KEY_BACKSPACE) {
        if(NULL != msg_btnsinfo->_btn_right_callback) {
            lv_indev_wait_release(lv_indev_get_act());
            msg_btnsinfo->_btn_right_callback(e);
            LV_LOG_USER("[%s] BACKSPACE is pressed", __func__);
        }else {
            LV_LOG_USER("[%s] BACKSPACE is pressed, _btn_right_callback is null!!!", __func__);
        }
        lv_mem_free(msg_btnsinfo);
        msg_btnsinfo = NULL;
        lv_msgbox_close(target);
    }
    if(code == LV_EVENT_KEY && lv_event_get_key(e) == LV_KEY_HOME) {
        lv_event_send(target, LV_EVENT_VALUE_CHANGED, NULL);
    }
    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t id = lv_btnmatrix_get_selected_btn(lv_msgbox_get_btns(target));
        lv_indev_wait_release(lv_indev_get_act());
        if(id == 0) {
            if(NULL != msg_btnsinfo->_btn_left_callback) {
                msg_btnsinfo->_btn_left_callback(e);
                LV_LOG_USER("[%s] left btn is pressed", __func__);
            }else {
                LV_LOG_ERROR("[%s] _btn_left_callback is null!!!", __func__);
            }
        } else if (id == 1) {
            if(NULL != msg_btnsinfo->_btn_right_callback) {
                msg_btnsinfo->_btn_right_callback(e);
                LV_LOG_USER("[%s] right btn is pressed", __func__);
            }else {
                LV_LOG_USER("[%s] _btn_right_callback is null!!!", __func__);
            }
        }
        lv_mem_free(msg_btnsinfo);
        msg_btnsinfo = NULL;
        lv_msgbox_close(target);
    }
}
static void comp_msg_event_cb(lv_event_t *e)
{
    lv_indev_wait_release(lv_indev_get_act());
    delete_msgbox_with_timer();
}
static void comp_msg_timer_handle(lv_timer_t * t)
{
    delete_msgbox_with_timer();
}
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int comp_msgbox_create(lv_obj_t * parent, comp_msgbox_info_t *p_msgbox_info)
{
    lv_obj_t *p_msgbox = NULL;
    comp_msg_btnsinfo_t * p_msg_btnsinfo = NULL;
    lv_obj_t *p_last_obj = NULL;

    /*Paream is NULL*/
    if(NULL == p_msgbox_info) {
        LV_LOG_ERROR("p_msgbox_info is null");
        return -1;
    }

    /*printf msgbox info*/
    LV_LOG_INFO("p_msgbox_info->btn_count = %d, p_msgbox_info->timeout = %ld", p_msgbox_info->btn_count, p_msgbox_info->timeout);
    LV_LOG_INFO("p_msgbox_info->title = %s, p_msgbox_info->content = %s", p_msgbox_info->title, p_msgbox_info->content);
    LV_LOG_INFO("p_msgbox_info->btn_left_txt = %s, p_msgbox_info->btn_right_txt = %s", p_msgbox_info->btn_left_txt, p_msgbox_info->btn_right_txt);

    /*Delete the msgbox with timer*/
    if(NULL != g_msg_timer_info) {
        delete_msgbox_with_timer();
    }

    if(0 == p_msgbox_info->btn_count) {
        p_msgbox = lv_msgbox_create(parent, p_msgbox_info->title, p_msgbox_info->content, NULL, false);
        /*Save the current focus infomation and Focus msgbox*/
        p_last_obj = lv_group_get_focused(lv_group_get_default());
        lv_group_add_obj(lv_group_get_default(), p_msgbox);
        lv_group_focus_obj(p_msgbox);
        lv_obj_add_event_cb(p_msgbox, comp_msg_event_cb, LV_EVENT_KEY, NULL);
    } else {
        /*Transfer btns callback function and user data*/
        p_msg_btnsinfo = (comp_msg_btnsinfo_t *)lv_mem_alloc(sizeof(comp_msg_btnsinfo_t));
        memset(p_msg_btnsinfo, 0, sizeof(comp_msg_btnsinfo_t));
        if(NULL != p_msgbox_info->btn_left_txt) {
            p_msg_btnsinfo->btns[0] = p_msgbox_info->btn_left_txt;
            p_msg_btnsinfo->_btn_left_callback = p_msgbox_info->_btn_left_callback;
        }
        if(NULL != p_msgbox_info->btn_right_txt) {
            p_msg_btnsinfo->btns[1] = p_msgbox_info->btn_right_txt;
            p_msg_btnsinfo->_btn_right_callback = p_msgbox_info->_btn_right_callback;
        }
        p_msgbox = lv_msgbox_create(parent, p_msgbox_info->title, p_msgbox_info->content, (const char **)p_msg_btnsinfo->btns, false);
        lv_obj_set_user_data(p_msgbox, (void *)p_msg_btnsinfo);
        lv_obj_add_event_cb(p_msgbox, comp_msg_btns_event_cb, LV_EVENT_ALL, p_msgbox_info->user_data);

        /*Focus the btns of msgbox*/
        lv_group_focus_obj(lv_msgbox_get_btns(p_msgbox));
    }

    if(0 != p_msgbox_info->timeout) {
        /*Create timer and save related infomation*/
        g_msg_timer_info = (comp_msg_timer_info_t *)lv_mem_alloc(sizeof(comp_msg_timer_info_t));
        memset(g_msg_timer_info, 0, sizeof(comp_msg_timer_info_t));
        g_msg_timer_info->msgbox = p_msgbox;
        g_msg_timer_info->p_last_obj = p_last_obj;
        g_msg_timer_info->timer = lv_timer_create(comp_msg_timer_handle,p_msgbox_info->timeout, NULL);
    }

    /*Set style of msgbox*/
    lv_obj_set_height(p_msgbox, lv_pct(COMP_MSGBOX_HEIGHT));
    lv_obj_set_width(p_msgbox, lv_pct(COMP_MSGBOX_WIDTH));
    lv_obj_center(p_msgbox);
    lv_obj_set_style_bg_color(p_msgbox, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(p_msgbox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(p_msgbox, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    /*Set style of title*/
    if(NULL != p_msgbox_info->title) {
        lv_obj_set_style_text_font(lv_msgbox_get_title(p_msgbox), &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(lv_msgbox_get_title(p_msgbox), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(lv_msgbox_get_title(p_msgbox), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    /*Set style of content*/
    if(NULL != p_msgbox_info->content) {
        lv_obj_set_style_text_align(lv_msgbox_get_text(p_msgbox), LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(lv_msgbox_get_text(p_msgbox), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(lv_msgbox_get_text(p_msgbox), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    /*Set style of btnmatrix*/
    if(NULL != p_msgbox_info->btn_left_txt) {
        lv_obj_set_style_bg_color(lv_msgbox_get_btns(p_msgbox), lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_msgbox_get_btns(p_msgbox), 255, LV_PART_ITEMS | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(lv_msgbox_get_btns(p_msgbox), lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_FOCUS_KEY);
        lv_obj_set_style_bg_opa(lv_msgbox_get_btns(p_msgbox), 255, LV_PART_ITEMS | LV_STATE_FOCUS_KEY);

        lv_obj_set_style_text_color(lv_msgbox_get_btns(p_msgbox), lv_color_hex(0x000000), LV_PART_ITEMS | LV_STATE_FOCUS_KEY);
        lv_obj_set_style_text_opa(lv_msgbox_get_btns(p_msgbox), 255, LV_PART_ITEMS | LV_STATE_FOCUS_KEY);

        lv_obj_set_style_outline_color(lv_msgbox_get_btns(p_msgbox), lv_color_hex(0x0050EF), LV_PART_ITEMS | LV_STATE_FOCUS_KEY);
        lv_obj_set_style_outline_opa(lv_msgbox_get_btns(p_msgbox), 255, LV_PART_ITEMS | LV_STATE_FOCUS_KEY);
        lv_obj_set_style_outline_width(lv_msgbox_get_btns(p_msgbox), 2, LV_PART_ITEMS | LV_STATE_FOCUS_KEY);

        lv_obj_set_style_shadow_width(lv_msgbox_get_btns(p_msgbox), 0, LV_PART_ITEMS | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_spread(lv_msgbox_get_btns(p_msgbox), 0, LV_PART_ITEMS | LV_STATE_DEFAULT);

        lv_obj_set_width(lv_msgbox_get_btns(p_msgbox), lv_pct(100));
    }
    return 0;
}

