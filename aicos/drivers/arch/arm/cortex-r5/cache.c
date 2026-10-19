/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines common cache operations interface for cortex-r5.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x033bc2
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <core_op.h>
#include <aic_assert.h>
#include <arch/arm/cortex-r5/l1c.h>
#include <cache.h>

void dcache_clean_all(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    l1dcache_clean_all();
    AIC_INT_RESTORE
}

void dcache_invcc_all(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    l1dcache_invcc_all();
    AIC_INT_RESTORE
}

void dcache_inv_by_range(u32 addr, size_t length)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    CHECK_CACHELINE_ALIGNED(addr, length);
    l1dcache_inv_by_range(addr, length);
    AIC_INT_RESTORE
}

void dcache_clean_by_range(u32 addr, size_t length)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    l1dcache_clean_by_range(addr, length);
    AIC_INT_RESTORE
}

void dcache_invcc_by_range(u32 addr, size_t length)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    l1dcache_invcc_by_range(addr, length);
    AIC_INT_RESTORE
}

void icache_inv_all(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    l1icache_inv_all();
    AIC_INT_RESTORE
}

void icache_inv_by_range(u32 addr, size_t length)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    l1icache_inv_by_range(addr, length);
    AIC_INT_RESTORE
}

void cache_invcc_all(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    dcache_invcc_all();
    l1icache_inv_all();
    AIC_INT_RESTORE
}

void cache_enable(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    l1dcache_enable();
    l1icache_enable();
    AIC_INT_RESTORE
}

void cache_disable(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    l1dcache_disable();
    l1icache_disable();
    AIC_INT_RESTORE
}

__attribute__ ((section(".idle_text")))
void slp_cache_enable(void)
{
    slp_l1c_enable();
    return;
}
