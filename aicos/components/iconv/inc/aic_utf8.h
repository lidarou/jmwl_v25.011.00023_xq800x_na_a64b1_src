
/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the interfaces to convert between unicode and utf8
 */
#ifndef __AIC_UTF8_H__
#define __AIC_UTF8_H__

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
 * aic_utf8_to_unicode - converter from utf8 to unicode
 * @utf8_str: [IN] string coded as utf8
 * @utf8_size: [IN] Size, in bytes, of the string indicated by the utf8_str
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
int32_t aic_utf8_to_unicode(const uint8_t *utf8_str, int32_t utf8_size,
                           uint16_t *uni_str, int32_t uni_size);
/**
 * aic_unicode_to_utf8 - converter from unicode to utf8
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
 * @utf8_str: [OUT] string coded as utf8
 * @utf8_size: [IN] Size, in bytes, of the buffer indicated by utf8_str. If this
 * value is 0, the function returns the required buffer size, in bytes,
 * including any terminating null character, and makes no use of the
 * utf8_str buffer.
 *
 * Return: If successful, returns the number of bytes written to the buffer
 * pointed to by utf8_str. If the function succeeds and utf8_size is 0, the return
 * value is the required size, in bytes, for the buffer indicated by utf8_str. The
 * function returns 0 if it does not succeed.
 */
int32_t aic_unicode_to_utf8(const uint16_t *uni_str, int32_t uni_size,
                           uint8_t *utf8_str, int32_t utf8_size);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_UTF8_H__ */