/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system media driver Interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04894d
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <fx_api.h>
#include "fxffs.h"

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fxffs_media_driver:
 *  This function is the entry point to the generic media driver.
 *
 * Parameter:
 * @media_ptr: fx media ptr
 *
 * Return:
 *
 */
void fxffs_media_driver(FX_MEDIA *media_ptr)
{
    ulong logical_sector, sectors, partition_start, partition_size;
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media_ptr->fx_media_driver_info;
    u8 *src_buf, *dst_buf;
    int ret = AIC_EPERM;
    u32 status;

    switch(media_ptr->fx_media_driver_request) {
        case FX_DRIVER_READ:
            {
                /* Setup the destination buffer and logical sector. */
                logical_sector = media_ptr->fx_media_driver_logical_sector +
                                 media_ptr->fx_media_hidden_sectors;
                sectors = media_ptr->fx_media_driver_sectors;
                dst_buf = (u8 *) media_ptr->fx_media_driver_buffer;

                if (fs_info->dev_ops->read)
                    ret = fs_info->dev_ops->read(&fs_info->part_info,
                                                 logical_sector, sectors,
                                                 dst_buf);
                /* Determine if write was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                /* Successful driver request. */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }
        case FX_DRIVER_WRITE:
            {
                /* Setup the source buffer and logical sector. */
                logical_sector = media_ptr->fx_media_driver_logical_sector +
                                 media_ptr->fx_media_hidden_sectors;
                sectors = media_ptr->fx_media_driver_sectors;
                src_buf = (u8 *) media_ptr->fx_media_driver_buffer;

                if (fs_info->dev_ops->write)
                    ret = fs_info->dev_ops->write(&fs_info->part_info,
                                                  logical_sector, sectors,
                                                  src_buf);
                /* Determine if write was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                /* Successful driver request. */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }
        case FX_DRIVER_RELEASE_SECTORS:
            {
                /* Setup the logical sector. */
                logical_sector =  media_ptr->fx_media_driver_logical_sector;

                if (fs_info->dev_ops->release)
                    ret = fs_info->dev_ops->release(&fs_info->part_info,
                                                    logical_sector, 1);
                /* Determine if release was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                /* Return driver success. */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }
        case FX_DRIVER_FLUSH:
            {
                if (fs_info->dev_ops->flush)
                    ret = fs_info->dev_ops->flush(&fs_info->part_info);
                /* Determine if flush was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                /* Return driver success. */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }
        case FX_DRIVER_ABORT:
            {
                if (fs_info->dev_ops->abort)
                    ret = fs_info->dev_ops->abort(&fs_info->part_info);
                /* Determine if abort was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                /* Return driver success. */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }
        case FX_DRIVER_INIT:
            {
                media_ptr->fx_media_driver_free_sector_update =  FX_TRUE;

                fs_info->part_info.devpath = fs_info->dev_ops->path;
                fs_info->part_info.target = fs_info->volume_name;
                fs_info->part_info.sector_size = fs_info->cfg->bytes_per_sector;
                fs_info->part_info.total_sectors = fs_info->cfg->total_sectors;

                if(fs_info->dev_ops->open)
                    ret = fs_info->dev_ops->open(&fs_info->part_info);
                /* Determine if open was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                fs_info->cfg->bytes_per_sector = fs_info->part_info.sector_size;
                fs_info->cfg->total_sectors = fs_info->part_info.total_sectors;

                /* Successful driver request.  */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }
        case FX_DRIVER_UNINIT:
            {
                if(fs_info->dev_ops->close) {
                    ret = fs_info->dev_ops->close(&fs_info->part_info);
                }
                /* Determine if close was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                /* Successful driver request.  */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break;
            }
        case FX_DRIVER_BOOT_READ:
            {
                /* Read the boot record and return to the caller. */

                /* Setup the destination buffer. */
                dst_buf =  (u8 *) media_ptr->fx_media_driver_buffer;

                /* Read boot sector. */
                if (fs_info->dev_ops->read) {
                    ret = fs_info->dev_ops->read(&fs_info->part_info,
                                                 0, 1, dst_buf);
                }
                /* Determine if boot read was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                status = _fx_partition_offset_calculate(dst_buf, 0,
                                                        &partition_start,
                                                        &partition_size);
                /* check partition read error. */
                if (status != FX_SUCCESS) {
                    /* Unsuccessful driver request. */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    return;
                }

                /* Now determint if there if a partition... */
                if (partition_size) {
                    ret = fs_info->dev_ops->read(&fs_info->part_info,
                                                 partition_start, 1, dst_buf);
                }
                /* Determine if boot read was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }

                /* Successful driver request.  */
                media_ptr ->fx_media_driver_status =  FX_SUCCESS;

                break;
            }
        case FX_DRIVER_BOOT_WRITE:
            {
                /* Write the boot record and return to the caller.  */

                /* Setup the source buffer.  */
                src_buf = (u8 *) media_ptr->fx_media_driver_buffer;

                /* Write boot sector. */
                if(fs_info->dev_ops->write) {
                    ret = fs_info->dev_ops->write(&fs_info->part_info,
                                                  0, 1, src_buf);
                }

                /* Determine if the boot write was successful.  */
                if (ret != AIC_SUCCESS)
                {
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    return;
                }
                /* Successful driver request.  */
                media_ptr->fx_media_driver_status =  FX_SUCCESS;
                break ;
            }
        default:
            {
                /* Invalid driver request.  */
                media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                break;
            }
    }
}

