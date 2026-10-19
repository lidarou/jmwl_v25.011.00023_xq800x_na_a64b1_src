/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used to help AIPC modules easily adpt to different OS.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x113f8f
#define LOG_MODULE    AMOD_AOS

#include <aic_bitmap.h>
#include <aic_mini_os_adapter.h>

#ifndef CONFIG_AIC_OSA
#error undef CONFIG_AIC_OSA, please check if support aic osa.
#error if not support please change bellow  #if 0 to #if 1 to open mini osa.
#if 0
#include <tx_thread.h>
#define AMOS_POOL_SIZE  0x80000

static u8 __attribute__ ((aligned(sizeof(ulong)))) s_adpter_pool[AMOS_POOL_SIZE];

static TX_BYTE_POOL s_byte_pool;
static ulong s_aos_alloc_size;

void aos_adpter_init_pools(void)
{
    UINT ret;

    ret = tx_byte_pool_create(&s_byte_pool, "aos", s_adpter_pool, AMOS_POOL_SIZE);
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);
}

void *aos_malloc_ex(ulong size, bool zero)
{
    UINT ret;
    void *addr;
    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    size = ALIGN(size, sizeof(ulong));

    /* TX_NO_MEMORY timeout */
    ret = tx_byte_allocate(&s_byte_pool, &addr, size, 0);

    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);
    s_aos_alloc_size += size;
    TX_RESTORE

    if (addr && zero)
        memset(addr, 0, size);

    aicx_debug(0x07bc5d, "dd", "aos alloc size=%d, total=%d\n", size, s_aos_alloc_size);
    return addr;
}

void aos_free_ex(void *addr, ulong size)
{
    UINT ret;
    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    size = ALIGN(size, sizeof(ulong));
    ret = tx_byte_release(addr);
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);
    s_aos_alloc_size -= size;
    TX_RESTORE

    aicx_debug(0x1e0316, "dd", "aos free size=%d, total=%d\n", size, s_aos_alloc_size);
}

static inline ULONG aos_ms_to_tick(ULONG time)
{
   ULONG mininum;

   if (TX_TIMER_TICKS_PER_SECOND == 1000)
        return time;

   if (time == AOS_NO_WAIT || time == AOS_WAIT_FOREVER)
        return time;

   mininum = 1000 / TX_TIMER_TICKS_PER_SECOND;
   if (time < mininum)
       time = mininum;

   return time / mininum;
}

typedef void TX_FUNC_T(ULONG p);

aos_thread_id aos_create_thread_simply(char *thread_name,
                                       aos_entry_func entry_func,
                                       void *input, u32 stack_size,
                                       u32 priority)
{
    aos_thread_t *aos_thread;
    UINT ret;

    aos_thread = aos_malloc(sizeof(*aos_thread));
    AIC_ASSERT(aos_thread);

    aos_thread->stack = aos_malloc(stack_size);
    AIC_ASSERT(aos_thread->stack);
    aos_thread->stack_size = stack_size;

    ret = tx_thread_create(&aos_thread->thread, thread_name,
                           (TX_FUNC_T *)entry_func, (ULONG)input,
                           aos_thread->stack, stack_size,
                           priority, priority, TX_NO_TIME_SLICE, TX_AUTO_START);
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);

    return (void*)aos_thread;
}

void aos_delete_thread(aos_thread_id thread)
{
    aos_thread_t * athread = thread;

    (void)tx_thread_terminate(&athread->thread);
    (void)tx_thread_delete(&athread->thread);

    aos_free_ex(athread->stack, athread->stack_size);
    aos_free_ex(athread, sizeof(*athread));
}

aos_queue_ptr aos_create_queue(char *name, u32 item_size, u32 queue_num)
{
    aos_queue_t *queue;
    UINT ret, queue_size;

    /* check item_size is sizeof(ULONG) align or not */
    AIC_ASSERT(IS_ALIGNED(item_size, sizeof(ULONG)));
    queue_size = item_size * queue_num;

    queue = aos_malloc(sizeof(*queue));
    AIC_ASSERT(queue);

    queue->queue_start = aos_malloc(queue_size);
    queue->queue_size = queue_size;
    AIC_ASSERT(queue->queue_start);

    ret = tx_queue_create(&queue->queue, name, item_size/sizeof(ULONG),
                          queue->queue_start, queue_size);

    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);

    return (void*)queue;
}

void aos_delete_queue(aos_queue_ptr queue)
{
    aos_queue_t *aqueue = queue;

    (void)tx_queue_delete(&aqueue->queue);
    aos_free_ex(aqueue->queue_start, aqueue->queue_size);
    aos_free_ex(aqueue, sizeof(*aqueue));
}

aos_status aos_send_to_queue(aos_queue_ptr queue, void *element, ulong wait)
{
    return (aos_status)tx_queue_send(queue, element, aos_ms_to_tick(wait));
}

