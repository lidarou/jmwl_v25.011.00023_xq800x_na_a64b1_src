/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the sdhost global function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x01c57f
#define LOG_MODULE    AMOD_SDMMC

#include <stdint.h>
#include <string.h>
#include <aic_type.h>
#include "sdhost_global.h"
#include "dfi.h"
#include "sdmmc_common.h"
#include <aic_errno.h>
#include <aic_osa.h>
#include <cache.h>
#include <gpio_inc.h>
#include <eipu.h>
#ifdef CONFIG_PMIC_I2C
#include <pmu_user.h>
#endif
#include <sdmmc_core.h>
#include <irq_chip.h>
#include <irq.h>
#include <cache.h>
#include <sdmmc_config.h>

/* sd detect pin defined in sdmmc_config.h */
#define SD_DETECT_PIN     CUSTOMIZE_DETECT_PIN

/*  */
#define SD_EVENT_ALL      0xFFFFFFFF
#define SD_EVENT_CC       1<<0
#define SD_EVENT_TC       1<<1
#define SD_EVENT_BGE      1<<2
#define SD_EVENT_DINT     1<<3
#define SD_EVENT_BWR      1<<4
#define SD_EVENT_BRR      1<<5
#define SD_EVENT_CINT     1<<8
#define SD_EVENT_INT_A    1<<9
#define SD_EVENT_INT_B    1<<10
#define SD_EVENT_INT_C    1<<11
#define SD_EVENT_RTNGE    1<<12
#define SD_EVENT_EINT     1<<15
#define SD_EVENT_ETC      1<<16
#define SD_EVENT_ECCRC    1<<17
#define SD_EVENT_ECEB     1<<18
#define SD_EVENT_ECI      1<<19
#define SD_EVENT_EDT      1<<20
#define SD_EVENT_EDCRC    1<<21
#define SD_EVENT_EDEB     1<<22
#define SD_EVENT_ECL      1<<23
#define SD_EVENT_EA12     1<<24
#define SD_EVENT_EADMA    1<<25

#define SDMMC_INSERT   1<<0
#define SDMMC_REMOVE   1<<1
#define SDMMC_RPM      2<<1

volatile int sd_isr_counter = 0;
bool b_card_insert = AIC_FALSE;
unsigned int sd_card_rca;
aos_event_ptr sdmmc_detect_event;

#if 0
#define check(func) { \
 int status = func;  \
 if (status){\
   sd_printf("ERROR nr 0x%X, %s %d\n", status, __FILE__, __LINE__); \
   return (status);\
}}
#endif

int pmic_writel(u16 addr, u32 val);
void sdio_irq_wakeup(sdmmc_host *host);

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

void sdhost_set_phy_reg(uint32_t addr, uint32_t data) {
    AIC_WRITE32(SD_REG_HRS4,addr);
    //sd_printf("data is equal %x \n",data);
    AIC_WRITE32(SD_REG_HRS5,data);
}

uint32_t sdhost_read_phy_reg(uint32_t addr) {
    uint32_t data;
    AIC_WRITE32(SD_REG_HRS4,addr);
    data = AIC_READ32(SD_REG_HRS5);
    //sd_printf("data is equal %x \n",data);
    return data;
}

// ----------------------------------------------------------------------------
void sdhost_set_dlyvr(unsigned char addr, unsigned char data) {
  unsigned long _dlyrv_reg;

  _dlyrv_reg = addr | (data << 8);
  AIC_WRITE32(SD_REG_HRS4, _dlyrv_reg); //set data and address
  _dlyrv_reg |= (1<<24);
  AIC_WRITE32(SD_REG_HRS4, _dlyrv_reg); //send write request

  while (1) { // wait for acknowledge
    _dlyrv_reg = AIC_READ32(SD_REG_HRS4);
    if ((_dlyrv_reg >> 26) & 1)
      break;
  }

  _dlyrv_reg &= ~(1<<24);
  AIC_WRITE32(SD_REG_HRS4, _dlyrv_reg); //clear write request

    while (1) { // wait for acknowledge to be cleared
    _dlyrv_reg = AIC_READ32(SD_REG_HRS4);
    if (~(_dlyrv_reg >> 26) & 1)
      break;
  }

}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
unsigned char sdhost_read_dlyvr(unsigned char addr) {
  unsigned long _dlyrv_reg;

  _dlyrv_reg = AIC_READ32(SD_REG_HRS4) & 0xffffff00;
  _dlyrv_reg |= addr;
  AIC_WRITE32(SD_REG_HRS4, _dlyrv_reg);   //set address
  _dlyrv_reg |= (1<<25);
  AIC_WRITE32(SD_REG_HRS4, _dlyrv_reg);   //send read request
  //_dlyrv_reg = AIC_READ32(SD_REG_HRS4);  //get data

  while (1) { // wait for acknowledge
    _dlyrv_reg = AIC_READ32(SD_REG_HRS4);
    if ((_dlyrv_reg >> 26) & 1)
      break;
  }

  _dlyrv_reg &= ~(1<<25);
  AIC_WRITE32(SD_REG_HRS4, 0x00);         //clear read request

  //   sd_printf("_dlyrv_reg = 0x%x\r\n", _dlyrv_reg);

  return (unsigned char)(_dlyrv_reg >> 16);
}
// ----------------------------------------------------------------------------

