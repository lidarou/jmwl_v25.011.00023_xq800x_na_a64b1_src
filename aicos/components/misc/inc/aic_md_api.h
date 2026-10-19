/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the misc base operation struct and interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_MISC_API_H__
#define __AIC_MISC_API_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_md_init - misc init.
 *
 * Parameter:
 *
 * Return:
 */
void aic_md_init(void);

/**
 * aic_md_write - write item data.
 *
 * Parameter:
 * @item_id: item id.
 * @size: write size.
 * @buf: write buf.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int aic_md_write(u16 item_id, u16 item_size, const void *buf);

/**
 * aic_md_read - read item data.
 *
 * Parameter:
 * @item_id: item id.
 * @size: read size.
 * @buf: read buf.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int aic_md_read(u16 item_id, u16 item_size, void *buf);

/**
 * aic_md_delete - delete item.
 *
 * Parameter:
 * @item_id: item id.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int aic_md_delete(u16 item_id);

/**
 * aic_md_get_length - get item length.
 *
 * Parameter:
 * @item_id: item id.
 *
 * Return:
 *    @status: 0 is fail, other success.
 */
u16 aic_md_get_length(u16 item_id);

/**
 * aic_md_restore_factory - restore factory settings.
 *
 * Parameter:
 *
 * Return:
 */
void aic_md_restore_factory(void);

/**
 * aic_md_userdata_restore_factory - restore userdata factory settings.
 *
 * Parameter:
 *
 * Return:
 */
void aic_md_userdata_restore_factory(void);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_MISC_API_H__ */

