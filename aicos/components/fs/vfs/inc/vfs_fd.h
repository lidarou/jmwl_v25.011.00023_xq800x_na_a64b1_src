/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * DESCRIPTION
 *
 * This file export the file system file description interface.
 */

#ifndef __AIC_VFS_FD_H__
#define __AIC_VFS_FD_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

struct vfs_file;
struct vfs_inode;

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
void vfs_fdtable_init(void);
int vfs_fd_alloc(struct vfs_inode *inode, struct vfs_file *file,
                 int minfd, bool addref);
int vfs_fd_release(int fd);
int vfs_fd_getfile(int fd, struct vfs_file **file);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_VFS_FD_H__ */

