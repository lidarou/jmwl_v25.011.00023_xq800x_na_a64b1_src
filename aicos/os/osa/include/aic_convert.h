/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** Export the data convert operation interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_CONVERT_H__
#define __AIC_CONVERT_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * string2hex - string to hex.
 *  "1234" --> 0x12 0x34
 *
 * Parameter:
 * @src: input string ptr.
 * @dst: output hex ptr.
 * @size: input string size.
 *
 * Return:
 */
boolean string2hex(u8 *src, u8 *dst, u32 size);

/**
 * hex2string - hex to string.
 *  0x12 0x34 --> "1234"
 *
 * Parameter:
 * @src: input hex ptr.
 * @dst: output string ptr.
 * @size: input hex size.
 *
 * Return:
 */
void hex2string(u8 *src, u8 *dst, u32 size);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_CONVERT_H__ */
