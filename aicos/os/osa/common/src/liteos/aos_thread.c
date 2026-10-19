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
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19d407
#define LOG_MODULE    AMOD_AOS

#include "aic_type.h"
#include "los_compiler.h"
#include <aic_osa.h>
#include <aos_thread.h>
#include <aos_config.h>
#include <aic_assert.h>
#include "los_task.h"
#include "los_queue.h"

#ifdef CONFIG_RTOS_THREADX
typedef VOID (*tx_entry_function)(ULONG id);
#endif

aos_thread_desc *g_thread_desc_tbl[AOS_MAX_THREADS];
extern aos_thread_config g_thread_config_tbl[];
extern u32 get_thread_config_tbl_idx(aos_thread_id thread_id);
extern BOOL g_taskScheduled;
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

static void _create_thread_queue(aos_thread_id thread_id, char *thread_name,
                                       aos_standard_entry_func entry_func,
                                       u32 input, u32 stack_size,
                                       u32 intq_num, char *intq_name,
                                       u32 extq_num, char *extq_name,
                                       u32 priority, u32 auto_start)
{
    aos_status ret;
    u8 *stack_ptr, intq_ptr, extq_ptr;
    u32 los_task, intSave;
    u32 intq_id = INVALID_QUEUE_ID, extq_id = INVALID_QUEUE_ID;
    TSK_INIT_PARAM_S param = {0};
    aos_thread_desc *thread_desc;

    /* 1.malloc thread control block and stack area */
    thread_desc =
        (aos_thread_desc *)aos_malloc(sizeof(*thread_desc) + stack_size);
    stack_ptr = (u8 *)(thread_desc + 1);
    AIC_DUMMY_ASSERT(thread_desc);
    memset(thread_desc, 0, sizeof(*thread_desc));

    /* 2.malloc internal queue area and create thread internal queue */
    if (intq_num) {
        /* NOTE: Lite queue msgSize = maxMsgSize + sizeof(UINT32); */
        intq_ptr = (u8 *)aos_malloc(intq_num * (sizeof(aos_msg_ptr) + sizeof(u32)));
        AIC_DUMMY_ASSERT(intq_ptr);

        ret = LOS_QueueCreateStatic(intq_name, intq_num,
                                    &intq_id, intq_ptr, 0, sizeof(aos_msg_ptr));
        AIC_ASSERT (ret == LOS_OK);
        thread_desc->intq_ptr = intq_ptr;
        thread_desc->intq_id = intq_id;
    }

    /* 3.malloc external queue area and create thread external queue */
    if (extq_num) {
        /* NOTE: Lite queue msgSize = maxMsgSize + sizeof(UINT32); */
        extq_ptr = (u8 *)aos_malloc(extq_num * (sizeof(aos_msg_ptr) + sizeof(u32)));
        AIC_DUMMY_ASSERT(extq_ptr);

        ret = LOS_QueueCreateStatic(extq_name, extq_num,
                                    &extq_id, extq_ptr, 0, sizeof(aos_msg_ptr));
        AIC_ASSERT (ret == LOS_OK);
        thread_desc->extq_id = extq_ptr;
        thread_desc->extq_id = extq_id;
    }

    /* 4.create thread with TX_DONT_START to avoid preempt */
    param.pfnTaskEntry = (TSK_ENTRY_FUNC)entry_func;
    param.usTaskPrio = priority;
    param.uwArg = input;
    param.stackAddr = (UINTPTR)stack_ptr;
    param.uwStackSize = stack_size;
    param.pcName = thread_name;
    param.uwResved = 0;
    ret = LOS_TaskCreateOnly(&los_task, &param);
    AIC_ASSERT (ret == LOS_OK);
    thread_desc->los_task = los_task;

    intSave = LOS_IntLock();
    /* 5.set value to global variable */
    AIC_ASSERT(!g_thread_desc_tbl[thread_id]);
    g_thread_desc_tbl[thread_id] = thread_desc;
    LOS_IntRestore(intSave);

    if (auto_start == AOS_AUTO_START) {
        /* 7.resume the created thread after global variable set */
        ret = (aos_status)LOS_TaskResume(los_task);
        AOS_ASSERT(ret == LOS_OK);
    }
}

