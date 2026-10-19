/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system manager Interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08ba67
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <aic_type.h>
#include <stdio.h>
#include <ctype.h>
#include "fx_api.h"
#include "fxffs.h"

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fxffs_media_open_notify - fs open media pass callback.
 *
 * Parameter:
 *
 * Return:
 *
 */
void fxffs_media_open_notify(void *fxffs_info)
{

}

/**
 * fxffs_media_close_notify - fs close media pass callback.
 *
 * Parameter:
 *
 * Return:
 *
 */
void fxffs_media_close_notify(void *fxffs_info)
{

}

/**
 * fxffs_file_write_notify - fs write file pass callback.
 *
 * Parameter:
 *
 * Return:
 *
 */
void fxffs_file_write_notify(void *fxffs_info)
{

}


FXFFS_NOTIFY s_fxffs_notify = {
    fxffs_media_open_notify,
    fxffs_media_close_notify,
    fxffs_file_write_notify,
};

/**
 * fxffs_media_init - fxffs media init
 *
 * Parameter:
 * @fxffs_info: fxffs info.
 *
 * Return:
 *  fxffs_SUCCESS pass, other fail.
 */
u32 fxffs_media_init(FXFFS_INFO *fxffs_info)
{
    u32 ret = FX_INVALID_OPTION, chkcnt = 0;
    ulong error, mem_size;
    u8 *mem_ptr = NULL;

    do {
        if (fxffs_info->initd) {
            ret = FX_SUCCESS;
            break;
        }

        if (!fxffs_info->dev_ops) {
            ret = FX_INVALID_OPTION;
            break;
        }

        if (!fxffs_info->cfg->memory)
            fxffs_info->cfg->memory = (u8 *)aos_zalloc(
                                            fxffs_info->cfg->memory_sectors *
                                            fxffs_info->cfg->bytes_per_sector);

        if (fxffs_info->cfg->fault_tolerant_eb) {
            if (!fxffs_info->cfg->fault_tolerant_memory)
                fxffs_info->cfg->fault_tolerant_memory = (u8 *)aos_zalloc(
                                    fxffs_info->cfg->fault_tolerant_sectors *
                                    fxffs_info->cfg->bytes_per_sector);
        }

        fxffs_info->media_init_ops = &s_fxffs_media_init_ops;
        fxffs_info->media_ops = &s_fxffs_media_ops;
        fxffs_info->media_dir_ops = &s_fxffs_media_dir_ops;
        fxffs_info->media_file_ops = &s_fxffs_media_file_ops;
        fxffs_info->media_unicode_ops = &s_fxffs_media_unicode_ops;

        fxffs_info->media_init_ops->init();

FXFFS_MEDIA_OPEN:
        ret = fxffs_info->media_ops->open(fxffs_info);

FXFFS_MEDIA_FORMAT:
        if (ret != FX_SUCCESS && fxffs_info->cfg->format_eb) {
                ret = fxffs_info->media_ops->format(fxffs_info);
                if (ret != FX_SUCCESS) {
                    break;
                }

                ret = fxffs_info->media_ops->open(fxffs_info);
                if (ret != FX_SUCCESS) {
                    break;
                }
        } else if (ret != FX_SUCCESS) {
            break;
        }

        if (fxffs_info->cfg->fault_tolerant_eb) {
            ret = fxffs_info->media_ops->fault_tolerant_enable(fxffs_info);
            if (ret != FX_SUCCESS) {
                alog_error("fs tolerant enable fail, ret(%d).\n", ret);
                fxffs_info->media_ops->close(fxffs_info);
                goto FXFFS_MEDIA_FORMAT;
            }
        }

        if (fxffs_info->cfg->check_eb && !chkcnt) {
            mem_size = ROUND_UP(((fxffs_info->cfg->total_sectors / fxffs_info->cfg->sectors_per_cluster) >> 3) + 1,
                                fxffs_info->cfg->bytes_per_sector) + fxffs_info->cfg->bytes_per_sector * 4;
            if (!mem_ptr) {
                mem_ptr = (u8 *)aos_zalloc(mem_size);
                AIC_ASSERT(mem_ptr);
            }
            ret = fxffs_info->media_ops->check(fxffs_info, mem_ptr, mem_size,
                                               FX_FAT_CHAIN_ERROR |
                                               FX_DIRECTORY_ERROR |
                                               FX_LOST_CLUSTER_ERROR,
                                               &error);
            aicx_info(0x00a0e3, "sxx", "fs volume(%s) check error(0x%x), ret(0x%x).\n",
                      fxffs_info->cfg->target, error, ret);
            //AOS_ASSERT(ret == FX_SUCCESS);

            aos_free(mem_ptr);
            mem_ptr = NULL;

            if ((error & ~FX_FILE_SIZE_ERROR) != FX_SUCCESS) {
                fxffs_info->media_ops->close(fxffs_info);
                if (chkcnt > 0) {
                    ret = AIC_ENOEXEC;
                    goto FXFFS_MEDIA_FORMAT;
                }
                chkcnt++;
                goto FXFFS_MEDIA_OPEN;
            }
        }

        fxffs_info->fs_notify = &s_fxffs_notify;
        fxffs_info->initd = 1;
    } while(0);

    if (ret != FX_SUCCESS) {
        if (fxffs_info->cfg->memory) {
            aos_free(fxffs_info->cfg->memory);
            fxffs_info->cfg->memory = NULL;
        }

        if (fxffs_info->cfg->fault_tolerant_memory) {
            aos_free(fxffs_info->cfg->fault_tolerant_memory);
            fxffs_info->cfg->fault_tolerant_memory = NULL;
        }

        aicx_error(0x0a9462, "d", "fxffs_media_init fail, ret=%d.\n", ret);
    }

    return ret;
}

/**
 * fs_media_uninit - fs media uninit
 *
 * Parameter:
 * @dev_type: fs device type.
 *
 * Return:
 *  FS_SUCCESS pass, other fail.
 */
u32 fxffs_media_uninit(FXFFS_INFO *fs_info)
{
    u32 ret = FX_INVALID_OPTION;

    if (!fs_info->initd)
        return FX_SUCCESS;

    do {
        fs_info->initd = 0;

        ret = fs_info->media_ops->flush(fs_info);

        ret = fs_info->media_ops->close(fs_info);
    } while(0);

    if (fs_info->cfg->memory) {
        aos_free(fs_info->cfg->memory);
        fs_info->cfg->memory = NULL;
    }

    if (fs_info->cfg->fault_tolerant_memory) {
        aos_free(fs_info->cfg->fault_tolerant_memory);
        fs_info->cfg->fault_tolerant_memory = NULL;
    }

    return ret;
}



