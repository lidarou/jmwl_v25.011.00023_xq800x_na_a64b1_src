/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines integer base 2 logarithm calculation.
 */

#ifndef __AIC_LOG2_H__
#define __AIC_LOG2_H__

/* Ensure that standard C is used to process the API information.  */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_bitmap.h>

/**
 * is_power_of_two() - Is x a power of two?
 * @x: value to check
 *
 * return true if x is a power of two, false otherwise,
 */
static inline boolean is_power_of_two(unsigned int x)
{
    return (x != 0U) && ((x & (x - 1U)) == 0U);
}

/**
 * ilog2 - log base 2 of 32-bit or 64-bit unsigned value
 * @n: parameter
 *
 */
static inline int ilog2(unsigned long n)
{
    return aic_find_last_bit(n);
}

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_MATH_H__ */

