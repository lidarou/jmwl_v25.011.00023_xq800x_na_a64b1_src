/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio effect headef file.
 *
 *****************************************************************************/
#ifndef __AUDIO_EFFECT_H__
#define __AUDIO_EFFECT_H__

void ae_mix(short *p1, short *p2, u32 cnt);
void ae_mix_lr(short *p, u32 cnt);
void ae_mono_2_stero(short *p, u32 cnt);
void ae_stero_2_mono(short *p, u32 cnt);

#endif /* __AUDIO_EFFECT_H__ */

