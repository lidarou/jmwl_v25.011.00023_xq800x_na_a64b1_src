/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_VFS_H__
#define __AIC_VFS_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_list.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "dirent.h"
#include "sys/dir.h"
#include "vfs_dev.h"
#include "vfs_inode.h"
#include "vfs_init.h"
#include "vfs_cfg_type.h"
#include "vfs_fd.h"
#include "vfs_type.h"
#include "vfs_check.h"
#include "fs_api.h"
#include "console/console.h"

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
** Pre-processor Definitions
**---------------------------------------------------------------------------*/
#define ALIAS_OF(of) __attribute__((alias(#of)))
#define FUNC_ALIAS(real_func, new_alias, args_list, return_type) \
      return_type new_alias args_list ALIAS_OF(real_func)

#define VFS_INODE_FLAG_TYPE_MASK       0x0000000f
#define VFS_INODE_FLAG_TYPE_DRIVER     0x00000001
#define VFS_INODE_FLAG_TYPE_BLOCK      0x00000002
#define VFS_INODE_FLAG_TYPE_MOUNTPT    0x00000003
#define VFS_INODE_FLAG_TYPE_SOFTLINK   0x00000008
#define VFS_INODE_FLAG_DELETED         0x00000010

#define VFS_INODE_IS_TYPE(i,t) \
  (((i)->flags & VFS_INODE_FLAG_TYPE_MASK) == (t))

#define VFS_INODE_IS_DRIVER(i)    VFS_INODE_IS_TYPE(i, VFS_INODE_FLAG_TYPE_DRIVER)
#define VFS_INODE_IS_BLOCK(i)     VFS_INODE_IS_TYPE(i, VFS_INODE_FLAG_TYPE_BLOCK)
#define VFS_INODE_IS_MOUNTPT(i)   VFS_INODE_IS_TYPE(i, VFS_INODE_FLAG_TYPE_MOUNTPT)

#define VFS_INODE_GET_TYPE(i)     ((i)->flags & VFS_INODE_FLAG_TYPE_MASK)
#define VFS_INODE_SET_TYPE(i,t) \
  do \
    { \
      (i)->flags = ((i)->flags & ~VFS_INODE_FLAG_TYPE_MASK) | (t); \
    } \
  while (0)

#define VFS_INODE_SET_DRIVER(i)   VFS_INODE_SET_TYPE(i, VFS_INODE_FLAG_TYPE_DRIVER)
#define VFS_INODE_SET_BLOCK(i)    VFS_INODE_SET_TYPE(i, VFS_INODE_FLAG_TYPE_BLOCK)
#define VFS_INODE_SET_MOUNTPT(i)  VFS_INODE_SET_TYPE(i, VFS_INODE_FLAG_TYPE_MOUNTPT)

/**---------------------------------------------------------------------------*
 ** Public Type Definitions
 **---------------------------------------------------------------------------*/
enum VFS_msg {
    VFS_MSG_INIT         = BIT_0,
    VFS_MSG_UNINIT       = BIT_1,
    VFS_MSG_DEV_MOUNT    = BIT_2,
    VFS_MSG_DEV_UNMOUNT  = BIT_3,
    VFS_MSG_ERROR        = BIT_31,
    VFS_MSG_MAX,
};

struct vfs_queue_msg {
    u32     msg;
    void    *data;
};

struct vfs_inode_ops {
    struct vfs_type_ops                 *fstypeops;
    uint32_t                            fstypehandle;
    struct vfs_blkdev_ops               *blkdevops;
    uint32_t                            devhandle;
    void                                *priv;
};

struct vfs_inode {
    char                                *path;
    struct vfs_inode_ops                 ops;
    int16_t                             refs;
    uint16_t                            flags;
    ino_t                               ino;
    mode_t                              mode;
    uid_t                               uid;
    gid_t                               gid;
    struct vfs_inode                    *parent;
    struct vfs_inode                    *peer;
    struct vfs_inode                    *child;
    void                                *priv;
    char                                name[1];
};

#define vfs_inode_SIZE(n) (sizeof(struct vfs_inode) + (n))

struct vfs_file {
    int                           oflags;
    off_t                         pos;
    u32                           owner;
    char                          *fullpath;
    struct vfs_inode               *inode;
    void                          *priv;
};

struct vfs_dirent {
    struct dirent                   dent;
    struct list_head                node;
};

struct vfs_dir {
    DIR                             dir;
    off_t                           pos;
    u32                             thread;
    char                            *fullpath;
    void                            *priv;
    struct vfs_inode                *inode;
    struct list_head                list;
};

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
void vfs_seterrno(int errcode);
int vfs_geterrno(void);

int vfs_config(void);
void vfs_get_config(FS_CFG **fs_cfg, u32 *fs_num);

int vfs_setcwd(const char *path);
char *vfs_getcwd(void);

int fxffs_init(void);

int fs_dev_ftl_init(void);

/**---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __AIC_VFS_H__ */

