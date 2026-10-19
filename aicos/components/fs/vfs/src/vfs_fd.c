/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system file description interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1e08b0
#define LOG_MODULE    AMOD_FS

#include <aic_type.h>
#include <aic_osa.h>
#include "vfs.h"

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/
#define VFS_FD_MAX_ROW_NUM                   1
#define VFS_FD_MAX_COLUMN_NUM                OPEN_MAX
#define VFS_FD_MUTEX_NAME                    "VFSFD"

/**---------------------------------------------------------------------------*
 ** Private Types
 **---------------------------------------------------------------------------*/
struct vfs_fdtable
{
    void              *fd_lock;
    uint8_t           fd_rows;
    struct vfs_file   **fds;
};

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
struct vfs_fdtable g_vfs_fdtable;

/**---------------------------------------------------------------------------*
 ** Private Functions
 **---------------------------------------------------------------------------*/
static void vfs_forcesync()
{
    struct vfs_fdtable *fdtable;
    struct vfs_file *file;
    int i, j;

    fdtable = &g_vfs_fdtable;
    if (vfs_get_mutex(fdtable->fd_lock, AOS_WAIT_FOREVER) < 0) {
        return;
    }

    for (i = 0; i < fdtable->fd_rows; i++) {
        for (j = 0; j < VFS_FD_MAX_COLUMN_NUM; j++) {
            file = &fdtable->fds[i][j];
            if (file != NULL && file->inode != NULL) {
              //file_fsync(file);
            }
        }
    }

    vfs_put_mutex(fdtable->fd_lock);
}

static int vfs_fd_extend(struct vfs_fdtable *fdtable, size_t row)
{
    struct vfs_file **tmp;
    u8 i;

    if (row <= fdtable->fd_rows) {
        return 0;
    }

    if (row * VFS_FD_MAX_COLUMN_NUM > OPEN_MAX) {
        return AIC_EMFILE;
    }

    tmp = aos_zalloc(sizeof(struct vfs_file *) * row);
    AIC_ASSERT(tmp);
    if (tmp == NULL) {
        return AIC_ENFILE;
    }

    i = fdtable->fd_rows;
    do
    {
        tmp[i] = aos_zalloc(sizeof(struct vfs_file) * VFS_FD_MAX_COLUMN_NUM);
        if (tmp[i] == NULL) {
            while (--i >= fdtable->fd_rows) {
                aos_free(tmp[i]);
            }

            aos_free(tmp);
            return AIC_ENFILE;
        }
    } while (++i < row);

    {
        aos_memcpy(tmp, fdtable->fds,
                   sizeof(struct fs_file *) * fdtable->fd_rows);
        aos_free(fdtable->fds);
        fdtable->fds = tmp;
    }

    fdtable->fds = tmp;
    fdtable->fd_rows = row;

    return 0;
}

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
int vfs_fd_alloc(struct vfs_inode *inode, struct vfs_file *file,
                 int minfd, bool addref)
{
    struct vfs_fdtable *fdtable;
    int ret, i, j;

    fdtable = &g_vfs_fdtable;
    AIC_ASSERT(fdtable != NULL);

    ret = vfs_get_mutex(fdtable->fd_lock, AOS_WAIT_FOREVER);
    if (ret < 0) {
        return ret;
    }

    i = minfd / VFS_FD_MAX_COLUMN_NUM;
    if (i >= fdtable->fd_rows) {
        ret = vfs_fd_extend(fdtable, i + 1);
        if (ret < 0) {
            vfs_put_mutex(fdtable->fd_lock);
            return ret;
        }
    }

    j = minfd % VFS_FD_MAX_COLUMN_NUM;
    do {
        do {
            if (!fdtable->fds[i][j].inode) {
                aos_memcpy(&fdtable->fds[i][j], file, sizeof(struct vfs_file));
                vfs_put_mutex(fdtable->fd_lock);

                if (addref) {
                    vfs_inode_addref(inode);
                }

                return i * VFS_FD_MAX_COLUMN_NUM + j;
            }
        } while (++j < VFS_FD_MAX_COLUMN_NUM);

        j = 0;
    } while (++i < fdtable->fd_rows);

    ret = vfs_fd_extend(fdtable, i + 1);
    if (ret < 0) {
        vfs_put_mutex(fdtable->fd_lock);
        return ret;
    }

    aos_memcpy(&fdtable->fds[i][0], file, sizeof(struct vfs_file));
    vfs_put_mutex(fdtable->fd_lock);

    if (addref) {
        vfs_inode_addref(inode);
    }

    return i * VFS_FD_MAX_COLUMN_NUM;
}

int vfs_fd_getfile(int fd, struct vfs_file **file)
{
    struct vfs_fdtable *fdtable;
    int ret;

    AIC_ASSERT(file != NULL);
    *file = NULL;

    fdtable = &g_vfs_fdtable;

    if (fd < 0 || fd >= fdtable->fd_rows * VFS_FD_MAX_COLUMN_NUM) {
        return AIC_EBADF;
    }

    ret = vfs_get_mutex(fdtable->fd_lock, AOS_WAIT_FOREVER);
    if (ret < 0) {
        return ret;
    }

    *file = &fdtable->fds[fd / VFS_FD_MAX_COLUMN_NUM]
                          [fd % VFS_FD_MAX_COLUMN_NUM];

    if (!(*file)->inode)
    {
        *file = NULL;
        ret = AIC_EBADF;
    }

    vfs_put_mutex(fdtable->fd_lock);
    return ret;
}
#if 0

