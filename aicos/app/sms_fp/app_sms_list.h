

#ifdef CONFIG_APP_SMS_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_APP_SMS_FP_LIST_H__
#define __APP_APP_SMS_FP_LIST_H__
#include "./app_sms_240x320/ui.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define     SMS_FP_APP_NAME         "sms"

    void app_sms_fp_init(void);
    void destory_app_sms_enter_param(void * p);
    void * create_app_sms_enter_param_fisrt_write_win(uint8_t * p_context,uint8_t * p_number);
    void * create_app_sms_enter_param_fisrt_rev_win(void);

    typedef enum {
        SMS_APP_FIRST_WIN_WRITE = 1,
        SMS_APP_FIRST_WIN_REV,
        } sms_app_frist_win_enum_t;


    typedef union {
        sms_write_ui_enter_param_t write;
        } sms_win_enter_param_t;


    typedef struct {
        sms_win_enter_param_t win_enter_param;
        sms_app_frist_win_enum_t first_win;
        } sms_app_enter_param_t;


#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __APP_APP_SMS_FP_LIST_H__ */

#endif

