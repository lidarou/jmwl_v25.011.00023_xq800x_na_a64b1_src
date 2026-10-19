/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic block device interface.
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x106e7e
#define LOG_MODULE    AMOD_FS

#include "fmd.h"
#include "aic_lfs.h"
#include "aos_utl.h"
#include "lfs.h"
#include "sys/errno.h"


int lfs_aicbd_read(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size)
{
    int ret = -ENOTSUP;
    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)cfg->context;

    /**
     * Check if read is valid.
     * @note fs_info->part_info.sector_size == cfg->read_size
     */
    LFS_ASSERT(block < cfg->block_count);
    LFS_ASSERT(off  % cfg->read_size == 0);
    LFS_ASSERT(size % cfg->read_size == 0);
    LFS_ASSERT(off + size <= cfg->block_size);

    // ret = fmd_read(fs_info->part_info.devhandle, block * cfg->block_size + off, size, buffer);
    // AIC_ASSERT(ret == AIC_SUCCESS);

    if (fs_info->dev_ops->read) {
        ret = fs_info->dev_ops->read(&fs_info->part_info, block * cfg->block_size + off, size, buffer);
    }

    return aic_to_lfs_errcode(ret);
}

int lfs_aicbd_prog(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size)
{
    int ret = -ENOTSUP;
    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)cfg->context;

    /**
     * Check if write is valid.
     * @note fs_info->part_info.sector_size == cfg->prog_size
     */
    LFS_ASSERT(block < cfg->block_count);
    LFS_ASSERT(off  % cfg->prog_size == 0);
    LFS_ASSERT(size % cfg->prog_size == 0);
    LFS_ASSERT(off + size <= cfg->block_size);

    // ret = fmd_write(fs_info->part_info.devhandle, block * cfg->block_size + off, size, (u8 *)buffer);
    // AIC_ASSERT(ret == AIC_SUCCESS);

    if (fs_info->dev_ops->write) {
        ret = fs_info->dev_ops->write(&fs_info->part_info, block * cfg->block_size + off, size, buffer);
    }

    return aic_to_lfs_errcode(ret);
}

int lfs_aicbd_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    int ret = -ENOTSUP;
    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)cfg->context;

    /* Check if erase is valid. */
    LFS_ASSERT(block < cfg->block_count);

    if (fs_info->dev_ops->erase) {
        ret = fs_info->dev_ops->erase(&fs_info->part_info, block * cfg->block_size, cfg->block_size);
    }

    // ret = fmd_erase(fs_info->part_info.devhandle, block * cfg->block_size, cfg->block_size);
    // AIC_ASSERT(ret == AIC_SUCCESS);

    return aic_to_lfs_errcode(ret);
}

int lfs_aicbd_sync(const struct lfs_config *cfg)
{
    int ret = LFS_ERR_OK;
    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)cfg->context;

    if (fs_info->dev_ops->flush) {
        ret = fs_info->dev_ops->flush(&fs_info->part_info);
    }

    return aic_to_lfs_errcode(ret);
}
