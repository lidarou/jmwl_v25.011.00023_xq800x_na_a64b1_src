/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic Application Interface (API) of file
 *    browser app flow(fbaf).  All app prototypes and data structure
 *    definitions are defined in this file.
 */

#ifndef __FILEBROWSER_FLOW_H__
#define __FILEBROWSER_FLOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include <lvgl.h>

/*******************************************
 *              DEFINES
 ******************************************/
#ifndef FB_APP_NAME
#define FB_APP_NAME "filebrowser"
#endif
#define CONFIG_FB_APP_USE_GESTURE_BACK

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
/**
 * flow_fb_init - init file browser app flow status
 * @app: [IN] pointer to app obj
 *
 * Return: the current page in show
 */
void flow_fb_init(lv_obj_t *app);

/**
 * flow_fb_destory - destroy file browser app flow status
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fb_destory(lv_obj_t *app);

/**
 * flow_fb_suspend - suspend the app
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fb_suspend(lv_obj_t *app);

/**
 * flow_fb_resume - resume the app
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fb_resume(lv_obj_t *app);

/**
 * ui_fb_folder_event_handler - handle backwards key event
 * @e: [IN] event
 *
 * Return: NA
 * Note: used in ui.c
 */
void ui_fb_folder_event_handler(lv_event_t *e);

/**
 * ui_fb_event_device_gesture_back - handle the ui events triggered by gesture
 * back in device list page
 * @e: [IN] event
 *
 * Return: NA
 * Note: used in ui.c
 */
void ui_fb_event_device_gesture_back(lv_event_t *e);

/**
 * ui_fb_event_gesture_back - handle the ui events triggered by gesture back
 * in folder/folder swap list list item
 * @e: [IN] event
 *
 * Return: NA
 * Note: used in ui.c
 */
void ui_fb_event_gesture_back(lv_event_t *e);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __FILEBROWSER_FLOW_H__ */
