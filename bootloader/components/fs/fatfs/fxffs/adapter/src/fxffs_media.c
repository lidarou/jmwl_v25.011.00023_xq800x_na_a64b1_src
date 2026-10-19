/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system media Interface.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18a3c7

#include <fx_api.h>
#include "fxffs.h"
#include "fxffs_media_driver.h"

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * fxffs_media_open_notify:
 *  This function is the entry point to the media open notify.
 *
 * Parameter:
 * @media_ptr: fx media ptr.
 *
 * Return:
 *
 */
void fx_media_open_notify(FX_MEDIA *media_ptr)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media_ptr->fx_media_driver_info;

    if (fs_info->fs_notify->media_open)
        fs_info->fs_notify->media_open(fs_info);
}

/**
 * fxffs_media_close_notify:
 *  This function is the entry point to the media close notify.
 *
 * Parameter:
 * @media_ptr: fx media ptr.
 *
 * Return:
 *
 */
void fx_media_close_notify(FX_MEDIA *media_ptr)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media_ptr->fx_media_driver_info;

    if (fs_info->fs_notify->media_close)
        fs_info->fs_notify->media_close(fs_info);
}

/**
 * fxffs_media_abort:
 *  This function marks all open files for the specified media as
 *  aborted and then removes the media control block from the open
 *  media list and marks it as aborted as well.
 *
 * Parameter:
 * @media: file system media.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_abort(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_abort(fx_media);
}

/**
 * fxffs_media_cache_invalidate:
 *  This function examines the logical cache, flushing written sectors
 *  out to the media and invalidating all sectors in the logical cache.
 *
 * Parameter:
 * @media: file system media.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_cache_invalidate(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_cache_invalidate(fx_media);
}

/**
 * fxffs_media_check:
 *  This function checks the specified media for basic structural
 *  errors, including cross linking, invalid FAT chains, and lost
 *  clusters. The function also provides the capability to correct
 *  errors.
 *
 * Parameter:
 * @media: file system media.
 * @option:correction option
 *       check none:                            0
 *       FS_FAT_CHAIN_ERROR                     0x01
 *       FS_DIRECTORY_ERROR                     0x02
 *       FS_LOST_CLUSTER_ERROR                  0x04
 * @error:errors detected
 *       FS_FAT_CHAIN_ERROR                     0x01
 *       FS_DIRECTORY_ERROR                     0x02
 *       FS_LOST_CLUSTER_ERROR                  0x04
 *       FS_FILE_SIZE_ERROR                     0x08
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_check(void *media, u8 *mem_ptr, ulong mem_size,
                    ulong correction_option, ulong *error)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_check(fx_media, mem_ptr, mem_size,
                          correction_option, error);
}

/**
 * fxffs_media_close - close media ffs.
 *
 * Parameter:
 * @media: file system media.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_close(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    u32 ret;

    ret = fx_media_close(fx_media);

    free(fx_media);
    fs_info->media_handle = 0;

    return ret;
}

/**
 * fxffs_media_flush - flush media cache memory.
 *
 * Parameter:
 * @fs_info: file system information.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_flush(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_flush(fx_media);
}

/**
 * fxffs_media_format - format media.
 *
 * Parameter:
 * @media: file system media.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_format(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    u32 ret;

    if (!fs_info->media_handle) {
        fx_media = (FX_MEDIA *)malloc(sizeof(FX_MEDIA));
        ASSERT_INFO(fx_media, "malloc size(0x%x) memory fail.\n",
                    sizeof(FX_MEDIA));
        memset(fx_media, 0x0, sizeof(FX_MEDIA));
    }

    if ((strlen(FS_FXFAT_TYPE_NAME) == strlen(fs_info->cfg->fstype)) &&
        !memcmp(FS_FXFAT_TYPE_NAME, fs_info->cfg->fstype,
                strlen(fs_info->cfg->fstype))) {
        ret = fx_media_format(fx_media, fxffs_media_driver, fs_info,
                              fs_info->cfg->memory,
                              fs_info->cfg->memory_sectors *
                              fs_info->cfg->bytes_per_sector,
                              fs_info->volume_name,
                              fs_info->cfg->number_of_fats,
                              fs_info->cfg->directory_entries,
                              fs_info->cfg->hidden_sectors,
                              fs_info->cfg->total_sectors,
                              fs_info->cfg->bytes_per_sector,
                              fs_info->cfg->sectors_per_cluster,
                              fs_info->cfg->heads,
                              fs_info->cfg->sectors_per_track);
        if (ret != FX_SUCCESS) {
            free(fx_media);
            fs_info->media_handle = 0;
            log_print("ffs media format error, ret(0x%x)!\n", ret);
            return ret;
        }

        fs_info->media_handle = (u32)fx_media;
        return ret;
    } else if ((strlen(FS_FXEXFAT_TYPE_NAME) == strlen(fs_info->cfg->fstype)) &&
               !memcmp(FS_FXEXFAT_TYPE_NAME, fs_info->cfg->fstype,
                strlen(fs_info->cfg->fstype))) {
#ifdef FX_ENABLE_EXFAT
        ret = fx_media_exFAT_format(fx_media, fxffs_media_driver, fs_info,
                                    fs_info->cfg->memory,
                                    fs_info->cfg->memory_sectors *
                                    fs_info->cfg->bytes_per_sector,
                                    fs_info->volume_name,
                                    fs_info->cfg->number_of_fats,
                                    fs_info->cfg->hidden_sectors,
                                    fs_info->cfg->total_sectors,
                                    fs_info->cfg->bytes_per_sector,
                                    fs_info->cfg->sectors_per_cluster,
                                    fs_info->cfg->volume_serial_number,
                                    fs_info->cfg->boundary_unit);
        if (ret != FX_SUCCESS) {
            free(fx_media);
            fs_info->media_handle = 0;
            log_print("exfat fs media format error, ret(0x%x)!\n", ret);
            return ret;
        }

        fs_info->media_handle = (u32)fx_media;
        return ret;
#endif
    }

    free(fx_media);
    return FX_INVALID_OPTION;
}

/**
 * fxffs_media_open:
 *    This function opens the specified media with the supplied device
 *    driver. The specified media must conform to the FAT compatible
 *    file format, which is verified during the media open process.
 *
 * Parameter:
 * @media: file system media.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_open(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *media_ptr = (FX_MEDIA *)fs_info->media_handle;
    u32 ret;

    if (!fs_info->media_handle) {
        media_ptr = malloc(sizeof(FX_MEDIA));
        ASSERT_INFO(media_ptr, "malloc size(0x%x) memory fail.\n",
                    sizeof(FX_MEDIA));
        memset(media_ptr, 0x0, sizeof(FX_MEDIA));
    }

    ret = fx_media_open(media_ptr, fs_info->volume_name,
                        fxffs_media_driver, fs_info,
                        fs_info->cfg->memory, fs_info->cfg->memory_sectors *
                        fs_info->cfg->bytes_per_sector);
    if (ret != FX_SUCCESS) {
        free(media_ptr);
        fs_info->media_handle = 0;
        log_print("ffs media open error, ret(0x%x)!\n", ret);
        return ret;
    }

    if (fs_info->part_info.sector_size != media_ptr->fx_media_bytes_per_sector ||
        fs_info->part_info.total_sectors != media_ptr->fx_media_total_sectors) {
        fxffs_media_close(media);
        ret = FX_MEDIA_INVALID;
    } else {
        fs_info->media_handle = (u32)media_ptr;
        fs_info->cfg->number_of_fats = media_ptr->fx_media_number_of_FATs;
        fs_info->cfg->directory_entries = media_ptr->fx_media_root_directory_entries;
        fs_info->cfg->hidden_sectors = media_ptr->fx_media_hidden_sectors;
        fs_info->cfg->sectors_per_cluster = media_ptr->fx_media_sectors_per_cluster;
        fs_info->cfg->heads = media_ptr->fx_media_heads;
        fs_info->cfg->sectors_per_track = media_ptr->fx_media_sectors_per_track;
#ifdef FX_ENABLE_EXFAT
        fs_info->cfg->volume_serial_number = media_ptr->fx_media_exfat_volume_serial_number;
#endif
    }

    return ret;
}

/**
 * fxffs_media_read:
 *    This function reads the specified raw logical sector from the
 *    specified media.
 *
 * Parameter:
 * @media: file system media.
 * @logical_sector: logical sector to read.
 * @buf: read buffer.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_read(void *media, ulong logical_sector, void *buf)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_read(fx_media, logical_sector, buf);
}

/**
 * fxffs_media_space_available:
 *    This function returns the number of bytes available in the
 *    specified media.
 *
 * Parameter:
 * @media: file system media.
 * @available_bytes: the number of available bytes.
 * @total_bytes: the number of total bytes.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_space_available(void *media, ulong *available_bytes,
                              ulong *total_bytes)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    u32 ret;

    ret = fx_media_space_available(fx_media, available_bytes);

    *total_bytes = fx_media->fx_media_total_clusters *
                   fx_media->fx_media_sectors_per_cluster *
                   fx_media->fx_media_bytes_per_sector;

    return ret;
}

/**
 * fxffs_media_volume_get:
 *  This function reads the volume name stored in the media's boot
 *  record or root directory.
 *
 * Parameter:
 * @media: file system media.
 * @volume_name: volume name.
 * @volume_source: volume source.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_volume_get(void *media, char *volume_name, u32 volume_source)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_volume_get(fx_media, volume_name, volume_source);
}

/**
 * fxffs_media_volume_get_extended:
 *  This function reads the volume name stored in the media's boot
 *  record or root directory.
 *
 * Parameter:
 * @media: file system media.
 * @volume_name: volume name.
 * @volume_name_len: buffer length for volume_name
 * @volume_source: volume source.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_volume_get_extended(void *media, char *volume_name,
                                  u32 volume_name_len, u32 volume_source)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_volume_get_extended(fx_media, volume_name,
                                        volume_name_len, volume_source);
}

/**
* fxffs_media_volume_set:
*  This function sets the volume name to the name supplied by the caller.
*
* Parameter:
* @media: file system media.
* @volume_name: volume name.
*
* Return:
*    FX_SUCCESS is success, other fail.
*/
u32 fxffs_media_volume_set(void *media, char *volume_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_volume_set(fx_media, volume_name);
}

