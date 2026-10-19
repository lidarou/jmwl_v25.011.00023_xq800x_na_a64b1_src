/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------
; INCLUDES
----------------------------------------------------------------------------*/
#define AIC_THIS_FILE    0x18736b
#define LOG_MODULE    AMOD_AUDIO

#include  <stdlib.h>
#include <alog_api.h>

#include "audio_util.h"
#include "mp3_dec_api.h"
#include "admp3_dec_wrapper.h"
#include "mp3reader.h"
#include "audio_client.h"

struct mp3_dec_res
{
    void *src_buf;
    void *reader;
    void *codec_handle;
};

static void *adcodec_malloc(void *state, unsigned int size)
{
    return audio_malloc(size);
}

static void adcodec_free(void *state, void *ptr)
{
    audio_free(ptr);
}

void *mp3_dec_create(void *inparam)
{
    struct mp3_dec_res *dec_res;
    void *reader, *srcbuf;
    struct mp3_dec_inparam *init_param = (struct mp3_dec_inparam *) inparam;
    memory_api_t memory_api;

    memory_api.pf_heap_free = adcodec_free;
    memory_api.pf_heap_malloc = adcodec_malloc;
    dec_res = malloc(sizeof(struct mp3_dec_res) + MP3_DEC_IN_MAX_SIZE);
    srcbuf = (uint8_t *)dec_res + sizeof(struct mp3_dec_res);
    if (!dec_res) {
        goto _error_alloc;
    }
    dec_res->src_buf = srcbuf;

    dec_res->codec_handle = admp3_dec_create(&memory_api, NULL , 0);
    if (!dec_res->codec_handle) {
        ALOGE("admp3_dec_create error");
        goto _error_codechandle;
    }
    /*init reader*/
    reader = ad_mp3reader_init(init_param->priv, &init_param->reader_ops, &init_param->reader_ext_info);
    if (!reader) {
        ALOGE("ad_mp3reader_init error!");
        goto _error_reader_init;
    }
    dec_res->reader = reader;

    return dec_res;

_error_reader_init:
    admp3_dec_delete(dec_res->codec_handle);
_error_codechandle:
    free(dec_res);
_error_alloc:
    return NULL;
}

void mp3_dec_delete(void *handle)
{
    struct mp3_dec_res *dec_res = (struct mp3_dec_res *) handle;

    if(dec_res) {
        ad_mp3reader_deinit(dec_res->reader);
        admp3_dec_delete(dec_res->codec_handle);
        free(dec_res);
    }
}

int mp3_dec_proc(void *handle, struct mp3_dec_procparam *param)
{
    struct mp3_dec_res *dec_res = (struct mp3_dec_res *) handle;
    int ret;
    unsigned int dec_out_size = 0;
    u8 eos_flag = 0;
    uint32_t real_size;

    param->outsize = 0;
    ret = ad_mp3reader_getframe(dec_res->reader, dec_res->src_buf, MP3_DEC_IN_MAX_SIZE, &real_size);
    if (ret) {
        ALOGW("mp3_dec_proc get frame error");
        if(ret != ASTREAM_DATA_EOS) {
            return MP3_ERROR;
        }
        ALOGW("mp3 read frame eos read size:%d", real_size);
        eos_flag = 1;
        if(real_size == 0) {
            goto _eos_return;
        }

    }
    dec_out_size = MP3_DEC_OUTPUT_SIZE;
    ret = admp3_dec_proc(dec_res->codec_handle, (char *)dec_res->src_buf, real_size, (char *)param->outbuf, &dec_out_size);

    if(ret != MP3_CODEC_SUCCESS) {
        ALOGW("mp3_dec_proc decode error ret:%d", ret);
        goto _err_return;
    }
    param->outsize = dec_out_size;

    if (eos_flag)
        goto _eos_return;

    return MP3_SUCCESS;

_err_return:
    if(eos_flag)
        goto _eos_return;
    return MP3_ERROR;
_eos_return:
    return MP3_STREAM_EOS;

}

int mp3_dec_ctrl(void *handle, int32_t cmd, void *param)
{
    struct mp3_dec_res *dec_res = (struct mp3_dec_res *) handle;

    switch (cmd) {
    case MP3_DEC_CTRL_CMD_SET_POS:
    {
        struct Mp3Reader *reader;
        reader = (struct Mp3Reader*)dec_res->reader;
        reader->mCurrentPos = *((int64_t *)param);
        ALOGI("set mcurrent pos:%lld", reader->mCurrentPos);
    }
        break;
    default:
        return MP3_ERROR;
    }

    return MP3_SUCCESS;
}


