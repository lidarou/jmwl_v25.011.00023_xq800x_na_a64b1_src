/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the assert Interface.
 */

#ifndef __ASSERT_H__
#define __ASSERT_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_type.h>

#ifdef CONFIG_LOG_DECREASE
#define sys_assert(f, ...)
#else
#define sys_assert(f, ...) \
      log_print("Assert!! Function=%s, Line=%d, Assert info ="f, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#define ASSERT(n) do { \
        if(!(n)) { \
            sys_assert(); \
            while(1); \
        } \
    } while(0)

#define ASSERT_INFO(n, fmt, ...) do { \
        if(!(n)) { \
            sys_assert(fmt, ##__VA_ARGS__); \
            while(1); \
        } \
    } while(0)

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __ASSERT_H__ */

