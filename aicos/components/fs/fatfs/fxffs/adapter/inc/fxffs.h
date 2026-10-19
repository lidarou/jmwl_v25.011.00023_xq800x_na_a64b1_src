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

#ifdef CONFIG_FX_UTF8_SUPPORT
    #define FS_CACHE_LEN (PATH_MAX * 3)
#else
    #define FS_CACHE_LEN (PATH_MAX)
#endif


/**---------------------------------------------------------------------------*
 ** Public Types
 **---------------------------------------------------------------------------*/
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
    FXFFS_MEDIA_UNICODE_OPS *media_unicode_ops;
    aos_systime systime;
    FXFFS_NOTIFY *fs_notify;
    aos_mutex_ptr mutex;
    aos_event_ptr event;
    u32 initd;
    boolean default_media;
    char cache_buf[FS_CACHE_LEN];
#ifdef CONFIG_FX_UTF8_SUPPORT
    /* UCS2 */
    char last_dir_buf[PATH_MAX * 2];
#endif
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
