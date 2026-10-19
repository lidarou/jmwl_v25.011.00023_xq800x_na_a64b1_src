/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines PCM stream interface, we should register it  to
 *    sound card that can be called during stream playing.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x071d24
#define LOG_MODULE    AMOD_SOUND

#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <dma.h>
#include <cache.h>
#include <dvfs_inc.h>

#include "snd_compat.h"
#include "snd_stream.h"
#include "snd_stream_driver.h"
#include "snd_dai.h"
#ifdef CONFIG_SOUND_CONV
#include "snd_data_conv.h"
#endif

#define PERIOD_SIZE_MAX     3840
#define PERIOD_COUNT_MAX    8

enum stream_state {
    STATE_IDLE = 0,
    STATE_OPENNED,
    STATE_CLOSED,
    STATE_STOPPED,
    STATE_PARAMSED,
    STATE_TRIGGERED,
    STATE_PARAMIZED,
    STATE_ABORT,
};

struct pcm_rtd {
    struct snd_stream *stream;
    int dir;
    enum stream_state stream_state;

    u32 app_pointer;
    u32 buffer_size;

    u32 copied_total;
    u32 dsp_copied_total;

    int sample_rate;
    u32 dma_chn;
    void *dma_cfg;

#ifdef CONFIG_SOUND_CONV
    void *proc_buf;
    u32 left_size;
#endif
    u32 irq_cnt;
};

void pcm_buf_done(void *param);

/* These are temp codes for simu */

/* #define SND_SIMU */

#ifdef SND_SIMU
struct dma_config {
    u32 period_count;
    u32 period_size;

    aos_thread_id th_dma;
    aos_thread_id th_breath;
    u32 breath_cnt;
};

struct dma_config g_dma_config;

static void stream_thread_loop(u32 data)
{
    struct pcm_rtd *rtd = (struct pcm_rtd *)data;

    SNX_DBG(0x0d4461, "s", "%s, entry\n", __func__);

    g_dma_config.breath_cnt = 0;
    while(rtd->stream_state == STATE_TRIGGERED) {
        SNX_DBG(0x1b4641, "s", "%s, before sleep\n", __func__);
        aos_sleep(500);
        pcm_buf_done(rtd);
    }
}

void stream_breath_thread_loop(u32 data)
{
    struct pcm_rtd *rtd = (struct pcm_rtd *)data;

    SNX_DBG(0x0d4461, "s", "%s, entry\n", __func__);

    while(rtd->stream_state == STATE_TRIGGERED) {
        aos_sleep(100);
        g_dma_config.breath_cnt++;
    }
}

static int dma_config(struct snd_stream *stream, struct snd_pcm_params *params)
{
    g_dma_config.period_count = params->period_cnt;
    g_dma_config.period_size = params->period_size;

    return 0;
}

static int dma_start(u32 dma_chn, u32 data)
{
    g_dma_config.th_dma =
        aos_create_thread("pcm stream dma", stream_thread_loop,
                          data, 2048, 5, "pcm stream dma queue", 10,
                          AOS_AUTO_START);
    g_dma_config.th_breath =
        aos_create_thread("pcm stream breath", stream_breath_thread_loop,
                          data, 1024, 5, "pcm stream breath queue", 10,
                          AOS_AUTO_START);

    return 0;
}

static int dma_get_addr(u32 dma_chn)
{
    return g_dma_config.period_size *
        (g_dma_config.breath_cnt % g_dma_config.period_count);
}

static int dma_stop(u32 dma_chn)
{
    return 0;
}

