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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x042974
#define LOG_MODULE    AMOD_DRV

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_size.h>
#include <cache.h>
#include <core_op.h>
#include <mpu.h>
#include <mpu_map.h>
#include "mpu_cmstar.h"

static mpu_mgr_t s_mpu_mgr = {0};

/* Collect MPU informations */
void mpu_initialize(void)
{
    MPU_TYPE_T type_reg;
    MPU_MAIR0_T mair0;
    MPU_MAIR1_T mair1;

    type_reg.w = AIC_READ32(MPU_TYPE);
    s_mpu_mgr.total_region = type_reg.b.dregion;

    mair0.b.attr0 = MPU_ATTR0_NGNRNE;
    mair0.b.attr1 = MPU_ATTR1_WTNWA;
    mair0.b.attr2 = MPU_ATTR2_WBNWA;
    mair0.b.attr3 = MPU_ATTR3_WBWA;
    mair1.b.attr4 = MPU_ATTR4_NC;
    AIC_WRITE32(MPU_MAIR0, mair0.w);
    AIC_WRITE32(MPU_MAIR1, mair1.w);
}

/* Enalbe MPU control */
void mpu_enable(void)
{
    MPU_CTRL_T ctrl;
    u32 ctrl_reg, primask;

    ctrl_reg = AIC_READ32(MPU_CTRL);
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
    AIC_WRITE32(MPU_CTRL, ctrl.w);
    __DSB();
    __ISB();
    __set_PRIMASK(primask);
}

