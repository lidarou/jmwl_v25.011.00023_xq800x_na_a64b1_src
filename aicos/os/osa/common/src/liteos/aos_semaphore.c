/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA semaphore function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04d647
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include "los_sem.h"

/**
* aos_create_semaphore() - Create specified semaphore.
* @name_ptr: Specified semaphore name
* @initial_count: initial semaphore count
*
* Return: Valid semaphore ptr means success, AOS_NULL means error.
*/
aos_semaphore_ptr aos_create_semaphore(char *name_ptr, u32 initial_count)
{
    u32 *semaphore_ptr = AOS_NULL;
    aos_status ret;

    if (!name_ptr)
        return AOS_NULL;

    /* malloc control block for semaphore */
    semaphore_ptr = (u32 *)aos_malloc(sizeof(u32));
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(semaphore_ptr);

    ret = LOS_SemCreate(initial_count, semaphore_ptr);
    if (ret != LOS_OK) {
        aicx_error(0x0f07e8, "sd", "%s: create sema ret=%d\n", name_ptr, ret);
        aos_free(semaphore_ptr);
        return AOS_NULL;
    }

    /* semaphore_ptr must be free when deleted */
    return (aos_semaphore_ptr)semaphore_ptr;
}

/**
* aos_delete_semaphore() - Delete specified semaphore.
* @semaphore_ptr: Specified semaphore ptr.
*/
void aos_delete_semaphore(aos_semaphore_ptr semaphore_ptr)
{
    u32 *los_semaphore_ptr = (u32 *)semaphore_ptr;
    u32 ret;

    AIC_ASSERT(los_semaphore_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = LOS_SemDelete(*los_semaphore_ptr);
    AOS_ASSERT(ret == LOS_OK);

    /* semaphore_ptr is malloc when created, must free it here */
    aos_free(los_semaphore_ptr);
}

/**
* aos_get_semaphore() - Get specified semaphore.
* @semaphore_ptr: Specified semaphore ptr
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_get_semaphore(aos_semaphore_ptr semaphore_ptr, u32 wait_option)
{
    u32 *los_semaphore_ptr = (u32 *)semaphore_ptr;
    u32 ret = AOS_SUCCESS;

    AOS_ASSERT(los_semaphore_ptr);

    ret = LOS_SemPend(*los_semaphore_ptr, aos_time_to_tick(wait_option));
    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != LOS_OK && ret != LOS_ERRNO_SEM_TIMEOUT){
        aicx_error(0x075109, "d", "get mutex ret=%d\n", ret);
    }
    return ret;
}

/**
* aos_put_semaphore() - Put specified semaphore.
* @semaphore_ptr: Specified semaphore ptr
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
void aos_put_semaphore(aos_semaphore_ptr semaphore_ptr)
{
    u32 *los_semaphore_ptr = (u32 *)semaphore_ptr;
    u32 ret = LOS_OK;

    AOS_ASSERT(los_semaphore_ptr);

    ret = LOS_SemPost(*los_semaphore_ptr);

    /* The caller is hard to handler the failed result, assert here. */
    AICX_ASSERT_IFX(ret == LOS_OK, 0x0f41fe, "d", "ret = %d\n", ret);
}
