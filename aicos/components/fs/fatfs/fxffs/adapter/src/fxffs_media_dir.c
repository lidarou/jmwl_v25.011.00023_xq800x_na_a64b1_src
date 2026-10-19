/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic filex fat file system Directory Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1a136d
#define LOG_MODULE    AMOD_FS

#include "fxffs.h"

/**---------------------------------------------------------------------------*
 ** Private Types
 **---------------------------------------------------------------------------*/
typedef struct _fxffs_local_path {
    char local_full_path[FX_MAXIMUM_PATH];
    FX_LOCAL_PATH fx_local_path;
} FXFFS_LOCAL_PATH;

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fxffs_dir_attri_read - read fxffs directory attributes.
 *
 * Parameter:
 * @meida: file system media.
 * @dir_name: directory name.
 * @attri: directory attributes.
 *              FS_READ_ONLY(BIT_1)
 *              FS_HIDDEN(BIT_2)
 *              FS_SYSTEM(BIT_3)
 *              FS_VOLUME(BIT_4)
 *              FS_DIRECTORY(BIT_5)
 *              FS_ARCHIVE(BIT_6)
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_attri_read(void *media, u8 *dir_name, u32 *attri)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_attributes_read(fx_media, dir_name, attri);
}

/**
 * fxffs_dir_attri_set - set fxffs directory attributes.
 *
 * Parameter:
 * @media: file system media.
 * @dir_name: directory name.
 * @attri: directory attributes.
 *              FS_DIR_READ_ONLY(BIT_1)
 *              FS_DIR_HIDDEN(BIT_2)
 *              FS_DIR_SYSTEM(BIT_3)
 *              FS_DIR_ARCHIVE(BIT_6)
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_attri_set(void *media, char *dir_name, u32 attri)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_attributes_set(fx_media, dir_name, attri);
}

/**
 * fxffs_dir_create - create fxffs directory.
 *
 * Parameter:
 * @media: file system media.
 * @dir_name: directory name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_create(void *media, char *dir_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_create(fx_media, dir_name);
}

/**
 * fxffs_dir_default_get -
 *      return the last path provided to the fxffs_dir_def_set function.
 *
 * Parameter:
 * @media: file system media.
 * @ret_path: return path name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_default_get(void *media, char **ret_path)
{
#if 0
    FS_INFO *fs_info = (FS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_default_get(fx_media, ret_path);
#else
    return FX_NOT_IMPLEMENTED;
#endif
}

/**
 * fxffs_dir_default_set - set default path.
 *
 * Parameter:
 * @media: file system media.
 * @new_path: new path name.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_default_set(void *media, char *new_path)
{
#if 0
    FS_INFO *fs_info = (FS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_default_set(fx_media, new_path);
#else
        return FX_NOT_IMPLEMENTED;
#endif
}

/**
 * fxffs_dir_default_get_copy -
 *    This function copies the last path provided to the
 *    fx_dir_def_set function into the specified buffer.
 *
 * Parameter:
 * @media: file system media.
 * @ret_path_name_buf: Destination buffer for name
 * @ret_path_name_buf_size: Size of buffer pointed to by return_path_name_buffer
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_default_get_copy(void *media, char *ret_path_name_buf,
                               u32 ret_path_name_buf_size)
{
#if 0
    FS_INFO *fs_info = (FS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_default_get_copy(fx_media, ret_path_name_buf,
                                         ret_path_name_buf_size);
#else
    return FX_NOT_IMPLEMENTED;
#endif
}

/**
 * fxffs_dir_delete - delete directory, and directory must be null.
 *
 * Parameter:
 * @media: file system media.
 * @dir: directory.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_delete(void *media, char *dir)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_delete(fx_media, dir);
}

/**
 * fxffs_dir_first_entry_find -
 *      This function returns the first directory entry of the current
 *      working directory.
 *
 * Parameter:
 * @media: file system media.
 * @dir: first directory entry.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_first_entry_find(void *media, char *dir)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_first_entry_find(fx_media, dir);
}

/**
 * fxffs_dir_first_full_entry_find -
 *      This function returns the first directory entry of the current
 *      working directory and selected information about the entry.
 *
 * Parameter:
 * @media: file system media.
 * @dir: directory.
 * @attri: directory attribute.
 *              FS_READ_ONLY(BIT_1)
 *              FS_HIDDEN(BIT_2)
 *              FS_SYSTEM(BIT_3)
 *              FS_VOLUME(BIT_4)
 *              FS_DIRECTORY(BIT_5)
 *              FS_ARCHIVE(BIT_6)
 * @size: first directory entry size.
 * @time: first directory entry time and date.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_first_full_entry_find(void *media, char *dir, u32 *attri,
                                  ulong *size, void *time)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    aos_systime *systime = (aos_systime *)time;

    return fx_directory_first_full_entry_find(fx_media, dir, attri, size,
                                              &systime->year, &systime->month,
                                              &systime->day, &systime->hour,
                                              &systime->minute,
                                              &systime->second);
}

/**
 * fxffs_dir_first_full_entry_find -
 *      This function returns the first directory entry of the current
 *      working directory and selected information about the entry.
 *
 * Parameter:
 * @media: file system media.
 * @dir: directory.
 * @attri: directory attribute.
 *              FS_READ_ONLY(BIT_1)
 *              FS_HIDDEN(BIT_2)
 *              FS_SYSTEM(BIT_3)
 *              FS_VOLUME(BIT_4)
 *              FS_DIRECTORY(BIT_5)
 *              FS_ARCHIVE(BIT_6)
 * @size: directory size.
 * @time: directory time and date.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_info_get(void *media, char *dir, u32 *attri, ulong *size,
                     void *time)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    aos_systime *systime = (aos_systime *)time;

    return fx_directory_information_get(fx_media, dir, attri, size,
                                        &systime->year, &systime->month,
                                        &systime->day, &systime->hour,
                                        &systime->minute, &systime->second);
}

/**
 * fxffs_dir_local_path_clear -
 *      clears the local default directory, so any path relative operations
 *      will start using the global default path again.
 *
 * Parameter:
 * @media: file system media.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_local_path_clear(void *media)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    FXFFS_LOCAL_PATH *fxffs_local_path;
    FX_LOCAL_PATH *fx_local_path;
    char *local_path;
    u32 ret;

    ret = fx_directory_local_path_get(fx_media, &local_path);
    if (ret == FX_SUCCESS) {
        ret = fx_directory_local_path_clear(fx_media);
        if (local_path && ret == FX_SUCCESS) {
            fx_local_path = (FX_LOCAL_PATH *)(local_path -
                                              offsetof(FX_LOCAL_PATH,
                                                       fx_path_string));
            fxffs_local_path = (FXFFS_LOCAL_PATH *)((u8 *)fx_local_path -
                                                offsetof(FXFFS_LOCAL_PATH,
                                                         fx_local_path));
            aos_free(fxffs_local_path);
        }
    }

    return ret;
}

/**
 * fxffs_dir_local_path_get -
 *      This function returns a pointer to the local default directory for
 *      this thread.
 *
 * Parameter:
 * @media: file system media.
 * @ret_local_full_path: return local full path.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_local_path_get(void *media, char **ret_local_full_path)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    FXFFS_LOCAL_PATH *fxffs_local_path;
    FX_LOCAL_PATH *fx_local_path;
    u32 ret;

    ret = fx_directory_local_path_get(fx_media, ret_local_full_path);
    if (ret == FX_SUCCESS && *ret_local_full_path) {
        fx_local_path = (FX_LOCAL_PATH *)(*ret_local_full_path -
                                          offsetof(FX_LOCAL_PATH,
                                                   fx_path_string));
        fxffs_local_path = (FXFFS_LOCAL_PATH *)((u8 *)fx_local_path -
                                                offsetof(FXFFS_LOCAL_PATH,
                                                fx_local_path));
        *ret_local_full_path = fxffs_local_path->local_full_path;
    }

    return ret;
}

/**
 * fxffs_dir_local_path_restore -
 *    This function restores the local default directory to
 *    the previous path specified by the caller.
 *
 * Parameter:
 * @media: file system media.
 * @local_full_path: previous local full path.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_local_path_restore(void *media, char* local_full_path)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    FXFFS_LOCAL_PATH *fxffs_local_path;

    fxffs_local_path = (FXFFS_LOCAL_PATH *)(local_full_path -
                                            offsetof(FXFFS_LOCAL_PATH,
                                            local_full_path));

    return fx_directory_local_path_restore(fx_media,
                                           &fxffs_local_path->fx_local_path);
}

/**
 * fxffs_dir_local_path_get_copy -
 *    This function copies the local default directory for this thread
 *    into the specified buffer.
 *
 * Parameter:
 * @media: file system media.
 * @ret_path_name_buf: Destination buffer for name
 * @ret_path_name_buf_size: Size of buffer pointed to by return_path_name_buffer
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_local_path_get_copy(void *media, char *ret_path_name_buf,
                                u32 ret_path_name_buf_size)
{
#if 0
    FS_INFO *fs_info = (FS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_local_path_get_copy(fx_media, ret_path_name_buf,
                                            ret_path_name_buf_size);
#else
    return FX_NOT_IMPLEMENTED;
#endif
}

/**
 * fxffs_dir_local_path_set -
 *  This function sets the local default directory to the path specified by
 *  the caller. If this path is not found, an error code is returned.
 *
 * Parameter:
 * @media: file system media.
 * @local_path: local path handle.
 * @new_full_path: new full path.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_local_path_set(void *media, u32 *local_path_handle,
                           char *new_full_path)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    FXFFS_LOCAL_PATH *ffx_local_path = (FXFFS_LOCAL_PATH *)*local_path_handle;
    u32 ret;

    if (!ffx_local_path)
        ffx_local_path = aos_zalloc(sizeof(FXFFS_LOCAL_PATH));

    aos_memcpy(ffx_local_path->local_full_path, new_full_path,
               strlen(new_full_path));
    ret = fx_directory_local_path_set(fx_media, &ffx_local_path->fx_local_path,
                                      new_full_path);
    if ((ret != FX_SUCCESS) && (*local_path_handle == 0)) {
        aos_free(ffx_local_path);
        ffx_local_path = NULL;
    }

    *local_path_handle = (u32)ffx_local_path;

    return ret;
}

/**
 * fxffs_dir_long_name_get -
 *    This function gets the long file name via the supplied short file
 *    name. If there is no long file name, the short file name will be
 *    returned.
 *
 * Parameter:
 * @media: file system media.
 * @short_name: short name
 * @long_name: long name
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_long_name_get(void *media, char *short_name, char *long_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_long_name_get(fx_media, short_name, long_name);
}

/**
 * fxffs_dir_long_name_get_extended -
 *    This function gets the long file name via the supplied short file
 *    name. If there is no long file name, the short file name will be
 *    returned.
 *
 * Parameter:
 * @media: file system media.
 * @short_name: short name
 * @long_name: long name
 * @long_name_buf_len: buffer length for long name
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_long_name_get_extended(void *media, char *short_name,
                                   char *long_name, u32 long_name_buf_len)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_long_name_get_extended(fx_media, short_name, long_name,
                                               long_name_buf_len);
}

/**
 * fxffs_dir_name_test -
 *    This function first attempts to find the specified directory name.
 *    If found, the attributes are checked to see if the entry is a
 *    directory. If not, the appropriate error code is returned to the
 *    caller.
 *
 * Parameter:
 * @media: file system media.
 * @dir_name: directory name
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_name_test(void *media, char *dir_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_name_test(fx_media, dir_name);
}

/**
 * fxffs_dir_next_entry_find -
 *    This function returns the name of the next entry in the current
 *    working directory.  The function that returns the first name in the
 *    current directory must be called prior to this function.
 *
 * Parameter:
 * @media: file system media.
 * @dir_name: directory name
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_next_entry_find(void *media, char *dir_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_next_entry_find(fx_media, dir_name);
}

/**
 * fxffs_dir_next_full_entry_find -
 *    This function returns the name of the next entry in the current
 *    working directory and various information about the name.  The
 *    function that returns the first name in the current directory must
 *    be called prior to this function.
 *
 * Parameter:
 * @media: file system media.
 * @dir: directory.
 * @attri: directory attribute.
 *              FS_READ_ONLY(BIT_1)
 *              FS_HIDDEN(BIT_2)
 *              FS_SYSTEM(BIT_3)
 *              FS_VOLUME(BIT_4)
 *              FS_DIRECTORY(BIT_5)
 *              FS_ARCHIVE(BIT_6)
 * @size: next directory entry size.
 * @time: next directory entry time and date.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_next_full_entry_find(void *media, char *dir, u32 *attri,
                                   ulong *size, void *time)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;
    aos_systime *systime = (aos_systime *)time;

    return fx_directory_next_full_entry_find(fx_media, dir, attri, size,
                                             &systime->year, &systime->month,
                                             &systime->day, &systime->hour,
                                             &systime->minute, &systime->second);
}

/**
 * fxffs_dir_rename -
 *    This function first attempts to find the specified directory.
 *    If found, the rename request is valid and the directory will be
 *    changed to the new name.  Otherwise, if the directory is not found,
 *    the appropriate error code is returned to the caller.
 *
 * Parameter:
 * @media: file system media.
 * @old_name: old directory path.
 * @new_name: new directory path.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_rename(void *media, char *old_name, char *new_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_rename(fx_media, old_name, new_name);
}

/**
 * fxffs_dir_short_name_get -
 *    This function gets the short file name via the supplied long file
 *    name. If the long file name is really a short file name, the short
 *    file name will be returned.
 *
 *    Note, this API is deprecated as fx_directory_short_name_get_extended
 *    should be used. The maximum written size to short_file_name could
 *    be FX_MAX_SHORT_NAME_LEN.
 *
 * Parameter:
 * @media: file system media.
 * @long_name: long directory path.
 * @short_name: short directory path.
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_short_name_get(void *media, char *long_name, char *short_name)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_short_name_get(fx_media, long_name, short_name);
}

/**
 * fxffs_dir_short_name_get_extended -
 *    This function gets the short file name via the supplied long file
 *    name. If the long file name is really a short file name, the short
 *    file name will be returned.
 *
 * Parameter:
 * @media: file system media.
 * @long_name: long directory path.
 * @short_name: short directory path.
 * @short_name_len: Buffer length for short name
 *
 * Return:
 *    FX_SUCCESS is success, other fail.
 */
