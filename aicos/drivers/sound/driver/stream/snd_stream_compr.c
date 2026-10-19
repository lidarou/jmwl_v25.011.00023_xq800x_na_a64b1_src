/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines compress stream interface and we should register is to
 *    sound card that can be call during stream playing.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b9850
#define LOG_MODULE    AMOD_SOUND

#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_bitmap.h>
#include <aic_io.h>
#include <dvfs_inc.h>
#include <aic_list.h>

#include "snd_stream.h"
#include "snd_stream_driver.h"
#include "snd_stream_compr.h"
#include "snd_ipc.h"
#include "adsp_mng.h"

enum stream_state {
    COMPR_IDLE = 0,
    COMPR_OPENNED,
    COMPR_CLOSED,
    COMPR_STOPPED,
    COMPR_PARAMSED,
    COMPR_TRIGGERED,
    COMPR_PARAMIZED,
    COMPR_ABORT,
};

struct stream_compr_rtd {
    struct snd_stream *cstream;
    int dir;
    u32 session_id;
    enum stream_state stream_state;

    u32 app_pointer;
    u32 buffer_size;

    u32 copied_total;
    u32 dsp_copied_total;

    int sample_rate;

    aos_thread_id thread;
};

struct stream_compr_cfg {
    u8 ipc_chn;
    u8 dma_chn;
    u8 stream_id;
};

struct adsp_cb_blk {
    struct list_head list;
    int (*cb)(u32 data, u32 remote_data);
    u32 data;
    u8 link;
};

static int stream_compr_trigger(struct snd_stream *cstream, int cmd);
static int stream_compr_buf_done(void *data, u8 *buf, u32 size);

int adsp_send_param(int stream_id, struct snd_stream_params *param)
{
    struct snd_stream_cmd_param *pcmd =
        (struct snd_stream_cmd_param *)snd_ipc_get_cmd_buf();
    int ret;

    pcmd->cmd = SND_STREAM_CMD_SET_PARAM;
    pcmd->stream_id = stream_id;
    pcmd->sample_rate = param->srate;
    pcmd->bit_rate = param->brate;
    pcmd->chn_cnt = param->channels;
    pcmd->bit_wid = snd_stream_get_pcm_bits(param->format);
    if (IS_PCM_PARAM(param))
        pcmd->format = SND_FMT_PCM;
    else
        pcmd->format = param->format;
    if (param->flag & FL_MM_VOIP)
        pcmd->mm_voip = 1;
    else
        pcmd->mm_voip = 0;

    ret = snd_ipc_send_cmd(pcmd);

    SNX_INFO(0x132ffe, "s", "adsp param sent %s!\n", ret ? "failed" : "ok");

    return ret;
}

