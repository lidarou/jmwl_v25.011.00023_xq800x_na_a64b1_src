/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system media struct.
 */

#ifndef __AIC_FXFFS_MEDIA_H__
#define __AIC_FXFFS_MEDIA_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 ** Public Data
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Public Types
 **---------------------------------------------------------------------------*/
/* Define media diagnostic constants.  */
typedef enum {
    FXFFS_FAT_CHAIN_ERROR =    BIT_0,
    FXFFS_DIRECTORY_ERROR =    BIT_1,
    FXFFS_LOST_CLUSTER_ERROR = BIT_2,
    FXFFS_FILE_SIZE_ERROR =    BIT_3,
} FXFFS_MEDIA_ERROR;

typedef struct fxffs_media_ops {
    u32 (*abort)(void *media);
    u32 (*cache_invalidate)(void *media);
    u32 (*check)(void *media, u8 *mem_ptr, ulong mem_size,
                 ulong correction_option, ulong *error);
    u32 (*close)(void *media);
    u32 (*flush)(void *media);
    u32 (*format)(void *media);
    u32 (*open)(void *media);
    u32 (*read)(void *media, ulong logical_sector, void *buf);
    u32 (*space_available)(void *media, ulong *available_bytes,
                           ulong *total_bytes);
    u32 (*volume_get)(void *media, char *volume_name, u32 volume_source);
    u32 (*volume_get_extended)(void *media, char *volume_name,
                               u32 volume_name_len, u32 volume_source);
    u32 (*volume_set)(void *media, char *volume_name);
    u32 (*write)(void *media, ulong logical_sector, void *buf);
    u32 (*extended_space_available)(void *media, u64 *available_bytes,
                                    u64 *total_bytes,
                                    u32 *sectors_per_cluster,
                                    u32 *bytes_per_sector);
    u32 (*fault_tolerant_enable)(void *media);
} FXFFS_MEDIA_OPS;

extern FXFFS_MEDIA_OPS s_fxffs_media_ops;

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_FXFFS_MEDIA_H__ */

