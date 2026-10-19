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

#include <aic_errno.h>
#include <aic_math.h>

#include "fmdcore.h"
#include "fmdpart.h"

static LIST_HEAD(s_fmd_dev_list);

int fmd_device_register(struct fmd_dev *dev)
{
    struct fmd_pdev *pdev;
    struct fmd_part part;
    u8 devno = dev->devno;
    u32 nr_parts, nr_groups, cur_offset = 0;
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
        FMD_ASSERT_INFO(ret == AIC_SUCCESS,
                            "fmd parse partition failed(%d)!\n", ret);

        part.size = dev->size;

        pdev = fmd_allocate_partition(dev, &part, 0);
        FMD_ASSERT_INFO(pdev != NULL,
                            "fmd allocate partition failed(no memory)!\n");

        list_add_tail(&pdev->node, &(dev->head));
    } else {
        for (i = 0; i < nr_parts; i++) {
            ret = fmd_parse_partitions(dev, i, &part);
            FMD_ASSERT_INFO(ret == AIC_SUCCESS,
                                "fmd parse partition failed(%d)!\n", ret);

            pdev = fmd_allocate_partition(dev, &part, cur_offset);
            FMD_ASSERT_INFO(pdev != NULL,
                                "fmd allocate partition failed(no memory)!\n");

            list_add_tail(&pdev->node, &(dev->head));

            cur_offset = pdev->size + pdev->offset;
        }

        nr_groups = fmd_get_num_of_groups(devno);
        for (i = 0; i < nr_groups; i++) {
            pdev = fmd_allocate_group(dev, i);
            FMD_ASSERT_INFO(pdev != NULL,
                            "fmd allocate group failed(no memory)!\n");

            list_add_tail(&pdev->node, &(dev->head));
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

    FMD_INT_DISABLE

    pdev = fmd_get_pdev_by_name(name);
    if (!pdev) {
        FMD_INT_RESTORE
        return AIC_ENODEV;
    }

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
    int ret;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_lock))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    ret = dev->_lock(dev, (pdev->offset + offset), size);

    return ret;
}

int fmd_unlock(u32 handle, u32 offset, u32 size)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;
    int ret;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_unlock))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    ret = dev->_unlock(dev, (pdev->offset + offset), size);

    return ret;
}

boolean fmd_is_locked(u32 handle, u32 offset, u32 size)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;
    int ret;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev || !(dev->_is_locked))
        return AIC_EOPNOTSUPP;

    if (offset >= pdev->size || size > pdev->size - offset)
        return AIC_EINVAL;

    if (!size)
        return AIC_EINVAL;

    ret = dev->_is_locked(dev, (pdev->offset + offset), size);

    return ret;
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

int fmd_get_partition_ofs(u32 handle, u32 *retofs)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retofs = pdev->offset;

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

int fmd_enter_read_mode(u32 handle)
{
    struct fmd_pdev *pdev = (struct fmd_pdev *)handle;
    struct fmd_dev *dev;

    if (!pdev)
        return AIC_EINVAL;

    dev = pdev->parent;
    if (!dev)
        return AIC_EOPNOTSUPP;

    dev->_enter_read(dev);

    return AIC_SUCCESS;
}

int fmd_erase_total_chip(u8 devno)
{
    struct fmd_dev *dev = NULL;
    boolean find_flag = FALSE;

    list_for_each_entry(dev, &s_fmd_dev_list, node, __typeof(*dev)) {
        if (dev && dev->devno == devno) {
            find_flag = TRUE;
            break;
        }
    }

    if (!find_flag)
        return AIC_EOPNOTSUPP;

    dev->_erase_total_chip(dev);

    return AIC_SUCCESS;
}

int fmd_get_dev_num(void)
{
    struct fmd_dev *dev = NULL;
    int i = 0;

    list_for_each_entry(dev, &s_fmd_dev_list, node, __typeof(*dev)) {
        i++;
    }

    return i;
}

u32 fmd_get_total_size(u8 devno)
{
    struct fmd_dev *dev = NULL;

    list_for_each_entry(dev, &s_fmd_dev_list, node, __typeof(*dev)) {
        if (dev && dev->devno == devno)
            return dev->size;
    }

    return 0;
}

u32 fmd_get_dev_handle(u8 devno)
{
    struct fmd_dev *dev = NULL;

    list_for_each_entry(dev, &s_fmd_dev_list, node, __typeof(*dev)) {
        if (dev && dev->devno == devno)
            return (u32)dev;
    }

    return 0;
}

