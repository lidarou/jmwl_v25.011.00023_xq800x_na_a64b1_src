
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-09-24 10:55:31
 */

#ifndef __PMU_MACRO_FPGA_H__
#define __PMU_MACRO_FPGA_H__

#define REG_PMU_MACRO_PMU_CHARGER                                               (REG_BASE_PMU_MACRO + 0x0)
#define REG_PMU_MACRO_PMU_CTRL0                                                 (REG_BASE_PMU_MACRO + 0x4)
#define REG_PMU_MACRO_PMU_CTRL1                                                 (REG_BASE_PMU_MACRO + 0x8)
#define REG_PMU_MACRO_PMU_CTRL2                                                 (REG_BASE_PMU_MACRO + 0xc)

/*REG_PMU_MACRO_PMU_CHARGER*/
#define BIT_PMU_MACRO_CHIP_ID_MAIN_VERSION(x)                                   (((x) & 0xffffff) << 8)
#define BIT_PMU_MACRO_DCD_DET_18_RF                                             BIT(7)
#define BIT_PMU_MACRO_PD_DET_18_RF                                              BIT(6)
#define BIT_PMU_MACRO_SD_DET_18_RF                                              BIT(5)
#define BIT_PMU_MACRO_PS2_DET_18_RF                                             BIT(4)
#define BIT_PMU_MACRO_ERROR_VBAT_18_RF                                          BIT(3)
#define BIT_PMU_MACRO_CHRG_OVER_18_RF                                           BIT(2)
#define BIT_PMU_MACRO_IDET_18_RF                                                BIT(1)
#define BIT_PMU_MACRO_CHG_INT_18R_RF                                            BIT(0)

/*REG_PMU_MACRO_PMU_CTRL0*/
#define BIT_PMU_MACRO_BUCK2_OC_18_TRIG                                          BIT(6)
#define BIT_PMU_MACRO_BUCK1_OC_18_TRIG                                          BIT(5)
#define BIT_PMU_MACRO_BUCK0_OC_18_TRIG                                          BIT(4)
#define BIT_PMU_MACRO_OT_DET2_18_TRIG                                           BIT(3)
#define BIT_PMU_MACRO_OT_DET1_18_RF                                             BIT(2)
#define BIT_PMU_MACRO_OVLO_VBAT_18_TRIG                                         BIT(1)
#define BIT_PMU_MACRO_UVLO_VBAT_18R_TRIG                                        BIT(0)

/*REG_PMU_MACRO_PMU_CTRL1*/
#define BIT_PMU_MACRO_PMU_FSM7(x)                                               (((x) & 0xf) << 28)
#define BIT_PMU_MACRO_PMU_FSM6(x)                                               (((x) & 0xf) << 24)
#define BIT_PMU_MACRO_PMU_FSM5(x)                                               (((x) & 0xf) << 20)
#define BIT_PMU_MACRO_PMU_FSM4(x)                                               (((x) & 0xf) << 16)
#define BIT_PMU_MACRO_PMU_FSM3(x)                                               (((x) & 0xf) << 12)
#define BIT_PMU_MACRO_PMU_FSM2(x)                                               (((x) & 0xf) << 8)
#define BIT_PMU_MACRO_PMU_FSM1(x)                                               (((x) & 0xf) << 4)
#define BIT_PMU_MACRO_PMU_FSM0(x)                                               (((x) & 0xf))

/*REG_PMU_MACRO_PMU_CTRL2*/
#define BIT_PMU_MACRO_PMU_FSM_CLEAN                                             BIT(0)

#endif /*__PMU_MACRO_FPGA_H__*/