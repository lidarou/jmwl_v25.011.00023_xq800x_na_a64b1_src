/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * DESCRIPTION
 *
 * This file export the file system type interface.
 */

#ifndef __AIC_VFS_TYPE_H__
#define __AIC_VFS_TYPE_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 ** Public Type Definitions
 **---------------------------------------------------------------------------*/
struct vfs_dir;
struct vfs_file;
struct vfs_inode;

struct vfs_type_mount_ops {
    int (*mount)(struct vfs_inode *mp, unsigned long mountflags,
                 const void *data);
    int (*umount)(struct vfs_inode *mp);
    int (*umount2)(struct vfs_inode *mp, int flag);
    //int (*statfs)(const char *, struct statfs *buf);
    int     (*statfs)(struct vfs_inode *mp, struct statfs *buf);
};

struct vfs_type_file_ops {
  /* File operations */
  int     (*open)(struct vfs_file *file, const char *path, int oflags,
                  mode_t mode);
  int     (*close)(struct vfs_file *file);
  ssize_t (*read)(struct vfs_file *file, char *buffer, size_t buflen);
  ssize_t (*write)(struct vfs_file *file, const char *buffer, size_t buflen);
  off_t   (*seek)(struct vfs_file *file, off_t offset, int whence);
  int     (*truncate)(struct vfs_file *file, off_t length);
  int     (*ioctl)(struct vfs_file *file, int cmd, void *arg);
  int     (*sync)(struct vfs_file *file);
  int     (*fstat)(struct vfs_file *file, struct stat * buf);
  int     (*dup)(const struct vfs_file *oldfile, struct vfs_file *newfile);

  /* Directory operations */
  int     (*opendir)(struct vfs_dir *dir, const char *path);
  int     (*readdir)(struct vfs_dir *dir, struct dirent *dirent);
  long     (*telldir)(struct vfs_dir *dir);
  int     (*seekdir)(struct vfs_dir *dir, long offset);
  int     (*rewinddir)(struct vfs_dir *dir);
  int     (*closedir)(struct vfs_dir *dir);

  /* Operations on paths */
  int     (*unlink)(struct vfs_inode *mp, const char *path);
  int     (*mkdir)(struct vfs_inode *mp, const char *path);
  int     (*rmdir)(struct vfs_inode *mp, const char *path);
  int     (*rename)(struct vfs_inode *mp, const char *oldpath,
                    const char *newpath);
  int     (*stat)(struct vfs_inode *mp, const char *path,
                  struct stat *buf);
};

struct vfs_type_management_ops {
    int (*format)(struct vfs_inode *mp);
};

struct vfs_type_ops {
    const char                                  *type;
    const struct vfs_type_mount_ops             *mntops;
    const struct vfs_type_file_ops              *fileops;
    const struct vfs_type_management_ops        *mgtops;
    uint32_t                                    refs;
    void                                        *priv;
    struct list_head                            node;
};

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
int vfs_type_register(const char *fstype,
                      const struct vfs_type_mount_ops *mntops,
                      const struct vfs_type_file_ops *fileops,
                      const struct vfs_type_management_ops *mgtops);
struct vfs_type_ops *vfs_find_type(const char *fstype);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_VFS_TYPE_H__ */

