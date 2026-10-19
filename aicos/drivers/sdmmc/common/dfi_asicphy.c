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
#define AIC_THIS_FILE    0x0361f9
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
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5,
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp &= ~(0x3f << 4);
  temp |= ((0x3<<4) | (0x1<<0));
  AIC_WRITE32(SD_REG_HRS5, temp);
  dfi_release();
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
}
void dfi_ds()  {
  uint32_t temp;
  dfi_configure();
  AIC_WRITE32(SD_REG_HRS4,PHY_DQS_TIMING_REG); // select phy_dqs_timing_reg - enable phony dqs for card init.
  AIC_WRITE32(SD_REG_HRS5,
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (1 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (1 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (0 << PHONY_DQS)
              | (1 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
                (0 << EXT_LPBK_DQS)
              | (0 << LPBK_DQS)
              | (0 << PHONY_DQS)
              | (0 << PHONY_DQS_CMD));

  AIC_WRITE32(SD_REG_HRS4, PHY_GATE_LPBK_CTRL_REG);
  AIC_WRITE32(SD_REG_HRS5,
                (1 << SYNC_METHOD)
              | (0 << SW_HALF_CYCLE_SHIFT)
              | (52 << RD_DEL_SEL)
              | (1 << UNDERRUN_SUPPRESS)
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
  temp = sdhost_read_phy_reg(0x00002000) & 0x07FFFF8;
  sdhost_set_phy_reg(0x00002000, temp
              | (0 << 31)
              | (0 << 27)
              | (0 << 24)
              | (1 << 0));
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
