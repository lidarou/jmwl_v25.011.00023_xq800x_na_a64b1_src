/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the aic data check operation interface.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x166286

#include "platform/aic_checksum.h"

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * aic_calc_checksum - calc the specified checksum.
 *
 * Parameter:
 * @dat: data.
 * @len: data length.
 *
 * Return:
 *    @checksum: checksum value.
 */
u16 aic_calc_checksum(const void *data, u32 len)
{
    u32 checksum = 0;

    if (0 == (u32)data % 2) {
        const u16 *pstart = (const u16*)data;
        const u16 *pend = pstart + len / 2;

        while (pstart < pend) {
            checksum += *pstart;
            ++pstart;
        }

        if (len % 2) {
            checksum += *(const u8*)pstart;
        }
    } else {
        const u8 *pstart = (const u8*)data;

        while (len > 1) {

#ifdef CONFIG_ENDIAN_BIG
            checksum += (((*pstart) << 8) | (*(pstart + 1)));
#else /* CONFIG_ENDIAN_LITTLE */
            checksum += ((*pstart) | ((*(pstart + 1))<<8));
#endif

            len -= 2;
            pstart += 2;
        }

        if (len) {
            checksum += *pstart;
        }
    }

    checksum = (checksum >> 16) + (checksum & 0x0FFFF);
    checksum += (checksum >> 16);
    return (~checksum);
}

