/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file make statment of mpu
 *
 */

#ifndef __MPU_H__
#define __MPU_H__

#include <io.h>

typedef enum {
    RG_ENABLE = 0,        /* status that region enable */
    RG_DISABLE           /* status that region disable */
} MPU_RG_STAT_T;

typedef enum {
    NACCESS = 0,      /* No access */
    RDONLY,           /* Read only */
    RDWR              /* Read and write */
} MPU_AP_T;

typedef enum {
    DEV_NGNRNE = 0,   /* Device */
    MEM_NC,           /* Normal memory, non-cacheable */
    MEM_WTNWA,        /* Normal memory, write-through read-allocate */
    MEM_WBNWA,        /* Normal memory, write-back read-allocate */
    MEM_WBWA          /* Normal memory, write-back read&write-allocate */
} MPU_CBTEX_T;

typedef enum {
    NO_EXEC = 0,      /* No execute */
    EXEC
} MPU_XN_T;

typedef struct {
    u32 base_addr;
    u32 end_addr;
    MPU_AP_T ap;
    MPU_CBTEX_T cbtex;
    MPU_XN_T xn;
    MPU_RG_STAT_T rg_stat;
} MPU_REGION_T;

/* rg size convert to rg end addr. */
#define MPU_RG(base, size) (base), (base) + (size) - 1

#ifdef CONFIG_BL_CPU_MPU
void mpu_config(void);
void mpu_disable(void);
#else
static inline void mpu_config(void) {}
static inline void mpu_disable(void) {}
#endif

void mpu_setup_region(u32 rg_num, const MPU_REGION_T rg);
MPU_REGION_T * mpu_get_region_table(void);
int mpu_get_region_num(void);

MPU_REGION_T *mpu_get_multi_cs_region_table(u8 cs);

int mpu_get_multi_cs_region_num(u8 cs);
void mpu_disable(void);

#endif /* __MPU_H__ */
