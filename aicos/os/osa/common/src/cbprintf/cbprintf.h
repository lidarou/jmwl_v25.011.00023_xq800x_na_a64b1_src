/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CBPRINTF_H__
#define __CBPRINTF_H__
/**
 * Enalble this feature, will support print the full integral,
 * otherwise not support print the long long integral.
 */
#define CONFIG_CBPRINTF_FULL_INTEGRAL 1

/**
 * Enalble this feature, will support the complete
 * function of sprintf, otherwise will only support the
 * mininal function of sprintf.
 */
#define CONFIG_CBPRINTF_COMPLETE 0

/* if support float point printf */
#define CONFIG_CBPRINTF_FP_SUPPORT 1

/**
 * if support %a or %A specifier.
 * hexadecimal floating point.
 * %a: lowercase.
 * %A: upcase.
 */
#define CONFIG_CBPRINTF_FP_A_SUPPORT 1

/* force %a specifier */
#define CONFIG_CBPRINTF_FP_ALWAYS_A 0

/**
 * if support %n specifier.
 * int val
 * printf("test %n test", &val);
 * val = 5, it is the len form start to %n.
 */
#define CONFIG_CBPRINTF_N_SPECIFIER 1

/**
 * the macor conf must be defined, whe conf > 0, return true,
 * otherwise, return false.
 */
#define IS_ENABLED(conf) conf

typedef int (*cbprintf_cb)(/* int c, void *ctx */);

int cbvprintf(cbprintf_cb out, void *ctx, const char *format, va_list ap);

#endif /* __CBPRINTF_H__ */
