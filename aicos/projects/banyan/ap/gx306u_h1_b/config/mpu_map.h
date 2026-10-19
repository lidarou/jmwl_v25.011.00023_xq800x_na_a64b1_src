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

#include <sys_mem_config.h>
#include <mpu.h>

/*
 * The total number of region is 16(0, 15), and the index of the array
 * element i is corresponds the region i, exceed regions will be ignored.
 * If the background region not enabled, say PRIVDEFENA set to 0
 * then any access doesn't any region in mpu_cfg should generate fault.
 * On ARM-v8 not support region overlay and subregion.
 */
const MPU_REGION_T mpu_cfg[] = {
    /* flash */
    /* NOR flash indac/dac write */
    {MPU_RG(FLASH0_INDAC_RW_BASE,    FLASH0_INDAC_RW_SIZE),
     RDWR,   MEM_NC,     NO_EXEC,    RG_ENABLE},

    /* NOR flash dac read and exe, [FLASH0_BASE, FLASH0_AP_EXTEND_BASE)*/
    {FLASH0_BASE,        (FLASH0_AP_EXTEND_BASE - 1),
     RDONLY, MEM_WTNWA,  EXEC,       RG_ENABLE},

    /* NOR flash dac read and exe, [FLASH0_FOTA_BAK_BASE, FLASH0_FOTA_BAK_END)*/
    {MPU_RG(FLASH0_FOTA_BAK_BASE,  FLASH0_FOTA_BAK_SIZE),
     RDONLY, MEM_WTNWA,  EXEC,       RG_ENABLE},

    /* NOR flash dac read and no-exe, [FLASH0_FOTA_BAK_END, FLASH0_END]*/
    {FLASH0_FOTA_BAK_BASE + FLASH0_FOTA_BAK_SIZE, (FLASH0_BASE + FLASH0_TOTAL_SIZE - 1),
     RDONLY, MEM_WTNWA,  EXEC,       RG_ENABLE},

    /* IRAM */
    /* CP vector, share meory, no exe, rw, nocache. */
    {MPU_RG(CP_VECTOR_REGION_BASE, CP_VECTOR_REGION_SIZE + AON_SHMEM_REGION_SIZE),
     RDWR,   MEM_NC,   NO_EXEC,    RG_ENABLE},

    /* sysstack, sleep, phy buffer. exe, rw, cache */
    {AP_SYSSTACK_REGION_BASE, (AONRAM_REGION_BASE + AONRAM_REGION_SIZE -1),
     RDWR,   MEM_WTNWA,  EXEC,    RG_ENABLE},

    /* PSRAM: AP need dump all psram, and need load subsys code.*/
    /* code */
    {MPU_RG(CODE_REGION_BASE, CODE_REGION_SIZE),
     RDONLY, MEM_WBWA,   EXEC,       RG_ENABLE},

    /* HEAP + rw + bss .*/
    {HEAP_REGION_BASE, AP_REGION_BASE + AP_REGION_SIZE - 1,
     RDWR,   MEM_WBWA,   NO_EXEC,    RG_ENABLE},

    /* share memory .*/
    {MPU_RG(SHMEM_REGION_BASE, SHMEM_REGION_SIZE),
     RDWR,   MEM_WBWA,   NO_EXEC,    RG_ENABLE},

    /* device */
    /* sys, top and aon register. */
    {0x40000000, 0x540005FF, RDWR,   DEV_NGNRNE, NO_EXEC,    RG_ENABLE},

    {0x54000600, 0x540007FF, RDWR,   MEM_WBWA,   EXEC,       RG_ENABLE},

    {0x54000800, 0x54FFFFFF, RDWR,   DEV_NGNRNE, NO_EXEC,    RG_ENABLE},

    /* SCB */
    {0xE0000000, 0xFFFFFFFF, RDWR,   DEV_NGNRNE, NO_EXEC,    RG_ENABLE},
};

#endif /* __MPU_MAP_H__ */
