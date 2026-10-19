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

#ifndef __AIC_MINI_OS_ADPTER_H__
#define __AIC_MINI_OS_ADPTER_H__
#include <aic_assert.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_math.h>
#include <aic_size.h>
#include <alog_api.h>
/* TODO */
//#include <irq.h>

#ifdef CONFIG_AIC_OSA
#include <aic_osa.h>
#include <aos_config.h>

#define aos_free_ex(p, size)        aos_free(p)
#define aos_dma_free_ex(p, size)    aos_dma_free(p)

#else
#error undef CONFIG_AIC_OSA, please check if support aic osa.
#error if not support please change bellow  #if 0 to #if 1 to open mini osa.
#if 0
#include <tx_api.h>

typedef void *aos_thread_id;
typedef void *aos_timer_ptr;
typedef void (*aos_expiration_func)(void *input);
typedef u32  aos_timer_value;
typedef void *aos_mutex_ptr;
typedef void *aos_semaphore_ptr;
typedef void *aos_event_ptr;
typedef void *aos_queue_ptr;

typedef u32 aos_status;

#define AOS_NULL                    0
#define AOS_SUCCESS                 (u32)0
#define AOS_ERROR                   (u32)0xFFFFFFFF
#define AOS_NO_MEMORY               0x10
#define AOS_TRUE                    AIC_TRUE
#define AOS_FALSE                   AIC_FALSE

#define AOS_INTERRUPT_DECLARE   TX_INTERRUPT_SAVE_AREA
#define AOS_INTERRUPT_DISABLE   TX_DISABLE
#define AOS_INTERRUPT_RESTORE   TX_RESTORE

#define aos_get_tickcount       (u32)tx_time_get

/* wait forever option define */
#define AOS_WAIT_FOREVER   (TX_WAIT_FOREVER)
#define AOS_NO_WAIT        (TX_NO_WAIT)

/* invalid id */
#define AOS_INVALID_ID              0xFFFFFFFF

/* thread */
#define AOS_INVALID_THREAD_ID       NULL
#define AOS_PROCESS_NUMS            8

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

typedef struct aos_thread {
    TX_THREAD       thread;
    unsigned int    stack_size;
    unsigned char   *stack;
} aos_thread_t;

typedef struct aos_queue {
    TX_QUEUE        queue;
    unsigned int    queue_size;
    unsigned char   *queue_start;
} aos_queue_t;

typedef void (*tx_timer_function_t)(ulong p);
typedef void (*aos_entry_func)(void *input);

void aos_adpter_init_pools(void);
void *aos_malloc_ex(ulong size, bool zero);
void aos_free_ex(void *addr, ulong size);
#define aos_malloc(size)    aos_malloc_ex(size, AOS_FALSE)
#define aos_zalloc(size)    aos_malloc_ex(size, AOS_TRUE)
#define aos_free(p) aos_free_ex(p, 0)

aos_thread_id aos_create_thread_simply(char *thread_name,
                                       aos_entry_func entry_func,
                                       void *input, u32 stack_size,
                                       u32 priority);
void aos_delete_thread(aos_thread_id thread_id);

aos_queue_ptr aos_create_queue(char *name, u32 message_size, u32 queue_size);
void aos_delete_queue(aos_queue_ptr queue);
aos_status aos_send_to_queue(aos_queue_ptr queue, void *element, ulong wait);
aos_status aos_recv_from_queue(aos_queue_ptr queue, void *element, ulong wait);

aos_mutex_ptr aos_create_mutex(char *name);
void aos_delete_mutex(aos_mutex_ptr mutex_ptr);
aos_status aos_get_mutex(aos_mutex_ptr mutex_ptr, ulong wait);
void aos_put_mutex(aos_mutex_ptr mutex_ptr);

aos_event_ptr aos_create_event(char *name);
void aos_delete_event(aos_event_ptr event_ptr);
aos_status aos_get_event(aos_event_ptr event_ptr, u32 requested_flags,
                         u32 get_option, u32 *actual_flags_ptr, u32 wait_option);
void aos_set_event(aos_event_ptr event_ptr, u32 flags_to_set, u32 set_option);

aos_semaphore_ptr aos_create_semaphore_simply(char *name);
void aos_delete_semaphore(aos_semaphore_ptr semaphore_ptr);
void aos_put_semaphore(aos_semaphore_ptr semaphore_ptr);
aos_status aos_get_semaphore(aos_semaphore_ptr sem, ULONG wait);

aos_timer_ptr aos_create_timer(char *name_ptr,
                               aos_expiration_func expiration_func, void *input,
                               u32 expiration_time, u32 auto_activate,
                               boolean b_period);
void aos_delete_timer(aos_timer_ptr timer_ptr);
void aos_change_timer(aos_timer_ptr timer_ptr, u32 expiration_time,
                      boolean b_period);
void aos_activate_timer(aos_timer_ptr timer_ptr);
void aos_deactivate_timer(aos_timer_ptr timer_ptr);

aos_status aos_sleep(ulong time);
#endif
#endif /* CONFIG_AIC_OSA */

#endif /* __AIC_MINI_OS_ADPTER_H__ */

