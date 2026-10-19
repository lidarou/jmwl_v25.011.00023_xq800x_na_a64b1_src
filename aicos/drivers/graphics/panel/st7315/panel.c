
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f2cf8
#define LOG_MODULE    AMOD_GPAPH
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include "gpio_inc.h"

#include "../../lcdc.h"
#include "../../lcd_panel.h"
#include "cache.h"

struct cmd_para {
    int index;
    uint8_t cmd;
    int para_lenght;
    uint8_t para[15];
    uint32_t ms_delay;
}st7315_init_cmd_para_array[] = {
		 {0 , 0xAE,    0,  {},0},
		 {1 , 0x0C,    0,  {},0},
		 {2 , 0x11,    0,  {},0},
		 {3 , 0x40,    0,  {},0},
		 {4 , 0xa4,    0,  {},0},
		 {5 , 0xa6,    0,  {},0},
		 {6 , 0xd5,    0,  {},0},
		 {7 , 0xf0,    0,  {},0},
		 {8 , 0xa8,    0,  {},0},
		 {9 , 0x27,    0,  {},0},
		 {10, 0xd3,    0,  {},0},
		 {11, 0x00,    0,  {},0},

		 {12, 0x20,    0,  {},0},
		 {13, 0x00,    0,  {},0},

		 {13, 0x21,    0,  {},0},
		 {13, 0x1c,     0,  {},0},
		 {13, 0x63,     0,  {},0},

		 {13, 0x22,    0,  {},0},
		 {13, 0x0,     0,  {},0},
		 {13, 0x04,     0,  {},0},

		 {14, 0xa1,    0,  {},0},
		 {15, 0xc8,    0,  {},0},
		 {17, 0xda,    0,  {},0},
		 {18, 0x12,    0,  {},0},
		 {19, 0xad,    0,  {},0},
		 {20, 0x30,    0,  {},0},
		 {21, 0x81,    0,  {},0},
		 {22, 0x1f,    0,  {},0},
		 {23, 0xd9,    0,  {},0},
		 {24, 0x1f,    0,  {},0},
		 {25, 0xdb,    0,  {},0},
		 {26, 0x20,    0,  {},0},
		 {27, 0x8d,    0,  {},0},
		 {28, 0x14,    0,  {},0},

};

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

void lcd_panel_framebuffer_init(u32 frame_base, u32 width,u32 height)
{
    lcdc_frame_data_flip_init((void *)frame_base, width, height);
}

void lcd_panel_refresh()
{
    lcdc_frame_data_flip();
}

void lcd_panel_refresh_sync()
{
    aicx_error(0x135f47, "ssd", "%s %s(%d) \n",__FILE__, __func__, __LINE__);

    lcdc_frame_data_flip_sync();
}

void lcd_panel_init(void)
{
	/* 4.2.1 Power up Sequence:
	1. Power up VDD
	2. Send Display off command
	3. Initialization
	4. Clear Screen
	5. Power up VCC/ VBAT
	6. Delay 100ms 	(When VCC is stable)
	7. Send Display on command
	*/

    int i, j, para_size;

    lcd_panel_reset();

	//3. Initialization
    for (i = 0; i < sizeof(st7315_init_cmd_para_array) / sizeof(struct cmd_para) ; i++) {

        if (lcdc_config.cmd_para_pack) {
            if (st7315_init_cmd_para_array[i].para_lenght != 0)
                lcdc_cmd_para_set(st7315_init_cmd_para_array[i].cmd, st7315_init_cmd_para_array[i].para,
st7315_init_cmd_para_array[i].para_lenght);
            else
                lcdc_cmd_set(st7315_init_cmd_para_array[i].cmd);
        } else {
            lcdc_cmd_set(st7315_init_cmd_para_array[i].cmd);
            para_size = sizeof(st7315_init_cmd_para_array[i].para);
            //disp_log("para size:%d\n para list:\n", para_size);
            for (j = 0; j < st7315_init_cmd_para_array[i].para_lenght; j ++)
                lcdc_para_byte_set(st7315_init_cmd_para_array[i].para[j]);
        }

        if(st7315_init_cmd_para_array[i].ms_delay)
            aic_mdelay(st7315_init_cmd_para_array[i].ms_delay);
        //aic_mdelay(200);
        //lcdc_cmd_read(st7315_init_cmd_para_array[i].cmd, &ret);
    }


	//4. Clear Screen
//	fill(0x00,0x00);//Ë¢ºÚÆÁ

	//5. Power up VCC/ VBAT
	aic_gpio_request(23);
	aic_gpio_output(23, 1);

	//6. Delay 100ms 	(When VCC is stable)
	aic_mdelay(100);

	//7. Send Display on command
	lcdc_cmd_set(0xAF);
}

