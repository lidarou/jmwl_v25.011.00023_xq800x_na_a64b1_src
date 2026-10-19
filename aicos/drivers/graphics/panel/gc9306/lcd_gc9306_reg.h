/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the lcd gc9306 register map used by lcd gc9306 panel driver.
 */

#ifndef __LCD_GC9306_H__
#define __LCD_GC9306_H__

#define LCD_GC9306_READ_DISPLAY_IDENTIFICATION_INFORMATION         (0x04)
#define LCD_GC9306_READ_DISPLAY_STATUS                             (0x09)
#define LCD_GC9306_ENTER_SLEEP_MODE                                (0x10)
#define LCD_GC9306_SLEEP_OUT_MODE                                  (0x11)
#define LCD_GC9306_PARTIAL_MODE_ON                                 (0x12)
#define LCD_GC9306_NORMAL_DISPLAY_MODE_ON                          (0x13)
#define LCD_GC9306_DISPLAY_INVERSION_OFF                           (0x20)
#define LCD_GC9306_DISPLAY_INVERSION_ON                            (0x21)
#define LCD_GC9306_DISPLAY_OFF                                     (0x28)
#define LCD_GC9306_DISPLAY_ON                                      (0x29)
#define LCD_GC9306_COLUMN_ADDRESS_SET                              (0x2A)
#define LCD_GC9306_ROW_ADDRESS_SET                                 (0x2B)
#define LCD_GC9306_MEMORY_WRITE                                    (0x2C)
#define LCD_GC9306_PARTIAL_AREA                                    (0x30)
#define LCD_GC9306_VERTICAL_SCROLLING_DEFINITION                   (0x33)
#define LCD_GC9306_TEARING_EFFECT_LINE_OFF                         (0x34)
#define LCD_GC9306_TEARING_EFFECT_LINE_ON                          (0x35)
#define LCD_GC9306_MEMORY_ACCESS_CONTROL                           (0x36)
#define LCD_GC9306_VERTICAL_SCROLLING_START_ADDRESS                (0x37)
#define LCD_GC9306_IDLE_MODE_OFF                                   (0x38)
#define LCD_GC9306_IDLE_MODE_ON                                    (0x39)
#define LCD_GC9306_PIXEL_FORMAT_SET                                (0x3A)
#define LCD_GC9306_WRITE_MEMORY_CONTIUE                            (0x3C)
#define LCD_GC9306_SET_TEAR_SCANLINE                               (0x44)
#define LCD_GC9306_GET_SCANLINE                                    (0x45)
#define LCD_GC9306_WRITE_DISPLAY_BRIGHTNESS                        (0x51)
#define LCD_GC9306_WRITE_CTRL_DISPLAY                              (0x53)
#define LCD_GC9306_READ_ID1                                        (0xDA)
#define LCD_GC9306_READ_ID2                                        (0xDB)
#define LCD_GC9306_READ_ID3                                        (0xDC)
#define LCD_GC9306_RGB_INTERFACE_SIGNAL_CONTROL                    (0xB0)
#define LCD_GC9306_BLANKING_PORCH_CONTROL                          (0xB5)
#define LCD_GC9306_DISPLAY_FUNCTION_CONTROL                        (0xB6)
#define LCD_GC9306_INTERFACE_CONTROL                               (0xF6)
#define LCD_GC9306_FRAME_RATE                                      (0xE8)
#define LCD_GC9306_SPI_2DATA_CONTROL                               (0xE9)
#define LCD_GC9306_POWER_CONTROL_1                                 (0xA3)
#define LCD_GC9306_POWER_CONTROL_2                                 (0xA6)
#define LCD_GC9306_POWER_CONTROL_3                                 (0xA7)
#define LCD_GC9306_POWER_CONTROL_4                                 (0xA8)
#define LCD_GC9306_POWER_CONTROL_5                                 (0xA9)
#define LCD_GC9306_POWER_CONTROL_6                                 (0xEC)
#define LCD_GC9306_POWER_CONTROL_7                                 (0xA4)
#define LCD_GC9306_INTER_REGISTER_ENABLE1                          (0xFE)
#define LCD_GC9306_INTER_REGISTER_ENABLE2                          (0xEF)
#define LCD_GC9306_SET_GAMMA1                                      (0xF0)
#define LCD_GC9306_SET_GAMMA2                                      (0xF1)
#define LCD_GC9306_SET_GAMMA3                                      (0xF2)
#define LCD_GC9306_SET_GAMMA4                                      (0xF3)
#define LCD_GC9306_SET_GAMMA5                                      (0xF4)
#define LCD_GC9306_SET_GAMMA6                                      (0xF5)

#endif /*__LCD_GC9306_H__*/