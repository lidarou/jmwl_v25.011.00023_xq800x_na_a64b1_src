/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of lcd panel module.
 *
*/
#include <aic_errno.h>

#include "../../lcdc.h"
#include "../../lcd_panel.h"

//#define  LCD_WIDTH 128     //LCD_PANEL_WIDTH
//#define  LCD_HEIGHT 160    //LCD_PANEL_HEIGHT

static void lcd_panel_reset(void)
{
    disp_log("ST7735 %s(%d)\n", __func__, __LINE__);
    lcdc_rstn(0);
    aic_display_wait_ms(50);
    lcdc_rstn(1);
    aic_display_wait_ms(150);
}

int lcd_panel_sleep_wakeup(int is_sleep)
{
    return 0;
}

int lcd_panel_set_window(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
    int ret;
    uint8_t para[4];

    #if 0
	lcdc_cmd_set(0x2a);                   // set hori start , end (left, right)
    lcdc_para_byte_set((left >> 8) & 0xff);  // left high 8 b
    lcdc_para_byte_set(left & 0xff);         // left low 8 b
    lcdc_para_byte_set((right >> 8) & 0xff); // right high 8 b
    lcdc_para_byte_set(right & 0xff);        // right low 8 b

    lcdc_cmd_set(0x2b);                    // set vert start , end (top, bot)
    lcdc_para_byte_set((top >> 8) & 0xff);    // top high 8 b
    lcdc_para_byte_set(top & 0xff);           // top low 8 b
    lcdc_para_byte_set((bottom >> 8) & 0xff); // bot high 8 b
    lcdc_para_byte_set(bottom & 0xff);        // bot low 8 b
	#else
    if (lcdc_config.cmd_para_pack) {
        para[0] = left >> 8;
        para[1] = left & 0xFF;
        para[2] = right >> 8;
        para[3] = right & 0xFF;
        ret = lcdc_cmd_para_set(0x2A, para, 4);

        para[0] = top >> 8;
        para[1] = top & 0xFF;
        para[2] = bottom >> 8;
        para[3] = bottom & 0xFF;
        ret = lcdc_cmd_para_set(0x2B, para, 4);
    } else {
        lcdc_cmd_set(0x2A);
        lcdc_para_byte_set(left >> 8);
        lcdc_para_byte_set(left & 0xFF);
        lcdc_para_byte_set(right >> 8);
        lcdc_para_byte_set(right & 0xFF);

        lcdc_cmd_set(0x2B);
        lcdc_para_byte_set(top >> 8);
        lcdc_para_byte_set(top & 0xFF);
        lcdc_para_byte_set(bottom >> 8);
        lcdc_para_byte_set(bottom & 0xFF);
    }
	#endif

    ret = lcdc_cmd_set(0x2C);//st7789 memory write

    return ret;
}

void lcd_panel_invalidate(void)
{
    disp_log("%s(%d)\n", __func__, __LINE__);
    lcd_panel_set_window(0, 0, LCD_PANEL_WIDTH - 1, LCD_PANEL_HEIGHT - 1);
}

void lcd_panel_invalidate_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
    lcd_panel_set_window(left, top, right, bottom);
}

static void lcd_panel_rotation_invalidate_rect(u16 left, u16 top, u16 right, u16 bottom, LCD_DIRECT_ANGLE angle)
{
    switch (angle)
    {
    case LCD_DIRECT_ROT_0:
        lcd_panel_set_window(left, top, right, bottom);
        break;
    case LCD_DIRECT_ROT_90:
        lcd_panel_set_window(left, top, bottom, right);
        break;
    case LCD_DIRECT_ROT_180:
        lcd_panel_set_window(left, top, right, bottom);
        break;
    case LCD_DIRECT_ROT_270:
        lcd_panel_set_window(left, top, bottom, right);
        break;
    default:
        lcd_panel_set_window(left, top, right, bottom);
        break;
    }
}
void lcd_panel_framebuffer_init(u32 frame_base, u32 width,u32 height)
{
    disp_log("%s(%d) frame_addr:0x%x start\n", __func__, __LINE__, frame_base);
    lcdc_frame_data_flip_init((void *)frame_base, width, height);
}

void lcd_panel_refresh()
{
    disp_log("%s\n", __func__);
    lcdc_frame_data_flip();
}

void lcd_panel_refresh_sync()
{
    lcdc_frame_data_flip_sync();
}