#if defined(CONFIG_SDMMC_SD)&&(SD_DETECT_PIN != 0xFF)
static void sd_detect_callback(u32 id, u32 state, u32 pol)
{
    u32 current_detect_level;

    current_detect_level = EIPU_GetIndata(NOSEC_MODE, EIPU_DBNC, SD_DETECT_PIN);
    if (current_detect_level == LOW_LEVEL) {
        if (!b_card_insert) {
            b_card_insert = AIC_TRUE;
            aos_set_event(sdmmc_detect_event, SDMMC_INSERT, AOS_OR);
            //sd_printf("sdcard insert!\n");
        }
    } else {
        if (b_card_insert) {
            b_card_insert = AIC_FALSE;
            aos_set_event(sdmmc_detect_event, SDMMC_REMOVE, AOS_OR);
            //sd_printf("sdcard removed!\n");
        }
    }
    EIPU_SetPol(NOSEC_MODE, EIPU_DBNC, SD_DETECT_PIN, !current_detect_level);
}
#endif

void sdhost_send_rpm()
{
    aos_set_event(sdmmc_detect_event, SDMMC_RPM, AOS_OR);
}

u32 sdhost_detect_card(u32 wait_option)
{
    u32 actual_flags;
    aos_status status;

    status = aos_get_event(sdmmc_detect_event, SDMMC_INSERT | SDMMC_REMOVE | SDMMC_RPM, AOS_OR_CLEAR,
                           (u32 *)&actual_flags, wait_option);

    AOS_ASSERT (status == AOS_SUCCESS || status == AOS_NO_EVENTS);

    /* rpm check timeout */
    if (status == AOS_NO_EVENTS) {
        //sd_printf("sdhost_detect_card: wait timeout!\n");
        return SDMMC_DETECT_RPM;
    }

    if (actual_flags & SDMMC_INSERT) {
        //sd_printf("sdhost_detect_card: get sd insert event!");
        return SDMMC_DETECT_INSERT;
    } else if (actual_flags & SDMMC_REMOVE) {
        //sd_printf("sdhost_detect_card: get sd remove event!");
        return SDMMC_DETECT_REMOVE;
    } else if (actual_flags & SDMMC_RPM) {
        //sd_printf("sdhost_detect_card: get sd remove event!");
        return SDMMC_DETECT_RPM;
    } else {
        AIC_ASSERT(0);
        return 0;
    }
}


#if SDMMC_INT_ENABLE
static ISR_RET_E sd_interrupt_handler(void *arg)
{
    volatile uint32_t int_reg;
    uint32_t reg_status;
    sdmmc_host *host = (sdmmc_host *)arg;

    sd_isr_counter++;

    AIC_WRITE32(SD_REG_SRS14, 0); // disable interrupts
    int_reg = AIC_READ32(SD_REG_SRS12);
    //sd_printf("srs12_reg = 0x%X\r\n", int_reg);
    AIC_WRITE32(SD_REG_SRS12, int_reg);  // clear flags

#if 0
    if (int_reg & (1<<CC)) {
      sd_printf("INTR: Command Complete\r\n");
    }

    if (int_reg & (1<<TC))  {
      sd_printf("INTR: Transfer Complete\r\n");
    }

    if (int_reg & (1<<EINT))  {
      sd_printf("INTR: Error\r\n");
    }
#endif

    if (int_reg & (1<<DINT))  {
      sd_printf("INTR: DMA int\r\n");
      reg_status = AIC_READ32(SD_REG_SRS0);
      AIC_WRITE32 (SD_REG_SRS12, (1<<DINT));  // clear DINT flag
      AIC_WRITE32 (SD_REG_SRS0, reg_status);     // resume transmission with new system address
    }

#ifdef CONFIG_SDMMC_SDIO
        if (int_reg & (1<<CINT)) {
            sdio_irq_wakeup(host);
        }
#endif

    AIC_WRITE32(SD_REG_SRS12, int_reg);
    AIC_WRITE32(SD_REG_SRS14, (1<<CC) | (1<<TC) | (1<<EINT) | (1 << DINT)); // enable interrupts

    aos_set_event(host->sdmmc_event, int_reg, AOS_OR);

    return NONE_WORKER;
}
#endif

bool is_card_exist()
{
#if (SD_DETECT_PIN == 0xFF)
    return TRUE;
#else
    u32 detect_level;
    detect_level = EIPU_GetIndata(NOSEC_MODE, EIPU_DBNC, SD_DETECT_PIN);
    //sd_printf("sdcard detect pin status: %d\n", detect_level);
    if (!(detect_level & 1<<SD_DETECT_PIN)) {
       return TRUE;
    } else
        return FALSE;
#endif
}

