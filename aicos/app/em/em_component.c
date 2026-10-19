/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is enter of engineering mode framework
*/

/*********************
 *      INCLUDES
*********************/
#include "aic_osa_ex.h"
#include "em_component.h"
#include "alv_app_mgr.h"


/**********************
 *      TYPEDEFS
 **********************/


/*********************
 *      DEFINES
*********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/*类型四的特性：按button执行，执行结果异步回来，回来后显示在屏幕上;*/


/**********************
 *   STATIC FUNCTIONS
**********************/
static void em_comp_act_btn_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    em_comp_act_display_param_t  *p_btn_cb_param = lv_event_get_user_data(p_event);

    if ( LV_EVENT_CLICKED == code ) {
        if(p_btn_cb_param) {
           /*TODO: 获取cb的参数，并将参数传入到btn cb中*/
           p_btn_cb_param->act_cb();
        }
    }
}


/**********************
 *   GLOBAL FUNCTIONS
**********************/
lv_obj_t *em_comp_act_display_create(lv_obj_t *p_parent,char *p_act_str,char *p_display_str,em_comp_act_display_param_t *p_param)
{
    lv_obj_t *p_act_btn = NULL;
    lv_obj_t *p_btn_label = NULL;

    lv_obj_t *p_display_label = NULL;

    if(NULL == p_param) {
#ifndef WIN32
        app_logi("[%s],param error.",__func__);
#endif
        return NULL;
    }

    p_act_btn = lv_btn_create(p_parent);
    p_btn_label  = lv_label_create(p_act_btn);
    lv_label_set_text(p_btn_label,p_act_str);

    p_display_label = lv_label_create(p_parent);
    lv_label_set_text(p_display_label,p_display_str);
    /*TODO: later to delete*/
    lv_obj_align_to(p_display_label, p_act_btn, LV_ALIGN_OUT_BOTTOM_MID, 20, 10);

#if LV_USE_USER_DATA
    lv_obj_set_user_data(p_act_btn,p_display_label);
#endif

    lv_obj_add_event_cb(p_act_btn, em_comp_act_btn_event_cb, LV_EVENT_CLICKED,(em_comp_act_display_param_t*)p_param);

    return (lv_obj_t *)p_act_btn;
}

/*fresh result:根据btn 找到label*/
void em_comp_act_display_fresh_result(lv_obj_t *p_obj,char *p_display_str)
{
    lv_obj_t *p_temp_label = NULL;

    if(NULL == p_obj)
    {
#ifndef WIN32
        app_logi("[%s],p_obj is null.",__func__);
#endif
        return;
    } else {
#if LV_USE_USER_DATA
        p_temp_label = lv_obj_get_user_data(p_obj);
#endif
    }
    if(NULL == p_temp_label)
    {
#ifndef WIN32
        app_logi("[%s],p_temp_label is null.",__func__);
#endif
        return;
    }

    if(NULL != p_display_str) {
        lv_label_set_text(p_temp_label,p_display_str);
    }else {
#ifndef WIN32
        app_logi("[%s],display string is null.",__func__);
#endif
    }
}
