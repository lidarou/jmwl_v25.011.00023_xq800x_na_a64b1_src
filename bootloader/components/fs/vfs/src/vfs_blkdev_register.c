/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system type map interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x032777

#include "sys/mount.h"
#include "vfs.h"
#include "vfs_inode.h"

/**---------------------------------------------------------------------------*
 ** Private Functions
 **---------------------------------------------------------------------------*/
int vfs_blkdev_mount(char *devpath)
{
    int ret = AIC_ENOSYS;
    FS_CFG *fs_cfg;
    u32 fs_num, i;

    vfs_get_config(&fs_cfg, &fs_num);

    for (i = 0; i < fs_num; i++) {
        fs_cfg += i;
        if ((strlen(fs_cfg->devpath) == strlen(devpath)) &&
            !memcmp(fs_cfg->devpath, devpath, strlen(devpath))) {
            ret = mount(fs_cfg->devpath, fs_cfg->target, fs_cfg->fstype,
                        fs_cfg->flags ? fs_cfg->flags : O_RDWR,
                        (void *)fs_cfg);
            break;
        }
    }

    return ret;
}

int vfs_blkdev_unmount(char *devpath)
{
    int ret = AIC_ENOSYS;

    FS_CFG *fs_cfg;
    u32 fs_num, i;

    vfs_get_config(&fs_cfg, &fs_num);

    for (i = 0; i < fs_num; i++) {
        fs_cfg += i;
        if ((strlen(fs_cfg->devpath) == strlen(devpath)) &&
            !memcmp(fs_cfg->devpath, devpath, strlen(devpath))) {
            ret = umount(fs_cfg->target);
            break;
        }
    }

    return ret;
}

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
int vfs_blkdev_register(struct vfs_blkdev_ops *blkdevops,
                        mode_t mode, void *priv)
{
    struct vfs_inode *node = NULL;
    int ret;

    ret = vfs_inode_lock();
    if (ret < 0) {
        return ret;
    }

    ret = vfs_inode_create(blkdevops->path, mode, &node);
    if (ret >= 0) {
        VFS_INODE_SET_BLOCK(node);

        blkdevops->mount    = vfs_blkdev_mount;
        blkdevops->unmount  = vfs_blkdev_unmount;

        node->ops.blkdevops = blkdevops;
        node->priv          = priv;
        ret                 = AIC_SUCCESS;
    }

    vfs_inode_unlock();
    return ret;
}

int vfs_blkdev_find(const char *path, int mountflags,
                    struct vfs_inode **ppinode)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    int ret = AIC_SUCCESS;

    ASSERT(path != NULL || ppinode != NULL);

    log_print("pathname=\"%s\"\n", path);

    VFS_SETUP_SEARCH(&desc, path, false);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        log_print("ERROR: Failed to find %s\n", path);
        ret = AIC_ENOENT;
        goto errout_with_search;
    }

    inode = desc.node;
    ASSERT(inode != NULL);

    if (!VFS_INODE_IS_BLOCK(inode)) {
        log_print("ERROR: %s is not a block driver\n", path);
        ret = AIC_ENOTBLK;
        goto errout_with_inode;
    }

    /* Make sure that the inode supports the requested access */

    if (!inode->ops.blkdevops || !inode->ops.blkdevops->read ||
        (!inode->ops.blkdevops->write && (mountflags & O_RDONLY) == 0)) {
        log_print("ERROR: %s does not support requested access\n", path);
        ret = AIC_EACCES;
        goto errout_with_inode;
    }

    *ppinode = inode;
    VFS_RELEASE_SEARCH(&desc);
    return AIC_SUCCESS;

errout_with_inode:
    vfs_inode_release(inode);
errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    return ret;
}

