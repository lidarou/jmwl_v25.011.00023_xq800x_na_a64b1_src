/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the misc device config interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13b919
#define LOG_MODULE    AMOD_MISC

#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_md_config.h>
#include "aic_md_cfg.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#ifndef MD_DL_DEVICE_NAME
#define MD_DL_DEVICE_NAME           "misc"
#endif

#ifndef MD_CALIBRATION_DEVICE_NAME
#define MD_CALIBRATION_DEVICE_NAME  "misc_protect"
#endif

#ifndef MD_NORMAL_DEVICE_NAME
#define MD_NORMAL_DEVICE_NAME       "misc_run"
#endif

#ifndef MD_USER_DEVICE_NAME
#define MD_USER_DEVICE_NAME         "misc_user"
#endif

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Static Function Prototypes
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
* aic_md_get_download_devname - get misc download device name.
*
* Parameter:
*
* Return:
*   misc download device name.
*/
char *aic_md_get_download_devname(void)
{
    return (char *)MD_DL_DEVICE_NAME;
}

/**
* aic_md_get_devname - get misc device name by device type.
*
* Parameter:
*   @devtype: misc device type, MD_TYPE_E.
*
* Return:
*   misc device name.
*/
char *aic_md_get_devname(u32 devtype)
{
    char *name = NULL;

    if (devtype == MD_CALIBRATION_TYPE) {
        name = (char *)MD_CALIBRATION_DEVICE_NAME;
    } else if (devtype == MD_NORMAL_TYPE) {
        name = (char *)MD_NORMAL_DEVICE_NAME;
    } else if (devtype == MD_USER_TYPE) {
        name = (char *)MD_USER_DEVICE_NAME;
    } else {
        AIC_ASSERT(0);
    }

    return name;
}

/**
* aic_md_get_blk_cfg - get misc device block config.
*
* Parameter:
*   @blk_cfg: misc device block config.
*
* Return:
*/
void aic_md_get_blk_cfg(MD_BLK_CFG *blk_cfg)
{
    blk_cfg->blk_size = MD_BLOCK_SIZE;
#ifdef WIN32
    blk_cfg->sector_size = MD_SECTOR_SIZE;
    blk_cfg->flash_sector_size = MD_FLASH_SECTOR_SIZE;
    blk_cfg->cali_blk_num = MD_CALI_BLOCK_NUM;
    blk_cfg->normal_blk_num = MD_NORMAL_BLOCK_NUM;
    blk_cfg->user_blk_num = MD_USER_BLOCK_NUM;
    blk_cfg->dl_size = MD_DL_SIZE;
#endif
}

/**
* aic_md_get_whitelist_cfg - get md whitelist id config.
*
* Parameter:
*   @whitelist: md whitelist item id.
*   @count: whitelist id count.
*
* Return:
*
*/
void aic_md_get_whitelist_cfg(u16 **whitelist, u32 *count)
{
    *count = sizeof(md_whitelist_id) / sizeof(u16);
    *whitelist = md_whitelist_id;
}

/**
* aic_md_get_max_id - get misc data max id.
*
* Parameter:
*
* Return:
*   return misc data max id.
*/
u16 aic_md_get_max_id()
{
    return MD_ID_MAX;
}

/**
* aic_md_get_min_id - get misc data min id.
*
* Parameter:
*
* Return:
*   return misc data min id.
*/
u16 aic_md_get_min_id()
{
    return MD_ID_MIN;
}

