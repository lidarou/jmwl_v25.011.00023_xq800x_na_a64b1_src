/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_srv_backlight.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_type.h>
#include <aic_osa_ex.h>
#include "app_msg.h"
#include "display.h"
#include "aic_srv_backlight.h"

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
static srv_func_callback g_backlight_cb = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void backlight_driver_cb(AIC_BL_SWITCH bl_status)
{
    int ret = 0;
    AIC_BL_SWITCH backlight_state = AIC_BL_TYPE_MAX;

    backlight_state = bl_status;

    if (NULL == g_backlight_cb) {
        app_loge("[%s] g_backlight_cb is null.\r\n", __func__);
    }
    else {
        ret = g_backlight_cb((void *)&backlight_state, sizeof(AIC_BL_SWITCH));
    }
    app_logd("[%s] ret = %d.\r\n", __func__,ret);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int aic_srv_backlight_init(srv_func_callback _srv_callback)
{
    int ret = 0;

    if (NULL == _srv_callback) {
        app_loge("[%s] param error.\r\n", __func__);
        return -1;
    }

    g_backlight_cb = _srv_callback;

    ret = register_backlight_switch_monitor("app", backlight_driver_cb);

    app_logd("[%s] ret = %d.\r\n", __func__, ret);

    return ret;
}

int aic_srv_backlight_turn_on(uint32_t level)
{
    int ret = 0;

    if (0 == level || 100 < level) {
        app_loge("[%s] invalid backlight level = %d.\r\n", __func__,level);
        return -1;
    }
    ret = aic_bl_brightness_set(level);

    app_logd("[%s] ret = %d.\r\n", __func__, ret);

    return ret;
}

int aic_srv_backlight_turn_off(void)
{
    int ret = 0;

    ret = aic_bl_brightness_set(0);

    app_logd("[%s] ret = %d.\r\n", __func__, ret);

    return ret;
}