int adsp_send_cmd(struct snd_stream *cstream, int cmd, u32 reserve1)
{
    union snd_stream_cmd *pcmd =
        (union snd_stream_cmd *)snd_ipc_get_cmd_buf();
    struct stream_compr_cfg *cfg = cstream->device->private_data;
    struct stream_compr_rtd *irtd = (struct stream_compr_rtd *)
                                        cstream->runtime->private_data;
    int ret = 0;

    switch(cmd) {
    case SND_STREAM_CMD_OPEN:
        pcmd->open.cmd = cmd;
        pcmd->open.stream_id = cfg->stream_id;
        pcmd->open.ipc_chn = cfg->ipc_chn;
        if (cfg->stream_id == SND_STREAM_ID_VOICE_DL)
            pcmd->open.ipc_buf_addr = (u32)snd_ipc_get_buf_header(SND_IPC_CH_DL_AP);
        else if (cfg->stream_id == SND_STREAM_ID_VOICE_UL)
            pcmd->open.ipc_buf_addr = (u32)snd_ipc_get_buf_header(SND_IPC_CH_UL_AP);
        else
            pcmd->open.ipc_buf_addr = (u32)snd_ipc_get_buf_header(cfg->ipc_chn);
        break;
    case SND_STREAM_CMD_START:
    case SND_STREAM_CMD_PAUSE:
    case SND_STREAM_CMD_RESUME:
    case SND_STREAM_CMD_STOP:
    case SND_STREAM_CMD_CLOSE:
        pcmd->common.cmd = cmd;
        pcmd->common.stream_id = cfg->stream_id;
        break;
    case SND_STREAM_CMD_DRAIN:
        pcmd->drain.cmd = cmd;
        pcmd->drain.session_id = irtd->session_id;
        pcmd->drain.stream_id = cfg->stream_id;
        pcmd->drain.total_size = irtd->copied_total;
        break;
    default:
        snd_ipc_put_cmd_buf();
        return AIC_EPERM;
    }

    SNX_INFO(0x09caca, "d", "adsp cmd=%d sending\n", cmd);
    if (cmd == SND_STREAM_CMD_DRAIN)
        ret = snd_ipc_send_cmd_async(pcmd, irtd->session_id);
    else
        ret = snd_ipc_send_cmd(pcmd);
    SNX_INFO(0x179c5f, "ds", "adsp cmd=%d sent %s!\n", cmd, ret ? "failed" : "ok");

    return ret;
}

u32 adsp_get_play_time(struct snd_stream *cstream)
{
    /* unit is mili-second */
    return readl((void *)0x41090088);
}

static int stream_compr_buf_done(void *data, u8 *buf, u32 size)
{
    struct stream_compr_rtd *irtd = (struct stream_compr_rtd *)data;
    struct snd_stream_runtime *runtime = irtd->cstream->runtime;
    struct snd_stream *cstream = irtd->cstream;
    struct stream_compr_cfg *cfg = cstream->device->private_data;
    enum snd_stream_direction dir = cstream->direction;

    if(dir == SND_STREAM_PLAYBACK) {
        irtd->dsp_copied_total += runtime->fragment_size;
    } else if (dir == SND_STREAM_CAPTURE) {
        u32 buf_pos = irtd->dsp_copied_total % runtime->buffer_size;
        u32 copy = runtime->buffer_size - buf_pos;

        if (copy > size)
            copy = size;
        memcpy((char *)runtime->buffer + buf_pos, buf, copy);
        if (copy < size)
            memcpy(runtime->buffer, buf + copy, size - copy);

        irtd->dsp_copied_total += size;
        if (cfg->stream_id != SND_STREAM_ID_HWCODEC_UL)
            snd_ipc_prepare_recv_buff(cfg->ipc_chn, (u32)-1);
    }
    SNX_DBG(0x1b98c7, "ddd", "buf done, dir=%d, ap_total=%d, dsp_total=%d\n",
            dir, irtd->copied_total, irtd->dsp_copied_total);

    snd_stream_fragment_elapsed(irtd->cstream);

    return 0;
}

static int stream_compr_buf_done_voip_ul(void *data, u8 *buf, u32 size)
{
    struct stream_compr_rtd *irtd = (struct stream_compr_rtd *)data;
    struct snd_stream_runtime *runtime = irtd->cstream->runtime;
    struct snd_stream *cstream = irtd->cstream;
    struct stream_compr_cfg *cfg = cstream->device->private_data;

    u32 buf_pos = irtd->dsp_copied_total % runtime->buffer_size;
    u32 copy = runtime->buffer_size - buf_pos - 1;
    char *p;


    p = (char *)runtime->buffer + buf_pos;
    /* add size at first byte */
    *p = size;

    /* then copy data behind size */
    if (copy > size)
        copy = size;
    memcpy(++p, buf, copy);

    /* if need wrap, then copy to buffer head */
    if (copy < size) {
        //memcpy(runtime->buffer, buf + copy, size - copy);
        SNX_INFO(0x189ef6, "dd", "should not wrap, copy=%d, size=%d\n", copy, size);
    }
    /**
     *  copied_total plus fragment_size, not size, we keep buffer
     *  manage as block type.
     */
    irtd->dsp_copied_total += runtime->fragment_size;
    snd_ipc_prepare_recv_buff(cfg->ipc_chn, (u32)-1);

    SNX_DBG(0x085a1c, "dd", "ul buf done, ap_total=%d, dsp_total=%d\n",
            irtd->copied_total, irtd->dsp_copied_total);

    snd_stream_fragment_elapsed(irtd->cstream);

    return 0;
}

