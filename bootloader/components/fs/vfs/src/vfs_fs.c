/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x185641

#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include "vfs.h"
#include "fs_api.h"

/**---------------------------------------------------------------------------*
 ** Private Types
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Private Functions Prototypes
 **---------------------------------------------------------------------------*/
static int vfs_file_open(struct vfs_file *file, const char *path,
                         int oflags, mode_t umask, va_list ap);
static int vfs_open(const char *path, int oflags, va_list ap);
static int vfs_file_close(struct vfs_file *file);
static ssize_t vfs_file_read(struct vfs_file *file, void *buf, size_t nbytes);
static ssize_t vfs_file_write(struct vfs_file *file, const void *buf,
                              size_t nbytes);
static off_t vfs_file_seek(struct vfs_file *file, off_t offset, int whence);
static int vfs_fsync(struct vfs_file *file);

/**---------------------------------------------------------------------------*
 ** Private Functions
 **---------------------------------------------------------------------------*/
/**
 * vfs_file_open - open file.
 *
 * Parameter:
 * @file: input file struct.
 * @path: input file path.
 * @oflags: input open flags.
 * @umask: input mode.
 * @ap: input open ap.
 *
 * Return:
 *  Zero (AIC_SUCCESS) is returned on success. On failure, a negated errno
 *  value is returned.
 */
static int vfs_file_open(struct vfs_file *file, const char *path,
                         int oflags, mode_t umask, va_list ap)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    char *fullpath = NULL;
    mode_t mode = 0666;
    int ret;

    if (path == NULL) {
        return AIC_EINVAL;
    }

    ret = vfs_getfullpath(path, &fullpath);
    if (ret < 0)
        return ret;

    if (((strlen("/") == strlen(fullpath)) &&
        !memcmp("/", fullpath, strlen(fullpath))) ||
        ((strlen("/dev") == strlen(fullpath)) &&
         !memcmp("/dev", fullpath, strlen(fullpath)))) {
        free(fullpath);
        return AIC_EISDIR;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, 0);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    ASSERT(inode != NULL);

    ret = vfs_inode_checkflags(inode, oflags);
    if (ret < 0) {
        goto errout_with_inode;
    }

    file->oflags = oflags;
    file->pos    = 0;
    file->inode  = inode;
    file->priv   = NULL;
    file->fullpath = fullpath;

    if (VFS_INODE_IS_MOUNTPT(inode)) {
        if (inode->ops.fstypeops &&
            inode->ops.fstypeops->fileops &&
            inode->ops.fstypeops->fileops->open) {
            ret = inode->ops.fstypeops->fileops->open(file, desc.relpath,
                                                      oflags, mode);
        }
    } else {
        ret = AIC_ENXIO;
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

    VFS_RELEASE_SEARCH(&desc);
    return AIC_SUCCESS;

errout_with_inode:
    file->inode = NULL;
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    free(fullpath);
    return ret;
}

/**
 * vfs_open - open file.
 *
 * Parameter:
 * @path: input file path.
 * @oflags: input open flags.
 * @ap: input open ap.
 *
 * Return:
 *  The new file descriptor is returned on success; a negated errno value is
 *  returned on any failure.
 */
static int vfs_open(const char *path, int oflags, va_list ap)
{
    struct vfs_file file;
    int ret, fd;

    ret = vfs_file_open(&file, path, oflags, 0, ap);
    if (ret < 0) {
        return ret;
    }

    fd = vfs_fd_alloc(file.inode, &file, 3, false);
    if (fd < 0) {
        vfs_file_close(&file);
        return fd;
    }

    return fd;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/**
 * fs_open - open file.
 *
 * Parameter:
 * @path: input file path.
 * @flags: input open flags.
 *
 * Return:
 *  The new file descriptor is returned on success; a negated errno value is
 *  returned on any failure.
 */
int fs_open(const char *path, int flags, ...)
{
    va_list ap;
    int fd;

    va_start(ap, flags);
    fd = vfs_open(path, flags, ap);
    va_end(ap);

    if (fd < 0) {
        vfs_seterrno(fd);
        log_print("Error:[Open] path(%s) flags(0x%x) open fail, ret=%d.\n",
                   path, flags, fd);
    }

    return fd;
}

/**
 * open - open file.
 *
 * Parameter:
 * @path: input file path.
 * @flags: input open flags.
 *
 * Return:
 *  The new file descriptor is returned on success; a negated errno value is
 *  returned on any failure.
 */
int open(const char *path, int flags, ...)
{
    va_list ap;
    int fd;

    if (path == NULL) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    va_start(ap, flags);
    fd = vfs_open(path, flags, ap);
    va_end(ap);

    if (fd < 0) {
        vfs_seterrno(fd);
        return -1;
    }

    return fd;
}

/**
 * creat - create file.
 *
 * Parameter:
 * @path: input file path.
 * @flags: input open flags.
 *
 * Return:
 *  The new file descriptor is returned on success; a negated errno value is
 *  returned on any failure.
 */
int creat(const char *path, mode_t mode)
{
    int fd;

    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, mode);
    if (fd < 0)
        return -1;

    return fd;
}

