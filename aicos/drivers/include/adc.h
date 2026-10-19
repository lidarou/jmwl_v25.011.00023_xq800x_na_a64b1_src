/*****************************************************************************/
/*                                                                           */
/*         Copyright 2022 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of adc module.
 *   It includes all the adc function api.
*/
#ifndef __ADC_H__
#define __ADC_H__

int adc_data_get(u32 chan, u32 *data);
int adc_chan_voltage_get(u32 chan, u32 *voltage_mv);
u32 vbat_vol_mv_get(void);
u32 vbus_vol_mv_get(void);

#endif /* __TSC_H__ */
