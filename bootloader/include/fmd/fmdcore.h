/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the flash management device drivers.
 */

#ifndef __FMDCORE_H__
#define __FMDCORE_H__

#include <aic_size.h>
#include <aic_type.h>
#include <assert.h>
#include <core_op.h>
#include <aic_list.h>
#include <fmd.h>

#define FMD_INT_SAVE        AIC_INT_SAVE_AREA
#define FMD_INT_DISABLE     AIC_INT_DISABLE
#define FMD_INT_RESTORE     AIC_INT_RESTORE

#define FMD_ASSERT          ASSERT
#define FMD_ASSERT_INFO     ASSERT_INFO

/**
 * FMD partitions access property
 * */
#define FMD_READONLY        0
#define FMD_WRITEABLE       1

struct fmd_part {
    char *name;
    u32 offset;
    u32 size;
    u32 flags;
};

/**
 * sturct fmd_dev - FMD device specific fields.
 *
 * @devno: device index.
 * @type: device type.
 * @size: total size of the FMD device.
 * @erasesize: minimal erase size.
 * @_erase: erase operation for FMD device.
 * @_read: read operation for FMD device.
 * @_write: write operation for FMD device.
 * @_lock: lock operation for FMD device.
 * @_unlock: unlock operation for FMD device.
 * @_is_locked: check whether the FMD device is locked.
 * @_get_mem_base: get flash memory map address.
 * @node: FMD device list node.
 * @head: FMD partition list head.
 * @priv: private data of FMD device.
 */
struct fmd_dev {
    u8 devno;
    u32 type;

    u32 size;
    u32 erasesize;

    void (*_erase)(struct fmd_dev *dev, u32 offset, u32 size);
    void (*_read)(struct fmd_dev *dev, u32 offset, u32 size, u8 *buf);
    void (*_write)(struct fmd_dev *dev, u32 offset, u32 size, u8 *buf);
    int (*_lock)(struct fmd_dev *dev, u32 offset, u32 size);
    int (*_unlock)(struct fmd_dev *dev, u32 offset, u32 size);
    int (*_is_locked)(struct fmd_dev *dev, u32 offset, u32 size);
    u32 (*_get_mem_base)(struct fmd_dev *dev);
    void (*_enter_read)(struct fmd_dev *dev);
    void (*_erase_total_chip)(struct fmd_dev *dev);

    /* fmd device list node */
    struct list_head node;

    /* fmd partition list head */
    struct list_head head;

    void *priv;
};

/**
 * struct fmd_pdev - FMD partition device
 *
 */
struct fmd_pdev {
    char *name;

    u32 ref_count;
    u32 offset;
    u32 size;
    u32 property;

    struct list_head node;

    struct fmd_dev *parent;
};

int fmd_device_register(struct fmd_dev *dev);
struct fmd_pdev *fmd_get_pdev_by_name(char *name);

#endif /* __FMDCORE_H__ */