void sdhost_disable_clk()
{
    uint32_t _srs11;

    _srs11 = AIC_READ32(SD_REG_SRS11);
    _srs11 &= ~(1<<SDCE);
    AIC_WRITE32(SD_REG_SRS11, _srs11 );  // enable SD clock (SRS11.SDCE)
    aicx_info(0x1700de, "", "SD clock disabled\r\n");
}

void sdhost_enable(boolean enable)
{
    AIC_WRITE32(REG_MCU_GLB_SDIOH_EB, enable);
    if (enable) {
        /* enable sdhc clk */
        AIC_WRITE32(REG_MCU_GLB_SDIOH_EB, BIT_MCU_GLB_SDIOH_EB);
        //sd_printf("Software reset...\r\n");
        AIC_WRITE32(SD_REG_HRS0, (1<<0) ) ;   // SWR = 1
        while ((AIC_READ32(SD_REG_HRS0) & (1<<0)) == 1);  //wait for HRS0.0 (SWR)= 0
        dfi_init();
        // enable flags
        AIC_WRITE32 (SD_REG_SRS13, 0xffffffff);
        AIC_WRITE32 (SD_REG_SRS1, SD_HOST_BLOCK_SIZE);  // block size

        /* adjust cmd/data delay t_SDCLK/2  */
        AIC_WRITE32(SD_REG_HRS16, 0x11110000);//CMD DRV

    }
}

#if 0
u16 fsm_data[30];
void sdhost_dump_fsm()
{
    u16 addr, i;

    /* save 0x0000-0x0016 */
    for(i=0; i<23; i++) {
        addr = i;
        AIC_WRITE32(SD_REG_HRS32, (1<<31 | addr<<16));
        fsm_data[i] = AIC_READ32(SD_REG_HRS32) & 0xFFFF;
        sd_printf("sdhost fsm dump: fsm[%u]: 0x%X\r\n", i, fsm_data[i]);
    }

    /* save 0x2000-0x2006 */
    for(i=0; i<7; i++) {
        addr = 0x2000 + i;
        AIC_WRITE32(SD_REG_HRS32, (1<<31 | addr<<16));
        fsm_data[23+i] = AIC_READ32(SD_REG_HRS32) & 0xFFFF;
        sd_printf("sdhost fsm dump: fsm[%u]: 0x%X\r\n", (23+i), fsm_data[23+i]);
    }

    AIC_WRITE32(SD_REG_HRS32, 0);
}

void sdhost_dump_hostreg()
{
    u32 i, reg_val;
    volatile uint32_t *reg_addr;

    /* enable sdmmc host controller */
    AIC_WRITE32(REG_MCU_GLB_SDIOH_EB, BIT_MCU_GLB_SDIOH_EB);

    aicx_info(0x05bd20, "", "sdhost dump host reg:\r\n");

    /* HRS00-14, HRS16, HRS29-34, HRS36, HRS40-43 test */
    reg_addr = (volatile uint32_t*)SD_REG_HRS_BASE;
    for (i=0; i<44; i++,reg_addr++) {
        if (i==15 || (i<29&&i>16) || i==35 || (i<40&&i>36))
            continue;
        reg_val = AIC_READ32(reg_addr);
        aicx_info(0x168cba, "uXX", "sdhost_dump_hostreg: HRS%02u[0x%08X] = 0x%08X\r\n",
                    i, reg_addr, reg_val);
    }

    /* SRS00-27, SRS30-31 test */
    reg_addr = (volatile uint32_t*)SD_REG_SRS_BASE;
    for (i=0; i<32; i++,reg_addr++) {
        if (i==28 || i==29)
            continue;
        reg_val = AIC_READ32(reg_addr);
        aicx_info(0x197264, "uXX", "sdhost_dump_hostreg: SRS%02u[0x%08X] = 0x%08X\r\n",
                    i, reg_addr, reg_val);
    }

    /* CRS63 RW test */
    reg_addr = (volatile uint32_t*)((SD_HOST_BASE + 0x2FC));
    reg_val = AIC_READ32(reg_addr);
    aicx_info(0x0cb1a2, "XX", "sdhost_dump_hostreg: CRS63[0x%08X] = 0x%08X\r\n",
                reg_addr, reg_val);

    /* CQRS00-14, CQRS16-18, CQRS20-23 test */
    reg_addr = (volatile uint32_t*)SD_REG_CQRS_BASE;
    for (i=0; i<24; i++,reg_addr++) {
        if (i==15 || i==19)
            continue;
        reg_val = AIC_READ32(reg_addr);
        aicx_info(0x071a69, "uXX", "sdhost_dump_hostreg: CQRS%02u[0x%08X] = 0x%08X\r\n",
                    i, reg_addr, reg_val);
    }

    aicx_info(0x15dd6c, "", "sdhost dump host reg finished!\r\n");

}

static uint32_t readPhyReg(uint32_t address)
{
    uint32_t value = 0;
    AIC_WRITE32(SD_REG_HRS4, address);
    value = AIC_READ32(SD_REG_HRS5);
    return value;
}

