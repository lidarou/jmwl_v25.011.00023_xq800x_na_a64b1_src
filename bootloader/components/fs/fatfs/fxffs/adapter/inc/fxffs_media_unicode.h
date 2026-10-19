/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system media unicode struct.
 */

#ifndef __AIC_FXFFS_MEDIA_UNICODE_H__
#define __AIC_FXFFS_MEDIA_UNICODE_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 ** Public Data
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Public Types
 **---------------------------------------------------------------------------*/
typedef struct fxffs_media_unicode_ops {
    u32 (*dir_create)(void *media, u8 *unicode_name, ulong unicode_len,
                      char *short_name);
    u32 (*dir_rename)(void *media, u8 *old_unicode_name, ulong old_unicode_len,
                      u8 *new_unicode_name, ulong new_unicode_len,
                      char *new_short_name);
    u32 (*file_create)(void *media, u8 *unicode_name, ulong unicode_len,
                       char *short_name);
    u32 (*file_rename)(void *media, u8 *old_unicode_name, ulong old_unicode_len,
                       u8 *new_unicode_name, ulong new_unicode_len,
                       char *new_short_name);
    u32 (*len_get)(u8 *unicode_name);
    u32 (*len_get_ext)(u8 *unicode_name, u32 buf_len);
    u32 (*name_get)(void *media, char *short_name, u8 *unicode_name,
                    ulong *unicode_len);
    u32 (*name_get_ext)(void *media, char *short_name, u8 *unicode_name,
                        ulong *unicode_len, ulong buf_len);
    u32 (*short_name_get)(void *media, u8 *unicode_name, ulong unicode_len,
                          char *short_name);
    u32 (*short_name_get_ext)(void *media, u8 *unicode_name, ulong unicode_len,
                              char *short_name, ulong buf_len);
} FXFFS_MEDIA_UNICODE_OPS;

extern FXFFS_MEDIA_UNICODE_OPS s_fxffs_media_unicode_ops;

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_FXFFS_MEDIA_UNICODE_H__ */

