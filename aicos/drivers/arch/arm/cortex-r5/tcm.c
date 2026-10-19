/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines TCM(Tightly Coupled Memory) operations interface for
 *    cortex-r5
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x053dac
#define LOG_MODULE    AMOD_DRV

#include <aic_assert.h>
#include <aic_io.h>
#include <aic_reg_base.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_errno.h>
#include <core_op.h>
#include <cache.h>
#include <dma.h>
#include <mpu.h>
#include <aic_osa.h>
#include <timer_inc.h>

/*
 * CONFIG_USE_CPU_TO_CPY
 * when dma driver is not ready, temp use cpu copy.
 * for the relase version must disable it.
 */
#define CONFIG_USE_CPU_TO_CPY   0

//#define TCM_DEBUG
#define TCM_EN_MASK     (0x1U)

typedef enum {
    ATCM_TYPE = 0,
    BTCM_TYPE,
    TCM_CNT
 } tcm_type_t;

#define ATCM_RAM_BASE   REG_ATCM0_BASE
#define BTCM_RAM_BASE   REG_BTCM0_BASE

#ifdef CONFIG_TCM_ROM_DEBUG_AS_RAM
#define ATCM_RAM_SIZE   REG_ATCM_SIZE
#define BTCM_RAM_SIZE   REG_BTCM_SIZE
#else
#define ATCM_RAM_SIZE   REG_ATCM0_SIZE
#define BTCM_RAM_SIZE   REG_BTCM0_SIZE
#endif

#define ATCM_BASE       REG_ATCM0_BASE
#define BTCM_BASE       REG_BTCM0_BASE
#define ATCM_SIZE       REG_ATCM_SIZE
#define BTCM_SIZE       REG_BTCM_SIZE

#ifdef TCM_DEBUG
#define TCM_LOG_NUM  10
u32 g_dbgtcm[TCM_LOG_NUM] = {0};
u32 g_dbgtcm_cnt = 0;
#endif

#if IS_CONF_ENABLED(CONFIG_USE_CPU_TO_CPY)
void copy_tcm_code(void *dst, void *src, size_t size)
{
    u32 left = size;
    u32 *u32_dst = (u32 *)dst;
    u32 *u32_src = (u32 *)src;
    u8 *u8_dst;
    u8 *u8_src;

    /* can't use memcpy here, the memcpy may place to tcm, not ready yet. */
    while (left >= sizeof(u32)) {
        *u32_dst++ = *u32_src++;
        left -= sizeof(u32);
    }

    u8_dst = (u8 *)u32_dst;
    u8_src = (u8 *)u32_src;

    while (left) {
        *u8_dst++ = *u8_src++;
        left -= sizeof(u8);
    }
}
#else
#define copy_tcm_code(d, s, sz)   aic_dma_bare_cpy((u32)(d), (u32)(s), (u32)(sz))
#endif

#ifndef CONFIG_TCM_ENABLE
void tcm_disable(void)
{
    u32 atcm_reg, btcm_reg;

    __asm __volatile__("mrc p15, 0, %0, c9, c1, 0":"=r"(btcm_reg));
    __asm __volatile__("mrc p15, 0, %0, c9, c1, 1":"=r"(atcm_reg));

   if (atcm_reg & TCM_EN_MASK) {
       atcm_reg &= ~TCM_EN_MASK;
       __asm __volatile__("mcr p15, 0, %0, c9, c1, 1"::"r"(atcm_reg));
   }
   if (btcm_reg & TCM_EN_MASK) {
       btcm_reg &= ~TCM_EN_MASK;
       __asm __volatile__("mcr p15, 0, %0, c9, c1, 0"::"r"(btcm_reg));
   }
   __DSB();
   __ISB();
}

s32 tcm_preload_enable(void)
{
    u32 atcm_reg, btcm_reg;

    tcm_disable();

    atcm_reg = ATCM_BASE | TCM_EN_MASK;
    __asm __volatile__("mcr p15, 0, %0, c9, c1, 1"::"r"(atcm_reg));
    btcm_reg = BTCM_BASE | TCM_EN_MASK;
    __asm __volatile__("mcr p15, 0, %0, c9, c1, 0"::"r"(btcm_reg));
    __DSB();
    __ISB();

    return AIC_SUCCESS;
}
#else
#if defined(__CC_ARM) /* Arm compiler 5 */

