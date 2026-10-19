/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA Interface.
 */

#ifndef __AIC_OSA_H__
#define __AIC_OSA_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_type.h>
#include <aic_assert.h>

#ifdef WIN32
#include "stdio.h"
#include <aos_win32_dummy.h>
/* WIN32 enable some feature */
#define CONFIG_MEM_BLOCK_POOL

#define CONFIG_FSMMSG_FEATURE
#define CONFIG_OSA_RMTE_MSG

#ifndef CONFIG_AOS_INT_DEBUG
#define CONFIG_AOS_INT_DEBUG
#endif

#ifndef CONFIG_RTOS_THREADX
#define CONFIG_RTOS_THREADX
#endif
#endif

#include <alog_api.h>
#include <aos_utl.h>
#include <aic_module.h>

/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/
/* common return value */
#define AOS_NULL                         ((void *) 0)
#define AOS_TRUE                         AIC_TRUE
#define AOS_FALSE                        AIC_FALSE
#define AOS_ERROR                        ((u32) 0xFFFFFFFF)
#define AOS_SUCCESS                      ((u32) 0x00)
#define AOS_DELETED                      ((u32) 0x01)
#define AOS_POOL_ERROR                   ((u32) 0x02)
#define AOS_PTR_ERROR                    ((u32) 0x03)
#define AOS_WAIT_ERROR                   ((u32) 0x04)
#define AOS_SIZE_ERROR                   ((u32) 0x05)
#define AOS_GROUP_ERROR                  ((u32) 0x06)
#define AOS_NO_EVENTS                    ((u32) 0x07)
#define AOS_OPTION_ERROR                 ((u32) 0x08)
#define AOS_QUEUE_ERROR                  ((u32) 0x09)
#define AOS_QUEUE_EMPTY                  ((u32) 0x0A)
#define AOS_QUEUE_FULL                   ((u32) 0x0B)
#define AOS_SEMAPHORE_ERROR              ((u32) 0x0C)
#define AOS_NO_INSTANCE                  ((u32) 0x0D)
#define AOS_THREAD_ERROR                 ((u32) 0x0E)
#define AOS_PRIORITY_ERROR               ((u32) 0x0F)
#define AOS_NO_MEMORY                    ((u32) 0x10)
#define AOS_START_ERROR                  ((u32) 0x10)
#define AOS_DELETE_ERROR                 ((u32) 0x11)
#define AOS_RESUME_ERROR                 ((u32) 0x12)
#define AOS_CALLER_ERROR                 ((u32) 0x13)
#define AOS_SUSPEND_ERROR                ((u32) 0x14)
#define AOS_TIMER_ERROR                  ((u32) 0x15)
#define AOS_TICK_ERROR                   ((u32) 0x16)
#define AOS_ACTIVATE_ERROR               ((u32) 0x17)
#define AOS_THRESH_ERROR                 ((u32) 0x18)
#define AOS_SUSPEND_LIFTED               ((u32) 0x19)
#define AOS_WAIT_ABORTED                 ((u32) 0x1A)
#define AOS_WAIT_ABORT_ERROR             ((u32) 0x1B)
#define AOS_MUTEX_ERROR                  ((u32) 0x1C)
#define AOS_NOT_AVAILABLE                ((u32) 0x1D)
#define AOS_NOT_OWNED                    ((u32) 0x1E)
#define AOS_INHERIT_ERROR                ((u32) 0x1F)
#define AOS_NOT_DONE                     ((u32) 0x20)
#define AOS_CEILING_EXCEEDED             ((u32) 0x21)
#define AOS_INVALID_CEILING              ((u32) 0x22)
#define AOS_FEATURE_NOT_ENABLED          ((u32) 0xFF)

/* enum, union, const, static type */
#define ENUM                        enum
#define UNION                       union
#define CONST                       const
#define STATIC                      static
#define NULLPTR                     (void *)0

/* invalid id */
#define AOS_INVALID_ID              0xFFFFFFFF

/* wait param */
#define AOS_NO_WAIT                 0
#define AOS_WAIT_FOREVER            0xFFFFFFFF

/* thread */
#define AOS_INVALID_THREAD_ID       (u8)0xFF
#define AOS_AUTO_START              1
#define AOS_DONT_START              0

/* mutex */
#define AOS_INHERIT                 1
#define AOS_NO_INHERIT              0

/* timer */
#define AOS_AUTO_ACTIVATE           1
#define AOS_NO_ACTIVATE             0
#define AOS_BE_PERIOD               1
#define AOS_NO_PERIOD               0

/* event flag group */
#define AOS_OR                      0
#define AOS_OR_CLEAR                1
#define AOS_AND                     2
#define AOS_AND_CLEAR               3

#ifdef CONFIG_FILENAME_TO_ID
typedef   unsigned int  filename_t;
#define CUR_CODE_POS    AIC_THIS_FILE, __LINE__
#else
typedef   char *        filename_t;
#define CUR_CODE_POS    __FILE__, __LINE__
#endif

#define AOS_SEMAPHORE_ID 0x53454D41UL
#define AOS_QUEUE_ID     0x51554555UL
#define AOS_MUTEX_ID     0x4D555445UL

/* ---message--- */
/* get src thread id from fsm msg */
#define FSM_SRC_THREAD_ID(fsm_msg_ptr)        ((fsm_msg_ptr)->src_thread_id)
/* get src process id from fsm msg */
#define FSM_SRC_PROCESS_ID(fsm_msg_ptr)     ((fsm_msg_ptr)->src_process_id)
/* get dest thread id from fsm msg */
#define FSM_DEST_THREAD_ID(fsm_msg_ptr)       ((fsm_msg_ptr)->dest_thread_id)
/* get dest process id from fsm msg */
#define FSM_DEST_PROCESS_ID(fsm_msg_ptr)    ((fsm_msg_ptr)->dest_process_id)
#define FSM_SAP_ID(fsm_msg_ptr)             ((fsm_msg_ptr)->sap_id)
#define FSM_PRIMITIVE_ID(fsm_msg_ptr)       ((fsm_msg_ptr)->prim_id)
#define FSM_SRC_INSTANCE_ID(fsm_msg_ptr)    ((fsm_msg_ptr)->src_instance_id)
#define FSM_DEST_INSTANCE_ID(fsm_msg_ptr)   ((fsm_msg_ptr)->dest_instance_id)
/* get parameters and peer info from fsm msg */
#define FSM_PARAM_SIZE(fsm_msg_ptr)\
        ((fsm_msg_ptr)->parameters.parameter_len)
#define FSM_PARAM_PTR(fsm_msg_ptr)\
        ((fsm_msg_ptr)->parameters.parameter_ptr)
#define FSM_PEER_BUF_SIZE(fsm_msg_ptr)\
        ((fsm_msg_ptr)->peer_msg.peer_msg_len)
#define FSM_PEER_BUF_FREE_SIZE(fsm_msg_ptr)\
        ((fsm_msg_ptr)->peer_msg.free_header_space_len)
#define FSM_PEER_BUF_PTR(fsm_msg_ptr)\
        (((fsm_msg_ptr)->peer_msg.peer_msg_ptr))
#define FSM_USED_PEER_BUF_PTR(fsm_msg_ptr)\
        ((((u8 *)FSM_PEER_BUF_PTR(fsm_msg_ptr)) +\
        (FSM_PEER_BUF_FREE_SIZE(fsm_msg_ptr))/8))
#define FSM_NEEDED_PEER_HEAD_PTR(fsm_msg_ptr,head_size)\
        ((((u8 *)FSM_PEER_BUF_PTR(fsm_msg_ptr)) -\
        (FSM_PEER_BUF_FREE_SIZE(fsm_msg_ptr) -\
        head_size)/8))
/* get timestamp from fsm msg */
#define FSM_TIMESTAMP(fsm_msg_ptr)          ((fsm_msg_ptr)->timestamp)
/* get dest rat type from fsm msg */
#define FSM_DEST_RAT_TYPE(fsm_msg_ptr)\
        ((fsm_msg_ptr)->parameters.dest_rat_type)

