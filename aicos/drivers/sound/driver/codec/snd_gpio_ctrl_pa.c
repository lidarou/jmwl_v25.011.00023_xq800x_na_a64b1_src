/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file snd_gpio_ctrl_pa.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0878d2
#define LOG_MODULE    AMOD_SOUND

#include <aic_type.h>
#include <aic_init.h>
#include <aos_thread.h>
#include <audio/snd_config.h>
#include <core_op.h>

#include "snd_dpm.h"
#include "gpio_inc.h"
#include "snd_ext_pa.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static u32 g_pa_mode;
static BOOL g_is_pa_on;
/**********************
 *   STATIC FUNCTIONS
 **********************/
static int audio_pa_on(u32 data)
{
    u32 cnt;

    if (SND_PA_PMU_PRE_TIME)
        aos_sleep(SND_PA_PMU_PRE_TIME);

    if (g_pa_mode) {
        /* disable int to make pulse time precise */
        AIC_INT_SAVE_AREA
        AIC_INT_DISABLE

        /* run gpio output and ndelay first to save time by cache */
        aic_gpio_output_raw(SND_PA_CTRL_GPIO_NUM, !SND_PA_CTRL_ON_GPIO_LEVEL);
        aic_ndelay(1000);
        aic_gpio_output_raw(SND_PA_CTRL_GPIO_NUM, !SND_PA_CTRL_ON_GPIO_LEVEL);
        aic_ndelay(1000);
        cnt = g_pa_mode;
        do {
            aic_gpio_output_raw(SND_PA_CTRL_GPIO_NUM, SND_PA_CTRL_ON_GPIO_LEVEL);
            aic_ndelay(SND_PA_MODE_HIGH_TIME * 1000);
            aic_gpio_output_raw(SND_PA_CTRL_GPIO_NUM, !SND_PA_CTRL_ON_GPIO_LEVEL);
            aic_ndelay(SND_PA_MODE_LOW_TIME * 1000);
        } while(--cnt);
        aic_gpio_output_raw(SND_PA_CTRL_GPIO_NUM, SND_PA_CTRL_ON_GPIO_LEVEL);

        AIC_INT_RESTORE
    }
    else
        aic_gpio_output_raw(SND_PA_CTRL_GPIO_NUM, SND_PA_CTRL_ON_GPIO_LEVEL);

    if (SND_PA_PMU_POST_TIME)
        aos_sleep(SND_PA_PMU_POST_TIME);

    g_is_pa_on = TRUE;
    SNX_INFO(0x0cccde, "d", "pa mode %d\n", g_pa_mode);

    return 0;
}
static int audio_pa_off(u32 data)
{
    if (SND_PA_PMD_PRE_TIME)
        aos_sleep(SND_PA_PMD_PRE_TIME);
    aic_gpio_request(SND_PA_CTRL_GPIO_NUM);
    aic_gpio_output(SND_PA_CTRL_GPIO_NUM, !SND_PA_CTRL_ON_GPIO_LEVEL);
    if (SND_PA_PMD_POST_TIME)
        aos_sleep(SND_PA_PMD_POST_TIME);

    g_is_pa_on = FALSE;

    return 0;
}

#if defined(CONFIG_SOUND_PA_CODEC_MASTER) || defined (CONFIG_SOUND_PA_CODEC_AUX)
static int gpio_pa_event(struct snd_dpm_context *dpm, int event)
{
    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        audio_pa_on(dpm->codec->private);
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        audio_pa_off(dpm->codec->private);
        break;
    default:
        break;
    }

    return 0;
}

static int pa_mode_get(struct snd_kcontrol *kcontrol,
                       struct snd_ctl_elem_value *ucontrol)
{
    ucontrol->value.integer.value[0] = g_pa_mode;

    return 0;
};

static int pa_mode_put(struct snd_kcontrol *kcontrol,
                       struct snd_ctl_elem_value *ucontrol)
{
    g_pa_mode = ucontrol->value.integer.value[0];
    if (g_is_pa_on) {
        audio_pa_off(0);
        audio_pa_on(0);
    }
    SNX_INFO(0x0a1512, "dd", "pa mode = %d, pa on = %d\n", g_pa_mode, g_is_pa_on);

    return 0;
};

static const struct snd_kcontrol_new pa_snd_controls[] = {
    SOC_SINGLE_EXT("PA mode", 0, 0, 10, 0, pa_mode_get, pa_mode_put)
};

static struct snd_codec_driver gpio_codec_driver = {
    .stream_event = gpio_pa_event,
    .controls = pa_snd_controls,
    .num_controls = ARRAY_SIZE(pa_snd_controls),
};
#endif

static int gpio_ctrl_pa_init(void)
{
    snd_ext_pa_reg(audio_pa_on, audio_pa_off, 0);
#ifdef CONFIG_SOUND_PA_CODEC_MASTER
    snd_card_register_codec("gpio codec", &gpio_codec_driver);
#elif defined (CONFIG_SOUND_PA_CODEC_AUX)
    snd_card_register_aux_codec("gpio codec", &gpio_codec_driver);
#endif

    return 0;
}

AIC_NORMAL_DRV_L1_INIT_EXPORT(gpio_ctrl_pa_init);