#if 0    /* Not Used Now */
static void writePhyReg(uint32_t address, uint32_t value)
{
    AIC_WRITE32(SD_REG_HRS4, address);
    AIC_WRITE32(SD_REG_HRS5, value);
    uint32_t readValue = readPhyReg(address);

    if (readValue != value) {
        AIC_PRINTF("Error, CCP: Writing failed!: address: 0x%x, value : 0x%x, readValue: 0x%x\r\n",
                 address, value, readValue);
    }
}
#endif


#ifdef CONFIG_SDMMC_FPGAPHY
#define DATA_TIMING_REG_PRINT(n)\
        alog_info("fpga_phy_reg_dump: DATA_TIMING_REG[0x%4X] = 0x%08X\r\n",\
                    n, readPhyReg(n));\

static void fpga_phy_reg_dump()
{
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
    aicx_info(0x16aa42, "XX", "fpga_phy_reg_dump: CONTROL_TIMING_REG[0x%4X] = 0x%08X\r\n",
                reg_addr, reg_val);

    /* Sampling and Delay Block Registers: */
    reg_addr = 0x2800;
    for (i=0; i<11; i++) {
        reg_val = readPhyReg(reg_addr);
        aicx_info(0x04229f, "XX", "fpga_phy_reg_dump: SAMP_DELAY_BLK_REG[0x%4X] = 0x%08X\r\n",
                    reg_addr, reg_val);
        reg_addr += 0x4;
    }

}
#else
static void soft_phy_reg_dump()
{
    u32 i, reg_val;
    uint32_t reg_addr;

    /* DLL PHY REG: 0x2000-0x2020, 0x2028, 0x2034, 0x2070, 0x2074 */
    reg_addr = 0x2000;
    for (i=0; i<9; i++) {
        reg_val = readPhyReg(reg_addr);
        aicx_info(0x06c8ee, "XX", "soft_phy_reg_dump: DLL_PHY_REG[0x%4X] = 0x%08X\r\n",
                    reg_addr, reg_val);
        reg_addr += 0x4;
    }
    aicx_info(0x06c8ee, "XX", "soft_phy_reg_dump: DLL_PHY_REG[0x%4X] = 0x%08X\r\n",
                0x2028, readPhyReg(0x2028));
    aicx_info(0x06c8ee, "XX", "soft_phy_reg_dump: DLL_PHY_REG[0x%4X] = 0x%08X\r\n",
                0x2034, readPhyReg(0x2034));
    aicx_info(0x06c8ee, "XX", "soft_phy_reg_dump: DLL_PHY_REG[0x%4X] = 0x%08X\r\n",
                0x2070, readPhyReg(0x2070));
    aicx_info(0x06c8ee, "XX", "soft_phy_reg_dump: DLL_PHY_REG[0x%4X] = 0x%08X\r\n",
                0x2074, readPhyReg(0x2074));

    /* CONTROL TIMING BLOCK REG: 0x2080-0x2094 */
    reg_addr = 0x2080;
    for (i=0; i<6; i++) {
        reg_val = readPhyReg(reg_addr);
        aicx_info(0x112c66, "XX", "soft_phy_reg_dump: CON_TIMING_BLK_REG[0x%4X] = 0x%08X\r\n",
                    reg_addr, reg_val);
        reg_addr += 0x4;
    }

}
#endif

void sdhost_dump_phyreg()
{

    /* enable sdmmc host controller */
    AIC_WRITE32(REG_MCU_GLB_SDIOH_EB, BIT_MCU_GLB_SDIOH_EB);

    aicx_info(0x043059, "", "sdhost dump phy reg:\r\n");
#ifdef CONFIG_SDMMC_FPGAPHY
    fpga_phy_reg_dump();
#else
    soft_phy_reg_dump();
#endif

    aicx_info(0x15d597, "", "sdhost dump phy reg finished!\r\n");
}
#endif

void sdhost_set_clk(sdmmc_host *host, uint32_t clock)
{
    uint32_t _sdclkfs;
    uint32_t _dtcvval;

    _sdclkfs = (SD_HOST_CLK / 2) / clock;
    if (clock < 400000)
        _sdclkfs = _sdclkfs + 1;    // adjust parameter
    _dtcvval = 0xc ;    /* data timeout value: t_sdmclk(100MHz)*2^(25+2) = 1.3S */

    aicx_info(0x18a9e2, "d", "Set SD clock to %d Hz\r\n", clock);
    // set clock
    AIC_WRITE32(SD_REG_SRS11, 0);   // disable SDCE
    AIC_WRITE32(SD_REG_SRS11, (_dtcvval << DTCV) | (_sdclkfs<<SDCLKFS) | (1<<ICE) );         // ICE (internal clock enable)
    while ((AIC_READ32(SD_REG_SRS11) & (1<<ICS)) == 0);                                // wait for ICS (internal clock stable)
    AIC_WRITE32(SD_REG_HRS9, AIC_READ32(SD_REG_HRS9) & ~0x00000001); // enable DLL reset
    AIC_WRITE32(SD_REG_HRS9, (AIC_READ32(SD_REG_HRS9) & 0xFFFFFFF7) | ((_sdclkfs > 0 ? 1<<3 : 0<<3)  )); // set extended_wr_mode
    AIC_WRITE32(SD_REG_HRS9, AIC_READ32(SD_REG_HRS9) | 1); // release DLL reset
    //AIC_WRITE32(SD_REG_HRS9, AIC_READ32(SD_REG_HRS9) | (3<<15)); // release DLL reset
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
    AIC_WRITE32(SD_REG_SRS11, (_dtcvval << DTCV) | (_sdclkfs<<SDCLKFS) | (1<<ICE) | (1<<SDCE) );  // enable SD clock (SRS11.SDCE)
    aicx_info(0x1299e5, "", "SD clock enabled\r\n");

    //host->io_cfg->clock = clock;
}

