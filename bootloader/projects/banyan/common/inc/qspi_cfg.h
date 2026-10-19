/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#ifndef __QSPI_CFG_H__
#define __QSPI_CFG_H__

#include <reg_base.h>
#include <cru_top.h>

#define FLASH_MEM_BASE          (REG_BASE_BLK_MEM_CTRL_FLASH)
#define QSPI_IO_BASE            (REG_BASE_BLK_MEM_CTRL_NORFLASH)
#define QSPI_CLK_BASE           (REG_CRU_TOP_CLK_REF_NORFLASH_CFG)
#define QSPI_CLK_SEL_SHIFT      (0)
#define QSPI_CLK_SEL_MASK       (0x3)
#define QSPI_CLK_DIV_SHIFT      (2)
#define QSPI_CLK_DIV_MASK       (0xF)

#define QSPI_CLK_26M            (26000000)
#define QSPI_CLK_DPLL0_192M     (192000000)
#define QSPI_CLK_DPLL0_256M     (256000000)

/* flash work max frequency */
#define FLASH_CLK_13M           (13000000)
#define FLASH_CLK_16M           (16000000)
#define FLASH_CLK_24M           (24000000)
#define FLASH_CLK_32M           (32000000)
#define FLASH_CLK_48M           (48000000)
#define FLASH_CLK_64M           (64000000)
#define FLASH_CLK_96M           (96000000)
#define FLASH_CLK_128M          (128000000)

#endif /* __QSPI_CFG_H__ */