static int stream_compr_buf_done_voip_ul_pcm(void *data, u8 *buf, u32 size)
{
    struct stream_compr_rtd *irtd = (struct stream_compr_rtd *)data;
    struct snd_stream_runtime *runtime = irtd->cstream->runtime;
    struct snd_stream *cstream = irtd->cstream;
    struct stream_compr_cfg *cfg = cstream->device->private_data;

    u32 buf_pos = irtd->dsp_copied_total % runtime->buffer_size;
    u32 copy = runtime->buffer_size - buf_pos;

    if (copy > size)
        copy = size;
    memcpy((char *)runtime->buffer + buf_pos, buf, copy);
    if (copy < size)
        memcpy(runtime->buffer, buf + copy, size - copy);

    irtd->dsp_copied_total += size;
    snd_ipc_prepare_recv_buff(cfg->ipc_chn, (u32)-1);

    SNX_DBG(0x159a22, "dd", "voip ul pcm buf done, ap_total=%d, dsp_total=%d\n",
            irtd->copied_total, irtd->dsp_copied_total);

    snd_stream_fragment_elapsed(irtd->cstream);

    return 0;
}

static int stream_compr_drain_ack(void *rsp_ptr, u32 data)
{
    struct stream_compr_rtd *irtd = (struct stream_compr_rtd *)data;
    struct snd_stream *cstream = irtd->cstream;

    snd_stream_drain_notify(cstream);

    return 0;
}

static int stream_compr_open(struct snd_stream *cstream)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_compr_rtd *irtd = NULL;
    int ret;

    SND_ENTRY;

    irtd = snd_zalloc(sizeof(struct stream_compr_rtd));
    if (!irtd) {
        SNX_ERROR(0x1897dc, "", "compr open failed, no memory\n");
        return AIC_ENOMEM;
    }
    irtd->cstream = cstream;
    runtime->private_data = irtd;

    aic_dvfs_force(RISE | AUDIO_TL420);
    ret = adsp_power_on(1);
    if (ret) {
        SNX_ERROR(0x07f6fc, "d", "adsp pwr on failed, ret=%d\n", ret);
        return AIC_EACCES;
    }

    irtd->session_id = snd_ipc_async_reg(stream_compr_drain_ack, (u32)irtd);
    ret = adsp_send_cmd(cstream, SND_STREAM_CMD_OPEN, 0);

    SND_EXIT;

    return ret;
}

static int stream_compr_free(struct snd_stream *cstream)
{
    struct stream_compr_cfg *cfg = cstream->device->private_data;
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_compr_rtd *irtd = runtime->private_data;
    int ret = 0;

    SND_ENTRY;

    if (irtd->stream_state == COMPR_TRIGGERED)
        stream_compr_trigger(cstream, SNDRV_STREAM_TRIGGER_STOP);

    ret = adsp_send_cmd(cstream, SND_STREAM_CMD_CLOSE, 0);
    if (ret < 0)
        SNX_ERROR(0x1651a6, "d", "send CMD_CLOSE ret %d\n", ret);

    /* TODO: wait thread complete */
    aos_delete_thread(irtd->thread);
    if (cfg->ipc_chn)
        snd_ipc_chn_close(cfg->ipc_chn);
    snd_ipc_async_unreg(irtd->session_id);
    snd_free(irtd);

    adsp_power_off();

    aic_dvfs_force(FALL | AUDIO_TL420);

    SND_EXIT;

    return 0;
}

