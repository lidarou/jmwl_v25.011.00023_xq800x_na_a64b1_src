/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * DESCRIPTION
 *
 * This file export the file system inode interface.
 */

#ifndef __AIC_VFS_INODE_H__
#define __AIC_VFS_INODE_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include "vfs.h"
#ifdef WIN32
#include "vfs_win32.h"
#else
#include <sys/syslimits.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/
#define VFS_SETUP_SEARCH(d,p,n) \
  do \
    { \
      (d)->path     = (p); \
      (d)->node     = NULL; \
      (d)->peer     = NULL; \
      (d)->parent   = NULL; \
      (d)->relpath  = NULL; \
      (d)->buffer   = NULL; \
      (d)->nofollow = (n); \
    } \
  while (0)

#define VFS_RELEASE_SEARCH(d) \
  do \
    { \
      if ((d)->buffer != NULL) \
        { \
          aos_free((d)->buffer); \
          (d)->buffer  = NULL; \
        } \
    } \
  while (0)

/**---------------------------------------------------------------------------*
 ** Public Types
 **---------------------------------------------------------------------------*/
struct vfs_inode_search_s
{
    const char          *path;
    struct vfs_inode     *node;
    struct vfs_inode     *peer;
    struct vfs_inode     *parent;
    const char          *relpath;
    char                *buffer;
    bool                nofollow;
};

typedef int (*vfs_foreach_inode_t)(struct vfs_inode *node,
                                   char dirpath[PATH_MAX],
                                   void *arg);

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
void vfs_inode_init(void);
int vfs_inode_lock(void);
void vfs_inode_unlock(void);
int vfs_inode_search(struct vfs_inode_search_s *desc);
int vfs_inode_create(const char *path, mode_t mode, struct vfs_inode **inode);
int vfs_inode_find(struct vfs_inode_search_s *desc);
int vfs_inode_addref(struct vfs_inode *inode);
int vfs_foreach_inode(vfs_foreach_inode_t handler, void *arg);
int vfs_inode_getpath(struct vfs_inode *node, char *path);
void vfs_inode_free(struct vfs_inode *node);
int vfs_inode_remove(const char *path);
struct vfs_inode *vfs_inode_unlink(const char *path);
void vfs_inode_release(struct vfs_inode *node);
void vfs_get_rootinode(struct vfs_inode **inode);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_VFS_INODE_H__ */

