/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio record module, it provided record API for
 *  audio service.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0c437c
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
#include "audio_nrec.h"
#include "audio_resample.h"

#define AREC_EVENT_START       0x1u
#define AREC_EVENT_STOP        0x2u
#define AREC_EVENT_PAUSE       0x4u
#define AREC_EVENT_RESUME      0x8u
#define AREC_EVENT_CLOSE       0x10u
#define AREC_EVENT_SERVER_EXIT 0x20u
#define AREC_EVENT_SERVER_STOP 0x40u
#define AREC_EVENT_SERVER_PAUSE 0x80u

#define CHECK_HANDLE(x)
#define CHECK_TYPE(type)

#define AREC_BUFF_SIZE              2048

#ifdef CONFIG_MCU_AUDIO
#define AREC_SERVICE_STACK_SIZE     4096
#else
#define AREC_SERVICE_STACK_SIZE     2048
#endif
#define AREC_THREAD_QUEUE_NUM       5
#define AREC_THREAD_PRI             5

#define AREC_TYPE_ID(type)          (type - ASTREAM_TYPE_INPUT_BASE)

struct arec_runtime {
    AHANDLE handle;
    int stream_dev;

    struct asink_buf *abuf;

    struct astream_property *property;
    struct apolicy_stream_config *config;

    char *data_buf;
    u32 data_len;
    char *file_name;
    u32 pos;
    enum store_type store_type;
    enum astream_state state;

    enum astream_type type;
    enum astream_format format;
    enum astream_source source;
    enum astream_channel chn;
    enum astream_direct direct;
    u32 flag;
    u32 sample_rate;
    u32 sample_rate_hw;

    ACLIENT aclient;
    u32 client_data;

    astream_user_cb user_cb;
    u32 cb_data;

    void *stack;
    aos_thread_id thread;
    aos_event_ptr event;
    aos_mutex_ptr lock;
#ifdef CONFIG_REC_EFFECT
    void *hae;
    void *ae_buf;
    void *hrsmpl;
#endif
};

/* TODO: each thread has it own buf */
static char arec_buf[AREC_BUFF_SIZE + 16];

static void *g_arec_runtime[ASTREAM_TYPE_INPUT_MAX];
/* for debug */
static AHANDLE g_arec_handle[ASTREAM_TYPE_INPUT_MAX];

const static char *arec_thread_name[ASTREAM_TYPE_INPUT_MAX] = {
    "record",
    "voice_ul",
    "voip_ul"
};

struct arec_runtime * audio_record_server_create(enum astream_type type);

static inline struct arec_runtime *get_arec_runtime(enum astream_type type)
{
    return g_arec_runtime[AREC_TYPE_ID(type)];
}

#ifdef CONFIG_VOICE_STREAM
int audio_get_voice_ul_dev(void)
{
    struct arec_runtime *runtime;

    runtime = get_arec_runtime(ASTREAM_TYPE_VOICE_UL);
    if (runtime)
        return runtime->stream_dev;

    return 0;
}
#endif

struct asink_buf *audio_record_get_buf(AHANDLE handle)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle = (struct audio_handle *)&handle;

    runtime = get_arec_runtime((enum astream_type)audio_handle->type);
    if (runtime)
        return runtime->abuf;

    return NULL;
}

static u32 audio_record_wait(struct arec_runtime *runtime, u32 event)
{
    u32 actual_flags = 0;

    aos_get_event(runtime->event, event, AOS_OR_CLEAR, &actual_flags,
                  AUDIO_WAIT_FOREVER);

    return actual_flags;
}

static u32 audio_record_notify(struct arec_runtime *runtime, u32 event)
{
    aos_set_event(runtime->event, event, AOS_OR);

    return 0;
}

static int audio_record_callback(enum astream_cb_event event, AHANDLE handle)
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

