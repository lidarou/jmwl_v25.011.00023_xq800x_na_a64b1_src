/* sprintf_nano.h */

/*
 * Copyright (c) 1997-2010, 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdarg.h>

#ifdef WIN32
#define snprintf_nano       snprintf
#define sprintf_nano        sprintf
#define vsnprintf_nano      vsnprintf
#define vsprintf_nano       vsprintf
#else
int snprintf_nano(char * str, size_t len, const char *format, ...);
int sprintf_nano(char * str, const char *format, ...);
int vsnprintf_nano(char * str, size_t len, const char *format, va_list vargs);
int vsprintf_nano(char * str, const char  *format, va_list vargs);
#endif

