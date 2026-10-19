/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines fpga phy Interface.
 */

#ifndef _PHY_REGS_FPGA_H_
  #define _PHY_REGS_FPGA_H_


  void set_cmd_delays (int cmd_idelay, int cmd_odelay);
  void sd_set_output_delays(uint32_t dq_out_delay, uint32_t dq_in_delay);
  void sd_set_phase_shift(uint32_t phase_shift, uint32_t phase_shift_cmd);
  void sd_set_dat_phase_shift(uint32_t phase_shift);
  void sd_set_data_select_oe_end(int _value);
  void sd_set_ctrl_odelay(int _value);
  void sd_set_hcsdclkadj(int _value);
  void set_dqs_delays(int dqs_idelay, int dqs_odelay);
  void set_dll_slave_ctrl (int read_dqs_delay);
  void set_extlpbk_dqs_delays (int ext_lpbk_dqs_idelay);
  void set_deskew_cmd_delays (int deskw_cmd_delay);

  //#ifndef REG_BASE
    #define REG_BASE 0x00000000
  //#endif


  #define R_PHY_TUNING_REG         (REG_BASE + 0x2800)
    #define F_CMD_DQS_SHIFT             24
    #define F_WR_DQS_ODELAY             16
    #define F_DQ_ODELAY                 8
    #define F_DATA_DQS_SHIFT            0

  #define R_PHY_DQS_IDELAY_REG     (REG_BASE + 0x2804)
    #define F_RD_DQS_IDELAY             16
    #define F_RD_EXT_LPBK_DQS_IDELAY    8
    #define F_RD_INT_LPBK_DQS_IDELAY    0

  #define R_PHY_IODELAY_REG        (REG_BASE + 0x2808)
    #define F_DQ_IDELAY                 24
    #define F_CMD_IDELAY                16
    #define F_CMD_ODELAY                8
    #define F_CTRL_ODELAY               0

  #define R_PHY_DQS_SHIFT_STAT_REG (REG_BASE + 0x280c)
    #define F_DATA_DQS_MMCM_LOCKED      31
    #define F_CMD_DQS_MMCM_LOCKED       30
    #define F_DATA_DQS_SHIFT_STAT       8
    #define F_CMD_DQS_SHIFT_STAT        0

  #define R_PHY_DQ_IDELAY_0_STAT_REG (REG_BASE + 0x2810)
    #define F_RD_DQ3_DELAY_STAT         24
    #define F_RD_DQ2_DELAY_STAT         16
    #define F_RD_DQ1_DELAY_STAT         8
    #define F_RD_DQ0_DELAY_STAT         0

  #define R_PHY_DQ_IDELAY_1_STAT_REG (REG_BASE + 0x2814)
    #define F_RD_DQ7_DELAY_STAT         24
    #define F_RD_DQ6_DELAY_STAT         16
    #define F_RD_DQ5_DELAY_STAT         8
    #define F_RD_DQ4_DELAY_STAT         0

  #define R_PHY_DQ_ODELAY_0_STAT_REG (REG_BASE + 0x2818)
    #define F_WR_DQ3_DELAY_STAT         24
    #define F_WR_DQ2_DELAY_STAT         16
    #define F_WR_DQ1_DELAY_STAT         8
    #define F_WR_DQ0_DELAY_STAT         0

  #define R_PHY_DQ_ODELAY_1_STAT_REG (REG_BASE + 0x281c)
    #define F_WR_DQ7_DELAY_STAT         24
    #define F_WR_DQ6_DELAY_STAT         16
    #define F_WR_DQ5_DELAY_STAT         8
    #define F_WR_DQ4_DELAY_STAT         0

  #define R_PHY_DQS_IDELAY_STAT_REG (REG_BASE + 0x2820)
    #define F_RD_DQS_IDELAY_STAT        16
    #define F_RD_EXT_LPBK_DQS_IDELAY_STAT 8
    #define F_RD_INT_LPBK_DQS_IDELAY_STAT 0

  #define R_PHY_ODELAY_STAT_REG    (REG_BASE + 0x2824)
    #define F_WR_DQS_ODELAY_STAT        8
    #define F_CTRL_ODELAY_STAT          0

  #define R_PHY_CMD_IODELAY_STAT_REG (REG_BASE + 0x2828)
    #define F_CMD_ODELAY_STAT           8
    #define F_CMD_IDELAY_STAT           0

#endif /* _PHY_REGS_FPGA_H_ */
