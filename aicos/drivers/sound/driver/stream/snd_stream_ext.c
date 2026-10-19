/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines external stream interface, we should register it  to
 *    sound card that can be called during stream playing.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1cf0bd
#define LOG_MODULE    AMOD_SOUND

#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <dma.h>
#include <cache.h>
#ifdef CONFIG_CMUX
#include <cmux.h>
#include <cmux_cfg.h>
#endif
#include "snd_compat.h"
#include "snd_stream.h"
#include "snd_stream_driver.h"

struct ext_rtd {
    struct snd_stream *stream;
    int dir;

    u32 app_pointer;
    u32 buffer_size;

    u32 copied_total;
    u32 dsp_copied_total;

    int sample_rate;

    void *ext_dev;
    aos_semaphore_ptr   semph;
};

static u8 *g_p_tmp_buf = NULL;

static int stream_ext_set_params(struct snd_stream *stream,
                                 struct snd_stream_params *stream_params)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct snd_pcm_params *params;
    u32 buff_size;
    int ret = 0;

    params = (struct snd_pcm_params *)stream_params;
    buff_size = params->period_size * params->period_cnt;


    runtime->buffer_size = buff_size;
    runtime->fragment_size = params->period_size;
    runtime->fragments = params->period_cnt;

    return ret;
}


static int stream_ext_trigger(struct snd_stream *stream, int cmd)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct ext_rtd *rtd = runtime->private_data;
    int ret = 0;

    SND_ENTRY;

    switch (cmd) {
    case SNDRV_STREAM_TRIGGER_START:
        SNX_INFO(0x195516, "s", "%s: SNDRV_STREAM_TRIGGER_START\n", __func__);
        break;
    case SNDRV_STREAM_TRIGGER_STOP:
        SNX_INFO(0x0855ad, "s", "%s: SNDRV_STREAM_TRIGGER_STOP\n", __func__);
        rtd->dsp_copied_total = 0;
        rtd->copied_total = 0;
        rtd->app_pointer = 0;
        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_PUSH:
        SNX_INFO(0x06d7de, "s", "%s: SNDRV_STREAM_TRIGGER_PAUSE_PUSH\n", __func__);
        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_RELEASE:
        SNX_INFO(0x1bfa33, "s", "%s: SNDRV_STREAM_TRIGGER_PAUSE_RELEASE\n", __func__);
        break;
    case SNDRV_STREAM_TRIGGER_PARTIAL_DRAIN:
    case SNDRV_STREAM_TRIGGER_DRAIN:
        SNX_INFO(0x021b49, "s", "%s: SNDRV_STREAM_DRAIN", __func__);
        break;
    case SNDRV_STREAM_TRIGGER_NEXT_TRACK:
        SNX_INFO(0x09e677, "s", "%s: SND_COMPR_TRIGGER_NEXT_TRACK\n", __func__);
        break;
    default:
        SNX_INFO(0x0e5b3a, "sd", "%s: no responsed cmd=%d\n", __func__, cmd);
        break;
    }

    SND_EXIT;

    return ret;
}

static int stream_ext_pointer(struct snd_stream *stream,
                    struct snd_stream_tstamp *arg)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct ext_rtd *rtd = runtime->private_data;

    arg->copied_total = rtd->dsp_copied_total;
    arg->pcm_frames = rtd->dsp_copied_total;
    arg->pcm_io_frames = rtd->dsp_copied_total;

    return 0;
}

#ifdef CONFIG_SND_STREAM_CMUX
static int stream_cmux_open(struct snd_stream *stream)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct ext_rtd *rtd = NULL;

    SND_ENTRY;

    rtd = snd_zalloc(sizeof(struct ext_rtd));
    if (!rtd) {
        SNX_ERROR(0x1b07fb, "", "ext stream open failed, no memory\n");
        return AIC_ENOMEM;
    }
    rtd->stream = stream;
    runtime->private_data = rtd;

    rtd->ext_dev = aic_cmux_open(AIC_CMUX_VOICE_COM);
    if (!rtd->ext_dev) {
        SNX_ERROR(0x029be6, "", "ext dev open failed!\n");
        return AIC_EPIPE;
    }

    if(NULL == g_p_tmp_buf) {
        g_p_tmp_buf = malloc(640);
        if (!g_p_tmp_buf) {
            SNX_ERROR(0x083f74, "", "malloc temp bug failed!\n");
            return AIC_ENOMEM;
        }
    }

    SND_EXIT;

    aic_get_wakelock();

    return 0;
}

static int stream_cmux_free(struct snd_stream *stream)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct ext_rtd *rtd = runtime->private_data;

    if (!rtd)
        return 0;

    if (rtd->ext_dev)
        aic_cmux_close(rtd->ext_dev);

    if (g_p_tmp_buf) {
        free(g_p_tmp_buf);
        g_p_tmp_buf = NULL;
    }

    snd_free(rtd);

    aic_put_wakelock();

    return 0;
}

