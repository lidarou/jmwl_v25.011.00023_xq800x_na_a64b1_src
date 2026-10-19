
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the header file of the subsys header.
 */
#ifndef __SUBSYS_HEADER_H__
#define __SUBSYS_HEADER_H__

#include <aic_type.h>
#include <aic_bitmap.h>

/* MACRO define */
#define MAX_BOOT_CODE       24
#define MAX_REGIN_NUM       8
#define MAX_NAME_LEN        8
#define SUBSYS_HEAD_MAGIC   0x43444546

#define SUBSYS_REGION_ATTR_SEC_VERIFY   BIT_0
#define SUBSYS_REGION_ATTR_BOOTCODE     BIT_1
#define SUBSYS_REGION_ATTR_RAM          BIT_2
#define SUBSYS_REGION_ATTR_IRAM         BIT_3
#define SUBSYS_REGION_ATTR_INIT_ZERO    BIT_4
#define SUBSYS_REGION_ATTR_NV           BIT_5
#define SUBSYS_REGION_ATTR_LZMA         BIT_6
#define SUBSYS_REGION_ATTR_ADSP_PTCM    BIT_7
#define SUBSYS_REGION_ATTR_ADSP_DTCM    BIT_8
#define SUBSYS_REGION_ATTR_NOLOAD       BIT_9
#define SUBSYS_REGION_ATTR_CP_TCM       BIT_10

#define SUBSYS_REGION_ATTR_AP           BIT_31
#define SUBSYS_REGION_ATTR_CP           BIT_30
#define SUBSYS_REGION_ATTR_ADSP         BIT_29

#define SUBSYS_REGION_FLASH         (SUBSYS_REGION_ATTR_CP | \
                                     SUBSYS_REGION_ATTR_SEC_VERIFY | \
                                     SUBSYS_REGION_ATTR_NOLOAD)

#define SUBSYS_REGION_CPRAM         (SUBSYS_REGION_ATTR_CP | \
                                     SUBSYS_REGION_ATTR_RAM |  \
                                     SUBSYS_REGION_ATTR_LZMA | \
                                     SUBSYS_REGION_ATTR_SEC_VERIFY)

#define SUBSYS_REGION_NV            (SUBSYS_REGION_ATTR_CP | \
                                     SUBSYS_REGION_ATTR_RAM | \
                                     SUBSYS_REGION_ATTR_NV)

#define SUBSYS_REGION_BOOTCODE      (SUBSYS_REGION_ATTR_CP | \
                                     SUBSYS_REGION_ATTR_BOOTCODE | \
                                     SUBSYS_REGION_ATTR_IRAM)

#define SUBSYS_REGION_ADSP_PTCM     (SUBSYS_REGION_ATTR_ADSP | \
                                     SUBSYS_REGION_ATTR_LZMA | \
                                     SUBSYS_REGION_ATTR_ADSP_PTCM)

#define SUBSYS_REGION_ADSP_DTCM     (SUBSYS_REGION_ATTR_ADSP | \
                                     SUBSYS_REGION_ATTR_LZMA | \
                                     SUBSYS_REGION_ATTR_ADSP_DTCM)

#define SUBSYS_REGION_ADSP_RAM      (SUBSYS_REGION_ATTR_ADSP | \
                                     SUBSYS_REGION_ATTR_LZMA | \
                                     SUBSYS_REGION_ATTR_RAM)

#ifdef WIN32
#pragma pack(push, 1)
typedef struct subsys_load_region {
     char    region_name[MAX_NAME_LEN + 1];
     base_t  region_base;
     size_t  region_size;
     u32     region_attr;
 } loader_region_t;

typedef struct sys_load_info_data {
    u32             magic;
    u16             reserv;
    u8              ver;
    u8              region_num;
    u32             boot_code[MAX_BOOT_CODE];
    loader_region_t region[MAX_REGIN_NUM];
} subsys_load_info;

#pragma pack(pop)
#else
typedef struct subsys_load_region {
     char    region_name[MAX_NAME_LEN + 1];
     base_t  region_base;
     size_t  region_size;
     u32     region_attr;
 } loader_region_t;

typedef struct sys_load_info_data {
    u32             magic;
    u16             reserv;
    u8              ver;
    u8              region_num;
    u32             boot_code[MAX_BOOT_CODE];
    loader_region_t region[MAX_REGIN_NUM];
} subsys_load_info;
#endif /* WIN32 */

#define BOOT_CODE_SIZE  MAX_BOOT_CODE *sizeof(u32)

typedef struct subsys_region {
     char    region_name[MAX_NAME_LEN + 1];
     base_t  region_base;
     size_t  region_size;
     u32     region_attr;
     u8      *sec_head;
 } subsys_region_t;

#endif /* __SUBSYS_HEADER_H__ */

