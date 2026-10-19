/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of display.
*/

#include <aic_errno.h>
#include "lcdc/lcdc_reg.h"
#include "lcdc.h"
#include "cache.h"
#include "lcd_panel.h"
#include <display.h>
#include <display_cfg.h>
#include <pmic_pmu.h>

static bool lcdc_eb = false;

#define __CACHE_LINE_SIZE             32U

#ifdef DEBUG
#define AIC_PRINTF_LCDC AIC_PRINTF
#else
#define AIC_PRINTF_LCDC(fmt, ...)
#endif

struct aic_lcdc_config lcdc_config = {
    .aic_wire_type = AIC_WIRE_TYPE,
    .aic_hw_te_trigger = AIC_HW_TE_TRIGGER, // 0'sw trigger 1'hw trigger
    .cmd_para_pack = AIC_CMD_PARA_PACK, // 0'disable para pack 1'enable para pack
    .color_format = AIC_COLOR_FORMAT,
};

/*4wire8bit 3wire9bit1lane 3wire9bit2lane*/
struct wire_type_string{
    int wire_type_id;
    char wire_type_name[20];
};

struct wire_type_string wire_type_group[] ={
    {AIC_LCDC_4WIRE_8BIT, "4wire8bit"},
    {AIC_LCDC_3WIRE_9BIT_1LANE, "3wire9bit1lane"},
    {AIC_LCDC_3WIRE_9BIT_2LANE, "3wire9bit2lane"}
};

/*4wire8bit 3wire9bit1lane 3wire9bit2lane*/
struct color_format_string{
    int color_format_id;
    char color_format_name[20];
};

struct color_format_string color_format_group[] ={
    {AIC_LCDC_COLOR_FORMAT_RGB444_UNPACK, "RGB444_UNPACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB444_COMPACT, "RGB444_COMPACT"},
    {AIC_LCDC_COLOR_FORMAT_RGB565, "RGB565"},
    {AIC_LCDC_COLOR_FORMAT_RGB666_UNPACK, "RGB666_UNPACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB888_UNPACK, "RGB888_UNPACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB444_PACK, "RGB444_PACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB666_PACK, "RGB666_PACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB888_PACK, "RGB888_PACK"}
};

char color_format_error[] = "unsupport color format";

char *get_color_format_name(aic_lcdc_color_format color_format)
{
    int i;

    for (i = 0; i < sizeof(color_format_group)/sizeof(color_format_group[0]); i++) {
        if (color_format == color_format_group[i].color_format_id)
            return color_format_group[i].color_format_name;
    }

    return color_format_error;
}

char te_trigger[] = "te hw trigger";
char sw_trigger[] = "sw trigger";
char trigger_fail[] = "unsupport trigger mode";

void aic_display_wait_ms(u32 wait_ms)
{
    aic_mdelay(wait_ms);
}

char *get_trigger_mode_name(aic_lcdc_start_mode trigger_mode)
{
    if (trigger_mode == AIC_LCDC_SW_TRIGGER)
        return sw_trigger;
    else if (trigger_mode == AIC_LCDC_TE_TRIGGER)
        return te_trigger;
    else
        return trigger_fail;
}

char pack_mode[] = "pack_mode";
char single_mode[] = "sig_mode";

char *get_cmd_para_mode_name(bool is_cmd_para_pack)
{
    if (is_cmd_para_pack)
        return pack_mode;
    else
        return single_mode;
}

char wrie_type_error[] = "unsupport wire type";

char *get_wrie_type_name(aic_lcdc_wire_type wire_type)
{
    int i;

    for (i = 0; i < sizeof(wire_type_group)/sizeof(wire_type_group[0]); i++) {
        if (wire_type == wire_type_group[i].wire_type_id)
            return wire_type_group[i].wire_type_name;
    }

    return wrie_type_error;
}

int aic_lcdc_panel_init(void)
{
    if (lcdc_eb)
        return 0;

    lcdc_enable();
    lcd_panel_init();
    lcdc_eb = true;

    return 0;
}
#if 0
static int aic_panel_poweron(bool enabled)
{
    int ret = 0;

    if (enabled)
        ret = pmu_enable(CONFIG_PANEL_POWER);
    else
        ret = pmu_disable(CONFIG_PANEL_POWER);

    return ret;
}
#endif
static int panel_init(void)
{
//    aic_panel_poweron(true);
    aic_lcdc_panel_init();

    return 0;
}

void display_refresh(u32 fb_addr)
{
    if (!lcdc_eb) {
        panel_init();
    }

    dcache_clean_by_range((u32)fb_addr, LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * AIC_PIXEL_BYTES);
    lcd_panel_framebuffer_init((u32)fb_addr, LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT);
    lcd_panel_refresh();

    aic_bl_brightness_set(100);
}

#if 0 //def CONFIG_BL
#include <display.h>
#include <hlzma.h>
#include <stdlib.h>
#include <display_cfg.h>
#include <cache.h>
#include <initcall.h>

#define __CACHE_LINE_SIZE             32U
char logo_enc[] __attribute__((aligned(__CACHE_LINE_SIZE)))= {
#if (LCD_PANEL_WIDTH == 128)
    #include "res_128_128_logo.h"
#else
    #include "res_240_320_logo.h"
#endif
};

int dec_logo(char *logo_buf, char *addr)
{
    int ret = AIC_SUCCESS;

    if (!logo_buf)
        return AIC_EPERM;

    /*make data clean*/
    dcache_invcc_by_range((u32)logo_buf, LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * 2);

    ret = hlzma_dec_polling((u32)addr, (u32)logo_buf);
    if (ret)
        log_print("decompress logo failed, 0x%s\n", ret);

    return ret;
}

int  show_logo(void)
{
    char *logo_buf = NULL;
    int ret;

    logo_buf = (char *)malloc(LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * 2);//240*320*2 RGB565

    ret = dec_logo(logo_buf,logo_enc);
    if(!ret)
        display_refresh((u32)logo_buf);

    return ret;
}
driver_init(show_logo);
#endif