u32 fxffs_dir_short_name_get_extended(void *media, char *long_name,
                                    char *short_name, u32 short_name_len)
{
    FXFFS_INFO *fs_info = (FXFFS_INFO *)media;
    FX_MEDIA *fx_media = (FX_MEDIA *)fs_info->media_handle;

    return fx_directory_short_name_get_extended(fx_media, long_name,
                                                short_name, short_name_len);
}

FXFFS_MEDIA_DIR_OPS s_fxffs_media_dir_ops = {
    fxffs_dir_attri_read,
    fxffs_dir_attri_set,
    fxffs_dir_create,
    fxffs_dir_default_get,
    fxffs_dir_default_set,
    fxffs_dir_default_get_copy,
    fxffs_dir_delete,
    fxffs_dir_first_entry_find,
    fxffs_dir_first_full_entry_find,
    fxffs_dir_info_get,
    fxffs_dir_local_path_clear,
    fxffs_dir_local_path_get,
    fxffs_dir_local_path_restore,
    fxffs_dir_local_path_get_copy,
    fxffs_dir_local_path_set,
    fxffs_dir_long_name_get,
    fxffs_dir_long_name_get_extended,
    fxffs_dir_name_test,
    fxffs_dir_next_entry_find,
    fxffs_dir_next_full_entry_find,
    fxffs_dir_rename,
    fxffs_dir_short_name_get,
    fxffs_dir_short_name_get_extended,
};