#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(open, _open, (const char *path, int flags, ...), int);
#endif

/**
 * vfs_file_close - close file.
 *
 * Parameter:
 * @file: input file struct.
 *
 * Return:
 *  Zero (AIC_SUCCESS) is returned on success. On failure, a negated errno
 *  value is returned.
 */
static int vfs_file_close(struct vfs_file *file)
{
    struct vfs_inode *inode;
    int ret = AIC_EBADF;

    ASSERT(file);
    inode = file->inode;
    ASSERT(inode);

    if (inode->ops.fstypeops &&
        inode->ops.fstypeops->fileops &&
        inode->ops.fstypeops->fileops->close) {
        ret = inode->ops.fstypeops->fileops->close(file);
    }

    vfs_inode_release(inode);

    file->oflags = 0;
    file->pos    = 0;
    file->inode  = NULL;
    file->priv   = NULL;
    if (file->fullpath)
        free(file->fullpath);

    return ret;
}

/**
 * fs_close - close file.
 *
 * Parameter:
 * @fd: input file descriptor.
 *
 * Return:
 *  Zero (AIC_SUCCESS) is returned on success. On failure, a negated errno
 *  value is returned.
 */
int fs_close(int fd)
{
    struct vfs_file *file;
    int ret;

    ret = vfs_fd_getfile(fd, &file);
    if (ret != AIC_SUCCESS) {
        log_print("Error:[Close] fd(%d) get file fail, ret=%d.\n", fd, ret);
        return ret;
    }

    vfs_fsync(file);
    vfs_seterrno(0);

    ret = vfs_file_close(file);
    if (ret != AIC_SUCCESS) {
        vfs_fd_release(fd);
        log_print("Error:[Close] fd(%d) close file fail, ret=%d.\n", fd, ret);
        return ret;
    }

    ret = vfs_fd_release(fd);
    if (ret != AIC_SUCCESS) {
        log_print("Error:[Close] fd(%d) release fail, ret=%d.\n", fd, ret);
        return ret;
    }

    return AIC_SUCCESS;
}

/**
 * close - close file.
 *
 * Parameter:
 * @fd: input file descriptor.
 *
 * Return:
 *  Zero (AIC_SUCCESS) is returned on success. On failure, a negated errno
 *  value is returned.
 */
int close(int fd)
{
    int ret;

    ret = fs_close(fd);
    if (ret < 0) {
        vfs_seterrno(ret);
        ret = -1;
    }

    return 0;
}
#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(close, _close, (int fd), int);
#endif

/**
 * vfs_file_read - read file.
 *
 * Parameter:
 * @file: input file struct.
 * @buf: input buffer to save the data.
 * @nbytes: input the maximum size of the user-provided buffer.
 *
 * Return:
 *  The positive non-zero number of bytes read on success, 0 on if an
 *  end-of-file condition, or a negated errno value on any failure.
 */
static ssize_t vfs_file_read(struct vfs_file *file, void *buf, size_t nbytes)
{
    struct vfs_inode *inode;
    int ret = AIC_EBADF;

    ASSERT(file);
    inode = file->inode;
    ASSERT(inode);

    if (((inode->flags & O_ACCMODE) == O_WRONLY) ||
        ((file->oflags & O_ACCMODE) == O_WRONLY)) {
        ret = AIC_EACCES;
    } else if (inode->ops.fstypeops &&
               inode->ops.fstypeops->fileops &&
               inode->ops.fstypeops->fileops->read) {
        ret = (int)inode->ops.fstypeops->fileops->read(file, (char *)buf,
                                                       nbytes);
    }

    return ret;
}

