/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#ifndef __QSPI_CFG_H__
#define __QSPI_CFG_H__

#include <reg_base.h>
#include <cru_top.h>

#define FLASH_MEM_BASE          (REG_BASE_MEM_NORFLASH)
#define QSPI_IO_BASE            (REG_BASE_TOP_QSPI_NORFLASH_RF)
#define QSPI_CLK_BASE           (REG_CRU_TOP_CLK_REF_NORFLASH_CFG)
#define QSPI_CLK_SEL_SHIFT      (0)
#define QSPI_CLK_SEL_MASK       (0x3)
#define QSPI_CLK_DIV_SHIFT      (2)
#define QSPI_CLK_DIV_MASK       (0xF)

#define QSPI_CLK_26M            (26000000)
#define QSPI_CLK_200M           (200000000)
#define QSPI_CLK_250M           (250000000)
#define QSPI_CLK_264M           (264000000)

/* flash work max frequency */
#define FLASH_CLK_25M           (25000000)
#define FLASH_CLK_31M25         (31250000)
#define FLASH_CLK_33M           (33000000)
#define FLASH_CLK_50M           (50000000)
#define FLASH_CLK_62M5          (62500000)
#define FLASH_CLK_66M           (66000000)
#define FLASH_CLK_100M          (100000000)
#define FLASH_CLK_125M          (125000000)
#define FLASH_CLK_132M          (132000000)

#endif /* __QSPI_CFG_H__ */
