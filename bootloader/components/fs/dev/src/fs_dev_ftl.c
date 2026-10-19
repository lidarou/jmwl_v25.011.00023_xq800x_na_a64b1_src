/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic fs ftl device operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x144fc9

#include <aic_errno.h>
#include <ftlblock.h>
#include "initcall.h"
#include "vfs_dev.h"
#include "vfs.h"

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
 * fs_ftl_open - open fs ftl device partition.
 *
 * Parameter:
 * @handle: output ftl device handle.
 * @name: input partition name.
 * @sector_size: input sector size.
 * @sector_num: input sector number.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ftl_open(struct vfs_blkdev_partinfo *partinfo)
{
    u32 retsize, retnum;
    char *name;
    int ret;

    name = strstr(partinfo->devpath, "dev/");
    if (!name)
        return AIC_EINVAL;
    name += strlen("dev/");

    ret = ftlblock_open(name, &partinfo->devhandle);
    if (ret != AIC_SUCCESS) {
        log_print("fs ftl dev open %s fail!\n", partinfo->devpath);
        return ret;
    }

    ret = ftlblock_get_sector_size(partinfo->devhandle, &retsize);
    ASSERT_INFO(ret == AIC_SUCCESS,
                "fs ftl dev sector_size(0x%x) ret(%d) err!\n",
                retsize, ret);
    partinfo->sector_size = retsize;

    ret = ftlblock_get_sector_num(partinfo->devhandle, &retnum);
    ASSERT_INFO(ret == AIC_SUCCESS,
                "fs ftl dev sector_num(0x%x) ret(%d) err!\n",
                retnum, ret);

    partinfo->total_sectors = retnum;

    return AIC_SUCCESS;
}

/**
 * fs_ftl_close - close fs ftl device partition.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ftl_close(struct vfs_blkdev_partinfo *partinfo)
{
    int ret;

    ret = ftlblock_close(partinfo->devhandle);
    ASSERT_INFO(ret == AIC_SUCCESS, "fs ftl dev close ret(%d) err!\n", ret);

    return AIC_SUCCESS;
}

/**
 * fs_ftl_read - read fs ftl device partition data.
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
int fs_ftl_read(struct vfs_blkdev_partinfo *partinfo, u32 logic_sector,
                u32 sector_num, u8 *buf)
{
    int ret;

    ret = ftlblock_read(partinfo->devhandle, logic_sector, sector_num,
                        buf);
    ASSERT_INFO(ret == AIC_SUCCESS,
                "fs ftl dev read logic_sector(%x) sec_num(0x%x) ret(%d) err!\n",
                logic_sector, sector_num, ret);

    return AIC_SUCCESS;
}

/**
 * fs_ftl_write - write fs ftl device partition data.
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
int fs_ftl_write(struct vfs_blkdev_partinfo *partinfo, u32 logic_sector,
                 u32 sector_num, u8 *buf)
{
    int ret;

    ret = ftlblock_write(partinfo->devhandle, logic_sector, sector_num, buf);
    ASSERT_INFO(ret == AIC_SUCCESS,
                "fs ftl dev write logic_sector(%x) sec_num(0x%x) ret(%d) err!\n",
                logic_sector, sector_num, ret);

    return AIC_SUCCESS;

}

/**
 * fs_ftl_abort - abort fs ftl device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ftl_abort(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

/**
 * fs_ftl_release - release fs ftl device logic sector.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ftl_release(struct vfs_blkdev_partinfo *partinfo, u32 logic_sector,
                   u32 sector_num)
{
    int ret;

    ret = ftlblock_release(partinfo->devhandle, logic_sector, 1);
    ASSERT_INFO(ret == AIC_SUCCESS,
                "fs ftl release logic_sector(%x) ret(%d) err!\n",
                logic_sector, ret);

    return AIC_SUCCESS;
}

/**
 * fs_ftl_flush - flush fs ftl device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_ftl_flush(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

struct vfs_blkdev_ops g_fs_ftl_ops = {
    .path = FS_FTL_DEV_PATH,
    .open = fs_ftl_open,
    .close = fs_ftl_close,
    .read = fs_ftl_read,
    .write = fs_ftl_write,
    .abort = fs_ftl_abort,
    .release = fs_ftl_release,
    .flush = fs_ftl_flush,
};

int fs_dev_ftl_init(void)
{
    int ret;

    ret = vfs_blkdev_register(&g_fs_ftl_ops, 0, NULL);
    if (ret != AIC_SUCCESS)
    {
        log_print("fs register ftl blockdriver failed: %d\n", ret);
    }

    return ret;
}

