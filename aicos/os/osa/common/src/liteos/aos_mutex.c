/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA mutex function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c32ed
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include "los_mux.h"

/**
* aos_create_mutex() - Create specified mutex.
* @name_ptr: Specified mutex name
*
* Return: Valid mutex ptr means success, AOS_NULL means error.
*/
aos_mutex_ptr aos_create_mutex(char *name_ptr)
{
    u32 *mutex_ptr = AOS_NULL;
    aos_status ret;

    if (!name_ptr)
        return AOS_NULL;

    /* malloc control block for mutex */
    mutex_ptr = (u32 *)aos_malloc(sizeof(u32));
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(mutex_ptr);

    ret = LOS_MuxCreate(mutex_ptr);
    if (ret != LOS_OK) {
        aicx_error(0x0b19ea, "sd", "%s: create mutex ret=%d\n", name_ptr, ret);
        aos_free(mutex_ptr);
        return AOS_NULL;
    }

    /* mutex_ptr must be free when deleted */
    return (aos_mutex_ptr)mutex_ptr;
}

/**
* aos_delete_mutex() - Delete specified mutex.
* @mutex_ptr: Specified mutex ptr.
*/
void aos_delete_mutex(aos_mutex_ptr mutex_ptr)
{
    u32 *los_mutex_ptr= (u32*)mutex_ptr;
    u32 ret;

    AOS_ASSERT(los_mutex_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = LOS_MuxDelete(*los_mutex_ptr);
    AOS_ASSERT(ret == LOS_OK);

    /* mutex_ptr is malloc when created, must free it here */
    aos_free(los_mutex_ptr);
}

/**
* aos_get_mutex() - Get specified mutex.
* @mutex_ptr: Specified mutex ptr
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_get_mutex(aos_mutex_ptr mutex_ptr, u32 wait_option)
{
    u32 *los_mutex_ptr = (u32 *)mutex_ptr;
    aos_status ret = AOS_ERROR;

    AOS_ASSERT(los_mutex_ptr);

    /* The return status of aos is fully compatibe with threadx. */
    ret = LOS_MuxPend(*los_mutex_ptr, aos_time_to_tick(wait_option));

    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != LOS_OK && ret != LOS_ERRNO_MUX_TIMEOUT) {
        aicx_error(0x075109, "d", "get mutex ret=%d\n", ret);
    }

    return ret;
}

/**
* aos_put_mutex() - Put specified mutex.
* @mutex_ptr: Specified mutex ptr
*/
void aos_put_mutex(aos_mutex_ptr mutex_ptr)
{
    u32 *los_mutex_ptr = (u32 *)mutex_ptr;
    aos_status ret;

    AOS_ASSERT(los_mutex_ptr);

    ret = LOS_MuxPost(*los_mutex_ptr);

    /* The caller is hard to handler the failed result, assert here. */
    AICX_ASSERT_IFX(ret == LOS_OK, 0x0f41fe, "d", "ret = %d\n", ret);

}
