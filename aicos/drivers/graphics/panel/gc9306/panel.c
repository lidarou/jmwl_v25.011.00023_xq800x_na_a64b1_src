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
#define AIC_THIS_FILE    0x0bdc37
#define LOG_MODULE    AMOD_GPAPH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>

#include "../../lcdc.h"
#include "lcd_gc9306_reg.h"
#include "../../lcd_panel.h"

struct cmd_para {
    int index;
    uint8_t cmd;
    int para_lenght;
    uint8_t para[15];
    uint32_t ms_delay;
}gc9306_init_cmd_para_array[] = {
    //------display control setting------------//
        {0 , 0xfe, 0, {},                                 5},//LCD_GC9306_INTER_REGISTER_ENABLE1
        {1 , 0xef, 0, {},                                 5},//LCD_GC9306_INTER_REGISTER_ENABLE2
        {4 , 0x36, 1, {0x48},                             0},//LCD_GC9306_MEMORY_ACCESS_CONTROL
    //------end display control setting----//
    //------Power Control Registers Initial----//
        {6 , 0xa4, 2, {0x44, 0x44},                       0},
        {7 , 0xa5, 2, {0x42, 0x42},                       0},
        {8 , 0xaa, 2, {0x88, 0x88},                       0},
        {9 , 0xe8, 2, {0x11, 0x0B},                       0},//LCD_GC9306_FRAME_RATE{11,0b}
        {10, 0xe3, 2, {0x01, 0x10},                       0},
        {11, 0xff, 1, {0x61},                             0},//framerate 0x0F->0x1F
        {12, 0xAC, 1, {0x00},                             0},
        {14, 0xAD, 1, {0x33},                             0},
        {13, 0xAE, 1, {0x2B},                             0},
        {15, 0xAF, 1, {0x55},                             0},
        {16, 0xA6, 2, {0x2a, 0x2a},                       0},//LCD_GC9306_POWER_CONTROL_2
        {17, 0xA7, 2, {0x2b, 0x2b},                       0},
        {18, 0xA8, 2, {0x18, 0x18},                       0},
        {19, 0xA9, 2, {0x2a, 0x2a},                       0},

        {15, 0x35, 1, {0x00},                             0},
        {19, 0x44, 2, {0x00, 0x0a},                       0},

    //------------gamma setting------------------//
        {22, 0xF0, 6, {0x2, 0x0, 0x0, 0x25, 0x26, 0x5},   0},
        {23, 0xF1, 6, {0x1, 0x3, 0x0, 0x33, 0x33, 0xA},   0},
        {24, 0xF2, 6, {0x5, 0x3, 0x3C, 0x4, 0x4, 0x3C},   0},
        {25, 0xF3, 6, {0x9, 0x4, 0x47, 0x3, 0x2, 0x52},   0},
        {26, 0xF4, 6, {0xA, 0x17, 0x16, 0x1F, 0x21, 0xF}, 0},
        {27, 0xF5, 6, {0x7, 0x12, 0x12, 0x1F, 0x20, 0xF}, 0},
    //--------end gamma setting--------------//
        {28, 0x11, 0, {},                               120},//LCD_GC9306_SLEEP_OUT_MODE
        {29, 0x3a, 0, {}, 0},//add for auto select color format
        {30, 0x29, 0, {},                               120},//LCD_GC9306_DISPLAY_ON
        {31, 0x2C, 0, {},                               120},//LCD_GC9306_MEMORY_WRITE
};

