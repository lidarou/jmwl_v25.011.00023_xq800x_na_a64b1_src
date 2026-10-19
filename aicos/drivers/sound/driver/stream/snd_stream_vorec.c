/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines vorecess stream interface and we should register is to
 *    sound card that can be call during stream playing.
 */


/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13d18b
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
#include "snd_stream_vorec.h"
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

struct stream_vorec_rtd {
    struct snd_stream *cstream;
    int dir;
    u32 session_id;
    enum stream_state stream_state;

    u32 app_pointer;
    u32 buffer_size;

    u32 copied_total;
    u32 dsp_copied_total;

    struct snd_stream_params params;
};

struct stream_vorec_cfg {
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

static int stream_vorec_trigger(struct snd_stream *cstream, int cmd);
static int stream_vorec_buf_done(void *data, u8 *buf, u32 size);

#ifdef CONFIG_VOICE_EFFECT
extern int voice_proc_record_start(u32 fmt, u32 srate, u32 brate, u32 chn,
                            void *cb, void *priv);
extern int voice_proc_record_stop(void);
extern int voice_proc_record_pause(void);
extern int voice_proc_record_resume(void);

static int stream_vorec_open(struct snd_stream *cstream)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = NULL;

    SND_ENTRY;

    irtd = snd_zalloc(sizeof(struct stream_vorec_rtd));
    if (!irtd) {
        SNX_ERROR(0x1d5d11, "", "vorec open failed, no memory\n");
        return AIC_ENOMEM;
    }
    memset(irtd, 0, sizeof(struct stream_vorec_rtd));
    irtd->cstream = cstream;
    runtime->private_data = irtd;

    SND_EXIT;

    return 0;
}

static int stream_vorec_free(struct snd_stream *cstream)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = runtime->private_data;

    if (!irtd)
        return 0;

    if (irtd->stream_state == COMPR_TRIGGERED)
        stream_vorec_trigger(cstream, SNDRV_STREAM_TRIGGER_STOP);

    snd_free(irtd);

    return 0;
}

static int stream_vorec_set_params(struct snd_stream *cstream,
                                   struct snd_stream_params *stream_params)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = runtime->private_data;

    SND_ENTRY;

    irtd->params = *stream_params;
    irtd->stream_state = COMPR_PARAMSED;

    return 0;
}

static int stream_vorec_buf_done(void *data, u8 *buf, u32 size)
{
    struct stream_vorec_rtd *irtd = (struct stream_vorec_rtd *)data;
    struct snd_stream_runtime *runtime = irtd->cstream->runtime;

    u32 buf_pos = irtd->dsp_copied_total % runtime->buffer_size;
    u32 copy = runtime->buffer_size - buf_pos;

    if (copy > size)
        copy = size;
    memcpy((char *)runtime->buffer + buf_pos, buf, copy);
    if (copy < size)
        memcpy(runtime->buffer, buf + copy, size - copy);

    irtd->dsp_copied_total += size;

    SNX_DBG(0x09d05f, "ddd", "buf done, copied_total=%d, dsp_total=%d, size=%d\n",
            irtd->copied_total, irtd->dsp_copied_total, size);

    snd_stream_fragment_elapsed(irtd->cstream);

    return 0;
}

static int stream_vorec_trigger(struct snd_stream *cstream, int cmd)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = runtime->private_data;
    int result = 0;

    SND_ENTRY;

    switch (cmd) {
    case SNDRV_STREAM_TRIGGER_START:
        SNX_INFO(0x1a5877, "", "vorec trigger start\n");
        struct snd_stream_params *param = &irtd->params;
        result = voice_proc_record_start(param->format, param->srate,
                                         param->brate, param->channels,
                                         stream_vorec_buf_done, irtd);
        if (result < 0)
            goto out_ops;
        irtd->stream_state = COMPR_TRIGGERED;

        break;
    case SNDRV_STREAM_TRIGGER_STOP:
        SNX_INFO(0x08cd9d, "", "vorec trigger stop\n");
        result = voice_proc_record_stop();
        if (result < 0)
            goto out_ops;
        irtd->stream_state = COMPR_STOPPED;
        irtd->dsp_copied_total = 0;
        irtd->copied_total = 0;
        irtd->app_pointer = 0;
        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_PUSH:
        SNX_INFO(0x087948, "", "vorec trigger pause\n");
        result = voice_proc_record_pause();
        if (result < 0)
            goto out_ops;

        irtd->stream_state = COMPR_STOPPED;
        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_RELEASE:
        SNX_INFO(0x118a72, "", "vorec trigger resume\n");
        result = voice_proc_record_resume();
        if (result < 0)
            goto out_ops;

        irtd->stream_state = COMPR_TRIGGERED;
        break;
    default:
        SNX_INFO(0x1e64fc, "d", "vorec unknown cmd=%d\n", cmd);
         break;
    }

    SND_EXIT;

    return 0;

