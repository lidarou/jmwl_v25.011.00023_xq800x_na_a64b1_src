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
#define LOG_MODULE    AMOD_FS

#include <aos_utl.h>
#include <aic_errno.h>
#ifndef WIN32
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#endif
#include "vfs.h"
#include "fs_api.h"

/**---------------------------------------------------------------------------*
 ** Private Types
 **---------------------------------------------------------------------------*/
struct fs_dir_path {
    char *srcpath;
    char *dstpath;
    char path[PATH_MAX];
};

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
static int vfs_file_fstat(struct vfs_file *file, struct stat *buf);
static int vfs_file_truncate(struct vfs_file *file, off_t length);
static int vfs_file_dup(struct vfs_file *file, int minfd, bool cloexec);
static int vfs_file_dup2(struct vfs_file *file1, struct vfs_file *file2);
static int vfs_file_fcntl(struct vfs_file *file, int cmd, void *arg);
static int vfs_file_ioctl(struct vfs_file *file, int req, void *arg);
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

    ret = vfs_is_sys_path(fullpath);
    if (ret && ((oflags & O_ACCMODE) != O_RDONLY)) {
        aos_free(fullpath);
        return AIC_EACCES;
    }

    if (((strlen("/") == strlen(fullpath)) &&
        !memcmp("/", fullpath, strlen(fullpath))) ||
        ((strlen("/dev") == strlen(fullpath)) &&
         !memcmp("/dev", fullpath, strlen(fullpath)))) {
        aos_free(fullpath);
        if ((unsigned)oflags & O_DIRECTORY) {
            return AIC_EPERM;
        }
        return AIC_EISDIR;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, (oflags & O_NOFOLLOW) != 0);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    AIC_ASSERT(inode != NULL);

    ret = vfs_inode_checkflags(inode, oflags);
    if (ret < 0) {
        goto errout_with_inode;
    }

    file->oflags = oflags;
    file->pos    = 0;
    file->inode  = inode;
    file->priv   = NULL;
    file->fullpath = fullpath;
    file->owner = (u32)aos_get_current_thread_name();

    if (oflags & O_DIRECTORY) {
        ret = AIC_EISDIR;
    } else if (VFS_INODE_IS_MOUNTPT(inode)) {
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
    aos_free(fullpath);
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

    AIC_ASSERT(file);
    inode = file->inode;
    AIC_ASSERT(inode);

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
        aos_free(file->fullpath);

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
        return ret;
    }

    vfs_fsync(file);

    ret = vfs_file_close(file);
    if (ret != AIC_SUCCESS) {
        vfs_fd_release(fd);
        return ret;
    }

    ret = vfs_fd_release(fd);
    if (ret != AIC_SUCCESS) {
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
        return -1;
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

    AIC_ASSERT(file);
    inode = file->inode;
    AIC_ASSERT(inode);

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
        return ret;
    }

    ret =  vfs_file_read(file, buf, nbytes);
    if (ret < 0) {
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

    AIC_ASSERT(file);
    inode = file->inode;
    AIC_ASSERT(inode);

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
        return ret;
    }

    ret = vfs_file_write(file, buf, nbytes);
    if (ret < 0) {
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

    AIC_ASSERT(file);
    inode =  file->inode;
    AIC_ASSERT(inode);

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
        return ret;
    }

    AIC_ASSERT(file != NULL);

    ret =  vfs_file_seek(file, offset, whence);
    if (ret < 0) {
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

    AIC_ASSERT(file);
    inode = file->inode;
    AIC_ASSERT(inode);

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
        return -1;
    }

    ret = vfs_file_fstat(file, buf);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}
#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(fstat, _fstat, (int fd, struct stat *buf), int);
#endif

