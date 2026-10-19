/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic fs fmd device operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00fc9a
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <fmd.h>
#include <aic_module.h>
#include <aic_init.h>
#include <aic_math.h>
#include "vfs_dev.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define FS_FMD_SECTOR_SIZE  0x200

/**---------------------------------------------------------------------------*
 ** Static Global Variables
 **---------------------------------------------------------------------------*/
u32 s_fs_fmd_sector_size = FS_FMD_SECTOR_SIZE;
u8 *s_fs_fmd_cache_buf = NULL;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_fmd_open - open fs fmd device partition.
 *
 * Parameter:
 * @handle: output fmd device handle.
 * @name: input partition name.
 * @sector_size: input sector size.
 * @sector_num: input sector number.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_fmd_open(struct vfs_blkdev_partinfo *partinfo)
{
    char *tmp_name;
    u32 retsize;
    int ret;

    tmp_name = aos_zalloc(strlen(partinfo->devpath));
    aos_memcpy(tmp_name, partinfo->devpath + strlen("/dev/"),
                strlen(partinfo->devpath + strlen("/dev/")));
    ret = fmd_open(tmp_name, &partinfo->devhandle);
    AIC_ASSERT(ret == AIC_SUCCESS);

    ret = fmd_get_erase_size(partinfo->devhandle, &retsize);
    AIC_ASSERT(ret == AIC_SUCCESS);

    s_fs_fmd_sector_size = retsize;
    partinfo->sector_size = FS_FMD_SECTOR_SIZE;

    ret = fmd_get_partition_size(partinfo->devhandle, &retsize);
    AIC_ASSERT(ret == AIC_SUCCESS);

    partinfo->total_sectors = retsize / partinfo->sector_size;

    aos_free(tmp_name);

    return AIC_SUCCESS;
}

/**
 * fs_fmd_close - close fs fmd device partition.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_fmd_close(struct vfs_blkdev_partinfo *partinfo)
{
    int ret;

    ret = fmd_close(partinfo->devhandle);
    AIC_ASSERT(ret == AIC_SUCCESS);

    return AIC_SUCCESS;
}

/**
 * fs_fmd_read - read fs fmd device partition data.
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
int fs_fmd_read(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                u32 sector_num, void *buf)
{
    u32 fmd_read_offset, fmd_read_size, fmd_sector_offset, fmd_sector_size;
    u32 read_size;
    u8 *data;
    int ret;

    fmd_sector_size = s_fs_fmd_sector_size;
    read_size = sector_num * partinfo->sector_size;
    fmd_sector_offset = 0;

    if (partinfo->sector_size != s_fs_fmd_sector_size) {
        fmd_read_offset = ALIGN_DOWN(start_sector * partinfo->sector_size,
                                     fmd_sector_size);
        fmd_sector_offset = start_sector * partinfo->sector_size -
                            fmd_read_offset;
        fmd_read_size = ALIGN(fmd_sector_offset + read_size, fmd_sector_size);

        data = aos_zalloc(fmd_read_size);
    } else {
        fmd_read_offset = start_sector * partinfo->sector_size;
        fmd_read_size = sector_num * partinfo->sector_size;
        data = (u8 *)buf;
    }

    ret = fmd_read(partinfo->devhandle, fmd_read_offset, fmd_read_size, data);
    AIC_ASSERT(ret == AIC_SUCCESS);

    if (partinfo->sector_size != s_fs_fmd_sector_size) {
        aos_memcpy(buf, data + fmd_sector_offset, read_size);
        aos_free(data);
    }

    return AIC_SUCCESS;
}

/**
 * fs_fmd_write - write fs fmd device partition data.
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
int fs_fmd_write(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                 u32 sector_num, const void *buf)
{
    u32 fmd_write_offset, fmd_write_size, fmd_sector_offset, fmd_sector_size;
    u32 write_size;
    u8 *data;
    int ret;

    fmd_sector_size = s_fs_fmd_sector_size;
    write_size = sector_num * partinfo->sector_size;

    if (partinfo->sector_size != s_fs_fmd_sector_size) {
        fmd_write_offset = ALIGN_DOWN(start_sector * partinfo->sector_size,
                                      fmd_sector_size);
        fmd_sector_offset = start_sector * partinfo->sector_size -
                            fmd_write_offset;
        fmd_write_size = ALIGN(fmd_sector_offset + write_size, fmd_sector_size);

        data = aos_zalloc(fmd_write_size);
        AIC_ASSERT(data);

        ret = fmd_read(partinfo->devhandle, fmd_write_offset, fmd_write_size,
                       data);
        AIC_ASSERT(ret == AIC_SUCCESS);
        aos_memcpy(data + fmd_sector_offset, (u8 *)buf, write_size);
    } else {
        fmd_write_offset = start_sector * partinfo->sector_size;
        fmd_write_size = sector_num * partinfo->sector_size;
        data = (u8 *)buf;
    }

    ret = fmd_erase(partinfo->devhandle, fmd_write_offset, fmd_write_size);
    AIC_ASSERT(ret == AIC_SUCCESS);

    ret = fmd_write(partinfo->devhandle, fmd_write_offset, fmd_write_size,
                    data);
    AIC_ASSERT(ret == AIC_SUCCESS);

    if (partinfo->sector_size != s_fs_fmd_sector_size) {
        aos_free(data);
    }

    return AIC_SUCCESS;
}

/**
 * fs_fmd_abort - abort fs fmd device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_fmd_abort(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

/**
 * fs_fmd_release - release fs fmd device logic sector.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_fmd_release(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                   u32 sector_num)
{
    return AIC_SUCCESS;
}

/**
 * fs_fmd_flush - flush fs fmd device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_fmd_flush(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

struct vfs_blkdev_ops g_fs_fmd_ops = {
    .path = FS_FMD_DEV_PATH,
    .open = fs_fmd_open,
    .close = fs_fmd_close,
    .read = fs_fmd_read,
    .write = fs_fmd_write,
    .abort = fs_fmd_abort,
    .release = fs_fmd_release,
    .flush = fs_fmd_flush,
};

struct vfs_blkdev_ops g_fs_fmd_fatdev_ops = {
    .path = FS_FMD_FAT_DEV_PATH,
    .open = fs_fmd_open,
    .close = fs_fmd_close,
    .read = fs_fmd_read,
    .write = fs_fmd_write,
    .abort = fs_fmd_abort,
    .release = fs_fmd_release,
    .flush = fs_fmd_flush,
};

int fs_dev_fmd_init(void)
{
    int ret;

    ret = vfs_blkdev_register(&g_fs_fmd_ops, 0, NULL);
    ret |= vfs_blkdev_register(&g_fs_fmd_fatdev_ops, 0, NULL);

    return ret;
}

AIC_COMP_P1_INIT_EXPORT(fs_dev_fmd_init);