/* filler macro for structure */
#define AOS_FILLER1             u8 filler1_1;
#define AOS_FILLER2             u8 filler2_1; u8 filler2_2;
#define AOS_FILLER3             u8 filler3_1; u8 filler3_2; u8 filler3_3;
#define AOS_FILLER1_VALUE       0
#define AOS_FILLER2_VALUE       0, 0
#define AOS_FILLER3_VALUE       0, 0, 0

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/

/* status */
typedef u32 aos_status;

/* thread */
typedef u8 aos_thread_id;
typedef u8 aos_process_id;         /* just for fsm message */
typedef void (*aos_entry_func)(void *input);
typedef void (*aos_standard_entry_func)(void *input);
typedef struct thread_info_st {
    void *handler;
    const char *name;
    u32 number;
    u32 state;
    u32 curr_priority;
    u32 base_priority;
    void *stack_base;
    u32 stack_alloc_size;
    u32 statck_min_remained;
} thread_info_t;


/* timer */
typedef void *aos_timer_ptr;
typedef void (*aos_expiration_func)(void *input);
typedef u32  aos_timer_value;

typedef void *aos_rlx_timer_ptr;

typedef enum {
    RLX_TMR_IDLE = 0,
    RLX_TMR_ACTIVE,
    RLX_TMR_REWORK,
    RLX_TMR_STRICT,
    RLX_TMR_TIMEOUT,
    RLX_TMR_DEACTIVE,
    RLX_TMR_DELETE
} AOS_RLX_TMR_ST;

typedef struct AOS_RELAXED_TMR_STRUCT
{
    /* Define the remaining ticks and re-initialization tick values.  */
    u32 relaxed_time;
    u32 strict_time;
    /* Define the timeout function and timeout function parameter.  */
    aos_expiration_func func;
    void *input;
    /* Define relaxed timer status */
    AOS_RLX_TMR_ST status;
    /* Define the relaxed and strict timer.  */
    aos_timer_ptr relaxed_timer;
    aos_timer_ptr strict_timer;
    /* Define the next and previous link pointers for active relaxed timers */
    struct AOS_RELAXED_TMR_STRUCT *rel_next, *rel_previous;
    /* Define period status */
    boolean b_period;
} AOS_RELAXED_TMR;

/* mutex */
typedef void *aos_mutex_ptr;
typedef u8 *aos_mutex_name;

/* semaphore */
typedef void *aos_semaphore_ptr;

/* event flag group */
typedef void *aos_event_ptr;

/* mutex */
typedef void *aos_queue_ptr;

/* message */
typedef void    *aos_msg_ptr;
typedef u32     aos_sap_id;         /* just for fsm message */
typedef u32     aos_prim_id;        /* just for fsm message */
typedef u16     aos_instance_id;    /* just for fsm message */
typedef u32     aos_timestamp;      /* just for fsm message */

/* fsm message peer */
typedef struct aos_peer_msg_st {
    u16             peer_msg_len;
    u16             free_header_space_len;
    void*           peer_msg_ptr;
} aos_peer_msg;

/* fsm message parameters */
typedef struct aos_parameters_st {
    u16               parameter_len;
    u8                dest_rat_type;  /* no use */
    u8                pos; /* store the pos in fsm block pool. */
    void*             parameter_ptr;
} aos_parameters;

/* fsm message */
typedef struct aos_fsm_msg_st {
    aos_thread_id     src_thread_id;
    aos_process_id    src_process_id;
    aos_thread_id     dest_thread_id;
    aos_process_id    dest_process_id;
    aos_sap_id        sap_id;
    aos_prim_id       prim_id;
    aos_instance_id   src_instance_id;  /*no use*/
    aos_instance_id   dest_instance_id; /*no use*/
    aos_parameters    parameters;
    aos_peer_msg      peer_msg;
    aos_timestamp     timestamp;
} aos_fsm_msg;

typedef struct aos_fsmmsgtimer_st {
    aos_fsm_msg *msg;
    aos_timer_ptr *timer;
    boolean b_period;
    boolean b_trace;
    boolean b_stop;
} aos_fsmmsgtimer;

/* fsm handler */
typedef aos_status (* aos_fsm_handler)(aos_fsm_msg *fsm_msg_ptr);

#if defined(WIN32) && (defined(CONFIG_COMM_PS) || defined(CONFIG_COMM_PHY))
/* msg filter function */
typedef u8 (*aos_filter_func)(const aos_fsm_msg*);
#define AOS_CONFIG_FILTER(call_back_func) (aos_config_filter(call_back_func))
#endif

/* fsm state table */
typedef struct aos_fsm_state_tbl_st
{
    aos_fsm_handler fsm_handler;
} aos_fsm_state_tbl;

typedef aos_status (*aos_func_init)(void);
typedef aos_status (*aos_func_main)(aos_fsm_msg *);
typedef aos_status (*aos_func_delete)(void);
typedef aos_status (*aos_func_reset)(void);
typedef struct aos_process_desc_tbl_st
{
    aos_thread_id        thread_id;
    u8                   module_nums;
    AOS_FILLER2
    struct task_func
    {
        aos_func_init    func_init;
        aos_func_main    func_main;
        aos_func_reset   func_reset;
        aos_func_delete  func_delete;
    } process_info[15];
} aos_process_desc_tbl;

/* time */
typedef struct aos_systim_st {
    u32               year;
    u32               month;
    u32               wday;
    u32               day;
    u32               hour;
    u32               minute;
    u32               second;
    u32               msecond;
    u32               usecond;
} aos_systime;

/**---------------------------------------------------------------------------*/
/**                          function prototype                               */
/**---------------------------------------------------------------------------*/
/* ---thread--- */
/**
* aos_init_thread_desc_tbl() - initialize global thread table.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_init_thread_desc_tbl(void);

/**
* aos_init_standard_thread() - create aos standard threads, which are described
* in g_thread_config_tbl in aos_config.c .
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_init_standard_thread(void);
/**
* common thread entry for standard thread
*/
void aos_standard_thread_entry(void *input);
/**
* aos_create_thread() - create common thread.
* @thread_name: specified thread name.
* @entry_func: specified thread entry func.
* @input: specified input param for thread entry func.
* @stack_size: specified stack size.
* @queue_num: specified external queue capacity, default message size is
* sizeof(aos_msg_ptr).
* @queue_name: specified external queue name.
* @priority: specified thread priority.
* @auto_start: specified thread automatic start selection.
*
* Return: thread id.
*/
aos_thread_id aos_create_thread(char *thread_name,
                                aos_entry_func entry_func,
                                void *input, u32 stack_size,
                                u32 queue_num, char *queue_name,
                                u32 priority, u32 autostart);

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
                                u32 priority, u32 auto_start);
/**
* aos_create_thread_simply() - create common thread simply.
* @thread_name: specified thread name.
* @entry_func: specified thread entry func.
* @input: specified input param for thread entry func.
* @stack_size: specified stack size.
* @priority: specified thread priority.
*
* Return: thread id.
*/
#define aos_create_thread_simply(name, func, input, stack_size, priority) \
    aos_create_thread(name, func, input, stack_size, 0, AOS_NULL, priority, AOS_AUTO_START)

/**
* aos_create_thread_simply() - create common thread simply.
* @thread_name: specified thread name.
* @entry_func: specified thread entry func.
* @input: specified input param for thread entry func.
* @stack_start: Specified stack start.
* @stack_size: specified stack size.
* @priority: specified thread priority.
*
* Return: thread id.
*/
#define aos_create_thread_simply_ex(name, func, input, stack_start, stack_size, priority) \
    aos_create_thread_ex(name, func, input, stack_start, stack_size, 0, AOS_NULL, priority, AOS_AUTO_START)


/**
* aos_create_thread_nostart() - create a simply thread, but don't start it automatically.
* @thread_name: specified thread name.
* @entry_func: specified thread entry func.
* @input: specified input param for thread entry func.
* @stack_size: specified stack size.
* @priority: specified thread priority.
*
* Return: thread id.
*/
#define aos_create_thread_nostart(name, func, input, stack_size, priority) \
    aos_create_thread(name, func, input, stack_size, 0, AOS_NULL, priority, AOS_DONT_START)

/**
* aos_start_thread() - Start a thread.
* @thread_id: Specified thread id.
*/
aos_status aos_start_thread(aos_thread_id thread_id);

