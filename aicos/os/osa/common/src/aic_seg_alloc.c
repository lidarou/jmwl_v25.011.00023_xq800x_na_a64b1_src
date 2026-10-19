/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file realize the aic segment allocate function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09577c
#define LOG_MODULE    AMOD_AOS

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_math.h>
#include <aic_size.h>
#include <aic_seg_alloc.h>
#include <aic_osa.h>
#include <stdlib.h>

struct aic_seg_alloc {
    addr_t addr;
    size_t size;

    /* next alloc in pool */
    struct list_head next_alloc;
};

static inline size_t segment_size(const struct aic_segment *segment)
{
    return segment->end_addr - segment->start_addr + 1;
}

/* aic_pool_find_free -find the minimum continuous free space.
 * @bitmap: the current bitmap.
 * @end_bit: the end bit.
 * @nbits: continuous bit number.
 *
 * return the start bits.
 */
static inline int aic_pool_find_free(unsigned long *bitmap,
                                     unsigned long size,
                                     unsigned long nbits)
{
    return aic_bitmap_find_minimum_z_area(bitmap, size, 0, nbits);
}

/**
 * aic_pool_create - create a new special memory pool
 * @min_alloc_order: log base 2 of number of bytes each bitmap bit represents
 *
 * Create a new special memory pool that can be used to manage special purpose
 * memory not managed by the malloc/free interface.
 */
struct aic_pool *aic_pool_create(int min_alloc_order)
{
    struct aic_pool *pool;

    pool = aos_malloc(sizeof(*pool));
    AIC_DUMMY_ASSERT(pool);
    list_head_init(&pool->segments);
    list_head_init(&pool->alloc_list);
    pool->min_alloc_order = min_alloc_order;

    return pool;
}

/**
 * aic_pool_add - add a new segment of special memory to the pool
 * @pool: pool to add new memory segment to
 * @addr:  starting address of memory segment to add to pool
 * @size: size in bytes of the memory segment to add to pool.
 *
 * Add a new segment of special memory to the specified pool.
 *
 * Returns 0 on success or a an errno on failure.
 */
int aic_pool_add(struct aic_pool *pool, addr_t addr, size_t size)
{
    struct aic_segment *segment;
    unsigned int nbits, nbytes;

    nbits = size >> pool->min_alloc_order;
    nbytes = BITS_TO_LONGS(nbits) * sizeof(long);

    segment = aos_malloc(sizeof(*segment) + nbytes);
    AIC_DUMMY_ASSERT(segment);

    segment->start_addr = addr;
    segment->end_addr = addr + size - 1;
    segment->avail = size;
    segment->bits_size = nbits;
    segment->bitmap = (unsigned long *)((addr_t)segment + sizeof(*segment));
    /* init bitmap to 0, all free status. */
    memset(segment->bitmap, 0, nbytes);

    list_add(&segment->next_segment, &pool->segments);

    return AIC_SUCCESS;
}

/**
 * aic_pool_destroy - destroy a special memory pool
 * @pool: pool to destroy
 *
 * Destroy the specified special memory pool. Verifies that there are no
 * outstanding allocations.
 */
void aic_pool_destroy(struct aic_pool *pool)
{
    struct list_head *pos, *next;
    struct aic_segment *segment;
    struct aic_seg_alloc *alloc;
    unsigned nbytes;

    /* free alloc list */
    list_for_each_safe(pos, next, &pool->alloc_list) {
        alloc = list_entry(pos, struct aic_seg_alloc, next_alloc);
        list_del(&alloc->next_alloc);
        aos_free(alloc);
    }

    /* free pool list */
    list_for_each_safe(pos, next, &pool->segments) {
        segment = list_entry(pos, struct aic_segment, next_segment);
        list_del(&segment->next_segment);
        nbytes = BITS_TO_LONGS(segment->bits_size) * sizeof(long);
        aos_free(segment);
    }
    aos_free(pool);
}

