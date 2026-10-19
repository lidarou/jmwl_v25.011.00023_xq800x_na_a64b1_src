/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log frame header file.

 */
#ifndef __ALOG_FRAME_H__
#define __ALOG_FRAME_H__

#include <aic_type.h>
#include <aic_sys.h>

/* log frame magic define. */
#define ALOG_FRAM_MAGIC     0x7e6b6b7e

/* log protocol type define. */
enum {
    /* trace */
    LOG_PRO_TRACE_BASE = 0x00,
    LOG_PRO_TRACE_ID_FILE_LINE,
    LOG_PRO_TRACE_TEXT_FILE_LINE,
    LOG_PRO_TRACE_ID_NO_FILE_LINE,
    LOG_PRO_TRACE_TEXT_NO_FILE_LINE,

    /* message */
    LOG_PRO_MSG_BASE = 0x20,
    LOG_PRO_MCORE_MSG,
    LOG_PRO_PF_TRACE,

    /* net data */
    LOG_PRO_NET_DATA_BASE = 0x40,
    LOG_PRO_NET_DATA_CAP,

    /* audio data */
    LOG_PRO_AUD_DATA_BASE = 0x60,
    LOG_PRO_AUD_DATA_PCM,

    /* adsp data */
    LOG_PRO_DSP_DATA_BASE = 0x80,
    LOG_PRO_DSP_DATA1,
    LOG_PRO_DSP_DATA2,
    LOG_PRO_DSP_DATA3,

    /* phy data */
    LOG_PRO_PHY_DATA_BASE = 0xa0,
    LOG_PRO_IQ_DATA_LTE,

    /* misc data */
    LOG_PRO_MISC_BASE = 0xc0,
    LOG_PRO_DATA_NORMAL,
    LOG_PRO_DATA_DUMP,
};

/* the subpro define */
enum {
    SUB_PRO_START = 0,
    SUB_PRO_SAVE,
    SUB_PRO_FINISH,
};

#ifdef CONFIG_LOG_FILE_LINE
#define LOG_PRO_TRACE_ID    LOG_PRO_TRACE_ID_FILE_LINE
#define LOG_PRO_TRACE_TEXT  LOG_PRO_TRACE_TEXT_FILE_LINE
#else
#define LOG_PRO_TRACE_ID    LOG_PRO_TRACE_ID_NO_FILE_LINE
#define LOG_PRO_TRACE_TEXT  LOG_PRO_TRACE_TEXT_NO_FILE_LINE
#endif

/*
 * magic: log frame magic, 0x7e6b6b7e.
 * @sn: the sn of the log that the protocol type is prot.
 * @length: the length of the all frame.
 * @sys: the log is printt by which sys.
 * @prot: the protocol type of the frame log.
 * @crc: optional. the crc of the data.
 * @link: optional. the link of the audio data.
 * @subpro: optional, the sub pro of the log.
 */
#ifdef WIN32
#pragma pack(push, 1)
typedef struct ALOG_FRAME_HEAD {
    u32     magic;
    u32     sn;
    u16     length;
    u8      sys;
    u8      prot;
    u16     crc;
    u8      link;
    u8      subpro;
} ALOG_FRAME_H_T;
#pragma pack(pop)
#else
typedef struct __attribute__((packed)) ALOG_FRAME_HEAD {
    u32     magic;
    u32     sn;
    u16     length;
    u8      sys;
    u8      prot;
    u16     crc;
    u8      link;
    u8      subpro;
} ALOG_FRAME_H_T;
#endif /* WIN32 */

#define ALOG_FRAME_H_SIZE     sizeof(ALOG_FRAME_H_T)
#define ALOG_SET_FRAME_HEAD(frame, sno, len, proto) \
    (frame)->magic = ALOG_FRAM_MAGIC; \
    (frame)->sys = AIC_THIS_SYS;\
    (frame)->sn = (sno); \
    (frame)->length = (len); \
    (frame)->prot = (proto); \
    (frame)->crc = 0; \
    (frame)->subpro = SUB_PRO_SAVE; \
    (frame)->link = INVALID_DATA_LINK

#define DATA_SET_FRAME_HEAD(frame, sno, len, proto, sum, lnk, sub) \
    (frame)->magic = ALOG_FRAM_MAGIC; \
    (frame)->sys = AIC_THIS_SYS;\
    (frame)->sn = (sno); \
    (frame)->length = (len); \
    (frame)->prot = (proto); \
    (frame)->crc = (sum); \
    (frame)->subpro = (sub); \
    (frame)->link = (lnk)

#define DATA_SET_FRAME_HEAD_EX(frame, sy, sno, len, proto, sum, lnk, sub) \
    (frame)->magic = ALOG_FRAM_MAGIC; \
    (frame)->sys = sy;\
    (frame)->sn = (sno); \
    (frame)->length = (len); \
    (frame)->prot = (proto); \
    (frame)->crc = (sum); \
    (frame)->subpro = (sub); \
    (frame)->link = (lnk)

#define DEF_DATA_LINK   0

#endif /* __ALOG_FRAME_H__ */

