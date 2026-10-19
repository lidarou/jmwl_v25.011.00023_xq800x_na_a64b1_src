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

#ifndef __MPU_MAP_MULTI_CS_H__
#define __MPU_MAP_MULTI_CS_H__

/*
 * The total number of region is 16(0, 15), and the index of the array
 * element i is corresponds the region i, exceed regions will be ignored.
 * If the background region not enabled, say PRIVDEFENA set to 0
 * then any access doesn't any region in mpu_cfg should generate fault.
 * On ARM-v8 not support regiong overlay and subregion.
 */
const MPU_REGION_T mpu_cfg[] = {
    /* flash0 */
    /* NOR flash indac/dac write */
    {MPU_RG(FLASH0_INDAC_RW_BASE,    FLASH0_INDAC_RW_SIZE),
     RDWR,   MEM_NC,     NO_EXEC,    RG_ENABLE},

    /* NOR flash dac read */
    {MPU_RG(FLASH0_DAC_RO_BASE,    FLASH0_DAC_RO_SIZE),
     RDONLY, MEM_WTNWA,  EXEC,       RG_ENABLE},

    /* flash1 */
    /* NOR flash dac read and exe, [FLASH0_BASE, FLASH0_AP_EXTEND_BASE)*/
    {FLASH1_BASE,        (FLASH1_AP_EXTEND_BASE - 1),
     RDONLY, MEM_WTNWA,  EXEC,       RG_ENABLE},

    /* NOR flash dac read and exe, [FLASH0_FOTA_BAK_BASE, FLASH0_FOTA_BAK_END)*/
    {MPU_RG(FLASH1_FOTA_BAK_BASE,  FLASH1_FOTA_BAK_SIZE),
     RDONLY, MEM_WTNWA,  EXEC,       RG_ENABLE},

    /* NOR flash dac read and no-exe, [FLASH0_FOTA_BAK_END, FLASH0_END]*/
    {FLASH1_FOTA_BAK_BASE + FLASH1_FOTA_BAK_SIZE, (FLASH1_BASE + FLASH1_TOTAL_SIZE - 1),
     RDONLY, MEM_WTNWA,  EXEC,       RG_ENABLE},

    /* IRAM */
    /* CP vector, AP vector and AP sys stack. */
    {AONRAM_REGION_BASE, (AON_SHMEM_REGION_BASE - 1),
     RDWR,   MEM_WBWA,   EXEC,    RG_ENABLE},

    /* share memory IRAM + phybuf IRAM */
    {MPU_RG(AON_SHMEM_REGION_BASE, AON_SHMEM_REGION_SIZE + PHY_BUF_REGION_SIZE),
     RDWR,   MEM_NC,     NO_EXEC,    RG_ENABLE},

    /* PSRAM: AP need dump all ps ram, and need load subsys code.*/
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

MPU_REGION_T mpu_cfg_cs[FLASH_DEVICE_NUM][1] = {
    {
        /* flash cs 0 */
        /* NOR flash indac/dac write */
        {MPU_RG(FLASH0_INDAC_RW_BASE,    FLASH0_INDAC_RW_SIZE),
         RDWR,   MEM_NC,     NO_EXEC,    RG_ENABLE},
    },
    {
        /* flash cs 1 */
        /* NOR flash indac/dac write */
        {MPU_RG(FLASH1_INDAC_RW_BASE,    FLASH1_INDAC_RW_SIZE),
         RDWR,   MEM_NC,     NO_EXEC,    RG_ENABLE},
    },
};

#endif /* __MPU_MAP_MULTI_CS_H__ */
