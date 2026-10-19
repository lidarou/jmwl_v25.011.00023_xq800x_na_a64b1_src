/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of adc HAL module
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x02c26b
#define LOG_MODULE    AMOD_PMIC

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#include "adc_phy.h"
#include <adc.h>
#include <pmu_driver.h>
#include <pmu_user.h>

#define AIC_PMIC_ADC_MUTEX      "pmic_adc"
aos_mutex_ptr aic_pmic_adc_mutex = AOS_NULL;
struct pmu_user *adc_handle = NULL;
struct adc_phy_ops *cur_adc_ops = NULL;

static aos_status adc_lock(void)
{
    int ret = AOS_SUCCESS;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_pmic_adc_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    return ret;
}

static void adc_unlock(void)
{
    if (aos_is_user_thread_context()) {
        aos_put_mutex(aic_pmic_adc_mutex);
    }
}

int adc_enable(bool enable)
{
    int ret;

    if (!adc_handle) {
        adc_handle = pmu_get("adc_hal", "adc");
        if (!adc_handle) {
            ret = AIC_ENXIO;
            aicx_warn(0x0f8060, "s", "%s get adc pmu operate handler fail!\n", __func__);
            goto out;
        }
    }

    if (enable)
        ret = pmu_enable(adc_handle);
    else
        ret = pmu_disable(adc_handle);

    if (ret)
        aicx_warn(0x0daf2f, "s", "%s adc pmu operate fail!\n", __func__);

out:
    return ret;
}

int adc_self_cal(void)
{
    int ret;

    adc_lock();

    ret = adc_enable(true);
    if (ret) {
        aicx_warn(0x1996eb, "", "adc enable fail!\n");
        goto out;
    }

    ret = cur_adc_ops->adc_cali();
    if (ret)
        aicx_warn(0x15ff0f, "", "adc self calibrate failed!!\n");

    ret = adc_enable(false);
    if (ret)
        aicx_warn(0x1e2901, "", "adc disable fail!\n");

out:
    adc_unlock();

    return ret;
}

int adc_data_get(u32 chan, u32 *data)
{
    int ret = 0;

    adc_self_cal();

    adc_lock();

    ret = adc_enable(true);
    if (ret) {
        aicx_warn(0x1996eb, "", "adc enable fail!\n");
        goto out;
    }

    ret = cur_adc_ops->adc_get(chan, data);
    if (ret < 0) {
        aicx_warn(0x1c7c81, "d", "get adc chan:%d val failed!!\n", chan);
    }

    ret = adc_enable(false);
    if (ret)
        aicx_warn(0x1e2901, "", "adc disable fail!\n");

out:
    adc_unlock();

    return ret;
}

int __adc_chan_voltage_get(u32 chan, u32 *voltage_mv)
{
    int ret = 0;
    u32 data = 0;

    ret = adc_enable(true);
    if (ret) {
        aicx_warn(0x1996eb, "", "adc enable fail!\n");
        goto out;
    }

    ret = cur_adc_ops->adc_get(chan, &data);
    if (ret < 0) {
        aicx_warn(0x1c7c81, "d", "get adc chan:%d val failed!!\n", chan);
    } else {
        *voltage_mv = cur_adc_ops->adc2voltage(data);
    }

    ret |= adc_enable(false);
    if (ret)
        aicx_warn(0x1e2901, "", "adc disable fail!\n");

out:

    return ret;
}

int adc_chan_voltage_get(u32 chan, u32 *voltage_mv)
{
    int ret;

    adc_self_cal();

    adc_lock();

    ret = __adc_chan_voltage_get(chan, voltage_mv);

    adc_unlock();

    return ret;
}

u32 vbat_vol_mv_get(void)
{
    u32 vbat_vol;

    adc_self_cal();

    adc_lock();

    cur_adc_ops->vbat_vbus_mode(true);
    cur_adc_ops->vbat_vbus_sel(true);
    __adc_chan_voltage_get(cur_adc_ops->vbat_adc_chan, &vbat_vol);
    cur_adc_ops->vbat_vbus_mode(false);

    adc_unlock();

    return vbat_vol * 3;
}

u32 vbus_vol_mv_get(void)
{
    u32 vbat_vol;

    adc_self_cal();

    adc_lock();

    cur_adc_ops->vbat_vbus_mode(true);
    cur_adc_ops->vbat_vbus_sel(false);
    __adc_chan_voltage_get(cur_adc_ops->vbat_adc_chan, &vbat_vol);
    cur_adc_ops->vbat_vbus_mode(false);

    adc_unlock();

    return vbat_vol * 3;
}

int aic_adc_init(void)
{
    aic_pmic_adc_mutex = aos_create_mutex(AIC_PMIC_ADC_MUTEX);
    AIC_ASSERT(aic_pmic_adc_mutex);

    return 0;
}
AIC_DRV_P1_INIT_EXPORT(aic_adc_init);

int adc_phy_ops_register(struct adc_phy_ops* ops)
{
    int ret = 0;

    if (!ops) {
        aicx_warn(0x1df68e, "s", "%s:NULL ops register fail!\n", __func__);
        ret = AIC_ENXIO;
        goto out;
    }

    if (cur_adc_ops) {
        aicx_warn(0x09eff2, "s", "%s:adc phy ops register fail!\n", __func__);
        ret = AIC_EOVERFLOW;
        goto out;
    }

    cur_adc_ops = ops;

out:
    return ret;
}

int pmu_adc_enable(struct pmu_unit *punit)
{
    cur_adc_ops->adc_enable(true);

    return 0;
}

int pmu_adc_disable(struct pmu_unit *punit)
{
    cur_adc_ops->adc_enable(false);

    return 0;
}

int pmu_adc_is_enabled(struct pmu_unit *punit)
{
    return cur_adc_ops->adc_is_enabled();
}

static const struct pmu_ops adc_ops = {
        .enable = pmu_adc_enable,
        .disable = pmu_adc_disable,
        .is_enabled = pmu_adc_is_enabled,
};

static struct pmu_info adc_info = {
    .unit_name = "adc",
    .parent_name = "VBAT",
    .pops = &adc_ops
};

int pmu_adc_driver_init(void)
{
    int ret;

    ret = pmu_register(&adc_info, NULL);
    if (ret)
        aicx_warn(0x0a1bd8, "ss", "%s pmu unit %s register fail!\n", __func__, adc_info.unit_name);

    return ret;
}
AIC_DRV_P2_INIT_EXPORT(pmu_adc_driver_init);
