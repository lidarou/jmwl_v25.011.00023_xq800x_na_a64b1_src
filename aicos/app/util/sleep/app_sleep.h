/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_sleep.h
 *
 */

#ifndef __APP_SLEEP_H__
#define __APP_SLEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
* app_enter_sleep - make app task enter sleep.
* Return: if success return 0, else return other value.
* Description: app enter sleep.
*/
int app_enter_sleep(void);

/**
* app_exit_sleep - make app task exit sleep.
* Return: if success return 0, else return other value.
* Description: app exit sleep.
*/
int app_exit_sleep(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __APP_SLEEP_H__ */