static u32 adjust_los_task_prio(u32 prio, TSK_ENTRY_FUNC func)
{
    u32 ret = prio;

    if ((prio == OS_TASK_PRIORITY_LOWEST)
        && (func != OS_IDLE_TASK_ENTRY)) {
        ret = OS_TASK_PRIORITY_LOWEST - 1;
    } else if (prio > OS_TASK_PRIORITY_LOWEST) {
        ret = OS_TASK_PRIORITY_LOWEST - 1;
    }
    return ret;
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
                                void *input, u32 stack_size,
                                u32 queue_num, char *queue_name,
                                u32 priority, u32 auto_start)
{
    u32 index;
    aos_thread_id thread_id = AOS_INVALID_THREAD_ID;

    if (!thread_name || !entry_func || !stack_size)
        return AOS_INVALID_THREAD_ID;

    /* adjust task priority for los */
    priority = adjust_los_task_prio(priority, (TSK_ENTRY_FUNC)entry_func);

    if (priority >= AOS_MAX_PRIORITY)
        return AOS_INVALID_THREAD_ID;

    for (index = AOS_STANDARD_THREADS; index < AOS_MAX_THREADS; index++) {
         if (!g_thread_desc_tbl[index])
             break;
     }

    if (index == AOS_MAX_THREADS)
        return AOS_INVALID_THREAD_ID;

    thread_id = (aos_thread_id)index;

    _create_thread_queue(thread_id, thread_name,
                         (aos_standard_entry_func)entry_func,
                         (u32)input, stack_size,
                         0, AOS_NULL,
                         queue_num, queue_name,
                         priority, auto_start);

    return thread_id;
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
    aos_status ret;

    if (thread_id >= AOS_STANDARD_THREADS || priority >= AOS_MAX_PRIORITY)
        return AOS_ERROR;

    if (!thread_name || !stack_size)
        return AOS_ERROR;

    AIC_ASSERT(!g_thread_desc_tbl[thread_id]);

    _create_thread_queue(thread_id, thread_name,
                         entry_func,
                         (u32)input, stack_size,
                         intq_num, intq_name,
                         extq_num, extq_name,
                         priority, auto_start);

    return AOS_SUCCESS;
}

/**
* aos_start_thread() - Start a thread.
* @thread_id: Specified thread id.
*/
aos_status aos_start_thread(aos_thread_id thread_id)
{
    aos_status ret = AOS_SUCCESS;
    u32 task;
    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    if (!g_thread_desc_tbl[thread_id])
        return AOS_ERROR;

    task = g_thread_desc_tbl[thread_id]->los_task;
    ret = LOS_TaskResume(task);

    if (ret != LOS_OK) {
        aicx_error(0x118d7a, "sd", "%s, resume thread ret=%d\n", LOS_TaskNameGet(task), ret);
    }

    return ret;
}

/**
* aos_delete_thread() - Delete thread.
* @thread_id: Specified thread id.
*/
void aos_delete_thread(aos_thread_id thread_id)
{
    u32 los_task, intSave, intq_id, extq_id;
    aos_thread_desc *thread_desc;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);

    intSave = LOS_IntLock();
    thread_desc = g_thread_desc_tbl[thread_id];
    if (!thread_desc) {
        LOS_IntRestore(intSave);
        return;
    }

    g_thread_desc_tbl[thread_id] = AOS_NULL;
    LOS_IntRestore(intSave);

    los_task = thread_desc->los_task;
    intq_id = thread_desc->intq_id;
    extq_id = thread_desc->extq_id;

    /* 1.free and delete tcb */
    (void)LOS_TaskDelete(los_task);


    /* 2.free and delete queue and qcb */
    if (intq_id != INVALID_QUEUE_ID) {
        (void)LOS_QueueDelete(intq_id);
    }
    if (thread_desc->intq_ptr)
        aos_free(thread_desc->intq_ptr);

    if (extq_id != INVALID_QUEUE_ID) {
        (void)LOS_QueueDelete(extq_id);
    }
    if (thread_desc->extq_ptr)
        aos_free(thread_desc->extq_ptr);

    /* 3.free thread */
    aos_free(thread_desc);

}

bool aos_hook_delete_thread(u32 los_task)
{
    u32 aos_thread_id;

    if (INVALID_TASK_ID == los_task) {
        return false;
    }
    for (aos_thread_id = 0; aos_thread_id < AOS_MAX_THREADS; aos_thread_id++ ) {
        if (g_thread_desc_tbl[aos_thread_id]) {
            if (g_thread_desc_tbl[aos_thread_id]->los_task == los_task) {
                aos_delete_thread(aos_thread_id);
                return true;
            }
        }
    }
    return false;
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
    u32 ret;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

    *old_priority = LOS_TaskPriGet(g_thread_desc_tbl[thread_id]->los_task);
    ret = LOS_TaskPriSet(g_thread_desc_tbl[thread_id]->los_task,
                                    new_priority);
    /* All errors are caused by improper use this api, assert it for debug. */
    AOS_ASSERT(ret == LOS_OK);
}

