/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system path format interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00d239

#include "vfs.h"

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
int vfs_pathcheck(const char *path, boolean isfile)
{
    size_t len;

    if (path == NULL) {
        return AIC_EINVAL;
    }

    len = strlen(path);
    if (len >= PATH_MAX) {
        return AIC_ENAMETOOLONG;
    }

    if (isfile && path[len - 1] == '/') {
        return AIC_EINVAL;
    }

    return AIC_SUCCESS;
}

size_t vfs_formatpath(const char *cwd, const char *path,
                      char **buf, size_t *bufsize)
{
    size_t offset, tmplen, totallen;
    char *tmpbuf, *p;

    if (!buf || !bufsize) {
        return AIC_EINVAL;
    }

    if (!path) {
        path = "";
    }

    tmpbuf = (char *)path;
    while((p = strstr(tmpbuf, "\\")) != NULL) {
        *p = '/';
    }

    if ((path[0] == '/') || (strstr(tmpbuf, ":"))) {
        cwd = "";
    } else {
        if (!cwd)
            cwd = vfs_get_default_target_path();
    }

    offset = strlen("///") + 1; // three '/' and one '\0'
    tmplen = strlen(cwd) + strlen(path) + offset;
    tmpbuf = (char *)malloc(tmplen);
    ASSERT_INFO(tmpbuf, "malloc size(0x%x) memory fail.\n", tmplen);
    memset(tmpbuf, 0x0, tmplen);

    if (-1 == snprintf(tmpbuf, tmplen, "/%s/%s/", cwd, path)) {
        free(tmpbuf);
        return AIC_EPERM;
    }

    /* replace :/ to / */
    offset = strlen(":/") - 1;
    while((p = strstr(tmpbuf, ":/")) != NULL) {
        memcpy(tmpbuf + (p - tmpbuf), p + 1, strlen(p + 1));
        tmpbuf[p - tmpbuf + strlen(p + 1)] = '\0';
    }

    /* replace /./ to / */
    offset = strlen("/./") - 1;
    while ((p = strstr(tmpbuf, "/./")) != NULL) {
        if (NULL == memmove(p, p + offset, tmplen - (p - tmpbuf) - offset)) {
            free(tmpbuf);
            return AIC_EPERM;
        }
    }

    /* replace // to / */
    while ((p = strstr(tmpbuf, "//")) != NULL) {
        if (NULL == memmove(p, p + 1, tmplen - (p - tmpbuf) - 1)) {
            free(tmpbuf);
            return AIC_EPERM;
        }
    }

    /* handle /../ (e.g., replace /aa/bb/../ to /aa/) */
    offset = strlen("/../") - 1;
    while ((p = strstr(tmpbuf, "/../")) != NULL) {
        char *start = p;
        while (start > tmpbuf && *(start - 1) != '/') {
            --start;
        }
        if (NULL == memmove(start, p + offset, tmplen - (p - tmpbuf) - offset)) {
            free(tmpbuf);
            return AIC_EPERM;
        }
    }

    totallen = strlen(tmpbuf);
    /* strip the last / */
    if (totallen > 1 && tmpbuf[totallen - 1] == '/') {
        tmpbuf[--totallen] = 0;
    }

    tmpbuf[totallen] = 0;
    *bufsize = totallen;
    *buf = tmpbuf;

    return totallen;
}

int vfs_inode_checkflags(struct vfs_inode *inode, int oflags)
{
    if (inode->ops.blkdevops == NULL) {
        return AIC_ENXIO;
    }

    if (((oflags & O_ACCMODE) == O_RDONLY) && inode->ops.blkdevops->read) {
        return AIC_SUCCESS;
    } else if (((oflags & O_ACCMODE) == O_WRONLY) && inode->ops.blkdevops->write) {
        return AIC_SUCCESS;
    } else if (((oflags & O_ACCMODE) == O_RDWR) && inode->ops.blkdevops->read &&
                inode->ops.blkdevops->write) {
        return AIC_SUCCESS;
    } else {
        return AIC_EACCES;
    }
}

int vfs_getfullpath(const char *path, char **fullpath)
{
    char *cwd = NULL;
    size_t pathlen;
    int ret;

    ret = vfs_pathcheck(path, 0);
    if (ret != AIC_SUCCESS)
        return ret;

    ret = vfs_formatpath(cwd, path, fullpath, &pathlen);
    if (ret <= 0) {
        return AIC_EPERM;
    }

    return AIC_SUCCESS;
}

char *vfs_getbasename(char *path)
{
    char *p;
    int len;

    if (!path || *path == '\0') {
        return ".";
    }

    len = strlen(path);
    while (path[len - 1] == '/') {
        if (len > 1) {
            path[len - 1] = '\0';
            len--;
        } else {
            return "/";
        }
    }

    p = strrchr(path, '/');
    if (p) {
        return p + 1;
    }

    return path;
}

u32 vfs_getumask()
{
    return (O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_CREAT |
            O_TRUNC | O_EXCL | O_BINARY);
}

char *vfs_getfilename(const char *path)
{
    int len = strlen(path), i;
    char *ptr = (char *)path;

    for (i = len - 1; i >= 0; i--) {
        if (ptr[i] == '/' || ptr[i] == ':')
            break;
    }

    ++i;
    if ((i != len) && (i != 0))
        return &ptr[i];
    else
        return NULL;
}

