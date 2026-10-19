/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of factoryreset.
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "alv_app_mgr.h"
#include "factoryreset.h"
#include "app_log.h"


/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *   STATIC FUNCTIONS
 **********************/
static void mgsbox_deep_factoryreset_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code =lv_event_get_code(p_event);
    lv_obj_t *p_btn =lv_event_get_target(p_event);
    lv_obj_t *p_msg_box =lv_obj_get_parent(p_btn);

    if(LV_EVENT_VALUE_CHANGED==code){
        const char * txt =lv_msgbox_get_active_btn_text(p_msg_box);

        if(0==strncmp(txt,"YES",strlen("YES"))){

            lv_msgbox_close(p_msg_box);
            factoryreset_nv();
            /* TODO : factoryreset_userdata();*/
            APP_LOGI("Deep factoryreset succeed");
            alv_am_win_exit(NULL,"factoryreset", NULL);
        } else if (0== strncmp(txt,"NO",strlen("NO"))){

            lv_msgbox_close(p_msg_box);
            APP_LOGI("Deep factoryreset unexecuted");
            alv_am_win_exit(NULL,"factoryreset", NULL);
        }
    }
}

static void mgsbox_shallow_factoryreset_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code =lv_event_get_code(p_event);
    lv_obj_t *p_btn =lv_event_get_target(p_event);
    lv_obj_t *p_msg_box =lv_obj_get_parent(p_btn);

    if(code==LV_EVENT_VALUE_CHANGED){
        const char * txt =lv_msgbox_get_active_btn_text(p_msg_box);

        if(0==strncmp(txt,"YES",strlen("YES"))){

            lv_msgbox_close(p_msg_box);
            factoryreset_nv();
            APP_LOGI("Shallow factoryreset succeed");
            alv_am_win_exit(NULL,"factoryreset", NULL);
        } else if (0== strncmp(txt,"NO",strlen("NO"))){

            lv_msgbox_close(p_msg_box);
            APP_LOGI("Shallow factoryreset unexecuted");
            alv_am_win_exit(NULL,"factoryreset", NULL);
        }
    }
}

static void deep_factoryreset_event_cb(void)
{
    static const char * deep_factoryreset_btns[]={"YES","NO",""};
    lv_obj_t * mbox_deep_factoryreset =lv_msgbox_create(NULL,NULL,"Factory reset imminent. \r\n Whether to continue?",deep_factoryreset_btns,false);

    lv_group_focus_obj(lv_msgbox_get_btns(mbox_deep_factoryreset));
    lv_obj_add_event_cb(mbox_deep_factoryreset,mgsbox_deep_factoryreset_event_cb,LV_EVENT_ALL,NULL);
    lv_obj_center(mbox_deep_factoryreset);
}

static void shallow_factoryreset_event_cb(void)
{
    static const char * shallow_factoryreset_btns[]={"YES","NO",""};
    lv_obj_t * mbox_shallow_factoryreset =lv_msgbox_create(NULL,NULL,"Factory reset imminent. \r\n Whether to continue?",shallow_factoryreset_btns,false);

    lv_group_focus_obj(lv_msgbox_get_btns(mbox_shallow_factoryreset));
    lv_obj_add_event_cb(mbox_shallow_factoryreset,mgsbox_shallow_factoryreset_event_cb,LV_EVENT_ALL,NULL);
    lv_obj_center(mbox_shallow_factoryreset);
}

static void mgsbox_factoryreset_event_cb(lv_event_t * p_event)
{
    lv_event_code_t code =lv_event_get_code(p_event);
    lv_obj_t *p_btn =lv_event_get_target(p_event);
    lv_obj_t *p_msg_box =lv_obj_get_parent(p_btn);

    if(LV_EVENT_VALUE_CHANGED==code){
        const char * txt =lv_msgbox_get_active_btn_text(p_msg_box);

        if(0==strncmp(txt,"Deep",strlen("Deep"))){

            lv_msgbox_close(p_msg_box);
            deep_factoryreset_event_cb();
        } else if (0== strncmp(txt,"Shallow",strlen("Shallow"))){

            lv_msgbox_close(p_msg_box);
            shallow_factoryreset_event_cb();
        }
        else if (0==strncmp(txt,"Cancel",strlen("Cancel"))){

            lv_msgbox_close(p_msg_box);
            alv_am_win_exit(NULL,"factoryreset", NULL);
            APP_LOGI("Factoryreset cancel");
        }
    }
}

/**********************
 *  GLOBAL FUNCTIONS
 **********************/
/**
 * factoryreset_active() - the active of factoryreset.
 * @arg1:
 * @arg2:
 * Return:
 */
void factoryreset_active(lv_obj_t *win, void *p_param)
{
    static const char * factoryreset_btns[]={"Deep","Shallow","Cancel",""};
    lv_obj_t * mbox_factoryreset =lv_msgbox_create(NULL,NULL,"Please choose the method of factoryreset",factoryreset_btns,false);

    lv_group_focus_obj(lv_msgbox_get_btns(mbox_factoryreset));
    lv_obj_add_event_cb(mbox_factoryreset,mgsbox_factoryreset_event_cb,LV_EVENT_ALL,NULL);
    lv_obj_center(mbox_factoryreset);
}
