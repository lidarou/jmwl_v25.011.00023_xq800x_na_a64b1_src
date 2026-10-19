/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is the memory layout file of the ap sys.
 *    Every region sub region be described like this:
      region name | (base, size)
 */

#ifndef __SYS_MEM_CONFIG_H__
#define __SYS_MEM_CONFIG_H__

#include <aonram_mem_config.h>
#include <psram_mem_config.h>
#include <flash_mem_config.h>

/***********************************************************
         ap psram mem layout: (AP_REGION_BASE, 0x00090400)
--------------------------------------------------------
             code | 0x00004800
--------------------------------------------------------
             heap | 0x00074800
--------------------------------------------------------
             rw   | 0x00017400
-------------------------------------------------------
************************************************************/

/*******************************************************
                  ALL ap PSRAM
********************************************************/
#define SYS_PSRAM_REGION_BASE   AP_REGION_BASE
#define SYS_PSRAM_REGION_SIZE   AP_REGION_SIZE

/*******************************************************
                  flash region
********************************************************/
#if defined (CONFIG_SECURE_BOOT)
#define BIN_HEAD_SIZE           0x30
#else
#define BIN_HEAD_SIZE           0x0
#endif

#ifdef CONFIG_MCX_SUPPORT
#define CP_BIN_HEAD_SIZE        0x30
#define MCX_REGION_BASE         (FLASH0_CP_BASE + CP_BIN_HEAD_SIZE)
#define MCX_REGION_SIZE         FLASH0_CP_SIZE
#endif

#define AP_FLASH_REGION_BASE    (FLASH0_AP_BASE + BIN_HEAD_SIZE)
#define AP_FLASH_REGION_SIZE    FLASH0_AP_SIZE
#define AP_EXT_REGION_BASE      (FLASH0_AP_EXTEND_BASE + BIN_HEAD_SIZE)
#define AP_EXT_REGION_SIZE      FLASH0_AP_EXTEND_SIZE

/*******************************************************
                  code region
********************************************************/
#define CODE_REGION_BASE        SYS_PSRAM_REGION_BASE
#define CODE_REGION_SIZE        0x00004800
#define LOAD_REGION_BASE        (CODE_REGION_BASE)
#define LOAD_REGION_SIZE        (CODE_REGION_SIZE + HEAP_REGION_SIZE)

/*******************************************************
                  Heap memory region
********************************************************/
#define HEAP_REGION_BASE        (CODE_REGION_BASE + CODE_REGION_SIZE)
#define HEAP_REGION_SIZE        0x00074800

/*******************************************************
                  rw memory region
********************************************************/
#define RW_REGION_BASE          (HEAP_REGION_BASE + HEAP_REGION_SIZE)
#define RW_REGION_SIZE          0x00017400

/*******************************************************
                  ap vector region
********************************************************/
#define VECTOR_REGION_BASE      AP_VECTOR_REGION_BASE
#define VECTOR_REGION_SIZE      AP_VECTOR_REGION_SIZE

/*******************************************************
                  sys stack memory region
********************************************************/
#define ALL_STACK_BASE          AP_SYSSTACK_REGION_BASE
#define ALL_STACK_SIZE          AP_SYSSTACK_REGION_SIZE

/*--------------------------------------------------------
                  sys stack memory region
--------------------------------------------------------*/
#define INIT_HEAP_SIZE          0x080
#define SYS_STACK_SIZE          (ALL_STACK_SIZE - INIT_HEAP_SIZE)

#define INIT_HEAP_BASE          ALL_STACK_BASE
#define SYS_STACK_BOTTOM        (ALL_STACK_BASE + ALL_STACK_SIZE - 4)

/*-------------------------------------------------------
                  to cp share memory
 -------------------------------------------------------*/
#define SYS_2_CP_SHM_BASE       SHM_AP2CP_BASE
#define SYS_2_CP_SHM_SIZE       SHM_AP2CP_SIZE

/*-------------------------------------------------------
                  bin path config
 -------------------------------------------------------*/
#define SUBSYS_BIN_PARTION_DIR  ""
#define CP_MAIN_IMG_BIN         "cp"

#endif /* __SYS_MEM_CONFIG_H__ */