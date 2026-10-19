
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x137407
#define LOG_MODULE    AMOD_GPAPH
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>

#include "../../lcdc.h"
#include "../../lcd_panel.h"
#include "cache.h"

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
    aicx_error(0x135f47, "ssd", "%s %s(%d) \n",__FILE__, __func__, __LINE__);
    return 0;
}

void lcd_panel_invalidate(void)
{
    aicx_error(0x135f47, "ssd", "%s %s(%d) \n",__FILE__, __func__, __LINE__);
    //lcd_panel_set_window(0, 0, 240 - 1, 320 - 1);
}

void lcd_panel_invalidate_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
    aicx_error(0x135f47, "ssd", "%s %s(%d) \n",__FILE__, __func__, __LINE__);

    //lcd_panel_set_window(left, top, right, bottom);
}

u32 cur_frame_base = 0;
void lcd_panel_framebuffer_init(u32 frame_base, u32 width,u32 height)
{
    lcdc_frame_data_flip_init((void *)frame_base, width, height);
    cur_frame_base = frame_base;
}

#define MAX_PAGE_NUM            8
#define PIXEL_WRITE_MIN_BITES   8
void lcd_panel_refresh()
{
    int i = 0;

    for (i = 0; i < MAX_PAGE_NUM; i++) {
        lcdc_cmd_set(0xB0 + i);      //page address
        lcdc_cmd_set(0x10);          //start line adress
        lcdc_cmd_set(0x04);          //start line adress
        lcdc_frame_data_flip_init((void *)(cur_frame_base + i * LCD_PANEL_WIDTH * PIXEL_WRITE_MIN_BITES), LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT / MAX_PAGE_NUM);
        lcdc_frame_data_flip();
    }
}

void lcd_panel_refresh_sync()
{
    lcd_panel_refresh();
}

void lcd_panel_init(void)
{
    lcd_panel_reset();//reset lcd//zfb20230828test
    lcdc_cmd_set(0xE2);     //set softreset
    lcdc_cmd_set(0xF8);     //set booster
    lcdc_cmd_set(0x01);     //5X
    lcdc_cmd_set(0x24);     //set resistor ratio
    lcdc_cmd_set(0x2F);      //select power circuit
    lcdc_cmd_set(0xA3);      //set bias: 1/7 bias
    lcdc_cmd_set(0x81);
    lcdc_cmd_set(0x1A);      //contrast value
    lcdc_cmd_set(0xA1);      //set SEG direction//0xA0//zfb20230830test
    lcdc_cmd_set(0xC8);      //set COM direction
    lcdc_cmd_set(0x40);      //start line adress
    lcdc_cmd_set(0xA6); //0xA6为设置字体为黑色，背景为白色---////--0xA7为设置字体为白色，背景为黑色---//
    lcdc_cmd_set(0xA4); //
    lcdc_cmd_set(0xB0);      //page address
    lcdc_cmd_set(0xEE);      //column address increment
    lcdc_cmd_set(0xAF);      //display on
    aic_mdelay(50);   //50us
}

static void pixel_swap(uint8_t *pixel_a, uint8_t *pixel_b)
{
    uint16_t temp;
    temp = *pixel_b;
    *pixel_b = *pixel_a;
    *pixel_a = temp;
}

static void lcd_buffer_fill_rgb565_chess(uint8_t pframe[LCD_PANEL_HEIGHT][LCD_PANEL_WIDTH], uint8_t rgb565_color_a, uint8_t rgb565_color_b)
{
    int i, j;
    uint8_t pixel_a,pixel_b,pixel_column_a,pixel_column_b;
    int chess_width = 8;
    pixel_a = rgb565_color_a;
    pixel_b = rgb565_color_b;
    pixel_column_a = rgb565_color_b;
    pixel_column_b = rgb565_color_a;

    for( i = 0; i < LCD_PANEL_HEIGHT; i++) {//64
        if ((i % chess_width) == 0) {
            pixel_swap(&pixel_column_a, &pixel_column_b);
            if (i != 0)
                pixel_swap(&pixel_a, &pixel_b);
        }

        for (j = 0; j < LCD_PANEL_WIDTH; j++) {//16
            if((j % chess_width) == 0)
                pixel_swap(&pixel_a, &pixel_b);

            pframe[i][j] = pixel_a;
        }

        pixel_a = pixel_column_a;
        pixel_b = pixel_column_b;
    }
}

int show_chess_cnt = 0;
int show_st7567_chess_128_64(void)
{
    int ret;
    char *chess_buf = NULL;

    if (!chess_buf) {
        chess_buf = aos_dma_malloc(LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT);//240*320*2 RGB565
        if (!chess_buf) {
            aicx_warn(0x0382d0, "", "allocate logo buf failed\n");
            return AIC_ENOMEM;
        }
    }

    if (show_chess_cnt % 2)
        lcd_buffer_fill_rgb565_chess((uint8_t (*)[LCD_PANEL_WIDTH])chess_buf, 0x0, 0xFF);
    else
        lcd_buffer_fill_rgb565_chess((uint8_t (*)[LCD_PANEL_WIDTH])chess_buf, 0xFF, 0x0);

//    display_refresh_sync((u32)chess_buf);

    aos_free(chess_buf);

//    ret = aic_bl_brightness_set(80);

    show_chess_cnt++;

    return ret;
}
