/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __PHOTO_FLOW_H__
#define __PHOTO_FLOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include "lvgl.h"
#include "photo_object.h"

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of photo service.
 */

/**
 * flow_photo_enter() - enter photo app flow.
 * @param: pointer to user data, reserved.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_enter(void *param);

/**
 * flow_photo_leave() - leavel photo app flow.
 * @param: pointer to user data, reserved.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_leave(void *param);

/**
 * flow_photo_preview_enter_evt_cb() - responding to preview
 *                                     events.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_preview_enter_evt_cb(lv_event_t *e);

/**
 * flow_photo_preview_exit_evt_cb() - responding to preview
 *                                    exit events.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_preview_exit_evt_cb(lv_event_t *e);

/**
 * flow_photo_prev_key_evt_cb - event callback of previous photo.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_prev_key_evt_cb(lv_event_t *e);

/**
 * flow_photo_next_key_evt_cb() - event callback of next photo.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_next_key_evt_cb(lv_event_t *e);

/**
 * flow_photo_prev_page_evt_cb - event callback of previous page.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_prev_page_evt_cb(lv_event_t *e);

/**
 * flow_photo_next_page_evt_cb() - event callback of next page.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_next_page_evt_cb(lv_event_t *e);

/**
 * flow_photo_rename_evt_cb() - event callback of photo rename operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_rename_evt_cb(lv_event_t *e);

/**
 * flow_photo_list_remove_evt_cb() - event callback of photo list
 *                                   remove operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_list_remove_evt_cb(lv_event_t *e);

/**
 * flow_photo_preview_remove_evt_cb() - event callback of photo preview
 *                                      remove operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_preview_remove_evt_cb(lv_event_t *e);

/**
 * flow_photo_preview_play_evt_cb() - event callback of photo preview
 *                                    play operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_preview_play_evt_cb(lv_event_t *e);

/**
 * flow_photo_thumbnail_remove_evt_cb() - event callback of remove chosen
 *                                        thumbnail operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_thumbnail_remove_evt_cb(lv_event_t *e);

/**
 * flow_photo_remove_all_evt_cb() - event callback of remove all photos
 *                                  operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_remove_all_evt_cb(lv_event_t *e);

/**
 * flow_photo_details_display_evt_cb() - event callback of photo details
 *                                       display.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_details_display_evt_cb(lv_event_t *e);

/**
 * flow_photo_zoom_evt_cb() - event callback of photo zoom in and restore.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_zoom_evt_cb(lv_event_t *e);

/**
 * flow_photo_rotate_evt_cb() - event callback of photo rotate operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_rotate_evt_cb(lv_event_t *e);

/**
 * flow_photo_matrix_display_evt_cb() - event callback of 3*3 matrix display.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_matrix_display_evt_cb(lv_event_t *e);

/**
 * flow_photo_back_key_evt_cb() - event callback for back key response.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_back_key_evt_cb(lv_event_t *e);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PHOTO_FLOW__ */
