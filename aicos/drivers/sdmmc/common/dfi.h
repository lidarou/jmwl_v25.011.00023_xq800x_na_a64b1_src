/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef _DFI_H_
  #define _DFI_H_
  void dfi_configure();
  void dfi_release();
  void dfi_init();
  void dfi_hs();
  void dfi_ds();
  void dfi_ddr50();
  void dfi_sdr50();
  void dfi_sdr();
  void dfi_ddr();
  void dfi_hs400();
  void dfi_hs400es();
  void dfi_sdr104();
  void dfi_hs200(uint32_t tune_value);
  #define PHY_DQ_TIMING_REG 0x2000
    #define DATA_SELECT_OE_END 0
    #define IO_MASK_END 27
    #define IO_MASK_ALWAYS_ON 31 0
  #define PHY_DQS_TIMING_REG 0x2004
    #define PHONY_DQS_CMD 19
    #define PHONY_DQS 20
    #define LPBK_DQS 21
    #define EXT_LPBK_DQS 22
  #define PHY_GATE_LPBK_CTRL_REG 0x2008
    #define GATE_CFG_ALWAYS_ON 6
    #define UNDERRUN_SUPPRESS 18
    #define RD_DEL_SEL 19
    #define SW_HALF_CYCLE_SHIFT 28
    #define SYNC_METHOD 31
  #define PHY_DLL_MASTER_CTRL_REG 0x200C
    #define DLL_BYPASS_MODE 23
    #define DLL_START_POINT 0
  #define PHY_DLL_SLAVE_CTRL_REG 0x2010
    #define READ_DQS_CMD_DELAY 24
    #define CLK_WRDQS_DELAY 16
    #define CLK_WR_DELAY 8
    #define READ_DQS_DELAY 0
  #define PHY_DLL_OBS_REG_0 0x201C
    #define DLL_LOCKED_MODE 1
#endif
