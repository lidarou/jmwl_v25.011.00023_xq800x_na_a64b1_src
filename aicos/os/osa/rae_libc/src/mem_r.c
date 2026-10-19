/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13e596
#define LOG_MODULE    AMOD_AOS

#include <stdlib.h>
#include <errno.h>

#include <aic_osa_ex.h>

void *_malloc_r (struct _reent *reent, size_t size)
{
    void *ptr;

    ptr = aos_malloc(size);
    if (!ptr)
        reent->_errno = ENOMEM;
    return ptr;
}

void _free_r (struct _reent *reent, void *buf)
{
    aos_free(buf);
}

void *_realloc_r (struct _reent *reent, void *buf, size_t  size)
{
    void *ptr;

    ptr = realloc(buf, size);
    if (!ptr)
        reent->_errno = ENOMEM;
    return ptr;
}

void *_calloc_r (struct _reent *reent, size_t nmemb,  size_t size)
{
    void *ptr;

    ptr = calloc(nmemb, size);
    if (!ptr)
        reent->_errno = ENOMEM;
    return ptr;
}

void *_memalign_r (struct _reent *reent, size_t a, size_t size)
{
    void *ptr = NULL;

    if (a == 4)
        ptr = aos_malloc(size); 
    else if (a == 8 || a == 16 || a == 32)
        ptr = aos_dma_malloc(size);

    if (!ptr)
        reent->_errno = ENOMEM;
    return ptr;
}

