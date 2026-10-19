/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio sink module, it provided sink process API for
 *  audio service.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18a926
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>

#include "audio_policy.h"
#include "audio_sink.h"
#include "audio_stream.h"
#include "audio_sink_record.h"
#include "audio_service.h"
#include "audio_util.h"
#include "audio_effect.h"
#include "audio_resample.h"
#include "audio_nrec.h"
#include "audio_dump.h"

#define ASINK_EVENT_START       0x1u
#define ASINK_EVENT_STOP        0x2u
#define ASINK_EVENT_PAUSE       0x4u
#define ASINK_EVENT_RESUME      0x8u
#define ASINK_EVENT_CLOSE       0x10u
#define ASINK_EVENT_SERVER_EXIT 0x20u
#define ASINK_EVENT_SERVER_STOP 0x40u
#define ASINK_EVENT_SERVER_PAUSE    0x80u

#define CHECK_HANDLE(x)
#define CHECK_TYPE(type)

#ifdef CONFIG_MCU_AUDIO
#define ASTREAM_BUFF_SIZE (4096 * 2)
#else
#define ASTREAM_BUFF_SIZE (4096)
#endif

#define ASTREAM_SERVICE_STACK_SIZE  4096
#define ASINK_THREAD_QUEUE_NUM      5
#define ASINK_THREAD_PRI            4

static void *g_astrem_runtime[ASTREAM_TYPE_OUTPUT_MAX];
/* for debug */
static AHANDLE g_astream_handle[ASTREAM_TYPE_OUTPUT_MAX];

const static char *astream_thread_name[ASTREAM_TYPE_OUTPUT_MAX] = {
    "music",
    "ring",
    "system",
    "tone",
    "dtmf",
    "voice",
    "voip",
    "fm",
    "tts",
    "alarm",
    "notification"
};

/* 0 - AUDIO_MVOL_LEVEL_MAX */
#ifndef CONFIG_VOLUME_NO_CODEC
const static u8 g_audio_volume_level_tbl[AUDIO_MVOL_LEVEL_MAX + 1] =
    AUDIO_VOLUME_LEVEL_TBL;
#endif

static inline struct astream_runtime *get_runtime(enum astream_type type)
{
    return g_astrem_runtime[type];
}

#ifdef CONFIG_VOICE_STREAM
int audio_get_voice_dl_dev(void)
{
    struct astream_runtime *runtime;

    runtime = get_runtime(ASTREAM_TYPE_VOICE);
    if (runtime)
        return runtime->stream_dev;

    return 0;
}
#endif

static inline int16_t clamp16(int32_t sample)
{
    if ((sample >> 15) ^ (sample >> 31))
        sample = 0x7FFF ^ (sample >> 31);

    return sample;
}

static void apply_volume(s16 *buf, u32 bytes, u32 volume)
{
    s32 i, scale;

#ifdef CONFIG_VOLUME_NO_CODEC
    scale = volume * (0.1f * (1 << 8));
#else
    scale = ((u32)g_audio_volume_level_tbl[volume] << 8) / 100;
#endif

    for (i = 0; i < (bytes >> 1); i++)
        buf[i] = clamp16(((s32)buf[i] * scale) >> 8);

    return;
}

#ifdef CONFIG_PLAYBACK_EFFECT
static u32 audio_sink_data_proc(struct astream_runtime *runtime,
                                void *inbuf, u32 bytes)
{
    u32 cnt = bytes >> 1;
    u32 new_bytes = 0;

    if (runtime->chn == ASTREAM_CHN_MONO &&
        runtime->hw_config.chn == ASTREAM_CHN_STEREO) {
        ae_mono_2_stero((short *)inbuf, cnt);
        new_bytes = bytes << 1;
    } else if (runtime->chn == ASTREAM_CHN_STEREO &&
        runtime->hw_config.chn == ASTREAM_CHN_STEREO) {
        ae_mix_lr((short *)inbuf, cnt);
        new_bytes = bytes;
    } else if (runtime->chn == ASTREAM_CHN_STEREO &&
        runtime->hw_config.chn == ASTREAM_CHN_MONO) {
        ae_stero_2_mono((short *)inbuf, cnt);
        new_bytes = bytes >> 1;
    } else {
        new_bytes = bytes;
    }

    return new_bytes;
}

