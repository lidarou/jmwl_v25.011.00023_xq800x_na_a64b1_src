/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#include "idle_flow.h"
#include "./ui/app_idle/ui.h"


/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __IDLE_H__
#define __IDLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * idle_init() - the init of idle app.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
#ifdef CONFIG_APP_IDLE
static void idle_init(void)
{
    /* Create APP for Idle */
    lv_obj_t *app_idle = alv_am_app_create(lv_scr_act(), "idle");
    alv_am_app_set_active(app_idle, idle_app_active);
    alv_am_app_set_focus(app_idle, idle_app_focus);
    alv_am_app_set_defocus(app_idle, idle_app_defocus);
}
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __IDLE_H__ */

