/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * DESCRIPTION
 *
 * This file export the file system at interface.
 */

#ifndef __AIC_FS_AT_H__
#define __AIC_FS_AT_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_at_resp_handle.h>
#include <aic_at_cmd_handle.h>
#include <aic_list.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define FS_AT_REPLY_MAX_SIZE    (0x2000)
#define FS_AT_TIMEOUT           (5) /* unit:s */

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
typedef struct _fs_at_open_file_ {
    char                *name;
    int                 fd;
    int                 mode;
    struct list_head    node;
} FS_AT_OPEN_FILE;

typedef struct _fs_at_cmd_ {
    AT_REQ_RESP *at_req;
    char        *cmd;
    u64         file_num;
    u64         file_size;
    u8          *ptr;
    char        *target;
    boolean     prefix;
} FS_AT_CMD;

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * xor_16_check - 16BIT XOR check.
 *
 * Parameter:
 *  @buf: input check buf.
 *  @len: input buf len.
 *
 * Return:
 *  16BIT XOR check value.
 */
u16 xor_16_check(u8 *buf, u32 len);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_FS_AT_H__ */

