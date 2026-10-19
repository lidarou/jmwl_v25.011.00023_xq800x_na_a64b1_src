/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic filex fat file system file Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b9f8c

#include "fxffs.h"

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fxffs_file_alloc -
 *  This function attempts to allocate the number of consecutive
 *  clusters required to satisfy the user's request.  If there are
 *  enough clusters, the clusters are allocated and linked to the file.
 *  Otherwise, if there are not enough consecutive clusters, an error
 *  code is returned to the caller.
 *
 * Parameter:
 * @handle: file handle
 * @size: Number of bytes to allocate
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_alloc(u32 handle, ulong size)
{
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
    FX_FILE *fx_file = (FX_FILE *)handle;
    u32 ret;

    ret = fx_file_allocate(fx_file, size);

    return ret;
#else
    return FX_INVALID_OPTION;
#endif
}

/**
 * fxffs_file_attri_read -
 *  This function first attempts to find the specified file.  If found,
 *  the attribute read request is valid and the directory entry will be
 *  in order to pickup the file's attributes.  Otherwise, if the file
 *  is not found, the appropriate error code is returned to the caller.
 *
 * Parameter:
 * @media: file system media.
 * @file_name: file name.
 * @attri: file attribute.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_attri_read(void *media, char *file_name, u32 *attri)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_file_attributes_read(fx_media, file_name, attri);
}

/**
 * fxffs_file_attri_set -
 *  This function first attempts to find the specified file.  If found,
 *  the attribute set request is valid and the directory entry will be
 *  modified with the new attributes.  Otherwise, if the file is not
 *  found, the appropriate error code is returned to the caller.
 *
 * Parameter:
 * @media: file system media.
 * @file_name: file name.
 * @attri: file attribute.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_attri_set(void *media, char *file_name, u32 attri)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_file_attributes_set(fx_media, file_name, attri);
}

/**
 * fxffs_file_best_effort_alloc -
 *  This function attempts to allocate the number of consecutive
 *  clusters required to satisfy the user's request.  If there are not
 *  enough clusters, the largest set of clusters are allocated and
 *  linked to the file.  If there are no free clusters, an error
 *  code is returned to the caller.
 *
 * Parameter:
 * @handle: file handle.
 * @size: file size.
 * @alloc_size: file actual allocate size.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_best_effort_alloc(u32 handle, ulong size, ulong *alloc_size)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_best_effort_allocate(fx_file, size, alloc_size);
}

/**
 * fxffs_file_close -
 *  This function closes the specified file.  If the file was written
 *  to this function will also write the directory entry (with the new
 *  size and time/date stamp) out to disk.
 *
 * Parameter:
 * @handle: file handle.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_close(u32 handle)
{
    FX_FILE *fx_file = (FX_FILE *)handle;
    u32 ret;

    ret = fx_file_close(fx_file);
    free(fx_file);

    return ret;
}

/**
 * fxffs_file_create -
 *  This function first attempts to find the specified file.  If found,
 *  the create request is invalid and an error is returned to the
 *  caller.  After the file name verification is made, a search for a
 *  free directory entry will be made.  If nothing is available, an
 *  error will be returned to the caller.  Otherwise, if all is okay, a
 *  file of 0 bytes will be created.
 *
 * Parameter:
 * @media: file system media.
 * @file_name: file name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_create(void *media, char *file_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_file_create(fx_media, file_name);
}

/**
 * fxffs_file_time_set -
 *  This function sets the specified file's date and time with the
 *  values provided.
 *
 * Parameter:
 * @media: file system media.
 * @file_name: file name.
 * @time: file date and time.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_time_set(void *media, char *file_name, void *time)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    aos_systime *systime = (aos_systime *)time;

    return fx_file_date_time_set(fx_media, file_name, systime->year,
                                 systime->month, systime->day, systime->hour,
                                 systime->minute, systime->second);
}

/**
 * fxffs_file_delete -
 *  This function first attempts to find the specified file.  If found,
 *  the delete request is valid and all of its clusters will be
 *  released and its directory entry will be marked as available.
 *  Otherwise, if the file is not found, the appropriate error code is
 *  returned to the caller.
 *
 * Parameter:
 * @media: file system media.
 * @file_name: file name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_delete(void *media, char *file_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_file_delete(fx_media, file_name);
}

/**
 * fxffs_file_open -
 *  This function first attempts to find the specified file.  If found,
 *  the open request is validated and the file is opened.  During the
 *  opening process, all of the FAT entries for this file are examined
 *  for their integrity.
 *
 * Parameter:
 * @media: file system media.
 * @file_name: file name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_open(void *media, u32 *handle, char *file_name, u32 open_type)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    u32 ret, fx_open_type;
    FX_FILE *fx_file;

    if (open_type == O_RDONLY)
        fx_open_type = FX_OPEN_FOR_READ_FAST;
    else
        fx_open_type = FX_OPEN_FOR_WRITE;

    fx_file = malloc(sizeof(FX_FILE));
    ASSERT_INFO(fx_file, "malloc size(0x%x) memory fail.\n", sizeof(FX_FILE));
    memset(fx_file, 0x0, sizeof(FX_FILE));

    ret = fx_file_open(fx_media, fx_file, file_name, fx_open_type);
    if (ret != FX_SUCCESS) {
        free(fx_file);
        return ret;
    }

    *handle = (u32)fx_file;

    return ret;
}

/**
 * fxffs_file_read -
 *  This function reads the specified number of bytes (or as many as
 *  possible into the buffer supplied by the caller.  The actual number
 *  of bytes and the status of the read operation is returned to the
 *  caller.  In addition, various internal file pointers in the file
 *  control block are also updated.
 *
 * Parameter:
 * @handle: file handle.
 * @buf: read buffer.
 * @size: Number of bytes requested
 * @rsize: Pointer to variable for the number of bytes read.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_read(u32 handle, void *buf, ulong size, ulong *rsize)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_read(fx_file, buf, size, rsize);
}

/**
 * fxffs_file_relative_seek -
 *  This function positions the internal file pointers to the specified
 *  byte relative offset such that the next read or write operation is
 *  performed there.
 *
 * Parameter:
 * @handle: file handle.
 * @offset: Byte offset of the seek.
 * @seek: Direction for relative seek, legal values are:
 *          FS_SEEK_BEGIN
 *          FS_SEEK_END
 *          FS_SEEK_FORWARD
 *          FS_SEEK_BACK
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_relative_seek(u32 handle, ulong offset, u32 seek)
{
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
    FX_FILE *fx_file = (FX_FILE *)handle;
    u32 seek_from;

    switch (seek) {
        case FS_SEEK_BEGIN:
            seek_from = FX_SEEK_BEGIN;
            break;
        case FS_SEEK_CUR:
            seek_from = FX_SEEK_FORWARD;
            break;
        case FS_SEEK_END:
            seek_from = FX_SEEK_END;
            break;
        case FS_SEEK_BACK:
            seek_from = FX_SEEK_BACK;
            break;
        default:
            seek_from = FX_SEEK_FORWARD;
            break;
    }

    return fx_file_relative_seek(fx_file, offset, seek_from);
#else
    return FX_INVALID_OPTION;
#endif
}

/**
 * fxffs_file_rename -
 *  This function first attempts to find the specified file.  If found,
 *  the rename request is valid and the directory entry will be changed
 *  to the new file name.  Otherwise, if the file is not found, the
 *  appropriate error code is returned to the caller.
 *
 * Parameter:
 * @media: file system media.
 * @old_name: old file name.
 * @new_name: new file name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_rename(void *media, char *old_name, char *new_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_file_rename(fx_media, old_name, new_name);
}

/**
 * fxffs_file_seek -
 *  This function positions the internal file pointers to the specified
 *  byte offset such that the next read or write operation will be
 *  performed there.  If the byte offset is greater than the size, the
 *  file pointers will be positioned to the end of the file.
 *
 * Parameter:
 * @handle: file handle.
 * @offset: Byte offset into the file.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_seek(u32 handle, ulong offset)
{
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_seek(fx_file, offset);
#else
    return FX_INVALID_OPTION;
#endif
}

/**
 * fxffs_file_offset_get - get file current file offset.
 *
 * Parameter:
 * @handle: file handle.
 * @offset: Byte offset into the file.
 *
 * Return:
 *
 */
