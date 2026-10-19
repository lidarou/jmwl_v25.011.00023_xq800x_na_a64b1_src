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

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aic_math.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "adc_phy.h"
#include "adc_reg.h"
#include <timer.h>

#include <adc.h>

struct adc_phy_ops *cur_adc_ops = NULL;
bool adc_phy_initialized = false;

void get_adc_phy_ops(void)
{
    if (!adc_phy_initialized) {
        adc_phy_ops_init();
        adc_phy_initialized = true;
    }
}

int adc_data_get(u32 chan, u32 *data)
{
    int ret = 0;

    get_adc_phy_ops();

    cur_adc_ops->adc_calibrate();

    ret = cur_adc_ops->adc_get(chan, data);
    if (ret < 0)
        log_print("get adc chan:%d val failed!!\n", chan);

    return ret;
}

int adc_chan_voltage_get(u32 chan, u32 *voltage_mv)
{
    int ret = 0;
    u32 data = 0;

    get_adc_phy_ops();

    cur_adc_ops->adc_calibrate();

    ret = cur_adc_ops->adc_get(chan, &data);
    if (ret < 0) {
        log_print("get adc chan:%d val failed!!\n", chan);
    } else {
        *voltage_mv = cur_adc_ops->adc2voltage(data);
    }

    return ret;
}

u32 vbat_vol_mv_get(void)
{
    u32 vbat_vol;

    get_adc_phy_ops();

    cur_adc_ops->vbat_vbus_mode(true);
    cur_adc_ops->vbat_vbus_sel(true);
    adc_chan_voltage_get(cur_adc_ops->vbat_adc_chan, &vbat_vol);
    cur_adc_ops->vbat_vbus_mode(false);

    return vbat_vol * 3;
}

int adc_phy_ops_register(struct adc_phy_ops* ops)
{
    int ret = 0;

    if (!ops) {
        ret = AIC_ENXIO;
        goto out;
    }

    if (cur_adc_ops) {
        ret = AIC_EOVERFLOW;
        goto out;
    }

    cur_adc_ops = ops;

out:
    return ret;
}

