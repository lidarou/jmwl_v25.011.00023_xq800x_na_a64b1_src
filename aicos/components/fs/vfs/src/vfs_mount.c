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
#define LOG_MODULE    AMOD_FS

#include "vfs.h"
#ifndef WIN32
#include <sys/mount.h>
#endif

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
    struct vfs_inode *mp_inode = NULL;
    struct vfs_type_ops *fstypeops = NULL;
    struct vfs_inode_search_s desc;
    FS_CFG *fs_cfg;
    int ret;

    if ((source == NULL) || (target == NULL) || (filesystemtype == NULL)) {
        ret = AIC_EINVAL;
        goto errout;
    }

    ret = vfs_get_target_config(target, &fs_cfg);
    if (ret != AIC_SUCCESS) {
        goto errout;
    }

    if ((strcmp(source, fs_cfg->devpath) != 0) ||
        (strcmp(filesystemtype, fs_cfg->fstype) != 0)) {
        ret = AIC_EINVAL;
        goto errout;
    }

    if (vfs_blkdev_find(source, mountflags, &dev_inode) >= 0) {
        fstypeops = vfs_find_type(filesystemtype);
        if (fstypeops == NULL) {
            ret = AIC_ENODEV;
            goto errout_with_inode;
        }
    } else {
        ret = AIC_ENOTBLK;
        goto errout;
    }

    if ((fstypeops->mntops == NULL) || (fstypeops->fileops == NULL)) {
        ret = AIC_EINVAL;
        goto errout_with_inode;
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
            AIC_ASSERT(mp_inode != NULL);
            ret = AIC_ENOTDIR;
            vfs_inode_release(mp_inode);
            goto errout_with_lock;
        } else {
            vfs_get_rootinode(&mp_inode);
        }
    } else {
        ret = vfs_inode_create(target, 0777, &mp_inode);
        if (ret < 0) {
            goto errout_with_lock;
        }
    }

    dev_inode->refs++;

    mp_inode->path = (char *)target;
    mp_inode->ops.blkdevops  = dev_inode->ops.blkdevops;
    mp_inode->ops.fstypeops = fstypeops;

    ret = fstypeops->mntops->mount(mp_inode, mountflags, (void *)fs_cfg);
    if (ret < 0) {
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
        ret = -1;
        goto MNT_FAIL;
    }

    fs_create_default_dir(target);

    mntinfo = aos_zalloc(sizeof(struct fs_mount_info));
    AIC_ASSERT(mntinfo);
    mntinfo->target = target;
    mntinfo->devpath = source;
    mntinfo->fstype = filesystemtype;
    mntinfo->flags = mountflags;
    mntinfo->mount = 1;
    vfs_mount_callback(mntinfo);
    aos_free(mntinfo);

    ret = 0;

MNT_FAIL:
    aicx_info(0x011dbe, "sssd", "device(%s) target(%s) fstype(%s) mount %d.\n",
                source, target, filesystemtype, ret);

    return ret;
}

