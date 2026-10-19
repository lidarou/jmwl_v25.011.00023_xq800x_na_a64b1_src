/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is music reocrd client file, it provides methods to fill data
 *  when record music.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04836b
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

#define RECORD_DUMP 0

/*------------------------------record to buffer-----------------------------*/

struct arec_buf {
    char *buf;
    u32 buf_len;
    u32 rec_len;
    /* struct codec_engine *codec; */
    /* u32 codec_obj; */
};

static inline struct arec_buf *get_arec_buf_obj(void)
{
    struct arec_buf *arec_buf_obj = audio_malloc(sizeof(*arec_buf_obj));

    if (!arec_buf_obj) {
        AIDXE(0x02d55e, "", "get arec buf failed, no memory!\n");
        return 0;
    }
    memset(arec_buf_obj, 0, sizeof(*arec_buf_obj));

    return arec_buf_obj;
}

static u32 arec_buf_open(enum astream_format format, void *msg)
{
    struct arec_buf *arec = get_arec_buf_obj();

    if (!arec)
        return 0;

    arec->buf = NULL;
    arec->buf_len = 0;

    /* TODO: should check:
     * 1. offload
     * 2. code or decode
     */
#if 0
    codec_type = audio_get_codec_type(format);
    if (codec_type != CODEC_TYPE_NONE)
        asink->codec = codec_get_by_type(codec_type);
#endif

    return (u32)arec;
}

static int arec_buf_close(u32 obj)
{
    struct arec_buf *arec = (struct arec_buf *)obj;

    /* close codec if need */

    if (arec)
        audio_free(arec);

    return 0;
}

static int arec_buf_start(u32 obj, void *aplay_data)
{
    struct arec_buf *arec = (struct arec_buf *)obj;
    struct audio_play_sub_buf *sub = (struct audio_play_sub_buf *)aplay_data;

    if (!sub) {
        AIDXE(0x1ae739, "", "arec music start error, null param\n");

        return AIC_EPERM;
    }
    arec->buf = sub->buf;
    arec->buf_len = sub->buf_len;
    arec->rec_len = 0;

    /* TODO: codec support in the future */
#if 0
    if (arec->codec && arec->codec->open) {
        arec->codec_obj = arec->codec->open((u8 *)arec->buf, arec->buf_len);
        if (!arec->codec_obj) {
            arec->codec = NULL;
            AIDXE(0x04dc5e, "", "arec buf start, codec open failed\n");
            return AIC_EACCES;
        }
        if (arec->codec->start) {
            ret = arec->codec->start(arec->codec_obj);
            if (ret) {
                AIDXE(0x0e148a, "", "arec buf start, codec start failed\n");
                return AIC_EACCES;
            }
        }
    }
#endif

    return 0;
}

static int arec_buf_stop(u32 obj)
{
    struct arec_buf *arec = (struct arec_buf *)obj;

    arec->buf = NULL;
    arec->buf_len = 0;

    /* TODO: codec support in the future */
#if 0
    if (arec->codec && arec->codec->stop) {
        ret = arec->codec->stop(arec->codec_obj);
        if (ret) {
            AIDXE(0x0d1f87, "", "arec buf stop, codec stop failed\n");
            return AIC_EACCES;
        }
    }
#endif

    return 0;
}

