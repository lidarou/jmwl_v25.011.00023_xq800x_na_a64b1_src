/*****************************************************************************/
/*                                                                           */
/*         Copyright 2022 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of tsc module.
 *   It includes all the tsc phy function api.
*/
#ifndef __TSC_H__
#define __TSC_H__

#include <aic_type.h>
#include <aic_errno.h>

#define TSC_POINT_EVENT_PRESS             (1)
#define TSC_POINT_EVENT_RELEASE           (2)
#define TSC_POS_MAX_VALUE_X             (256)
#define TSC_POS_MAX_VALUE_Y             (256)

/**
 * struct aic_tsc_point_t - tsc point info.
 * @point_x: the value of x position of point.
 * @point_y: the value of y position of point.
 * @point_event: the event of point:
 *               TSC_POINT_EVENT_PRESS:pressed,.
 *               TSC_POINT_EVENT_RELEASE:released
 * @tick_count:the tick count of point.
 *
 * tsc point info.
 */
typedef struct aic_tsc_point {
    u8  point_x;
    u8  point_y;
    u16 point_event;
    u32 tick_count;
}aic_tsc_point_t;

/**
* aic_tsc_read_point - Brief description of function.
* @p_tsc_point: output the tsc point info.
* Return: AIC_SUCCESS.
* Description: This function is used to read tsc point.
*/
int aic_tsc_read_point( aic_tsc_point_t *p_tsc_point );

#endif /* __TSC_H__ */
