/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system mount interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1291ff

#include "vfs.h"
#include "sys/mount.h"

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
LIST_HEAD(vfs_mount_list);

/**---------------------------------------------------------------------------*
 ** Private Functions Prototypes
 **---------------------------------------------------------------------------*/
static void vfs_mount_callback(struct fs_mount_info *mntinfo);

/**---------------------------------------------------------------------------*
 ** Private Functions
 **---------------------------------------------------------------------------*/
static void vfs_mount_callback(struct fs_mount_info *mntinfo)
{
    FS_MOUNT_CB_T *mount_cb = NULL;
    struct list_head *pos, *next;

    list_for_each_safe(pos, next, &vfs_mount_list) {
        mount_cb = list_entry(pos, FS_MOUNT_CB_T, node);
        mount_cb->func(mntinfo, mount_cb->param);
    }
}

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * vfs_mount - mount fs.
 *
 * Parameter:
 *  @source: input mount source path.
 *  @target: input mount target path.
 *  @filesystemtype: input mount file system type.
 *  @mountflags: input mount flags.
 *  @data: input mount data.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int vfs_mount(const char *source, const char *target,
             const char *filesystemtype, unsigned long mountflags,
             const void *data)
{
    struct vfs_inode *dev_inode = NULL;
    struct vfs_inode *mp_inode;
    struct vfs_type_ops *fstypeops = NULL;
    struct vfs_inode_search_s desc;
    int ret;

    ASSERT(target && filesystemtype);

    if (source != NULL &&
        vfs_blkdev_find(source, mountflags, &dev_inode) >= 0) {
        fstypeops = vfs_find_type(filesystemtype);
        if (fstypeops == NULL) {
            log_print("ERROR: Failed to find block based file system %s\n",
                       filesystemtype);

            ret = AIC_ENODEV;
            goto errout_with_inode;
        }
    } else {
        log_print("ERROR: Failed to find fs driver %s\n", source);
        ret = AIC_ENOTBLK;
        goto errout;
    }

    if ((fstypeops->mntops == NULL) || (fstypeops->fileops == NULL)) {
        log_print("ERROR: Filesystem does not support mountops or fileops\n");
        ret = AIC_EINVAL;
        goto errout;
    }

    ret = vfs_inode_lock();
    if (ret < 0) {
        goto errout_with_inode;
    }

    VFS_SETUP_SEARCH(&desc, target, false);
    ret = vfs_inode_find(&desc);
    if (ret >= 0) {
        if ((strlen("/") != strlen(target)) ||
            memcmp("/", target, strlen(target))) {
            mp_inode = desc.node;
            ASSERT(mp_inode != NULL);

            log_print("ERROR: target %s exists and is a special node\n", target);
            ret = AIC_ENOTDIR;
            vfs_inode_release(mp_inode);
            goto errout_with_lock;
        } else {
            vfs_get_rootinode(&mp_inode);
        }
    } else {
        ret = vfs_inode_create(target, 0777, &mp_inode);
        if (ret < 0) {
            log_print("ERROR: Failed to reserve inode for target %s\n", target);
            goto errout_with_lock;
        }
    }

    dev_inode->refs++;

    mp_inode->path = (char *)target;
    mp_inode->ops.blkdevops  = dev_inode->ops.blkdevops;
    mp_inode->ops.fstypeops = fstypeops;

    ret = fstypeops->mntops->mount(mp_inode, mountflags, data);
    if (ret < 0) {
        log_print("ERROR: mount failed: %d\n", ret);
        dev_inode->refs--;
        goto errout_with_mountpt;
    }

    VFS_INODE_SET_MOUNTPT(mp_inode);

    vfs_inode_unlock();

    vfs_inode_release(dev_inode);

    VFS_RELEASE_SEARCH(&desc);

    return AIC_SUCCESS;

errout_with_mountpt:
    vfs_inode_release(mp_inode);
    vfs_inode_remove(target);

errout_with_lock:
    vfs_inode_unlock();
    VFS_RELEASE_SEARCH(&desc);

errout_with_inode:
    vfs_inode_release(dev_inode);

errout:
    vfs_seterrno(ret);
    return ret;
}

