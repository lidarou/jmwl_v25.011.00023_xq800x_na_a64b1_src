/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * DESCRIPTION
 *
 * This file export the file system os interface.
 */

#ifndef __AIC_VFS_OS_H__
#define __AIC_VFS_OS_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
void *vfs_create_mutex(const char *name);
int vfs_get_mutex(void *mutex_ptr, u32 wait_timeout);
void vfs_put_mutex(void *mutex_ptr);
void vfs_delete_mutex(void *mutex_ptr);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_VFS_OS_H__ */

