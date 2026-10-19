/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic filex fat file system operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x067ad9
#define LOG_MODULE    AMOD_FS

#include <stdbool.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <fx_api.h>
#ifndef WIN32
#include <rtc.h>
#endif
#if defined(CONFIG_ICONV_UTF8) || defined(WIN32)
#include <aic_utf8.h>
#endif
#include "fxffs.h"

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Private Type
 **---------------------------------------------------------------------------*/
struct fxffs_dirent_s {
    u32 local_handle;
    char *default_dir;
    char path[FS_CACHE_LEN];
};

/**---------------------------------------------------------------------------*
 ** Private Functions
 **---------------------------------------------------------------------------*/
#ifdef CONFIG_FX_UTF8_SUPPORT
static inline bool ascii_only(const char *utf8_str, int len)
{
    if (len == -1) {
        len = strlen(utf8_str);
    }

    for (int i = 0; i < len; i++) {
        if (utf8_str[i] == '\0') {
            break;
        } else if (utf8_str[i] & 0x80) {
            return false;
        }
    }

    return true;
}

/* ascii character should not exceed PATH_MAX limit */
static char g_ascii_path_buf[PATH_MAX];

static int path_to_ascii(FXFFS_INFO *fs_info, char *const utf8_str, bool *is_unicode, int *unicode_len)
{
    bool is_last = false, need_unicode = false, is_chdir = false;
    u32 err = 0, pos = 0, ascii_pos = 0, last_slash = 0, uni_len = 0;
    const char *work_ptr = utf8_str;
    char *local_path = NULL;
    u32 path_handle = 0;
    char temp_short_name[FX_MAX_SHORT_NAME_LEN];

    /* Make sure utf8_str is not empty */
    AIC_ASSERT(*work_ptr != '\0');

    *is_unicode = false;
    *unicode_len = 0;

    if (ascii_only(work_ptr, -1)) {
        /* quick path */
        return 0;
    }

    err = fs_info->media_dir_ops->local_path_get(fs_info, &local_path);
    AOS_ASSERT(err == FX_SUCCESS);

    if (local_path) {
        /* Not at root path: force chdir to root path. */
        err = fs_info->media_dir_ops->local_path_set(fs_info, &path_handle, "/");
        AOS_ASSERT(err == FX_SUCCESS);
        is_chdir = true;
    }

    do {
        if (*work_ptr == '/') {
            g_ascii_path_buf[ascii_pos++] = '/';
            work_ptr++;
        }

        /* bypass slashes */
        while (*work_ptr == '/') {
            work_ptr++;
        }

        /* loop through the remaining string */
        pos = 0;
        last_slash = 0;
        need_unicode = false;
        while (1) {
            if (work_ptr[pos] == '/') {
                if (need_unicode) {
                    break;
                }
                /* skip consecutive slashes */
                if ((!last_slash) || (pos != last_slash + 1)) {
                    last_slash = pos;
                }
            } else if (work_ptr[pos] & 0x80) {
                if (last_slash) {
                    pos = last_slash;
                    break;
                }
                need_unicode = true;
            } else if (work_ptr[pos] == '\0') {
                is_last = true;
                break;
            }

            pos++;
        }

        if (need_unicode) {
            uni_len = aic_utf8_to_unicode(work_ptr, pos, (uint16_t *)fs_info->last_dir_buf, PATH_MAX * 2);
            if (uni_len == 0) {
                goto try_direct_input;
            }

            err = fs_info->media_unicode_ops->short_name_get_ext(
                    fs_info,
                    fs_info->last_dir_buf,
                    uni_len,
                    temp_short_name,
                    FX_MAX_SHORT_NAME_LEN
                );

            if (err) {
                if (is_last) {
                    err = FX_SUCCESS;
                    *is_unicode = true;
                    *unicode_len = uni_len;

                    /* strip trailing '/' s */
                    while (g_ascii_path_buf[ascii_pos - 1] == '/') {
                        ascii_pos--;
                    }
                } else {
                    goto exit;
                }
            } else {
                aos_memcpy(&g_ascii_path_buf[ascii_pos], temp_short_name, strlen(temp_short_name));
                ascii_pos += strlen(temp_short_name);
            }
        } else {
try_direct_input:
            aos_memcpy(&g_ascii_path_buf[ascii_pos], work_ptr, pos);
            ascii_pos += pos;
        }

        if (!is_last) {
            g_ascii_path_buf[ascii_pos] = '\0';
            err = fs_info->media_dir_ops->local_path_set(fs_info, &path_handle, g_ascii_path_buf);
            if (err) {
                goto exit;
            }
            is_chdir = true;
        }

        work_ptr += pos;
    } while (!is_last);

    AIC_ASSERT(ascii_pos <= strlen(utf8_str));
    aos_memcpy(utf8_str, g_ascii_path_buf, ascii_pos);
    utf8_str[ascii_pos] = '\0';

exit:
    if (is_chdir) {
        char *cur_path;
        fs_info->media_dir_ops->local_path_get(fs_info, &cur_path);

        /* check to avoid double free */
        if (cur_path && local_path && (cur_path == local_path)) {
            if (path_handle) {
                aos_free((u8 *)(uintptr_t)path_handle);
            }
        } else {
            fs_info->media_dir_ops->local_path_clear(fs_info);
        }

        if (local_path) {
            fs_info->media_dir_ops->local_path_restore(fs_info, local_path);
        }
    }

    return err;
}
#endif

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
int fxffs_errcode(int error)
{
    int errcode;

    if (error <= 0) {
        return error;
    }

    switch(error) {
        case FX_NO_MORE_SPACE:
            errcode = AIC_ENOSPC;
            break;
        case FX_NOT_FOUND:
        case FX_NO_MORE_ENTRIES:
            errcode = AIC_ENOENT;
            break;
        case FX_INVALID_OPTION:
            errcode = AIC_EINVAL;
            break;
        case FX_DIR_NOT_EMPTY:
            errcode = AIC_EEXIST;
            break;
        default:
            errcode = AIC_EIO;
            break;
    }

    return errcode;
}

