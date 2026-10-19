/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * DESCRIPTION
 *
 * This file export the file system initialization interface.
 */

#ifndef __AIC_VFS_INIT_H__
#define __AIC_VFS_INIT_H__

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
void vfs_init(void);

#ifdef WIN32
int fs_dev_win32_init(void);
int fs_dev_ram_init(void);
int fxffs_init(void);
int romfs_init(void);
int fs_dev_file_init(void);
#endif

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __AIC_VFS_INIT_H__ */

