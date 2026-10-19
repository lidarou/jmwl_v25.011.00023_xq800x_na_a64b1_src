/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the lcd st7365 register map used by lcd st7365 panel driver.
 */

#ifndef __LCD_ST7365_H__
#define __LCD_ST7365_H__

#define LCD_ST7365_NOP              (0x00)
#define LCD_ST7365_SWRESET          (0x01)          // Software Reset
#define LCD_ST7365_RDDID            (0x04)          // Read Display ID
#define LCD_ST7365_RDDST            (0x09)          // Read Display Status
#define LCD_ST7365_RDDPM            (0x0A)          // Read Display Power Mode
#define LCD_ST7365_RDDMADCTL        (0x0B)          // Read Display MADCTL
#define LCD_ST7365_RDDCOLMOD        (0x0C)          // Read Display Pixel Format
#define LCD_ST7365_RDDIM            (0x0D)          // Read Display Image Mode
#define LCD_ST7365_RDDSM            (0x0E)          // Read Display Signal Mode
#define LCD_ST7365_RDDSDR           (0x0F)          // Read Display Self-Diagnostic Result
#define LCD_ST7365_SLPIN            (0x10)          // Sleep in
#define LCD_ST7365_SLPOUT           (0x11)          // Sleep Out
#define LCD_ST7365_PTLON            (0x12)          // Partial Display Mode On
#define LCD_ST7365_NORON            (0x13)          // Normal Display Mode On
#define LCD_ST7365_INVOFF           (0x20)          // Display Inversion Off
#define LCD_ST7365_INVON            (0x21)          // Display Inversion On
#define LCD_ST7365_GAMSET           (0x26)          // Gamma Set
#define LCD_ST7365_DISPOFF          (0x28)          // Display Off
#define LCD_ST7365_DISPON           (0x29)          // Display On
#define LCD_ST7365_CASET            (0x2A)          // Column Address Set
#define LCD_ST7365_RASET            (0x2B)          // Row Address Set
#define LCD_ST7365_RAMWR            (0x2C)          // Memory Write
#define LCD_ST7365_RAMRD            (0x2E)          // Memory Read
#define LCD_ST7365_PTLAR            (0x30)          // Partial Area
#define LCD_ST7365_VSCRDEF          (0x33)          // Vertical Scrolling Definition
#define LCD_ST7365_TEOFF            (0x34)          // Tearing Effect Line OFF
#define LCD_ST7365_TEON             (0x35)          // Tearing Effect Line On
#define LCD_ST7365_MADCTL           (0x36)          // Memory Data Access Control
#define LCD_ST7365_VSCSAD           (0x37)          // Vertical Scroll Start Address of RAM
#define LCD_ST7365_IDMOFF           (0x38)          // Idle Mode Off
#define LCD_ST7365_IDMON            (0x39)          // Idle mode on
#define LCD_ST7365_COLMOD           (0x3A)          // Interface Pixel Format
#define LCD_ST7365_WRMEMC           (0x3C)          // Write Memory Continue
#define LCD_ST7365_RDMEMC           (0x3E)          // Read Memory Continue
#define LCD_ST7365_STE              (0x44)          // Set Tear Scanline
#define LCD_ST7365_GSCAN            (0x45)          // Get Scanline
#define LCD_ST7365_WRDISBV          (0x51)          // Write Display Brightness
#define LCD_ST7365_RDDISBV          (0x52)          // Read Display Brightness Value
#define LCD_ST7365_WRCTRLD          (0x53)          // Write CTRL Display
#define LCD_ST7365_RDCTRLD          (0x54)          // Read CTRL Value Display
#define LCD_ST7365_WRCACE           (0x55)          // Write Content Adaptive Brightness Control and Color Enhancement
#define LCD_ST7365_RDCABC           (0x56)          // Read Content Adaptive Brightness Control
#define LCD_ST7365_WRCABCMB         (0x5E)          // Write CABC Minimum Brightness
#define LCD_ST7365_RDCABCMB         (0x5F)          // Read CABC Minimum Brightness
#define LCD_ST7365_RDABCSDR         (0x68)          // Read Automatic Brightness Control Self-Diagnostic Result
#define LCD_ST7365_RDID1            (0xDA)          // Read ID1
#define LCD_ST7365_RDID2            (0xDB)          // Read ID2
#define LCD_ST7365_RDID3            (0xDC)          // Read ID3
#define LCD_ST7365_RAMCTRL          (0xB0)          // RAM Control
#define LCD_ST7365_RGBCTRL          (0xB1)          // RGB Interface Control
#define LCD_ST7365_PORCTRL          (0xB2)          // Porch Setting
#define LCD_ST7365_FRCTRL1          (0xB3)          // Frame Rate Control 1 (0xIn partial mode/ idle colors)
#define LCD_ST7365_PARCTRL          (0xB5)          // Partial mode Control
#define LCD_ST7365_GCTRL            (0xB7)          // Gate Control
#define LCD_ST7365_GTADJ            (0xB8)          // Gate On Timing Adjustment
#define LCD_ST7365_DGMEN            (0xBA)          // Digital Gamma Enable
#define LCD_ST7365_VCOMS            (0xBB)          // VCOMS Setting
#define LCD_ST7365_LCMCTRL          (0xC0)          // LCM Control
#define LCD_ST7365_IDSET            (0xC1)          // ID Code Setting
#define LCD_ST7365_VDVVRHEN         (0xC2)          // VDV and VRH Command Enable
#define LCD_ST7365_VRHS             (0xC3)          // VRH Set
#define LCD_ST7365_VDVS             (0xC4)          // VDV Set
#define LCD_ST7365_VCMOFSET         (0xC5)          // VCOMS Offset Set
#define LCD_ST7365_FRCTRL2          (0xC6)          // Frame Rate Control in Normal Mode
#define LCD_ST7365_CABCCTRL         (0xC7)          // CABC Control
#define LCD_ST7365_REGSEL1          (0xC8)          // Register Value Selection 1
#define LCD_ST7365_REGSEL2          (0xCA)          // Register Value Selection 2
#define LCD_ST7365_PWMFRSEL         (0xCC)          // PWM Frequency Selection
#define LCD_ST7365_PWCTRL1          (0xD0)          // Power Control 1
#define LCD_ST7365_VAPVANEN         (0xD2)          // Enable VAP/VAN signal output
#define LCD_ST7365_CMD2EN           (0xDF)          // Command 2 Enable
#define LCD_ST7365_PVGAMCTRL        (0xE0)          // Positive Voltage Gamma Control
#define LCD_ST7365_NVGAMCTRL        (0xE1)          // Negative Voltage Gamma Control
#define LCD_ST7365_DGMLUTR          (0xE2)          // Digital Gamma Look-up Table for Red
#define LCD_ST7365_DGMLUTB          (0xE3)          // Digital Gamma Look-up Table for Blue
#define LCD_ST7365_GATECTRL         (0xE4)          // Gate Control
#define LCD_ST7365_SPI2EN           (0xE7)          // SPI2 Enable
#define LCD_ST7365_PWCTRL2          (0xE8)          // Power Control 2
#define LCD_ST7365_EQCTRL           (0xE9)          // Equalize time control
#define LCD_ST7365_PROMCTRL         (0xEC)          // Program Mode Control
#define LCD_ST7365_PROMEN           (0xFa)          // Program Mode Enable
#define LCD_ST7365_NVMSET           (0xFC)          // NVM Setting
#define LCD_ST7365_PROMACT          (0xFE)          // Program action
#endif