out_ops:
    SNX_ERROR(0x199328, "d", "vorec trigger failed, result=%d\n", result);

    return 0;
}

static int stream_vorec_pointer(struct snd_stream *cstream,
                    struct snd_stream_tstamp *arg)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = runtime->private_data;

    arg->copied_total = irtd->dsp_copied_total;

    arg->pcm_frames = arg->copied_total;
    arg->pcm_io_frames = arg->pcm_frames;

    return 0;
}

static struct snd_stream_ops stream_vorec_ops = {
    .open = stream_vorec_open,
    .free = stream_vorec_free,
    .set_params = stream_vorec_set_params,
    .trigger = stream_vorec_trigger,
    .pointer = stream_vorec_pointer,
    .copy = NULL,
};

#else
static u32 adsp_get_play_time(struct snd_stream *cstream)
{
    /* unit is mili-second */
    return readl((void *)0x41090088);
}


static int adsp_send_vorec_param(int stream_id, struct snd_stream_params *param)
{
    struct snd_stream_cmd_param *pcmd =
        (struct snd_stream_cmd_param *)snd_ipc_get_cmd_buf();
    int ret;

    pcmd->cmd = SND_STREAM_CMD_VOREC_START;
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
    aicx_error(0x186052, "d", "send vorec param bitrate:%d\n" , pcmd->bit_rate);
    SNX_INFO(0x132ffe, "s", "adsp param sent %s!\n", ret ? "failed" : "ok");

    return ret;
}