static u32 audio_sink_rsmpl(struct astream_runtime *runtime,
                            void *inbuf, void *outbuf, u32 bytes)
{
    u32 frm = bytes >> runtime->hw_config.chn, out_frm = 0;

    if (bytes == 0)
        return 0;

    audio_rsmpl_run(runtime->hrsmpl, inbuf, frm, outbuf, &out_frm);

    return out_frm << runtime->hw_config.chn;
}

static u32 audio_sink_effect(struct astream_runtime *runtime, void *in_buf,
                             u32 in_size, void *out_buf, u32 *out_size)
{
    return audio_nr_proc(runtime->hae, in_buf, in_size, out_buf, out_size);
}
#endif

static u32 audio_sink_get_period_size(struct astream_runtime *runtime)
{
    if (runtime->flag & AUDIO_OUTPUT_FLAG_COMPRESS)
        return runtime->config->period_size;

    if ((runtime->format >= ASTREAM_FORMAT_OFFLD_BEGIN &&
         runtime->format <= ASTREAM_FORMAT_OFFLD_END)) {
        /* if compress, use default */
        if (runtime->flag & AUDIO_OUTPUT_FLAG_COMPRESS)
            return runtime->config->period_size;
        else
            return runtime->config->period_size * runtime->chn *
                     (runtime->sample_rate / 8000);
    } else
        return runtime->config->period_size * runtime->chn *
                     (runtime->sample_rate / 8000);
}

static u32 audio_sink_wait(struct astream_runtime *runtime, u32 event)
{
    u32 actual_flags = 0;

    aos_get_event(runtime->event, event, AOS_OR_CLEAR, &actual_flags,
                  AUDIO_WAIT_FOREVER);

    return actual_flags;
}

static u32 audio_sink_notify(struct astream_runtime *runtime, u32 event)
{
    aos_set_event(runtime->event, event, AOS_OR);

    return 0;
}

static int audio_sink_callback(enum astream_cb_event event, AHANDLE handle)
{
    struct audio_msg_user_cb *msg;

    msg = (struct audio_msg_user_cb *)audio_malloc(sizeof(*msg));
    if (!msg)
        return AIC_ENOMEM;
    msg->cmd = AUDIO_CMD_USER_CB;
    msg->event = event;
    msg->handle = handle;
    audio_send_msg(msg, NULL, 0);

    return 0;
}

#define SWAP_BUF(p1, p2)    {   \
    void *p;    \
    p = p1; \
    p1 = p2;    \
    p2 = p; \
}

