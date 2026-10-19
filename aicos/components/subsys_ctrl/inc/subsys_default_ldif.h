
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the load info header file of the subsys.
 */
#ifndef __SUBSYS_DEFAULT_LDIF_H__
#define __SUBSYS_DEFAULT_LDIF_H__

#ifdef WIN32
#define SUBSYS_BIN_PARTION_DIR  "../../../../out/win32/debug/"
#define CP_REGION_BASE          0x00000000
#define CP_REGION_SIZE          0x00000000
#define SHM_NV_BASE             0x10000000
#define CP_VECTOR_REGION_BASE   0x20000000
#define ADSP_REGION_BASE        0x00000000
#define CP_EXE_START_ADDR       0x00000000
#define CP_REGION_BASE          0x00000000
#define SHMEM_REGION_BASE       0x00000000
#define SHMEM_REGION_SIZE       0x00000000
#define AONRAM_REGION_BASE      0x00000000
#define AONRAM_REGION_SIZE      0x00000000
#define CP_MAIN_IMG_BIN       "cp"
#else
#include <psram_mem_config.h>
#include <aonram_mem_config.h>
#include <sys_mem_config.h>
#endif

/*cp default load info */
#define CP_MEM_BASE         CP_REGION_BASE
#define CP_MEM_SIZE         0x200000
#define NV_MEM_BASE         SHM_NV_BASE
#define NV_MEM_SIZE         0x00040000
#define CP_IRAM_BASE        CP_VECTOR_REGION_BASE

#define CP_IMG_BIN          "cp"
#define NV_IMG_BIN          "nvram"
#define BOOT_CODE_BIN       "bootcode"

#ifdef WIN32
subsys_load_info s_cp_load_info = 
{
    0,
    0,
    1,
    3,
    {
        0xE3A00000, 0xEE061F12, 0xE3A00000, 0xEE060F11,
        0xE3A0003F, 0xEE060F51, 0xE3A00E32, 0xEE060F91,
        0xEE110F10, 0xE3800001, 0xEE010F10, 0xF57FF06F,
        0xE51FF004, 0x801e0000
    },
    {
        {CP_IMG_BIN,    CP_MEM_BASE,    CP_MEM_SIZE,    SUBSYS_REGION_FLASH},
        {NV_IMG_BIN,    NV_MEM_BASE,    NV_MEM_SIZE,    SUBSYS_REGION_NV},
        {BOOT_CODE_BIN, CP_IRAM_BASE,   BOOT_CODE_SIZE, SUBSYS_REGION_BOOTCODE}
    }
};
#endif

static const subsys_dump_t cp_dump_tab[] = {
    {"psr",   CP_REGION_BASE,     CP_REGION_SIZE,     SUBSYS_REGION_CPRAM},
    {"shm",   SHMEM_REGION_BASE,  SHMEM_REGION_SIZE,  SUBSYS_REGION_ATTR_RAM},
    {"aon",   AONRAM_REGION_BASE, AONRAM_REGION_SIZE, SUBSYS_REGION_ATTR_IRAM}
};

#ifdef CONFIG_SUPPORT_ADSP
#define ADSP_MAIN_IMG_BIN      "adsptcm"

static loader_region_t adsp_load_tab[] = {
    {"adspdtcm",   ADSP_DTCM_ADDR,     ADSP_DTCM_SIZE,     SUBSYS_REGION_ADSP_DTCM},
    {"adspptcm",   ADSP_PTCM_ADDR,     ADSP_PTCM_SIZE,     SUBSYS_REGION_ADSP_PTCM},
    {"adspram",    ADSP_REGION_BASE,   ADSP_REGION_SIZE,   SUBSYS_REGION_ADSP_RAM},
};

static const subsys_dump_t adsp_dump_tab[] = {
    {"arm",     ADSP_REGION_BASE,   ADSP_REGION_SIZE,   SUBSYS_REGION_ADSP_RAM},
    {"adt",     ADSP_DTCM_ADDR,     ADSP_DTCM_SIZE,     SUBSYS_REGION_ADSP_DTCM}
};
#endif

#endif /* __SUBSYS_DEFAULT_LDIF_H__ */

