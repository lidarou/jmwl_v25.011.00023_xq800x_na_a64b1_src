/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines cmux voice virtual driver, we should register it  to
 *    sound card that can be called during stream playing.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00df27
#define LOG_MODULE    AMOD_SOUND

#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <cache.h>
#include <cmux.h>
#include <cmux_cfg.h>

#include "snd_compat.h"
#include "snd_stream.h"
#include "snd_stream_driver.h"

struct cmux_rtd {
    struct snd_stream *stream;
    int dir;

    u32 app_pointer;
    u32 buffer_size;
    char *buf;

    u32 copied_total;
    u32 dsp_copied_total;

    int sample_rate;
};

static void *g_voice_cmux_handle;

static int stream_cmux_set_params(struct snd_stream *stream,
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

static int stream_cmux_trigger(struct snd_stream *stream, int cmd)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct cmux_rtd *rtd = runtime->private_data;
    int ret = 0;

    SND_ENTRY;

    switch (cmd) {
    case SNDRV_STREAM_TRIGGER_START:
        SNX_INFO(0x195516, "s", "%s: SNDRV_STREAM_TRIGGER_START\n", __func__);
        if (stream->direction == SND_STREAM_CAPTURE)
            rtd->dsp_copied_total = runtime->buffer_size;

        break;
    case SNDRV_STREAM_TRIGGER_STOP:
        SNX_INFO(0x0855ad, "s", "%s: SNDRV_STREAM_TRIGGER_STOP\n", __func__);
        rtd->copied_total = 0;
        rtd->app_pointer = 0;
        if (stream->direction == SND_STREAM_PLAYBACK)
            rtd->dsp_copied_total = 0;
        else
            rtd->dsp_copied_total = runtime->buffer_size;
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

static int stream_cmux_pointer(struct snd_stream *stream,
                    struct snd_stream_tstamp *arg)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct cmux_rtd *rtd = runtime->private_data;

    arg->copied_total = rtd->dsp_copied_total;
    arg->pcm_frames = rtd->dsp_copied_total;
    arg->pcm_io_frames = rtd->dsp_copied_total;

    return 0;
}

#ifdef CONFIG_SND_STREAM_CMUX
static int stream_cmux_open(struct snd_stream *stream)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct cmux_rtd *rtd = NULL;

    SND_ENTRY;

    rtd = snd_zalloc(sizeof(struct cmux_rtd));
    if (!rtd) {
        SNX_ERROR(0x1b07fb, "", "ext stream open failed, no memory\n");
        return AIC_ENOMEM;
    }
    memset(rtd, 0, sizeof(*rtd));
    rtd->stream = stream;
    runtime->private_data = rtd;

    if (!g_voice_cmux_handle) {
        g_voice_cmux_handle = aic_cmux_open(AIC_CMUX_VOICE_COM);
        if (!g_voice_cmux_handle) {
            SNX_ERROR(0x029be6, "", "ext dev open failed!\n");
            return AIC_EPIPE;
        }
    }

    SND_EXIT;

    aic_get_wakelock();

    return 0;
}

static int stream_cmux_free(struct snd_stream *stream)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct cmux_rtd *rtd = runtime->private_data;


    if (g_voice_cmux_handle) {
        aic_cmux_close(g_voice_cmux_handle);
        g_voice_cmux_handle = 0;
    }

    aic_put_wakelock();

    if (rtd)
        snd_free(rtd);

    return 0;
}

static int stream_cmux_copy(struct snd_stream *stream, char *buf, size_t count)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct cmux_rtd *rtd = runtime->private_data;
    enum snd_stream_direction dir = stream->direction;
    u32 rw = 0;

    if (!g_voice_cmux_handle) {
        SNX_ERROR(0x064fd4, "", "voice cmux handle not opened\n");
        aos_sleep(5);
        return 0;
    }

    if (dir == SND_STREAM_PLAYBACK) {
        rw = aic_cmux_write(g_voice_cmux_handle, buf, count, 41);
        if (!rw) {
            SNX_ERROR(0x19c6bd, "", "voice cmux write err\n");
            aos_sleep(5);
        }
        rtd->copied_total += count;
        rtd->dsp_copied_total = rtd->copied_total;
    } else {
        rw = aic_cmux_read(g_voice_cmux_handle, buf, count, 41);
        if (!rw) {
            SNX_ERROR(0x1173d3, "", "voice cmux read err\n");
            aos_sleep(5);
        }
        rtd->copied_total += count;
        rtd->dsp_copied_total += count;
    }

    SNX_INFO(0x1a3b37, "sdddd", "%s, dir=%d, count=%d, rw=%d, copied_total=%d\n",
            __func__, dir, count, rw, rtd->copied_total);

    return count;
}

static struct snd_stream_ops stream_cmux_ops = {
    .open = stream_cmux_open,
    .free = stream_cmux_free,
    .set_params = stream_cmux_set_params,
    .trigger = stream_cmux_trigger,
    .pointer = stream_cmux_pointer,
    .copy = stream_cmux_copy,
};
#endif

const struct snd_stream_vdev cmux_vdev[] = {
    {
        .name = "extv_p0",
        .dpm_stream = NULL,
        .ops = &stream_cmux_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = NULL,
    },
    {
        .name = "extv_c0",
        .dpm_stream = NULL,
        .ops = &stream_cmux_ops,
        .direction = SND_STREAM_CAPTURE,
        .private_data = NULL,
    },
};

int stream_cmux_init(void)
{
    int ret, i;

    for(i = 0; i < sizeof(cmux_vdev) / sizeof(struct snd_stream_vdev); i++) {
        ret = snd_stream_register(&cmux_vdev[i]);
        SNX_INFO(0x0af801, "sd", "%s, ret=%d\n", cmux_vdev[i].name, ret);
    }

    return 0;
}
AIC_NORMAL_DRV_P2_INIT_EXPORT(stream_cmux_init);

