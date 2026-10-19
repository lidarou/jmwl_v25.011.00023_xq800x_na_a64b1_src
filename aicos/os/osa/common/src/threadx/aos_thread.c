/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA thread function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08c141
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include <aos_thread.h>
#include <aos_config.h>
#include <aic_assert.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#include <tx_thread.h>
#include <tx_timer.h>
#endif
#include <sprintf_nano.h>

#define AOS_THREAD_OWNED_PTR        (void *)(0x11223344)

#ifdef CONFIG_RTOS_THREADX
typedef VOID (*tx_entry_function)(ULONG id);
#endif

aos_thread_desc *g_thread_desc_tbl[AOS_MAX_THREADS];
extern aos_thread_config g_thread_config_tbl[];
extern u32 get_thread_config_tbl_idx(aos_thread_id thread_id);

/**
* aos_init_thread_desc_tbl() - initialize global thread table.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_init_thread_desc_tbl(void)
{
    return AOS_SUCCESS;
}

static void _aos_thread_entry_init(void *input)
{
    u32 index;
    aos_thread_id tid;
    aos_status init_ret;

    tid = *(aos_thread_id *)input;

    AIC_ASSERT(tid < AOS_MAX_THREADS);

    index = get_thread_config_tbl_idx(tid);
    AIC_ASSERT(index != AOS_ERROR);

    if(g_thread_config_tbl[index].task_func.task_init) {
        init_ret = (*g_thread_config_tbl[index].task_func.task_init)();
        if (init_ret != AOS_SUCCESS)
            aicx_error(0x0717f9, "u", "thread id: %u, task_init fail!", tid);
    }
    return;
}

static void _aos_thread_entry_main(void *input)
{
    u32 index;
    aos_fsm_msg* fsm_msg_ptr;
    aos_thread_id tid;

    tid = *(aos_thread_id *)input;

    AIC_ASSERT(tid < AOS_MAX_THREADS);

    index = get_thread_config_tbl_idx(tid);
    AIC_ASSERT(index != AOS_ERROR);
    AIC_ASSERT(g_thread_config_tbl[index].task_func.task_main);

    while(AOS_TRUE) {
        /* 1.get and handle msg from thread external queue */
        fsm_msg_ptr = aos_recv_msg_from_extq(tid, AOS_WAIT_FOREVER);
        (*g_thread_config_tbl[index].task_func.task_main)(fsm_msg_ptr);

        /* 2.get and handle all msg from thread internal queue */
        if(g_thread_config_tbl[index].intq_num) {
            fsm_msg_ptr = aos_recv_msg_from_intq(tid, AOS_NO_WAIT);
            while(fsm_msg_ptr) {
                (*g_thread_config_tbl[index].task_func.task_main)(fsm_msg_ptr);
                fsm_msg_ptr = aos_recv_msg_from_intq(tid, AOS_NO_WAIT);
            }
        }
    }

    return;
}

/**
* common thread entry for standard thread
*/
void aos_standard_thread_entry(void *input)
{
    _aos_thread_entry_init(input);
    _aos_thread_entry_main(input);
}

static TX_QUEUE *aos_create_queue_for_thread(u16 q_num, char *thread_name,
                                            char *ext_name, char *q_name)
{
    TX_QUEUE *qcb_ptr;
    int q_alloc_len;
    char *qname;
    u8 *q_ptr;
    UINT ret;

    q_alloc_len = q_name ? 0: Q_NAME_SIZE;
    qcb_ptr = aos_malloc(sizeof(*qcb_ptr) + q_alloc_len +
                         q_num * sizeof(aos_msg_ptr));
    AIC_DUMMY_ASSERT(qcb_ptr);
    memset(qcb_ptr, 0, sizeof(*qcb_ptr) + q_alloc_len);

    q_ptr = (u8 *)(qcb_ptr + 1) + q_alloc_len;

    if (q_name) {
        qname = q_name;
    } else {
        qname = (char *)(qcb_ptr + 1);
        snprintf_nano(qname, Q_NAME_SIZE, "%s_%s", thread_name, ext_name);
    }

    ret = tx_queue_create(qcb_ptr, qname,
                          sizeof(aos_msg_ptr)/sizeof(ULONG),
                          q_ptr, q_num*sizeof(aos_msg_ptr));
    AIC_ASSERT(ret == TX_SUCCESS);

    return qcb_ptr;
}

