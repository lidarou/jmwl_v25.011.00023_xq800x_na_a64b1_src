/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  kpled backlight user demo.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15ad9d

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <display.h>

void backlight_demo_task(void *input)
{
    u32 brt_perc;
    int ret, i;

    while(1) {
        for ( i = 0; i <= 100; i++) {
            ret = aic_bl_brightness_set(i);
            if (ret)
                alog_warn("brightness scaling fail!");

            ret = aic_bl_brightness_get(&brt_perc);
            if (ret)
                alog_warn("get backlight brightness fail!");
            else
                alog_info("current backlight brightness:%d\n", brt_perc);

            aos_sleep(500);
        }
    }
}

void backlight_demo_cb(AIC_BL_SWITCH bl_status)
{
    switch (bl_status) {
    case AIC_BL_TURN_ON:
        alog_warn("%s backlight turn on!\n", __func__);
        break;

    case AIC_BL_TURN_OFF:
        alog_warn("%s backlight turn off!\n", __func__);
        break;

    default:
        alog_warn("%s unsupport bl status cb!\n");
        break;
    }

}

static int backlight_demo_init(void)
{
    int ret;

    aos_create_thread("backlight_demo", backlight_demo_task, AOS_NULL, 1024, 8,
                      "backlight_demo", 16, AOS_AUTO_START);

    ret = register_backlight_switch_monitor("backlight_demo", backlight_demo_cb);

    return 0;
}
AIC_NORMAL_APP_L1_INIT_EXPORT(backlight_demo_init);
