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
#define AIC_THIS_FILE    0x108df2
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

/**
* aos_create_mutex() - Create specified mutex.
* @name_ptr: Specified mutex name
*
* Return: Valid mutex ptr means success, AOS_NULL means error.
*/
aos_mutex_ptr aos_create_mutex(char *name_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_MUTEX *mutex_ptr = NULL;
    aos_status ret;
#endif

    if (!name_ptr)
        return AOS_NULL;

#ifdef CONFIG_RTOS_THREADX
    /* malloc control block for mutex */
    mutex_ptr = (TX_MUTEX *)aos_malloc(sizeof(TX_MUTEX));
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(mutex_ptr);

    ret = (aos_status)tx_mutex_create(mutex_ptr, name_ptr, TX_INHERIT);
    if (ret != TX_SUCCESS) {
        aicx_error(0x0b19ea, "sd", "%s: create mutex ret=%d\n", name_ptr, ret);
        aos_free(mutex_ptr);
        return AOS_NULL;
    }

    /* mutex_ptr must be free when deleted */
    return (aos_mutex_ptr)mutex_ptr;
#else
    return AOS_NULL;
#endif
}

/**
* aos_delete_mutex() - Delete specified mutex.
* @mutex_ptr: Specified mutex ptr.
*/
void aos_delete_mutex(aos_mutex_ptr mutex_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_MUTEX * tx_mutex_ptr= (TX_MUTEX *)mutex_ptr;
    UINT ret;

    AOS_ASSERT(tx_mutex_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = tx_mutex_delete(tx_mutex_ptr);
    AOS_ASSERT(ret == AOS_SUCCESS);

    /* mutex_ptr is malloc when created, must free it here */
    aos_free(tx_mutex_ptr);
#else
    /* TODO */
#endif
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
#ifdef CONFIG_RTOS_THREADX
    TX_MUTEX *tx_mutex_ptr = (TX_MUTEX *)mutex_ptr;
    aos_status ret = AOS_ERROR;

    AOS_ASSERT(tx_mutex_ptr);

    /* The return status of aos is fully compatibe with threadx. */
    ret = (aos_status)tx_mutex_get(tx_mutex_ptr, aos_time_to_tick(wait_option));

    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != AOS_SUCCESS && ret != AOS_NOT_AVAILABLE)
        aicx_error(0x10f7f5, "sd", "%s, get mutex ret=%d\n", tx_mutex_ptr->tx_mutex_name, ret);

    return ret;
#else
    return AOS_ERROR;
#endif
}

/**
* aos_put_mutex() - Put specified mutex.
* @mutex_ptr: Specified mutex ptr
*/
void aos_put_mutex(aos_mutex_ptr mutex_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_MUTEX *tx_mutex_ptr = (TX_MUTEX *)mutex_ptr;
    aos_status ret;

    AOS_ASSERT(tx_mutex_ptr);

    ret = (aos_status)tx_mutex_put(tx_mutex_ptr);

    /* The caller is hard to handler the failed result, assert here. */
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);
#else
    /* TODO */
#endif
}
