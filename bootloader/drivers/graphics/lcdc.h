/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of lcdc module.
 *   It includes all the lcdc function api.
*/
#ifndef __LCDC_H__
#define __LCDC_H__
#include <aic_bitmap.h>
#include <console/console.h>
#include <timer.h>
#include <display_cfg.h>

#define DISP_DEBUG
#ifdef DISP_DEBUG
#define disp_log log_print
#else
#define disp_log(fmt, ...)
#endif

#define CLK_RATE 0x7
#define LCDC_FINISH_TIMEOUT 1000

typedef enum {
    AIC_LCDC_SW_TRIGGER = 0,
    AIC_LCDC_TE_TRIGGER = 1
} aic_lcdc_start_mode;

typedef enum {
    AIC_RGB444_PACK = 0,
    AIC_RGB444_UNPACK,
    AIC_RGB444_COMPACT,
    AIC_RGB565,
    AIC_RGB666_PACK,
    AIC_RGB666_UNPACK,
    AIC_RGB888_PACK,
    AIC_RGB888_UNPACK,
    AIC_SURRPOT_COLOR_DEPTH
} color_depth;

typedef enum {
    LCD_DIRECT_ROT_0 = 0,
    LCD_DIRECT_ROT_90 = 1,
    LCD_DIRECT_ROT_180 = 2,
    LCD_DIRECT_ROT_270 = 3
} LCD_DIRECT_ANGLE;

/*
 *[9:8] pxl_fmt 0:rgb444 1:rgb565 2:rgb666 3:rgb888
 *[11]pack_mode rgb666/rgb444/rgb888 0:4 byte in memory high byte unused; 1:3 byte in memory;
 */
typedef enum {
    AIC_LCDC_COLOR_FORMAT_RGB444_UNPACK = 0,
    AIC_LCDC_COLOR_FORMAT_RGB444_COMPACT,
    AIC_LCDC_COLOR_FORMAT_RGB565 = BIT(8),
    AIC_LCDC_COLOR_FORMAT_RGB666_UNPACK = BIT(9),
    AIC_LCDC_COLOR_FORMAT_RGB888_UNPACK = BIT(8) | BIT(9),
    AIC_LCDC_COLOR_FORMAT_RGB444_PACK = BIT(11),
    AIC_LCDC_COLOR_FORMAT_RGB666_PACK = BIT(9) | BIT(11),
    AIC_LCDC_COLOR_FORMAT_RGB888_PACK = BIT(8) | BIT(9)| BIT(11),
    AIC_LCDC_COLOR_FORMAT_MONO = BIT(8) | BIT(12)
} aic_lcdc_color_format;

struct aic_lcdc_config{
    uint32_t aic_wire_type; //AIC_LCDC_4WIRE_8BIT,AIC_LCDC_3WIRE_9BIT_1LANE,AIC_LCDC_3WIRE_9BIT_2LANE
    uint32_t aic_hw_te_trigger;//0,1
    bool cmd_para_pack;//sig/pack
    uint32_t color_format;
};

extern struct aic_lcdc_config lcdc_config;

#define AIC_CMD_PARA_PACK 0

#ifdef COLOR_FORMAT_RGB444_PACK
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_RGB444_PACK
#define AIC_PIXEL_BYTES 3
#elif defined(COLOR_FORMAT_RGB444_UNPACK)
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_RGB444_UNPACK
#define AIC_PIXEL_BYTES 4
#elif defined(COLOR_FORMAT_RGB444_COMPACT)
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_RGB444_COMPACT
#define AIC_PIXEL_BYTES 3
#elif defined(COLOR_FORMAT_RGB565_1P1T)
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_RGB565
#define AIC_PIXEL_BYTES 2
#elif defined(COLOR_FORMAT_RGB666_PACK)
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_RGB666_PACK
#define AIC_PIXEL_BYTES 3
#elif defined(COLOR_FORMAT_RGB666_UNPACK)
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_RGB666_UNPACK
#define AIC_PIXEL_BYTES 4
#elif defined(COLOR_FORMAT_RGB888_PACK)
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_RGB888_PACK
#define AIC_PIXEL_BYTES 3
#elif defined(COLOR_FORMAT_RGB888_UNPACK)
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_RGB888_UNPACK
#define AIC_PIXEL_BYTES 4
#elif defined(COLOR_FORMAT_MONO_1P1T)
#define AIC_COLOR_FORMAT AIC_LCDC_COLOR_FORMAT_MONO
#define AIC_PIXEL_BYTES 1
#endif



int lcdc_all_reg_reset_val(void);
int lcdc_para_wide(int wide);
int lcdc_rstn(int high);
int lcdc_cmd_set(u32 cmd);
int lcdc_cmd_para_set(u32 cmd, uint8_t *para, u32 para_wide);
int lcdc_frame_data_flip_init(void *base, u32 width, u32 height);
int lcdc_frame_data_flip(void);
int lcdc_frame_data_flip_sync(void);
int lcdc_para_byte_set(uint8_t para);
int lcdc_cmd_read(u32 cmd, u32 *data, bool dummy, int para_num);
void lcdc_pixel_pack_mode(int pack);
int lcdc_data_lane_type(int data_lane_num);
void lcdc_wire_type(int wire_type);
void lcdc_enable(void);
void lcdc_disable(void);
void aic_lcdc_irq_init(void);
void aic_lcdc_clear_int(void);
u32 aic_lcdc_get_status_bypass_int(void);
void aic_lcdc_next_ready(void);
void aic_lcdc_trigger_mode_set(aic_lcdc_start_mode mode);
bool is_panel_init_success(void);
int lcdc_init(void);
void aic_display_wait_ms(u32 wait_ms);
#endif /*__LCDC_H__*/