int vfiles_duplist(struct vfs_fdtable *pfdtable, struct vfs_fdtable *cfdtable)
{
    int ret;
    int i;
    int j;

    AIC_ASSERT(cfdtable);
    AIC_ASSERT(pfdtable);

    ret = fs_get_mutex(pfdtable->fd_lock, AOS_WAIT_FOREVER);
    if (ret < 0) {
        return ret;
    }

    for (i = 0; i < pfdtable->fd_rows; i++) {
        for (j = 0; j < VFS_FD_MAX_COLUMN_NUM; j++) {
            struct file *filep;
#ifdef CONFIG_FDCLONE_STDIO

            /* Determine how many file descriptors to clone.  If
            * CONFIG_FDCLONE_DISABLE is set, no file descriptors will be
            * cloned.  If CONFIG_FDCLONE_STDIO is set, only the first
            * three descriptors (stdin, stdout, and stderr) will be
            * cloned.  Otherwise all file descriptors will be cloned.
            */

            if (i * VFS_FD_MAX_COLUMN_NUM + j >= 3) {
                goto out;
            }
#endif

            filep = &pfdtable->fds[i][j];
            AIC_ASSERT(filep);

            if (filep && (filep->f_inode == NULL ||
                (filep->f_oflags & O_CLOEXEC) != 0)) {
                continue;
            }

            ret = files_extend(cfdtable, i + 1);
            if (ret < 0) {
                goto out;
            }

            /* Yes... duplicate it for the child */

            ret = file_dup2(filep, &cfdtable->fds[i][j]);
            if (ret < 0) {
                goto out;
            }
        }
    }

out:
    aos_put_mutex(pfdtable->fd_lock);
    return ret;
}


int vfs_dup2(int fd1, int fd2)
{
    struct fdtable *fdtable;
    struct file     *filep;
    struct file      file;
    int ret;

    if (fd1 == fd2) {
        return fd1;
    }

    fdtable = &g_fs_fdtable;
    AIC_ASSERT(fdtable != NULL);

    if (fd1 < 0 || fd1 >= VFS_FD_MAX_COLUMN_NUM * fdtable->fd_rows ||
        fd2 < 0) {
        return AIC_EBADF;
    }

    ret = fs_get_mutex(fdtable->fd_lock, AOS_WAIT_FOREVER);
    if (ret < 0) {
        return ret;
    }

    if (fd2 >= VFS_FD_MAX_COLUMN_NUM * fdtable->fd_rows) {
        ret = files_extend(fdtable, fd2 / VFS_FD_MAX_COLUMN_NUM + 1);
        if (ret < 0) {
            fs_put_mutex(fdtable->fd_lock);
            return ret;
        }
    }

    filep = &fdtable->fds[fd2 / VFS_FD_MAX_COLUMN_NUM]
                         [fd2 % VFS_FD_MAX_COLUMN_NUM];
    memcpy(&file, filep, sizeof(struct file));
    memset(filep, 0,     sizeof(struct file));

    ret = file_dup2(&fdtable->fds[fd1 / VFS_FD_MAX_COLUMN_NUM]
                                 [fd1 % VFS_FD_MAX_COLUMN_NUM],
                    filep);
    fs_put_mutex(fdtable->fd_lock);

    file_close(&file);
    return ret < 0 ? ret : fd2;
}

int vfs_dup2(int fd1, int fd2)
{
    int ret;

    ret = nx_dup2(fd1, fd2);
    if (ret < 0) {
        set_errno(-ret);
        ret = -1;
    }

    return ret;
}
#endif

int vfs_fd_release(int fd)
{
    struct vfs_fdtable *fdtable;
    struct vfs_file     *file;
    int                 ret;

    fdtable = &g_vfs_fdtable;
    AIC_ASSERT(fdtable != NULL);

    ret = vfs_get_mutex(fdtable->fd_lock, AOS_WAIT_FOREVER);
    if (ret < 0) {
        return ret;
    }

    if (fd < 0 || fd >= fdtable->fd_rows * VFS_FD_MAX_COLUMN_NUM ||
        !&fdtable->fds[fd / VFS_FD_MAX_COLUMN_NUM]
                      [fd % VFS_FD_MAX_COLUMN_NUM]) {
        vfs_put_mutex(fdtable->fd_lock);
        return AIC_EBADF;
    }

    file = &fdtable->fds[fd / VFS_FD_MAX_COLUMN_NUM]
                         [fd % VFS_FD_MAX_COLUMN_NUM];
    memset(file, 0, sizeof(struct vfs_file));

    vfs_put_mutex(fdtable->fd_lock);
    return ret;
}

void sync(void)
{
  vfs_forcesync();
}

void vfs_fdtable_init(void)
{
    u32 i;

    g_vfs_fdtable.fd_lock = vfs_create_mutex(VFS_FD_MUTEX_NAME);
    AIC_ASSERT(g_vfs_fdtable.fd_lock);

    g_vfs_fdtable.fd_rows = (u8)VFS_FD_MAX_ROW_NUM;

    g_vfs_fdtable.fds = aos_zalloc(VFS_FD_MAX_ROW_NUM * sizeof(u32));
    AIC_ASSERT(g_vfs_fdtable.fds);

    for (i =0 ; i < VFS_FD_MAX_ROW_NUM; i++) {
        g_vfs_fdtable.fds[i] = aos_zalloc(sizeof(struct vfs_file) * VFS_FD_MAX_COLUMN_NUM);
        AIC_ASSERT(g_vfs_fdtable.fds[i]);
    }
}

