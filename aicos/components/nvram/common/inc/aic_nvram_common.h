/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic nvram common Interface (API)
 */

#ifndef __AIC_NVRAM_COMMON_H__
#define __AIC_NVRAM_COMMON_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_module.h>
#include <alog_api.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define NVRAM_INVALID_TIMESTAMP       0xFFFFFFFF
#define NVRAM_INVALID_ITEM_ID         0xFFFF
#define NVRAM_INVALID_ITEM_SIZE       0xFFFF
#define NVRAM_RESERVED_ITEM_ID        0xFFFE

/**---------------------------------------------------------------------------*
 ** OS Macro Definitions
 **---------------------------------------------------------------------------*/
/* mutex */
#define NVRAM_ACQUIRE_MUTEX(mutex_ptr, wait_timeout) \
if (AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    u32 mutex_status; \
    AIC_ASSERT(NULL != mutex_ptr); \
    mutex_status = aos_get_mutex(mutex_ptr, wait_timeout); \
    AIC_ASSERT(AOS_SUCCESS == mutex_status); \
}

#define NVRAM_RELEASE_MUTEX(mutex_ptr) \
if (AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    AIC_ASSERT(NULL != mutex_ptr); \
    aos_put_mutex(mutex_ptr); \
}

/**---------------------------------------------------------------------------*
 ** Debug Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * aic_nvram_crc - Software CRC implementation with small lookup table
 *
 * Parameter:
 * @crc:.
 * @buffer:.
 * @size:.
 *
 * Return:
 *  crc value.
 */
u32 aic_nvram_crc(u32 crc, const void *buffer, size_t size);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_NVRAM_COMMON_H__ */

