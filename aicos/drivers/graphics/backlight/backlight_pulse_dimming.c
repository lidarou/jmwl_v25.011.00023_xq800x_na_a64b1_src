/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  pulse dimming backlight operate interface.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04be8c
#define LOG_MODULE    AMOD_GPAPH

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <display.h>
#include <pmu_user.h>
#include <gpio_inc.h>
#include <display_cfg.h>

static u32 cur_bl_perc = 0;
/*
    index       current     perc
    0           20000uA     100%
    1           17500uA     87.5%
    2           15000uA     75%
    3           12500uA     62.5%
    4           10000uA     50%
    5           7500uA      37.5%
    6           5000uA      25%
    7           2500uA      12.5%
*/
static const int backlight_perc[] = {
                10000, 8750, 7500, 6250,
                5000, 3750, 2500, 1250
                };

void falling_edge_generate(int dege_num)
{
    int i;

    aic_gpio_output(BACKLIGHT_CTRL_GPIO_NUM, GPIO_OUTPUT_LVL_HIGH);
    for (i = 0; i < dege_num; i++) {
        aic_gpio_output(BACKLIGHT_CTRL_GPIO_NUM, GPIO_OUTPUT_LVL_LOW);
        aic_udelay(2);
        aic_gpio_output(BACKLIGHT_CTRL_GPIO_NUM, GPIO_OUTPUT_LVL_HIGH);
        aic_udelay(2);
    }
}

void backlight_pd(void)
{
    aic_gpio_output(BACKLIGHT_CTRL_GPIO_NUM, GPIO_OUTPUT_LVL_LOW);
}

int get_backlight_idx(int perc_min, int perc_max)
{
    int sel = -1;
    int i;

    for ( i = 0; i < (sizeof(backlight_perc) / sizeof(backlight_perc[0])); i ++) {
        if (perc_min < backlight_perc[i] && backlight_perc[i] <= perc_max) {
            sel = i;
            break;
        }
    }

    return sel;
}

int aic_bl_brightness_set(u32 brt_perc)
{
    int tolerance, perc_min, perc_max;
    int ret = 0, idx = 0;

    tolerance = (backlight_perc[0] - backlight_perc[1]) / 2 + 1;

    perc_min = brt_perc * 100 - tolerance;
    if (perc_min < 0)
        perc_min =  0;
    perc_max = brt_perc * 100 + tolerance;
    if (perc_max > 10000)
        perc_max = 10000;

    if (brt_perc == 0) {
        backlight_pd();
        cur_bl_perc = 0;
        broadcast_backlight_switch(AIC_BL_TURN_OFF);
        goto out;
    }

    idx = get_backlight_idx(perc_min, perc_max);
    if (idx < 0) {
        ret = AIC_EIO;
        aicx_warn(0x1c9437, "s", "%s failed!\n", __func__);
        goto out;
    }

    backlight_pd();
    aos_usleep(200);

    falling_edge_generate(idx);
    if (!cur_bl_perc)
        broadcast_backlight_switch(AIC_BL_TURN_ON);

    cur_bl_perc = backlight_perc[idx] / 100;

out:
    return ret;
}

int aic_bl_brightness_get(u32* brt_perc)
{
    *brt_perc = cur_bl_perc;

    return 0;
}
