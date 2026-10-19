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
#define AIC_THIS_FILE    0x1bd826

#include "fxffs.h"
#include "initcall.h"

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Private Type
 **---------------------------------------------------------------------------*/
struct fxffs_dirent_s {
    u32 local_handle;
    char *default_dir;
    char path[PATH_MAX];
};

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
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
            log_print("Error:[FXMNT] mount type name(%s) (%s) is different.\n",
                       FS_FXEXFAT_TYPE_NAME, mp->name);
            return AIC_EINVAL;
        }
    }

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        fs_info = (FXFFS_INFO *)malloc(sizeof(FXFFS_INFO));
        ASSERT_INFO(fs_info,
                    "malloc size(0x%x) memory fail.\n", sizeof(FXFFS_INFO));
        memset(fs_info, 0x0, sizeof(FXFFS_INFO));

        fs_info->cfg = (FS_CFG *)data;
        fs_info->volume_name = mp->name;
        fs_info->dev_ops = mp->ops.blkdevops;
        ret = fxffs_media_init(fs_info);
    } else if (!fs_info->initd) {
        fs_info->cfg = (FS_CFG *)data;
        fs_info->dev_ops = mp->ops.blkdevops;
        ret = fxffs_media_init(fs_info);
    }
 
    if (ret != FX_SUCCESS) {
        if (fs_info)
            free(fs_info);
        log_print("Error:[FXMNT] (%s) fxffs mount fail, ret(0x%x)!\n",
                   mp->name, ret);
        return AIC_EIO;
    } else {
        mp->ops.fstypehandle = (u32)fs_info;
        mp->ops.priv = &fs_info->media_handle;
        mp->ops.devhandle = (u32)&fs_info->part_info;
    }

    return AIC_SUCCESS;
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
            log_print("Error:[FXUMNT] unmount type name(%s) (%s) is different.\n",
                       FS_FXEXFAT_TYPE_NAME, mp->name);
            return AIC_EINVAL;
        }
    }

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXUMNT] (%s) not mount.\n", mp->name);
        return AIC_EINVAL;
    }

    fs_info->media_dir_ops->local_path_clear(fs_info);

    fs_info->media_dir_ops->default_set(fs_info, FS_ROOT_PATH);

    ret = fxffs_media_uninit(fs_info);

    mp->ops.fstypehandle = 0;

    free(fs_info);

    if (ret != FX_SUCCESS) {
        log_print("Error:[FXUMNT] (%s) umount fail, ret(0x%x)!\n",
                   mp->name, ret);
        return AIC_EBUSY;
    }

    return AIC_SUCCESS;
}

int fxffs_statfs(struct vfs_inode *mp, struct statfs *buf)
{
    u32 sectors_per_cluster, bytes_per_sector;
    u64 free_bytes, total_bytes;
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXSTATFS] (%s) not mount.\n",
                   mp->ops.fstypeops->type);
        return AIC_EINVAL;
    }

    ret = fs_info->media_ops->extended_space_available(fs_info, &free_bytes,
                                                       &total_bytes,
                                                       &sectors_per_cluster,
                                                       &bytes_per_sector);

    if (ret != FX_SUCCESS) {
        log_print("Error:[FXSTATFS] %s get statfs error, ret(0x%x)!\n",
                   mp->ops.fstypeops->type, ret);
        return AIC_EIO;
    }

    memset(buf, 0x0, sizeof(struct statfs));
    buf->f_type = FXFFS_TYPE;
    buf->f_bsize = bytes_per_sector * sectors_per_cluster;
    buf->f_bfree = (long)(free_bytes / (u64)buf->f_bsize);
    buf->f_bavail = buf->f_bfree;
    buf->f_blocks = (long)(total_bytes / (u64)buf->f_bsize);
    buf->f_namelen = FX_MAXIMUM_PATH;

    return AIC_SUCCESS;
}

