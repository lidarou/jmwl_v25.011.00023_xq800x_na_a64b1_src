/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is the memory layout file of the psram.
 *    Every region sub region be described like this:
      region name | (base, size)
 */

#ifndef __PSRAM_MEM_CONFIG_H__
#define __PSRAM_MEM_CONFIG_H__

#ifndef CONFIG_SUBSYS_CP
#include <flash_config.h>
#endif

/*******************************************************
                  ALL PSRAM
********************************************************/
#define PSRAM_REGION_BASE       0x80000000
#define PSRAM_REGION_SIZE       0x00200000

#ifdef CONFIG_CP_PSRAM_REMAP
/***********************************************************
                 PSRAM layout: (0x80000000, 0x00200000)
------------------------------------------------------------
                 AP | (0x80000000, 0x00095000)
------------------------------------------------------------
                 CP | (0x80095000, 0x00148000)
--------------------------------------------------------
       share memory | (0x801DD000, 0x00023000)
--------------------------------------------------------
************************************************************/

/*******************************************************
                  AP region
********************************************************/
#define AP_REGION_BASE          PSRAM_REGION_BASE
#define AP_REGION_SIZE          0x00075000

/*******************************************************
                  CP memory region
********************************************************/
#define CP_REGION_BASE          (AP_REGION_BASE + AP_REGION_SIZE)
#define CP_REGION_SIZE          0x00168000

/*******************************************************
                  share memory region
********************************************************/
#define SHMEM_REGION_BASE       (CP_REGION_BASE + CP_REGION_SIZE)
#define SHMEM_REGION_SIZE       0x00023000

#define CP_PSRAM_REMAP_BASE     0x80000000
#define CP_PSRAM_REMAP_SIZE     CP_REGION_SIZE
#define CP_PSRAM_REMAP_OFFSET   (CP_REGION_BASE - CP_PSRAM_REMAP_BASE)
#define CP_REMAP_TO_PSRAM(addr) ((addr) + CP_PSRAM_REMAP_OFFSET)
#else
/***********************************************************
                 PSRAM layout: (0x80000000, 0x00200000)
------------------------------------------------------------
                 CP | (0x80000000, 0x00148000)
------------------------------------------------------------
                 AP | (0x80148000, 0x0009CC00)
--------------------------------------------------------
       share memory | (0x801E4C00, 0x0001B400)
--------------------------------------------------------
************************************************************/

/*******************************************************
                  CP memory region
********************************************************/
#define CP_REGION_BASE          PSRAM_REGION_BASE
#define CP_REGION_SIZE          0x00148000

#ifdef CONFIG_SUBSYS_AP
#if (QSPI_NOR_DEVICE_NUM == 2)
#define CP_CODE_SIZE            0x00094000
#define CP_HEAP_SIZE            0x00074800
#define CP_LOAD_BASE            (CP_REGION_BASE + CP_CODE_SIZE + CP_HEAP_SIZE)
//#define CP_COMPRESS_TO_EXT_FLASH
#endif
#endif
/*******************************************************
                  AP region
********************************************************/
#define AP_REGION_BASE          (CP_REGION_BASE + CP_REGION_SIZE)
#define AP_REGION_SIZE          0x0009CC00

/*******************************************************
                  share memory region
********************************************************/
#define SHMEM_REGION_BASE       (AP_REGION_BASE + AP_REGION_SIZE)
#define SHMEM_REGION_SIZE       0x0001B400
#endif /* #ifdef CONFIG_CP_PSRAM_REMAP */

/***********************************************************
            share memory layout: (0x801E4C00, 0x0001B400)
--------------------------------------------------------
            cmdline   | 0x00000800
--------------------------------------------------------
            cp --> ap | 0x0000AE00
--------------------------------------------------------
            ap --> cp | 0x00006800
--------------------------------------------------------
            NV        | 0x00009600
--------------------------------------------------------

************************************************************/
#ifdef CONFIG_CP_PSRAM_REMAP
/* share memory region can't overlay with cp psram map region */
#if !(((SHMEM_REGION_BASE + SHMEM_REGION_SIZE) <= CP_PSRAM_REMAP_BASE) || \
    (SHMEM_REGION_BASE >= (CP_PSRAM_REMAP_BASE + CP_PSRAM_REMAP_SIZE)))
#error share memory region can not overlay with cp psram map region.
#endif
#endif

#define SHM_CMDLINE_BASE        (SHMEM_REGION_BASE)
#define SHM_CMDLINE_SIZE        0x00000400

#define SHM_CP2AP_BASE          (SHM_CMDLINE_BASE + SHM_CMDLINE_SIZE)
#define SHM_CP2AP_SIZE          0x0000B200

#define SHM_AP2CP_BASE          (SHM_CP2AP_BASE + SHM_CP2AP_SIZE)
#define SHM_AP2CP_SIZE          0x00006800

#define SHM_NV_BASE             (SHM_AP2CP_BASE + SHM_AP2CP_SIZE)
#define SHM_NV_SIZE             0x00009600

