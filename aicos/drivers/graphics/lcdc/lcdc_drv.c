/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of lcdc module.
 *   It includes all the lcdc operate interface.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x185d25
#define LOG_MODULE    AMOD_GPAPH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_interrupt.h>
#include <aic_reg_base.h>
#include <aic_osa.h>
#include "lcdc_reg.h"
#include "../lcdc.h"
#include <reg_base.h>
#include <top_glb.h>
#include <timer_inc.h>
#include <irq.h>
#include <irq_chip.h>
#include <chip_id.h>

#define LOG_TAG         "lcdc_driver"
#define LCDC_FINISH     0x1

//#define LCDC_REG_DEBUG
aos_event_ptr lcdc_refresh_finish;

bool display_init_success = false;
u32 (*lcdc_fsm_status)(void);
int lcdc_flip_done = 0;
int lcdc_fsm_error = 0;

#ifndef LCDC_REG_DEBUG
#define AIC_WRITE32_LCDC AIC_WRITE32
#define AIC_READ32_LCDC AIC_READ32
#else
void aic_write32_log(u32 addr, u32 value)
{
    aicx_error(0x19ce5e, "xx", "wirte32 reg:0x%x val:0x%x\n", addr, value);
    AIC_WRITE32(addr, value);
}

u32 aic_read32_log(u32 addr)
{
    u32 val;

    val = AIC_READ32(addr);
    aicx_error(0x01a9a7, "xx", "read32 reg:0x%x val:0x%x\n", addr, val);

    return val;
}
#define AIC_WRITE32_LCDC aic_write32_log
#define AIC_READ32_LCDC aic_read32_log
#endif
/* cmd + parameters offset*/
#define BITS_LCDC_PARA_BYTE_OFFSET(x,y)           ((x) << y * 8 & GENMASK(y * 8, y * 8 + 7))
u32 para_info_range[4] = {REG_LCDC_LCDC_PARA_INFO, REG_LCDC_LCDC_PARA_INFO2,
                          REG_LCDC_LCDC_PARA_INFO3, REG_LCDC_LCDC_PARA_INFO4};

struct lcdc_reg_val
{
    u32 lcdc_reg;
    u32 lcdc_reg_reset_value;
};

struct lcdc_reg_val lcdc_reg_group[] = {
    {REG_LCDC_LCDC_START,         0x00000000},
    {REG_LCDC_LCDC_CMD_INFO,      0x00000000},
    {REG_LCDC_LCDC_PARA_INFO,     0x00000000},
    {REG_LCDC_LCDC_DATA_INFO,     0x00080008},
    {REG_LCDC_LCDC_CR0,           0x03080504},
    {REG_LCDC_LCDC_ADDR,          0x00000000},
    {REG_LCDC_LCDC_CR1,           0x00000008},
    {REG_LCDC_LCDC_ST0,           0x00000000},
    {REG_LCDC_LCDC_ST1,           0x00000000},
    {REG_LCDC_SSP_INT_OUT,        0x00000000},
    {REG_LCDC_SSP_INT_MASK,       0x00000000},
    {REG_LCDC_SSP_INT_CLEAR,      0x00000000},
    {REG_LCDC_SSP_INT_RAW,        0x00000000},
    {REG_LCDC_SSP_RST,            0x00000000},
    {REG_LCDC_SSP_RDY,            0x00000000},
    {REG_LCDC_LCDC_DELAY,         0x00000000},
    {REG_LCDC_LCDC_PARA_INFO2,    0x00000000},
    {REG_LCDC_LCDC_PARA_INFO3,    0x00000000},
    {REG_LCDC_LCDC_PARA_INFO4,    0x00000000}
};

int lcdc_all_reg_reset_val(void)
{
    int i, ret;
    u32 val, check_val;

    ret = 0;
    check_val = 0;
    for (i = 0; i < sizeof(lcdc_reg_group) / sizeof(struct lcdc_reg_val); i++) {
        val = AIC_READ32_LCDC(lcdc_reg_group[i].lcdc_reg);
        if ( val != lcdc_reg_group[i].lcdc_reg_reset_value) {
            aicx_error(0x0785e5, "xxx", "reset val mismatch, lcdc reg:0x%x reset_val:0x%x actual_val:0x%x\n",
                        lcdc_reg_group[i].lcdc_reg, lcdc_reg_group[i].lcdc_reg_reset_value, val);
            check_val |= BIT(i);
        }
    }

    if(check_val)
        ret = AIC_ENOTSUP;

    return ret;
}