/**
* aos_create_standard_thread() - create standard thread.
* @thread_id: specified thread id. Note: thread id must < AOS_STANDARD_THREADS
* @thread_name: specified thread name.
* @entry_func: specified thread entry func.
* @input: specified input param for thread entry func.
* @stack_size: specified stack size.
* @intq_num: specified internal queue capacity, default message size is
* sizeof(aos_msg_ptr).
* @intq_name: specified internal queue name.
* @extq_num: specified external queue capacity, default message size is
* sizeof(aos_msg_ptr).
* @extq_name: specified external queue name.
* @priority: specified thread priority.
* @auto_start: specified thread automatic start selection.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_create_standard_thread(aos_thread_id thread_id, char *thread_name,
                               aos_standard_entry_func entry_func,
                               void *input, u32 stack_size,
                               u32 intq_num, char *intq_name,
                               u32 extq_num, char *extq_name,
                               u32 priority, u32 auto_start);
typedef u32 (*standard_thread_ptr)(aos_thread_id thread_id, char *thread_name,
                                aos_standard_entry_func entry_func,
                                void *input, u32 stack_size,
                                u32 intq_num, char *intq_name,
                                u32 extq_num, char *extq_name,
                                u32 priority, u32 auto_start);
/**
* aos_delete_thread() - Delete thread.
* @thread_id: specified thread id.
*/
void aos_delete_thread(aos_thread_id thread_id);

/**
* aos_get_thread_pri() - get thread priority.
* @thread_id: specified thread id.
* @npriority: return priority
*/
void aos_get_thread_pri(aos_thread_id thread_id, u32 *priority);

/**
* aos_hook_delete_thread() - hook function for delete task.
* @task: specified los task id.
* Return: true thread deleted by aos thread sys; false not deleted.
*/
bool aos_hook_delete_thread(u32 task);

/**
* aos_change_current_thread_pri() - change current thread priority.
* @new_priority: new priority to set.
* @old_priority: return old priority.
*/
void aos_change_current_thread_pri(u32 new_priority,
                          u32 *old_priority);

/**
* aos_change_thread_pri() - change thread priority.
* @thread_id: specified thread id.
* @new_priority: new priority to set.
* @old_priority: return old priority.
*/
void aos_change_thread_pri(aos_thread_id thread_id, u32 new_priority,
                          u32 *old_priority);

/**
* aos_get_thread_count() - get thread count.
* Return: count fo means success, AOS_ERROR means error.
*/
u32 aos_get_thread_count();

/**
* aos_get_all_thread_status() - get all thread info.
* @info: pointer to thread_info_t array.
* @count: size of thread_info_t array.
* Return: actual count of threads.
*/
u32 aos_get_all_thread_status(thread_info_t *info, u32 count);
/**
* aos_identify_thread() - get current thread id.
* Return: thread id.
*/
aos_thread_id aos_identify_thread(void);
/**
* aos_suspend_thread() - suspend specific thread.
* @thread_id: specified thread id.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_suspend_thread(aos_thread_id thread_id);
/**
* aos_resume_thread() - resume specific thread.
* @thread_id: specified thread id.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_resume_thread(aos_thread_id thread_id);
/**
* aos_terminate_thread() - terminate specific thread.
* @thread_id: specified thread id.
*/
void aos_terminate_thread(aos_thread_id thread_id);

/**
* aos_is_thread_created() - judge specific thread is created or not.
* @thread_id: specified thread id.
* Return: AOS_SUCCESS means created, AOS_ERROR means not created.
*/
boolean aos_is_thread_created(aos_thread_id thread_id);

/**
 * aos_is_user_thread_context() - judge current thread is in user thread context
 * that means: 1. not in assert process
               1. not in initialization process
 *             2. not in ISR
 *             3. not in Timer thread context
 * Return: AOS_TRUE means user thread context, AOS_FALSE means others.
 */
boolean aos_is_user_thread_context(void);

/**
* aos_is_thread_suspended() - judge specific thread is suspend or not.
* @thread_id: specified thread id.
* Return: true means suspend, false means not.
*/
boolean aos_is_thread_suspended(aos_thread_id thread_id);

/**
* aos_is_queue_suspended() - judge specific thread is suspend on a queue.
* @thread_id: specified thread id.
* Return: true means suspend, false means not.
*/
boolean aos_is_queue_suspended(aos_thread_id thread_id);

/**
* aos_is_thread_ready() - judge specific thread is ready or not.
* @thread_id: specified thread id.
* Return: true means ready, false means not.
*/
boolean aos_is_thread_ready(aos_thread_id thread_id);

/**
 * aos_get_current_thread_name() - get current thread name.
 * Return: current thread name ptr, if no thread, return NULL.
 */
char* aos_get_current_thread_name(void);

/**
 * aos_get_running_thread_stack_top() - get the running thread stack top.
 * Return: the stack top of runnning thread, if ti is not a thread context,
 * will return NULL.
 */
void* aos_get_running_thread_stack_top(void);

/**
 * aos_get_thread_extq_depth() - get the depth of a thread extern queue.
 * @thread_id: specified thread id.
 * Return: the depth of a thread extern queue.
 */
u32 aos_get_thread_extq_depth(aos_thread_id thread_id);

/* ---timer--- */
/**
* aos_create_timer() - create specified timer.
* @name_ptr: specified timer name.
* @expiration_func: specified timer expiration function.
* @input: Expiration function input parameter.
* @expiration_time: Expiration time (ms).
* @auto_activate: AOS_AUTO_ACTIVATE or AOS_NO_ACTIVATE.
* @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
*
* Return: Valid timer ptr means success, AOS_NULL means error.
*/
aos_timer_ptr aos_create_timer(char *name_ptr,
                               aos_expiration_func expiration_func, void *input,
                               u32 expiration_time, u32 auto_activate,
                               boolean b_period);

/**
* aos_delete_timer() - delete specified timer.
* @timer_ptr: specified timer ptr.
*/
void aos_delete_timer(aos_timer_ptr timer_ptr);
/**
* aos_activate_timer() - activate specified timer.
* @timer_ptr: specified timer ptr.
*/
void aos_activate_timer(aos_timer_ptr timer_ptr);
/**
* aos_deactivate_timer() - deactivate specified timer.
* @timer_ptr: specified timer ptr.
*/
void aos_deactivate_timer(aos_timer_ptr timer_ptr);
/**
* aos_reactive_timer() - Reactive specified timer.
* @timer_ptr: Specified timer ptr.
* @expiration_time: Expiration time (ms).
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
void aos_reactive_timer(aos_timer_ptr timer_ptr, u32 expiration_time);
/**
* aos_change_timer() - change specified timer's  expiration time.
* @timer_ptr: specified timer ptr.
* @expiration_time: expiration time (ms).
* @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
*/
void aos_change_timer(aos_timer_ptr timer_ptr, u32 expiration_time,
                      boolean b_period);
/**
* aos_get_remaining_time() - get remaining time of specified timer.
* @timer_ptr: specified timer pointer.
* @value_ptr: value pointer to save remaining time.
*/
void aos_get_remaining_time(aos_timer_ptr timer_ptr, u32 *value_ptr);
/**
* aos_get_current_remaining_time() - Get remaining time of current timer.
* @value_ptr: Value pointer to save remaining time.
*/
void aos_get_current_remaining_time(u32 *value_ptr);
/**
* aos_is_timer_created() - judge specified timer is created or not.
* @timer_ptr: specified timer pointer.
*
* Return: AOS_TURE means timer created, AOS_FALSE means not created.
*/
boolean aos_is_timer_created(aos_timer_ptr timer_ptr);
/**
 * aos_get_timer_info() - get timer info of specified timer.
 * @timer_ptr: specified timer pointer.
 * @name: to save the timer name.
 * @active: to save timer active status.
 * @remaining: to save current remaining time(ms).
 * @reschedule: to save reschedule time(ms) fo period timer.
 *
 * Return: AOS_SUCCESS means success , others means error.
 */
aos_status aos_get_timer_info(aos_timer_ptr timer_ptr, char **name, u32 *active,
                              u32 *remaining, u32 *reschedule);
