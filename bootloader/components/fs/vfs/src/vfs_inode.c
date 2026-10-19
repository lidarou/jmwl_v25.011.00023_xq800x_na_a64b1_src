/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system inode interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x156b7c

#include <aic_type.h>
#include "vfs.h"

/**---------------------------------------------------------------------------*
 ** Pre-processor Definitions
 **---------------------------------------------------------------------------*/
#define VFS_INODE_MUTEX_NAME     "VFS_INODE"

/**---------------------------------------------------------------------------*
 ** Private Types
 **---------------------------------------------------------------------------*/
struct vfs_inode_path_s
{
    vfs_foreach_inode_t handler;
    void                *arg;
    char                path[PATH_MAX];
};

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
static ino_t s_vfs_ino;
struct vfs_inode *g_vfs_root_inode;

/**---------------------------------------------------------------------------*
 ** Private Functions Prototypes
 **---------------------------------------------------------------------------*/
static void vfs_inode_root_create(void);
static int vfs_inode_namelen(const char *name);
static void vfs_inode_namecopy(char *dest, const char *src);
static struct vfs_inode *vfs_inode_alloc(const char *name, mode_t mode);
static int vfs_inode_compare(const char *fname, struct vfs_inode *node);
static void vfs_inode_insert(struct vfs_inode *node,
                             struct vfs_inode *peer,
                             struct vfs_inode *parent);
static char *vfs_inode_getcwd(void);
static int vfs_inode_searchall(struct vfs_inode_search_s *desc);
static const char *vfs_inode_nextname(const char *name);
static int vfs_foreach_inodelevel(struct vfs_inode *node,
                                  struct vfs_inode_path_s *info);

/**---------------------------------------------------------------------------*
 ** Private Functions
 **---------------------------------------------------------------------------*/
static void vfs_inode_root_create(void)
{
    g_vfs_root_inode = vfs_inode_alloc("", 0777);
}

static int vfs_inode_namelen(const char *name)
{
    const char *tmp = name;

    while (*tmp && *tmp != '/') {
        tmp++;
    }

    return tmp - name;
}

static void vfs_inode_namecopy(char *dest, const char *src)
{
    while (*src && *src != '/') {
        *dest++ = *src++;
    }

    *dest = '\0';
}

static struct vfs_inode *vfs_inode_alloc(const char *name, mode_t mode)
{
    struct vfs_inode *node;
    int namelen, len;

    namelen = vfs_inode_namelen(name);
    len = vfs_inode_SIZE(namelen);
    node = (struct vfs_inode *)malloc(len);
    ASSERT_INFO(node, "malloc size(0x%x) memory fail.\n", len);
    memset(node, 0x0, len);
    if (node) {
        node->ino   = s_vfs_ino++;
        node->mode  = mode;
        vfs_inode_namecopy(node->name, name);
    }

    return node;
}

static int vfs_inode_compare(const char *fname, struct vfs_inode *node)
{
    char *nname = node->name;

    if (!nname) {
        return 1;
    }

    if (!fname) {
        return -1;
    }

    for (; ; ) {
        if (!*nname) {
            if (!*fname || *fname == '/') {
                return 0;
            } else {
                return 1;
            }
        } else if (!*fname || *fname == '/') {
            return -1;
        } else if (*fname > *nname) {
            return 1;
        } else if (*fname < *nname) {
            return -1;
        } else {
            fname++;
            nname++;
        }
    }
}

static void vfs_inode_insert(struct vfs_inode *node,
                            struct vfs_inode *peer,
                            struct vfs_inode *parent)
{
    if (peer) {
        node->peer   = peer->peer;
        node->parent = parent;
        peer->peer   = node;
    } else {
        ASSERT(parent != NULL);
        node->peer    = parent->child;
        node->parent  = parent;
        parent->child = node;
    }
}

static char *vfs_inode_getcwd(void)
{
    char *pwd = "/";

    return pwd;
}

