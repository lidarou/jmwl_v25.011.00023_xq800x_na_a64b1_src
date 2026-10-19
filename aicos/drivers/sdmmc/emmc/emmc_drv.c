/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15be58
#define LOG_MODULE    AMOD_SDMMC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* aic header */
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_printf.h>
#include <aic_bitmap.h>
#include <irq.h>
#include <reg_base.h>
#include <aic_case.h>
#include <emmc_case_cfg.h>
#include <mcu_glb.h>
#include <aic_assert.h>
#include <eipu.h>

#include "sv_tb/sw_hw_wrapper.h"
#include "sv_tb/clk_generator.h"
#include "sd_tests.h"
#include "sv_tb/svdpi.h"
#include "SD_Host_global.h"
#include "SD_Host_eMMC.h"
#include "sdmmc_common.h"
#include "dfi.h"
#include "phy_regs_fpga.h"

/**---------------------------------------------------------------------------*/
/**                            macros define                                  */
/**---------------------------------------------------------------------------*/
#define LOG_TAG "[EMMC]"

/* defined in Kconfig by default */
//#define CONFIG_SDMMC_FPGAPHY

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                           global variables                                */
/**---------------------------------------------------------------------------*/
volatile int emmc_isr_counter = 0;
void emmc_set_phase_shift();

/**---------------------------------------------------------------------------*/
/**                               functions                                   */
/**---------------------------------------------------------------------------*/
static uint32_t readPhyReg(uint32_t address)
{
    uint32_t value = 0;
    AIC_WRITE32(SD_REG_HRS4, address);
    value = AIC_READ32(SD_REG_HRS5);
    return value;
}

#if 0
#define DATA_TIMING_REG_PRINT(n)\
        sd_printf("FPGA PHY REG Value: DATA_TIMING_REG[0x%4X] = 0x%08X\n",\
                    n, readPhyReg(n));\

static int fpga_phy_reg_check()
{
    int ret = AIC_SUCCESS;
    u32 i, reg_val;
    uint32_t reg_addr;

    /* Data Timing Block Registers:
    0x2000, 0x2004, 0x2008, 0x2010, 0x2018, 0x201C,
    0x202C, 0x2030, 0x203C, 0x2070, 0x2074 */
    DATA_TIMING_REG_PRINT(0x2000);
    DATA_TIMING_REG_PRINT(0x2004);
    DATA_TIMING_REG_PRINT(0x2008);
    DATA_TIMING_REG_PRINT(0x2010);
    DATA_TIMING_REG_PRINT(0x2018);
    DATA_TIMING_REG_PRINT(0x201C);
    DATA_TIMING_REG_PRINT(0x202C);
    DATA_TIMING_REG_PRINT(0x2030);
    DATA_TIMING_REG_PRINT(0x203C);
    DATA_TIMING_REG_PRINT(0x2070);
    DATA_TIMING_REG_PRINT(0x2074);

    /* Control Timing Block Registers: 0x2080 */
    reg_addr = 0x2080;
    reg_val = readPhyReg(reg_addr);
    sd_printf("FPGA PHY REG Value: CONTROL_TIMING_REG[0x%4X] = 0x%08X\n",
                reg_addr, reg_val);

    /* Sampling and Delay Block Registers: */
    reg_addr = 0x2800;
    for (i=0; i<11; i++) {
        reg_val = readPhyReg(reg_addr);
        sd_printf("FPGA PHY REG Value: SAMP_DELAY_BLK_REG[0x%4X] = 0x%08X\n",
                    reg_addr, reg_val);
        reg_addr += 0x4;
    }

    return ret;
}