/**
 * fs_mount_target - mount by target.
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

    ret = vfs_get_target_config(target, &fs_cfg);
    if (ret != AIC_SUCCESS) {
        vfs_seterrno(ret);
        return -1;
    }

    ret = mount(fs_cfg->devpath, fs_cfg->target, fs_cfg->fstype,
                fs_cfg->flags ? fs_cfg->flags : O_RDWR,
                NULL);

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
    int ret, ret1;

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
    AIC_ASSERT(mountpt_inode != NULL);

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

    mountpt_inode->priv = NULL;
    mountpt_inode->ops.fstypeops  = NULL;

    if (mountpt_inode->child != NULL) {
        AIC_ASSERT(mountpt_inode->refs > 0);
        mountpt_inode->refs--;
        vfs_inode_unlock();
    } else {
        ret1 = vfs_inode_remove(target);
        vfs_inode_unlock();

        if (ret1 != AIC_SUCCESS && ret1 != AIC_EBUSY) {
            ret = ret1;
            goto errout_with_mountpt;
        }

        vfs_inode_release(mountpt_inode);
    }

    VFS_RELEASE_SEARCH(&desc);

    return ret;

errout_with_mountpt:
    vfs_inode_release(mountpt_inode);

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);

errout:
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
    FS_CFG *fs_cfg;
    int ret;

    ret = vfs_get_target_config(target, &fs_cfg);
    if (ret != AIC_SUCCESS) {
        vfs_seterrno(ret);
        ret = -1;
        goto UMNT_FAIL;
    }

    ret = vfs_umount(target);
    if (ret < 0) {
        vfs_seterrno(ret);
        ret = -1;
    }

    mntinfo = aos_zalloc(sizeof(struct fs_mount_info));
    AIC_ASSERT(mntinfo);
    mntinfo->target = target;
    mntinfo->devpath = fs_cfg->devpath;
    mntinfo->fstype = fs_cfg->fstype;
    mntinfo->flags = fs_cfg->flags;
    mntinfo->mount = 0;
    vfs_mount_callback(mntinfo);
    aos_free(mntinfo);

UMNT_FAIL:
    aicx_info(0x15ba1b, "sssd", "device(%s) target(%s) fstype(%s) umount %d.\n",
                fs_cfg->devpath, target, fs_cfg->fstype, ret);

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
    struct fs_loopfind_info *findinfo;
    struct vfs_inode_search_s desc;
    struct vfs_inode *inode;
    char *fullpath = NULL;
    FS_CFG *fs_cfg;
    int ret;

    if (path == NULL) {
        return AIC_EINVAL;
    }

    ret = vfs_getfullpath(path, &fullpath);
    if (ret != AIC_SUCCESS)
        return ret;

    ret = vfs_get_target_config(fullpath, &fs_cfg);
    if (ret != AIC_SUCCESS) {
        aos_free(fullpath);
        return ret;
    }

    if (((strlen("/") == strlen(fullpath)) &&
        !memcmp("/", fullpath, strlen(fullpath))) ||
        ((strlen("/dev") == strlen(fullpath)) &&
         !memcmp("/dev", fullpath, strlen(fullpath)))) {
        aos_free(fullpath);
        return AIC_EACCES;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, 0);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    AIC_ASSERT(inode != NULL);

    if (VFS_INODE_IS_MOUNTPT(inode)) {
        if (fs_cfg->format_eb && inode->ops.fstypeops &&
            inode->ops.fstypeops->mgtops &&
            inode->ops.fstypeops->mgtops->format) {
            ret = inode->ops.fstypeops->mgtops->format(inode);
        } else {
            findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
            AIC_ASSERT(findinfo);
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->deltype = FS_DEL_ALL;
            findinfo->dir = fullpath;
            ret = fs_loopfind(fullpath, findinfo);
            aos_free(findinfo);
        }
    } else {
        ret = AIC_ENOSYS;
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

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

/**
 * fs_getmntinfo - get fs mount information.
 *
 * Parameter:
 *  @path: input mount target path.
 *  @mntinfo: input get mount information ptr.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_getmntinfo(const char *path, struct fs_mount_info *mntinfo)
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
        aos_free(fullpath);
        return AIC_EACCES;
    }

    VFS_SETUP_SEARCH(&desc, fullpath, 0);

    ret = vfs_inode_find(&desc);
    if (ret < 0) {
        goto errout_with_search;
    }

    inode = desc.node;
    AIC_ASSERT(inode != NULL);

    mntinfo->target = path;
    mntinfo->devpath = inode->ops.blkdevops->path;
    mntinfo->fstype = inode->ops.fstypeops->type;
    mntinfo->flags = inode->flags;
    mntinfo->mount = 1;

    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullpath);
    return AIC_SUCCESS;

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    aos_free(fullpath);

    mntinfo->mount = 0;

    return ret;
}

/**
 * fs_register_mount_hook - register mount callback.
 *
 * Parameter:
 *  @mount_func: input mount callback function.
 *  @param: input mount callback function param.
 *
 * Return:
 */
void fs_register_mount_hook(FS_MOUNT_HOOK_FUNC mount_func, void* param)
{
    FS_MOUNT_CB_T *mount_cb;

    mount_cb = aos_zalloc(sizeof(FS_MOUNT_CB_T));

    mount_cb->func = mount_func;
    mount_cb->param = param;

    list_add(&mount_cb->node, &vfs_mount_list);
}

/**
 * fs_unregister_mount_hook - unregister mount callback.
 *
 * Parameter:
 *  @mount_func: input mount callback function.
 *
 * Return:
 */
void fs_unregister_mount_hook(FS_MOUNT_HOOK_FUNC mount_func)
{
    FS_MOUNT_CB_T *mount_cb = NULL;
    struct list_head *pos, *next;

    list_for_each_safe(pos, next, &vfs_mount_list) {
        mount_cb = list_entry(pos, FS_MOUNT_CB_T, node);
        if (mount_cb->func == mount_func) {
            list_del(&mount_cb->node);
            aos_free(mount_cb);
        }
    }
}

