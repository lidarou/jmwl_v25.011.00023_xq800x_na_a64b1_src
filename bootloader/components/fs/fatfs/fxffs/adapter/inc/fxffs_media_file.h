/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system edia file struct.
 */

#ifndef __AIC_FXFFS_MEDIA_FILE_H__
#define __AIC_FXFFS_MEDIA_FILE_H__

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
typedef struct fxffs_media_file_ops {
    u32 (*alloc)(u32 handle, ulong size);
    u32 (*attri_read)(void *media, char *file_name, u32 *attri);
    u32 (*attri_set)(void *media, char *file_name, u32 attri);
    u32 (*best_effort_alloc)(u32 handle, ulong size, ulong *alloc_size);
    u32 (*close)(u32 handle);
    u32 (*create)(void *media, char *file_name);
    u32 (*time_set)(void *media, char *file_name, void *time);
    u32 (*delete)(void *media, char *file_name);
    u32 (*open)(void *media, u32 *handle, char *file_name, u32 open_type);
    u32 (*read)(u32 handle, void *buf, ulong size, ulong *rsize);
    u32 (*relative_seek)(u32 handle, ulong offset, u32 seek);
    u32 (*rename)(void *media, char *old_name, char *new_name);
    u32 (*seek)(u32 handle, ulong offset);
    void (*offset_get)(u32 handle, ulong *offset);
    u32 (*truncate)(u32 handle, ulong size);
    u32 (*truncate_release)(u32 handle, ulong size);
    u32 (*write)(u32 handle, void *buf, ulong size);
    u32 (*write_notify_set)(u32 handle);
    u32 (*extended_allocate)(u32 handle, u64 size);
    u32 (*extended_best_effort_alloc)(u32 handle, u64 size, u64 *alloc_size);
    u32 (*extended_relative_seek)(u32 handle, u64 offset, u32 seek);
    u32 (*extended_seek)(u32 handle, u64 offset);
    u32 (*extended_truncate)(u32 handle, u64 size);
    u32 (*extended_truncate_release)(u32 handle, u64 size);
} FXFFS_MEDIA_FILE_OPS;

extern FXFFS_MEDIA_FILE_OPS s_fxffs_media_file_ops;

/**---------------------------------------------------------------------------*/
#endif /* __AIC_FXFFS_MEDIA_FILE_H__ */

