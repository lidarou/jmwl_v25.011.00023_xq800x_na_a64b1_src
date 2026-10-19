/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is music sink client file, it provides methods to fill data
 *  when play music.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07947f
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_init.h>
#include <alog_api.h>
#include <aic_errno.h>
#ifdef CONFIG_FS_AUDIO_SUPPORT
#include <fs_api.h>
#endif
#include "audio_system.h"
#include "audio_service.h"
#include "audio_sink.h"
#include "audio_stream.h"
#include "audio_policy.h"
#include "audio_sink_record.h"
#include "codec_mng.h"

struct asink_music_buf {
    char *buf;
    AHANDLE handle;
    u32 buf_len;
    u32 consumed;
    u32 seek_timestamp;
    struct codec_engine *codec;
    struct codec_info codec_info;
    u32 codec_obj;
};

static inline struct asink_music_buf *get_asink_music_buf(void)
{
    struct asink_music_buf *music_buf_obj = audio_malloc(sizeof(*music_buf_obj));

    if (!music_buf_obj) {
        AIDXE(0x1619c5, "", "get asink music buf failed, no memory!\n");
        return 0;
    }
    memset(music_buf_obj, 0, sizeof(*music_buf_obj));

    return music_buf_obj;
}

static int asink_music_get(u32 obj, u8 *buffer, u32 get_size, u32 *real_size)
{
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;
    u32 copy_len;

    /* TODO: add mutex */

    if (get_size < asink->buf_len - asink->consumed)
        copy_len = get_size;
    else
        copy_len = asink->buf_len - asink->consumed;

    *real_size = copy_len;
    memcpy(buffer, asink->buf + asink->consumed, copy_len);
    asink->consumed += copy_len;

    AIDXD(0x0368db, "ddd", "asink music get=%d, consumed=%d, total=%d\n",
          copy_len, asink->consumed, asink->buf_len);

    if (asink->consumed >= asink->buf_len)
        return ASTREAM_DATA_EOS;

    return 0;
}

static s64 asink_music_seek_cb(u32 obj, s64 offset, u32 mode)
{
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;
    s64 ret;

    switch(mode) {
    case CODEC_SEEK_BEGIN:
    {
        if((offset < 0) || ((offset + 1) > asink->buf_len)) {
            ret = -1;
            break;
        }
        ret = asink->consumed = offset;
        break;
    }
    case CODEC_SEEK_CUR:
    {
        s64 pos = (s64)asink->consumed + (s64)offset;
        if(pos < 0 || pos > asink->buf_len) {
            ret = -1;
            break;
        }
        asink->consumed = pos;
        ret = pos;
        break;
    }
    case CODEC_SEEK_END:
    {
        if((offset > 0) || (ret = (offset + (s64)asink->buf_len)) < 0){
            ret = -1;
            break;
        }
        asink->consumed = ret;
        break;
    }
    case CODEC_SEEK_SIZE:
        ret = asink->buf_len;
        break;
    default:
        AIDXE(0x14525b, "", "err seek mode!\n");
        return AIC_EINVAL;
    }

    return ret;
}

static u32 asink_music_open(enum astream_format format, void *msg)
{
    struct asink_music_buf *asink = get_asink_music_buf();
    struct audio_msg_open *pmsg = msg;
    struct codec_info *info;
    enum codec_type codec_type;

    asink->buf = NULL;
    asink->buf_len = 0;

    /* TODO: should check:
     * 1. offload
     * 2. code or decode
     */
    codec_type = audio_get_codec_type(format);
    if (codec_type != CODEC_TYPE_NONE)
        asink->codec = codec_get_by_type(codec_type, CODEC_DEC);

    if (asink->codec) {
        info = &asink->codec_info;
        info->srate = pmsg->sample_rate;
        info->chn = pmsg->chn;
        info->fmt = format;
        info->brate = pmsg->bit_rate;
        info->wid = 2;
    }
    AIDXI(0x1ced72, "p", "%p, codec type=&d\n", asink->codec, codec_type);

    return (u32)asink;
}

static int asink_music_close(u32 obj)
{
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;

    audio_free(asink);

    return 0;
}