static int vfs_inode_searchall(struct vfs_inode_search_s *desc)
{
    struct vfs_inode *node    = g_vfs_root_inode;
    struct vfs_inode *left    = NULL;
    struct vfs_inode *above   = NULL;
    const char   *relpath = NULL;
    int ret = AIC_ENOENT;
    const char   *name;

    ASSERT(desc != NULL && desc->path != NULL);
    name  = desc->path;

    if (*name != '/') {
        return AIC_EINVAL;
    }

    while (node != NULL) {
        int result = vfs_inode_compare(name, node);
        if (result < 0) {
            node = NULL;
            break;
        } else if (result > 0) {
            left = node;
            node = node->peer;
        } else {
            name = vfs_inode_nextname(name);
            if (*name == '\0' || (VFS_INODE_IS_MOUNTPT(node) &&
                memcmp("/", node->name, strlen(node->name)))) {
                relpath = name;
                ret = AIC_SUCCESS;
                break;
            } else {
                above = node;
                left  = NULL;
                node  = node->child;
            }
        }
    }

    if ((ret != AIC_SUCCESS) && VFS_INODE_IS_MOUNTPT(g_vfs_root_inode)) {
        node = g_vfs_root_inode;
        relpath = name;
        ret = AIC_SUCCESS;
    }

    desc->path    = name;
    desc->node    = node;
    desc->peer    = left;
    desc->parent  = above;
    desc->relpath = relpath;
    return ret;
}

static const char *vfs_inode_nextname(const char *name)
{
    while (*name != '\0' && *name != '/') {
        name++;
    }

    while (*name == '/') {
        name++;
    }

    return name;
}

const char *vfs_inode_basename(const char *name)
{
    const char *basename = NULL;

    for (; ; ) {
        name = vfs_inode_nextname(name);
        if (name == NULL || *name == '\0') {
            break;
        }

        basename = name;
    }

    return basename;
}

static int vfs_foreach_inodelevel(struct vfs_inode *node,
                                 struct vfs_inode_path_s *info)
{
    int ret = AIC_SUCCESS;

    for (; node; node = node->peer) {
        ret = info->handler(node, info->path, info->arg);
        if (ret != 0) {
            break;
        }

        if (node->child) {
            int pathlen = strlen(info->path);
            int namlen  = strlen(node->name) + 1;

            if (pathlen + namlen > PATH_MAX) {
                    ret = AIC_ENAMETOOLONG;
                    break;
            }

            sprintf(&info->path[pathlen], "/%s", node->name);
            ret = vfs_foreach_inodelevel(node->child, info);

            info->path[pathlen] = '\0';

            if (ret != 0) {
                break;
            }
        }
    }

