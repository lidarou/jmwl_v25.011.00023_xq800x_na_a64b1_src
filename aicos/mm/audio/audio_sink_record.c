/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is sine general function used by both audio sink and record.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x130554
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_errno.h>

#include "audio_sink_record.h"
#include "audio_system.h"
#include "audio_util.h"
#include "audio_sink.h"
#include <codec_mng.h>

u32 audio_create_handle(enum astream_type type, int flag,
                        enum astream_format format, enum astream_direct dir,
                        enum astream_source source)
{
    struct audio_handle *handle;
    static int uniq = 0;
    u32  ahandle = 0;

    handle = (struct audio_handle *)&ahandle;
    handle->type = type;
    handle->flag = flag;
    handle->fmt = format;
    handle->dir = dir;
    handle->src = source;
    handle->uniq = uniq++;
    /* uniq cannot be 0 */
    if (!handle->uniq)
        handle->uniq = uniq++;

    return ahandle;
}

u32 is_rec_handle(u32 handle)
{
    return (AHANDLE_DIR(1) & handle);
}

struct asink_buf *abuf_create(u32 period_size, u32 period_cnt)
{
    u32 size = period_size * period_cnt;
    struct asink_buf *abuf;

    if (size == 0)
        return NULL;

    abuf = audio_malloc(sizeof(struct asink_buf));

    if (!abuf) {
        AIDXE(0x0064fc, "", "create abuf failed, no memory\n");
        return NULL;
    }
    abuf->buf = audio_malloc(size * 2);
    if (!abuf->buf) {
        audio_free(abuf);
        AIDXE(0x1749a3, "", "create abuf failed, no memory2\n");

        return NULL;
    }
    abuf->buf_len = size * 2;
    abuf->rd = 0;
    abuf->wr = 0;
    abuf->drain = 0;
    abuf->event = aos_create_event("abuf event");

    return abuf;
}

void abuf_free(struct asink_buf *abuf)
{
    if (!abuf)
        return;

    if (abuf->buf)
        audio_free(abuf->buf);

    aos_delete_event(abuf->event);
    audio_free(abuf);
}

static int abuf_event_get(struct asink_buf *abuf, u32 wait_flag, u32 wait_time)
{
    u32 actual_flag;
    u32 ret;

    if (!abuf)
        return AIC_EACCES;

    /* TODO: wait time should get by sample rate and buffer size */
    ret = aos_get_event(abuf->event, wait_flag, AOS_OR_CLEAR,
                        &actual_flag, wait_time);

    if (ret)
        AIDXE(0x0af801, "sd", "%s, ret=%d\n", __func__, ret);

    return ret;
}

static int abuf_event_set(struct asink_buf *abuf, u32 flag)
{
    if (!abuf)
        return AIC_EACCES;

    aos_set_event(abuf->event, flag, AOS_OR);

    return 0;
}

char *abuf_write_get(struct asink_buf *abuf, u32 *size, u32 wait_time)
{
    u32 wr_pos , rd_pos;

    if (!abuf)
        return NULL;

    AASSERT(abuf->wr >= abuf->rd);

    AIDXD(0x1d55fb, "sdd", "%s, wr=%d, rd=%d\n", __func__, abuf->wr, abuf->rd);

    /**
     * wr - rd >= buf_len, means full.
     * Here we use 'while' but not 'if', because server side maybe
     * set EVENT before but user not get, we need clear this EVENT first.
     */
    while (abuf->wr - abuf->rd >= abuf->buf_len) {
        ADEBUG;
        /* wait empty */
        if(abuf_event_get(abuf, ABUF_EVENT_EMPTY | ABUF_EVENT_ALMOST_EMPTY,
                          wait_time) != AIC_SUCCESS) {
            AIDXE(0x19c916, "", "abuf write get failed\n");

            return 0;
        }
    }

    wr_pos = abuf->wr % abuf->buf_len;
    rd_pos = abuf->rd % abuf->buf_len;
    if (wr_pos < rd_pos)
        *size = (*size > (rd_pos - wr_pos)) ? (rd_pos - wr_pos) : *size;
    /* do not wrap, only return left of buffer */
    else if (wr_pos + *size > abuf->buf_len)
        *size = abuf->buf_len - wr_pos;

    ALOGV("%s, wr_pos=%d, size=%d\n", __func__, wr_pos, *size);

    return abuf->buf + wr_pos;
}