static int asink_music_start(u32 obj, void *aplay_data)
{
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;
    struct audio_play_sub_buf *sub = (struct audio_play_sub_buf *)aplay_data;
    struct codec_info *info;
    int ret;

    if (!sub) {
        AIDXE(0x0872c2, "", "asink music start error, null param\n");
        return AIC_EPERM;
    }
    asink->buf = sub->buf;
    asink->buf_len = sub->buf_len;
    asink->consumed = 0;

    if (asink->codec && asink->codec->open) {
        struct codec_ops codec_ops;
        info = &asink->codec_info;
        codec_ops.get_func = asink_music_get;
        codec_ops.send_func = NULL;
        codec_ops.seek_func = asink_music_seek_cb;
        info->seek_timestamp = asink->seek_timestamp;
        info->duration = 0;
        asink->codec_obj = asink->codec->open(obj, &codec_ops, info);
        if (!asink->codec_obj) {
            asink->codec = NULL;
            AIDXE(0x18b884, "", "asink music start, codec open failed\n");
            return AIC_EACCES;
        }
        sub->srate = info->srate;
        sub->chn = info->chn;
        sub->brate = info->brate;
        if (asink->codec->start) {
            ret = asink->codec->start(asink->codec_obj);
            if (ret) {
                AIDXE(0x1a6984, "", "asink music start, codec start failed\n");
                return AIC_EACCES;
            }
        }
    }

    return 0;
}

static int asink_music_seek(u32 obj, u32 timestamp)
{
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;
    asink->seek_timestamp = timestamp;

    return 0;
}


static int asink_music_prestop(u32 obj)
{
    int ret;
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;

    if (asink->codec && asink->codec->stop) {
        ret = asink->codec->stop(asink->codec_obj);
        if (ret) {
            AIDXE(0x064bdc, "", "asink music pre_stop, codec pre_stop failed\n");
            return AIC_EACCES;
        }
    }
    return 0;
}

static int asink_music_stop(u32 obj)
{
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;
    int ret;

    asink->buf = NULL;
    asink->buf_len = 0;
    if (asink->codec && asink->codec->close) {
        ret = asink->codec->close(asink->codec_obj);
        if (ret) {
            AIDXE(0x0e607e, "", "asink music close, codec close failed\n");
            return AIC_EACCES;
        }
    }

    return 0;
}

static int asink_music_get_seektime(u32 obj, u32 *timestamp)
{
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;

    *timestamp = asink->codec_info.seek_timestamp;

    return 0;
}


static int asink_music_callback(u32 obj, int event, char *buffer,
                                u32 *get_size)
{
    struct asink_music_buf *asink = (struct asink_music_buf *)obj;
    u32 gen_count;
    int ret;

    /* TODO: add mutex */
    if (asink->codec) {
        /* TODO: now codec engine buffer is sample, not char buffer */
        ret = asink->codec->get_data(asink->codec_obj, (short *)buffer,
                                     *get_size, &gen_count);
        if (ret < 0) {
            AIDXE(0x115bc0, "d", "asink buf get_data failed, ret=%d\n", ret);
            return ret;
        }

        /* TODO: change all len to samples? */
        *get_size = gen_count;
    } else
        ret = asink_music_get(obj, buffer, *get_size, get_size);

    return ret;
}

struct audio_client asink_client_music_buf = {
    .type = ASTREAM_TYPE_MUSIC,
    .aplay_type = APLAY_TYPE_BUF,
    .open = asink_music_open,
    .start = asink_music_start,
    .stop = asink_music_stop,
    .seek = asink_music_seek,
    .close = asink_music_close,
    .pre_stop = asink_music_prestop,
    .cb = asink_music_callback,
    .get_seek_time = asink_music_get_seektime,

};

#ifdef CONFIG_FS_AUDIO_SUPPORT

struct asink_music_file {
    char *fname;
    int fd;
    u32 file_size;
    u32 consumed;
    u32 seek_timestamp;
    struct codec_engine *codec;
    struct codec_info codec_info;
    u32 codec_obj;
};


static inline struct asink_music_file *get_asink_music_file(void)
{
    struct asink_music_file *asink = audio_malloc(sizeof(*asink));

    if (!asink) {
        AIDXE(0x0af7e8, "", "get asink music file failed, no memory!\n");
        return 0;
    }
    memset(asink, 0, sizeof(*asink));

    return asink;
}

