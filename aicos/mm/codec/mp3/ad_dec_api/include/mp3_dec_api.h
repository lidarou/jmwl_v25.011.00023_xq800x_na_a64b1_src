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


#ifndef _MP3_API_H
#define _MP3_API_H
#include "admp3_dec_wrapper.h"
#include "mp3reader.h"

#ifdef __cplusplus
extern "C"
{
#endif



enum {
    MP3_SUCCESS,
    MP3_STREAM_EOS,
    MP3_ERROR,
};

enum {
    MP3_DEC_CTRL_CMD_SET_POS,
};

typedef struct mp3_dec_inparam
{
    u32 priv;
    struct ad_mp3reader_ops reader_ops;
    struct reader_extra_data reader_ext_info;
}mp3_dec_inparam_t;

typedef struct mp3_dec_procparam
{
    u8 *inbuf;
    u32 insize;
    int16_t *outbuf;
    u32 outsize;

}mp3_dec_procparam_t;

void *mp3_dec_create(void *inparam);

void mp3_dec_delete(void *handle);

int mp3_dec_proc(void *handle, struct mp3_dec_procparam *param);

int mp3_dec_ctrl(void *handle, int32_t cmd, void *param);
#ifdef __cplusplus
}
#endif

#endif


