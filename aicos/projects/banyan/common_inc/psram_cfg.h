/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#ifndef __PSRAM_CFG_H__
#define __PSRAM_CFG_H__

#include <aic_reg_base.h>
#include <cru_top.h>
#include <aon_pmu.h>

#define PSRAM_BASE_CTRL                 (REG_BASE_BLK_MEM_SW_PSRAMC)
#define PSRAM_PHY_CTRL                  (REG_BASE_BLK_MEM_SW_PSRAM_PHY)
#define PSRAM_TIMER_BASE                (REG_BASE_AON_SYSTMR)
#define PSRAM_TIMER_EB_BASE             (REG_CKM_EN_GLB_AON_AON_SYSTMR_CNT_EB)
#define PSRAM_TIMER_EB_BIT              (BIT_CKM_EN_GLB_AON_AON_SYSTMR_CNT_EB)
#define PSRAM_CLK_BASE                  (REG_CRU_TOP_CLK_PSRAMC_2X_CFG)
#define PSRAM_CLK_SEL_SHIFT             (0)
#define PSRAM_CLK_SEL_MASK              (0x3)
#define PSRAM_CLK_DIV_SHIFT             (2)
#define PSRAM_CLK_DIV_MASK              (0xF)
#define PSRAM_MEM_ACC_RDY               (REG_AON_PMU_EXT_MEM_ACC_RDY)
#define PSRAM_ACC_RDY_SHIFT             (0)
#define PSRAM_ACC_RDY_MASK              (0x1)
#define PSRAM_IO_RET                    (REG_AON_PMU_AON_IO_RET)
#define PSRAM_IO_RET_SET                (REG_AON_PMU_AON_IO_RET_SET)
#define PSRAM_IO_RET_CLR                (REG_AON_PMU_AON_IO_RET_CLR)
#define PSRAM_LATCH_CLR                 (0x1 << 3)
#define PSRAM_MODE_SEL_SHIFT            (0)
#define PSRAM_MODE_SEL_MASK             (0x3)

#define PSRAM_2X_CLK_26M                (26000000)
#define PSRAM_2X_CLK_DPLL0_307M2        (307200000)
#define PSRAM_2X_CLK_DPLL0_384M         (384000000)
#define PSRAM_2X_CLK_DPLL1_390M         (390000000)
#define PSRAM_2X_CLK_DPLL1_475M136      (475136000)

#define PSRAM_CLK_13M                   (13000000)
#define PSRAM_CLK_38M4                  (38400000)
#define PSRAM_CLK_48M                   (48000000)
#define PSRAM_CLK_48M75                 (48750000)
#define PSRAM_CLK_59M392                (59392000)
#define PSRAM_CLK_76M8                  (76800000)
#define PSRAM_CLK_96M                   (96000000)
#define PSRAM_CLK_97M5                  (97500000)
#define PSRAM_CLK_118M784               (118784000)
#define PSRAM_CLK_153M6                 (153600000)
#define PSRAM_CLK_192M                  (192000000)
#define PSRAM_CLK_195M                  (195000000)
#define PSRAM_CLK_237M568               (237568000)

#define MTB_CODE_START                  (REG_BASE_BLK_MCU_MTB + 0x600)

#endif /* __PSRAM_CFG_H__ */
