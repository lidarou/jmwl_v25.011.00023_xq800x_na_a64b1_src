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
#define AIC_THIS_FILE    0x11db49

#include "vfs.h"
#include "vfs_inode.h"

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
LIST_HEAD(g_vfs_typeops_list);

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
const char *vfs_gettype(struct statfs *statbuf)
{
    const char *fstype;

    switch (statbuf->f_type) {
#ifdef CONFIG_FS_FAT
        case FXFFS_TYPE:
            fstype = "fxffs";
            break;
#endif

#ifdef CONFIG_FS_ROMFS
        case ROMFS_TYPE:
            fstype = "romfs";
            break;
#endif

        default:
            fstype = "Unrecognized";
            break;
    }

    return fstype;
}

int vfs_type_register(const char *fstype,
                      const struct vfs_type_mount_ops *mntops,
                      const struct vfs_type_file_ops *fileops,
                      const struct vfs_type_management_ops *mgtops)
{
    struct vfs_type_ops *newfs;

    if ((fstype == NULL) || (fileops == NULL)) {
        vfs_seterrno(AIC_EINVAL);
        return AIC_EINVAL;
    }

    newfs = (struct vfs_type_ops *)malloc(sizeof(struct vfs_type_ops));
    ASSERT_INFO(newfs, "malloc size(0x%x) memory fail.\n",
                sizeof(struct vfs_type_ops));
    memset(newfs, 0x0, sizeof(struct vfs_type_ops));

    newfs->type = fstype;
    newfs->mntops = mntops;
    newfs->fileops = fileops;
    newfs->mgtops = mgtops;
    newfs->refs = 0;

    list_add(&newfs->node, &g_vfs_typeops_list);

    return AIC_SUCCESS;
}

struct vfs_type_ops *vfs_find_type(const char *fstype)
{
    struct vfs_type_ops *fstypemap;
    struct list_head *pos, *next;

    list_for_each_safe(pos, next, &g_vfs_typeops_list) {
        fstypemap = list_entry(pos, struct vfs_type_ops, node);

        if ((strlen(fstype) == strlen(fstypemap->type)) &&
            !memcmp(fstype, fstypemap->type, strlen(fstypemap->type))) {
            return fstypemap;
        }
    }

    return NULL;
}

