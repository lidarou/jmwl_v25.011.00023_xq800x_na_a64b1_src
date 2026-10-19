/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle flow.
 */
#ifndef __IDLE_FLOW_H__
#define __IDLE_FLOW_H__

#ifdef __cplusplus
    extern "C" {
#endif

/**
 * idle_flow_get_datetime.
 * @arg1:p_date:date value.
 * @arg2:p_time:time value.
 * Return: none.
 */
void idle_flow_get_datetime(char *p_date, char *p_time);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __IDLE_FLOW_H__ */

