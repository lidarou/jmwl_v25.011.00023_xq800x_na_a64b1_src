/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the fs device register Interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0bea0b
#define LOG_MODULE    AMOD_FS

#ifndef WIN32
#include <sys/mount.h>
#include <aic_boot_mode.h>
#endif

#include "vfs.h"
#include "fs_config.h"

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
/* if config ,directory set to readonly. */
char s_fs_sys_dir[][FS_DEFAULT_DIR_MAX_NAME_LEN] = {
#if defined(CONFIG_FS_ROMFS) || defined(WIN32)
    "usr"
#endif
};

boolean vfs_sys_path_check = true;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * vfs_config - mount fs by config.
 *
 * Parameter:
 *
 * Return:
 */
void vfs_config(void)
{
    u32 i, mountnum;
    FS_CFG *fs_cfg;

    mountnum = sizeof(s_fs_cfg) / sizeof(FS_CFG);

    for (i = 0; i < mountnum; i++) {
        fs_cfg = &s_fs_cfg[i];
        mount(fs_cfg->devpath, fs_cfg->target, fs_cfg->fstype, fs_cfg->flags, NULL);
    }
}

/**
 * vfs_unconfig - unmount fs by config.
 *
 * Parameter:
 *
 * Return:
 */
void vfs_unconfig(void)
{
    u32 i, mountnum;
    FS_CFG *fs_cfg;

    mountnum = sizeof(s_fs_cfg) / sizeof(FS_CFG);

    for (i = 0; i < mountnum; i++) {
        fs_cfg = &s_fs_cfg[i];
        umount(fs_cfg->target);
    }
}

/**
 * vfs_get_default_dir - get fs default directory.
 *
 * Parameter:
 *  @addr: input default directory address ptr.
 *  @count: input default directory count ptr.
 *  @max_name_len: input default directory max name len ptr.
 *
 * Return:
 */
void vfs_get_default_dir(u32 *addr, u32 *count, u32 *max_name_len)
{
    *addr = (u32)s_fs_default_dir_cfg;
    *count = (sizeof(s_fs_default_dir_cfg) / FS_DEFAULT_DIR_MAX_NAME_LEN);
    *max_name_len = FS_DEFAULT_DIR_MAX_NAME_LEN;
}

/**
 * vfs_get_default_path - get fs default target path.
 *
 * Parameter:
 *
 * Return:
 *  fs default target path.
 */
char *vfs_get_default_target_path()
{
    return FS_DEFAULT_TARGET_NAME;
}

/**
 * vfs_get_config - get fs config.
 *
 * Parameter:
 *  @fs_cfg: input get fs config ptr.
 *  @fs_num: input fs number ptr.
 *
 * Return:
 */
void vfs_get_config(FS_CFG **fs_cfg, u32 *fs_num)
{
    *fs_num = sizeof(s_fs_cfg) / sizeof(FS_CFG);
    *fs_cfg = s_fs_cfg;
}

/**
 * vfs_get_target_config - get fs target config.
 *
 * Parameter:
 *  @target: input fs target.
 *  @fs_cfg: input get fs config ptr.
 *
 * Return:
 *  AIC_SUCCESS is pass, other fail.
 */
int vfs_get_target_config(const char *target, FS_CFG **fs_cfg)
{
    u32 fs_num, i;
    FS_CFG *cfg;

    vfs_get_config(&cfg, &fs_num);

    for (i = 0; i < fs_num; i++, cfg++) {
        if ((strlen(cfg->target) == strlen(target)) &&
            !memcmp(cfg->target, target, strlen(target))) {
            break;
        } else if (target[0] != '/') {
            /* /C and C compare */
            if ((strlen(cfg->target) == (strlen(target) + 1)) &&
                !memcmp(cfg->target + 1, target, strlen(target))) {
                break;
            }
        }
    }

    if (i == fs_num) {
        return AIC_EINVAL;
    }

    *fs_cfg = cfg;

    return AIC_SUCCESS;
}

/**
 * fs_create_default_dir - fs create default directory.
 *
 * Parameter:
 *  @path: input mount target path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_create_default_dir(const char *path)
{
    u8 *default_dir_cfg = NULL, *dir = NULL;
    u32 i, count, addr, max_name_len;
    int ret = AIC_SUCCESS;
    FS_CFG *fs_cfg;

    ret = vfs_get_target_config(path, &fs_cfg);
    if (ret != AIC_SUCCESS) {
        return ret;
    }

    if (!fs_cfg->create_default_dir || (fs_cfg->flags == O_RDONLY)) {
        return AIC_SUCCESS;
    }

    vfs_get_default_dir(&addr, &count, &max_name_len);
    default_dir_cfg = (u8 *)addr;
    if (count > 0) {
        dir = aos_malloc(NAME_MAX);
        AIC_ASSERT(dir);
        for (i = 0; i < count; i++) {
            aos_memset(dir, 0x0, NAME_MAX);
            snprintf(dir, NAME_MAX, "%s/%s", path,
                     default_dir_cfg + max_name_len * i);
            vfs_sys_path_check = false;
            ret = mkdir(dir, O_RDWR | O_CREAT);
            vfs_sys_path_check = true;
            if (ret != AIC_SUCCESS)
                break;
        }
        aos_free(dir);
    }

    return ret;
}

boolean vfs_is_sys_path(const char *path)
{
    char *p, *tmppath;
    u8 count, i;

    if (!vfs_sys_path_check || aic_boot_mode_is_basic())
        return false;

    /* skip /C/ */
    tmppath = (char *)path + 3;

    count = (sizeof(s_fs_sys_dir) / FS_DEFAULT_DIR_MAX_NAME_LEN);
    for (i = 0; i < count; i++) {
        p = strchr(tmppath, '/');
        if (p) {
            if ((strlen(s_fs_sys_dir[i]) == (p - tmppath)) &&
                !memcmp(s_fs_sys_dir[i], tmppath, p - tmppath))
                return true;
        } else {
            if ((strlen(s_fs_sys_dir[i]) == strlen(tmppath)) &&
                !memcmp(s_fs_sys_dir[i], tmppath, strlen(tmppath))) {
                return true;
            }
        }
    }

    return false;
}

