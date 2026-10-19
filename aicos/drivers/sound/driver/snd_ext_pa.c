/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines external PA general API.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d9e0c
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <stdlib.h>
#include <aic_assert.h>
#include <aic_type.h>
#include <aic_bitmap.h>
#include <aic_errno.h>

#include "snd_card.h"

struct snd_ext_pa {
    u32 data;
    int (*pa_on)(u32 data);
    int (*pa_off)(u32 data);
};

struct snd_ext_pa g_ext_pa;

int snd_ext_pa_on(void)
{
    if (g_ext_pa.pa_on)
        return g_ext_pa.pa_on(g_ext_pa.data);

    return 0;
}

int snd_ext_pa_off(void)
{
    if (g_ext_pa.pa_off)
        return g_ext_pa.pa_off(g_ext_pa.data);

    return 0;
}

int snd_ext_pa_reg(int (*pa_on)(u32), int (*pa_off)(u32), u32 data)
{
    g_ext_pa.pa_on = pa_on;
    g_ext_pa.pa_off = pa_off;
    g_ext_pa.data = data;

    return 0;
}
