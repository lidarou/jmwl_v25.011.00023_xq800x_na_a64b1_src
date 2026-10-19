/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defined sound stream contrl interface and related struct,
 *    MACRO, etc.
 *
 */

#ifndef __SND_STREAM_H__
#define __SND_STREAM_H__

#include "snd_compat.h"
#include "snd_compr_params.h"

#define FL_PCM_IN      1
#define FL_PCM_OUT     2
#define FL_COMPR_IN    4
#define FL_COMPR_OUT   8
#define FL_NO_IRQ      16
#define FL_MM_VOIP     32

#define SND_STREAM_WIDTH_DEFAULT    16

#define IS_PCM_PARAM(x) (x->flag & (FL_PCM_IN | FL_PCM_OUT))

#define SND_STREAM_PARAM_COM    \
    u32 flag;                   \
    u32 srate;                  \
    u32 brate;                  \
    u32 channels;               \
    u32 format;                 \
    u32 period_cnt;             \
    u32 period_size;            \
    u32 start_threshold;        \
    u32 stop_threshold;

/* Bit formats */
enum pcm_fmt {
    PCM_FMT_INVALID = -1,
    PCM_FMT_S16_LE = 0,  /* 16-bit signed */
    PCM_FMT_S32_LE,      /* 32-bit signed */
    PCM_FMT_S8,          /* 8-bit signed */
    PCM_FMT_S24_LE,      /* 24-bits in 4-bytes */
    PCM_FMT_S24_3LE,     /* 24-bits in 3-bytes */

    PCM_FMT_MAX,
};

/**
 * struct snd_compr_params - compress stream params
 * @pext: codec parameters, see struct snd_codec_param
 */
struct snd_compr_params {
    SND_STREAM_PARAM_COM
    void *pext;
} __attribute__((packed, aligned(4)));

struct snd_pcm_params {
    SND_STREAM_PARAM_COM
} __attribute__((packed, aligned(4)));

/**
 * struct snd_stream_params - stream params
 * @pext: extend parameters
 */
struct snd_stream_params {
    SND_STREAM_PARAM_COM
    void *pext;
};

/**
 * struct snd_stream_tstamp - timestamp descriptor
 * @byte_offset: Byte offset in ring buffer to DSP
 * @copied_total: Total number of bytes copied from/to ring buffer to/by DSP
 * @pcm_frames: Frames decoded or encoded by DSP. This field will evolve by
 *    large steps and should only be used to monitor encoding/decoding
 *    progress. It shall not be used for timing estimates.
 * @pcm_io_frames: Frames rendered or received by DSP into a mixer or an audio
 * output/input. This field should be used for A/V sync or time estimates.
 * @sampling_rate: sampling rate of audio
 */
struct snd_stream_tstamp {
    __u32 byte_offset;
    __u32 copied_total;
    __u32 pcm_frames;
    __u32 pcm_io_frames;
    __u32 sampling_rate;
} __attribute__((packed, aligned(4)));

/**
 * struct snd_stream_avail - avail descriptor
 * @avail: Number of bytes available in ring buffer for writing/reading
 * @tstamp: timestamp information
 */
struct snd_stream_avail {
    __u64 avail;
    struct snd_stream_tstamp tstamp;
} __attribute__((packed, aligned(4)));

enum snd_stream_direction {
    SND_STREAM_PLAYBACK = 0,
    SND_STREAM_CAPTURE
};

#define STREAM_PCM_IN      1
#define STREAM_PCM_OUT     2
#define STREAM_COMPR_IN    4
#define STREAM_COMPR_OUT   8
#define STREAM_NO_IRQ      16

/**
 * struct snd_stream_caps - caps descriptor
 * @codecs: pointer to array of codecs
 * @direction: direction supported. Of type snd_stream_direction
 * @min_fragment_size: minimum fragment supported by DSP
 * @max_fragment_size: maximum fragment supported by DSP
 * @min_fragments: min fragments supported by DSP
 * @max_fragments: max fragments supported by DSP
 * @num_codecs: number of codecs supported
 * @reserved: reserved field
 */
struct snd_stream_caps {
    __u32 num_codecs;
    __u32 direction;
    __u32 min_fragment_size;
    __u32 max_fragment_size;
    __u32 min_fragments;
    __u32 max_fragments;
    __u32 codecs[MAX_NUM_CODECS];
    __u32 reserved[11];
} __attribute__((packed, aligned(4)));

/**
 * struct snd_stream_codec_caps - query capability of codec
 * @codec: codec for which capability is queried
 * @num_descriptors: number of codec descriptors
 * @descriptor: array of codec capability descriptor
 */
struct snd_stream_codec_caps {
    __u32 codec;
    __u32 num_descriptors;
    struct snd_codec_desc descriptor[MAX_NUM_CODEC_DESCRIPTORS];
} __attribute__((packed, aligned(4)));

/**
 * enum sndrv_stream_encoder
 * @SNDRV_STREAM_ENCODER_PADDING: no of samples appended by the encoder at the
 * end of the track
 * @SNDRV_STREAM_ENCODER_DELAY: no of samples inserted by the encoder at the
 * beginning of the track
 */
enum sndrv_stream_encoder {
    SNDRV_STREAM_ENCODER_PADDING = 1,
    SNDRV_STREAM_ENCODER_DELAY = 2,
};

/**
 * struct snd_stream_metadata - stream metadata
 * @key: key id
 * @value: key value
 */
struct snd_stream_metadata {
     __u32 key;
     __u32 value[8];
} __attribute__((packed, aligned(4)));

#ifndef WIN32
#define _IOR(m, c, t) (c)
#define _IOW(m, c, t) (c)
#define _IO(m, c) (c)
#endif
#define _IOC_NR(x) (x)
#define _IOWR(m, c, t) (c)

