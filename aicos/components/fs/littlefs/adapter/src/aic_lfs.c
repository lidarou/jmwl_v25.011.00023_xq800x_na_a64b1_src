/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic littlefs operation interface.
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15a557
#define LOG_MODULE    AMOD_FS

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_errno.h>
#include <stdbool.h>
#include "alog_api.h"
#include "lfs.h"
#include "lfs_util.h"
#include "vfs_cfg_type.h"
#include "aic_osa.h"
#include "aic_lfs.h"

#ifndef WIN32
#include <rtc.h>
#endif

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Private Type
 **---------------------------------------------------------------------------*/
struct lfs_dirent_s{
    struct lfs_dir _dir;
    char path[LFS_NAME_MAX + 1];
};

struct lfs_file_s{
    struct lfs_file _file;
    char path[LFS_NAME_MAX + 1];
};

/**---------------------------------------------------------------------------*
 ** Private Functions
 **---------------------------------------------------------------------------*/
static int littlefs_init_cfg(AIC_LFS_INFO *fs_info)
{
    int ret = 0;
    FS_CFG *fs_cfg = fs_info->cfg;
    struct lfs_config *lfs_cfg = &fs_info->lfs_cfg;
    AIC_ASSERT(fs_cfg && lfs_cfg);

    fs_info->part_info.devpath = fs_info->dev_ops->path;
    fs_info->part_info.target = fs_info->volume_name;
    fs_info->part_info.fstype = fs_info->cfg->fstype;
    fs_info->part_info.sector_size = fs_info->cfg->bytes_per_sector;
    fs_info->part_info.total_sectors = fs_info->cfg->total_sectors;

    if (fs_info->dev_ops->open) {
        ret = fs_info->dev_ops->open(&fs_info->part_info);
        if (ret != AIC_SUCCESS) {
            aicx_error(0x1ded8c, "sd", "[LFS] dev open %s failed with %d", fs_info->volume_name, ret);
            return ret;
        }

        fs_info->cfg->block_size = fs_info->part_info.block_size;
        fs_info->cfg->block_count = fs_info->part_info.block_count;
    }

    lfs_cfg->prog_size = lfs_cfg->read_size = fs_cfg->bytes_per_sector;
    lfs_cfg->block_size = fs_info->part_info.block_size;
    lfs_cfg->block_count = fs_info->part_info.block_count;
    lfs_cfg->block_cycles = CONFIG_FS_LITTLEFS_BLOCK_CYCLES;
    lfs_cfg->cache_size = fs_info->part_info.block_size;
    lfs_cfg->lookahead_size = ALIGN(CONFIG_FS_LITTLEFS_LOOKAHEAD_SIZE, 8);

    aicx_debug(0x04fb1b, "uuuuuu", "[LFS] cfg: prog_size(%u) cache_size(%u) lookahead_size(%u)"
            "block_size(%u) block_count(%u) block_cycles(%u)",
            lfs_cfg->prog_size, lfs_cfg->cache_size, lfs_cfg->lookahead_size,
            lfs_cfg->block_size, lfs_cfg->block_count, lfs_cfg->block_cycles);

    lfs_cfg->read = lfs_aicbd_read;
    lfs_cfg->prog = lfs_aicbd_prog;
    lfs_cfg->erase = lfs_aicbd_erase;
    lfs_cfg->sync = lfs_aicbd_sync;
    lfs_cfg->context = (void *)fs_info;

    return AIC_SUCCESS;
}

static boolean lfs_is_path_opened(lfs_t *lfs, const char *path)
{
    const char *node_path = NULL;

    for (struct lfs_mlist **p = &lfs->mlist; *p; p = &(*p)->next) {
        if ((*p)->type == LFS_TYPE_DIR) {
            struct lfs_dirent_s *node = (struct lfs_dirent_s *)*p;
            node_path = node->path;
        } else if ((*p)->type == LFS_TYPE_REG) {
            struct lfs_file_s *node = (struct lfs_file_s *)*p;
            node_path = node->path;
        } else {
            AIC_ASSERT(0);
        }

        if (node_path && !memcmp(node_path, path, strlen(path) + 1)) {
            return true;
        }
    }

    return false;
}

