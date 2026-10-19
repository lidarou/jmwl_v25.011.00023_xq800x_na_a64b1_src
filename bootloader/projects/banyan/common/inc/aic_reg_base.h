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

/* core ppb reg base address */
#define REG_BASE_SCB                    (0xE0000000)

/* rccalibre reg base */
#define REG_BASE_RCCALIBRE_PD           REG_BASE_TOP_RCCALIBRE_PD

#endif /* __AIC_REG_BASE_H__ */
