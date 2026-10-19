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

#include <aic_type.h>
#include "vfs.h"

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/
#define VFS_FD_MAX_ROW_NUM                   1
#define VFS_FD_MAX_COLUMN_NUM                OPEN_MAX
#define VFS_FD_MUTEX_NAME                    "VFS_FD"

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
 ** Public Functions
 **---------------------------------------------------------------------------*/
int vfs_fd_alloc(struct vfs_inode *inode, struct vfs_file *file,
                 int minfd, boolean addref)
{
    struct vfs_fdtable *fdtable;
    int i, j;

    fdtable = &g_vfs_fdtable;
    ASSERT(fdtable != NULL);

    i = minfd / VFS_FD_MAX_COLUMN_NUM;
    if (i >= fdtable->fd_rows) {
        ASSERT(0);
    }

    j = minfd % VFS_FD_MAX_COLUMN_NUM;
    do {
        do {
            if (!fdtable->fds[i][j].inode) {
                memcpy(&fdtable->fds[i][j], file, sizeof(struct vfs_file));

                if (addref) {
                    vfs_inode_addref(inode);
                }

                return i * VFS_FD_MAX_COLUMN_NUM + j;
            }
        } while (++j < VFS_FD_MAX_COLUMN_NUM);

        j = 0;
    } while (++i < fdtable->fd_rows);

    ASSERT(0);

    return AIC_ENFILE;
}

int vfs_fd_getfile(int fd, struct vfs_file **file)
{
    struct vfs_fdtable *fdtable;
    int ret = AIC_SUCCESS;

    ASSERT(file != NULL);
    *file = NULL;

    fdtable = &g_vfs_fdtable;

    if (fd < 0 || fd >= fdtable->fd_rows * VFS_FD_MAX_COLUMN_NUM) {
        return AIC_EBADF;
    }

    *file = &fdtable->fds[fd / VFS_FD_MAX_COLUMN_NUM]
                          [fd % VFS_FD_MAX_COLUMN_NUM];

    if (!(*file)->inode)
    {
        *file = NULL;
        ret = AIC_EBADF;
    }

    return ret;
}

int vfs_fd_release(int fd)
{
    struct vfs_fdtable *fdtable;
    struct vfs_file     *file;

    fdtable = &g_vfs_fdtable;
    ASSERT(fdtable != NULL);

    if (fd < 0 || fd >= fdtable->fd_rows * VFS_FD_MAX_COLUMN_NUM ||
        !&fdtable->fds[fd / VFS_FD_MAX_COLUMN_NUM]
                      [fd % VFS_FD_MAX_COLUMN_NUM]) {
        return AIC_EBADF;
    }

    file = &fdtable->fds[fd / VFS_FD_MAX_COLUMN_NUM]
                         [fd % VFS_FD_MAX_COLUMN_NUM];
    memset(file, 0, sizeof(struct vfs_file));

    return AIC_SUCCESS;
}

void vfs_fdtable_init(void)
{
    u32 i, len;

    g_vfs_fdtable.fd_rows = (u8)VFS_FD_MAX_ROW_NUM;

    len = VFS_FD_MAX_ROW_NUM * sizeof(u32);
    g_vfs_fdtable.fds = malloc(len);
    ASSERT_INFO(g_vfs_fdtable.fds, "malloc size(0x%x) memory fail.\n", len);
    memset(g_vfs_fdtable.fds, 0x0, len);

    for (i =0 ; i < VFS_FD_MAX_ROW_NUM; i++) {
        len = sizeof(struct vfs_file) * VFS_FD_MAX_COLUMN_NUM;
        g_vfs_fdtable.fds[i] = malloc(len);
        ASSERT_INFO(g_vfs_fdtable.fds[i], "malloc size(0x%x) memory fail.\n",
                    len);
        memset(g_vfs_fdtable.fds[i], 0x0, len);
    }
}