void abuf_write_release(struct asink_buf *abuf, u32 size)
{
    if (!abuf)
        return;

    if (abuf->rd == abuf->wr) {
        abuf->wr += size;
        abuf_event_set(abuf, ABUF_EVENT_DATA_IN);
    } else
        abuf->wr += size;

    AIDXD(0x1d55fb, "sdd", "%s, wr=%d, rd=%d\n", __func__, abuf->wr, abuf->rd);
}

char *abuf_read_get(struct asink_buf *abuf, u32 *size, u32 wait_time)
{
    u32 rd_pos, avail;

    if (!abuf) {
        AIDXE(0x0981d9, "", "abuf read get abuf is NULL!\n");
        return NULL;
    }

    AASSERT(abuf->wr >= abuf->rd);

    AIDXD(0x1d55fb, "sdd", "%s, wr=%d, rd=%d\n", __func__, abuf->wr, abuf->rd);

    /* if buffer empty, wait here */
    if (abuf->rd == abuf->wr) {
        if (abuf->drain)
            return ABUF_DRAIN;
        if (abuf_event_get(abuf, ABUF_EVENT_DATA_IN, wait_time) != AIC_SUCCESS){
            AIDXE(0x00080d, "", "abuf read get, wait failed\n");
            return NULL;
        }
    } else if (abuf->wr - abuf->rd < abuf->buf_len / 2) {
        /**
         * if left data less than half buffer, set almost empty,
         * but don't wait.
         * Now move it to 'release' function.
         */
        /* abuf_event_set(abuf, ABUF_EVENT_ALMOST_EMPTY); */
    }

    rd_pos = abuf->rd % abuf->buf_len;

    /* if enough data */
    if (abuf->rd + *size < abuf->wr) {
        /**
         * if need wrap, return left of buffer,
         * and read from head next time.
         */
        if (rd_pos + *size > abuf->buf_len)
            *size = abuf->buf_len - rd_pos;
    } else {
        /* else no enough data */
        avail = abuf->wr - abuf->rd;
        /* need wrap, read only tail of buffer */
        if (rd_pos + avail > abuf->buf_len)
            *size = abuf->buf_len - rd_pos;
        else
            /* if not need wrap, return left data */
            *size = avail;
    }

    ALOGV("%s, rd_pos=%d, size=%d\n", __func__, rd_pos, *size);

    return abuf->buf + rd_pos;
}

void abuf_read_release(struct asink_buf *abuf, u32 size)
{
    if (!abuf)
        return;

    abuf->rd += size;

    AIDXD(0x1d55fb, "sdd", "%s, wr=%d, rd=%d\n", __func__, abuf->wr, abuf->rd);

    /* if buffer empty, set empty */
    if (abuf->rd == abuf->wr)
        abuf_event_set(abuf, ABUF_EVENT_EMPTY);
    /* if left is equal or less than half buffer, set ALMOST_EMPTY */
    else if (abuf->wr - abuf->rd <= abuf->buf_len / 2)
        abuf_event_set(abuf, ABUF_EVENT_ALMOST_EMPTY);
}

void abuf_set_drain(struct asink_buf *abuf)
{
    if (!abuf)
        return;

    abuf->drain = TRUE;
}

enum codec_type audio_get_codec_type(enum astream_format format)
{
    switch(format) {
    case ASTREAM_FORMAT_WAV:
        return CODEC_TYPE_WAV;
    case ASTREAM_FORMAT_AAC:
        return CODEC_TYPE_AAC;
    case ASTREAM_FORMAT_MP3:
        return CODEC_TYPE_MP3;
    case ASTREAM_FORMAT_AMR_NB:
    case ASTREAM_FORMAT_AMR_WB:
        return CODEC_TYPE_AMR;
    case ASTREAM_FORMAT_MIDI:
        return CODEC_TYPE_MIDI;
    default:
        return CODEC_TYPE_NONE;
    }
}
