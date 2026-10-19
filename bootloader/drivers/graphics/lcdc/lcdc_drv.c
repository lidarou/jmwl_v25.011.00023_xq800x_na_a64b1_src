/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of lcdc module.
 *   It includes all the lcdc operate interface.
*/

#include <aic_errno.h>
#include <aic_io.h>
#include "lcdc_reg.h"
#include "../lcdc.h"
#include <reg_base.h>
#include <mcu_glb.h>
#include <top_glb.h>
#include <timer.h>
#include <cru_mcu.h>
#include <pinlist.h>

#define LCDC_FINISH     0x1

#ifndef LCDC_REG_DEBUG
#define AIC_WRITE32_LCDC AIC_WRITE32
#define AIC_READ32_LCDC AIC_READ32
#else
void aic_write32_log(u32 addr, u32 value)
{
    log_print("wirte32 reg:0x%x val:0x%x\n", addr, value);
    AIC_WRITE32(addr, value);
}

u32 aic_read32_log(u32 addr)
{
    u32 val;

    val = AIC_READ32(addr);
    log_print("read32 reg:0x%x val:0x%x\n", addr, val);

    return val;
}
#define AIC_WRITE32_LCDC aic_write32_log
#define AIC_READ32_LCDC aic_read32_log
#endif
/* cmd + parameters offset*/
#define BITS_LCDC_PARA_BYTE_OFFSET(x,y)           ((x) << y * 8 & GENMASK(y * 8, y * 8 + 7))
u32 para_info_range[4] = {REG_LCDC_LCDC_PARA_INFO, REG_LCDC_LCDC_PARA_INFO2,
                          REG_LCDC_LCDC_PARA_INFO3, REG_LCDC_LCDC_PARA_INFO4};

static void check_st_idle(void)
{
    while(BIT_LCDC_LCDC_STATE_MASK(AIC_READ32(REG_LCDC_LCDC_ST1)) != 0)
        ;
}

static void lcdc_start(void)
{
    check_st_idle();

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_START, BIT_LCDC_LCDC_START);
}

static int lcdc_wait_finish(void)
{
    u32 val, begin_time;
    int ret = 0;

    begin_time = aic_get_tick();
    while(1) {
        val = AIC_READ32(REG_LCDC_SSP_INT_RAW);
        if( val & BIT(0))
            break;

        if ((aic_get_tick() - begin_time) > LCDC_FINISH_TIMEOUT) {
            ret = AIC_ETIME;
            break;
        }
    }
    AIC_WRITE32(REG_LCDC_SSP_INT_CLEAR, 0XFFFF);

    if (ret)
        log_print("wait lcdc finish timeout!\n");

    return ret;
}

int lcdc_frame_data_flip_init(void *base, u32 width, u32 height)
{
    u32 val;
    u32 pixel_width;
    int pixel_fmt;

    check_st_idle();

    /*0. set frame data address*/
    val = (u32)base;
    val = BIT_LCDC_FRAME_ADDR(val);
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_ADDR, val);

    if (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_RGB565) {
        pixel_width = width/2;
        pixel_fmt = 1;
    } else if (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_MONO) {
        pixel_width = width/4;
        pixel_fmt = 1;
    } else if (lcdc_config.color_format & BIT_LCDC_PACK_MODE) {
        pixel_width = width*3/4;
        pixel_fmt = 2;
    } else {
        pixel_width = width;
        pixel_fmt = 3;
    }

    /*1. set data width*/
    val = BIT_LCDC_IMG_WIDTH(pixel_width)| BIT_LCDC_IMG_PITCH(pixel_width);
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_DATA_INFO, val);

    /*2. set data height*/
    val = BIT_LCDC_IMG_HEIGHT(height);
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR1, val);

    /*3. set lcdc data flag*/
    val = 0;
    /*set cmd & para flag*/
    val |= BIT_LCDC_DATA_FLAG;
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CMD_INFO, val);

    /*4. set pixel write & pixel format*/
    val = 0;
    val |= BIT_LCDC_TXFIFO_THRD(8);
    val |= BIT_LCDC_PXLWR_CMD; //si/so share 1pad
    val |= BIT_LCDC_BAUD_RATE(FRAME_CLK_DIV);
    val |= BIT_LCDC_PXL_FMT(pixel_fmt); // RGB565
    val |= lcdc_config.aic_wire_type & BIT_LCDC_LCD_3WIRE_FLAG;
    val |= lcdc_config.aic_wire_type & BIT_LCDC_WIRE_TYPE;
    val |= lcdc_config.color_format & BIT_LCDC_PXL_FMT(0x3);
    val |= lcdc_config.color_format & BIT_LCDC_PACK_MODE;
    val |= BIT_LCDC_WRITE_OP;

    if (lcdc_config.color_format == AIC_LCDC_COLOR_FORMAT_MONO)
        val |= BIT_LCDC_LCD_TOG_MODE;

    if(AIC_HW_TE_TRIGGER)
        val |= BIT_LCDC_LCD_START_MODE(AIC_LCDC_TE_TRIGGER);

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR0, val);

    return 0;
}

