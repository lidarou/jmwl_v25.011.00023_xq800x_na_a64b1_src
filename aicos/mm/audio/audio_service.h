/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio service header file, in which defined MACROs,
 *  structs and APIs for module internal.
 *
 *****************************************************************************/

#ifndef __AUDIO_SERVICE_H__
#define __AUDIO_SERVICE_H__

#include <aic_type.h>
#include "audio_util.h"
#include "audio_sink_record.h"

#define AUDIO_FIX_OUTPUT_SRATE  48000
#ifdef CONFIG_SOUND_CONV
#define AUDIO_FIX_OUTPUT_CHN    1
#else
#define AUDIO_FIX_OUTPUT_CHN    2
#endif

enum audio_cmd {
    AUDIO_CMD_NONE = -1,
    /* 0 - 4 */
    AUDIO_CMD_OPEN,
    AUDIO_CMD_SET_PARAM,
    AUDIO_CMD_PLAY,
    AUDIO_CMD_STOP,
    AUDIO_CMD_PAUSE,
    /* 5 - 9 */
    AUDIO_CMD_RESUME,
    AUDIO_CMD_CLOSE,
    AUDIO_CMD_FORCE_OUT_DEVICE,
    AUDIO_CMD_FORCE_IN_DEVICE,
    AUDIO_CMD_GET_DEVICE,
    /* 10 - 14 */
    AUDIO_CMD_GET_FORCE_DEVICE,
    AUDIO_CMD_SET_VOLUME,
    AUDIO_CMD_GET_VOLUME,
    AUDIO_CMD_SET_MASTER_VOLUME,
    AUDIO_CMD_GET_MASTER_VOLUME,
    /* 15 - 19 */
    AUDIO_CMD_MUTE,
    AUDIO_CMD_UNMUTE,
    AUDIO_CMD_MUTE_ALL,
    AUDIO_CMD_UNMUTE_ALL,
    AUDIO_CMD_SET_VOICE_INFO,
    /* 20 - 24 */
    AUDIO_CMD_GET_TIMESTAMP,
    AUDIO_CMD_USER_CB,
    AUDIO_CMD_MUTE_UL,

    AUDIO_CMD_MAX
};

#define AUDIO_EVENT_DONE            1

/* These flags are used internal */
enum audio_flag_inter {
    AUDIO_FLAG_AP_VOICE =           0x01 << 16,
    AUDIO_FLAG_VOICE_STREAM =       0x01 << 17,
    AUDIO_FLAG_TRANCEIVER =         0x01 << 18,
};

struct audio_service_info {
    aos_thread_id       thread;
    aos_queue_ptr       queue;
    aos_event_ptr       event;
    aos_semaphore_ptr   semph;
    aos_mutex_ptr       mutex;
    u32                 master_volume;
    bool                mute_all;
    bool                mute_ul;
    u32                 force_device;
    u32                 force_in_device;
    u32                 voice_srate;
    void                *queue_addr;
};

struct audio_msg_base {
    enum audio_cmd  cmd;
};

struct audio_rsp_base {
    int result;
};

struct audio_msg_open {
    enum audio_cmd  cmd;
    enum astream_type type;
    enum aplay_type aplay_type;
    enum astream_format format;
    enum astream_channel chn;
    enum astream_source source;
    enum astream_direct dir;
    u32 flag;
    u32 sample_rate;
    u32 bit_rate;
    char *buf;
    ACLIENT aclient;
};

struct audio_open_rsp {
    int result;
    AHANDLE handle;
};

struct audio_msg_set_param {
    enum audio_cmd  cmd;
    AHANDLE handle;
    void *param;
};

struct audio_msg_play {
    enum audio_cmd  cmd;
    AHANDLE handle;
    enum aplay_type aplay_type;
    void *aplay_data;
};

struct audio_play_sub_buf {
    char *buf;
    u32 buf_len;
    u32 srate;
    u32 chn;
    u32 brate;
    void *user_cb;
    u32 cb_data;
};

struct audio_play_sub_file {
    char *file_name;
    u32 srate;
    u32 chn;
    u32 brate;
    void *user_cb;
    u32 cb_data;
};

struct audio_play_sub_id {
    u32 id;
    void *user_cb;
    u32 cb_data;
};

struct audio_play_sub_tone {
    enum tone_type tone_id;
    u32 duation;
};

struct audio_play_sub_tts {
    char *buf;
    u32 buf_len;
    void *user_cb;
    u32 cb_data;
};

struct audio_play_sub_stream {
    struct asink_buf *abuf;
    u32 srate;
    u32 chn;
    u32 brate;
    void *user_cb; /* read more data by this callback */
    u32 cb_data;
};

struct audio_msg_play_tone {
    enum audio_cmd  cmd;
    AHANDLE handle;
    u32 id;
    void *user_cb;
    u32 cb_data;
};

struct audio_msg_stop {
    enum audio_cmd  cmd;
    AHANDLE handle;
};

struct audio_msg_close {
    enum audio_cmd  cmd;
    AHANDLE handle;
};

struct audio_msg_resume {
    enum audio_cmd  cmd;
    AHANDLE handle;
};

struct audio_msg_pause {
    enum audio_cmd  cmd;
    AHANDLE handle;
};

struct audio_msg_set_device {
    enum audio_cmd  cmd;
    u32 device;
};

struct audio_msg_get_device {
    enum audio_cmd  cmd;
};

struct audio_msg_set_volume {
    enum audio_cmd  cmd;
    AHANDLE handle;
    u32 vol;
};

struct audio_msg_get_volume {
    enum audio_cmd  cmd;
    AHANDLE handle;
};

struct audio_msg_mute {
    enum audio_cmd  cmd;
    AHANDLE handle;
};

struct audio_msg_set_master_volume {
    enum audio_cmd  cmd;
    u32 vol;
};

struct audio_msg_get_master_volume {
    enum audio_cmd  cmd;
};

struct audio_msg_mute_all {
    enum audio_cmd  cmd;
};

struct audio_msg_set_voice_info {
    enum audio_cmd  cmd;
};

struct audio_msg_get_timestamp {
    enum audio_cmd  cmd;
    AHANDLE handle;
};

struct audio_msg_user_cb {
    enum audio_cmd  cmd;
    enum astream_cb_event event;
    AHANDLE handle;
};

struct audio_msg_mute_ul {
    enum audio_cmd  cmd;
    u32 is_mute;
};

int audio_bind(void *msg, void *ret_data, int wait_time);
int audio_send_msg(void *msg, void *ret_data, int wait_time);

int audio_get_voice_rate(void);
int audio_set_voice_rate(u32 srate, u32 brate);
int audio_svc_master_volume(void);
bool audio_is_mute_ul(void);

#endif /* __AUDIO_SERVICE_H__ */