static int dma_close(u32 dma_chn)
{
    aos_delete_thread(g_dma_config.th_breath);
    aos_delete_thread(g_dma_config.th_dma);
    g_dma_config.th_breath = 0;
    g_dma_config.th_dma = 0;

    return 0;
}
#else
static int dma_config(struct snd_stream *stream, struct snd_pcm_params *params)
{
    struct pcm_rtd *rtd = stream->runtime->private_data;
    struct snd_dai_cfg *dai_cfg = snd_dai_get_cfg(stream->runtime->dai);
    struct dma_chn_cfg *ll_cfg;
    u32 dma_cfg_addr;

    u32 blk_len, mblk_len, frm_len;
    int i, ll_node, chn, ret;

    /* force dma link list only one node */
    ll_node = 1;

    blk_len = 20 * 2;
#ifdef CONFIG_SOUND_CONV
    mblk_len = stream->runtime->fragment_size << pcm_conv_multi();
#else
    mblk_len = stream->runtime->fragment_size;
#endif
    frm_len = mblk_len * stream->runtime->fragments;

    /* mblk_len must be multiple of blk_len */
    AIC_ASSERT(!(mblk_len % blk_len));
#ifndef CONFIG_SOUND_CONV
    /* restrict max len */
    AIC_ASSERT(mblk_len <= PERIOD_SIZE_MAX);
    /* restrict max len */
    AIC_ASSERT(frm_len <= PERIOD_SIZE_MAX * PERIOD_COUNT_MAX);
#endif
    ll_cfg = (struct dma_chn_cfg *)snd_zalloc(sizeof(struct dma_chn_cfg) *
                                              ll_node * 3);
    if (!ll_cfg) {
        SNX_ERROR(0x09689e, "", "dma, no memory!\n");
        return AIC_ENOMEM;
    }
    rtd->dma_cfg = ll_cfg;

    /* 32bytes align */
    dma_cfg_addr = ALIGN((u32)ll_cfg + (sizeof(struct dma_chn_cfg) * ll_node),
                         32);

    chn = aic_dma_chn_request(pcm_buf_done, rtd);
    if (chn < AIC_SUCCESS) {
        SNX_ERROR(0x162f1c, "", "DMA request channel failed!");
        snd_free(rtd->dma_cfg);
        rtd->dma_cfg = 0;
        return AIC_ENXIO;
    }
    rtd->dma_chn = chn;

    memset(&ll_cfg[0], 0, sizeof(ll_cfg[ll_node]));

    for (i = 0; i < ll_node; i++) {
        if (stream->direction == SND_STREAM_PLAYBACK) {
            ll_cfg[i].src_addr   = (u32)stream->runtime->buffer;
            ll_cfg[i].dst_addr   = dai_cfg->phy_addr;
            ll_cfg[i].trsf_type  = DMA_MEM_TO_DEV;
        } else {
            ll_cfg[i].src_addr   = dai_cfg->phy_addr;
            ll_cfg[i].dst_addr   = (u32)stream->runtime->buffer;
            ll_cfg[i].trsf_type  = DMA_DEV_TO_MEM;
        }
        ll_cfg[i].llp        = dma_cfg_addr;
        ll_cfg[i].blk_len    = blk_len;
        ll_cfg[i].mblk_len   = mblk_len;
        ll_cfg[i].frame_len  = frm_len;
        ll_cfg[i].src_ds     = DMA_HWORD_SIZE;
        ll_cfg[i].dst_ds     = DMA_HWORD_SIZE;
        ll_cfg[i].int_type   = DMA_MBLK_INT;
        ll_cfg[i].req        = DMA_BLOCK_REQ;
        ll_cfg[i].end        = DMA_LLP_CONTINUE;
        ll_cfg[i].hwid       = dai_cfg->dma_hw_id;
    }

    ret = aic_dma_ll_cfg(chn, dma_cfg_addr, ll_cfg, ll_node);

    return ret;
}

static int dma_start(u32 dma_chn, u32 data)
{
    aic_dma_chn_resume(dma_chn);
    aic_dma_chn_start(dma_chn);

    return 0;
}

static int dma_pause(u32 dma_chn, u32 data)
{
    return aic_dma_chn_suspend(dma_chn);
}

static int dma_resume(u32 dma_chn, u32 data)
{
    aic_dma_chn_resume(dma_chn);

    return 0;
}

static int dma_get_addr(u32 dma_chn)
{
    /*
     * extern u32 iis_get_int_cnt(void);
     * return iis_get_int_cnt();
     */
    return 0;
}

static int dma_stop(u32 dma_chn)
{
    aic_dma_chn_free(dma_chn);

    return 0;
}

static int dma_close(u32 dma_chn)
{
    return 0;
}

#endif

void pcm_buf_done(void *param)
{
    struct pcm_rtd *rtd = (struct pcm_rtd *)param;
    struct snd_stream_runtime *runtime = rtd->stream->runtime;

    rtd->dsp_copied_total += runtime->fragment_size;
    if (rtd->stream->direction == SND_STREAM_PLAYBACK) {
        rtd->irq_cnt++;
        if(rtd->dsp_copied_total >= rtd->copied_total) {
            rtd->dsp_copied_total = rtd->copied_total;
            rtd->app_pointer = runtime->fragment_size *
                                    (rtd->irq_cnt % runtime->fragments);
            SNX_ERROR(0x042705, "", "U\n");
        }
    }

    /* snd_stream_fragment_elapsed(rtd->stream); */
    SNX_DBG(0x107461, "ddd", "total=%d, dsp=%d, frag=%d\n", rtd->copied_total,
            rtd->dsp_copied_total, runtime->fragment_size);
}