/*sync*/
int lcdc_frame_data_flip()
{
    int ret = 0;

    if (AIC_HW_TE_TRIGGER)
        AIC_WRITE32_LCDC(REG_LCDC_SSP_RDY, BIT_LCDC_LCD_NXT_READY);

    lcdc_start();
    ret = lcdc_wait_finish();

    return ret;
}
int lcdc_frame_data_flip_sync()
{
    return lcdc_frame_data_flip();
}

int lcdc_cmd_para_set(u32 cmd, uint8_t *para, u32 para_wide)
{
    u32 val;
    int i, j, k;
    uint8_t *op;
    int ret;

    check_st_idle();

    op = para;
    if (para_wide > 16) {
        log_print("para wide ultra limit.\n");
        para_wide = 16;
        //return -1;
    }

    /*1.set cmd & para*/
    //val = AIC_READ32_LCDC(REG_LCDC_LCDC_CMD_INFO);
    val = 0;
    /*clear para & cmd*/
    val &= ~ BIT_LCDC_PARA_BYTE(0xFF);
    val &= ~ BIT_LCDC_CMD_WORD(0xFF);
    /*set para wide*/
    val |= BIT_LCDC_PARA_BYTE(para_wide - 1);
    val |= BIT_LCDC_CMD_WORD(cmd);
    /*set cmd & para flag*/
    val |= BIT_LCDC_CMD_FLAG;
    val |= BIT_LCDC_PARA_FLAG;
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CMD_INFO, val);

    //val = AIC_READ32_LCDC(REG_LCDC_CTRL_REG0);
    val = 0;
    val |= BIT_LCDC_WRITE_OP; //write option
    //val |= BIT_LCDC_LCD_TOG_MODE;
    val |= lcdc_config.aic_wire_type & BIT_LCDC_LCD_3WIRE_FLAG;
//    val |= lcdc_config.aic_wire_type & BIT_LCDC_WIRE_TYPE;
    val |= BIT_LCDC_WIRE1_COMBINE; //si/so share 1pad
    val &= ~ BIT_LCDC_PXLWR_CMD;
    val |= BIT_LCDC_BAUD_RATE(CLK_RATE);

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR0, val);

    k = para_wide -1;
    for (i = 0; i < para_wide;) {
        val = 0;
        if(i < 4) {
            for( j = 0; j < 4; j++){
                if (i >=  para_wide)
                    break;
                i++;
                val |= BITS_LCDC_PARA_BYTE_OFFSET(*(op + k--), j);
            }
            AIC_WRITE32_LCDC(para_info_range[0], val);
        } else if (i < 8) {
            for( j = 0; j < 4; j++){
                if (i >=  para_wide)
                    break;
                i++;
                val |= BITS_LCDC_PARA_BYTE_OFFSET(*(op + k--), j);
            }
            AIC_WRITE32_LCDC(para_info_range[1], val);
        } else if (i < 12) {
            for( j = 0; j < 4; j++){
                if (i >=  para_wide)
                    break;
                i++;
                val |= BITS_LCDC_PARA_BYTE_OFFSET(*(op + k--), j);
            }
            AIC_WRITE32_LCDC(para_info_range[2], val);
        } else if (i < 16) {
            for( j = 0; j < 4; j++){
                if (i >=  para_wide)
                    break;
                i++;
                val |= BITS_LCDC_PARA_BYTE_OFFSET(*(op + k--), j);
            }
            AIC_WRITE32_LCDC(para_info_range[3], val);
        }
    }

    lcdc_start();
    ret = lcdc_wait_finish();

    //aic_mdelay(5);

    return ret;
}

int lcdc_para_byte_set(uint8_t para)
{
    u32 val;
    int ret;

    check_st_idle();

    /*1.set cmd & para*/
    //val = AIC_READ32_LCDC(REG_LCDC_LCDC_CMD_INFO);
    val = 0;
    /*clear para & cmd*/
    val &= ~ BIT_LCDC_PARA_BYTE(0xFF);
    val &= ~ BIT_LCDC_CMD_WORD(0xFF);
    /*set para wide*/
    val |= BIT_LCDC_PARA_BYTE(0);
    /*set cmd & para flag*/
    val |= BIT_LCDC_PARA_FLAG;
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CMD_INFO, val);

    //val = AIC_READ32_LCDC(REG_LCDC_CTRL_REG0);
    val = 0;
    val |= BIT_LCDC_WIRE1_COMBINE; //si/so share 1pad
    val &= ~ BIT_LCDC_PXLWR_CMD;
    val |= BIT_LCDC_BAUD_RATE(CLK_RATE);
    val |= lcdc_config.aic_wire_type & BIT_LCDC_LCD_3WIRE_FLAG;
