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
#define AIC_THIS_FILE    0x158656

#ifndef WIN32
#include <sys/mount.h>
#endif

#include "vfs.h"
#include <fs/fs_config.h>

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
int vfs_config(void)
{
    int ret = AIC_EINVAL;
    u32 i, mountnum;
    FS_CFG *fs_cfg;

    mountnum = sizeof(s_fs_cfg) / sizeof(FS_CFG);

    for (i = 0; i < mountnum; i++) {
        fs_cfg = &s_fs_cfg[i];
        ret = mount(fs_cfg->devpath, fs_cfg->target, fs_cfg->fstype,
                    fs_cfg->flags, (void *)fs_cfg);
        if (ret != AIC_SUCCESS) {
            log_print("Error:[FSCFG] device(%s) target(%s) fstype(%s) mount fail.\n",
                       fs_cfg->devpath, fs_cfg->target, fs_cfg->fstype);
        }
    }

    return ret;
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
 * vfs_get_config - get fs default directory.
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
    char *default_dir_cfg = NULL, *dir = NULL;
    u32 i, count, addr, max_name_len;
    int ret = AIC_SUCCESS;

    vfs_get_default_dir(&addr, &count, &max_name_len);
    default_dir_cfg = (char *)addr;
    if (count > 0) {
        dir = malloc(NAME_MAX);
        ASSERT_INFO(dir, "malloc size(0x%x) memory fail.\n", NAME_MAX);
        for (i = 0; i < count; i++) {
            memset(dir, 0x0, NAME_MAX);
            snprintf(dir, NAME_MAX, "%s/%s", path,
                     default_dir_cfg + max_name_len * i);
            ret = mkdir(dir, O_RDWR | O_CREAT);
            if (ret != AIC_SUCCESS)
                break;
        }
        free(dir);
    }

    return ret;
}

