

#ifdef CONFIG_APP_REMIND_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_PROMPT_BOX_COMMON_H__
#define __APP_PROMPT_BOX_COMMON_H__
#include "./app_240x320/ui.h"

#ifdef __cplusplus
extern "C"
{
#endif



    typedef struct {
        uint32_t        timeout;                    /* 自动消失时间，如果timeout = 0 不会消失消失 */
        char *          title;                      /* the title of the message box */
        char *          content;                    /* the content of the message box */
        char *          btn_left_txt;               /* the text of the left button */
        char *          btn_right_txt;              /* the text of the right button, set NULL if only one button */
        lv_event_cb_t   _btn_left_callback;         /* the callback corresponding to the left button and 左上按键 
            */
        lv_event_cb_t   _btn_right_callback;        /* the callback corresponding to the right button and 右上按键
             */
        void *          user_data;                  /* user data that callback functions are concerned about */
        } prompt_box_common_info_t;


    int32_t prompt_box_common_create(lv_obj_t * parent,prompt_box_common_info_t * p_prompt_box_common_info);


#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __IDLE_H__ */

#endif

