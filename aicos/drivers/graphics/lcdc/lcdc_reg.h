
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-05-12 19:33:09
 */

#ifndef __LCDC_RF_H__
#define __LCDC_RF_H__

#include <aic_auto_reg.h>

#define REG_BASE_LCDC                                                           REG_BASE_AP_SPI_LCD
#define REG_LCDC_LCDC_START                                                     (REG_BASE_LCDC + 0x0)
#define REG_LCDC_LCDC_CMD_INFO                                                  (REG_BASE_LCDC + 0x4)
#define REG_LCDC_LCDC_PARA_INFO                                                 (REG_BASE_LCDC + 0x8)
#define REG_LCDC_LCDC_DATA_INFO                                                 (REG_BASE_LCDC + 0xc)
#define REG_LCDC_LCDC_CR0                                                       (REG_BASE_LCDC + 0x10)
#define REG_LCDC_LCDC_ADDR                                                      (REG_BASE_LCDC + 0x14)
#define REG_LCDC_LCDC_CR1                                                       (REG_BASE_LCDC + 0x18)
#define REG_LCDC_LCDC_ST0                                                       (REG_BASE_LCDC + 0x1c)
#define REG_LCDC_LCDC_ST1                                                       (REG_BASE_LCDC + 0x20)
#define REG_LCDC_SSP_INT_OUT                                                    (REG_BASE_LCDC + 0x24)
#define REG_LCDC_SSP_INT_MASK                                                   (REG_BASE_LCDC + 0x28)
#define REG_LCDC_SSP_INT_CLEAR                                                  (REG_BASE_LCDC + 0x2c)
#define REG_LCDC_SSP_INT_RAW                                                    (REG_BASE_LCDC + 0x30)
#define REG_LCDC_SSP_RST                                                        (REG_BASE_LCDC + 0x34)
#define REG_LCDC_SSP_RDY                                                        (REG_BASE_LCDC + 0x38)
#define REG_LCDC_LCDC_DELAY                                                     (REG_BASE_LCDC + 0x3c)
#define REG_LCDC_LCDC_PARA_INFO2                                                (REG_BASE_LCDC + 0x40)
#define REG_LCDC_LCDC_PARA_INFO3                                                (REG_BASE_LCDC + 0x44)
#define REG_LCDC_LCDC_PARA_INFO4                                                (REG_BASE_LCDC + 0x48)
#define REG_LCDC_LCDC_IMG_TOTAL_BYTES                                           (REG_BASE_LCDC + 0x4C)

#define GX31X_IDLE_STATUS(a)    (GENMASK(0, 31) & a)
#define GX30X_IDLE_STATUS(a)    ((GENMASK(0, 6) & a) | (GENMASK(16, 31) & a))

/*REG_LCDC_LCDC_START                   0x0*/
#define BIT_LCDC_LCDC_START                                                     BIT(0)

/*REG_LCDC_LCDC_CMD_INFO                0x4*/
#define BIT_LCDC_CMD_FLAG                                                       BIT(31)
#define BIT_LCDC_DUM_FLAG                                                       BIT(30)
#define BIT_LCDC_PARA_FLAG                                                      BIT(29)
#define BIT_LCDC_DATA_FLAG                                                      BIT(28)
#define BIT_LCDC_BUSY_BD                                                        BIT(27)
#define BIT_LCDC_DUM_LENGTH(x)                                                  (((x) & 0xf) << 16)
#define BIT_LCDC_PARA_BYTE(x)                                                   (((x) & 0xf) << 8)
#define BIT_LCDC_CMD_WORD(x)                                                    (((x) & 0xff))

/*REG_LCDC_LCDC_PARA_INFO               0x8*/
#define BIT_LCDC_PARA_WORD(x)                                                   (((x) & 0xffffffff))

/*REG_LCDC_LCDC_DATA_INFO               0xc*/
#define BIT_LCDC_IMG_PITCH(x)                                                   (((x) & 0x3ff) << 16)
#define BIT_LCDC_IMG_WIDTH(x)                                                   (((x) & 0x3ff))