void audio_record_save_user_data(struct arec_runtime *runtime,
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
        break;
    }
    case APLAY_TYPE_FILE: {
        struct audio_play_sub_file *sub =
            (struct audio_play_sub_file *)aplay_data;
        runtime->user_cb = (astream_user_cb)sub->user_cb;
        runtime->cb_data = sub->cb_data;
        break;
    }
    default:
        runtime->user_cb = NULL;
        break;
    }
}

static u32 audio_record_get_period_size(struct arec_runtime *runtime)
{
    u32 period_size;

#ifdef CONFIG_MCU_AUDIO
    return runtime->config->period_size * runtime->chn *
                         (runtime->sample_rate_hw / 8000);
#endif

    if (runtime->format == ASTREAM_FORMAT_PCM || runtime->format == ASTREAM_FORMAT_WAV)
        period_size = runtime->config->period_size * runtime->chn *
                             (runtime->sample_rate / 8000);
    else
        period_size = runtime->config->period_size;

    return period_size;
}

static void audio_record_thread_loop(void *data)
{
    struct arec_runtime *runtime = (struct arec_runtime *)data;
    u32 len = 320, period_size;
    u32 flag = 0;
    int ret, client_cb = 0;
    void *pbuf;

    if (!runtime)
        return;

    while (1) {

        AIDXD(0x1d7cc4, "", "arec thread loop wait...\n");

        flag = audio_record_wait(runtime,
                              AREC_EVENT_START | AREC_EVENT_STOP |
                              AREC_EVENT_PAUSE | AREC_EVENT_RESUME |
                              AREC_EVENT_CLOSE);
        if (flag & (AREC_EVENT_START | AREC_EVENT_RESUME)) {
            /* Active only, Nothing to do */
            if (!runtime->aclient)
                continue;
            period_size = audio_record_get_period_size(runtime);
        } else if (flag & AREC_EVENT_CLOSE) {
            /* do not exit thread loop */
            audio_record_notify(runtime, AREC_EVENT_SERVER_EXIT);
            break;
        } else {
            continue;
            /* Nothing else to do, continue wait */
        }

        ADEBUG;
        while (runtime->state == ASTREAM_PLAYING) {
            len = period_size;
            pbuf = arec_buf;
            ret = audio_stream_read(runtime->stream_dev, pbuf, len);
            /* if get, send to upper layer */
            if (ret >= 0) {
                len = ret;
                AIDXD(0x173f71, "d", "read notify:%d\n", len);

#ifdef CONFIG_REC_EFFECT
                if (runtime->hae) {
                    void *pbuf_out = runtime->ae_buf;

                    audio_ec_proc(runtime->hae, pbuf, 0, len,
                                  runtime->ae_buf, &len);
                    if (runtime->hrsmpl) {
                        audio_data(pbuf_out, len, 0);
                        ret = audio_rsmpl_run(runtime->hrsmpl, pbuf_out,
                                              len >> 1, pbuf, &len);
                        len = len << 1;
                        if (ret < 0 || len == 0) {
                            AIDXE(0x01a261, "d", "record rsmpl error, ret=%d\n", ret);
                            continue;
                        }
                    } else
                        pbuf = pbuf_out;
                }
#endif
                if ((runtime->type == ASTREAM_TYPE_VOICE_UL ||
                    runtime->type == ASTREAM_TYPE_VOIP_UL) &&
                    audio_is_mute_ul()) {
                    memset(pbuf, 0, len);
                }
                client_cb = audio_client_callback(runtime->aclient,
                                      AUDIO_CLIENT_CMD_DATA_NOTIFY,
                                      pbuf, &len);
                if (client_cb == ASTREAM_DATA_EOS) {
                    audio_record_callback(ASTREAM_CB_EVENT_FINISH,
                                          runtime->handle);
                    break;
                }
            } else {
                /**
                 * TODO:
                 * 1. restart audio stream ?
                 * 2. ignore and go on?
                 */
                AIDXE(0x08285b, "d", "rec could not get data, ret=%d\n", ret);
                /* TBD */
                /* audio_record_callback(ASTREAM_CB_EVENT_BREAK, runtime->handle);
                break;
                */
            }
        }
        if (runtime->state == ASTREAM_STOPPING || client_cb == ASTREAM_DATA_EOS)
            audio_record_notify(runtime, AREC_EVENT_SERVER_STOP);
        else if (runtime->state == ASTREAM_PAUSE)
            audio_record_notify(runtime, AREC_EVENT_SERVER_PAUSE);
    }
    AEXIT;
}

