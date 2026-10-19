/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * DESCRIPTION
 *
 * This file export the file system check interface.
 */

#ifndef __AIC_VFS_CHECK_H__
#define __AIC_VFS_CHECK_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
size_t vfs_formatpath(const char *cwd, const char *path,
                      char **buf, size_t *bufsize);
int vfs_pathcheck(const char *path, boolean isFile);
int vfs_getfullpath(const char *path, char **fullpath);
int vfs_inode_checkflags(struct vfs_inode *inode, int oflags);
char *vfs_get_default_target_path();
u32 vfs_getumask();
char *vfs_getbasename(char *path);
char *vfs_getfilename(const char *path);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_VFS_CHECK_H__ */

