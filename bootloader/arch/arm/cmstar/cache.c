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

#include <io.h>
#include <cmsis/core_star.h>

void dcache_clean_all(void)
{
    SCB_CleanDCache();
}

void dcache_invcc_all(void)
{
    SCB_CleanInvalidateDCache();
}

void dcache_inv_by_range(u32 addr, size_t length)
{
    SCB_InvalidateDCache_by_Addr((void*)addr, length);
}

void dcache_clean_by_range(u32 addr, size_t length)
{
    SCB_CleanDCache_by_Addr((uint32_t *)addr, length);
}

void dcache_invcc_by_range(u32 addr, size_t length)
{
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)addr, length);
}

void icache_inv_by_range(u32 addr, size_t length)
{
    SCB_InvalidateICache_by_Addr ((void *)addr, length);
}

void icache_enable(void)
{
    SCB_EnableICache ();
}

void icache_disable(void)
{
    SCB_DisableICache();
}

void icache_inv_all(void)
{
    SCB_InvalidateICache();
}

void dcache_enable(void)
{
    SCB_EnableDCache();
}

void dcache_disable(void)
{
    SCB_DisableDCache();
}

void cache_enable(void)
{
    icache_enable();
    dcache_enable();
}

void cache_disable(void)
{
    icache_disable();
    dcache_disable();
}
