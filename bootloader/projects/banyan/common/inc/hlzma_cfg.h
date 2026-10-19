/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __HLZMA_CFG_H__
#define __HLZMA_CFG_H__

#include <ckm_en_glb_mcu.h>

#define REG_LZMA_EB    REG_CKM_EN_GLB_MCU_MCU_LZMA_EB
#define BIT_LZMA_EB    BIT_CKM_EN_GLB_MCU_MCU_LZMA_EB

#define REG_LZMA_RESET                      REG_CKM_EN_GLB_MCU_MCU_LZMA_SW_RST_GLB_CFG
#define LZMA_REG_BASE                       REG_BASE_BLK_MCU_LZMA

/* define for 31x, undefine for 30x */
#define xLZMA_CHANGE_CLK

#endif /* __HLZMA_CFG_H__ */
