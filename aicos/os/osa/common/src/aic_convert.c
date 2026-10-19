/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the data covert operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16b9cf
#define LOG_MODULE    AMOD_AOS

#include <aic_convert.h>
#include <stdio.h>

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
 * @size: input string.
 *
 * Return:
 */
boolean string2hex(u8 *src, u8 *dst, u32 size)
{
    u8 temp;
    u32 i;

    for (i = 0; i < size; i++) {
        temp = src[i];
        if (('0' <= temp) && ('9' >= temp)) {
            *dst = (temp - '0') << 4;
        } else if (('A' <= temp) && ('F' >= temp)) {
            *dst = (temp - 'A' + 0xA) << 4;
        } else if (('a' <= temp) && ('f' >= temp)) {
            *dst = (temp - 'a' + 0xA) << 4;
        } else {
            return false;
        }

        i++;
        if (i < size) {
            temp = src[i];
            if (('0' <= temp) && ('9' >= temp)) {
                *dst |= temp - '0';
            } else if (('A' <= temp) && ('F' >= temp)) {
                *dst |= temp - 'A' + 0xA;
            } else if (('a' <= temp) && ('f' >= temp)) {
                *dst |= temp - 'a' + 0xA;
            } else {
                return false;
            }
        }

        dst++;
    }

    return true;
}

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
void hex2string(u8 *src, u8 *dst, u32 size)
{
    while (size) {
        sprintf(dst, "%02X", *src);
        dst += 2;
        src++;
        size -= 1;
    }

    *dst = '\0';
}
