/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system media init struct.
 */

#ifndef __AIC_FXFFS_MEDIA_INIT_H__
#define __AIC_FXFFS_MEDIA_INIT_H__

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
typedef struct fxffs_media_init_ops {
    u32 initd;
    void (*init)(void);
    u32 (*time_set)(void *time);
    u32 (*time_get)(void *time);
} FXFFS_MEDIA_INIT_OPS;

extern FXFFS_MEDIA_INIT_OPS s_fxffs_media_init_ops;

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_FXFFS_MEDIA_INIT_H__ */

