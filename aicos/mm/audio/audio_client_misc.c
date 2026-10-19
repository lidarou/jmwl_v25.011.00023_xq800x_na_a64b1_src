/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is music client file, including volte and other
 *  less code client.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x12737b
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_init.h>
#include <alog_api.h>
#include <aic_errno.h>

#include "audio_system.h"
#include "audio_service.h"
#include "audio_sink.h"
#include "audio_stream.h"
#include "audio_policy.h"
#include "audio_sink_record.h"
#include "codec_mng.h"

/* general client, if cannot find matched client, we can use this one*/
struct client_gen {
    u32 dir;
    u32 state;
    enum astream_format format;
    void *aplay_data;
};

static inline struct client_gen *get_client_gen_buf(void)
{
    struct client_gen *client_gen = audio_malloc(sizeof(*client_gen));

    if (!client_gen) {
        AIDXE(0x0c77ce, "", "get client voice buf failed, no memory!\n");
        return 0;
    }
    memset(client_gen, 0, sizeof(*client_gen));

    return client_gen;
}

static u32 client_gen_open(enum astream_format format, void *msg)
{
    struct client_gen *client_gen = get_client_gen_buf();

    if (client_gen)
        client_gen->format = format;

    return (u32)client_gen;
}

static int client_gen_close(u32 obj)
{
    if (obj)
        audio_free(obj);

    return 0;
}

static int client_gen_start(u32 obj, void *aplay_data)
{
    return 0;
}

static int client_gen_stop(u32 obj)
{
    return 0;
}

static int client_gen_callback(u32 obj, int event, char *buffer,
                               u32 *get_size)
{
    return 0;
}

struct audio_client audio_client_gen = {
    .type = ASTREAM_TYPE_DEFAULT,
    .aplay_type = APLAY_TYPE_NONE,
    .open = client_gen_open,
    .start = client_gen_start,
    .stop = client_gen_stop,
    .close = client_gen_close,
    .cb = client_gen_callback
};

static int audio_client_gen_init(void)
{
    audio_client_register(&audio_client_gen);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(audio_client_gen_init);

