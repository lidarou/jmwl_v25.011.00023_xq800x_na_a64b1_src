/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic fs block device operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14f8c9
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <fmd.h>
#include <aic_module.h>
#include <aic_init.h>
#include <aic_math.h>
#include <aic_assert_def.h>
#include <flash_mem_config.h>
#include "vfs_dev.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Static Global Variables
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_bd_open - open fs block device partition.
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
int fs_bd_open(struct vfs_blkdev_partinfo *partinfo)
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

    partinfo->block_size = retsize;

    ret = fmd_get_partition_size(partinfo->devhandle, &retsize);
    AIC_ASSERT(ret == AIC_SUCCESS);

    partinfo->block_count = retsize / partinfo->block_size;

    aos_free(tmp_name);

    return AIC_SUCCESS;
}

/**
 * fs_fmd_close - close fs block device partition.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_bd_close(struct vfs_blkdev_partinfo *partinfo)
{
    int ret;

    ret = fmd_close(partinfo->devhandle);
    AIC_ASSERT(ret == AIC_SUCCESS);

    return AIC_SUCCESS;
}

/**
 * fs_fmd_read - read fs block device partition data.
 *
 * Parameter:
 * @handle: input device handle.
 * @offset: input block offset.
 * @size: input read size.
 * @buf: output buf.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_bd_read(struct vfs_blkdev_partinfo *partinfo, u32 offset,
                u32 size, void *buf)
{
    int ret;

    ret = fmd_read(partinfo->devhandle, offset, size, (u8 *)buf);
    AIC_ASSERT(ret == AIC_SUCCESS);
    return AIC_SUCCESS;
}

/**
 * fs_fmd_release - release fs block device.
 *
 * Parameter:
 * @handle: input device handle.
 * @off: input block offset.
 * @size: input erase size.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_bd_erase(struct vfs_blkdev_partinfo *partinfo, u32 offset,
                u32 size)
{
    int ret;

    ret = fmd_erase(partinfo->devhandle, offset, size);
    AIC_ASSERT(ret == AIC_SUCCESS);

    return AIC_SUCCESS;
}

/**
 * fs_fmd_write - write fs block device partition data.
 *
 * Parameter:
 * @handle: input device handle.
 * @off: input block offset.
 * @size: input write size.
 * @buf: input write buf.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_bd_write(struct vfs_blkdev_partinfo *partinfo, u32 offset,
                 u32 size, const void *buf)
{
    int ret;

    ret = fmd_write(partinfo->devhandle, offset, size, (u8 *)buf);
    AIC_ASSERT(ret == AIC_SUCCESS);

    return AIC_SUCCESS;
}

/**
 * fs_fmd_abort - abort fs block device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_bd_abort(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

/**
 * fs_fmd_release - release fs block device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_bd_release(struct vfs_blkdev_partinfo *partinfo, u32 offset,
                   u32 size)
{
    return AIC_SUCCESS;
}

/**
 * fs_fmd_flush - flush fs block device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_bd_flush(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

struct vfs_blkdev_ops g_fs_bd_ops = {
    .path = FS_BLOCK_DEV_PATH,
    .open = fs_bd_open,
    .close = fs_bd_close,
    .read = fs_bd_read,
    .erase = fs_bd_erase,
    .write = fs_bd_write,
    .abort = fs_bd_abort,
    .release = fs_bd_release,
    .flush = fs_bd_flush,
};

#ifdef FLASH1_E_BASE
struct vfs_blkdev_ops g_fs_exbd_ops = {
    .path = FS_EXBLOCK_DEV_PATH,
    .open = fs_bd_open,
    .close = fs_bd_close,
    .read = fs_bd_read,
    .erase = fs_bd_erase,
    .write = fs_bd_write,
    .abort = fs_bd_abort,
    .release = fs_bd_release,
    .flush = fs_bd_flush,
};
#endif

int fs_dev_bd_init(void)
{
    int ret;

    ret = vfs_blkdev_register(&g_fs_bd_ops, 0, NULL);
    AIC_ASSERT(ret == AIC_SUCCESS);

#ifdef FLASH1_E_BASE
    ret = vfs_blkdev_register(&g_fs_exbd_ops, 0, NULL);
    AIC_ASSERT(ret == AIC_SUCCESS);
#endif

    return ret;
}

AIC_COMP_P1_INIT_EXPORT(fs_dev_bd_init);

