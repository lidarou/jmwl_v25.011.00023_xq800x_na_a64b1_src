/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is enter of em framework
*/

#ifndef __EM_COMPONENT_H__
#define __EM_COMPONENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
*********************/
#include "lvgl.h"


/**********************
 *      TYPEDEFS
 **********************/
typedef struct{
    char     *p_display_str;           /*label display string*/

    void (*get_param)(void *p_param); /*when act ,get param from user*/
    void (*act_cb)(void);             /*act callback*/
}em_comp_act_display_param_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * em_comp_act_display_create() - the init of act & display component in em
 * @arg1: obj parent.
 * @arg2: btn display string
 * @arg3: label display string
 * Return: act & display param.
 */
lv_obj_t *em_comp_act_display_create(lv_obj_t *p_parent,char *p_act_str,char *p_display_str,em_comp_act_display_param_t *p_param);

/**
 * em_comp_act_display_create() - the init of act & display component in em
 * @arg1: btn.
 * @arg2: display string
 * Return: none.
 */
void em_comp_act_display_fresh_result(lv_obj_t *p_obj,char *p_result_str);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __EM_COMPONENT_H__ */