static aos_thread_id _create_thread_queue(aos_thread_id thread_id, char *thread_name,
                                 aos_standard_entry_func entry_func, u32 input,
                                 u8 *stack_start, u32 stack_size,
                                 u32 intq_num, char *intq_name,
                                 u32 extq_num, char *extq_name,
                                 u32 priority, u32 auto_start)
{
#ifdef CONFIG_RTOS_THREADX
    TX_INTERRUPT_SAVE_AREA

    aos_thread_desc *thread_desc;
    aos_thread_desc **thread_pptr;
    aos_thread_id index;
    aos_status ret;
    u8 *stack_ptr;

    TX_DISABLE

    /* invalid id means need dynamic alloc a id. */
    if (thread_id == AOS_INVALID_THREAD_ID) {
        for (index = AOS_STANDARD_THREADS; index < AOS_MAX_THREADS; index++) {
            if (!g_thread_desc_tbl[index]) {
                /* own it fisrtly to avoid next alloc search it again. */
                g_thread_desc_tbl[index] = AOS_THREAD_OWNED_PTR;
                break;
            }
        }

        if (index == AOS_MAX_THREADS) {
            TX_RESTORE
            return AOS_INVALID_THREAD_ID;
        }
    } else {
        index = thread_id;
        AIC_ASSERT(!g_thread_desc_tbl[index]);
    }

    /* 5.set value to global variable */
    thread_pptr = &g_thread_desc_tbl[index];
    TX_RESTORE

    /* 1.malloc thread control block and stack area */
    if (stack_start) {
        thread_desc = (aos_thread_desc *)aos_malloc(sizeof(*thread_desc));
        stack_ptr = stack_start;
    } else {
        thread_desc =
            (aos_thread_desc *)aos_malloc(sizeof(*thread_desc) + stack_size);
        stack_ptr = (u8 *)(thread_desc + 1);
    }
    AIC_DUMMY_ASSERT(thread_desc);
    memset(thread_desc, 0, sizeof(*thread_desc));
    *thread_pptr = thread_desc;

    /* 2.malloc internal queue area and create thread internal queue */
    if (intq_num)
        thread_desc->int_qcb =
            aos_create_queue_for_thread(intq_num, thread_name, "INT_Q", intq_name);

    /* 3.malloc external queue area and create thread external queue */
    if (extq_num)
        thread_desc->ext_qcb =
            aos_create_queue_for_thread(extq_num, thread_name, "EXT_Q", extq_name);

    /* 4.create thread with TX_DONT_START to avoid preempt */
    ret = tx_thread_create(&thread_desc->tcb_ptr, thread_name,
                           (tx_entry_function)entry_func, (ULONG)input,
                           (VOID *)stack_ptr, (ULONG)stack_size,
                           (UINT)priority, (UINT)priority,
                           (ULONG)TX_NO_TIME_SLICE, (UINT)TX_DONT_START);
    AIC_ASSERT(ret == TX_SUCCESS);

    if (auto_start == AOS_AUTO_START) {
        /* 7.resume the created thread after global variable set */
        ret = (aos_status)tx_thread_resume(&thread_desc->tcb_ptr);
        AIC_ASSERT (ret == TX_SUCCESS);
    }
#endif

    return index;
}

