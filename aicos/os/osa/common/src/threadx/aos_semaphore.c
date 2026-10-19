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
#define AIC_THIS_FILE    0x123288
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

/**
* aos_create_semaphore() - Create specified semaphore.
* @name_ptr: Specified semaphore name
* @initial_count: initial semaphore count
*
* Return: Valid semaphore ptr means success, AOS_NULL means error.
*/
aos_semaphore_ptr aos_create_semaphore(char *name_ptr, u32 initial_count)
{
#ifdef CONFIG_RTOS_THREADX
    TX_SEMAPHORE *semaphore_ptr = AOS_NULL;
    aos_status ret;
#endif

    if (!name_ptr)
        return AOS_NULL;

#ifdef CONFIG_RTOS_THREADX
    /* malloc control block for semaphore */
    semaphore_ptr = (TX_SEMAPHORE *)aos_malloc(sizeof(TX_SEMAPHORE));
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(semaphore_ptr);

    ret = (aos_status)tx_semaphore_create(semaphore_ptr, name_ptr, initial_count);
    if (ret != TX_SUCCESS) {
        aicx_error(0x0f07e8, "sd", "%s: create sema ret=%d\n", name_ptr, ret);
        aos_free(semaphore_ptr);
        return AOS_NULL;
    }

    /* semaphore_ptr must be free when deleted */
    return (aos_semaphore_ptr)semaphore_ptr;
#else
    return AOS_NULL;
#endif
}

/**
* aos_delete_semaphore() - Delete specified semaphore.
* @semaphore_ptr: Specified semaphore ptr.
*/
void aos_delete_semaphore(aos_semaphore_ptr semaphore_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_SEMAPHORE *tx_semaphore_ptr = (TX_SEMAPHORE *)semaphore_ptr;
    UINT ret;

    AIC_ASSERT(tx_semaphore_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = tx_semaphore_delete(tx_semaphore_ptr);
    AOS_ASSERT(ret == AOS_SUCCESS);

    /* semaphore_ptr is malloc when created, must free it here */
    aos_free(tx_semaphore_ptr);
#else
    /* TODO */
#endif
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
#ifdef CONFIG_RTOS_THREADX
    TX_SEMAPHORE *tx_semaphore_ptr = (TX_SEMAPHORE *)semaphore_ptr;
    aos_status ret = AOS_SUCCESS;

    AOS_ASSERT(tx_semaphore_ptr);

    ret = tx_semaphore_get(tx_semaphore_ptr, aos_time_to_tick(wait_option));
    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != AOS_SUCCESS && ret != TX_NO_INSTANCE)
        aicx_error(0x143d62, "sd", "%s, get sema ret=%d\n",
                   tx_semaphore_ptr->tx_semaphore_name, ret);

    return ret;
#else
    return AOS_ERROR;
#endif
}

/**
* aos_put_semaphore() - Put specified semaphore.
* @semaphore_ptr: Specified semaphore ptr
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
void aos_put_semaphore(aos_semaphore_ptr semaphore_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_SEMAPHORE *tx_semaphore_ptr = (TX_SEMAPHORE *)semaphore_ptr;
    aos_status ret = AOS_SUCCESS;

    AOS_ASSERT(tx_semaphore_ptr);

    ret = tx_semaphore_put(tx_semaphore_ptr);

    /* The caller is hard to handler the failed result, assert here. */
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);
#else
    /* TODO */
#endif
}