static int asink_music_file_get(u32 obj, u8 *buffer, u32 get_size,
                                u32 *real_size)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;
    int size;

    /* TODO: add mutex */
    if (!asink->fd) {
        aos_sleep(5);
        *real_size = 0;

        return 0;
    }
    size = fs_read(asink->fd, buffer, get_size);
    if (size > 0) {
        asink->consumed += size;
        *real_size = size;
        AIDXD(0x0b93b7, "ddd", "asink file get=%d, consumed=%d, total=%d\n",
              size, asink->consumed, asink->file_size);
        if (asink->consumed >= asink->file_size) {
            AIDXE(0x090c03, "d", "asink file read complete, size=%d\n", size);
            return ASTREAM_DATA_EOS;
        }
    } else {
        AIDXE(0x00f13c, "d", "asink file read done, size=%d\n", size);
        return ASTREAM_DATA_EOS;
    }

    return 0;
}

static s64 asink_music_file_seek_cb(u32 obj, s64 offset, u32 mode)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;
    s32 fs_mode;
    off_t ret;

    if(!asink->fd) {
        aos_sleep(5);
        return AIC_ENOENT;
    }
    switch(mode) {
    case CODEC_SEEK_BEGIN:
        fs_mode = FS_SEEK_BEGIN;
        break;
    case CODEC_SEEK_CUR:
        fs_mode = FS_SEEK_CUR;
        break;
    case CODEC_SEEK_END:
        fs_mode = FS_SEEK_END;
        break;
    case CODEC_SEEK_SIZE:
        fs_mode = FS_SEEK_END;
        break;
    default:
        AIDXE(0x14525b, "", "err seek mode!\n");
        return AIC_EINVAL;
    }
    if(mode != CODEC_SEEK_SIZE) {
        ret = fs_seek(asink->fd, offset, fs_mode);
        asink->consumed = ret;
    }
    else {
        struct stat st_info;

        if(fstat(asink->fd, &st_info)) {
            ret = AIC_EIO;
        } else {
            ret = st_info.st_size;
        }
    }

    return ret;

}

static u32 asink_music_file_open(enum astream_format format, void *msg)
{
    struct asink_music_file *asink = get_asink_music_file();
    struct audio_msg_open *pmsg = msg;
    struct codec_info *info;
    enum codec_type codec_type;

    /* TODO: should check:
     * 1. offload
     * 2. code or decode
     */
    codec_type = audio_get_codec_type(format);
    if (codec_type != CODEC_TYPE_NONE)
        asink->codec = codec_get_by_type(codec_type, CODEC_DEC);

    if (asink->codec) {
        info = &asink->codec_info;
        info->srate = pmsg->sample_rate;
        info->chn = pmsg->chn;
        info->fmt = format;
        info->brate = pmsg->bit_rate;
        info->wid = 2;
    }

    AIDXI(0x1d24a6, "pdd", "asink music file open, %p, codec type=%d, format=%d\n",
          asink->codec, codec_type, format);

    return (u32)asink;
}

static int asink_music_file_close(u32 obj)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;

    audio_free(asink);
    return 0;
}