AHANDLE audio_record_open(ACLIENT aclient, enum astream_type type,
                        enum aplay_type aplay_type,
                        u32 flag, enum astream_format format, u32 sample_rate,
                        u32 bit_rate, enum astream_channel chn,
                        enum astream_direct dir)
{
    struct arec_runtime *runtime;
    struct astream_property *property;
    struct stream_config config;
    AHANDLE handle = 0;
    u32 stream_flag;
    int stream_chn, stream_dev;

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
        AIDXE(0x1b79b7, "", "arecord cannot get property\n");
        return 0;
    }

    if (property->is_dynamic)
        runtime = audio_record_server_create(type);
    else
        runtime = get_arec_runtime(type);
    if (!runtime) {
        AIDXE(0x18636b, "", "arecord no runtime\n");
        return 0;
    }

    runtime->aclient = aclient;
    /* save input param */
    runtime->type = type;
    runtime->sample_rate = sample_rate;
    runtime->format = format;
    runtime->flag = flag;
    runtime->chn = chn;

    runtime->config = (struct apolicy_stream_config *)
        apolicy_get_config(type, format, ASTREAM_DIR_RECORD, flag);

#if defined(CONFIG_MCU_AUDIO) && defined(CONFIG_RECORD_FIX_16K)
    config.sample_rate = 16000;
#else
    config.sample_rate = sample_rate;
#endif
    runtime->sample_rate_hw = config.sample_rate;
    config.chn = chn;
    config.period_cnt = runtime->config->period_cnt;
    config.period_size = audio_record_get_period_size(runtime);
    config.start_threshold = runtime->config->start_threshold;
    config.stop_threshold = runtime->config->stop_threshold;
    config.bit_rate = bit_rate;

    if (aplay_type == APLAY_TYPE_STREAM) {
        struct apolicy_stream_config *cfg = (struct apolicy_stream_config *)
            apolicy_get_compr_config(type, format, ASTREAM_DIR_RECORD, flag);

        runtime->abuf = abuf_create(cfg->period_size, cfg->period_cnt);
        if (!runtime->abuf) {
            AIDXE(0x0813bc, "", "audio sink create buf failed\n");
            return AIC_ENOMEM;
        }
    }

    if (flag & AUDIO_INPUT_FLAG_COMPRESS) {
        stream_flag = COMPR_IN;
        config.fmt = format;
    } else {
        stream_flag = PCM_IN;
        config.fmt = APOLICY_DEFAULT_PCM_FORMAT; /* fixed value */
    }

#ifndef CONFIG_MCU_AUDIO
    /* if voice is PCM format, regard it as getting data from DSP */
    if (type == ASTREAM_TYPE_VOICE_UL && format == ASTREAM_FORMAT_PCM) {
        stream_flag = COMPR_OUT;
        config.fmt = ASTREAM_FORMAT_AMR_WB;
    }
#endif
    stream_dev = audio_stream_open(stream_chn, &config, stream_flag);
    if (stream_dev) {
        handle = audio_create_handle(type, flag, format, ASTREAM_DIR_RECORD,
                                     ASTREAM_SOURCE_DEFAULT);
        runtime->handle = handle;
        runtime->state = ASTREAM_OPEN;
        runtime->stream_dev = stream_dev;
        /* save it for debug */
        g_arec_handle[AREC_TYPE_ID(type)] = handle;
    } else {
        /* first delete thread if has */
        if (runtime->property->is_dynamic && runtime->property->has_stream)
            aos_delete_thread(runtime->thread);

        /* then delete mutex and event if need */
        if (runtime->property->is_dynamic) {
            aos_delete_event(runtime->event);
            aos_delete_mutex(runtime->lock);
            g_arec_runtime[AREC_TYPE_ID(type)] = 0;
            audio_free(runtime);
        }

        AIDXE(0x070348, "", "arecord: audio stream open failed\n");

        return 0;
    }

