/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Interface of bspatch.
*/

#ifndef __BSPATCH_H__
#define __BSPATCH_H__

#include <aic_type.h>

/**
 * patch_partition() - Apply patch to specified partition image
 *
 * @p_orig   : Point to old partition start address
 * @origsz   : Old partition size in byte, not old image size
 * @p_diff   : Point to difference package start address
 * @diffsz   : Difference package size in byte
 * @p_new    : Point to buffer which store new image
 * @newsz    : Size of generated new image, auto set by function
 *
 * Apply difference patch to specified partition image.
 *
 * Return: AIC_SUCCESS if no error;errno if there is error.
 *
 */
s32 patch_partition (u8 *p_orig, u32 origsz,  u8 *p_diff,
                     u32 diffsz,   u8 *p_new, u32 *newsz);

/**
 * patch_partition_fast() - Fast apply patch to specified partition image
 *
 * @p_orig   : Point to old partition start address
 * @origsz   : Old partition size in byte, not old image size
 * @p_diff   : Point to difference package start address
 * @diffsz   : Difference package size in byte
 * @p_new    : Point to buffer which store new image
 * @newsz    : Size of generated new image, auto set by function
 *
 * Apply difference patch to specified partition image.
 *
 * Return: AIC_SUCCESS if no error;errno if there is error.
 *
 */
s32 patch_partition_fast (u8 *p_orig, u32 origsz,  u8 *p_diff,
                          u32 diffsz,   u8 *p_new, u32 *newsz);
#endif /* __BSPATCH_H__ */