/**
 * fstatfs - fs stat.
 *
 * Parameter:
 *  @fd: input file description.
 *  @buf: input statfs buf.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fstatfs(int fd, struct statfs *buf)
{
    struct vfs_inode *inode;
    struct vfs_file *filep;
    int ret;

    if (buf == NULL) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    ret = vfs_fd_getfile(fd, &filep);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    AIC_ASSERT(filep);
    inode = filep->inode;
    AIC_ASSERT(inode);

    if (VFS_INODE_IS_MOUNTPT(inode)) {
        ret = AIC_ENOSYS;
        if (inode->ops.fstypeops &&
            inode->ops.fstypeops->mntops &&
            inode->ops.fstypeops->mntops->statfs) {
            ret = inode->ops.fstypeops->mntops->statfs(inode, buf);
        }
    } else {
        ret = AIC_EBADF;
    }

    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

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

    AIC_ASSERT(file);
    inode = file->inode;
    AIC_ASSERT(inode);

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
        return -1;
    }

    AIC_ASSERT(file != NULL);

    ret = vfs_fsync(file);
    if (ret < 0) {
        vfs_seterrno(ret);
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
 * vfs_file_truncate - fs file truncate.
 *
 * Parameter:
 *  @file: input file struct.
 *  @length: input truncate length.

 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_file_truncate(struct vfs_file *file, off_t length)
{
    struct vfs_inode *inode;
    int ret = AIC_EBADF;

    AIC_ASSERT(file);

    if ((file->oflags & O_ACCMODE) == O_RDONLY) {
        return AIC_EBADF;
    }

    inode = file->inode;
    AIC_ASSERT(inode);

    if (inode->ops.fstypeops && inode->ops.fstypeops->fileops &&
        inode->ops.fstypeops->fileops->write &&
        inode->ops.fstypeops->fileops->truncate) {
        ret = inode->ops.fstypeops->fileops->truncate(file, length);
    }

    return ret;
}

/**
 * ftruncate - fs file truncate.
 *
 * Parameter:
 *  @fd: input file description.
 *  @length: input truncate length.

 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int ftruncate(int fd, off_t length)
{
    struct vfs_file *file;
    int ret = AIC_EINVAL;

    if (length < 0) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    ret = vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    AIC_ASSERT(file != NULL);

    ret = vfs_file_truncate(file, length);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

/**
 * vfs_file_dup2 - fs file dup2.
 *
 * Parameter:
 *  @file1: input file struct.
 *  @file2: input file struct.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_file_dup2(struct vfs_file *file1, struct vfs_file *file2)
{
    struct vfs_file temp = {0};
    struct vfs_inode *inode;
    int ret = AIC_EBADF;

    if (file1 == NULL || file1->inode == NULL || file2 == NULL) {
        return AIC_EBADF;
    }

    if (file1 == file2) {
        return AIC_SUCCESS;
    }

    inode = file1->inode;

    ret   = vfs_inode_addref(inode);
    if (ret < 0) {
        return ret;
    }

    temp.oflags = file1->oflags;
    temp.pos    = file1->pos;
    temp.inode  = inode;
    temp.priv   = NULL;
    temp.fullpath = file1->fullpath;

    if (inode->ops.fstypeops && inode->ops.fstypeops->fileops) {
        ret = AIC_ENOSYS;
        if (VFS_INODE_IS_MOUNTPT(inode)) {
            if (inode->ops.fstypeops->fileops->dup) {
                ret = inode->ops.fstypeops->fileops->dup(file1, &temp);
            }
        }

        if (ret < 0) {
            vfs_inode_release(inode);
            return ret;
        }
    }

    ret = vfs_file_close(file2);
    AIC_ASSERT(ret == 0);

    memcpy(file2, &temp, sizeof(temp));

    return AIC_SUCCESS;
}

/**
 * vfs_file_dup - fs file dup.
 *
 * Parameter:
 *  @file: input file struct.
 *  @minfd: input min fd.
 *  @cloexec: input cloexec.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_file_dup(struct vfs_file *file, int minfd, bool cloexec)
{
    struct vfs_file file2;
    int fd2;
    int ret;

    memset(&file2, 0, sizeof(file2));
    ret = vfs_file_dup2(file, &file2);
    if (ret < 0) {
        return ret;
    }

    if (cloexec) {
        file2.oflags |= O_CLOEXEC;
    }

    fd2 = vfs_fd_alloc(file2.inode, &file2, minfd, false);
    if (fd2 < 0) {
        vfs_file_close(&file2);
        return fd2;
    }

    return fd2;
}

/**
 * dup - fs file dup.
 *
 * Parameter:
 *  @fd: input file description.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int dup(int fd)
{
    struct vfs_file *file;
    int ret;

    ret = vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    AIC_ASSERT(file != NULL);

    ret = vfs_file_dup(file, 0, 0);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

/**
 * vfs_file_fcntl - fs file fcntl.
 *
 * Parameter:
 *  @file: input file struct.
 *  @cmd: input fcntl cmd.
 *  @arg: input fcntl arg.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_file_fcntl(struct vfs_file *file, int cmd, void *arg)
{
    int ret = AIC_EINVAL;

    if (!file->inode) {
        return AIC_EBADF;
    }

    switch (cmd) {
        case F_DUPFD:
            ret = vfs_file_dup(file, (int)arg, false);
            break;

        case F_DUPFD_CLOEXEC:
            ret = vfs_file_dup(file, (int)arg, true);
            break;

        case F_GETFD:
            ret = file->oflags & O_CLOEXEC ? FD_CLOEXEC : 0;
            break;

        case F_SETFD:
            {
                int oflags = (int)arg;

                if (oflags & ~FD_CLOEXEC) {
                    ret = AIC_ENOSYS;
                    break;
                }

                if (oflags & FD_CLOEXEC)
                {
                    ret = vfs_file_ioctl(file, FIOCLEX, 0);
                }
                else
                {
                    ret = vfs_file_ioctl(file, FIONCLEX, 0);
                }
            }
            break;

        case F_GETFL:
            ret = file->oflags;
            break;
        case F_SETFL:
            {
                int oflags = (int)arg;
                int nonblock = !!(oflags & O_NONBLOCK);

                ret = vfs_file_ioctl(file, FIONBIO, &nonblock);
                if (ret == AIC_SUCCESS) {
                    oflags          &=  (FFCNTL & ~O_NONBLOCK);
                    file->oflags &= ~(FFCNTL & ~O_NONBLOCK);
                    file->oflags |=  oflags;

                    if ((file->oflags & O_APPEND) != 0) {
                        vfs_file_seek(file, 0, SEEK_END);
                    }
                }
            }
            break;

        case F_GETOWN:
        case F_SETOWN:
            ret = AIC_EBADF;
            break;
        case F_GETLK:
        case F_SETLK:
        case F_SETLKW:
            ret =AIC_ENOSYS;
            break;
        case F_GETPATH:
            ret = vfs_file_ioctl(file, FIOC_FILEPATH, arg);
        default:
            break;
    }

    return ret;
}

/**
 * fs_fcntl - fs file fcntl.
 *
 * Parameter:
 *  @fd: input file description.
 *  @cmd: input fcntl cmd.
 *  @arg: input fcntl arg.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_fcntl(int fd, int cmd, void *arg)
{
    struct vfs_file *file;
    int ret;

    ret = vfs_fd_getfile(fd, &file);
    if (ret >= 0) {
        AIC_ASSERT(file != NULL);
        ret = vfs_file_fcntl(file, cmd, arg);
    }

    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

/**
 * vfs_file_ioctl - fs file ioctl.
 *
 * Parameter:
 *  @file: input file struct.
 *  @req: input ioctl req.
 *  @arg: input ioctl arg.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_file_ioctl(struct vfs_file *file, int req, void *arg)
{
    struct vfs_inode *inode;
    int ret = AIC_ENOTTY;

    AIC_ASSERT(file != NULL);

    inode = file->inode;
    if (!inode) {
        return AIC_EBADF;
    }

    if (inode->ops.fstypeops && inode->ops.fstypeops->fileops &&
        inode->ops.fstypeops->fileops->ioctl) {
        ret = inode->ops.fstypeops->fileops->ioctl(file, req, arg);
    }

    switch (req) {
        case FIONBIO:
            if (ret == AIC_SUCCESS || ret == AIC_ENOTTY) {
                int *nonblock = (int *)(uintptr_t)arg;
                if (nonblock && *nonblock) {
                    file->oflags |= O_NONBLOCK;
                } else {
                    file->oflags &= ~O_NONBLOCK;
                }

                ret = AIC_SUCCESS;
            }
            break;

        case FIOCLEX:
            if (ret == AIC_SUCCESS || ret == AIC_ENOTTY) {
                file->oflags |= O_CLOEXEC;
                ret = AIC_SUCCESS;
            }
            break;

        case FIONCLEX:
            if (ret == AIC_SUCCESS || ret == AIC_ENOTTY) {
                file->oflags &= ~O_CLOEXEC;
                ret = AIC_SUCCESS;
            }
            break;

        case FIOC_FILEPATH:
            if (ret == AIC_ENOTTY && !VFS_INODE_IS_MOUNTPT(inode)) {
                ret = vfs_inode_getpath(inode, (char *)(uintptr_t)arg);
            }
            break;
    }

  return ret;
}

/**
 * fs_ioctl - fs file ioctl.
 *
 * Parameter:
 *  @fd: input file description.
 *  @req: input ioctl req.
 *  @arg: input ioctl arg.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_ioctl(int fd, int req, void *arg)
{
    struct vfs_file *file;
    int ret;

    ret = vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    ret = vfs_file_ioctl(file, req, arg);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

/**
 * opendir - fs open directory.
 *
 * Parameter:
 *  @path: input directory path.
 *
 * Return:
 *  Upon successful completion, non-empty is returned.
 *  Otherwise, NULL is returned.
 */
