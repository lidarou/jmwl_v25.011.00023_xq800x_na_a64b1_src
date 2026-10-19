/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines MPU(Memory Protecting Unit) operations interface for
 *    cortex-m7
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x055e36
#define LOG_MODULE    AMOD_DRV

#include <core_op.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aic_size.h>
#include <cache.h>
#include <mpu.h>
#include <mpu_map.h>
#include "mpu_cm7.h"

static u32 total_regions = 0;
static u32 separate = 0;
static const struct rg_size size_array[] = {
    { RG_32B,    SIZE_32B},
    { RG_64B,    SIZE_64B},
    { RG_128B,   SIZE_128B},
    { RG_256B,   SIZE_256B},
    { RG_512B,   SIZE_512B},
    { RG_1KB,    SIZE_1K},
    { RG_2KB,    SIZE_2K},
    { RG_4KB,    SIZE_4K},
    { RG_8KB,    SIZE_8K},
    { RG_16KB,   SIZE_16K},
    { RG_32KB,   SIZE_32K},
    { RG_64KB,   SIZE_64K},
    { RG_128KB,  SIZE_128K},
    { RG_256KB,  SIZE_256K},
    { RG_512KB,  SIZE_512K},
    { RG_1MB,    SIZE_1M},
    { RG_2MB,    SIZE_2M},
    { RG_4MB,    SIZE_4M},
    { RG_8MB,    SIZE_8M},
    { RG_16MB,   SIZE_16M},
    { RG_32MB,   SIZE_32M},
    { RG_64MB,   SIZE_64M},
    { RG_128MB,  SIZE_128M},
    { RG_256MB,  SIZE_256M},
    { RG_512MB,  SIZE_512M},
    { RG_1GB,    SIZE_1G},
    { RG_2GB,    SIZE_2G},
    { RG_4GB,    SIZE_4G}
};

/* Collect ions */
void mpu_initialize(void)
{
    MPU_TYPE_T type_reg;

    type_reg.w = readl_relaxed((u32 *)MPU_TYPE);
    total_regions = type_reg.b.dregion;
    separate = type_reg.b.separate;
}

/* Enalbe MPU control */
void mpu_enable(void)
{
    MPU_CTRL_T ctrl;
    u32 ctrl_reg, primask;

    ctrl_reg = readl_relaxed((u32 *)MPU_CTRL);
    if (ctrl_reg & MPU_EN)
        return;

    ctrl.b.enable = MPU_EN;
    ctrl.b.hfnmiena = MPU_HFNMIENA_EN;
    ctrl.b.privdefena = 0;
    primask = __get_PRIMASK();
    if (!primask)
        __set_PRIMASK(0x1);
    dcache_invcc_all();
    icache_inv_all();
    writel_relaxed((u32 *)MPU_CTRL, ctrl.w);
    __DSB();
    __ISB();
    __set_PRIMASK(primask);
}

/* Disable MPU control */
void mpu_disable(void)
{
    MPU_CTRL_T ctrl;
    u32 ctrl_reg, primask;

    ctrl_reg = readl_relaxed((u32 *)MPU_CTRL);
    if (!(ctrl_reg & MPU_EN))
        return;

    ctrl.b.enable = 0;
    primask = __get_PRIMASK();
    if (!primask)
        __set_PRIMASK(0x1);
    dcache_invcc_all();
    icache_inv_all();
    writel_relaxed((u32 *)MPU_CTRL, ctrl.w);
    __DSB();
    __ISB();
    __set_PRIMASK(primask);
}

/*
 * Note: the function Config mpu region one by one and without
 * necessary cache operations.So it should be used in processing
 * mpu_config_table items which will do cache operations in
 * mpu_enable
 *
 */
void mpu_setup_region(u32 rg_num, const MPU_REGION_T *rg)
{
    MPU_RBAR_T rbar;
    MPU_RASR_T rasr;
    u64 length;
    int i;

    rbar.b.base = rg->base_addr >> 5;
    if (rg_num >= total_regions)
        return;
    rbar.b.region = rg_num;
    rbar.b.valid = 0x1;

    length = rg->end_addr - rg->base_addr + 1;
    for (i = 0; i < sizeof(size_array) / sizeof(struct rg_size); i++) {
        if (length <= size_array[i].size) {
            rasr.b.size = size_array[i].idx;
            break;
        }
    }

    switch (rg->ap) {
    case NACCESS:
        rasr.b.ap = MPU_AP_NACCESS;
        break;
    case RDONLY:
        rasr.b.ap = MPU_AP_RDONLY;
        break;
    case RDWR:
        rasr.b.ap = MPU_AP_RDWR;
        break;
    default:
        return;
    };

    switch (rg->cbtex) {
    case DEV_NGNRNE:
        rasr.b.attrs = MPU_DEV_NGNRNE;
        break;
    case MEM_NC:
        rasr.b.attrs = MPU_MEM_NC;
        break;
    case MEM_WTNWA:
        rasr.b.attrs = MPU_MEM_WTNWA;
        break;
    case MEM_WBNWA:
        rasr.b.attrs = MPU_MEM_WBNWA;
        break;
    case MEM_WBWA:
        rasr.b.attrs = MPU_MEM_WBWA;
        break;
    default:
        return;
    };

    switch (rg->xn) {
    case NO_EXEC:
        rasr.b.xn = MPU_NO_EXEC;
        break;
    default:
        rasr.b.xn = 0;
        break;
    };

    switch (rg->rg_stat) {
    case RG_ENABLE:
        rasr.b.en = MPU_RG_EN;
        rasr.b.srd = rg->rg_stat;
        break;
    case RG_DISABLE:
    default:
        rasr.b.en = 0x0;
        rasr.b.srd = 0xFF; /* Disable all sub regions */
        break;
    };

    writel_relaxed((u32 *)MPU_RBAR, rbar.w);
    writel_relaxed((u32 *)MPU_RASR, rasr.w);
}

/*
 * Note: Set all regions and enable mpu
 */
void mpu_config(void)
{
    int i;

    mpu_initialize();
    mpu_disable();

    for (i = 0; i < sizeof(mpu_cfg) / sizeof(MPU_REGION_T); i++)
        mpu_setup_region(i, &mpu_cfg[i]);

    mpu_enable();
}

/*
 * Note: After load image call it for
 * append the readonly ap for protect the code region.
 */
void mpu_config_after_load_image(void)
{
    /* dumy for compile. */
}