static int adsp_send_vorec_cmd(struct snd_stream *cstream, int cmd, u32 reserve1)
{
    union snd_stream_cmd *pcmd =
        (union snd_stream_cmd *)snd_ipc_get_cmd_buf();
    struct stream_vorec_cfg *cfg = cstream->device->private_data;
    struct stream_vorec_rtd *irtd = (struct stream_vorec_rtd *)
                                        cstream->runtime->private_data;
    int ret = 0;

    switch(cmd) {
    case SND_STREAM_CMD_VOREC_PAUSE:
    case SND_STREAM_CMD_VOREC_RESUME:
    case SND_STREAM_CMD_VOREC_STOP:
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

static int stream_vorec_buf_done(void *data, u8 *buf, u32 size)
{
    struct stream_vorec_rtd *irtd = (struct stream_vorec_rtd *)data;
    struct snd_stream_runtime *runtime = irtd->cstream->runtime;
    struct snd_stream *cstream = irtd->cstream;
    struct stream_vorec_cfg *cfg = cstream->device->private_data;
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


static int stream_vorec_drain_ack(void *rsp_ptr, u32 data)
{
    struct stream_vorec_rtd *irtd = (struct stream_vorec_rtd *)data;
    struct snd_stream *cstream = irtd->cstream;

    snd_stream_drain_notify(cstream);

    return 0;
}

static int stream_vorec_open(struct snd_stream *cstream)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = NULL;
    int ret;

    SND_ENTRY;

    irtd = snd_zalloc(sizeof(struct stream_vorec_rtd));
    if (!irtd) {
        SNX_ERROR(0x1d5d11, "", "vorec open failed, no memory\n");
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

    irtd->session_id = snd_ipc_async_reg(stream_vorec_drain_ack, (u32)irtd);

    SND_EXIT;

    return ret;
}

static int stream_vorec_free(struct snd_stream *cstream)
{
    struct stream_vorec_cfg *cfg = cstream->device->private_data;
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = runtime->private_data;
    int ret = 0;

    SND_ENTRY;

    if (irtd->stream_state == COMPR_TRIGGERED)
        stream_vorec_trigger(cstream, SNDRV_STREAM_TRIGGER_STOP);

    ret = adsp_send_vorec_cmd(cstream, SND_STREAM_CMD_CLOSE, 0);
    if (ret < 0)
        SNX_ERROR(0x1651a6, "d", "send CMD_CLOSE ret %d\n", ret);

    if (cfg->ipc_chn)
        snd_ipc_chn_close(cfg->ipc_chn);
    snd_ipc_async_unreg(irtd->session_id);
    snd_free(irtd);

    adsp_power_off();

    aic_dvfs_force(FALL | AUDIO_TL420);

    SND_EXIT;

    return 0;
}

static int stream_vorec_set_params(struct snd_stream *cstream,
                                   struct snd_stream_params *stream_params)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = runtime->private_data;
    struct stream_vorec_cfg *cfg = cstream->device->private_data;

    int result = 0;

    SND_ENTRY;

    /* TODO: param size is temp value */
    result = adsp_send_vorec_param(cfg->stream_id, stream_params);

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
        SNX_ERROR(0x059700, "", "vorecess no memory!\n");
        return -ENOMEM;
    }
    memset(buff, 0, buff_size);
    runtime->buffer = buff;
    runtime->buffer_size = buff_size;
#endif

    if (cfg->ipc_chn != 0) {
        snd_ipc_cb cb;
        cb = stream_vorec_buf_done;
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

static int stream_vorec_trigger(struct snd_stream *cstream, int cmd)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = runtime->private_data;
    struct stream_vorec_cfg *cfg = cstream->device->private_data;
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
        result = adsp_send_vorec_cmd(cstream, SND_STREAM_CMD_VOREC_START, 0);
        if (result < 0)
            goto out_ops;

        break;
    case SNDRV_STREAM_TRIGGER_STOP:
        SNX_INFO(0x182548, "", "SNDRV_STREAM_TRIGGER_STOP\n");
        result = adsp_send_vorec_cmd(cstream, SND_STREAM_CMD_VOREC_STOP, 0);
        if (result < 0)
            goto out_ops;
        irtd->stream_state = COMPR_STOPPED;
        irtd->dsp_copied_total = 0;
        irtd->copied_total = 0;
        irtd->app_pointer = 0;

        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_PUSH:
        SNX_INFO(0x12858c, "", "SNDRV_STREAM_TRIGGER_PAUSE_PUSH\n");
        result = adsp_send_vorec_cmd(cstream, SND_STREAM_CMD_VOREC_PAUSE, 0);
        if (result < 0)
            goto out_ops;

        irtd->stream_state = COMPR_STOPPED;
        break;
    case SNDRV_STREAM_TRIGGER_PAUSE_RELEASE:
        SNX_INFO(0x166d16, "", "SNDRV_STREAM_TRIGGER_PAUSE_RELEASE\n");

        result = adsp_send_vorec_cmd(cstream, SND_STREAM_CMD_VOREC_RESUME, 0);
        if (result < 0)
            goto out_ops;

        irtd->stream_state = COMPR_TRIGGERED;
        break;
    case SNDRV_STREAM_TRIGGER_PARTIAL_DRAIN:
    case SNDRV_STREAM_TRIGGER_DRAIN:
        result = adsp_send_vorec_cmd(cstream, SND_STREAM_CMD_DRAIN,
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

static int stream_vorec_pointer(struct snd_stream *cstream,
                    struct snd_stream_tstamp *arg)
{
    struct snd_stream_runtime *runtime = cstream->runtime;
    struct stream_vorec_rtd *irtd = runtime->private_data;

    arg->copied_total = irtd->dsp_copied_total;

    /* msecond */
    arg->pcm_frames = adsp_get_play_time(cstream);
    arg->pcm_io_frames = arg->pcm_frames;

    return 0;
}

static int stream_vorec_ack(struct snd_stream *cstream,
                    size_t bytes)
{
    SND_ENTRY;

    return 0;
}

static int stream_vorec_get_caps(struct snd_stream *cstream,
                    struct snd_stream_caps *caps)
{
    SND_ENTRY;

    return 0;
}

static int stream_vorec_get_codec_caps(struct snd_stream *cstream,
                    struct snd_stream_codec_caps *codec)
{
    SND_ENTRY;

    return 0;
}

static int stream_vorec_set_metadata(struct snd_stream *cstream,
                    struct snd_stream_metadata *metadata)
{
    SND_ENTRY;

    return 0;
}

static struct snd_stream_ops stream_vorec_ops = {
    .open = stream_vorec_open,
    .free = stream_vorec_free,
    .set_params = stream_vorec_set_params,
    .set_metadata = stream_vorec_set_metadata,
    .trigger = stream_vorec_trigger,
    .pointer = stream_vorec_pointer,
    .copy = NULL,
    .ack = stream_vorec_ack,
    .get_caps = stream_vorec_get_caps,
    .get_codec_caps = stream_vorec_get_codec_caps,
};


struct stream_vorec_cfg stream_vorec_cfg[] = {
    {
        .ipc_chn = SND_IPC_CH_REC,
        .stream_id = SND_STREAM_ID_REC,
    },
};
#endif

const struct snd_stream_vdev stream_vorec[] = {
    {
        .name = "voice_rec",
        .ops = &stream_vorec_ops,
        .direction = SND_STREAM_CAPTURE,
#ifndef CONFIG_MCU_AUDIO
        .private_data = &stream_vorec_cfg[0],
#endif
    },
};

int stream_vorec_init(void)
{
    int ret, i;

    for(i = 0; i < sizeof(stream_vorec) / sizeof(struct snd_stream_vdev); i++) {
        ret = snd_stream_register(&stream_vorec[i]);
        SNX_INFO(0x0af801, "sd", "%s, ret=%d\n", stream_vorec[i].name, ret);
    }

    return 0;
}
AIC_NORMAL_DRV_P2_INIT_EXPORT(stream_vorec_init);