static void astream_thread_loop(void *data)
{
    struct astream_runtime *runtime = (struct astream_runtime *)data;
    u32 len = 320, period_size;
    u32 flag = 0;
    int ret, cb_ret;
    void *pbuf, *inbuf, *outbuf, *wrbuf;

    if (!runtime)
        return;

    pbuf = audio_malloc(ASTREAM_BUFF_SIZE + 16);
    AIC_ASSERT(pbuf);
    inbuf = pbuf;
    outbuf = (char *)inbuf + (ASTREAM_BUFF_SIZE >> 1);
    wrbuf = outbuf;
    while (1) {
        AIDXD(0x0a3bca, "", "astream thread loop wait...\n");
        flag = audio_sink_wait(runtime,
                               ASINK_EVENT_START | ASINK_EVENT_STOP |
                               ASINK_EVENT_PAUSE | ASINK_EVENT_RESUME |
                               ASINK_EVENT_CLOSE);
        if (flag & (ASINK_EVENT_START | ASINK_EVENT_RESUME)) {
            /* Active only, Nothing to do */
            if (!runtime->aclient)
                continue;
#ifdef CONFIG_PLAYBACK_EFFECT
            if (runtime->hrsmpl)
                period_size = runtime->config->period_size * runtime->chn *
                                (runtime->sample_rate / 8000);
            else
#endif
                period_size = audio_sink_get_period_size(runtime);

            if (runtime->type == ASTREAM_TYPE_VOICE) {
                memset(wrbuf, 0, period_size);
                audio_stream_write(runtime->stream_dev, wrbuf, 640);
                audio_stream_write(runtime->stream_dev, wrbuf, 640);
                audio_stream_write(runtime->stream_dev, wrbuf, 640);
            }
        } else if (flag & ASINK_EVENT_CLOSE) {
            /* exit thread loop */
            audio_sink_notify(runtime, ASINK_EVENT_SERVER_EXIT);
            break;
        } else {
            continue;
            /* Nothing else to do, continue wait */
        }

        ADEBUG;
        while (runtime->state == ASTREAM_PLAYING) {
            len = period_size;
            cb_ret = audio_client_callback(runtime->aclient,
                                           AUDIO_CLIENT_CMD_GET_MORE,
                                           inbuf, &len);
            if (cb_ret < 0) {
                /* sleep a while to avoid occupy CPU long time */
                aos_sleep(5);
                continue;
            }

            if (!(runtime->flag & AUDIO_OUTPUT_FLAG_COMPRESS))
                apply_volume((s16 *)inbuf, len, audio_svc_master_volume());
#ifdef CONFIG_PLAYBACK_EFFECT
            if (runtime->hae) {
                audio_dump(inbuf, len, 16);
                len = audio_sink_data_proc(runtime, inbuf, len);
                if (runtime->hrsmpl) {
                    audio_dump(inbuf, len, 17);
                    len = audio_sink_rsmpl(runtime, inbuf, outbuf, len);
                    audio_dump(outbuf, len, 18);
                    SWAP_BUF(inbuf, outbuf)
                }
                audio_sink_effect(runtime, inbuf, len, outbuf, &len);
                audio_dump(outbuf, len, 19);
                wrbuf = outbuf;
            } else
#endif
                wrbuf = inbuf;

            ret = audio_stream_write(runtime->stream_dev, wrbuf, len);
            if (ret < 0) {
                AIDXE(0x0aa0c5, "d", "stream write failed, ret=%d\n", ret);
                /**
                 * TODO:
                 * 1. restart audio stream ?
                 * 2. ignore and go on?
                 */
            }

            if (cb_ret == ASTREAM_DATA_EOS) {
                if (runtime->flag & AUDIO_OUTPUT_FLAG_COMPRESS) {
                   audio_stream_drain(runtime->stream_dev);
                   AIDXW(0x020ec2, "", "audio sink drain out\n");
                } else {
                    memset(wrbuf, 0, period_size);
                    audio_stream_write(runtime->stream_dev, wrbuf, period_size);
                    audio_stream_write(runtime->stream_dev, wrbuf, period_size);
                }
                /* notify client */
                audio_sink_callback(ASTREAM_CB_EVENT_FINISH, runtime->handle);

                /* no more data, exit and wait */
                break;
            }
        }
        if (runtime->state == ASTREAM_STOPPING)
            audio_sink_notify(runtime, ASINK_EVENT_SERVER_STOP);
        else if (runtime->state == ASTREAM_PAUSE)
            audio_sink_notify(runtime, ASINK_EVENT_SERVER_PAUSE);
    }
    audio_free(pbuf);
    AEXIT;
}

void *audio_sink_init_server(enum astream_type type)
{
    struct astream_runtime *runtime;
    struct astream_property *property;

    if (type >= ASTREAM_TYPE_OUTPUT_MAX) {
        AIDXE(0x03a7f0, "d", "audio sink init server failed, type=%d\n", type);
        return NULL;
    }
    if (g_astrem_runtime[type]) {
        AIDXE(0x0abe2d, "d", "stream thread already created, type=%d\n", type);
        return NULL;
    }

    property = apolicy_get_stream_property(type);
    if (!property)
        return NULL;

    runtime = audio_malloc(sizeof(struct astream_runtime));
    if (!runtime) {
        AIDXE(0x08abbb, "", "tone inie stream, no memory!\n");
        return NULL;
    }
    memset(runtime, 0, sizeof(*runtime));
    runtime->property = property;

    runtime->event = aos_create_event((char *)astream_thread_name[type]);
    runtime->lock = aos_create_mutex((char *)astream_thread_name[type]);

    if (property->has_stream) {
        runtime->thread = aos_create_thread((char *)astream_thread_name[type],
                                            astream_thread_loop, runtime,
                                            ASTREAM_SERVICE_STACK_SIZE,
                                            ASINK_THREAD_QUEUE_NUM,
                                            (char *)astream_thread_name[type],
                                            ASINK_THREAD_PRI,
                                            AOS_AUTO_START);
    }

    g_astrem_runtime[type] = (void *)runtime;

    return runtime;
}

