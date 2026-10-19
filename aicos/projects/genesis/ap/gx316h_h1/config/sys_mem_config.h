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
         ap psram mem layout: (0x80378000, 0x00440000)
--------------------------------------------------------
             code | 0x00034000
--------------------------------------------------------
             heap | 0x0029C000
--------------------------------------------------------
             rw   | 0x00170000
-------------------------------------------------------
************************************************************/

/*******************************************************
                  ALL ap PSRAM
********************************************************/
#define SYS_PSRAM_REGION_BASE   AP_REGION_BASE
#define SYS_PSRAM_REGION_SIZE   AP_REGION_SIZE

/*******************************************************
                  code region
********************************************************/
#define CODE_REGION_BASE        SYS_PSRAM_REGION_BASE
#define CODE_REGION_SIZE        0x00034000
#define LOAD_REGION_BASE        (CODE_REGION_BASE)
#define LOAD_REGION_SIZE        (CODE_REGION_SIZE + HEAP_REGION_SIZE)

/*******************************************************
                  Heap memory region
********************************************************/
#define HEAP_REGION_BASE        (CODE_REGION_BASE + CODE_REGION_SIZE)
#define HEAP_REGION_SIZE        0x0029C000

/*******************************************************
                  rw memory region
********************************************************/
#define RW_REGION_BASE          (HEAP_REGION_BASE + HEAP_REGION_SIZE)
#define RW_REGION_SIZE          0x00170000

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
#define CP_MAIN_IMG_BIN         "cpram2"
#define CP_MAIN_IMG_IS_LZMA

#endif /* __SYS_MEM_CONFIG_H__ */