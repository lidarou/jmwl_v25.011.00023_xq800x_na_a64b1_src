

#ifdef CONFIG_APP_CALLLOG_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_CALLLOG_FP_LIST_H__
#define __APP_CALLLOG_FP_LIST_H__
#include "./app_240x320/ui.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define     CALLLOG_FP_APP_NAME     "calllog"

    void * create_app_calllog_enter_param_fisrt_dial_win(uint8_t call_type);
    void destory_app_calllog_enter_param(void * p);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __APP_CALLLOG_FP_LIST_H__ */

#endif