void audio_sink_save_user_data(struct astream_runtime *runtime,
                               enum aplay_type aplay_type, void *aplay_data)
{
    if (!aplay_data) {
        runtime->user_cb = NULL;
        return;
    }
    switch(aplay_type) {
    case APLAY_TYPE_BUF: {
        struct audio_play_sub_buf *sub =
            (struct audio_play_sub_buf *)aplay_data;
        runtime->user_cb = (astream_user_cb)sub->user_cb;
        runtime->cb_data = sub->cb_data;
        if (sub->srate) {
            runtime->sample_rate = sub->srate;
            runtime->chn = sub->chn;
            /* bit rate not set now */
        }
        break;
    }
    case APLAY_TYPE_FILE: {
        struct audio_play_sub_file *sub =
            (struct audio_play_sub_file *)aplay_data;
        runtime->user_cb = (astream_user_cb)sub->user_cb;
        runtime->cb_data = sub->cb_data;
        if (sub->srate) {
            if (runtime->type == ASTREAM_TYPE_VOIP)
                runtime->sample_rate = 16000;
            else
                runtime->sample_rate = sub->srate;
            runtime->chn = sub->chn;
            /* bit rate not set now */
        }
        break;
    }
    case APLAY_TYPE_ID: {
        struct audio_play_sub_id *sub =
            (struct audio_play_sub_id *)aplay_data;
        runtime->user_cb = (astream_user_cb)sub->user_cb;
        runtime->cb_data = sub->cb_data;
        break;
    }
    case APLAY_TYPE_TTS: {
        struct audio_play_sub_tts *sub =
            (struct audio_play_sub_tts *)aplay_data;
        runtime->user_cb = (astream_user_cb)sub->user_cb;
        runtime->cb_data = sub->cb_data;
        break;
    }
    case APLAY_TYPE_STREAM: {
        struct audio_play_sub_stream *sub =
            (struct audio_play_sub_stream *)aplay_data;
        runtime->user_cb = (astream_user_cb)sub->user_cb;
        runtime->cb_data = sub->cb_data;
        runtime->abuf = sub->abuf;
        if (sub->srate) {
            runtime->sample_rate = sub->srate;
            runtime->chn = sub->chn;
            /* bit rate not set now */
        }
        break;
    }
    default:
        runtime->user_cb = NULL;
        break;
    }
}

AHANDLE audio_sink_open(ACLIENT aclient, enum astream_type type,
                        enum aplay_type aplay_type,
                        u32 flag, enum astream_format format, u32 sample_rate,
                        u32 bit_rate, enum astream_channel chn,
                        enum astream_direct dir)
{
    struct astream_runtime *runtime;
    struct astream_property *property;
    struct asink_buf *abuf = 0;
    AHANDLE handle = 0;
    int stream_chn;

    AENTRY;

    CHECK_TYPE(type);

    /* if chn got, means hardware is idle */
    stream_chn = apolicy_get_stream_chn(type, format, flag);
    if (stream_chn < 0) {
        AIDXE(0x1b26cc, "", "cannot get valide stream chn!\n");
        return 0;
    }

    property = apolicy_get_stream_property(type);
    if (!property) {
        AIDXE(0x108a51, "", "cannot get property\n");
        return 0;
    }

    if (aplay_type == APLAY_TYPE_STREAM) {
        struct apolicy_stream_config *config;

        config = (struct apolicy_stream_config *)
            apolicy_get_compr_config(type, format, ASTREAM_DIR_PLAY, flag);
        abuf = abuf_create(config->period_size, config->period_cnt);
        if (!abuf) {
            AIDXE(0x0a6077, "", "audio sink open, create buf failed\n");
            return 0;
        }
    }