/**
 * stream path ioctl definitions
 * SNDRV_STREAM_GET_CAPS: Query capability of DSP
 * SNDRV_STREAM_GET_CODEC_CAPS: Query capability of a codec
 * SNDRV_STREAM_SET_PARAMS: Set codec and stream parameters
 * Note: only codec params can be changed runtime and stream params cant be
 * SNDRV_STREAM_GET_PARAMS: Query codec params
 * SNDRV_STREAM_TSTAMP: get the current timestamp value
 * SNDRV_STREAM_AVAIL: get the current buffer avail value.
 * This also queries the tstamp properties
 * SNDRV_STREAM_PAUSE: Pause the running stream
 * SNDRV_STREAM_RESUME: resume a paused stream
 * SNDRV_STREAM_START: Start a stream
 * SNDRV_STREAM_STOP: stop a running stream, discarding ring buffer content
 * and the buffers currently with DSP
 * SNDRV_STREAM_DRAIN: Play till end of buffers and stop after that
 * SNDRV_STREAM_IOCTL_VERSION: Query the API version
 */
#define SNDRV_STREAM_IOCTL_VERSION  _IOR('C', 0x00, int)
#define SNDRV_STREAM_GET_CAPS       _IOWR('C', 0x10, struct snd_stream_caps)
#define SNDRV_STREAM_GET_CODEC_CAPS _IOWR('C', 0x11,\
                                    struct snd_stream_codec_caps)
#define SNDRV_STREAM_SET_PARAMS     _IOW('C', 0x12, struct snd_stream_params)
#define SNDRV_STREAM_GET_PARAMS     _IOR('C', 0x13, struct snd_codec_param)
#define SNDRV_STREAM_SET_METADATA   _IOW('C', 0x14,\
                                    struct snd_stream_metadata)
#define SNDRV_STREAM_GET_METADATA   _IOWR('C', 0x15,\
                                    struct snd_stream_metadata)
#define SNDRV_STREAM_TSTAMP         _IOR('C', 0x20, struct snd_stream_tstamp)
#define SNDRV_STREAM_AVAIL          _IOR('C', 0x21, struct snd_stream_avail)
#define SNDRV_STREAM_PAUSE          _IO('C', 0x30)
#define SNDRV_STREAM_RESUME         _IO('C', 0x31)
#define SNDRV_STREAM_START          _IO('C', 0x32)
#define SNDRV_STREAM_STOP           _IO('C', 0x33)
#define SNDRV_STREAM_DRAIN          _IO('C', 0x34)
#define SNDRV_STREAM_NEXT_TRACK     _IO('C', 0x35)
#define SNDRV_STREAM_PARTIAL_DRAIN  _IO('C', 0x36)

#define SNDRV_STREAM_TRIGGER_STOP           0
#define SNDRV_STREAM_TRIGGER_START          1
#define SNDRV_STREAM_TRIGGER_PAUSE_PUSH     3
#define SNDRV_STREAM_TRIGGER_PAUSE_RELEASE  4
#define SNDRV_STREAM_TRIGGER_SUSPEND        5
#define SNDRV_STREAM_TRIGGER_RESUME         6
#define SNDRV_STREAM_TRIGGER_DRAIN          7
#define SNDRV_STREAM_TRIGGER_NEXT_TRACK     8
#define SNDRV_STREAM_TRIGGER_PARTIAL_DRAIN  9


typedef int snd_pcm_state_t;
#define SNDRV_STREAM_STATE_OPEN         ((__force snd_pcm_state_t) 0) /* stream is open */
#define SNDRV_STREAM_STATE_SETUP        ((__force snd_pcm_state_t) 1) /* stream has a setup */
#define SNDRV_STREAM_STATE_PREPARED     ((__force snd_pcm_state_t) 2) /* stream is ready to start */
#define SNDRV_STREAM_STATE_RUNNING      ((__force snd_pcm_state_t) 3) /* stream is running */
#define SNDRV_STREAM_STATE_XRUN         ((__force snd_pcm_state_t) 4) /* stream reached an xrun */
#define SNDRV_STREAM_STATE_DRAINING     ((__force snd_pcm_state_t) 5) /* stream is draining */
#define SNDRV_STREAM_STATE_PAUSED       ((__force snd_pcm_state_t) 6) /* stream is paused */
#define SNDRV_STREAM_STATE_SUSPENDED    ((__force snd_pcm_state_t) 7) /* hardware is suspended */
#define SNDRV_STREAM_STATE_DISCONNECTED ((__force snd_pcm_state_t) 8) /* hardware is disconnected */
#define SNDRV_STREAM_STATE_LAST         SNDRV_STREAM_STATE_DISCONNECTED


int snd_stream_open(char *stream_name, int flag, u32 *file);
int snd_stream_start(u32 file);
int snd_stream_stop(u32 file);
int snd_stream_close(u32 file);
int snd_stream_pause(u32 file);
int snd_stream_resume(u32 file);
int snd_stream_set_params(u32 file, u32 arg);
int snd_stream_write(u32 file, const char *buf, size_t count, void *offset);
int snd_stream_read(u32 file, char *buf, size_t count, void *offset);
const char *snd_stream_get_stream_name(u32 file);
const char *snd_stream_get_dpm_stream_name(u32 file);
size_t snd_stream_get_avail(u32 file);
unsigned int snd_stream_poll(u32 file, u32 wait);
int snd_stream_drain(u32 file);
unsigned int snd_stream_get_pcm_bits(int format);
int snd_stream_tstamp(u32 file, unsigned long arg);

#endif /* __SND_STREAM_H__ */

