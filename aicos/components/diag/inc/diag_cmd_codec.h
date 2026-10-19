/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This diag codec header file
 *
 */
#ifndef __DIAG_CMD_CODEC_H__
#define __DIAG_CMD_CODEC_H__

#include <aic_type.h>
#include <diag_cmd_api.h>

/* This magic means the diag cmd is passed between ap and cp. */
#define DIAG_MAGIC_2    0x7e5b5b7e

#define DIAG_MAGIC      0x7e5a5a7e
#define DIAG_MAGIC_CNT  4
#define DIAG_MAGIC0     0x7e
#define DIAG_MAGIC1     0x5a
#define DIAG_MAGIC2     0x5a
#define DIAG_MAGIC3     0x7e

#define DIAG_TAIL       0xed

#define DIAG_CMD_PACKET_LEN(cmd) (sizeof(*(cmd)) + (cmd)->data_len + 1)

/*
 * diag cmd format:
 * header(15 byte) + data(0~65520 byte) + tail(1 byte).
 * diag header:
 * magic:   4byte - magicmumber 0x7e5a5a7e.
 * sn:      4byte - the sn of this diag packet.
 * sys:     1byte - which sys is the cmd belong to.
 * err_no:  1byte - The error number of the cmd response. 0 - succ.
 * cmd:     1byte - the cmd.
 * subcmd:  2byte - the sub cmd.
 * data_len:2byte - the len of the diag data, except the header and the tail.
 */
#ifdef WIN32
#pragma pack(push, 1)
typedef struct DIAG_CMD_HEAD {
    u32     magic;
    u32     sn;
    u8      sys;
    u8      err_no;
    u8      cmd;
    u16     subcmd;
    u16     data_len;
} DIAG_CMD_HEAD_T;
#pragma pack(pop)
#else
typedef struct __attribute__((packed)) DIAG_CMD_HEAD {
    u32     magic;
    u32     sn;
    u8      sys;
    u8      err_no;
    u8      cmd;
    u16     subcmd;
    u16     data_len;
} DIAG_CMD_HEAD_T;
#endif

#define diag_recv_cmd_free(cmd) do { \
        if (cmd->data) \
            aos_free(cmd->data); \
        aos_free(cmd); \
    } while (0)

/**
 * diag_codec_create - Create a diag codec
 *
 * @return: the codec handle.
 */
void *diag_codec_create(void);

/**
 * diag_codec_destroy - destroy a diag codec
 *
 * @handle: the codec handle.
 */
void diag_codec_destroy(void *handle);

/**
 * diag_decode_cmd - Decode the diag cmd packet.
 *
 * @handle: the codec handle.
 * @data: the data will decode.
 * @size: the size of the data.
 * @decode: save the the number of the byte stream that has been decodeed,
 *          only valid when the return value is not null.
 *
 * @return: NULL, not finish the cmd decode this timer; otherwise,
 *          finish the cmd decode this time, reurn the diag cmd data pointer.
 */
diag_recv_cmd_t *diag_decode_cmd(void *handle, void *data, size_t size, size_t *decode);

/**
 * diag_encode_cmd - encode a diag cmd.
 *
 * @rsp_cmd: the response cmd. The more info see struct diag_rsp_cmd define.
 */
DIAG_CMD_HEAD_T *diag_encode_cmd(diag_rsp_cmd_t *rsp_cmd);

#endif /* __DIAG_CMD_CODEC_H__ */