/**
 * mount - mount fs.
 *
 * Parameter:
 *  @source: input mount source path.
 *  @target: input mount target path.
 *  @filesystemtype: input mount file system type.
 *  @mountflags: input mount flags.
 *  @data: input mount data.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data)
{
    struct fs_mount_info *mntinfo;
    int ret;

    ret = vfs_mount(source, target, filesystemtype, mountflags, data);
    if (ret < 0) {
        vfs_seterrno(ret);
        log_print("dev(%s) type(%s) target(%s) flags(0x%x) mount fail, ret=%d.\n",
                  source, filesystemtype, target, mountflags, ret);
        return -1;
    }

    if ((mountflags != O_RDONLY) ||
        (strlen("romfs") != strlen(filesystemtype)) ||
         memcmp("romfs", filesystemtype, strlen("romfs"))) {
        ret = fs_create_default_dir(target);
        if (ret < 0) {
            log_print("target(%s) create dir fail, ret=%d.\n", target, ret);
            ASSERT(0);
        }
    }

    mntinfo = malloc(sizeof(struct fs_mount_info));
    ASSERT_INFO(mntinfo, "malloc size(0x%x) memory fail.\n",
                sizeof(struct fs_mount_info));
    memset(mntinfo, 0x0, sizeof(struct fs_mount_info));
    mntinfo->target = target;
    mntinfo->devpath = source;
    mntinfo->fstype = filesystemtype;
    mntinfo->flags = mountflags;
    mntinfo->mount = 1;
    vfs_mount_callback(mntinfo);
    free(mntinfo);

    log_print("dev(%s) type(%s) target(%s) flags(0x%x) mount success.\n",
              source, filesystemtype, target, mountflags);

    return 0;
}

/**
 * fs_mount_target - mount target by fs config.
 *
 * Parameter:
 *  @target: input mount target path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_mount_target(const char *target)
{
    int ret = AIC_ENOSYS;
    FS_CFG *fs_cfg;
    u32 fs_num, i;

    vfs_get_config(&fs_cfg, &fs_num);

    for (i = 0; i < fs_num; i++) {
        if ((strlen(fs_cfg->target) == strlen(target)) &&
            !memcmp(fs_cfg->target, target, strlen(target))) {
            ret = mount(fs_cfg->devpath, fs_cfg->target, fs_cfg->fstype,
                        fs_cfg->flags ? fs_cfg->flags : O_RDWR,
                        (void *)fs_cfg);
            break;
        } else if (target[0] != '/') {
            /* /C and C compare */
            if ((strlen(fs_cfg->target) == (strlen(target) + 1)) &&
                !memcmp(fs_cfg->target + 1, target, strlen(target))) {
                ret = mount(fs_cfg->devpath, fs_cfg->target, fs_cfg->fstype,
                            fs_cfg->flags ? fs_cfg->flags : O_RDWR,
                            (void *)fs_cfg);
                break;
            }
        }
        fs_cfg += i;
    }

    if (i == fs_num) {
        vfs_seterrno(AIC_EINVAL);
        ret = AIC_EINVAL;
    }

    return ret;
}

