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
#define AIC_THIS_FILE    0x131dfb
#define LOG_MODULE    AMOD_AUDIO

#include  <stdlib.h>
#include <stdint.h>
#include "admp3_dec_wrapper.h"
#include "pvmp3decoder_api.h"
#include "aicxtek_audio_api.h"
#include <alog_api.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#if 0
void  alog_output(unsigned char mode, unsigned char level, const char* fmt, ...);
static char buff[255];// = "yzl add for test";

void alog_output2(const char* fmt, ...)
{
    va_list arg;
    unsigned char mode = 16, level = 1;
    va_start(arg, fmt);
    vsnprintf(buff, 255, fmt, arg);
    alog_output(mode, level, "%s", buff);
    va_end(arg);
    return;
}
#endif
struct mp3_dec_res
{
    tPVMP3DecoderExternal config;
    void *decoderbuf;
    memory_malloc *mp3_malloc;
    memory_free *mp3_free;
    log_func* log_api;
};

void *admp3_dec_create(memory_api_t* memory_api, int* inparam, unsigned int insize)
{
    struct mp3_dec_res *dec_res;
    uint32_t memRequirements = pvmp3_decoderMemRequirements();
    void* decoderbuf;

    if ((memory_api->pf_heap_free == NULL)
        || (memory_api->pf_heap_malloc == NULL))
    {
        goto _error_param;;
    }
    dec_res = memory_api->pf_heap_malloc(NULL, sizeof(struct mp3_dec_res) + memRequirements);
    if ((!dec_res)) {
        goto _error_alloc;
    }
    decoderbuf = (uint8_t *)dec_res + sizeof(struct mp3_dec_res);

    dec_res->config.equalizerType = flat;
    dec_res->config.crcEnabled = 0;
    dec_res->log_api = NULL;
    dec_res->mp3_malloc = memory_api->pf_heap_malloc;
    dec_res->mp3_free = memory_api->pf_heap_free;
    pvmp3_InitDecoder(&dec_res->config, decoderbuf);
    dec_res->decoderbuf = decoderbuf;

    return dec_res;

_error_alloc:
_error_param:
    return NULL;
}

void admp3_dec_delete(void *handle)
{
    struct mp3_dec_res *dec_res = (struct mp3_dec_res *) handle;

    if (dec_res) {
        dec_res->mp3_free(NULL, dec_res);
    }
}

int admp3_dec_proc(void *handle, char* in_buf, unsigned int in_size8, char* out_buf, unsigned int* io_out_size8)
{
    struct mp3_dec_res *dec_res = (struct mp3_dec_res *) handle;
    int ret;
    uint32_t consumed = 0;
    uint32_t out_buf_size = *io_out_size8;

    *io_out_size8 = 0;
    while (in_size8 > consumed) {
        dec_res->config.inputBufferCurrentLength = in_size8 - consumed;
        dec_res->config.inputBufferMaxLength = 0;
        dec_res->config.inputBufferUsedLength = 0;
        dec_res->config.pInputBuffer = in_buf + consumed;
        dec_res->config.pOutputBuffer = (int16_t*)out_buf + (*io_out_size8 / sizeof(int16_t));
        dec_res->config.outputFrameSize = (out_buf_size - *io_out_size8) / sizeof(int16_t);//MP3_DEC_OUTPUT_SIZE / sizeof(int16_t);
        if ((*io_out_size8 + dec_res->config.outputFrameSize * sizeof(int16_t)) > out_buf_size) {
            //ALOGW("may overflow output buffer");
//            alog_output2("yzl add error too small:%d, %d", (*io_out_size8 + dec_res->config.outputFrameSize * sizeof(int16_t)),
//                out_buf_size);
            return MP3_OUTPUT_BUFFER_TOO_SMALL;
        }

        ret = pvmp3_framedecoder(&dec_res->config, dec_res->decoderbuf);

        if (ret != NO_DECODING_ERROR) {
            //ALOGW("mp3_dec_proc decode error ret:%d", ret);
            goto _err_return;
        }
        consumed += dec_res->config.inputBufferUsedLength;
        *io_out_size8 += dec_res->config.outputFrameSize * sizeof(int16_t);
//       alog_output2("yzl add dec outsize:%d, consumed:%d", *io_out_size8, consumed);
    }
    return MP3_CODEC_SUCCESS;

_err_return:
    return ret;
}

int admp3_dec_ctrl(void* handle, int param_code, int* p_param, unsigned int param_size32)
{

    return MP3_CODEC_SUCCESS;
}


