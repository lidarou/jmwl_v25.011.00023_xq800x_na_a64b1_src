/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_srv_backlight.h
 *
 */

#ifndef __AIC_SRV_BACKLIGHT_H__
#define __AIC_SRV_BACKLIGHT_H__

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
typedef enum {
    SRV_BACKLIGHT_TURN_ON = 1,
    SRV_BACKLIGHT_TURN_OFF,
    SRV_BACKLIGHT_TYPE_MAX
}srv_backlight_state_e;

typedef int32_t (*srv_func_callback)(void *p_param, uint32_t size);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
* aic_srv_backlight_init - init backlight service.
* @_srv_callback: [IN] pointer of callback which receive backlight event.
* Return: if success return 0, else return other value.
* Description: init app backlight.
*/
int aic_srv_backlight_init(srv_func_callback _srv_callback);

/**
* aic_srv_backlight_turn_on - trun on backlight.
* @level: [IN] level of maximum brightness, 1 ~ 100.
* Return: if success return 0, else return other value.
* Description: trun on backlight.
*/
int aic_srv_backlight_turn_on(uint32_t level);

/**
* aic_srv_backlight_turn_off - trun off backlight.
* Return: if success return 0, else return other value.
* Description: trun off backlight.
*/
int aic_srv_backlight_turn_off(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_SRV_BACKLIGHT_H__ */


