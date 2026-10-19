/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement sdmmc dev function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05f2b0
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <vfs_dev.h>
#include <aic_init.h>
#include <sdmmc_dev.h>

static char *sd_volume_name = NULL;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_sdmmc_open - open fs sd device partition.
 *
 * Parameter:
 * @handle: output sd device handle.
 * @name: input partition name.
 * @sector_size: input sector size.
 * @sector_num: input sector number.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_sdmmc_open(struct vfs_blkdev_partinfo *partinfo)
{
    int ret;
    struct sdmmc_dev_st *sdmmc_dev_handle = NULL;

    AIC_ASSERT(partinfo);

    sd_volume_name = partinfo->devpath;

    ret = sdmmc_open(sd_volume_name, (u32 *)&sdmmc_dev_handle);
    if (ret)
        return ret;

    partinfo->devhandle = (u32)sdmmc_dev_handle;
    partinfo->sector_size = sdmmc_dev_handle->sector_size;
    partinfo->total_sectors = sdmmc_dev_handle->sector_num;

    return AIC_SUCCESS;

}

/**
 * fs_sdmmc_close - close fs sd device partition.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_sdmmc_close(struct vfs_blkdev_partinfo *partinfo)
{
    sdmmc_close(partinfo->devhandle);
    return AIC_SUCCESS;
}

/**
 * fs_sdmmc_read - read fs sd device partition data.
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
int fs_sdmmc_read(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                       u32 sector_num, void *buf)
{
    return sdmmc_read(partinfo->devhandle, start_sector, sector_num, (u8 *)buf);
}

/**
 * fs_sdmmc_write - write fs sd device partition data.
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
int fs_sdmmc_write(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                u32 sector_num, const void *buf)
{
    return sdmmc_write(partinfo->devhandle, start_sector, sector_num, (u8 *)buf);
}

/**
 * fs_sdmmc_abort - abort fs sd device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_sdmmc_abort(struct vfs_blkdev_partinfo *partinfo)
{
    fs_sdmmc_close(partinfo);
    return AIC_SUCCESS;
}

/**
 * fs_sdmmc_release - release fs sd device logic sector.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_sdmmc_release(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                   u32 sector_num)
{
    return AIC_SUCCESS;
}

/**
 * fs_sdmmc_flush - flush fs sd device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_sdmmc_flush(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

struct vfs_blkdev_ops s_fs_dev_sdmmc_ops = {
    .path = FS_SDMMC_DEV_PATH,
    .open = fs_sdmmc_open,
    .close = fs_sdmmc_close,
    .read = fs_sdmmc_read,
    .write = fs_sdmmc_write,
    .abort = fs_sdmmc_abort,
    .release = fs_sdmmc_release,
    .flush = fs_sdmmc_flush,
};

int fs_dev_sdmmc_init(void)
{
    int ret;

    ret = vfs_blkdev_register(&s_fs_dev_sdmmc_ops, 0, NULL);

    return ret;
}

AIC_COMP_P1_INIT_EXPORT(fs_dev_sdmmc_init);


int32_t fs_dev_sdmmc_add()
{
    int32_t err = 0;

    if (sd_volume_name) {
        err = s_fs_dev_sdmmc_ops.mount(sd_volume_name);
    }

    return err;
}

int32_t fs_dev_sdmmc_remove()
{
    int32_t err = 0;

    if (sd_volume_name) {
        err = s_fs_dev_sdmmc_ops.unmount(sd_volume_name);
    }

    return err;
}