#ifdef CONFIG_REC_EFFECT
    if (type == ASTREAM_TYPE_RECORD) {
        u32 rsmp_frm = (config.period_size * config.chn *
                       (config.sample_rate / 8000)) >> 1;
        if (config.sample_rate != runtime->sample_rate)
            runtime->hrsmpl = audio_rsmpl_open(config.sample_rate,
                                               runtime->sample_rate,
                                               config.chn, rsmp_frm);
        runtime->hae = audio_ec_create(config.sample_rate, config.chn, 0);
        runtime->ae_buf = audio_malloc(config.period_size);
    }
#endif

    AEXIT;

    return handle;
}

int audio_record_set_param(AHANDLE handle, void *param)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_arec_runtime(audio_handle->type);
    if (!runtime) {
        AIDXE(0x1b9770, "", "record start, no runtime\n");
        return AIC_ENODATA;
    }
    mutex_lock(&runtime->lock);
    /* TODO */
    mutex_unlock(&runtime->lock);

    return 0;
}

int audio_record_start(AHANDLE handle, enum aplay_type aplay_type,
                       void *aplay_data)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;
    int ret = 0;

    AENTRY;

    CHECK_HANDLE(ahandle);

    /* TODO: should check state here */

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_arec_runtime(audio_handle->type);
    if (!runtime) {
        AIDXE(0x1b9770, "", "record start, no runtime\n");

        return AIC_ENODATA;
    }
    mutex_lock(runtime->lock);

    if (runtime->state == ASTREAM_PLAYING || runtime->state == ASTREAM_IDLE) {
        AIDXE(0x0b626a, "d", "record start error, wrong state = %d\n", runtime->state);
        ret = AIC_EPERM;
        goto out;
    }
    audio_record_save_user_data(runtime, aplay_type, aplay_data);

    /* First, add handle to audio policy */
    apolicy_add_stream_handle(handle);
    apolicy_device_update();
    /* Then, update param, but only record type now , but when flag is voice record not update param*/
    if ((runtime->type == ASTREAM_TYPE_RECORD) &&
        ((runtime->flag & AUDIO_INPUT_FLAG_VOREC) == 0))
        apolicy_param_update();

    ret = audio_stream_start(runtime->stream_dev);
    if (ret) {
        AIDXE(0x0c5159, "d", "record start failed, ret=%d\n", ret);
        goto out;
    }

    if (runtime->property->has_stream) {
        if (runtime->state == ASTREAM_OPEN) {
            runtime->state = ASTREAM_PLAYING;

            audio_record_notify(runtime, AREC_EVENT_START);
        } else if (runtime->state == ASTREAM_PAUSE) {
            /* TODO */
        } else {
            /* invalide state */
            ret = AIC_EPERM;
        }
    }

out:
    if (ret)
        apolicy_delete_stream_handle(handle);

    mutex_unlock(runtime->lock);

    AIDXD(0x0f567f, "d", "record, start exit, ret=%d\n", ret);

    return ret;
}

int audio_record_stop(AHANDLE handle)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    AENTRY;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;

    /* TODO: check state */

    runtime = get_arec_runtime(audio_handle->type);
    if (!runtime) {
        AIDXE(0x0fb440, "", "record stop, no runtime\n");
        return AIC_ENODATA;
    }

    mutex_lock(runtime->lock);

    /* stop server first */
    if (runtime->property->has_stream) {
        if (runtime->state == ASTREAM_PLAYING) {
            runtime->state = ASTREAM_STOPPING;
            /* do not need notify, set state only */
            /* audio_record_notify(runtime, AREC_EVENT_STOP); */
            audio_record_wait(runtime, AREC_EVENT_SERVER_STOP);
        }
        runtime->state = ASTREAM_OPEN;
    }

    /* then hardware */
    audio_stream_stop(runtime->stream_dev);
    apolicy_delete_stream_handle(handle);
    ADEBUG

    mutex_unlock(runtime->lock);

    AEXIT;

    return 0;
}

