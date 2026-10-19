/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is stream client file, it provides methods to fill audio data
 *  when play stream, app can call API to write data.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18f802
#define LOG_MODULE    AMOD_AUDIO

#include <aic_osa.h>

#include <aic_type.h>
#include <aic_init.h>
#include <aic_errno.h>

#include "audio_system.h"
#include "audio_service.h"
#include "audio_sink.h"
#include "audio_stream.h"
#include "audio_policy.h"
#include "audio_util.h"
#include "codec_mng.h"
#include "audio_dump.h"

#define ASTREAM_RECORD_WITH_HEADER  0

struct asink_stream {
    struct asink_buf *abuf;
    struct codec_engine *codec;
    struct codec_info codec_info;
    u32 codec_obj;
    u32 consumed;
};

static inline struct asink_stream *asink_stream_new(void)
{
    struct asink_stream *asink_stream;

    asink_stream = audio_malloc(sizeof(*asink_stream));
    if (!asink_stream) {
        AIDXE(0x018d29, "", "asink stream create failed, no mem!\n");
        return 0;
    }
    memset(asink_stream, 0, sizeof(*asink_stream));

    return asink_stream;
}

static void asink_stream_delete(struct asink_stream *asink_stream)
{
    if (asink_stream)
        audio_free(asink_stream);
}

static int asink_stream_get(u32 obj, u8 *buffer, u32 get_size, u32 *real_size)
{
    struct asink_stream *asink = (struct asink_stream *)obj;
    char *pdata;

    *real_size = get_size;
    /* TODO: wait time should calculate by buffer size */
    pdata = abuf_read_get(asink->abuf, real_size, 21);

    ALOGV("stream get, pdata=%p, get_size=%d %d\n", pdata, get_size, *real_size);

    if (!pdata) {
        AIDXE(0x0e9dab, "", "asink stream callback, under run!\n");
        *real_size = 0;
        return ASTREAM_DATA_NODATA;
    }
    if (IS_ABUF_DRAIN(pdata)) {
         *real_size = 0;
         AIDXW(0x1762c7, "", "read over!\n");
         return ASTREAM_DATA_EOS;
    }
    memcpy(buffer, pdata, *real_size);
    abuf_read_release(asink->abuf, *real_size);

    return 0;
}

static int asink_stream_send(u32 obj, u8 *buffer, u32 len)
{
    struct asink_stream *asink = (struct asink_stream *)obj;
    u32 copy = 0, copied_total = 0;
    char *dest_buf;

    while (len > 0) {
        copy = len;
        dest_buf = abuf_write_get(asink->abuf, &copy, 60);
        if (!dest_buf) {
            AIDXE(0x024005, "", "asink stream write failed, can't get buf!\n");
            break;
        }

        memcpy(dest_buf, buffer, copy);
        len -= copy;
        copied_total += copy;
        buffer += copy;
        abuf_write_release(asink->abuf, copy);
    }
    asink->consumed += copied_total;
    AIDXD(0x1d8a96, "ddd", "stream len=%d, copied=%d, total=%d\n",
          len, copied_total, asink->consumed);

    return copied_total;
}

static u32 asink_stream_open(enum astream_format format, void *msg)
{
    struct asink_stream *asink = asink_stream_new();
    struct audio_msg_open *pmsg = msg;
    struct codec_info *info;
    enum codec_type codec_type;

    codec_type = audio_get_codec_type(format);
    if (codec_type != CODEC_TYPE_NONE
        && pmsg->type != ASTREAM_TYPE_VOIP_UL
        && pmsg->type != ASTREAM_TYPE_VOIP)
        asink->codec = codec_get_by_type(codec_type, pmsg->dir);

    if (asink->codec) {
        info = &asink->codec_info;
        info->srate = pmsg->sample_rate;
        info->chn = pmsg->chn;
        info->fmt = format;
        info->brate = pmsg->bit_rate;
        info->wid = 2;
    }
    AIDXI(0x055cc7, "pd", "%p, codec type=%d\n", asink->codec, codec_type);

    return (u32)asink;

}

