/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is enter of engineering mode wifi display
*/

/*********************
 *      INCLUDES
*********************/
#include "lvgl.h"
#include "em_component.h"

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *   STATIC VARIABLES
**********************/
static lv_obj_t *s_comp_wifiscan = NULL;


/**********************
 *   STATIC FUNCTIONS
**********************/
/*TODO: demo code begin,异步结果调用接口em_comp_act_display_fresh_result，将结果呈现出来*/
static void em_test_btn_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);

    if ( LV_EVENT_CLICKED == code ) {
        char test_string[20] = "AA:BB:CC:DD";
        em_comp_act_display_fresh_result(s_comp_wifiscan, test_string);
    }
}
/*TODO: demo code end,异步结果调用接口em_comp_act_display_fresh_result，将结果呈现出来*/

static void em_wifiscan_start(void)
{
    /*TODO:添加业务逻辑 */

    /*TODO:返回的结果,使用接口em_comp_act_display_fresh_result(s_start_btn, test_string);将结果显示出来*/
}

/**********************
 *   GLOBAL FUNCTIONS
**********************/
/*窗口实现*/
void em_wifiscan_active(lv_obj_t *p_parent, void *p_param)
{
    em_comp_act_display_param_t  p_start_btn_param = {0};


    p_start_btn_param.act_cb = em_wifiscan_start;
    char *p_btn_str = "Start";
    char *p_dis_str = "Search Result:";


    s_comp_wifiscan = em_comp_act_display_create(p_parent,p_btn_str,p_dis_str,&p_start_btn_param);

    /*TODO:demo code begin,delete later */
    lv_obj_t *p_test_btn =  NULL;
    p_test_btn = lv_btn_create(p_parent);
    lv_obj_set_align(p_test_btn, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_add_event_cb(p_test_btn,em_test_btn_event_cb,LV_EVENT_CLICKED,NULL);

    lv_group_focus_obj(p_test_btn);
    /*TODO:demo code end,delete later */
}