int audio_record_pause(AHANDLE handle)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;
    int ret = 0;

    AENTRY;

    CHECK_HANDLE(ahandle);

    /* TODO: should check state here */

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_arec_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x1b9770, "", "record start, no runtime\n");

        return AIC_ENODATA;
    }

    mutex_lock(runtime->lock);

    if (runtime->state == ASTREAM_PLAYING) {
        runtime->state = ASTREAM_PAUSE;
        /* do not need notify, set state only */
        audio_record_wait(runtime, AREC_EVENT_SERVER_PAUSE);

        ret = audio_stream_pause(runtime->stream_dev);
        if (ret) {
            AIDXE(0x13718f, "d", "record stream pause failed, ret=%d\n", ret);

            goto out;
        }
    } else {
        AIDXE(0x17fd9c, "d", "record pause failed, invalid state = %d\n", runtime->state);

        ret = AIC_EPERM;
    }

out:
    mutex_unlock(runtime->lock);

    return ret;
}

int audio_record_resume(AHANDLE handle)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;
    int ret = 0;

    AENTRY;

    CHECK_HANDLE(ahandle);

    /* TODO: should check state here */

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_arec_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x1b9770, "", "record start, no runtime\n");

        return AIC_ENODATA;
    }

    mutex_lock(runtime->lock);

    if (runtime->state == ASTREAM_PAUSE) {
        ret = audio_stream_resume(runtime->stream_dev);
        if (ret) {
            AIDXE(0x05cadd, "d", "record stream resume failed, ret=%d\n", ret);

            goto out;
        }
        runtime->state = ASTREAM_PLAYING;
        audio_record_notify(runtime, AREC_EVENT_RESUME);
    } else {
        AIDXE(0x0789e8, "d", "record resume failed, invalid state = %d\n", runtime->state);

        ret = AIC_EPERM;
    }

out:
    mutex_unlock(runtime->lock);

    return ret;
}


int audio_record_close(AHANDLE handle)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    AENTRY;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;

    runtime = get_arec_runtime(audio_handle->type);
    if (!runtime) {
        AIDXE(0x0fb440, "", "record stop, no runtime\n");
        return AIC_ENODATA;
    }

    mutex_lock(runtime->lock);
    if (runtime->state == ASTREAM_OPEN) {
        /* close driver, it can unblock record server. */
        audio_stream_close(runtime->stream_dev);

        runtime->state = ASTREAM_IDLE;

        /* if dynamic and has stream, notify stream thread to exit */
        if (runtime->property->is_dynamic && runtime->property->has_stream) {
            /* notify thread to exit */
            audio_record_notify(runtime, AREC_EVENT_CLOSE);
            /**
             * record server thread priority is high, now wait it to exit.
             * it can be deleted when back to here.
             */
            audio_record_wait(runtime, AREC_EVENT_SERVER_EXIT);

            /* now thread can be deleted */
            aos_delete_thread(runtime->thread);
        }
    }

    /* if dynamic stream, realease system resource */
    if (runtime->property->is_dynamic) {
        aos_delete_event(runtime->event);
        g_arec_handle[AREC_TYPE_ID(audio_handle->type)] = 0;
        g_arec_runtime[AREC_TYPE_ID(audio_handle->type)] = 0;
        if (runtime->abuf)
            abuf_free(runtime->abuf);
    }

#ifdef CONFIG_REC_EFFECT
    if (runtime->hae)
        audio_ec_delete(runtime->hae);
    if (runtime->ae_buf)
        audio_free(runtime->ae_buf);
    if (runtime->hrsmpl)
        audio_rsmpl_close(runtime->hrsmpl);
