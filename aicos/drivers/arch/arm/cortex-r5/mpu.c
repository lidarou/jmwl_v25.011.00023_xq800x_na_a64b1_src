/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines MPU(Memory Protecting Unit) operations interface for
 *    cortex-r5
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16d1ef
#define LOG_MODULE    AMOD_DRV

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_size.h>
#include <cache.h>
#include <mpu.h>
#include <mpu_map.h>
#include <core_op.h>
#include "mpu_cr5.h"

static mpu_mgr_t s_mpu_mgr;

/* Enalbe MPU control */
void mpu_enable(void)
{
    u32 sctlr;
    AIC_INT_SAVE_AREA

    sctlr = get_sctlr();
    if (sctlr & MPU_CTRL_BIT)
        return;
    sctlr |= MPU_CTRL_BIT;
    AIC_INT_DISABLE
    set_sctlr(sctlr);
    __DSB();
    __ISB();
    AIC_INT_RESTORE
}

/* Disable MPU control */
void mpu_disable(void)
{
    u32 sctlr;
    AIC_INT_SAVE_AREA

    sctlr = get_sctlr();
    if (!(sctlr & MPU_CTRL_BIT))
        return;
    sctlr &= ~MPU_CTRL_BIT;
    AIC_INT_DISABLE
    cache_invcc_all();
    set_sctlr(sctlr);
    __DSB();
    __ISB();
    AIC_INT_RESTORE
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
    MPU_RGNR_T rgnr;
    MPU_DRBAR_T drbar;
    MPU_DRSR_T drsr;
    MPU_RACR_T racr;
    u64 length, size;
    u32 rg_size;

    /* CONF_APPEND_RDONLY type, only config RDONLY region */
    if (type == CONF_APPEND_RDONLY)
    {
        if (rg->ap != RDONLY)
            return;
    }

    /* caculate the region size */
    length = (u64)rg->end_addr - (u64)rg->base_addr + 1;
    size = SIZE_32B;
    for (rg_size = RG_MIN_SIZE; rg_size < RG_MAX_SIZE; rg_size++) {
        if (length <= size) {
            drsr.b.size = rg_size;
            break;
        }
        size <<= 1;
    }

    /* the addr must align with the size of the region. */
    AIC_ASSERT(IS_ALIGNED(rg->base_addr, size));

    drbar.b.base = rg->base_addr >> 2;
    rgnr.b.region = rg_num;

    switch (rg->ap) {
    case NACCESS:
        racr.b.ap = MPU_AP_NACCESS;
        break;
    case RDONLY:
        racr.b.ap = MPU_AP_RDONLY;
        break;
    case RDWR:
        racr.b.ap = MPU_AP_RDWR;
        break;
    default:
        return;
    };

    /* CONF_EXCLUDE_RDONLY type, change RDONLY to RDWR. */
    if (type == CONF_EXCLUDE_RDONLY)
    {
        if (rg->ap == RDONLY)
            racr.b.ap = MPU_AP_RDWR;
    }

    switch (rg->cbtex) {
    case DEV_NGNRNE:
        racr.b.attrs = MPU_DEV_NGNRNE;
        break;
    case DEV_SH:
        racr.b.attrs = MPU_DEV_SH;
        break;
    case DEV_NSH:
        racr.b.attrs = MPU_DEV_NSH;
        break;
    case MEM_NC:
        racr.b.attrs = MPU_MEM_NC;
        break;
    case MEM_WTNWA:
        racr.b.attrs = MPU_MEM_WTNWA;
        break;
    case MEM_WBNWA:
        racr.b.attrs = MPU_MEM_WBNWA;
        break;
    case MEM_WBWA:
        racr.b.attrs = MPU_MEM_WBWA;
        break;
    default:
        return;
    };

    switch (rg->xn) {
    case NO_EXEC:
        racr.b.xn = MPU_NO_EXEC;
        break;
    default:
        racr.b.xn = 0x0;
        break;
    };

    if (rg->rg_stat == RG_DISABLE) {
        drsr.b.en = 0x0;
        drsr.b.srd = 0xFF;
    } else {
        drsr.b.en = MPU_RG_EN;
        drsr.b.srd = rg->rg_stat;
    }

    set_rgnr(rgnr.w);
    set_rbar(drbar.w);
    set_racr(racr.w);
    set_drsr(drsr.w);
    __ISB();
    __DSB();
}

/*
 * Note: the function used for disable a mpu region.
 */
static void mpu_enable_region(u32 rg_num)
{
    MPU_DRSR_T drsr = {0};
    MPU_RGNR_T rgnr;

    rgnr.b.region = rg_num;
    set_rgnr(rgnr.w);

    drsr.w = get_drsr();
    drsr.b.en = MPU_RG_EN;
    set_drsr(drsr.w);

    __ISB();
    __DSB();
}

/*
 * Note: the function used for disable a mpu region.
 */
static void mpu_disable_region(u32 rg_num)
{
    MPU_DRSR_T drsr = {0};
    MPU_RGNR_T rgnr;

    rgnr.b.region = rg_num;
    set_rgnr(rgnr.w);

    drsr.w = get_drsr();
    drsr.b.en = 0x0;
    set_drsr(drsr.w);

    __ISB();
    __DSB();
}

/*
 * Note: Set all regions and enable mpu
 */
static void mpu_config(MPU_CONFIG_T type)
{
    int i;
    u32 regions_cnt;
    MPU_IR_T type_reg;

    type_reg.w = get_mpuir();
    regions_cnt = type_reg.b.dregion;
    s_mpu_mgr.total_region = regions_cnt;

    AIC_INT_SAVE_AREA

    cache_disable();
    AIC_INT_DISABLE

    /* Reserve region 0 config for run code, start form region 1. */
    for (i = 1;
         i < ARRAY_SIZE(mpu_cfg) && i < regions_cnt;
         i++)
        mpu_setup_region(i, &mpu_cfg[i], type);

    /*
     * After all region enable, update the region 0 to cover
     * the default init config.
     */
    mpu_setup_region(0, &mpu_cfg[0], type);

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
 * mpu_enable_region: Enable a mpu region with specified region config.
 *
 * @rg: the pointer of the specified region config.

 * @return: < 0 -- failed; >=0 -- success, return the region index.
 *
 * This funtin will try to get a free region to finish the mpu config,
 * If cant't get a free region will return AIC_ENODEV. If config succ
 * will return the region index to caller.
 * After the caller no need this mpu region, must call
 * the function  mpu_disable_region to release the region.
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
 * mpu_disable_region: Enable a mpu region with specified region index.
 *
 * @index: the index of the region.

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
u32 g_mpu_bkarea[REGION_NUM*3] = {0};

__attribute__ ((section(".idle_text")))
/* Called when resume, as interrupt is disable at that time, it is no need to disable int again */
void __mpu_enable(void)
{
    u32 sctlr;

    sctlr = get_sctlr();
    if (sctlr & MPU_CTRL_BIT)
        return;
    sctlr |= MPU_CTRL_BIT;
    set_sctlr(sctlr);
    __DSB();
    __ISB();
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

    for(i = 0;i < REGION_NUM; i++) {
        set_rgnr(i);
        *st_addr++ = get_rbar();
        *st_addr++ = get_racr();
        *st_addr++ = get_drsr();
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
        set_rgnr(i);
        set_rbar(*st_addr++);
        set_racr(*st_addr++);
        set_drsr(*st_addr++);
        __DSB();
        __ISB();
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
