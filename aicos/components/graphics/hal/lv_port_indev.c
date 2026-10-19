/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/
/**
 * @file lv_port_indev.c
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x151962
#define LOG_MODULE    AMOD_LVGL

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev_keypad.h"
#include "lv_port_indev_touchpad.h"

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
 *   GLOBAL FUNCTIONS
 **********************/
/**
* lv_port_indev_init - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to initialize input device based on lvgl,
*              such as keypad, mouse, touchpad, etc.
*/
void lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */
    /*------------------
     * Keypad
     * -----------------*/
#ifdef CONFIG_KPC
    /*Initialize your keypad or keyboard if you have*/
    keypad_init();
    /*Register a keypad input device*/
    keypad_register();
#endif

    /*------------------
     * Touchpad
     * -----------------*/
#ifdef CONFIG_TSC
    /*Initialize your touchpad if you have*/
    touchpad_init();
    /*Register a touchpad input device*/
    touchpad_register();
#endif
}
