/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  GPIO Network status light user demo.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b12a2
#define LOG_MODULE    AMOD_GPIO

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_at_process_common.h>
#include <gpio_inc.h>

/*
 * Assume that GPIO30 control the green light which turn on when network online,
 * GPIO33 control the red light which turn on when network offline.
 * Notice! The PINs of GPIO should be configured as gpio function.
 */
static void aic_gpio_netlight_demo_cb(u8 cereg_state)
{
    if (cereg_state == 1) {
        aic_gpio_output(30, GPIO_OUTPUT_LVL_HIGH);
        aic_gpio_output(33, GPIO_OUTPUT_LVL_LOW);
    } else {
        aic_gpio_output(30, GPIO_OUTPUT_LVL_LOW);
        aic_gpio_output(33, GPIO_OUTPUT_LVL_HIGH);
    }
}

static int aic_gpio_netlight_demo_init(void)
{
    aic_gpio_output(30, GPIO_OUTPUT_LVL_LOW);
    aic_gpio_output(33, GPIO_OUTPUT_LVL_HIGH);

    aic_register_cereg_state_cb(aic_gpio_netlight_demo_cb);

    return 0;
}
AIC_NORMAL_APP_L1_INIT_EXPORT(aic_gpio_netlight_demo_init);