static int host_reg_check(u32 param)
{
    int ret = AIC_SUCCESS;
    u32 i, reg_val;
    volatile uint32_t *reg_addr;

    /* HRS00-14, HRS16, HRS29-34, HRS36, HRS40-43 test */
    reg_addr = (volatile uint32_t*)SD_REG_HRS_BASE;
    for (i=0; i<44; i++,reg_addr++) {
        if (i==15 || (i<29&&i>16) || i==35 || (i<40&&i>36))
            continue;
        reg_val = AIC_READ32(reg_addr);
        sd_printf("HOST REG Value: HRS%02u[0x%08X] = 0x%08X\n",
                    i, reg_addr, reg_val);
    }

    /* SRS00-27, SRS30-31 test */
    reg_addr = (volatile uint32_t*)SD_REG_SRS_BASE;
    for (i=0; i<32; i++,reg_addr++) {
        if (i==28 || i==29)
            continue;
        reg_val = AIC_READ32(reg_addr);
        sd_printf("HOST REG Value: SRS%02u[0x%08X] = 0x%08X\n",
                    i, reg_addr, reg_val);
    }

    /* CRS63 RW test */
    reg_addr = (volatile uint32_t*)((SD_HOST_BASE + 0x2FC));
    reg_val = AIC_READ32(reg_addr);
    sd_printf("HOST REG Value: CRS63[0x%08X] = 0x%08X\n",
                reg_addr, reg_val);

    /* CQRS00-14, CQRS16-18, CQRS20-23 test */
    reg_addr = (volatile uint32_t*)SD_REG_CQRS_BASE;
    for (i=0; i<24; i++,reg_addr++) {
        if (i==15 || i==19)
            continue;
        reg_val = AIC_READ32(reg_addr);
        sd_printf("HOST REG Value: CQRS%02u[0x%08X] = 0x%08X\n",
                    i, reg_addr, reg_val);
    }

    return ret;
}
#endif

static ISR_RET_E emmc_interrupt_handler(void* arg)
{
    emmc_isr_counter++;
    SDHost_mark_interrupts(arg);
    return NONE_WORKER;
}

uint32_t phase_shift = 0x0;
uint32_t read_dqs_delay = 0xCA;       //phy reg: 0x2010 [31:24]
uint32_t deskew_cmd_delay = 0x0;      //phy reg: 0x2030 [27:24]
uint32_t phase_shift_cmd = 0x7B;      //phy reg: 0x2800 [31:24]
uint32_t ext_lpbk_dqs_idelay = 0x0;   //phy reg: 0x2804 [15:8]
uint32_t cmd_idelay = 0x8A;           //phy reg: 2808 [23:16]
uint32_t cmd_odelay = 0x0;

void emmc_set_phase_shift()
{
    uint32_t temp;

    sd_printf("\ndeskew_cmd_delay=0x%X\n", deskew_cmd_delay);
    sd_printf("\nread_dqs_delay=0x%X\n", read_dqs_delay);
    sd_printf("\next_lpbk_dqs_idelay=0x%X\n", ext_lpbk_dqs_idelay);
    sd_printf("\ncmd_idelay=0x%X, cmd_odelay=0x%X\n", cmd_idelay, cmd_odelay);
    sd_printf("\nphase_shift_cmd=0x%X, phase_shift=0x%X\n", phase_shift_cmd, phase_shift);

    dfi_configure();
    set_cmd_delays(cmd_idelay, cmd_odelay);  //phy reg: 0x2808
    set_deskew_cmd_delays(deskew_cmd_delay);  //phy reg: 0x2030

    set_dll_slave_ctrl (read_dqs_delay);  //phy reg: 0x2010
    sd_set_phase_shift(phase_shift, phase_shift_cmd);  //phy reg: 0x2800
    set_extlpbk_dqs_delays (ext_lpbk_dqs_idelay);  //phy reg: 0x2804

    dfi_release();
    do {
        AIC_WRITE32(SD_REG_HRS4, 0x201C);
        temp = AIC_READ32(SD_REG_HRS5);
    } while(~temp & 0x1); // wait dll_lock to high

    sd_printf("\nphy reg[0x2808]=0x%X\n", readPhyReg(0x2808));
    sd_printf("\nphy reg[0x2030]=0x%X\n", readPhyReg(0x2030));
    sd_printf("\nphy reg[0x2010]=0x%X\n", readPhyReg(0x2010));
    sd_printf("\nphy reg[0x2800]=0x%X\n", readPhyReg(0x2800));
    sd_printf("\nphy reg[0x2804]=0x%X\n", readPhyReg(0x2804));
}

