/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x065bd1
#define LOG_MODULE    AMOD_AOS

#include <aic_osa_ex.h>

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
    /* alloc failed, return NULL and don't free old buf. */
    if (!new)
        return NULL;

    memcpy(new, buf, size);
    aos_free(buf);

    return new;
}

void *calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0)
        return NULL;

    return aos_zalloc(nmemb *size);
}

