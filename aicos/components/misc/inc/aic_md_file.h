/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the misc file interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_MISC_FILE_H__
#define __AIC_MISC_FILE_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * md_file_open - misc file open.
 *
 * Parameter:
 * @name: device name.
 * @handle: open handle.
 *
 * Return:
 *    AIC_SUCCESS is success, other fail.
 */
int md_file_open(char *name, u32 *handle);

/**
 * md_file_info_get - misc info get.
 *
 * Parameter:
 * @handle: handle.
 * @size: misc partition size.
 * @sector_size: misc partition sector size.
 *
 * Return:
 */
void md_file_info_get(u32 handle, u32 *size, u32 *sector_size);

/**
 * md_file_read - misc file read.
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
u32 md_file_read(u32 handle, void *buf, u32 size, u32 offset);

/**
 * md_file_write - misc file write.
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
void md_file_write(u32 handle, void *buf, u32 size, u32 offset);

/**
 * md_file_close - misc file close.
 *
 * Parameter:
 * @handle: file handle.
 *
 * Return:
 *
 */
void md_file_close(u32 *handle);

/**
 * md_fmd_open - misc fmd file open.
 *
 * Parameter:
 * @name: device name.
 *
 * Return:
 *    @handle: open handle.
 */
u32 md_fmd_open(char *name);

/**
 * md_fmd_info_get - misc fmd file info get.
 *
 * Parameter:
 * @handle: handle.
 * @addr: misc partition address.
 * @size: misc partition size.
 * @sector_size: misc size.
 *
 * Return:
 */
void md_fmd_info_get(u32 handle, u32 *addr, u32 *size, u32 *sector_size);

/**
 * md_fmd_read - misc fmd file read.
 *
 * Parameter:
 * @handle: handle.
 * @buf: to store read data.
 * @size: read size.
 * @offset: read offset.
 *
 * Return:
 *    @u32 return_len: read size.
 *    @buf: read buf
 */
u32 md_fmd_read(u32 handle, void *buf, u32 size, u32 offset);

/**
 * md_fmd_write - misc fmd file write.
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
void md_fmd_write(u32 handle, void *buf, u32 size, u32 offset);

/**
 * md_fmd_close - misc fmd  file close.
 *
 * Parameter:
 * @handle: file handle.
 *
 * Return:
 *
 */
void md_fmd_close(u32 *handle);

#ifdef WIN32
/**
 * md_set_file_path - set misc file path.
 *
 * Parameter:
 * @download_bin_path: download bin path.
 * @download_bin_name: download bin name.
 * @output_bin_path: output bin path.
 *
 * Return:
 *
 */
void md_set_file_path(char *download_bin_path, char *download_bin_name,
                      char *output_bin_path);
#endif

/**---------------------------------------------------------------------------*/
#endif /* __AIC_MISC_FILE_H__ */