DIR *opendir(const char *path)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    char *fullpath = NULL;
    struct vfs_dir *dir;
    int ret;

    if (path == NULL) {
        vfs_seterrno(AIC_EINVAL);
        return NULL;
    }

    ret = vfs_getfullpath(path,  &fullpath);
    if (ret != AIC_SUCCESS) {
        vfs_seterrno(ret);
        return NULL;
    }

    if (((strlen("/") == strlen(fullpath)) &&
        !memcmp("/", fullpath, strlen(fullpath))) ||
        ((strlen("/dev") == strlen(fullpath)) &&
         !memcmp("/dev", fullpath, strlen(fullpath)))) {
        aos_free(fullpath);
        vfs_seterrno(AIC_EISDIR);
        return NULL;
    }

    dir = (struct vfs_dir *)aos_zalloc(sizeof(struct vfs_dir));
    if (dir == NULL) {
        aos_free(fullpath);
        vfs_seterrno(AIC_ENOMEM);
        return NULL;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, 0);
    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    AIC_ASSERT(inode != NULL);
    dir->inode = inode;
    dir->pos = 0;

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(inode)) {
        ret = AIC_ENOSYS;
        if (inode->ops.fstypeops &&
            inode->ops.fstypeops->fileops &&
            inode->ops.fstypeops->fileops->opendir) {
            ret = inode->ops.fstypeops->fileops->opendir(dir, desc.relpath);
        }
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

    dir->fullpath = fullpath;

    VFS_RELEASE_SEARCH(&desc);

    return (DIR *)dir;

errout_with_inode:
    dir->inode = NULL;
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullpath);
    aos_free(dir);
    vfs_seterrno(ret);
    return NULL;
}

/**
 * readdir - fs read directory.
 *
 * Parameter:
 *  @dir: input directory struct.
 *
 * Return:
 *  Upon successful completion, non-empty is returned.
 *  Otherwise, NULL is returned.
 */
struct dirent *readdir(DIR *dir)
{
    struct dirent *dent = NULL;
    struct vfs_dir *d = NULL;
    int ret;

    if (dir == NULL) {
        vfs_seterrno(AIC_EINVAL);
        return NULL;
    }

    d = (struct vfs_dir *)dir;
    if (d->inode == NULL) {
        vfs_seterrno(AIC_EINVAL);
        return NULL;
    }

    dent = (struct dirent *)&d->dent;
    if (dent == NULL) {
        vfs_seterrno(AIC_EINVAL);
        return NULL;
    }
    aos_memset(dent, 0x0, sizeof(struct dirent ));

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(d->inode)) {
        ret = AIC_ENOSYS;
        if ((d->inode->ops.fstypeops) && (d->inode->ops.fstypeops->fileops) &&
            (d->inode->ops.fstypeops->fileops->readdir)) {
            ret = d->inode->ops.fstypeops->fileops->readdir(d, dent);
        }
    }

    if (ret < 0) {
        vfs_seterrno(ret);
        dent = NULL;
    }

    return dent;
}

/**
 * telldir - tell fs directory.
 *
 * Parameter:
 *  @dir: input directory struct.
 *
 * Return:
 *  Upon successful completion, directory position is returned.
 *  Otherwise, a negated errno value is returned.
 */
long telldir(DIR *dir)
{
    struct vfs_dir *d = (struct vfs_dir *)dir;
    int ret;

    if ((d == NULL) || (d->inode == NULL)) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(d->inode)) {
        ret = AIC_ENOSYS;
        if ((d->inode->ops.fstypeops) && (d->inode->ops.fstypeops->fileops) &&
            (d->inode->ops.fstypeops->fileops->telldir)) {
            ret = d->inode->ops.fstypeops->fileops->telldir(d);
            AIC_ASSERT((long)ret == d->pos);
        } else {
            ret = AIC_SUCCESS;
        }
    }

    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return d->pos;
}

/**
 * seekdir - seek fs directory.
 *
 * Parameter:
 *  @dir: input directory struct.
 *  @offset: input directory offset.
 *
 * Return:.
 */
void seekdir(DIR *dir, long offset)
{
    struct vfs_dir *d = (struct vfs_dir *)dir;
    int ret;

    if ((d == NULL) || (d->inode == NULL)) {
        vfs_seterrno(AIC_EINVAL);
        return;
    }

    if (d->pos > offset) {
        d->pos = 0;
    }

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(d->inode)) {
        ret = AIC_ENOSYS;
        if ((d->inode->ops.fstypeops) && (d->inode->ops.fstypeops->fileops) &&
            (d->inode->ops.fstypeops->fileops->seekdir)) {
            ret = d->inode->ops.fstypeops->fileops->seekdir(d, offset);
        }
    }

    if (ret < 0) {
        vfs_seterrno(ret);
    }
}

/**
 * rewinddir - rewind fs directory.
 *
 * Parameter:
 *  @dir: input directory struct.
 *
 * Return:.
 */
void rewinddir(DIR *dir)
{
    struct vfs_dir *d = (struct vfs_dir *)dir;
    int ret;

    if ((d == NULL) || (d->inode == NULL)) {
        vfs_seterrno(AIC_EINVAL);
        return;
    }

    if (d->pos) {
        d->pos = 0;
    }

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(d->inode)) {
        ret = AIC_ENOSYS;
        if ((d->inode->ops.fstypeops) && (d->inode->ops.fstypeops->fileops) &&
            (d->inode->ops.fstypeops->fileops->rewinddir)) {
            ret = d->inode->ops.fstypeops->fileops->rewinddir(d);
        }
    }

    if (ret < 0) {
        vfs_seterrno(ret);
    }

    return;
}

