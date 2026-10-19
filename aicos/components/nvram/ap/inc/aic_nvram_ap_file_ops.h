/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * Export the basic nvram file operation Interface (API).
 */

#ifndef __AIC_AP_NVRAM_FILE_OPS_H__
#define __AIC_AP_NVRAM_FILE_OPS_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * nvram_ap_file_open - nvram file open.
 *
 * Parameter:
 * @name: device name.
 *
 * Return:
 *    @handle: open handle.
 */
u32 nvram_ap_file_open(char *name);

/**
 * nvram_ap_file_info_get - nvram file info get.
 *
 * Parameter:
 * @handle: handle.
 * @size: nvram partition size.
 * @sector_size: nvram sector size.
 *
 * Return:
 */
void nvram_ap_file_info_get(u32 handle, u32 *size, u32 *sector_size);

/**
 * nvram_ap_file_read - nvram file read.
 *
 * Parameter:
 * @handle: handle.
 * @buf: to store read data.
 * @size: read sector size.
 * @offset: read offset.
 *
 * Return:
 *    @read size.
 *    @read buf
 */
u32 nvram_ap_file_read(u32 handle, void *buf, u32 size, u32 offset);

/**
 * nvram_ap_file_write - nvram file write.
 *
 * Parameter:
 * @handle: handle.
 * @buf: write data.
 * @size: write sector size.
 * @offset: write offset.
 *
 * Return:
 *
 */
void nvram_ap_file_write(u32 handle, void *buf, u32 size, u32 offset);

/**
 * nvram_ap_file_close - nvram file close.
 *
 * Parameter:
 * @handle: file handle.
 *
 * Return:
 *
 */
void nvram_ap_file_close(u32 *handle);

/**
 * nvram_ap_fmd_open - nvram fmd file open.
 *
 * Parameter:
 * @name: device name.
 *
 * Return:
 *    @handle: open handle.
 */
u32 nvram_ap_fmd_open(char *name);

/**
 * nvram_ap_fmd_info_get - nvram fmd file info get.
 *
 * Parameter:
 * @handle: handle.
 * @addr: nvram partition address.
 * @size: nvram partition size.
 * @sector_size: nvram sector size.
 *
 * Return:
 */
void nvram_ap_fmd_info_get(u32 handle, u32 *addr, u32 *size, u32 *sector_size);

/**
 * nvram_ap_fmd_read - nvram fmd file read.
 *
 * Parameter:
 * @handle: handle.
 * @buf: to store read data.
 * @size: read sector size.
 * @offset: read offset.
 *
 * Return:
 *    @u32 return_len: read size.
 *    @buf: read buf
 */
u32 nvram_ap_fmd_read(u32 handle, void *buf, u32 size, u32 offset);

/**
 * nvram_ap_fmd_write - nvram fmd file write.
 *
 * Parameter:
 * @handle: handle.
 * @buf: write data.
 * @size: write sector size.
 * @offset: write offset.
 *
 * Return:
 *
 */
void nvram_ap_fmd_write(u32 handle, void *buf, u32 size, u32 offset);

/**
 * nvram_ap_fmd_close - nvram fmd file close.
 *
 * Parameter:
 * @handle: file handle.
 *
 * Return:
 *
 */
void nvram_ap_fmd_close(u32 *handle);

#ifdef WIN32
/**
 * nvram_ap_set_file_path - set nvram file path.
 *
 * Parameter:
 * @download_bin_path: download bin path.
 * @download_bin_name: aic nvram download bin name.
 * @output_bin_path: output bin path.
 *
 * Return:
 *
 */
void nvram_ap_set_file_path(char *download_bin_path,
                            char *download_bin_name,
                            char *output_bin_path);

/**
 * nvram_ap_set_custom_file_path - set nvram custom file path.
 *
 * Parameter:
 * @custom_download_bin_name: custom nvram download bin name.
 *
 * Return:
 *
 */
void nvram_ap_set_custom_file_path(char *custom_download_bin_name);

/**
 * nvram_ap_set_diff_file_path - set nvram diff file path.
 *
 * Parameter:
 * @diff_bin_name: nvram diff bin name.
 *
 * Return:
 *
 */
void nvram_ap_set_diff_file_path(char *diff_bin_name);
#endif

/**---------------------------------------------------------------------------*/
#endif /* __AIC_AP_NVRAM_FILE_OPS_H__ */