static int asink_music_file_start(u32 obj, void *aplay_data)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;
    struct audio_play_sub_file *sub = (struct audio_play_sub_file *)aplay_data;
    struct codec_info *codec_info;
    FS_FILE_INFO info;

    int ret;

    if (!sub || !sub->file_name) {
        AIDXE(0x0aabf3, "", "asink music file start error, null param\n");
        return AIC_EPERM;
    }
    asink->fname = sub->file_name;
    asink->consumed = 0;

    if(fs_getinfo(sub->file_name, &info)) {
        AIDXE(0x0d37af, "", "asink music get file size failed!\n");
        return AIC_EACCES;
    }
    asink->file_size = info.size;

    asink->fd = fs_open(sub->file_name, FS_O_RDONLY);
    if (asink->fd <= 0) {
        AIDXE(0x094248, "d", "asink music file start, file open failed, ret=%d\n",
              asink->fd);
        return AIC_EPIPE;
    }
    fs_seek(asink->fd, 0, FS_SEEK_BEGIN);

    if (asink->codec && asink->codec->open) {
        struct codec_ops codec_ops;
        codec_info = &asink->codec_info;
        codec_ops.get_func = asink_music_file_get;
        codec_ops.send_func = NULL;
        codec_ops.seek_func = asink_music_file_seek_cb;
        codec_info->seek_timestamp = asink->seek_timestamp;
        codec_info->duration = 0;
        asink->codec_obj = asink->codec->open(obj, &codec_ops, codec_info);
        if (!asink->codec_obj) {
            asink->codec = NULL;
            if (asink->fd) {
                fs_close(asink->fd);
                asink->fd = 0;
            }
            AIDXE(0x18284a, "", "asink music file start, codec open failed\n");
            return AIC_EACCES;
        }

        /* change to real music info */
        sub->srate = codec_info->srate;
        sub->chn = codec_info->chn;
        sub->brate = codec_info->brate;
        if (asink->codec->start) {
            ret = asink->codec->start(asink->codec_obj);
            if (ret) {
                if (asink->fd) {
                    fs_close(asink->fd);
                    asink->fd = 0;
                }
                AIDXE(0x0fa134, "", "asink music file start, codec start failed\n");
                return AIC_EACCES;
            }
        }
    }

    return 0;
}

static int asink_music_file_seek(u32 obj, u32 timestamp)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;
    asink->seek_timestamp = timestamp;

    return 0;
}

static int asink_music_file_get_seektime(u32 obj, u32 *timestamp)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;

    *timestamp = asink->codec_info.seek_timestamp;

    return 0;
}

static int asink_music_file_prestop(u32 obj)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;
    int ret;

    if (asink->codec && asink->codec->stop) {
        ret = asink->codec->stop(asink->codec_obj);
        if (ret) {
            AIDXE(0x19baf5, "", "asink music file pre_stop, codec pre_stop failed\n");
            return AIC_EACCES;
        }
    }
    return 0;
}

static int asink_music_file_stop(u32 obj)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;
    int ret;

    if (asink->codec && asink->codec->close) {
        ret = asink->codec->close(asink->codec_obj);

        if (ret) {
            return AIC_EACCES;
        }
    }

    asink->file_size = 0;
    if (asink->fd > 0) {
        fs_close(asink->fd);
        asink->fd = 0;
    }

    return 0;
}

static int asink_music_file_callback(u32 obj, int event, char *buffer,
                                u32 *get_size)
{
    struct asink_music_file *asink = (struct asink_music_file *)obj;
    u32 gen_count;
    int ret;

    /* TODO: add mutex */
    if (asink->codec) {
        /* TODO: now codec engine buffer is sample, not char buffer */
        ret = asink->codec->get_data(asink->codec_obj, (short *)buffer,
                                     *get_size, &gen_count);
        if (ret < 0) {
            AIDXE(0x16cedb, "d", "asink codec get_data failed, ret=%d\n", ret);
            return ret;
        }

        *get_size = gen_count;

        return ret;
    } else
        return asink_music_file_get(obj, buffer, *get_size, get_size);
}

struct audio_client asink_client_music_file = {
    .type = ASTREAM_TYPE_MUSIC,
    .aplay_type = APLAY_TYPE_FILE,
    .open = asink_music_file_open,
    .start = asink_music_file_start,
    .seek = asink_music_file_seek,
    .stop = asink_music_file_stop,
    .close = asink_music_file_close,
    .pre_stop = asink_music_file_prestop,
    .cb = asink_music_file_callback,
    .get_seek_time = asink_music_file_get_seektime,
};

struct audio_client asink_client_voip_file = {
    .type = ASTREAM_TYPE_VOIP,
    .aplay_type = APLAY_TYPE_FILE,
    .open = asink_music_file_open,
    .start = asink_music_file_start,
    .seek = NULL,
    .stop = asink_music_file_stop,
    .close = asink_music_file_close,
    .pre_stop = asink_music_file_prestop,
    .cb = asink_music_file_callback
};
#endif

static int asink_music_init(void)
{
    audio_client_register(&asink_client_music_buf);
#ifdef CONFIG_FS_AUDIO_SUPPORT
    audio_client_register(&asink_client_music_file);
    audio_client_register(&asink_client_voip_file);
#endif
    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(asink_music_init);