void fxffs_file_offset_get(u32 handle, ulong *offset)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    *offset = (ulong)fx_file->fx_file_current_file_offset;
}

/**
 * fxffs_file_truncate -
 *  This function sets the file to the specified size, if smaller than
 *  the current file size.  If the new file size is less than the
 *  current file read/write position, the internal file pointers will
 *  also be modified.
 *
 * Parameter:
 * @handle: file handle.
 * @size: new size of the file in bytes.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_truncate(u32 handle, ulong size)
{
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_truncate(fx_file, size);
#else
    return FX_INVALID_OPTION;
#endif
}

/**
 * fxffs_file_truncate_release -
 *  This function sets the file to the specified size, if smaller than
 *  the current file size.  If the new file size is less than the
 *  current file read/write position, the internal file pointers will
 *  also be modified.  Any unused clusters are released back to the
 *  media.
 *
 * Parameter:
 * @handle: file handle.
 * @size: new size of the file in bytes.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_truncate_release(u32 handle, ulong size)
{
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_truncate_release(fx_file, size);
#else
    return FX_INVALID_OPTION;
#endif
}

/**
 * fxffs_file_write_notify -
 *  Notify function called when file is written to.
 *
 * Parameter:
 * @fx_file: file control block pointer.
 *
 * Return:
 */
