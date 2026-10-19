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

#ifndef __FMD_H__
#define __FMD_H__

#include <aic_type.h>
#include <aic_list.h>

#define FMD_QSPI_NORFLASH 1
#define FMD_SSP_NORFLASH  2

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

    /* fmd device list node */
    struct list_head node;

    /* fmd partition list head */
    struct list_head head;

    void *priv;
};

int fmd_device_register(struct fmd_dev *dev);
int fmd_open(char *name, u32 *handle);
int fmd_close(u32 handle);
int fmd_read(u32 handle, u32 offset, u32 size, u8 *buf);
int fmd_write(u32 handle, u32 offset, u32 size, u8 *buf);
int fmd_erase(u32 handle, u32 offset, u32 size);
int fmd_lock(u32 handle, u32 offset, u32 size);
int fmd_unlock(u32 handle, u32 offset, u32 size);
boolean fmd_is_locked(u32 handle, u32 offset, u32 size);
int fmd_get_physical_address(u32 handle, u32 offset, u32 *retaddr);
int fmd_get_partition_size(u32 handle, u32 *retsize);
int fmd_get_erase_size(u32 handle, u32 *retsize);
int fmd_get_device_type(u32 handle, u32 *rettype);

#endif /* __FMD_H__ */