/**
* aos_create_thread_ex() - Create common thread ex.
* @thread_name: Specified thread name.
* @entry_func: Specified thread entry func.
* @input: Specified input param for thread entry func.
* @stack_start: Specified stack start.
* @stack_size: Specified stack size.
* @queue_num: Specified external queue capacity, default message size is
* sizeof(aos_msg_ptr).
* @queue_name: Specified external queue name.
* @priority: Specified thread priority.
* @auto_start: Specified thread automatic start selection.
*
* Return: thread id.
*/
aos_thread_id aos_create_thread_ex(char *thread_name,
                                aos_entry_func entry_func,
                                void *input,
                                u8 *stack_start,
                                u32 stack_size,
                                u32 queue_num, char *queue_name,
                                u32 priority, u32 auto_start)
{
    if (!thread_name || !entry_func || !stack_size)
        return AOS_INVALID_THREAD_ID;

    if (priority >= AOS_MAX_PRIORITY)
        return AOS_INVALID_THREAD_ID;

    return _create_thread_queue(AOS_INVALID_THREAD_ID, thread_name,
                         (aos_standard_entry_func)entry_func,
                         (u32)input, stack_start, stack_size,
                         0, AOS_NULL,
                         queue_num, queue_name,
                         priority, auto_start);
}

/**
* aos_create_thread() - Create common thread.
* @thread_name: Specified thread name.
* @entry_func: Specified thread entry func.
* @input: Specified input param for thread entry func.
* @stack_size: Specified stack size.
* @queue_num: Specified external queue capacity, default message size is
* sizeof(aos_msg_ptr).
* @queue_name: Specified external queue name.
* @priority: Specified thread priority.
* @auto_start: Specified thread automatic start selection.
*
* Return: thread id.
*/
aos_thread_id aos_create_thread(char *thread_name,
                                aos_entry_func entry_func,
                                void *input,
                                u32 stack_size,
                                u32 queue_num, char *queue_name,
                                u32 priority, u32 auto_start)
{
    return aos_create_thread_ex(thread_name,
                                entry_func,
                                input,
                                NULL,
                                stack_size,
                                queue_num,
                                queue_name,
                                priority,
                                auto_start);
}

/**
* aos_create_standard_thread() - Create standard thread
* @thread_id: Specifidd thread id. Note: thread id must < AOS_STANDARD_THREADS
* @thread_name: Specified thread name.
* @entry_func: Specified thread entry func.
* @input: Specified input param for thread entry func.
* @stack_size: Specified stack size.
* @intq_num: Specified internal queue capacity, default message size is
* sizeof(aos_msg_ptr).
* @intq_name: Specified internal queue name.
* @extq_num: Specified external queue capacity, default message size is
* sizeof(aos_msg_ptr).
* @extq_name: Specified external queue name.
* @priority: Specified thread priority.
* @auto_start: Specified thread automatic start selection.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_create_standard_thread(aos_thread_id thread_id, char *thread_name,
                               aos_standard_entry_func entry_func,
                               void *input, u32 stack_size,
                               u32 intq_num, char *intq_name,
                               u32 extq_num, char *extq_name,
                               u32 priority, u32 auto_start)
{

    if (thread_id >= AOS_STANDARD_THREADS || priority >= AOS_MAX_PRIORITY)
        return AOS_ERROR;

    if (!thread_name || !stack_size)
        return AOS_ERROR;

    if (AOS_STANDARD_THREADS == _create_thread_queue(thread_id, thread_name,
                                                     entry_func,
                                                     (u32)input,
                                                     NULL, stack_size,
                                                     intq_num, intq_name,
                                                     extq_num, extq_name,
                                                     priority, auto_start))
        return AOS_ERROR;

    return AOS_SUCCESS;
}

/**
* aos_start_thread() - Start a thread.
* @thread_id: Specified thread id.
*/
aos_status aos_start_thread(aos_thread_id thread_id)
{
    aos_status ret = AOS_SUCCESS;

#ifdef CONFIG_RTOS_THREADX
    TX_THREAD *tcb_ptr = AOS_NULL;
#endif

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    if (!g_thread_desc_tbl[thread_id])
        return AOS_ERROR;

#ifdef CONFIG_RTOS_THREADX
    tcb_ptr = &(g_thread_desc_tbl[thread_id]->tcb_ptr);
    ret = (aos_status)tx_thread_resume(tcb_ptr);
    AIC_ASSERT (ret == TX_SUCCESS);
#endif

    return ret;
}

