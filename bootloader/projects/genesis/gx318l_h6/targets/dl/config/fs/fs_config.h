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

#define FS_DEFAULT_DIR_MAX_NAME_LEN     30
char s_fs_default_dir_cfg[][FS_DEFAULT_DIR_MAX_NAME_LEN] = {
    "usr/share/firmware"
};

FS_CFG s_fs_cfg[] = {
    {
        .devpath = FS_FTL_DEV_PATH,
        .fstype = FS_FXFAT_TYPE_NAME,
        .target = "/C",
        .flags = O_RDWR,
        .format_eb = 1,
        .check_eb = 1,
        .fault_tolerant_eb = 1,
        .fault_tolerant_sectors = 6,
        .memory_sectors = 256,
        .number_of_fats = 1,
        .directory_entries = 32,
        .hidden_sectors = 0,
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .heads = 1,
        .sectors_per_track = 1,
    }
};

/**---------------------------------------------------------------------------*/
#endif /* __FS_USER_CONFIG_H__ */

