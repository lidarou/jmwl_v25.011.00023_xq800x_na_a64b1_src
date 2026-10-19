/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#ifndef __PSRAM_CFG_H__
#define __PSRAM_CFG_H__

#include <reg_base.h>
#include <cru_top.h>
#include <pmu_ctrl.h>

#define PSRAM_BASE_CTRL             (REG_BASE_TOP_PSRAM_RF)
#define PSRAM_PHY_CTRL              (REG_BASE_TOP_PSRAM_RF + 0x4000)
#define PSRAM_TIMER_BASE            (REG_BASE_AON_SYSTMR)
#define PSRAM_TIMER_EB_BASE         (REG_AON_GLB_SYSTMR_EB)
#define PSRAM_TIMER_EB_BIT          (BIT_TOP_GLB_SW_RST_GLB_CLK_AON_SYSTMR)
#define PSRAM_CLK_BASE              (REG_CRU_TOP_CLK_PSRAMC_2X_CFG)
#define PSRAM_CLK_SEL_SHIFT         (0)
#define PSRAM_CLK_SEL_MASK          (0x3)
#define PSRAM_CLK_DIV_SHIFT         (2)
#define PSRAM_CLK_DIV_MASK          (0xF)
#define PSRAM_MEM_ACC_RDY           (REG_PMU_CTRL_PSRAM_ACC_RDY)
#define PSRAM_ACC_RDY_SHIFT         (0)
#define PSRAM_ACC_RDY_MASK          (0x1)

#define PSRAM_2X_CLK_26M            (26000000)
#define PSRAM_2X_CLK_333M           (333000000)
#define PSRAM_2X_CLK_396M           (396000000)
#define PSRAM_2X_CLK_500M           (500000000)

#define PSRAM_CLK_13M               (13000000)
#define PSRAM_CLK_62M               (62500000)
#define PSRAM_CLK_125M              (125000000)
#define PSRAM_CLK_166M              (166500000)
#define PSRAM_CLK_198M              (198000000)
#define PSRAM_CLK_250M              (250000000)

#define MTB_CODE_START              (REG_BASE_AP_STAR_MTB + 0x600)

#endif /* __PSRAM_CFG_H__ */