/**
 * fxffs_media_write:
 *  This function writes the specified raw logical sector to the
 *  specified media.
 *
 * Parameter:
 * @media: file system media.
 * @logical_sector: logical sector to write.
 * @buf: write buffer.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_write(void *media, ulong logical_sector, void *buf)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_write(fx_media, logical_sector, buf);
}

/**
 * fxffs_media_open_notify_set:
 *  This function sets the notify function called when media is open.
 *
 * Parameter:
 * @media: file system media.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_open_notify_set(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_open_notify_set(fx_media, fx_media_open_notify);
}

/**
 * fxffs_media_close_notify_set:
 *  This function sets the notify function called when media is close.
 *
 * Parameter:
 * @media: file system media.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_close_notify_set(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_media_close_notify_set(fx_media, fx_media_close_notify);
}

/**
 * fxffs_media_extended_space_available:
 *    This function returns the number of bytes available in the
 *    specified media.
 *
 * Parameter:
 * @media: file system media.
 * @available_bytes: the number of available bytes.
 * @total_bytes: the number of total bytes.
 * @sectors_per_cluster: the sector number of cluste.
 * @bytes_per_sector: sector size.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_extended_space_available(void *media, u64 *available_bytes,
                                       u64 *total_bytes,
                                       u32 *sectors_per_cluster,
                                       u32 *bytes_per_sector)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    u32 ret;

    ret = fx_media_extended_space_available(fx_media, available_bytes);

    *total_bytes = (u64)fx_media->fx_media_total_clusters *
                   (u64)fx_media->fx_media_sectors_per_cluster *
                   (u64)fx_media->fx_media_bytes_per_sector;

    *sectors_per_cluster = fx_media->fx_media_sectors_per_cluster;
    *bytes_per_sector = fx_media->fx_media_bytes_per_sector;

    return ret;
}

/**
 * fxffs_media_extended_space_available:
 *  This function enables the ffs Fault Tolerant feature. It first
 *  searches for a valid log file.  A valid log file indicates the
 *  previous write operation failed, and appropriate action now must
 *  be taken to restore the integrity of the file system.  Once the
 *  recovery effort is completed, the file system is properly restored.
 *  An empty log file indicates the previous write operaiton was
 *  successfully completed and no action needs to be taken at this
 *  point.  If the file system does not have a log file, or the
 *  checksum is not valid, it is an indication either the file system
 *  is not under the protection of FileX Fault Tolerant.  A new log
 *  file is created.
 *
 * Parameter:
 * @media: file system media.
 * @available_bytes: the number of available bytes.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_media_fault_tolerant_enable(void *media)
{
#ifdef FX_ENABLE_FAULT_TOLERANT
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_fault_tolerant_enable(fx_media, fs_info->cfg->fault_tolerant_memory,
                                    fs_info->cfg->fault_tolerant_sectors *
                                    fs_info->cfg->bytes_per_sector);
#else
    return FX_INVALID_OPTION;
#endif
}

FXFFS_MEDIA_OPS s_fxffs_media_ops = {
    fxffs_media_abort,
    fxffs_media_cache_invalidate,
    fxffs_media_check,
    fxffs_media_close,
    fxffs_media_flush,
    fxffs_media_format,
    fxffs_media_open,
    fxffs_media_read,
    fxffs_media_space_available,
    fxffs_media_volume_get,
    fxffs_media_volume_get_extended,
    fxffs_media_volume_set,
    fxffs_media_write,
    fxffs_media_extended_space_available,
    fxffs_media_fault_tolerant_enable,
};

