/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is audio steam HAL, by which, application can control audio
 *    stream.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x111388
#define LOG_MODULE    AMOD_SOUND

#include <aic_type.h>

#include "audio_stream.h"

#ifdef CONFIG_SOUND_SUPPORT
#include "snd_stream.h"
#include "snd_dpm.h"

#define PCM_WAIT_COUNT      10
#define IS_PCM_STREAM(stream) (stream->config.flag & (PCM_IN | PCM_OUT))

struct stream {
    u32 file;
    struct stream_config config;
};


/* came from driver and corresponding with STREAM CHN */
const char *stream_name_list[] = {
    "pcm_p0", /* STREAM_CHN_PCMP0   */
    "hwcodec_dl", /* STREAM_CHN_HWCODEC_DL */
    "hwcodec_ul", /* STREAM_CHN_HWCODEC_UL */
    "pcm_c0", /* STREAM_CHN_PCMC0   */
    "voip_dl", /* STREAM_CHN_VOIP_DL */
    "voip_ul", /* STREAM_CHN_VOIP_UL */
    "ipc_compr_playback0", /* STREAM_CHN_COMPRP0 */
    "ipc_compr_playback1", /* STREAM_CHN_COMPRP1 */
    "voice_dl", /* STREAM_CHN_COMPRP2 */
    "ipc_compr_capture", /* STREAM_CHN_COMPRC0 */
    "voice_rec", /* STREAM_CHN_COMPRC1 */
    "voice_ul", /* STREAM_CHN_COMPRC2 */
    "voice_dl_compr_pcm",
    "voice_ul_compr_pcm",
    "ext_p0",
    "voice_mix_p0",
    "extv_p0",
    "extv_c0",
};

static unsigned int pcm_format_to_bits(int format)
{
    switch (format) {
    case PCM_FORMAT_S32_LE:
    case PCM_FORMAT_S24_LE:
        return 32;
    case PCM_FORMAT_S24_3LE:
        return 24;
    default:
    case PCM_FORMAT_S16_LE:
        return SND_STREAM_WIDTH_DEFAULT;
    };
}

static struct snd_pcm_params * audio_stream_get_config(int file)
{
    if (file) {
        /* TODO */
    }

    return 0;
}

unsigned int pcm_bytes_to_frames(int file, unsigned int bytes)
{
    struct snd_pcm_params *param;

    if (!file)
        return 0;

    param = audio_stream_get_config(file);
    if (!param)
        return 0;

    return bytes / (param->channels * (pcm_format_to_bits(param->format) >> 3));
}

unsigned int pcm_frames_to_bytes(int file, unsigned int frames)
{
    struct snd_pcm_params *param;

    if (!file)
        return 0;

    param = audio_stream_get_config(file);
    if (!param)
        return 0;

    return frames * param->channels * (pcm_format_to_bits(param->format) >> 3);
}

static unsigned int get_time_by_byte(struct stream_config *config, unsigned int byte)
{
    return byte * 1000 / config->chn / config->sample_rate / 2;
}

