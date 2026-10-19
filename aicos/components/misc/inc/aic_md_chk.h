/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the misc check operation interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_MISC_CHECK_H__
#define __AIC_MISC_CHECK_H__

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
 * md_calc_checksum - calc misc checksum.
 *
 * Parameter:
 * @dat: data.
 * @len: data length.
 * @checksum: add checksum
 * @flag: cali final checksum or not.
 *
 * Return:
 *    @checksum: checksum value.
 */
u32 md_calc_checksum(const void *dat, u32 len, u32 checksum, bool flag);

/**
 * md_check_item - check misc item.
 *
 * Parameter:
 * @header: item header.
 * @offset: item offset.
 * @total_size: partition size.
 *
 * Return:
 *    @staus: AIC_SUCCESS is pass, other is error.
 */
int md_check_item(MD_HEADER_S *header, u32 offset, u32 total_size);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_MISC_CHECK_H__ */