void lcd_panel_read_id(void)
{
    u32 val_before_reset;

    lcdc_cmd_read(0x04, &val_before_reset, true, 4);
    disp_log("gc9306 0x04 ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDA, &val_before_reset, false, 1);
    disp_log("gc9306 0xDA ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDB, &val_before_reset, false, 1);
    disp_log("gc9306 0xDB ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDC, &val_before_reset, false, 1);
    disp_log("gc9306 0xDC ID:0x%x\n", val_before_reset);
}

void lcd_panel_reset(void)
{
    lcdc_rstn(0);
    aic_display_wait_ms(10);
    lcdc_rstn(1);
    aic_display_wait_ms(150);
}

void lcd_panel_pixel_format_set()
{
    u32 cmd, val = 0;

    if (lcdc_config.aic_wire_type == AIC_LCDC_3WIRE_9BIT_2LANE) {
        cmd = 0xE9;
        if (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB565) {
            val = 0x8;
        } else if ((lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB666_PACK) || (lcdc_config.color_format ==
AIC_LCDC_COLOR_FORMAT_RGB666_UNPACK)){
            val = 0x9;
        } else if ((lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB888_PACK) || (lcdc_config.color_format ==
AIC_LCDC_COLOR_FORMAT_RGB888_UNPACK)){
            val = 0xC;
        } else {
            disp_log("gc9306 unsupport color format:%d\n", lcdc_config.color_format);
        }
    } else {
        /*disable panel 2lane*/
        lcdc_cmd_set(0xE9);
        lcdc_para_byte_set(0x0);

        cmd = 0x3a;
        if (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB565) {
            val = 0x5;
        } else if ((lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB666_PACK) || (lcdc_config.color_format ==
AIC_LCDC_COLOR_FORMAT_RGB666_UNPACK)){
            val = 0x6;
        } else {
            disp_log("gc9306 unsupport color format:%d\n", lcdc_config.color_format);
        }
    }

    lcdc_cmd_set(cmd);
    lcdc_para_byte_set(val);
}

int lcd_panel_sleep_wakeup(int is_sleep)
{
    return 0;
}

int lcd_panel_set_window(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
    int ret;

    if (lcdc_config.cmd_para_pack) {
        uint8_t para[4];

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
        //-----display window 240X320---------//
        lcdc_cmd_set(0x2a);
        lcdc_para_byte_set(left>>8);
        lcdc_para_byte_set(left&0xFF);
        lcdc_para_byte_set(right>>8);
        lcdc_para_byte_set(right&0xFF);
        lcdc_cmd_set(0x2b);
        lcdc_para_byte_set(top>>8);
        lcdc_para_byte_set(top&0xFF);
        lcdc_para_byte_set(bottom>>8);
        lcdc_para_byte_set(bottom&0xFF);
    }

    ret = lcdc_cmd_set(0x2C);//st7789 memory write

    return 0;
}

void lcd_panel_invalidate(void)
{
    lcd_panel_set_window(0, 0, 240 - 1, 320 - 1);
}

void lcd_panel_invalidate_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
    lcd_panel_set_window(left, top, right, bottom);
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
    //aic_mdelay(1600);
    //lcdc_cmd_set(0x29, &ret);
}

void lcd_panel_refresh()
{
    lcdc_frame_data_flip();//lcdc_frame_flip_mix_cmd();
}

void lcd_panel_refresh_sync()
{
    lcdc_frame_data_flip_sync();
}

void lcd_panel_init(void)
{
    int i, j, para_size;

    lcd_panel_reset();

    for (i = 0; i < sizeof(gc9306_init_cmd_para_array) / sizeof(struct cmd_para) ; i++) {

        /*auto select panel color format match witch lcdc color format*/
        if (gc9306_init_cmd_para_array[i].cmd == 0x3a) {
            lcd_panel_pixel_format_set();
            continue;
        }

        if (lcdc_config.cmd_para_pack) {
            if (gc9306_init_cmd_para_array[i].para_lenght != 0)
                lcdc_cmd_para_set(gc9306_init_cmd_para_array[i].cmd, gc9306_init_cmd_para_array[i].para,
gc9306_init_cmd_para_array[i].para_lenght);
            else
                lcdc_cmd_set(gc9306_init_cmd_para_array[i].cmd);
        } else {
            lcdc_cmd_set(gc9306_init_cmd_para_array[i].cmd);
            para_size = sizeof(gc9306_init_cmd_para_array[i].para);
            //AIC_PRINTF_GC9306("para size:%d\n para list:\n", para_size);
            for (j = 0; j < gc9306_init_cmd_para_array[i].para_lenght; j ++)
                lcdc_para_byte_set(gc9306_init_cmd_para_array[i].para[j]);
        }

        if(gc9306_init_cmd_para_array[i].ms_delay)
            aic_mdelay(gc9306_init_cmd_para_array[i].ms_delay);
    }

    lcd_panel_read_id();
    lcd_panel_set_window(0, 0, LCD_PANEL_WIDTH - 1, LCD_PANEL_HEIGHT - 1);
}
