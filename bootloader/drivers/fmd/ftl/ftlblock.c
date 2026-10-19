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

#include <aic_errno.h>
#include <aic_type.h>

#include "ftlcore.h"

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
            ASSERT_INFO(status == LX_SUCCESS,
                            "[ftlblock] read failed(%d)!\n", status);

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
            ASSERT_INFO(status == LX_SUCCESS,
                            "[ftlblock] write failed(%d)!\n", status);

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
            ASSERT_INFO(status != LX_ERROR,
                            "[ftlblock] release failed(%d)!\n", status);
        }
    }

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
