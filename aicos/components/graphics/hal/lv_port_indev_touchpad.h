/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * @file lv_port_indev_touchpad.h
 *
 */
#ifndef LV_PORT_INDEV_TOUCHPAD_H
#define LV_PORT_INDEV_TOUCHPAD_H
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
* touchpad_init - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to initialize touchpad.
*/
void touchpad_init(void);

/**
* touchpad_register - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to register touchpad.
*/
void touchpad_register(void);

/**********************
 *      MACROS
 **********************/
#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif /*LV_PORT_INDEV_TOUCHPAD_H*/