uint32_t sdhost_wait_event(sdmmc_host *host, uint32_t requested_flags, uint32_t *p_actual_flags)
{
    uint32_t status;

    status = aos_get_event(host->sdmmc_event, requested_flags, AOS_OR_CLEAR, (u32 *)p_actual_flags, 1000);
    return status;

}

uint32_t sdhost_send_cmd(sdmmc_host *host, sdmmc_cmd *cmd)
{
    uint32_t srs2, srs3;
#if SDMMC_INT_ENABLE
    uint32_t actual_flags, ret;
#else
    uint32_t reg_status
#endif
    uint32_t dma_flag = 0;

#if TRACE_SD_CMD_RSP_DETAILS
    sd_printf("---->> Issuing CMD%2d cmd_arg=0x%8X\r\n", cmd->cmd_code, cmd->arg);
    if (cmd->data)
        sd_printf("---->> blksize=%d, blk cnt=%d\r\n", cmd->data->blksize, cmd->data->blks);
#endif

    srs2 = cmd->arg;
    srs3 = (cmd->cmd_code << CI);

    switch (cmd->flags & 0xF) {
    case 0:
        srs3 |= (0 << RTS) | (0 << CRCCE);    /* no response */
        break;
    case RESP_R1B:
        srs3 |= (3 << RTS) | (3 << CRCCE);    /* R1b */
        break;
    case RESP_R2:
        srs3 |= (1 << RTS) | (1 << CRCCE);    /* R2 */
        break;
    case RESP_R3:
    case RESP_R4:
        srs3 |= (2 << RTS) | (0 << CRCCE);    /* R3/R4 */
        break;
    default:
        srs3 |= (2 << RTS) | (3 << CRCCE);    /* R1/R5/R6/R7 */
        break;
    }

    if (cmd->data) {
        srs3 |= (1 << DPS);    /* data present */
#if SDMMC_DMA_ENABLE
        if (cmd->cmd_code == READ_MULTIPLE_BLOCK ||
            cmd->cmd_code == WRITE_MULTIPLE_BLOCK ||
            ((cmd->cmd_code == SD_IO_RW_EXTENDED)&&(cmd->data->blks > 1))) {
            AIC_WRITE32 (SD_REG_SRS1, (7 << HDMABB) | (cmd->data->blks << BCCT) | cmd->data->blksize);
            AIC_WRITE32 (SD_REG_SRS0, (u32)cmd->data->buf);
            srs3 |= (1 << MSBS) | (1 << BCE) | (1 << DMAE);  /* multi blk/blk cnt enable/DMA enable */
#ifndef CONFIG_SDMMC_SDIO
            srs3 |= (1 << ACE);  /* auto CMD12 */
#endif
            dma_flag = 1;
        } else {
            AIC_WRITE32 (SD_REG_SRS1, (cmd->data->blks << BCCT) | cmd->data->blksize);
            if (cmd->data->blks > 1) {
                srs3 |= (1 << MSBS) | (1 << ACE) | (1 << BCE);  /* multi blk/auto CMD12/blk cnt enable */
            }
        }
#else
        AIC_WRITE32 (SD_REG_SRS1, (cmd->data->blks << BCCT) | cmd->data->blksize);
        if (cmd->data->blks > 1) {
            srs3 |= (1 << MSBS) | (1 << ACE) | (1 << BCE);  /* multi blk/auto CMD12/blk cnt enable */
        }
#endif
        if (cmd->data->flags == DATA_DIR_READ) {
            srs3 |= (1 << DTDS);    /* read */
            if (dma_flag)
                dcache_invcc_by_range((u32)(cmd->data->buf), cmd->data->blks*cmd->data->blksize);
        }else {
            srs3 |= (0 << DTDS);    /* write */
            if (dma_flag)
                dcache_clean_by_range((u32)(cmd->data->buf), cmd->data->blks*cmd->data->blksize);
        }
    }

#if TRACE_SD_CMD_RSP_DETAILS
    sd_printf("srs2 to write 0x%X, srs3 to write 0x%X\r\n", srs2, srs3);
#endif
#if 0
    cmd->resp[0] = SD_REG_SRS4;
    if((cmd->flags & 0xF) == RESP_R2) 
    {
        cmd->resp[1] = SD_REG_SRS5;
        cmd->resp[2] = SD_REG_SRS6;
        cmd->resp[3] = SD_REG_SRS7;
    }
#endif
    AIC_WRITE32 (SD_REG_SRS12, 0xffffffff);    /* clear all flags */
    AIC_WRITE32(SD_REG_SRS2, srs2);    /* set cmd art */

#if SDMMC_INT_ENABLE
    /* use int mode */
#ifdef CONFIG_SDMMC_SDIO
    AIC_WRITE32(SD_REG_SRS14, (1<<CC) | (1<<TC) | (1<<EINT) | (1 << DINT) | (1 << CINT)); // enable interrupts
#else
    AIC_WRITE32(SD_REG_SRS14, (1<<CC) | (1<<TC) | (1<<EINT) | (1 << DINT)); // enable interrupts
#endif
    AIC_WRITE32 (SD_REG_SRS3, srs3);        // execute command

    ret = sdhost_wait_event(host, SD_EVENT_EINT | SD_EVENT_CC , &actual_flags);
    if (ret) {
        sd_printf("sdhost_wait_event error! ret: 0x%X, cmd: %d\r\n", ret, cmd->cmd_code);
        return AOS_ERROR;
    }

    if (actual_flags & SD_EVENT_EINT) {
        sd_printf("send cmd error! cmd: %2d, cmd_arg: 0x%8X, actual_flags: 0x%8X\r\n",
                   cmd->cmd_code, cmd->arg, actual_flags);
        return actual_flags & 0xffff8000;
    }

#else
    /* use polling mode */
    AIC_WRITE32(SD_REG_SRS14, 0x00000000);  // disable interrupts
    AIC_WRITE32 (SD_REG_SRS3, srs3);        // execute command
    do {
      reg_status = AIC_READ32(SD_REG_SRS12);
      sd_delay(100);
    } while ((reg_status & ((1<<CC) | (1<<EINT)) ) == 0); // exit loop when CC (command complete) flag is set to 1

    reg_status = AIC_READ32(SD_REG_SRS12);
#if TRACE_SD_CMD_RSP_DETAILS
    sd_printf("<<---- send CMD status=0x%8X (CC=%0d EINT=%0d)\r\n",
    reg_status, (reg_status & (1<<CC)) != 0, (reg_status & (1<<EINT)) != 0);
#endif  //TRACE_SD_CMD_RSP_DETAILS

#endif

    cmd->resp[0] = AIC_READ32(SD_REG_SRS4);
    cmd->resp[1] = AIC_READ32(SD_REG_SRS5);
    cmd->resp[2] = AIC_READ32(SD_REG_SRS6);
    cmd->resp[3] = AIC_READ32(SD_REG_SRS7);

#if TRACE_SD_CMD_RSP_DETAILS
    sd_printf("response[0]: 0x%X\r\n", AIC_READ32(SD_REG_SRS4));
    sd_printf("response[1]: 0x%X\r\n", AIC_READ32(SD_REG_SRS5));
    sd_printf("response[2]: 0x%X\r\n", AIC_READ32(SD_REG_SRS6));
    sd_printf("response[3]: 0x%X\r\n", AIC_READ32(SD_REG_SRS7));
    sd_printf("actual event flags: 0x%X\r\n", actual_flags);
#endif

    return 0;
}

