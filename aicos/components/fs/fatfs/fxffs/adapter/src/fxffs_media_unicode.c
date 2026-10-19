/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system unicode operation.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ed508
#define LOG_MODULE    AMOD_FS

#include "fxffs.h"

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fxffs_unicode_dir_create:
 *  This function creates the specified unicode directory.
 *
 * Parameter:
 * @media: fs media.
 * @unicode_name: unicode directory name.
 * @unicode_len: unicode directory length.
 * @short_name: short name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_dir_create(void *media, u8 *unicode_name, ulong unicode_len,
                           char *short_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_unicode_directory_create(fx_media, unicode_name, unicode_len,
                                       short_name);
}

/**
 * fxffs_unicode_dir_rename -
 *    This function creates the specified unicode directory.
 *    This function first attempts to find the specified directory.
 *    If found, the unicode rename request is valid and the directory
 *    will be changed to the new unicode name. Otherwise, if the directory
 *    is not found, the appropriate error code is returned to the caller.
 *
 * Parameter:
 * @media: fs media.
 * @old_unicode_name: old unicode directory name.
 * @old_unicode_len: old unicode directory length.
 * @new_unicode_name: new unicode directory name.
 * @new_unicode_len: new unicode directory length.
 * @new_short_name: new short name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_dir_rename(void *media,
                           u8 *old_unicode_name, ulong old_unicode_len,
                           u8 *new_unicode_name, ulong new_unicode_len,
                           char *new_short_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_unicode_directory_rename(fx_media,
                                      old_unicode_name, old_unicode_len,
                                      new_unicode_name, new_unicode_len,
                                      new_short_name);
}

/**
 * fxffs_unicode_file_create -
 *    This function creates the specified unicode file name.
 *
 * Parameter:
 * @media: fs media.
 * @unicode_name: unicode file name.
 * @unicode_len: unicode file length.
 * @short_name: short name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_file_create(void *media, u8 *unicode_name, ulong unicode_len,
                            char *short_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_unicode_file_create(fx_media,
                                  unicode_name, unicode_len,
                                  short_name);
}

/**
 * fxffs_unicode_file_rename -
 *  This function first attempts to find the specified file. If found,
 *  the rename request is valid and the directory entry will be changed
 *  to the new file name.  Otherwise, if the file is not found, the
 *  appropriate error code is returned to the caller.
 *
 * Parameter:
 * @media: fs media.
 * @old_unicode_name: old unicode file name.
 * @old_unicode_len: old unicode file length.
 * @new_unicode_name: new unicode file name.
 * @new_unicode_len: new unicode file length.
 * @new_short_name: new short name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_file_rename(void *media,
                            u8 *old_unicode_name, ulong old_unicode_len,
                            u8 *new_unicode_name, ulong new_unicode_len,
                            char *new_short_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_unicode_file_rename(fx_media,
                                 old_unicode_name, old_unicode_len,
                                 new_unicode_name, new_unicode_len,
                                 new_short_name);
}

/**
 * fxffs_unicode_len_get -
 *  This function returns the length of the supplied unicode name.
 *  Note, this API is deprecated as fxffs_unicode_len_get_ext
 *  should be used. The maximum buffer size of unicode_name is 256.
 *
 * Parameter:
 * @unicode_name: unicode file name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_len_get(u8 *unicode_name)
{
    return fx_unicode_length_get(unicode_name);
}

/**
 * fxffs_unicode_len_get_ext -
 *  This function returns the length of the supplied unicode name.
 *
 * Parameter:
 * @unicode_name: unicode file name.
 * @len: buffer length for unicode name
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_len_get_ext(u8 *unicode_name, u32 buf_len)
{
    return fx_unicode_length_get_extended(unicode_name, buf_len);
}

/**
 * fxffs_unicode_name_get -
 *    Note, this API is deprecated as fxffs_unicode_name_get_ext should
 *    be used. The maximum written size to destination_unicode_name could
 *    be FX_MAX_LONG_NAME_LEN * 2.
 *
 * Parameter:
 * @media: fs media.
 * @short_name: short name.
 * @unicode_name: destination file name.
 * @unicode_len: destination for name length.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_name_get(void *media, char *short_name,
                         u8 *unicode_name, ulong *unicode_len)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_unicode_name_get(fx_media, short_name,
                               unicode_name, unicode_len);
}

/**
 * fxffs_unicode_name_get_ext -
 *  This function returns the length of the supplied unicode name.
 *
 * Parameter:
 * @media: fs media.
 * @short_name: short file name.
 * @unicode_name: destination file name.
 * @unicode_len: destination for name length.
 * @buf_len: buffer length of unicode name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_name_get_ext(void *media, char *short_name,
                             u8 *unicode_name, ulong *unicode_len, ulong buf_len)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_unicode_name_get_extended(fx_media, short_name,
                                        unicode_name, unicode_len, buf_len);
}

/**
 * fxffs_unicode_short_name_get -
 *    Note, this API is deprecated as fxffs_unicode_name_get_ext should
 *    be used. The maximum written size to destination_unicode_name could
 *    be FX_MAX_LONG_NAME_LEN * 2.
 *
 * Parameter:
 * @media: fs media.
 * @short_name: short name.
 * @unicode_name: destination file name.
 * @unicode_len: destination for name length.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_short_name_get(void *media,
                               u8 *unicode_name, ulong unicode_len,
                               char *short_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_unicode_short_name_get(fx_media,
                                     unicode_name, unicode_len,
                                     short_name);
}

/**
 * fxffs_unicode_short_name_get_ext -
 *      This function returns the length of the supplied unicode name.
 *
 * Parameter:
 * @media: fs media.
 * @short_name: short file name.
 * @unicode_name: destination file name.
 * @unicode_len: destination for name length.
 * @buf_len: buffer length of unicode name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_unicode_short_name_get_ext(void *media,
                                   u8 *unicode_name, ulong unicode_len,
                                   char *short_name, ulong buf_len)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_unicode_short_name_get_extended(fx_media,
                                              unicode_name, unicode_len,
                                              short_name, buf_len);
}

FXFFS_MEDIA_UNICODE_OPS s_fxffs_media_unicode_ops = {
    fxffs_unicode_dir_create,
    fxffs_unicode_dir_rename,
    fxffs_unicode_file_create,
    fxffs_unicode_file_rename,
    fxffs_unicode_len_get,
    fxffs_unicode_len_get_ext,
    fxffs_unicode_name_get,
    fxffs_unicode_name_get_ext,
    fxffs_unicode_short_name_get,
    fxffs_unicode_short_name_get_ext,
};

