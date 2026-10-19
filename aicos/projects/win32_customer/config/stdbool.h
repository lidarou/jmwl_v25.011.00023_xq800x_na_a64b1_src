/* stdbool.h */

/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __STDBOOL_H__
#define __STDBOOL_H__

#ifndef __cplusplus
#define _Bool  unsigned char
#define bool   unsigned char 
//#define BOOL   _Bool
#define true   1
#define false  0
#endif

#define __bool_true_false_are_defined  1

#endif /* __STDBOOL_H__ */
