/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is codec header file, codec related struct and
 *  exported APIs are declared here.
 *
 *****************************************************************************/

#ifndef __CODEC_MNG_H__
#define __CODEC_MNG_H__

#include <aic_list.h>
#include <aic_type.h>

enum codec_type {
    CODEC_TYPE_NONE = 0,
    CODEC_TYPE_WAV,
    CODEC_TYPE_MIDI,
    CODEC_TYPE_MP3,
    CODEC_TYPE_AAC,
    CODEC_TYPE_AMR
};

#define CODEC_DEC       0
#define CODEC_ENC       1

enum codec_seek_mode {
    CODEC_SEEK_BEGIN = 0,
    CODEC_SEEK_CUR,
    CODEC_SEEK_END,
    CODEC_SEEK_SIZE = 0x1000,
};

enum codec_info_flag {
    CODEC_INFO_NORMAL = 0,
    CODEC_INFO_NO_HEADER = 0x1 << 1,
    CODEC_INFO_FRAME_MODE = 0x1 << 2,
    CODEC_INFO_DTX = 0x1 << 3,
};

typedef int (*codec_obj_cb)(void *engine, u32 cmd, u32 data);

typedef int (*codec_get)(u32 priv, u8 *buf, u32 cnt, u32 *real_cnt);
typedef int (*codec_send)(u32 priv, u8 *buf, u32 cnt);
typedef s64 (*codec_seek)(u32 priv, s64 offset, u32 mode);

struct codec_ops {
    codec_get get_func;
    codec_send send_func;
    codec_seek seek_func;
};

struct codec_info {
    u32 srate;
    u32 brate;
    u16 chn;
    u16 wid;
    u16 fmt;
    u32 seek_timestamp;
    u32 duration;
    u32 flag;
};

struct codec_inf {
    int (*codec_get)(u32 priv, u8 *buf, u32 cnt, u32 *real_cnt);
    int (*codec_send)(u32 priv, u8 *buf, u32 cnt);
    s64 (*codec_seek)(u32 priv, s64 offset, u32 mode);
    u32 priv;
};

struct codec_engine {
    struct list_head list;
    enum codec_type type;
    u32 dir;

    u32 (*open)(u32 priv, struct codec_ops *codec_ops, struct codec_info *info);
    int (*close)(u32 engine);
    int (*start)(u32 engine);
    int (*stop)(u32 engine);
    int (*pause)(u32 engine);
    int (*resume)(u32 engine);
    int (*get_data)(u32 engine, short *pbuf, u32 count, u32 *real_count);
    int (*send_data)(u32 engine, short *pbuf, u32 count);
    int (*seek)(u32 engine, u32 pos);
    int (*set_gain)(u32 engine, u32 gain);
    int (*set_speed)(u32 engine, u32 speed);
    int (*get_seek_time)(u32 engine, struct codec_info *info);
    int (*get_header)(u32 engine, u8 **buf, u32 *size);
    int (*get_tail)(u32 engine, u8 **buf, u32 *size);
};

void codec_register(struct codec_engine *engine);
struct codec_engine *codec_get_by_type(enum codec_type type, int dec_enc);

#endif /* __CODEC_MNG_H__ */