aos_status aos_recv_from_queue(aos_queue_ptr queue,  void *element, ulong wait)
{
     return (aos_status)tx_queue_receive(queue, element, aos_ms_to_tick(wait));
}

aos_mutex_ptr aos_create_mutex(char *name)
{
    TX_MUTEX *mutex;
    UINT ret;

    mutex = aos_malloc(sizeof(*mutex));
    AIC_ASSERT(mutex);

    ret = tx_mutex_create(mutex, name, TX_TRUE);

    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);

    return mutex;
}

void aos_delete_mutex(aos_mutex_ptr mutex)
{
    (void)tx_mutex_delete(mutex);
    aos_free_ex(mutex, sizeof(TX_MUTEX));
}

aos_status aos_get_mutex(aos_mutex_ptr mutex, ULONG wait)
{
    /* Is the call from an ISR or Initialization? */
    if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
        return AOS_SUCCESS;

    return (aos_status)tx_mutex_get(mutex, aos_ms_to_tick(wait));
}

void aos_put_mutex(aos_mutex_ptr mutex)
{
    UINT ret;

    /* Is the call from an ISR or Initialization? */
    if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
        return;

    ret = tx_mutex_put(mutex);
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);
}

aos_event_ptr aos_create_event(char *name)
{
    TX_EVENT_FLAGS_GROUP *group;
    UINT ret;

    group = aos_malloc(sizeof(*group));
    AIC_ASSERT(group);

    ret = tx_event_flags_create(group, name);
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);

    return (aos_event_ptr)group;
}

void aos_delete_event(void *event)
{
    (void)tx_event_flags_delete(event);
    aos_free_ex(event, sizeof(TX_EVENT_FLAGS_GROUP));
}

void aos_set_event(aos_event_ptr event_ptr, u32 flags_to_set, u32 set_option)
{
    UINT ret;

    ret = tx_event_flags_set(event_ptr, flags_to_set, set_option);
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);
}

aos_status aos_get_event(aos_event_ptr event_ptr, u32 requested_flags,
                         u32 get_option, u32 *actual_flags_ptr, u32 wait_option)
{
    return (aos_status)tx_event_flags_get(event_ptr, requested_flags,
                                          get_option, (ulong*)actual_flags_ptr,
                                          aos_ms_to_tick(wait_option));
}

aos_semaphore_ptr aos_create_semaphore_simply(char *name)
{
    TX_SEMAPHORE *sem;
    UINT ret;

    sem = aos_malloc(sizeof(*sem));
    AIC_ASSERT(sem);

    ret = tx_semaphore_create(sem, name, 0);

    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);

    return (aos_semaphore_ptr)sem;
}

void aos_delete_semaphore(aos_semaphore_ptr sem)
{
    UINT ret;
    ULONG suspend_count = 0;

    ret = tx_semaphore_info_get(sem, NULL, NULL, NULL, &suspend_count, NULL);
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);

    while(suspend_count--)
        tx_semaphore_put(sem);

    (void)tx_semaphore_delete(sem);
    aos_free_ex(sem, sizeof(TX_SEMAPHORE));
}

void aos_put_semaphore(aos_semaphore_ptr sem)
{
    UINT ret;

    ret = tx_semaphore_put(sem);
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);
}

aos_status aos_get_semaphore(aos_semaphore_ptr sem, ULONG wait)
{
    return (aos_status)tx_semaphore_get(sem, aos_ms_to_tick(wait));
}

aos_timer_ptr aos_create_timer(char *name,
                               aos_expiration_func expiration_func, void *p,
                               u32 expiration_time, u32 auto_activate,
                               boolean b_period)
{
    TX_TIMER *timer;
    UINT ret;

    timer = aos_malloc(sizeof(*timer));
    AIC_ASSERT(timer);

    ret = tx_timer_create(timer, name, (tx_timer_function_t)expiration_func,
                          (ulong)p, aos_ms_to_tick(expiration_time),
                          (b_period ? aos_ms_to_tick(expiration_time) : 0),
                          auto_activate);

    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x0f41fe, "d", "ret = %d\n", ret);

    return timer;
}

void aos_delete_timer(aos_timer_ptr timer)
{
    (void)tx_timer_delete(timer);
    aos_free_ex(timer, sizeof(TX_TIMER));
}

void aos_change_timer(aos_timer_ptr timer, u32 expiration_time,
                      boolean b_period)
{
    (void)tx_timer_deactivate(timer);
    (void)tx_timer_change(timer, aos_ms_to_tick(expiration_time),
                          b_period ? aos_ms_to_tick(expiration_time) : 0);
    (void)tx_timer_activate(timer);
}

void aos_activate_timer(aos_timer_ptr timer)
{
    (void)tx_timer_activate(timer);
}

void aos_deactivate_timer(aos_timer_ptr timer)
{
    (void)tx_timer_deactivate(timer);
}

aos_status aos_sleep(ulong time)
{
    return (aos_status)tx_thread_sleep(aos_ms_to_tick(time));
}
#endif

#endif /* CONFIG_AIC_OSA */