/**
* aos_identify_thread() - get current thread id.
* Return: thread id.
*/
aos_thread_id aos_identify_thread(void)
{
    u32 index, thread_id = AOS_INVALID_THREAD_ID;
    u32 los_task;

    los_task = LOS_CurTaskIDGet();

    if (los_task == LOS_ERRNO_TSK_ID_INVALID)
        return thread_id;

    for(index = 0; index < AOS_MAX_THREADS; index++) {
        if((g_thread_desc_tbl[index]->los_task) == los_task) {
            thread_id = index;
            break;
        }
    }

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

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);
    thread_desc = g_thread_desc_tbl[thread_id];

    /* The return status of aos is fully compatibe with threadx. */
    ret = LOS_TaskSuspend(thread_desc->los_task);
    /* can't be TX_THREAD_ERROR, other error return to user. */
    AOS_ASSERT(ret != LOS_ERRNO_TSK_SUSPEND_LOCKED);
    if (ret != LOS_OK)
        aicx_error(0x177353, "sd", "%s: suspend ret=%d\n",
                   LOS_TaskNameGet(thread_desc->los_task), ret);

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

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);
    thread_desc = g_thread_desc_tbl[thread_id];

    /* The return status of aos is fully compatibe with threadx. */
    ret = LOS_TaskResume(thread_desc->los_task);
    /* can't be TX_THREAD_ERROR, other error return to user. */
    if (ret != LOS_OK)
        aicx_error(0x16ea5e, "sd", "%s: resume ret=%d\n",
                   LOS_TaskNameGet(thread_desc->los_task), ret);

    return ret;
}

/**
* aos_sleep() - force specific thread to sleep for time_ms.
* @time_ms: time_ms ms to sleep.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_sleep(u32 time_ms)
{
    LOS_Msleep(time_ms);
    return AOS_SUCCESS;
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

    LOS_Msleep(time_ms);
    return AOS_SUCCESS;
}

/**
* aos_terminate_thread() - terminate specific thread.
* @thread_id: Specified thread id.
*/
void aos_terminate_thread(aos_thread_id thread_id)
{
    return aos_delete_thread(thread_id);
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

    return g_thread_desc_tbl[thread_id] = AOS_NULL;
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
    /* Is system already assert */
    if (aic_sys_is_assert())
        return AOS_FALSE;

    /* Is the call from an ISR or Initialization */
    if ((OS_INT_ACTIVE) || (!g_taskScheduled)) {
        return AOS_FALSE;
    } else {
        /* Is the current thread the timer thread */
        if (LOS_CurTaskIDGet() == g_swtmrTaskID) {
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
    return LOS_CurTaskNameGet();

}

u32 aos_get_thread_extq_depth(aos_thread_id thread_id)
{
    QUEUE_INFO_S q_info;
    u32 ret;

    if (thread_id >= AOS_MAX_THREADS)
        return 0;

    if (!g_thread_desc_tbl[thread_id]->extq_id)
        return 0;

    ret = LOS_QueueInfoGet(g_thread_desc_tbl[thread_id]->extq_id, &q_info);
    if (ret != LOS_OK)
        return 0;

    return q_info.readableCnt;
}

boolean aos_is_thread_suspended(aos_thread_id thread_id)
{
    u32 ret;
    u32 status;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

    ret = LOS_TaskStatusGet(g_thread_desc_tbl[thread_id]->los_task, &status);
    if (ret == LOS_OK) {
        return (status &  (OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND |
            OS_TASK_STATUS_SUSPEND | OS_TASK_STATUS_PEND_TIME));
    } else {
        return false;
    }
}

boolean aos_is_queue_suspended(aos_thread_id thread_id)
{
    u32 ret;
    u32 status;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

    /* TODO, PHY need the interface, only queue suspend. */
    ret = LOS_TaskStatusGet(g_thread_desc_tbl[thread_id]->los_task, &status);
    if (ret == LOS_OK) {
        return (status &  (OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND |
            OS_TASK_STATUS_SUSPEND | OS_TASK_STATUS_PEND_TIME));
    } else {
        return false;
    }
}

boolean aos_is_thread_ready(aos_thread_id thread_id)
{
    u32 ret;
    u32 status;

    AOS_ASSERT(thread_id < AOS_MAX_THREADS);
    AOS_ASSERT(g_thread_desc_tbl[thread_id]);

    ret = LOS_TaskStatusGet(g_thread_desc_tbl[thread_id]->los_task, &status);
    if (ret == LOS_OK) {
        return (status & OS_TASK_STATUS_READY);
    } else {
        return false;
    }
}