int audio_stream_open(u32 stream_chn, struct stream_config *config, int flag)
{
    struct snd_stream_params param;
    struct stream *stream;
    char *stream_name;
    u32 file = 0, bit_width;
    int ret;

    if (stream_chn >= sizeof(stream_name_list) / sizeof(char *)) {
        aicx_error(0x14e0d3, "s", "%s, error stream chn\n", __func__);
        return 0;
    }

    if (!config) {
        aicx_error(0x0c5d3e, "s", "%s, config is NULL\n", __func__);
        return 0;
    }
    stream_name = (char *)stream_name_list[stream_chn];
    if (!stream_name) {
        aicx_error(0x068cfd, "s", "%s, no this stream chn\n", __func__);
        return 0;
    }

    stream = snd_malloc(sizeof(struct stream));
    if(!stream) {
        aicx_error(0x00daea, "s", "%s, no memroy!", __func__);
        return 0;
    }

    ret = snd_stream_open(stream_name, flag, &file);
    if (ret || !file) {
        snd_free(stream);
        aicx_error(0x078e0b, "d", "audio stream open error, ret=%d\n", ret);
        return 0;
    }

    stream->file = file;

    param.flag = flag;
    param.srate = config->sample_rate;
    param.brate = config->bit_rate;
    param.format = config->fmt;
    param.channels = config->chn;
    param.period_size = config->period_size;
    param.period_cnt = config->period_cnt;
    param.start_threshold = config->start_threshold;
    param.stop_threshold = config->stop_threshold;
    param.pext = NULL; /* TODO */
    ret = snd_stream_set_params(file, (u32)&param);
    if (ret) {
        aicx_error(0x02be41, "dd", "audio_stream_open failed, ret=%d,flat=%d", ret, flag);
        snd_stream_close(file);
        snd_free(stream);
        return 0;
    }

    if (IS_PCM_STREAM(stream))
        bit_width = pcm_format_to_bits(config->fmt);
    else
        bit_width = 16;
    if (stream_chn == STREAM_CHN_VOIP_DL || stream_chn == STREAM_CHN_VOIP_UL)
        snd_card_set_param(stream_name, bit_width, 1, 16000);
    else
#ifdef CONFIG_MUSIC_FIX_48K
    if (stream_chn == STREAM_CHN_OFFLD_PCM || stream_chn == STREAM_CHN_OFFLD_COMPR)
        snd_card_set_param(stream_name, bit_width, 2, 48000);
    else
#endif
#ifdef CONFIG_RECORD_FIX_16K
    if (stream_chn == STREAM_CHN_OFFLD_REC)
        snd_card_set_param(stream_name, bit_width, 1, 16000);
    else
#endif
#ifdef CONFIG_HW_CODEC_SUPPORT
    if (stream_chn == STREAM_CHN_HWCODEC_DL || stream_chn == STREAM_CHN_HWCODEC_UL) {
        /* do not set snd card param */
    }
    else
#endif
    snd_card_set_param(stream_name, bit_width, config->chn,
                       config->sample_rate);

    /* at last, save config */
    stream->config = *config;
    stream->config.flag = flag;

    return (int)stream;
}


int audio_stream_start(int stream)
{
    char *stream_name;

    if (!stream)
        return AIC_EBADF;

    stream_name = (char *)snd_stream_get_dpm_stream_name(
                            ((struct stream *)stream)->file);
    if (stream_name)
        snd_card_stream_event(stream_name, SND_DPM_STREAM_START);

    return snd_stream_start(((struct stream *)stream)->file);
}

int audio_stream_stop(int stream)
{
    char *stream_name;

    if (!stream)
        return AIC_EBADF;

    stream_name = (char *)snd_stream_get_dpm_stream_name(
                            ((struct stream *)stream)->file);
    if (stream_name)
        snd_card_stream_event(stream_name, SND_DPM_STREAM_STOP);

    snd_stream_stop(((struct stream *)stream)->file);

    return 0;
}

int audio_stream_pause(int stream)
{
    char *stream_name;

    if (!stream)
        return AIC_EBADF;

    stream_name = (char *)snd_stream_get_dpm_stream_name(
                            ((struct stream *)stream)->file);
    if (stream_name)
        snd_card_stream_event(stream_name, SND_DPM_STREAM_STOP);

    return snd_stream_pause(((struct stream *)stream)->file);
}

int audio_stream_resume(int stream)
{
    char *stream_name;

    if (!stream)
        return AIC_EBADF;

    stream_name = (char *)snd_stream_get_dpm_stream_name(
                            ((struct stream *)stream)->file);
    if (stream_name)
        snd_card_stream_event(stream_name, SND_DPM_STREAM_START);

    return snd_stream_resume(((struct stream *)stream)->file);
}

