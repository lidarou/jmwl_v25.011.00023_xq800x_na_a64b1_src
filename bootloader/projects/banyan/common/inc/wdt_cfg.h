/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __WDT_CFG_H__
#define __WDT_CFG_H__
#include <top_glb.h>
#include <ckm_en_glb_top.h>

#define REG_BASE_AON_WDT0 					REG_BASE_TOP_WDT_MCU
#define REG_AON_GLB_WDT_INT_REQ_ENABLE 		REG_TOP_GLB_WDT_INT_REQ_ENABLE
#define REG_AON_GLB_WDT0_EB					REG_CKM_EN_GLB_TOP_TOP_WDT_MCU_EB
#define BIT_AON_GLB_WDT0_EB					BIT(0)
#define REG_TOP_GLB_SW_RST_WDT0				REG_CKM_EN_GLB_TOP_TOP_WDT_MCU_SW_RST_GLB_CFG
#define BIT_TOP_GLB_SW_RST_GLB_CLK_AON_WDT0	BIT_CKM_EN_GLB_TOP_SW_RST_GLB_TOP_WDT_MCU
#endif /* __WDT_CFG_H__ */