#endif

    ADEBUG
    mutex_unlock(runtime->lock);

    if (runtime->property->is_dynamic) {
        aos_delete_mutex(runtime->lock);
        audio_free(runtime);
    }

    AEXIT;

    return 0;
}

u32 audio_record_get_time(AHANDLE handle)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;
    runtime = get_arec_runtime((enum astream_type)audio_handle->type);
    if (!runtime) {
        AIDXE(0x18baaf, "", "sink get time, no runtime\n");
        return 0;
    }

    return audio_stream_get_timestamp(runtime->stream_dev);
}

#ifdef CONFIG_REC_EFFECT
int audio_record_param_update(void *param)
{
    struct arec_runtime *runtime;
    int type;

    for (type = 0; type < ASTREAM_TYPE_INPUT_MAX; type++) {
        runtime = g_arec_runtime[type];
        if (!runtime)
            continue;
        switch (runtime->type) {
        case ASTREAM_TYPE_RECORD:
            if (runtime->hae)
                audio_ec_control(runtime->hae, AIC_VOICE_CMD_NV, param);
            break;
        default:
            break;
        }
    }
    return 0;
}
#endif

int audio_record_user_cb(AHANDLE handle, enum astream_cb_event event)
{
    struct arec_runtime *runtime;
    struct audio_handle *audio_handle;
    AHANDLE ahandle = handle;

    AENTRY;

    CHECK_HANDLE(ahandle);

    audio_handle = (struct audio_handle *)&ahandle;

    runtime = get_arec_runtime(audio_handle->type);
    if (!runtime) {
        AIDXE(0x04930b, "", "record user cb, no runtime\n");
        return AIC_ENODATA;
    }

    /**
     * TODO: stop client or wait for user to stop?
     *       now stop client, but not close, wait user to close.
     */
    audio_record_stop(handle);

    if (runtime->user_cb)
        runtime->user_cb(runtime->cb_data, event);

    /* if user callback is null, we must close auto */
    if (!runtime->user_cb && runtime->property->is_dynamic)
        audio_record_close(handle);

    AEXIT;

    return 0;
}

struct arec_runtime * audio_record_server_create(enum astream_type type)
{
    struct arec_runtime *runtime;
    struct astream_property *property;
    char *name;

    if (g_arec_runtime[AREC_TYPE_ID(type)]) {
        AIDXE(0x09f22b, "", "arec thread already created\n");
        return 0;
    }

    property = apolicy_get_stream_property(type);
    if (!property) {
        AIDXE(0x0a9b33, "", "cannot get record property!\n");
        return 0;
    }
    runtime = audio_malloc(sizeof(struct arec_runtime));
    if (!runtime) {
        AIDXE(0x19af4b, "", "record server init, no memory!\n");
        return 0;
    }
    memset(runtime, 0, sizeof(*runtime));
    runtime->property = property;

    name = (char *)arec_thread_name[AREC_TYPE_ID(type)];
    runtime->event = aos_create_event(name);
    if (property->has_stream) {
        runtime->thread = aos_create_thread(name,
                                            audio_record_thread_loop, runtime,
                                            AREC_SERVICE_STACK_SIZE,
                                            AREC_THREAD_QUEUE_NUM,
                                            name,
                                            AREC_THREAD_PRI,
                                            AOS_AUTO_START);
    }

    runtime->lock = aos_create_mutex(name);

    g_arec_runtime[AREC_TYPE_ID(type)] = (void *)runtime;

    return runtime;
}

int audio_record_server_init(void)
{
    struct astream_property *property;
    enum astream_type type;

    for (type = ASTREAM_TYPE_INPUT_BASE; type < ASTREAM_TYPE_MAX; type++) {
        property = apolicy_get_stream_property(type);
        if (property && !property->is_dynamic && property->has_stream)
            audio_record_server_create(type);
    }

    return 0;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(audio_record_server_init);

