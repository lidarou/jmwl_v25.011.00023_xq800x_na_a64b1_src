/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the fpga combo phy function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x113a58
#define LOG_MODULE    AMOD_SDMMC

#include "sdhost_global.h"
#include "phy_regs_fpga.h"
#include "dfi.h"

void sd_set_output_delays(uint32_t dq_out_delay, uint32_t dq_in_delay)
{
    //uint32_t _status = 0;
    uint32_t _tmp;

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete

    AIC_WRITE32(SD_REG_HRS4,R_PHY_TUNING_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_TUNING_REG);
    _tmp |= (dq_out_delay << F_DQ_ODELAY);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    AIC_WRITE32(SD_REG_HRS4,R_PHY_IODELAY_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_IODELAY_REG);
    _tmp |= (dq_in_delay  << F_DQ_IDELAY);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void sd_set_phase_shift(uint32_t phase_shift, uint32_t phase_shift_cmd)
{
    //uint32_t _status = 0;
    uint32_t _tmp;

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete

    AIC_WRITE32(SD_REG_HRS4,R_PHY_TUNING_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_TUNING_REG);
//    _tmp |= (dqs_delay << F_DQS_OUT_DELAY);
    _tmp &= ~(0xff  << F_DATA_DQS_SHIFT);
    _tmp &= ~(0xff  << F_CMD_DQS_SHIFT);
    _tmp |= (phase_shift_cmd*8 << F_CMD_DQS_SHIFT);
    _tmp |= (phase_shift*8  << F_DATA_DQS_SHIFT);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void sd_set_dat_phase_shift(uint32_t phase_shift)
{
    //uint32_t _status = 0;
    uint32_t _tmp;

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete


    AIC_WRITE32(SD_REG_HRS4,R_PHY_TUNING_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_TUNING_REG);

    _tmp &= ~(0xff  << F_DATA_DQS_SHIFT);
    _tmp |= (phase_shift*8  << F_DATA_DQS_SHIFT);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void sd_set_data_select_oe_end(int _value)
{
    int32_t _tmp;

    AIC_WRITE32(SD_REG_HRS4,PHY_DQ_TIMING_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,PHY_DQ_TIMING_REG);
    _tmp |= (_value << DATA_SELECT_OE_END);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void sd_set_ctrl_odelay(int _value)
{
    int32_t _tmp;

    AIC_WRITE32(SD_REG_HRS4,R_PHY_IODELAY_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_IODELAY_REG);
    _tmp |= (_value << F_CTRL_ODELAY);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void sd_set_hcsdclkadj(int _value)
{
    int32_t _tmp;

    _tmp=AIC_READ32(SD_REG_HRS10);
    _tmp&=0xFFF0FFFF;
    AIC_WRITE32(SD_REG_HRS10, _tmp | (_value << 16));
}

void set_cmd_delays (int cmd_idelay, int cmd_odelay)
{
    int32_t _tmp;

    AIC_WRITE32(SD_REG_HRS4,R_PHY_IODELAY_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_IODELAY_REG);
    _tmp |= (cmd_idelay  << F_CMD_IDELAY);
    _tmp |= (cmd_odelay  << F_CMD_ODELAY);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void set_deskew_cmd_delays (int deskw_cmd_delay)
{
    int32_t _tmp;

    AIC_WRITE32(SD_REG_HRS4,0x2030);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,0x2030);
    _tmp &= ~(0xF<<24);
    _tmp |= (deskw_cmd_delay  << 24);
//    _tmp |= (cmd_odelay  << F_CMD_ODELAY);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void set_extlpbk_dqs_delays (int ext_lpbk_dqs_idelay)
{
    int32_t _tmp;

    AIC_WRITE32(SD_REG_HRS4,R_PHY_DQS_IDELAY_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_DQS_IDELAY_REG);
    _tmp &= ~(0xFF<<8);
    _tmp |= (ext_lpbk_dqs_idelay  << F_RD_EXT_LPBK_DQS_IDELAY);
    AIC_WRITE32(SD_REG_HRS5, _tmp);
    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void set_dll_slave_ctrl (int read_dqs_delay)
{
    int32_t _tmp;

    AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,PHY_DLL_SLAVE_CTRL_REG);
    _tmp &= ~(0xFF<<24);
    _tmp |= (read_dqs_delay  << 24);
    AIC_WRITE32(SD_REG_HRS5, _tmp);
    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}

void set_dqs_delays(int dqs_idelay, int dqs_odelay)
{
    int32_t _tmp;

    AIC_WRITE32(SD_REG_HRS4,R_PHY_TUNING_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_TUNING_REG);
    _tmp |= (dqs_odelay  << F_WR_DQS_ODELAY);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    AIC_WRITE32(SD_REG_HRS4,R_PHY_DQS_IDELAY_REG);
    _tmp = AIC_READ32(SD_REG_HRS5);
    AIC_WRITE32(SD_REG_HRS4,R_PHY_DQS_IDELAY_REG);
    _tmp |= (dqs_idelay  << F_RD_DQS_IDELAY);
    _tmp |= (dqs_idelay  << F_RD_INT_LPBK_DQS_IDELAY);
    _tmp |= (dqs_idelay  << F_RD_EXT_LPBK_DQS_IDELAY);
    AIC_WRITE32(SD_REG_HRS5, _tmp);

    //check phy_init_complete
    while(~AIC_READ32(SD_REG_HRS9) & (1 << 1)); // phy_init_complete
}
