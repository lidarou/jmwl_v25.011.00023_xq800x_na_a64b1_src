/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system initialization interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15f233
#define LOG_MODULE    AMOD_FS

#include "vfs.h"

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
void *vfs_create_mutex(const char *name)
{
    aos_mutex_ptr *mutex_ptr;

    mutex_ptr = aos_create_mutex((char *)name);
    AIC_ASSERT(mutex_ptr);

    return (void *)mutex_ptr;
}

int vfs_get_mutex(void *mutex_ptr, u32 wait_timeout)
{
    aos_status mutex_status;

    AOS_ASSERT(NULL != mutex_ptr);

    if (AOS_INVALID_THREAD_ID != aos_identify_thread()) {
        mutex_status = aos_get_mutex((aos_mutex_ptr)mutex_ptr, wait_timeout);
        AOS_ASSERT(AOS_SUCCESS == mutex_status);
    }

    return AIC_SUCCESS;
}

void vfs_put_mutex(void *mutex_ptr)
{
    AOS_ASSERT(NULL != mutex_ptr);

    if (AOS_INVALID_THREAD_ID != aos_identify_thread()) {
        aos_put_mutex((aos_mutex_ptr)mutex_ptr);
    }
}

void vfs_delete_mutex(void *mutex_ptr)
{
    aos_delete_mutex((aos_mutex_ptr)mutex_ptr);
}

