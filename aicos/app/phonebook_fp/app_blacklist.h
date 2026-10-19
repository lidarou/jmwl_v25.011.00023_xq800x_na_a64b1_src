

#ifdef CONFIG_APP_PHONEBOOK_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_BLACKLIST_LIST_H__
#define __APP_BLACKLIST_LIST_H__
#include "./app_phonebook_240_320/ui.h"
#include "blacklist.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define     BACKLIST_FP_APP_NAME    "blacklist"

    void app_blacklist_fp_init(void);
    void destory_app_blacklist_enter_param(void * p,void * exit_fun);
    void * create_app_blacklist_enter_param_fisrt_add_num(uint8_t * p_number,void * exit_fun);
    void * create_app_blacklist_enter_param_fisrt_del_num(blacklist_data_t * p_blacklist_data,uint32_t focus_index,
        void * exit_fun);

    typedef void(*exit_fun_t) (void);

    typedef struct UI_SCREEN_BLACKLIST_ENTER_PARAM_H {
        blacklist_data_t * p_blacklist_data;
        uint32_t        focus_index;
        exit_fun_t      exit_fun;                   //但添加，删除黑名单成功后，退出界面时函数时调用此函数
        } ui_screen_blacklist_enter_param_t;


    typedef struct UI_SCREEN_BLACKLIST_ADD_NUM_ENTER_PARAM_H {
        uint8_t *       p_number;
        exit_fun_t      exit_fun;                   //但添加黑名单成功后，退出界面时函数时调用此函数
        } ui_screen_blacklist_add_num_enter_param_t;


    typedef struct UI_SCREEN_BLACKLIST_FIND_NUM_ENTER_PARAM_H {
        uint8_t *       p_number;
        uint32_t        number_size;
        } ui_screen_blacklist_find_num_enter_param_t;


#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __APP_BLACKLIST_LIST_H__ */

#endif

