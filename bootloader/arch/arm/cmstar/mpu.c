/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines MPU(Memory Protecting Unit) operations interface for
 *    cortex-mstar
 */

#include <aic_size.h>
#include <cache.h>
#include <cmsis/core_star.h>
#include <mpu.h>
#include <mpu/mpu_map.h>
#include "mpu_cmstar.h"

static u32 total_regions = 0;
static u32 separate = 0;

/* Collect MPU informations */
void mpu_initialize(void)
{
    MPU_TYPE_T type_reg;
    MPU_MAIR0_T mair0;
    MPU_MAIR1_T mair1;

    type_reg.w = readl((u32 *)MPU_TYPE);
    total_regions = type_reg.b.dregion;
    separate = type_reg.b.separate;
    mair0.b.attr0 = MPU_ATTR0_NGNRNE;
    mair0.b.attr1 = MPU_ATTR1_WTNWA;
    mair0.b.attr2 = MPU_ATTR2_WBNWA;
    mair0.b.attr3 = MPU_ATTR3_WBWA;
    mair1.b.attr4 = MPU_ATTR4_NC;
    writel((u32 *)MPU_MAIR0, mair0.w);
    writel((u32 *)MPU_MAIR1, mair1.w);
}

/* Enalbe MPU control */
void mpu_enable(void)
{
    MPU_CTRL_T ctrl;
    u32 ctrl_reg, primask;

    ctrl_reg = readl((u32 *)MPU_CTRL);
    if (ctrl_reg & MPU_EN)
        return;

    ctrl.b.enable = MPU_EN;
    ctrl.b.hfnmiena = MPU_HFNMIENA_EN;
    ctrl.b.privdefena = 0x0;
    primask = __get_PRIMASK();
    if (!primask)
        __set_PRIMASK(0x1);
    dcache_invcc_all();
    icache_inv_all();
    writel((u32 *)MPU_CTRL, ctrl.w);
    __DSB();
    __ISB();
    __set_PRIMASK(primask);
}

/* Disable MPU control */
void mpu_disable(void)
{
    u32 ctrl_reg, primask;
    MPU_CTRL_T ctrl;

    ctrl_reg = readl((u32 *)MPU_CTRL);
    if (!(ctrl_reg & MPU_EN))
        return;

    ctrl.b.enable = 0x0;
    primask = __get_PRIMASK();
    if (!primask)
        __set_PRIMASK(0x1);
    dcache_invcc_all();
    icache_inv_all();
    writel((u32 *)MPU_CTRL, ctrl.w);
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
void mpu_setup_region(u32 rg_num, const MPU_REGION_T rg)
{
    MPU_RBAR_T rbar;
    MPU_RLAR_T rlar;
    MPU_RNR_T rnr;

    /* No shareable */
    rbar.b.sh = 0x0;
    rbar.b.base = rg.base_addr >> 5;
    if (rg_num >= total_regions)
        return;
    rnr.b.region = rg_num;
    rlar.b.limit = rg.end_addr >> 5;
    rlar.b.pxn = 0x0;

    switch (rg.ap) {
    case RDONLY:
        rbar.b.ap = MPU_AP_RDONLY;
        break;
    case RDWR:
        rbar.b.ap = MPU_AP_RDWR;
        break;
    case NACCESS:
    default:
        return;
    };

    switch (rg.cbtex) {
    case DEV_NGNRNE:
        rlar.b.attridx = MPU_DEV_NGNRNE;
        break;
    case MEM_NC:
        rlar.b.attridx = MPU_MEM_NC;
        break;
    case MEM_WTNWA:
        rlar.b.attridx = MPU_MEM_WTNWA;
        break;
    case MEM_WBNWA:
        rlar.b.attridx = MPU_MEM_WBNWA;
        break;
    case MEM_WBWA:
        rlar.b.attridx = MPU_MEM_WBWA;
        break;
    default:
        return;
    };

    switch (rg.xn) {
    case NO_EXEC:
        rbar.b.xn = MPU_NO_EXEC;
        break;
    default:
        rbar.b.xn = 0x0;
        break;
    };

    switch (rg.rg_stat) {
    case RG_ENABLE:
        rlar.b.en = MPU_RG_EN;
        break;
    case RG_DISABLE:
    default:
        rlar.b.en = 0x0;
        break;
    };

    writel((u32 *)MPU_RNR, rnr.w);
    writel((u32 *)MPU_RBAR_A0, rbar.w);
    writel((u32 *)MPU_RLAR_A0, rlar.w);
}

/*
 * Note: Set all regions and enable mpu
 */
void mpu_config(void)
{
    int i;

    mpu_initialize();
    cache_disable();

    for (i = 0; i < sizeof(mpu_cfg) / sizeof(MPU_REGION_T); i++)
        mpu_setup_region(i, mpu_cfg[i]);

    mpu_enable();
    cache_enable();
}

MPU_REGION_T * mpu_get_region_table(void)
{
    return (MPU_REGION_T *)mpu_cfg;
}

int mpu_get_region_num(void)
{
    return ARRAY_SIZE(mpu_cfg);
}

#if (QSPI_NOR_DEVICE_NUM > 1)
MPU_REGION_T *mpu_get_multi_cs_region_table(u8 cs)
{
    return mpu_cfg_cs[cs];
}

int mpu_get_multi_cs_region_num(u8 cs)
{
    return ARRAY_SIZE(mpu_cfg_cs[cs]);
}
#else
MPU_REGION_T *mpu_get_multi_cs_region_table(u8 cs)
{
    return NULL;
}

int mpu_get_multi_cs_region_num(u8 cs)
{
    return 0;
}
#endif
