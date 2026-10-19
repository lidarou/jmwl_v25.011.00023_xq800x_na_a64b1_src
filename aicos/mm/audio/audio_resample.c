/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is audio resample wrapper.
 *
 */

#define AIC_THIS_FILE    0x0e43af
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_errno.h>

#include "aic_resampler_api.h"
#include "audio_util.h"

static void* rsmpl_malloc(void* mem_state, unsigned int size8)
{
    return audio_malloc(size8);
}

static void rsmpl_free(void* mem_state, void* ptr)
{
    audio_free(ptr);
}

void *audio_rsmpl_open(u32 in_sr, u32 out_sr, u32 chn, u32 frm_num)
{
    AICInit_t param;
    memory_api_t mem_api;

    param.fs_i = in_sr;
    param.fs_o = out_sr;
    param.ch_i = chn;
    param.ch_o = chn;
    param.in_samplenum = frm_num;
    param.log_param.log_func = 0;

    mem_api.heap_state = NULL;
    mem_api.pf_heap_malloc = rsmpl_malloc;
    mem_api.pf_heap_free = rsmpl_free;

    return resampler_init(&mem_api, &param);
}

u32 audio_rsmpl_get_out_num(void *handle)
{
    int ret;
    u32 num;

    ret = resampler_control(handle, AIC_RESAMPLER_GET_MAXOUTSIZE, 0, &num);
    if (ret) {
        AIDXE(0x0ea0d8, "d", "get rsmpl out num error, ret=%d\n", ret);
        return 0;
    }

    return num;
}

int audio_rsmpl_run(void *handle, s16 *in, u32 in_num, s16 *out, u32 *out_num)
{
    return resampler_process(handle, in, in_num, out, out_num);
}

void audio_rsmpl_close(void *handle)
{
    resampler_deinit(handle);
}

