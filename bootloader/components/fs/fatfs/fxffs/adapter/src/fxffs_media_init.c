/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the filex fat file system init Interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0d1e41

#include <aic_errno.h>
#include <fx_api.h>
#include "fxffs.h"

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fxffs_media_system_init -
 *  This function initializes the various control data structures for
 *  the ffs component.
 *
 * Parameter:
 *
 * Return:
 *
 */
void fxffs_media_system_init(void);

FXFFS_MEDIA_INIT_OPS s_fxffs_media_init_ops = {
    false,
    fxffs_media_system_init,
    NULL,
    NULL,
};

/**
 * fxffs_media_system_init -
 *  This function initializes the various control data structures for
 *  the ffs component.
 *
 * Parameter:
 *
 * Return:
 *
 */
void fxffs_media_system_init(void)
{
    if (!s_fxffs_media_init_ops.initd) {
        fx_system_initialize();
        s_fxffs_media_init_ops.initd = true;
    }
}

