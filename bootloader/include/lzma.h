/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Interface of lzma.
*/

#ifndef __LZMA_H__
#define __LZMA_H__

#include <aic_type.h>

/**
 * enc_partition() - Compress using lzma
 *
 * @dest   : Output buffer of compressed data
 * @destlen: Total length of data after compressed,
 *           auto set by this function.
 * @src    : Input buffer of data to be compressed
 * @srclen : Input data length in byte
 *
 * Compress some data with lzma, using settings:
 *         -d12 -lc0 -lp0 -pb0
 *
 * Return: AIC_SUCCESS if no error;errno if there is error.
 *
 */
s32 enc_partition (u8 *dest, u32 *destlen, u8 *src, u32 srclen);

#endif /* __LZMA_H__ */
