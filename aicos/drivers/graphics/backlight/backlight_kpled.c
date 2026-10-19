/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  kpled backlight control interface.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0c7241
#define LOG_MODULE    AMOD_GPAPH

#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <display.h>
#include <pmu_user.h>

struct pmu_user *kpled_handle = NULL;

struct bl_ops {
    int (*p_bl_brightness_set)(u32 brt_perc);
    int (*p_bl_brightness_get)(u32* brt_perc);
};

struct bl_ops *cur_bl_ops;

int gp201_bl_brightness_set(u32 brt_perc)
{
    u32 kpled_cur;
    int ret;

    if (!kpled_handle) {
        kpled_handle = pmu_get("kpled_backlight", "kpled");
        if (!kpled_handle) {
            ret = AIC_ENXIO;
            aicx_warn(0x14723e, "s", "%s get kpled backlight operate handler fail!\n", __func__);
            goto out;
        }
    }

    if (brt_perc == 0) {
        ret = pmu_force_disable(kpled_handle);
        if (ret) {
            aicx_warn(0x18f9bf, "s", "%s set kpled backlight to zero fail!\n", __func__);
        }

        broadcast_backlight_switch(AIC_BL_TURN_OFF);

        goto out;
    }

    if (brt_perc < 100)
        kpled_cur = brt_perc * 800;/*brt_perc * 80000 / 100*/
    else
        kpled_cur = 80000;

    ret = pmu_current_set(kpled_handle, kpled_cur, 800);
    if (ret) {
        aicx_warn(0x132284, "s", "%s set voltage fail!\n", __func__);
        goto out;
    }

    ret = pmu_is_enabled(kpled_handle);
    if (ret) {
        ret = 0;
        goto out;
    }

    ret = pmu_enable(kpled_handle);
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
    u32 kpled_curr;
    int ret;

    if (!kpled_handle) {
        kpled_handle = pmu_get("kpled_backlight", "kpled");
        if (!kpled_handle) {
            ret = AIC_ENXIO;
            aicx_warn(0x14723e, "s", "%s get kpled backlight operate handler fail!\n", __func__);
            goto out;
        }
    }

    ret = pmu_is_enabled(kpled_handle);
    if (ret <= 0) {
        *brt_perc = 0;
        goto out;
    }

    ret = pmu_current_get(kpled_handle, &kpled_curr);
    if (ret) {
        aicx_warn(0x1abef6, "s", "%s get current fail!\n", __func__);
        goto out;
    }

    *brt_perc = kpled_curr / 800;/*kpled_curr * 100 / 80000*/
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