s32 tcm_enable(bool en)
{
    u32 atcm_reg, btcm_reg;
    AIC_INT_SAVE_AREA

    __asm __volatile__("mrc p15, 0, btcm_reg, c9, c1, 0");
    __asm __volatile__("mrc p15, 0, atcm_reg, c9, c1, 1");
    AIC_INT_DISABLE

    if (en) {
        if (!(atcm_reg & TCM_EN_MASK)) {
            atcm_reg = ATCM_BASE | TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, atcm_reg, c9, c1, 1");
        }
        if (!(btcm_reg & TCM_EN_MASK)) {
            btcm_reg = BTCM_BASE | TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, btcm_reg, c9, c1, 0");
        }
    } else {
        if (atcm_reg & TCM_EN_MASK) {
            atcm_reg &= ~TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, atcm_reg, c9, c1, 1");
        }
        if (btcm_reg & TCM_EN_MASK) {
            btcm_reg &= ~TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, btcm_reg, c9, c1, 0");
        }
    }
    AIC_INT_RESTORE
}
#elif defined(__clang__) /* AC6 */

extern u32 Image$$TCM$$Base;
extern u32 Image$$TCM$$Limit;
extern u32 Image$$TCM$$Length;

void tcm_disable(void);

#define __CACHE_LINE_SIZE  32U
s32 tcm_preload_enable(void)
{
    u32 atcm_reg, btcm_reg;
    int region;
#ifdef TCM_DEBUG
    u32 ticks;
#endif
    AIC_INT_SAVE_AREA

    tcm_disable();
    AIC_INT_DISABLE
#ifdef TCM_DEBUG
    ticks = aic_get_tick();
#endif
    if ((u32)&Image$$TCM$$Limit > (u32)&Image$$TCM$$Base) {
        if ((u32)&Image$$TCM$$Length <= ATCM_RAM_SIZE) {
            copy_tcm_code((void *)ATCM_RAM_BASE, (void *)&Image$$TCM$$Base, (u32)&Image$$TCM$$Length);
            /* invalidate icache before enable tcm */
            icache_inv_by_range((u32)&Image$$TCM$$Base, ATCM_SIZE);
            atcm_reg = (u32)&Image$$TCM$$Base | TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, %0, c9, c1, 1"::"r"(atcm_reg));
        } else {
            copy_tcm_code((void *)ATCM_RAM_BASE, (void *)&Image$$TCM$$Base, ATCM_RAM_SIZE);
            copy_tcm_code((void *)BTCM_RAM_BASE, (void *)((u32)&Image$$TCM$$Base + ATCM_RAM_SIZE), (u32)&Image$$TCM$$Length - ATCM_RAM_SIZE);
            /* invalidate icache before enable tcm */
            icache_inv_by_range((u32)&Image$$TCM$$Base, ATCM_SIZE + BTCM_SIZE);
            atcm_reg = (u32)&Image$$TCM$$Base | TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, %0, c9, c1, 1"::"r"(atcm_reg));
            btcm_reg = ((u32)&Image$$TCM$$Base + ATCM_SIZE) | TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, %0, c9, c1, 0"::"r"(btcm_reg));
        }
       __DSB();
       __ISB();
    }
#ifdef TCM_DEBUG
    if (g_dbgtcm_cnt >= TCM_LOG_NUM)
        g_dbgtcm_cnt = 0;
    g_dbgtcm[g_dbgtcm_cnt++] = aic_get_tick() - ticks;
#endif
   AIC_INT_RESTORE
   return AIC_SUCCESS;
}

void tcm_disable(void)
{
    u32 atcm_reg, btcm_reg;
    AIC_INT_SAVE_AREA

    __asm __volatile__("mrc p15, 0, %0, c9, c1, 0":"=r"(btcm_reg));
    __asm __volatile__("mrc p15, 0, %0, c9, c1, 1":"=r"(atcm_reg));
    AIC_INT_DISABLE

   if (atcm_reg & TCM_EN_MASK) {
       atcm_reg &= ~TCM_EN_MASK;
       __asm __volatile__("mcr p15, 0, %0, c9, c1, 1"::"r"(atcm_reg));
   }
   if (btcm_reg & TCM_EN_MASK) {
       btcm_reg &= ~TCM_EN_MASK;
       __asm __volatile__("mcr p15, 0, %0, c9, c1, 0"::"r"(btcm_reg));
   }
   __DSB();
   __ISB();
   AIC_INT_RESTORE
}