static int asink_stream_close(u32 obj)
{
    struct asink_stream *asink = (struct asink_stream *)obj;

    asink_stream_delete(asink);
    return 0;

}

static int asink_stream_start(u32 obj, void *aplay_data)
{
    struct asink_stream *asink = (struct asink_stream *)obj;
    struct audio_play_sub_stream *sub = (struct audio_play_sub_stream *)aplay_data;
    struct codec_info *info;
    int ret;

    if (!sub) {
        AIDXE(0x1ddb87, "", "asink stream start error, null param\n");
        return AIC_EPERM;
    }
    asink->consumed = 0;
    asink->abuf = sub->abuf;

    if (asink->codec && asink->codec->open) {
        info = &asink->codec_info;
        struct codec_ops codec_ops;
        codec_ops.get_func = asink_stream_get;
        codec_ops.seek_func = NULL;
        codec_ops.send_func = asink_stream_send;
        asink->codec_obj = asink->codec->open(obj, &codec_ops, info);
        if (!asink->codec_obj) {
            asink->codec = NULL;
            AIDXE(0x1d8f43, "", "asink stream start, codec open failed\n");
            return AIC_EACCES;
        }
        sub->srate = info->srate;
        sub->chn = info->chn;
        sub->brate = info->brate;
        if (asink->codec->start) {
            ret = asink->codec->start(asink->codec_obj);
            if (ret) {
                AIDXE(0x13eeb4, "", "asink stream start, codec start failed\n");
                return AIC_EACCES;
            }
        }
    }

    return 0;
}

static int asink_stream_rec_start(u32 obj, void *aplay_data)
{
    int ret;

    ret = asink_stream_start(obj, aplay_data);

#if ASTREAM_RECORD_WITH_HEADER
    struct asink_stream *asink = (struct asink_stream *)obj;
    u32 header_size;
    u8 *header;
    if (!ret && asink->codec && asink->codec->get_header) {
        asink->codec->get_header(asink->codec_obj, &header, &header_size);
        asink_stream_send((u32)asink, header, header_size);
    }
#endif

    return ret;
}

static int asink_stream_prestop(u32 obj)
{
    int ret;
    struct asink_stream *asink = (struct asink_stream *)obj;

    if (asink->codec && asink->codec->stop) {
        ret = asink->codec->stop(asink->codec_obj);
        if (ret) {
            AIDXE(0x000b24, "", "asink stream pre_stop, codec pre_stop failed\n");
            return AIC_EACCES;
        }
    }
    return 0;
}


static int asink_stream_stop(u32 obj)
{
    struct asink_stream *asink = (struct asink_stream *)obj;
    int ret = 0;

    if (asink->codec && asink->codec->close && asink->codec_obj) {
        ret = asink->codec->close(asink->codec_obj);
        if (ret) {
            AIDXE(0x00a04b, "", "asink stream stop, codec close failed\n");

            return AIC_EACCES;
        }
    }

    return 0;
}

static int asink_stream_callback(u32 obj, int event, char *buffer,
                                u32 *get_size)
{
    struct asink_stream *asink = (struct asink_stream *)obj;
    u32 gen_count;
    int ret;

    if (asink->codec) {
        ret = asink->codec->get_data(asink->codec_obj, (short *)buffer,
                                     *get_size, &gen_count);
        if (ret < 0) {
            *get_size = 0;
            AIDXE(0x0b4a11, "d", "asink stream get_data failed, ret=%d\n", ret);
            return ret;
        }

        *get_size = gen_count;

        return ret;
    } else {
        char *pdata;
        /* TODO: wait time should calculate by buffer size */
        pdata = abuf_read_get(asink->abuf, get_size, 21);
        AIDXD(0x0494a0, "pd", "stream callbacked, pdata=%p, get_size=%d\n", pdata, *get_size);
        if (!pdata) {
            AIDXE(0x0e9dab, "", "asink stream callback, under run!\n");
            *get_size = 0;
            return AIC_EIO;
        }
        if (IS_ABUF_DRAIN(pdata)) {
            *get_size = 0;
            return ASTREAM_DATA_EOS;
        }
        memcpy(buffer, pdata, *get_size);
        abuf_read_release(asink->abuf, *get_size);

        return 0;
    }
}

