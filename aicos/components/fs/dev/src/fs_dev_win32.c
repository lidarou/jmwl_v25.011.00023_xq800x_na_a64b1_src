/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic fs win32 device operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16b089
#define LOG_MODULE    AMOD_FS

#include <aic_type.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_assert.h>
#include <errno.h>
#include "vfs_dev.h"
#include "vfs.h"
/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Static Global Variables
 **---------------------------------------------------------------------------*/
static const char *s_output_bin_path = "../../../../out/win32/fs/";

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_win32_open - open fs win32 device partition.
 *
 * Parameter:
 * @handle: output win32 device handle.
 * @name: input partition name.
 * @sector_size: input sector size.
 * @sector_num: input sector number.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_win32_open(struct vfs_blkdev_partinfo *partinfo)
{
    u8 *tmp_name, *ptr;
    u32 size, ret_len;
    errno_t ferrno;
    FILE *file;

    AOS_ASSERT(s_output_bin_path);

    size = strlen(s_output_bin_path) + strlen(partinfo->target) + strlen(".bin") + 0x10;
    tmp_name = (u8 *)aos_zalloc(size);

    if (partinfo->target[0] != '\0' && partinfo->target[0] != 'O') {
        aos_memcpy(tmp_name, s_output_bin_path, strlen(s_output_bin_path));
        aos_memcpy(tmp_name + strlen(s_output_bin_path), partinfo->target, strlen(partinfo->target));
    } else {
        snprintf(tmp_name, size, "%s", "../fs/romfs");
    }

    aos_memcpy(tmp_name + strlen(tmp_name), ".bin", strlen(".bin"));

    tmp_name[strlen(tmp_name)] = '\0';
    ferrno = fopen_s(&file, tmp_name, "rb+");
    if (ENOENT == ferrno) {
        ferrno = fopen_s(&file, tmp_name, "wb+");
        AICX_ASSERT_IFX(!ferrno, 0x166acf, "d", "fs win open fail, ferrno(%d).\n", ferrno);

        size = partinfo->sector_size * partinfo->total_sectors;
        ptr = aos_malloc(size);
        aos_memset(ptr, 0xFF, size);
        ret_len = fwrite(ptr, 1, size, file);
        AICX_ASSERT_IFX(ret_len == size,
                        0x0865fc, "xx", "fs win write fail, size(0x%x 0x%x).\n", ret_len, size);
        fflush(file);
        aos_free(ptr);
    }

    aos_free(tmp_name);

    partinfo->devhandle = (u32)file;

    return AIC_SUCCESS;
}

/**
 * fs_win32_close - close fs win32 device partition.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_win32_close(struct vfs_blkdev_partinfo *partinfo)
{
    FILE *file = (FILE *)partinfo->devhandle;

    fflush(file);

    fclose(file);

    return AIC_SUCCESS;
}

/**
 * fs_win32_read - read fs win32 device partition data.
 *
 * Parameter:
 * @handle: input device handle.
 * @logic_sector: input logic sector.
 * @sector_num: input sector number.
 * @buf: output buf.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_win32_read(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                  u32 sector_num, void *buf)
{
    FILE *file = (FILE *)partinfo->devhandle;
    u32 ret_len, total_size;
    ulong offset, size;
    fpos_t fpost;

    offset = partinfo->sector_size * start_sector;

    fpost = offset;
    fsetpos(file, &fpost);
    ret_len = ftell(file);
    AICX_ASSERT_IFX(ret_len == offset, 0x163cbf, "xx", "fs win ftell offset(0x%x 0x%x) fail.\n",
                    ret_len, offset);

    total_size = partinfo->total_sectors * partinfo->sector_size;
    size = partinfo->sector_size * sector_num;
    AICX_ASSERT_IFX((size + offset) <= total_size,
                    0x1303e8, "xxx", "fs win read fail, size(0x%x) offset(0x%x) total(0x%x).\n",
                    size, offset, total_size);

    ret_len = fread(buf, 1, size, file);
    AICX_ASSERT_IFX(ret_len == size,
                    0x037470, "xxx", "fs win read offset(0x%x) fail, size(0x%x 0x%x).\n",
                    offset, ret_len, size);

    return AIC_SUCCESS;
}

/**
 * fs_win32_write - write fs win32 device partition data.
 *
 * Parameter:
 * @handle: input device handle.
 * @logic_sector: input logic sector.
 * @sector_num: input sector number.
 * @buf: input buf.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_win32_write(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                   u32 sector_num, const void *buf)
{
    FILE *file = (FILE *)partinfo->devhandle;
    u32 ret_len, total_size;
    ulong offset, size;
    fpos_t fpost;

    offset = partinfo->sector_size * start_sector;
    fpost = offset;
    fsetpos(file, &fpost);
    ret_len = ftell(file);
    AICX_ASSERT_IFX(ret_len == offset, 0x163cbf, "xx", "fs win ftell offset(0x%x 0x%x) fail.\n",
                    ret_len, offset);

    total_size = partinfo->total_sectors * partinfo->sector_size;
    size = partinfo->sector_size * sector_num;
    AICX_ASSERT_IFX((size + offset) <= total_size,
                    0x0f4e51, "xxx", "fs win write fail, size(0x%x) offset(0x%x) total(0x%x).\n",
                    size, offset, total_size);

    ret_len = fwrite(buf, 1, size, file);
    AICX_ASSERT_IFX(ret_len == size,
                    0x01c5a3, "xxx", "fs win write offset(0x%x) fail, size(0x%x 0x%x).\n",
                    offset, ret_len, size);

    fflush(file);

    return AIC_SUCCESS;
}

/**
 * fs_win32_abort - abort fs win32 device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_win32_abort(struct vfs_blkdev_partinfo *partinfo)
{
    return AIC_SUCCESS;
}

/**
 * fs_win32_release - release fs win32 device logic sector.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_win32_release(struct vfs_blkdev_partinfo *partinfo, u32 start_sector,
                     u32 sector_num)
{
    return AIC_SUCCESS;
}

/**
 * fs_win32_flush - flush fs win32 device.
 *
 * Parameter:
 * @handle: input device handle.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int fs_win32_flush(struct vfs_blkdev_partinfo *partinfo)
{
    FILE *file = (FILE *)partinfo->devhandle;

    fflush(file);

    return AIC_SUCCESS;
}

struct vfs_blkdev_ops s_fs_win32_ops = {
    FS_WIN32_DEV_PATH,
    fs_win32_open,
    fs_win32_close,
    fs_win32_read,
    NULL,
    fs_win32_write,
    fs_win32_abort,
    fs_win32_release,
    fs_win32_flush,
};

int fs_dev_win32_init(void)
{
    int ret;

    ret = vfs_blkdev_register(&s_fs_win32_ops, 0, NULL);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0e5628, "d", "fs register win32 block device failed: %d\n", ret);
    }

    return ret;
}

AIC_NORMAL_COMP_P1_INIT_EXPORT(fs_dev_win32_init);

