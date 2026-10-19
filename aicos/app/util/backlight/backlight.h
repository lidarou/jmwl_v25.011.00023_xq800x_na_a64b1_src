/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file backlight.h
 *
 */

#ifndef __BACKLIGHT_H__
#define __BACKLIGHT_H__

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
typedef struct backlight_config{
    uint32_t turn_off_timeout;
    uint32_t half_off_timeout;
    uint32_t brightness_on;
    uint32_t brightness_half_on;
} backlight_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
* backlight_set_config - set the backlight config.
*Return: if success return 0, else return other value.
*Description: set the backlight config.
*/
int backlight_set_config(backlight_config_t config);

/**
* backlight_get_config - get the current backlight config.
*Return: if success return 0, else return other value.
*Description: get the current backlight config.
*/
int backlight_get_config(backlight_config_t *p_config);

/**
* backlight_turn_on - trun on backlight.
* Return: if success return 0, else return other value.
* Description: trun on backlight with timer.
*/
int backlight_turn_on(void);

/**
* backlight_turn_off - trun off backlight.
* Return: if success return 0, else return other value.
* Description: trun off backlight.
*/
int backlight_turn_off(void);

/**
* backlight_turn_on_always - trun on backlight always.
* Return: if success return 0, else return other value.
* Description: trun on backlight always.
*/
int backlight_turn_on_always(void);

/**
* backlight_exit_turn_on_always -exit trun on backlight always.
* Return: if success return 0, else return other value.
* Description: exit trun on backlight always.
*/
int backlight_exit_turn_on_always(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __BACKLIGHT_H__ */

