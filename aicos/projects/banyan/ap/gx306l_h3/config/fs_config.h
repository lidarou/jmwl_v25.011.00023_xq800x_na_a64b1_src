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
    "usr"    //chengjun
};

FS_CFG s_fs_cfg[] = {
    {
        .devpath = FS_BLOCK_DEV_PATH,
        .fstype = FS_LFS_TYPE_NAME,
        .target = "/C",
        .flags = O_RDWR,
        .format_eb = 1,
        .bytes_per_sector = 256,
        .create_default_dir = 1,  //chengjun
    },
#ifdef CONFIG_SDMMC_SD
    {
        .devpath = FS_SDMMC_DEV_PATH,
        .fstype = FS_FXFAT_TYPE_NAME,
        .target = "/D",
        .flags = O_RDWR,
        .memory_sectors = 1,
        .number_of_fats = 0x2,
        .directory_entries = 0x200,
        .hidden_sectors = 0x0,
        .total_sectors = 0x3B72400,
        .bytes_per_sector = 0x200,
        .sectors_per_cluster = 0x20,
        .heads = 0xFF,
        .sectors_per_track = 0x3F,
    },
#endif
#ifdef CONFIG_FS_ROMFS
    {
        .devpath = FS_FILE_DEV_PATH,
        .fstype = FS_ROMFS_TYPE_NAME,
        .target = "/O",
        .filepath = "/C/usr/share/firmware/romfs.bin",
        .flags = O_RDONLY,
        .memory_sectors = 2,
        .bytes_per_sector = 0x200,
    }
#endif
};

/**---------------------------------------------------------------------------*/
#endif /* __FS_USER_CONFIG_H__ */

