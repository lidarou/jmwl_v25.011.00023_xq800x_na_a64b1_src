/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_MSC_CFG_H__
#define __USB_MSC_CFG_H__

#include <ftlblock.h>
#include <fs_api.h>
#include <sdmmc_dev.h>
#include <ux_api.h>
#include <ux_device_class_storage.h>

#define USB_STORAGE_LUN_NUM         1

enum DISK_TYPE {
    STORAGE_FLASH,
    STORAGE_SDCARD,
};

struct disk_ops {
    int (*open)(char *name, u32 *handle);
    int (*close)(u32 handle);
    int (*read)(u32 handle, u32 sector_id, u32 sector_num, u8 *buf);
    int (*write)(u32 handle, u32 sector_id, u32 sector_num, u8 *buf);
    int (*get_sector_size)(u32 handle, u32 *retsize);
    int (*get_sector_num)(u32 handle, u32 *retnum);
};

#define DISK_OPS(type) \
    .open = type##_open, \
    .close = type##_close, \
    .read = type##_read, \
    .write = type##_write, \
    .get_sector_size = type##_get_sector_size, \
    .get_sector_num = type##_get_sector_num


struct UX_STORAGE {
    u32 handle;
    char *name;
    char *fs_name;
    u32 last_lba;
    u32 block_len;
    struct disk_ops ops;
    enum DISK_TYPE type;
    u32 mount;
    struct fs_mount_info mount_info;
};

struct MSC {
    UX_SLAVE_CLASS_STORAGE *class_storage;
    struct UX_STORAGE *storage;
    u32 disk_num;
    int initialization;
};

struct UX_STORAGE storage_cfg[USB_STORAGE_LUN_NUM] = {
   /* {
     .name = "C",
     .fs_name = "/C",
     .type = STORAGE_FLASH,
     .ops = {
            DISK_OPS(ftlblock),
        },
    }, */ {
     .name = "D",
     .fs_name = "/D",
     .type = STORAGE_SDCARD,
     .ops = {
        DISK_OPS(sdmmc),
      },
    },
};

struct MSC mass_storage = {
 .storage = storage_cfg,
 .disk_num = USB_STORAGE_LUN_NUM,
};

#endif  /* __USB_MSC_CFG_H__ */
