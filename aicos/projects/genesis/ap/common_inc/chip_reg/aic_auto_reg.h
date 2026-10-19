/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip ap auto registers files.
 */


#ifndef __AIC_AUTO_REG_H__
#define __AIC_AUTO_REG_H__

#include <aon_glb.h>
#include <ckm_en_mcu.h>
#include <ckm_en_top.h>
#include <cpu_glb.h>
#include <cpu_main_mtx_lpc.h>
#include <cru_mcu.h>
#include <cru_top.h>
#include <mcu_glb.h>
#include <mcu_main_mtx_lpc.h>
#include <mem_ctrl.h>
#include <top_glb.h>
#include <top_main_mtx.h>
#ifndef CONFIG_SDK_ENABLE
#include <audio_glb.h>
#include <audio_main_mtx_lpc.h>
#include <ckm_en_audio.h>
#include <cru_audio.h>
#include <pmu_ctrl.h>
#endif
#define REG_ADSP_POWER_CTRL     (REG_BASE_AON_PMU + 0x210)
#define REG_ADSP_PD_STATE       (REG_BASE_AON_PMU + 0x214)
#define BIT_ADSP_FRC_SHUTDOWN   BIT(1)

#define SPINLOCK_EN_REG              REG_TOP_GLB_SPINLOCK_EB
#define LCDC_SW_RST_REG              REG_MCU_GLB_SW_RST_GLB_CLK_MCU_LCDC
#endif /* __AIC_AUTO_REG_H__ */
