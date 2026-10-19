
/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the interfaces to convert between unicode and gbk
 */
#ifndef __AIC_GBK_H__
#define __AIC_GBK_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#ifdef WIN32
#include "stdint.h"
#else
#include "aic_type.h"
#endif

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
/**
 * aic_gbk_to_unicode - converter from gbk to unicode
 * @gb_str: [IN] string coded as gbk
 * @gb_size: [IN] Size, in bytes, of the string indicated by the gb_str
 * parameter. If this parameter is -1, the function processes the entire input
 * string, including the terminating null character. Therefore, the resulting
 * Unicode string has a terminating null character, and the length returned by
 * the function includes this character. If this parameter is set to a positive
 * integer, the function processes exactly the specified number of bytes. If the
 * provided size does not include a terminating null character, the resulting
 * Unicode string is not null-terminated, and the returned length does not
 * include this character.
 * @uni_str: [OUT] string coded as unicode
 * @uni_size: [IN] Size, in characters, of the buffer indicated by uni_str.
 *
 * Return: the number of characters written to the buffer indicated by uni_str
 * if successful. If the function succeeds and uni_size is 0, the return value
 * is the required uni_size for uni_str. The function returns 0 if it does not
 * succeed.
 */
int32_t aic_gbk_to_unicode(const uint8_t *gb_str, int32_t gb_size,
                           uint16_t *uni_str, int32_t uni_size);
/**
 * aic_unicode_to_gbk - converter from unicode to gbk
 * @uni_str: [IN] string coded as unicode
 * @uni_size: [IN] Size, in characters, of the string indicated by uni_str
 * parameter. Alternatively, this parameter can be set to -1 if the string is
 * null-terminated. If uni_size is set to 0, the function fails. If this
 * parameter is -1, the function processes the entire input string, including
 * the terminating null character. Therefore, the resulting character string has
 * a terminating null character, and the length returned by the function
 * includes this character. If this parameter is set to a positive integer, the
 * function processes exactly the specified number of characters. If the
 * provided size does not include a terminating null character, the resulting
 * character string is not null-terminated, and the returned length does not
 * include this character.
 * @gb_str: [OUT] string coded as gbk
 * @gb_size: [IN] Size, in bytes, of the buffer indicated by gb_str. If this
 * value is 0, the function returns the required buffer size, in bytes,
 * including any terminating null character, and makes no use of the
 * gb_str buffer.
 *
 * Return: If successful, returns the number of bytes written to the buffer
 * pointed to by gb_str. If the function succeeds and gb_size is 0, the return
 * value is the required size, in bytes, for the buffer indicated by gb_str. The
 * function returns 0 if it does not succeed.
 */
int32_t aic_unicode_to_gbk(const uint16_t *uni_str, int32_t uni_size,
                           uint8_t *gb_str, int32_t gb_size);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_GBK_H__ */