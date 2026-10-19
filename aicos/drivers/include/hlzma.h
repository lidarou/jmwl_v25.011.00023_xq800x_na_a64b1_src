/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Interface of hardware lzma.
*/

#ifndef __HLZMA_H__
#define __HLZMA_H__

#include <aic_type.h>

/**
 * hlzma_dec_polling() - Do decompress and poll to done
 *
 * @in_addr: Start address of compressed data
 * @out_addr: Output address of decompressed data
 *
 * Decompress some data and polling until finish.
 *
 * Return: AIC_SUCCESS if no error;errno if there is error.
 *
 */
s32 hlzma_dec_polling(u32 in_addr, u32 out_addr);

/**
 * hlzma_dec() - Do decompress and return after start
 *
 * @in_addr: Start address of compressed data
 * @out_addr: Output address of decompressed data
 *
 * Decompress some data and return immediately.
 *
 * Return: AIC_SUCCESS if no error;errno if there is error.
 *
 */
s32 hlzma_dec(u32 in_addr, u32 out_addr);

#endif /* __HLZMA_H__ */
