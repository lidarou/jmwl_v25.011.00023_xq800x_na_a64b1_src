/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the aicos lvgl initialize.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x079fd2
#define LOG_MODULE    AMOD_LVGL

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <tx_api.h>
#include <lvgl.h>
#include <lv_graphics_hal.h>
#include <aic_osa.h>
#include <timer_inc.h>
#include "lv_port_indev.h"

#if LV_USE_LOG
static void lv_aic_log(const char *buf)
{
    /*soon add, for aic platform print to log buffer*/
}
#endif

int lv_port_init(void)
{
    aicx_info(0x0ae8f2, "sdd", "%s %d, curr_tmr = %d\n", __func__, __LINE__, aic_get_tick());
#if LV_USE_LOG
//    lv_log_register_print_cb(lv_aic_log);
#endif
    lv_init();

    /*for lcdc&framebuffer operate*/
    lv_disp_init();

    /*add for input dev*/
    lv_port_indev_init();

    aicx_info(0x0ae8f2, "sdd", "%s %d, curr_tmr = %d\n", __func__, __LINE__, aic_get_tick());
    return 0;
}
AIC_APP_P1_INIT_EXPORT(lv_port_init);

