/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is about audio dump.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x172294
#define LOG_MODULE    AMOD_AUDIO

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_cmd_api.h>
#include <aic_init.h>
#include <alog_api.h>

#include "audio_util.h"

#define AUDIO_DUMP_POS_MAX  31

static u32 g_audio_dump_pos;

extern int adsp_dump_set(u32 en, u32 pos);

void audio_dump(void *buf, u32 size, u32 pos)
{
    if (!buf || !size)
        return;

    if (pos > AUDIO_DUMP_POS_MAX) {
        AIDXE(0x04086c, "d", "Max pos is %d\n", AUDIO_DUMP_POS_MAX);
        return;
    }

    /* check if pos is on */
    if (!(g_audio_dump_pos & (1 << pos)))
        return;

    alog_data_ex((u8 *)buf, size, ALOG_DATA_AUD, pos, 0);
}

static int adump(int argc, char **argv)
{
    u32 pos;
    int is_on;

    if (argc != 2)
        return AIC_EINVAL;

    if (!argv[0] || !argv[1])
        return AIC_EINVAL;

    is_on = atoi(argv[0]);
    pos = atoi(argv[1]);

    /* TODO: add lock */
    if (is_on)
        g_audio_dump_pos |= pos;
    else
        g_audio_dump_pos &= ~pos;

    if (g_audio_dump_pos)
        CONS_OUT("audio dump current pos: 0x%x\n", g_audio_dump_pos);
    else
        CONS_OUT("audio dump all points are closed!n");

    return AIC_SUCCESS;
}
CONSOLE_EXPORT_CMD(adump, adump, adump test);

#ifdef CONFIG_ADSP_DUMP

static int adspdump(int argc, char **argv)
{
    u32 pos;
    int is_on;

    if (argc != 2)
        return AIC_EINVAL;

    if (!argv[0] || !argv[1])
        return AIC_EINVAL;

    is_on = atoi(argv[0]);
    pos = atoi(argv[1]);

    adsp_dump_set(is_on, pos);

    return 0;
}
CONSOLE_EXPORT_CMD(adspdump, adspdump, adspdump test);

#endif
