/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of statusbar.
 */

#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "statusbar_ui.h"
#include "statusbar_flow.h"
/*********************
 *      DEFINES
 *********************/
#ifdef CONFIG_COMP_STATUSBAR
#define SHOW_STATUSBAR comp_statusbar_show(STATUSBAR_TYPE_ALL, NULL)
#define HIDE_STATUSBAR
#define SHOW_STATUSBAR_MAIN_WIN(win_name) comp_statusbar_show(STATUSBAR_TYPE_MAIN_WIN, win_name)
#define HIDE_STATUSBAR_MAIN_WIN
#define SHOW_STATUSBAR_SUB_WIN(win_name) comp_statusbar_show(STATUSBAR_TYPE_SUB_WIN, win_name)
#define HIDE_STATUSBAR_SUB_WIN
#else
#define SHOW_STATUSBAR
#define HIDE_STATUSBAR
#define SHOW_STATUSBAR_MAIN_WIN(win_name)
#define HIDE_STATUSBAR_MAIN_WIN
#define SHOW_STATUSBAR_SUB_WIN(win_name)
#define HIDE_STATUSBAR_SUB_WIN
#endif


/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * comp_statusbar_init
 * @arg1: none
 * @arg2: none.
 * Return: none.
 */
void comp_statusbar_init(void);

/**
 * comp_statusbar_show(status_type_e, char *) - show status bar.
 * @arg1: type:the type of statusbar that needs to be displayed.
 * @arg2: win_name:the win name of statusbar that needs to be displayed. NULL : to hidden the win name.
 * Return: none.
 */
void comp_statusbar_show(statusbar_type_e type, char *win_name);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __STATUSBAR_H__ */


