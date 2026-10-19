/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the wrapper of audio arec.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09b6e1
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <alog_api.h>
#include <aic_errno.h>

#include "audio_util.h"
#include "audio_nrec.h"

static void* audio_nrec_malloc(void* mem_state, unsigned int size8)
{
    return audio_malloc(size8);
}

static void audio_nrec_free(void* mem_state, void* ptr)
{
    audio_free(ptr);
}

void *audio_ec_create(u32 srate, u32 chn, u32 wid)
{
    memory_api_t mem_api = {0};
    voice_proc_param_t param = {0};

    mem_api.heap_state = NULL;
    mem_api.pf_heap_malloc = audio_nrec_malloc;
    mem_api.pf_heap_free = audio_nrec_free;
    param.chn = chn;
    param.fs = srate;

    return aic_voice_create(&mem_api, &param, 0);
}

int audio_ec_proc(void *haec, char *in_buf, char *ref_buf, u32 in_size,
                  char *out_buf, u32 *out_size)
{
    return aic_voice_process(haec, in_buf, ref_buf, in_size,
                             out_buf, out_size);
}

int audio_ec_control(void *haec, u32 ctl_cmd, void *param)
{
    return aic_voice_control(haec, ctl_cmd, param, 0);
}

void audio_ec_delete(void *haec)
{
    aic_voice_delete(haec);
}

void *audio_nr_create(u32 srate, u32 chn, u32 wid)
{
    memory_api_t mem_api = {0};
    voice_proc_param_t param = {0};

    mem_api.heap_state = NULL;
    mem_api.pf_heap_malloc = audio_nrec_malloc;
    mem_api.pf_heap_free = audio_nrec_free;
    param.chn = chn;
    param.fs = srate;

    return aic_playback_create(&mem_api, &param, 0);
}

int audio_nr_proc(void *hanr, char *in_buf, u32 in_size,
                  char *out_buf, u32 *out_size)
{
    return aic_playback_process(hanr, in_buf, in_size, out_buf, out_size);
}

void audio_nr_delete(void *hanr)
{
    aic_playback_delete(hanr);
}

int audio_nr_control(void *haec, u32 ctl_cmd, void *param)
{
    return aic_playback_control(haec, ctl_cmd, param, 0);
}

