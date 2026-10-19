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
#include <aon_pmu.h>
#include <cru_mcu.h>
#include <cru_top.h>
#include <ckm_en_glb_aon.h>
#include <ckm_en_glb_mcu.h>
#include <ckm_en_glb_top.h>
#include <mcu_glb.h>
#include <mem_ctrl.h>
#include <top_glb.h>
#include <sys_nd_glb.h>

#define REG_ADSP_POWER_CTRL         (REG_BASE_AON_PMU + 0x210)
#define REG_ADSP_PD_STATE           (REG_BASE_AON_PMU + 0x214)
#define BIT_ADSP_FRC_SHUTDOWN       BIT(1)

#define REG_TOP_GLB_MBOX_EB         (REG_BASE_CKM_EN_GLB_TOP + 0xc)

#define SPINLOCK_EN_REG             REG_CKM_EN_GLB_TOP_TOP_SPINLOCK_EB

/*ap lcdc redefine*/
#define REG_BASE_AP_SPI_LCD             REG_BASE_BLK_MCU_LCDC
#define REG_MCU_GLB_LCDC_EB             REG_CKM_EN_GLB_MCU_MCU_LCDC_EB
#define BIT_MCU_GLB_LCDC_EB             BIT_CKM_EN_GLB_MCU_MCU_LCDC_EB
#define LCD_POWER_CONFIG_REG            REG_CRU_MCU_CLK_EN_FORCE_CLK_MCU_LCDC_CFG
#define LCD_POWER_CONFIG_VAL            0x2
#define LCDC_SW_RST_REG                 REG_CKM_EN_GLB_MCU_MCU_LCDC_SW_RST_GLB_CFG

/*ap csi redefine */
#define REG_MCU_GLB_CSI_EB_SET                  REG_CKM_EN_GLB_MCU_MCU_CSI_EB
#define BIT_MCU_GLB_CSI_EB                      BIT_CKM_EN_GLB_MCU_MCU_CSI_EB
#define REG_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI_SET  REG_CKM_EN_GLB_MCU_MCU_CSI_SW_RST_GLB_CFG
#define BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI      BIT_CKM_EN_GLB_MCU_SW_RST_GLB_MCU_CSI

/* mtb buffer redefine */
#define BIT_MTB_BUF_EB                          REG_CKM_EN_GLB_MCU_MCU_MTB_EB

/*bb msrc related redefine*/
#define REG_BB_RC32K_SEL                REG_SYS_ND_GLB_PP_CLK_SEL_CFG

#endif /* __AIC_AUTO_REG_H__ */