static int asink_stream_rec_callback(u32 obj, int event, char *buffer,
                                     u32 *size)
{
    struct asink_stream *asink = (struct asink_stream *)obj;
    u32 len = *size, cnt = 0;
    char *pdata;
    int ret;

    if (asink->codec && asink->codec->send_data) {
        ret = asink->codec->send_data(asink->codec_obj, (short *)buffer, *size);
        if (ret < 0) {
            AIDXE(0x0b4a11, "d", "asink stream get_data failed, ret=%d\n", ret);
            return ret;
        }
        return 0;
    }

    while (cnt < *size) {
        len = *size - cnt;
        /* TODO: wait time should calculate by buffer size */
        pdata = abuf_write_get(asink->abuf, &len, 21);
        AIDXD(0x1b7b80, "pd", "stream rec callbacked, pdata=%p, size=%d\n", pdata, len);

        if (!pdata) {
            AIDXE(0x1072ac, "", "asink stream callback failed\n");

            return AIC_EIO;
        }

        memcpy(pdata, buffer + cnt, len);
        abuf_write_release(asink->abuf, len);

        cnt += len;
    }

    return 0;
}

#ifdef CONFIG_MCU_AUDIO
#ifdef CONFIG_MCU_AUDIO_VOIP_CLIENT
#include "voice_proc.h"

#define CLIENT_VOIP_UL_HDL      0x5650554c  /* VPUL */
#define CLIENT_VOIP_DL_HDL      0x5650444c  /* VPDL */

#define CLIENT_LOCK
#define CLIENT_SEMPH_SET(s)    if (s) aos_put_semaphore(s);
#define CLIENT_SEMPH_GET(s)    if (s) aos_get_semaphore(s, 41);

struct client_voip {
    void *hvp;
    u32 client_ul;
    u32 client_dl;
    struct codec_info info;
    aos_semaphore_ptr   semph;
    u32 ref_cnt;

    struct asink_buf *ul_abuf;
    struct asink_buf *dl_abuf;
};

struct client_voip *g_client_voip;

static struct client_voip *get_client_voip(void)
{
    struct client_voip *client_voip;

    if (g_client_voip)
        return g_client_voip;

    client_voip = audio_malloc(sizeof(*client_voip));
    if (!client_voip) {
        AIDXE(0x084244, "", "get client voip buf failed, no memory!\n");
        return 0;
    }
    memset(client_voip, 0, sizeof(*client_voip));

    client_voip->semph = aos_create_semaphore("client voip semph", 0);
    client_voip->client_ul = CLIENT_VOIP_UL_HDL;
    client_voip->client_dl = CLIENT_VOIP_DL_HDL;
    g_client_voip = client_voip;

    return client_voip;
}

static int voip_get_data(void *obj, u8 *buffer, u32 get_size,
                         u32 *real_size, u32 *fmt, u32 *mode)
{
    struct client_voip *client_voip = (struct client_voip *)obj;
    char *pdata;
    u32 len;

    *fmt = client_voip->info.fmt;
    if (*fmt == ASTREAM_FORMAT_AMR_NB || *fmt == ASTREAM_FORMAT_AMR_WB)
        len = 64;
    else if (*fmt == ASTREAM_FORMAT_PCM) {
        len = get_size;
    } else
        AIC_ASSERT(0);

    pdata = abuf_read_get(client_voip->dl_abuf, &len, 41);
    if (!pdata) {
        AIDXE(0x03dd6c, "", "voip get under run\n");
        *real_size = 0;
        return ASTREAM_DATA_NODATA;
    }

    memcpy(buffer, pdata, len);
    *real_size = len;
    abuf_read_release(client_voip->dl_abuf, len);

    return 0;
}