/**
 * aos_create_relaxed_timer() - Create relaxed timer.
 * @name_ptr: Relaxed timer name.
 * @expiration_func: Specified timer expiration function.
 * @input: Expiration function input parameter.
 * @relaxed_time: Expiration relaxed time (ms), which do not wakeup system.
 * @strict_time: Expiration strict time (ms), which will wakeup system.
 * @auto_activate: AOS_AUTO_ACTIVATE or AOS_NO_ACTIVATE.
 * @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
 *
 * Return: Valid timer ptr means success, AOS_NULL means error.
 */
aos_rlx_timer_ptr aos_create_relaxed_timer(char *name_ptr,
                            aos_expiration_func expiration_func, void *input,
                            u32 relaxed_time, u32 strict_time, u32 auto_activate,
                            boolean b_period);
/**
 * aos_delete_relaxed_timer() - Delete relaxed timer.
 * @timer_ptr: Relaxed timer ptr.
 */
void aos_delete_relaxed_timer(aos_rlx_timer_ptr timer_ptr);
/**
 * aos_activate_relaxed_timer() - Activate relaxed timer.
 * @timer_ptr: Relaxed timer ptr.
 */
void aos_activate_relaxed_timer(aos_rlx_timer_ptr timer_ptr);
/**
 * aos_deactivate_relaxed_timer() - Deactivate relaxed timer.
 * @timer_ptr: Relaxed timer ptr.
 */
void aos_deactivate_relaxed_timer(aos_rlx_timer_ptr timer_ptr);
/**
 * aos_reactive_relaxed_timer() - Reactive relaxed timer.
 * @timer_ptr: Relaxed timer ptr.
 *
 * Return: AOS_SUCCESS means success, AOS_ERROR means error.
 */
void aos_reactive_relaxed_timer(aos_rlx_timer_ptr timer_ptr);
/**
 * aos_change_relaxed_timer() - Change relaxed timer's  expiration time.
 * @timer_ptr: Relaxed timer ptr.
 * @expiration_time: Expiration time (ms).
 * @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD..
 *
 * Return: AOS_SUCCESS means success, AOS_ERROR means error.
 */
void aos_change_relaxed_timer(aos_rlx_timer_ptr timer_ptr, u32 relaxed_time, u32 strict_time,
                            boolean b_period);
/**
 * aos_relaxed_timer_suspend() - Switch relaxed timer as strict timer.
 */
void aos_relaxed_timer_suspend(void);
/**
 * aos_relaxed_timer_resume() - Switch strict timer as relaxed timer.
 */
void aos_relaxed_timer_resume(void);
/**
 * aos_get_relaxed_timer() - Get current relaxed timer.
 */
aos_rlx_timer_ptr aos_get_relaxed_timer(void);

/**
* aos_create_fsm_timer() - create fsm timer with AOS_NO_ACTIVATE. It will
* provide default timeout value (1000ms) and default timeout func internally.
* @name_ptr: specified fsm timer name.
* @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
*
* Return: Valid timer ptr means success, AOS_NULL means error.
*/
aos_timer_ptr aos_create_fsm_timer(char *name_ptr, boolean b_period);
/**
* aos_start_fsm_timer() - create fsm timer with AOS_NO_ACTIVATE.
* @timer_ptr: specified fsm timer pointer.
* @expiration_time: specified fsm timer timeout value.
* @msg_ptr: specified fsm  msg ptr to send when timeout.
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_start_fsm_timer(aos_timer_ptr timer_ptr,
                               u32 expiration_time,
                               aos_fsm_msg* msg_ptr);
/**
* aos_stop_fsm_timer() - stop fsm timer.
* @timer_ptr: specified fsm timer pointer.
*/
void aos_stop_fsm_timer(aos_timer_ptr timer_ptr);
/**
* aos_delete_fsm_timer() - delete specified fsm timer.
* @timer_ptr: specified fsm timer ptr.
*/
void aos_delete_fsm_timer(aos_timer_ptr timer_ptr);

/**
* aos_create_fsmmsg_timer() - This function will create a fsmmsg and a timer.
*
* @name_ptr: specified fsm timer name.
* @parameter_len: specified parameter length.
* @peer_msg_len: specified peer length.
* @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
* @b_trace: if trace the msg.
*
* Return: Valid timer ptr means success, AOS_NULL means error.
*/
aos_fsmmsgtimer *aos_create_fsmmsg_timer(char *name_ptr,
                                         u16 parameter_len, u16 peer_msg_len,
                                         boolean b_period, boolean b_trace);

/**
* aos_start_fsm_timer() - start a msg timer.
* @ptr: the aos_fsmmsgtimer ptr that createed by aos_create_fsmmsg_timer.
* @expiration_time: specified fsmmsg timer timeout value.
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_start_fsmmsg_timer(aos_fsmmsgtimer *ptr, u32 expiration_time);
/**
* aos_stop_fsm_timer() - stop a fsmmsg timer.
* @ptr: the aos_fsmmsgtimer ptr that createed by aos_create_fsmmsg_timer.
*/
void aos_stop_fsmmsg_timer(aos_fsmmsgtimer *ptr);

/**
* aos_delete_fsm_timer() - delete a fsmmsg timer.
* Will both delete the timer and the msg.
* @ptr: the aos_fsmmsgtimer ptr that createed by aos_create_fsmmsg_timer.
*/
void aos_delete_fsmmsg_timer(aos_fsmmsgtimer *ptr);

/**
* aos_is_fsm_timer_runing() - If aos timer is runing.
* runing means that the timer is expired.
* Attention: This api can't be used for an periodic timer.
*
* @timer_ptr: Specified fsm timer pointer.
*/
#ifdef CONFIG_RTOS_THREADX
#define aos_is_fsm_timer_runing(timer_ptr) \
    ( ((TX_TIMER*)(timer_ptr))->extension_ptr != NULL)
#else
    #define aos_is_fsm_timer_runing(timer_ptr)  FALSE
#endif

/* ---mutex--- */
/**
* aos_create_mutex() - create specified mutex.
* @name_ptr: specified mutex name
*
* Return: Valid mutex ptr means success, AOS_NULL means error.
*/
aos_mutex_ptr aos_create_mutex(char *name_ptr);
/**
* aos_delete_mutex() - delete specified mutex.
* @mutex_ptr: specified mutex ptr
*/
void aos_delete_mutex(aos_mutex_ptr mutex_ptr);
/**
* aos_get_mutex() - get specified mutex.
* @mutex_ptr: specified mutex ptr
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, other means error.
*/
aos_status aos_get_mutex(aos_mutex_ptr mutex_ptr, u32 wait_option);
/**
* aos_put_mutex() - put specified mutex.
* @mutex_ptr: specified mutex ptr.
*/
void aos_put_mutex(aos_mutex_ptr mutex_ptr);


/* ---semaphore--- */
/**
* aos_create_semaphore() - create specified semaphore.
* @name_ptr: specified semaphore name
* @initial_count: initial semaphore count
*
* Return: Valid semaphore ptr means success, AOS_NULL means error.
*/
aos_semaphore_ptr aos_create_semaphore(char *name_ptr, u32 initial_count);
/**
* aos_create_semaphore_simply() - create specified semaphore simply.
* @name_ptr: specified semaphore name
*
* Return: Valid semaphore ptr means success, AOS_NULL means error.
*/
#define aos_create_semaphore_simply(name_ptr) \
    aos_create_semaphore(name_ptr, 0)
/**
* aos_delete_semaphore() - delete specified semaphore.
* @semaphore_ptr: specified semaphore ptr.
*/
void aos_delete_semaphore(aos_semaphore_ptr semaphore_ptr);
/**
* aos_get_semaphore() - get specified semaphore.
* @semaphore_ptr: specified semaphore ptr
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_get_semaphore(aos_semaphore_ptr semaphore_ptr, u32 wait_option);
/**
* aos_put_semaphore() - put specified semaphore.
* @semaphore_ptr: specified semaphore ptr.
*/
void aos_put_semaphore(aos_semaphore_ptr semaphore_ptr);

