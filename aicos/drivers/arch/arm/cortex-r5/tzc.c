/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines interface of tzc
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0eaabb
#define LOG_MODULE    AMOD_DRV

#include <aic_io.h>

#define MC_SEC_PRIV_ATTR    0xAA

/*
 * Set on mcu:
 * 0x30 aud mem 0x31 aud io
 * 0x10 mcu dma 0x2  eip
 * 0x3  usb     0x4  sdioh
 * 0x5  lcdc    0x6  csi
 * 0x7  pppe    0x8  pppd
 * 0x38 aud dma
 *
 * Set on cr5:
 * 0x20 cr5     0x21 axi
 * 0x22 ciphc   0x28 cr5 dma
 * 0x23 mdm     0x9  lzma
 *
 * Top:
 * 0xE  dap     0xd  tpi
 *
 * GRP0, GPR2, GPR3 for mcu and top
 * GPR4 for cr5
 *
 */
void tzc_resume(void)
{
    /* Set GPR4 laster address*/
    AIC_WRITE32(0x41144124, 0x48000);
    /* Set GPR4 master: cr5, axi, ciphic, dma, mdm, lzma */
    AIC_WRITE32(0x41144040, 0x20212228);
    AIC_WRITE32(0x41144044, 0x2309);
    return;
}
