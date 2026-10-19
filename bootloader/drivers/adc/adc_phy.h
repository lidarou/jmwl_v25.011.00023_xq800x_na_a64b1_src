/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of adc phy module.
 *   It includes all the adc phy function api.
*/
#ifndef __ADC_PHY_H__
#define __ADC_PHY_H__
#include <aic_bitmap.h>

#define ADC_TOLERANCE           0xA
#define ADC_CHANNEL_MAX         0xF
#define ADC_WAIT_TIME_CNT       100

struct adc_phy_ops {
    u32 vbat_adc_chan;
    int (*adc_get)(u32 chan, u32 *data);
    int (*adc_cali)(void);
    u32 (*adc2voltage)(u32 adc_val);
    void (*adc_enable)(bool enabled);
    bool (*adc_is_enabled)(void);
    void (*vbat_vbus_mode)(bool enabled);
    void (*vbat_vbus_sel)(bool is_vbat);
    int (*adc_calibrate)(void);
};

int adc_phy_ops_init(void);
int adc_phy_ops_register(struct adc_phy_ops* ops);

#endif /* __ADC_PHY_H__ */