#elif defined(__GNUC__) /* Normal GCC */
extern const u32 __tcm_start__;
extern const u32 __tcm_end__;

#ifdef CONFIG_TCM_MAP_NEQ_LOAD
/* the map region and the load region are not equal. */
extern const u32 __atcm_load_start__;
extern const u32 __btcm_load_end__;

#define TCM_LOAD_START      ((u32)&__atcm_load_start__)
#define TCM_LOAD_END        ((u32)&__btcm_load_end__)
#define TCM_MAP_START       ((u32)&__tcm_start__)
#define TCM_MAP_END         ((u32)&__tcm_end__)
#else
/* the map region and the load region are equal. */
#define TCM_LOAD_START      ((u32)&__tcm_start__)
#define TCM_LOAD_END        ((u32)&__tcm_end__)
#define TCM_MAP_START       ((u32)&__tcm_start__)
#define TCM_MAP_END         ((u32)&__tcm_end__)
#endif

void tcm_disable(void);

#define __CACHE_LINE_SIZE  32U
s32 tcm_preload_enable(void)
{
    u32 atcm_reg, btcm_reg;
    u32 tcm_length;
#ifdef TCM_DEBUG
    u32 ticks;
#endif

    AIC_INT_SAVE_AREA
    tcm_disable();
    AIC_INT_DISABLE
#ifdef TCM_DEBUG
    ticks = aic_get_tick();
#endif

    if (TCM_LOAD_END > TCM_LOAD_START) {
        tcm_length = TCM_LOAD_END - TCM_LOAD_START;
        if (tcm_length <= ATCM_RAM_SIZE) {
            copy_tcm_code((void *)ATCM_RAM_BASE, (void *)TCM_LOAD_START, tcm_length);
            icache_inv_by_range(TCM_MAP_START, ATCM_SIZE);
            atcm_reg = TCM_MAP_START | TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, %0, c9, c1, 1"::"r"(atcm_reg));
        } else {
            copy_tcm_code((void *)ATCM_RAM_BASE, (void *)TCM_LOAD_START, ATCM_RAM_SIZE);
            copy_tcm_code((void *)BTCM_RAM_BASE, (void *)(TCM_LOAD_START + ATCM_RAM_SIZE), tcm_length - ATCM_RAM_SIZE);
            icache_inv_by_range(TCM_MAP_START, ATCM_SIZE + BTCM_SIZE);
            atcm_reg = TCM_MAP_START | TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, %0, c9, c1, 1"::"r"(atcm_reg));
            btcm_reg = (TCM_MAP_START + ATCM_SIZE) | TCM_EN_MASK;
            __asm __volatile__("mcr p15, 0, %0, c9, c1, 0"::"r"(btcm_reg));
        }
        __DSB();
        __ISB();
    }
#ifdef TCM_DEBUG
    if (g_dbgtcm_cnt >= TCM_LOG_NUM)
        g_dbgtcm_cnt = 0;
    g_dbgtcm[g_dbgtcm_cnt++] = aic_get_tick() - ticks;
#endif
   AIC_INT_RESTORE
   return AIC_SUCCESS;
}

void tcm_disable(void)
{
    u32 atcm_reg, btcm_reg;
    AIC_INT_SAVE_AREA

    __asm __volatile__("mrc p15, 0, %0, c9, c1, 0":"=r"(btcm_reg));
    __asm __volatile__("mrc p15, 0, %0, c9, c1, 1":"=r"(atcm_reg));
    AIC_INT_DISABLE

   if (atcm_reg & TCM_EN_MASK) {
       atcm_reg &= ~TCM_EN_MASK;
       __asm __volatile__("mcr p15, 0, %0, c9, c1, 1"::"r"(atcm_reg));
   }
   if (btcm_reg & TCM_EN_MASK) {
       btcm_reg &= ~TCM_EN_MASK;
       __asm __volatile__("mcr p15, 0, %0, c9, c1, 0"::"r"(btcm_reg));
   }
   __DSB();
   __ISB();
   AIC_INT_RESTORE
}

#else
    #error Unknown compiler
#endif
#endif /* CONFIG_TCM_ENABLE */

