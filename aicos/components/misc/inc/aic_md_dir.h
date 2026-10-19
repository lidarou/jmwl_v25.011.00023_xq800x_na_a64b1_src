/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the misc directory operation struct and interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_MISC_DIRECTORY_H__
#define __AIC_MISC_DIRECTORY_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
/**
 * misc item directory.
 *
 * @size: item id.
 * @category: MD_CATEGORY_E.
 * @status: item status, BIT(0-3) MD_ITEM_STATUS_E, BIT(4-7) MD_TYPE_E.
 * @offset: offset of the item.
 */
typedef struct MD_DIRECTORY_ITEM {
    u16                 size;
    u8                  category;
    u8                  status;
    u32                 offset;
} MD_DIR_ITEM_S;

/**
 * misc directory information.
 *
 * @dir: file handle.
 * @dir_mutex: directory mutex.
 */
typedef struct MD_DIRECTORY_INFO {
    MD_DIR_ITEM_S       *dir;
    aos_mutex_ptr       *dir_mutex;
}MD_DIR_INFO_S;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * md_dir_init - directory init.
 *
 * Parameter:
 * @dir_info: misc directory info.
 *
 * Return:
 *
 */
void md_dir_init(MD_DIR_INFO_S *dir_info);

/**
 * md_dir_get - get item directory.
 *
 * Parameter:
 * @item_id: item id.
 * @dir_item: item directory.
 * @dir_info: misc directory info.
 *
 * Return:
 *
 */
void md_dir_get(u16 item_id, MD_DIR_ITEM_S *dir_item, MD_DIR_INFO_S *dir_info);

/**
 * md_dir_set - set item directory.
 *
 * Parameter:
 * @item_id: item id.
 * @dir_item: item directory.
 * @dir_info: misc directory info.
 *
 * Return:
 *
 */
void md_dir_set(u16 item_id, MD_DIR_ITEM_S *dir_item, MD_DIR_INFO_S *dir_info);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_MISC_DIRECTORY_H__ */

