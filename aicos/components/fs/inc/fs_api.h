/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system interface define (API)
 */

#ifndef __AIC_FS_API_H__
#define __AIC_FS_API_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_osa.h>
#include <aic_bitmap.h>
#include <aic_list.h>
#ifndef WIN32
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/statfs.h>
#include <sys/syslimits.h>
#else
#include "vfs_win32.h"
#endif

/**---------------------------------------------------------------------------*
** Pre-processor Definitions
**---------------------------------------------------------------------------*/
#define FS_MAX_PATH_LEN         256

#define FS_MAX_MOUNT_NAME_LEN   20

#define FS_ROOT_PATH            "/"

/**---------------------------------------------------------------------------*
 ** Public Type Definitions
 **---------------------------------------------------------------------------*/
typedef enum {
    FS_FIND_CURDIR =        (0x1),
    FS_FIND_SUBDIR =        (0x2),
    FS_FIND_ALLDIR =        (0xFFFFFFFF),
} FS_LOOPFIND_TYPE;

typedef enum {
    FS_DEL_FILE =           (0x1),
    FS_DEL_ALL =            (0xFFFFFFFF),
} FS_LOOPDEL_TYPE;

struct fs_loopfind_info {
    struct dirent *dent;
    struct stat *st;
    char *dir;
    char fullpath[NAME_MAX];
    off_t dirlevel;
    u32 findtype;
    u32 deltype;
    u32 callback;
    void *param;
    u32 found_cnt;
};

typedef int (*FS_LOOPFIND)(struct fs_loopfind_info *findinfo);

struct fs_mount_info {
    const char *target;
    const char *devpath;
    const char *fstype;
    u32 flags;
    u32 mount;
};

typedef void (*FS_MOUNT_HOOK_FUNC)(struct fs_mount_info *mntinfo, void *param);

typedef struct _fs_mount_cb {
    FS_MOUNT_HOOK_FUNC func;
    void *param;
    struct fs_mount_info mntinfo;
    struct list_head node;
} FS_MOUNT_CB_T;

typedef enum {
    FS_O_RDONLY =       O_RDONLY,
    FS_O_WRONLY =       O_WRONLY,
    FS_O_RDWR =         O_RDWR,
    FS_O_APPEND =       O_APPEND,
    FS_O_CREATE =       O_CREAT,
    FS_O_TRUNC =        O_TRUNC,
    FS_O_EXCL =         O_EXCL,
    FS_O_NONBLOCK =     O_NONBLOCK,
    FS_O_DIRECTORY=     O_DIRECTORY,
} FS_OPEN_FLAGS;

/**
 * fs seek attributes.
 *
 * @FS_SEEK_BEGIN: begin file position.
 * @FS_SEEK_END: end file position.
 * @FS_SEEK_FORWARD: forward from the current file position.
 * @FS_SEEK_BACK: back from the current file position.
 */
typedef enum {
    FS_SEEK_BEGIN =     0,
    FS_SEEK_CUR =       1,
    FS_SEEK_END =       2,
    FS_SEEK_BACK =      3,
} FS_SEEK;

/**
 * fs attributes.
 *
 * @FS_ATTRI_RDWR: read and write attribute.
 * @FS_ATTRI_READ_ONLY: read only attribute.
 * @FS_ATTRI_HIDDEN: hidden attribute.
 * @FS_ATTRI_SYSTEM: system attribute.
 * @FS_ATTRI_VOLUME: volume attribute.
 * @FS_ATTRI_DIRECTORY: sub attribute.
 * @FS_ATTRI_ARCHIVE: archive attribute.
 */
typedef enum {
    FS_ATTRI_RDWR =       0x0,
    FS_ATTRI_READ_ONLY =  BIT_0,
    FS_ATTRI_HIDDEN =     BIT_1,
    FS_ATTRI_SYSTEM =     BIT_2,
    FS_ATTRI_VOLUME =     BIT_3,
    FS_ATTRI_DIRECTORY =  BIT_4,
    FS_ATTRI_ARCHIVE =    BIT_5,
} FS_ATTRI;

typedef struct fs_file_info {
    struct stat buf;
    char name[PATH_MAX];
    ulong size;
    u32 num;
} FS_FILE_INFO;

struct fs_dir_info {
    struct dirent dent;
    struct list_head node;
};

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * fs_open - open file.
 *
 * Parameter:
 * @path: input file path.
 * @flags: input open flags.
 *
 * Return:
 *  The new file descriptor is returned on success; a negated errno value is
 *  returned on any failure.
 */
int fs_open(const char *path, int flags, ...);

/**
 * fs_close - close file.
 *
 * Parameter:
 * @fd: input file descriptor.
 *
 * Return:
 *  Zero (AIC_SUCCESS) is returned on success. On failure, a negated errno
 *  value is returned.
 */
int fs_close(int fd);

/**
 * fs_read - read file.
 *
 * Parameter:
 * @fd: input file descriptor.
 * @buf: input buffer to save the data.
 * @nbytes: input the maximum size of the user-provided buffer.
 *
 * Return:
 *  The positive non-zero number of bytes read on success, 0 on if an
 *  end-of-file condition, or a negated errno value on any failure.
 */
ssize_t fs_read(int fd, void *buf, size_t nbytes);

