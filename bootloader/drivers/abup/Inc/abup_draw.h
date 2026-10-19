/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_draw.h
* Description: abup_draw.cÍ·ÎÄ¼þ
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef __ABUP_DRAW_H__
#define __ABUP_DRAW_H__
#include "abup_typedef.h"

#if !defined(ABUP_C51)&&!defined(__CCRL__)
#if defined(STM32F4_DISCO)||defined(STM32L0_DISCO)
#define ABUP_FOTA_DISPLAY
#endif
#endif
#if !defined(ABUP_BOOTLOADER)&&defined(STM32F4_DISCO)
#include "abup_stdlib.h"
#define ABUP_LCD_BG_COLOR GUI_LIGHTYELLOW//0xb2c7ca
#define ABUP_LCD_TXT_COLOR GUI_RED
#define ABUP_LCD_TXT_DISABLE_COLOR GUI_GRAY
#include "GUI.h"
void AbupDisplayString(abup_int16 Line,const abup_char *content, ...);
void AbupDisplayClearLine(abup_int16 Line);
#elif defined(STM32L0_DISCO)
#include "stm32l0538_discovery_epd.h"
#define STM32L053C8_EPD_MAX_LINE 4
#define STM32L053C8_EPD_MAX_LINE_DISCHAR 40
#define AbupDisplayString(Line,CONTENT,...)  \
        do{ \
          abup_memset(dis[Line], 0, abup_sizeof(dis[Line])); \
          abup_snprintf(dis[Line],abup_sizeof(dis[Line]),CONTENT,##__VA_ARGS__); \
					AbupDrawString(); \
        }while(0);

abup_extern abup_char dis[STM32L053C8_EPD_MAX_LINE][STM32L053C8_EPD_MAX_LINE_DISCHAR];
abup_extern void AbupDrawString(void);
abup_extern void AbupDisplayClearLine(abup_uint Line);
#elif defined(ABUP_FOTA_DISPLAY)
#define AbupDisplayString(Line,CONTENT,...)
#define AbupDisplayClearLine(Line)
#endif
#endif