static int stream_compr_set_params(struct snd_stream *cstream,
                                   struct snd_stream_params *stream_params)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_compr_rtd *irtd = runtime->private_data;
    struct stream_compr_cfg *cfg = cstream->device->private_data;

    int result = 0;

    SND_ENTRY;

    /* TODO: param size is temp value */
    result = adsp_send_param(cfg->stream_id, stream_params);

    if (result < 0) {
        SNX_INFO(0x09a1cf, "s", "%s failed\n", __func__);
        return result;
    }

/**
  * Do not need alloc memory if transfer through IPC, these codes are keeped
  * here as sample for other stream if need.
  */
#ifdef __SND_NO_IPC__
    u32 buff_size;
    u8 *buff;
    buff_size = stream_params->period_size * stream_params->period_cnt;
    buff = snd_malloc(buff_size);
    if (!buff) {
        SNX_ERROR(0x1de355, "", "compress no memory!\n");
        return -ENOMEM;
    }
    memset(buff, 0, buff_size);
    runtime->buffer = buff;
    runtime->buffer_size = buff_size;
#endif

    if (cfg->ipc_chn != 0) {
        snd_ipc_cb cb;
        if (cfg->stream_id == SND_STREAM_ID_VOIP_UL) {
            if (stream_params->format == SND_FMT_PCM)
                cb = stream_compr_buf_done_voip_ul_pcm;
            else
                cb = stream_compr_buf_done_voip_ul;
        } else
            cb = stream_compr_buf_done;
        if (cfg->ipc_chn)
            result = snd_ipc_chn_open(cfg->ipc_chn, stream_params->period_size,
                                      stream_params->period_cnt,
                                      cb, irtd);
        if (result) {
            SNX_ERROR(0x09f010, "dd", "snd ipc chn open failed, ipc chn=%d, stream_id=%d\n",
                      cfg->ipc_chn, cfg->stream_id);
            return result;
        }
    }
    irtd->stream_state = COMPR_PARAMSED;

    SND_EXIT;

    return 0;
}

static int stream_compr_trigger(struct snd_stream *cstream, int cmd)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_compr_rtd *irtd = runtime->private_data;
    struct stream_compr_cfg *cfg = cstream->device->private_data;
    int result = 0, i;

    SND_ENTRY;

    switch (cmd) {
    case SNDRV_STREAM_TRIGGER_START:
        SNX_INFO(0x038e91, "", "SNDRV_STREAM_TRIGGER_START\n");
        if (cstream->direction == SND_STREAM_CAPTURE &&
            IS_SND_IPC_DATA_CHN(cfg->ipc_chn)) {
            for (i = 0; i < runtime->fragments; i++)
                snd_ipc_prepare_recv_buff(cfg->ipc_chn, (u32)-1);
        }
        irtd->stream_state = COMPR_TRIGGERED;
        result = adsp_send_cmd(cstream, SND_STREAM_CMD_START, 0);
        if (result < 0)
            goto out_ops;

        break;
    case SNDRV_STREAM_TRIGGER_STOP:
        SNX_INFO(0x182548, "", "SNDRV_STREAM_TRIGGER_STOP\n");
        result = adsp_send_cmd(cstream, SND_STREAM_CMD_STOP, 0);
        if (result < 0)
            goto out_ops;
        irtd->stream_state = COMPR_STOPPED;
        irtd->dsp_copied_total = 0;
        irtd->copied_total = 0;
        irtd->app_pointer = 0;

        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_PUSH:
        SNX_INFO(0x12858c, "", "SNDRV_STREAM_TRIGGER_PAUSE_PUSH\n");
        result = adsp_send_cmd(cstream, SND_STREAM_CMD_PAUSE, 0);
        if (result < 0)
            goto out_ops;

        irtd->stream_state = COMPR_STOPPED;
        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_RELEASE:
        SNX_INFO(0x166d16, "", "SNDRV_STREAM_TRIGGER_PAUSE_RELEASE\n");

        result = adsp_send_cmd(cstream, SND_STREAM_CMD_RESUME, 0);
        if (result < 0)
            goto out_ops;

        irtd->stream_state = COMPR_TRIGGERED;
        break;
    case SNDRV_STREAM_TRIGGER_PARTIAL_DRAIN:
    case SNDRV_STREAM_TRIGGER_DRAIN:
        result = adsp_send_cmd(cstream, SND_STREAM_CMD_DRAIN,
                               irtd->copied_total);
        if (result < 0) {
            SNX_ERROR(0x0444c5, "", "drain out err!");
            goto out_ops;
        }
        SNX_INFO(0x1bb92b, "", "out of drain\n");
        break;
    case SNDRV_STREAM_TRIGGER_NEXT_TRACK:
        SNX_INFO(0x059d93, "", "SND_COMPR_TRIGGER_NEXT_TRACK\n");
        break;
    default:
        SNX_INFO(0x12409f, "d", "no responsed cmd=%d\n", cmd);
        break;
    }

    SND_EXIT;

    return 0;

