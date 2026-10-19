/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio sink module header file, it defined related data
 *  struct and API used for audio service.
 *
 *****************************************************************************/

#ifndef __AUDIO_SINK_H__
#define __AUDIO_SINK_H__

#include <aic_type.h>
#include "audio_system.h"
#include "audio_client.h"
#include "audio_util.h"
#include "audio_sink_record.h"
#include "audio_stream.h"

enum store_type {
    STORE_TYPE_BUF = 0,
    STORE_TYPE_FILE,
    STORE_TYPE_ID
};

enum astream_state {
    ASTREAM_IDLE,
    ASTREAM_OPEN,
    ASTREAM_PLAYING,
    ASTREAM_PAUSE,
    ASTREAM_STOPPING
};

struct astream_runtime {
    AHANDLE handle;
    int stream_dev;

    struct asink_buf *abuf;

    struct astream_property *property;
    struct apolicy_stream_config *config;
    struct stream_config hw_config;
    void *hrsmpl;
    void *hae;
    char *data_buf;
    u32 data_len;
    char *file_name;
    u32 pos;
    enum store_type store_type;
    enum astream_state state;

    enum astream_type type;
    enum aplay_type aplay_type;
    enum astream_format format;
    enum astream_source source;
    enum astream_channel chn;
    enum astream_direct direct;
    u32 flag;
    u32 sample_rate;

    ACLIENT aclient;
    u32 client_data;

    astream_user_cb user_cb;
    u32 cb_data;

    void *stack;
    aos_thread_id thread;
    aos_event_ptr event;
    aos_mutex_ptr lock;
};

AHANDLE audio_sink_open(ACLIENT aclient, enum astream_type type,
                        enum aplay_type aplay_type,
                        u32 flag, enum astream_format format, u32 sample_rate,
                        u32 bit_rate, enum astream_channel chn,
                        enum astream_direct dir);
int audio_sink_set_param(AHANDLE handle, void *param);
int audio_sink_start(AHANDLE handle, enum aplay_type aplay_type,
                     void *aplay_data);
int audio_sink_pause(AHANDLE handle);
int audio_sink_resume(AHANDLE handle);
int audio_sink_stop(AHANDLE handle);
int audio_sink_close(AHANDLE handle);
u32 audio_sink_get_time(AHANDLE handle);
int audio_sink_user_cb(AHANDLE handle, enum astream_cb_event event);
struct asink_buf *audio_sink_get_buf(AHANDLE handle);
#ifdef CONFIG_VOICE_STREAM
int audio_get_voice_dl_dev(void);
#endif
#ifdef CONFIG_PLAYBACK_EFFECT
int audio_sink_param_update(void *param);
#endif

#endif /* __AUDIO_SINK_H__ */