static int stream_pcm_open(struct snd_stream *stream)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct pcm_rtd *rtd = NULL;

    SND_ENTRY;

    rtd = snd_zalloc(sizeof(struct pcm_rtd));
    if (!rtd) {
        SNX_ERROR(0x1897dc, "", "compr open failed, no memory\n");
        return AIC_ENOMEM;
    }
    rtd->stream = stream;
    runtime->private_data = rtd;
#if defined(CONFIG_MCU_AUDIO) || defined(CONFIG_SOUND_CONV)
    snd_dvfs_set(1);
#endif
    SND_EXIT;

    return 0;
}

static int stream_pcm_free(struct snd_stream *stream)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct pcm_rtd *rtd = runtime->private_data;

    SND_ENTRY;

    if (!rtd)
        return 0;

    dma_close(rtd->dma_chn);
    if (rtd->dma_cfg)
        snd_free(rtd->dma_cfg);

    snd_free(rtd);
#if defined(CONFIG_MCU_AUDIO) || defined(CONFIG_SOUND_CONV)
    snd_dvfs_set(0);
#endif
    return 0;
}

static int stream_pcm_set_params(struct snd_stream *stream,
                                 struct snd_stream_params *stream_params)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct pcm_rtd *rtd = runtime->private_data;
    struct snd_pcm_params *params;
    u32 buff_size;
    char *buff;
    int ret = 0;

    SND_ENTRY;

    params = (struct snd_pcm_params *)stream_params;
    buff_size = params->period_size * params->period_cnt;
#ifdef CONFIG_SOUND_CONV
    pcm_conv_init(params->srate);
    buff = aos_dma_malloc((buff_size << pcm_conv_multi()) +
                          params->period_size + 16);
#else
    buff = aos_dma_malloc(buff_size);
#endif
    if (!buff) {
        SNX_ERROR(0x1de355, "", "compress no memory!\n");
        return AIC_ENOMEM;
    }
#ifdef CONFIG_SOUND_CONV
    memset(buff, 0, (buff_size << pcm_conv_multi()) + params->period_size + 16);
    rtd->proc_buf = buff + (buff_size << pcm_conv_multi());
#else
    memset(buff, 0, buff_size);
#endif
    runtime->buffer = buff;
    runtime->buffer_size = buff_size;
    runtime->fragment_size = params->period_size;
    runtime->fragments = params->period_cnt;
    ret = dma_config(stream, params);
    if (ret) {
        SNX_ERROR(0x0b0b0e, "d", "dma config error, ret=%d\n", ret);
        return ret;
    }
    rtd->stream_state = STATE_PARAMSED;

    SND_EXIT;

    return ret;
}


static int stream_pcm_trigger(struct snd_stream *stream, int cmd)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct pcm_rtd *rtd = runtime->private_data;
    int ret = 0;

    SND_ENTRY;

    switch (cmd) {
    case SNDRV_STREAM_TRIGGER_START:
        SNX_INFO(0x195516, "s", "%s: SNDRV_STREAM_TRIGGER_START\n", __func__);
        wake_lock();
        /* if STOPPED we should update DMA config because it had already
           been freed */
        if (rtd->stream_state == STATE_STOPPED)
            dma_config(stream, 0);

        rtd->stream_state = STATE_TRIGGERED;
        dma_start(rtd->dma_chn, (u32)rtd);
        break;
    case SNDRV_STREAM_TRIGGER_STOP:
        SNX_INFO(0x0855ad, "s", "%s: SNDRV_STREAM_TRIGGER_STOP\n", __func__);

        rtd->stream_state = STATE_STOPPED;
        rtd->dsp_copied_total = 0;
        rtd->copied_total = 0;
        rtd->app_pointer = 0;
        dma_stop(rtd->dma_chn);
        wake_unlock();
        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_PUSH:
        SNX_INFO(0x06d7de, "s", "%s: SNDRV_STREAM_TRIGGER_PAUSE_PUSH\n", __func__);

        dma_pause(rtd->dma_chn, 0);
        rtd->stream_state = STATE_STOPPED;
        wake_unlock();
        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_RELEASE:
        SNX_DBG(0x1bfa33, "s", "%s: SNDRV_STREAM_TRIGGER_PAUSE_RELEASE\n", __func__);
        wake_lock();
        dma_resume(rtd->dma_chn, 0);
        rtd->stream_state = STATE_TRIGGERED;
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

static int stream_pcm_pointer(struct snd_stream *stream,
                    struct snd_stream_tstamp *arg)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct pcm_rtd *rtd = runtime->private_data;

    arg->copied_total = rtd->dsp_copied_total + dma_get_addr(0);
    arg->pcm_frames = rtd->dsp_copied_total;
    arg->pcm_io_frames = rtd->dsp_copied_total;

    return 0;
}

