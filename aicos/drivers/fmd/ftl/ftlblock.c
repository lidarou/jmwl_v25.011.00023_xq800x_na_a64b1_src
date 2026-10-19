/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the flash translation layer driver.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x183709
#define LOG_MODULE    AMOD_FLASH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>

#include "ftlcore.h"

extern aos_mutex_ptr s_ftl_mutex;

int ftlblock_open(char *name, u32 *handle)
{
    struct ftl_pdev *pdev = NULL;
    FTL_INT_SAVE

    pdev = ftl_get_pdev(name);
    if (!pdev)
        return AIC_ENODEV;

    FTL_INT_DISABLE

    pdev->ref_count++;
    *handle = (u32)pdev;

    FTL_INT_RESTORE

    return AIC_SUCCESS;
}

int ftlblock_close(u32 handle)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    FTL_INT_SAVE

    if (!pdev)
        return AIC_EINVAL;

    FTL_INT_DISABLE

    pdev->ref_count--;

    FTL_INT_RESTORE

    return AIC_SUCCESS;
}

int ftlblock_read(u32 handle, u32 sector_id, u32 sector_num, u8 *buf)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    struct ftl_dev *dev;
    UINT off_id, status;
    u8 *ptr = buf;
    int i, j, factor;

    if (!pdev)
        return AIC_EOPNOTSUPP;

    if (!buf)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->lx_nor))
        return AIC_EOPNOTSUPP;

    if (sector_id >= pdev->sector_num ||
        sector_num > (pdev->sector_num - sector_id))
        return AIC_EINVAL;

    off_id = pdev->offset + sector_id;

    factor = dev->sector_size / dev->lx_sector_size;

    for (i = 0; i < sector_num; i++) {
        for (j = 0; j < factor; j++) {
            status = lx_nor_flash_sector_read(dev->lx_nor,
                                                (off_id + i) * factor + j,
                                                (VOID *)ptr);
            FTL_ASSERT(status == LX_SUCCESS);

            ptr += dev->lx_sector_size;
        }
    }

    return AIC_SUCCESS;
}

int ftlblock_write(u32 handle, u32 sector_id, u32 sector_num, u8 *buf)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    struct ftl_dev *dev;
    UINT off_id, status;
    u8 *ptr = buf;
    int i, j, factor;

    if (!pdev)
        return AIC_EOPNOTSUPP;

    if (!buf)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->lx_nor))
        return AIC_EOPNOTSUPP;

    if (sector_id >= pdev->sector_num ||
        sector_num > (pdev->sector_num - sector_id))
        return AIC_EINVAL;

    off_id = pdev->offset + sector_id;

    factor = dev->sector_size / dev->lx_sector_size;

    for (i = 0; i < sector_num; i++) {
        for (j = 0; j < factor; j++) {
            status = lx_nor_flash_sector_write(dev->lx_nor,
                                                (off_id + i) * factor + j,
                                                (VOID *)ptr);
            FTL_ASSERT(status == LX_SUCCESS);

            ptr += dev->lx_sector_size;
        }
    }

    return AIC_SUCCESS;
}

int ftlblock_release(u32 handle, u32 sector_id, u32 sector_num)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    struct ftl_dev *dev;
    UINT off_id, status;
    int i, j, factor;

    if (!pdev)
        return AIC_EOPNOTSUPP;

    dev = pdev->parent;
    if (!dev || !(dev->lx_nor))
        return AIC_EOPNOTSUPP;

    if (sector_id >= pdev->sector_num ||
        sector_num > (pdev->sector_num - sector_id))
        return AIC_EINVAL;

    off_id = pdev->offset + sector_id;

    factor = dev->sector_size / dev->lx_sector_size;

    for (i = 0; i < sector_num; i++) {
        for (j = 0; j < factor; j++) {
            status = lx_nor_flash_sector_release(dev->lx_nor,
                                                 (off_id + i) * factor + j);
            FTL_ASSERT(status != LX_ERROR);
        }
    }

    return AIC_SUCCESS;
}

int ftlblock_defragment(u32 handle, u32 sector_num)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    struct ftl_dev *dev;
    UINT status;
    int factor;
    u32 lx_sector_num, sectors_per_block, block_num;

    if (!pdev)
        return AIC_EOPNOTSUPP;

    dev = pdev->parent;
    if (!dev || !(dev->lx_nor))
        return AIC_EOPNOTSUPP;

    if (sector_num > pdev->sector_num)
        return AIC_EINVAL;

    factor = dev->sector_size / dev->lx_sector_size;
    lx_sector_num = sector_num * factor;

    sectors_per_block = lx_nor_flash_get_sectors_per_block(dev->lx_nor);
    block_num = (lx_sector_num + sectors_per_block) / sectors_per_block;

    status = lx_nor_flash_partial_defragment(dev->lx_nor, block_num);
    FTL_ASSERT(status == LX_SUCCESS);

    return AIC_SUCCESS;
}

int ftlblock_get_sector_size(u32 handle, u32 *retsize)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    struct ftl_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev)
        return AIC_EINVAL;

    *retsize = dev->sector_size;

    return AIC_SUCCESS;
}

int ftlblock_get_sector_num(u32 handle, u32 *retnum)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retnum = pdev->sector_num;

    return AIC_SUCCESS;
}

int ftlblock_get_partition_size(u32 handle, u32 *retsize)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retsize = pdev->size;

    return AIC_SUCCESS;
}

int ftlblock_get_erase_count(u32 handle, u32 *reterasecount)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    struct ftl_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->lx_nor))
        return AIC_EINVAL;

    *reterasecount = lx_nor_flash_get_erase_count(dev->lx_nor);

    return AIC_SUCCESS;
}
