/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the amem api head file.
 */

#ifndef __AIPC_AMEM_H__
#define __AIPC_AMEM_H__

#include <aic_seg_alloc.h>

typedef struct AMEM {
    u32 smem_base;
    u32 smem_size;
    u32 avail;
    struct aic_pool *amem_pool;
} AMEM_T;

/**
 * amem_create - create amem with a pre-reserved memory area.
 *
 * @smem_base: the address of memory area.
 * @smem_size: the size of memory area.
 * @min_alloc: the mini alloc order.
 *
 * @return: amem handle, NULL failed.
 */
AMEM_T *amem_create(u32 smem_base, u32 smem_size, int min_alloc);

/**
 * amem_destroy - destroy an amem.
 *
 * @amem: amem handle.
 */
void amem_destroy(AMEM_T *amem);

/**
 * amem_malloc - alloc a memory .
 *
 * @amem: amem handle.
 * @size: the size of memory area.
 *
 * @return: the address of the memory, 0 failed.
 */
addr_t amem_malloc(AMEM_T *amem, size_t size);

/**
 * amem_free - init amem with a pre-reserved memory area.
 *
 * @addr: the address of the memory.
 *
 */
void amem_free(AMEM_T *amem, addr_t addr);

/**
 * amem_get_total - get the total size of share memory.
 *
 * @amem: amem handle.
 *
 * @return: the total size of the memory.
 */
size_t amem_get_total(AMEM_T *amem);

/**
 * amem_get_avail - get the available size of share memory.
 *
 *
 * @return: the available size of the memory.
 *
 */
size_t amem_get_avail(AMEM_T *amem);

#endif /* __AIPC_AMEM_H__ */

