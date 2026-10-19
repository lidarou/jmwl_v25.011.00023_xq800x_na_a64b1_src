/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines chip mpu map parameters.
 */

#ifndef __MPU_MAP_H__
#define __MPU_MAP_H__

#include <flash_config.h>
#include <flash_mem_config.h>

#if (QSPI_NOR_DEVICE_NUM == 1)
/* !The maxium region number is 16, exceed regions will be ignored.
 * If the background region not enabled, say PRIVDEFENA set to 0
 * then any access doesn't any region in mpu_cfg should generate
 * fault
 */
const MPU_REGION_T mpu_cfg[] = {
    /* flash */
    {MPU_RG(FLASH_BASE, FLASH0_TOTAL_SIZE), RDWR, MEM_NC, EXEC, RG_ENABLE},
    {MPU_RG(FLASH0_BASE, FLASH0_TOTAL_SIZE), RDONLY, MEM_WTNWA, EXEC, RG_ENABLE},
    /* zbt ram 1 cycle latency */
    {0x20000000, 0x20047FFF, RDWR,   MEM_WBWA,   EXEC,       RG_ENABLE},
    /* psram */
    {0x80000000, 0x807FFFFF, RDWR,   MEM_WBWA,   EXEC,       RG_ENABLE},
    /* SCB */
    {0xE0000000, 0xFFFFFFFF, RDWR,   DEV_NGNRNE, NO_EXEC,    RG_ENABLE},
    {0x50000000, 0x7FFFFFFF, RDWR,   DEV_NGNRNE,  NO_EXEC,   RG_ENABLE},
    {0x40000000, 0x4FFFFFFF, RDWR,   DEV_NGNRNE,  NO_EXEC,   RG_ENABLE},
};
#else
#include <mpu_map_multi_cs.h>
#endif

#endif /* __MPU_MAP_H__ */
