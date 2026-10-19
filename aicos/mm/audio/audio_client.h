/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio client header file, audio client manage struct and
 *  exported APIs are declared.
 *
 *****************************************************************************/

#ifndef __AUDIO_CLIENT_H__
#define __AUDIO_CLIENT_H__

#include <aic_type.h>

#include "aic_list.h"
#include "audio_system.h"

#define AUDIO_CLIENT_CMD_GET_MORE           1
#define AUDIO_CLIENT_CMD_DATA_NOTIFY        2

#define ASTREAM_DATA_EOS 0x1
#define ASTREAM_DATA_NODATA 0x2
#define ASTREAM_FRM_EOS 0x3

typedef int (*audio_client_cb)(u32 obj, int event, char *buffer, u32 *size);

struct audio_client {
    struct list_head list;
    enum astream_type type;
    enum aplay_type aplay_type;
    audio_client_cb cb;

    u32 (*open)(enum astream_format format, void *msg);
    int (*set_params)(u32 obj, void *params);
    int (*start)(u32 obj, void *aplay_data);
    int (*stop)(u32 obj);
    int (*pause)(u32 obj);
    int (*seek)(u32 obj, u32 timestamp);
    int (*resume)(u32 obj);
    int (*close)(u32 obj);
    int (*pre_stop)(u32 obj);
    int (*get_seek_time)(u32 obj, u32 *timestamp);
};

void audio_client_register(struct audio_client *client);

u32 audio_client_open(enum astream_type type, enum aplay_type aplay_type,
                      enum astream_format format, void *msg);
int audio_client_play(ACLIENT aclient, void *msg);
int audio_client_pause(ACLIENT aclient);
int audio_client_resume(ACLIENT aclient);
int audio_client_stop(ACLIENT aclient);
int audio_client_seek(ACLIENT aclient , u32 percent);
int audio_client_close(ACLIENT aclient);
int audio_client_close_async(ACLIENT aclient);
int audio_client_get_timestamp(ACLIENT aclient);

/* delete client without notify server */
int audio_client_delete(ACLIENT aclient);

struct asink_buf * audio_client_get_buf(ACLIENT aclient);
int audio_client_callback(ACLIENT aclient, int event, char *buff, u32 *size);

#endif /* __AUDIO_CLIENT_H__ */