void lcdc_dump_all_reg(void)
{
#ifdef DISP_DEBUG
    int i, ret;
    u32 val, check_val;

    ret = 0;
    check_val = 0;
    aicx_error(0x16e3d6, "", "-----------------------lcdc dump regs-----------------------\n");
    for (i = 0; i < sizeof(lcdc_reg_group) / sizeof(struct lcdc_reg_val); i++) {
        val = AIC_READ32_LCDC(lcdc_reg_group[i].lcdc_reg);
        aicx_error(0x1df7f8, "xx", "lcdc reg:0x%x val:0x%x\n", lcdc_reg_group[i].lcdc_reg, val);
    }
#endif
}

u32 gx31x_lcdc_fsm(void)
{
    return GX31X_IDLE_STATUS(AIC_READ32(REG_LCDC_LCDC_ST1));
}

u32 gx30x_lcdc_fsm(void)
{
    return GX30X_IDLE_STATUS(AIC_READ32(REG_LCDC_LCDC_ST1));
}

void lcdc_sw_rst(void)
{
#ifdef LCDC_SW_RST_REG
        AIC_WRITE32(LCDC_SW_RST_REG, 0x1);
        AIC_WRITE32(LCDC_SW_RST_REG, 0x0);
        aic_udelay(2);
#endif
}

static void check_st_idle(void)
{
    aic_udelay(2);
    while(lcdc_fsm_status() != 0)
        ;
}

static void lcdc_start(void)
{
    u32 lcdc_finish_flag;

    check_st_idle();

    AIC_WRITE32(REG_LCDC_SSP_INT_CLEAR, 0XFFFF);

    if (aos_is_user_thread_context()) {
        aos_get_event(lcdc_refresh_finish, LCDC_FINISH, AOS_AND_CLEAR,
                      &lcdc_finish_flag, AOS_NO_WAIT);
    }

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_START, BIT_LCDC_LCDC_START);
}

static int lcdc_wait_finish(void)
{
    u32 lcdc_finish_flag;
    u32 val, begin_time;
    int ret = 0;

    if (aos_is_user_thread_context()) {
        ret = aos_get_event(lcdc_refresh_finish, LCDC_FINISH, AOS_AND_CLEAR,
                      &lcdc_finish_flag, LCDC_FINISH_TIMEOUT);
    } else {
        begin_time = aic_get_tick();
        while(1) {
            val = AIC_READ32(REG_LCDC_SSP_INT_RAW);
            if( val & BIT(0)) {
                lcdc_flip_done = 1;
                break;
            }

            if ((aic_get_tick() - begin_time) > LCDC_FINISH_TIMEOUT) {
                ret = AIC_ETIME;
                break;
            }
        }
        AIC_WRITE32(REG_LCDC_SSP_INT_CLEAR, 0XFFFF);
    }

    if (ret)
        aicx_warn(0x0ac335, "", "wait lcdc finish timeout!\n");

    return ret;
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

static void lcdc_frame_para_init(void)
{
    u32 frame_total_bytes;

    lcdc_d_ness_timing();

    if (lcdc_config.aic_wire_type == AIC_LCDC_3WIRE_9BIT_2LANE)
        frame_total_bytes = LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * AIC_PIXEL_BYTES / 2;
    else
        frame_total_bytes = LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * AIC_PIXEL_BYTES;

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_IMG_TOTAL_BYTES, frame_total_bytes);
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

    lcdc_frame_para_init();
    lcdc_fsm_error = 0;

    return 0;
}


int aic_lcdc_irq_handler(void *val)
{
    u32 irq_status;

    irq_status = AIC_READ32(REG_LCDC_SSP_INT_OUT);

    if (irq_status & BIT(0)) {
        lcdc_flip_done = 1;
        aos_set_event(lcdc_refresh_finish, LCDC_FINISH, AOS_OR);
    }

    if (irq_status & BIT_LCDC_INT_ERROR)
        aicx_error(0x1734c2, "sx", "%s lcdc error:0x%x\n", __func__, irq_status);

    /*lcdc clear int*/
    AIC_WRITE32(REG_LCDC_SSP_INT_CLEAR, 0XFFFF);

    display_init_success = true;

    return 0;
}

