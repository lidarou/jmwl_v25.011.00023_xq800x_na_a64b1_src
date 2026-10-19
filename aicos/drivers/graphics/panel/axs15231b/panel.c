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
#define AIC_THIS_FILE    0x076c1a
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

static void lcd_panel_set_direction(LCD_DIRECT_ANGLE direct_type)
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

    ret = lcdc_cmd_set(0x2C);//axs15231 memory write

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
    lcdc_cmd_set(0x2C);
    disp_log("%s(%d) frame_addr:0x%x start\n", __func__, __LINE__, frame_base);
    lcdc_frame_data_flip_init((void *)frame_base, width, height);
}

void lcd_panel_refresh()
{
    disp_log("%s\n", __func__);
    lcdc_frame_flip_mix_cmd();
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
    disp_log("axs15231 0x04 ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDA, &val_before_reset, false, 1);
    disp_log("axs15231 0xDA ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDB, &val_before_reset, false, 1);
    disp_log("axs15231 0xDB ID:0x%x\n", val_before_reset);
    lcdc_cmd_read(0xDC, &val_before_reset, false, 1);
    disp_log("axs15231 0xDC ID:0x%x\n", val_before_reset);
}

void lcd_panel_init(void)
{
    lcd_panel_reset();
    lcdc_cmd_set(0x11); //Sleep out
    aic_mdelay(5); //Delay 120ms

    lcdc_cmd_set(0xBB);
    lcdc_para_byte_set(0x00);//0
    lcdc_para_byte_set(0x00);//1
    lcdc_para_byte_set(0x00);//2
    lcdc_para_byte_set(0x00);//3
    lcdc_para_byte_set(0x00);//4
    lcdc_para_byte_set(0x00);//5
    lcdc_para_byte_set(0x5A);//6
    lcdc_para_byte_set(0xA5);//7

    lcdc_cmd_set(0xA0);
    lcdc_para_byte_set(0xC0);//0
    lcdc_para_byte_set(0x10);//1
    lcdc_para_byte_set(0x00);//2
    lcdc_para_byte_set(0x02);//3
    lcdc_para_byte_set(0x00);//4
    lcdc_para_byte_set(0x00);//5
    lcdc_para_byte_set(0x64);//6
    lcdc_para_byte_set(0x3F);//7
    lcdc_para_byte_set(0x20);//8
    lcdc_para_byte_set(0x05);//9
    lcdc_para_byte_set(0x3F);//10
    lcdc_para_byte_set(0x3F);//11
    lcdc_para_byte_set(0x00);//12
    lcdc_para_byte_set(0x00);//13
    lcdc_para_byte_set(0x00);//14
    lcdc_para_byte_set(0x00);//15
    lcdc_para_byte_set(0x00);//16

    lcdc_cmd_set(0xA2);
    lcdc_para_byte_set(0x31);//0
    lcdc_para_byte_set(0x3C);//1
    lcdc_para_byte_set(0x29);//2
    lcdc_para_byte_set(0x14);//3
    lcdc_para_byte_set(0xA6);//4
    lcdc_para_byte_set(0xA6);//5
    lcdc_para_byte_set(0xA8);//6
    lcdc_para_byte_set(0xE0);//7
    lcdc_para_byte_set(0x40);//8
    lcdc_para_byte_set(0x19);//9
    lcdc_para_byte_set(0x80);//10
    lcdc_para_byte_set(0x80);//11
    lcdc_para_byte_set(0x80);//12
    lcdc_para_byte_set(0x20);//13
    lcdc_para_byte_set(0xf9);//14
    lcdc_para_byte_set(0x10);//15 cr_dsprim_col_str,cr_dsprim_col_end
    lcdc_para_byte_set(0x02);//16 cr_dsprim_row_str,cr_dsprim_row_end
    lcdc_para_byte_set(0xff);//17 cr_pat_sel[15:8]
    lcdc_para_byte_set(0xff);//18 cr_pat_sel[7:0]
    lcdc_para_byte_set(0xF0);//19
    lcdc_para_byte_set(0x90);//20
    lcdc_para_byte_set(0x01);//21
    lcdc_para_byte_set(0x32);//22
    lcdc_para_byte_set(0xA0);//23
    lcdc_para_byte_set(0x91);//24
    lcdc_para_byte_set(0x40);//25
    lcdc_para_byte_set(0x20);//26
    lcdc_para_byte_set(0x7F);//27
    lcdc_para_byte_set(0xFF);//28
    lcdc_para_byte_set(0x00);//29
    lcdc_para_byte_set(0x04);//30

    lcdc_cmd_set(0xD0);
    lcdc_para_byte_set(0xE0);//0
    lcdc_para_byte_set(0x40);//1
    lcdc_para_byte_set(0x51);//2
    lcdc_para_byte_set(0x24);//3
    lcdc_para_byte_set(0x08);//4
    lcdc_para_byte_set(0x05);//5
    lcdc_para_byte_set(0x10);//6
    lcdc_para_byte_set(0x01);//7
    lcdc_para_byte_set(0x90);//8
    lcdc_para_byte_set(0x12);//9  cr_n_ln_1
    lcdc_para_byte_set(0xC2);//10
    lcdc_para_byte_set(0x42);//11
    lcdc_para_byte_set(0x22);//12
    lcdc_para_byte_set(0x22);//13
    lcdc_para_byte_set(0xAA);//14
    lcdc_para_byte_set(0x03);//15
    lcdc_para_byte_set(0x10);//16
    lcdc_para_byte_set(0x12);//17
    lcdc_para_byte_set(0x60);//18 cr_dsh_shr
    lcdc_para_byte_set(0x14);//19 cr_dual_pattern
    lcdc_para_byte_set(0x1E);//20
    lcdc_para_byte_set(0x51);//21
    lcdc_para_byte_set(0x15);//22
    lcdc_para_byte_set(0x00);//23
    lcdc_para_byte_set(0x20);//24 cr_line_f_half[7:0]
    lcdc_para_byte_set(0x10);//25 NC 1BIT,cr_sub_pix_ini,cr_line_f_half[9:8],D[3-0]cr_pixnum_offset
    lcdc_para_byte_set(0x00);//26
    lcdc_para_byte_set(0x03);//27
    lcdc_para_byte_set(0x3D);//28
    lcdc_para_byte_set(0x12);//29

    lcdc_cmd_set(0xA3);
    lcdc_para_byte_set(0xA0);//0
    lcdc_para_byte_set(0x06);//1
    lcdc_para_byte_set(0xAa);//2
    lcdc_para_byte_set(0x00);//3
    lcdc_para_byte_set(0x08);//4
    lcdc_para_byte_set(0x02);//5
    lcdc_para_byte_set(0x0A);//6
    lcdc_para_byte_set(0x04);//7
    lcdc_para_byte_set(0x04);//8
    lcdc_para_byte_set(0x04);//9
    lcdc_para_byte_set(0x04);//10
    lcdc_para_byte_set(0x04);//11
    lcdc_para_byte_set(0x04);//12
    lcdc_para_byte_set(0x04);//13
    lcdc_para_byte_set(0x04);//14
    lcdc_para_byte_set(0x04);//15
    lcdc_para_byte_set(0x04);//16
    lcdc_para_byte_set(0x04);//17
    lcdc_para_byte_set(0x04);//18
    lcdc_para_byte_set(0x00);//19
    lcdc_para_byte_set(0x55);//20
    lcdc_para_byte_set(0x55);//21

    lcdc_cmd_set(0xA4);
    lcdc_para_byte_set(0x85);//0
    lcdc_para_byte_set(0x85);//1
    lcdc_para_byte_set(0x95);//2
    lcdc_para_byte_set(0x82);//3
    lcdc_para_byte_set(0xAF);//4
    lcdc_para_byte_set(0xED);//5
    lcdc_para_byte_set(0xED);//6
    lcdc_para_byte_set(0x80);//7
    lcdc_para_byte_set(0x10);//8
    lcdc_para_byte_set(0x30);//9
    lcdc_para_byte_set(0x40);//10
    lcdc_para_byte_set(0x40);//11
    lcdc_para_byte_set(0x20);//12
    lcdc_para_byte_set(0x50);//13
    lcdc_para_byte_set(0x60);//14
    lcdc_para_byte_set(0x35);//15

    lcdc_cmd_set(0xC1);
    lcdc_para_byte_set(0x33);//0 cr_mipi_ckln_reset,cr_src_opt,cr_bgr_opt,cr_ret
    lcdc_para_byte_set(0x04);//1
    lcdc_para_byte_set(0x02);//2
    lcdc_para_byte_set(0x02);//3
    lcdc_para_byte_set(0x71);//4
    lcdc_para_byte_set(0x05);//5
    lcdc_para_byte_set(0x24);//6
    lcdc_para_byte_set(0x55);//7
    lcdc_para_byte_set(0x02);//8
    lcdc_para_byte_set(0x00);//9
    lcdc_para_byte_set(0x41);//10
    lcdc_para_byte_set(0x00);//11 fps 0x0 90fps 0x1 60fps 0x2 45fps 0x3 30fps
    lcdc_para_byte_set(0x53);//12
    lcdc_para_byte_set(0xFF);//13
    lcdc_para_byte_set(0xFF);//14
    lcdc_para_byte_set(0xFF);//15
    lcdc_para_byte_set(0x4F);//16
    lcdc_para_byte_set(0x52);//17
    lcdc_para_byte_set(0x00);//18
    lcdc_para_byte_set(0x4F);//19
    lcdc_para_byte_set(0x52);//20
    lcdc_para_byte_set(0x00);//21
    lcdc_para_byte_set(0x45);//22
    lcdc_para_byte_set(0x3B);//23
    lcdc_para_byte_set(0x0B);//24
    lcdc_para_byte_set(0x02);//25
    lcdc_para_byte_set(0x0D);//26
    lcdc_para_byte_set(0x00);//27
    lcdc_para_byte_set(0xFF);//28
    lcdc_para_byte_set(0x40);//29

    lcdc_cmd_set(0xC3);
    lcdc_para_byte_set(0x00);//0
    lcdc_para_byte_set(0x00);//1
    lcdc_para_byte_set(0x00);//2
    lcdc_para_byte_set(0x50);//3
    lcdc_para_byte_set(0x03);//4
    lcdc_para_byte_set(0x00);//5
    lcdc_para_byte_set(0x00);//6
    lcdc_para_byte_set(0x00);//7
    lcdc_para_byte_set(0x01);//8
    lcdc_para_byte_set(0x80);//9
    lcdc_para_byte_set(0x01);//10

    lcdc_cmd_set(0xC4);
    lcdc_para_byte_set(0x00);//0
    lcdc_para_byte_set(0x24);//1
    lcdc_para_byte_set(0x33);//2
    lcdc_para_byte_set(0x80);//3
    lcdc_para_byte_set(0x00);//4 cr_vcom_vsel
    lcdc_para_byte_set(0xea);//5
    lcdc_para_byte_set(0x64);//6
    lcdc_para_byte_set(0x32);//7
    lcdc_para_byte_set(0xC8);//8
    lcdc_para_byte_set(0x64);//9
    lcdc_para_byte_set(0xC8);//10
    lcdc_para_byte_set(0x32);//11
    lcdc_para_byte_set(0x90);//12
    lcdc_para_byte_set(0x90);//13
    lcdc_para_byte_set(0x11);//14
    lcdc_para_byte_set(0x06);//15
    lcdc_para_byte_set(0xDC);//16
    lcdc_para_byte_set(0xFA);//17
    lcdc_para_byte_set(0x00);//18
    lcdc_para_byte_set(0x00);//19
    lcdc_para_byte_set(0x80);//20
    lcdc_para_byte_set(0xFE);//21
    lcdc_para_byte_set(0x10);//22
    lcdc_para_byte_set(0x10);//23
    lcdc_para_byte_set(0x00);//24
    lcdc_para_byte_set(0x0A);//25
    lcdc_para_byte_set(0x0A);//26
    lcdc_para_byte_set(0x44);//27
    lcdc_para_byte_set(0x50);//28

    lcdc_cmd_set(0xC5);
    lcdc_para_byte_set(0x18);//0
    lcdc_para_byte_set(0x00);//1
    lcdc_para_byte_set(0x00);//2
    lcdc_para_byte_set(0x03);//3
    lcdc_para_byte_set(0xFE);//4
    lcdc_para_byte_set(0x50);//5 cr_te_str[7:0]
    lcdc_para_byte_set(0x38);//6 cr_te_end[7:0]
    lcdc_para_byte_set(0x20);//7 cr_te_str[11:8],cr_te_end[11:8]
    lcdc_para_byte_set(0x30);//8
    lcdc_para_byte_set(0x10);//9
    lcdc_para_byte_set(0x88);//10
    lcdc_para_byte_set(0xDE);//11
    lcdc_para_byte_set(0x0D);//12
    lcdc_para_byte_set(0x08);//13
    lcdc_para_byte_set(0x0F);//14
    lcdc_para_byte_set(0x0F);//15
    lcdc_para_byte_set(0x01);//16
    lcdc_para_byte_set(0x50);//17 cr_tp_str[7:0]
    lcdc_para_byte_set(0x38);//18 cr_tp_end[7:0]
    lcdc_para_byte_set(0x20);//19 cr_tp_str[11:8],cr_tp_end[11:8]
    lcdc_para_byte_set(0x10);//20
    lcdc_para_byte_set(0x10);//21
    lcdc_para_byte_set(0x00);//22

    lcdc_cmd_set(0xC6);
    lcdc_para_byte_set(0x05);//0
    lcdc_para_byte_set(0x0A);//1
    lcdc_para_byte_set(0x05);//2
    lcdc_para_byte_set(0x0A);//3
    lcdc_para_byte_set(0x00);//4
    lcdc_para_byte_set(0xE0);//5
    lcdc_para_byte_set(0x2E);//6
    lcdc_para_byte_set(0x0B);//7
    lcdc_para_byte_set(0x12);//8
    lcdc_para_byte_set(0x22);//9
    lcdc_para_byte_set(0x12);//10
    lcdc_para_byte_set(0x22);//11
    lcdc_para_byte_set(0x01);//12
    lcdc_para_byte_set(0x03);//13
    lcdc_para_byte_set(0x00);//14
    lcdc_para_byte_set(0x02);//15
    lcdc_para_byte_set(0x6A);//16
    lcdc_para_byte_set(0x18);//17
    lcdc_para_byte_set(0xC8);//18
    lcdc_para_byte_set(0x22);//19

    lcdc_cmd_set(0xC7);
    lcdc_para_byte_set(0x50);//0
    lcdc_para_byte_set(0x32);//1 cr_lvd18_en cr_lvd30_en
    lcdc_para_byte_set(0x28);//2
    lcdc_para_byte_set(0x00);//3
    lcdc_para_byte_set(0xa2);//4 cr_lvd18_adj cr_lvd30_adj
    lcdc_para_byte_set(0x80);//5
    lcdc_para_byte_set(0x8f);//6
    lcdc_para_byte_set(0x00);//7
    lcdc_para_byte_set(0x80);//8
    lcdc_para_byte_set(0xff);//9
    lcdc_para_byte_set(0x07);//10
    lcdc_para_byte_set(0x11);//11
    lcdc_para_byte_set(0x9c);//12
    lcdc_para_byte_set(0x67);//13
    lcdc_para_byte_set(0xff);//14
    lcdc_para_byte_set(0x24);//15 cr_igzo_timing_en
    lcdc_para_byte_set(0x0c);//16
    lcdc_para_byte_set(0x0d);//17
    lcdc_para_byte_set(0x0e);//18
    lcdc_para_byte_set(0x0f);//19

    lcdc_cmd_set(0xC9);
    lcdc_para_byte_set(0x33);//0
    lcdc_para_byte_set(0x44);//1
    lcdc_para_byte_set(0x44);//2
    lcdc_para_byte_set(0x01);//3

    lcdc_cmd_set(0xCF);
    lcdc_para_byte_set(0x2C);//0
    lcdc_para_byte_set(0x1E);//1
    lcdc_para_byte_set(0x88);//2
    lcdc_para_byte_set(0x58);//3
    lcdc_para_byte_set(0x13);//4
    lcdc_para_byte_set(0x18);//5
    lcdc_para_byte_set(0x56);//6
    lcdc_para_byte_set(0x18);//7
    lcdc_para_byte_set(0x1E);//8
    lcdc_para_byte_set(0x68);//9 D[4-0] OSC_base
    lcdc_para_byte_set(0x88);//10
    lcdc_para_byte_set(0x00);//11
    lcdc_para_byte_set(0x65);//12
    lcdc_para_byte_set(0x09);//13
    lcdc_para_byte_set(0x22);//14
    lcdc_para_byte_set(0xC4);//15
    lcdc_para_byte_set(0x0C);//16
    lcdc_para_byte_set(0x77);//17
    lcdc_para_byte_set(0x22);//18
    lcdc_para_byte_set(0x44);//19
    lcdc_para_byte_set(0xAA);//20
    lcdc_para_byte_set(0x55);//21
    lcdc_para_byte_set(0x08);//22
    lcdc_para_byte_set(0x08);//23
    lcdc_para_byte_set(0x12);//24
    lcdc_para_byte_set(0xA0);//25
    lcdc_para_byte_set(0x08);//26

    lcdc_cmd_set(0xD5);
    lcdc_para_byte_set(0x00);//0 cr_gspf2
    lcdc_para_byte_set(0x3e);//1 cr_gsps2
    lcdc_para_byte_set(0x89);//2 cr_vst_pre_adj
    lcdc_para_byte_set(0x01);//3 cr_vst_post_adj
    lcdc_para_byte_set(0x35);//4 cr_hig_wdt cr_low_wdt
    lcdc_para_byte_set(0x04);//5 cr_rst_wdt
    lcdc_para_byte_set(0x92);//6 cr_gpwr1_star[7:0]
    lcdc_para_byte_set(0x6F);//7 cr_gpwr1_end[7:0]
    lcdc_para_byte_set(0x04);//8 cr_gpwr1_star[11:8]cr_gpwr1_end[11:8]
    lcdc_para_byte_set(0x92);//9 cr_gpwr2_star[7:0]
    lcdc_para_byte_set(0x6F);//10 cr_gpwr2_end[7:0]
    lcdc_para_byte_set(0x04);//11 cr_gpwr2_star[11:8] cr_gpwr2_end[11:8]
    lcdc_para_byte_set(0x08);//12 cr_dum_num
    lcdc_para_byte_set(0x6A);//13 cr_rst_star[7:0]
    lcdc_para_byte_set(0x04);//14 cr_gpwr1_sel,cr_gpwr2_sel,cr_fv,cr_frm_sel,cr_rst_star[11:8]
    lcdc_para_byte_set(0x46);//15 cr_fl,cr_gck_bgn
    lcdc_para_byte_set(0x03);//16 cr_gp1r
    lcdc_para_byte_set(0x03);//17 cr_gp1f
    lcdc_para_byte_set(0x03);//18 cr_gp2r
    lcdc_para_byte_set(0x03);//19 cr_gp2f
    lcdc_para_byte_set(0x00);//20 cr_gck_en_adj
    lcdc_para_byte_set(0x01);//21 cr_vg_fix_high[7:0]
    lcdc_para_byte_set(0x03);//22 cr_vg_fix_low[7:0]
    lcdc_para_byte_set(0x00);//23 cr_vg_fix_high[11:8],cr_vg_fix_low[11:8]
    lcdc_para_byte_set(0xE0);//24 cr_nl[7:0]
    lcdc_para_byte_set(0x51);//25 cr_gip_igzo_opt cr_nl[11:8]
    lcdc_para_byte_set(0xa1);//26 cr_bp A1-A7
    lcdc_para_byte_set(0x00);//27 cr_gspf1
    lcdc_para_byte_set(0x00);//28 cr_gsps1
    lcdc_para_byte_set(0x00);//29 cr_gto_max

    lcdc_cmd_set(0xD6);
    lcdc_para_byte_set(0x10);//0
    lcdc_para_byte_set(0x32);//1
    lcdc_para_byte_set(0x54);//2
    lcdc_para_byte_set(0x76);//3
    lcdc_para_byte_set(0x98);//4
    lcdc_para_byte_set(0xBA);//5
    lcdc_para_byte_set(0xDC);//6
    lcdc_para_byte_set(0xFE);//7
    lcdc_para_byte_set(0x93);//8
    lcdc_para_byte_set(0x00);//9
    lcdc_para_byte_set(0x01);//10
    lcdc_para_byte_set(0x83);//11
    lcdc_para_byte_set(0x07);//12
    lcdc_para_byte_set(0x07);//13
    lcdc_para_byte_set(0x00);//14
    lcdc_para_byte_set(0x07);//15
    lcdc_para_byte_set(0x07);//16
    lcdc_para_byte_set(0x00);//17
    lcdc_para_byte_set(0x03);//18
    lcdc_para_byte_set(0x03);//19
    lcdc_para_byte_set(0x03);//20
    lcdc_para_byte_set(0x03);//21
    lcdc_para_byte_set(0x03);//22
    lcdc_para_byte_set(0x03);//23
    lcdc_para_byte_set(0x00);//24
    lcdc_para_byte_set(0x84);//25
    lcdc_para_byte_set(0x00);//26
    lcdc_para_byte_set(0x20);//27
    lcdc_para_byte_set(0x01);//28
    lcdc_para_byte_set(0x00);//29

    lcdc_cmd_set(0xD7);
    lcdc_para_byte_set(0x03);//0
    lcdc_para_byte_set(0x01);//1
    lcdc_para_byte_set(0x0b);//2
    lcdc_para_byte_set(0x09);//3
    lcdc_para_byte_set(0x0f);//4
    lcdc_para_byte_set(0x0d);//5
    lcdc_para_byte_set(0x1E);//6
    lcdc_para_byte_set(0x1F);//7
    lcdc_para_byte_set(0x18);//8
    lcdc_para_byte_set(0x1d);//9
    lcdc_para_byte_set(0x1f);//10
    lcdc_para_byte_set(0x19);//11
    lcdc_para_byte_set(0x00);//12
    lcdc_para_byte_set(0x3e);//13
    lcdc_para_byte_set(0x04);//14
    lcdc_para_byte_set(0x00);//15 cr_gip_hs_dual1[11:8],cr_gip_hs_dual2[11:8]
    lcdc_para_byte_set(0x1d);//16 cr_gip_hs_dual1[7:0]
    lcdc_para_byte_set(0x40);//17 cr_gip_hs_dual2[7:0]
    lcdc_para_byte_set(0x1F);//18

    lcdc_cmd_set(0xD8);
    lcdc_para_byte_set(0x02);//0
    lcdc_para_byte_set(0x00);//1
    lcdc_para_byte_set(0x0a);//2
    lcdc_para_byte_set(0x08);//3
    lcdc_para_byte_set(0x0e);//4
    lcdc_para_byte_set(0x0c);//5
    lcdc_para_byte_set(0x1E);//6
    lcdc_para_byte_set(0x1F);//7
    lcdc_para_byte_set(0x18);//8
    lcdc_para_byte_set(0x1d);//9
    lcdc_para_byte_set(0x1f);//10
    lcdc_para_byte_set(0x19);//11

    lcdc_cmd_set(0xD9);
    lcdc_para_byte_set(0x1F);//0
    lcdc_para_byte_set(0x1F);//1
    lcdc_para_byte_set(0x1F);//2
    lcdc_para_byte_set(0x1F);//3
    lcdc_para_byte_set(0x1F);//4
    lcdc_para_byte_set(0x1F);//5
    lcdc_para_byte_set(0x1F);//6
    lcdc_para_byte_set(0x1F);//7
    lcdc_para_byte_set(0x1F);//8
    lcdc_para_byte_set(0x1F);//9
    lcdc_para_byte_set(0x1F);//10
    lcdc_para_byte_set(0x1F);//11

    lcdc_cmd_set(0xDD);
    lcdc_para_byte_set(0x1F);//0
    lcdc_para_byte_set(0x1F);//1
    lcdc_para_byte_set(0x1F);//2
    lcdc_para_byte_set(0x1F);//3
    lcdc_para_byte_set(0x1F);//4
    lcdc_para_byte_set(0x1F);//5
    lcdc_para_byte_set(0x1F);//6
    lcdc_para_byte_set(0x1F);//7
    lcdc_para_byte_set(0x1F);//8
    lcdc_para_byte_set(0x1F);//9
    lcdc_para_byte_set(0x1F);//10
    lcdc_para_byte_set(0x1F);//11

    lcdc_cmd_set(0xDF);
    lcdc_para_byte_set(0x44);//0
    lcdc_para_byte_set(0x73);//1
    lcdc_para_byte_set(0x4B);//2
    lcdc_para_byte_set(0x69);//3
    lcdc_para_byte_set(0x00);//4
    lcdc_para_byte_set(0x0A);//5
    lcdc_para_byte_set(0x02);//6
    lcdc_para_byte_set(0x90);//7

    lcdc_cmd_set(0xE0);
    lcdc_para_byte_set(0x3B);//0  vgmp
    lcdc_para_byte_set(0x00);//1  j0p V0P      255
    lcdc_para_byte_set(0x0f);//2  v1p          251
    lcdc_para_byte_set(0x14);//3  v2p          247
    lcdc_para_byte_set(0x0c);//4  v4p          239
    lcdc_para_byte_set(0x03);//5  v6p          231
    lcdc_para_byte_set(0x11);//6  v13p         203
    lcdc_para_byte_set(0x26);//7  v20p         179
    lcdc_para_byte_set(0x4b);//8  v27p v36p    147   111
    lcdc_para_byte_set(0x21);//9  v43p         83
    lcdc_para_byte_set(0x0d);//10 v50p         55
    lcdc_para_byte_set(0x36);//11 v57p         27
    lcdc_para_byte_set(0x13);//12 v59p         19
    lcdc_para_byte_set(0x2a);//13 v61p         11
    lcdc_para_byte_set(0x2f);//14 v62p         7
    lcdc_para_byte_set(0x28);//15 j1p v63p     3
    lcdc_para_byte_set(0x0D);//16 vgsp

    lcdc_cmd_set(0xE1);
    lcdc_para_byte_set(0x37);//0  vgmn
    lcdc_para_byte_set(0x00);//1  j0n V0n      255
    lcdc_para_byte_set(0x0f);//2  v1n          251
    lcdc_para_byte_set(0x14);//3  v2n          247
    lcdc_para_byte_set(0x0b);//4  v4n          239
    lcdc_para_byte_set(0x03);//5  v6n          231
    lcdc_para_byte_set(0x11);//6  v13n         203
    lcdc_para_byte_set(0x26);//7  v20n         179
    lcdc_para_byte_set(0x4b);//8  v27n v36n    147   111
    lcdc_para_byte_set(0x21);//9  v43n         83
    lcdc_para_byte_set(0x0d);//10 v50n         55
    lcdc_para_byte_set(0x36);//11 v57n         27
    lcdc_para_byte_set(0x13);//12 v59n         19
    lcdc_para_byte_set(0x2a);//13 v61n         11
    lcdc_para_byte_set(0x2D);//14 v62n         7
    lcdc_para_byte_set(0x28);//15 j1n v63n     3
    lcdc_para_byte_set(0x0F);//16 vgsn

    lcdc_cmd_set(0xE2);
    lcdc_para_byte_set(0x3B);//0
    lcdc_para_byte_set(0x07);//1
    lcdc_para_byte_set(0x12);//2
    lcdc_para_byte_set(0x18);//3
    lcdc_para_byte_set(0x0E);//4
    lcdc_para_byte_set(0x0D);//5
    lcdc_para_byte_set(0x17);//6
    lcdc_para_byte_set(0x35);//7
    lcdc_para_byte_set(0x44);//8
    lcdc_para_byte_set(0x32);//9
    lcdc_para_byte_set(0x0C);//10
    lcdc_para_byte_set(0x14);//11
    lcdc_para_byte_set(0x14);//12
    lcdc_para_byte_set(0x36);//13
    lcdc_para_byte_set(0x3A);//14
    lcdc_para_byte_set(0x0F);//15
    lcdc_para_byte_set(0x0D);//16

    lcdc_cmd_set(0xE3);
    lcdc_para_byte_set(0x37);//0
    lcdc_para_byte_set(0x07);//1
    lcdc_para_byte_set(0x12);//2
    lcdc_para_byte_set(0x18);//3
    lcdc_para_byte_set(0x0E);//4
    lcdc_para_byte_set(0x0D);//5
    lcdc_para_byte_set(0x17);//6
    lcdc_para_byte_set(0x35);//7
    lcdc_para_byte_set(0x44);//8
    lcdc_para_byte_set(0x32);//9
    lcdc_para_byte_set(0x0C);//10
    lcdc_para_byte_set(0x14);//11
    lcdc_para_byte_set(0x14);//12
    lcdc_para_byte_set(0x36);//13
    lcdc_para_byte_set(0x32);//14
    lcdc_para_byte_set(0x2F);//15
    lcdc_para_byte_set(0x0F);//16

    lcdc_cmd_set(0xE4);
    lcdc_para_byte_set(0x3B);//0
    lcdc_para_byte_set(0x07);//1
    lcdc_para_byte_set(0x12);//2
    lcdc_para_byte_set(0x18);//3
    lcdc_para_byte_set(0x0E);//4
    lcdc_para_byte_set(0x0D);//5
    lcdc_para_byte_set(0x17);//6
    lcdc_para_byte_set(0x39);//7
    lcdc_para_byte_set(0x44);//8
    lcdc_para_byte_set(0x2E);//9
    lcdc_para_byte_set(0x0C);//10
    lcdc_para_byte_set(0x14);//11
    lcdc_para_byte_set(0x14);//12
    lcdc_para_byte_set(0x36);//13
    lcdc_para_byte_set(0x3A);//14
    lcdc_para_byte_set(0x2F);//15
    lcdc_para_byte_set(0x0D);//16

    lcdc_cmd_set(0xE5);
    lcdc_para_byte_set(0x37);//0
    lcdc_para_byte_set(0x07);//1
    lcdc_para_byte_set(0x12);//2
    lcdc_para_byte_set(0x18);//3
    lcdc_para_byte_set(0x0E);//4
    lcdc_para_byte_set(0x0D);//5
    lcdc_para_byte_set(0x17);//6
    lcdc_para_byte_set(0x39);//7
    lcdc_para_byte_set(0x44);//8
    lcdc_para_byte_set(0x2E);//9
    lcdc_para_byte_set(0x0C);//10
    lcdc_para_byte_set(0x14);//11
    lcdc_para_byte_set(0x14);//12
    lcdc_para_byte_set(0x36);//13
    lcdc_para_byte_set(0x3A);//14
    lcdc_para_byte_set(0x2F);//15
    lcdc_para_byte_set(0x0F);//16

    lcdc_cmd_set(0xBB);
    lcdc_para_byte_set(0x00);//0
    lcdc_para_byte_set(0x00);//1
    lcdc_para_byte_set(0x00);//2
    lcdc_para_byte_set(0x00);//3
    lcdc_para_byte_set(0x00);//4
    lcdc_para_byte_set(0x00);//5
    lcdc_para_byte_set(0x00);//6
    lcdc_para_byte_set(0x00);//7

    lcdc_cmd_set(0x11);
    aic_display_wait_ms(200);
    lcdc_cmd_set(0x29);
    aic_display_wait_ms(100);

    lcd_panel_read_id();
    lcd_panel_set_direction(LCD_PANEL_ROTATION_ANGLE);
    lcd_panel_rotation_invalidate_rect(0, 0, LCD_PANEL_WIDTH - 1, LCD_PANEL_HEIGHT - 1, LCD_PANEL_ROTATION_ANGLE);
}