out_ops:
    SNX_DBG(0x09a1cf, "s", "%s failed\n", __func__);

    return 0;
}

static int stream_compr_pointer(struct snd_stream *cstream,
                    struct snd_stream_tstamp *arg)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_compr_rtd *irtd = runtime->private_data;

    arg->copied_total = irtd->dsp_copied_total;

    /* msecond */
    arg->pcm_frames = adsp_get_play_time(cstream);
    arg->pcm_io_frames = arg->pcm_frames;

    return 0;
}

static int stream_compr_copy(struct snd_stream *cstream, char *buf, size_t count)
{
    struct stream_compr_cfg *cfg = cstream->device->private_data;
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_compr_rtd *irtd = runtime->private_data;
    enum snd_stream_direction dir = cstream->direction;
    void *ipc_buf;

#ifndef __SND_DEBUG__
    int ret;

    if (!count || !buf)
        return 0;
    if (dir == SND_STREAM_PLAYBACK) {
        ipc_buf = snd_ipc_buf_get(cfg->ipc_chn, 120);
        if (!ipc_buf) {
            /* may be under run */
            SNX_ERROR(0x0087c3, "", "cannot get snd ipc buf!\n");
            return 0;
        }
        memcpy(ipc_buf, buf, count);

        ret = snd_ipc_send_data(cfg->ipc_chn, ipc_buf, count);
        if (ret) {
            SNX_ERROR(0x16a0bd, "d", "copy error, ret=%d\n", ret);
            snd_ipc_buf_recycle(cfg->ipc_chn);
            return 0;
        }
        irtd->copied_total += count;
    } else {
        /* TODO */
    }
#else
    if (dir == SND_STREAM_PLAYBACK)
        memcpy((char *)runtime->buffer + irtd->app_pointer, buf, count);

    else if (dir == SND_STREAM_CAPTURE)
        memcpy(buf, (char *)runtime->buffer + irtd->app_pointer, count);

    irtd->copied_total += count;
    irtd->app_pointer += count;
    if (irtd->app_pointer >= runtime->buffer_size)
        irtd->app_pointer -= runtime->buffer_size;
#endif
    SNX_DBG(0x113c15, "sdddd", "%s, dir=%d, count=%d, app_pointer=%d, copied_total=%d\n",
            __func__, dir, count, irtd->app_pointer, irtd->copied_total);

    return count;
}

static int stream_compr_ack(struct snd_stream *cstream,
                    size_t bytes)
{
    SND_ENTRY;

    return 0;
}

static int stream_compr_get_caps(struct snd_stream *cstream,
                    struct snd_stream_caps *caps)
{
    SND_ENTRY;

    return 0;
}

static int stream_compr_get_codec_caps(struct snd_stream *cstream,
                    struct snd_stream_codec_caps *codec)
{
    SND_ENTRY;

