/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system errno interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ffbee
#define LOG_MODULE    AMOD_FS
#include "vfs.h"

/**---------------------------------------------------------------------------*
 ** Private Types
 **---------------------------------------------------------------------------*/
struct vfs_cwd {
    char    *taskname;
    char    *cwd;
};

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
struct vfs_cwd *g_vfs_cwd_table[OPEN_MAX];

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
int vfs_gethash(const char *path)
{
    int hash = 0;

    while ('\0' != *path) {
        hash = ((hash << 4) + *path) % OPEN_MAX;
        ++path;
    }
    return hash;
}

int vfs_setcwd(const char *path)
{
    char *taskname, *cwdpath, *fullpath = NULL;
    int hash, len, ret, cmdlen;

    cwdpath = vfs_getcwd();
    if (!cwdpath) {
        cmdlen = 0;
    } else {
        cmdlen = strlen(cwdpath);
    }

    ret = vfs_formatpath(cwdpath, path, &fullpath, &len);
    if (ret <= 0) {
        aos_free(fullpath);
        return ret;
    }

    fullpath[strlen(fullpath)] = '\0';

    taskname = aos_get_current_thread_name();
    hash = vfs_gethash(taskname);

    if (!g_vfs_cwd_table[hash]) {
        g_vfs_cwd_table[hash] = (struct vfs_cwd *)aos_zalloc(sizeof(struct vfs_cwd));
        if (!g_vfs_cwd_table[hash]) {
            aos_free(fullpath);
            return AIC_ENOMEM;
        }

        g_vfs_cwd_table[hash]->taskname = taskname;
        g_vfs_cwd_table[hash]->cwd = fullpath;
    } else {
        g_vfs_cwd_table[hash]->taskname = taskname;
        aos_free(g_vfs_cwd_table[hash]->cwd);
        g_vfs_cwd_table[hash]->cwd = fullpath;
    }

    return ret;
}

char *vfs_getcwd(void)
{
    char *taskname;
    int hash;

    taskname = aos_get_current_thread_name();
    hash = vfs_gethash(taskname);

    if (!g_vfs_cwd_table[hash]) {
        return NULL;
    } else {
    #if 0
        len = strlen(g_vfs_cwd_table[hash].cwd);
        len = size > len ? len : size;
        aos_memcpy(path, g_vfs_cwd_table[hash].cwd, len);
    #else
        return g_vfs_cwd_table[hash]->cwd;
    #endif
    }
}