#ifdef CONFIG_CP_PSRAM_REMAP
/****************************************************************
 layout for bootloader(fota) + aos backup (extend img + fota img)
----------------------------------------------------------------
            bl heap | (0x80004000, 0x0019C000) can't overlay backup
----------------------------------------------------------------
           ext heap | (cp base, 0x00100000)
----------------------------------------------------------------
             backup | (0x801A0000, 0x00060000)
----------------------------------------------------------------
*****************************************************************/
#define FOTA_AOS_BL_HEAP_BASE   0x80000000
#define FOTA_AOS_BL_HEAP_SIZE   0x001A0000

#define FOTA_AOS_EXT_HEAP_BASE  CP_REGION_BASE
#define FOTA_AOS_EXT_HEAP_SIZE  (CP_REGION_SIZE - FOTA_AOS_BACKUP_SIZE)

#define FOTA_AOS_BACKUP_BASE    (0x801A0000)
#define FOTA_AOS_BACKUP_SIZE    0x00060000
#else
/****************************************************************
 layout for bootloader(fota) + aos backup (extend img + fota img)
----------------------------------------------------------------
             backup | (cp base, 0x00100000)
----------------------------------------------------------------
           ext heap | (0x80100000, ap base - backup size)
----------------------------------------------------------------
            bl heap | (0x80180000, end) can't overlay backup
----------------------------------------------------------------
*****************************************************************/
#define FOTA_AOS_BACKUP_BASE    CP_REGION_BASE
#define FOTA_AOS_BACKUP_SIZE    0x00090000

#define FOTA_AOS_EXT_HEAP_BASE  (FOTA_AOS_BACKUP_BASE + FOTA_AOS_BACKUP_SIZE)
#define FOTA_AOS_EXT_HEAP_SIZE  (AP_REGION_BASE - FOTA_AOS_EXT_HEAP_BASE)

#define FOTA_AOS_BL_HEAP_BASE   (FOTA_AOS_BACKUP_BASE + FOTA_AOS_BACKUP_SIZE)
#define FOTA_AOS_BL_HEAP_SIZE   0x001A0000
#endif /* CONFIG_CP_PSRAM_REMAP */

#define BASIC_SYS_MIN_HEAP_SIZE  (200 * 1024)
#define FOTA_EXTIMG_PARTION_NAME "fota_bak"
#define RUNDATA_PTNAME           "normdata"
#define FOTA_PARA_PTNAME         "fota"
#define FOTA_DIFF_PAC_MAX_SIZE   (50 * FLASH_BLOCK_SIZE)

/*
 * FOTA_ONLY_BACK_UP_EXTEND_IMAGE.
 *
 * If the fota_bak group only has one image, can open it.
 * when update the basic image.
 * if open it, just backup the extend image to ram.
 * otherwise,
 * will backup all theota_bak group area to ram.
 */
#define FOTA_ONLY_BACK_UP_EXTEND_IMAGE  1

#ifdef CONFIG_FOTA_SUPPORT

#ifdef CONFIG_CP_PSRAM_REMAP
/*
 * FOTA_AOS_BL_HEAP_BASE + FOTA_AOS_BL_HEAP_SIZE must <= FOTA_AOS_BACKUP_BASE,
 * can't overlay backup arer.
 *  Atention:
 * because the bl heap base and size was
 * fiexd gx_board.ld, here must define as same as gx_board.ld.
 * if change here, also need change it in gx_board.ld.
 */
#if (FOTA_AOS_BL_HEAP_BASE + FOTA_AOS_BL_HEAP_SIZE > FOTA_AOS_BACKUP_BASE)
#error FOTA_AOS_BL_HEAP_BASE + FOTA_AOS_BL_HEAP_SIZE must <= FOTA_AOS_BACKUP_BASE
#endif
#else
/*
 * FOTA_AOS_BL_HEAP_BASE must >= FOTA_AOS_EXT_HEAP_BASE,
 * can't overlay backup arer,.but can't use the same feature as
 * FOTA_AOS_EXT_HEAP_BASE, because the FOTA_AOS_BL_HEAP_BASE was
 * fiexd gx_board.ld, here must define as same as gx_board.ld.
 */
#if (FOTA_AOS_BL_HEAP_BASE < FOTA_AOS_EXT_HEAP_BASE)
#error FOTA_AOS_BL_HEAP_BASE must >= FOTA_AOS_EXT_HEAP_BASE
#endif
#endif /* CONFIG_CP_PSRAM_REMAP */

/*
 * FOTA_AOS_BACKUP_SIZE must <= CP_REGION_SIZE.
 * only cp region can be used to backup in the basic mode.
 */
#if (FOTA_AOS_BACKUP_SIZE > CP_REGION_SIZE)
#error FOTA_AOS_BACKUP_SIZE must <= CP_REGION_SIZE
#endif
#endif /* CONFIG_FOTA_SUPPORT */
#endif /* __PSRAM_MEM_CONFIG_H__ */

