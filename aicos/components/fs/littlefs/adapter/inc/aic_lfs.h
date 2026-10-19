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

#ifndef __AIC_LFS_H__
#define __AIC_LFS_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <stddef.h>
#include <errno.h>
#include "aic_assert_def.h"
#include "aic_errno.h"
#include "aic_lfs.h"
#include "lfs.h"
#include "lfs_util.h"
#include "vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FS_CACHE_LEN        (LFS_NAME_MAX)
#define LFS_TIMESTAMP_ATTR  't'
#define LFS_ERR_OPENED      (-EBUSY)

/**---------------------------------------------------------------------------*
 ** Public Types
 **---------------------------------------------------------------------------*/
typedef struct aic_lfs_info {
    struct lfs_config lfs_cfg;
    struct lfs lfs;

    char *volume_name;
    FS_CFG *cfg;
    struct vfs_blkdev_partinfo part_info;
    struct vfs_blkdev_ops *dev_ops;
    aos_mutex_ptr mutex;
    u32 initd;
    char cache_buf[FS_CACHE_LEN + 1];
} AIC_LFS_INFO;

static inline int lfs_convert_oflags(int oflags)
{
    const u16 vfs_flags = (u16)oflags;
    u16 lfs_flags = 0;

    if ((vfs_flags & O_RDWR) == O_RDWR) {
        lfs_flags |= LFS_O_RDWR;
    } else if ((vfs_flags & O_WRONLY) == O_WRONLY) {
        lfs_flags |= LFS_O_WRONLY;
    } else {
        lfs_flags |= LFS_O_RDONLY;
    }

    if ((vfs_flags & O_APPEND) == O_APPEND) {
        lfs_flags |= LFS_O_APPEND;
    }

    if ((vfs_flags & O_CREAT) == O_CREAT) {
        lfs_flags |= LFS_O_CREAT;
    }

    if ((vfs_flags & O_TRUNC) == O_TRUNC) {
        lfs_flags |= LFS_O_TRUNC;
    }

    if ((vfs_flags & O_EXCL) == O_EXCL) {
        lfs_flags |= LFS_O_EXCL;
    }

    return (int)lfs_flags;
}

static inline int lfs_to_aic_errcode(int error)
{
    if (error >= 0) {
        return error;
    }

    switch (error) {
        case LFS_ERR_IO:
            return AIC_EIO;

        case LFS_ERR_CORRUPT:
            return AIC_EFAULT;

        case LFS_ERR_NOENT:
            return AIC_ENOENT;

        case LFS_ERR_EXIST:
            return AIC_EEXIST;

        case LFS_ERR_NOTDIR:
            return AIC_ENOTDIR;

        case LFS_ERR_ISDIR:
            return AIC_EISDIR;

        case LFS_ERR_NOTEMPTY:
            return AIC_ENOTEMPTY;

        case LFS_ERR_BADF:
            return AIC_EBADF;

        case LFS_ERR_FBIG:
            return AIC_EFBIG;

        case LFS_ERR_INVAL:
            return AIC_EINVAL;

        case LFS_ERR_NOSPC:
            return AIC_ENOSPC;

        case LFS_ERR_NOMEM:
            return AIC_ENOMEM;

        case LFS_ERR_OPENED:
            return AIC_EBUSY;

        case LFS_ERR_NOATTR:
            return AIC_ENODATA;

        case LFS_ERR_NAMETOOLONG:
            return AIC_ENAMETOOLONG;

        default:
            return error;
    }
}

static inline int aic_to_lfs_errcode(int error)
{
    if (error >= 0) {
        return LFS_ERR_OK;
    }

    switch (error) {
        default:
        case AIC_EIO:
            return LFS_ERR_IO;
        case AIC_EFAULT:
            return LFS_ERR_CORRUPT;
        case AIC_ENOENT:
            return LFS_ERR_NOENT;
        case AIC_EEXIST:
            return LFS_ERR_EXIST;
        case AIC_ENOTDIR:
            return LFS_ERR_NOTDIR;
        case AIC_EISDIR:
            return LFS_ERR_ISDIR;
        case AIC_ENOTEMPTY:
            return LFS_ERR_NOTEMPTY;
        case AIC_EBADF:
            return LFS_ERR_BADF;
        case AIC_EFBIG:
            return LFS_ERR_FBIG;
        case AIC_EINVAL:
            return LFS_ERR_INVAL;
        case AIC_ENOSPC:
            return LFS_ERR_NOSPC;
        case AIC_ENOMEM:
            return LFS_ERR_NOMEM;
    }
}

int lfs_aicbd_read(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_aicbd_prog(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_aicbd_erase(const struct lfs_config *cfg, lfs_block_t block);
int lfs_aicbd_sync(const struct lfs_config *cfg);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/**---------------------------------------------------------------------------*/
#endif /* __AIC_LFS_H__ */
