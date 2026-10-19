/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines file system configurations.
 */

#ifndef __FS_USER_CONFIG_H__
#define __FS_USER_CONFIG_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include "vfs_cfg_type.h"

/**---------------------------------------------------------------------------*
 ** file system config
 **---------------------------------------------------------------------------*/
#define FS_DEFAULT_TARGET_NAME          "/C"

#define FS_DEFAULT_DIR_MAX_NAME_LEN     10
char s_fs_default_dir_cfg[][FS_DEFAULT_DIR_MAX_NAME_LEN] = {
    "Apps",
    "Images",
    "Audio",
    "Video",
    "System",
    "Data",
    "usr",
    "home"
};

FS_CFG s_fs_cfg[] = {
    {
        "/dev/ramblk",
        "fxffs",
        "/RAM",
        NULL,
        O_RDWR,
        1,
        1,
        1,
        NULL,
        6,
        NULL,
        1,
        1,
        32,
        0,
        256,
        512,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
    },
    {
        "/dev/win32blk",
        "fxffs",
        "/C",
        NULL,
        O_RDWR,
        1,
        1,
        1,
        NULL,
        6,
        NULL,
        1,
        1,
        32,
        0,
        4117,
        512,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
    },
    {
        "/dev/win32blk",
        "fxffs",
        "/D",
        NULL,
        O_RDWR,
        1,
        1,
        1,
        NULL,
        6,
        NULL,
        1,
        1,
        32,
        0,
        217,
        512,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
    },
    {
        "/dev/win32blk",
        "romfs",
        "/O",
        NULL,
        O_RDONLY,
        1,
        1,
        1,
        NULL,
        6,
        NULL,
        1,
        1,
        32,
        0,
        240,
        512,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
    }
};

/**---------------------------------------------------------------------------*/
#endif /* __FS_USER_CONFIG_H__ */

