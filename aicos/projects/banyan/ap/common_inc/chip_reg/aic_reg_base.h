/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip reg base files.
 */


#ifndef __AIC_REG_BASE_H__
#define __AIC_REG_BASE_H__

#include <reg_base.h>

#define REG_BASE_PMIC_START             0x0
#define REG_BASE_PMIC_END               0xFFFF
#define REG_BASE_TOP_MBOX               REG_BASE_BLK_MEM_CTRL_MBOX

#define REG_BASE_AP_ROMCODE             (0x0)
#define REG_BASE_MEM_NORFLASH           (0x10000000)
#define REG_BASE_MEM_SRAM               (0x20000000)
#define REG_BASE_IOMUX                  REG_BASE_IO_PD_IOMUX
#define REG_BASE_PG1_PAD_CTRL           REG_BASE_PD_PG1_PAD_CTRL
#define REG_BASE_PG2_PAD_CTRL           REG_BASE_PD_PG2_PAD_CTRL
#define REG_BASE_PG3_PAD_CTRL           REG_BASE_PD_PG3_PAD_CTRL
#define REG_BASE_PG4_PAD_CTRL           REG_BASE_PD_PG4_PAD_CTRL
#define REG_BASE_PG5_PAD_CTRL           REG_BASE_PD_PG5_PAD_CTRL
#define REG_BASE_PG7_PAD_CTRL           REG_BASE_PD_PG7_PAD_CTRL
#define REG_BASE_PG8_PAD_CTRL           REG_BASE_PD_PG8_PAD_CTRL
#define REG_BASE_PG9_PAD_CTRL           REG_BASE_PD_PG9_PAD_CTRL
#define REG_BASE_PG10_PAD_CTRL          REG_BASE_PD_PG10_PAD_CTRL
#define REG_BASE_PG11_PAD_CTRL          REG_BASE_PD_PG11_PAD_CTRL
#define REG_BASE_MEM_PSRAM              (0x80000000)
#define REG_BASE_DEVICE                 (0x30000000)
#define REG_BASE_MEM_SRAM_TEST          REG_BASE_MEM_SRAM

/* core ppb reg base address */
#define REG_BASE_SCB                    (0xE0000000)

/*aon sub sys wdt base*/
#define AON_WDT_EB                      REG_CKM_EN_GLB_TOP_TOP_WDT_MCU_EB
#define AON1_WDT_EB                     REG_CKM_EN_GLB_TOP_TOP_WDT_CP_EB
#define WDT_REB_BASE                    REG_BASE_TOP_WDT_MCU
#define WDT1_REB_BASE                   REG_BASE_TOP_WDT_CP
#define REG_GLB_SW_RST_WDT              REG_CKM_EN_GLB_TOP_TOP_WDT_MCU_SW_RST_GLB_CFG
#define BIT_GLB_SW_RST_GLB_CLK_AON_WDT  BIT_CKM_EN_GLB_TOP_SW_RST_GLB_TOP_WDT_MCU
#define CP_RST_REQ                      IRQ_TOP_WDT_CP_RST_REQ
#define WDT_RST_REQ                     IRQ_TOP_WDT_MCU

/* lzma redefine */
#define REG_BASE_LZMA                   (0x52100000)
#define REG_LZMA_EB                     (0x4200000C)
#define REG_LZMA_RESET                  (void *)(0x42000088)
/* int definition */
#define BIT_DEC_ERR                     BIT(5)
#define BIT_DEC_DONE                    BIT(5)

/* csi regbase redefine */
#define REG_BASE_AP_SPI_CSI             REG_BASE_BLK_MCU_CSI

/* chipid definition */
#define REG_CHIP_ID                     REG_AON_GLB_CHIP_ID
#define REG_VERSION_ID                  0x400D0FFC
#define CHIP_VERSION_AA                 0xA0000000
#define CHIP_VERSION_AB                 0xA0000001
#define CHIP_VERSION_AC                 0xA0000002
#define CHIP_VERSION_AD                 0xA0000003
#define VAL_CHIP_ID_AB                  0xA306000A

/* efuse definition*/
#define REG_TOP_GLB_EFUSE_EB            REG_CKM_EN_GLB_TOP_TOP_EFUSE_EB
#define BIT_TOP_GLB_EFUSE_EB            BIT_CKM_EN_GLB_TOP_TOP_EFUSE_EB

/* bb rccalibrate definition */
#define REG_BASE_RCCALIBRE_PD           REG_BASE_TOP_RCCALIBRE_PD

#endif /* __AIC_REG_BASE_H__ */