/*REG_LCDC_LCDC_CR0                     0x10*/
#define BIT_LCDC_BAUD_RATE(x)                                                   (((x) & 0xff) << 24)
#define BIT_LCDC_TXFIFO_THRD(x)                                                 (((x) & 0x1f) << 16)
#define BIT_LCDC_PACK_MODE                                                      BIT(11)
#define BIT_LCDC_PXLWR_CMD                                                      BIT(10)
#define BIT_LCDC_PXL_FMT(x)                                                     (((x) & 0x3) << 8)
#define BIT_LCDC_WIRE1_COMBINE                                                  BIT(7)
#define BIT_LCDC_LCD_3WIRE_FLAG                                                 BIT(6)
#define BIT_LCDC_LCD_CMD_DEF                                                    BIT(5)
#define BIT_LCDC_LCD_TOG_MODE                                                   BIT(4)
#define BIT_LCDC_WIRE_TYPE                                                      BIT(3)
#define BIT_LCDC_WRITE_OP                                                       BIT(2)
#define BIT_LCDC_LCD_START_MODE(x)                                              (((x) & 0x3))

/*REG_LCDC_LCDC_ADDR                    0x14*/
#define BIT_LCDC_FRAME_ADDR(x)                                                  (((x) & 0xffffffff))

/*REG_LCDC_LCDC_CR1                     0x18*/
#define BIT_LCDC_IMG_HEIGHT(x)                                                  (((x) & 0x3ff))

/*REG_LCDC_LCDC_ST0                     0x1c*/
#define BIT_LCDC_LCDC_ST0(x)                                                    (((x) & 0xffffffff))

/*REG_LCDC_LCDC_ST1                     0x20*/
#define BIT_LCDC_LCDC_ST1(x)                                                    (((x) & 0xffffffff))
#define BIT_LCDC_LCDC_STATE_MASK(x)                                             (((x) & 0x7f))

/*REG_LCDC_SSP_INT_OUT*/
#define BIT_LCDC_INT_OUT(x)                                                     (((x) & 0xffff))
#define BIT_LCDC_INT_ERROR                                                      0xC

/*REG_LCDC_SSP_INT_MASK*/
#define BIT_LCDC_INT_MASK(x)                                                    (((x) & 0xffff))

/*REG_LCDC_SSP_INT_CLEAR*/
#define BIT_LCDC_INT_CLR(x)                                                     (((x) & 0xffff))

/*REG_LCDC_SSP_INT_RAW*/
#define BIT_LCDC_INT_RAW(x)                                                     (((x) & 0xffff))

/*REG_LCDC_SSP_RST*/
#define BIT_LCDC_LCD_RSTN                                                       BIT(0)

/*REG_LCDC_SSP_RDY*/
#define BIT_LCDC_LCD_NXT_READY                                                  BIT(0)

/*REG_LCDC_LCDC_DELAY*/
#define BIT_LCDC_CAP_DAT_DLY(x)                                                 (((x) & 0x3) << 24)
#define BIT_LCDC_D_NSS(x)                                                       (((x) & 0xf) << 16)
#define BIT_LCDC_D_END(x)                                                       (((x) & 0xf) << 8)
#define BIT_LCDC_D_INIT(x)                                                      (((x) & 0xf))

/*REG_LCDC_LCDC_PARA_INFO2*/
#define BIT_LCDC_PARA_WORD2(x)                                                  (((x) & 0xffffffff))

/*REG_LCDC_LCDC_PARA_INFO3*/
#define BIT_LCDC_PARA_WORD3(x)                                                  (((x) & 0xffffffff))

/*REG_LCDC_LCDC_PARA_INFO4*/
#define BIT_LCDC_PARA_WORD4(x)                                                  (((x) & 0xffffffff))

#endif /*__LCDC_RF_H__*/
