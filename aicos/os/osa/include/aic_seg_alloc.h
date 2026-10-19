/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This header file of  aic segment allocate module.
 */

#ifndef __AIC_SEG_ALLOC_H__
#define __AIC_SEG_ALLOC_H__

#include <aic_type.h>
#include <aic_list.h>
/**
 *  General purpose special memory pool descriptor.
 */
struct aic_pool {
    /* list of segments in this pool */
    struct list_head segments;
    /* the alloc list */
    struct list_head alloc_list;
    /* minimum allocation order */
    int min_alloc_order;
};

/*
 *  General purpose special memory pool segment descriptor.
 */
struct aic_segment {
    /* next segment in pool */
    struct list_head next_segment;
    /* start address of memory segment */
    addr_t start_addr;
    /* end address of memory segment (inclusive) */
    addr_t end_addr;
    /* availble size */
    unsigned long avail;
    /* valid bits cnt */
    unsigned long bits_size;
     /* bitmap for allocating memory segment */
    unsigned long *bitmap;
};

/**
 * aic_pool_create - create a new special memory pool.
 *
 * @min_alloc_order: log base 2 of number of bytes each bitmap bit represents.
 *
 * Returns the pool instance or a null pointer on failure.
 */
struct aic_pool *aic_pool_create(int min_alloc_order);

/**
 * aic_pool_add - add a new segment to the pool.
 *
 * @pool: the pool instance.
 * @addr: the starting address of memory segment to add to pool.
 * @size: size in bytes of the memory segment to add to pool.

 * Add a new segment of special memory to the specified pool.
 *
 * Returns 0 on success or an errno on failure.
 */
int aic_pool_add(struct aic_pool *pool, addr_t addr, size_t size);

void aic_pool_destroy(struct aic_pool *pool);
unsigned long aic_pool_alloc(struct aic_pool *pool, size_t size);
void aic_pool_free(struct aic_pool *pool, addr_t addr);
size_t aic_pool_avail(struct aic_pool *);
size_t aic_pool_size(struct aic_pool *);

#endif /* __AIC_SEG_ALLOC_H__ */