/* ---event flag group--- */
/**
* aos_create_event() - create specified event.
* @name_ptr: specified event name.
*
* Return: Valid event ptr means success, AOS_NULL means error.
*/
aos_event_ptr aos_create_event(char *name_ptr);
/**
* aos_delete_event() - delete specified event.
* @event_ptr: specified event ptr.
*/
void aos_delete_event(aos_event_ptr event_ptr);
/**
* aos_get_event() - get specified event.
* @event_ptr: specified event ptr.
* @requested_flags: requested flags.
* @get_option: AOS_OR or AOS_OR_CLEAR or AOS_AND or AOS_AND_CLEAR.
* @actual_flags_ptr: actual flags of event flag group.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_get_event(aos_event_ptr event_ptr, u32 requested_flags, u32 get_option,
                  u32 *actual_flags_ptr, u32 wait_option);
/**
* aos_set_event() - set specified event.
* @event_ptr: specified event ptr.
* @flags_to_set: flags to set.
* @set_option: AOS_OR or AOS_AND.
*/
void aos_set_event(aos_event_ptr event_ptr, u32 flags_to_set, u32 set_option);


/* ---queue--- */
/**
* aos_create_queue() - create specified queue.
* @queue_name: specified queue name.
* @item_size: specified item size.
* @queue_num: specified queue capacity.
*
* Return: Valid queue ptr means success, AOS_NULL means error.
*/
aos_queue_ptr aos_create_queue(char *queue_name, u32 item_size, u32 queue_num);
/**
* aos_delete_queue() - delete specified queue.
* @queue_ptr: specified queue ptr
*/
void aos_delete_queue(aos_queue_ptr queue_ptr);
/**
* aos_recv_from_queue() - get item from specified queue.
* @queue_ptr: specified queue ptr
* @item_ptr: specified item ptr to store content from queue.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_recv_from_queue(aos_queue_ptr queue_ptr, void *item_ptr,
                               u32 wait_option);
/**
* aos_send_to_queue() - send specified item to queue tail.
* @queue_ptr: specified queue ptr
* @item_ptr: specified item ptr which points to the content to send
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_send_to_queue(aos_queue_ptr queue_ptr, void *item_ptr,
                             u32 wait_option);
/**
* aos_send_to_queue() - send specified item to queue head.
* @queue_ptr: specified queue ptr
* @item_ptr: specified item ptr which points to the content to send
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_send_to_queue_head(aos_queue_ptr queue_ptr, void *item_ptr,
                                  u32 wait_option);

/**
* aos_get_queue_used_count() - get queue used count.
* @queue_ptr: specified queue ptr
* @count: pointer to counter
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_get_queue_used_count(aos_queue_ptr queue_ptr, u32 *count);

/**
* aos_get_queue_available_count() - get queue available count.
* @queue_ptr: specified queue ptr
* @available_count: pointer to counter
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_get_queue_available_count(aos_queue_ptr queue_ptr, u32 *available_count);

/**
* aos_get_queue_item_check() - check item exist in queue.
* @queue_ptr: specified queue ptr
* @item_ptr: specified message item ptr
*
* Return: AOS_TRUE means exists, others means no.
*/
boolean aos_get_queue_item_check(aos_queue_ptr queue_ptr, void *item_ptr);

/**
* aos_queue_reset() - resets the specified queue.
* @queue_ptr: specified queue ptr
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_queue_reset(aos_queue_ptr queue_ptr);

/* ---message--- */
#define aos_fsm_msg_total_size(fsm) \
    (sizeof(*fsm) + FSM_PARAM_SIZE(fsm) + FSM_PEER_BUF_SIZE(fsm))

#define aos_init_fsm_msg_header(dst, v) do { \
    void *param = FSM_PARAM_PTR(dst); \
    void *peer = FSM_PEER_BUF_PTR(dst); \
 \
    memset(dst, v, sizeof(*dst)); \
    FSM_PARAM_PTR(dst) = param; \
    FSM_PEER_BUF_PTR(dst) = peer; \
} while (0)

#define aos_copy_fsm_msg_header(dst, src) do { \
    void *param = FSM_PARAM_PTR(dst); \
    void *peer = FSM_PEER_BUF_PTR(dst); \
 \
    memcpy(dst, src, sizeof(*dst)); \
    FSM_PARAM_PTR(dst) = param; \
    FSM_PEER_BUF_PTR(dst) = peer; \
} while (0)

#define aos_copy_fsm_msg(dst, src) do { \
    void *param = FSM_PARAM_PTR(dst); \
    void *peer = FSM_PEER_BUF_PTR(dst); \
 \
    memcpy(dst, src, sizeof(*dst)); \
    FSM_PARAM_PTR(dst) = param; \
    FSM_PEER_BUF_PTR(dst) = peer; \
    if (FSM_PARAM_SIZE(src)) \
        memcpy(FSM_PARAM_PTR(dst), FSM_PARAM_PTR(src), FSM_PARAM_SIZE(src)); \
    if (FSM_PEER_BUF_SIZE(src)) \
        memcpy(FSM_PEER_BUF_PTR(dst), FSM_PEER_BUF_PTR(src), FSM_PEER_BUF_SIZE(src)); \
} while (0)

#define aos_get_fsm_msg(para_len, peer_len)\
        aos_get_fsm_msg_debug(para_len, peer_len, CUR_CODE_POS)
#define aos_free_fsm_msg(fsm_msg_ptr)\
        aos_free_fsm_msg_debug(fsm_msg_ptr, CUR_CODE_POS)
#define aos_link_fsm_msg(fsm_msg_ptr)\
        aos_link_fsm_msg_debug(fsm_msg_ptr, CUR_CODE_POS)
#define aos_send_fsm_msg(fsm_msg_ptr)\
        aos_send_fsm_msg_debug(fsm_msg_ptr->dest_thread_id,\
                               fsm_msg_ptr, AOS_NO_WAIT, TRUE)
#define aos_send_fsm_msg_no_trace(fsm_msg_ptr)\
        aos_send_fsm_msg_debug(fsm_msg_ptr->dest_thread_id,\
                               fsm_msg_ptr, AOS_NO_WAIT, FALSE)
#define aos_send_fsm_msg_head(fsm_msg_ptr)\
        aos_send_fsm_msg_head_debug(fsm_msg_ptr->dest_thread_id,\
                                    fsm_msg_ptr, AOS_NO_WAIT, TRUE)
#define aos_send_fsm_msg_head_no_trace(fsm_msg_ptr)\
        aos_send_fsm_msg_head_debug(fsm_msg_ptr->dest_thread_id,\
                                    fsm_msg_ptr, AOS_NO_WAIT, FALSE)
#define aos_recv_fsm_msg_from_intq(thread_id, wait_option)\
        aos_recv_msg_from_intq(thread_id, (u32)wait_option)
#define aos_recv_fsm_msg_from_extq(thread_id, wait_option)\
        aos_recv_msg_from_extq(thread_id, (u32)wait_option)

#ifdef CONFIG_OSA_RMTE_MSG
#ifdef WIN32
#pragma pack(push, 1)
struct msg2fsm_data {
    aos_thread_id     src_thread_id;
    aos_process_id    src_process_id;
    aos_thread_id     dest_thread_id;
    aos_process_id    dest_process_id;
    aos_prim_id       prim_id;

    aos_msg_ptr       msg;
    u16 msg_size;
    u16 pad;
};
#pragma pack(pop)
#else
struct __attribute__((packed)) msg2fsm_data {
    aos_thread_id     src_thread_id;
    aos_process_id    src_process_id;
    aos_thread_id     dest_thread_id;
    aos_process_id    dest_process_id;
    aos_prim_id       prim_id;

    aos_msg_ptr       msg;
    u16 msg_size;
    u16 pad;
};
#endif

typedef struct msg2fsm_data msg2fsm_t;
typedef struct msg2fsm_data *msg2fsm_ptr_t;

/**
* aos_send_msg2fsm_to_rmte() - send specific msg to specific
*                              thread of specific sys.
* @sys: remote sys id. such as AIC_SYS_AP, AIC_SYS_CP and so on.
* @msg: specified message ptr to send.
* @b_free: if is need free after send succ.
*
* Return: AOS_SUCCESS means success, will return a error number.
*         if return failed, the call must free the msg ptr.
*/
int aos_send_msg2fsm_to_rmte(u8 sys, msg2fsm_ptr_t msg, bool b_free);

