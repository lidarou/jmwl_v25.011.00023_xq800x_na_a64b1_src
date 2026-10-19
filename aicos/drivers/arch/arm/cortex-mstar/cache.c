/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines cache operations interface for cortex-mstar
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1ad1fa
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_assert.h>
/* Ugly to include irq.h, as core_star need it */
#include <irq.h>
#include <core_star.h>
#include <core_op.h>
#include <cache.h>

void dcache_clean_all(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_CleanDCache();
    AIC_INT_RESTORE
}

void dcache_invcc_all(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_CleanInvalidateDCache();
    AIC_INT_RESTORE
}

void dcache_inv_by_range(u32 addr, size_t length)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    CHECK_CACHELINE_ALIGNED(addr, length);
    SCB_InvalidateDCache_by_Addr((void*)addr, length);
    AIC_INT_RESTORE
}

void dcache_clean_by_range(u32 addr, size_t length)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_CleanDCache_by_Addr((uint32_t *)addr, length);
    AIC_INT_RESTORE
}

void dcache_invcc_by_range(u32 addr, size_t length)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)addr, length);
    AIC_INT_RESTORE
}

void icache_inv_by_range(u32 addr, size_t length)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_InvalidateICache_by_Addr ((void *)addr, length);
    AIC_INT_RESTORE
}

void icache_enable(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_EnableICache ();
    AIC_INT_RESTORE
}

void icache_disable(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_DisableICache();
    AIC_INT_RESTORE
}

void icache_inv_all(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_InvalidateICache();
    AIC_INT_RESTORE
}

void dcache_enable(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_EnableDCache();
    AIC_INT_RESTORE
}

void dcache_disable(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    SCB_DisableDCache();
    AIC_INT_RESTORE
}

void cache_enable(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    icache_enable();
    dcache_enable();
    AIC_INT_RESTORE
}

void cache_disable(void)
{
    AIC_INT_SAVE_AREA
    AIC_INT_DISABLE
    icache_disable();
    dcache_disable();
    AIC_INT_RESTORE
}

__attribute__ ((section(".idle_text")))
void slp_cache_enable(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();
}

__attribute__ ((section(".idle_text")))
void slp_cache_disable(void)
{
    SCB_DisableICache();
    SCB_DisableDCache();
}