int fxffs_mount(struct vfs_inode *mp, unsigned long mountflags,
                const void *data)
{
    FXFFS_INFO *fs_info = NULL;
    u32 ret = FX_SUCCESS;

    if ((strlen(FS_FXFAT_TYPE_NAME) != strlen(mp->ops.fstypeops->type)) ||
        (memcmp(FS_FXFAT_TYPE_NAME,
                mp->ops.fstypeops->type, strlen(mp->ops.fstypeops->type)))) {
#ifdef FX_ENABLE_EXFAT
        if ((strlen(FS_FXEXFAT_TYPE_NAME) != strlen(mp->ops.fstypeops->type)) ||
            (memcmp(FS_FXEXFAT_TYPE_NAME,
                    mp->ops.fstypeops->type, strlen(mp->ops.fstypeops->type))))
#endif
        {
            return AIC_EINVAL;
        }
    }

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        fs_info = (FXFFS_INFO *)aos_zalloc(sizeof(FXFFS_INFO));

        fs_info->mutex = vfs_create_mutex(mp->name);

        fs_info->cfg = (FS_CFG *)data;
        fs_info->volume_name = mp->name;
        fs_info->dev_ops = mp->ops.blkdevops;
        ret = fxffs_media_init(fs_info);

        if (ret != FX_SUCCESS) {
            vfs_delete_mutex(fs_info->mutex);
            aos_free(fs_info);
        }
    } else if (!fs_info->initd) {
        vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

        fs_info->cfg = (FS_CFG *)data;
        fs_info->dev_ops = mp->ops.blkdevops;
        ret = fxffs_media_init(fs_info);

        vfs_put_mutex(fs_info->mutex);
    }

    if (ret == FX_SUCCESS) {
        mp->ops.fstypehandle = (u32)fs_info;
        mp->ops.priv = &fs_info->media_handle;
        mp->ops.devhandle = (u32)&fs_info->part_info;
    }

    return fxffs_errcode(ret);
}

int fxffs_umount(struct vfs_inode *mp)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;

    if ((strlen(FS_FXFAT_TYPE_NAME) != strlen(mp->ops.fstypeops->type)) ||
        (memcmp(FS_FXFAT_TYPE_NAME,
                mp->ops.fstypeops->type, strlen(mp->ops.fstypeops->type)))) {
#ifdef FX_ENABLE_EXFAT
        if ((strlen(FS_FXEXFAT_TYPE_NAME) != strlen(mp->ops.fstypeops->type)) ||
            (memcmp(FS_FXEXFAT_TYPE_NAME,
                    mp->ops.fstypeops->type, strlen(mp->ops.fstypeops->type))))
#endif
        {
            return AIC_EINVAL;
        }
    }

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        return AIC_EINVAL;
    }

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    fs_info->media_dir_ops->local_path_clear(fs_info);

    fs_info->media_dir_ops->default_set(fs_info, FS_ROOT_PATH);

    ret = fxffs_media_uninit(fs_info);

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_umount2(struct vfs_inode *mp, int flag)
{
    return AIC_ENOSYS;
}

