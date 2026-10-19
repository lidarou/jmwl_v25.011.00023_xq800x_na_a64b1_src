/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file alv_app_mgr.h
 *
 */

#ifndef __ALV_APP_MGR_H__
#define __ALV_APP_MGR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "src/core/lv_obj.h"
#include "src/misc/lv_ll.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/* app */

/**
 * Create an app (base win) object, and its parent should be screan
 * @param parent    pointer to an object, it will be the parent of the new app
 * @return          pointer to the created application
 */
lv_obj_t *alv_am_app_create(lv_obj_t *parent, char *name);

/**
 * alv_am_app_enter - enter app
 * @app: [IN] app object
 * @name: [IN] app name
 * @p_param: [IN] app user data
 * @return: 0,success,-1,fail.
 */
int32_t alv_am_app_enter(lv_obj_t *app, char *name, void *p_param);
/**
 * alv_am_app_exit - exit app
 * @app: [IN] app object
 * @name: [IN] app name
 * @p_param: [IN] app user data
 * @return: 0,success,-1,fail.
 */
int32_t alv_am_app_exit(lv_obj_t *app, char *name, void *p_param);

/**
 * alv_am_app_set_first_win - set first win with obj of app
 * @app: [IN] app object
 * @win: [IN] win object
 * @return: 0,success,-1,fail.
 */
int32_t alv_am_app_set_first_win(lv_obj_t *app, lv_obj_t *win);

void alv_am_app_set_active  (lv_obj_t *app, void   (*active)(lv_obj_t *app, void *p_param));
void alv_am_app_set_deactive(lv_obj_t *app, void (*deactive)(lv_obj_t *app, void *p_param));
void alv_am_app_set_focus   (lv_obj_t *app, void    (*focus)(lv_obj_t *app, void *p_param));
void alv_am_app_set_defocus(lv_obj_t *app, void  (*defocus)(lv_obj_t *app, void *p_param));

/**
 * alv_am_app_is_exist - this app is exist
 * @app_name: [IN] app string
 * @return: true,exist,false,not exist.
 */
bool alv_am_app_is_exist(char *app_name);
/**
 * alv_am_app_is_active - this app is active
 * @app_name: [IN] app string
 * @return: true,active,false,not active.
 */
bool alv_am_app_is_active(char *app_name);


/* win */

/**
 * Create a win object, and its parent should be an app
 * @param parent    pointer to an object, it will be the parent of the new win
 * @return          pointer to the created window
 */
lv_obj_t *alv_am_win_create(lv_obj_t *parent, char *name);
/**
 * alv_am_win_enter - enter win
 * @app: [IN] win object
 * @name: [IN] win name
 * @p_param: [IN] win user data
 * @return: 0,success,-1,fail.
 */
int32_t alv_am_win_enter(lv_obj_t *win, char *name, void *p_param);
/**
 * alv_am_win_exit - exit win
 * @app: [IN] win object
 * @name: [IN] win name
 * @p_param: [IN] win user data
 * @return: 0,success,-1,fail.
 */
int32_t alv_am_win_exit(lv_obj_t *win, char *name, void *p_param);

void alv_am_win_set_active  (lv_obj_t *win, void   (*active)(lv_obj_t *win, void *p_param));
void alv_am_win_set_deactive(lv_obj_t *win, void (*deactive)(lv_obj_t *win, void *p_param));
void alv_am_win_set_focus   (lv_obj_t *win, void    (*focus)(lv_obj_t *win, void *p_param));
void alv_am_win_set_defocus (lv_obj_t *win, void  (*defocus)(lv_obj_t *win, void *p_param));
/**
 * alv_am_win_exit_current - exit current win
 * @p_param: [IN] win user data
 * @return: 0,success,-1,fail.
 */
int32_t alv_am_win_exit_current(void *p_param);

#ifdef CONFIG_FP
lv_obj_t *get_app_by_name(char *name);
lv_obj_t *get_win_by_name(lv_obj_t *app, char *name);
#ifdef CONFIG_APP_ENTRY_IDLE_FP
int32_t alv_am_app_enter_action(lv_obj_t *app, char *name, void *p_param);
int32_t alv_am_app_exit_action(lv_obj_t *app, char *name, void *p_param);
int32_t alv_am_app_jump_action(lv_obj_t *app, char *name, void *p_param);
int32_t alv_am_app_exit_action_all();
int32_t alv_am_app_enter_defocus_action(lv_obj_t *app, char *name, void *p_param);
int32_t alv_am_win_jump_action(lv_obj_t *win, char *name, void *p_param);
int32_t alv_am_win_exit_action(lv_obj_t *win, char *name, void *p_param);
int32_t alv_am_win_exit_action_by_app(char *app_name, char *win_name, void *p_param);
#endif
int32_t alv_am_win_get_cur_winname(char *win_name);
#endif


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __ALV_APP_MGR_H__ */