/**
 * fs_read - read file.
 *
 * Parameter:
 * @fd: input file descriptor.
 * @buf: input buffer to save the data.
 * @nbytes: input the maximum size of the user-provided buffer.
 *
 * Return:
 *  The positive non-zero number of bytes read on success, 0 on if an
 *  end-of-file condition, or a negated errno value on any failure.
 */
ssize_t fs_read(int fd, void *buf, size_t nbytes)
{
    struct vfs_file *file = NULL;
    ssize_t ret;

    if (buf == NULL || !nbytes) {
        return AIC_EINVAL;
    }

    ret = (ssize_t)vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        log_print("Error:[Read] fd(%d) get file fail, ret=%d.\n", fd, ret);
        return ret;
    }

    ret =  vfs_file_read(file, buf, nbytes);
    if (ret < 0) {
        log_print("Error:[Read] fd(%d) read file fail, ret=%d.\n", fd, ret);
        return ret;
    }

    return ret;
}

/**
 * read - read file.
 *
 * Parameter:
 * @fd: input file descriptor.
 * @buf: input buffer to save the data.
 * @nbytes: input the maximum size of the user-provided buffer.
 *
 * Return:
 *  The positive non-zero number of bytes read on success, 0 on if an
 *  end-of-file condition, or a negated errno value on any failure.
 */
ssize_t read(int fd, void *buf, size_t nbytes)
{
    ssize_t ret;

    ret = fs_read(fd, buf, nbytes);
    if (ret < 0) {
        vfs_seterrno(ret);
        ret = -1;
    }

    return ret;
}
#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(read, _read, (int fd, void *buf, size_t nbytes), int);
#endif

/**
 * vfs_file_write - write file.
 *
 * Parameter:
 * @file: input file struct.
 * @buf: input data to write.
 * @nbytes: input length of data to write.
 *
 * Return:
 *  On success, the number of bytes written are returned (zero indicates
 *  nothing was written).  On any failure, a negated errno value is returned
 *  (see comments withwrite() for a description of the appropriate errno
 *  values).
 */
static ssize_t vfs_file_write(struct vfs_file *file, const void *buf,
                              size_t nbytes)
{
    struct vfs_inode *inode;
    int ret = AIC_EBADF;

    ASSERT(file);
    inode = file->inode;
    ASSERT(inode);

    if (((inode->flags & O_ACCMODE) == O_RDONLY) ||
        ((file->oflags & O_ACCMODE) == O_RDONLY)) {
        ret = AIC_EROFS;
    } else if (inode->ops.fstypeops &&
               inode->ops.fstypeops->fileops &&
               inode->ops.fstypeops->fileops->write) {
        ret = (int)inode->ops.fstypeops->fileops->write(file, (char *)buf,
                                                        nbytes);
    }

    return ret;
}

/**
 * fs_write - write file.
 *
 * Parameter:
 * @fd: input file descriptor.
 * @buf: input data to write.
 * @nbytes: input length of data to write.
 *
 * Return:
 *  On success, the number of bytes written are returned (zero indicates
 *  nothing was written).  On any failure, a negated errno value is returned
 *  (see comments withwrite() for a description of the appropriate errno
 *  values).
 */
ssize_t fs_write(int fd, const void *buf, size_t nbytes)
{
    struct vfs_file *file;
    ssize_t ret;

    if (buf == NULL || !nbytes) {
        return AIC_EINVAL;
    }

    ret = (ssize_t)vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        log_print("Error:[write] fd(%d) get file fail, ret=%d.\n", fd, ret);
        return ret;
    }

    ret = vfs_file_write(file, buf, nbytes);
    if (ret < 0) {
        log_print("Error:[write] fd(%d) write file fail, ret=%d.\n", fd, ret);
        return ret;
    }

    return ret;
}

/**
 * write - write file.
 *
 * Parameter:
 * @fd: input file descriptor.
 * @buf: input data to write.
 * @nbytes: input length of data to write.
 *
 * Return:
 *  On success, the number of bytes written are returned (zero indicates
 *  nothing was written).  On any failure, a negated errno value is returned
 *  (see comments withwrite() for a description of the appropriate errno
 *  values).
 */