/**
* aos_delete_thread() - Delete thread.
* @thread_id: Specified thread id.
*/
void aos_delete_thread(aos_thread_id thread_id)
{
#ifdef CONFIG_RTOS_THREADX
    TX_QUEUE *intq_ptr, *extq_ptr;
    aos_thread_desc *thread_desc;
    TX_THREAD *tcb_ptr;
    TX_INTERRUPT_SAVE_AREA

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);

    TX_DISABLE
    thread_desc = g_thread_desc_tbl[thread_id];
    if (!thread_desc) {
        TX_RESTORE
        return;
    }

    g_thread_desc_tbl[thread_id] = AOS_NULL;
    TX_RESTORE

    intq_ptr = thread_desc->int_qcb;
    extq_ptr = thread_desc->ext_qcb;
    tcb_ptr = &(thread_desc->tcb_ptr);

    /* 1.free and delete tcb */
    (void)tx_thread_terminate(tcb_ptr);
    (void)tx_thread_delete(tcb_ptr);

    /* 2.free and delete queue and qcb */
    if (intq_ptr) {
        (void)tx_queue_delete(intq_ptr);
        aos_free(intq_ptr);
    }

    if (extq_ptr) {
        (void)tx_queue_delete(extq_ptr);
        aos_free(extq_ptr);
    }

    /* 3.free thead tcb */
    aos_free(thread_desc);

#endif
}

/**
* aos_change_current_thread_pri() - change current thread priority.
* @new_priority: new priority to set.
* @old_priority: return old priority.
*/
void aos_change_current_thread_pri(u32 new_priority,
                          u32 *old_priority)
{
#ifdef CONFIG_RTOS_THREADX
    TX_THREAD *tcb_ptr;
    UINT ret;

    tcb_ptr = tx_thread_identify();
    ret = tx_thread_priority_change(tcb_ptr,
                                    new_priority, old_priority);
    /* All errors are caused by improper use this api, assert it for debug. */
    AOS_ASSERT(ret == AOS_SUCCESS);
#else
    /* TODO */
#endif
}

/**
* aos_change_thread_pri() - change thread priority.
* @thread_id: Specified thread id.
* @new_priority: new priority to set.
* @old_priority: return old priority.
*/
void aos_change_thread_pri(aos_thread_id thread_id, u32 new_priority,
                          u32 *old_priority)
{
#ifdef CONFIG_RTOS_THREADX
    UINT ret;
#endif

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

#ifdef CONFIG_RTOS_THREADX
    ret = tx_thread_priority_change(&(g_thread_desc_tbl[thread_id]->tcb_ptr),
                                    new_priority, old_priority);
    /* All errors are caused by improper use this api, assert it for debug. */
    AOS_ASSERT(ret == AOS_SUCCESS);
#else
    /* TODO */
#endif
}

u32 aos_get_thread_count()
{
    return _tx_thread_created_count;
}

aos_status aos_get_all_thread_status(thread_info_t *info, u32 count)
{
    TX_INTERRUPT_SAVE_AREA
    u32      index;                  /* Pointer inside thread's stack.  */
    TX_THREAD * tp;                  /* Pointer to each thread.  */

    /* If no threads are created, return immediately.  */
    if (!_tx_thread_created_count)
        return 0;

    index = 0;
    /* Disable interrupts.  */
    TX_DISABLE
    /* Start iterating through the threads in the system.  Assume that we always
       have at least one thread (the system timer thread) in the system.  */
    tp = _tx_thread_created_ptr;

    do {
        if (index >= count) {
            break;
        }

        info[index].handler = tp;
        info[index].name = tp->tx_thread_name;
        info[index].number = index;
        info[index].state = tp->tx_thread_state;
        info[index].curr_priority = tp->tx_thread_user_priority;
        info[index].base_priority = tp->tx_thread_priority;
        info[index].stack_base = tp->tx_thread_stack_start;
        info[index].stack_alloc_size = tp->tx_thread_stack_size;
        info[index].statck_min_remained = (u32)tp->tx_thread_stack_ptr - (u32)tp->tx_thread_stack_start;

        /* Continue with the next thread.  */
        tp = tp->tx_thread_created_next;
        index++;
    /* Loop until we point to the first thread again.  */
    } while ( tp != _tx_thread_created_ptr );
    /* Restore interrupts.  */
    TX_RESTORE

    return index;
}