    if (property->is_dynamic)
        runtime = audio_sink_init_server(type);
    else
        runtime = get_runtime(type);
    if (!runtime) {
        AIDXE(0x009380, "", "astream no runtime\n");

        if (abuf)
            audio_free(abuf);
        return 0;
    }

    runtime->aclient = aclient;
    runtime->abuf = abuf;
    handle = audio_create_handle(type, flag, format, ASTREAM_DIR_PLAY,
                                 ASTREAM_SOURCE_DEFAULT);
    runtime->handle = handle;
    runtime->state = ASTREAM_OPEN;
    /* save it for debug */
    g_astream_handle[type] = handle;


    /* save input param */
    runtime->type = type;
    runtime->sample_rate = sample_rate;
    runtime->format = format;
    runtime->flag = flag;
    runtime->chn = chn;
    runtime->direct = dir;
    runtime->aplay_type = aplay_type;

    AEXIT;

    return handle;
}

int audio_sink_set_param(AHANDLE handle, void *param)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x114ae0, "", "sink start, no runtime\n");
        return AIC_ENODATA;
    }
    mutex_lock(&runtime->lock);
    /* TODO */
    mutex_unlock(&runtime->lock);

    return 0;
}

int audio_sink_open_hw(struct astream_runtime *runtime)
{
    int ret, stream_dev, stream_chn;
    struct stream_config config = {0};
    u32 stream_flag;

#ifdef CONFIG_PLAYBACK_EFFECT
    switch(runtime->type) {
    case ASTREAM_TYPE_MUSIC:
    case ASTREAM_TYPE_SYSTEM:
    case ASTREAM_TYPE_ALARM:
    case ASTREAM_TYPE_TTS:
        /* keep 48k stero always */
        config.chn = AUDIO_FIX_OUTPUT_CHN;
        config.sample_rate = AUDIO_FIX_OUTPUT_SRATE;
        if (runtime->sample_rate != AUDIO_FIX_OUTPUT_CHN) {
            u32 rsmp_frm = (runtime->config->period_size * runtime->chn *
                                (runtime->sample_rate / 8000)) >> 1;
            runtime->hrsmpl = audio_rsmpl_open(runtime->sample_rate,
                                               config.sample_rate,
                                               config.chn, rsmp_frm);
            AIDXI(0x079a81, "dddd", "open hw:srate=%d, chn=%d, rsmpl sr=%d, frm=%d\n",
                  config.sample_rate, config.chn, runtime->sample_rate,
                  config.period_size >> 2);
        }
        if (runtime->hrsmpl) /* TODO: get from audio reampler */
            config.period_size = runtime->config->period_size *
                                 AUDIO_FIX_OUTPUT_CHN *
                                 (AUDIO_FIX_OUTPUT_SRATE / 8000);
        else
            config.period_size = runtime->config->period_size *
                                 AUDIO_FIX_OUTPUT_CHN *
                                 (AUDIO_FIX_OUTPUT_SRATE / 8000);

        runtime->hae = audio_nr_create(config.sample_rate, config.chn, 0);
        break;
    default:
        config.chn = runtime->chn;
        config.sample_rate = runtime->sample_rate;
        config.period_size = audio_sink_get_period_size(runtime);
        break;
    }
    config.period_cnt = runtime->config->period_cnt;
    config.start_threshold = runtime->config->start_threshold;
    config.stop_threshold = runtime->config->stop_threshold;
    config.bit_rate = 0;
#else
    config.chn = runtime->chn;
    config.period_cnt = runtime->config->period_cnt;
    config.period_size = audio_sink_get_period_size(runtime);
    config.start_threshold = runtime->config->start_threshold;
    config.stop_threshold = runtime->config->stop_threshold;
    config.sample_rate = runtime->sample_rate;
    config.bit_rate = 0;
#endif

    if (runtime->direct == ASTREAM_DIR_PLAY) {
        if (runtime->flag & AUDIO_OUTPUT_FLAG_COMPRESS) {
            stream_flag = COMPR_OUT;

            /* Check format and sample rate again, make compatible with
               AMR-NB and AMR-WB */
            if (runtime->format == ASTREAM_FORMAT_AMR_NB &&
                    runtime->sample_rate == 16000)
                config.fmt = ASTREAM_FORMAT_AMR_WB;
            else if (runtime->format == ASTREAM_FORMAT_AMR_WB &&
                     runtime->sample_rate == 8000)
                config.fmt = ASTREAM_FORMAT_AMR_NB;
            else
                config.fmt = runtime->format;

            if (runtime->type == ASTREAM_TYPE_VOIP &&
                runtime->aplay_type != APLAY_TYPE_STREAM)
                stream_flag |= MM_VOIP;
        } else {
            stream_flag = PCM_OUT;
            config.fmt = APOLICY_DEFAULT_PCM_FORMAT; /* fixed value */
        }
    } else {
        if (runtime->flag & AUDIO_OUTPUT_FLAG_COMPRESS) {
            stream_flag = COMPR_IN;
            config.fmt = runtime->format;
        } else {
            stream_flag = PCM_IN;
            config.fmt = APOLICY_DEFAULT_PCM_FORMAT; /* fixed value */
        }
    }

#ifndef CONFIG_MCU_AUDIO
    /* if voice is PCM format, regard it as sending data to DSP */
    if (runtime->type == ASTREAM_TYPE_VOICE &&
            runtime->format == ASTREAM_FORMAT_PCM) {
        stream_flag = COMPR_IN;
        config.fmt = ASTREAM_FORMAT_AMR_WB;
    }
#endif
    stream_chn = apolicy_get_stream_chn(runtime->type, runtime->format,
                                        runtime->flag);

    AIDXE(0x1b7155, "dd", "open hw2:srate=%d, chn=%d\n", config.sample_rate, config.chn);
    stream_dev = audio_stream_open(stream_chn, &config, stream_flag);
    if (stream_dev) {
        runtime->stream_dev = stream_dev;
        runtime->hw_config = config;
    } else {
        AIDXE(0x180c94, "", "audio stream open failed\n");
        ret = AIC_EPIPE;
        goto out;
    }
    return 0;

out:
    return ret;
}

