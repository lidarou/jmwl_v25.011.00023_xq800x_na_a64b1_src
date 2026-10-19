/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ASSERT_H__
#define __ASSERT_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_system_assert.h>
#include <aic_assert_def.h>

#ifndef NDEBUG
#ifndef assert
#define assert(test) AIC_ASSERT_S(test)
#endif
#else
#ifndef assert
#define assert(test) ((void)0)
#endif
#endif

void __assert_func(char *fmt, int c, const char *s1, const char *s2);

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif  /* __ASSERT_H__ */
