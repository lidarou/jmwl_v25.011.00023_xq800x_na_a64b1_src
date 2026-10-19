/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the size-related common operatons and macros.
 */

#ifndef __AIC_SIZE_H__
#define __AIC_SIZE_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern   "C" {
#endif

#include <aic_type.h>
#include <aic_bitmap.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define KB(x)       (((size_t)x) << 10)
#define MB(x)       (KB(x) << 10)
#define GB(x)       (MB(x) << 10)

#define SIZE_32B    BIT_5
#define SIZE_64B    BIT_6
#define SIZE_128B   BIT_7
#define SIZE_256B   BIT_8
#define SIZE_512B   BIT_9
#define SIZE_1K     BIT_10
#define SIZE_2K     BIT_11
#define SIZE_4K     BIT_12
#define SIZE_8K     BIT_13
#define SIZE_16K    BIT_14
#define SIZE_32K    BIT_15
#define SIZE_64K    BIT_16
#define SIZE_128K   BIT_17
#define SIZE_256K   BIT_18
#define SIZE_512K   BIT_19
#define SIZE_1M     BIT_20
#define SIZE_2M     BIT_21
#define SIZE_4M     BIT_22
#define SIZE_8M     BIT_23
#define SIZE_16M    BIT_24
#define SIZE_32M    BIT_25
#define SIZE_64M    BIT_26
#define SIZE_128M   BIT_27
#define SIZE_256M   BIT_28
#define SIZE_512M   BIT_29
#define SIZE_1G     BIT_30
#define SIZE_2G     BIT_31
#define SIZE_4G     0x100000000ULL

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_SIZE_H__ */
