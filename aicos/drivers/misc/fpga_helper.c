/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Interface of fpga helper.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04f2cd
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_errno.h>
#include <aic_io.h>

/*
 * 0x400D0FF8 as fpga id register
 * Bit0~1 fpga version: 0, bit0; 1, bit1; 2, bit2
 * Bit2~3 fpga frequency: 0, 24MHZ; 2, 26MHZ
 */

u32 fpga_bitfile_id(void)
{
    u32 val;

    val = AIC_READ32(0x400D0FF8);
    val &= 0x3;
    return val;
}

u32 fpga_bitfile_freq(void)
{
    u32 val;

    val = AIC_READ32(0x400D0FF8);
    val >>= 2;
    val &= 0x3;
    return val;
}
