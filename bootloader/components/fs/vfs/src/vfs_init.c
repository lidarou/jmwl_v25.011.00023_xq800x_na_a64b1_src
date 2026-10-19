/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system initialization interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16bdcb

#include <aic_type.h>
#include "vfs.h"
#include "fs_api.h"
#include "initcall.h"
#include "ftlblock.h"

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
boolean s_vfs_init = false;

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * Name: fs_init
 *
 * Description:
 *   This is called from the OS initialization logic to configure the file
 *   system.
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 */
int vfs_init(void)
{
    int ret;

    if (s_vfs_init)
        return AIC_SUCCESS;

    do {
        ret = ftl_init();
        if (ret != AIC_SUCCESS) {
            log_print("[FS] ftl init fail, ret=%d.\n", ret);
            break;
        }

        vfs_fdtable_init();

        vfs_inode_init();

        ret = fxffs_init();
        if (ret != AIC_SUCCESS) {
            log_print("[FS] fxffs_init fail, ret=%d.\n", ret);
            break;
        }

        ret = fs_dev_ftl_init();
        if (ret != AIC_SUCCESS) {
            log_print("[FS] ftl device init fail, ret=%d.\n", ret);
            break;
        }

        ret = vfs_config();
        if (ret != AIC_SUCCESS) {
            log_print("[FS] vfs_config fail, ret=%d.\n", ret);
            break;
        }

        s_vfs_init = true;
    } while(0);

    return ret;
}

