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
#define AIC_THIS_FILE    0x0fc0bb
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
    /* Set master control to secure and n-privilege */
    AIC_WRITE32(0x42090000, MC_SEC_PRIV_ATTR); // audio mem
    AIC_WRITE32(0x42090004, MC_SEC_PRIV_ATTR); // audio io
    AIC_WRITE32(0x51070000, MC_SEC_PRIV_ATTR); // mcu dma
    AIC_WRITE32(0x51070004, MC_SEC_PRIV_ATTR); // eip
    AIC_WRITE32(0x51070008, MC_SEC_PRIV_ATTR); // usb
    AIC_WRITE32(0x5107000C, MC_SEC_PRIV_ATTR); // sdioh
    AIC_WRITE32(0x51070010, MC_SEC_PRIV_ATTR); // lcdc
    AIC_WRITE32(0x51070014, MC_SEC_PRIV_ATTR); // csi
    AIC_WRITE32(0x51070018, MC_SEC_PRIV_ATTR); // pppe
    AIC_WRITE32(0x5107001C, MC_SEC_PRIV_ATTR); // pppd
    /* Set GPR2 last address */
    AIC_WRITE32(0x41144114, 0x48000);
    /* Set GPR2 master: aud mem, aud io, mcu dma, eip */
    AIC_WRITE32(0x41144020, 0x30311002);
    /* Set GPR2 master: usb, sdioh */
    AIC_WRITE32(0x41144024, 0x304);
    /* Set GPR3 last address */
    AIC_WRITE32(0x4114411C, 0x48000);
    /* Set GPR3 master:lcdc, csi, pppe, pppd, aud dma, dap */
    AIC_WRITE32(0x41144030, 0x5060708);
    AIC_WRITE32(0x41144034, 0x380e);
    return;
}
