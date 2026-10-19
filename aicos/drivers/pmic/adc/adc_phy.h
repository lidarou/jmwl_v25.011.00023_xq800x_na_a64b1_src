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
#define ADC_CHANNEL_MAX         12
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
};

/*
int aic_adc_chan_data_get(u32 chan, u32 *data);
int aic_adc_calibrate(void);
u32 aic_adc2voltage(u32 adc_val);
void aic_adc_enable(bool enable);
bool aic_adc_is_enabled(void);
void vcc5v_test_mode(bool enabled);
void vcc5v_source_sel(bool is_vbat);
*/

int adc_phy_ops_register(struct adc_phy_ops* ops);
#endif /* __ADC_PHY_H__ */
