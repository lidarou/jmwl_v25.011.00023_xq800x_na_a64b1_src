/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The trace log formater source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ad620
#define LOG_MODULE    AMOD_ALOG

#include <alog_inner.h>
#include <alog_frame.h>
#include <alog_trace_format.h>
#include <stdio.h>

/* temp format:[trace][size]: */
#define ALOG_TRACE_HEADER_SIZE  28
#ifdef CONFIG_SUBSYS_AP
#define ALOG_TRACE_HEAD_FMT "\n[ap][sn %010d][%04x]:"
#else
#define ALOG_TRACE_HEAD_FMT "\n[cp][sn %010d][%04x]:"
#endif

void alog_trace_format(u32 sn, u8 *buf, size_t size)
{
#ifdef CONFIG_ALOG_FRAME_SUPPORT
    ALOG_FRAME_H_T *frame = (ALOG_FRAME_H_T *)buf;

#ifdef CONFIG_ALOG_TRACE_IDX
    ALOG_SET_FRAME_HEAD(frame, sn, size, LOG_PRO_TRACE_ID);
#else
    ALOG_SET_FRAME_HEAD(frame, sn, size, LOG_PRO_TRACE_TEXT);
#endif
#else
    snprintf(buf, ALOG_TRACE_HEADER_SIZE, ALOG_TRACE_HEAD_FMT, sn, size);
    buf[ALOG_TRACE_HEADER_SIZE - 1] = '\n';
#endif
}

size_t alog_trace_get_header_size(void)
{
#ifdef CONFIG_ALOG_FRAME_SUPPORT
    return ALOG_FRAME_H_SIZE;
#else
    return ALOG_TRACE_HEADER_SIZE;
#endif
}

