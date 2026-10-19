/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of fresh statusbar ui.
 */

#ifndef __STATUSBAR_UI_H__
#define __STATUSBAR_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "statusbar_flow.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    STATUSBAR_TYPE_ALL,
    STATUSBAR_TYPE_MAIN_WIN,
    STATUSBAR_TYPE_SUB_WIN,
    STATUSBAR_TYPE_MAX
}statusbar_type_e;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * statusbar_ui_init
 * @arg1: type:statusbar type
 * @arg2: win_name:win name
 * Return: none.
 */
void statusbar_ui_init(statusbar_type_e type, char *win_name);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __STATUSBAR_UI_H__ */