int fxffs_open(struct vfs_file *file, const char *path, int oflags,
               mode_t mode)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;
    ulong roffset;
    int errcode;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXOPEN] (%s) not mount.\n",
                   file->inode->name);
        return AIC_EINVAL;
    }

    memset(fs_info->cache_buf, 0x0, PATH_MAX);
    snprintf(fs_info->cache_buf, PATH_MAX, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';
    errcode = AIC_SUCCESS;

    do {
        ret = fs_info->media_file_ops->open(fs_info, &filehandle,
                                            (char *)fs_info->cache_buf, oflags);
        if (ret != FX_SUCCESS) {
            if (oflags & O_CREAT) {
                ret = fs_info->media_file_ops->create(fs_info, (char *)fs_info->cache_buf);

                if (ret == FX_SUCCESS)
                    ret = fs_info->media_file_ops->open(fs_info, &filehandle,
                                                        (char *)fs_info->cache_buf, oflags);
            }
            if (ret != FX_SUCCESS) {
                if (ret == FX_NO_MORE_SPACE) {
                    errcode = AIC_ENOSPC;
                } else if (ret == FX_NOT_FOUND) {
                    errcode = AIC_ENOENT;
                } else {
                    errcode = AIC_EIO;
                }
            }
            break;
        } else {
            ret = fs_info->media_file_ops->relative_seek(filehandle, 0, 0);
            if (ret != FX_SUCCESS) {
                errcode = AIC_EIO;
            } else {
                file->pos = 0;
            }
        }

        if ((oflags & O_CREAT) && (oflags & O_EXCL)) {
            fs_info->media_file_ops->close(filehandle);
            errcode =  AIC_ENOTSUP;
            break;
        }

        if (oflags & O_APPEND) {
            ret = fs_info->media_file_ops->relative_seek(filehandle, 0, SEEK_END);
            if (ret != FX_SUCCESS) {
                fs_info->media_file_ops->close(filehandle);
                errcode = AIC_EOVERFLOW;
            } else {
                fs_info->media_file_ops->offset_get(filehandle, &roffset);
                file->pos = roffset;
            }
            break;
        }

        if (oflags & O_TRUNC) {
            ret = fs_info->media_file_ops->truncate(filehandle, 0);
            if (ret != FX_SUCCESS) {
                fs_info->media_file_ops->close(filehandle);
                errcode = AIC_EIO;
            } else {
                file->pos = 0;
            }
            break;
        }
    } while(0);

    if (errcode == AIC_SUCCESS) {
        file->priv = (void *)filehandle;
    } else {
        log_print("Error:[FXOPEN] file(%s) open error, ret(0x%x)!\n",
                   file->fullpath, ret);
    }

    return errcode;
}

int fxffs_close(struct vfs_file *file)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXCLOSE] (%s) not mount.\n",
                   file->inode->name);
        return AIC_EINVAL;
    }

    filehandle = (u32)file->priv;
    ret= fs_info->media_file_ops->close(filehandle);
    file->priv = NULL;
    file->pos = 0;

    if (ret != FX_SUCCESS) {
        log_print("Error:[FXCLOSE] file(%s) close error, ret(0x%x)!\n",
                   file->fullpath, ret);
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

ssize_t fxffs_read(struct vfs_file *file, char *buffer, size_t buflen)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;
    ulong roffset;
    size_t rsize;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXREAD] (%s) not mount.\n",
                   file->inode->name);
        return AIC_EINVAL;
    }

    filehandle = (u32)file->priv;
    ret = fs_info->media_file_ops->read(filehandle, buffer, buflen,
                                        (ulong *)&rsize);

    if (ret == FX_SUCCESS) {
        fs_info->media_file_ops->offset_get(filehandle, &roffset);
        file->pos = roffset;
    }

    if (ret != FX_SUCCESS && ret != FX_END_OF_FILE) {
        log_print("Error:[FXREAD] file(0x%x) read error, ret(0x%x)!\n",
                   file->fullpath, ret);
        rsize = AIC_EIO;
    } else if (ret == FX_END_OF_FILE) {
        rsize = 0;
    }

    return rsize;
}

ssize_t fxffs_write(struct vfs_file *file, const char *buffer, size_t buflen)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;
    ulong roffset;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXWRITE] (%s) not mount.\n",
                   file->inode->name);
        return AIC_EINVAL;
    }

    filehandle = (u32)file->priv;
    ret = fs_info->media_file_ops->write(filehandle, (void *)buffer, buflen);

    if (ret == FX_SUCCESS) {
        fs_info->media_file_ops->offset_get(filehandle, &roffset);
        file->pos = roffset;
    }

    if (ret != FX_SUCCESS) {
        log_print("Error:[FXWRITE] file(0x%x) write error, ret(0x%x)!\n",
                   file->fullpath, ret);
        if (ret == FX_NO_MORE_SPACE)
            return AIC_ENOSPC;
        else
            return AIC_EIO;
    }

    return buflen;
}

off_t fxffs_lseek(struct vfs_file *file, off_t offset, int whence)
{
    u32 ret = FX_SUCCESS, filehandle;
    FXFFS_INFO *fs_info;
    ulong roffset;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXLSEEK] (%s) not mount.\n",
                   file->inode->name);
        return AIC_EINVAL;
    }

    filehandle = (u32)file->priv;
    ret = fs_info->media_file_ops->relative_seek(filehandle, offset, whence);

    if (ret == FX_SUCCESS) {
        fs_info->media_file_ops->offset_get(filehandle, &roffset);
        file->pos = roffset;
    }

    if (ret != FX_SUCCESS) {
        log_print("Error:[FXLSEEK] file(%s) seek offset(%ld) whence(%d) error, ret(0x%x)!\n",
                   file->fullpath, offset, whence, ret);
        return AIC_EIO;
    }

    return roffset;
}