int voip_send_data(void *obj, u8 *buf, u32 size)
{
    struct client_voip *client_voip = (struct client_voip *)obj;
    u32 len, fmt;
    char *pdata;

    fmt = client_voip->info.fmt;
    if (fmt == ASTREAM_FORMAT_AMR_NB || fmt == ASTREAM_FORMAT_AMR_WB)
        len = 64;
    else if (fmt == ASTREAM_FORMAT_PCM) {
        len = size;
    } else
        AIC_ASSERT(0);

    pdata = abuf_write_get(client_voip->ul_abuf, &len, 41);
    if (!pdata) {
        AIDXE(0x0648dd, "", "voip send over run\n");
        return AIC_EIO;
    }

    if (fmt == ASTREAM_FORMAT_PCM) {
        memcpy(pdata, buf, len);
    } else {
        *pdata = size;
        memcpy(pdata + 1, buf, size);
    }
    abuf_write_release(client_voip->ul_abuf, len);

    return size;
}

static u32 voip_open(enum astream_format format, void *msg)
{
    struct client_voip *client_voip = get_client_voip();
    struct audio_msg_open *pmsg = msg;
    struct codec_info *info;

    if (!client_voip) {
        AIDXE(0x09deab, "", "no voip client\n");
        return 0;
    }
    info = &client_voip->info;
    if (client_voip->hvp)
        goto out;

    info->srate = pmsg->sample_rate;
    info->chn = pmsg->chn;
    info->fmt = format;
    info->brate = pmsg->bit_rate;
    info->wid = 2;
    info->flag = CODEC_INFO_NO_HEADER | CODEC_INFO_FRAME_MODE;
    client_voip->hvp = voice_proc_open(client_voip, info,
                                       voip_get_data, voip_send_data);
    if (!client_voip->hvp) {
        AIDXE(0x17562d, "", "voip client, voice process open failed\n");
        return 0;
    }

out:
    AIDXI(0x10f21e, "ddd", "client voip open sucess, fmt=%d, srate=%d, dir=%d\n",
          info->fmt, info->srate, pmsg->dir);

    client_voip->ref_cnt++;
    if (pmsg->dir == ASTREAM_DIR_PLAY)
        return client_voip->client_dl;
    else
        return client_voip->client_ul;
}

static int voip_close(u32 obj)
{
    struct client_voip *client_voip = (struct client_voip *)g_client_voip;

    if (client_voip->ref_cnt)
        client_voip->ref_cnt--;
    if (client_voip->ref_cnt == 0 && client_voip->hvp) {
        voice_proc_close(client_voip->hvp);
        client_voip->hvp = 0;
    }

    return 0;
}

static int voip_start(u32 obj, void *aplay_data)
{
    struct client_voip *client_voip = (struct client_voip *)g_client_voip;
    struct audio_play_sub_stream *sub = (struct audio_play_sub_stream *)aplay_data;

    if (!sub) {
        AIDXE(0x04fa4d, "", "voip start error, null param\n");
        return AIC_EPERM;
    }
    if (obj == CLIENT_VOIP_UL_HDL)
        client_voip->ul_abuf = sub->abuf;
    else if (obj == CLIENT_VOIP_DL_HDL)
        client_voip->dl_abuf = sub->abuf;
    else
        AIC_ASSERT(0);

    AIDXI(0x089b11, "sp", "voip start success, %s, abuf=%p\n",
          obj == CLIENT_VOIP_UL_HDL ? "UL" : "DL", sub->abuf);

    return 0;
}

static int voip_prestop(u32 obj)
{
    return 0;
}


static int voip_stop(u32 obj)
{
    return 0;
}

