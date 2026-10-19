/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __CAMERA_SERVICE_H__
#define __CAMERA_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include "aic_type.h"

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of camera service.
 */

/**
 * @definition of camera status
 */
typedef enum camera_state {
    CAM_OPEN = 1,
    CAM_PREVIEW,
    CAM_PREPARE,
    CAM_SHOOTING,
    CAM_IMG_SAVING,
    CAM_CLOSE,
} cam_state_e;

/**
 * @definition of camera flash light status
 */
typedef enum camera_flashlight_state {
    CAM_FLASHLIGHT_ON,
    CAM_FLASHLIGHT_OFF,
} cam_flashlihgt_state_e;

/**
 * @definition of camera capture mode
 */
typedef enum camera_capture_mode {
    CAM_MODE_SINGLE,
    CAM_MODE_CONTINUOUS,
    CAM_MODE_COUNTDOWN,
} cam_capture_mode_e;

typedef void (*camera_notify_callback)(void *img_data, u32 size);

/**
 * app_srv_camera_start() - start the camera service.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_start(void);

/**
 * app_srv_camera_stop() - stop the camera service.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_stop(void);

/**
 * app_srv_camera_capture_preview() - take camera into preview mode.
 * @arg1: none.
 * @arg2: none.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_camera_capture_preview(void);

/**
 * app_srv_camera_capture_prepare() - prepare the capture according to
 *                                    the currently set mode.
 * @img_size: the image size will be return by shooting.
 * @arg2: none.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_camera_capture_prepare(u32 *img_size);

/**
 * app_srv_camera_shooting_start() - start shooting
 * @data: data buffer for storing captured photo data .
 * @size: size of data buffer input.
 * @cb: callback of time-lapse shooting to notify finishing capture.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_camera_shooting_start(u8 *data, u32 size,
                                  camera_notify_callback cb);

/**
 * app_srv_camera_shooting_stop() - stop shooting, generally used for
 *                                  time-lapse shooting.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_shooting_stop(void);

/**
 * app_srv_camera_image_save() - save the image of camera capture
 * @path: path to save the image.
 * @data: image data.
 * @size: data size.
 * @cb: callback of finishing the image saving.
 * @sync: 1 means saving asynchronous.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_camera_image_save(char *path, u8 *data, u32 size,
                              camera_notify_callback cb, u8 async);

/**
 * app_srv_camera_state_get() - get current state of camera.
 * @arg1: none.
 * @arg2: none.
 * Return: return the state of camera.
 */
cam_state_e app_srv_camera_state_get(void);

/**
 * app_srv_camera_capture_mode_set() - set a camera capture mode.
 * @mode: capture mode, see definition of cam_capture_mode_e.
 * @time: if countdown mode, time should be set; if continous,
 *        number of consecutive shots should be set.
 * Return: none.
 */
void app_srv_camera_capture_mode_set(cam_capture_mode_e mode, int time);

/**
 * app_srv_camera_capture_mode_get() - get the current camera capture mode.
 * @arg1: none.
 * @arg2: none.
 * Return: capture mode, see definition of cam_capture_mode_e.
 */
cam_capture_mode_e app_srv_camera_capture_mode_get(void);

/**
 * app_srv_camera_frame_rate_set() - set the frame rate of camera.
 * @fps: frame rate to set for camera.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_frame_rate_set(u8 fps);

/**
 * app_srv_camera_frame_rate_get() - get the frame rate of camera.
 * @arg1: none.
 * @arg2: none.
 * Return: return the valid frame rate of current camera setting.
 */
u8 app_srv_camera_frame_rate_get(void);

/**
 * app_srv_camera_flashlight_switch() - main menu display.
 * @state: flash light state to switch, see definition of
 *         cam_flashlihgt_state_e.
 * @arg2: none.
 * Return: none.
 */
void app_srv_camera_flashlight_switch(cam_flashlihgt_state_e state);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __CAMERA_SERVICE_H__ */