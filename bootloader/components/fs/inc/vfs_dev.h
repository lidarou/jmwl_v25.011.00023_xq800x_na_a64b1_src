/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the fs device struct.
 */

#ifndef __AIC_VFS_DEV_H__
#define __AIC_VFS_DEV_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

#ifdef WIN32
#include "vfs_win32.h"
#else
#include <sys/types.h>
#endif
#include "vfs_cfg_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 ** Public Type Definitions
 **---------------------------------------------------------------------------*/
struct vfs_inode;

struct vfs_blkdev_partinfo {
    char *devpath;
    char *target;
    char *fstype;
    char *filepath;
    u32 flags;
    u8 *address;
    u32 sector_size;
    u32 total_sectors;
    u32 devhandle;
};

/**
 * fs device operations.
 *
 * @open: open device.
 * @close: close device.
 * @read: read data from device.
 * @write: write data to device.
 * @abort: fs abort will be callback abort interface.
 * @release: release device logic sector.
 * @mount: mount device, register by fs, device can use it and no need register.
 * @unmount: unmount device, register by fs, device can use it and no need register.
 */
struct vfs_blkdev_ops {
    char *path;

    int (*open)(struct vfs_blkdev_partinfo *partinfo);
    int (*close)(struct vfs_blkdev_partinfo *partinfo);
    int (*read)(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                u32 sector_num, u8 *buf);
    int (*write)(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                 u32 sector_num, u8 *buf);
    int (*abort)(struct vfs_blkdev_partinfo *partinfo);
    int (*release)(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                   u32 sector_num);
    int (*flush)(struct vfs_blkdev_partinfo *partinfo);
    int (*mount)(char *devpath);
    int (*unmount)(char *devpath);
};

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
int vfs_blkdev_register(struct vfs_blkdev_ops *blkdevops,
                        mode_t mode, void *priv);
int vfs_blkdev_find(const char *path, int mountflags,
                    struct vfs_inode **ppinode);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_FS_DEV_H__ */
