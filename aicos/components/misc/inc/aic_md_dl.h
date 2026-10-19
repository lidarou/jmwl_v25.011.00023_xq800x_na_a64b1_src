/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the misc download operation struct and interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_MISC_DOWNLOAD_H__
#define __AIC_MISC_DOWNLOAD_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
/**
 * misc item header.
 *
 * @id: item id.
 * @size: item size.
 * @checksum: item data checksum.
 * @category: MD_CATEGORY_E
 * @status: MD_ITEM_STATUS_E
 */
typedef struct MD_HEADER{
    u16     id;
    u16     size;
    u16     checksum;
    u8      category;
    u8      status;
} MD_HEADER_S;

/**
 * misc item information.
 *
 * @header: item header information.
 * @offset: offset of the item.
 * @next: next item infomation.
 */
typedef struct _MD_ITEM_INFO {
    MD_HEADER_S             header;
    u32                     offset;
    struct _MD_ITEM_INFO    *next;
} MD_ITEM_INFO_S;

/**
 * misc download information.
 *
 * @handle: download handle
 * @addr: download rom address
 * @size: download size
 * @sector_size: sector size
 * @timestamp: download timestamp
 * @cali_item_list: calibration item list
 * @normal_item_list: normal item list
 */
typedef struct _MD_DOWNLOAD_INFO {
    u32                     handle;
    u32                     addr;
    u32                     size;
    u32                     sector_size;
    u32                     timestamp;
    MD_ITEM_INFO_S          *cali_item_list;
    MD_ITEM_INFO_S          *normal_item_list;
} MD_DOWNLOAD_INFO_S;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * md_dl_open - open download misc partition.
 *
 * Parameter:
 * @dl_info: download information.
 *
 * Return:
 */
void md_dl_open(MD_DOWNLOAD_INFO_S *dl_info);

/**
 * md_dl_close - close download misc partition.
 *
 * Parameter:
 * @dl_info: download information.
 *
 * Return:
 */
void md_dl_close(MD_DOWNLOAD_INFO_S *dl_info);

/**
 * md_dl_build_item_list - build item list.
 *
 * Parameter:
 * @dl_info: download information.
 *
 * Return:
 *
 */
void md_dl_build_item_list(MD_DOWNLOAD_INFO_S *dl_info);

/**
 * md_dl_get_list - get item list.
 *
 * Parameter:
 * @dl_info: download information.
 * @type: misc type.
 * @list: item list.
 *
 * Return:
 *
 */
void md_dl_get_list(MD_DOWNLOAD_INFO_S *dl_info, u8 type,
                    MD_ITEM_INFO_S **list);

/**
 * md_dl_item_is_had - download list had item or not.
 *
 * Parameter:
 * @dl_info: download information.
 * @type: misc type.
 * @header: item header.
 *
 * Return:
 *
 */
boolean md_dl_item_is_had(MD_DOWNLOAD_INFO_S *dl_info, u8 type,
                          MD_HEADER_S *header);

/**
 * md_dl_free_item_list - free item list.
 *
 * Parameter:
 * @dl_info: download information.
 *
 * Return:
 *
 */
void md_dl_free_item_list(MD_DOWNLOAD_INFO_S *dl_info);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_MISC_DOWNLOAD_H__ */