/**
* aos_send_msg_to_remote() - send a msg to specific thread of specific sys.
*
* @sys: remote sys id. such as AIC_SYS_AP, AIC_SYS_CP and so on.
* @dest_id: specified thread id.
* @src_msg_ptr: specified message ptr to send.
* @msg_size: the whole size of the msg..
* @b_free: if is need free after send succ.
*
* Return: AOS_SUCCESS means success, will return a error number.
*         if return failed, the call must free the msg ptr.
*/
int aos_send_msg_to_remote(u8 sys, aos_thread_id dest_id,
                           aos_msg_ptr msg, u16 msg_size, bool b_free);

#ifdef CONFIG_FSMMSG_FEATURE
/**
* aos_send_fsm_msg_to_remote() - send a fsmmsg to specific thread of remote sys.
*
* @sys: remote sys id. such as AIC_SYS_AP, AIC_SYS_CP and so on.
* @msg: specified fsmmsg ptr to send.
* @b_free: if is need free after send succ.
*
* Return: AOS_SUCCESS means send success, otherwise will return a error number.
*         if send succ, will free the fsmmsg ptr automatically.
*         if send fail, the caller must free the fsmmsg ptr.
*/
int aos_send_fsm_msg_to_remote(u8 sys, aos_fsm_msg *msg,  bool b_free);
#endif
#endif

/**
* aos_send_msg() - send specific message to specific thread queue tail.
* @dest_id: specified thread id.
* @src_msg_ptr: specified message ptr to send.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_send_msg(aos_thread_id dest_id, aos_msg_ptr src_msg_ptr,
                 u32 wait_option);
/**
* aos_send_fsm_msg_debug() - send fsm message to specific thread queue tail.
* @dest_id: specified thread id.
* @src_msg_ptr: specified fsm message ptr to send.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/

PLAT_TCM_CODE u32 aos_send_fsm_msg_debug(aos_thread_id dest_id,
                                         aos_fsm_msg *src_msg_ptr,
                                         u32 wait_option, bool b_trace_msg);
/**
* aos_send_msg_head() - send specific message to specific thread queue head.
* @dest_id: specified thread id.
* @src_msg_ptr: specified message ptr to send.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
* @b_trace_msg: TRUE - trace the message.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_send_msg_head(aos_thread_id dest_id, aos_msg_ptr src_msg_ptr,
                      u32 wait_option);
/**
* aos_send_fsm_msg_head_debug() - send fsm message to specific thread queue head.
* @dest_id: specified thread id.
* @src_msg_ptr: specified fsm message ptr to send.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
* @b_trace_msg: TRUE - trace the message.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_send_fsm_msg_head_debug(aos_thread_id dest_id, aos_fsm_msg *src_msg_ptr,
                                u32 wait_option, bool b_trace_msg);
/**
* aos_send_fsm_msg_to_api() - call destination thread's task_main to handle
* specific fsm message directly.
* @dest_id: specified thread id.
* @src_msg_ptr: specified fsm message ptr to send.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_send_fsm_msg_to_api(aos_fsm_msg *fsm_msg_ptr);
/**
* aos_recv_msg_from_intq() - receive message from specific thread internal queue
* @dest_id: specified thread id.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_NULL or valid msg ptr from queue.
*/
aos_msg_ptr aos_recv_msg_from_intq(aos_thread_id dest_id, u32 wait_option);
/**
* aos_recv_msg_from_extq() - receive message from specific thread external queue
* @dest_id: specified thread id.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_NULL or valid msg ptr from queue.
*/
aos_msg_ptr aos_recv_msg_from_extq(aos_thread_id dest_id, u32 wait_option);
/**
* aos_recv_msg() - receive message from specific thread default queue.
* (default queue is external queue)
* @dest_id: specified thread id.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_NULL or valid msg ptr from queue.
*/
aos_msg_ptr aos_recv_msg(aos_thread_id dest_id, u32 wait_option);
/**
* aos_get_fsm_msg_debug() - allocate fsm message, include fsm msg header,
* parameter and peer.
* @parameter_len: specified parameter length.
* @peer_msg_len: specified peer length.
* @file_name: file name to get fsm msg.
* @line: file line number to get fsm msg.
*
* Return: Valid fsm msg ptr means success, AOS_NULL means error.
*/

PLAT_TCM_CODE aos_fsm_msg *aos_get_fsm_msg_debug(u16 parameter_len,
                                                 u16 peer_msg_len,
                                                 filename_t file_name, u32 line);
/**
* aos_free_fsm_msg_debug() - free fsm message, include fsm msg header,
* parameter and peer.
* @fsm_msg_ptr: specified fsm msg ptr.
* @file_name: file name to free fsm msg.
* @line: file line number to free fsm msg.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_free_fsm_msg_debug(aos_fsm_msg *fsm_msg_ptr, filename_t file_name, u32 line);
/**
* aos_link_fsm_msg_debug() - link fsm message, include fsm msg header,
* parameter and peer.
* @fsm_msg_ptr: specified fsm msg ptr.
* @file_name: file name to free fsm msg.
* @line: file line number to free fsm msg.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_link_fsm_msg_debug(aos_fsm_msg *fsm_msg_ptr, filename_t file_name, u32 line);

/*
 * aos_malloc_init and aos_zalloc_init api:
 * Allocate memory from byte pool preferentially.
 * It is more fit for alloc in init hase and never release.
*/
#define aos_malloc_init(size)\
        aos_malloc_prefer_bytepool((u32)(size), AOS_FALSE, CUR_CODE_POS, AOS_TRUE)
#define aos_zalloc_init(size)\
        aos_malloc_prefer_bytepool((u32)(size), AOS_TRUE, CUR_CODE_POS, AOS_TRUE)

#ifdef CONFIG_MEM_MODE_TLSF
#define aos_malloc(size)\
        aos_malloc_prefer_bytepool((u32)(size), AOS_FALSE, CUR_CODE_POS, AOS_TRUE)
#define aos_zalloc(size)\
        aos_malloc_prefer_bytepool((u32)(size), AOS_TRUE, CUR_CODE_POS, AOS_TRUE)
#define aos_malloc_relaxed(size)\
                aos_malloc_prefer_bytepool((u32)(size), AOS_FALSE, CUR_CODE_POS, AOS_FALSE)
#define aos_zalloc_relaxed(size)\
                aos_malloc_prefer_bytepool((u32)(size), AOS_TRUE, CUR_CODE_POS, AOS_FALSE)
#define aos_free(mem_ptr)\
        aos_free_debug((u8 *)(mem_ptr), CUR_CODE_POS)

#define aos_dma_malloc(size)\
            aos_dma_malloc_debug((u32)(size), CUR_CODE_POS)
#define aos_dma_free(mem_ptr)\
            aos_free_debug((u8 *)(mem_ptr), CUR_CODE_POS)
#define aos_get_mem(pool_id, size)\
        aos_malloc_prefer_bytepool((u32)(size), AOS_FALSE, CUR_CODE_POS, AOS_TRUE)
#define aos_free_mem(mem_ptr)\
        aos_free_debug((u8 *)(mem_ptr), CUR_CODE_POS)
#define aos_link_mem(mem_ptr)\
        aos_link_mem_debug((u8 *)(mem_ptr), CUR_CODE_POS)

#define aos_get_mem_internel(pool_id, size, strict, file, line)\
        aos_malloc_prefer_bytepool(size, AOS_FALSE, file, line, AOS_TRUE)
#define aos_free_mem_internel(mem_ptr, file, line)\
        aos_free_debug((u8 *)(mem_ptr), file, line)
#else
#define aos_malloc(size)\
        aos_malloc_debug((u32)(size), AOS_FALSE, CUR_CODE_POS)
#define aos_zalloc(size)\
        aos_malloc_debug((u32)(size), AOS_TRUE, CUR_CODE_POS)
#define aos_malloc_relaxed(size)\
                aos_malloc_debug((u32)(size), AOS_FALSE, CUR_CODE_POS)
#define aos_zalloc_relaxed(size)\
                aos_malloc_debug((u32)(size), AOS_TRUE, CUR_CODE_POS)