/* Disable MPU control */
void mpu_disable(void)
{
    u32 ctrl_reg, primask;
    MPU_CTRL_T ctrl;

    ctrl_reg = AIC_READ32(MPU_CTRL);
    if (!(ctrl_reg & MPU_EN))
        return;

    ctrl.b.enable = 0x0;
    primask = __get_PRIMASK();
    if (!primask)
        __set_PRIMASK(0x1);
    dcache_invcc_all();
    icache_inv_all();
    AIC_WRITE32(MPU_CTRL, ctrl.w);
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
static void mpu_setup_region(u32 rg_num, const MPU_REGION_T *rg, MPU_CONFIG_T type)
{
    MPU_RBAR_T rbar;
    MPU_RLAR_T rlar;
    MPU_RNR_T rnr;

    /* CONF_APPEND_RDONLY type, only config RDONLY region */
    if (type == CONF_APPEND_RDONLY)
    {
        if (rg->ap != RDONLY)
            return;
    }

    /* No shareable */
    rbar.b.sh = 0x0;
    rbar.b.base = rg->base_addr >> 5;
    rnr.b.region = rg_num;
    rlar.b.limit = rg->end_addr >> 5;
    rlar.b.pxn = 0x0;

    switch (rg->ap) {
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

    /* CONF_EXCLUDE_RDONLY type, change RDONLY to RDWR. */
    if (type == CONF_EXCLUDE_RDONLY)
    {
        if (rg->ap == RDONLY)
           rbar.b.ap = MPU_AP_RDWR;
    }

    switch (rg->cbtex) {
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

    switch (rg->xn) {
    case NO_EXEC:
        rbar.b.xn = MPU_NO_EXEC;
        break;
    default:
        rbar.b.xn = 0x0;
        break;
    };

    if (rg->rg_stat == RG_DISABLE)
        rlar.b.en = 0;
    else
        rlar.b.en = MPU_RG_EN;

    AIC_WRITE32(MPU_RNR, rnr.w);
    AIC_WRITE32(MPU_RBAR_A0, rbar.w);
    AIC_WRITE32(MPU_RLAR_A0, rlar.w);
}

/*
 * Note: the function used for disable a mpu region.
 */
static void mpu_disable_region(u32 rg_num)
{
    MPU_RLAR_T rlar = {0};
    MPU_RNR_T rnr;

    rnr.b.region = rg_num;
    AIC_WRITE32(MPU_RNR, rnr.w);

    rlar.w = AIC_READ32(MPU_RLAR_A0);
    rlar.b.en = 0;
    AIC_WRITE32(MPU_RLAR_A0, rlar.w);
}

/*
 * Note: the function used for enable a mpu region.
 */
static void mpu_enable_region(u32 rg_num)
{
    MPU_RLAR_T rlar = {0};
    MPU_RNR_T rnr;

    rnr.b.region = rg_num;
    AIC_WRITE32(MPU_RNR, rnr.w);

    rlar.w = AIC_READ32(MPU_RLAR_A0);
    rlar.b.en = MPU_RG_EN;
    AIC_WRITE32(MPU_RLAR_A0, rlar.w);
}

/*
 * Note: Set all regions and enable mpu
 */
static void mpu_config(MPU_CONFIG_T type)
{
    int i;
    u32 regions_cnt;
    AIC_INT_SAVE_AREA

    mpu_initialize();
    cache_disable();
    AIC_INT_DISABLE

    mpu_disable();

    regions_cnt = s_mpu_mgr.total_region;
    for (i = 0;
         i < ARRAY_SIZE(mpu_cfg) && i < regions_cnt;
         i++)
        mpu_setup_region(i, &mpu_cfg[i], type);

    /* mark the free regions and identify the alloc base region index. */
    s_mpu_mgr.free_bits = GENMASK(i, regions_cnt);
    s_mpu_mgr.alloc_base = i;

    mpu_enable();
    AIC_INT_RESTORE
    cache_enable();
}

/*
 * Note: find the mpu region num of the specified area .
 */
int mpu_find_region_by_area(MPU_REGION_AREA_T *rg_area)
{
    const MPU_REGION_T *rg = &mpu_cfg[0];
    int rg_num;

    for (rg_num = 0; rg_num < ARRAY_SIZE(mpu_cfg); rg_num++, rg++) {
        if (rg->base_addr == rg_area->base_addr &&
            rg->end_addr == rg_area->end_addr) {
            return rg_num;
        }
    }

    return -1;
}

/*
 * Note: Before load image call it for
 * Set all regions and enable mpu, exclude readonly ap for load image.
 */
void mpu_config_before_load_image(void)
{
    mpu_config(CONF_EXCLUDE_RDONLY);
}

/*
 * Note: After load image call it for
 * append the readonly ap for protect ro regions.
 */
void mpu_config_after_load_image(void)
{
    mpu_config(CONF_APPEND_RDONLY);
}

/*
 * Note: Set all regions and enable mpu
 */
void mpu_config_normal(void)
{
    mpu_config(CONF_NORMAL);
}

/*
 * mpu_dynamic_config_region: Config a mpu region with specified region config.
 *
 * @rg: the pointer of the specified region config.

 * @return: < 0 -- failed; >=0 -- success, return the region index.
 *
 * This funtin will try to get a free region to finish the mpu config,
 * If cant't get a free region will return AIC_ENODEV. If config succ
 * will return the region index to caller.
 * After the caller no need this mpu region, must call
 * the function  mpu_dynamic_unconfig_region to release the region.
 */
int mpu_dynamic_conf_region(const MPU_REGION_T *rg, bool cache_op)
{
    u32 i;
    int ret = AIC_ENODEV;
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

    for (i = s_mpu_mgr.alloc_base; i < s_mpu_mgr.total_region; i++)
        if (s_mpu_mgr.free_bits & BIT(i))
            break;

    if (i != s_mpu_mgr.total_region) {
        if (cache_op)
            cache_disable();

        mpu_setup_region(i, rg, CONF_NORMAL);
        if (cache_op)
            cache_enable();

        AIC_CLR_BIT(s_mpu_mgr.free_bits, i);
        ret = (int)i;
    }

    AIC_INT_RESTORE

    return ret;
}

/*
 * mpu_dynamic_unconf_region:Unconfig a mpu region with the region index that
 *                             returned by mpu_dynamic_conf_region function.
 *
 * @index: the index of the region that get from mpu_dynamic_conf_region func.

 * This funtin will disable and release the region.
 */
void mpu_dynamic_unconf_region(int index, bool cache_op)
{
    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    if (index >= s_mpu_mgr.alloc_base) {
        if (cache_op)
            cache_disable();

        mpu_disable_region(index);

        if (cache_op)
            cache_enable();

        AIC_SET_BIT(s_mpu_mgr.free_bits, index);
    }

    AIC_INT_RESTORE
}

/*
 * Note: the function can disable or enable a mpu region
 * by specified region area.
 *
 * return: succ, return AIC_SUCCESS, otherwise failed.
 */
int mpu_switch_region_by_area(MPU_REGION_AREA_T *rg_area, bool enable)
{
    int rg_num = mpu_find_region_by_area(rg_area);

    if (rg_num < 0)
        return AIC_EINVAL;

    if (enable)
        mpu_enable_region(rg_num);
    else
        mpu_disable_region(rg_num);

    return AIC_SUCCESS;
}

/*
 * Note: the function used reconfig a mpu region by
 * specified mpu region info.
 *
 * return: succ, return AIC_SUCCESS, otherwise failed.
 */
int mpu_reconfig_region_by_area(const MPU_REGION_T *rg)
{
    MPU_REGION_AREA_T rg_area;
    int rg_num;

    rg_area.base_addr = rg->base_addr;
    rg_area.end_addr = rg->end_addr;
    rg_num = mpu_find_region_by_area(&rg_area);

    if (rg_num < 0)
        return AIC_EINVAL;

    mpu_setup_region(rg_num, rg, CONF_NORMAL);

    return AIC_SUCCESS;
}

#define REGION_NUM    16
__attribute__ ((section(".idle_data")))
u32 g_mpu_bkarea[REGION_NUM*2] = {0};

__attribute__ ((section(".idle_text")))
/* Called when resume, as interrupt is disable at that time, it is no need to disable int again */
void __mpu_enable(void)
{
    MPU_CTRL_T ctrl;
    MPU_MAIR0_T mair0;
    MPU_MAIR1_T mair1;
    u32 ctrl_reg;

    ctrl_reg = AIC_READ32(MPU_CTRL);
    if (ctrl_reg & MPU_EN)
        return;

    mair0.b.attr0 = MPU_ATTR0_NGNRNE;
    mair0.b.attr1 = MPU_ATTR1_WTNWA;
    mair0.b.attr2 = MPU_ATTR2_WBNWA;
    mair0.b.attr3 = MPU_ATTR3_WBWA;
    mair1.b.attr4 = MPU_ATTR4_NC;
    AIC_WRITE32(MPU_MAIR0, mair0.w);
    AIC_WRITE32(MPU_MAIR1, mair1.w);
    ctrl.b.enable = MPU_EN;
    ctrl.b.hfnmiena = MPU_HFNMIENA_EN;
    ctrl.b.privdefena = 0x0;
    AIC_WRITE32(MPU_CTRL, ctrl.w);
    return;
}

/**
 * mpu_suspend() - MPU backup
 *
 * @addr: buffer address to save mpu registers.
 *
 */
void mpu_suspend(u32 *addr)
{
    u32 *st_addr;
    u32 i;

    if (addr != AIC_NULL)
        st_addr = addr;
    else
        st_addr = (u32 *)g_mpu_bkarea;

    for(i = 0; i < REGION_NUM; i++) {
        AIC_WRITE32(MPU_RNR, i);
        *st_addr++ = AIC_READ32(MPU_RBAR_A0);
        *st_addr++ = AIC_READ32(MPU_RLAR_A0);
    }
    return;
}

__attribute__ ((section(".idle_text")))
/**
 * mpu_resume() - MPU restore
 *
 * @addr: buffer address to save mpu registers.
 *
 */
void mpu_resume(u32 *addr)
{
    u32 *st_addr;
    u32 i;

    if (addr != AIC_NULL)
        st_addr = addr;
    else
        st_addr = (u32 *)g_mpu_bkarea;

    for(i = 0;i < REGION_NUM; i++) {
        AIC_WRITE32(MPU_RNR, i);
        AIC_WRITE32(MPU_RBAR_A0, *st_addr++);
        AIC_WRITE32(MPU_RLAR_A0, *st_addr++);
    }
    __mpu_enable();
    return;
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
void __attribute__((section(".mpu_text")))
mpu_reconfig_region(u32 rg_num, const MPU_REGION_T *rg)
{
    MPU_RBAR_T rbar;
    MPU_RLAR_T rlar;
    MPU_RNR_T rnr;

    /* No shareable */
    rbar.b.sh = 0x0;
    rbar.b.base = rg->base_addr >> 5;
    rnr.b.region = rg_num;
    rlar.b.limit = rg->end_addr >> 5;
    rlar.b.pxn = 0x0;

    switch (rg->ap) {
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

    switch (rg->cbtex) {
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

    switch (rg->xn) {
    case NO_EXEC:
        rbar.b.xn = MPU_NO_EXEC;
        break;
    default:
        rbar.b.xn = 0x0;
        break;
    };

    if (rg->rg_stat == RG_DISABLE)
        rlar.b.en = 0;
    else
        rlar.b.en = MPU_RG_EN;

    AIC_WRITE32(MPU_RNR, rnr.w);
    AIC_WRITE32(MPU_RBAR_A0, rbar.w);
    AIC_WRITE32(MPU_RLAR_A0, rlar.w);
}

MPU_REGION_T* __attribute__((section(".mpu_text")))
mpu_get_multi_cs_region_table(u8 cs)
{
    return mpu_cfg_cs[cs];
}

int __attribute__((section(".mpu_text"))) mpu_get_multi_cs_region_num(u8 cs)
{
    return ARRAY_SIZE(mpu_cfg_cs[cs]);
}
#else
void __attribute__((section(".mpu_text")))
mpu_reconfig_region(u32 rg_num, const MPU_REGION_T *rg)
{
}

MPU_REGION_T* __attribute__((section(".mpu_text")))
mpu_get_multi_cs_region_table(u8 cs)
{
    return NULL;
}

int __attribute__((section(".mpu_text"))) mpu_get_multi_cs_region_num(u8 cs)
{
    return 0;
}
#endif