static int arec_buf_callback(u32 obj, int event, char *buffer, u32 *size)
{
    struct arec_buf *arec = (struct arec_buf *)obj;
    u32 copy_len;

    /* TODO: add mutex */

    /* TODO: codec support in the future */
#if 0
    if (arec->codec) {
        /* TODO: now codec engine buffer is sample, not char buffer */
        ret = arec->codec->get_data(arec->codec_obj, (short *)buffer,
                                     *get_size / sizeof(short), &gen_count);
        if (ret < 0) {
            AIDXE(0x1a3549, "d", "arec buf get_data failed, ret=%d\n", ret);
            return ret;
        }

        /* TODO: change all len to samples? */
        *get_size = gen_count * sizeof(short);

        if (ret == 0)
            return ASTREAM_DATA_EOS;
    } else
#endif
    {
        /* get length that can copy */
        if (*size < arec->buf_len - arec->rec_len)
            copy_len = *size;
        else
            copy_len = arec->buf_len - arec->rec_len;

        memcpy(arec->buf + arec->rec_len, buffer, copy_len);
        arec->rec_len += copy_len;

        AIDXD(0x1a1867, "ddd", "arec buf get=%d, rec_len=%d, total=%d\n",
              copy_len, arec->rec_len, arec->buf_len);

        if (arec->rec_len >= arec->buf_len) {
            AIDXE(0x1cd89e, "", "rec full\n");
            return ASTREAM_DATA_EOS;
        }
    }

    return 0;
}

struct audio_client arec_client_buf = {
    .type = ASTREAM_TYPE_RECORD,
    .aplay_type = APLAY_TYPE_BUF,
    .open = arec_buf_open,
    .start = arec_buf_start,
    .stop = arec_buf_stop,
    .close = arec_buf_close,
    .cb = arec_buf_callback
};