//    val |= lcdc_config.aic_wire_type & BIT_LCDC_WIRE_TYPE;
    val |= BIT_LCDC_WRITE_OP;
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR0, val);

    /*para word*/
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_PARA_INFO, BIT_LCDC_PARA_WORD(para));

    lcdc_start();
    ret = lcdc_wait_finish();

    return ret;

}

int lcdc_cmd_set(u32 cmd)
{
    u32 val;
    int ret;

    check_st_idle();

    /*1.set cmd & para*/
    //val = AIC_READ32_LCDC(REG_LCDC_LCDC_CMD_INFO);
    val = 0;
    /*2.clear para & cmd*/
    val &= ~ BIT_LCDC_CMD_WORD(0xFF);
    /*3.set cmd & para*/
    val |= BIT_LCDC_CMD_WORD(cmd);
    /*4.set cmd & para flag*/
    val |= BIT_LCDC_CMD_FLAG;
    val &= ~BIT_LCDC_DUM_FLAG;
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CMD_INFO, val);

    //val = AIC_READ32_LCDC(REG_LCDC_CTRL_REG0);
    val = 0;
    val |= BIT_LCDC_WRITE_OP; //write option
    val |= BIT_LCDC_WIRE1_COMBINE; //si/so share 1pad
    val &= ~ BIT_LCDC_PXLWR_CMD;
    val |= BIT_LCDC_BAUD_RATE(CLK_RATE);
    val |= lcdc_config.aic_wire_type & BIT_LCDC_LCD_3WIRE_FLAG;
//    val |= lcdc_config.aic_wire_type & BIT_LCDC_WIRE_TYPE;

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR0, val);

    lcdc_start();
    ret = lcdc_wait_finish();

    return ret;
}

int lcdc_cmd_read(u32 cmd, u32 *data, bool dummy, int para_num)
{
    u32 val;
    int ret;

    if (para_num > 12)
        return AIC_ENFILE;

    check_st_idle();

    /*1.set cmd & para*/
    //val = AIC_READ32_LCDC(REG_LCDC_CMD_INFO);
    val = 0;
    /*2.clear para & cmd*/
    val &= ~ BIT_LCDC_CMD_WORD(0xFF);
    /*3.set cmd & para*/
    val |= BIT_LCDC_PARA_BYTE(para_num - 1);
    val |= BIT_LCDC_CMD_WORD(cmd);
    /*4.set cmd & para flag*/
    val |= BIT_LCDC_CMD_FLAG;
    if (dummy)
        val |= BIT_LCDC_DUM_FLAG;
    else
        val &= ~BIT_LCDC_DUM_FLAG;

    val |= BIT_LCDC_PARA_FLAG;
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CMD_INFO, val);

    //val = AIC_READ32_LCDC(REG_LCDC_CTRL_REG0);
    val = 0;
    val &= ~ BIT_LCDC_WRITE_OP; //read option
    val |= BIT_LCDC_WIRE1_COMBINE; //si/so share 1pad
    val &= ~ BIT_LCDC_PXLWR_CMD;
    val |= BIT_LCDC_BAUD_RATE(CLK_RATE);
    val |= lcdc_config.aic_wire_type & BIT_LCDC_LCD_3WIRE_FLAG;
//    val |= lcdc_config.aic_wire_type & BIT_LCDC_WIRE_TYPE;

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR0, val);

    lcdc_start();
    aic_mdelay(5);

    *data = AIC_READ32_LCDC(REG_LCDC_LCDC_ST0);
    log_print("%s(%d) cmd:0x%x ret val:0x%x\n", __func__, __LINE__, cmd, (*data));

    ret = lcdc_wait_finish();

    return ret;
}

/*
    0. unpack 32bit per pixel
    1. pack 23bit per pixel
*/
void lcdc_pixel_pack_mode(int pack)
{
    u32 val;

    check_st_idle();

    val = AIC_READ32_LCDC(REG_LCDC_LCDC_CR0);
    if (pack)
        val |= BIT_LCDC_PACK_MODE;
    else
        val &= ~BIT_LCDC_PACK_MODE;

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR0, val);
}

int lcdc_rstn(int high)
{
    if (high)
        AIC_WRITE32_LCDC(REG_LCDC_SSP_RST, BIT_LCDC_LCD_RSTN);
    else
        AIC_WRITE32_LCDC(REG_LCDC_SSP_RST, ~BIT_LCDC_LCD_RSTN);

    return 0;
}