/**
 * emmc_common_test - emmc test common API.
 *
 * @dev_type: device type select, DEV_SD/DEV_EMMC/DEV_SDIO
 * @bus_width: bus width select, [3:0]BUS_WIDTH_1/BUS_WIDTH_4/BUS_WIDTH_8, [4]ddr, [5]e_strobe
 * @emmc_mode: emmc speed select, eMMC_MODE_SDR/eMMC_MODE_HS/eMMC_MODE_HS200/eMMC_MODE_400
 * @dma_mode: dma mode select, DMA_MODE_NONE/DMA_MODE_SDMA/DMA_MODE_ADMA2
 * @use_int: use interrupt or polling
 * @sector_addr: sector number to access
 * @size: data size to read/write
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int emmc_common_test(unsigned char dev_type,
                     unsigned char bus_width,
                     unsigned char emmc_mode,
                     unsigned char dma_mode,
                     boolean use_int,
                     uint32_t sector_addr,
                     uint32_t size)
{
    unsigned char* rd_buff;
    unsigned char* wr_buff;

    sdmmc_test_init();

    sd_printf("===== emmc_common_test ======\r\n");
    sd_printf("dev_type: %u\r\n", dev_type);
    sd_printf("bus_width: %u, ddr_mode: %u, es_mode: %u\r\n",
               bus_width&0xF, (bus_width&0x10)>>4, (bus_width&0x20)>>5);
    sd_printf("emmc_mode: %u\r\n", emmc_mode);
    sd_printf("dma_mode: %u\r\n", dma_mode);
    sd_printf("use_int: %u\r\n", use_int);
    sd_printf("sector addr: 0x%X, size: 0x%X\r\n", sector_addr, size);

    AIC_ASSERT(size <= BUFFER_SIZE);
    rd_buff = (unsigned char* )read_buffer;
    wr_buff = (unsigned char* )write_buffer;

    if (dev_type == DEV_EMMC)
    {
        SDHost_init(0);    /* not support uhs2 */

#ifdef CONFIG_SDMMC_FPGAPHY
        emmc_set_phase_shift();
#endif
        eMMC_Card_init(1);

        SDHost_card_read_state(sd_card_rca);
        eMMC_select_card (sd_card_rca);
        SDHost_card_read_state(sd_card_rca);

        eMMC_set_mode(bus_width, emmc_mode);

#if 1
        switch (emmc_mode) {
        case eMMC_MODE_SDR:
            dfi_sdr();
            break;
        case eMMC_MODE_HS:
            dfi_ddr();
            break;
        case eMMC_MODE_HS200:
#if 0
            dfi_hs200(0);
            #ifdef CONFIG_SDMMC_FPGAPHY
            sd_set_ctrl_odelay(0x1ff);
            #endif
            eMMC_card_tuning(4);
#endif
            break;
        case eMMC_MODE_HS400:
            if(bus_width & 1<<E_STROBE_BIT) {
                dfi_hs400es();
                #ifdef CONFIG_SDMMC_FPGAPHY
                sd_set_phase_shift(0x6,0x6);
                set_cmd_delays(0, 0xff);
                #endif
            } else {
                dfi_configure();
                AIC_WRITE32(SD_REG_HRS4,0x2010); // select phy_dll_slave_ctrl_reg
                AIC_WRITE32(SD_REG_HRS5, (IORD_32_SRS(SD_REG_HRS5) & 0xff0000ff)
                              | (192 << 16) // write command delay
                              | (64  <<  8) // write data delay
                              );
                dfi_release();
                #ifdef CONFIG_SDMMC_FPGAPHY
                set_cmd_delays(0, 0xff);
                #endif
                dfi_hs400();
                #ifdef CONFIG_SDMMC_FPGAPHY
                sd_set_dat_phase_shift(0x4);
                #endif
            }
            break;
        }
#endif

#ifdef CONFIG_SDMMC_FPGAPHY
    emmc_set_phase_shift();