int fxffs_statfs(struct vfs_inode *mp, struct statfs *buf)
{
    u32 sectors_per_cluster, bytes_per_sector;
    u64 free_bytes, total_bytes;
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(buf, 0x0, sizeof(struct statfs));

    do {
        ret = fs_info->media_ops->flush(fs_info);
        if (ret != FX_SUCCESS)
            break;

        ret = fs_info->media_ops->extended_space_available(fs_info, &free_bytes,
                                                            &total_bytes,
                                                            &sectors_per_cluster,
                                                            &bytes_per_sector);
        if (ret != FX_SUCCESS)
            break;

        buf->f_type = FXFFS_TYPE;
        buf->f_bsize = bytes_per_sector * sectors_per_cluster;
        buf->f_bfree = (long)(free_bytes / (u64)buf->f_bsize);
        buf->f_bavail = buf->f_bfree;
        buf->f_blocks = (long)(total_bytes / (u64)buf->f_bsize);
        buf->f_namelen = FX_MAXIMUM_PATH;
    } while(0);

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_open(struct vfs_file *file, const char *path, int oflags,
               mode_t mode)
{
    u32 ret = FX_SUCCESS, filehandle;
    aos_systime systime = {0};
    FXFFS_INFO *fs_info;
    ulong size;
    u32 attri;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';

    do {
#ifdef CONFIG_FX_UTF8_SUPPORT
        bool need_unicode;
        int unicode_len;

        ret = path_to_ascii(fs_info, fs_info->cache_buf, &need_unicode, &unicode_len);
        if (ret) {
            break;
        }

        if (!need_unicode) {
            /* File found, try open. */
            ret = fs_info->media_file_ops->open(fs_info, &filehandle, (char *)fs_info->cache_buf, oflags);
        } else {
            /* Set not found return value */
            ret = FX_NOT_FOUND;
        }
#else
        ret = fs_info->media_file_ops->open(fs_info, &filehandle,
                                            (char *)fs_info->cache_buf, oflags);
#endif

        if (ret != FX_SUCCESS) {
            if (oflags & O_CREAT) {
#ifdef CONFIG_FX_UTF8_SUPPORT
                if (need_unicode) {
                    char *local_path = NULL;
                    u32 str_len = 0, path_handle = 0;
                    char temp_short_name[FX_MAX_SHORT_NAME_LEN];

                    if (fs_info->cache_buf[0] != '\0') {
                        ret = fs_info->media_dir_ops->local_path_get(fs_info, &local_path);
                        AOS_ASSERT(ret == FX_SUCCESS);
                        ret = fs_info->media_dir_ops->local_path_set(fs_info, &path_handle, fs_info->cache_buf);
                        AOS_ASSERT(ret == FX_SUCCESS);
                    }

                    aos_memset(temp_short_name, 0, FX_MAX_SHORT_NAME_LEN);
                    ret = fs_info->media_unicode_ops->file_create(fs_info, fs_info->last_dir_buf, unicode_len, temp_short_name);
                    if (ret == FX_SUCCESS) {
                        str_len = strlen(fs_info->cache_buf);
                        fs_info->cache_buf[str_len] = '/';
                        aos_memcpy(&fs_info->cache_buf[str_len + 1], temp_short_name, strlen(temp_short_name));
                        fs_info->cache_buf[str_len + 1 + strlen(temp_short_name)] = '\0';
                    }

                    if (path_handle) {
                        char *cur_path;
                        fs_info->media_dir_ops->local_path_get(fs_info, &cur_path);

                        /* check to avoid double free */
                        if (cur_path && local_path && (cur_path == local_path)) {
                            aos_free((u8 *)(uintptr_t)path_handle);
                        } else {
                            fs_info->media_dir_ops->local_path_clear(fs_info);
                        }

                        fs_info->media_dir_ops->local_path_clear(fs_info);
                        if (local_path) {
                            fs_info->media_dir_ops->local_path_restore(fs_info, local_path);
                        }
                    }
                } else {
                    ret = fs_info->media_file_ops->create(fs_info, (char *)fs_info->cache_buf);
                }

                if (ret == FX_SUCCESS) {
                    ret = fs_info->media_file_ops->open(fs_info, &filehandle,
                                                (char *)fs_info->cache_buf, oflags);
                }
#else
                ret = fs_info->media_file_ops->create(fs_info, (char *)fs_info->cache_buf);
                if (ret == FX_SUCCESS) {
                    ret = fs_info->media_file_ops->open(fs_info, &filehandle,
                                            (char *)fs_info->cache_buf, oflags);
                }
#endif
            }
            break;
        } else {
            ret = fs_info->media_file_ops->relative_seek(filehandle, 0, FX_SEEK_BEGIN);
            if (ret == FX_SUCCESS) {
                file->pos = 0;
            }
        }

        if ((oflags & O_CREAT) && (oflags & O_EXCL)) {
            fs_info->media_file_ops->close(filehandle);
            ret = FX_NOT_AVAILABLE;
            break;
        }

        if (oflags & O_TRUNC) {
            ret = fs_info->media_file_ops->truncate(filehandle, 0);
            if (ret != FX_SUCCESS) {
                fs_info->media_file_ops->close(filehandle);
            } else {
                file->pos = 0;
            }
            break;
        }
    } while(0);

    if (ret == FX_SUCCESS) {
        file->priv = (void *)filehandle;
#ifdef CONFIG_FX_UTF8_SUPPORT
        ret = fs_info->media_dir_ops->info_get(fs_info, fs_info->cache_buf,
                                                &attri, &size, &systime);
#else
        ret = fs_info->media_dir_ops->info_get(fs_info,
                                                file->fullpath + strlen("/") +
                                                strlen(fs_info->volume_name),
                                                &attri, &size, &systime);
#endif
        if (ret == FX_SUCCESS) {
            file->size = size;
        }
    } else {
        aicx_error(0x160ad8, "sxx", "Error:[FXOPEN] file(%s) oflags(0x%x) open error, ret(0x%x)!\n",
                   file->fullpath, oflags, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_close(struct vfs_file *file)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    filehandle = (u32)file->priv;
    ret= fs_info->media_file_ops->close(filehandle);
    file->priv = NULL;
    file->pos = 0;

    if (ret != FX_SUCCESS) {
        aicx_error(0x0a853f, "sx", "Error:[FXCLOSE] file(%s) close error, ret(0x%x)!\n",
                   file->fullpath, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

ssize_t fxffs_read(struct vfs_file *file, char *buffer, size_t buflen)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;
    ulong roffset;
    size_t rsize;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    filehandle = (u32)file->priv;
    if ((u32)file->pos >= file->size) {
        ret = FX_END_OF_FILE;
    } else {
        ret = fs_info->media_file_ops->relative_seek(filehandle, file->pos, FX_SEEK_BEGIN);
        if (ret == FX_SUCCESS) {
            ret = fs_info->media_file_ops->read(filehandle, buffer, buflen,
                                                (ulong *)&rsize);
        }
    }

    if (ret == FX_SUCCESS) {
        fs_info->media_file_ops->offset_get(filehandle, &roffset);
        file->pos = roffset;
    }

    if (ret != FX_SUCCESS && ret != FX_END_OF_FILE) {
        aicx_error(0x04e387, "xx", "Error:[FXREAD] file(0x%x) read error, ret(0x%x)!\n",
                   file->fullpath, ret);
        rsize = fxffs_errcode(ret);
    } else if (ret == FX_END_OF_FILE) {
        rsize = 0;
    }

    vfs_put_mutex(fs_info->mutex);

    return rsize;
}

ssize_t fxffs_write(struct vfs_file *file, const char *buffer, size_t buflen)
{
    u32 ret = FX_SUCCESS, filehandle, wsize, tsize;
    FXFFS_INFO *fs_info;
    ulong roffset;
    size_t rsize;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

#ifdef CONFIG_CUSTOMER_CM_AT
    do {
        filehandle = (u32)file->priv;
        if ((u32)file->pos >= file->size) {
            ret = fs_info->media_file_ops->relative_seek(filehandle, 0, FX_SEEK_END);
            if (ret != FX_SUCCESS)
                break;

            tsize = file->pos - file->size;
            aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
            while (tsize) {
                wsize = FS_CACHE_LEN > tsize ? tsize : FS_CACHE_LEN;
                ret = fs_info->media_file_ops->write(filehandle, fs_info->cache_buf, wsize);
                if (ret != FX_SUCCESS)
                    break;
                tsize -= wsize;
            }
        } else if ((file->oflags & O_APPEND) == O_APPEND) {
            ret = fs_info->media_file_ops->relative_seek(filehandle, 0, FX_SEEK_END);
        } else {
            ret = fs_info->media_file_ops->relative_seek(filehandle, file->pos, FX_SEEK_BEGIN);
        }
        if (ret != FX_SUCCESS)
            break;

        ret = fs_info->media_file_ops->write(filehandle, (void *)buffer, buflen);
        if (ret == FX_SUCCESS) {
            fs_info->media_file_ops->offset_get(filehandle, &roffset);
            file->pos = roffset;
            if (file->pos > file->size) {
                file->size = file->pos;
            }
        }
    } while(0);
#else
    do {
        filehandle = (u32)file->priv;
        if ((file->oflags & O_APPEND) == O_APPEND) {
            ret = fs_info->media_file_ops->relative_seek(filehandle, 0, FX_SEEK_END);
        } else if ((u32)file->pos >= file->size) {
            ret = fs_info->media_file_ops->relative_seek(filehandle, 0, FX_SEEK_END);
            if (ret != FX_SUCCESS)
                break;

            tsize = file->pos - file->size;
            aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
            while (tsize) {
                wsize = FS_CACHE_LEN > tsize ? tsize : FS_CACHE_LEN;
                ret = fs_info->media_file_ops->write(filehandle, fs_info->cache_buf, wsize);
                if (ret != FX_SUCCESS)
                    break;
                tsize -= wsize;
            }
        } else {
            ret = fs_info->media_file_ops->relative_seek(filehandle, file->pos, FX_SEEK_BEGIN);
        }
        if (ret != FX_SUCCESS)
            break;

        ret = fs_info->media_file_ops->write(filehandle, (void *)buffer, buflen);
        if (ret == FX_SUCCESS) {
            fs_info->media_file_ops->offset_get(filehandle, &roffset);
            file->pos = roffset;
            if (file->pos > file->size) {
                file->size = file->pos;
            }
        }
    } while(0);
#endif

    if (ret != FX_SUCCESS) {
        aicx_error(0x05dc73, "sx", "Error:[FXWRITE] file(%s) write error, ret(0x%x)!\n",
                   file->fullpath, ret);
        rsize = fxffs_errcode(ret);
    } else {
        rsize = buflen;
    }

    vfs_put_mutex(fs_info->mutex);

    return rsize;
}

off_t fxffs_lseek(struct vfs_file *file, off_t offset, int whence)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;
    off_t pos;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    do {
        filehandle = (u32)file->priv;
        pos = file->pos;
        if (whence == SEEK_SET) {
            if (offset >= 0) {
                pos = offset;
            } else {
                ret = FX_INVALID_OPTION;
                break;
            }
        } else if (whence == SEEK_CUR) {
            pos += offset;
            if (pos < 0) {
                ret = FX_INVALID_OPTION;
                break;
            }
        } else if (whence == SEEK_END) {
            pos = file->size + offset;
            if (pos < 0) {
                ret = FX_INVALID_OPTION;
                break;
            }
        } else {
            ret = FX_INVALID_OPTION;
        }
        file->pos = pos;
    } while(0);

    if (ret != FX_SUCCESS) {
        aicx_error(0x12fb84, "slddx", "Error:[FXLSEEK] file(%s) seek offset(%ld) whence(%d) error, ret(0x%x)!\n",
                   file->fullpath, offset, whence, ret);
        pos =  fxffs_errcode(ret);
    } else {
        pos = file->pos;
    }

    vfs_put_mutex(fs_info->mutex);

    return pos;
}

int fxffs_truncate(struct vfs_file *file, off_t length)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;
    ulong roffset;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    filehandle = (u32)file->priv;
    ret = fs_info->media_file_ops->truncate(filehandle, length);

    if (ret == FX_SUCCESS) {
        fs_info->media_ops->flush(fs_info);
        fs_info->media_file_ops->offset_get(filehandle, &roffset);
        file->pos = roffset;
    }

    if (ret != FX_SUCCESS) {
        aicx_error(0x0c8b35, "sldx", "Error:[FXTRUNCATE] file(%s) truncate length(%ld) error, ret(0x%x)!\n",
                   file->fullpath, length, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_ioctl(struct vfs_file *file, int cmd, void *arg)
{
    return AIC_ENOSYS;
}

int fxffs_sync(struct vfs_file *file)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    ret = fs_info->media_ops->flush(fs_info);

    if (ret != FX_SUCCESS) {
        aicx_error(0x1907b7, "sx", "Error:[FXSYNC] file(%s) flush error, ret(0x%x)!\n",
                   file->fullpath, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_fstat(struct vfs_file *file, struct stat * buf)
{
    FXFFS_INFO *fs_info = NULL;
    aos_systime systime = {0};
    boolean rootdir = false;
    u32 ret = FX_SUCCESS;
    ulong size = 0;
    u32 attri;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    if ((strlen("/") == strlen(file->fullpath)) &&
        !memcmp("/", file->fullpath, strlen(file->fullpath))) {
        buf->st_mode &= ~S_IFREG;
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        attri = FX_DIRECTORY;
        rootdir = true;
    } else {
        do {
            ret = fs_info->media_ops->flush(fs_info);
            if (ret != FX_SUCCESS)
                break;

#ifdef CONFIG_FX_UTF8_SUPPORT
            bool need_unicode;
            int unicode_len;

            aos_memset(fs_info->cache_buf, 0, FS_CACHE_LEN);
            strcpy(fs_info->cache_buf, file->fullpath + strlen("/") + strlen(fs_info->volume_name));

            ret = path_to_ascii(fs_info, fs_info->cache_buf, &need_unicode, &unicode_len);
            if (ret != FX_SUCCESS) {
                break;
            } else if (need_unicode){
                ret = FX_NOT_AVAILABLE;
                break;
            } else {
                ret = fs_info->media_dir_ops->info_get(fs_info,
                                                        fs_info->cache_buf,
                                                        &attri, &size, &systime);
            }
#else
            ret = fs_info->media_dir_ops->info_get(fs_info,
                                                    file->fullpath + strlen("/") +
                                                    strlen(fs_info->volume_name),
                                                    &attri, &size, &systime);
#endif
            if (ret != FX_SUCCESS)
                break;
        } while(0);
    }

    if (ret == FX_SUCCESS) {
        aos_memset(buf, 0x0, sizeof(struct stat));
        buf->st_mode  = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                        S_IWUSR | S_IWGRP | S_IWOTH;
        if (attri & FX_DIRECTORY) {
            buf->st_mode &= ~S_IFREG;
            buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        }
        buf->st_size    = size;
#ifndef WIN32
        if (!rootdir) {
            buf->st_mtim.tv_sec = rtc_tm_to_days(
                                (struct rtc_time *)&systime) * 24 * 3600 +
                                systime.hour*3600 + systime.minute*60 +
                                systime.second;
        }
#endif
    } else {
        aicx_error(0x09d7fc, "sx", "Error:[FXFSTAT] file(%s) get fstat error, ret(0x%x)!\n",
                   file->fullpath, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_dup(const struct vfs_file *oldfile, struct vfs_file *newfile)
{
    return AIC_ENOSYS;
}

int fxffs_opendir(struct vfs_dir *dir, const char *path)
{
    struct fxffs_dirent_s *fxffs_dent;
    FXFFS_INFO *fs_info;
    u32 ret;

    fs_info = (FXFFS_INFO *)(dir->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    fxffs_dent = aos_zalloc(sizeof(struct fxffs_dirent_s));

    do {
        snprintf(fxffs_dent->path, FS_CACHE_LEN, "/%s", path);
#ifdef CONFIG_FX_UTF8_SUPPORT
        bool need_unicode;
        int unicode_len;
        ret = path_to_ascii(fs_info, fxffs_dent->path, &need_unicode, &unicode_len);
        if (ret != FX_SUCCESS) {
            break;
        } else if (need_unicode){
            ret = FX_NOT_AVAILABLE;
            break;
        }
#endif
        if ((strlen("/") != strlen(fxffs_dent->path)) ||
            memcmp("/", fxffs_dent->path, strlen(fxffs_dent->path))) {
            ret = fs_info->media_dir_ops->name_test(fs_info, fxffs_dent->path);
            if (ret != FX_SUCCESS) {
                break;
            }
        }

        ret = fs_info->media_dir_ops->local_path_get(fs_info,
                                                     &fxffs_dent->default_dir);
        if (ret != FX_SUCCESS) {
            break;
        }

        ret = fs_info->media_dir_ops->local_path_set(fs_info,
                                                    &fxffs_dent->local_handle,
                                                    fxffs_dent->path);
        if (ret != FX_SUCCESS) {
            break;
        }

        dir->thread = aos_identify_thread();
    } while(0);

    dir->dir.__entry_ptr = (int)fs_info->media_dir_ops->first_full_entry_find;
    dir->priv = (void *)fxffs_dent;

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_readdir(struct vfs_dir *dir, struct dirent *dirent)
{
    FXFFS_FIND_FULL_ENTRY findentry;
    aos_systime systime = {0};
    char *find_name = NULL;
    u32 attributes, ret;
    FXFFS_INFO *fs_info;
    ulong size;

    if (!dir || (dir->thread != aos_identify_thread())) {
        return AIC_EINVAL;
    }

    fs_info = (FXFFS_INFO *)(dir->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    find_name = (char *)aos_zalloc(FS_CACHE_LEN + 1);

    while(1) {
        findentry = (FXFFS_FIND_FULL_ENTRY)dir->dir.__entry_ptr;
        ret = findentry(fs_info, find_name, &attributes, &size, &systime);
        if (ret != FX_SUCCESS) {
            break;
        } else if (!find_name[0]) {
            ret = FX_NO_MORE_ENTRIES;
            break;
        }

        dir->dir.__entry_ptr = (int)fs_info->media_dir_ops->next_full_entry_find;

        if (((strlen(".") == strlen(find_name)) &&
             !memcmp(".", find_name, strlen(find_name))) ||
            ((strlen("..") == strlen(find_name)) &&
             !memcmp("..", find_name, strlen(find_name))) ||
            attributes & FX_SYSTEM) {
            continue;
        }

#ifdef CONFIG_FX_UTF8_SUPPORT
        /**
         * First check if is short name.
         */
        ulong uni_len;
        int result;

        aos_memset(fs_info->cache_buf, 0, FS_CACHE_LEN);
        ret = fs_info->media_unicode_ops->name_get_ext(fs_info, find_name,
                                                       fs_info->cache_buf, &uni_len, FS_CACHE_LEN);
        if (ret == FX_SUCCESS) {
            result = aic_unicode_to_utf8((uint16_t *)fs_info->cache_buf, -1, find_name, FS_CACHE_LEN);
            if (result == 0) {
                ret = AIC_ENOEXEC;
                break;
            }
            find_name[FS_CACHE_LEN] = '\0';
        } else if (ret == FX_NOT_FOUND) {
            ret = FX_SUCCESS;
        }
#endif

        aos_memcpy(dirent->d_name, find_name, strlen(find_name));

        dirent->d_type = attributes & FX_DIRECTORY ? DT_DIR : DT_REG;
        dirent->d_off = dir->pos;
        dirent->d_reclen = ALIGN((sizeof(struct dirent) -
                                    sizeof(dirent->d_name) +
                                    strlen(dirent->d_name) + 1),
                                    4);
        dir->pos += (off_t)1;
        break;
    }

    if (find_name)
        aos_free(find_name);

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_seekdir(struct vfs_dir *dir, long offset)
{
    FXFFS_FIND_FULL_ENTRY findentry;
    aos_systime systime = {0};
    char *find_name = NULL;
    u32 attributes, ret;
    FXFFS_INFO *fs_info;
    ulong size;

    fs_info = (FXFFS_INFO *)(dir->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    find_name = aos_zalloc(NAME_MAX);

    while (dir->pos < offset) {
        findentry = (FXFFS_FIND_FULL_ENTRY)dir->dir.__entry_ptr;
        ret = findentry(fs_info, find_name, &attributes, &size, &systime);
        if (ret != FX_SUCCESS) {
            break;
        } else if (!find_name[0]) {
            ret = FX_NO_MORE_ENTRIES;
            break;
        }

        if ('.' == find_name[0]) {
            continue;
        }

        dir->dir.__entry_ptr = (int)fs_info->media_dir_ops->next_full_entry_find;
        dir->pos++;
    }

    if (find_name)
        aos_free(find_name);

    if (ret != FX_SUCCESS) {
        aicx_error(0x0f7e52, "sdx", "Error:[FXSEEKDIR] %s seek %d error, ret(0x%x)!\n",
                    dir->inode->name, offset, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

long fxffs_telldir(struct vfs_dir *dir)
{
    FXFFS_INFO *fs_info;
    off_t pos;

    fs_info = (FXFFS_INFO *)(dir->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    pos = dir->pos;

    vfs_put_mutex(fs_info->mutex);

    return pos;
}

int fxffs_rewinddir(struct vfs_dir *dir)
{
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(dir->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    dir->dir.__entry_ptr = (int)fs_info->media_dir_ops->first_full_entry_find;
    dir->pos = 0;

    vfs_put_mutex(fs_info->mutex);

    return AIC_SUCCESS;
}

int fxffs_closedir(struct vfs_dir *dir)
{
    struct fxffs_dirent_s *fxffs_dent;
    FXFFS_INFO *fs_info;
    u32 ret;

    if (!dir || (dir->thread != aos_identify_thread())) {
        return AIC_EINVAL;
    }

    fs_info = (FXFFS_INFO *)(dir->inode->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    fxffs_dent = (struct fxffs_dirent_s *)dir->priv;
    if (!fxffs_dent) {
        vfs_put_mutex(fs_info->mutex);
        return AIC_EINVAL;
    }

    do {
        ret = fs_info->media_dir_ops->local_path_clear(fs_info);
        if (ret != FX_SUCCESS) {
            break;
        }

        if (fxffs_dent->default_dir) {
            ret = fs_info->media_dir_ops->local_path_restore(fs_info,
                                                             fxffs_dent->default_dir);
            if (ret != FX_SUCCESS) {
                break;
            }
        }
    } while(0);

    aos_free(fxffs_dent);
    dir->priv = NULL;

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_unlink(struct vfs_inode *mp, const char *path)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';

    do {
#ifdef CONFIG_FX_UTF8_SUPPORT
        bool need_unicode;
        int unicode_len;

        ret = path_to_ascii(fs_info, fs_info->cache_buf, &need_unicode, &unicode_len);
        if (ret != FX_SUCCESS || need_unicode) {
            break;
        }
#endif
        if ((strlen("/") == strlen(fs_info->cache_buf)) &&
            !memcmp("/", fs_info->cache_buf, strlen(fs_info->cache_buf))) {
            ret = FX_NOT_AVAILABLE;
            break;
        }

        ret = fs_info->media_dir_ops->name_test(fs_info,(char *)fs_info->cache_buf);
        if (ret != FX_SUCCESS)
            ret = fs_info->media_file_ops->delete(fs_info, (char *)fs_info->cache_buf);
        else
            ret = fs_info->media_dir_ops->delete(fs_info, (char *)fs_info->cache_buf);

        if (ret != FX_SUCCESS) {
            break;
        }

        ret = fs_info->media_ops->flush(fs_info);
        if (ret != FX_SUCCESS) {
            break;
        }
    } while(0);

    if (ret != FX_SUCCESS) {
        aicx_error(0x1c678f, "sx", "Error:[FXUNLINK] path(%s) unlink error, ret(0x%x)!\n",
                   path, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

static void fxffs_dirpathsearch(const char *path, u32 search_pos, u32 *pos)
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

int fxffs_mkdir(struct vfs_inode *mp, const char *path)
{
    boolean dirname_test = AOS_TRUE;
    u32 ret = FX_SUCCESS, pos = 0;
    char *createdir = NULL;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';

    do {
        if ((strlen("/") == strlen(fs_info->cache_buf)) &&
            !memcmp("/", fs_info->cache_buf, strlen(fs_info->cache_buf))) {
            ret = FX_SUCCESS;
            break;
        }

#ifdef CONFIG_FX_UTF8_SUPPORT
        u32 last_pos, path_handle = 0, uni_len;
        bool need_unicode, dirrestore = false;
        char *local_path = NULL;

        createdir = aos_zalloc(strlen(fs_info->cache_buf) + 2);

        ret = fs_info->media_dir_ops->local_path_get(fs_info, &local_path);
        AOS_ASSERT(ret == FX_SUCCESS);

        char temp_short_name[FX_MAX_SHORT_NAME_LEN];

        if (local_path) {
            /* Not at root path: force chdir to root path. */
            ret = fs_info->media_dir_ops->local_path_set(fs_info, &path_handle, "/");
            AOS_ASSERT(ret == FX_SUCCESS);
            dirrestore = true;
        }

        while (pos < strlen(fs_info->cache_buf)) {
            /* bypass slashes */
            do {
                pos++;
            } while (fs_info->cache_buf[pos] == '/');

            last_pos = pos;
            fxffs_dirpathsearch(fs_info->cache_buf, last_pos, &pos);
            aos_memcpy(createdir, fs_info->cache_buf + last_pos, pos - last_pos);
            createdir[pos - last_pos] = '\0';

            if (ascii_only(createdir, pos - last_pos)) {
                need_unicode = false;
            } else {
                need_unicode = true;
            }

            if (dirname_test) {
                if (!need_unicode) {
                    ret = fs_info->media_dir_ops->name_test(fs_info, createdir);
                } else {
                    aos_memset(temp_short_name, 0, FX_MAX_SHORT_NAME_LEN);
                    uni_len = aic_utf8_to_unicode(createdir, pos - last_pos, (uint16_t *)fs_info->last_dir_buf, PATH_MAX * 2);
                    ret = fs_info->media_unicode_ops->short_name_get_ext(
                                fs_info,
                                fs_info->last_dir_buf,
                                uni_len,
                                temp_short_name,
                                FX_MAX_SHORT_NAME_LEN);
                }

                if (ret != FX_SUCCESS) {
                    dirname_test = AOS_FALSE;
                }
            }

            if (!dirname_test) {
                if (need_unicode) {
                    aos_memset(temp_short_name, 0, FX_MAX_SHORT_NAME_LEN);
                    uni_len = aic_utf8_to_unicode(createdir, pos - last_pos, (uint16_t *)fs_info->last_dir_buf, PATH_MAX * 2);
                    ret = fs_info->media_unicode_ops->dir_create(fs_info, fs_info->last_dir_buf, uni_len, temp_short_name);
                } else {
                    ret = fs_info->media_dir_ops->create(fs_info, createdir);
                }

                if (ret != FX_SUCCESS) {
                    break;
                }
            }

            if (need_unicode) {
                ret = fs_info->media_dir_ops->local_path_set(fs_info, &path_handle, temp_short_name);
            } else {
                ret = fs_info->media_dir_ops->local_path_set(fs_info, &path_handle, createdir);
            }
            AOS_ASSERT(ret == FX_SUCCESS);

            dirrestore = true;
        };

        if (dirrestore) {
            char *cur_path;
            fs_info->media_dir_ops->local_path_get(fs_info, &cur_path);

            /* check to avoid double free */
            if (cur_path && local_path && (cur_path == local_path)) {
                if (path_handle) {
                    aos_free((u8 *)(uintptr_t)path_handle);
                }
            } else {
                fs_info->media_dir_ops->local_path_clear(fs_info);
            }

            if (local_path) {
                fs_info->media_dir_ops->local_path_restore(fs_info, local_path);
            }
        }
#else
        ret = fs_info->media_dir_ops->name_test(fs_info, fs_info->cache_buf);
        if (ret == FX_SUCCESS) {
            break;
        }

        createdir = aos_zalloc(strlen(fs_info->cache_buf) + 2);
        if (fs_info->cache_buf[0] == '/')
            pos = 1;
        else
            pos = 0;
        while (pos < strlen(fs_info->cache_buf)) {
            fxffs_dirpathsearch(fs_info->cache_buf, pos, &pos);

            aos_memcpy(createdir, fs_info->cache_buf, pos);
            if (dirname_test) {
                ret = fs_info->media_dir_ops->name_test(fs_info, createdir);
                if (ret != FX_SUCCESS) {
                    dirname_test = AOS_FALSE;
                }
            }
            if (!dirname_test) {
                ret = fs_info->media_dir_ops->create(fs_info, createdir);
                if (ret != FX_SUCCESS) {
                    break;
                }
            }

            ret = fs_info->media_dir_ops->name_test(fs_info, createdir);
            AOS_ASSERT(ret == FX_SUCCESS);
            pos += 1;
        };
#endif
    } while(0);

    if (createdir)
        aos_free(createdir);

    if (ret != FX_SUCCESS) {
        aicx_error(0x12e7f5, "sx", "Error:[FXMKDIR] create path(%s) error, ret(0x%x)!\n",
                   path, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_rmdir(struct vfs_inode *mp, const char *path)
{
    return fxffs_unlink(mp, path);
}

int fxffs_rename(struct vfs_inode *mp, const char *oldpath, const char *newpath)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;
    char *npath;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", oldpath);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';

    npath = aos_zalloc(strlen(newpath) + 4);
    snprintf(npath, strlen(newpath) + 4, "/%s", newpath);

    do {
#ifdef CONFIG_FX_UTF8_SUPPORT
        bool need_unicode;
        int unicode_len;
        char temp_short_name[FX_MAX_SHORT_NAME_LEN];
        int errcode;

        ret = path_to_ascii(fs_info, fs_info->cache_buf, &need_unicode, &unicode_len);
        if (ret || need_unicode) {
            ret = FX_NOT_FOUND;
            break;
        }

        ret = path_to_ascii(fs_info, npath, &need_unicode, &unicode_len);

        ret = fs_info->media_dir_ops->name_test(fs_info, (char *)fs_info->cache_buf);

        if (need_unicode) {
            u32 path_handle = 0;
            char *local_path = NULL;

            if (npath[0] != '\0') {
                errcode = fs_info->media_dir_ops->local_path_get(fs_info, &local_path);
                AOS_ASSERT(errcode == FX_SUCCESS);
                errcode = fs_info->media_dir_ops->local_path_set(fs_info, &path_handle, npath);
                AOS_ASSERT(errcode == FX_SUCCESS);
            }

            static const char *temp_file_name = "FXFFS_RENAME_OPS_ONGOING.TEMP";

            if (ret != FX_SUCCESS)
                errcode = fs_info->media_file_ops->rename(fs_info,
                                                    (char *)fs_info->cache_buf,
                                                    (char *)temp_file_name);
            else
                errcode = fs_info->media_dir_ops->rename(fs_info,
                                                    (char *)fs_info->cache_buf,
                                                    (char *)temp_file_name);

            if (!errcode) {
                ulong temp_uni_len = aic_utf8_to_unicode(temp_file_name, strlen(temp_file_name), (uint16_t *)fs_info->cache_buf, FS_CACHE_LEN);
                AOS_ASSERT(temp_uni_len != 0);

                if (ret != FX_SUCCESS) {
                    ret = fs_info->media_unicode_ops->file_rename(fs_info,
                                                        (char *)fs_info->cache_buf, temp_uni_len,
                                                        (char *)fs_info->last_dir_buf, unicode_len,
                                                        temp_short_name);
                } else {
                    ret = fs_info->media_unicode_ops->dir_rename(fs_info,
                                                        (char *)fs_info->cache_buf, temp_uni_len,
                                                        (char *)fs_info->last_dir_buf, unicode_len,
                                                        temp_short_name);
                }
            } else {
                ret = errcode;
            }

            if (path_handle) {
                char *cur_path;
                fs_info->media_dir_ops->local_path_get(fs_info, &cur_path);

                /* check to avoid double free */
                if (cur_path && local_path && (cur_path == local_path)) {
                    aos_free((u8 *)(uintptr_t)path_handle);
                } else {
                    fs_info->media_dir_ops->local_path_clear(fs_info);
                }

                if (local_path) {
                    fs_info->media_dir_ops->local_path_restore(fs_info, local_path);
                }
            }
        } else {
            if (ret != FX_SUCCESS)
                ret = fs_info->media_file_ops->rename(fs_info,
                                                    (char *)fs_info->cache_buf,
                                                    (char *)npath);
            else
                ret = fs_info->media_dir_ops->rename(fs_info,
                                                    (char *)fs_info->cache_buf,
                                                    (char *)npath);
        }
#else
        ret = fs_info->media_dir_ops->name_test(fs_info,
                                                (char *)fs_info->cache_buf);
        if (ret != FX_SUCCESS)
            ret = fs_info->media_file_ops->rename(fs_info,
                                                  (char *)fs_info->cache_buf,
                                                  (char *)npath);
        else
            ret = fs_info->media_dir_ops->rename(fs_info,
                                                 (char *)fs_info->cache_buf,
                                                 (char *)npath);
#endif
    } while(0);

    if (ret != FX_SUCCESS) {
        aicx_error(0x0cba12, "ssx", "Error:[FXRENAME] oldpath(%s) rename (%s) error, ret(0x%x)!\n",
                   oldpath, newpath, ret);
    }

    aos_free(npath);
    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_stat(struct vfs_inode *mp, const char *path, struct stat *buf)
{
    aos_systime systime = {0};
    boolean rootdir = false;
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;
    ulong size = 0;
    u32 attri;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(buf, 0x0, sizeof(struct stat));
    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';

    if ((strlen("/") == strlen(fs_info->cache_buf)) &&
        !memcmp("/", fs_info->cache_buf, strlen(fs_info->cache_buf))) {
        buf->st_mode &= ~S_IFREG;
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        attri = FX_DIRECTORY;
        rootdir = true;
    } else {
#ifdef CONFIG_FX_UTF8_SUPPORT
        bool need_unicode;
        int unicode_len;

        ret = path_to_ascii(fs_info, fs_info->cache_buf, &need_unicode, &unicode_len);
        if (ret != AIC_SUCCESS || need_unicode) {
            ret = FX_NOT_FOUND;
        } else {
#endif
        do {
            ret = fs_info->media_ops->flush(fs_info);
            if (ret != FX_SUCCESS)
                break;

            ret = fs_info->media_dir_ops->info_get(fs_info, (char *)fs_info->cache_buf,
                                                    &attri, &size, &systime);
            if (ret != FX_SUCCESS)
                break;
        } while(0);
#ifdef CONFIG_FX_UTF8_SUPPORT
        }
#endif
    }

    if (ret == FX_SUCCESS) {
        aos_memset(buf, 0x0, sizeof(struct stat));
        buf->st_mode  = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                        S_IWUSR | S_IWGRP | S_IWOTH;
        if (attri & FX_DIRECTORY) {
            buf->st_mode &= ~S_IFREG;
            buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        }
        buf->st_size = (off_t)size;
#ifndef WIN32
        if (!rootdir) {
            buf->st_mtim.tv_sec = rtc_tm_to_days(
                                (struct rtc_time *)&systime) * 24 * 3600 +
                                systime.hour*3600 + systime.minute*60 +
                                systime.second;
        }
#endif
    } else {
        aicx_error(0x09d7fc, "sx", "Error:[FXFSTAT] file(%s) get fstat error, ret(0x%x)!\n",
                   path, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_format(struct vfs_inode *mp)
{
    ulong mem_size, error;
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;
    u8 *mem_ptr = NULL;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    do {
        ret = fs_info->media_ops->close(fs_info);
        if (ret != FX_SUCCESS) {
            break;
        }

        ret = fs_info->media_ops->format(fs_info);
        if (ret != FX_SUCCESS) {
            aicx_error(0x19e927, "sx", "Error:[FXFMT] %s format error, ret(0x%x)!\n",
                       mp->name, ret);
            break;
        }

        ret = fs_info->media_ops->open(fs_info);
        if (ret != FX_SUCCESS) {
            aicx_error(0x0bd877, "sx", "Error:[FXFMT] %s open error, ret(0x%x)!\n",
                       mp->name, ret);
            break;
        }

        if (fs_info->cfg->check_eb) {
            mem_size = ROUND_UP(((fs_info->cfg->total_sectors / fs_info->cfg->sectors_per_cluster) >> 3) + 1,
                                fs_info->cfg->bytes_per_sector) + fs_info->cfg->bytes_per_sector * 4;
            mem_ptr = (u8 *)aos_zalloc(mem_size);
            ret = fs_info->media_ops->check(fs_info, mem_ptr, mem_size,
                                            FX_FAT_CHAIN_ERROR |
                                            FX_DIRECTORY_ERROR |
                                            FX_LOST_CLUSTER_ERROR,
                                            &error);
            aos_free(mem_ptr);
            if (ret != FX_SUCCESS) {
                aicx_error(0x01fc37, "sxx", "%s check error(0x%x), ret(0x%x).\n",
                            mp->name, error, ret);
                break;
            }
        }

        if (fs_info->cfg->fault_tolerant_eb) {
            ret = fs_info->media_ops->fault_tolerant_enable(fs_info);
            if (ret != FX_SUCCESS) {
                aicx_error(0x1adaf0, "sx", "%s tolerant enable fail, ret(0x%x).\n",
                            mp->name, ret);
                break;
            }
        }
    } while(0);

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

int fxffs_longname_get(struct vfs_inode *mp, char *short_name,
                        char *longname, u32 longname_len)
{
#if defined(CONFIG_ICONV_UTF8) || defined(WIN32)

    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;
    ulong unicode_len;
    int32_t result;
    int errcode = AIC_SUCCESS;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    aos_memset(fs_info->cache_buf, 0x0, FS_CACHE_LEN);
    ret = fs_info->media_unicode_ops->name_get(fs_info, short_name, fs_info->cache_buf,
                                                &unicode_len);

    if (ret != FX_SUCCESS) {
        vfs_put_mutex(fs_info->mutex);
        return AIC_EIO;
    }

    result = aic_unicode_to_utf8((uint16_t *)fs_info->cache_buf, -1,
                                longname, (int32_t)longname_len);

    if (result == 0) {
        errcode = AIC_ENOEXEC;
    }

    vfs_put_mutex(fs_info->mutex);

    return errcode;
#else
    return AIC_EPROTONOSUPPORT;
#endif
}

/**
 * fs_setfileattri - set file attribute.
 *
 * Parameter:
 * @file_path: input file path.
 * @attri: input file attribute.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fxffs_fileattri_set(struct vfs_inode *mp, const char *path, u32 attri)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    AIC_ASSERT(fs_info);

    vfs_get_mutex(fs_info->mutex, AOS_WAIT_FOREVER);

    snprintf(fs_info->cache_buf, FS_CACHE_LEN, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';

    if ((strlen("/") == strlen(fs_info->cache_buf)) &&
        !memcmp("/", fs_info->cache_buf, strlen(fs_info->cache_buf))) {
        ret = FX_SUCCESS;
    } else {
        ret = fs_info->media_dir_ops->name_test(fs_info, fs_info->cache_buf);
    }

    if (ret != FX_SUCCESS)
        ret = fs_info->media_file_ops->attri_set(fs_info, fs_info->cache_buf, attri);
    else
        ret = fs_info->media_dir_ops->attri_set(fs_info, fs_info->cache_buf, attri);

    if (ret != FX_SUCCESS) {
        aicx_error(0x1580b9, "sxx", "file(%s) attribute(0x%x) set error, ret(0x%x)!\n",
                   path, ret);
    }

    vfs_put_mutex(fs_info->mutex);

    return fxffs_errcode(ret);
}

struct vfs_type_mount_ops g_fxffs_mnt_ops = {
    fxffs_mount,
    fxffs_umount,
    fxffs_umount2,
    fxffs_statfs
};

struct vfs_type_file_ops g_fxffs_file_ops = {
    fxffs_open,
    fxffs_close,
    fxffs_read,
    fxffs_write,
    fxffs_lseek,
    fxffs_truncate,
    fxffs_ioctl,
    fxffs_sync,
    fxffs_fstat,
    fxffs_dup,
    fxffs_opendir,
    fxffs_readdir,
    fxffs_telldir,
    fxffs_seekdir,
    fxffs_rewinddir,
    fxffs_closedir,
    fxffs_unlink,
    fxffs_mkdir,
    fxffs_rmdir,
    fxffs_rename,
    fxffs_stat,
    fxffs_longname_get,
    fxffs_fileattri_set
};

struct vfs_type_management_ops g_fxffs_mgt_ops = {
    fxffs_format
};

int fxffs_init(void)
{
    int ret;

    ret = vfs_type_register(FS_FXFAT_TYPE_NAME, &g_fxffs_mnt_ops,
                            &g_fxffs_file_ops, &g_fxffs_mgt_ops);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0d1042, "sd", "fs register type %s failed: %d\n",
                   FS_FXFAT_TYPE_NAME, ret);
    }

#ifdef FX_ENABLE_EXFAT
    ret = vfs_type_register(FS_FXEXFAT_TYPE_NAME, &g_fxffs_mnt_ops,
                            &g_fxffs_file_ops, &g_fxffs_mgt_ops);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0d1042, "sd", "fs register type %s failed: %d\n",
                   FS_FXEXFAT_TYPE_NAME, ret);
    }
#endif

    return ret;
}

AIC_COMP_P1_INIT_EXPORT(fxffs_init);
