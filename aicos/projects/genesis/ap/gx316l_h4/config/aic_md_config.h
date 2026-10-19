
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines misc config parameters.
 */

#ifndef __MISC_CONFIG_H__
#define __MISC_CONFIG_H__

#include <aic_md_id.h>

#define MD_BLOCK_SIZE               0x2000

#define MD_DOWNLOAD_DEVICE_NAME     "misc"
#define MD_PROTECT_DEVICE_NAME      "misc_protect"
#define MD_RUN_DEVICE_NAME          "misc_run"
#define MD_USER_DEVICE_NAME         "misc_user"

/**
 * whitelist misc id: if defined and this id is calibration item,
 * this id will be support write to calibration partition in normal mode.
 */
u16 md_whitelist_id[] = {
    MD_ID_SN0,
    MD_ID_SN1,
    MD_ID_MAC
};

#endif /* __MISC_CONFIG_H__ */

