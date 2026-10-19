/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file make statment of normal cache operations
 *
 */

#ifndef __CACHE_H__
#define __CACHE_H__

#include <io.h>

/*
 * Now, both the dcache cacheline size of cr5 or cmstar
 * and the icache cacheline size of the of cr5 or cmstar are 32 byte.
 */
#define CACHE_LINE_SIZE         32U

/* Get data cache line size in bytes*/
#define get_dcache_linesize()   CACHE_LINE_SIZE

/* Get instruction cache line size in bytes */
#define get_icache_linesize()   CACHE_LINE_SIZE

/* Use this macro to makesure the addr of a global val is cacheline aligned. */
#define CACHE_LINE_ALIGN __attribute__ ((aligned(CACHE_LINE_SIZE)))

/* Use this macro to check both the addr and the size are cacheline aligned.*/
#define CHECK_CACHELINE_ALIGNED(addr, size) do {\
    AIC_ASSERT(IS_ALIGNED(addr, CACHE_LINE_SIZE) && \
               IS_ALIGNED(size, CACHE_LINE_SIZE)); \
    } while (0)

/* Use this macro to see if the specified value is cacheline aligned.*/
#define IS_CACHELINE_ALIGNED(value) IS_ALIGNED(value, CACHE_LINE_SIZE)

/* align to dcache line */
#define DCLINE_ALIGN(addr)       ROUND_DOWN(addr, get_dcache_linesize())

/* integer multiplies of dcache line */
#define DCLINE_GRANULARITY(addr, len) \
    ROUND_UP(len + ((unsigned long)addr - DCLINE_ALIGN(addr)), get_dcache_linesize())

void cache_enable(void);

void cache_disable(void);

void cache_invcc_all(void);

#ifdef CONFIG_BL_CPU_CACHE
void icache_enable(void);

void icache_disable(void);

/* The addr isn't require cache line alignment */
void icache_inv_by_range(u32 addr, size_t length);

void icache_inv_all(void);
#else /* CONFIG_BL_CPU_CACHE */
static inline void icache_enable(void)
{
}

static inline void icache_disable(void)
{
}

/* The addr isn't require cache line alignment */
static inline void icache_inv_by_range(u32 addr, size_t length)
{
}

static inline void icache_inv_all(void)
{
}
#endif /* CONFIG_BL_CPU_CACHE */

#ifdef CONFIG_BL_CPU_CACHE
void dcache_enable(void);

void dcache_disable(void);

void dcache_clean_all(void);

void dcache_invcc_all(void);

/* The addr isn't require cache line alignment */
void dcache_inv_by_range(u32 addr, size_t length);

/* The addr isn't require cache line alignment */
void dcache_clean_by_range(u32 addr, size_t length);

/* The addr isn't require cache line alignment */
void dcache_invcc_by_range(u32 addr, size_t length);
#else /* CONFIG_BL_CPU_CACHE */
static inline void  dcache_enable(void)
{
}

static inline void dcache_disable(void)
{
}

static inline void dcache_clean_all(void)
{
}
static inline void dcache_invcc_all(void)
{
}

/* The addr isn't require cache line alignment */
static inline  void dcache_inv_by_range(u32 addr, size_t length)
{
}

/* The addr isn't require cache line alignment */
static inline  void dcache_clean_by_range(u32 addr, size_t length)
{
}

/* The addr isn't require cache line alignment */
static inline void dcache_invcc_by_range(u32 addr, size_t length)
{
}
#endif /* CPU_ICACHE_ENBALE */

#endif /* __CACHE_H__ */
