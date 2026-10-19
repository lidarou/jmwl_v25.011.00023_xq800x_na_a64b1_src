/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    this file is use_no_semihosting for arm compile retarget .
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0d3013
#define LOG_MODULE    AMOD_AOS

#if defined ( __CC_ARM ) || (__ARMCC_VERSION)

#include <aic_assert.h>
#include <aic_osa.h>
#include <aos_mem.h>
#include <rt_heap.h>

/* As same as aos_dynamic_mem_pool. */
struct __Heap_Descriptor
{
    u32                  total_mem_size;
    u32                  max_mem_size;
    u32                  total_mem_num;
    aos_dynamic_block    *block_head;
};

//extern u32 __use_no_heap;
extern _ARMABI void __use_no_heap(void);

int aic_rt_no_heap_init(void)
{
    /* open __use_no_heap */
    //__asm(".global __use_no_heap\n\t");

    return 0;
}

int __Heap_DescSize(int zero)
{
    return sizeof(aos_dynamic_block);
}

void __Heap_Initialize(struct __Heap_Descriptor *h)
{
    /* donothing, init not here. */
}

void __Heap_Finalize(struct __Heap_Descriptor *h)
{
    /* donothing, deinit not here */
}

void __Heap_ProvideMemory(struct __Heap_Descriptor *h, void *buf, size_t size)
{
    /* no need implement this function. */
}

void *__Heap_Alloc(struct __Heap_Descriptor *h, size_t size)
{
    return aos_malloc(size);
}

void __Heap_Free(struct __Heap_Descriptor *h, void *mem)
{
    aos_free(mem);
}

#if 0
/* has move bellow apis to mem_ext.c */
void *malloc(size_t size)
{
    return aos_malloc(size);
}

void free(void *mem)
{
    aos_free(mem);
}

void *realloc(void *buf, size_t size)
{
    void *new;

    if (!buf)
        return aos_malloc(size);

    new = aos_malloc(size);
    /* alloc failed, return old buf. */
    if (!new)
        return buf;

    memcpy(new, buf, size);
    aos_free(buf);

    return new;
}
#endif

void *__Heap_Realloc(struct __Heap_Descriptor *h, void *buf, size_t size)
{
    void *new;

    if (!buf)
        return aos_malloc(size);

    new = aos_malloc(size);
    /* alloc failed, return old buf. */
    if (!new)
        return buf;

    memcpy(new, buf, size);
    aos_free(buf);

    return new;
}

void *__Heap_Stats(struct __Heap_Descriptor *h,
                          int (*print)(void *, char const *format, ...),
                          void *printparam)
{
    /* no need implement this function. */
    return NULL;
}

int __Heap_Valid(struct __Heap_Descriptor *h,
                        int (*print)(void *, char const *format, ...),
                        void *printparam, int verbose)
{
    /* no need implement this function. */
    return NULL;
}

/*
 * The heap can call these functions when it is full or
 * inconsistent. __Heap_Full is passed the extra size that the heap
 * needs, and will call back to __Heap_ProvideMemory if it can find
 * some more storage. These are not functions you should override.
 */

int __Heap_Full(struct __Heap_Descriptor *h, size_t size)
{
    /* no need implement this function. */
    return 0;
}

void __Heap_Broken(struct __Heap_Descriptor *h)
{
    AIC_ASSERT(0);
}

/*
 * Call _init_alloc, and define __rt_heap_extend, if you are trying
 * to run a heap on the bare metal.
 */
void _init_alloc(uintptr_t base, uintptr_t top)
{
    /* no need implement this function. */
}

size_t __rt_heap_extend(size_t size, void **block)
{
    /* no need implement this function. */
    return 0;
}
#endif