static int stream_cmux_copy(struct snd_stream *stream, char *buf, size_t count)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct ext_rtd *rtd = runtime->private_data;
    enum snd_stream_direction dir = stream->direction;
    u32 sent = count;

    if (dir == SND_STREAM_PLAYBACK) {
        if (rtd->ext_dev) {
            if (g_p_tmp_buf) {
                aic_cmux_read(rtd->ext_dev, g_p_tmp_buf, 640, 22);
            }
            sent = aic_cmux_write(rtd->ext_dev, buf, count, 41);
        }
    }
    if (sent) {
        rtd->copied_total += count;
        rtd->dsp_copied_total = rtd->copied_total;
    }
    SNX_DBG(0x141b22, "sdddd", "%s, dir=%d, count=%d, sent=%d, copied_total=%d\n",
            __func__, dir, count, sent, rtd->copied_total);

    return count;
}

static struct snd_stream_ops stream_cmux_ops = {
    .open = stream_cmux_open,
    .free = stream_cmux_free,
    .set_params = stream_ext_set_params,
    .trigger = stream_ext_trigger,
    .pointer = stream_ext_pointer,
    .copy = stream_cmux_copy,
};
#endif

#ifdef CONFIG_SND_STREAM_VOICE_MIX
static void *g_p_voice_mix_rtd;

extern void voice_proc_mix_start(void);
extern void voice_proc_mix_stop(void);

static int stream_voice_mix_open(struct snd_stream *stream)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct ext_rtd *rtd = NULL;

    rtd = snd_zalloc(sizeof(struct ext_rtd));
    if (!rtd) {
        SNX_ERROR(0x1b07fb, "", "ext stream open failed, no memory\n");
        return AIC_ENOMEM;
    }
    rtd->stream = stream;
    runtime->private_data = rtd;
    g_p_tmp_buf = NULL;
    g_p_voice_mix_rtd = rtd;
    rtd->semph = aos_create_semaphore("voice mix semph", 0);
    voice_proc_mix_start();
    AIC_ASSERT(rtd->semph);

    return 0;
}

static int stream_voice_mix_free(struct snd_stream *stream)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct ext_rtd *rtd = runtime->private_data;

    voice_proc_mix_stop();

    g_p_voice_mix_rtd = 0;
    g_p_tmp_buf = 0;

    if (!rtd)
        return 0;

    if (rtd->semph)
        aos_delete_semaphore(rtd->semph);
    snd_free(rtd);

    return 0;
}

static int stream_voice_mix_copy(struct snd_stream *stream,
                                 char *buf, size_t count)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct ext_rtd *rtd = runtime->private_data;
    int ret;

    /* Do not copy, only save buffer pointer */
    g_p_tmp_buf = buf;
    ret = poll_wait(runtime->sleep, 100);

    rtd->copied_total += count;
    rtd->dsp_copied_total = rtd->copied_total;
    SNX_DBG(0x035eb6, "sddd", "%s, count=%d, copied_total=%d, ret=%d\n",
            __func__, count, rtd->copied_total, ret);

    aos_put_semaphore(rtd->semph);

    return count;
}

u8 *snd_stream_voice_mix_get(void)
{
    struct ext_rtd *rtd = g_p_voice_mix_rtd;

    aos_get_semaphore(rtd->semph, 21);

    return g_p_tmp_buf;
}

void snd_stream_voice_mix_release(void)
{
    struct ext_rtd *rtd = g_p_voice_mix_rtd;
    struct snd_stream_runtime *runtime = rtd->stream->runtime;

    g_p_tmp_buf = 0;
    SNX_INFO(0x0ecda8, "", "mix r\n");
    wake_up(runtime->sleep);
}

static struct snd_stream_ops stream_voice_mix_ops = {
    .open = stream_voice_mix_open,
    .free = stream_voice_mix_free,
    .set_params = stream_ext_set_params,
    .trigger = stream_ext_trigger,
    .pointer = stream_ext_pointer,
    .copy = stream_voice_mix_copy,
};
#endif

const struct snd_stream_vdev ext_vdev[] = {
#ifdef CONFIG_SND_STREAM_CMUX
    {
        .name = "ext_p0",
        .dpm_stream = NULL,
        .ops = &stream_cmux_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = NULL,
    },
#endif
#ifdef CONFIG_SND_STREAM_VOICE_MIX
    {
        .name = "voice_mix_p0",
        .dpm_stream = NULL,
        .ops = &stream_voice_mix_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = NULL,
    },
#endif
};

int stream_ext_init(void)
{
    int ret, i;

    for(i = 0; i < sizeof(ext_vdev) / sizeof(struct snd_stream_vdev); i++) {
        ret = snd_stream_register(&ext_vdev[i]);
        SNX_INFO(0x0af801, "sd", "%s, ret=%d\n", ext_vdev[i].name, ret);
    }

    return 0;
}
AIC_NORMAL_DRV_P2_INIT_EXPORT(stream_ext_init);