#endif
    SDHost_set_clk(SD_TRAN_CLK);
    } else {
        sd_printf("dev_type parameter error!\r\n");
        return AIC_EFAULT;
    }

    // --------------------------------------------------------------------------
    sd_printf("************************************************\r\n");

    sd_printf("Fill write buffer...\r\n");
    fill_buffer(wr_buff, size, RND_TYPE_INC_B, 1);

    sd_printf("Write data...\r\n");
    if (SDHost_card_write(sector_addr, wr_buff, size, dma_mode, use_int) != AIC_SUCCESS)
        return AIC_EIO;

    sd_printf("Clear read buffer\r\n");
    fill_buffer(rd_buff, size, RND_TYPE_CONST, 0);

    sd_printf("Read data...\r\n");
    if (SDHost_card_read(sector_addr, rd_buff, size, dma_mode, use_int) != AIC_SUCCESS)
        return AIC_EIO;

    // Compare
    sd_printf("Compare buffers\r\n");
    if (compare_buffers(wr_buff, rd_buff, size) == 0) {
        sd_printf("Test passed\r\n");
    } else {
        sd_printf("Test failed\r\n");
        return AIC_EFAULT;
    }

    sd_printf("************************************************\r\n");

    // --------------------------------------------------------------------------
    eMMC_select_card (0x0000);  // deselect
    SDHost_card_read_state(sd_card_rca);
    // To avoid timing violations on Card when next test is starting with hardware reset, and switching off power from card, switch off clock
    AIC_WRITE32(SD_REG_SRS11,0);
    sd_delay(100);

    sd_printf("Test completed\r\n");
    return AIC_SUCCESS;
}

/**---------------------------------------------------------------------------*/
/**                           emmc test cases                                 */
/**---------------------------------------------------------------------------*/
/* case 1: emmc single blk (non-dma/8bit/ddr) test */
static int emmc_singleblk_test(u32 param)
{
    int ret = AIC_SUCCESS;

    sd_printf("\ncase 1: emmc single blk (non-dma/8bit/ddr) test\n");
    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8 | 1<<DDR_MODE_BIT,
                           eMMC_MODE_HS, DMA_MODE_NONE,
                           AIC_FALSE, 0, 512);

    sd_printf("emmc_singleblk_test ret = %d\n", ret);

    return ret;
}

/* case 2: emmc multi blk (non-dma/8bit/ddr) test with */
static int emmc_multiblk_test(u32 param)
{
    int ret = AIC_SUCCESS;

    sd_printf("\ncase 2: emmc multi blk (non-dma/8bit/ddr) test\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8 | 1<<DDR_MODE_BIT,
                           eMMC_MODE_HS, DMA_MODE_NONE,
                           AIC_FALSE, 0, 4096);


    sd_printf("emmc_multiblk_test ret = %d\n", ret);
    return ret;
}

/* case 3: emmc sdma (sdma/8bit/ddr) test */
static int emmc_sdma_test(u32 param)
{
    int ret = AIC_SUCCESS;

    sd_printf("\ncase 3: emmc sdma (sdma/8bit/ddr) test ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8 | 1<<DDR_MODE_BIT,
                           eMMC_MODE_HS, DMA_MODE_SDMA,
                           AIC_FALSE, 0, 4096);

    sd_printf("emmc_sdma_test ret = %d\n", ret);
    return ret;
}

/* case 4: emmc adma2 (sdma/8bit/ddr) test */
static int emmc_adma2_test(u32 param)
{
    int ret = AIC_SUCCESS;

    sd_printf("\ncase 4: emmc adma2 (sdma/8bit/ddr) test ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8 | 1<<DDR_MODE_BIT,
                           eMMC_MODE_HS, DMA_MODE_ADMA2,
                           AIC_FALSE, 0, 4096);

    sd_printf("emmc_adma2_test ret = %d\n", ret);
    return ret;
}

/* case 5: emmc 1bit (sdma/sdr) test */
static int emmc_1bit_test(u32 param)
{
    int ret = AIC_EFAULT;

    sd_printf("\ncase 5: emmc 1bit (sdma/sdr) test begin ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_1,
                           eMMC_MODE_SDR, DMA_MODE_SDMA,
                           AIC_FALSE, 0, 4096);

    sd_printf("emmc_1bit_test ret = %d\n", ret);
    return ret;
}

/* case 6: emmc 4bit (sdma/sdr) test */
static int emmc_4bit_test(u32 param)
{
    int ret = AIC_EFAULT;

    sd_printf("\ncase 6: emmc 4bit (sdma/sdr) test begin ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_4,
                           eMMC_MODE_SDR, DMA_MODE_SDMA,
                           AIC_FALSE, 0, 4096);

    sd_printf("emmc_4bit_test ret = %d\n", ret);
    return ret;
}