#ifdef CONFIG_SOUND_CONV
static int stream_pcm_copy(struct snd_stream *stream, char *inbuf, size_t incnt)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct pcm_rtd *rtd = runtime->private_data;
    enum snd_stream_direction dir = stream->direction;
    u32 offs, mul, count = 0;
    char *buf;

    if (dir == SND_STREAM_PLAYBACK) {
        if (incnt > runtime->fragment_size) {
            SNX_ERROR(0x1dfd95, "dd", "pcm copy error, incnt(%d) is exceed period(%d)\n",
                      incnt, runtime->fragment_size);
            return 0;
        }
        mul = pcm_conv_multi();
        offs = rtd->app_pointer << mul;

        buf = rtd->proc_buf;
        count = (rtd->left_size + incnt) >= runtime->fragment_size ?
                runtime->fragment_size : 0;

        if (count == runtime->fragment_size) {
            memcpy(buf + rtd->left_size, inbuf, count - rtd->left_size);
            if (rtd->app_pointer + count <= runtime->buffer_size) {
                pcm_conv(buf, (char *)runtime->buffer + offs, count);
            } else {
                /* it should never get here now, but keep it for later use */
                size_t copy = runtime->buffer_size - rtd->app_pointer;

                AIC_ASSERT(!(copy % runtime->fragment_size));
                SNX_ERROR(0x0706b0, "dd", "!!!!!count=%d, copy=%d\n", count, copy);
                pcm_conv(buf, (char *)runtime->buffer + offs, copy);
                pcm_conv(buf + copy, (char *)runtime->buffer, count - copy);
            }
            dcache_clean_by_range((u32)runtime->buffer, runtime->buffer_size << mul);

            rtd->left_size = incnt - (count - rtd->left_size);
            memcpy(buf, inbuf + (incnt - rtd->left_size), rtd->left_size);
        } else {
            memcpy(buf + rtd->left_size, inbuf, incnt);
            rtd->left_size += incnt;
        }
    }

    rtd->copied_total += count;
    rtd->app_pointer += count;
    if (rtd->app_pointer >= runtime->buffer_size)
        rtd->app_pointer -= runtime->buffer_size;

    SNX_DBG(0x113c15, "sdddd", "%s, dir=%d, count=%d, app_pointer=%d, copied_total=%d\n",
            __func__, dir, count, rtd->app_pointer, rtd->copied_total);

    return count;
}

#else
static int stream_pcm_copy(struct snd_stream *stream, char *buf, size_t count)
{
    struct snd_stream_runtime *runtime = stream->runtime;
    struct pcm_rtd *rtd = runtime->private_data;
    enum snd_stream_direction dir = stream->direction;

    if (dir == SND_STREAM_PLAYBACK) {
        if (rtd->app_pointer + count <= runtime->buffer_size)
            memcpy((char *)runtime->buffer + rtd->app_pointer, buf, count);
        else {
            size_t copy = runtime->buffer_size - rtd->app_pointer;
            memcpy((char *)runtime->buffer + rtd->app_pointer, buf, copy);
            memcpy((char *)runtime->buffer, buf + copy, count - copy);
        }
        dcache_clean_by_range((u32)runtime->buffer, runtime->buffer_size);
    }
    else if (dir == SND_STREAM_CAPTURE) {
        if (rtd->dsp_copied_total - rtd->copied_total > runtime->buffer_size) {
            rtd->copied_total = rtd->dsp_copied_total - runtime->buffer_size;
            rtd->app_pointer = rtd->copied_total % runtime->buffer_size;
            SNX_ERROR(0x1e2910, "", "O\n");
        }
        dcache_inv_by_range((u32)runtime->buffer, runtime->buffer_size);
        memcpy(buf, (char *)runtime->buffer + rtd->app_pointer, count);
    }
    rtd->copied_total += count;
    rtd->app_pointer += count;
    if (rtd->app_pointer >= runtime->buffer_size)
        rtd->app_pointer -= runtime->buffer_size;

    SNX_DBG(0x113c15, "sdddd", "%s, dir=%d, count=%d, app_pointer=%d, copied_total=%d\n",
            __func__, dir, count, rtd->app_pointer, rtd->copied_total);

    return count;
}
#endif

