/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This is the header file for audio sink and record common, some structs
 *  and APIs are used both audio sink and record.
 *
 *****************************************************************************/

#ifndef __AUDIO_SINK_RECORD_H__
#define __AUDIO_SINK_RECORD_H__

#include <aic_bitmap.h>
#include <aic_osa.h>

#include "audio_system.h"

#define AHANDLE unsigned int

struct audio_handle {
    u32 type  : 5;
    u32 fmt   : 4;
    u32 mode  : 3;
    u32 dir   : 1;
    u32 flag  : 10;
    u32 src   : 3;
    u32 uniq  : 6;
};

#define AHANDLE_TYPE(x)     ((x & GENMASK(0, 4)))
#define AHANDLE_FMT(x)      ((x << 5) & GENMASK(5, 8))
#define AHANDLE_MODE(x)     ((x << 9) & GENMASK(9, 11))
#define AHANDLE_DIR(x)      ((x << 12) & GENMASK(12, 12))
#define AHANDLE_FLAG(x)     ((x << 13) & GENMASK(13, 22))
#define AHANDLE_SRC(x)      ((x << 23) & GENMASK(23, 25))
#define AHANDLE_UNIQ(x)     ((x << 26) & GENMASK(26, 31))

#define IS_SINK(h)          (!(AHANDLE_DIR(1) & h))
#define IS_REC(h)           (AHANDLE_DIR(1) & h)

#define ABUF_DRAIN          ((char *)0xffffffff)
#define IS_ABUF_DRAIN(a)    (a == ABUF_DRAIN)

#define ABUF_EVENT_EMPTY        1
#define ABUF_EVENT_ALMOST_EMPTY 2
#define ABUF_EVENT_DATA_IN      4
#define ABUF_EVENT_FULL         8
#define ABUF_EVNET_ALMOST_FULL  10

struct asink_buf {
    char *buf;
    u32 rd;
    u32 wr;
    u32 buf_len;
    u32 drain;
    aos_event_ptr event;
};

u32 audio_create_handle(enum astream_type type, int flag,
                        enum astream_format format, enum astream_direct dir,
                        enum astream_source source);

struct asink_buf *abuf_create(u32 period_size, u32 period_cnt);
void abuf_free(struct asink_buf *abuf);
char *abuf_write_get(struct asink_buf *abuf, u32 *size, u32 wait_time);
void abuf_write_release(struct asink_buf *abuf, u32 size);
char *abuf_read_get(struct asink_buf *abuf, u32 *size, u32 wait_time);
void abuf_read_release(struct asink_buf *abuf, u32 size);
void abuf_set_drain(struct asink_buf *abuf);
enum codec_type audio_get_codec_type(enum astream_format format);

#endif /* __AUDIO_SINK_RECORD_H__ */

