/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the sdmmc dfi function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16d7e7
#define LOG_MODULE    AMOD_SDMMC

#include <stdlib.h>
#include "sdhost_global.h"
#include "dfi.h"

  void dfi_configure()
  {
    AIC_WRITE32(SD_REG_HRS9, AIC_READ32(SD_REG_HRS9) & ~1); // enable DLL reset
  }
  void dfi_release()
  {
    AIC_WRITE32(SD_REG_HRS9, AIC_READ32(SD_REG_HRS9) | 1); // release DLL reset
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
  }
void dfi_init()  {
#ifdef CONFIG_SDMMC_FPGAPHY
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (1 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0 << READ_DQS_CMD_DELAY)
              | (0 << CLK_WRDQS_DELAY)
              | (0 <<  CLK_WR_DELAY)
              | (0 << READ_DQS_DELAY)
              );
  AIC_WRITE32(SD_REG_HRS4, 0x2080);
  temp = AIC_READ32(SD_REG_HRS5);
  temp &= ~(0x3f << 4); // phony_dqs_timing=0
  temp |= 3<<4;
  AIC_WRITE32(SD_REG_HRS5, temp);
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  dfi_release();

  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (1 << 3)
              | (1 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (1 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (0 << 28)
              | (0 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (1 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (9 << 16)
              | (0 << 0));
#else
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (0 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << 18)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (1 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0 << READ_DQS_CMD_DELAY)
              | (0 << CLK_WRDQS_DELAY)
              | (0 <<  CLK_WR_DELAY)
              | (0 << READ_DQS_DELAY)
              );
  AIC_WRITE32(SD_REG_HRS4, 0x2080);
  temp = AIC_READ32(SD_REG_HRS5);
  temp &= ~(0x3f << 4); // phony_dqs_timing=0
//  temp |= 3<<4;
  AIC_WRITE32(SD_REG_HRS5, temp);
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
//  dfi_release();

  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (1 << 3)
              | (1 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (1 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (0 << 28)
              | (0 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (1 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (9 << 16)
              | (0 << 0));

#if 0
/* host and asicPhy Settings for chip from romcode */
/* default host settings from ASIC
 * HRS07(0x001C): bit[20:16]=8, bit[4:0]=0
 * HRS09(0x0024): bit[16:15]=3, bit[3:2]=3
 * HRS10(0x0028): bit[19:16]=3
 * HRS16(0x0040): bit[31:0]=0
 * SRS11(0x022C): bit[15:8]=2, bit[7:6]=0
 */
    u32 tmp;

    tmp = AIC_READ32(SD_REG_HRS7);
    tmp &= ~(0x1F<<16 | 0x1F);
    tmp |= (0x8<<16);
    AIC_WRITE32(SD_REG_HRS7, tmp);

    tmp = AIC_READ32(SD_REG_HRS9);
    tmp &= ~(0x3<<15 | 0x3<<2);
    tmp |= (0x3<<15 | 0x3<<2);
    AIC_WRITE32(SD_REG_HRS9, tmp);

    tmp = AIC_READ32(SD_REG_HRS10);
    tmp &= ~(0xF<<16);
    tmp |= (0x3<<16);
    AIC_WRITE32(SD_REG_HRS10, tmp);

    AIC_WRITE32(SD_REG_HRS16, 0x0);

    tmp = AIC_READ32(SD_REG_SRS11);
    tmp &= ~(0xFF<<8 | 0x3<<6);
    tmp |= (0x2<<8);
    AIC_WRITE32(SD_REG_SRS11, tmp);

/* default asicphy settings from ASIC
 * 0x2000: bit[31]=0, bit[29:27]=0, bit[26:24]=0, bit[2:0]=1
 * 0x2004: bit[22:19]=7
 * 0x2008: bit[31]=1, bit[23:19]=20, bit[18]=1, bit[6]=1
 * 0x200C: bit[23]=1, bit[22:20]=2, bit[7:0]=4
 * 0x2010: bit[31:24]=0, bit[15:8]=0, bit[7:0]=0
 * 0x2034: bit[4]=0
 */
    dfi_configure();

    tmp = SDHost_read_phy_reg(0x2000);
    tmp &= ~(0x1<<31 | 0x7<<27 | 0x7<<24 | 0x7);
    tmp |= 0x1;
    SDHost_set_phy_reg(0x2000, tmp);

    tmp = SDHost_read_phy_reg(0x2004);
    tmp &= ~(0xF<<19);
    tmp |= (0x7<<19);
    SDHost_set_phy_reg(0x2004, tmp);

    tmp = SDHost_read_phy_reg(0x2008);
    tmp &= ~(0x1<<31 | 0x1F<<19 | 0x1<<18 | 0x1<<6);
    tmp |= (0x1<<31 | 0x14<<19 | 0x1<<18 | 0x1<<6);
    SDHost_set_phy_reg(0x2008, tmp);

    tmp = SDHost_read_phy_reg(0x200C);
    tmp &= ~(0x1<<23 | 0x7<<20 | 0xFF);
    tmp |= (0x1<<23 | 0x2<<20 | 0x4);
    SDHost_set_phy_reg(0x200C, tmp);

    tmp = SDHost_read_phy_reg(0x2010);
    tmp &= ~(0xFF<<24 | 0xFF<<8 | 0xFF);
    SDHost_set_phy_reg(0x2010, tmp);

    tmp = SDHost_read_phy_reg(0x2034);
    tmp &= ~(0x1<<4);
    SDHost_set_phy_reg(0x2034, tmp);

    dfi_release();
#endif
#endif
}
void dfi_ds()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (1 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0 << READ_DQS_CMD_DELAY)
              | (0 << CLK_WRDQS_DELAY)
              | (0 <<  CLK_WR_DELAY)
              | (0 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (1 << 3)
              | (1 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (2 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (0 << 28)
              | (0 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (1 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (9 << 16)
              | (0 << 0));
}
void dfi_hs()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (1 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0 << READ_DQS_CMD_DELAY)
              | (0 << CLK_WRDQS_DELAY)
              | (0 <<  CLK_WR_DELAY)
              | (0 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (1 << 3)
              | (1 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (3 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (0 << 28)
              | (0 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (0 << 8)
              | (0 << 4)
              | (0 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (8 << 16)
              | (0 << 0));
}
void dfi_sdr50()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (1 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0 << READ_DQS_CMD_DELAY)
              | (0 << CLK_WRDQS_DELAY)
              | (0 <<  CLK_WR_DELAY)
              | (0 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (1 << 3)
              | (1 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (5 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (0 << 28)
              | (0 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (0 << 8)
              | (0 << 4)
              | (0 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (8 << 16)
              | (0 << 0));
}
void dfi_sdr104()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (0 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0 << READ_DQS_CMD_DELAY)
              | (77 << CLK_WRDQS_DELAY)
              | (77 <<  CLK_WR_DELAY)
              | (0 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (0 << 3)
              | (0 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (8 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (0 << 28)
              | (0 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (1 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (9 << 16)
              | (0 << 0));
}
void dfi_ddr50()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (1 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0 << READ_DQS_CMD_DELAY)
              | (0 << CLK_WRDQS_DELAY)
              | (0 <<  CLK_WR_DELAY)
              | (0 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (1 << 3)
              | (1 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (3 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (1 << 28)
              | (1 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (0 << 8)
              | (0 << 4)
              | (0 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (8 << 16)
              | (0 << 0));
}
void dfi_sdr()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (1 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0 << READ_DQS_CMD_DELAY)
              | (0 << CLK_WRDQS_DELAY)
              | (0 <<  CLK_WR_DELAY)
              | (0 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (1 << 3)
              | (1 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (2 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (0 << 28)
              | (0 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (1 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (9 << 16)
              | (0 << 0));
}
void dfi_ddr()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (1 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (0xC8 << READ_DQS_CMD_DELAY)
              | (0 << CLK_WRDQS_DELAY)
              | (0 <<  CLK_WR_DELAY)
              | (0xC8 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (1 << 3)
              | (1 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (2 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (1 << 28)
              | (1 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (0 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (8 << 16)
              | (0 << 0));
}
void dfi_hs200(uint32_t tune_val)  {
volatile uint32_t _tune_val;
 _tune_val = 0xff & ((tune_val<<8)/40); // N * 256 / 40 
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (1 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (0 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (_tune_val << READ_DQS_CMD_DELAY)
              | (77 << CLK_WRDQS_DELAY)
              | (77 <<  CLK_WR_DELAY)
              | (_tune_val << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (0 << 3)
              | (0 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (8 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (0 << 28)
              | (0 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (1 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (9 << 16)
              | (0 << 0));
}
void dfi_hs400()  {
  uint32_t temp;
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  temp = AIC_READ32(SD_REG_HRS5) & 0xFF000000;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (0 << PHONY_DQS) 
              | (1 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (0 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                temp
              | (77 << CLK_WRDQS_DELAY)
              | (75 <<  CLK_WR_DELAY)
              | (64 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (0 << 3)
              | (0 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (9 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (1 << 28)
              | (1 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (0 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (8 << 16)
              | (0 << 0));
}
void dfi_hs400es()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << EXT_LPBK_DQS) 
              | (1 << LPBK_DQS) 
              | (0 << PHONY_DQS) 
              | (0 << PHONY_DQS_CMD)); 

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, 
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << GATE_CFG_ALWAYS_ON)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_MASTER_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5, (AIC_READ32(SD_REG_HRS5)
                            & ~((0x1<<23) | (0x7<<20) | (0xff<<0)))
              | (0 << DLL_BYPASS_MODE)
              | (2 << 20)
              | (4 <<  DLL_START_POINT)
              );
  AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (64 << READ_DQS_CMD_DELAY)
              | (77 << CLK_WRDQS_DELAY)
              | (75 <<  CLK_WR_DELAY)
              | (64 << READ_DQS_DELAY)
              );
  dfi_release();
  temp = SDHost_read_phy_reg(0x00002000) & 0x07FFFF8;
  SDHost_set_phy_reg(0x00002000, temp  
              | (0 << 31)
              | (1 << 27)
              | (0 << 24)
              | (2 << 0));
  temp=AIC_READ32(SD_REG_HRS9);
  temp&=0xFFFE7FF3;
  AIC_WRITE32(SD_REG_HRS9,temp
              | (1 << 16)
              | (1 << 15)
              | (0 << 3)
              | (0 << 2));
  temp=AIC_READ32(SD_REG_HRS10);
  temp&=0xFFF0FFFF;
  AIC_WRITE32(SD_REG_HRS10,temp
              | (9 << 16));
  AIC_WRITE32(SD_REG_HRS16,
                (1 << 28)
              | (1 << 24)
              | (0 << 20)
              | (0 << 16)
              | (0 << 12)
              | (0 << 8)
              | (0 << 4)
              | (1 << 0));
  AIC_WRITE32(SD_REG_HRS7,
                (8 << 16)
              | (0 << 0));
}