int fxffs_sync(struct vfs_file *file)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXSYNC] (%s) not mount.\n",
                   file->inode->name);
        return AIC_EINVAL;
    }

    ret = fs_info->media_ops->flush(fs_info);

    if (ret != FX_SUCCESS) {
        log_print("Error:[FXSYNC] file(%s) flush error, ret(0x%x)!\n",
                   file->fullpath, ret);
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

int fxffs_fstat(struct vfs_file *file, struct stat * buf)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;
    aos_systime systime;
    ulong size = 0;
    int errcode;
    u32 attri;

    fs_info = (FXFFS_INFO *)(file->inode->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXFSTAT] (%s) not mount.\n", file->inode->name);
        return AIC_EINVAL;
    }

    if ((strlen("/") == strlen(file->fullpath)) &&
        !memcmp("/", file->fullpath, strlen(file->fullpath))) {
        buf->st_mode &= ~S_IFREG;
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        attri = FX_DIRECTORY;
    } else {
        ret = fs_info->media_dir_ops->info_get(fs_info,
                                               file->fullpath + strlen("/") +
                                               strlen(fs_info->volume_name),
                                               &attri, &size, &systime);
    }

    if (ret == FX_SUCCESS) {
        memset(buf, 0x0, sizeof(struct stat));
        buf->st_mode  = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                        S_IWUSR | S_IWGRP | S_IWOTH;
        if (attri & FX_DIRECTORY) {
            buf->st_mode &= ~S_IFREG;
            buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        }
        buf->st_size    = size;
        errcode = AIC_SUCCESS;
    } else {
        log_print("Error:[FXFSTAT] file(%s) get fstat error, ret(0x%x)!\n",
                  file->fullpath, ret);
        errcode = AIC_EIO;
    }

    return errcode;
}

int fxffs_unlink(struct vfs_inode *mp, const char *path)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;
    int errcode;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXUNLINK] (%s) not mount.\n", mp->name);
        return AIC_EINVAL;
    }

    errcode = AIC_SUCCESS;

    memset(fs_info->cache_buf, 0x0, PATH_MAX);
    snprintf(fs_info->cache_buf, PATH_MAX, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';

    do {
        if ((strlen("/") == strlen(fs_info->cache_buf)) &&
            !memcmp("/", fs_info->cache_buf, strlen(fs_info->cache_buf))) {
            errcode = AIC_ENOEXEC;
            break;
        }

        ret = fs_info->media_dir_ops->name_test(fs_info,(char *)fs_info->cache_buf);
        if (ret != FX_SUCCESS)
            ret = fs_info->media_file_ops->delete(fs_info, (char *)fs_info->cache_buf);
        else
            ret = fs_info->media_dir_ops->delete(fs_info, (char *)fs_info->cache_buf);

        if (ret == FX_NOT_FOUND) {
            errcode = AIC_ENOENT;
            break;
        } else if (ret == FX_DIR_NOT_EMPTY) {
            errcode = AIC_EEXIST;
            break; 
        } else if (ret != FX_SUCCESS) {
            errcode = AIC_EIO;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        log_print("Error:[FXUNLINK] path(%s) unlink error, ret(0x%x)!\n",
                   path, ret);
    }

    return errcode;
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
    boolean dirname_test = true;
    u32 ret = FX_SUCCESS, pos;
    char *createdir = NULL;
    FXFFS_INFO *fs_info;
    int errcode;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXMKDIR] (%s) not mount.\n", mp->name);
        return AIC_EINVAL;
    }

    memset(fs_info->cache_buf, 0x0, PATH_MAX);
    snprintf(fs_info->cache_buf, PATH_MAX, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';
    errcode = AIC_SUCCESS;

    do {
        if ((strlen("/") == strlen(fs_info->cache_buf)) &&
            !memcmp("/", fs_info->cache_buf, strlen(fs_info->cache_buf))) {
            errcode = AIC_SUCCESS;
            break;
        }

        ret = fs_info->media_dir_ops->name_test(fs_info, fs_info->cache_buf);
        if (ret == FX_SUCCESS) {
            errcode = AIC_SUCCESS;
            break;
        }

        createdir = malloc(strlen(fs_info->cache_buf) + 2);
        ASSERT_INFO(createdir, "malloc size(0x%x) memory fail.\n",
                    strlen(fs_info->cache_buf) + 2);
        memset(createdir, 0x0, strlen(fs_info->cache_buf) + 2);

        if (path[0] == '/')
            pos = 1;
        else
            pos = 0;
        while (pos < strlen(fs_info->cache_buf)) {
            fxffs_dirpathsearch(fs_info->cache_buf, pos, &pos);

            memcpy(createdir, fs_info->cache_buf, pos);
            if (dirname_test) {
                ret = fs_info->media_dir_ops->name_test(fs_info, createdir);
                if (ret != FX_SUCCESS) {
                    dirname_test = false;
                }
            }
            if (!dirname_test) {
                ret = fs_info->media_dir_ops->create(fs_info, createdir);
                if (ret != FX_SUCCESS) {
                    log_print("Error:[FXMKDIR] dir(%s) create error, ret(0x%x)!\n",
                               createdir, ret);
                    if (ret == FX_NO_MORE_SPACE)
                        errcode = AIC_ENOSPC;
                    else
                        errcode = AIC_EIO;

                    break;
                }
            }

            ret = fs_info->media_dir_ops->name_test(fs_info, createdir);
            ASSERT(ret == FX_SUCCESS);
            pos += 1;
        };

    } while(0);

    if (createdir)
        free(createdir);

    if (errcode != AIC_SUCCESS) {
        log_print("Error:[FXMKDIR] create path(%s) error, ret(0x%x)!\n",
                   path, ret);
    }

    return errcode;
}

