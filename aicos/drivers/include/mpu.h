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

#include <aic_type.h>
#include <aic_bitmap.h>

/*
 * One region is triviallzed into 8 subregions accrording to the size
 * of the region, Each bit of SRD represents a sub-region, When it set to 1
 * will disable this subregion.
 */
#define RG_ENABLE   0x00
#define RG_DISABLE  0xFF
#define RG_DIS_SUB(l, h)    (u8)GENMASK(l, h)
#define RG_ENA_SUB(l, h)    (u8)(~(GENMASK(l, h)))
#define RG_DIS_A_SUB(n)     (u8)BIT(n)
#define RG_ENA_A_SUB(n)     (u8)(~BIT(n))

/* rg size convert to rg end addr. */
#define MPU_RG(base, size) (base), (base) + (size) - 1

typedef enum {
    NACCESS = 0,      /* No access */
    RDONLY,           /* Read only */
    RDWR              /* Read and write */
} MPU_AP_T;

typedef enum {
    DEV_NGNRNE = 0,   /* Strongly-ordered device */
    DEV_SH,           /* Shareable device. */
    DEV_NSH,          /* Non-shareable device. */
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
    u8  rg_stat;
} MPU_REGION_T;

typedef struct {
    u32 base_addr;
    u32 end_addr;
} MPU_REGION_AREA_T;

typedef enum {
    CONF_EXCLUDE_RDONLY = 0,
    CONF_APPEND_RDONLY,
    CONF_NORMAL
} MPU_CONFIG_T;

typedef struct mpu_mgr {
    u16  total_region;
    u16  alloc_base;
    u32 free_bits;
} mpu_mgr_t;

/*
 * Note: Before load image call it for
 * Set all regions and enable mpu, exclude readonly ap for load image.
 */
void mpu_config_before_load_image(void);

/*
 * Note: After load image call it for
 * append the readonly ap for protect the code region.
 */
void mpu_config_after_load_image(void);

/*
 * Note: Set all regions and enable mpu
 */
void mpu_config_normal(void);

/*
 * mpu_dynamic_conf_region: Config a mpu region with specified region config.
 *
 * @rg: the pointer of the specified region config.
 * @cache_op: if need do disable and enable operation.

 * @return: < 0 -- failed; >=0 -- success, return the region index.
 *
 * This funtin will try to get a free region to finish the mpu config,
 * If cant't get a free region will return AIC_ENODEV. If config succ
 * will return the region index to caller.
 * After the caller no need this mpu region, must call
 * the function  mpu_disable_a_region to release the region.
 *
 * Attention:
 * The cache disable and enable operation will take tool much time,
 * It may cause phy work unnormaly. So.If just use it to protect the memory,
 * no need change the cache attribute of the memory, so can remove cache operation.
 */
int mpu_dynamic_conf_region(const MPU_REGION_T *rg, bool cache_op);

/*
 * mpu_dynamic_unconf_region:Unconfig a mpu region with the region index that
 *                             returned by mpu_dynamic_conf_a_region function.
 *
 * @index: the index of the region that get from mpu_dynamic_conf_a_region func.

 * This funtin will disable and release the region.
 */
void mpu_dynamic_unconf_region(int index, bool cache_op);


/*
 * Note: the function can disable or enable a mpu region
 * by specified region area.
 *
 * return: succ, return AIC_SUCCESS, otherwise failed.
 */
int mpu_switch_region_by_area(MPU_REGION_AREA_T *rg_area, bool enable);

/*
 * Note: the function used for disable a mpu region by specified region area.
 */
#define mpu_enable_region_by_area(area) \
    mpu_switch_region_by_area(area, TRUE)


/*
 * Note: the function used for disable a mpu region by specified region area.
 */
#define mpu_disable_region_by_area(area) \
    mpu_switch_region_by_area(area, FALSE)

/*
 * Note: the function used reconfig a mpu region by
 * specified mpu region info.
 * 
 * return: succ, return AIC_SUCCESS, otherwise failed.
 */
int mpu_reconfig_region_by_area(const MPU_REGION_T *rg);

/**
 * mpu_suspend() - MPU backup
 *
 * @addr: buffer address to save mpu registers.
 *
 */
void mpu_suspend(u32 *addr);

/**
 * mpu_resume() - MPU restore
 *
 * @addr: buffer address to save mpu registers.
 *
 */
void mpu_resume(u32 *addr);

int mpu_find_region_by_area(MPU_REGION_AREA_T *rg_area);

MPU_REGION_T* mpu_get_region_table(void);

int mpu_get_region_num(void);

void mpu_reconfig_region(u32 rg_num, const MPU_REGION_T *rg);

MPU_REGION_T* mpu_get_multi_cs_region_table(u8 cs);

int mpu_get_multi_cs_region_num(u8 cs);

/**
 * mpu_auto_align_region() - auto align the region
 *
 * @addr: buffer address to save mpu registers.
 *
 */
static inline void mpu_auto_align_region(u32 *addr_ptr, u32 *size_ptr)
{
    u32 o_addr = *addr_ptr, o_size = *size_ptr;
    u32 s, n_addr, n_size;

    /* size must be a power of 2. */
    for (s = 32; s < o_size; s <<= 1)
        ;

    /* addr must align with the size. */
    do {
        n_size = s;
        n_addr = ((o_addr) & ~(n_size - 1));

        if (s == 0x80000000)
            break;

        s <<= 1;
    } while (n_addr + n_size < o_addr + o_size);

    *addr_ptr = n_addr;
    *size_ptr = n_size;
}

#endif /* __MPU_H__ */