#ifdef CONFIG_SYNC_FLIP
bool sync_flip = true;

/*sync*/
int lcdc_frame_data_flip()
{
    int ret = 0;

    if (AIC_HW_TE_TRIGGER)
        AIC_WRITE32_LCDC(REG_LCDC_SSP_RDY, BIT_LCDC_LCD_NXT_READY);

    lcdc_start();
    lcdc_flip_done = 0;
    ret = lcdc_wait_finish();

    return ret;
}
int lcdc_frame_data_flip_sync()
{
    return lcdc_frame_data_flip();
}

#else
bool first_flip = true;
bool sync_flip = false;

void aic_lcdc_flip_init(void)
{
    lcdc_refresh_finish = aos_create_event("lcdc_refresh_finish");
    register_irq(IRQ_LCDC, (UISR_FUNC)aic_lcdc_irq_handler, NULL, NULL, NULL);
}

void aic_lcdc_flip_deinit(void)
{
    unregister_irq(IRQ_LCDC);
    aos_delete_event(lcdc_refresh_finish);
}

/*async*/
int lcdc_frame_data_flip()
{
    int ret = 0;

    if (sync_flip) {
        aic_lcdc_flip_deinit();
        aic_lcdc_flip_init();
        sync_flip = false;
    }

    if (first_flip) {
        first_flip = false;
        aic_mdelay(30);
    } else {
        ret = lcdc_wait_finish();
    }

    if (AIC_HW_TE_TRIGGER)
        AIC_WRITE32_LCDC(REG_LCDC_SSP_RDY, BIT_LCDC_LCD_NXT_READY);

    lcdc_dump_all_reg();

    lcdc_start();
    lcdc_flip_done = 0;

    return ret;
}

/*async flip mix cmd*/
int lcdc_frame_flip_mix_cmd()
{
    int ret = 0;

    if (sync_flip) {
        aic_lcdc_flip_deinit();
        aic_lcdc_flip_init();
        sync_flip = false;
    }

    if (first_flip) {
        first_flip = false;
    }

    if (AIC_HW_TE_TRIGGER)
        AIC_WRITE32_LCDC(REG_LCDC_SSP_RDY, BIT_LCDC_LCD_NXT_READY);

    lcdc_start();
    lcdc_flip_done = 0;

    return ret;
}

int lcdc_frame_data_flip_sync()
{
    int ret = 0;

    if (!sync_flip) {
        sync_flip = true;
        first_flip = true;
    }

    lcdc_start();
    lcdc_flip_done = 0;
    lcdc_wait_finish();
    check_st_idle();

    return ret;
}
#endif

