/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __CAMERA_H__
#define __CAMERA_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of camera app.
 */

/**
 * app_camera_init() - register camera app
 * @param: pointer to user data for new framework, can be 0.
 * @arg2: none.
 * Return: pointer to camera app object.
 */
void app_camera_init(void *param);

/**
 * app_camera_deinit() - unregister camera app
 * @param: pointer to user data for new framework, can be 0.
 * @arg2: none.
 * Return: pointer to camera app object.
 */
void app_camera_deinit(void *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __CAMERA__ */
