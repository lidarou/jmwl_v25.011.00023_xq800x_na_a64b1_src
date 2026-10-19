/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file export the filex fat file system media driver Interface.
 */

#ifndef __FXFFS_MEDIA_DRIVER_H__
#define __FXFFS_MEDIA_DRIVER_H__

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
#include <fx_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/**---------------------------------------------------------------------------*
 ** Public Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * fxffs_media_driver:
 *  This function is the entry point to the generic media driver.
 *
 * Parameter:
 * @media_ptr: fx media ptr
 *
 * Return:
 *
 */
void fxffs_media_driver(FX_MEDIA *media_ptr);

/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __FXFFS_MEDIA_DRIVER_H__ */

