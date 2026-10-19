/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the adc demo for user
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0fe432
#define LOG_MODULE    AMOD_PMIC

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <adc.h>
#include <timer_inc.h>

void adc_channel_scan(void *input)
{
    u32 voltage_mv;
    int i, ret;

    while(1)
        for (i = 0; i < 0xF; i++) {
            ret = adc_chan_voltage_get(i, &voltage_mv);
            if (ret)
                aicx_warn(0x0a2c49, "d", "get channel:%d adc value failed!!\n", i);
            else
                aicx_warn(0x155946, "dd", "channel:%d voltage_mv:%d\n", i, voltage_mv);

            aos_sleep(2000);
        }
}

static int adc_demo_init(void)
{
    aos_create_thread("adc_demo", adc_channel_scan, AOS_NULL, 1024, 8,
                      "adc_demo", 16, AOS_AUTO_START);

    return 0;
}
AIC_NORMAL_APP_L1_INIT_EXPORT(adc_demo_init);
