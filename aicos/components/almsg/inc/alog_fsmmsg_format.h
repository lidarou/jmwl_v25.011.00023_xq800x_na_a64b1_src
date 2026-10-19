/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The fsm msg log formater header file.
 */
#ifndef __ALOG_FSMMSG_FORMATER_H__
#define __ALOG_FSMMSG_FORMATER_H__
#include <alog_msg_format.h>
#include <aic_osa.h>

void alog_fsmmsg_frame_format(u32 sn, u8 *buf, size_t size);
size_t alog_fsmmsg_frame_get_header_size(void);
const ALMF_OPS_T *alog_fsmmsg_get_formater_ops(void);

/**
 ************************************************************************
 *                        TT package                                    *
 *-----------------------------------------------------------------------
 *  | SEG HEAD  |  EXT_DATA_HEAD  |  LTE_STAND + PARAM + PEER  |
 ************************************************************************
 * SEGHEAD: | SEG HEAD[0](8) | SEG HEAD[1](8) |
 *-----------------------------------------------------------------------
 * EXTDATA: | SEGLEN(16) | IFFLAG(8) | IFVER(8) | MSGSN(16) |
 *-----------------------------------------------------------------------
 * LTE      |  MSGLEN (16) |
            | SRCTASKID(8) | SRCPROCID(8) | DSTTASKID(8) | DSTPROCID(8) |
 * STAND:   | SAPID(32)    | PRIMID(32)   | TIMESTAMP(32)|
 *-----------------------------------------------------------------------
 * PARAM:   | PARAMSIZE(16) |     PRAM DATA                             |
 *-----------------------------------------------------------------------
 * PEER:   | BUFSIZE(16) FREESIZE(16)|    BUF DATA    |    FREEDATA     |
 *-----------------------------------------------------------------------
 ************************************************************************
 */

#ifdef WIN32
#pragma pack(push, 1)

typedef struct {
    u8 aucSegHead[2];
} PF_EXT_SEGHEAD_ST;

typedef struct {
    u16 usSegLen; /* (total len - sizeof(PF_EXT_SEGHEAD_ST)) */
    u8  ucIfFlag; /* interface flag, in lte, it is always 0. */
    u8  ucIfVer; /* interface version. */
    u16 usMsgSn;
} PF_EXT_DATA_HEAD_LTE_ST;

/* LTE, Diag<-->EXT interface data struct. */
typedef struct {
    u16 usMsgLen; /* LTESTAND len, contains head and data. */
    u8  ucSrcTaskId;
    u8  ucSrcProcId;
    u8  ucDestTaskId;
    u8  ucDestProcId;
    u32 ulSapId;
    u32 ulPrimId;
    u32 ulTimeStamp;
} DIAG_EXT_DATA_HEAD_LTE_STAND_IF_ST;

#pragma pack(pop)
#else
typedef struct __attribute__((packed)) {
    u8 aucSegHead[2];
} PF_EXT_SEGHEAD_ST;

typedef struct __attribute__((packed)) {
    u16 usSegLen; /* (ulTotalDataLen - sizeof(PF_EXT_SEGHEAD_ST)) */
    u8  ucIfFlag; /* interface flag, in lte, it is always 0. */
    u8  ucIfVer; /* interface version. */
    u16 usMsgSn;
} PF_EXT_DATA_HEAD_LTE_ST;

/* LTE, Diag<-->EXT interface data struct. */
typedef struct __attribute__((packed)) {
    u16 usMsgLen; /* LTESTAND len, contains head and data. */
    u8  ucSrcTaskId;
    u8  ucSrcProcId;
    u8  ucDestTaskId;
    u8  ucDestProcId;
    u32 ulSapId;
    u32 ulPrimId;
    u32 ulTimeStamp;
} DIAG_EXT_DATA_HEAD_LTE_STAND_IF_ST;
#endif /* WIN32 */

/*
 * define the fixed part of fsm header.
 */
#define FSM_INTERFACE_FLAG      (0x00)
#define FSM_INTERFACE_VER_2     (0x02)
#define FSM_INTERFACE_VER_3     (0x03)

#define FSM_CUR_VER             FSM_INTERFACE_VER_2

#define FSM_SEG_HEAD_1          (0xF3)
#define FSM_SEG_HEAD_2          (0x64)

/*
 * The member xxProcId contains 2 parts:
 * bit[5, 7]: intance id.
 * bit[0, 4]: ProcId.
 */
#define INSTANCE_ID_SHIFT       (0x05)
#define INSTANCE_ID_MASK        (0xff << INSTANCE_ID_SHIFT)
#define FSM_GET_SRC_INSTANCE_ID(fsm) \
    ((FSM_SRC_INSTANCE_ID(fsm) << INSTANCE_ID_SHIFT) & INSTANCE_ID_MASK)
#define FSM_GET_DST_INSTANCE_ID(fsm) \
    ((FSM_DEST_INSTANCE_ID(fsm) << INSTANCE_ID_SHIFT) & INSTANCE_ID_MASK)

/*
 * define fsm header size.
 */
#define FSM_SEG_HEAD_SIZE       sizeof(PF_EXT_SEGHEAD_ST)
#define FSM_DATA_HEAD_SIZE      sizeof(PF_EXT_DATA_HEAD_LTE_ST)
#define FSM_LTE_HEAD_SIZE       sizeof(DIAG_EXT_DATA_HEAD_LTE_STAND_IF_ST)
#define FSM_SEG_DATA_HEAD_SIZE  (FSM_SEG_HEAD_SIZE + FSM_DATA_HEAD_SIZE)
#define FSM_MSG_HEAD_SIZE       (FSM_SEG_DATA_HEAD_SIZE + FSM_LTE_HEAD_SIZE)

/*
 * FSM_ALL_DATA_SIZE - total data len, contain PARAM data, PEER data.
 */
#define FSM_ALL_DATA_SIZE(msg) \
    (FSM_PARAM_SIZE((aos_fsm_msg *)msg) + FSM_PEER_BUF_SIZE((aos_fsm_msg *)msg))

/*
 * define PARAM LEN, PEER BUF LEN, PEER FREE LEN
 */
#define FSM_PARAM_LEN   sizeof(u16)
#define PEER_BUF_LEN    sizeof(u16)
#define PEER_FREE_LEN   sizeof(u16)
#define FSM_PEER_LEN    (PEER_BUF_LEN + PEER_FREE_LEN)

/*
 * FSM_ALL_DATA_LEN - total data len, contain PARAM LEN, PEER BUF and PEER FREE LEN.
 */
#define FSM_ALL_DATA_LEN    (FSM_PARAM_LEN + FSM_PEER_LEN)

/*
 * FSM param data offset.
 */
#define FSM_MSG_PARAM_OFFSET    (FSM_MSG_HEAD_SIZE + FSM_PARAM_LEN)

/*
 * FSM peer data offset.
 */
#define FSM_MSG_PEER_OFFSET(msg) \
    (FSM_MSG_PARAM_OFFSET + FSM_PARAM_SIZE((msg)) + FSM_PEER_LEN)

void alog_fsmmsg_format_header(aos_fsm_msg *fsmmsg, u8 *buf, u8 pad, size_t size);

#endif /* __ALOG_FSMMSG_FORMATER_H__ */

