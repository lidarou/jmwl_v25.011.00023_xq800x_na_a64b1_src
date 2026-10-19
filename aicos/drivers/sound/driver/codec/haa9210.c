/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file haa9210.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e83a3
#define LOG_MODULE    AMOD_SOUND

#include <aic_type.h>
#include <aic_init.h>
#include <aos_thread.h>
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

/**********************
 *   STATIC FUNCTIONS
 **********************/
static int audio_pa_on(u32 data)
{
    aic_gpio_request(GPIO_AON_15);
    aic_gpio_output(GPIO_AON_15, GPIO_OUTPUT_LVL_HIGH);

    return 0;
}
static int audio_pa_off(u32 data)
{
    aic_gpio_request(GPIO_AON_15);
    aic_gpio_output(GPIO_AON_15, GPIO_OUTPUT_LVL_LOW);

    /* sleep 1ms to resolve pop audio */
    aos_sleep(1);

    return 0;
}

static int haa9210_pa_init(void)
{
    snd_ext_pa_reg(audio_pa_on, audio_pa_off, 0);

    return 0;
}

AIC_NORMAL_DRV_L2_INIT_EXPORT(haa9210_pa_init);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