#define aos_free(mem_ptr)\
        aos_free_debug((u8 *)(mem_ptr), CUR_CODE_POS)

#define aos_dma_malloc(size)\
            aos_dma_malloc_debug((u32)(size), CUR_CODE_POS)
#define aos_dma_free(mem_ptr)\
            aos_dma_free_debug((u8 *)(mem_ptr), CUR_CODE_POS)

#define aos_get_mem(pool_id, size)\
        aos_get_mem_debug(pool_id, size, AOS_TRUE, CUR_CODE_POS)
#define aos_free_mem(mem_ptr)\
        aos_free_mem_debug((u8 *)(mem_ptr), CUR_CODE_POS)
#define aos_link_mem(mem_ptr)\
        aos_link_mem_debug((u8 *)(mem_ptr), CUR_CODE_POS)

#define aos_get_mem_internel(pool_id, size, strict, file, line)\
        aos_get_mem_debug(pool_id, size, strict, file, line)
#define aos_free_mem_internel(mem_ptr, file, line)\
        aos_free_mem_debug(mem_ptr, file, line)
#endif

/**
* aos_init_pools() - initialize aos memory pool.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means fail.
*/
u32 aos_init_pools(void);
/**
* aos_pool_check() - check specified memory overflow or not.
* @dest_ptr: specified memory ptr to check
* @length: specified memory length to check.
*
* Return: AOS_SUCCESS means success, others means fail.
*/
aos_status aos_pool_check(u8 *dest_ptr, u32 length);
/**
* aos_is_heap_ptr() - check specified memory located in heap or not.
* @dest_ptr: specified memory ptr to check
*
* Return: AOS_TRUE means heap space, AOS_FALSE means other space.
*/
boolean aos_is_heap_ptr(u8 *dest_ptr);

/**
* aos_get_heap_size() - get aos heap region size.
*
* Return: aos heap region size in bytes.
*/
u32 aos_get_heap_size(void);

typedef struct aos_heap_size_st {
    u32 total_heap;             /* total heap size */
    u32 total_used;             /* total used heap size */
    u32 total_free;             /* total free heap size */
    u32 max_free_node;          /* max node size can be allocated */
    u32 usage_water_line;       /* max total used heap size */
} aos_heap_size;

/**
* aos_get_heap_size_ex() - get aos heap size in detail,NOTE: this function
* takes many time to calc the memery nodes.
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
u32 aos_get_heap_size_ex(aos_heap_size *heap_size);

/**
* aos_get_heap_usage() - get aos heap usage, it is more faster than
* aos_get_heap_size_ex().
* @total_size: total memory size.
* @used_size: used memory size.
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
u32 aos_get_heap_usage(u32 *total_size, u32 *used_size);


/**
* CONFIG_MEM_CHECK_API
*
* Open it can use below apis, this api will check all or part of the memory
* nodes that malloc by below malloc api. If open the api may need add the cp
* heap size and the block pool cnt. The detail can see the gerrit 3420.
* if want check it every thread scheduling, please see the gerrit 3422.
* if defined CONFIG_CHECK_ALL_NODE, will check all of the memory nodes,
* otherwise, check part of the memory nodes.
*/
//#define CONFIG_MEM_CHECK_API
#ifdef CONFIG_MEM_CHECK_API
//#define CONFIG_CHECK_ALL_NODE
#define CHECK_NODE_NUM 60

void *aos_check_malloc(u32 size, filename_t file_name, u32 line);
void aos_check_free(u8 *mem_ptr, filename_t file_name, u32 line);
void aos_check_all_mem(void);
void *aos_check_dma_malloc_debug(u32 size, filename_t file_name, u32 line);
u32 aos_check_link_mem(u8 *mem_ptr, filename_t file_name, u32 line);

#define aos_malloc_check(size)\
        aos_check_malloc((u32)(size), CUR_CODE_POS)
#define aos_free_check(mem_ptr)\
        aos_check_free((u8 *)(mem_ptr), CUR_CODE_POS)
#define aos_dma_malloc_check(size)\
        aos_check_dma_malloc_debug((u32)(size), CUR_CODE_POS)
#define aos_link_mem_check(mem_ptr)\
        aos_check_link_mem((u8 *)(mem_ptr), CUR_CODE_POS)
#define aos_dma_free_check(mem_ptr) \
        aos_check_free((u8 *)(mem_ptr), CUR_CODE_POS)
#endif

/**
* aos_malloc_debug() - allocate memory from block pool preferentially.
* @size: memory size to alloc.
* @zero: if init the memory to 0.
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
void *aos_malloc_debug(u32 size, bool zero, filename_t file_name, u32 line);

/**
* aos_malloc_prefer_bytepool() - Allocate memory from byte pool preferentially.
* @size: memory size to alloc.
* @zero: if init the memory to 0.
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
* @b_strict: must get available memory, or it will assert.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
void *aos_malloc_prefer_bytepool(u32 size, bool zero, filename_t file_name, u32 line, boolean b_strict);

/**
* aos_dma_malloc_debug() - allocate memory from block pool or byte pool, which
*                          must be cacheline-aligned
* @size: memory size to alloc.
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
void *aos_dma_malloc_debug(u32 size, filename_t file_name, u32 line);

/**
* aos_dma64_malloc_debug() - allocate memory from block pool or byte pool, which
*                          must be 64bit-aligned
* @size: memory size to alloc.
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
void *aos_dma64_malloc_debug(u32 size, filename_t file_name, u32 line);

/**
* aos_free_debug() - free mem allocated from block pool or byte pool.
* @mem_ptr: memory to free
* @file_name: name of the file to free memory.
* @line: line number of the file to free memory.
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
u32 aos_free_debug(u8 *mem_ptr, filename_t file_name, u32 line);
/**
* aos_dma_free_debug() - free dma mem allocated from block pool or byte pool.
* @mem_ptr: memory to free
* @file_name: name of the file to free memory.
* @line: line number of the file to free memory.
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
u32 aos_dma_free_debug(u8 *dma_mem_ptr, filename_t file_name, u32 line);

/**
* aos_mem_get_size() - Get allocated memory size.
* @mem_ptr: memory pointer
*
* Return: 0 means error, else the memory size.
*/
u32 aos_mem_get_size(u8 *mem_ptr);

/**
* aos_dma_cpy - use dma copy memory. if needed, the caller
*               must makesure the addre and size is align wtih cacheline size
*               and flush the src cache and invalid the dst cache itself.
*               may block in it, must call it in thread context.
*               if don't think cache issue, yuo can use aos_dma_cpy_safe.
* @src: source addr.
* @dst: dst addr.
* @line: size
*
*/
void aos_dma_cpy(void *dst, void *src, size_t size);
/**
* aos_dma_cpy_raw - It is simular with aos_dma_cpy,
*               the different of this api is that there is no interrupt,
*               and it can be used in no threadx env.
* @src: source addr.
* @dst: dst addr.
* @line: size
*
*/
void aos_dma_cpy_raw(void *dst, void *src, size_t size);
/*
 * aos_dma_bare_cpy
 * It will always use dma chn0 to copy memory.
 * Notice!!!
 * It can be only used when boot up or resume, normal mode cannot use
 * this api.
 */
void aos_dma_bare_cpy(void *dst, void *src, size_t size);

/**
* aos_get_mem_debug() - fet mem block from specified block pool.
* @pool_id: id of the pool to allocate from.
* @size: memory size to allocate.
* @b_strict: must get available block, or it will assert
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
*
* Return: Allocated memory ptr.
*/
PLAT_TCM_CODE void *aos_get_mem_debug(u32 pool_id, u32 size, bool b_strict,
                                      filename_t file_name, u32 line);

/**
* aos_free_mem_debug() - free mem block.
* @ptr: memory ptr to free.
* @file_name: name of the file to free memory.
* @line: line number of the file to free memory.
*
* Return: AOS_SUCCESS means success, AOC_ERROR means error.
*/
PLAT_TCM_CODE u32 aos_free_mem_debug(u8 *mem_ptr, filename_t file_name, u32 line);

