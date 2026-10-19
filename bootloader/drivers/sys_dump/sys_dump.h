/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines sys dump related infomations.
 */

#ifndef __SYS_DUMP_H__
#define __SYS_DUMP_H__

#include <aic_type.h>

enum sys_dump_type {
    DUMP_MEM = 0,
    DUMP_REG
};

struct sys_dump_head {
    u32 magic;
    enum sys_dump_type type;
    u32 address;
    u32 length;
};

struct sys_dump_cfg_info {
    enum sys_dump_type type;
    u32 address;
    u32 length;
};

#endif /* __SYS_DUMP_H__ */
