/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the eipu phy elements used by eipu driver.
 */

#ifndef __EIPU_REG_H__
#define __EIPU_REG_H__

#include <aic_bitmap.h>

//SAME OFFSET IN EIPU_DBNC/EIPU_LATCH/EIPU_EDGE_32K/EIPU_EDGE
#define EIPU_INT_INDATA            ( 0x0000 )
#define EIPU_INT_RAWSTS            ( 0x0004 )
#define EIPU_INT_EN                ( 0x0008 )
#define EIPU_INT_MSKSTS            ( 0x000C )
#define EIPU_INT_CLR               ( 0x0010 )
#define EIPU_INT_POL               ( 0x0014 )

// EIPU_DBNC contral registers address map : D-Die
#define EIPU_DBNC_TRG_CTRL            ( 0x0018 )
#define EIPU_DBNC_CHANNEL_CTRL(x)     ( 0x0040 + (x)*0x04)
#define EIPU_ND_DBNC_CHANNEL_CTRL(x)  ( 0x0020 + (x)*0x04)

#define EIPU_NORMAL_TYPE_OFFSET    ( 0x40 )
#define EIPU_ND_TYPE_OFFSET        ( 0x20 )

// EIPU_EDGE_32K contral registers address map
#define EIPU_EDGE_32K_INT_MODE     ( 0x0018 )
#define EIPU_EDGE_32K_INT_BOTH     ( 0x001C )

// EIPU_EDGE contral registers address map
#define EIPU_EDGE_INT_MODE         ( 0x0018 )
#define EIPU_EDGE_INT_BOTH         ( 0x001C )

// EIPU bits field define:
// EIPUCTRL:
//        [31:16] : reserved
//        [   15] : reserved
//        [   14] : DBNC_EN
//        [13:12] : reserved
//        [11:00] : DBNC_CNT
#define EIPUCTRL_DBNC_EN           BIT_14
#define EIPUCTRL_DBNC_CNT_BITMSK   ( 0x00FFF )
#define EIPUCTRL_DBNC_CNT_BITPOS   ( 0 )

#define EIPUCTRL_SOFTMODE_EN       ( 0xFF0 )
#define EIPUCTRL_SOFTMODE_DATA     ( 0xFF4 )

#endif /* __EIPU_REG_H__ */
