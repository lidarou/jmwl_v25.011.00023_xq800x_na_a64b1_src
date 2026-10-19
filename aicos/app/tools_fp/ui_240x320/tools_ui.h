

/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of tools app menu.
 */
#ifndef __TOOLS_UI_H__
#define __TOOLS_UI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    /**
     * tools_ui_init() - the init of tools app wins.
     * @arg1: none.
     * @arg2: none.
     * Return: none.
     */
    void active_recent_alarm(unsigned char type);   //同时检查闹钟和备忘录，把最近的一个闹钟设置为有效
    void tools_ui_init(void);
    void reset_power_on_alarm_id(void);
    void active_alarm_delay(uint8_t type);
    void active_calendar_delay(uint8_t type);
    extern unsigned char alarm_doing_number;

    //extern aos_systime copy_alarm_time;
    extern signed char alarm_para;
    extern signed char copy_alarm_para;
    extern unsigned char shortcut_in;

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __TOOLS_UI_H__ */