ssize_t write(int fd, const void *buf, size_t nbytes)
{
    ssize_t ret;

    ret = fs_write(fd, buf, nbytes);
    if (ret < 0) {
        vfs_seterrno(ret);
        ret = -1;
    }

    return ret;
}
#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(write, _write, (int fd, const void *buf, size_t nbytes), int);
#endif

/**
 * vfs_file_seek - seek file.
 *
 * Parameter:
 *  @file: input file structure instance.
 *  @offset: input defines the offset to position to.
 *  @whence: input defines how to use offset.
 *
 * Return:
 *   The resulting offset on success.  A negated errno value is returned on
 *   any failure.
 */
static off_t vfs_file_seek(struct vfs_file *file, off_t offset, int whence)
{
    struct vfs_inode *inode;
    off_t ret = AIC_EBADF;

    ASSERT(file);
    inode =  file->inode;
    ASSERT(inode);

    if (inode->ops.fstypeops &&
        inode->ops.fstypeops->fileops &&
        inode->ops.fstypeops->fileops->seek) {
        ret = inode->ops.fstypeops->fileops->seek(file, offset, whence);
        if (ret < 0) {
            return ret;
        } else {
            file->pos = ret;
        }
    } else {
        return AIC_ENOSYS;
    }

    return file->pos;
}

/**
 * fs_seek - seek file.
 *
 * Parameter:
 *  @fd: input file descriptor.
 *  @offset: input defines the offset to position to.
 *  @whence: input defines how to use offset.
 *
 * Return:
 *   The resulting offset on success.  A negated errno value is returned on
 *   any failure.
 */
off_t fs_seek(int fd, off_t offset, int whence)
{
    struct vfs_file *file;
    off_t ret;

    ret = vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        log_print("Error:[seek] fd(%d) get file fail, ret=%d.\n", fd, ret);
        return ret;
    }

    ASSERT(file != NULL);

    ret =  vfs_file_seek(file, offset, whence);
    if (ret < 0) {
        log_print("Error:[seek] fd(%d) seek file fail, ret=%d.\n", fd, ret);
        return ret;
    }

    return ret;
}

/**
 * lseek - lseek file.
 *
 * Parameter:
 *  @fd: input file descriptor.
 *  @offset: input defines the offset to position to.
 *  @whence: input defines how to use offset.
 *
 * Return:
 *   The resulting offset on success.  A negated errno value is returned on
 *   any failure.
 */
off_t lseek(int fd, off_t offset, int whence)
{
    off_t newpos;

    newpos = fs_seek(fd, offset, whence);
    if (newpos < 0) {
        vfs_seterrno(newpos);
        return -1;
    }

    return newpos;
}
#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(lseek, _lseek, (int fd, off_t offset, int whence), int);
#endif

