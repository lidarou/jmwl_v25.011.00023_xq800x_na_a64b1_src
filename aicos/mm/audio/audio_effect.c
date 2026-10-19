/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio effect module, it provided audio effect process API
 *  for audio module internal.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b4355
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>

void ae_mix(short *p1, short *p2, u32 cnt)
{
    if (!p1 || !p2)
        return;

    while(cnt--) {
        *p1 = ((s32)(*p1) + (s32)(*p2)) >> 1;
        p1++;
        p2++;
    }
}

void ae_mix_lr(short *p, u32 cnt)
{
    u32 frm = cnt >> 1;
    short *p1, *p2;

    if (!p)
        return;

    p1 = p;
    p2 = p + 1;
    while(frm--) {
        *p1 = ((s32)(*p1) + (s32)(*p2)) >> 1;
        *p2 = *p1;
        p1 += 2;
        p2 += 2;
    }
}

void ae_mono_2_stero(short *p, u32 cnt)
{
    short *p1, *p2;

    if (!p)
        return;

    p1 = p + (cnt - 1);
    p2 = p + ((cnt << 1) - 1);
    while (cnt--) {
        *p2-- = *p1;
        *p2-- = *p1--;
    }
}

void ae_stero_2_mono(short *p, u32 cnt)
{
    short *p1;
    u32 i = 0;

    if (!p)
        return;
    p1 = p;
    while (i++ < cnt / 2) {
        *p++ = ((s32)(*p1) + (s32)(*(p1 + 1))) >> 1;
        p1 += 2;
    }
}

