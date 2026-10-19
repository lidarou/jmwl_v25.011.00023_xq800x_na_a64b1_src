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
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x010648
#define LOG_MODULE    AMOD_GPAPH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>

#include "../../lcdc.h"
#include "../../lcd_panel.h"

typedef union {
    struct {
        u32 resv   :2;
        u32 mh     :1; // display data latch order
        u32 rgb    :1; // rgb order
        u32 ml     :1; // line address order
        u32 mv     :1; // x/y exchanged
        u32 mx     :1; // horizontal mirror
        u32 my     :1; // vertical mirror
    } bitsval;
    u32 dataval;
} MADCTL_U;

static MADCTL_U panel_madc_u;

void lcd_panel_set_direction(LCD_DIRECT_ANGLE direct_type)
{
    switch(direct_type)
    {
        case LCD_DIRECT_ROT_0:
            panel_madc_u.bitsval.mx = 0;
            panel_madc_u.bitsval.my = 0;
            panel_madc_u.bitsval.mv = 0;
            break;

        case LCD_DIRECT_ROT_90:
            panel_madc_u.bitsval.mx = 1;
            panel_madc_u.bitsval.my = 0;
            panel_madc_u.bitsval.mv = 1;
            break;

        case LCD_DIRECT_ROT_180:
            panel_madc_u.bitsval.mx = 1;
            panel_madc_u.bitsval.my = 1;
            panel_madc_u.bitsval.mv = 0;
            break;

        case LCD_DIRECT_ROT_270:
            panel_madc_u.bitsval.mx = 0;
            panel_madc_u.bitsval.my = 0;
            panel_madc_u.bitsval.mv = 1;
            break;

        default://Normal
            panel_madc_u.bitsval.mx = 0;
            panel_madc_u.bitsval.my = 0;
            panel_madc_u.bitsval.mv = 0;
            break;
    }

    lcdc_cmd_set(0x36);

    lcdc_para_byte_set(panel_madc_u.dataval);

    lcdc_cmd_set(0x2c);
}

static void lcd_panel_reset(void)
{
    disp_log("%s%(d)\n", __func__, __LINE__);
    lcdc_rstn(0);
    aic_display_wait_ms(10);
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
#if 0
    u32 status = 0;

    lcdc_cmd_read(0x09, &status, true, 4);
    aicx_warn(0x07fc97, "x", "lcd panel status:0x%x\n", status);
    if (status != 0xa4632600)//panel normal status 0xa4632600
        display_reset();
    else
        lcd_panel_set_window(0, 0, LCD_PANEL_WIDTH - 1, LCD_PANEL_HEIGHT - 1);
#endif
    //lcd_panel_invalidate();
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

static void lcd_panel_read_id(void)
{
    u32 val_before_reset;
    //return;
    lcdc_cmd_read(0x04, &val_before_reset, true, 4);
    disp_log("st7789 0x04 ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDA, &val_before_reset, false, 1);
    disp_log("st7789 0xDA ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDB, &val_before_reset, false, 1);
    disp_log("st7789 0xDB ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDC, &val_before_reset, false, 1);
    disp_log("st7789 0xDC ID:0x%x\n", val_before_reset);
}

void lcd_panel_init(void)
{
    lcd_panel_reset();
    lcdc_cmd_set(0xfe);
    lcdc_cmd_set(0xef);
    lcdc_cmd_set(0x36);
    lcdc_para_byte_set(0x48); 
    lcdc_cmd_set(0x3a);
    lcdc_para_byte_set(0x05);

    //=============		
    lcdc_cmd_set(0x21);
            
    lcdc_cmd_set(0x86);
    lcdc_para_byte_set(0x98);//98
    lcdc_cmd_set(0x89);
    lcdc_para_byte_set(0x03);


    lcdc_cmd_set(0x8b);
    lcdc_para_byte_set(0x84);
    lcdc_cmd_set(0x8d);
    lcdc_para_byte_set(0x33);

    lcdc_cmd_set(0x8e);
    lcdc_para_byte_set(0x0f);
    lcdc_cmd_set(0xe8);
    lcdc_para_byte_set(0x12);
    lcdc_para_byte_set(0x00);	
        
        
    lcdc_cmd_set(0xff);
    lcdc_para_byte_set(0x62);


    lcdc_cmd_set(0x99);
    lcdc_para_byte_set(0x3e);
    lcdc_cmd_set(0x9d);	
    lcdc_para_byte_set(0x4b);
        
    lcdc_cmd_set(0x98);//bvdd 3倍压 
    lcdc_para_byte_set(0x3e);
    lcdc_cmd_set(0x9c);	
    lcdc_para_byte_set(0x4b);//20210104
        
    lcdc_cmd_set(0xc3);	
    lcdc_para_byte_set(0x38);//1a  38:5.2  45
    lcdc_cmd_set(0xc4);	 
    lcdc_para_byte_set(0x34);//34   20
    lcdc_cmd_set(0xc9);	
    lcdc_para_byte_set(0x08);//0A
    lcdc_cmd_set(0xF0);
    lcdc_para_byte_set(0x83);
    lcdc_para_byte_set(0x00);
    lcdc_para_byte_set(0x1F);
    lcdc_para_byte_set(0x15);
    lcdc_para_byte_set(0x0B);
    lcdc_para_byte_set(0x34);

    lcdc_cmd_set(0xF1);
    lcdc_para_byte_set(0x45);
    lcdc_para_byte_set(0x97);
    lcdc_para_byte_set(0xB6);
    lcdc_para_byte_set(0x18);
    lcdc_para_byte_set(0x1B);
    lcdc_para_byte_set(0xBF);

    lcdc_cmd_set(0xF2);
    lcdc_para_byte_set(0x4B);
    lcdc_para_byte_set(0x00);
    lcdc_para_byte_set(0x00);
    lcdc_para_byte_set(0x1D);
    lcdc_para_byte_set(0x0F);
    lcdc_para_byte_set(0x40);

    lcdc_cmd_set(0xF3);
    lcdc_para_byte_set(0x4E);
    lcdc_para_byte_set(0x95);
    lcdc_para_byte_set(0x72);
    lcdc_para_byte_set(0x19);
    lcdc_para_byte_set(0x1B);
    lcdc_para_byte_set(0xAF);

    //2019-12-07 16:12:44
    lcdc_cmd_set(0x35);
    lcdc_para_byte_set(0x00);

    lcdc_cmd_set(0x44);
    lcdc_para_byte_set(0x00);
    lcdc_para_byte_set(0x0a);
    lcdc_cmd_set(0x11);
    aic_mdelay(120);
    lcdc_cmd_set(0x29);
    lcdc_cmd_set(0x2c);

    lcd_panel_read_id();
    lcd_panel_rotation_invalidate_rect(0, 0, LCD_PANEL_WIDTH - 1, LCD_PANEL_HEIGHT - 1, LCD_PANEL_ROTATION_ANGLE);
}
