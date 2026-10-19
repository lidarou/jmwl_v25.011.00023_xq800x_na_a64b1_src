/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __CAMERA_FLOW_H__
#define __CAMERA_FLOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include "lvgl.h"

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of camera flow layer.
 */

/**
 * flow_camera_enter() - enter camera app flow.
 * @param: pointer to user data, reserved.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_enter(void *param);

/**
 * flow_camera_leave() - exit camera app flow.
 * @param: pointer to user data, reserved.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_leave(void *param);

/**
 * flow_camera_capture_evt_cb() - event callback of capture key operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_capture_evt_cb(lv_event_t *e);

/**
 * flow_camera_capture_mode_select_evt_cb() - event callback of mode selection.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_capture_mode_select_evt_cb(lv_event_t *e);

/**
 * flow_camera_capture_mode_option_evt_cb() - event callback of mode option
 * list.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_capture_mode_option_evt_cb(lv_event_t *e);

/**
 * flow_camera_flashlight_switch_evt_cb - event callback for flashlight switch.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_flashlight_switch_evt_cb(lv_event_t *e);

/**
 * flow_camera_countdown_evt_cb - event callback for countdown operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_camera_countdown_evt_cb(lv_event_t *e);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __CAMERA_H__ */