/* case 7: emmc sdr_bc (sdma/8bit) mode test */
static int emmc_sdr_bc_test(u32 param)
{
    int ret = AIC_EFAULT;
    sd_printf("\ncase 7: emmc sdr_bc mode (sdma/8bit) test begin ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8,
                           eMMC_MODE_SDR, DMA_MODE_SDMA,
                           AIC_FALSE, 0, 4096);

    sd_printf("emmc_sdr_bc_test ret = %d\n", ret);

    return ret;
}

/* case 8: emmc sdr (sdma/8bit) mode test */
static int emmc_sdr_test(u32 param)
{
    int ret = AIC_EFAULT;

    sd_printf("\ncase 8: emmc sdr (sdma/8bit) mode test begin ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8,
                           eMMC_MODE_HS, DMA_MODE_SDMA,
                           AIC_FALSE, 0, 4096);

    sd_printf("emmc_sdr_test ret = %d\n", ret);

    return ret;
}

/* case 9: emmc ddr (sdma/8bit) mode test */
static int emmc_ddr_test(u32 param)
{
    int ret = AIC_EFAULT;

    sd_printf("\ncase 9: emmc ddr (sdma/8bit) mode test begin ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8 | 1<<DDR_MODE_BIT,
                           eMMC_MODE_HS, DMA_MODE_NONE,
                           AIC_FALSE, 0, 512);

    sd_printf("emmc_ddr_test ret = %d\n", ret);

    return ret;
}

/* case 10: emmc hs200 (sdma/8bit) mode test */
static int emmc_hs200_test(u32 param)
{
    int ret = AIC_EFAULT;

    sd_printf("\ncase 10: emmc hs200 (sdma/8bit) mode test begin ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8,
                           eMMC_MODE_HS200, DMA_MODE_SDMA,
                           AIC_FALSE, 0, 4096);

    sd_printf("emmc_hs200_test ret = %d\n", ret);

    return ret;
}

/* case 11: emmc hs400 (sdma/8bit) mode test */
static int emmc_hs400_test(u32 param)
{
    int ret = AIC_EFAULT;

    sd_printf("\ncase 11: emmc hs400 (sdma/8bit) mode test begin ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8 | 1<<DDR_MODE_BIT,
                           eMMC_MODE_HS400, DMA_MODE_SDMA,
                           AIC_FALSE, 0, 4096);

    sd_printf("emmc_hs400_test ret = %d\n", ret);

    return ret;
}

/* case 12: emmc hs400 with enhanced strobe (sdma/8bit) mode test */
static int emmc_hs400es_test(u32 param)
{
    int ret = AIC_EFAULT;

    sd_printf("\ncase 12: emmc hs400 with enhanced strobe (sdma/8bit) mode test begin ...\n");

    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8 | 1<<DDR_MODE_BIT | 1<<E_STROBE_BIT,
                           eMMC_MODE_HS400, DMA_MODE_SDMA,
                           AIC_FALSE, 0, 512);

    sd_printf("emmc_hs400es_test ret = %d\n", ret);

    return ret;
}

/* case 13: emmc interrupt test */
static int emmc_int_test(u32 param)
{
    int ret = AIC_EFAULT;

    sd_printf("\ncase 13: emmc interrupt test begin ...\n");

    /* register irq handler */
    register_irq(SD_INT_NUM, emmc_interrupt_handler, (void *)NULL, NULL, NULL);

//    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8 | 1<<DDR_MODE_BIT,
    ret = emmc_common_test(DEV_EMMC, BUS_WIDTH_8,
                           eMMC_MODE_HS, DMA_MODE_SDMA,
                           AIC_TRUE, 0, 512);

    if (emmc_isr_counter && ret == AIC_SUCCESS) {
        ret = AIC_SUCCESS;
        sd_printf("emmc_int_test: isr handler called %u times!\n", emmc_isr_counter);
    } else
        ret = AIC_EFAULT;

    sd_printf("emmc_int_test ret = %d\n", ret);
    return ret;
}

aic_case_register(emmc_case, NULL);

