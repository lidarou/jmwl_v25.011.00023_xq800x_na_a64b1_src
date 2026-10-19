/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the fs device config struct.
 */

#ifndef __AIC_VFS_CFG_TYPE_H__
#define __AIC_VFS_CFG_TYPE_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/
#define FS_FXFAT_TYPE_NAME          "fxffs"
#define FS_FXEXFAT_TYPE_NAME        "fxexffs"
#define FS_ROMFS_TYPE_NAME          "romfs"
#define FS_LFS_TYPE_NAME            "littlefs"

#define FS_SDMMC_DEV_PATH           "/dev/sdmmc"
#define FS_FMD_DEV_PATH             "/dev/romfs"
#define FS_FMD_FAT_DEV_PATH         "/dev/E"
#define FS_BLOCK_DEV_PATH           "/dev/C"
#define FS_EXBLOCK_DEV_PATH         "/dev/E"
#define FS_FTL_DEV_PATH             "/dev/C"
#define FS_EXFTL_DEV_PATH           "/dev/E"
#define FS_RAM_DEV_PATH             "/dev/ramblk"
#define FS_WIN32_DEV_PATH           "/dev/win32blk"
#define FS_FILE_DEV_PATH            "/dev/file"

/**---------------------------------------------------------------------------*
 ** Public Types
 **---------------------------------------------------------------------------*/
/**
 * fs config.
 *
 * @devpath: mount fs device path.
 * @fstype: mount fs type.
 * @target: mount fs mount target.
 * @filepath: if fs mount device path is FS_FILE_DEV_PATH, must be set filepath.
 * @flags: mount flags.
 * @fault_tolerant_eb: enable fault toleranet.
 * @fault_tolerant_memory: fault tolerant memory.
 * @fault_tolerant_sectors: fault tolerant sectors.
 * @memory: cache memory.
 * @memory_sectors: cache memory sectors.
 * @number_of_fats: fats number.
 * @directory_entries: directory entries.
 * @hidden_sectors: hidden sectors.
 * @total_sectors: total sectors.
 * @bytes_per_sector: bytes per sectir.
 * @sectors_per_cluster: sectors per cluster.
 * @heads: heads.
 * @sectors_per_track: sectors per track.
 * @volume_serial_number: volume serial number, use for exfat.
 * @boundary_unit: boundary unit, use for exfat.
 * @block_size: block size.
 * @block_count: block count.
 */
typedef struct fs_cfg {
    char *devpath;
    char *fstype;
    char *target;
    char *filepath;
    u32 flags;
    u8 format_eb;
    u8 check_eb;
    u32 fault_tolerant_eb;
    u8 *fault_tolerant_memory;
    u32 fault_tolerant_sectors;
    u8 *memory;
    u32 memory_sectors;
    u32 number_of_fats;
    u32 directory_entries;
    u32 hidden_sectors;
    ulong total_sectors;
    u32 bytes_per_sector;
    u32 sectors_per_cluster;
    u32 heads;
    u32 sectors_per_track;
    u32 volume_serial_number;
    u32 boundary_unit;
    u32 block_size;
    u32 block_count;
    boolean create_default_dir;
} FS_CFG;

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_VFS_CFG_TYPE_H__ */