/**
 * fs_write - write file.
 *
 * Parameter:
 * @fd: input file descriptor.
 * @buf: input data to write.
 * @nbytes: input length of data to write.
 *
 * Return:
 *  On success, the number of bytes written are returned (zero indicates
 *  nothing was written).  On any failure, a negated errno value is returned
 *  (see comments withwrite() for a description of the appropriate errno
 *  values).
 */
ssize_t fs_write(int fd, const void *buf, size_t nbytes);

/**
 * fs_seek - seek file.
 *
 * Parameter:
 *  @fd: input file descriptor.
 *  @offset: input defines the offset to position to.
 *  @whence: input defines how to use offset.
 *
 * Return:
 *   The resulting offset on success.  A negated errno value is returned on
 *   any failure.
 */
off_t fs_seek(int fd, off_t offset, int whence);

/**
 * fs_flush - fs flush.
 *
 * Parameter:
 *  @fd: input file description.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_flush(int fd);

/**
 * fs_delete - fs delete path.
 *
 * Parameter:
 *  @pathname: input file or directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_delete(const char *pathname);

/**
 * fs_createdir - create fs directory.
 *
 * Parameter:
 *  @path: input directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_createdir(const char *path);

/**
 * fs_rmdir - remove fs directory.
 *
 * Parameter:
 *  @path: input directory path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_rmdir(const char *path);

/**
 * fs_rename - fs rename.
 *
 * Parameter:
 *  @oldpath: input old path.
 *  @newpath: input new path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_rename(const char *oldpath, const char *newpath);

/**
 * fs_getdiskfreespace - fs disk free space and total space.
 *
 * Parameter:
 *  @path: input mount target path.
 *  @free_bytes: input to store free bytes ptr.
 *  @total_bytes: input to store total bytes ptr.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */

int fs_getdiskfreespace(const char *path, u64 *free_bytes,
                        u64 *total_bytes);

/**
 * fs_pathisexist - fs path is exist or not.
 *
 * Parameter:
 *  @path: input path.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_pathisexist(const char *path);

/**
 * fs_loopfind - fs loop find file.
 *
 * Parameter:
 *  @path: input find path.
 *  @findinfo: input find information.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_loopfind(const char *path, struct fs_loopfind_info *findinfo);

/**
 * fs_move - move source file to destination directory.
 *
 * Parameter:
 * @src_path: input source file path.
 * @dst_path: input destination directory path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_move(const char *srcpath, const char *dstpath);

/**
 * fs_copy - copy source file to destination directory.
 *
 * Parameter:
 * @src_path: input source file path.
 * @dst_path: input destination directory path.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_copy(const char *srcpath, const char *dstpath);

/**
 * fs_lsdir - ls directory.
 *
 * Parameter:
 * @path: input directory path.
 * @list: input to store file list.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_lsdir(const char *path, struct list_head *list);

/**
 * lsdir - ls directory.
 *
 * Parameter:
 * @path: input directory path.
 * @list: input to store file list.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int lsdir(const char *path, struct list_head *list);

/**
 * fs_geterrno - get file errno.
 *
 * Parameter:
 *
 * Return:
 *   return error number.
 */
int fs_geterrno(void);

/**
 * fs_getpos - get file position.
 *
 * Parameter:
 * @fd: input file description.
 *
 * Return:
 *   The resulting offset on success.  A negated errno value is returned on
 *   any failure.
 */
off_t fs_getpos(int fd);

/**
 * fs_ioctl - fs file ioctl.
 *
 * Parameter:
 *  @fd: input file description.
 *  @req: input ioctl req.
 *  @arg: input ioctl arg.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_ioctl(int fd, int cmd, void *arg);

/**
 * fs_fcntl - fs file fcntl.
 *
 * Parameter:
 *  @fd: input file description.
 *  @cmd: input fcntl cmd.
 *  @arg: input fcntl arg.
 *
 * Return:
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_fcntl(int fd, int cmd, void *arg);

/**
 * fs_getinfo - get fs path information.
 *
 * Parameter:
 *  @path: input path.
 *  @fileinfo: file infotmation ptr.
 *
 * Return:.
 *   Upon successful completion, 0 is returned. Otherwise, a negated errno
 *   value is returned.
 */
int fs_getinfo(const char *path, FS_FILE_INFO *fileinfo);

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
int fs_create_default_dir(const char *path);

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
int fs_getmntinfo(const char *path, struct fs_mount_info *mntinfo);

/**
 * fs_register_mount_hook - register mount callback.
 *
 * Parameter:
 *  @mount_func: input mount callback function.
 *  @param: input mount callback function param.
 *
 * Return:
 */
void fs_register_mount_hook(FS_MOUNT_HOOK_FUNC mount_func, void* param);

/**
 * fs_unregister_mount_hook - unregister mount callback.
 *
 * Parameter:
 *  @mount_func: input mount callback function.
 *
 * Return:
 */
void fs_unregister_mount_hook(FS_MOUNT_HOOK_FUNC mount_func);

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
int fs_mount_target(const char *target);

/**
 * fs_longname_get - get long name by short name.
 *
 * Parameter:
 * @path: input short name path.
 * @longname: input long name ptr.
 * @longname_len: input long name len.
 *
 * Return:
 *  Upon successful completion, 0 is returned. Otherwise, a negated errno
 *  value is returned.
 */
int fs_longname_get(char *path, char *longname, u32 longname_len);

/**
 * fs_setfileattri - set file attribute.
 *
 * Parameter:
 * @file_path: input file path.
 * @attri: input file attribute.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_setfileattri(const char *path, u32 attri);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_FS_API_H__ */