    return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int vfs_inode_search(struct vfs_inode_search_s *desc)
{
    char *fullpath, *cwd;
    int ret, len;

    ASSERT(desc != NULL && desc->path != NULL);

    if (*desc->path != '/') {
        cwd = vfs_inode_getcwd();
        len = strlen(cwd) + strlen(desc->path) + 4;
        fullpath = malloc(len);
        ASSERT_INFO(fullpath, "malloc size(0x%x) memory fail.\n", len);
        memset(fullpath, 0x0, len);

        snprintf(fullpath, len, "%s/%s", cwd, desc->path);
        desc->path = fullpath;
    }

    ret = vfs_inode_searchall(desc);

    return ret;
}

int vfs_inode_create(const char *path, mode_t mode, struct vfs_inode **inode)
{
    struct vfs_inode_search_s desc;
    const char *name, *nextname;
    struct vfs_inode *parent;
    struct vfs_inode *left;
    struct vfs_inode *node;
    int ret;

    ASSERT(path != NULL && inode != NULL);
    *inode = NULL;

    if (path[0] == '\0') {
        return AIC_EINVAL;
    }

    VFS_SETUP_SEARCH(&desc, path, false);

    ret = vfs_inode_search(&desc);
    if (ret >= 0) {
        ret = AIC_EEXIST;
        goto errout_with_search;
    }

    name   = desc.path;
    left   = desc.peer;
    parent = desc.parent;

    for (; ; ) {
        nextname = vfs_inode_nextname(name);
        if (*nextname != '\0') {
            node = vfs_inode_alloc(name, 0777);
            if (node != NULL) {
                vfs_inode_insert(node, left, parent);
                name   = nextname;
                left   = NULL;
                parent = node;
                continue;
            }
        } else {
            node = vfs_inode_alloc(name, mode);
            if (node != NULL) {
                vfs_inode_insert(node, left, parent);
                *inode = node;
                ret = AIC_SUCCESS;
                break;
            }
        }

        ret = AIC_ENOMEM;
        break;
    }

errout_with_search:
    VFS_RELEASE_SEARCH(&desc);
    return ret;
}

int vfs_inode_find(struct vfs_inode_search_s *desc)
{
    struct vfs_inode *node;
    int ret;

    ret = vfs_inode_lock();
    if (ret < 0) {
        return ret;
    }

    ret = vfs_inode_search(desc);
    if (ret >= 0) {
        node = desc->node;
        ASSERT(node != NULL);
        node->refs++;
    }

    vfs_inode_unlock();
    return ret;
}

int vfs_inode_addref(struct vfs_inode *inode)
{
    int ret = AIC_SUCCESS;

    if (inode) {
        ret = vfs_inode_lock();
        if (ret >= 0) {
            inode->refs++;
            vfs_inode_unlock();
        }
    }

    return ret;
}

int fs_foreach_inode(vfs_foreach_inode_t handler, void *arg)
{
    struct vfs_inode_path_s *info;
    int ret, len;

    len = sizeof(struct vfs_inode_path_s);
    info = (struct vfs_inode_path_s *)malloc(len);
    ASSERT_INFO(info, "malloc size(0x%x) memory fail.\n", len);
    memset(info, 0x0, len);

    info->handler = handler;
    info->arg     = arg;
    info->path[0] = '\0';

    ret = vfs_inode_lock();
    if (ret >= 0) {
        ret = vfs_foreach_inodelevel(g_vfs_root_inode->child, info);
        vfs_inode_unlock();
    }

    free(info);
    return ret;
}

int vfs_inode_getpath(struct vfs_inode *node, char *path)
{
    if (path == NULL) {
        return AIC_EINVAL;
    } else if (node == NULL) {
        path[0] = '\0';
        return AIC_SUCCESS;
    } else {
        int ret = vfs_inode_getpath(node->parent, path);
        if (ret < 0) {
            return ret;
        }
    }

    strcat(path, node->name);
    if (node->child || VFS_INODE_IS_MOUNTPT(node)) {
        strcat(path, "/");
    }

    return AIC_SUCCESS;
}

void vfs_inode_free(struct vfs_inode *node)
{
    if (node != NULL) {
        vfs_inode_free(node->peer);
        vfs_inode_free(node->child);
        free(node);
    }
}

int vfs_inode_remove(const char *path)
{
    struct vfs_inode *node;

    if ((strlen("/") == strlen(path)) && !memcmp("/", path, strlen(path)))
        return AIC_SUCCESS;

    node = vfs_inode_unlink(path);
    if (node) {
        if (node->refs) {
            node->flags |= VFS_INODE_FLAG_DELETED;
            return AIC_EBUSY;
        } else {
            ASSERT(node->peer == NULL);
            vfs_inode_free(node);
            return AIC_SUCCESS;
        }
    }

    return AIC_ENOENT;
}

struct vfs_inode *vfs_inode_unlink(const char *path)
{
    struct vfs_inode_search_s desc;
    struct vfs_inode *node = NULL;
    int ret;

    if (path == NULL) {
        return NULL;
    }

    VFS_SETUP_SEARCH(&desc, path, true);

    ret = vfs_inode_search(&desc);
    if (ret >= 0) {
        node = desc.node;
        ASSERT(node != NULL);

        if (desc.peer != NULL) {
            desc.peer->peer = node->peer;
        } else {
            ASSERT(desc.parent != NULL);
            desc.parent->child = node->peer;
        }

        node->peer   = NULL;
        node->parent = NULL;
    }

    VFS_RELEASE_SEARCH(&desc);
    return node;
}

void vfs_inode_release(struct vfs_inode *node)
{
    int ret;

    if (node) {
        do {
            ret = vfs_inode_lock();
            ASSERT(ret == AIC_SUCCESS || ret == AIC_ECANCELED);
        } while (ret < 0);

        if (node->refs) {
            node->refs--;
        }

        if (node->refs <= 0 && (node->flags & VFS_INODE_FLAG_DELETED) != 0) {
            vfs_inode_unlock();

            ASSERT(node->peer == NULL);
            vfs_inode_free(node);
        } else {
            vfs_inode_unlock();
        }
    }
}

void vfs_get_rootinode(struct vfs_inode **inode)
{
    *inode = g_vfs_root_inode;
}

int vfs_inode_lock(void)
{
    return AIC_SUCCESS;
}

void vfs_inode_unlock(void)
{
    return;
}

void vfs_inode_init(void)
{
    s_vfs_ino = 0;

    vfs_inode_root_create();
}

