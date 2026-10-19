/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __DISPLAY_CFG_H__
#define __DISPLAY_CFG_H__

#include "aic_bitmap.h"

typedef enum {
    AIC_LCDC_4WIRE_8BIT = 0,
    AIC_LCDC_3WIRE_9BIT_1LANE = BIT(6),
    AIC_LCDC_3WIRE_9BIT_2LANE = BIT(6) | BIT(3)
} aic_lcdc_wire_type;

#define LCD_PANEL_WIDTH   240
#define LCD_PANEL_HEIGHT  320

#define AIC_WIRE_TYPE AIC_LCDC_4WIRE_8BIT
#define AIC_HW_TE_TRIGGER 1
#define LCD_PANEL_ROTATION_ANGLE    LCD_DIRECT_ROT_0
/*spi clk = 100M/((FRAME_CLK_DIV + 1)*2) */
#define FRAME_CLK_DIV 0x0

//#define COLOR_FORMAT_RGB444_PACK
//#define COLOR_FORMAT_RGB444_UNPACK
//#define COLOR_FORMAT_RGB444_COMPACT
#define COLOR_FORMAT_RGB565_1P1T
//#define COLOR_FORMAT_RGB666_PACK
//#define COLOR_FORMAT_RGB666_UNPACK
//#define COLOR_FORMAT_RGB888_PACK
//#define COLOR_FORMAT_RGB888_UNPACK
//#define COLOR_FORMAT_MONO_1P1T

#define CONFIG_PANEL_POWER "DVDD3V"

#endif /* __DISPLAY_CFG_H__ */