int lcdc_cmd_para_set(u32 cmd, uint8_t *para, u32 para_wide)
{
    u32 val;
    int i, j, k;
    uint8_t *op;
    int ret = 0;

    check_st_idle();

    op = para;
    if (para_wide > 16) {
        aicx_error(0x0eb42e, "", "para wide ultra limit.\n");
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

    lcdc_dump_all_reg();
    lcdc_start();
    check_st_idle();

    //aic_mdelay(5);

    return ret;
}

int lcdc_para_byte_set(uint8_t para)
{
    u32 val;
    int ret = 0;

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

    lcdc_dump_all_reg();

    lcdc_start();
    check_st_idle();

    return ret;

}

int lcdc_cmd_set(u32 cmd)
{
    u32 val;
    int ret = 0;

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

    lcdc_dump_all_reg();
    lcdc_start();
    check_st_idle();

    return ret;
}

int lcdc_cmd_read(u32 cmd, u32 *data, bool dummy, int para_num)
{
    u32 val;

    if (para_num > 12)
        return AIC_ENFILE;

    check_st_idle();

    lcdc_sw_rst();

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

    lcdc_dump_all_reg();
    lcdc_start();

    check_st_idle();

    *data = AIC_READ32_LCDC(REG_LCDC_LCDC_ST0);
    aicx_error(0x18d1c6, "sdxx", "%s(%d) cmd:0x%x ret val:0x%x\n", __func__, __LINE__, cmd, (*data));

    return 0;
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

#ifdef LCD_RST_GPO_NUM
#include <gpo_inc.h>
int lcdc_rstn(int high)
{
    if (high)
        aic_gpo_output(LCD_RST_GPO_NUM, GPO_OUTPUT_HIGH);
    else
        aic_gpo_output(LCD_RST_GPO_NUM, GPO_OUTPUT_LOW);

    return 0;
}

#else
int lcdc_rstn(int high)
{
    if (high)
        AIC_WRITE32_LCDC(REG_LCDC_SSP_RST, BIT_LCDC_LCD_RSTN);
    else
        AIC_WRITE32_LCDC(REG_LCDC_SSP_RST, ~BIT_LCDC_LCD_RSTN);

    return 0;
}
#endif

int lcdc_para_wide(int wide)
{
    u32 val;
    if (wide > 4) {
        aicx_error(0x0eb42e, "", "para wide ultra limit.\n");
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

void aic_lcdc_irq_init(void)
{
    aos_request_irq(IRQ_LCDC, aic_lcdc_irq_handler, (void *)NULL);
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
        aicx_error(0x061797, "", "lcdc not finish send cur frame!\n");
    else
        break;
    }

    if (!i)
        aicx_error(0x173118, "", "wait lcdc send cur frame timeout!\n");

    AIC_WRITE32_LCDC(BIT_LCDC_LCD_NXT_READY, val);
}

void lcdc_disable(void)
{
    check_st_idle();

    if (!lcdc_flip_done) {
        lcdc_fsm_error = 1;
        aic_display_wait_ms(100);
    }

    AIC_WRITE32_LCDC(REG_LCDC_LCDC_CR0, 0x0);

#ifdef LCD_POWER_CONFIG_REG
    AIC_WRITE32(LCD_POWER_CONFIG_REG, 0x0);
    AIC_WRITE32(REG_CRU_MCU_CLK_EN_FORCE_CLK_MCU_AHB_LCDC_CFG, 0x0);
    AIC_WRITE32(REG_CRU_MCU_CLK_EN_FORCE_CLK_MCU_APB_LCDC_CFG, 0x0);
#endif

    AIC_WRITE32_LCDC(REG_MCU_GLB_LCDC_EB, 0x0);
    display_init_success = false;
}

void lcdc_enable(void)
{
#ifdef LCD_POWER_CONFIG_REG
    AIC_WRITE32(LCD_POWER_CONFIG_REG, LCD_POWER_CONFIG_VAL);
    AIC_WRITE32(REG_CRU_MCU_CLK_EN_FORCE_CLK_MCU_AHB_LCDC_CFG, LCD_POWER_CONFIG_VAL);
    AIC_WRITE32(REG_CRU_MCU_CLK_EN_FORCE_CLK_MCU_APB_LCDC_CFG, LCD_POWER_CONFIG_VAL);
#endif

    /*switch lcdc div clk to 100M*/
    AIC_WRITE32_LCDC(REG_CRU_MCU_CLK_MCU_LCDC_CFG_CLR, BIT_CRU_MCU_SEL_CLK_MCU_LCDC_RF);
    AIC_WRITE32_LCDC(REG_MCU_GLB_LCDC_EB, BIT_MCU_GLB_LCDC_EB);
#ifndef CONFIG_SYNC_FLIP
    first_flip = true;
#endif
}


int lcdc_init(void)
{
    u32 chip_id;

    chip_id = get_chip_id();
    if ((chip_id >> 16) == 0xA310) {
        lcdc_fsm_status = gx31x_lcdc_fsm;
    } else {
        lcdc_fsm_status = gx30x_lcdc_fsm;
    }

    register_irq(IRQ_LCDC, (UISR_FUNC)aic_lcdc_irq_handler, NULL, NULL, NULL);

    lcdc_refresh_finish = aos_create_event("lcdc_refresh_finish");
    AIC_ASSERT(lcdc_refresh_finish);

    return 0;
}

bool is_panel_init_success(void)
{
    return display_init_success;
}
