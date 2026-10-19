/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x124e1e
#define LOG_MODULE    AMOD_AOS

#include <aic_assert.h>

void __assert_func(char *fmt, int c, const char *s1, const char *s2)
{
    aic_sys_assert("fmt=%s, line=%d, s1=%s, s2=%s.\n",
                    fmt ? fmt : "NULL",
                    c,
                    s1 ? s1 : "NULL",
                    s2 ? s1 : "NULL"
                   );
}

