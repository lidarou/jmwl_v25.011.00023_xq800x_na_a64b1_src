/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "statusbar_flow.h"
#include "statusbar_ui.h"

/**********************
 *   STATIC VARIABLES
 **********************/


/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *   STATIC FUNCTIONS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * comp_statusbar_show(status_type_e, char *) - show status bar.
 * @arg1: type:the type of statusbar that needs to be displayed.
 * @arg2: win_name:the win name of statusbar that needs to be displayed. NULL : to hidden the win name.
 * Return: none.
 */
void comp_statusbar_show(statusbar_type_e type, char *win_name)
{
    /* init ui position */
    statusbar_ui_init(type, win_name);
    /* set each item value */
    statusbar_flow_set_latest_value();
}

/**
 * comp_statusbar_init
 * @arg1: none
 * @arg2: none.
 * Return: none.
 */
void comp_statusbar_init(void)
{
    statusbar_flow_init();
}
