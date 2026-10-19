

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
extern "C"
{
#endif

#include "settings_ui.h"
#include "settings_fp_flow.h"


    /**
     * settings_fp_init() - the init of settings app.
     * @arg1: none.
     * @arg2: none.
     * Return: none.
     */
#ifdef CONFIG_APP_SETTINGS_FP
    void settings_fp_init(void) {
        settings_ui_init();
        }
#endif

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __SETTINGS_H__ */