/**
 * closedir - close fs directory.
 *
 * Parameter:
 *  @dir: input directory struct.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int closedir(DIR *dir)
{
    struct vfs_dir *d = (struct vfs_dir *)dir;
    int ret;

    if ((d == NULL) || (d->inode == NULL)) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    if ((d->inode->ops.fstypeops) && (d->inode->ops.fstypeops->fileops) &&
        (d->inode->ops.fstypeops->fileops->closedir)) {
        ret = d->inode->ops.fstypeops->fileops->closedir(d);
        if (ret != AIC_SUCCESS) {
            vfs_seterrno(ret);
            ret = -1;
        }
    } else {
        vfs_seterrno(AIC_ENOTSUP);
        ret = -1;
    }

    vfs_inode_release(d->inode);
    aos_free(d->fullpath);

    aos_free(d);
    dir = NULL;

    return ret;
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

    ret = vfs_is_sys_path(fullpath);
    if (ret) {
        aos_free(fullpath);
        return AIC_EACCES;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, false);

    ret = vfs_inode_find(&desc);
    if (ret >= 0) {
        inode = desc.node;
        AIC_ASSERT(inode != NULL);

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

    aos_free(fullpath);
    VFS_RELEASE_SEARCH(&desc);
    return 0;

errout_with_inode:
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullpath);
    vfs_seterrno(ret);
    return -1;
}

/**
 * fs_createdir - create fs directory.
 *
 * Parameter:
 *  @path: input directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_createdir(const char *path)
{
    return mkdir(path, O_RDWR | O_CREAT);
}

/**
 * rmdir - remove fs directory.
 *
 * Parameter:
 *  @path: input directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int rmdir(const char *path)
{
    struct vfs_inode_search_s desc;
    int errcode = AIC_SUCCESS;
    struct vfs_inode *inode;
    char *fullPath = NULL;
    int ret;

    ret = vfs_getfullpath(path,  &fullPath);
    if (ret != AIC_SUCCESS) {
        vfs_seterrno(ret);
        return -1;
    }

    ret = vfs_is_sys_path(fullPath);
    if (ret) {
        aos_free(fullPath);
        return AIC_EACCES;
    }

    VFS_SETUP_SEARCH(&desc, fullPath, true);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        errcode = -ret;
        goto errout_with_search;
    }

    inode = desc.node;
    AIC_ASSERT(inode != NULL);

    if (VFS_INODE_IS_MOUNTPT(inode) && inode->ops.fstypeops->fileops) {
        if ((inode->flags & O_ACCMODE) == O_RDONLY) {
            ret = AIC_EROFS;
            goto errout_with_inode;
        }

        if (inode->ops.fstypeops->fileops->rmdir) {
            ret = inode->ops.fstypeops->fileops->rmdir(inode, desc.relpath);
            if (ret < 0) {
                errcode = -ret;
                goto errout_with_inode;
            }
        } else {
            errcode = AIC_ENOSYS;
            goto errout_with_inode;
        }
    } else {
        errcode = AIC_ENXIO;
        goto errout_with_inode;
    }

    vfs_inode_release(inode);
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullPath);
    return 0;

errout_with_inode:
    vfs_inode_release(inode);
errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullPath);
    vfs_seterrno(errcode);
    return -1;
}

/**
 * fs_rmdir - remove fs directory.
 *
 * Parameter:
 *  @path: input directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_rmdir(const char *path)
{
    return rmdir(path);
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

    ret = vfs_is_sys_path(fullpath);
    if (ret) {
        aos_free(fullpath);
        return AIC_EACCES;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, true);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    AIC_ASSERT(inode != NULL);

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
    aos_free(fullpath);
    return 0;

errout_with_inode:
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullpath);
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
 * fs_delete - fs delete path.
 *
 * Parameter:
 *  @pathname: input file or directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_delete(const char *pathname)
{
    return fs_unlink(pathname);
}

/**
 * vfs_mprename - fs rename.
 *
 * Parameter:
 *  @oldpath: input old path.
 *  @oldinode: input old inode.
 *  @oldrelpath: input old relpath.
 *  @newpath: input new path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_mprename(const char *oldpath, struct vfs_inode *oldinode,
                        const char *oldrelpath, const char *newpath)
{
    char *subdir = NULL, *newfullpath = NULL;
    struct vfs_inode_search_s newdesc;
    struct vfs_inode *newinode;
    const char *newrelpath;
    int ret, len;

    AIC_ASSERT(oldinode->ops.fstypeops);

    if (oldinode->ops.fstypeops->fileops == NULL ||
        oldinode->ops.fstypeops->fileops->rename == NULL) {
        return AIC_ENOSYS;
    }

    len = vfs_formatpath(NULL, newpath, &newfullpath, &len);
    if (len < 0)
        return len;

    VFS_SETUP_SEARCH(&newdesc, newfullpath, true);
    ret = vfs_inode_find(&newdesc);
    if (ret < 0) {
        goto errout_with_newsearch;
    }

    newinode   = newdesc.node;
    newrelpath = newdesc.relpath;
    AIC_ASSERT(newinode != NULL && newrelpath != NULL);

    if (oldinode != newinode) {
        ret = AIC_EXDEV;
        goto errout_with_newinode;
    }

    if ((strlen(oldrelpath) == strlen(newrelpath)) &&
        !memcmp(oldrelpath, newrelpath, strlen(newrelpath))) {
        ret = 0;
        goto errout_with_newinode;
    }

    if (oldinode->ops.fstypeops->fileops->stat != NULL) {
        struct stat buf;

next_subdir:
        ret = oldinode->ops.fstypeops->fileops->stat(oldinode, newrelpath,
                                                     &buf);
        if (ret >= 0) {
            if (S_ISDIR(buf.st_mode)) {
                char *subdirname;

                if (subdir != NULL) {
                    aos_free(subdir);
                    subdir = NULL;
                }

                subdirname = vfs_getbasename((char *)oldrelpath);

                if (*newrelpath == '\0') {
                    newrelpath = subdirname;
                } else {
                    len = strlen(newrelpath) + strlen(subdirname) + 2;
                    subdir = aos_malloc(len);
                    if (subdir == NULL) {
                        ret = AIC_ENOMEM;
                        goto errout_with_newinode;
                    }
                    snprintf(subdir, len, "%s/%s", newrelpath, subdirname);
                    subdir[len] = '\0';
                    newrelpath = subdir;
                }

                goto next_subdir;
            } else {
                ret = oldinode->ops.fstypeops->fileops->stat(oldinode,
                                                             oldrelpath, &buf);
                if (ret < 0) {
                    goto errout_with_newinode;
                }

                if (oldinode->ops.fstypeops->fileops->unlink) {
                    oldinode->ops.fstypeops->fileops->unlink(oldinode,
                                                             newrelpath);
                }
            }
        }
    }

    ret = oldinode->ops.fstypeops->fileops->rename(oldinode, oldrelpath,
                                                   newrelpath);

errout_with_newinode:
    vfs_inode_release(newinode);

errout_with_newsearch:
    VFS_RELEASE_SEARCH(&newdesc);
    if (subdir != NULL) {
        aos_free(subdir);
    }

    if (newfullpath != NULL) {
        aos_free(newfullpath);
    }

    return ret;
}

/**
 * rename - fs rename.
 *
 * Parameter:
 *  @oldpath: input old path.
 *  @newpath: input new path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int rename(const char *oldpath, const char *newpath)
{
    struct vfs_inode_search_s olddesc;
    struct vfs_inode *oldinode;
    char *fullpath = NULL;
    int ret;

    if (!oldpath || *oldpath == '\0' || !newpath || *newpath == '\0') {
        ret = AIC_EINVAL;
        goto errout;
    }

    ret = vfs_getfullpath(oldpath,  &fullpath);
    if (ret != AIC_SUCCESS)
        return ret;

    VFS_SETUP_SEARCH(&olddesc, fullpath, true);
    ret = vfs_inode_find(&olddesc);
    if (ret < 0) {
        goto errout_with_oldsearch;
    }

    oldinode = olddesc.node;
    AIC_ASSERT(oldinode != NULL);

    if (VFS_INODE_IS_MOUNTPT(oldinode) && *olddesc.relpath != '\0') {
        if ((oldinode->flags & O_ACCMODE) == O_RDONLY) {
            ret = AIC_EROFS;
        } else {
            ret = vfs_mprename(fullpath, oldinode, olddesc.relpath, newpath);
        }
    } else {
        ret = AIC_ENXIO;
    }

    vfs_inode_release(oldinode);

errout_with_oldsearch:
    VFS_RELEASE_SEARCH(&olddesc);
    aos_free(fullpath);

errout:
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

/**
 * fs_rename - fs rename.
 *
 * Parameter:
 *  @oldpath: input old path.
 *  @newpath: input new path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_rename(const char *oldpath, const char *newpath)
{
    return rename(oldpath, newpath);
}

/**
 * vfs_getdirinfo - fs get directory information.
 *
 * Parameter:
 *  @find_info: input find info.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_getdirinfo(struct fs_loopfind_info *find_info)
{
    FS_FILE_INFO *dir_info = (FS_FILE_INFO *)find_info->param;
    struct stat buf;
    int ret;

    ret = 0;
    if (find_info->dent->d_type == DT_REG) {
        dir_info->num++;
        ret = stat(find_info->fullpath, &buf);
        if (ret == 0)
            dir_info->size += buf.st_size;
    }

    return ret;
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
    AIC_ASSERT(inode);

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
    aos_free(fullpath);
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
 * fs_pathisexist - fs path is exist or not.
 *
 * Parameter:
 *  @path: input path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_pathisexist(const char *path)
{
    int ret;
    struct stat buf;

    ret = stat(path, &buf);
    if (ret < 0)
        return ret;

    return 0;
}

/**
 * fs_getinfo - get fs path information.
 *
 * Parameter:
 *  @path: input path.
 *  @fileinfo: input file information ptr.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_getinfo(const char *path, FS_FILE_INFO *fileinfo)
{
    struct fs_loopfind_info *findinfo;
    int ret;

    aos_memset(fileinfo, 0x0, sizeof(FS_FILE_INFO));
    ret = stat(path, &fileinfo->buf);
    if (ret < 0)
        return ret;

    fileinfo->size = fileinfo->buf.st_size;

    if (S_ISDIR(fileinfo->buf.st_mode)) {
        findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
        AIC_ASSERT(findinfo);

        findinfo->st = &fileinfo->buf;
        findinfo->callback = (u32)vfs_getdirinfo;
        findinfo->findtype = FS_FIND_ALLDIR;
        findinfo->param = (void *)fileinfo;
        ret = fs_loopfind(path, findinfo);
        aos_free(findinfo);
        if (ret != AIC_SUCCESS) {
            vfs_seterrno(ret);
            ret = -1;
        }
    }

    return ret;
}

/**
 * lstat - get fs path stat.
 *
 * Parameter:
 *  @path: input path.
 *  @buf: input stat buf.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int lstat(const char *path, struct stat *buf)
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

/**
 * getcwd - get fs current work directory.
 *
 * Parameter:
 *  @buf: input to store path.
 *  @size: input path size.
 *
 * Return:.
 *   Upon successful completion, non empty is returned. Otherwise, null
 *   value is returned.
 */
