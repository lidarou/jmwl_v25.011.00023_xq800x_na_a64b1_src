/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system media directory struct.
 */

#ifndef __AIC_FXFFS_MEDIA_DIR_H__
#define __AIC_FXFFS_MEDIA_DIR_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/
typedef u32 (*FXFFS_FIND_FULL_ENTRY)(void *media, char *dir, u32 *attri,
                                     ulong *size, void *time);

/**---------------------------------------------------------------------------*
 ** Public Data
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Public Types
 **---------------------------------------------------------------------------*/
typedef struct fxffs_media_dir_ops {
    u32 (*attri_read)(void *media, u8 *dir_name, u32 *attri);
    u32 (*attri_set)(void *media, char *dir_name, u32 attri);
    u32 (*create)(void *media, char *dir_name);
    u32 (*default_get)(void *media, char **ret_path);
    u32 (*default_set)(void *media, char *new_path);
    u32 (*default_get_copy)(void *media, char *ret_path_name_buf,
                            u32 ret_path_name_buf_size);
    u32 (*delete)(void *media, char *dir);
    u32 (*first_entry_find)(void *media, char *dir);
    u32 (*first_full_entry_find)(void *media, char *dir, u32 *attri,
                                 ulong *size, void *time);
    u32 (*info_get)(void *media, char *dir, u32 *attri, ulong *size,
                    void *time);
    u32 (*local_path_clear)(void *media);
    u32 (*local_path_get)(void *media, char **ret_local_path);
    u32 (*local_path_restore)(void *media, char *local_path);
    u32 (*local_path_get_copy)(void *media, char *ret_path_name_buf,
                               u32 ret_path_name_buf_size);
    u32 (*local_path_set)(void *media, u32 *local_path_handle, char *new_path);
    u32 (*long_name_get)(void *media, char *short_name, char *long_name);
    u32 (*long_name_get_extended)(void *media, char *short_name,
                                  char *long_name, u32 long_name_buf_len);
    u32 (*name_test)(void *media, char *dir_name);
    u32 (*next_entry_find)(void *media, char *dir_name);
    u32 (*next_full_entry_find)(void *media, char *dir, u32 *attri,
                                ulong *size, void *time);
    u32 (*rename)(void *media, char *old_name, char *new_name);
    u32 (*short_name_get)(void *media, char *long_name, char *short_name);
    u32 (*short_name_get_extended)(void *media, char *long_name,
                                   char *short_name, u32 short_name_len);
} FXFFS_MEDIA_DIR_OPS;

extern FXFFS_MEDIA_DIR_OPS s_fxffs_media_dir_ops;

/**---------------------------------------------------------------------------*/
#endif /* __AIC_FXFFS_MEDIA_DIR_H__ */