int lcdc_para_wide(int wide)
{
    u32 val;
    if (wide > 4) {
        log_print("para wide ultra limit.\n");
        return -1;
    }

    wide = wide - 1;
    val = AIC_READ32_LCDC(REG_LCDC_LCDC_CMD_INFO);
    val &= ~ BIT_LCDC_PARA_BYTE(0xF);
    val |= BIT_LCDC_PARA_BYTE(wide);
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CMD_INFO, val);

    return 0;
}

u32 aic_lcdc_get_status_bypass_int(void)
{
    return AIC_READ32(REG_LCDC_SSP_INT_RAW);
}

void aic_lcdc_clear_int(void)
{
    /*lcdc clear int*/
    AIC_WRITE32(REG_LCDC_SSP_INT_CLEAR, 0XFFFF);
}

void lcdc_d_ness_timing(void)
{
    u32 val;

    val = 0;
    val |= BIT_LCDC_D_NSS(0x8);
    val |= BIT_LCDC_D_END(0x8);
    val |= BIT_LCDC_D_INIT(0x2);
    val |= BIT_LCDC_CAP_DAT_DLY(0x1);
    AIC_WRITE32_LCDC(REG_LCDC_LCDC_DELAY, val);
}

void aic_lcdc_trigger_mode_set(aic_lcdc_start_mode mode)
{

    u32 val;

    val = AIC_READ32_LCDC(REG_LCDC_LCDC_CR0);
    val &= ~ BIT_LCDC_LCD_START_MODE(0x3);
    val |= BIT_LCDC_LCD_START_MODE(mode);

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR0, val);
}

void aic_lcdc_next_ready(void)
{

    u32 val;
    int i;

    i = 10;
    val = 0;
    while(i--) {
    val = AIC_READ32_LCDC(BIT_LCDC_LCD_NXT_READY);
    if (val)
        log_print("lcdc not finish send cur frame!\n");
    else
        break;
    }

    if (!i)
        log_print("wait lcdc send cur frame timeout!\n");

    AIC_WRITE32_LCDC(BIT_LCDC_LCD_NXT_READY, val);
}

void lcdc_disable(void)
{
    AIC_WRITE32_LCDC(REG_MCU_GLB_LCDC_EB, 0x0);
}

static void lcdc_function_pin_init(void)
{
    AIC_WRITE32(REG_IOMUX_SPI_LCD_SDO, BIT_FUNC_SEL(0));
    AIC_WRITE32(REG_PG5_PAD_CTRL_SPI_LCD_SDO, BIT_PAD_DRV(5)|BIT_PAD_WPD|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z);
    AIC_WRITE32(REG_IOMUX_SPI_LCD_DCX, BIT_FUNC_SEL(0));
    AIC_WRITE32(REG_PG5_PAD_CTRL_SPI_LCD_DCX, BIT_PAD_DRV(5)|BIT_PAD_WPD|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z);
    AIC_WRITE32(REG_IOMUX_SPI_LCD_CS, BIT_FUNC_SEL(0));
    AIC_WRITE32(REG_PG5_PAD_CTRL_SPI_LCD_CS, BIT_PAD_DRV(5)|BIT_PAD_WPD|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z);
    AIC_WRITE32(REG_IOMUX_SPI_LCD_CLK, BIT_FUNC_SEL(0));
    AIC_WRITE32(REG_PG5_PAD_CTRL_SPI_LCD_CLK, BIT_PAD_DRV(5)|BIT_PAD_WPD|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z);
    AIC_WRITE32(REG_IOMUX_LCD_FMARK, BIT_FUNC_SEL(0));
    AIC_WRITE32(REG_PG5_PAD_CTRL_LCD_FMARK, BIT_PAD_DRV(1)|BIT_PAD_WPD|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z);
    AIC_WRITE32(REG_IOMUX_LCD_RSTN, BIT_FUNC_SEL(0));
    AIC_WRITE32(REG_PG5_PAD_CTRL_LCD_RSTN, BIT_PAD_DRV(1)|BIT_PAD_WPD|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPD|BIT_PAD_SLP_Z);
}

void lcdc_enable(void)
{
    u32 frame_total_bytes;

    lcdc_function_pin_init();

    /*switch lcdc div clk to 100M*/
    AIC_WRITE32_LCDC(REG_CRU_MCU_CLK_MCU_LCDC_CFG_CLR, BIT_CRU_MCU_SEL_CLK_MCU_LCDC_RF);
    AIC_WRITE32_LCDC(REG_MCU_GLB_LCDC_EB, BIT_MCU_GLB_LCDC_EB);
    lcdc_d_ness_timing();

    if (lcdc_config.aic_wire_type == AIC_LCDC_3WIRE_9BIT_2LANE)
        frame_total_bytes = LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * AIC_PIXEL_BYTES / 2;
    else
        frame_total_bytes = LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * AIC_PIXEL_BYTES;

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_IMG_TOTAL_BYTES, frame_total_bytes);
}