int audio_sink_start(AHANDLE handle, enum aplay_type aplay_type,
                     void *aplay_data)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;
    int ret = 0;

    AENTRY;

    CHECK_HANDLE(ahandle);

    /* TODO: should check state here */

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x114ae0, "", "sink start, no runtime\n");
        return AIC_ENODATA;
    }
    mutex_lock(runtime->lock);

    if (runtime->state != ASTREAM_OPEN) {
        AIDXE(0x00430a, "", "start, already played\n");
        ret = AIC_EPERM;
        goto out;
    }

    /* save first, need update srate etc. */
    audio_sink_save_user_data(runtime, aplay_type, aplay_data);

    /* then add handle to audio policy, now can update device */
    apolicy_add_stream_handle(handle);
    apolicy_device_update();

    runtime->config = (struct apolicy_stream_config *)
        apolicy_get_config(runtime->type, runtime->format, ASTREAM_DIR_PLAY,
                           runtime->flag);
    /* then open hw */
    ret = audio_sink_open_hw(runtime);
    if (ret) {
        goto out;
    }
    /* Then, update param */
    apolicy_param_update();

    /**
     * TODO:
     * Now start write data first, then start HW,
     * maybe blocked by DSP if start HW failed, would fix this issue in
     * the future.
     */
    if (runtime->property->has_stream) {
        runtime->state = ASTREAM_PLAYING;
        audio_sink_notify(runtime, ASINK_EVENT_START);
#ifdef CONFIG_HW_CODEC_SUPPORT
        aos_sleep(20);
#endif
    }

    ret = audio_stream_start(runtime->stream_dev);
    if (ret) {
        apolicy_delete_stream_handle(handle);
        audio_stream_close(runtime->stream_dev);
        runtime->stream_dev = 0;
        runtime->state = ASTREAM_OPEN;
        AIDXE(0x13c068, "d", "stream start failed, ret=%d\n", ret);
        goto out;
    }

