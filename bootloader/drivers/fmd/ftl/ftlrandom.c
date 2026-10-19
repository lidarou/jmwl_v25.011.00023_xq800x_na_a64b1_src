/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the ftl random read/write access driver. Based on
 *    flash translation layer.
 */

#include <aic_errno.h>
#include <aic_math.h>

#include "ftlcore.h"

int ftlrandom_open(char *name, u32 *handle)
{
    struct ftl_pdev *pdev = NULL;
    FTL_INT_SAVE

    FTL_INT_DISABLE

    pdev = ftl_get_pdev(name);
    if (!pdev) {
        FTL_INT_RESTORE
        return AIC_ENODEV;
    }

    pdev->ref_count++;
    *handle = (u32)pdev;

    FTL_INT_RESTORE

    return AIC_SUCCESS;
}

int ftlrandom_close(u32 handle)
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

int ftlrandom_read(u32 handle, u32 offset, u32 size, u8 *buf)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    struct ftl_dev *dev;
    UINT base_id, sector_id, status;
    u32 read_off, read_size;
    u32 sector_size, sector_mask;
    u8 *sector_cache;
    int ret = AIC_SUCCESS, i, factor;

    if (!pdev)
        return AIC_EOPNOTSUPP;

    if (!buf || !size)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !dev->lx_nor)
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    sector_size = dev->sector_size;
    sector_mask = sector_size - 1;

    sector_cache = malloc(sector_size);
    ASSERT_INFO(sector_cache != NULL, "[ftlrandom] malloc failed!\n");

    base_id = pdev->offset;

    factor = dev->sector_size / dev->lx_sector_size;

    if (offset & sector_mask) {
        sector_id = offset / sector_size;

        for (i = 0; i < factor; i++) {
            status = lx_nor_flash_sector_read(dev->lx_nor,
                                              (base_id + sector_id) * factor + i,
                                              (VOID *)sector_cache);
            ASSERT_INFO(status == LX_SUCCESS,
                            "[ftlrandom] read failed(%d)!\n", status);
        }

        read_off = offset & sector_mask;
        if (size <= (sector_size - read_off))
            read_size = size;
        else
            read_size = sector_size - read_off;

        memcpy(buf, sector_cache + read_off, read_size);

        size -= read_size;
        if (size > 0) {
            buf += read_size;
            offset += read_size;
        }
    }

    while (size > 0) {
        sector_id = offset / sector_size;

        if (size < sector_size) {
            for (i = 0; i < factor; i++) {
                status = lx_nor_flash_sector_read(dev->lx_nor,
                                                  (base_id + sector_id) * factor + i,
                                                  (VOID *)sector_cache);
                ASSERT_INFO(status == LX_SUCCESS,
                                "[ftlrandom] read failed(%d)!\n", status);
            }

            memcpy(buf, sector_cache, size);
            size = 0;

            break;
        }

        for (i = 0; i < factor; i++) {
            status = lx_nor_flash_sector_read(dev->lx_nor,
                                              (base_id + sector_id * factor + i),
                                              (VOID *)buf);
            ASSERT_INFO(status == LX_SUCCESS,
                            "[ftlrandom] read failed(%d)!\n", status);

            buf += dev->lx_sector_size;
        }

        offset += sector_size;
        size -= sector_size;
    }

    free(sector_cache);

    return ret;
}

int ftlrandom_write(u32 handle, u32 offset, u32 size, u8 *buf)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;
    struct ftl_dev *dev;
    UINT base_id, sector_id, status;
    u32 write_off, write_size;
    u32 sector_size, sector_mask;
    u8 *sector_cache;
    int ret = AIC_SUCCESS, i, factor;

    if (!pdev)
        return AIC_EOPNOTSUPP;

    if (!buf || !size)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !dev->lx_nor)
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    sector_size = dev->sector_size;
    sector_mask = sector_size - 1;

    sector_cache = malloc(sector_size);
    ASSERT_INFO(sector_cache != NULL, "[ftlrandom] malloc failed!\n");

    base_id = pdev->offset;

    factor = dev->sector_size / dev->lx_sector_size;

    if (offset & sector_mask) {
        sector_id = offset / sector_size;

        for (i = 0; i < factor; i++) {
            status = lx_nor_flash_sector_read(dev->lx_nor,
                                              (base_id + sector_id * factor + i),
                                              (VOID *)sector_cache);
            ASSERT_INFO(status == LX_SUCCESS,
                            "[ftlrandom] read failed(%d)!\n", status);
        }

        write_off = offset & sector_mask;
        if (size <= (sector_size - write_off))
            write_size = size;
        else
            write_size = sector_size - write_off;

        memcpy(sector_cache + write_off, buf, write_size);

        size -= write_size;
        if (size > 0) {
            buf += write_size;
            offset += write_size;
        }

        for (i = 0; i < factor; i++) {
            status = lx_nor_flash_sector_write(dev->lx_nor,
                                                (base_id + sector_id * factor + i),
                                                (VOID *)sector_cache);
            ASSERT_INFO(status == LX_SUCCESS,
                            "[ftlrandom] write failed(%d)!\n", status);
        }
    }

    while (size > 0) {
        sector_id = offset / sector_size;

        if (size < sector_size) {
            for (i = 0; i < factor; i++) {
                status = lx_nor_flash_sector_read(dev->lx_nor,
                                                  (base_id + sector_id * factor + i),
                                                  (VOID *)sector_cache);
                ASSERT_INFO(status == LX_SUCCESS,
                                "[ftlrandom] read failed(%d)!\n", status);
            }

            memcpy(sector_cache, buf, size);
            size = 0;

            for (i = 0; i < factor; i++) {
                status = lx_nor_flash_sector_write(dev->lx_nor,
                                                   (base_id + sector_id * factor + i),
                                                   (VOID *)sector_cache);
                ASSERT_INFO(status == LX_SUCCESS,
                                "[ftlrandom] write failed(%d)!\n", status);
            }

            break;
        }

        for (i = 0; i < factor; i++) {
            status = lx_nor_flash_sector_write(dev->lx_nor,
                                                (base_id + sector_id * factor + i),
                                                (VOID *)buf);
            ASSERT_INFO(status == LX_SUCCESS,
                            "[ftlrandom] write failed(%d)!\n", status);

            buf += dev->lx_sector_size;
        }

        offset += sector_size;
        size -= sector_size;
    }

    free(sector_cache);

    return ret;
}

int ftlrandom_get_sector_size(u32 handle, u32 *retsize)
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

int ftlrandom_get_sector_num(u32 handle, u32 *retnum)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retnum = pdev->sector_num;

    return AIC_SUCCESS;
}

int ftlrandom_get_partition_size(u32 handle, u32 *retsize)
{
    struct ftl_pdev *pdev = (struct ftl_pdev *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retsize = pdev->size;

    return AIC_SUCCESS;
}
