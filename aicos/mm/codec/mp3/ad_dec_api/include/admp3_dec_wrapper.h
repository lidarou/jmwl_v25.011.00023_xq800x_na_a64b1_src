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


#ifndef _ADMP3_CODEC_WRAPPER_H
#define _ADMP3_CODEC_WRAPPER_H
#include <stdint.h>
#include "aicxtek_audio_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MP3_DEC_OUTPUT_SIZE (4608)

/*((samplenum * (1/srate) *bitrate) / 8) + padding
-->1152/8 * (bitrate/srate) + padding
max frame size--> 144 *(maxbitrate/minsrate) + padding=
144 * (320/8) + padding = 5760+padding
= 5761?
does above right? may be check later*/
#define MP3_DEC_IN_MAX_SIZE  (6 * 1024)





typedef enum MP3_ERROR_CODE
{
    MP3_CODEC_SUCCESS = 0,
    MP3_UNSUPPORTED_LAYER = 1,
    MP3_UNSUPPORTED_FREE_BITRATE = 2,
    MP3_FILE_OPEN_ERROR = 3,          /* error opening file */
    MP3_CHANNEL_CONFIG_ERROR = 4,     /* error in channel configuration */
    MP3_SYNTHESIS_WINDOW_ERROR = 5,   /* error in synthesis window table */
    MP3_READ_FILE_ERROR = 6,          /* error reading input file */
    MP3_SIDE_INFO_ERROR = 7,          /* error in side info */
    MP3_HUFFMAN_TABLE_ERROR = 8,      /* error in Huffman table */
    MP3_COMMAND_LINE_ERROR = 9,       /* error in command line */
    MP3_MEMORY_ALLOCATION_ERROR = 10,   /* error allocating memory */
    MP3_NO_ENOUGH_MAIN_DATA_ERROR = 11,
    MP3_SYNCH_LOST_ERROR = 12,
    MP3_OUTPUT_BUFFER_TOO_SMALL = 13     /* output buffer can't hold output */
} MP3_ERROR_CODE;

typedef void (*log_func)(const char *fmt, ...);
typedef int (*read_func)(unsigned int priv, unsigned char *buf, unsigned int size, unsigned int *real_size);
typedef int64_t (*seek_func)(unsigned int priv, int64_t offset, unsigned int mode);


void *admp3_dec_create(memory_api_t *memory_api, int *inparam, unsigned int insize);

void admp3_dec_delete(void *handle);

int admp3_dec_proc(void *handle, char* in_buf, unsigned int in_size8, char* out_buf, unsigned int* io_out_size8);

int admp3_dec_ctrl(void* handle, int param_code, int* p_param, unsigned int param_size32);
#ifdef __cplusplus
}
#endif

#endif