out:
    if (ret)
        apolicy_delete_stream_handle(handle);
    mutex_unlock(runtime->lock);

    AIDXD(0x1444bc, "d", "sink, start exit, ret=%d\n", ret);

    return ret;
}

int audio_sink_stop(AHANDLE handle)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    AENTRY;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;

    /* TODO: check state */

    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x15651a, "", "sink stop, no runtime\n");
        return 0;
    }

    mutex_lock(runtime->lock);

    /* stop server first */
    if (runtime->property->has_stream) {
        if (runtime->state == ASTREAM_PLAYING) {
            runtime->state = ASTREAM_STOPPING;
            /* do not need notify, set state only */
            /* audio_sink_notify(runtime, ASINK_EVENT_STOP); */
            audio_sink_wait(runtime, ASINK_EVENT_SERVER_STOP);
        }
        runtime->state = ASTREAM_OPEN;
    }

    /* then hardware */
    if (runtime->stream_dev) {
        audio_stream_stop(runtime->stream_dev);
        apolicy_delete_stream_handle(handle);

        audio_stream_close(runtime->stream_dev);
        runtime->stream_dev = 0;
    } else
        AIDXE(0x1305c1, "", "sink already stopped\n");

#ifdef CONFIG_PLAYBACK_EFFECT
    if (runtime->hrsmpl) {
        audio_rsmpl_close(runtime->hrsmpl);
        runtime->hrsmpl = 0;
    }
    if (runtime->hae) {
        audio_nr_delete(runtime->hae);
        runtime->hae = 0;
    }
#endif

    mutex_unlock(runtime->lock);

    AEXIT;

    return 0;
}

int audio_sink_close(AHANDLE handle)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    AENTRY;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;

    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x18cbdf, "", "sink close, no runtime\n");
        return 0;
    }

    mutex_lock(runtime->lock);
    if (runtime->state == ASTREAM_OPEN) {
        /* close driver, it can unblock sink server. */

        runtime->state = ASTREAM_IDLE;

        /* if dynamic and has stream, notify stream thread to exit */
        if (runtime->property->is_dynamic && runtime->property->has_stream) {
            /* notify thread to exit */
            audio_sink_notify(runtime, ASINK_EVENT_CLOSE);
            /**
             * sink server thread priority is high, now wait it to exit.
             * it can be deleted when back to here.
             */
            audio_sink_wait(runtime, ASINK_EVENT_SERVER_EXIT);
            /* now thread can be deleted */
            aos_delete_thread(runtime->thread);
        }
    }

    /* if dynamic stream, realease system resource */
    if (runtime->property->is_dynamic) {
        aos_delete_event(runtime->event);
        g_astrem_runtime[audio_handle->type] = NULL;
        g_astream_handle[audio_handle->type] = 0;
        if (runtime->abuf)
            abuf_free(runtime->abuf);
    }
    mutex_unlock(runtime->lock);

    if (runtime->property->is_dynamic) {
        aos_delete_mutex(runtime->lock);
        audio_free(runtime);
    }

    AEXIT;

    return 0;
}

int audio_sink_finish(struct astream_runtime *runtime, BOOL is_close)
{
    AENTRY;

    mutex_lock(runtime->lock);

    /* change state only, need't stop sink again */
    if (runtime->property->has_stream)
        runtime->state = ASTREAM_OPEN;

    /* stop hardware */
    if (runtime->stream_dev) {
        audio_stream_stop(runtime->stream_dev);
        apolicy_delete_stream_handle(runtime->handle);

        audio_stream_close(runtime->stream_dev);
        runtime->stream_dev = 0;
    } else
        AIDXE(0x1305c1, "", "sink already stopped\n");

#ifdef CONFIG_PLAYBACK_EFFECT
        if (runtime->hrsmpl) {
            audio_rsmpl_close(runtime->hrsmpl);
            runtime->hrsmpl = 0;
        }
        if (runtime->hae) {
            audio_nr_delete(runtime->hae);
            runtime->hae = 0;
        }
#endif

    mutex_unlock(runtime->lock);

    if (!runtime->user_cb && runtime->property->is_dynamic)
        return audio_sink_close(runtime->handle);

    return 0;
}

