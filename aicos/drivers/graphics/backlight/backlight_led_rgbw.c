/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  rgbw led backlight control interface.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x081193
#define LOG_MODULE    AMOD_GPAPH

#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <display.h>
#include <lamp.h>

struct bl_ops {
    int (*p_bl_brightness_set)(u32 brt_perc);
    int (*p_bl_brightness_get)(u32* brt_perc);
};

struct bl_ops *cur_bl_ops;

int gp201_bl_brightness_set(u32 brt_perc)
{
    u32 led_curr;
    int ret;

    if (brt_perc == 0) {
        ret = lamp_enabled(false);
        if (ret) {
            aicx_warn(0x1978db, "s", "%s set backlight to zero fail!\n", __func__);
        }

        broadcast_backlight_switch(AIC_BL_TURN_OFF);

        goto out;
    }

    if (brt_perc < 100)
        led_curr = brt_perc * (CONFIG_LED_RGBW_BACKLIGHT_MAX_CURRENT / 100);/*brt_perc * 80000 / 100*/
    else
        led_curr = CONFIG_LED_RGBW_BACKLIGHT_MAX_CURRENT;

    ret = lamp_set_current(led_curr);
    if (ret) {
        aicx_warn(0x12eec6, "s", "%s set backlight current fail!\n", __func__);
        goto out;
    }

    ret = lamp_is_enabled();
    if (ret) {
        ret = 0;
        goto out;
    }

    ret = lamp_enabled(true);
    if (ret) {
        aicx_warn(0x06e33d, "s", "%s enable fail!\n", __func__);
        goto out;
    }

    broadcast_backlight_switch(AIC_BL_TURN_ON);

out:
    return ret;
}

int gp201_bl_brightness_get(u32* brt_perc)
{
    u32 led_curr = 0;
    int ret;

    ret = lamp_is_enabled();
    if (ret <= 0) {
        *brt_perc = 0;
        goto out;
    }

    ret = lamp_get_current(&led_curr);
    if (ret) {
        aicx_warn(0x1abef6, "s", "%s get current fail!\n", __func__);
        goto out;
    }

    *brt_perc = led_curr / (CONFIG_LED_RGBW_BACKLIGHT_MAX_CURRENT / 100);
out:
    return ret;
}

struct bl_ops gp201_bl_ops = {
    .p_bl_brightness_set = gp201_bl_brightness_set,
    .p_bl_brightness_get = gp201_bl_brightness_get,

};

int gp202_bl_brightness_set(u32 brt_perc) {return 0;}
int gp202_bl_brightness_get(u32* brt_perc) {return 0;}

struct bl_ops gp202_bl_ops = {
    .p_bl_brightness_set = gp202_bl_brightness_set,
    .p_bl_brightness_get = gp202_bl_brightness_get,

};

int aic_bl_brightness_set(u32 brt_perc)
{
    return cur_bl_ops->p_bl_brightness_set(brt_perc);
}

int aic_bl_brightness_get(u32* brt_perc)
{
    return cur_bl_ops->p_bl_brightness_get(brt_perc);
}

int backlight_ops_init(void)
{
    int ret =0, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202)
        cur_bl_ops = &gp202_bl_ops;
    else
        cur_bl_ops = &gp201_bl_ops;

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(backlight_ops_init);