int audio_stream_close(int stream)
{
    snd_stream_close(((struct stream *)stream)->file);
    snd_free((void *)stream);
    return 0;
}

int audio_stream_write(int stream, char *buff, u32 size)
{
    struct stream *pstream;
    struct stream_config *config;
    u32 avail;

    if (!stream)
        return AIC_EBADF;

    pstream = (struct stream *)stream;
    config = &pstream->config;
    avail = snd_stream_get_avail(pstream->file);

    if (IS_PCM_STREAM(pstream)) {
        if (avail < (config->period_size)) {
            u32 time = get_time_by_byte(config, size);
            u32 cnt = PCM_WAIT_COUNT;
            do {
                aos_sleep(time + 1);
                avail = snd_stream_get_avail(pstream->file);
            } while (cnt-- && avail < config->period_size);

            if (avail < config->period_size) {
                aicx_error(0x05da9e, "", "audio write, lost one frame\n");
                return AIC_EIO;
            }
        }
    } else {
        if (avail < config->period_size) {
            snd_stream_poll(pstream->file, (u32)-1);
            /* check again, sometimes event was already set */
            avail = snd_stream_get_avail(pstream->file);
            if (avail < config->period_size)
                snd_stream_poll(pstream->file, (u32)-1);
        }
    }

    return snd_stream_write(pstream->file, buff, size, 0);
}

int audio_stream_read(int stream, char *buff, u32 size)
{
    struct stream *pstream;
    struct stream_config *config;
    u32 avail;

    if (!stream)
        return AIC_EBADF;

    pstream = (struct stream *)stream;
    config = &pstream->config;
    avail = snd_stream_get_avail(pstream->file);

    if (IS_PCM_STREAM(pstream)) {
        if (avail < pstream->config.period_size) {
            u32 time = get_time_by_byte(config, size);
            aos_sleep(time + 1);
            avail = snd_stream_get_avail(pstream->file);
            if (avail < config->period_size) {
                aicx_error(0x003fdd, "", "audio read, one frame timeout\n");
                return AIC_EIO;
            }
        }
    } else {
        if (avail < pstream->config.period_size) {
            snd_stream_poll(pstream->file, 100);
            avail = snd_stream_get_avail(pstream->file);
            if (avail < pstream->config.period_size)
                snd_stream_poll(pstream->file, 21);
        }
    }

    return snd_stream_read(pstream->file, buff, size, 0);
}

u32 audio_stream_get_timestamp(int stream)
{
    struct stream *pstream;
    struct snd_stream_tstamp tstamp = {0};
    int ret;

    if (!stream)
        return 0;

    pstream = (struct stream *)stream;
    ret = snd_stream_tstamp(pstream->file, (ulong)&tstamp);
    if (!ret) {
        if (IS_PCM_STREAM(pstream))
            return tstamp.pcm_io_frames / ((pstream->config.sample_rate / 1000)
                                           * pstream->config.chn << 1);
        else
            return tstamp.pcm_io_frames;
    }

    return 0;
}

int audio_stream_drain(int stream)
{
    return snd_stream_drain(((struct stream *)stream)->file);
}

#else
int audio_stream_open(u32 stream_chn, struct stream_config *config, int flag)
{
    return 0;
}
int audio_stream_close(int stream)
{
    return 0;
}
int audio_stream_start(int stream)
{
    return 0;
}
int audio_stream_stop(int stream)
{
    return 0;
}
int audio_stream_pause(int stream)
{
    return 0;
}
int audio_stream_resume(int stream)
{
    return 0;
}
int audio_stream_write(int stream, char *buff, u32 size)
{
    return 0;
}
int audio_stream_read(int stream, char *buff, u32 size)
{
    return 0;
}
u32 audio_stream_get_timestamp(int stream)
{
    return 0;
}
int audio_stream_drain(int stream)
{
    return 0;
}
#endif