/**
 * vfs_umount - umount fs.
 *
 * Parameter:
 *  @target: input mount target path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int vfs_umount(const char *target)
{
    struct vfs_inode *mountpt_inode;
    struct vfs_inode_search_s desc;
    int ret;

    if (!target) {
        ret = AIC_EFAULT;
        goto errout;
    }

    VFS_SETUP_SEARCH(&desc, target, false);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    mountpt_inode = desc.node;
    ASSERT(mountpt_inode != NULL);

    if (!VFS_INODE_IS_MOUNTPT(mountpt_inode)) {
        ret = AIC_EINVAL;
        goto errout_with_mountpt;
    }

    if (!mountpt_inode->ops.fstypeops->mntops->umount) {
        ret = AIC_EINVAL;
        goto errout_with_mountpt;
    }

    ret = vfs_inode_lock();
    if (ret < 0) {
        goto errout_with_mountpt;
    }

    ret = mountpt_inode->ops.fstypeops->mntops->umount(mountpt_inode);
    if (ret < 0) {
        goto errout_with_lock;
    } else if (ret > 0) {
        ret = AIC_EBUSY;
        goto errout_with_lock;
    }

    mountpt_inode->priv = NULL;
    mountpt_inode->ops.fstypeops  = NULL;

    if (mountpt_inode->child != NULL) {
        ASSERT(mountpt_inode->refs > 0);
        mountpt_inode->refs--;
        vfs_inode_unlock();
    } else {
        ret = vfs_inode_remove(target);
        vfs_inode_unlock();

        if (ret != AIC_SUCCESS && ret != AIC_EBUSY) {
            goto errout_with_mountpt;
        }

        vfs_inode_release(mountpt_inode);
    }

    VFS_RELEASE_SEARCH(&desc);

    log_print("target(%s) umount success.\n", target);

    return AIC_SUCCESS;

errout_with_lock:
    vfs_inode_unlock();

errout_with_mountpt:
    vfs_inode_release(mountpt_inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);

errout:
    log_print("target(%s) umount fail, ret=%d.\n", target, ret);
    return ret;
}

/**
 * umount - umount fs.
 *
 * Parameter:
 *  @target: input mount target path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int umount(const char *target)
{
    struct fs_mount_info *mntinfo;
    int ret;

    ret = vfs_umount(target);
    if (ret < 0) {
        vfs_seterrno(ret);
        ret = -1;
    }

    mntinfo = malloc(sizeof(struct fs_mount_info));
    ASSERT_INFO(mntinfo, "malloc size(0x%x) memory fail.\n",
                sizeof(struct fs_mount_info));
    memset(mntinfo, 0x0, sizeof(struct fs_mount_info));
    mntinfo->target = target;
    mntinfo->mount = 0;
    vfs_mount_callback(mntinfo);
    free(mntinfo);

    return ret;
}

/**
 * vfs_mkfs - format fs.
 *
 * Parameter:
 *  @fsType: input mount fs type.
 *  @path: input mount target path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int vfs_mkfs(const char *fsType, const char *path)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    char *fullpath = NULL;
    int ret;

    if (path == NULL) {
        return AIC_EINVAL;
    }

    ret = vfs_getfullpath(path, &fullpath);
    if (ret != AIC_SUCCESS)
        return ret;

    if (((strlen("/") == strlen(fullpath)) &&
        !memcmp("/", fullpath, strlen(fullpath))) ||
        ((strlen("/dev") == strlen(fullpath)) &&
         !memcmp("/dev", fullpath, strlen(fullpath)))) {
        free(fullpath);
        return AIC_EACCES;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, 0);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    ASSERT(inode != NULL);

    if (VFS_INODE_IS_MOUNTPT(inode)) {
        if (inode->ops.fstypeops &&
            inode->ops.fstypeops->mgtops &&
            inode->ops.fstypeops->mgtops->format) {
            ret = inode->ops.fstypeops->mgtops->format(inode);
        }
    } else {
        ret = AIC_ENOSYS;
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

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
 * mkfs - format fs.
 *
 * Parameter:
 *  @fsType: input mount fs type.
 *  @path: input mount target path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int mkfs(const char *fsType, const char *path)
{
    int ret;

    ret = vfs_mkfs(fsType, path);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    ret = fs_create_default_dir(path);
    if (ret < 0) {
        vfs_seterrno(ret);
        return -1;
    }

    return 0;
}

