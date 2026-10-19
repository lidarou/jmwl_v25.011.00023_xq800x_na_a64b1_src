/*
 * Copyright (C) 2014 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef MP3READER_H_
#define MP3READER_H_
#include <stdio.h>
#include <aic_type.h>
#include "codec_mng.h"


struct ad_mp3reader_ops
{
    codec_get read_func;
    codec_seek seek_func;
};

struct reader_extra_data {
    uint32_t size;
    u8 *data;
};

struct Mp3Reader {
    u32 priv;
    struct ad_mp3reader_ops ops;
    uint8_t *sync_buf;
    uint32_t sync_buf_size;
    uint8_t *back_buf;
    uint32_t back_buf_size;
    uint32_t back_buf_totalsize;
    uint32_t back_buf_offset;
    uint32_t mFixedHeader;
    int64_t  mCurrentPos;
    uint32_t mSampleRate;
    uint32_t mNumChannels;
    uint32_t mBitrate;
};

void *ad_mp3reader_init(u32 priv, struct ad_mp3reader_ops *ops, struct reader_extra_data *extra_info);
int ad_mp3reader_getframe(void* obj, void *buffer, uint32_t insize, uint32_t *size);
void ad_mp3reader_deinit(void *obj);



#endif /* MP3READER_H_ */
