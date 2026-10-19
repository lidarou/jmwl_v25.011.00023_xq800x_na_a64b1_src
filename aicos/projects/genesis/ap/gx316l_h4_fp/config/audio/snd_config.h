/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is sound driver config file, all configs related different
 *    project are defined here.
 */

#ifndef __SND_CONFIG_H__

#ifdef CONFIG_SOUND_CODEC_ES8311
#define SND_CODEC_DEFALT_ADC_GAIN   0xBF
#define SND_CODEC_DEFALT_DAC_GAIN   0xBF
#define SND_CODEC_DMIC_EN           0
#define SND_CODEC_MCLK_INV          0
#define SND_CODEC_SCLK_INV          0

#define ES8311_I2C_SLAVE_ADDR       0x18

#elif defined (CONFIG_SOUND_CODEC_ES8374)
#define SND_CODEC_DEFALT_ADC_GAIN   0xBF
#define SND_CODEC_DEFALT_DAC_GAIN   0xBF
#define SND_CODEC_DMIC_EN           0
#define SND_CODEC_MCLK_INV          0
#define SND_CODEC_SCLK_INV          0

#else
#define SND_CODEC_DEFALT_ADC_GAIN   0
#define SND_CODEC_DEFALT_DAC_GAIN   0
#define SND_CODEC_DMIC_EN           0
#define SND_CODEC_MCLK_INV          0
#define SND_CODEC_SCLK_INV          0

#endif

#define SND_CODEC_MCLK              24576000
#define SND_CODEC_I2C_ID            3

#define SND_PA_CTRL_GPIO_NUM        15
#define SND_PA_CTRL_ON_GPIO_LEVEL   1

#define SND_PMU_CODEC               "DVDD3V"
#define SND_PMU_CODEC_IO            NULL
#define SND_PMU_CODEC_MICBIAS       "VDDMIC"

#define SND_PA_PMU_PRE_TIME         0
#define SND_PA_PMU_POST_TIME        10

#define SND_PA_PMD_PRE_TIME         0
#define SND_PA_PMD_POST_TIME        10

#define SND_PA_MODE_PRE_TIME        20  /* ms */
#define SND_PA_MODE_LOW_TIME        70  /* us */
#define SND_PA_MODE_HIGH_TIME       70  /* us */

#endif /* __SND_CONFIG_H__ */