char *getcwd(char *buf, size_t size)
{
    size_t tmpsize;
    char *cwd;

    if (buf == NULL || size == 0) {
        vfs_seterrno(AIC_EINVAL);
        return NULL;
    }

    cwd = vfs_getcwd();
    if (cwd == NULL) {
        return NULL;
    } else {
        tmpsize = strlen(cwd);
        if (size < tmpsize) {
            vfs_seterrno(AIC_EINVAL);
            return NULL;
        }
        aos_memcpy(buf, cwd, tmpsize);
        buf[tmpsize] = '\0';
    }

    return buf;
}

/**
 * chdir - set fs work path.
 *
 * Parameter:
 *  @path: input path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int chdir(const char *path)
{
    struct stat st;
    int ret;

    if (path == NULL) {
        vfs_seterrno(AIC_EFAULT);
        return -1;
    }

    ret = stat(path, &st);
    if (ret < 0)
        return -1;

    ret = vfs_setcwd(path);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

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
    AIC_ASSERT(inode != NULL);

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
    aos_free(fullpath);
    return AIC_SUCCESS;

errout_with_inode:
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullpath);
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
 * access - fs access path or not.
 *
 * Parameter:
 *  @path: input path.
 *  @amode: input access path mode.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int access(const char *path, int amode)
{
    struct stat st;

    if (stat(path, &st)) {
        return -1;
    }

    if (st.st_mode & S_IFDIR) {
        return 0;
    }

    if (amode & O_WRONLY) {
        if (st.st_mode & S_IWUSR) {
            return 0;
        }

        return -1;
    }

    return 0;
}

/**
 * isatty - fs is atty or not.
 *
 * Parameter:
 *  @fd: input file description.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int isatty(int fd)
{
    vfs_seterrno(AIC_ENOSYS);
    return -1;
}
#ifdef CONFIG_PLAT_RAC_LIBC
FUNC_ALIAS(isatty, _isatty, (int fd), int);
#endif

/**
 * vfs_refind - fs recursion find file.
 *
 * Parameter:
 *  @findinfo: input find information.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int vfs_refind(struct fs_loopfind_info *findinfo)
{
    struct fs_loopfind_info *nextfindinfo;
    struct dirent *dent;
    FS_LOOPFIND find_cb;
    int ret, ret1;
    DIR *dir;

    ret = 0;
    ret1 = 0;
    dir = opendir(findinfo->dir);
    if (!dir) {
        ret = -(vfs_geterrno());
        return ret;
    }

    while (ret == 0) {
        dent = readdir(dir);
        if (dent) {
            if (findinfo->dir[strlen(findinfo->dir) - 1] == '/' ||
                findinfo->dir[strlen(findinfo->dir) - 1] == '\\' ) {
                snprintf(findinfo->fullpath, PATH_MAX, "%s%s", findinfo->dir,
                         dent->d_name);
            } else if (findinfo->dir[0] == '\0') {
                snprintf(findinfo->fullpath, PATH_MAX, "%s", dent->d_name);
            } else {
                snprintf(findinfo->fullpath, PATH_MAX, "%s/%s", findinfo->dir,
                         dent->d_name);
            }

            if (findinfo->callback) {
                findinfo->dent = dent;
                find_cb = (FS_LOOPFIND)findinfo->callback;
                ret = find_cb(findinfo);
                if (ret != 0)
                    break;
            }

            if (dent->d_type & DT_DIR) {
                findinfo->findtype &= ~(1 << findinfo->dirlevel);
                if (findinfo->findtype) {
                    nextfindinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
                    if (!nextfindinfo) {
                        ret = AIC_ENOMEM;
                        break;
                    }

                    nextfindinfo->dir = findinfo->fullpath;
                    nextfindinfo->dirlevel = findinfo->dirlevel + 1;
                    nextfindinfo->findtype = findinfo->findtype;
                    nextfindinfo->deltype = findinfo->deltype;
                    nextfindinfo->callback = findinfo->callback;
                    nextfindinfo->param = findinfo->param;
                    ret = vfs_refind(nextfindinfo);
                    aos_free(nextfindinfo);
                    if (ret != AIC_SUCCESS)
                        break;

                    if (findinfo->deltype == FS_DEL_ALL) {
                        ret = unlink(findinfo->fullpath);
                        if (ret != AIC_SUCCESS) {
                            ret = -(fs_geterrno());
                            if (ret != AIC_EACCES)
                                break;
                            else
                                ret = AIC_SUCCESS;
                        }
                    }
                }
            } else if (dent->d_type & DT_REG) {
                if (findinfo->deltype & FS_DEL_FILE) {
                    ret = unlink(findinfo->fullpath);
                    if (ret != AIC_SUCCESS) {
                        ret = -(fs_geterrno());
                        if (ret != AIC_EACCES)
                            break;
                        else
                            ret = AIC_SUCCESS;
                    }
                }
            }
        } else {
            ret = -(vfs_geterrno());
            break;
        }
    }

    if (ret == AIC_ENOENT)
        ret = AIC_SUCCESS;

    if (dir)
        ret1 = closedir(dir);

    if (ret == AIC_SUCCESS)
        ret = ret1;

    return ret;
}

/**
 * fs_loopfind - fs loop find file.
 *
 * Parameter:
 *  @path: input find path.
 *  @findinfo: input find information.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_loopfind(const char *path, struct fs_loopfind_info *findinfo)
{
    char *tmppath, *fullPath;
    struct dirent *dent;
    FS_LOOPFIND find_cb;
    boolean flag;
    int ret;

    tmppath = aos_zalloc(strlen(path) + 1);
    aos_memcpy(tmppath, path, strlen(path));

    if (tmppath[strlen(tmppath) - 1] == '*') {
        if (!findinfo->deltype)
            findinfo->findtype = FS_FIND_SUBDIR;

        tmppath[strlen(tmppath) - 1] = '\0';
        if (tmppath[strlen(tmppath) - 1] == '*') {
            if (!findinfo->deltype)
                findinfo->findtype = FS_FIND_ALLDIR;
            tmppath[strlen(tmppath) - 1] = '\0';
        }

        if ((strlen("/") == strlen(tmppath)) &&
            !memcmp("/", tmppath, strlen(tmppath))) {
            tmppath[strlen(tmppath) - 1] = '\0';
        }
    }

    ret = vfs_getfullpath(tmppath, &fullPath);
    aos_free(tmppath);
    if (ret != AIC_SUCCESS) {
        vfs_seterrno(ret);
        return -1;
    }
    tmppath = fullPath;

    flag = 0;
    if (!findinfo->st) {
        findinfo->st = aos_malloc(sizeof(struct stat));
        if (!findinfo->st) {
            vfs_seterrno(AIC_ENOMEM);
            aos_free(tmppath);
            return -1;
        }
        flag = 1;

        ret = stat(tmppath, findinfo->st);
        if (ret != 0) {
            aos_free(findinfo->st);
            aos_free(tmppath);
            return -1;
        }
    }

    if (S_ISREG(findinfo->st->st_mode)) {
        if (findinfo->callback) {
            dent = aos_zalloc(sizeof(struct dirent));
            if (!dent) {
                vfs_seterrno(AIC_ENOMEM);
                return -1;
            } else {
                aos_memcpy(dent->d_name, tmppath, strlen(tmppath));
                dent->d_type = S_ISDIR(findinfo->st->st_mode) ? DT_DIR : DT_REG;
                dent->d_reclen = ALIGN((sizeof(struct dirent) -
                                        sizeof(dent->d_name) +
                                        strlen(dent->d_name) + 1),
                                        4);

                findinfo->dent = dent;
                findinfo->dir = tmppath;
                aos_memcpy(findinfo->fullpath, tmppath, strlen(tmppath));
                findinfo->dirlevel = 0;
                find_cb = (FS_LOOPFIND)findinfo->callback;
                ret = find_cb(findinfo);
                aos_free(dent);
            }
        }

        if (findinfo->deltype & FS_DEL_FILE) {
            ret = unlink(tmppath);
        }
    } else {
        findinfo->dir = tmppath;
        findinfo->dirlevel = 0;
        ret = vfs_refind(findinfo);
        if (ret != 0) {
            vfs_seterrno(ret);
            ret = -1;
        } else {
            if (findinfo->deltype == FS_DEL_ALL) {
                if ((tmppath[0] == '/') && (strlen(tmppath) == 2)) {
                    ret = fs_create_default_dir(tmppath);
                } else {
                    ret = unlink(tmppath);
                }
            }
        }
    }

    if (flag)
        aos_free(findinfo->st);
    aos_free(tmppath);

    return ret;
}

/**
 * vfs_filecopy - copy source file to destination directory.
 *
 * Parameter:
 * @src_path: input source file path.
 * @dst_path: input destination directory path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_filecopy(const char *srcpath, const char *dstpath)
{
    int errcode, src_fd = -1, dst_fd = -1;
    char *buf = NULL;
    struct stat *st;
    ssize_t rsize;
    int ret;

    do {
        st = aos_zalloc(sizeof(struct stat));
        if (!st) {
            vfs_seterrno(AIC_ENOMEM);
            return -1;
        }

        src_fd = open(srcpath, O_RDONLY);
        if (src_fd < 0) {
            errcode = src_fd;
            break;
        }

        ret = stat(srcpath, st);
        if (ret != 0) {
            aos_free(st);
            return ret;
        }

        if (st->st_size == 0) {
            errcode = AIC_SUCCESS;
            break;
        }

        dst_fd = open(dstpath, O_RDWR | O_CREAT);
        if (dst_fd < 0) {
            errcode = dst_fd;
            break;
        }

        buf = aos_zalloc(st->st_size);
        if (!buf) {
            vfs_seterrno(AIC_ENOMEM);
			errcode = -1;
            break;
        }

        rsize = lseek(src_fd, 0, SEEK_SET);
        if (rsize < 0) {
            errcode = rsize;
            break;
        }

        rsize = lseek(dst_fd, 0, SEEK_SET);
        if (rsize < 0) {
            errcode = rsize;
            break;
        }

        rsize = read(src_fd, buf, st->st_size);
        if (rsize < 0) {
            errcode = rsize;
            break;
        }

        rsize = write(dst_fd, buf, st->st_size);
        if (rsize < 0) {
            errcode = rsize;
            break;
        }

        errcode = close(src_fd);
        if (errcode != AIC_SUCCESS) {
            break;
        }
        src_fd = -1;

        errcode = close(dst_fd);
        if (errcode != AIC_SUCCESS) {
            break;
        }
        dst_fd = -1;
    } while(0);

    if (st) {
        aos_free(st);
    }

    if (buf) {
        aos_free(buf);
    }

    if (src_fd >= 0)
        fs_close(src_fd);

    if (dst_fd >= 0)
        fs_close(dst_fd);

    return errcode;
}

/**
 * vfs_loopcopy - loop copy source file to destination directory.
 *
 * Parameter:
 *  @findinfo: input find information.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_loopcopy(struct fs_loopfind_info *findinfo)
{
    struct fs_dir_path *dirpath = (struct fs_dir_path *)findinfo->param;
    int ret;

    if (findinfo->dent->d_type == DT_DIR) {
        aos_memset(dirpath->path, 0, NAME_MAX);
        if (strlen(dirpath->srcpath) == strlen(findinfo->fullpath)) {
            snprintf(dirpath->path, NAME_MAX, "%s/%s",
                     dirpath->dstpath, findinfo->dent->d_name);
        } else {
            snprintf(dirpath->path, NAME_MAX, "%s/%s",
                     dirpath->dstpath,
                     &findinfo->fullpath[strlen(dirpath->srcpath) + 1]);
        }
        ret = mkdir(dirpath->path, O_RDWR | O_CREAT);
    } else {
        aos_memset(dirpath->path, 0, NAME_MAX);
        if (strlen(dirpath->srcpath) == strlen(findinfo->fullpath)) {
            snprintf(dirpath->path, NAME_MAX, "%s", dirpath->dstpath);
        } else {
            snprintf(dirpath->path, NAME_MAX, "%s/%s", dirpath->dstpath,
                     &findinfo->fullpath[strlen(dirpath->srcpath) + 1]);
        }

        ret = vfs_filecopy(findinfo->fullpath, dirpath->path);
    }

    return ret;
}

/**
 * fs_copy - copy source file to destination directory.
 *
 * Parameter:
 * @src_path: input source file path.
 * @dst_path: input destination directory path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_copy(const char *srcpath, const char *dstpath)
{
    struct fs_loopfind_info findinfo;
    struct fs_dir_path *dirpath;
    struct stat *st;
    int ret, len;

    st = aos_malloc(sizeof(struct stat));
    if (!st) {
        vfs_seterrno(AIC_ENOMEM);
        return -1;
    }

    ret = stat(srcpath, st);
    if (ret != 0) {
        aos_free(st);
        return ret;
    }

    if (S_ISREG(st->st_mode)) {
        ret = vfs_filecopy(srcpath, dstpath);
    } else {
        dirpath = (struct fs_dir_path *)aos_zalloc(sizeof(struct fs_dir_path));
        AIC_ASSERT(dirpath);

        len = vfs_formatpath(NULL, srcpath, &dirpath->srcpath, &len);
        AIC_ASSERT(len > 0);

        len = vfs_formatpath(NULL, dstpath, &dirpath->dstpath, &len);
        AIC_ASSERT(len > 0);

        findinfo.dir = (char *)srcpath;
        findinfo.dirlevel = 0;
        findinfo.findtype = FS_FIND_ALLDIR;
        findinfo.deltype = 0;
        findinfo.callback = (u32)vfs_loopcopy;
        findinfo.param = dirpath;

        ret = vfs_refind(&findinfo);
        if (ret != AIC_SUCCESS) {
            vfs_seterrno(ret);
            ret = -1;
        }
        aos_free(dirpath->srcpath);
        aos_free(dirpath->dstpath);
    }

    aos_free(st);

    return ret;
}

/**
 * fs_move - move source file to destination directory.
 *
 * Parameter:
 * @src_path: input source file path.
 * @dst_path: input destination directory path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_move(const char *srcpath, const char *dstpath)
{
    struct fs_loopfind_info findinfo;
    struct fs_dir_path *dirpath;
    struct stat *st;
    int ret, len;

    st = aos_zalloc(sizeof(struct stat));
    if (!st) {
        vfs_seterrno(AIC_ENOMEM);
        return -1;
    }

    ret = stat(srcpath, st);
    if (ret != 0) {
        aos_free(st);
        return -1;
    }

    if (S_ISREG(st->st_mode)) {
        ret = vfs_filecopy(srcpath, dstpath);
        if (ret == AIC_SUCCESS)
            ret = unlink(srcpath);
    } else {
        dirpath = (struct fs_dir_path *)aos_zalloc(sizeof(struct fs_dir_path));
        AIC_ASSERT(dirpath);

        len = vfs_formatpath(NULL, srcpath, &dirpath->srcpath, &len);
        AIC_ASSERT(len > 0);

        len = vfs_formatpath(NULL, dstpath, &dirpath->dstpath, &len);
        AIC_ASSERT(len > 0);

        findinfo.dir = (char *)srcpath;
        findinfo.dirlevel = 0;
        findinfo.findtype = FS_FIND_ALLDIR;
        findinfo.deltype = FS_DEL_ALL;
        findinfo.callback = (u32)vfs_loopcopy;
        findinfo.param = dirpath;

        ret = vfs_refind(&findinfo);
        if (ret != AIC_SUCCESS) {
            vfs_seterrno(ret);
            ret = -1;
        }
        aos_free(dirpath->srcpath);
        aos_free(dirpath->dstpath);
        aos_free(dirpath);
    }

    aos_free(st);

    return ret;
}

/**
 * vfs_adddir - add directory information to list..
 *
 * Parameter:
 * @findinfo: input file information.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
static int vfs_adddir(struct fs_loopfind_info *findinfo)
{
    struct list_head *list = (struct list_head *)findinfo->param;
    struct fs_dir_info *dir;

    dir = aos_zalloc(sizeof(struct fs_dir_info ));
    AIC_ASSERT(dir);
    aos_memcpy(&dir->dent, findinfo->dent, sizeof(struct dirent));

    list_add(&dir->node, list);

    return AIC_SUCCESS;
}

/**
 * fs_lsdir - ls directory.
 *
 * Parameter:
 * @path: input directory path.
 * @list: input to store file list.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_lsdir(const char *path, struct list_head *list)
{
    struct fs_loopfind_info findinfo;
    struct stat *st;
    int ret;

    st = aos_zalloc(sizeof(struct stat));
    if (!st) {
        vfs_seterrno(AIC_ENOMEM);
        return -1;
    }

    ret = stat(path, st);
    if (ret != 0) {
        aos_free(st);
        return -1;
    }

    if (S_ISDIR(st->st_mode)) {
        findinfo.dir = (char *)path;
        findinfo.dirlevel = 0;
        findinfo.findtype = FS_FIND_CURDIR;
        findinfo.deltype = 0;
        findinfo.callback = (u32)vfs_adddir;
        findinfo.param = (void *)list;

        ret = vfs_refind(&findinfo);
        if (ret != AIC_SUCCESS) {
            vfs_seterrno(ret);
            ret = -1;
        }
    }

    aos_free(st);

    return ret;
}

/**
 * lsdir - ls directory.
 *
 * Parameter:
 * @path: input directory path.
 * @list: input to store file list.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int lsdir(const char *path, struct list_head *list)
{
    return fs_lsdir(path, list);
}

/**
 * fs_getpos - get file position.
 *
 * Parameter:
 * @fd: input file description.
 *
 * Return:
 *   The resulting offset on success.  A negated errno value is returned on
 *   any failure.
 */
