/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file comp_msgbox.h
 *
 */

#ifndef __COMP_MSGBOX_H__
#define __COMP_MSGBOX_H__

#ifdef __cplusplus
extern "C" {
#endif
/*******************
 *      INCLUDES
 *******************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint8_t       btn_count;             /* 按钮个数，目前支持 0,1,2 */
    uint32_t      timeout;               /* 自动消失时间，如果timeout = 0 不会消失消失 */
    char          *title;                /* the title of the message box */
    char          *content;              /* the content of the message box */
    char          *btn_left_txt;         /* the text of the left button */
    char          *btn_right_txt;        /* the text of the right button, set NULL if only one button */
    lv_event_cb_t _btn_left_callback;    /* the callback corresponding to the left button and 左上按键 */
    lv_event_cb_t _btn_right_callback;   /* the callback corresponding to the right button and 右上按键 */
    void * user_data;                    /* user data that callback functions are concerned about */
}comp_msgbox_info_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * comp_msgbox_create.
 * @[IN] param parent  pointer       to parent or NULL to create a full screen modal message box.
 * @[IN] param comp_msgbox_t         the text of the message box.
 * @return                           if success return 0, else return other value.
 */
int comp_msgbox_create(lv_obj_t * parent, comp_msgbox_info_t *p_msgbox_info);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __COMP_MSGBOX_H__ */

