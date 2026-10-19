/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the flash management device driver.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16bcfd
#define LOG_MODULE    AMOD_FLASH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_math.h>
#include <aic_osa.h>
#include "fmdcore.h"
#include "fmdpart.h"

static LIST_HEAD(s_fmd_dev_list);

int fmd_device_register(struct fmd_dev *dev)
{
    struct fmd_pdev *pdev;
    struct fmd_part part;
    u8 devno = dev->devno;
    u32 nr_parts, nr_groups, cur_offset = 0, lock_region = 0;
    int i, ret = 0;

    /* dev list node init */
    list_head_init(&(dev->head));
    list_head_init(&(dev->node));

    /* add dev node to fmd dev list */
    list_add_tail(&dev->node, &(s_fmd_dev_list));

    /* get fmd partition number of this master */
    nr_parts = fmd_get_num_of_parts(devno);
    if (nr_parts == 1) {
        /* if thera is only one partition in the partition table, the partition
         * size is automatically adjusted to the actual size of the flash.
         */
        ret = fmd_parse_partitions(dev, 0, &part);
        FMD_ASSERT(ret == AIC_SUCCESS);

        part.size = dev->size;

        pdev = fmd_allocate_partition(dev, &part, 0);
        FMD_ASSERT(pdev != NULL);

        list_add_tail(&pdev->node, &(dev->head));
    } else {
        for (i = 0; i < nr_parts; i++) {
            ret = fmd_parse_partitions(dev, i, &part);
            FMD_ASSERT(ret == AIC_SUCCESS);

            pdev = fmd_allocate_partition(dev, &part, cur_offset);
            FMD_ASSERT(pdev != NULL);

            list_add_tail(&pdev->node, &(dev->head));

            cur_offset = pdev->size + pdev->offset;
        }

        nr_groups = fmd_get_num_of_groups(devno);
        for (i = 0; i < nr_groups; i++) {
            pdev = fmd_allocate_group(dev, i);
            FMD_ASSERT(pdev != NULL);

            list_add_tail(&pdev->node, &(dev->head));
        }
    }

    if (dev->_lock) {
        /* Find the continuous "RO" partition and lock it */
        list_for_each_entry(pdev, &(dev->head), node, __typeof(*pdev)) {
            if (pdev && (pdev->property != FMD_READONLY))
                break;

            lock_region = pdev->size + pdev->offset;
        }

        if (lock_region > 0) {
            ret = dev->_lock(dev, 0, lock_region);
            FMD_ASSERT(ret == AIC_SUCCESS);
        }
    }

    return ret;
}

struct fmd_pdev *fmd_get_pdev_by_name(char *name)
{
    struct fmd_dev *dev = NULL;
    struct fmd_pdev *pdev = NULL;

    list_for_each_entry(dev, &s_fmd_dev_list, node, __typeof(*dev)) {
        if (dev) {
            list_for_each_entry(pdev, &(dev->head), node, __typeof(*pdev)) {
                if (pdev && (strcmp(pdev->name, name) == 0))
                    return pdev;
            }
        }
    }

    return NULL;
}

int fmd_open(char *name, u32 *handle)
{
    struct fmd_pdev *pdev = NULL;
    FMD_INT_SAVE

    pdev = fmd_get_pdev_by_name(name);
    if (!pdev)
        return AIC_ENODEV;

    FMD_INT_DISABLE

    pdev->ref_count++;
    *handle = (u32)pdev;

    FMD_INT_RESTORE

    return AIC_SUCCESS;
}

int fmd_close(u32 handle)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    FMD_INT_SAVE

    if (!pdev)
        return AIC_EINVAL;

    FMD_INT_DISABLE

    pdev->ref_count--;

    FMD_INT_RESTORE

    return AIC_SUCCESS;
}

int fmd_read(u32 handle, u32 offset, u32 size, u8 *buf)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev || !buf)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_read))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    dev->_read(dev, (pdev->offset + offset), size, buf);

    return AIC_SUCCESS;
}

int fmd_write(u32 handle, u32 offset, u32 size, u8 *buf)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev || !buf)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_write))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    dev->_write(dev, (pdev->offset + offset), size, buf);

    return AIC_SUCCESS;
}

int fmd_erase(u32 handle, u32 offset, u32 size)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_erase))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    dev->_erase(dev, (pdev->offset + offset), size);

    return AIC_SUCCESS;
}

int fmd_lock(u32 handle, u32 offset, u32 size)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_lock))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    return dev->_lock(dev, (pdev->offset + offset), size);
}

int fmd_unlock(u32 handle, u32 offset, u32 size)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_unlock))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    return dev->_unlock(dev, (pdev->offset + offset), size);
}

boolean fmd_is_locked(u32 handle, u32 offset, u32 size)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_is_locked))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    return dev->_is_locked(dev, (pdev->offset + offset), size);
}

int fmd_get_physical_address(u32 handle, u32 offset, u32 *retaddr)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;
    u32 base;

    if (!pdev || offset >= pdev->size)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_get_mem_base))
        return AIC_EOPNOTSUPP;

    base = dev->_get_mem_base(dev);

    *retaddr = base + (pdev->offset + offset);

    return AIC_SUCCESS;
}

int fmd_get_partition_size(u32 handle, u32 *retsize)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retsize = pdev->size;

    return AIC_SUCCESS;
}

int fmd_get_erase_size(u32 handle, u32 *retsize)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev)
        return AIC_EOPNOTSUPP;

    *retsize = dev->erasesize;

    return AIC_SUCCESS;
}

int fmd_get_device_type(u32 handle, u32 *rettype)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev)
        return AIC_EOPNOTSUPP;

    *rettype = dev->type;

    return AIC_SUCCESS;
}
