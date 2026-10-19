

#ifdef CONFIG_APP_REMIND_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_APP_REMIND_FP_LIST_H__
#define __APP_APP_REMIND_FP_LIST_H__
#include "./app_240x320/ui.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define     REMIND_FP_APP_NAME      "remind"



    void remind_fp_win_exit_current_ui();
    void remind_fp_reset_call_count();
    void remind_fp_reset_sms_count();
    uint8_t remind_fp_get_call_count();
    uint8_t remind_fp_get_sms_count();
    int32_t box_bat_remind_show(uint8_t news_type,uint32_t count);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __APP_APP_REMIND_FP_LIST_H__ */

#endif

