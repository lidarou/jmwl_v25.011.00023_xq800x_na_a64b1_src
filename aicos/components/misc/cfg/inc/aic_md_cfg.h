/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the misc device config.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_MISC_DEVICE_CONFIG_H__
#define __AIC_MISC_DEVICE_CONFIG_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_bitmap.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define MD_SPARE_BLOCK_NUM          0x1

/* Please pay attention
 * MD_BLK_CHK_INFO_SIZE        sector_size
 * MD_MAX_DATA_AREA_SIZE       (MD_BLOCK_SIZE - sector_size)
 * MD_MAX_ITEM_DATA_SIZE       (MD_MAX_DATA_AREA_SIZE - sizeof(MD_HEADER_S))
*/

#define MD_DEFAULT_DATA             0xFF

#define MD_MAX_MALLOC_SECTOR_NUM    0x3

#define MD_INVALID_ITEM_ID          0xFFFF
#define MD_INVALID_TIMESTAMP        0xFFFFFFFF

/* MD Attribute */
#define MD_ATTRIBUTE_READ          BIT_0
#define MD_ATTRIBUTE_WRITE         BIT_1
#define MD_ATTRIBUTE_RDWR          MD_ATTRIBUTE_READ | MD_ATTRIBUTE_WRITE
#define MD_ATTRIBUTE_DISABLE_RDWR  BIT_7

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
/**
 * misc type.
 *
 * @MD_CALIBRATION_TYPE: calibration type
 * @MD_NORMAL_TYPE: normal type
 * @MD_USER_TYPE: user type.
 */
typedef enum MD_TYPE {
    MD_CALIBRATION_TYPE = 0,
    MD_NORMAL_TYPE,
    MD_USER_TYPE,
    MD_MAX_TYPE
} MD_TYPE_E;

/**
 * misc block status.
 *
 * @MD_BLK_ERASE: ERAS->0x45524153
 * @MD_BLK_PASS: PASS->0x50415353
 * @MD_USER_TYPE: REST->0x52455354, restore factore settings.
 */
typedef enum MD_BLK_STATUS {
    MD_BLK_ERASE    = 0x45524153,
    MD_BLK_PASS     = 0x50415353,
    MD_BLK_RESTORE  = 0x52455354,
    MD_BLK_NOWR     = 0xFFFFFFFF
} MD_BLK_STATUS_E;

/**
 * misc item status.
 *
 * @MD_ITEM_NONE: item no use
 * @MD_ITEM_USED: use item
 * @MD_ITEM_DELETE: delete item.
 */
typedef enum MD_ITEM_STATUS {
    MD_ITEM_NONE = 0,
    MD_ITEM_USED,
    MD_ITEM_DELETE
} MD_ITEM_STATUS_E;

/**
 * misc item category.
 *
 * @MD_CATEGORY_NORMAL: normal item
 * @MD_CATEGORY_CALIBRATION: calibration item
 */
typedef enum _MD_CATEGORY{
    MD_CATEGORY_NORMAL = 1,
    MD_CATEGORY_CALIBRATION,
    MD_CATEGORY_MAX
} MD_CATEGORY_E;

/**
 * misc block config.
 *
 * @blk_size: block size.
 * @sector_size: sector size.
 * @flash_sector_size: flash sector size.
 * @cali_blk_num: calibration block number.
 * @normal_blk_num: normal block number.
 * @user_blk_num: user block number.
 * @dl_size: download size.
 */
typedef struct _MD_BLK_CFG {
    u32 blk_size;
    u32 sector_size;
    u32 flash_sector_size;
    u32 cali_blk_num;
    u32 normal_blk_num;
    u32 user_blk_num;
    u32 dl_size;
} MD_BLK_CFG;

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
char *aic_md_get_download_devname(void);

/**
* aic_md_get_devname - get misc device name by device type.
*
* Parameter:
*   @devtype: misc device type, MD_TYPE_E.
*
* Return:
*   misc device name.
*/
char *aic_md_get_devname(u32 devtype);

/**
* aic_md_get_blk_cfg - get misc device block config.
*
* Parameter:
*   @blk_cfg: misc device block config.
*
* Return:
*/
void aic_md_get_blk_cfg(MD_BLK_CFG *blk_cfg);

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
void aic_md_get_whitelist_cfg(u16 **whitelist, u32 *count);

/**
* aic_md_get_max_id - get misc data max id.
*
* Parameter:
*
* Return:
*   return misc data max id.
*/
u16 aic_md_get_max_id();

/**
* aic_md_get_min_id - get misc data min id.
*
* Parameter:
*
* Return:
*   return misc data min id.
*/
u16 aic_md_get_min_id();

/**---------------------------------------------------------------------------*/
#endif /* __AIC_MISC_DEVICE_CONFIG_H__ */

