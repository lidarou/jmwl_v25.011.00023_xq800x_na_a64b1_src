

#ifdef CONFIG_APP_CALL_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_CALL_FP_LIST_H__
#define __APP_CALL_FP_LIST_H__
#include "./app_240x320/ui.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define     CALL_FP_APP_NAME        "call"

    int32_t call_dial_number(uint8_t * number,uint8_t * name);
    uint8_t call_app_key_huangup_get();
    void call_app_key_huangup_add();


#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __APP_CALL_FP_LIST_H__ */

#endif

