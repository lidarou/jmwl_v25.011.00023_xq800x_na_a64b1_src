/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __HLZMA_CFG_H__
#define __HLZMA_CFG_H__

#define REG_LZMA_EB    REG_MCU_GLB_LZMA_MCU_EB
#define BIT_LZMA_EB    BIT_MCU_GLB_LZMA_MCU_EB

#define REG_LZMA_RESET                      0x42080004
#define LZMA_REG_BASE                       REG_BASE_LZMA

/* define for 31x, undefine for 30x */
#define LZMA_CHANGE_CLK

#endif /* __HLZMA_CFG_H__ */
