/* sprintf.c */

/*
 * Copyright (c) 1997-2010, 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1e6c1d
#define LOG_MODULE    AMOD_AOS

#include <stdarg.h>
#include <stdio.h>

#include "cbprintf.h"

struct emitter {
    char *ptr;
    int len;
};

static int sprintf_out(int c, struct emitter *p)
{
    if (p->len > 1) { /* need to reserve a byte for EOS */
        *(p->ptr) = c;
        p->ptr += 1;
        p->len -= 1;
    }
    return 0; /* indicate keep going so we get the total count */
}

int snprintf_nano(char * str, size_t len, const char *format, ...)
{
    va_list vargs;

    struct emitter p;
    int     r;
    char    dummy;

    if (len == 0) {
        str = &dummy; /* write final NUL to dummy, can't change *s */
    }

    p.ptr = str;
    p.len = (int) len;

    va_start(vargs, format);
    r = cbvprintf(sprintf_out, (void *) (&p), format, vargs);
    va_end(vargs);

    *(p.ptr) = 0;
    return r;
}

int sprintf_nano(char * str, const char *format, ...)
{
    va_list vargs;

    struct emitter p;
    int     r;

    p.ptr = str;
    p.len = (int) 0x7fffffff; /* allow up to "maxint" characters */

    va_start(vargs, format);
    r = cbvprintf(sprintf_out, (void *) (&p), format, vargs);
    va_end(vargs);

    *(p.ptr) = 0;
    return r;
}

int vsnprintf_nano(char * str, size_t len, const char *format, va_list vargs)
{
    struct emitter p;
    int     r;
    char    dummy;

    if (len == 0) {
        str = &dummy; /* write final NUL to dummy, can't change * *s */
    }

    p.ptr = str;
    p.len = (int) len;

    r = cbvprintf(sprintf_out, (void *) (&p), format, vargs);

    *(p.ptr) = 0;
    return r;
}

int vsprintf_nano(char * str, const char  *format, va_list vargs)
{
    struct emitter p;
    int     r;

    p.ptr = str;
    p.len = (int) 0x7fffffff; /* allow up to "maxint" characters */

    r = cbvprintf(sprintf_out, (void *) (&p), format, vargs);

    *(p.ptr) = 0;
    return r;
}