/**
 * vfs_file_fstat - file fstat.
 *
 * Parameter:
 *  @file: input file struct.
 *  @buf: input stat buf.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_file_fstat(struct vfs_file *file, struct stat *buf)
{
    struct vfs_inode *inode;
    int ret = AIC_EBADF;

    if (buf == NULL) {
        return AIC_EINVAL;
    }

    ASSERT(file);
    inode = file->inode;
    ASSERT(inode);

    if (inode->ops.fstypeops &&
        inode->ops.fstypeops->fileops &&
        inode->ops.fstypeops->fileops->fstat) {
        ret = inode->ops.fstypeops->fileops->fstat(file, buf);
    }

    return ret;
}

/**
 * fstat - file fstat.
 *
 * Parameter:
 *  @fd: input file description.
 *  @buf: input stat buf.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fstat(int fd, struct stat *buf)
{
    struct vfs_file *file;
    int ret;

    if (buf == NULL) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    ret = vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        vfs_seterrno(ret);
        log_print("Error:[fstat] fd(%d) get file fail, ret=%d.\n", fd, ret);
        return -1;
    }

    ret = vfs_file_fstat(file, buf);
    if (ret < 0) {
        vfs_seterrno(ret);
        log_print("Error:[fstat] fd(%d) get fstat fail, ret=%d.\n", fd, ret);
        return -1;
    }

    return 0;
}
#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(fstat, _fstat, (int fd, struct stat *buf), int);
#endif

/**
 * vfs_fsync - fs sync.
 *
 * Parameter:
 *  @file: input file struct.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_fsync(struct vfs_file *file)
{
    struct vfs_inode *inode;
    int ret = AIC_EBADF;

    ASSERT(file);
    inode = file->inode;
    ASSERT(inode);

    if (VFS_INODE_IS_MOUNTPT(inode) && inode->ops.fstypeops &&
        inode->ops.fstypeops->fileops &&
        inode->ops.fstypeops->fileops->sync) {
        ret = inode->ops.fstypeops->fileops->sync(file);
    }

    return ret;
}

/**
 * fsync - fs sync.
 *
 * Parameter:
 *  @fd: input file description.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fsync(int fd)
{
    struct vfs_file *file;
    int ret;

    ret = vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        vfs_seterrno(ret);
        log_print("Error:[fsync] fd(%d) get file fail, ret=%d.\n", fd, ret);
        return -1;
    }

    ASSERT(file != NULL);

    ret = vfs_fsync(file);
    if (ret < 0) {
        vfs_seterrno(ret);
        log_print("Error:[fsync] fd(%d) fsync fail, ret=%d.\n", fd, ret);
        return -1;
    }

    return 0;
}

/**
 * fs_flush - fs flush.
 *
 * Parameter:
 *  @fd: input file description.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_flush(int fd)
{
    return fsync(fd);
}

/**
 * mkdir - create fs directory.
 *
 * Parameter:
 *  @path: input directory path.
 *  @mode: input create dir mode.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int mkdir(const char *path, mode_t mode)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    char *fullpath = NULL;
    int ret;

    ret = vfs_getfullpath(path,  &fullpath);
    if (ret != AIC_SUCCESS)
        return ret;

    VFS_SETUP_SEARCH(&desc, fullpath, false);

    ret = vfs_inode_find(&desc);
    if (ret >= 0) {
        inode = desc.node;
        ASSERT(inode != NULL);

        if (desc.relpath[0] == '\0') {
            ret = AIC_EEXIST;
            goto errout_with_inode;
        }

        if (!VFS_INODE_IS_MOUNTPT(inode) || !inode->ops.fstypeops ||
            !inode->ops.fstypeops->fileops) {
            ret = AIC_ENXIO;
            goto errout_with_inode;
        }

        if ((inode->flags & O_ACCMODE) == O_RDONLY) {
            ret = AIC_EROFS;
            goto errout_with_inode;
        }

        if (inode->ops.fstypeops->fileops->mkdir) {
            inode->mode = mode;
            ret = inode->ops.fstypeops->fileops->mkdir(inode, desc.relpath);
            if (ret < 0) {
                goto errout_with_inode;
            }
        } else {
            ret = AIC_ENOSYS;
            goto errout_with_inode;
        }

        vfs_inode_release(inode);
    } else {
        ret = AIC_ENXIO;
        goto errout_with_search;
    }

    free(fullpath);
    VFS_RELEASE_SEARCH(&desc);
    return 0;

errout_with_inode:
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    free(fullpath);
    vfs_seterrno(ret);
    return -1;
}

/**
 * fs_unlink - fs unlink path.
 *
 * Parameter:
 *  @path: input file or directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_unlink(const char *path)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    char *fullpath = NULL;
    int ret;

    ret = vfs_getfullpath(path,  &fullpath);
    if (ret != AIC_SUCCESS)
        return ret;

    VFS_SETUP_SEARCH(&desc, fullpath, true);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    ASSERT(inode != NULL);

    if (VFS_INODE_IS_MOUNTPT(inode) && inode->ops.fstypeops &&
        inode->ops.fstypeops->fileops) {
        if ((inode->flags & O_ACCMODE) == O_RDONLY) {
            ret = AIC_EROFS;
            goto errout_with_inode;
        }

        if (inode->ops.fstypeops->fileops->unlink) {
            ret = inode->ops.fstypeops->fileops->unlink(inode, desc.relpath);
            if (ret < 0) {
                goto errout_with_inode;
            }
        } else {
            ret = AIC_ENOSYS;
            goto errout_with_inode;
        }
    } else {
        ret = AIC_ENXIO;
        goto errout_with_inode;
    }

    vfs_inode_release(inode);
    VFS_RELEASE_SEARCH(&desc);
    free(fullpath);
    return 0;

errout_with_inode:
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    free(fullpath);
    return ret;
}

/**
 * unlink - fs unlink path.
 *
 * Parameter:
 *  @pathname: input file or directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int unlink(const char *pathname)
{
    int ret;

    ret = fs_unlink(pathname);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

/**
 * vfs_stat - fs get path stat.
 *
 * Parameter:
 *  @path: input stat path.
 *  @buf: input stat buf.
 *  @resolve: input resolve.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int vfs_stat(const char *path, struct stat *buf, int resolve)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    char *fullpath = NULL;
    int ret;

    if (path == NULL  || buf == NULL) {
        return AIC_EFAULT;
    }

    ret = vfs_getfullpath(path,  &fullpath);
    if (ret != AIC_SUCCESS)
        return ret;

    VFS_SETUP_SEARCH(&desc, fullpath, true);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    ASSERT(inode);

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(inode)) {
        if (inode->ops.fstypeops && inode->ops.fstypeops->fileops &&
            inode->ops.fstypeops->fileops->stat) {
            ret = inode->ops.fstypeops->fileops->stat(inode, desc.relpath, buf);
        } else {
            ret = AIC_ENOSYS;
        }
    }

    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    free(fullpath);
    return ret;
}

/**
 * stat - fs get path stat.
 *
 * Parameter:
 *  @path: input stat path.
 *  @buf: input stat buf.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int stat(const char *path, struct stat *buf)
{
    int ret;

    if (path == NULL  || buf == NULL) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    ret = vfs_stat(path, buf, 0);
    if (ret < 0) {
        vfs_seterrno(ret);
        ret = -1;
    }

    return ret;
}
#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(stat, _stat, (const char *path, struct stat *stat), int);
#endif

/**
 * vfs_statfs - fs get path stat.
 *
 * Parameter:
 *  @path: input stat path.
 *  @buf: input stat buf.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int vfs_statfs(const char *path, struct statfs *buf)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    int ret = AIC_SUCCESS;
    char *fullpath = NULL;

    if (path == NULL  || buf == NULL) {
        return AIC_EFAULT;
    }

    ret = vfs_getfullpath(path,  &fullpath);
    if (ret != AIC_SUCCESS)
        return ret;

    VFS_SETUP_SEARCH(&desc, fullpath, false);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    ASSERT(inode != NULL);

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(inode)) {
        ret = AIC_ENOSYS;
        if (inode->ops.fstypeops && inode->ops.fstypeops->fileops &&
            inode->ops.fstypeops->mntops->statfs) {
            ret = inode->ops.fstypeops->mntops->statfs(inode, buf);
        }
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

    vfs_inode_release(inode);
    VFS_RELEASE_SEARCH(&desc);
    free(fullpath);
    return AIC_SUCCESS;

errout_with_inode:
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    free(fullpath);
    return ret;
}

/**
 * statfs - fs get path stat.
 *
 * Parameter:
 *  @path: input stat path.
 *  @buf: input stat buf.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int statfs(const char *path, struct statfs *buf)
{
    int ret;

    if (path == NULL  || buf == NULL) {
        vfs_seterrno(AIC_EFAULT);
        return -1;
    }

    ret = vfs_statfs(path, buf);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

/**
 * fs_getdiskfreespace - fs disk free space and total space.
 *
 * Parameter:
 *  @path: input mount target path.
 *  @free_bytes: input to store free bytes ptr.
 *  @total_bytes: input to store total bytes ptr.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_getdiskfreespace(const char *path, u64 *free_bytes,
                        u64 *total_bytes)
{
    struct statfs buf;
    int ret;

    ret = statfs(path, &buf);
    if (ret < 0)
        return ret;

    *free_bytes = (u64)buf.f_bfree * (u64)buf.f_bsize;
    *total_bytes = (u64)buf.f_blocks * (u64)buf.f_bsize;

    return ret;
}

/**
 * fs_geterrno - get file errno.
 *
 * Parameter:
 *
 * Return:
 *   return error number.
 */
int fs_geterrno(void)
{
    return vfs_geterrno();
}

