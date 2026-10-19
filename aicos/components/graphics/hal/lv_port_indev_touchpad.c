/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/
/**
 * @file lv_port_indev_touchpad.c
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x042a85
#define LOG_MODULE    AMOD_LVGL

/*********************
 *      INCLUDES
 *********************/
#ifndef WIN32
#endif
#include "lvgl.h"
#include "lv_port_indev_touchpad_cali.h"
#include "tsc.h"

#include "tp_ts.h"

/*********************
 *      DEFINES
 *********************/
#define TOUCHPAD_DEBUG 0

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void touchpad_read(lv_indev_drv_t *p_indev_drv, lv_indev_data_t *p_data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(lv_coord_t *p_x, lv_coord_t *p_y);

/**********************
 *  STATIC VARIABLES
 **********************/
#ifdef CONFIG_BL6XX3
static u16 s_tp_x = 0;
static u16 s_tp_y = 0;
#else
static u8 s_tsc_x = 0;
static u8 s_tsc_y = 0;
#endif
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
* touchpad_init - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to initialize touchpad.
*/
void touchpad_init(void)
{
    /*Your code comes here*/
    touchpad_clear_cali_coordinate_data();
}

/**
* touchpad_register - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to register touchpad.
*/
void touchpad_register(void)
{
    static lv_indev_drv_t touchpad_indev_drv = {0};
    lv_indev_t *p_indev_touchpad = NULL;

    lv_indev_drv_init(&touchpad_indev_drv);
    touchpad_indev_drv.type = LV_INDEV_TYPE_POINTER;
    touchpad_indev_drv.read_cb = touchpad_read;
    p_indev_touchpad = lv_indev_drv_register(&touchpad_indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_drv_t *p_indev_drv, lv_indev_data_t *p_data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    if (NULL == p_data) {
        return;
    }

    /*Save the pressed coordinate and the state*/
    if (touchpad_is_pressed()) {
        touchpad_get_xy(&last_x, &last_y);
        p_data->state = LV_INDEV_STATE_PR;
#if TOUCHPAD_DEBUG
        aicx_info(0x10553d, "sdd", "[%s] last_x = %d, last_y = %d.\r\n", __func__, last_x, last_y);
#endif
    } else {
        p_data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinate*/
    p_data->point.x = last_x;
    p_data->point.y = last_y;
}

#ifdef CONFIG_BL6XX3
/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    //u16 x = 0;
    //u16 y = 0;

    /*Your code comes here*/
    if(get_down_point(&s_tp_x, &s_tp_y)) {
        return true;
    }

    if(get_up_point(&s_tp_x, &s_tp_y)) {
        return true;
    }

    if(get_move_point(&s_tp_x, &s_tp_y)) {
        return true;
    }

    return false;
}

/*Get the x and y coordinate if the touchpad is pressed*/
static void touchpad_get_xy(lv_coord_t *p_x, lv_coord_t *p_y)
{
    /*Your code comes here*/

    (*p_x) = (lv_coord_t)(s_tp_x);
    (*p_y) = (lv_coord_t)(s_tp_y);
#if TOUCHPAD_DEBUG
    aicx_info(0x00694f, "sdd", "[%s] [x, y] = [%d, %d].\r\n", __func__, (*p_x), (*p_y));
#endif
}
#else
/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    /*Your code comes here*/
    aic_tsc_point_t tsc_point = {0};
    int aic_tsc_ret = -1;

    aic_tsc_ret = aic_tsc_read_point(&tsc_point);
    if (AIC_SUCCESS != aic_tsc_ret) {
        return false;
    }

    s_tsc_x = tsc_point.point_x;
    s_tsc_y = tsc_point.point_y;
    if (TSC_POINT_EVENT_PRESS == tsc_point.point_event) {
        return true;
    }

    return false;
}

/*Get the x and y coordinate if the touchpad is pressed*/
static void touchpad_get_xy(lv_coord_t *p_x, lv_coord_t *p_y)
{
    /*Your code comes here*/
    tp_cali_data_t *p_tp_cali_data = NULL;
    float   rate_x = 1;
    float   rate_y = 1;
    int16_t delta_x = 0;
    int16_t delta_y = 0;
    uint32_t temp_value_x = 0;
    uint32_t temp_value_y = 0;

    p_tp_cali_data = touchpad_get_cali_coordinate_data();
    if (NULL == p_tp_cali_data) {
        rate_x = 1;
        rate_y = 1;
        delta_x = 0;
        delta_y = 0;
    } else {
        rate_x  = p_tp_cali_data->rate_x;
        rate_y  = p_tp_cali_data->rate_y;
        delta_x = p_tp_cali_data->delta_x;
        delta_y = p_tp_cali_data->delta_y;
    }

    /* 2.Calculate offset */
    temp_value_x = (lv_disp_get_hor_res(NULL) * s_tsc_x) / TSC_POS_MAX_VALUE_X;
    temp_value_y = (lv_disp_get_ver_res(NULL) * s_tsc_y) / TSC_POS_MAX_VALUE_Y;
#if TOUCHPAD_DEBUG
    aicx_info(0x13e0a2, "sdddddd", "[%s] [w, h] = [%d, %d], before transfer = [%d, %d], after transfer = [%d, %d].\r\n", __func__, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL), s_tsc_x, s_tsc_y, temp_value_x, temp_value_y);
#endif
    (*p_x) = (lv_coord_t)((temp_value_x - delta_x)/rate_x);
    (*p_y) = (lv_coord_t)((temp_value_y - delta_y)/rate_y);
#if TOUCHPAD_DEBUG
    aicx_info(0x1a050c, "sddffdd", "[%s] [x, y] = [%d, %d], rate  = [%f, %f], delta = [%d, %d].\r\n", __func__, (*p_x), (*p_y), rate_x, rate_y, delta_x, delta_y);
#endif
}

#endif

