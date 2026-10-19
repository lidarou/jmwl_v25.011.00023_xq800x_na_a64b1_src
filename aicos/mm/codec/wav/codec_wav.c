
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x096b5c
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <aic_list.h>

#include "audio_util.h"
#include "codec_mng.h"
#include "audio_client.h"

#define ID_RIFF     0x46464952
#define ID_WAVE     0x45564157
#define ID_FMT      0x20746d66
#define ID_DATA     0x61746164

#define FMT_PCM     1

struct wav_header {
    u32 riff_id;
    u32 riff_size;
    u32 riff_fmt;
    u32 fmt_id;
    u32 fmt_sz;
    u16 aud_fmt;
    u16 chn;
    u32 srate;
    u32 brate;
    u16 align;
    u16 wid;
    u32 data_id;
    u32 data_sz;
};

struct codec_wav_obj {
    struct list_head list;
    struct wav_header header;
    struct codec_inf inf;

    u32 size;
    u32 pos;
};

static u32 wav_dec_open(u32 priv, struct codec_ops *ops,
                        struct codec_info *info)
{
    struct codec_wav_obj *codec;
    struct wav_header *header;
    int size, ret_size;

    AENTRY

    if (!ops->get_func) {
        ALOGE("wav open, fget is null\n");
        return 0;
    }
    codec = audio_malloc(sizeof(*codec));

    if (!codec) {
        ALOGE("wav open, no mem\n");
        return 0;
    }
    memset(codec, 0, sizeof(*codec));
    codec->inf.codec_get = ops->get_func;
    codec->inf.priv = priv;

    size = sizeof(codec->header);
    header = &codec->header;
    ops->get_func(priv, (u8 *)header, size, &ret_size);
    if (ret_size != size) {
        ALOGE("read wav header failed\n");
        goto header_err;
    }

    if (header->riff_id != ID_RIFF) {// || header->data_id != ID_DATA) {
        ALOGE("wav header fmt failed, riff_id=0x%x\n", header->riff_id);
        goto header_err;
    }

    if (info) {
        info->chn = header->chn;
        info->srate = header->srate;
        info->brate =  header->brate;
        ALOGI("wav header info:chn:%d,srate=%d,brate=%d\n",
              header->chn, header->srate, header->brate);
    }

    return (u32)codec;

header_err:
    audio_free(codec);

    return 0;
}

int wav_dec_close(u32 obj)
{
    struct codec_wav_obj *codec = (struct codec_wav_obj *)obj;

    if (codec)
        audio_free(codec);

    AEXIT

    return 0;
}
#ifdef CONFIG_SOUND_CONV
static int wav_dec_get_data(u32 obj, short *pbuf, u32 count, u32 *real_count)
{
    struct codec_wav_obj *codec = (struct codec_wav_obj *)obj;
    struct codec_inf *inf = &codec->inf;
    u32 rd_count = 0, total_count = 0;
    int ret = 0;

    while (total_count < count) {
        ret = inf->codec_get(inf->priv, (u8 *)pbuf + total_count,
                             count - total_count, &rd_count);
        ALOGE("wav codec get=%d\n", rd_count);
        if (ret == ASTREAM_DATA_EOS || ret < 0) {
            memset((u8 *)pbuf + total_count, 0, count - total_count);
            break;
        }
        total_count += rd_count;
    }
    /* keep real_count as a full frame */
    *real_count = count;

    return ret;
}
#else
/* if < 0: error; if == 0: finished; if > 0: real sample count */
static int wav_dec_get_data(u32 obj, short *pbuf, u32 count, u32 *real_count)
{
    struct codec_wav_obj *codec = (struct codec_wav_obj *)obj;
    struct codec_inf *inf = &codec->inf;

    return inf->codec_get(inf->priv, (u8 *)pbuf, count, real_count);
}
#endif

/* do not define as const */
static struct codec_engine wav_dec = {
    .type = CODEC_TYPE_WAV,
    .dir = CODEC_DEC,
    .open = wav_dec_open,
    .close = wav_dec_close,
    .get_data = wav_dec_get_data,
};

static u32 wav_enc_open(u32 priv, struct codec_ops *ops,
                        struct codec_info *info)
{
    struct codec_wav_obj *codec;
    struct wav_header *header;

    if (!ops->send_func) {
        ALOGE("wav open, fsend is null\n");
        return 0;
    }
    codec = audio_malloc(sizeof(*codec));

    if (!codec) {
        ALOGE("wav open, no mem\n");
        audio_free(codec);

        return 0;
    }
    memset(codec, 0, sizeof(*codec));
    codec->inf.codec_send = ops->send_func;
    codec->inf.priv = priv;

    header = &codec->header;
    header->riff_id = ID_RIFF;
    header->riff_fmt = ID_WAVE;
    header->fmt_sz = 16; /* fix value */
    header->aud_fmt = FMT_PCM; /* PCM */
    header->data_id = ID_DATA;
    header->srate = info->srate;
    header->brate =  (header->fmt_sz / 8) * info->chn * info->srate;
    header->chn = info->chn;
    header->wid = 16;
    header->align = info->chn * header->wid / 8;
    header->fmt_id = ID_FMT;
    header->data_id = ID_DATA;

    return (u32)codec;
}

int wav_enc_stop(u32 obj)
{
    struct codec_wav_obj *codec = (struct codec_wav_obj *)obj;

    codec->header.data_sz = codec->size;
    codec->header.riff_size = codec->header.data_sz + sizeof(codec->header) - 8;

    return 0;
}

int wav_enc_close(u32 obj)
{
    struct codec_wav_obj *codec = (struct codec_wav_obj *)obj;

    audio_free(codec);

    return 0;
}

/* if < 0: error; if == 0: finished; if > 0: real sample count */
static int wav_enc_send_data(u32 obj, short *pbuf, u32 count)
{
    struct codec_wav_obj *codec = (struct codec_wav_obj *)obj;
    struct codec_inf *inf = &codec->inf;

    codec->size += count;

    return inf->codec_send(inf->priv, (u8 *)pbuf, count);
}

int wav_enc_get_header(u32 obj, u8 **buf, u32 *size)
{
    struct codec_wav_obj *codec = (struct codec_wav_obj *)obj;

    *buf = (u8 *)&codec->header;
    *size = sizeof(codec->header);

    return 0;
}

/* do not define as const */
static struct codec_engine wav_enc = {
    .type = CODEC_TYPE_WAV,
    .dir = CODEC_ENC,
    .open = wav_enc_open,
    .stop = wav_enc_stop,
    .close = wav_enc_close,
    .send_data = wav_enc_send_data,
    .get_header = wav_enc_get_header,
};

static int wav_codec_init(void)
{
    codec_register(&wav_dec);
    codec_register(&wav_enc);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(wav_codec_init);

