

/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#ifdef CONFIG_APP_HOME_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __HOME_FP_H__
#define __HOME_FP_H__
#include "./app_home_240x320/ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define     HOME_FP_APP_NAME        "home_fp"
#define     HOME_FP_WIN_NAME_PHONEBOOK "phonebook"
#define     HOME_FP_WIN_NAME_CALLLOG "calllog"
#define     HOME_FP_WIN_NAME_MESSAGE "message"
#define     HOME_FP_WIN_NAME_MULTIMEDIA "multimedia"
#define     HOME_FP_WIN_NAME_TOOLKIT "toolkit"
#define     HOME_FP_WIN_NAME_SETTING "setting"

    void home_fp_init(void);
    void home_app_win_uninit(lv_obj_t * cur_win);
    void home_app_win_key_action(uint32_t key,void * param);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __HOME_FP_H__ */

#endif

