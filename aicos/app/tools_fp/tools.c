

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
#ifndef __TOOLS_H__
#define __TOOLS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tools_ui.h"
#include "tools_flow.h"


    /**
     * settings_fp_init() - the init of settings app.
     * @arg1: none.
     * @arg2: none.
     * Return: none.
     */
#ifdef CONFIG_APP_TOOLS_FP
    void tools_fp_init(void) {
        tools_ui_init();
        }
#endif

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __SETTINGS_H__ */