/**
 * aic_pool_alloc - allocate special memory from the pool
 * @pool: pool to allocate from
 * @size: number of bytes to allocate from the pool
 *
 * Allocate the requested number of bytes from the specified pool.
 */
addr_t aic_pool_alloc(struct aic_pool *pool, size_t size)
{
    struct aic_segment *segment;
    struct aic_seg_alloc *alloc;
    unsigned long nbits, start_bit;
    int order = pool->min_alloc_order;
    addr_t addr = 0;

    if (size == 0)
        return 0;

    nbits = (size + (1UL << order) - 1) >> order;
    list_for_each_entry(segment, &pool->segments, next_segment,
                        struct aic_segment) {
        if (size > segment->avail)
            continue;

        /* find the Minimum continuous free space */
        start_bit = aic_pool_find_free(segment->bitmap, segment->bits_size,
                                       nbits);
        if (start_bit >= segment->bits_size)
            continue;

        /* mask used bits */
        aic_bitmap_set(segment->bitmap, segment->bits_size, start_bit, nbits);

        /* calculate addr.*/
        addr = segment->start_addr + ((unsigned long)start_bit << order);
        segment->avail -= (nbits << order);

        /* add to alloc list */
        alloc = aos_malloc(sizeof(*alloc));
        AIC_DUMMY_ASSERT(alloc);

        alloc->addr = addr;
        alloc->size = size;
        list_add(&alloc->next_alloc, &pool->alloc_list);
        break;
    }

    return addr;
}

/**
 * aic_pool_free - free allocated special memory back to the pool
 * @pool: pool to free to
 * @addr: starting address of memory to free back to pool
 * @size: size in bytes of memory to free
 *
 * Free previously allocated special memory back to the specified.
 */
void aic_pool_free(struct aic_pool *pool, addr_t addr)
{
    struct aic_segment *segment;
    int order = pool->min_alloc_order;
    int start_bit, nbits;
    struct list_head *pos, *next;
    struct aic_seg_alloc *alloc;
    size_t size = 0;

    /* find the addr, and remove from alloc list */
    list_for_each_safe(pos, next, &pool->alloc_list) {
        alloc = list_entry(pos, struct aic_seg_alloc, next_alloc);
        if (alloc->addr == addr) {
            size = alloc->size;
            list_del(&alloc->next_alloc);
            aos_free(alloc);
            break;
        }
    }

    /*size can't be 0 */
    if (!size)
        return;

    /* free the addr in pool */
    nbits = (size + (1UL << order) - 1) >> order;
    list_for_each_entry(segment, &pool->segments, next_segment,
                        struct aic_segment) {
        if (addr >= segment->start_addr && addr <= segment->end_addr) {
            AIC_DUMMY_ASSERT(addr + size - 1 <= segment->end_addr);
            start_bit = (addr - segment->start_addr) >> order;
            aic_bitmap_clear(segment->bitmap, segment->bits_size,
                             start_bit, nbits);
            size = nbits << order;
            segment->avail += size;
            return;
        }
    }

    /*can't be here. */
    AIC_DUMMY_ASSERT(0);
}

/**
 * aic_pool_avail - get available free space of the pool
 * @pool: pool to get available free space
 *
 * Return available free space of the specified pool.
 */
size_t aic_pool_avail(struct aic_pool *pool)
{
    struct aic_segment *segment;
    size_t avail = 0;

    list_for_each_entry(segment, &pool->segments, next_segment,
                        struct aic_segment)
        avail += segment->avail;

    return avail;
}

/**
 * aic_pool_size - get size in bytes of memory managed by the pool
 * @pool: pool to get size
 *
 * Return size in bytes of memory managed by the pool.
 */
size_t aic_pool_size(struct aic_pool *pool)
{
    struct aic_segment *segment;
    size_t size = 0;

    list_for_each_entry(segment, &pool->segments, next_segment,
                        struct aic_segment)
        size += segment_size(segment);

    return size;
}