int fxffs_stat(struct vfs_inode *mp, const char *path, struct stat *buf)
{
    aos_systime systime = {0};
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;
    ulong size = 0;
    int errcode;
    u32 attri;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXFSTAT] (%s) not mount.\n", mp->name);
        return AIC_EINVAL;
    }

    memset(buf, 0x0, sizeof(struct stat));
    snprintf(fs_info->cache_buf, PATH_MAX, "/%s", path);
    fs_info->cache_buf[strlen(fs_info->cache_buf)] = '\0';
    if ((strlen("/") == strlen(fs_info->cache_buf)) &&
        !memcmp("/", fs_info->cache_buf, strlen(fs_info->cache_buf))) {
        buf->st_mode &= ~S_IFREG;
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        attri = FX_DIRECTORY;
    } else {
        ret = fs_info->media_dir_ops->info_get(fs_info, (char *)fs_info->cache_buf,
                                               &attri, &size, &systime);
    }

    if (ret == FX_SUCCESS) {
        memset(buf, 0x0, sizeof(struct stat));
        buf->st_mode  = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                        S_IWUSR | S_IWGRP | S_IWOTH;
        if (attri & FX_DIRECTORY) {
            buf->st_mode &= ~S_IFREG;
            buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        }
        buf->st_size = (off_t)size;
        errcode = AIC_SUCCESS;
    } else {
        log_print("Error:[FXFSTAT] file(%s) get fstat error, ret(0x%x)!\n",
                   path, ret);
        errcode = AIC_EIO;
    }

    return errcode;
}

int fxffs_format(struct vfs_inode *mp)
{
    u32 ret = FX_SUCCESS;
    FXFFS_INFO *fs_info;

    fs_info = (FXFFS_INFO *)(mp->ops.fstypehandle);
    if (!fs_info) {
        log_print("Error:[FXFMT] (%s) not mount.\n", mp->name);
        return AIC_EINVAL;
    }

    do {
        ret = fs_info->media_ops->close(fs_info);
        if (ret != FX_SUCCESS) {
            log_print("Error:[FXFMT] %s close error, ret(0x%x)!\n",
                       mp->name, ret);
            break;
        }

        ret = fs_info->media_ops->format(fs_info);
        if (ret != FX_SUCCESS) {
            log_print("Error:[FXFMT] %s format error, ret(0x%x)!\n",
                       mp->name, ret);
            break;
        }

        ret = fs_info->media_ops->open(fs_info);
        if (ret != FX_SUCCESS) {
            log_print("Error:[FXFMT] %s open error, ret(0x%x)!\n",
                       mp->name, ret);
            break;
        }
    } while(0);

    if (ret != FX_SUCCESS) {
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

struct vfs_type_mount_ops g_fxffs_mnt_ops = {
    fxffs_mount,
    fxffs_umount,
    NULL,
    fxffs_statfs
};

struct vfs_type_file_ops g_fxffs_file_ops = {
    fxffs_open,
    fxffs_close,
    fxffs_read,
    fxffs_write,
    fxffs_lseek,
    NULL,
    NULL,
    fxffs_sync,
    fxffs_fstat,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    fxffs_unlink,
    fxffs_mkdir,
    NULL,
    NULL,
    fxffs_stat
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
        log_print("fs register type %s failed: %d\n",
                   FS_FXFAT_TYPE_NAME, ret);
    }

#ifdef FX_ENABLE_EXFAT
    ret = vfs_type_register(FS_FXEXFAT_TYPE_NAME, &g_fxffs_mnt_ops,
                            &g_fxffs_file_ops, &g_fxffs_mgt_ops);
    if (ret != AIC_SUCCESS) {
        log_print("fs register type %s failed: %d\n",
                   FS_FXEXFAT_TYPE_NAME, ret);
    }
#endif

    return ret;
}