static void lfs_dirpathsearch(const char *path, u32 search_pos, u32 *pos)
{
    u32 len = strlen(path), i;

    if (path[0] == '/')
        search_pos++;

    for (i = search_pos; i < len; i++) {
        if (path[i] == '/') {
            break;
        }
    }

    *pos = i;
}

static void lfs_set_timestamp(struct lfs *lfs, const char *path)
{
    u64 timestamp = 0;
#ifndef WIN32
    struct rtc_time rtc_tm;
#endif

    /* first time create */
#ifndef WIN32
    gx_rtc_get_time(&rtc_tm);
    timestamp = rtc_tm_to_ms(&rtc_tm) / (u64)1000;
#endif
    lfs_setattr(lfs, path, LFS_TIMESTAMP_ATTR, &timestamp, sizeof(timestamp));
}

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
int aic_lfs_mount(struct vfs_inode *mp, unsigned long mountflags, const void *data)
{
    boolean firstinit = false;
    int ret = 0;

    if ((strlen(FS_LFS_TYPE_NAME) != strlen(mp->ops.fstypeops->type)) ||
        (memcmp(FS_LFS_TYPE_NAME, mp->ops.fstypeops->type, strlen(mp->ops.fstypeops->type)))) {
        aicx_error(0x0df60e, "ss", "[LFS][MOUNT] type name(%s) (%s) is different", FS_LFS_TYPE_NAME, mp->ops.fstypeops->type);
        return AIC_EINVAL;
    }

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        fs_info = (AIC_LFS_INFO *)aos_zalloc(sizeof(AIC_LFS_INFO));

        fs_info->mutex = vfs_create_mutex(mp->name);

        firstinit = true;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    if (!fs_info->initd) {
        fs_info->cfg = (FS_CFG *)data;
        fs_info->volume_name = mp->name;
        fs_info->dev_ops = mp->ops.blkdevops;

        ret = littlefs_init_cfg(fs_info);
        if (ret != AIC_SUCCESS) {
            goto exit;
        }

        ret = lfs_mount(&fs_info->lfs, &fs_info->lfs_cfg);
        if (ret < 0 && fs_info->cfg->format_eb) {
            aicx_info(0x1daea7, "d", "[LFS][MOUNT] mount failed %d, try formatting ...", ret);
            ret = lfs_format(&fs_info->lfs, &fs_info->lfs_cfg);
            if (ret == LFS_ERR_OK) {
                ret = lfs_mount(&fs_info->lfs, &fs_info->lfs_cfg);
            }
        }

        if (ret == LFS_ERR_OK) {
            fs_info->initd = 1;
        } else {
            aicx_error(0x12b779, "d", "[LFS][MOUNT] failed with %d", ret);
        }
    }

exit:
    if (ret != AIC_SUCCESS) {
        vfs_put_mutex(fs_info->mutex);
        if (firstinit) {
            vfs_delete_mutex(fs_info->mutex);
            aos_free(fs_info);
        }
    } else {
        mp->ops.fstypehandle = (u32)fs_info;
        mp->ops.devhandle = (u32)&fs_info->part_info;
        vfs_put_mutex(fs_info->mutex);
    }

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_umount(struct vfs_inode *mp)
{
    int ret = 0;

    if ((strlen(FS_LFS_TYPE_NAME) != strlen(mp->ops.fstypeops->type)) ||
        (memcmp(FS_LFS_TYPE_NAME, mp->ops.fstypeops->type, strlen(mp->ops.fstypeops->type)))) {
        aicx_error(0x09ed83, "ss", "[LFS] mount type name(%s) (%s) is different", FS_LFS_TYPE_NAME, mp->ops.fstypeops->type);
        return AIC_EINVAL;
    }

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    if (fs_info->lfs.mlist) {
        vfs_put_mutex(fs_info->mutex);
        return AIC_EBUSY;
    }

    ret = lfs_unmount(&fs_info->lfs);

    if (fs_info->dev_ops->flush)
        fs_info->dev_ops->flush(&fs_info->part_info);

    if (fs_info->dev_ops->close)
        fs_info->dev_ops->close(&fs_info->part_info);

    fs_info->initd = 0;

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_umount2(struct vfs_inode *mp, int flag)
{
    return AIC_ENOSYS;
}

int aic_lfs_statfs(struct vfs_inode *mp, struct statfs *buf)
{
    int ret = 0;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(buf, 0x0, sizeof(struct statfs));
    buf->f_type    = LITTLEFS_TYPE;
    buf->f_namelen = FS_CACHE_LEN;
    buf->f_bsize   = fs_info->lfs_cfg.block_size;
    buf->f_blocks  = fs_info->lfs_cfg.block_count;
    buf->f_bfree   = fs_info->lfs_cfg.block_count;
    buf->f_bavail  = fs_info->lfs_cfg.block_count;

    lfs_ssize_t used_blocks = lfs_fs_size(&fs_info->lfs);
    if (used_blocks >= 0) {
        buf->f_bfree    -= used_blocks;
        buf->f_bavail   -= used_blocks;
    }

    if (used_blocks < 0) {
        ret = (int)used_blocks;
        aicx_error(0x019f59, "d", "[LFS][STATFS] failed with %d", ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_open(struct vfs_file *file, const char *path, int oflags, mode_t mode)
{
    int ret;
    int flags;
    u64 timestamp;
    struct lfs_file_s *lfs_file;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(file->inode->ops.fstypehandle);

    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    if ((oflags & O_CREAT) && (oflags & O_EXCL)) {
        vfs_put_mutex(fs_info->mutex);
        return AIC_ENOTSUP;
    }

    lfs_file = (struct lfs_file_s *)aos_zalloc(sizeof(struct lfs_file_s));

    snprintf(lfs_file->path, LFS_NAME_MAX, "/%s", path);
    flags = lfs_convert_oflags(oflags);

    ret = lfs_file_open(&fs_info->lfs, &lfs_file->_file, (char *)lfs_file->path, flags);
    if (ret >= 0) {
        file->priv = (void *)lfs_file;
        file->size = lfs_file->_file.ctz.size;
        file->pos = lfs_file->_file.pos;

        if (lfs_getattr(&fs_info->lfs, lfs_file->path, LFS_TIMESTAMP_ATTR, &timestamp, sizeof(timestamp)) == LFS_ERR_NOATTR) {
            lfs_set_timestamp(&fs_info->lfs, lfs_file->path);
        }

        /* Sync here in case of O_TRUNC haven't actually done immediately,
        * e.g. total 8M, fileA 6M, O_TRUNC re-wrting fileA 6M, meet error.
        */
        lfs_file_sync(&fs_info->lfs, &lfs_file->_file);
    }

    if (ret < 0) {
        aicx_error(0x002bb7, "sxxd", "[LFS][OPEN] file(%s)[0x%x->0x%x] open error, ret(%d)",
                    lfs_file->path, oflags, flags, ret);
        aos_free(lfs_file);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_close(struct vfs_file *file)
{
    int ret;
    struct lfs_file_s *lfs_file;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_file = (struct lfs_file_s *)file->priv;
    ret = lfs_file_close(&fs_info->lfs, &lfs_file->_file);

    if (ret < 0) {
        aicx_error(0x10656a, "sd", "[LFS][CLOSE] file(%s) close error, ret(%d)!", lfs_file->path, ret);
    } else {
        file->priv = NULL;
        aos_free(lfs_file);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

ssize_t aic_lfs_read(struct vfs_file *file, char *buffer, size_t buflen)
{
    int ret;
    struct lfs_file_s *lfs_file;
    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(file->inode->ops.fstypehandle);

    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_file = (struct lfs_file_s *)file->priv;
    ret = lfs_file_read(&fs_info->lfs, &lfs_file->_file, buffer, (lfs_size_t )buflen);
    /* adjust offset*/
    file->pos = lfs_file->_file.pos;

    if (ret < 0) {
        aicx_error(0x02d24b, "sud", "[LFS][READ] file(%s) read %u Bytes error, ret(%d)!", lfs_file->path, buflen, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return (ssize_t)lfs_to_aic_errcode(ret);
}

ssize_t aic_lfs_write(struct vfs_file *file, const char *buffer, size_t buflen)
{
    int ret;
    struct lfs_file_s *lfs_file;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(file->inode->ops.fstypehandle);

    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_file = (struct lfs_file_s *)file->priv;
    if (lfs_file->_file.flags & LFS_F_ERRED) {
        ret = LFS_ERR_IO;
    } else {
#ifndef CONFIG_CUSTOMER_CM_AT
        if (lfs_file->_file.flags & LFS_O_APPEND) {
            lfs_file->_file.pos = lfs_file->_file.ctz.size;
        }
#endif

        ret = lfs_file_write(&fs_info->lfs, &lfs_file->_file, buffer, (lfs_size_t)buflen);
        /* adjust offset */
        file->pos = lfs_file->_file.pos;
    }

    if (ret >= 0) {
        if (lfs_file->_file.flags & LFS_O_APPEND) {
            // update file->ctz.size
            lfs_file_sync(&fs_info->lfs, &lfs_file->_file);
        }
    } else if (ret == LFS_ERR_NOSPC) {
        /* Clear error if just no space */
        lfs_file->_file.flags &= ~(LFS_F_ERRED);
    }

    if (ret < 0) {
        aicx_error(0x10e492, "sud", "[LFS][WRITE] file(%s) write %u Bytes error, ret(%d)", lfs_file->path, buflen, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return (ssize_t)lfs_to_aic_errcode(ret);
}

off_t aic_lfs_lseek(struct vfs_file *file, off_t offset, int whence)
{
    int ret;
    struct lfs_file_s *lfs_file;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_file = (struct lfs_file_s *)file->priv;
    /** @note unistd whence same as LFS whence */
    ret = lfs_file_seek(&fs_info->lfs, &lfs_file->_file, offset, whence);
    /* adjust offset */
    file->pos = lfs_file->_file.pos;

    if (ret < 0) {
        aicx_error(0x0122b1, "slddd", "[LFS][SEEK] file(%s) seek offset(%ld) whence(%d) error(%d)",
                   lfs_file->path, offset, whence, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_truncate(struct vfs_file *file, off_t length)
{
    int ret;
    struct lfs_file_s *lfs_file;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_file = (struct lfs_file_s *)file->priv;
    ret = lfs_file_truncate(&fs_info->lfs, &lfs_file->_file, length);

    if (ret == LFS_ERR_OK)
        lfs_file_sync(&fs_info->lfs, &lfs_file->_file);

    /* adjust offset */
    file->pos = lfs_file->_file.pos;

    if (ret < 0) {
        aicx_error(0x06381f, "sud", "[LFS][TRUNCATE] file(%s) truncate to %u error(%d)", lfs_file->path, length, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return AIC_SUCCESS;
}

int aic_lfs_ioctl(struct vfs_file *file, int cmd, void *arg)
{
    return AIC_ENOSYS;
}

int aic_lfs_sync(struct vfs_file *file)
{
    int ret;
    struct lfs_file_s *lfs_file;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_file = (struct lfs_file_s *)file->priv;
    if (lfs_file->_file.flags & LFS_F_ERRED) {
        ret = LFS_ERR_IO;
    } else {
        lfs_set_timestamp(&fs_info->lfs, lfs_file->path);

        ret = lfs_file_sync(&fs_info->lfs, &lfs_file->_file);
    }

    if (ret < 0) {
        aicx_error(0x02a9f3, "sd", "[LFS][SYNC] file(%s) sync error(%d)", lfs_file->path, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_fstat(struct vfs_file *file, struct stat *buf)
{
    struct lfs_file_s *lfs_file;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_file = (struct lfs_file_s *)file->priv;
    lfs_file_sync(&fs_info->lfs, &lfs_file->_file);
    vfs_put_mutex(fs_info->mutex);

    int aic_lfs_stat(struct vfs_inode *mp, const char *path, struct stat *buf);
    /* skip '/' from lfs_file->path */
    return aic_lfs_stat(file->inode, &lfs_file->path[1], buf);
}

int aic_lfs_dup(const struct vfs_file *oldfile, struct vfs_file *newfile)
{
    return AIC_ENOSYS;
}

int aic_lfs_opendir(struct vfs_dir *dir, const char *path)
{
    struct lfs_dirent_s *lfs_dir;
    int ret;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(dir->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_dir = aos_zalloc(sizeof(struct lfs_dirent_s));

    do {
        snprintf(lfs_dir->path, LFS_NAME_MAX, "/%s", path);

        ret = lfs_dir_open(&fs_info->lfs, &lfs_dir->_dir, lfs_dir->path);
        if (ret != LFS_ERR_OK) {
            aos_free(lfs_dir);
            break;
        }

        dir->thread = aos_identify_thread();
        dir->priv = (void *)lfs_dir;
    } while (0);

    if (ret < 0) {
        aicx_error(0x0aa7cd, "sd", "[LFS][OPENDIR] </%s> error(%d)", path, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_readdir(struct vfs_dir *dir, struct dirent *dirent)
{
    int ret;

    if (!dir || (dir->thread != aos_identify_thread())) {
        return AIC_EINVAL;
    }

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(dir->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    struct lfs_info *info = aos_zalloc(sizeof(struct lfs_info));

    struct lfs_dir *lfs_dir = &(((struct lfs_dirent_s *)dir->priv)->_dir);
do_read:
    ret = lfs_dir_read(&fs_info->lfs, lfs_dir, info);
    if (ret > 0) {
        if (((strlen(".") == strlen(info->name)) &&
             !memcmp(".", info->name, strlen(info->name))) ||
            ((strlen("..") == strlen(info->name)) &&
             !memcmp("..", info->name, strlen(info->name)))) {
            /* skip . & .. */
            goto do_read;
        }

        snprintf(dirent->d_name, NAME_MAX, "%s", info->name);
        dirent->d_type = (info->type == LFS_TYPE_DIR) ? DT_DIR : DT_REG;
        dirent->d_off = dir->pos;
        dirent->d_reclen = ALIGN((sizeof(struct dirent) -
                                    sizeof(dirent->d_name) +
                                    strlen(dirent->d_name) + 1),
                                    4);

        dir->pos = lfs_dir->pos;
    }

    if (ret == 0) {
        dirent->d_name[0] = 0;
        /* read dir end */
        ret = LFS_ERR_NOENT;
    } else if (ret < 0) {
        aicx_error(0x1bb52c, "sd", "[LFS][READDIR] </%s> failed: %d", dir->inode->name, ret);
    } else {
        ret = 0;
    }

    aos_free(info);
    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_seekdir(struct vfs_dir *dir, long offset)
{
    int ret;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(dir->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    struct lfs_dir *lfs_dir = &(((struct lfs_dirent_s *)dir->priv)->_dir);
    ret = lfs_dir_seek(&fs_info->lfs, lfs_dir, (lfs_off_t)offset);
    dir->pos = lfs_dir->pos;

    if (ret < 0) {
        aicx_error(0x1b6c2e, "sldd", "[LFS][SEEKDIR] </%s> offset(%ld) error(%d)", dir->inode->name, offset, ret);
    } else {
        ret = dir->pos;
    }

    vfs_put_mutex(fs_info->mutex);
    return lfs_to_aic_errcode(ret);
}

long aic_lfs_telldir(struct vfs_dir *dir)
{
    int ret;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(dir->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    ret = dir->pos;

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_rewinddir(struct vfs_dir *dir)
{
    int ret;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(dir->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    struct lfs_dir *lfs_dir = &(((struct lfs_dirent_s *)dir->priv)->_dir);
    ret = lfs_dir_rewind(&fs_info->lfs, lfs_dir);
    dir->pos = lfs_dir->pos;

    if (ret < 0) {
        aicx_error(0x0ecb5d, "sd", "[LFS][REWINDDIR] </%s> failed: %d", dir->inode->name, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_closedir(struct vfs_dir *dir)
{
    struct lfs_dirent_s *lfs_dir;
    int ret;

    if (!dir || !dir->priv || (dir->thread != aos_identify_thread())) {
        return AIC_EINVAL;
    }

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(dir->inode->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    lfs_dir = (struct lfs_dirent_s *)dir->priv;
    ret = lfs_dir_close(&fs_info->lfs, &lfs_dir->_dir);

    if (ret < 0) {
        aicx_error(0x137f8a, "sd", "[LFS][CLOSEDIR] </%s> failed: %d", dir->inode->name, ret);
    } else {
        dir->priv = NULL;
        aos_free(lfs_dir);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_unlink(struct vfs_inode *mp, const char *path)
{
    int ret;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    if (path[0] == '\0') {
        vfs_put_mutex(fs_info->mutex);
        return AIC_ENOEXEC;
    }

    aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", path);

    if (!lfs_is_path_opened(&fs_info->lfs, fs_info->cache_buf)) {
        ret = lfs_remove(&fs_info->lfs, fs_info->cache_buf);
    } else {
        ret = LFS_ERR_OPENED;
    }

    if (ret < 0) {
        aicx_error(0x12c3c8, "sd", "[LFS][UNLINK] </%s> failed: %d", path, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_mkdir(struct vfs_inode *mp, const char *path)
{
    int ret = 0, pos = 0;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", path);

    ret = lfs_mkdir(&fs_info->lfs, fs_info->cache_buf);
    if (ret >= 0) {
        lfs_set_timestamp(&fs_info->lfs, fs_info->cache_buf);
    } else if (ret == LFS_ERR_EXIST) {
        /* Dir already exists, consider this as success. */
        ret = LFS_ERR_OK;
    } else if (ret == LFS_ERR_NOENT) {
        /* Recursive mkdir */
        char *createdir = aos_zalloc(strlen(path) + 2);

        while (pos < strlen(fs_info->cache_buf)) {
            lfs_dirpathsearch(fs_info->cache_buf, pos, &pos);
            aos_memcpy(createdir, fs_info->cache_buf, pos);

            ret = lfs_mkdir(&fs_info->lfs, createdir);
            if (ret >= 0) {
                lfs_set_timestamp(&fs_info->lfs, createdir);
            } else if (ret == LFS_ERR_EXIST) {
                /* Dir already exists, consider this as success. */
                ret = LFS_ERR_OK;
            } else {
                break;
            }

            pos += 1;
        };

        aos_free(createdir);
    }

    if (ret < 0) {
        aicx_error(0x0f137c, "sd", "[LFS][MKDIR] </%s> failed: %d", path, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_rmdir(struct vfs_inode *mp, const char *path)
{
    return aic_lfs_unlink(mp, path);
}

int aic_lfs_rename(struct vfs_inode *mp, const char *oldpath, const char *newpath)
{
    int ret = 0;
    char *npath;

    AIC_LFS_INFO *fs_info = (AIC_LFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    npath = aos_zalloc(strlen(newpath) + 4);

    aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", oldpath);
    snprintf(npath, strlen(newpath) + 4, "/%s", newpath);

    if (!lfs_is_path_opened(&fs_info->lfs, fs_info->cache_buf)
    && !lfs_is_path_opened(&fs_info->lfs, npath)) {
        ret = lfs_rename(&fs_info->lfs, fs_info->cache_buf, npath);
    } else {
        ret = LFS_ERR_OPENED;
    }

    if (ret < 0) {
        aicx_error(0x02f606, "ssd", "[LFS][RENAME] </%s> to </%s> failed: %d", oldpath, newpath, ret);
    }
    aos_free(npath);

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_stat(struct vfs_inode *mp, const char *path, struct stat *buf)
{
    int ret = 0;
    u64 timestamp = 0;

    AIC_LFS_INFO* fs_info = (AIC_LFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    struct lfs_info *info = aos_zalloc(sizeof(struct lfs_info));

    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';

    ret = lfs_stat(&fs_info->lfs, fs_info->cache_buf, info);
    if (ret == LFS_ERR_OK) {
        lfs_getattr(&fs_info->lfs, fs_info->cache_buf, LFS_TIMESTAMP_ATTR, &timestamp, sizeof(timestamp));
    }

    if (ret == LFS_ERR_OK) {
        aos_memset(buf, 0x0, sizeof(struct stat));
        buf->st_mode  = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                        S_IWUSR | S_IWGRP | S_IWOTH;
        if (info->type == LFS_TYPE_DIR) {
            buf->st_mode &= ~S_IFREG;
            buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        }
        buf->st_size = (off_t)info->size;
        buf->st_mtim.tv_sec = timestamp;
        buf->st_blksize = fs_info->lfs_cfg.block_size;
        buf->st_blocks = DIV_ROUND_UP(info->size, buf->st_blksize);
    } else {
        aicx_error(0x1691fd, "sd", "[LFS][STAT] </%s> failed: %d", path, ret);
    }

    aos_free(info);

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

int aic_lfs_format(struct vfs_inode *mp)
{
    int ret = 0, ret1;

    AIC_LFS_INFO * fs_info = (AIC_LFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    do {
        if (fs_info->lfs.mlist) {
            ret  = AIC_EBUSY;
            break;
        }

        ret = lfs_unmount(&fs_info->lfs);
        if (ret != LFS_ERR_OK)
            break;

        ret = lfs_format(&fs_info->lfs, &fs_info->lfs_cfg);

        ret1 = lfs_mount(&fs_info->lfs, &fs_info->lfs_cfg);
        if (ret == LFS_ERR_OK)
            ret = ret1;
    } while(0);

    if (ret < 0) {
        aicx_error(0x0dbf75, "sd", "[LFS][FORMAT] (%s) ret: %d", mp->name, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return lfs_to_aic_errcode(ret);
}

struct vfs_type_mount_ops g_lfs_mnt_ops = {
    aic_lfs_mount,
    aic_lfs_umount,
    aic_lfs_umount2,
    aic_lfs_statfs
};

struct vfs_type_file_ops g_lfs_file_ops = {
    aic_lfs_open,
    aic_lfs_close,
    aic_lfs_read,
    aic_lfs_write,
    aic_lfs_lseek,
    aic_lfs_truncate,
    aic_lfs_ioctl,
    aic_lfs_sync,
    aic_lfs_fstat,
    aic_lfs_dup,
    aic_lfs_opendir,
    aic_lfs_readdir,
    aic_lfs_telldir,
    aic_lfs_seekdir,
    aic_lfs_rewinddir,
    aic_lfs_closedir,
    aic_lfs_unlink,
    aic_lfs_mkdir,
    aic_lfs_rmdir,
    aic_lfs_rename,
    aic_lfs_stat,
    NULL
};

struct vfs_type_management_ops g_lfs_mgt_ops = {
    aic_lfs_format
};

int aic_lfs_init(void)
{
    int ret;

    ret = vfs_type_register(FS_LFS_TYPE_NAME, &g_lfs_mnt_ops,
                            &g_lfs_file_ops, &g_lfs_mgt_ops);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x1afed7, "sd", "fs register type %s failed: %d", FS_LFS_TYPE_NAME, ret);
    }

    return ret;
}

AIC_COMP_P1_INIT_EXPORT(aic_lfs_init);
