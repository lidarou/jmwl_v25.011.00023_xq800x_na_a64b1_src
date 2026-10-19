/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic fs ram device operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f754f
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_init.h>
#include "vfs_dev.h"
#include "vfs.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define FS_RAM_SECTOR_SIZE      512
#define FS_RAM_SECTOR_NUMBER    256
#define FS_RAM_TOTAL_SIZE       (FS_RAM_SECTOR_SIZE * FS_RAM_SECTOR_NUMBER)

/**---------------------------------------------------------------------------*
 ** Static Global Variables
 **---------------------------------------------------------------------------*/
u8 s_fs_ram_disk_memory[FS_RAM_TOTAL_SIZE];

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_ram_open - open fs ram device partition.
 *
 * Parameter:
 * @handle: output ram device handle.
 * @name: input partition name.
 * @sector_size: input sector size.
 * @sector_num: input sector number.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ram_open(struct vfs_blkdev_partinfo *partinfo)
{
    char *name;

    name = strstr(partinfo->devpath, "dev/");
    if (!name)
        return AIC_EINVAL;
    name += strlen("dev/");

    AOS_ASSERT((partinfo->sector_size == FS_RAM_SECTOR_SIZE) &&
                (partinfo->total_sectors == FS_RAM_SECTOR_NUMBER));

    partinfo->address = s_fs_ram_disk_memory;
    partinfo->sector_size = FS_RAM_SECTOR_SIZE;
    partinfo->total_sectors = FS_RAM_SECTOR_NUMBER;

    return AIC_SUCCESS;
}

/**
 * fs_ram_close - close fs ram device partition.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ram_close(struct vfs_blkdev_partinfo *partinfo)
{
    /* don`t free, format will be close and data is lost if free. */
    //aos_free(s_fs_ram_handle);
    //aos_free(s_fs_ram_handle->address);
    //s_fs_ram_handle = NULL;
    return AIC_SUCCESS;
}

/**
 * fs_ram_read - read fs ram device partition data.
 *
 * Parameter:
 * @handle: input device handle.
 * @logic_sector: input logic sector.
 * @sector_num: input sector number.
 * @buf: output buf.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ram_read(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                u32 sector_num, void *buf)
{
    aos_memcpy((u8 *)buf, partinfo->address +
               start_sector * partinfo->sector_size,
               sector_num * partinfo->sector_size);
    return AIC_SUCCESS;
}

/**
 * fs_ram_write - write fs ram device partition data.
 *
 * Parameter:
 * @handle: input device handle.
 * @logic_sector: input logic sector.
 * @sector_num: input sector number.
 * @buf: input buf.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ram_write(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                 u32 sector_num, const void *buf)
{
    aos_memcpy(partinfo->address +
               start_sector * partinfo->sector_size, (const u8 *)buf,
               sector_num * partinfo->sector_size);

    return AIC_SUCCESS;
}

/**
 * fs_ram_abort - abort fs ram device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ram_abort(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

/**
 * fs_ram_release - release fs ram device logic sector.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ram_release(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                   u32 sector_num)
{
    return AIC_SUCCESS;
}

/**
 * fs_ram_flush - flush fs ram device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ram_flush(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

struct vfs_blkdev_ops s_fs_dev_ram_ops = {
    FS_RAM_DEV_PATH,
    fs_ram_open,
    fs_ram_close,
    fs_ram_read,
    NULL,
    fs_ram_write,
    fs_ram_abort,
    fs_ram_release,
    fs_ram_flush,
};

int fs_dev_ram_init(void)
{
    int ret;

    ret = vfs_blkdev_register(&s_fs_dev_ram_ops, 0, NULL);

    return ret;
}

AIC_COMP_P1_INIT_EXPORT(fs_dev_ram_init);

