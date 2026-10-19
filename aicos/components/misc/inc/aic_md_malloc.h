/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the misc malloc operation interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_MISC_MALLOC_H__
#define __AIC_MISC_MALLOC_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include "aic_md_dev.h"

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * md_dev_blk_uselist_get - get misc use list.
 *
 * Parameter:
 * @md_obj: misc object.
 * @type: misc type.
 * @blk_index: block index.
 * @uselist: use item list.
 *
 * Return:
 *
 */
void md_dev_blk_uselist_get(MD_OBJ_S *md_obj, MD_TYPE_E type, u32 blk_index,
                            MD_ITEM_INFO_S **uselist);

/**
 * md_dev_blk_free - free invalid data and erase block.
 *
 * Parameter:
 * @md_obj: misc object.
 * @type: misc type.
 * @blk_index: block index.
 *
 * Return:
 *
 */
void md_dev_blk_free(MD_OBJ_S *md_obj, MD_TYPE_E type, u32 blk_index);

/**
 * md_dev_blk_malloc - malloc data.
 *
 * Parameter:
 * @md_obj: misc object.
 * @type: misc type.
 * @size: malloc size.
 * @blk_index: block index.
 *
 * Return:
 *      @store data offset.
 */
u32 md_dev_blk_malloc(MD_OBJ_S *md_obj, MD_TYPE_E type, u16 size,
                      u32 *blk_index);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_MISC_MALLOC_H__ */

