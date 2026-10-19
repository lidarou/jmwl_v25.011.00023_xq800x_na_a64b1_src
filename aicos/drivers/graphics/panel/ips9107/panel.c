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
#define AIC_THIS_FILE    0x033ad5
#define LOG_MODULE    AMOD_GPAPH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>

#include "../../lcdc.h"
#include "lcd_ips9107_reg.h"
#include "../../lcd_panel.h"

struct cmd_para {
    int index;
    uint8_t cmd;
    int para_lenght;
    uint8_t para[15];
    uint32_t ms_delay;
}ips9107_init_cmd_para_array[] = {
        {0 , 0xfe, 0, {}, 0},
        {1 , 0xef, 0, {}, 0},
        {2 , 0xb0, 1, {0xc0}, 0},
        {3 , 0xb2, 1, {0x2f}, 0},
        {4 , 0xb3, 1, {0x03}, 0},
        {5 , 0xb6, 1, {0x19}, 0},
        {6 , 0xb7, 1, {0x01}, 0},
        {7 , 0xac, 1, {0xc8}, 0},
        {8 , 0xab, 1, {0x07}, 0},
        {9 , 0xb4, 1, {0x04}, 0},
        {10, 0xa8, 1, {0x0c}, 0},
		{11, 0x35, 1, {}, 0},//framerate 30 fps        
        {12, 0x3a, 1, {0x05}, 0},//framerate 30 fps
        {13, 0xb8, 1, {0x08}, 0},
        {14, 0xe8, 1, {0x20}, 0},
        {15, 0xe9, 1, {0x44}, 0},
        {16, 0xea, 1, {0x44}, 0},//LCD_IPS9107_NVGAMCTRL request delay 5ms after setting
        {17, 0xeb, 1, {0xb0}, 0},
		{18, 0xed, 1, {0x03}, 0},
		{19, 0xc6, 1, {0x3d}, 0},
		{20, 0xc7, 1, {0x10}, 0},
		{21, 0xf0, 14, {0x0f,0x20,0x25,0x35,0x26,0x3f,0x30,0x60,0x26,0x03,0x00,0x09,0x13,0x1a}, 0},
		{22, 0xf1, 14, {0x00,0x28,0x7f,0x39,0xa5,0x0f,0x30,0x60,0x00,0x05,0x01,0x0f,0x0f,0x1f}, 0},
		{23, 0x11, 0, {}, 10},
		{24, 0x29, 0, {}, 10},
		{25, 0x2c, 0, {}, 10},
};

static void lcd_panel_reset(void)
{
    disp_log("%s%(d)\n", __func__, __LINE__);
    lcdc_rstn(0);
    aic_display_wait_ms(10);
    lcdc_rstn(1);
    aic_display_wait_ms(150);
}

void lcd_panel_pixel_format_set()
{
    u32 cmd, val;

    cmd = LCD_IPS9107_COLMOD;
    if (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB565) {
        val = 0x5;
    } else if ((lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB444_PACK) ||
              (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB444_UNPACK) ||
              (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB444_COMPACT)) {
        val = 0x3;
    } else if ((lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB666_PACK) ||
              (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB666_UNPACK)) {
        val = 0x6;
    } else {
        disp_log("st7789 unsupport color format:%d\n", lcdc_config.color_format);
        return;
    }

    lcdc_cmd_set(cmd);
    lcdc_para_byte_set(val);
    aic_mdelay(200);

    if (lcdc_config.aic_wire_type == AIC_LCDC_3WIRE_9BIT_2LANE) {
        lcdc_cmd_set(LCD_IPS9107_SPI2EN);
        lcdc_para_byte_set(0x11);
        aic_mdelay(200);
    }

}


int lcd_panel_sleep_wakeup(int is_sleep)
{
    return 0;
}

static int lcd_panel_set_window(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
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

void lcd_panel_framebuffer_init(u32 frame_base, u32 width,u32 height)
{
    disp_log("%s(%d) frame_addr:0x%x start\n", __func__, __LINE__, frame_base);
    //lcd_panel_invalidate();
    lcdc_frame_data_flip_init((void *)frame_base, width, height);
    //aic_mdelay(1600);
    //lcdc_cmd_set(0x29, &ret);
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
    disp_log("ips9107 0x04 ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDA, &val_before_reset, false, 1);
    disp_log("ips9107 0xDA ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDB, &val_before_reset, false, 1);
    disp_log("ips9107 0xDB ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDC, &val_before_reset, false, 1);
    disp_log("ips9107 0xDC ID:0x%x\n", val_before_reset);
}

void lcd_panel_init(void)
{
    int i, j, para_size;

    lcd_panel_reset();

    for (i = 0; i < sizeof(ips9107_init_cmd_para_array) / sizeof(struct cmd_para) ; i++) {

        if (ips9107_init_cmd_para_array[i].cmd == LCD_IPS9107_COLMOD) {
            lcd_panel_pixel_format_set();
            continue;
        }

        if (lcdc_config.cmd_para_pack) {
            if (ips9107_init_cmd_para_array[i].para_lenght != 0)
                lcdc_cmd_para_set(ips9107_init_cmd_para_array[i].cmd, ips9107_init_cmd_para_array[i].para,
ips9107_init_cmd_para_array[i].para_lenght);
            else
                lcdc_cmd_set(ips9107_init_cmd_para_array[i].cmd);
        } else {
            lcdc_cmd_set(ips9107_init_cmd_para_array[i].cmd);
            para_size = sizeof(ips9107_init_cmd_para_array[i].para);
            //disp_log("para size:%d\n para list:\n", para_size);
            for (j = 0; j < ips9107_init_cmd_para_array[i].para_lenght; j ++)
                lcdc_para_byte_set(ips9107_init_cmd_para_array[i].para[j]);
        }

        if(ips9107_init_cmd_para_array[i].ms_delay)
            aic_mdelay(ips9107_init_cmd_para_array[i].ms_delay);
        //aic_mdelay(200);
        //lcdc_cmd_read(ips9107_init_cmd_para_array[i].cmd, &ret);
    }

    lcd_panel_read_id();
    lcd_panel_set_window(0, 0, LCD_PANEL_WIDTH - 1, LCD_PANEL_HEIGHT - 1);
}

