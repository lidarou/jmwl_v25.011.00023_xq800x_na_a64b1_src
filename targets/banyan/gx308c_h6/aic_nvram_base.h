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
#define NVRAM_CALIBRATION_SIZE      (2 * NVRAM_BLOCK_DATA_SIZE)
#define NVRAM_NORMAL_SIZE           (3 * NVRAM_BLOCK_DATA_SIZE)

/**
 * flash partition name config
 */
#define NVRAM_DOWNLOAD_DEVICE_NAME          "nv"
#define NVRAM_NORMAL_DEVICE_NAME            "nvnorm"
#define NVRAM_CALIBRATION_DEVICE_NAME       "nvcali"
#define NVRAM_DIFF_DEVICE_NAME              "nvdiff"

#ifdef CONFIG_SUBSYS_CP
#include <aic_nvram_id.h>

/**
 * whitelist nvram id: if defined and this id is calibration item,
 * this id will be support write to calibration partition in normal mode.
 */
u16 nvram_whitelist_id[] = {
    NVRAM_ID_IMEI0,
    NVRAM_ID_IMEI1,
};
#endif

#endif /* __AIC_NVRAM_BASE_H__ */

