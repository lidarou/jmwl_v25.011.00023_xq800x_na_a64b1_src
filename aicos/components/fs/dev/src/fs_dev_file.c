/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic fs file device operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1ae998
#define LOG_MODULE    AMOD_FS

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_assert.h>
#include <errno.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include "vfs_dev.h"
#include "vfs.h"

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
#ifdef WIN32
int fs_file_win32_init(struct vfs_blkdev_partinfo *partinfo)
{
    u32 ret_len, total_len;
    errno_t ferrno;
    size_t rsize;
    int fd, ret;
    FILE *file;
    char *buf;

    ferrno = fopen_s(&file, "../fs/romfs.bin", "rb");
    AICX_ASSERT_IFX(!ferrno, 0x1b8314, "d", "romfs.bin open fail, ferrno(%d).\n", ferrno);

    total_len = 0x32000;
    buf = aos_zalloc(total_len);
    AIC_ASSERT(buf);

    ret_len = fread(buf, 1, total_len, file);
    AICX_ASSERT_IFX(ret_len, 0x1b3d03, "d", "romfs.bin read fail, ret=%d.\n", ret_len);

    fclose(file);

    fd = open(partinfo->filepath, O_RDWR | O_CREAT);
    if (fd < 0) {
        ret = -errno;
        aicx_error(0x1ab369, "sxd", "open %s flag(0x%x) error, ret=%d.\n",
                   partinfo->filepath, partinfo->flags, ret);

        aos_free(buf);
        return ret;
    }

    rsize = write(fd, buf, ret_len);
    if (rsize < 0) {
        ret = -errno;
        aicx_error(0x1482e9, "sxd", "%s write size(0x%x) error, ret=%d.\n",
                   partinfo->filepath, ret_len, ret);

        aos_free(buf);
        return ret;
    }

    ret = close(fd);
    if (ret < 0) {
        ret = -errno;
        aicx_error(0x064e8a, "sd", "%s close error, ret=%d.\n",
                   partinfo->filepath, ret);

        aos_free(buf);
        return ret;
    }

    aos_free(buf);

    return ret;
}
#endif

/**
 * fs_file_open - open fs file.
 *
 * Parameter:
 * @partinfo: input device partition information.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_file_open(struct vfs_blkdev_partinfo *partinfo)
{
    int fd = -1, ret = -1;
    struct stat st;

    fd = open(partinfo->filepath, partinfo->flags);
    if (fd < 0) {
        ret = -errno;
#ifdef WIN32
        ret = fs_file_win32_init(partinfo);
        AIC_ASSERT(ret == AIC_SUCCESS);

        fd = open(partinfo->filepath, partinfo->flags);
        AIC_ASSERT(fd > 0);

        ret = 0;
        partinfo->devhandle = (u32)fd;
#endif
        return ret;
    }

    ret = fstat(fd, &st);
    if (ret < 0) {
        ret = -errno;
        close(fd);
        return ret;
    }

    partinfo->total_sectors = ALIGN(st.st_size, partinfo->sector_size) /
                              partinfo->sector_size;

    partinfo->devhandle = (u32)fd;

    return AIC_SUCCESS;
}

/**
 * fs_file_close - close fs file.
 *
 * Parameter:
 * @partinfo: input device partition information.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_file_close(struct vfs_blkdev_partinfo *partinfo)
{
    int fd, ret;

    fd = (int)partinfo->devhandle;
    ret = close(fd);
    if (ret < 0) {
        ret = -errno;
        return ret;
    }

    return AIC_SUCCESS;
}

/**
 * fs_file_read - read fs file.
 *
 * Parameter:
 * @partinfo: input device partition information.
 * @start_sector: input start sector.
 * @sector_num: input sector number.
 * @buf: output buf.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_file_read(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                  u32 sector_num, void *buf)
{
    ssize_t rsize;
    size_t size;
    int fd, ret;
    long offset;

    fd = (int)partinfo->devhandle;

    offset = partinfo->sector_size * start_sector;
    ret = lseek(fd, offset, SEEK_SET);
    if (ret < 0) {
        ret = -errno;
        return ret;
    }

    size = partinfo->sector_size * sector_num;

    rsize = read(fd, buf, size);
    if (rsize < 0) {
        ret = -errno;
        return ret;
    }

    if (rsize != size) {
        return AIC_ESRCH;
    }

    return AIC_SUCCESS;
}

/**
 * fs_file_write - write fs file.
 *
 * Parameter:
 * @partinfo: input device partition information.
 * @start_sector: input start sector.
 * @sector_num: input sector number.
 * @buf: input buf.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_file_write(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                   u32 sector_num, const void *buf)
{
    ssize_t rsize;
    size_t size;
    int fd, ret;
    long offset;

    fd = (int)partinfo->devhandle;

    offset = partinfo->sector_size * start_sector;
    ret = lseek(fd, offset, SEEK_SET);
    if (ret < 0) {
        ret = -errno;
        return ret;
    }

    size = partinfo->sector_size * sector_num;

    rsize = write(fd, buf, size);
    if (rsize < 0) {
        ret = -errno;
        return ret;
    }

    if (rsize != size) {
        return AIC_ESRCH;
    }

    return AIC_SUCCESS;
}

/**
 * fs_file_abort - abort fs file.
 *
 * Parameter:
 * @partinfo: input device partition information.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_file_abort(struct vfs_blkdev_partinfo *partinfo)
{
    int fd, ret;

    fd = (int)partinfo->devhandle;
    ret = close(fd);
    if (ret < 0) {
        ret = -errno;
        return ret;
    }

    return AIC_SUCCESS;
}

/**
 * fs_file_release - release fs file logic sector.
 *
 * Parameter:
 * @partinfo: input device partition information.
 * @start_sector: input start sector.
 * @sector_num: input sector number.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_file_release(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                     u32 sector_num)
{
    return AIC_SUCCESS;
}

/**
 * fs_file_flush - flush fs file.
 *
 * Parameter:
 * @partinfo: input device partition information.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_file_flush(struct vfs_blkdev_partinfo *partinfo)
{
    int fd, ret;

    fd = (int)partinfo->devhandle;
    ret = fsync(fd);
    if (ret < 0) {
        ret = -errno;
        return ret;
    }

    return AIC_SUCCESS;
}

struct vfs_blkdev_ops s_fs_file_ops = {
    FS_FILE_DEV_PATH,
    fs_file_open,
    fs_file_close,
    fs_file_read,
    NULL,
    fs_file_write,
    fs_file_abort,
    fs_file_release,
    fs_file_flush,
};

int fs_dev_file_init(void)
{
    int ret;

    ret = vfs_blkdev_register(&s_fs_file_ops, 0, NULL);

    return ret;
}

AIC_COMP_P1_INIT_EXPORT(fs_dev_file_init);