void aos_get_thread_pri(aos_thread_id thread_id, u32 *priority)
{
    UINT ret;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

    ret = tx_thread_info_get(&(g_thread_desc_tbl[thread_id]->tcb_ptr),
                            NULL, NULL, NULL, priority,
                            NULL, NULL, NULL, NULL);
    /* All errors are caused by improper use this api, assert it for debug. */
    AOS_ASSERT(ret == AOS_SUCCESS);
}

/**
* aos_identify_thread() - get current thread id.
* Return: thread id.
*/
aos_thread_id aos_identify_thread(void)
{
    u32 index, thread_id = AOS_INVALID_THREAD_ID;
#ifdef CONFIG_RTOS_THREADX
    TX_THREAD *tcb_ptr;

    tcb_ptr = tx_thread_identify();

    if (!tcb_ptr)
        return thread_id;

    for(index = 0; index < AOS_MAX_THREADS; index++) {
        if(&(g_thread_desc_tbl[index]->tcb_ptr) == tcb_ptr) {
            thread_id = index;
            break;
        }
    }
#endif

    return thread_id;
}

/**
* aos_suspend_thread() - suspend specific thread.
* @thread_id: Specified thread id.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_suspend_thread(aos_thread_id thread_id)
{
    aos_status ret = AOS_ERROR;
    aos_thread_desc *thread_desc;
    TX_THREAD *tcb_ptr;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

    thread_desc = g_thread_desc_tbl[thread_id];
    tcb_ptr = &thread_desc->tcb_ptr;

#ifdef CONFIG_RTOS_THREADX
    /* The return status of aos is fully compatibe with threadx. */
    ret = (aos_status)tx_thread_suspend(tcb_ptr);
    /* can't be TX_THREAD_ERROR, other error return to user. */
    AOS_ASSERT(ret != TX_THREAD_ERROR);
    if (ret != TX_SUCCESS)
        aicx_error(0x177353, "sd", "%s: suspend ret=%d\n", tcb_ptr->tx_thread_name, ret);
#endif

    return ret;
}

/**
* aos_resume_thread() - resume specific thread.
* @thread_id: Specified thread id.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_resume_thread(aos_thread_id thread_id)
{
    aos_status ret = AOS_ERROR;
    aos_thread_desc *thread_desc;
    TX_THREAD *tcb_ptr;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

    thread_desc = g_thread_desc_tbl[thread_id];
    tcb_ptr = &thread_desc->tcb_ptr;

#ifdef CONFIG_RTOS_THREADX
    /* The return status of aos is fully compatibe with threadx. */
    ret = tx_thread_resume(tcb_ptr);
    /* can't be TX_THREAD_ERROR, other error return to user. */
    AOS_ASSERT(ret != TX_THREAD_ERROR);
    if ((ret != TX_SUCCESS) && (ret != TX_SUSPEND_LIFTED))
        aicx_error(0x16ea5e, "sd", "%s: resume ret=%d\n", tcb_ptr->tx_thread_name, ret);
#endif

    return ret;
}

/**
* aos_sleep() - force specific thread to sleep for time_ms.
* @time_ms: time_ms ms to sleep.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_sleep(u32 time_ms)
{
#ifdef CONFIG_RTOS_THREADX
    return (aos_status)tx_thread_sleep(aos_time_to_tick(time_ms));
#else
    return AOS_ERROR;
#endif
}


/**
* aos_usleep() - force specific thread to sleep for time_us.
* @time_us: time_us us to sleep.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_usleep(u32 time_us)
{
    u32 time_ms;

    if (time_us < 1000)
        time_ms = 1;
    else
        time_ms = time_us / 1000;

#ifdef CONFIG_RTOS_THREADX
        return (aos_status)tx_thread_sleep(aos_time_to_tick(time_ms));
#else
        return AOS_ERROR;
#endif
}

/**
* aos_terminate_thread() - terminate specific thread.
* @thread_id: Specified thread id.
*/
void aos_terminate_thread(aos_thread_id thread_id)
{
    aos_status ret = AOS_ERROR;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

#ifdef CONFIG_RTOS_THREADX
    ret = (aos_status)tx_thread_terminate(&(g_thread_desc_tbl[thread_id]->tcb_ptr));
    /* All errors are caused by improper use this api, assert it for debug. */
    AOS_ASSERT(ret == AOS_SUCCESS);
#endif
}

