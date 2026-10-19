/*****************************************************************************
 *
 *  Copyright (c) AIC Corporation. All rights reserved.
 *
 *****************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** Export the audio nv operation interface.
 **---------------------------------------------------------------------------*/

#ifndef __AUDIO_NV_H__
#define __AUDIO_NV_H__

#include "aic_type.h"

#define AUDIO_PGA_MAX 15

enum audio_nv_id {
    AUDIO_NVID_CONFIG = 0,
    AUDIO_NVID_DSP,
    AUDIO_NVID_VOICE_PROC,
    AUDIO_NVID_MAX
};

#define NV_STRUCT_BASE  \
    char *name;         \
    u32 index;

struct audio_pga {
    u16 gain[AUDIO_PGA_MAX];
};

struct audio_nv_base {
    NV_STRUCT_BASE
};

struct audio_nv_config {
    NV_STRUCT_BASE
};

struct audio_nv_dsp {
    NV_STRUCT_BASE
};

struct audio_nv_voice_proc {
    NV_STRUCT_BASE
};

u32 audio_nv_get_num(u32 nvid);
int audio_nv_read(u32 nvid, char *name, void *buf);
int audio_nv_write(u32 nvid, char *name, void *buf);

#endif /* __AUDIO_NV_H__ */

