/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the math-related common operatons.
 */

#ifndef __AIC_MATH_H__
#define __AIC_MATH_H__

/* Ensure that standard C is used to process the API information.  */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_type.h>

#ifndef MAX
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(n)      ((n < 0) ? -n : n)
#endif

/**
 * ROUND_UP - x rounded up to the next multiple of align.
 *
 * @x    : the rounded up number.
 * @align: the multiple base number, must be a power of 2.
 */
#define ROUND_UP(x, align) \
    (((unsigned long)(x) + ((unsigned long)(align) - 1)) & \
     ~((unsigned long)(align) - 1))

/**
 * ROUND_DOWN - x rounded down to the next multiple of align.
 *
 * @x     : the rounded down number.
 * @align: the multiple base number, must be a power of 2.
 */
#define ROUND_DOWN(x, align) \
    ((unsigned long)(x) & ~((unsigned long)(align) - 1))

/**
 * DIV_ROUND_UP - rounded up n/d.
 *
 * @n: the dividend.
 * @d: the divisor.
 */
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

/*
* some align marcos define.
* The align featrue as same as round up, the a must be a power of 2.
*/
#define ALIGN(x, a)         ROUND_UP(x, a)
#define ALIGN_DOWN(x, a)    ROUND_DOWN(x, a)

#if defined ( __GNUC__ )
#define IS_ALIGNED(x, a)    (((x) & ((typeof(x))(a) - 1)) == 0)
#else
#define IS_ALIGNED(x, a)    (((x) & ((a) - 1)) == 0)
#endif

/**
 * The below micros is used to operator the byte stream.
 * The param x is (uint8 *) type.
 */
#define AIC_GET_WORD(x)    (*(x)) | (*((x)+1) << 8)
#define AIC_GET_DWORD(x) \
    (uint32_t)(*(x)) | \
    ((uint32_t)*((x)+1) << 8) | \
    ((uint32_t)*((x)+2) << 16) | \
    ((uint32_t)*((x)+3) << 24)

#define AIC_SET_WORD(x, v) \
    do \
    { \
        *(x) = (v) & 0x00ff; \
        (*((x)+1)) = (v) >> 8; \
    } while (0)

#define AIC_SET_DWORD(x, v) \
    do \
    { \
        *(x) = (uint32_t)(v)  & 0x000000ff; \
        (*((x)+1)) = ((uint32_t)(v) >> 8) & 0x000000ff; \
        (*((x)+2)) = ((uint32_t)(v) >> 16) & 0x000000ff; \
        (*((x)+3)) = ((uint32_t)(v) >> 24); \
    } while (0)

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_MATH_H__ */

