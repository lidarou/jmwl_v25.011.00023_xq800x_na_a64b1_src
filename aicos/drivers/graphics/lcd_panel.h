/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the lcd st7789 register map used by lcd st7789 panel driver.
 */
#ifndef __LCD_PANEL_H__
#define __LCD_PANEL_H__
void lcd_panel_init(void);
int lcd_panel_sleep_wakeup(int is_sleep);
void lcd_panel_invalidate(void);
void lcd_panel_invalidate_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
void lcd_panel_framebuffer_init(u32 frame_base, u32 width,u32 height);
void lcd_panel_refresh(void);
void lcd_panel_refresh_sync(void);
void display_reset();

#endif /*__LCD_PANEL_H__*/