void fx_file_write_notify(FX_FILE * fx_file)
{
    FX_MEDIA *fx_media = (FX_MEDIA *)fx_file->fx_file_media_ptr;
    FXFFS_INFO *fs_info = (FXFFS_INFO *)fx_media->fx_media_driver_info;

    if (fs_info->fs_notify->file_write)
        fs_info->fs_notify->file_write(fs_info);
}

/**
 * fxffs_file_write_notify_set -
 *  This function sets the notify function called whenever file is
 *  is written to.
 *
 * Parameter:
 * @handle: file handle.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_write_notify_set(u32 handle)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_write_notify_set(fx_file, fx_file_write_notify);
}

/**
 * fxffs_file_write -
 *  This function writes the specified number of bytes into the file's
 *  data area.  The status of the write operation is returned to the
 *  caller.  In addition, various internal file pointers in the file
 *  control block are also updated.
 *
 * Parameter:
 * @handle: file handle.
 * @buf: buffer pointer
 * @size: number of bytes to write.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_write(u32 handle, void *buf, ulong size)
{
    FX_FILE *fx_file = (FX_FILE *)handle;
    u32 ret;

    ret = fx_file_write(fx_file, buf, size);
    if (ret == FX_SUCCESS) {
        ret = fxffs_file_write_notify_set(handle);
    }

    return ret;
}

/**
 * fxffs_file_extended_allocate -
 *  This function attempts to allocate the number of consecutive
 *  clusters required to satisfy the user's request.  If there are
 *  enough clusters, the clusters are allocated and linked to the file.
 *  Otherwise, if there are not enough consecutive clusters, an error
 *  code is returned to the caller.
 *
 * Parameter:
 * @handle: file handle.
 * @size: Number of bytes to allocate.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_extended_allocate(u32 handle, u64 size)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_extended_allocate(fx_file, size);
}

/**
 * fxffs_file_extended_best_effort_alloc -
 *  This function attempts to allocate the number of consecutive
 *  clusters required to satisfy the user's request.  If there are not
 *  enough clusters, the largest set of clusters are allocated and
 *  linked to the file.  If there are no free clusters, an error
 *  code is returned to the caller.
 *
 * Parameter:
 * @handle: file handle.
 * @size: Number of bytes to allocate.
 * @alloc_size: Number of bytes allocated.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_extended_best_effort_alloc(u32 handle, u64 size, u64 *alloc_size)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_extended_best_effort_allocate(fx_file, size, alloc_size);
}

/**
 * fxffs_file_extended_relative_seek -
 *  This function positions the internal file pointers to the specified
 *  byte relative offset such that the next read or write operation is
 *  performed there.
 *
 * Parameter:
 * @handle: file handle.
 * @offset: Byte offset of the seek.
 * @seek: Direction for relative seek, legal values are:
 *          FS_SEEK_BEGIN
 *          FS_SEEK_END
 *          FS_SEEK_FORWARD
 *          FS_SEEK_BACK
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_extended_relative_seek(u32 handle, u64 offset, u32 seek)
{
    FX_FILE *fx_file = (FX_FILE *)handle;
    u32 seek_from;

    switch (seek) {
        case FS_SEEK_BEGIN:
            seek_from = FX_SEEK_BEGIN;
            break;
        case FS_SEEK_CUR:
            seek_from = FX_SEEK_FORWARD;
            break;
        case FS_SEEK_END:
            seek_from = FX_SEEK_END;
            break;
        case FS_SEEK_BACK:
            seek_from = FX_SEEK_BACK;
            break;
        default:
            seek_from = FX_SEEK_FORWARD;
            break;
    }

    return fx_file_extended_relative_seek(fx_file, offset, seek_from);
}

/**
 * fxffs_file_extended_seek -
 *  This function positions the internal file pointers to the specified
 *  byte offset such that the next read or write operation will be
 *  performed there.  If the byte offset is greater than the size, the
 *  file pointers will be positioned to the end of the file.
 *
 * Parameter:
 * @handle: file handle.
 * @offset: Byte offset into the file.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_extended_seek(u32 handle, u64 offset)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_extended_seek(fx_file, offset);
}

/**
 * fxffs_file_extended_truncate -
 *  This function sets the file to the specified size, if smaller than
 *  the current file size.  If the new file size is less than the
 *  current file read/write position, the internal file pointers will
 *  also be modified.
 *
 * Parameter:
 * @handle: file handle.
 * @size: new size of the file in bytes.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_extended_truncate(u32 handle, u64 size)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_extended_truncate(fx_file, size);
}

/**
 * fxffs_file_extended_truncate_release -
 *  This function sets the file to the specified size, if smaller than
 *  the current file size.  If the new file size is less than the
 *  current file read/write position, the internal file pointers will
 *  also be modified.  Any unused clusters are released back to the
 *  media.
 *
 * Parameter:
 * @handle: file handle.
 * @size: new size of the file in bytes.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_file_extended_truncate_release(u32 handle, u64 size)
{
    FX_FILE *fx_file = (FX_FILE *)handle;

    return fx_file_extended_truncate_release(fx_file, size);
}

FXFFS_MEDIA_FILE_OPS s_fxffs_media_file_ops = {
    fxffs_file_alloc,
    fxffs_file_attri_read,
    fxffs_file_attri_set,
    fxffs_file_best_effort_alloc,
    fxffs_file_close,
    fxffs_file_create,
    fxffs_file_time_set,
    fxffs_file_delete,
    fxffs_file_open,
    fxffs_file_read,
    fxffs_file_relative_seek,
    fxffs_file_rename,
    fxffs_file_seek,
    fxffs_file_offset_get,
    fxffs_file_truncate,
    fxffs_file_truncate_release,
    fxffs_file_write,
    fxffs_file_write_notify_set,
    fxffs_file_extended_allocate,
    fxffs_file_extended_best_effort_alloc,
    fxffs_file_extended_relative_seek,
    fxffs_file_extended_seek,
    fxffs_file_extended_truncate,
    fxffs_file_extended_truncate_release,
};

