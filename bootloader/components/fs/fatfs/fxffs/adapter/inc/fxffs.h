/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system information define.
 */

#ifndef __AIC_FXFFS_H__
#define __AIC_FXFFS_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <fx_api.h>
#include "vfs.h"
#include "fxffs_media.h"
#include "fxffs_media_init.h"
#include "fxffs_media_dir.h"
#include "fxffs_media_file.h"
#include "fxffs_media_unicode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 ** Public Types
 **---------------------------------------------------------------------------*/
typedef struct aos_systim_st {
    u32               year;
    u32               month;
    u32               wday;
    u32               day;
    u32               hour;
    u32               minute;
    u32               second;
    u32               msecond;
    u32               usecond;
} aos_systime;

typedef struct fxffs_notify {
    void (*media_open)(void *fxffs_info);
    void (*media_close)(void *fxffs_info);
    void (*file_write)(void *fxffs_info);
} FXFFS_NOTIFY;

typedef struct fxffs_info {
    char *volume_name;
    FS_CFG *cfg;
    struct vfs_blkdev_partinfo part_info;
    struct vfs_blkdev_ops *dev_ops;
    u32 media_handle;
    FXFFS_MEDIA_OPS *media_ops;
    FXFFS_MEDIA_INIT_OPS *media_init_ops;
    FXFFS_MEDIA_DIR_OPS *media_dir_ops;
    FXFFS_MEDIA_FILE_OPS *media_file_ops;
    aos_systime systime;
    FXFFS_NOTIFY *fs_notify;
    u32 initd;
    boolean default_media;
    char cache_buf[PATH_MAX];
} FXFFS_INFO;

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * fxffs_media_init - fxffs media init
 *
 * Parameter:
 * @fxffs_info: fxffs info.
 *
 * Return:
 *  fxffs_SUCCESS pass, other fail.
 */
u32 fxffs_media_init(FXFFS_INFO *fxffs_info);

/**
 * fs_media_uninit - fs media uninit
 *
 * Parameter:
 * @dev_type: fs device type.
 *
 * Return:
 *  FS_SUCCESS pass, other fail.
 */
u32 fxffs_media_uninit(FXFFS_INFO *fs_info);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/**---------------------------------------------------------------------------*/
#endif /* __AIC_FXFFS_H__ */
