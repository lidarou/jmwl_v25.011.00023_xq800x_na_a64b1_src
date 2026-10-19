/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * @file lv_port_indev_keypad.h
 *
 */
#ifndef LV_PORT_INDEV_KEYPAD_H
#define LV_PORT_INDEV_KEYPAD_H
#ifdef __cplusplus
extern "C" {
#endif
/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
/* User defined lv key value */
typedef enum
{
    LV_KEY_GREEN = 200, /* prevent repeat with native lvgl key */
    LV_KEY_RED,
    LV_KEY_STAR,
    LV_KEY_HASH,
    LV_KEY_0 = 48, /* 和0的ascii码值相同 */
    LV_KEY_1,
    LV_KEY_2,
    LV_KEY_3,
    LV_KEY_4,
    LV_KEY_5,
    LV_KEY_6,
    LV_KEY_7,
    LV_KEY_8,
    LV_KEY_9
}LV_PORT_KEY_E;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
* keypad_write - Brief description of function.
* Parameter: [In]: void * p_data
*            [Out]: None
*            [Return]: if success return 0, else return other value.
* Description: This function is used to write keypad.
*/
int keypad_write(void * p_data);
/**
* keypad_init - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to initialize keypad.
*/
void keypad_init(void);
/**
* keypad_register - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to register keypad.
*/
void keypad_register(void);
/**********************
 *      MACROS
 **********************/
#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif /*LV_PORT_INDEV_KEYPAD_H*/
