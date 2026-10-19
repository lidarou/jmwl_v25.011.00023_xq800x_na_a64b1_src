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
#define AIC_THIS_FILE    0x062dff

#include <aic_errno.h>
#include <aic_type.h>
#include <stdio.h>
#include <ctype.h>
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

        if (!fxffs_info->cfg->memory) {
            mem_size = fxffs_info->cfg->memory_sectors *
                       fxffs_info->cfg->bytes_per_sector;
            fxffs_info->cfg->memory = (u8 *)malloc(mem_size);
            ASSERT_INFO(fxffs_info->cfg->memory,
                        "malloc size(0x%x) memory fail.\n", mem_size);
            memset(fxffs_info->cfg->memory, 0x0, mem_size);
        }

        if (fxffs_info->cfg->fault_tolerant_eb) {
            if (!fxffs_info->cfg->fault_tolerant_memory) {
                mem_size = fxffs_info->cfg->fault_tolerant_sectors *
                           fxffs_info->cfg->bytes_per_sector;
                fxffs_info->cfg->fault_tolerant_memory = (u8 *)malloc(mem_size);
                ASSERT_INFO(fxffs_info->cfg->fault_tolerant_memory,
                            "malloc size(0x%x) memory fail.\n", mem_size);
                memset(fxffs_info->cfg->fault_tolerant_memory, 0x0, mem_size);
            }
        }

        fxffs_info->media_init_ops = &s_fxffs_media_init_ops;
        fxffs_info->media_ops = &s_fxffs_media_ops;
        fxffs_info->media_dir_ops = &s_fxffs_media_dir_ops;
        fxffs_info->media_file_ops = &s_fxffs_media_file_ops;

        fxffs_info->media_init_ops->init();

FXFFS_MEDIA_OPEN:
        ret = fxffs_info->media_ops->open(fxffs_info);

        if (ret != FX_SUCCESS && fxffs_info->cfg->format_eb) {
                ret = fxffs_info->media_ops->format(fxffs_info);
                if (ret != FX_SUCCESS) {
                    log_print("fs volume(%s) format error, ret(0x%x).\n",
                               fxffs_info->cfg->target, ret);
                    break;
                }

                ret = fxffs_info->media_ops->open(fxffs_info);
                if (ret != FX_SUCCESS) {
                    log_print("fs volume(%s) open error, ret(0x%x).\n",
                               fxffs_info->cfg->target, ret);
                    break;
                }
        } else if (ret != FX_SUCCESS) {
            log_print("fs volume(%s) open error, ret(0x%x).\n",
                       fxffs_info->cfg->target, ret);
            break;
        }

        if (fxffs_info->cfg->check_eb && !chkcnt) {
            mem_size = ROUND_UP(((fxffs_info->cfg->total_sectors / fxffs_info->cfg->sectors_per_cluster) >> 3) + 1,
                                fxffs_info->cfg->bytes_per_sector) + fxffs_info->cfg->bytes_per_sector * 4;
            if (!mem_ptr) {
                mem_ptr = (u8 *)malloc(mem_size);
                ASSERT_INFO(mem_ptr,
                            "malloc size(0x%x) memory fail.\n", mem_size);
                memset(mem_ptr, 0x0, mem_size);
            }
            ret = fxffs_info->media_ops->check(fxffs_info, mem_ptr, mem_size,
                                               FX_FAT_CHAIN_ERROR |
                                               FX_DIRECTORY_ERROR |
                                               FX_LOST_CLUSTER_ERROR,
                                               &error);
            log_print("fs volume(%s) check error(0x%x), ret(0x%x).\n",
                      fxffs_info->cfg->target, error, ret);

            ASSERT(ret == FX_SUCCESS);

            free(mem_ptr);
            mem_ptr = NULL;

            if ((error & ~FX_FILE_SIZE_ERROR) != FX_SUCCESS) {

                ASSERT(chkcnt == 0);
                chkcnt++;

                ret = fxffs_info->media_ops->close(fxffs_info);
                ASSERT_INFO(ret == FX_SUCCESS,
                                "fs volume(%s) close error, ret(0x%x).\n",
                                fxffs_info->cfg->target, ret);
                goto FXFFS_MEDIA_OPEN;
            }
        }

        if (fxffs_info->cfg->fault_tolerant_eb) {
            ret = fxffs_info->media_ops->fault_tolerant_enable(fxffs_info);
            ASSERT_INFO(ret == FX_SUCCESS,
                            "fs volume(%s) fault tolerant enable error, ret(0x%x).\n",
                            fxffs_info->cfg->target, ret);
        }

        fxffs_info->fs_notify = &s_fxffs_notify;
        fxffs_info->initd = 1;

        log_print("fs volume(%s) open success.\n", fxffs_info->cfg->target);
    } while(0);

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
        if(ret != FX_SUCCESS) {
            log_print("fs volume(%s) flush error, ret(0x%x).\n",
                       fs_info->cfg->target, ret);
        }

        ret = fs_info->media_ops->close(fs_info);
        if(ret != FX_SUCCESS) {
            log_print("fs volume(%s) close error, ret(0x%x).\n",
                       fs_info->cfg->target, ret);
        }
    } while(0);

    return ret;
}