    return 0;
}

static int stream_compr_set_metadata(struct snd_stream *cstream,
                    struct snd_stream_metadata *metadata)
{
    SND_ENTRY;

    return 0;
}

#ifdef WIN32
static struct snd_stream_ops stream_compr_ops = {
    stream_compr_open, /* int (*open)(struct snd_stream *stream); */
    stream_compr_free, /* int (*free)(struct snd_stream *stream); */
    stream_compr_set_params, /* int (*set_params)(struct snd_stream *stream,
                                        struct snd_stream_params *params); */
    NULL, /* int (*get_params)(struct snd_stream *stream,
                               struct snd_codec_param *params); */
    stream_compr_set_metadata, /* int (*set_metadata)(struct snd_stream *stream,
                                    struct snd_stream_metadata *metadata); */
    NULL, /* int (*get_metadata)(struct snd_stream *stream,
                                 struct snd_stream_metadata *metadata); */
    stream_compr_trigger, /* int (*trigger)(struct snd_stream *stream, int cmd); */
    stream_compr_pointer, /* int (*pointer)(struct snd_stream *stream,
                                         struct snd_stream_tstamp *tstamp); */
    stream_compr_copy, /* int (*copy)(struct snd_stream *stream, char *buf,
                                   size_t count); */
    stream_compr_ack, /* int (*ack)(struct snd_stream *stream, size_t bytes); */
    stream_compr_get_caps, /* int (*get_caps) (struct snd_stream *stream,
                                            struct snd_stream_caps *caps); */
    stream_compr_get_codec_caps, /* int (*get_codec_caps) (struct snd_stream *stream,
                                    struct snd_stream_codec_caps *codec); */
};

struct stream_compr_cfg stream_compr_cfg_playback = {
    2, 2, 0
};

struct stream_compr_cfg stream_compr_cfg_capture = {
    3, 2 ,1
};

const struct snd_stream_vdev stream_compr[1] = {
    "ipc_compr_playback0", /* const char *name; */
    "compress_ipc_p",      /* const char *dpm_stream; */
    &stream_compr_ops,        /* struct snd_stream_ops *ops; */
    &stream_compr_cfg_playback, /* void *private_data; */
    SND_STREAM_PLAYBACK,   /* unsigned int direction; */
};

#else
static struct snd_stream_ops stream_compr_ops = {
    .open = stream_compr_open,
    .free = stream_compr_free,
    .set_params = stream_compr_set_params,
    .set_metadata = stream_compr_set_metadata,
    .trigger = stream_compr_trigger,
    .pointer = stream_compr_pointer,
    .copy = stream_compr_copy,
    .ack = stream_compr_ack,
    .get_caps = stream_compr_get_caps,
    .get_codec_caps = stream_compr_get_codec_caps,
};

static struct snd_stream_ops stream_compr_ops_rec = {
    .open = stream_compr_open,
    .free = stream_compr_free,
    .set_params = stream_compr_set_params,
    .set_metadata = stream_compr_set_metadata,
    .trigger = stream_compr_trigger,
    .pointer = stream_compr_pointer,
    .ack = stream_compr_ack,
    .get_caps = stream_compr_get_caps,
    .get_codec_caps = stream_compr_get_codec_caps,
};