/**
* aos_link_mem_debug() - link allocated mem block.
* @ptr: memory ptr to link.
* @file_name: name of the file to link memory.
* @line: line number of the file to link memory.
*
* Return: AOS_SUCCESS means success, AOC_ERROR means error.
*/
u32 aos_link_mem_debug(u8 *mem_ptr, filename_t file_name, u32 line);
/**
* aos_delete_all_mem() - free all allocated mem block.
*
* Return: AOS_SUCCESS means success, AOC_ERROR means error.
*/
u32 aos_delete_all_mem(void);
/**
 * aos_pool_dump() - dump aos pool information to file.
 */
void aos_pool_dump(char *path);

/* ---interrupt--- */
/*
 * The return value of irq handle function.
 * @IRQ_ISR_ONLY: the iqr only has irq handle function.
 * @IRQ_HALF_HISR: the iqr has half bottom handle function,
 *                 will be run on high priority isr task.
 * @IRQ_HALF_HISR: the iqr has half bottom handle function,
 *                 will be run on high priority isr task.
 */
#define IRQ_ISR_ONLY    0
#define IRQ_HAS_HISR    1
#define IRQ_HAS_MISR    2

/**
 * aos_disable_irq - disable the irq of system(set CPSID i and f flag).
 *
 * @return: return the old irq state of system, the caller must save it
 *          and restore system irq later.
 */
u32 aos_disable_irq(void);
/**
 * aos_restore_irq - restore the irq  state of system.
 *
 * @interrupt_save: The old irq state of system saved when call aos_disable_irq.
 */
void aos_restore_irq(u32 interrupt_save);

#if defined (CONFIG_AOS_INT_DEBUG) || defined(_WIN32)
#define AOS_INTERRUPT_DECLARE   u32 interrupt_save;
#define AOS_INTERRUPT_DISABLE   interrupt_save = aos_disable_irq();
#define AOS_INTERRUPT_RESTORE   aos_restore_irq(interrupt_save);
#else
#include <core_op.h>

#define AOS_INTERRUPT_DECLARE   unsigned int interrupt_save;
#define AOS_INTERRUPT_DISABLE   interrupt_save = disable_interrupts();
#define AOS_INTERRUPT_RESTORE   restore_interrupts(interrupt_save);
#endif

/**
 * aos_request_irq - request a irq with isr handler.
 *
 * @irq: the irq number.
 * @handler: the irq handler.
 * @p: the handler callback paramter.
 */
void aos_request_irq(int irq, int (*handler)(void *p), void *p);
/**
 * aos_request_irq_with_half_bottom - request a irq with isr handler and
 *                                    bottom half handler.
 *
 * @irq: the irq number.
 * @handler: the irq handler.
 * @half: the bottom half handler.
 * @p: the handler callback parameter.
 */
void aos_request_irq_with_half_bottom(int irq,
                                      int (*handler)(void *p),
                                      void (*half)(void *p),
                                      void *p);
/**
 * aos_release_irq - release a irq.
 *
 * @irq: the irq number.
 */
void aos_release_irq(int irq);

/* ---time--- */
/**
* aos_get_tickcount() - get system tickcount since system bootup.
*
* Return: current tickcout (ms).
*/

#ifdef WIN32
u32 aos_get_time_tick(void);
#define aos_get_tickcount()     aos_get_time_tick()
#define aos_get_aon_raw_tick()  aos_get_time_tick()
#else
#include <timer_inc.h>
#include <aic_reg_base.h>

#define aos_get_tickcount()   (u32)aic_get_tick()
#define aos_get_aon_raw_tick()  *(volatile ulong *)(REG_BASE_AON_SYSTMR)
#endif

/**
 * aos_get_aon_raw_tick() - aos_get_aon_raw tick count.
 * It just get the raw tick of aon 32k timer,
 * takes as few mips as possible.
 */

/**
* aos_time_to_tick() - translate time(ms) to tick.
* @time: time value(ms) to translate.
* Return: Tick value.
*/

/**
* aos_tick_to_time() - translate tick to time(ms).
* @tick: tick to translate.
* Return: time value(ms) .
*/
#ifdef CONFIG_RTOS_THREADX
#ifndef WIN32
#define aos_time_to_tick(time)  (time)
#define aos_tick_to_time(tick)  (tick)
#else
u32 aos_time_to_tick(u32 time);
u32 aos_tick_to_time(u32 tick);
#endif
#else
u32 aos_time_to_tick(u32 time);
u32 aos_tick_to_time(u32 tick);
#endif

/**
* aos_sleep() - force specific thread to sleep for time_ms.
* @time_ms: time_ms to sleep.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_sleep(u32 time_ms);

/**
* aos_usleep() - force specific thread to sleep for time_us.
* @time_us: time_us us to sleep.
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_usleep(u32 time_us);

/**
* aos_get_systime() - Get local time for AP SYS.
* @time: to store the local time.
*
* Return: void.
*/
void aos_get_systime(aos_systime *time);
/**
* aos_set_systime() - Set local time for AP SYS.
* @time: local time to set.
*
* Return: void.
*/
void aos_set_systime(aos_systime *time);
/**
* aos_get_utctime() - Get utc time for AP SYS.
* @time: to store the utc time.
*
* Return: void.
*/
void aos_get_utctime(aos_systime *time);
/**
* aos_set_utctime() - Set utc time for AP SYS.
* @time: utc time to set.
*
* Return: void.
*/
void aos_set_utctime(aos_systime *time);
/**
* aos_time_change_request() - Request time change callback.
* If aos time is changed, it will call the callback funtion.
* @cb: callback function.
* @param: callback paramater
*
* Return: void.
*/
void aos_time_change_request(void *cb, void *param);
/**
* aos_time_change_unrequest() - Unrequest time change callback.
*
* Return: void.
*/
void aos_time_change_unrequest(void);
/**
* aos_get_utc_alarm_time() - Get utc alarm time for AP SYS.
* @time: to store the utc time.
*
* Return: void.
*/
void aos_get_utc_alarm_time(aos_systime *time);
/**
* aos_set_utc_alarm_time() - Set utc alarm time for AP SYS.
* @time: utc time to set.
*
* Return: void.
*/
void aos_set_utc_alarm_time(aos_systime *time, void *cb, void *param);
/**
* aos_get_alarm_time() - Get local alarm time for AP SYS.
* @time: to store the local time.
*
* Return: void.
*/
void aos_get_alarm_time(aos_systime *time);
/**
* aos_set_alarm_time() - Set local alarm time for AP SYS.
* @time: local time to set.
*
* Return: void.
*/
void aos_set_alarm_time(aos_systime *time, void *cb, void *param);
/**
* aos_del_alarm_time() - Delete local alarm time.
*
* Return: void.
*/
void aos_del_alarm_time(void);

/**
* aos_adjust_time_with_time_zone - Adjust time with time zone.
*
* Return: void.
*/
void aos_adjust_time_with_time_zone(aos_systime *current_time, s32 time_zone);

/* assert */
#define AOS_ASSERT              AIC_ASSERT
#define AOS_ASSERT_INFO         AIC_ASSERT_INFO
#define AOS_ASSERT_INFX         AIC_ASSERT_INFX

/* trace and message report */
void aos_report(aos_thread_id thread_id, aos_process_id proc_id,
                u8 *str, u16 length, boolean force_critical,
                char *filename, u32 line);

/* output msg to msg port, catched by TT Tool */
#define AOS_REPORT(thread_id,proc_id,err_no,err_str,force_critical,reserved)\
        aos_report((aos_thread_id)thread_id,\
                   (aos_process_id)proc_id,\
                   (u8 *)err_str,\
                   (u16)strlen(err_str),\
                   (boolean)force_critical,\
                   (char *)__FILE__,\
                   (u32)__LINE__)
#define AOS_MESSAGE(msg_ptr)        alog_message((void *)msg_ptr)
#define AOS_GET_REPORT_LEVEL(ptr)   ALOG_LV_INFO
/* output trace to trace port, catched by PF Trace Tool
 * using INFO as its default log level */
#define AOS_PRINT                   alog_info
#define AOS_IDXPR                   aicx_info

extern char *strcasestr(const char *s, const char *find);
#ifdef WIN32
#define strncasecmp _strnicmp
extern char *strtok_r(char *str, const char *sep, char **state);
#endif
/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __AIC_OSA_H__ */