static int voip_callback(u32 obj, int event, char *buffer, u32 *rw_size)
{
    struct client_voip *client_voip = (struct client_voip *)g_client_voip;
    int ret = 0;

    if (!client_voip->hvp) {
        AIDXI(0x194c27, "", "voip callback failed, no process handle\n");
        return AIC_EPIPE;
    }
    switch (event) {
    case AUDIO_CLIENT_CMD_GET_MORE:
        CLIENT_SEMPH_GET(client_voip->semph);
        ret = voice_proc_get(client_voip->hvp, buffer, rw_size);
        if (ret < 0)
            AIDXE(0x11a377, "d", "voip get failed, ret=%d\n", ret);
        break;
    case AUDIO_CLIENT_CMD_DATA_NOTIFY:
        ret = voice_proc_send(client_voip->hvp, buffer, *rw_size);
        if (ret < 0)
            AIDXE(0x1220df, "d", "voip send failed, ret=%d\n", ret);
        CLIENT_SEMPH_SET(client_voip->semph);

        break;
    default:
        break;
    }

    return ret;
}
#endif
#endif

struct audio_client asink_client_stream = {
    .type = ASTREAM_TYPE_MUSIC,
    .aplay_type = APLAY_TYPE_STREAM,
    .open = asink_stream_open,
    .start = asink_stream_start,
    .pre_stop = asink_stream_prestop,
    .stop = asink_stream_stop,
    .close = asink_stream_close,
    .cb = asink_stream_callback
};

struct audio_client asink_client_stream_rec = {
    .type = ASTREAM_TYPE_RECORD,
    .aplay_type = APLAY_TYPE_STREAM,
    .open = asink_stream_open,
    .start = asink_stream_rec_start,
    .stop = asink_stream_stop,
    .close = asink_stream_close,
    .cb = asink_stream_rec_callback
};

#ifdef CONFIG_MCU_AUDIO
#ifdef CONFIG_MCU_AUDIO_VOIP_CLIENT
struct audio_client asink_client_voip_dl = {
    .type = ASTREAM_TYPE_VOIP,
    .aplay_type = APLAY_TYPE_STREAM,
    .open = voip_open,
    .close = voip_close,
    .start = voip_start,
    .pre_stop = voip_prestop,
    .stop = voip_stop,
    .cb = voip_callback
};

struct audio_client asink_client_voip_ul = {
    .type = ASTREAM_TYPE_VOIP_UL,
    .aplay_type = APLAY_TYPE_STREAM,
    .open = voip_open,
    .close = voip_close,
    .start = voip_start,
    .pre_stop = voip_prestop,
    .stop = voip_stop,
    .cb = voip_callback
};
#else
struct audio_client asink_client_voip_dl = {
    .type = ASTREAM_TYPE_VOIP,
    .aplay_type = APLAY_TYPE_STREAM,
};

struct audio_client asink_client_voip_ul = {
    .type = ASTREAM_TYPE_VOIP_UL,
    .aplay_type = APLAY_TYPE_STREAM,
};
#endif
#else
struct audio_client asink_client_voip_dl = {
    .type = ASTREAM_TYPE_VOIP,
    .aplay_type = APLAY_TYPE_STREAM,
    .open = asink_stream_open,
    .start = asink_stream_start,
    .stop = asink_stream_stop,
    .close = asink_stream_close,
    .cb = asink_stream_callback
};

struct audio_client asink_client_voip_ul = {
    .type = ASTREAM_TYPE_VOIP_UL,
    .aplay_type = APLAY_TYPE_STREAM,
    .open = asink_stream_open,
    .start = asink_stream_start,
    .stop = asink_stream_stop,
    .close = asink_stream_close,
    .cb = asink_stream_rec_callback
};
#endif

static int asink_stream_init(void)
{
    audio_client_register(&asink_client_stream);
    audio_client_register(&asink_client_stream_rec);
    audio_client_register(&asink_client_voip_dl);
    audio_client_register(&asink_client_voip_ul);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(asink_stream_init);


