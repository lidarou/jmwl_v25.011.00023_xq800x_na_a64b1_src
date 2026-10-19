/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defined sound stream interface for driver module, not for HAL
 *    or application.
 *
 */

#ifndef __SND_STREAM_DRIVER_H__
#define __SND_STREAM_DRIVER_H__

#include "snd_stream.h"

#define EVENT_WAKEUP    0x01
#define EVENT_DRAIN     0x02

struct snd_stream_ops;

/**
 * struct snd_stream_runtime: runtime stream description
 * @state: stream state
 * @ops: pointer to DSP callbacks
 * @buffer: pointer to kernel buffer, valid only when not in mmap mode or
 *    DSP doesn't implement copy
 * @buffer_size: size of the above buffer
 * @fragment_size: size of buffer fragment in bytes
 * @fragments: number of such fragments
 * @total_bytes_available: cumulative number of bytes made available in
 *    the ring buffer
 * @total_bytes_transferred: cumulative bytes transferred by offload DSP
 * @sleep: poll sleep
 * @private_data: driver private data pointer
 */
struct snd_stream_runtime {
    snd_pcm_state_t state;
    struct snd_stream_ops *ops;
    struct snd_dai *dai;
    void *buffer;
    u32 buffer_size;
    u32 fragment_size;
    u32 fragments;
    u32 total_bytes_available;
    u32 total_bytes_transferred;
    EVENT_PTR sleep;
    void *private_data;
};

/**
 * struct snd_stream: compressed stream
 * @name: device name
 * @ops: pointer to DSP callbacks
 * @runtime: pointer to runtime structure
 * @device: device pointer
 * @error_work: delayed work used when closing the stream due to an error
 * @direction: stream direction, playback/recording
 * @metadata_set: metadata set flag, true when set
 * @next_track: has userspace signal next track transition, true when set
 * @private_data: pointer to DSP private data
 */
struct snd_stream {
    const char *name;
    struct snd_stream_ops *ops;
    struct snd_stream_runtime *runtime;
    struct snd_stream_vdev *device;
    enum snd_stream_direction direction;
    bool metadata_set;
    bool next_track;
    void *private_data;
};

struct snd_stream_ops {
    int (*open)(struct snd_stream *stream);
    int (*free)(struct snd_stream *stream);
    int (*set_params)(struct snd_stream *stream,
            struct snd_stream_params *params);
    int (*get_params)(struct snd_stream *stream,
            struct snd_codec_param *params);
    int (*set_metadata)(struct snd_stream *stream,
            struct snd_stream_metadata *metadata);
    int (*get_metadata)(struct snd_stream *stream,
            struct snd_stream_metadata *metadata);
    int (*trigger)(struct snd_stream *stream, int cmd);
    int (*pointer)(struct snd_stream *stream,
            struct snd_stream_tstamp *tstamp);
    int (*copy)(struct snd_stream *stream, char *buf,
               size_t count);
    int (*ack)(struct snd_stream *stream, size_t bytes);
    int (*get_caps) (struct snd_stream *stream,
            struct snd_stream_caps *caps);
    int (*get_codec_caps) (struct snd_stream *stream,
            struct snd_stream_codec_caps *codec);
};

struct snd_stream_vdev {
    const char *name;
    const char *dpm_stream;
    struct snd_stream_ops *ops;
    void *private_data;
    unsigned int direction;
    MUTEX_PTR lock;
    int active;
};

int snd_stream_register(const struct snd_stream_vdev *vdev);


static inline void init_waitqueue_head(EVENT_PTR *sleep)
{
    *sleep = aos_create_event("snd_stream_event");
}

static inline void cancel_waitqueue_head(EVENT_PTR sleep)
{
    aos_delete_event(sleep);
}

static inline int poll_wait(EVENT_PTR sleep, u32 wait)
{
    u32 actual_flags, ret;

    ret = aos_get_event(sleep, EVENT_WAKEUP, AOS_OR_CLEAR, &actual_flags, wait);

    return ret;
}

static inline void wake_up(EVENT_PTR sleep)
{
    aos_set_event(sleep, EVENT_WAKEUP, AOS_OR);
}

static inline void snd_stream_fragment_elapsed(struct snd_stream *stream)
{
    wake_up(stream->runtime->sleep);
}

static inline int snd_stream_wait_event_drain(EVENT_PTR sleep)
{
    u32 actual_flags;
    int ret;

    ret = aos_get_event(sleep, EVENT_DRAIN, AOS_OR_CLEAR,
                        &actual_flags, SND_WAIT_FOREVER);

    return ret;
}

static inline void snd_stream_drain_notify(struct snd_stream *stream)
{
    snd_BUG_ON(!stream);

    if (!stream)
        return;

    if (stream->direction == SND_STREAM_PLAYBACK)
        stream->runtime->state = SNDRV_STREAM_STATE_SETUP;
    else
        stream->runtime->state = SNDRV_STREAM_STATE_PREPARED;

    aos_set_event(stream->runtime->sleep, EVENT_DRAIN, AOS_OR);
}

#endif /* __SND_STREAM_DRIVER_H__ */