static int arec_buf_init(void)
{
    audio_client_register(&arec_client_buf);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(arec_buf_init);

#ifdef CONFIG_FS_AUDIO_SUPPORT

/*-------------------------------record to file -----------------------------*/
#ifdef CONFIG_SAVE_REC_FILE_ASYNC

#define DATA_RDY    0x1u
#define THD_STOP    0x2u
#define THD_EXIT    0x4u

#define AREC_ASYNC_BUF_SIZE     4096

#define AREC_ASYNC_THREAD_STACK 1536
#define AREC_ASYNC_THREAD_PRI   40

struct arec_async_ctx {
    u8 *buf;
    u32 buf_len;
    u32 rd;
    u32 wr;
    aos_mutex_ptr mutex;
    aos_event_ptr event;
    aos_thread_id th;
    u8 is_run;
    u8 is_over;     /* AIC_TRUE: write failed */
};
#endif

struct arec_file {
    struct codec_engine *codec;
    u32 codec_obj;
    struct codec_info codec_info;

    char *fname;
    int fd;
    u32 rec_len;
#ifdef CONFIG_SAVE_REC_FILE_ASYNC
    struct arec_async_ctx async_ctx;
#endif
};

#ifdef CONFIG_SAVE_REC_FILE_ASYNC
static void arec_async_thread_loop(void *data)
{
    struct arec_file *arec = (struct arec_file *)data;
    struct arec_async_ctx *async_ctx = &arec->async_ctx;
    u32 event = 0;
    u8 *pbuf;
    ssize_t ret;

    while (1) {
        aos_get_event(async_ctx->event, DATA_RDY | THD_STOP, AOS_OR_CLEAR,
                      &event, AUDIO_WAIT_FOREVER);
        /* now any event we'll check if have data need save */
        /* if (event & DATA_RDY) */ {
            while (async_ctx->rd < async_ctx->wr && !async_ctx->is_over) {
                pbuf = async_ctx->buf +
                       ((async_ctx->rd & 0x1) ? async_ctx->buf_len : 0);
                ret = fs_write(arec->fd, pbuf, async_ctx->buf_len);
                if (ret < 0) {
                    AIDXE(0x1787e5, "dd", "arec aysnc write file size(%d) failed , ret=%d", async_ctx->buf_len, ret);
                    async_ctx->is_over = AIC_TRUE;
                } else {
                    async_ctx->rd++;
                }
                //ALOGE("arec async write:rd=%d,wr=%d\n", async_ctx->rd, async_ctx->wr);
            }
        }

        if (event & THD_STOP)
            break;
    }

    /* now thread exit, save left data that not full in block */
    if (arec->rec_len > async_ctx->rd * async_ctx->buf_len && !async_ctx->is_over) {
        u32 left;

        pbuf = async_ctx->buf + ((async_ctx->rd & 0x1) ? async_ctx->buf_len : 0);
        left = arec->rec_len - async_ctx->rd * async_ctx->buf_len;
        if (left > async_ctx->buf_len) {
            AIDXE(0x01ab2d, "d", "arec async write critical, left=%d\n", left);
            left = async_ctx->buf_len;
        }
        fs_write(arec->fd, pbuf, left);
        //ALOGE("arec async write done:lef=%d\n", left);
    }
    aos_set_event(async_ctx->event, THD_EXIT, AOS_OR);
}

static int creat_save_thread(struct arec_file *arec)
{
    int ret;
    void *pbuff;

    pbuff = audio_malloc(AREC_ASYNC_BUF_SIZE * 2);
    if (!pbuff) {
        AIDXE(0x15ba35, "", "arec async, no memory!\n");
        return AIC_ENOMEM;
    }
    arec->async_ctx.th = aos_create_thread("arec aysnc",
                                           arec_async_thread_loop,
                                           (void *)arec,
                                           AREC_ASYNC_THREAD_STACK,
                                           0,
                                           NULL,
                                           AREC_ASYNC_THREAD_PRI,
                                           AOS_DONT_START);

    if (arec->async_ctx.th) {
        arec->async_ctx.mutex = aos_create_mutex("arec async mutex");
        if (!arec->async_ctx.mutex) {
            AIDXE(0x0597a3, "", "arec async create mutex failed\n");
            ret = AIC_EPERM;
            goto err;
        }
        arec->async_ctx.event = aos_create_event("arec async event");
        if (!arec->async_ctx.event) {
            AIDXE(0x0bcb96, "", "arec async create event failed\n");
            ret = AIC_EPERM;
            goto err;
        }
    }
    arec->async_ctx.buf = pbuff;
    arec->async_ctx.buf_len = AREC_ASYNC_BUF_SIZE;

#if RECORD_DUMP
    audio_data("#!AMR-WB\n", 9, 0);
#endif

    aos_start_thread(arec->async_ctx.th);

    return 0;

err:
    if (pbuff)
        audio_free(pbuff);
    if (arec->async_ctx.mutex) {
        aos_delete_mutex(arec->async_ctx.mutex);
        arec->async_ctx.mutex = 0;
    }
    if (arec->async_ctx.event) {
        aos_delete_event(arec->async_ctx.event);
        arec->async_ctx.event = 0;
    }
    return 0;
}

#endif

static inline struct arec_file *get_arec_file_obj(void)
{
    struct arec_file *arec_file_obj = audio_malloc(sizeof(*arec_file_obj));

    if (!arec_file_obj) {
        AIDXE(0x02d55e, "", "get arec buf failed, no memory!\n");
        return 0;
    }
    memset(arec_file_obj, 0, sizeof(*arec_file_obj));

    return arec_file_obj;
}

#ifdef CONFIG_SAVE_REC_FILE_ASYNC
static int arec_file_save(u32 priv, u8 *buffer, u32 size)
{
    struct arec_file *arec = (struct arec_file *)priv;
    struct arec_async_ctx *async_ctx = &arec->async_ctx;
    u8 *pbuf;
    u32 offs, avail;

    if (!buffer || !size || !arec->fd)
        return 0;
    /* last write operation is failed */
    if (async_ctx->is_over) {
        AIDXE(0x0f6c91, "", "arec_file_save: is over");
        return ASTREAM_DATA_EOS;
    }
    /* no empty block, return directly */
    if (async_ctx->wr - async_ctx->rd >= 2) {
        AIDXE(0x1dcb4d, "", "arec async, no space, lost one frame!\n");
        return 0;
    }
    offs = arec->rec_len % (async_ctx->buf_len << 1);
    pbuf = async_ctx->buf + offs;

#if RECORD_DUMP
    audio_data(buffer, size, 0);
#endif
    avail = async_ctx->buf_len - arec->rec_len % (async_ctx->buf_len);
    if (size < avail) {
        memcpy(pbuf, buffer, size);
        arec->rec_len += size;
        //ALOGE("arec save:rec_len=%d,size=%d\n", arec->rec_len, size);
    } else {
        if (async_ctx->wr == async_ctx->rd) {
            /* copy to tail of current buffer */
            memcpy(pbuf, buffer, avail);

            /* then copy left to next buffer header */
            if (offs < async_ctx->buf_len)
                pbuf = async_ctx->buf + async_ctx->buf_len;
            else
                pbuf = async_ctx->buf;
            memcpy(pbuf, buffer + avail, size - avail);

            arec->rec_len += size;
            async_ctx->wr++;
            //ALOGE("arec save done:rec_len=%d,size=%d\n", arec->rec_len, size);
            /* notify to save */
            aos_set_event(async_ctx->event, DATA_RDY, AOS_OR);
        } else {
            /* only one block, but left space is not enough */
            AIDXE(0x0139d2, "", "arec async, lost one frame!\n");
        }
    }

    return 0;
}
#else
static int arec_file_save(u32 priv, u8 *buffer, u32 size)
{
    struct arec_file *arec = (struct arec_file *)priv;
    ssize_t ret;

    if (!buffer || !size || !arec->fd)
        return 0;

    ret = fs_write(arec->fd, buffer, size);
    if (ret == size) {
        arec->rec_len += size;
        AIDXE(0x140966, "dd", "arec size=%d, rec_len=%d\n", size, arec->rec_len);
    } else {
        AIDXE(0x0dec26, "d", "rec save file failed, ret=%d\n", ret);
        return ASTREAM_DATA_EOS;
    }

    return 0;
}
#endif
static u32 arec_file_open(enum astream_format format, void *msg)
{
    struct arec_file *arec = get_arec_file_obj();
    struct audio_msg_open *pmsg = msg;
    struct codec_info *info;
    enum codec_type codec_type;

    if (!arec)
        return 0;

    codec_type = audio_get_codec_type(format);
    if (codec_type != CODEC_TYPE_NONE)
        arec->codec = codec_get_by_type(codec_type, CODEC_ENC);

    if (arec->codec) {
        info = &arec->codec_info;
        info->srate = pmsg->sample_rate;
        info->chn = pmsg->chn;
        info->fmt = format;
        info->brate = pmsg->bit_rate;
        info->wid = 2;
    }

    return (u32)arec;
}

static int arec_file_close(u32 obj)
{
    struct arec_file *arec = (struct arec_file *)obj;

    if (arec)
        audio_free(arec);

    return 0;
}

static int arec_file_start(u32 obj, void *aplay_data)
{
    struct arec_file *arec = (struct arec_file *)obj;
    struct audio_play_sub_file *sub = (struct audio_play_sub_file *)aplay_data;
    struct codec_info *info;
    int fd, ret;

    if (!sub || !sub->file_name) {
        AIDXE(0x0d279f, "", "audio record start error, null param\n");
        return AIC_EPERM;
    }

    fd = fs_open(sub->file_name, FS_O_WRONLY | FS_O_CREATE | FS_O_TRUNC);
    if (fd < 0) {
        AIDXE(0x1d9a0e, "d", "audio record create file failed, fd=%d\n", fd);
        return AIC_EPIPE;
    }
    arec->fd = fd;
    arec->fname = sub->file_name;

    if (arec->codec && arec->codec->open) {
        info = &arec->codec_info;
        struct codec_ops codec_ops;
        codec_ops.get_func = NULL;
        codec_ops.send_func = arec_file_save;
        codec_ops.seek_func = NULL;
        arec->codec_obj = arec->codec->open(obj, &codec_ops,
                                            info);
        if (!arec->codec_obj) {
            arec->codec = NULL;
            AIDXE(0x0011f0, "", "arec file start, codec open failed\n");
            return AIC_EACCES;
        }
        if (arec->codec->start) {
            ret = arec->codec->start(arec->codec_obj);
            if (ret) {
                AIDXE(0x110acd, "", "arec file start, codec start failed\n");
                return AIC_EACCES;
            }
        }
    }

    /* save file header */
    if (arec->codec_obj && arec->codec->get_header) {
        u8 *header;
        u32 header_size;

        arec->codec->get_header(arec->codec_obj, &header, &header_size);
        if (fs_write(fd, header, header_size) < 0)
            AIDXE(0x080fac, "", "arec save file header failed\n");
    }

#ifdef CONFIG_SAVE_REC_FILE_ASYNC
    return creat_save_thread(arec);
#else
    return 0;
#endif
}

static int arec_file_stop(u32 obj)
{
    struct arec_file *arec = (struct arec_file *)obj;

#ifdef CONFIG_SAVE_REC_FILE_ASYNC
    struct arec_async_ctx *async_ctx = &arec->async_ctx;
    u32 event;

    aos_set_event(async_ctx->event, THD_STOP, AOS_OR);
    aos_get_event(async_ctx->event, THD_EXIT, AOS_OR_CLEAR,
                  &event, AUDIO_WAIT_FOREVER);
    if (arec->async_ctx.buf) {
        audio_free(arec->async_ctx.buf);
        arec->async_ctx.buf = 0;
    }
    if (arec->async_ctx.mutex) {
        aos_delete_mutex(arec->async_ctx.mutex);
        arec->async_ctx.mutex = 0;
    }
    if (arec->async_ctx.event) {
        aos_delete_event(arec->async_ctx.event);
        arec->async_ctx.event = 0;
    }
    if (arec->async_ctx.th) {
        aos_delete_thread(arec->async_ctx.th);
        arec->async_ctx.th = 0;
    }
#endif

    if (arec->codec_obj) {
        if (arec->codec->stop)
            arec->codec->stop(arec->codec_obj);

        /* save file tail */
        if (arec->codec->get_tail) {
            u8 *tail;
            u32 tail_size;

            arec->codec->get_tail(arec->codec_obj, &tail, &tail_size);
            if (fs_write(arec->fd, tail, tail_size) < 0)
                AIDXE(0x0fc7d7, "", "arec save file tail failed\n");
        }

        /* update file header */
        if (arec->codec->get_header) {
            u8 *header;
            u32 header_size;

            arec->codec->get_header(arec->codec_obj, &header, &header_size);
            fs_seek(arec->fd, 0, FS_SEEK_BEGIN);
            if (fs_write(arec->fd, header, header_size) < 0)
                AIDXE(0x100774, "", "arec update file header failed\n");
        }
        if (arec->codec->close)
            arec->codec->close(arec->codec_obj);
    }

    if (arec->fd) {
        fs_close(arec->fd);
        arec->fd = 0;
    }

    arec->rec_len = 0;

    return 0;
}

static int arec_file_callback(u32 obj, int event, char *buffer, u32 *size)
{
    struct arec_file *arec = (struct arec_file *)obj;

    if (arec->codec_obj) {
        if (arec->codec->send_data)
            return arec->codec->send_data(arec->codec_obj, (u16 *)buffer, *size);
    } else
        return arec_file_save(obj, buffer, *size);

    return 0;
}

struct audio_client arec_client_file = {
    .type = ASTREAM_TYPE_RECORD,
    .aplay_type = APLAY_TYPE_FILE,
    .open = arec_file_open,
    .start = arec_file_start,
    .stop = arec_file_stop,
    .close = arec_file_close,
    .cb = arec_file_callback
};

static int arec_file_init(void)
{
    audio_client_register(&arec_client_file);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(arec_file_init);

#endif

