/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * @file lv_port_indev_touchpad_cali.h
 *
 */
#ifndef LV_PORT_INDEV_TOUCHPAD_CALI_H
#define LV_PORT_INDEV_TOUCHPAD_CALI_H
#ifdef __cplusplus
extern "C" {
#endif
/*********************
 *      INCLUDES
 *********************/
#include "aic_type.h"
#include <alog_api.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    float   rate_x;
    float   rate_y;
    int16_t delta_x;
    int16_t delta_y;
}tp_cali_data_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
* touchpad_clear_cali_coordinate_data - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to clear calibration coordinate delta value.
*/
void touchpad_clear_cali_coordinate_data(void);

/**
* touchpad_save_cali_coordinate_data - Brief description of function.
* Parameter: [In]: p_tp_cali_data: the calibration coordinate delta value
*            [Out]: None
*            [Return]: None
* Description: This function is used to save calibration coordinate delta value.
*/
void touchpad_set_cali_coordinate_data(tp_cali_data_t *p_tp_cali_data);

/**
* touchpad_get_cali_coordinate_data - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: the calibration coordinate delta value pointer
* Description: This function is used to save calibration coordinate delta value.
*/
tp_cali_data_t *touchpad_get_cali_coordinate_data(void);

/**
* touchpad_is_calibrated - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: true: have calibrated; false: have not calibrated.
* Description: This function is used to get calibrated flag.
*/
bool touchpad_is_calibrated(void);

/**********************
 *      MACROS
 **********************/
#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif /*LV_PORT_INDEV_TOUCHPAD_CALI_H*/