static int stream_pcm_ack(struct snd_stream *stream,
                    size_t bytes)
{
    SND_ENTRY;

    return 0;
}

static int stream_pcm_get_caps(struct snd_stream *stream,
                    struct snd_stream_caps *caps)
{
    SND_ENTRY;

    return 0;
}

static int stream_pcm_get_codec_caps(struct snd_stream *stream,
                    struct snd_stream_codec_caps *codec)
{
    SND_ENTRY;

    return 0;
}

static int stream_pcm_set_metadata(struct snd_stream *stream,
                    struct snd_stream_metadata *metadata)
{
    SND_ENTRY;

    return 0;
}

#ifdef WIN32
static struct snd_stream_ops stream_pcm_ops = {
    stream_pcm_open,//int (*open)(struct snd_stream *stream);
    stream_pcm_free,//int (*free)(struct snd_stream *stream);
    stream_pcm_set_params,//int (*set_params)(struct snd_stream *stream,
            //struct snd_stream_params *params);
    NULL,//int (*get_params)(struct snd_stream *stream,
            //struct snd_codec_param *params);
    stream_pcm_set_metadata,//int (*set_metadata)(struct snd_stream *stream,
            //struct snd_stream_metadata *metadata);
    NULL,//int (*get_metadata)(struct snd_stream *stream,
           // struct snd_stream_metadata *metadata);
    stream_pcm_trigger,//int (*trigger)(struct snd_stream *stream, int cmd);
    stream_pcm_pointer,//int (*pointer)(struct snd_stream *stream,
            //struct snd_stream_tstamp *tstamp);
    stream_pcm_copy,//int (*copy)(struct snd_stream *stream, char *buf,
               //size_t count);
    stream_pcm_ack,//int (*ack)(struct snd_stream *stream, size_t bytes);
    stream_pcm_get_caps,//int (*get_caps) (struct snd_stream *stream,
            //struct snd_stream_caps *caps);
    stream_pcm_get_codec_caps,//int (*get_codec_caps) (struct snd_stream *stream,
            //struct snd_stream_codec_caps *codec);
};
#else
static struct snd_stream_ops stream_pcm_ops = {
    .open = stream_pcm_open,
    .free = stream_pcm_free,
    .set_params = stream_pcm_set_params,
    .set_metadata = stream_pcm_set_metadata,
    .trigger = stream_pcm_trigger,
    .pointer = stream_pcm_pointer,
    .copy = stream_pcm_copy,
    .ack = stream_pcm_ack,
    .get_caps = stream_pcm_get_caps,
    .get_codec_caps = stream_pcm_get_codec_caps,
};
#endif

#ifdef WIN32
const struct snd_stream_vdev pcm_vdev[] = {
    {
        "pcm_p0",//const char *name;
        "dpm_p0",//const char *dpm_stream;
        &stream_pcm_ops,//struct snd_stream_ops *ops;
        NULL,//void *private_data;
        SND_STREAM_PLAYBACK,//unsigned int direction;
        NULL,//TX_MUTEX lock;
        TRUE,//int active;
    },
};
#else
const struct snd_stream_vdev pcm_vdev[] = {
    {
        .name = "pcm_p0",
        .dpm_stream = "dpm_p0",
        .ops = &stream_pcm_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = NULL,
    },
    {
        .name = "pcm_p1",
        .dpm_stream = "dpm_p1",
        .ops = &stream_pcm_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = NULL,
    },
    {
        .name = "pcm_c0",
        .dpm_stream = "dpm_c0",
        .ops = &stream_pcm_ops,
        .direction = SND_STREAM_CAPTURE,
        .private_data = NULL,
    }
};
#endif
int stream_pcm_init(void)
{
    int ret, i;

    for(i = 0; i < sizeof(pcm_vdev) / sizeof(struct snd_stream_vdev); i++) {
        ret = snd_stream_register(&pcm_vdev[i]);
        SNX_INFO(0x0af801, "sd", "%s, ret=%d\n", pcm_vdev[i].name, ret);
    }

    return 0;
}
AIC_NORMAL_DRV_P2_INIT_EXPORT(stream_pcm_init);