off_t fs_getpos(int fd)
{
    struct vfs_file *file;
    off_t ret;

    ret = vfs_fd_getfile(fd, &file);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    AIC_ASSERT(file != NULL);

    return file->pos;
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

/**
 * fs_longname_get - get long name by short name.
 *
 * Parameter:
 * @path: input short name path.
 * @longname: input long name ptr.
 * @longname_len: input long name len.
 *
 * Return:
 *  Upon successful completion, 0 is returned. Otherwise, a negated errno
 *  value is returned.
 */
int fs_longname_get(char *path, char *longname, u32 longname_len)
{
    char *fullpath = NULL, *tmppath = NULL;
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    int ret, ret1, lastslash;
    struct vfs_dir *dir;

    ret1 = 0;

    if (path == NULL || longname == NULL || longname_len == 0) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    lastslash = vfs_getlastslash(path);
    if (lastslash == 0) {
        vfs_seterrno(AIC_EINVAL);
        return -1;
    }

    tmppath = aos_zalloc(strlen(path) + 2);
    AIC_ASSERT(tmppath);

    aos_memcpy(tmppath, path, lastslash);

    ret = vfs_getfullpath(tmppath,  &fullpath);
    if (ret != AIC_SUCCESS) {
        aos_free(tmppath);
        vfs_seterrno(ret);
        return -1;
    }

    if (((strlen("/") == strlen(fullpath)) &&
        !memcmp("/", fullpath, strlen(fullpath))) ||
        ((strlen("/dev") == strlen(fullpath)) &&
         !memcmp("/dev", fullpath, strlen(fullpath)))) {
        aos_free(fullpath);
        aos_free(tmppath);
        vfs_seterrno(AIC_EISDIR);
        return -1;
    }

    dir = (struct vfs_dir *)aos_zalloc(sizeof(struct vfs_dir));
    if (dir == NULL) {
        aos_free(fullpath);
        aos_free(tmppath);
        vfs_seterrno(AIC_ENOMEM);
        return -1;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, 0);
    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    AIC_ASSERT(inode != NULL);
    dir->inode = inode;
    dir->pos = 0;

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(inode)) {
        ret = AIC_ENOSYS;
        if (inode->ops.fstypeops &&
            inode->ops.fstypeops->fileops &&
            inode->ops.fstypeops->fileops->opendir &&
            inode->ops.fstypeops->fileops->closedir &&
            inode->ops.fstypeops->fileops->longname_get) {
            ret = inode->ops.fstypeops->fileops->opendir(dir, desc.relpath);
        }
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

    ret = inode->ops.fstypeops->fileops->longname_get(inode, path + lastslash + 1,
                                                        longname, longname_len);

    ret1 = inode->ops.fstypeops->fileops->closedir(dir);

errout_with_inode:
    dir->inode = NULL;
    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullpath);
    aos_free(dir);
    aos_free(tmppath);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    } else if (ret1 < 0) {
        vfs_seterrno(ret1);
        return -1;
    }

    return 0;
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
int fs_setfileattri(const char *path, u32 attri)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    char *fullpath = NULL;
    int ret;

    if (path == NULL) {
        return AIC_EINVAL;
    }

    ret = vfs_getfullpath(path, &fullpath);
    if (ret < 0)
        return ret;

    ret = vfs_is_sys_path(fullpath);
    if (ret) {
        aos_free(fullpath);
        return AIC_EACCES;
    }

    if (((strlen("/") == strlen(fullpath)) &&
        !memcmp("/", fullpath, strlen(fullpath))) ||
        ((strlen("/dev") == strlen(fullpath)) &&
         !memcmp("/dev", fullpath, strlen(fullpath)))) {
        aos_free(fullpath);
        return AIC_EISDIR;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, true);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    AIC_ASSERT(inode);

    ret = AIC_EBADF;
    if (VFS_INODE_IS_MOUNTPT(inode)) {
        if (inode->ops.fstypeops && inode->ops.fstypeops->fileops &&
            inode->ops.fstypeops->fileops->fileattri_set) {
            ret = inode->ops.fstypeops->fileops->fileattri_set(inode, desc.relpath, attri);
        } else {
            ret = AIC_ENOSYS;
        }
    }

    vfs_inode_release(inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullpath);
    return ret;
}