int audio_sink_pause(AHANDLE handle)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;
    int ret = 0;

    AENTRY;

    CHECK_HANDLE(ahandle);

    /* TODO: should check state here */

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x114ae0, "", "sink start, no runtime\n");
        return AIC_ENODATA;
    }

    mutex_lock(runtime->lock);

    if (runtime->state == ASTREAM_PLAYING) {
        runtime->state = ASTREAM_PAUSE;
        audio_sink_wait(runtime, ASINK_EVENT_SERVER_PAUSE);
        ret = audio_stream_pause(runtime->stream_dev);
        if (ret) {
            AIDXE(0x0628d7, "d", "stream pause failed, ret=%d\n", ret);
            goto out;
        }
    } else {
        AIDXE(0x0491e8, "d", "pause failed, invalid state = %d\n", runtime->state);
        ret = AIC_EPERM;
    }

out:
    mutex_unlock(runtime->lock);

    return ret;
}

int audio_sink_resume(AHANDLE handle)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;
    int ret = 0;

    AENTRY;

    CHECK_HANDLE(ahandle);

    /* TODO: should check state here */

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x114ae0, "", "sink start, no runtime\n");
        return AIC_ENODATA;
    }

    mutex_lock(runtime->lock);

    if (runtime->state == ASTREAM_PAUSE) {
        runtime->state = ASTREAM_PLAYING;
        audio_sink_notify(runtime, ASINK_EVENT_RESUME);
        ret = audio_stream_resume(runtime->stream_dev);
        if (ret) {
            AIDXE(0x07451e, "d", "stream resume failed, ret=%d\n", ret);
            runtime->state = ASTREAM_PAUSE;
            goto out;
        }
    } else {
        AIDXE(0x03e52c, "d", "resume failed, invalid state = %d\n", runtime->state);
        ret = AIC_EPERM;
    }

out:
    mutex_unlock(runtime->lock);

    return ret;
}

u32 audio_sink_get_time(AHANDLE handle)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x18baaf, "", "sink get time, no runtime\n");
        return 0;
    }

    return audio_stream_get_timestamp(runtime->stream_dev);
}

int audio_sink_user_cb(AHANDLE handle, enum astream_cb_event event)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    AENTRY;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;

    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x09236f, "", "audio sink user cb, no runtime\n");
        return AIC_ENODATA;
    }

    audio_sink_finish(runtime, !runtime->user_cb);

    if (runtime->user_cb)
        runtime->user_cb(runtime->cb_data, event);

    AEXIT;

    return 0;
}

struct asink_buf *audio_sink_get_buf(AHANDLE handle)
{
    struct astream_runtime *runtime;
    struct audio_handle *audio_handle = (struct audio_handle *)&handle;

    runtime = get_runtime((enum astream_type)audio_handle->type);
    if (runtime)
        return runtime->abuf;

    return NULL;
}

void audio_sink_server_init(void)
{
    struct astream_property *property;
    enum astream_type type;

    for (type = 0; type < ASTREAM_TYPE_OUTPUT_MAX; type++) {
        property = apolicy_get_stream_property(type);
        if (property && !property->is_dynamic && property->has_stream)
            audio_sink_init_server(type);
    }
}

#ifdef CONFIG_PLAYBACK_EFFECT
int audio_sink_param_update(void *param)
{
    struct astream_runtime *runtime;
    int type;

    for (type = 0; type < ASTREAM_TYPE_OUTPUT_MAX; type++) {
        runtime = get_runtime(type);
        if (!runtime)
            continue;
        switch (runtime->type) {
        case ASTREAM_TYPE_MUSIC:
        case ASTREAM_TYPE_SYSTEM:
        case ASTREAM_TYPE_ALARM:
        case ASTREAM_TYPE_VOICE:
        case ASTREAM_TYPE_VOIP:
            if (runtime->hae)
                audio_nr_control(runtime->hae, AIC_VOICE_CMD_NV, param);
            break;
        default:
            break;
        }
    }

    return 0;
}
#endif

int audio_sink_init(void)
{
    audio_sink_server_init();

    return 0;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(audio_sink_init);