struct stream_compr_cfg stream_compr_cfg[] = {
    {
        .ipc_chn = SND_IPC_CH_PCM,
        .stream_id = SND_STREAM_ID_PCM,
    },
    {
        .ipc_chn = SND_IPC_CH_COMPR,
        .stream_id = SND_STREAM_ID_COMPR,
    },
    {
        .ipc_chn = SND_IPC_CH_REC,
        .stream_id = SND_STREAM_ID_REC,
    },
    {
        .ipc_chn = 0,
        .stream_id = SND_STREAM_ID_VOICE_DL,
    },
    {
        .ipc_chn = 0,
        .stream_id = SND_STREAM_ID_VOICE_UL,
    },
    {
        .ipc_chn = SND_IPC_CH_COMPR, /* same as compress music play */
        .stream_id = SND_STREAM_ID_VOIP_DL,
    },
    {
        .ipc_chn = SND_IPC_CH_REC,   /* same as record channle */
        .stream_id = SND_STREAM_ID_VOIP_UL,
    },
    {
        .ipc_chn = SND_IPC_CH_COMPR, /* same as compress music play */
        .stream_id = SND_STREAM_ID_HWCODEC_DL,
    },
    {
        .ipc_chn = SND_IPC_CH_REC,   /* same as record channle */
        .stream_id = SND_STREAM_ID_HWCODEC_UL,
    },
    /* 9 */
    {
        .ipc_chn = SND_IPC_CH_COMPR,
        .stream_id = SND_STREAM_ID_VOICE_DL,
    },
    /* 10 */
    {
        .ipc_chn = SND_IPC_CH_REC,
        .stream_id = SND_STREAM_ID_VOICE_UL,
    },
};

const struct snd_stream_vdev stream_compr[] = {
    {
        .name = "ipc_compr_playback0",
        .dpm_stream = "compress_ipc_p",
        .ops = &stream_compr_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = &stream_compr_cfg[0],
    },
    {
        .name = "ipc_compr_playback1",
        .dpm_stream = "compress_ipc_p",
        .ops = &stream_compr_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = &stream_compr_cfg[1],
    },
    {
        .name = "ipc_compr_capture",
        .dpm_stream = "compress_ipc_c",
        .ops = &stream_compr_ops_rec,
        .direction = SND_STREAM_CAPTURE,
        .private_data = &stream_compr_cfg[2],
    },
    {
        .name = "voice_dl_compr_pcm",
        .dpm_stream = "compress_ipc_p",
        .ops = &stream_compr_ops_rec,
        .direction = SND_STREAM_CAPTURE,
        .private_data = &stream_compr_cfg[9],
    },
    {
        .name = "voice_ul_compr_pcm",
        .dpm_stream = "compress_ipc_c",
        .ops = &stream_compr_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = &stream_compr_cfg[10],
    },
    {
        .name = "voice_dl",
        .dpm_stream = "compress_ipc_p",
        .ops = &stream_compr_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = &stream_compr_cfg[3],
    },
    {
        .name = "voice_ul",
        .dpm_stream = "compress_ipc_c",
        .ops = &stream_compr_ops,
        .direction = SND_STREAM_CAPTURE,
        .private_data = &stream_compr_cfg[4],
    },
    {
        .name = "voip_dl",
        .dpm_stream = "compress_ipc_p",
        .ops = &stream_compr_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = &stream_compr_cfg[5],
    },
    {
        .name = "voip_ul",
        .dpm_stream = "compress_ipc_c",
        .ops = &stream_compr_ops_rec,
        .direction = SND_STREAM_CAPTURE,
        .private_data = &stream_compr_cfg[6],
    },
    {
        .name = "hwcodec_dl",
        .dpm_stream = 0,
        .ops = &stream_compr_ops,
        .direction = SND_STREAM_PLAYBACK,
        .private_data = &stream_compr_cfg[7],
    },
    {
        .name = "hwcodec_ul",
        .dpm_stream = 0,
        .ops = &stream_compr_ops_rec,
        .direction = SND_STREAM_CAPTURE,
        .private_data = &stream_compr_cfg[8],
    }
};
#endif

int stream_compr_init(void)
{
    int ret, i;

    for(i = 0; i < sizeof(stream_compr) / sizeof(struct snd_stream_vdev); i++) {
        ret = snd_stream_register(&stream_compr[i]);
        SNX_INFO(0x0af801, "sd", "%s, ret=%d\n", stream_compr[i].name, ret);
    }

    return 0;
}
AIC_NORMAL_DRV_P2_INIT_EXPORT(stream_compr_init);

