/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/


/**
 *  DESCRIPTION
 *
 *    This file defines the apis of settings app.
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "settings_ui.h"
#include "settings_flow.h"


/**
 * settings_init() - the init of settings app.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
#ifdef CONFIG_APP_SETTINGS
static void settings_init(void)
{
    settings_ui_init();
}
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __SETTINGS_H__ */

