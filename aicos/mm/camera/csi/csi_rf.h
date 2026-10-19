
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-06-20 10:06:21
 */

#ifndef __CSI_RF_H__
#define __CSI_RF_H__

#include <aic_reg_base.h>

#define REG_BASE_CSI                                                            REG_BASE_AP_SPI_CSI

#define REG_CSI_CSI_START                                                       (REG_BASE_CSI + 0x0)
#define REG_CSI_CSI_CR0                                                         (REG_BASE_CSI + 0x4)
#define REG_CSI_CSI_SYNC                                                        (REG_BASE_CSI + 0x8)
#define REG_CSI_CSI_PACKET                                                      (REG_BASE_CSI + 0xc)
#define REG_CSI_CSI_INT_OUT                                                     (REG_BASE_CSI + 0x10)
#define REG_CSI_CSI_INT_MASK                                                    (REG_BASE_CSI + 0x14)
#define REG_CSI_CSI_INT_CLEAR                                                   (REG_BASE_CSI + 0x18)
#define REG_CSI_CSI_INT_RAW                                                     (REG_BASE_CSI + 0x1c)
#define REG_CSI_CSI_FRAME_ADDR                                                  (REG_BASE_CSI + 0x20)
#define REG_CSI_CSI_FRAME_ADDR_2P                                               (REG_BASE_CSI + 0x24)
#define REG_CSI_CSI_SW_RST                                                      (REG_BASE_CSI + 0x28)
#define REG_CSI_CSI_TO                                                          (REG_BASE_CSI + 0x2c)
#define REG_CSI_CSI_STATUS0                                                     (REG_BASE_CSI + 0x30)
#define REG_CSI_CSI_STATUS1                                                     (REG_BASE_CSI + 0x34)
#define REG_CSI_CSI_STATUS2                                                     (REG_BASE_CSI + 0x38)
#define REG_CSI_CSI_STATUS3                                                     (REG_BASE_CSI + 0x3c)
#define REG_CSI_CSI_ADDR_LIMIT                                                  (REG_BASE_CSI + 0x40)

/*REG_CSI_CSI_START*/
#define BIT_CSI_CSI_ENABLE                                                      BIT(0)

/*REG_CSI_CSI_CR0*/
#define BIT_CSI_IMG_WIDTH(x)                                                    (((x) & 0xfff) << 20)
#define BIT_CSI_WIDTH_EN                                                        BIT(19)
#define BIT_CSI_PXL_BYTE                                                        BIT(18)
#define BIT_CSI_ENDIAN(x)                                                       (((x) & 0x3) << 16)
#define BIT_CSI_DDR_MODE                                                        BIT(15)
#define BIT_CSI_INT_PULSE_SEL                                                   BIT(14)
#define BIT_CSI_EOF_SEL                                                         BIT(13)
#define BIT_CSI_RXFIFO_THRD(x)                                                  (((x) & 0x1f) << 8)
#define BIT_CSI_PLANE2_FLAG                                                     BIT(7)
#define BIT_CSI_WIRE_SWAP                                                       BIT(6)
#define BIT_CSI_WIRE_TYPE(x)                                                    (((x) & 0x3) << 4)
#define BIT_CSI_LSB_FIRST                                                       BIT(2)
#define BIT_CSI_CRC_EN                                                          BIT(1)
#define BIT_CSI_MFORMAT_EN                                                      BIT(0)

/*REG_CSI_CSI_SYNC*/
#define BIT_CSI_BUSY_BD                                                         BIT(31)
#define BIT_CSI_DEBUG_EN                                                        BIT(30)
#define BIT_CSI_CAP_MODE                                                        BIT(29)
#define BIT_CSI_SYNC_WORD(x)                                                    (((x) & 0xffffff))

/*REG_CSI_CSI_PACKET*/
#define BIT_CSI_SOF_WORD(x)                                                     (((x) & 0xff) << 24)
#define BIT_CSI_SOL_WORD(x)                                                     (((x) & 0xff) << 16)
#define BIT_CSI_EOL_WORD(x)                                                     (((x) & 0xff) << 8)
#define BIT_CSI_EOF_WORD(x)                                                     (((x) & 0xff))

/*REG_CSI_CSI_INT_OUT*/
#define BIT_CSI_END_OF_FRAME                                                    BIT_0
#define BIT_CSI_START_OF_FRAME                                                  BIT_1
#define BIT_CSI_START_RX_IS_FULL                                                BIT_2
#define BIT_CSI_TIMEOUT                                                         BIT_3
#define BIT_CSI_EOL                                                             BIT_4
#define BIT_CSI_SOL                                                             BIT_5

/*REG_CSI_CSI_INT_MASK*/
#define BIT_CSI_INT_MASK(x)                                                     (((x) & 0xffff))

/*REG_CSI_CSI_INT_CLEAR*/
#define BIT_CSI_INT_CLR(x)                                                      (((x) & 0xffff))

/*REG_CSI_CSI_INT_RAW*/
#define BIT_CSI_INT_RAW(x)                                                      (((x) & 0xffff))

/*REG_CSI_CSI_FRAME_ADDR*/
#define BIT_CSI_FRAME_ADDR(x)                                                   (((x) & 0x3fffffff) << 2)

/*REG_CSI_CSI_FRAME_ADDR_2P*/
#define BIT_CSI_FRAME_ADDR_2P(x)                                                (((x) & 0x3fffffff) << 2)

/*REG_CSI_CSI_SW_RST*/
#define BIT_CSI_CSI_SW_RSTN                                                     BIT(0)

/*REG_CSI_CSI_TO*/
#define BIT_CSI_TMOT_EN                                                         BIT(31)
#define BIT_CSI_TMOT_VALUE(x)                                                   (((x) & 0xfff))

/*REG_CSI_CSI_STATUS0*/
#define BIT_CSI_STATUS0(x)                                                      (((x) & 0xffffffff))

/*REG_CSI_CSI_STATUS1*/
#define BIT_CSI_STATUS1(x)                                                      (((x) & 0xffffffff))

/*REG_CSI_CSI_STATUS2*/
#define BIT_CSI_STATUS2(x)                                                      (((x) & 0xffffffff))

/*REG_CSI_CSI_STATUS3*/
#define BIT_CSI_STATUS3(x)                                                      (((x) & 0xffffffff))

/*REG_CSI_CSI_ADDR_LIMIT*/
#define BIT_CSI_MEM_SIZE_LIMIT                                                  BIT(31)
#define BIT_CSI_MEM_MAX_SIZE(x)                                                 (((x) & 0xff))

#endif /*__CSI_RF_H__*/
