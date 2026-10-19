/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * @file lv_port_indev_touchpad_cali.c
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0033ec
#define LOG_MODULE    AMOD_LVGL

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev_touchpad_cali.h"

/*********************
 *      DEFINES
 *********************/
#define TOUCHPAD_CALI_DEBUG 0

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
/* TODO: Use nv instead */
static tp_cali_data_t s_tp_cali_data = {0};
/* TODO: Use nv instead */
static bool s_is_calibrated = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void touchpad_clear_cali_coordinate_data(void)
{
    s_is_calibrated = false;

    s_tp_cali_data.rate_x  = 1;
    s_tp_cali_data.rate_y  = 1;
    s_tp_cali_data.delta_x = 0;
    s_tp_cali_data.delta_y = 0;
}

void touchpad_set_cali_coordinate_data(tp_cali_data_t *p_tp_cali_data)
{
    if (NULL == p_tp_cali_data) {
        aicx_info(0x0af5ec, "s", "[%s]: p_tp_cali_data is NULL.\r\n", __func__);
        return;
    }

    /* the delta value should be sumed with last set */
    aicx_info(0x0f6790, "sdd", "[%s]: s_tp_cali_data delta_x = %d, delta_y = %d.\r\n", __func__, s_tp_cali_data.delta_x, s_tp_cali_data.delta_y);
    aicx_info(0x0ac04c, "sdd", "[%s]: p_tp_cali_data delta_x = %d, delta_y = %d.\r\n", __func__, p_tp_cali_data->delta_x, p_tp_cali_data->delta_y);

    s_tp_cali_data.rate_x  = p_tp_cali_data->rate_x;
    s_tp_cali_data.rate_y  = p_tp_cali_data->rate_y;
    s_tp_cali_data.delta_x = p_tp_cali_data->delta_x;
    s_tp_cali_data.delta_y = p_tp_cali_data->delta_y;

    /* TODO: write NV */
    aicx_info(0x049e90, "sdd", "[%s]: set value: s_tp_cali_data delta_x = %d, delta_y = %d.\r\n", __func__, s_tp_cali_data.delta_x, s_tp_cali_data.delta_y);

    /* set calibrated flag */
    s_is_calibrated = true;
}

tp_cali_data_t *touchpad_get_cali_coordinate_data(void)
{
    /* TODO: Use nv to get(if first get, should read nv(maybe need some time); if not first, return global variable.) */
#if TOUCHPAD_CALI_DEBUG
    aicx_info(0x0f6790, "sdd", "[%s]: s_tp_cali_data delta_x = %d, delta_y = %d.\r\n", __func__, s_tp_cali_data.delta_x, s_tp_cali_data.delta_y);
#endif
    return &s_tp_cali_data;
}

bool touchpad_is_calibrated(void)
{
    aicx_info(0x1a338c, "sd", "[%s]: s_is_calibrated = %d.\r\n", __func__, s_is_calibrated);
    return s_is_calibrated;
}
