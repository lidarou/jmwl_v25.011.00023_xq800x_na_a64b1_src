/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes aic nvram base address map.
 */

#ifndef __AIC_NVRAM_BASE_H__
#define __AIC_NVRAM_BASE_H__

#if 0 /* user define */
#define NVRAM_DOWNLOAD_BIN_PATH "../nvram/"
#define NVRAM_DOWNLOAD_BIN_NAME "aic_nvram_data.bin"
#define NVRAM_OUTPUT_BIN_PATH   "../../../../out/win32/nvram/"
#endif

/**
 * sector size of nvram device
 */
#define NVRAM_DEVICE_SECTOR_SIZE    0x200

/**
 * block config
 */
#define NVRAM_BLOCK_SIZE            0x2000
#define NVRAM_BLOCK_CHK_SIZE        NVRAM_DEVICE_SECTOR_SIZE
#define NVRAM_BLOCK_DATA_SIZE       (NVRAM_BLOCK_SIZE - NVRAM_BLOCK_CHK_SIZE)

/**
 * size config
 */
#define NVRAM_DOWNLOAD_SIZE         0x10000
#define NVRAM_CALIBRATION_SIZE      (5 * NVRAM_BLOCK_DATA_SIZE)
#define NVRAM_PROTECT_SIZE          0x0
#define NVRAM_NORMAL_SIZE           (5 * NVRAM_BLOCK_DATA_SIZE)

/**
 * flash sector size config
 */
#define NVRAM_FLASH_SECTOR_SIZE     0x1000

#include <aic_nvram_id.h>

/**
 * whitelist nvram id: if defined and this id is calibration item,
 * this id will be support write to calibration partition in normal mode.
 */
u16 nvram_whitelist_id[] = {
    NVRAM_ID_IMEI0,
    NVRAM_ID_IMEI1,
};

#endif /* __AIC_NVRAM_BASE_H__ */