/**
* aos_is_thread_created() - judge specific thread is created or not.
* @thread_id: Specified thread id.
* Return: AOS_TRUE means created, AOS_FALSE means not created.
*/
boolean aos_is_thread_created(aos_thread_id thread_id)
{
    if (thread_id >= AOS_MAX_THREADS)
        return AOS_FALSE;

    return g_thread_desc_tbl[thread_id] != AOS_NULL;
}

/**
 * aos_is_user_thread_context() - judge current thread is in user thread context
 * that means: 1. not in assert process
               1. not in initialization process
 *             2. not in ISR
 *             3. not in Timer thread context
 * Return: AOS_TRUE means user thread context, AOS_FALSE means others.
 */
boolean aos_is_user_thread_context(void)
{
    TX_THREAD *current_thread;

    /* Is system already assert */
    if (aic_sys_is_assert())
        return AOS_FALSE;

    /* Is the call from an ISR or Initialization */
    if (TX_THREAD_GET_SYSTEM_STATE() != ((u32) 0)) {
        return AOS_FALSE;
    } else {
        /* Pickup thread pointer */
        TX_THREAD_GET_CURRENT(current_thread)
        /* Is the current thread the timer thread */
        if (current_thread == &_tx_timer_thread) {
            return AOS_FALSE;
        }
    }

    return AOS_TRUE;
}

/**
 * aos_get_current_thread_name() - get current thread name.
 * Return: current thread name ptr, if no thread, return NULL.
 */
char* aos_get_current_thread_name(void)
{
    TX_THREAD *current_thread;

    TX_THREAD_GET_CURRENT(current_thread)
    if (!current_thread) {
        return NULL;
    }

    return current_thread->tx_thread_name;
}

void* aos_get_running_thread_stack_top(void)
{
    TX_THREAD *current_thread;

    /* Is the call from an ISR or Initialization */
    if (TX_THREAD_GET_SYSTEM_STATE() != ((u32) 0)) {
        return NULL;
    }

    TX_THREAD_GET_CURRENT(current_thread)
    if (!current_thread) {
        return NULL;
    }

    return current_thread->tx_thread_stack_start;
}

u32 aos_get_thread_extq_depth(aos_thread_id thread_id)
{
    if (thread_id >= AOS_MAX_THREADS)
        return 0;

#ifdef CONFIG_RTOS_THREADX
    if (!g_thread_desc_tbl[thread_id]->ext_qcb)
        return 0;

    return g_thread_desc_tbl[thread_id]->ext_qcb->tx_queue_enqueued;
#endif

    return 0;
}

boolean aos_is_thread_suspended(aos_thread_id thread_id)
{
    UINT state = g_thread_desc_tbl[thread_id]->tcb_ptr.tx_thread_state;

    return (TX_QUEUE_SUSP == state ||
            TX_MUTEX_SUSP == state ||
            TX_SEMAPHORE_SUSP == state ||
            TX_SUSPENDED  == state ||
            TX_SLEEP == state ||
            TX_TCP_IP == state);
}

boolean aos_is_queue_suspended(aos_thread_id thread_id)
{
    return (TX_QUEUE_SUSP == g_thread_desc_tbl[thread_id]->tcb_ptr.tx_thread_state);
}


boolean aos_is_thread_ready(aos_thread_id thread_id)
{
    return (TX_READY == g_thread_desc_tbl[thread_id]->tcb_ptr.tx_thread_state);
}