#include <gpio_inc.h>
void lcd_panel_init(void)
{
    aic_gpio_output(14, GPIO_OUTPUT_LVL_HIGH);
#if 1
    lcd_panel_reset();
    lcdc_cmd_set(0x11); //Sleep out
    aic_mdelay(60); //Delay 120ms

	lcdc_cmd_set(0xF0);
	lcdc_para_byte_set(0x11);

	lcdc_cmd_set(0xD6);
	lcdc_para_byte_set(0xCB);

	//------------------------------------ST7735S Frame Rate-----------------------------------------//
	lcdc_cmd_set(0xB1);
	lcdc_para_byte_set(0x05);
	lcdc_para_byte_set(0x3C);
	lcdc_para_byte_set(0x3C);
	lcdc_cmd_set(0xB2);
	lcdc_para_byte_set(0x05);
	lcdc_para_byte_set(0x3C);
	lcdc_para_byte_set(0x3C);
	lcdc_cmd_set(0xB3);
	lcdc_para_byte_set(0x05);
	lcdc_para_byte_set(0x3C);
	lcdc_para_byte_set(0x3C);
	lcdc_para_byte_set(0x05);
	lcdc_para_byte_set(0x3C);
	lcdc_para_byte_set(0x3C);
	//------------------------------------End ST7735S Frame Rate-----------------------------------------//
	lcdc_cmd_set(0xB4); //Dot inversion
	lcdc_para_byte_set(0x03);
	lcdc_para_byte_set(0x02);
	lcdc_cmd_set(0xC0);
	lcdc_para_byte_set(0x64);
	lcdc_para_byte_set(0x04);
	lcdc_para_byte_set(0x84);
	lcdc_cmd_set(0xC1);
	lcdc_para_byte_set(0xC9);
	lcdc_cmd_set(0xC2);
	lcdc_para_byte_set(0x6D);
	lcdc_para_byte_set(0x40);
	lcdc_cmd_set(0xC3);
	lcdc_para_byte_set(0x8D);
	lcdc_para_byte_set(0x2A);
	lcdc_cmd_set(0xC4);
	lcdc_para_byte_set(0x8D);
	lcdc_para_byte_set(0xEE);
	//---------------------------------End ST7735S Power Sequence-------------------------------------//
	lcdc_cmd_set(0xC5); //VCOM
	lcdc_para_byte_set(0x1A);
	//------------------------------------ST7735S Gamma Sequence-----------------------------------------//
	lcdc_cmd_set(0xE0);
	lcdc_para_byte_set(0x07);
	lcdc_para_byte_set(0x3F);
	lcdc_para_byte_set(0x02);
	lcdc_para_byte_set(0x0A);
	lcdc_para_byte_set(0x10);
	lcdc_para_byte_set(0x0E);
	lcdc_para_byte_set(0x0A);
	lcdc_para_byte_set(0x10);
	lcdc_para_byte_set(0x12);
	lcdc_para_byte_set(0x13);
	lcdc_para_byte_set(0x20);
	lcdc_para_byte_set(0x2F);
	lcdc_para_byte_set(0x00);
	lcdc_para_byte_set(0x04);
	lcdc_para_byte_set(0x09);
	lcdc_para_byte_set(0x3F);
	lcdc_cmd_set(0xE1);
	lcdc_para_byte_set(0x04);
	lcdc_para_byte_set(0x3F);
	lcdc_para_byte_set(0x04);
	lcdc_para_byte_set(0x08);
	lcdc_para_byte_set(0x12);
	lcdc_para_byte_set(0x10);
	lcdc_para_byte_set(0x10);
	lcdc_para_byte_set(0x19);
	lcdc_para_byte_set(0x1D);
	lcdc_para_byte_set(0x1F);
	lcdc_para_byte_set(0x28);
	lcdc_para_byte_set(0x37);
	lcdc_para_byte_set(0x00);
	lcdc_para_byte_set(0x01);
	lcdc_para_byte_set(0x0B);
	lcdc_para_byte_set(0x3F);

	//lcdc_cmd_set(0x2A);
	//lcdc_para_byte_set(0x00);
	//lcdc_para_byte_set(0x00);
	//lcdc_para_byte_set(0x00);
	//lcdc_para_byte_set(0x7F);

	//lcdc_cmd_set(0x2B);
	//lcdc_para_byte_set(0x00);
	//lcdc_para_byte_set(0x00);
	//lcdc_para_byte_set(0x00);
	//lcdc_para_byte_set(0x9F);

	//------------------------------------End ST7735S Gamma Sequence-----------------------------------------//
	lcdc_cmd_set(0x35);
	lcdc_para_byte_set(0x00);

	lcdc_cmd_set(0x3A); //65k mode
	lcdc_para_byte_set(0x05);

	lcdc_cmd_set(0x36); //MX, MY, RGB mode
	lcdc_para_byte_set(0x60);  //0xD0 LCD_DIRECT_ROT_90   0x60  D0
	lcdc_cmd_set(0x21);

	lcdc_cmd_set(0x29); //Display on
    lcd_panel_rotation_invalidate_rect(0, 0, LCD_PANEL_WIDTH - 1, LCD_PANEL_HEIGHT - 1, LCD_PANEL_ROTATION_ANGLE);
	//lcd_panel_set_direction(LCD_PANEL_ROTATION_ANGLE);
#else
    lcd_panel_reset();
    lcdc_cmd_set(0x11); //Sleep out
    aic_mdelay(5); //Delay 120ms
//------------------------------------ST7735S Frame Rate----------------------//
    lcdc_cmd_set(0xB1);
    lcdc_para_byte_set(0x05);
    lcdc_para_byte_set(0x3A);
    lcdc_para_byte_set(0x3A);
    lcdc_cmd_set(0xB2);
    lcdc_para_byte_set(0x05);
    lcdc_para_byte_set(0x3A);
    lcdc_para_byte_set(0x3A);
    lcdc_cmd_set(0xB3);
    lcdc_para_byte_set(0x05);
    lcdc_para_byte_set(0x3A);
    lcdc_para_byte_set(0x3A);
    lcdc_para_byte_set(0x05);
    lcdc_para_byte_set(0x3A);
    lcdc_para_byte_set(0x3A);
//------------------------------------End ST7735S Frame Rate------------------//
    lcdc_cmd_set(0xB4); //Dot inversion
    lcdc_para_byte_set(0x03);
//------------------------------------ST7735S Power Sequence-----------------------//
    lcdc_cmd_set(0xC0);
    lcdc_para_byte_set(0x28);
    lcdc_para_byte_set(0x08);
    lcdc_para_byte_set(0x84);
    lcdc_cmd_set(0xC1);
    lcdc_para_byte_set(0XC0);
    lcdc_cmd_set(0xC2);
    lcdc_para_byte_set(0x0D);
    lcdc_para_byte_set(0x00);
    lcdc_cmd_set(0xC3);
    lcdc_para_byte_set(0x8D);
    lcdc_para_byte_set(0x2A);
    lcdc_cmd_set(0xC4);
    lcdc_para_byte_set(0x8D);
    lcdc_para_byte_set(0xEE);
//---------------------------------End ST7735S Power Sequence---------------------------//
    lcdc_cmd_set(0xC5); //VCOM
    lcdc_para_byte_set(0x0C);
    lcdc_cmd_set(0x36); //MX, MY, RGB mode
    lcdc_para_byte_set(0xC8);
//------------------------------ST7735S Gamma Sequence----------------------------------//
    lcdc_cmd_set(0xE0);
    lcdc_para_byte_set(0x05);
    lcdc_para_byte_set(0x1A);
    lcdc_para_byte_set(0x0C);
    lcdc_para_byte_set(0x0E);
    lcdc_para_byte_set(0x3A);
    lcdc_para_byte_set(0x34);
    lcdc_para_byte_set(0x2D);
    lcdc_para_byte_set(0x2F);
    lcdc_para_byte_set(0x2D);
    lcdc_para_byte_set(0x2A);
    lcdc_para_byte_set(0x2F);
    lcdc_para_byte_set(0x3C);
    lcdc_para_byte_set(0x00);
    lcdc_para_byte_set(0x01);
    lcdc_para_byte_set(0x02);
    lcdc_para_byte_set(0x10);
    lcdc_cmd_set(0xE1);
    lcdc_para_byte_set(0x04);
    lcdc_para_byte_set(0x1B);
    lcdc_para_byte_set(0x0D);
    lcdc_para_byte_set(0x0E);
    lcdc_para_byte_set(0x2D);
    lcdc_para_byte_set(0x29);
    lcdc_para_byte_set(0x24);
    lcdc_para_byte_set(0x29);
    lcdc_para_byte_set(0x28);
    lcdc_para_byte_set(0x26);
    lcdc_para_byte_set(0x31);
    lcdc_para_byte_set(0x3B);
    lcdc_para_byte_set(0x00);
    lcdc_para_byte_set(0x00);
    lcdc_para_byte_set(0x03);
    lcdc_para_byte_set(0x12);
//------------------------------------End ST7735S Gamma Sequence-----------------//
    lcdc_cmd_set(0xFC); //Enable Gate Pump Clock Frequency Variable
    lcdc_para_byte_set(0x8C);
    lcdc_cmd_set(0x3A); //65k mode
    lcdc_para_byte_set(0x05);

    /*enable v-blank te*/
    lcdc_cmd_set(0x35);
    lcdc_para_byte_set(0x00);
    lcdc_cmd_set(0x29); //Display on
    lcd_panel_read_id();
    lcd_panel_set_direction(LCD_PANEL_ROTATION_ANGLE);
    lcd_panel_rotation_invalidate_rect(0, 0, LCD_PANEL_WIDTH - 1, LCD_PANEL_HEIGHT - 1, LCD_PANEL_ROTATION_ANGLE);
	#endif
}