static uint32_t sdhost_xfer_data(sdmmc_host *host, sdmmc_cmd *cmd, sdmmc_data *data)
{
    uint32_t status;
    uint32_t xfer_size;
    uint32_t handled_size = 0;
    uint32_t *pBuf = AIC_NULL;

#if SDMMC_INT_ENABLE
    uint32_t actual_flags, ret;
#endif

#if SDMMC_DMA_ENABLE
    if (cmd->cmd_code == READ_MULTIPLE_BLOCK ||
        cmd->cmd_code == WRITE_MULTIPLE_BLOCK ||
        ((cmd->cmd_code == SD_IO_RW_EXTENDED)&&(cmd->data->blks > 1)))
        goto WAIT_TC;
#endif

    AIC_ASSERT(data);

    xfer_size = data->blks * data->blksize;

    pBuf = data->buf;
    if(data->flags & DATA_DIR_READ)
    {
        while(handled_size < xfer_size)
        {
            status = AIC_READ32(SD_REG_SRS9);
            if (status & 1<<BRE)
            {
                *pBuf++ = AIC_READ32 (SD_REG_SRS8);
                handled_size += 4;
            }
            else
            {
                status = AIC_READ32(SD_REG_SRS12);
                if (status & 1<<EINT) {
                    sd_printf("sdhost read error! status: 0x%X\r\n", status);
                    break;
                }
            }
        }
    }
    else 
    {
        while(handled_size < xfer_size)
        {
            status = AIC_READ32(SD_REG_SRS9);
            if (status & 1<<BWE)
            {
                AIC_WRITE32 (SD_REG_SRS8, *pBuf++);
                handled_size += 4;
            }
            else
            {
                status = AIC_READ32(SD_REG_SRS12);
                if (status & 1<<EINT) {
                    sd_printf("sdhost write error! status: 0x%X\r\n", status);
                    break;
                }
            }
        }
    }

#if SDMMC_DMA_ENABLE
WAIT_TC:
#endif

#if SDMMC_INT_ENABLE
    ret = sdhost_wait_event(host, SD_EVENT_TC, &actual_flags);
    if (ret)
        return AOS_ERROR;

    return AIC_SUCCESS;
#else
    // wait for end
    do {
          status = AIC_READ32(SD_REG_SRS12);
        } while ((status & (1<<TC) ) == 0); // exit loop when TC (transfer complete) flag is set to 1

    return AIC_SUCCESS;
#endif
}

