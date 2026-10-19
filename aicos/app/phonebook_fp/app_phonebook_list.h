

#ifdef CONFIG_APP_PHONEBOOK_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_PHONEBOOK_LIST_H__
#define __APP_PHONEBOOK_LIST_H__
#include "./app_phonebook_240_320/ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define     PHONEBOOK_FP_APP_NAME   "phonebook"


    void app_phonebook_fp_init(void);
    void destory_app_phonebook_enter_param(void * p);
    void * create_app_phonebook_enter_param_fisrt_write_win(uint8_t * name,uint8_t * p_number);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __IDLE_H__ */

#endif

