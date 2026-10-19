
/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the interfaces to convert between unicode and utf8
 */
/*******************************************
 *              INCLUDES
 ******************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17cadb
#define LOG_MODULE    AMOD_SYS

#include "../inc/aic_utf8.h"
#include "aic_converter.h"
#include "stdlib.h"
#ifdef WIN32
#include "string.h"
#endif

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
static int32_t utf8_mbtowc(uint32_t *pwc, const uint8_t *s, size_t n)
{
    uint8_t c = s[0];

    if (c < 0x80) {
        *pwc = c;
        return 1;
    } else if (c < 0xc2) {
        return AIC_RET_ILSEQ;
    } else if (c < 0xe0) {
        if (n < 2)
            return AIC_RET_TOOFEW(0);
        if (!((s[1] ^ 0x80) < 0x40))
            return AIC_RET_ILSEQ;
        *pwc = ((uint32_t)(c & 0x1f) << 6) | (uint32_t)(s[1] ^ 0x80);
        return 2;
    } else if (c < 0xf0) {
        if (n < 3)
            return AIC_RET_TOOFEW(0);
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 &&
              (c >= 0xe1 || s[1] >= 0xa0) && (c != 0xed || s[1] < 0xa0)))
            return AIC_RET_ILSEQ;
        *pwc = ((uint32_t)(c & 0x0f) << 12) | ((uint32_t)(s[1] ^ 0x80) << 6) |
               (uint32_t)(s[2] ^ 0x80);
        return 3;
    } else if (c < 0xf8 && sizeof(uint32_t) * 8 >= 32) {
        if (n < 4)
            return AIC_RET_TOOFEW(0);
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 &&
              (s[3] ^ 0x80) < 0x40 && (c >= 0xf1 || s[1] >= 0x90) &&
              (c < 0xf4 || (c == 0xf4 && s[1] < 0x90))))
            return AIC_RET_ILSEQ;
        *pwc = ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(s[1] ^ 0x80) << 12) |
               ((uint32_t)(s[2] ^ 0x80) << 6) | (uint32_t)(s[3] ^ 0x80);
        return 4;
    } else
        return AIC_RET_ILSEQ;
}

static int32_t utf8_wctomb(uint8_t *r, uint32_t wc,
                       size_t n) /* n == 0 is acceptable */
{
    size_t count;

    if (wc < 0x80)
        count = 1;
    else if (wc < 0x800)
        count = 2;
    else if (wc < 0x10000) {
        if (wc < 0xd800 || wc >= 0xe000)
            count = 3;
        else
            return AIC_RET_ILUNI;
    } else if (wc < 0x110000)
        count = 4;
    else
        return AIC_RET_ILUNI;

    if (n < count)
        return AIC_RET_TOOSMALL;

    switch (count) { /* note: code falls through cases! */
        case 4:
            r[3] = 0x80 | (wc & 0x3f);
            wc = wc >> 6;
            wc |= 0x10000;
        case 3:
            r[2] = 0x80 | (wc & 0x3f);
            wc = wc >> 6;
            wc |= 0x800;
        case 2:
            r[1] = 0x80 | (wc & 0x3f);
            wc = wc >> 6;
            wc |= 0xc0;
        case 1:
            r[0] = wc;
    }

    return (int32_t)count;
}

/*******************************************
 *              GLOBAL FUNCTIONS
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
                            uint16_t *uni_str, int32_t uni_size)
{
    int32_t result = 0;
    int32_t insize = utf8_size;
    int32_t insize_bk = 0;
    const uint8_t *inptr = utf8_str;
    int32_t count = 0;
    uint32_t wc = 0;
    int32_t res = 0;

    if (!utf8_str || (!uni_str && uni_size != 0)) {
        return 0;
    }
    if (utf8_size == -1) {
        insize = (int32_t)strlen((const char *)inptr) + 1;
    }
    insize_bk = insize;

    /* Determine the length we need. */
    while (insize > 0) {
        res = utf8_mbtowc(&wc, inptr, (size_t)insize);
        if (res > 0) {
            count++;
            inptr += res;
            insize -= res;
        } else {
            return 0;
        }
    }

    if (uni_size == 0) {
        return count;
    } else {
        if (uni_size < count) {
            return 0;
        }
    }

    /* Do the conversion for real. */
    result = count;
    insize = insize_bk;
    inptr = utf8_str;
    while (insize > 0) {
        res = utf8_mbtowc(&wc, inptr, insize);
        if (res > 0) {
            *uni_str = (uint16_t)wc;
            uni_str++;
            inptr += res;
            insize -= res;
        } else {
            return 0;
        }
    }

    return result;
}

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
 * pointed to by utf8_str. If the function succeeds and utf8_size is 0, the
 * return value is the required size, in bytes, for the buffer indicated by
 * utf8_str. The function returns 0 if it does not succeed.
 */
int32_t aic_unicode_to_utf8(const uint16_t *uni_str, int32_t uni_size,
                            uint8_t *utf8_str, int32_t utf8_size)
{
    int32_t result = 0;
    int32_t insize = uni_size;
    int32_t insize_bk = 0;
    const uint16_t *inptr = uni_str;
    int32_t count = 0;
    int32_t res = 0;
    uint8_t temp_utf8[4] = {0};
    uint8_t *out_str = utf8_str;

    if (!uni_str || (!utf8_str && utf8_size != 0)) {
        return 0;
    }
    if (uni_size == -1) {
        const uint16_t *temp = uni_str;

        insize = 0;
        while (*temp != 0) {
            temp++;
            insize++;
        }
        insize++;
    }
    insize_bk = insize;

    /* Determine the length we need. */
    while (insize > 0) {
        res = utf8_wctomb(temp_utf8, (uint32_t)*inptr, 4);
        if (res > 0) {
            count += res;
            inptr++;
            insize--;
        } else {
            return 0;
        }
    }

    if (utf8_size == 0) {
        return count;
    } else {
        if (utf8_size < count) {
            return 0;
        }
    }

    /* Do the conversion for real. */
    result = count;
    insize = insize_bk;
    inptr = uni_str;
    while (insize > 0) {
        res = utf8_wctomb(temp_utf8, (uint32_t)*inptr, 4);
        if (res > 0) {
            memcpy(out_str, temp_utf8, res);
            out_str += res;
            inptr++;
            insize--;
        } else {
            return 0;
        }
    }

    return result;
}