static void sdhost_request(struct sdmmc_host_st *host, struct sdmmc_req_st *req)
{
    uint32_t ret;
    sdmmc_cmd *cmd;
    sdmmc_data *data;

    if(req->cmd == AIC_NULL)
    {
        goto out;
    }

    cmd = req->cmd;
    ret = sdhost_send_cmd(host,req->cmd);
    if(ret) {
        cmd->err = ret;
        goto out;
    }

    if(req->data)
    {
        data = req->data;
        if((data->blksize & 0x3) != 0)
        {
            goto out;
        }

        /*do transfer data*/
        ret = sdhost_xfer_data(host, cmd, data);
        if(ret != AIC_SUCCESS)
        {
            data->err = ret;
            goto out;
        }
    }

out:
    sdmmc_req_complete(host);
}

void sdhost_set_power(sdmmc_host *host, uint32_t voltage, boolean b_on)
{
#ifdef SD_POWER_SWITCH_GPIO
    /* GPIO11 for 1.8v, GPIO13 for 3.3v, active high */
    if (b_on) {
        if (VOLTAGE_3_3 == voltage) {
            AIC_WRITE32(SD_REG_SRS10, (7<<BVS) | (1<<BP));
            /* set to 3.3 */
            aic_gpio_output(13, GPIO_OUTPUT_LVL_LOW);
            aic_gpio_output(11, GPIO_OUTPUT_LVL_LOW);
        } else if (VOLTAGE_1_8 == voltage) {
            AIC_WRITE32(SD_REG_SRS10, (5<<BVS) | (1<<BP));
            AIC_WRITE32(SD_REG_SRS15, (0x01 << 19) );       // enable 1V8SE flag
            /* set to 1.8 */
            aic_gpio_output(11, GPIO_OUTPUT_LVL_HIGH);
            aic_gpio_output(13, GPIO_OUTPUT_LVL_HIGH);
        } else {
            sd_printf("voltage set error!\r\n");
        }
    } else {
            /* set to 3.3 */
            aic_gpio_output(11, GPIO_OUTPUT_LVL_LOW);
            aic_gpio_output(13, GPIO_OUTPUT_LVL_LOW);
    }
#else
    struct pmu_user *puser_io = (struct pmu_user *)host->power_io;
    struct pmu_user *puser_card = (struct pmu_user *)host->power_card;

    if (b_on) {
        /* turn on card power */
        pmu_voltage_set(puser_card, 3300000, 10000);
        pmu_enable(puser_card);

        /* turn on IO power */
        if (VOLTAGE_3_3 == voltage) {
            AIC_WRITE32(SD_REG_SRS10, (7<<BVS) | (1<<BP));
            /* turn on and set IO power to 3.3V */
            pmu_voltage_set(puser_io, 3000000, 10000);
            pmu_enable(puser_io);
            sd_printf("sdmmc power on 3.3V!\r\n");

#ifdef CONFIG_SDMMC_SDIO
            sdmmc_delay_ms(7);
            aic_gpio_output(32, GPIO_OUTPUT_LVL_LOW);
            sdmmc_delay_ms(2);
            aic_gpio_output(32, GPIO_OUTPUT_LVL_HIGH);
            sdmmc_delay_ms(50);
#endif
        } else if (VOLTAGE_1_8 == voltage) {
            AIC_WRITE32(SD_REG_SRS10, (5<<BVS) | (1<<BP));
            AIC_WRITE32(SD_REG_SRS15, (0x01 << 19) );       // enable 1V8SE flag
            /* turn on and set IO power to 1.8V */
            pmu_voltage_set(puser_io, 1800000, 10000);
            pmu_enable(puser_io);
            sd_printf("sdmmc power on 1.8V!\r\n");
        } else {
            sd_printf("voltage set error!\r\n");
        }
    } else {
            /* turn off IO power LDO12 */
            pmu_disable(puser_io);
            /* turn off card power LDO7 */
            pmu_disable(puser_card);
            sd_printf("sdmmc power off!\r\n");
    }
#endif
}

