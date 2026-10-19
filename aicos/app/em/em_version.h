/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#ifdef WIN32
#include "../../components/graphics/lvgl/lvgl.h"
#else
#include <lvgl.h>
#endif

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of engineering version.
 */

/**
 * em_version_set_focus() - set focus of em version.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void em_version_set_focus(void);

/**
 * em_version_enter_win() - enter em version window.
 * @arg1: the lvgl menu object.
 * @arg2: the lvgl subpage object.
 * Return: none.
 */
lv_obj_t *em_version_enter_win( lv_obj_t *p_menu );