void sdhost_set_width(uint32_t width)
{
    uint32_t status;

    status = AIC_READ32(SD_REG_SRS10);

    switch (width) {
    case SDMMC_BUS_WIDTH_1:
        status &= ~(1<<DTW);
        status &= ~(1<<EDTW);
        sd_printf("Set sdhost bit width to 1 bit\r\n");
        break;
    case SDMMC_BUS_WIDTH_4:
        status |= (1<<DTW);
        status &= ~(1<<EDTW);
        sd_printf("Set sdhost bit width to 4 bit\r\n");
        break;
    case SDMMC_BUS_WIDTH_8:
        status |= (1<<EDTW);
        sd_printf("Set sdhost bit width to 8 bit\r\n");
        break;
    default:
        sd_printf("ERROR: wrong bit width selected\r\n");
    }

    AIC_WRITE32(SD_REG_SRS10, status);
}

static int32_t sdhost_get_card_status(struct sdmmc_host_st *host)
{
    return AIC_SUCCESS;
}

void sdhost_enable_sdio_irq(struct sdmmc_host_st *host, int32_t en)
{
    if (en) {
        AIC_WRITE32(SD_REG_SRS14, AIC_READ32(SD_REG_SRS14) | (1 << CINT));
        AIC_WRITE32(SD_REG_SRS13, AIC_READ32(SD_REG_SRS13) | (1 << CINT));
    }
    else {
        AIC_WRITE32(SD_REG_SRS14, AIC_READ32(SD_REG_SRS14) & ~(1 << CINT));
        AIC_WRITE32(SD_REG_SRS13, AIC_READ32(SD_REG_SRS13) & ~(1 << CINT));
    }
}

sdmmc_host_ops ops =
{
    sdhost_request,
    sdhost_get_card_status,
    sdhost_enable_sdio_irq
};

void sdhost_clk_src_sel(void)
{
    /* sdio clk source select: 0:200MHz, 1:125MHz, 2:26MHz, 4:100MHz */
    if (SD_HOST_CLK == 100000000)
        AIC_WRITE32(0x411E0030, 0x4);
    else if (SD_HOST_CLK == 200000000)
        AIC_WRITE32(0x411E0030, 0x0);
    else
        AIC_ASSERT(0);
}

void sdhost_init(void)
{
#if 0
    /* pin cfg for SDIO */
    AIC_WRITE32(0x41270024, 0x10403);//REB DRV
    AIC_WRITE32(0x41260010, 0x10432);//D0 DRV
    AIC_WRITE32(0x41260014, 0x10432);//D1 DRV
    AIC_WRITE32(0x41260018, 0x10432);//D2 DRV
    AIC_WRITE32(0x4126001C, 0x10432);//D3 DRV
    AIC_WRITE32(0x41260020, 0x08010403);//CLK DRV
    AIC_WRITE32(0x41260024, 0x10432);//CMD DRV
#endif

#ifdef SD_POWER_SWITCH_GPIO
    /* request gpio for 1.8/3.3v switch */
    aic_gpio_request(11);
    aic_gpio_request(13);
    AIC_WRITE32(0x40170110, 0x6);//GPIO11 for 1.8V
    AIC_WRITE32(0x4017013C, 0x6);//GPIO13 for 3.3V
#endif

    sdmmc_detect_event = aos_create_event("sdmmc_detect_event");
    AOS_ASSERT(sdmmc_detect_event);

#ifdef CONFIG_SDMMC_SD
#if (SD_DETECT_PIN == 0xFF)
    /* HW board not support hot-plug, so send insert event directly */
    aos_set_event(sdmmc_detect_event, SDMMC_INSERT, AOS_OR);
#else
    /* card insert: 0, card remove: 1 */
    EIPU_RegCallback(SD_DETECT_PIN, EIPU_DBNC,
                     sd_detect_callback,
                     LOW_LEVEL, 100, NOSEC_MODE);
#endif

#else
    /* SDIO/EMMC not support hot-plug, so send insert event directly */
    aos_set_event(sdmmc_detect_event, SDMMC_INSERT, AOS_OR);
#endif

}

void sdhost_config(sdmmc_host *host)
{
#if SDMMC_INT_ENABLE
    /* register irq handler */
    register_irq(SD_INT_NUM, sd_interrupt_handler, (void *)host, NULL, NULL);
#endif

    host->ops = &ops;

#ifdef CONFIG_SDMMC_SD
    host->power_io = (void *)pmu_get("pmu_sdmmc_io", "VDDSD3V");        /* IO power LDO12: VDDSD3V */
    host->power_card = (void *)pmu_get("pmu_sdmmc_card", "VDDBK3V3");    /* card power LDO7: VDDBK3V3 */
#endif

#ifdef CONFIG_SDMMC_SDIO
    host->power_io = (void *)pmu_get("pmu_sdmmc_io", "VDDBK3V3");        /* IO power LDO7: VDDBK3V3 */
    host->power_card = (void *)pmu_get("pmu_sdmmc_card", "VDDBK3V3");    /* card power LDO7: VDDBK3V3 */
    aic_gpio_request(32);    /* GPIO32 t0 control SDIO WIFI LDO_EN */
#endif
}

void sdhost_disconfig(sdmmc_host *host)
{
#if SDMMC_INT_ENABLE
    /* register irq handler */
    unregister_irq(SD_INT_NUM);
#endif

    host->ops = AIC_NULL;
    pmu_put(host->power_io);
    pmu_put(host->power_card);
}


