/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ffdb8
#define LOG_MODULE    AMOD_AOS

#include "cmsis_os2.h"
#include "cmsis_os2_tx_extensions.h"

#include <stdbool.h>
#include <stdint.h>
#include <tx_api.h>
#include <tx_initialize.h>
#include <tx_thread.h>
#include "aic_osa.h"

#define TX_CMSIS_RTOS_API_VERSION 20010003 // API version 2.1.3

#ifndef TX_CMSIS_JOINABLE
#define TX_CMSIS_JOINABLE 1
#endif

#ifndef TX_CMSIS_THREAD_FLAGS
#define TX_CMSIS_THREAD_FLAGS 1
#endif

#ifndef OS_STACK_SIZE
#define OS_STACK_SIZE (512 + 256)
#endif

#define TX_CMSIS_USE_AOS_MEM

#ifdef TX_CMSIS_USE_AOS_MEM
#ifndef OS_DYNAMIC_MEM_SIZE
#define OS_DYNAMIC_MEM_SIZE 0x4000
#endif
#endif



#ifndef OS_ROBIN_TIMEOUT
/* Set default round-robin timeout to 5 ticks (if time-slicing is enabled via
 * the OS_ROBIN_ENABLE macro) */
#define OS_ROBIN_TIMEOUT 5
#endif

#ifndef TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION
#error The line "#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION return;" \
    must be added to your tx_user.h file to use the CMSIS-RTOSv2 adaptation \
    layer.
#endif

#ifndef TX_THREAD_USER_EXTENSION
#error The line "#define TX_THREAD_USER_EXTENSION VOID *tx_cmsis_extension;" \
    must be added to your tx_user.h file to use the CMSIS-RTOSv2 adaptation \
    layer.
#endif

#if TX_MAX_PRIORITIES < 64
#error TX_MAX_PRIORITIES must be defined to at least 64 to use the \
    CMSIS-RTOSv2 adaptation layer.
#endif

// ThreadX message size is at most 64 bytes large
#define TX_MESSAGE_MAX_BYTE_SIZE 64

/* Node in a linked list of memory regions. */
typedef struct tx_cmsis_region tx_cmsis_region_t;
struct tx_cmsis_region {
    void *start;
    void *after_end;
    tx_cmsis_region_t *next;
};

/* Struct to wrap ThreadX mutex */
typedef struct tx_cmsis_mutex tx_cmsis_mutex_t;
struct tx_cmsis_mutex {
    /* Internal ThreadX mutex */
    TX_MUTEX tx_mutex;
    /* Attributes with which the mutex was created */
    uint32_t attr_bits;
};

/* Struct to wrap ThreadX semaphore */
typedef struct tx_cmsis_semaphore tx_cmsis_semaphore_t;
struct tx_cmsis_semaphore {
    /* Internal ThreadX semaphore */
    TX_SEMAPHORE tx_semaphore;
    /* Maximum number of tokens */
    uint32_t max_count;
};

/* Struct to wrap ThreadX timer */
typedef struct tx_cmsis_timer tx_cmsis_timer_t;
struct tx_cmsis_timer {
    TX_TIMER tx_timer;
    osTimerType_t type;
};

/* Struct to wrap ThreadX block pool */
typedef struct tx_cmsis_memory_pool tx_cmsis_memory_pool_t;
struct tx_cmsis_memory_pool {
    /* Internal ThreadX block pool */
    TX_BLOCK_POOL tx_block_pool;
    /* Block count and block size of the pool */
    uint32_t block_count;
    uint32_t block_size;
    /* The memory area that backs the pool */
    tx_cmsis_region_t *backing_region;
};

/* Struct to wrap ThreadX queue */
typedef struct tx_cmsis_msg_queue tx_cmsis_msg_queue_t;
struct tx_cmsis_msg_queue {
    /* Internal ThreadX queue */
    TX_QUEUE tx_queue;
    /* Message count and message size of the pool */
    uint32_t msg_count;
    uint32_t msg_size;
    uint32_t msg_storage_size;
    /* Pointer to the start of queue memory that we have allocated
     * Note: Should be set to NULL when memory is supplied by the
     * application */
    void *queue_mem;
};

/* Thread extension struct */
typedef struct tx_cmsis_thread_ext tx_cmsis_thread_ext_t;
struct tx_cmsis_thread_ext {
    /* The memory area of the thread's stack */
    tx_cmsis_region_t *stack_region;

#if TX_CMSIS_THREAD_FLAGS
    TX_EVENT_FLAGS_GROUP thread_flags;
#endif /* TX_CMSIS_THREAD_FLAGS */

#if TX_CMSIS_JOINABLE
    /* For joinable thread support
     * Note: The volatile keyword here is necessary to prevent compiler
     * optimizations where thread-switching may occur. Memory barriers are not
     * necessary as all threads are running on the same core. */
    volatile bool is_joinable;
    TX_SEMAPHORE join_semaphore;
    volatile bool finished;
#endif /* TX_CMSIS_JOINABLE */
};

/* Pack a TX_THREAD with its tx_cmsis_thread_ext_t. This allows both to be
 * allocated in one go if necessary, but the TX_THREAD's tx_cmsis_extension
 * pointer must still be set to point to the tx_cmsis_thread_ext_t. */
typedef struct tx_cmsis_extended_thread tx_cmsis_extended_thread_t;
struct tx_cmsis_extended_thread {
    /* ThreadX thread */
    TX_THREAD tx_thread;
    /* CMSIS layer extension */
    tx_cmsis_thread_ext_t thread_ext;
};

static osKernelState_t tx_cmsis_kernel_state = osKernelInactive;

/* Record whether the scheduler is 'locked' */
static bool tx_cmsis_kernel_locked = false;

#ifndef TX_CMSIS_USE_AOS_MEM
TX_BYTE_POOL tx_cmsis_heap = {0};
uint8_t tx_cmsis_heap_bytes[OS_DYNAMIC_MEM_SIZE] = {0};
#endif

#ifdef TX_CMSIS_USE_AOS_MEM
#define tx_cmsis_alloc(size)    aos_malloc(size)
#define tx_cmsis_free(mem_ptr)    aos_free(mem_ptr)
#else
static inline VOID *tx_cmsis_alloc(size)
{
    UINT ret;
    VOID *memptr;
    ret = tx_byte_allocate(&tx_cmsis_heap, (VOID **)&memptr, size, TX_NO_WAIT);
    if (ret != TX_SUCCESS) {
        return NULL;
    }
    return memptr;
}
#define tx_cmsis_free(mem_ptr)    tx_byte_release(mem_ptr)
#endif

/* Linked list of regions that are within our heap but not a part of it
 * (because they back memory pools). Objects allocated within these regions
 * were not allocated by us so should not be free'd by us and should be
 * reported as not in our heap by tx_cmsis_is_in_our_heap(). */
tx_cmsis_region_t *tx_cmsis_nonheap_regions = NULL;

#ifdef TX_CMSIS_USE_AOS_MEM
static bool tx_cmsis_is_within_our_heap(VOID *ptr)
{
    /* Check if the pointer is within the bounds of the heap. */
    return aos_is_heap_ptr(ptr);
}
#else
static bool tx_cmsis_is_within_our_heap(VOID *ptr)
{
    /* Check if the pointer is within the bounds of the heap. */
    return (ptr >= (VOID *)tx_cmsis_heap_bytes) && (ptr < (VOID *)(tx_cmsis_heap_bytes + sizeof(tx_cmsis_heap_bytes)));
}
#endif

static bool tx_cmsis_is_in_our_heap(VOID *ptr)
{
    bool within_heap = tx_cmsis_is_within_our_heap(ptr);

    bool in_nonheap_region = false;

    /* Walk through linked-list of nonheap regions and check if we are within
     * any of them. */
    tx_cmsis_region_t *head = tx_cmsis_nonheap_regions;
    while (head != NULL) {
        if ((ptr >= (VOID *)head->start) && (ptr < (VOID *)head->after_end)) {
            in_nonheap_region = true;
            break;
        }
        head = head->next;
    }

    return (within_heap && (!in_nonheap_region));
}

static bool tx_cmsis_in_isr(void)
{
    return (((TX_THREAD_GET_SYSTEM_STATE() != ((ULONG)0)) && (TX_THREAD_GET_SYSTEM_STATE() < TX_INITIALIZE_IN_PROGRESS)));
}

static tx_cmsis_region_t *tx_cmsis_nonheap_region_add(void *region_start, uint32_t region_size)
{
    tx_cmsis_region_t *nonheap_region;
    nonheap_region = tx_cmsis_alloc(sizeof(tx_cmsis_region_t));
    if (nonheap_region == NULL) {
        return NULL;
    }
    nonheap_region->start = region_start;
    nonheap_region->after_end = (uint8_t *)region_start + region_size;
    nonheap_region->next = NULL;

    /* Prepend new region to the list. */
    if (tx_cmsis_nonheap_regions != NULL) {
        nonheap_region->next = tx_cmsis_nonheap_regions;
    }
    tx_cmsis_nonheap_regions = nonheap_region;

    return nonheap_region;
}

static void tx_cmsis_nonheap_region_remove(tx_cmsis_region_t *nonheap_region)
{
    if (tx_cmsis_nonheap_regions == NULL) {
        return;
    } else if (tx_cmsis_nonheap_regions->next == NULL) {
        /* Singleton list - overwrite with NULL */
        tx_cmsis_nonheap_regions = NULL;
    } else if (tx_cmsis_nonheap_regions == nonheap_region) {
        /* Unlink first element of list */
        tx_cmsis_nonheap_regions = tx_cmsis_nonheap_regions->next;
    } else {
        tx_cmsis_region_t *prev = tx_cmsis_nonheap_regions;
        tx_cmsis_region_t *current = tx_cmsis_nonheap_regions->next;

        while (current != NULL) {
            if (current == nonheap_region) {
                /* Unlink region from the list */
                prev->next = current->next;
                break;
            }
            prev = current;
            current = current->next;
        }
        if (current == NULL) {
            /* Region not found in the list */
            return;
        }
    }

    /* Free the tx_cmsis_region_t struct */
    tx_cmsis_free((VOID *)nonheap_region);
}

osKernelState_t osKernelGetState(void)
{
    if (tx_cmsis_kernel_locked) {
        return osKernelLocked;
    } else {
        return tx_cmsis_kernel_state;
    }
}

osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size)
{
    if (NULL != version) {
        uint32_t threadx_version =
            (THREADX_MAJOR_VERSION << 24) | (THREADX_MINOR_VERSION << 16) | (THREADX_PATCH_VERSION);
        version->api = TX_CMSIS_RTOS_API_VERSION;
        version->kernel = threadx_version;
    }
    if (NULL != id_buf) {
        strlcpy(id_buf, "ThreadX", id_size);
    }
    return osOK;
}

osStatus_t osKernelInitialize(void)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (tx_cmsis_kernel_state != osKernelInactive) {
        return osError;
    }
#ifdef TX_CMSIS_USE_AOS_MEM
    tx_cmsis_kernel_state = osKernelReady;
    return osOK;
#else
    /* Prepare to create objects.
     * Note that the line:
     * #define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION return;
     * must be added to tx_user.h to prevent this from starting the
     * scheduler.
     * Without this define, tx_kernel_enter() will begin trying to schedule
     * threads (which have not been created yet) and enter an infinite loop,
     * forever waiting for a thread to become ready. */
    tx_kernel_enter();

    UINT ret = TX_SUCCESS;

    ret = tx_byte_pool_create(&tx_cmsis_heap,             /* pool ptr */
                              "CMSIS OS2 Heap",           /* name */
                              &tx_cmsis_heap_bytes[0],    /* pool start */
                              sizeof(tx_cmsis_heap_bytes) /* pool size */
    );

    switch (ret) {
        case TX_SUCCESS:
            tx_cmsis_kernel_state = osKernelReady;
            return osOK;
        case TX_NO_MEMORY:
            return osErrorNoMemory;
        default:
            return osError;
    }
#endif
}

osStatus_t osKernelStart(void)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (tx_cmsis_kernel_state != osKernelReady) {
        return osError;
    }

    tx_cmsis_kernel_state = osKernelRunning;
#ifndef TX_CMSIS_USE_AOS_MEM
    _tx_thread_schedule();
#endif

    /* This line should not be reached */
    return osOK;
}

uint32_t osKernelGetTickCount(void)
{
    return (uint32_t)tx_time_get();
}

#ifdef TX_TIMER_TICKS_PER_SECOND
uint32_t osKernelGetTickFreq(void)
{
    return (uint32_t)TX_TIMER_TICKS_PER_SECOND;
}
#else
#warning "TX_TIMER_TICKS_PER_SECOND not defined - osKernelGetTickFreq() will be unavailable."
#endif

int32_t osKernelLock(void)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    bool prev_state = tx_cmsis_kernel_locked;

    UINT ret;
    UINT old_preempt_thresh;

    /* We 'lock' the kernel by setting our preemption threshold to 0
     * so that nothing can preempt us */

    TX_THREAD *this_thread = tx_thread_identify();

    /* If the kernel has not been started yet, we cannot lock it */
    if (this_thread == NULL) {
        return osError;
    }

    ret = tx_thread_preemption_change(this_thread, 0, &old_preempt_thresh);

    if (ret != TX_SUCCESS) {
        /* Unexpected error */
        return osError;
    }

    /* We do not use the old preemption threshold so suppress warnings */
    (void)old_preempt_thresh;

    tx_cmsis_kernel_locked = true;

    return (prev_state ? 1 : 0);
}

int32_t osKernelUnlock(void)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    bool prev_state = tx_cmsis_kernel_locked;

    /* Set the state while the kernel is still locked and we can't be
     * preempted */
    tx_cmsis_kernel_locked = false;

    /* 'Unlock' the kernel by setting the thread's preemption threshold equal
     * to its priority */

    TX_THREAD *this_thread = tx_thread_identify();

    UINT ret;
    UINT priority;
    UINT old_preempt_thresh;

    /* If the kernel has not been started yet, we cannot unlock it */
    if (this_thread == NULL) {
        return osError;
    }

    ret = tx_thread_info_get(this_thread,
                             NULL,      /* name */
                             NULL,      /* state */
                             NULL,      /* run_count */
                             &priority, /* priority */
                             NULL,      /* preemption_threshold */
                             NULL,      /* time_slice */
                             NULL,      /* next_thread */
                             NULL       /* suspended_thread */
    );
    if (ret != TX_SUCCESS) {
        /* Unexpected error */
        return osError;
    }

    ret = tx_thread_preemption_change(this_thread, priority, &old_preempt_thresh);

    if (ret != TX_SUCCESS) {
        /* Unexpected error */
        return osError;
    }

    /* We do not use the old preemption threshold so suppress warnings */
    (void)old_preempt_thresh;

    return (prev_state ? 1 : 0);
}

int32_t osKernelRestoreLock(int32_t lock)
{
    UINT ret;
    if (lock == 0) {
        ret = osKernelUnlock();
    } else if (lock == 1) {
        ret = osKernelLock();
    } else {
        return osError;
    }

    if ((ret == 1) || (ret == 0)) {
        /* Success, so we've set the state to that requested. */
        return lock;
    } else {
        /* Propagate the error from osKernelLock()/osKernelUnlock() */
        return ret;
    }
}

osMutexId_t osMutexNew(const osMutexAttr_t *attr)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }

    osMutexAttr_t default_attr = {.name = NULL, .attr_bits = 0, .cb_mem = NULL, .cb_size = 0};
    if (attr == NULL) {
        attr = &default_attr;
    }

    tx_cmsis_mutex_t *new_mutex = (tx_cmsis_mutex_t *)attr->cb_mem;
    bool alloced_new_mutex = false;

    if (new_mutex == NULL) {
        new_mutex = tx_cmsis_alloc(sizeof(tx_cmsis_mutex_t));
        if (new_mutex == NULL) {
            return NULL;
        }
        alloced_new_mutex = true;
    } else if (attr->cb_size < sizeof(tx_cmsis_mutex_t)) {
        return NULL;
    }

    new_mutex->attr_bits = attr->attr_bits;

    UINT inherit;
    if ((new_mutex->attr_bits & osMutexPrioInherit) != 0) {
        inherit = TX_INHERIT;
    } else {
        inherit = TX_NO_INHERIT;
    }

    if (tx_mutex_create(&(new_mutex->tx_mutex), (CHAR *)attr->name, inherit) != TX_SUCCESS) {
        if (alloced_new_mutex) {
            tx_cmsis_free((VOID *)new_mutex);
        }
        return NULL;
    } else {
        return (osMutexId_t)new_mutex;
    }
}

osStatus_t osMutexDelete(osMutexId_t mutex_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (mutex_id == NULL) {
        return osErrorParameter;
    }

    tx_cmsis_mutex_t *mutex = (tx_cmsis_mutex_t *)mutex_id;
    UINT ret = tx_mutex_delete(&(mutex->tx_mutex));
    switch (ret) {
        case TX_MUTEX_ERROR:
            return osErrorParameter;
        case TX_CALLER_ERROR:
            /* Kernel not initialized yet */
            return osErrorResource;
        case TX_SUCCESS:
            break;
        default:
            /* Unexpected error */
            return osError;
    }

    if (tx_cmsis_is_in_our_heap((VOID *)mutex_id)) {
        ret = tx_cmsis_free((VOID *)mutex_id);
        switch (ret) {
            case TX_SUCCESS:
                break;
            case TX_PTR_ERROR:
                return osErrorParameter;
            case TX_CALLER_ERROR:
                /* Kernel not initialized yet */
                return osErrorResource;
            default:
                /* Unexpected error */
                return osError;
        }
    }
    return osOK;
}

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (mutex_id == NULL) {
        return osErrorParameter;
    }

    ULONG wait_option;
    if (timeout == osWaitForever) {
        wait_option = TX_WAIT_FOREVER;
    } else if (timeout == 0) {
        wait_option = TX_NO_WAIT;
    } else {
        wait_option = (ULONG)timeout;
    }

    tx_cmsis_mutex_t *mutex = (tx_cmsis_mutex_t *)mutex_id;

    /* If we are non-recursive, check if we already own the mutex. */
    if ((mutex->attr_bits & osMutexRecursive) == 0) {
        TX_THREAD *mutex_owner;
        UINT ret = tx_mutex_info_get(&(mutex->tx_mutex),
                                     NULL,         /* name */
                                     NULL,         /* count */
                                     &mutex_owner, /* owner */
                                     NULL,         /* first_suspended */
                                     NULL,         /* suspended_count */
                                     NULL          /* next_mutex */
        );
        if (ret != TX_SUCCESS) {
            /* Invalid mutex id */
            return osErrorParameter;
        }
        if (tx_thread_identify() == mutex_owner) {
            /* We already own the mutex */
            return osErrorResource;
        }
    }

    UINT ret = tx_mutex_get(&(mutex->tx_mutex), wait_option);
    switch (ret) {
        case TX_SUCCESS:
            return osOK;
        case TX_NOT_AVAILABLE:
            if (wait_option == TX_NO_WAIT) {
                return osErrorResource;
            } else {
                return osErrorTimeout;
            }
        case TX_MUTEX_ERROR:
            /* Fallthrough */
        case TX_WAIT_ERROR:
            return osErrorParameter;
        case TX_DELETED:
            /* Fallthrough */
        case TX_WAIT_ABORTED:
            /* Fallthrough */
        case TX_CALLER_ERROR:
            /* Kernel not initialized yet */
            return osErrorResource;
        default:
            /* Unexpected error */
            return osError;
    }
}

osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (mutex_id == NULL) {
        return osErrorParameter;
    }

    tx_cmsis_mutex_t *mutex = (tx_cmsis_mutex_t *)mutex_id;
    UINT ret = tx_mutex_put(&(mutex->tx_mutex));
    switch (ret) {
        case TX_SUCCESS:
            return osOK;
        case TX_NOT_OWNED:
            return osErrorResource;
        case TX_MUTEX_ERROR:
            return osErrorParameter;
        case TX_CALLER_ERROR:
            /* Kernel not initialized yet */
            return osErrorResource;
        default:
            /* Unexpected error */
            return osError;
    }
}

const char *osMutexGetName(osMutexId_t mutex_id)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }
    if (mutex_id == NULL) {
        return NULL;
    }

    const char *name = NULL;
    UINT ret;
    tx_cmsis_mutex_t *mutex = (tx_cmsis_mutex_t *)mutex_id;
    ret = tx_mutex_info_get(&(mutex->tx_mutex),
                            (CHAR **)&name, /* name */
                            NULL,           /* count */
                            NULL,           /* owner */
                            NULL,           /* first_suspended */
                            NULL,           /* suspend_count */
                            NULL            /* next_mutex */
    );
    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return name;
    }
}

osThreadId_t osMutexGetOwner(osMutexId_t mutex_id)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }
    if (mutex_id == NULL) {
        return NULL;
    }

    TX_THREAD *owner = NULL;
    UINT ret;
    tx_cmsis_mutex_t *mutex = (tx_cmsis_mutex_t *)mutex_id;
    ret = tx_mutex_info_get(&(mutex->tx_mutex),
                            NULL,   /* name */
                            NULL,   /* count */
                            &owner, /* owner */
                            NULL,   /* first_suspended */
                            NULL,   /* suspend_count */
                            NULL    /* next_mutex */
    );
    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return (osThreadId_t)owner;
    }
}

osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr)
{
    if (tx_cmsis_in_isr() || (max_count == 0) || (initial_count > max_count)) {
        return NULL;
    }

    osSemaphoreAttr_t default_attr = {.name = NULL, .attr_bits = 0, .cb_mem = NULL, .cb_size = 0};
    if (attr == NULL) {
        attr = &default_attr;
    }

    tx_cmsis_semaphore_t *new_semaphore = (tx_cmsis_semaphore_t *)attr->cb_mem;
    UINT ret;
    bool alloced_new_semaphore = false;

    if (new_semaphore == NULL) {
        new_semaphore = tx_cmsis_alloc(sizeof(tx_cmsis_semaphore_t));
        if (new_semaphore == NULL) {
            return NULL;
        }
        alloced_new_semaphore = true;
    } else if (attr->cb_size < sizeof(tx_cmsis_semaphore_t)) {
        return NULL;
    }

    new_semaphore->max_count = max_count;

    ret = tx_semaphore_create(&(new_semaphore->tx_semaphore), (CHAR *)attr->name, (ULONG)initial_count);
    if (ret != TX_SUCCESS) {
        if (alloced_new_semaphore) {
            tx_cmsis_free((VOID *)new_semaphore);
        }
        return NULL;
    } else {
        return (osSemaphoreId_t)new_semaphore;
    }
}

osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (semaphore_id == NULL) {
        return osErrorParameter;
    }

    tx_cmsis_semaphore_t *semaphore = (tx_cmsis_semaphore_t *)semaphore_id;
    UINT ret = tx_semaphore_delete(&(semaphore->tx_semaphore));
    switch (ret) {
        case TX_SEMAPHORE_ERROR:
            return osErrorParameter;
        case TX_CALLER_ERROR:
            /* Kernel not initialized yet */
            return osErrorResource;
        case TX_SUCCESS:
            break;
        default:
            /* Unexpected error */
            return osError;
    }

    if (tx_cmsis_is_in_our_heap((VOID *)semaphore_id)) {
        ret = tx_cmsis_free((VOID *)semaphore_id);
        switch (ret) {
            case TX_SUCCESS:
                break;
            case TX_PTR_ERROR:
                return osErrorParameter;
            case TX_CALLER_ERROR:
                /* Kernel not initialized yet */
                return osErrorResource;
            default:
                /* Unexpected error */
                return osError;
        }
    }
    return osOK;
}

osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout)
{
    if (((timeout != 0) && tx_cmsis_in_isr()) || (semaphore_id == NULL)) {
        return osErrorParameter;
    }

    ULONG wait_option;
    if (timeout == osWaitForever) {
        wait_option = TX_WAIT_FOREVER;
    } else if (timeout == 0) {
        wait_option = TX_NO_WAIT;
    } else {
        wait_option = (ULONG)timeout;
    }

    tx_cmsis_semaphore_t *semaphore = (tx_cmsis_semaphore_t *)semaphore_id;

    UINT ret = tx_semaphore_get(&(semaphore->tx_semaphore), wait_option);
    switch (ret) {
        case TX_SUCCESS:
            return osOK;
        case TX_NO_INSTANCE:
            if (wait_option == TX_NO_WAIT) {
                return osErrorResource;
            } else {
                return osErrorTimeout;
            }
        case TX_SEMAPHORE_ERROR:
            /* Fallthrough */
        case TX_WAIT_ERROR:
            return osErrorParameter;
        case TX_DELETED:
            /* Fallthrough */
        case TX_WAIT_ABORTED:
            return osErrorResource;
        default:
            /* Unexpected error */
            return osError;
    }
}

osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id)
{
    if (semaphore_id == NULL) {
        return osErrorParameter;
    }

    tx_cmsis_semaphore_t *semaphore = (tx_cmsis_semaphore_t *)semaphore_id;
    UINT ret = tx_semaphore_ceiling_put(&(semaphore->tx_semaphore), semaphore->max_count);
    switch (ret) {
        case TX_SUCCESS:
            return osOK;
        case TX_CEILING_EXCEEDED:
            return osErrorResource;
        case TX_INVALID_CEILING:
            /* Fallthrough */
        case TX_SEMAPHORE_ERROR:
            return osErrorParameter;
        default:
            /* Unexpected error */
            return osError;
    }
}

const char *osSemaphoreGetName(osSemaphoreId_t semaphore_id)
{
    if (tx_cmsis_in_isr() || (semaphore_id == NULL)) {
        return NULL;
    }

    const char *name = NULL;
    UINT ret;
    tx_cmsis_semaphore_t *semaphore = (tx_cmsis_semaphore_t *)semaphore_id;

    ret = tx_semaphore_info_get(&(semaphore->tx_semaphore), (CHAR **)&name, NULL, NULL, NULL, NULL);
    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return name;
    }
}

uint32_t osSemaphoreGetCount(osSemaphoreId_t semaphore_id)
{
    if (semaphore_id == NULL) {
        return 0;
    }

    ULONG count = 0;
    UINT ret;
    tx_cmsis_semaphore_t *semaphore = (tx_cmsis_semaphore_t *)semaphore_id;

    ret = tx_semaphore_info_get(&(semaphore->tx_semaphore), NULL, &count, NULL, NULL, NULL);
    if (ret != TX_SUCCESS) {
        return 0;
    } else {
        return (uint32_t)count;
    }
}

osThreadId_t osThreadGetId(void)
{
    return (osThreadId_t)tx_thread_identify();
}

/* Helper function to clean up a thread's resources (without reference to
 * whether it is correct to do so). */
static void tx_cmsis_thread_cleanup(TX_THREAD *thread)
{
    tx_cmsis_thread_ext_t *thread_ext = (tx_cmsis_thread_ext_t *)thread->tx_cmsis_extension;

#if TX_CMSIS_JOINABLE
    if (thread_ext->is_joinable) {
        /* Delete join semaphore */
        tx_semaphore_delete(&thread_ext->join_semaphore);
    }
#endif /* TX_CMSIS_JOINABLE */

#if TX_CMSIS_THREAD_FLAGS
    /* Delete thread flags object */
    tx_event_flags_delete(&thread_ext->thread_flags);
#endif /* TX_CMSIS_THREAD_FLAGS */

    /* Delete thread */
    tx_thread_delete(thread);

    /* Free control block and stack if we allocated them. */
    if (thread_ext->stack_region != NULL) {
        VOID *stack_start = thread_ext->stack_region->start;

        tx_cmsis_nonheap_region_remove(thread_ext->stack_region);
        thread_ext->stack_region = NULL;

        /* Note: we must free the memory last as its being held prevents
         * anyone from allocating in it before we have deleted its non-heap
         * region information. */
        tx_cmsis_free(stack_start);
    }
    if (tx_cmsis_is_in_our_heap((VOID *)thread)) {
        tx_cmsis_free(thread);
    }
}

static void tx_cmsis_entry_exit_notify(TX_THREAD *thread, UINT entry_exit)
{
    if (entry_exit == TX_THREAD_EXIT) {
        tx_cmsis_thread_ext_t *thread_ext = (tx_cmsis_thread_ext_t *)thread->tx_cmsis_extension;

#if TX_CMSIS_JOINABLE
        /* Set the finished flag. Note that osThreadDetach() requires us to do
         * this before starting any cleanup or other operations. */
        thread_ext->finished = true;

        if (thread_ext->is_joinable) {
            /* For joinable threads, simply signal on the join semaphore */
            tx_semaphore_put(&thread_ext->join_semaphore);
        } else {
#endif /* TX_CMSIS_JOINABLE */

            tx_cmsis_thread_cleanup(thread);

#if TX_CMSIS_JOINABLE
        }
#endif /* TX_CMSIS_JOINABLE */
    }
}

osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }

    osThreadAttr_t default_attr = {.name = NULL,
                                   .attr_bits = osThreadDetached,
                                   .cb_mem = NULL,
                                   .cb_size = 0,
                                   .stack_mem = NULL,
                                   .stack_size = 0,
                                   .priority = osPriorityNormal,
                                   .tz_module = 0,
                                   .reserved = 0};
    if (attr == NULL) {
        attr = &default_attr;
    }

    UINT threadx_priority;
    if (attr->priority == osPriorityNone) {
        /* If zero-initialized, use default priority. */
        threadx_priority = (UINT)MAP_CMSIS_PRIORITY_TO_THREADX(osPriorityNormal);
    } else if ((attr->priority == osPriorityIdle) || (attr->priority == osPriorityISR)) {
        /* Requested priority is reserved for Idle or ISR deferred thread. */
        return NULL;
    } else {
        threadx_priority = (UINT)MAP_CMSIS_PRIORITY_TO_THREADX(attr->priority);
    }

    if (threadx_priority == TX_PRIO_INVALID_FOR_CMSIS) {
        /* Invalid priority */
        return NULL;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    VOID (*localfunc)(ULONG) = (VOID(*)(ULONG))func;
#pragma GCC diagnostic pop
    tx_cmsis_extended_thread_t *new_thread = (tx_cmsis_extended_thread_t *)attr->cb_mem;
    UINT ret;
    bool alloced_new_thread = false;
    if (new_thread == NULL) {
        /* No control block supplied, allocate one */
        new_thread = tx_cmsis_alloc(sizeof(tx_cmsis_extended_thread_t));
        if (new_thread == NULL) {
            return NULL;
        }
        alloced_new_thread = true;
    } else if (attr->cb_size < sizeof(tx_cmsis_extended_thread_t)) {
        /* Control block provided, but is not big enough */
        return NULL;
    }
    VOID *stack_mem = (VOID *)attr->stack_mem;
    ULONG stack_size = (ULONG)attr->stack_size;
    tx_cmsis_region_t *stack_region = NULL;
    bool alloced_stack_mem = false;
    if (stack_mem == NULL) {
        /* Make sure we allocate some stack */
        if (stack_size == 0) {
            stack_size = OS_STACK_SIZE;
        }

        stack_mem = tx_cmsis_alloc(stack_size);
        if (stack_mem == NULL) {
            if (alloced_new_thread) {
                tx_cmsis_free(new_thread);
            }
            return NULL;
        }
        alloced_stack_mem = true;
        /* Register the thread stack as a non-heap region so that objects
         * can be created from it. */
        stack_region = tx_cmsis_nonheap_region_add(stack_mem, stack_size);
        if (stack_region == NULL) {
            if (alloced_new_thread) {
                tx_cmsis_free(new_thread);
            }
            if (alloced_stack_mem) {
                tx_cmsis_free(stack_mem);
            }
            return NULL;
        }
    }

    TX_THREAD *new_tx_thread = (TX_THREAD *)new_thread;

    ULONG time_slice;
#if OS_ROBIN_ENABLE
    time_slice = OS_ROBIN_TIMEOUT;
#else
    time_slice = TX_NO_TIME_SLICE;
#endif

    ret = tx_thread_create(new_tx_thread,      /* thread ptr */
                           (CHAR *)attr->name, /* name */
                           localfunc,          /* entry function */
                           (ULONG)argument,    /* entry input */
                           stack_mem,          /* stack start */
                           stack_size,         /* stack size */
                           threadx_priority,   /* priority */
                           threadx_priority,   /* preempt threshold */
                           time_slice,         /* time slice */
                           TX_DONT_START       /* auto start */
                                               /* Don't start the thread yet so it doesn't preempt us before we
                                                * can register a callback that frees its resources when it exits. */
    );
    if (ret != TX_SUCCESS) {
        if (stack_region != NULL) {
            tx_cmsis_nonheap_region_remove(stack_region);
            tx_cmsis_free(stack_mem);
        }
        if (alloced_new_thread) {
            tx_cmsis_free(new_thread);
        }
        return NULL;
    }

    /* Register our callback to free the thread's resources on exit. */
    ret = tx_thread_entry_exit_notify(new_tx_thread, &tx_cmsis_entry_exit_notify);

    tx_cmsis_thread_ext_t *thread_ext = &new_thread->thread_ext;

#if TX_CMSIS_THREAD_FLAGS
    if (ret == TX_SUCCESS) {
        /* Create the CMSIS thread flags for the thread. */
        ret = tx_event_flags_create(&thread_ext->thread_flags, "");
    }
#endif /* TX_CMSIS_THREAD_FLAGS */

#if TX_CMSIS_JOINABLE
    thread_ext->is_joinable = false;
    thread_ext->finished = false;
    if ((ret == TX_SUCCESS) && ((attr->attr_bits & osThreadJoinable) != 0)) {

        thread_ext->is_joinable = true;

        /* Create the join semaphore for the thread. */
        ret = tx_semaphore_create(&thread_ext->join_semaphore, "", 0);

#if TX_CMSIS_THREAD_FLAGS
        if (ret != TX_SUCCESS) {
            tx_event_flags_delete(&thread_ext->thread_flags);
        }
#endif /* TX_CMSIS_THREAD_FLAGS */
    }
#endif /* TX_CMSIS_JOINABLE */

    if (ret != TX_SUCCESS) {
        tx_thread_terminate(new_tx_thread);
        tx_thread_delete(new_tx_thread);
        if (stack_region != NULL) {
            tx_cmsis_nonheap_region_remove(stack_region);
            tx_cmsis_free(stack_mem);
        }
        if (alloced_new_thread) {
            tx_cmsis_free(new_thread);
        }
        return NULL;
    }

    /* Keep a pointer to the stack non-heap region we created earlier */
    thread_ext->stack_region = stack_region;

    /* Set the thread extension field to point to the extension. */
    new_thread->tx_thread.tx_cmsis_extension = (VOID *)thread_ext;

    /* Now that we have registered our callback tx_cmsis_entry_exit_notify(),
     * we can safely resume the thread. */
    tx_thread_resume(new_tx_thread);
    return (osThreadId_t)new_thread;
}

osStatus_t osThreadYield(void)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    tx_thread_relinquish();

    return osOK;
}

osStatus_t osThreadSuspend(osThreadId_t thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    UINT ret;
    UINT old_preempt_thresh;

    TX_THREAD *this_thread = tx_thread_identify();
    if (this_thread == (TX_THREAD *)thread_id) {
        /* We are suspending ourselves, so no need to check our state as we
         * know we're in the TX_READY state. */
        tx_thread_suspend(this_thread);
        return osOK;
    }

    if (this_thread != NULL) {
        /* Change our preemption threshold to 0 so nothing can preempt us. */
        ret = tx_thread_preemption_change(this_thread, 0, &old_preempt_thresh);
        if (ret != TX_SUCCESS) {
            return osErrorParameter;
        }
    }

    UINT state = 0;
    ret = tx_thread_info_get((TX_THREAD *)thread_id,
                             NULL,   /* name */
                             &state, /* state */
                             NULL,   /* run_count */
                             NULL,   /* priority */
                             NULL,   /* preempt_threshold */
                             NULL,   /* time_slice */
                             NULL,   /* next_thread */
                             NULL    /* next_suspended_thread */
    );

    if (ret != TX_SUCCESS) {
        if (this_thread != NULL) {
            tx_thread_preemption_change(this_thread, old_preempt_thresh, &old_preempt_thresh);
        }
        return osErrorParameter;
    }

    osStatus_t status;
    switch (state) {
        case TX_READY:
            tx_thread_suspend((TX_THREAD *)thread_id);

            status = osOK;
            break;

        case TX_SLEEP:
            /* Fallthrough */
        case TX_QUEUE_SUSP:
            /* Fallthrough */
        case TX_SEMAPHORE_SUSP:
            /* Fallthrough */
        case TX_EVENT_FLAG:
            /* Fallthrough */
        case TX_BLOCK_MEMORY:
            /* Fallthrough */
        case TX_BYTE_MEMORY:
            /* Fallthrough */
        case TX_IO_DRIVER:
            /* Fallthrough */
        case TX_FILE:
            /* Fallthrough */
        case TX_TCP_IP:
            /* Fallthrough */
        case TX_MUTEX_SUSP:
            /* Suspend the thread, putting it into a 'delayed suspend' state.
             * When it is resumed via osThreadResume(), its current wait will
             * be aborted. */
            tx_thread_suspend((TX_THREAD *)thread_id);

            status = osOK;
            break;

        case TX_SUSPENDED:
            /* Thread is already suspended, nothing to do. */
            status = osOK;
            break;

        default:
            /* TX_COMPLETED or TX_TERMINATED, can't suspend. */
            status = osErrorResource;
            break;
    }

    if (this_thread != NULL) {
        tx_thread_preemption_change(this_thread, old_preempt_thresh, &old_preempt_thresh);
    }
    return status;
}

osStatus_t osThreadResume(osThreadId_t thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    UINT ret;
    UINT old_preempt_thresh;

    TX_THREAD *this_thread = tx_thread_identify();

    if (this_thread != NULL) {
        /* Change our preemption threshold to 0 so nothing can preempt us. */
        ret = tx_thread_preemption_change(this_thread, 0, &old_preempt_thresh);
        if (ret != TX_SUCCESS) {
            return osErrorParameter;
        }
    }

    UINT state = 0;
    ret = tx_thread_info_get((TX_THREAD *)thread_id,
                             NULL,   /* name */
                             &state, /* state */
                             NULL,   /* run_count */
                             NULL,   /* priority */
                             NULL,   /* preempt_threshold */
                             NULL,   /* time_slice */
                             NULL,   /* next_thread */
                             NULL    /* next_suspended_thread */
    );

    if (ret != TX_SUCCESS) {
        if (this_thread != NULL) {
            tx_thread_preemption_change(this_thread, old_preempt_thresh, &old_preempt_thresh);
        }
        return osErrorParameter;
    }

    osStatus_t status;
    /* CMSIS resumes threads regardless of how they were suspended, but
     * ThreadX uses a different function to wake waiting threads vs
     * threads that were suspended with tx_thread_suspend(). */
    switch (state) {
        case TX_SUSPENDED:
            /* Thread was previously suspended, simply resume it.
             * This will always work as we know the thread_id is valid
             * and the state is correct. */
            tx_thread_resume((TX_THREAD *)thread_id);
            status = osOK;
            break;

        case TX_SLEEP:
            /* Fallthrough */
        case TX_QUEUE_SUSP:
            /* Fallthrough */
        case TX_SEMAPHORE_SUSP:
            /* Fallthrough */
        case TX_EVENT_FLAG:
            /* Fallthrough */
        case TX_BLOCK_MEMORY:
            /* Fallthrough */
        case TX_BYTE_MEMORY:
            /* Fallthrough */
        case TX_IO_DRIVER:
            /* Fallthrough */
        case TX_FILE:
            /* Fallthrough */
        case TX_TCP_IP:
            /* Fallthrough */
        case TX_MUTEX_SUSP:
            /* The thread is waiting on something. CMSIS-RTOSv2 requires that
             * we wake it regardless. From the documentation:
             * "The thread becomes ready regardless of the reason why the
             *  thread was blocked. Thus it is not recommended to resume a
             *  thread not suspended by osThreadSuspend."
             * See
             * https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#ga3dbad90eff394b02de76a452c84c5d80
             * for more detail.
             *
             * First suspend the thread properly to put it in 'delayed suspend'
             * state if it is not already. This will always succeed as we know
             * thread_id is valid and we know the thread's state is correct. */
            tx_thread_suspend((TX_THREAD *)thread_id);

            /* Now, cancel the wait so the thread is waiting suspended.
             * This will always succeed for the same reasons as above. */
            tx_thread_wait_abort((TX_THREAD *)thread_id);

            /* Resume thread from having been suspended by us.
             * Again, we know there will be no error. */
            tx_thread_resume((TX_THREAD *)thread_id);

            status = osOK;
            break;

        default:
            status = osErrorResource;
            break;
    }

    if (this_thread != NULL) {
        tx_thread_preemption_change(this_thread, old_preempt_thresh, &old_preempt_thresh);
    }
    return status;
}

#if TX_CMSIS_JOINABLE
osStatus_t osThreadDetach(osThreadId_t thread_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (thread_id == NULL) {
        return osErrorParameter;
    }

    TX_THREAD *thread = (TX_THREAD *)thread_id;
    tx_cmsis_thread_ext_t *thread_ext = (tx_cmsis_thread_ext_t *)thread->tx_cmsis_extension;

    if (thread_ext == NULL) {
        return osErrorParameter;
    }
    if (!thread_ext->is_joinable) {
        return osErrorResource;
    }

    UINT ret;
    TX_INTERRUPT_SAVE_AREA;

    TX_DISABLE;

    if (thread_ext->finished) {

        TX_RESTORE;
        /* The thread has already finished, so clean up its resources by
         * Join()ing it. */
        return osThreadJoin(thread_id);
    } else {
        /* Thread has not yet finished, so make it non-joinable. */

        thread_ext->is_joinable = false;

        /* Delete the join semaphore. Note that this might cause us to be
         * preempted but that is okay because this is our last operation. */
        ret = tx_semaphore_delete(&thread_ext->join_semaphore);

        TX_RESTORE;

        if (ret != TX_SUCCESS) {
            return osErrorResource;
        }
    }

    return osOK;
}

osStatus_t osThreadJoin(osThreadId_t thread_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (thread_id == NULL) {
        return osErrorParameter;
    }

    TX_THREAD *thread = (TX_THREAD *)thread_id;

    /* Check thread is valid by getting its name */
    const char *name = NULL;
    UINT ret = tx_thread_info_get(thread, (CHAR **)&name, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    if (ret != TX_SUCCESS) {
        /* Thread is invalid - this likely means it has already been
         * terminated or joined by another thread */
        return osErrorParameter;
    }
    /* Suppress compiler warnings about name being unused */
    (void)name;

    tx_cmsis_thread_ext_t *thread_ext = (tx_cmsis_thread_ext_t *)thread->tx_cmsis_extension;

    if (thread_ext == NULL) {
        return osErrorParameter;
    }
    if (!thread_ext->is_joinable) {
        return osErrorResource;
    }

    /* Wait for the thread to finish */
    ret = tx_semaphore_get(&thread_ext->join_semaphore, TX_WAIT_FOREVER);

    if (ret != TX_SUCCESS) {
        /* Thread has been detached or our wait has been aborted */
        return osErrorResource;
    }

    /* Thread has finished, so clean it up */
    tx_cmsis_thread_cleanup(thread);

    return osOK;
}
#endif /* TX_CMSIS_JOINABLE */

osStatus_t osThreadTerminate(osThreadId_t thread_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    UINT ret = tx_thread_terminate((TX_THREAD *)thread_id);
    switch (ret) {
        case TX_SUCCESS:
            return osOK;
        case TX_THREAD_ERROR:
            return osErrorParameter;
        default:
            return osErrorResource;
    }
}

__NO_RETURN void osThreadExit(void)
{
    TX_THREAD *thread_id = tx_thread_identify();
    tx_thread_terminate(thread_id);
    /* If we reach here, it means tx_thread_terminate() returned an error,
     * which means that osThreadExit() was called from a non-thread context.
     * Exiting from a non-thread makes no sense, so we can't really do much
     * here except spin (causing a complete lockup since we are a non-thread).
     */
    while (true)
        ;
}

const char *osThreadGetName(osThreadId_t thread_id)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }

    UINT ret;
    const char *name = NULL;
    ret = tx_thread_info_get((TX_THREAD *)thread_id,
                             (CHAR **)&name, /* name */
                             NULL,           /* state */
                             NULL,           /* run_count */
                             NULL,           /* priority */
                             NULL,           /* preempt_threshold */
                             NULL,           /* time_slice */
                             NULL,           /* next_thread */
                             NULL            /* next_suspended_thread */
    );
    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return name;
    }
}

osStatus_t osThreadSetPriority(osThreadId_t thread_id, osPriority_t priority)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    if ((priority == osPriorityIdle) || (priority == osPriorityISR)) {
        /* Priority is reserved for Idle or ISR deferred thread. */
        return osErrorParameter;
    }

    UINT new_prio = (UINT)MAP_CMSIS_PRIORITY_TO_THREADX(priority);
    UINT old_prio;
    UINT ret;

    if (new_prio == TX_PRIO_INVALID_FOR_CMSIS) {
        /* Priority invalid */
        return osErrorParameter;
    }

    ret = tx_thread_priority_change((TX_THREAD *)thread_id, new_prio, &old_prio);

    /* We need old_prio to get a valid pointer to pass to
     * tx_thread_priority_change() but we don't care about the value.
     * Suppress unused warnings here: */
    (void)old_prio;

    switch (ret) {
        case TX_SUCCESS:
            return osOK;
        case TX_THREAD_ERROR:
            return osErrorParameter;
        default:
            return osErrorResource;
    }
}

osPriority_t osThreadGetPriority(osThreadId_t thread_id)
{
    if (tx_cmsis_in_isr()) {
        return osPriorityError;
    }

    UINT priority = 0;

    /* Ideally we would use tx_thread_info_get() to get the priority, but
     * this doesn't give us the true priority that osThreadGetPriority()
     * is expected to return when priority inheritance is happening. */
    priority = ((TX_THREAD *)thread_id)->tx_thread_priority;

    return (osPriority_t)MAP_THREADX_PRIORITY_TO_CMSIS((int)priority);
}

/* Get the CMSIS state from ThreadX's state. Note that the conversion is not
 * reversible as ThreadX distinguishes between different kinds of blocked
 * and terminated states, whereas CMSIS does not. */
static osThreadState_t tx_cmsis_state_from_threadx(UINT threadx_state)
{
    switch (threadx_state) {
        case TX_READY:
            return osThreadReady;

        case TX_COMPLETED:
            /* Fallthrough */
        case TX_TERMINATED:
            return osThreadTerminated;

        case TX_SUSPENDED:
            /* Fallthrough */
        case TX_SLEEP:
            /* Fallthrough */
        case TX_QUEUE_SUSP:
            /* Fallthrough */
        case TX_SEMAPHORE_SUSP:
            /* Fallthrough */
        case TX_MUTEX_SUSP:
            /* Fallthrough */
        case TX_BLOCK_MEMORY:
            /* Fallthrough */
        case TX_BYTE_MEMORY:
            /* Fallthrough */
        case TX_EVENT_FLAG:
            /* Fallthrough */
        case TX_IO_DRIVER:
            /* Fallthrough */
        case TX_FILE:
            /* Fallthrough */
        case TX_TCP_IP:
            return osThreadBlocked;

        default:
            return osThreadError;
    }
}

osThreadState_t osThreadGetState(osThreadId_t thread_id)
{
    if (tx_cmsis_in_isr()) {
        return osThreadError;
    }
    /* ThreadX has no running state, but the thread can only be running
     * currently if it is the caller (i.e. we are thread_id). */
    if (tx_thread_identify() == (TX_THREAD *)thread_id) {
        return osThreadRunning;
    }

    UINT ret;
    UINT state = 0;
    ret = tx_thread_info_get((TX_THREAD *)thread_id,
                             NULL,   /* name */
                             &state, /* state */
                             NULL,   /* run_count */
                             NULL,   /* priority */
                             NULL,   /* preempt_threshold */
                             NULL,   /* time_slice */
                             NULL,   /* next_thread */
                             NULL    /* next_suspended_thread */
    );
    if (ret != TX_SUCCESS) {
        return osThreadError;
    } else {
        return tx_cmsis_state_from_threadx(state);
    }
}

uint32_t osThreadGetStackSize(osThreadId_t thread_id)
{
    if ((thread_id == NULL) || tx_cmsis_in_isr()) {
        return 0;
    }
    TX_THREAD *thread = (TX_THREAD *)thread_id;

    return (uint32_t)thread->tx_thread_stack_size;
}

#ifndef TX_DISABLE_STACK_FILLING
uint32_t osThreadGetStackSpace(osThreadId_t thread_id)
{
    if ((thread_id == NULL) || tx_cmsis_in_isr()) {
        return 0;
    }
    TX_THREAD *thread = (TX_THREAD *)thread_id;

    ULONG *start = (ULONG *)thread->tx_thread_stack_start;
    ULONG *ptr = start;

    /* This will definitely finish as it will eventually run into the stack
     * frame created in _tx_thread_stack_build if nothing else. */
    while (*ptr == TX_STACK_FILL) {
        ptr++;
    }
    uint32_t space = (uint32_t)(ptr - start) * sizeof(ULONG);

    return space;
}
#endif /* !defined(TX_DISABLE_STACK_FILLING) */

#if TX_CMSIS_THREAD_FLAGS
uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    if (thread_id == NULL) {
        return osFlagsErrorParameter;
    }

    TX_THREAD *thread = (TX_THREAD *)thread_id;

    if (thread->tx_cmsis_extension == NULL) {
        /* Thread was not created by CMSIS layer */
        return osFlagsErrorParameter;
    }

    tx_cmsis_thread_ext_t *thread_ext = (tx_cmsis_thread_ext_t *)thread->tx_cmsis_extension;
    osEventFlagsId_t thread_flags_group = (osEventFlagsId_t)&thread_ext->thread_flags;

    return osEventFlagsSet(thread_flags_group, flags);
}

uint32_t osThreadFlagsClear(uint32_t flags)
{
    if (tx_cmsis_in_isr()) {
        return osFlagsErrorISR;
    }

    TX_THREAD *current_thread = (TX_THREAD *)tx_thread_identify();
    if (current_thread == NULL) {
        return osFlagsErrorUnknown;
    }

    if (current_thread->tx_cmsis_extension == NULL) {
        /* Thread was not created by CMSIS layer */
        return osFlagsErrorParameter;
    }

    tx_cmsis_thread_ext_t *thread_ext = (tx_cmsis_thread_ext_t *)current_thread->tx_cmsis_extension;
    osEventFlagsId_t thread_flags_group = (osEventFlagsId_t)&thread_ext->thread_flags;

    return osEventFlagsClear(thread_flags_group, flags);
}

uint32_t osThreadFlagsGet(void)
{
    if (tx_cmsis_in_isr()) {
        return osFlagsErrorISR;
    }

    TX_THREAD *current_thread = (TX_THREAD *)tx_thread_identify();
    if (current_thread == NULL) {
        return 0;
    }

    if (current_thread->tx_cmsis_extension == NULL) {
        /* Thread was not created by CMSIS layer */
        return osFlagsErrorParameter;
    }

    tx_cmsis_thread_ext_t *thread_ext = (tx_cmsis_thread_ext_t *)current_thread->tx_cmsis_extension;
    osEventFlagsId_t thread_flags_group = (osEventFlagsId_t)&thread_ext->thread_flags;

    return osEventFlagsGet(thread_flags_group);
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{

    if (tx_cmsis_in_isr()) {
        return osFlagsErrorISR;
    }

    TX_THREAD *current_thread = (TX_THREAD *)tx_thread_identify();
    if (current_thread == NULL) {
        return osFlagsErrorUnknown;
    }

    if (current_thread->tx_cmsis_extension == NULL) {
        /* Thread was not created by CMSIS layer */
        return osFlagsErrorParameter;
    }

    tx_cmsis_thread_ext_t *thread_ext = (tx_cmsis_thread_ext_t *)current_thread->tx_cmsis_extension;
    osEventFlagsId_t thread_flags_group = (osEventFlagsId_t)&thread_ext->thread_flags;

    return osEventFlagsWait(thread_flags_group, flags, options, timeout);
}
#endif /* TX_CMSIS_THREAD_FLAGS */

osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t *attr)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }

    osEventFlagsAttr_t default_attr = {.name = NULL, .attr_bits = 0, .cb_mem = NULL, .cb_size = 0};
    if (attr == NULL) {
        attr = &default_attr;
    }

    TX_EVENT_FLAGS_GROUP *new_event_flags = attr->cb_mem;
    UINT ret;
    bool alloced_new_event_flags = false;

    if (new_event_flags == NULL) {
        new_event_flags = tx_cmsis_alloc(sizeof(TX_EVENT_FLAGS_GROUP));
        if (new_event_flags == NULL) {
            return NULL;
        }
        alloced_new_event_flags = true;
    } else if (attr->cb_size < sizeof(TX_EVENT_FLAGS_GROUP)) {
        return NULL;
    }

    ret = tx_event_flags_create(new_event_flags, (CHAR *)attr->name);

    if (ret != TX_SUCCESS) {
        if (alloced_new_event_flags) {
            tx_cmsis_free((VOID *)new_event_flags);
        }
        return NULL;
    } else {
        return (osEventFlagsId_t)new_event_flags;
    }
}

osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    UINT ret = tx_event_flags_delete((TX_EVENT_FLAGS_GROUP *)ef_id);

    switch (ret) {
        case TX_SUCCESS:
            break;
        case TX_GROUP_ERROR:
            return osErrorParameter;
        default:
            /* Unexpected error */
            return osError;
    }

    if (tx_cmsis_is_in_our_heap((VOID *)ef_id)) {
        ret = tx_cmsis_free((VOID *)ef_id);
        switch (ret) {
            case TX_SUCCESS:
                break;
            case TX_PTR_ERROR:
                return osErrorParameter;
            case TX_CALLER_ERROR:
                /* Kernel not initialized yet */
                return osErrorResource;
            default:
                /* Unexpected error */
                return osError;
        }
    }
    return osOK;
}

uint32_t osEventFlagsSet(osEventFlagsId_t ef_id, uint32_t flags)
{
    if ((flags & 0x80000000U) != 0U) {
        /* Highest bit of flags parameter is set */
        return osFlagsErrorParameter;
    }
    if (ef_id == NULL) {
        return osFlagsErrorParameter;
    }

    UINT ret;

    TX_EVENT_FLAGS_GROUP *event_flags_group = (TX_EVENT_FLAGS_GROUP *)ef_id;

    ULONG flags_after;

    /* Set the specified flags */
    ret = tx_event_flags_set(event_flags_group, (ULONG)flags, TX_OR);
    if (ret != TX_SUCCESS) {
        return osFlagsErrorUnknown;
    }

    /* Now get the value of the flags (note that we allow other threads to
     * preempt us and this is intentional - we return the value of the flags
     * after some waiting threads have been woken and cleared them.
     * Look for any set flag (the top bit is not counted as we use this
     * for error codes). */
    ret = tx_event_flags_get(event_flags_group, 0x7fffffff, TX_OR, &flags_after, TX_NO_WAIT);
    if ((ret != TX_SUCCESS) && (ret != TX_NO_EVENTS)) {
        return osFlagsErrorUnknown;
    }
    if (ret == TX_NO_EVENTS) {
        /* No flags were set, so the value of the flags must be 0 */
        flags_after = 0;
    }

    /* Return the flags after setting */
    return (uint32_t)flags_after;
}

uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags)
{
    if ((flags & 0x80000000U) != 0U) {
        /* Highest bit of flags parameter is set */
        return osFlagsErrorParameter;
    }
    if (ef_id == NULL) {
        return osFlagsErrorParameter;
    }

    UINT ret;

    TX_EVENT_FLAGS_GROUP *event_flags_group = (TX_EVENT_FLAGS_GROUP *)ef_id;

    TX_INTERRUPT_SAVE_AREA;
    ULONG current_flags;

    /* Ensure noone changes the flags between reading and clearing them */
    TX_DISABLE;

    /* Look for any set flag (the top bit is not counted as we use this
     * for error codes). */
    ret = tx_event_flags_get(event_flags_group, 0x7fffffff, TX_OR, &current_flags, TX_NO_WAIT);
    if ((ret != TX_SUCCESS) && (ret != TX_NO_EVENTS)) {
        TX_RESTORE;
        return osFlagsErrorUnknown;
    }
    if (ret == TX_NO_EVENTS) {
        /* No flags were set, so the value of the flags must be 0 */
        current_flags = 0;
    }

    /* Clear the specified flags */
    ret = tx_event_flags_set(event_flags_group, ~((ULONG)flags), TX_AND);
    if (ret != TX_SUCCESS) {
        TX_RESTORE;
        return osFlagsErrorUnknown;
    }

    TX_RESTORE;

    return (uint32_t)current_flags;
}

uint32_t osEventFlagsGet(osEventFlagsId_t ef_id)
{
    TX_EVENT_FLAGS_GROUP *event_flags_group = (TX_EVENT_FLAGS_GROUP *)ef_id;

    UINT ret;
    ULONG flags;
    /* Look for any set flag (the top bit is not counted as we use this
     * for error codes). */
    ret = tx_event_flags_get(event_flags_group, 0x7fffffff, TX_OR, &flags, TX_NO_WAIT);

    if ((ret != TX_SUCCESS) && (ret != TX_NO_EVENTS)) {
        /* Return 0 on error */
        return 0;
    } else {
        if (ret == TX_NO_EVENTS) {
            /* No flags were set, so the value of the flags must be 0 */
            flags = 0;
        }
        return (uint32_t)flags;
    }
}

uint32_t osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout)
{
    if ((timeout != 0) && tx_cmsis_in_isr()) {
        return osFlagsErrorParameter;
    }
    if ((flags & 0x80000000U) != 0U) {
        /* Highest bit of flags parameter is set */
        return osFlagsErrorParameter;
    }
    if (ef_id == NULL) {
        return osFlagsErrorParameter;
    }

    UINT ret;

    TX_EVENT_FLAGS_GROUP *event_flags_group = (TX_EVENT_FLAGS_GROUP *)ef_id;

    ULONG current_flags;
    UINT get_option;
    ULONG wait_option;

    if ((options & osFlagsWaitAll) != 0) {
        if ((options & osFlagsNoClear) != 0) {
            get_option = TX_AND;
        } else {
            get_option = TX_AND_CLEAR;
        }
    } else {
        if ((options & osFlagsNoClear) != 0) {
            get_option = TX_OR;
        } else {
            get_option = TX_OR_CLEAR;
        }
    }

    switch (timeout) {
        case 0:
            wait_option = TX_NO_WAIT;
            break;
        case osWaitForever:
            wait_option = TX_WAIT_FOREVER;
            break;
        default:
            wait_option = (ULONG)timeout;
            break;
    }

    ret = tx_event_flags_get(event_flags_group, flags, get_option, &current_flags, wait_option);
    switch (ret) {
        case TX_SUCCESS:
            return (uint32_t)current_flags;
        case TX_NO_EVENTS:
            if (timeout == 0) {
                return osFlagsErrorResource;
            } else {
                return osFlagsErrorTimeout;
            }
        case TX_GROUP_ERROR:
            return osFlagsErrorParameter;
        default:
            return osFlagsErrorUnknown;
    }
}

const char *osEventFlagsGetName(osEventFlagsId_t ef_id)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }

    const char *name = NULL;

    TX_EVENT_FLAGS_GROUP *event_flags_group = (TX_EVENT_FLAGS_GROUP *)ef_id;

    UINT ret = tx_event_flags_info_get(event_flags_group, /* group_ptr */
                                       (CHAR **)&name,    /* name */
                                       NULL,              /* current_flags */
                                       NULL,              /* first_suspended */
                                       NULL,              /* suspended_count */
                                       NULL               /* next_group */
    );

    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return name;
    }
}

osTimerId_t osTimerNew(osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr)
{
    if ((func == NULL) || (tx_cmsis_in_isr())) {
        return NULL;
    }

    osTimerAttr_t default_attr = {.name = NULL, .attr_bits = 0, .cb_mem = NULL, .cb_size = 0};
    if (attr == NULL) {
        attr = &default_attr;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    VOID (*timer_func)(ULONG) = (VOID(*)(ULONG))func;
#pragma GCC diagnostic pop

    /* The values of initial_ticks and reschedule_ticks do not matter as
     * we'll change them in osTimerStart() anyway. Use round numbers in
     * human-friendly base 10. If a bug causes these numbers to be used,
     * the timer will fire regularly and obviously, to aid debugging. */
    ULONG initial_ticks = 10;
    ULONG reschedule_ticks = 10;

    tx_cmsis_timer_t *new_timer = (tx_cmsis_timer_t *)attr->cb_mem;
    UINT ret;
    bool alloced_new_timer = false;

    if (new_timer == NULL) {
        new_timer = tx_cmsis_alloc(sizeof(tx_cmsis_timer_t));
        if (new_timer == NULL) {
            return NULL;
        }
        alloced_new_timer = true;
    } else if (attr->cb_size < sizeof(tx_cmsis_timer_t)) {
        return NULL;
    }

    new_timer->type = type;

    ret = tx_timer_create(&new_timer->tx_timer,
                          (CHAR *)attr->name,
                          timer_func,
                          (ULONG)argument,
                          initial_ticks,
                          reschedule_ticks,
                          TX_NO_ACTIVATE);

    if (ret != TX_SUCCESS) {
        if (alloced_new_timer) {
            tx_cmsis_free(new_timer);
        }
        return NULL;
    } else {
        return (osTimerId_t)new_timer;
    }
}

osStatus_t osTimerDelete(osTimerId_t timer_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    tx_cmsis_timer_t *timer = (tx_cmsis_timer_t *)timer_id;
    UINT ret = tx_timer_delete(&timer->tx_timer);

    switch (ret) {
        case TX_SUCCESS:
            break;
        case TX_TIMER_ERROR:
            return osErrorParameter;
        case TX_CALLER_ERROR:
            return osErrorResource;
        default:
            /* Unexpected error */
            return osError;
    }

    if (tx_cmsis_is_in_our_heap((VOID *)timer)) {
        ret = tx_cmsis_free((VOID *)timer);
        switch (ret) {
            case TX_SUCCESS:
                break;
            case TX_PTR_ERROR:
                return osErrorParameter;
            case TX_CALLER_ERROR:
                /* Kernel not initialized yet */
                return osErrorResource;
            default:
                /* Unexpected error */
                return osError;
        }
    }
    return osOK;
}

osStatus_t osTimerStart(osTimerId_t timer_id, uint32_t ticks)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (timer_id == 0) {
        return osErrorParameter;
    }
    if (ticks == 0) {
        return osErrorParameter;
    }

    UINT ret;
    TX_INTERRUPT_SAVE_AREA;

    ULONG initial_ticks;
    ULONG reschedule_ticks;

    initial_ticks = (ULONG)ticks;

    tx_cmsis_timer_t *timer = (tx_cmsis_timer_t *)timer_id;
    if (timer->type == osTimerPeriodic) {
        reschedule_ticks = initial_ticks;
    } else {
        reschedule_ticks = 0;
    }

    /* Ensure we modify and start timer atomically */
    TX_DISABLE;

    /* First, make sure the timer is deactivated */
    ret = tx_timer_deactivate(&timer->tx_timer);
    if (ret != TX_SUCCESS) {
        TX_RESTORE;
        return osErrorParameter;
    }
    /* Next change the timer to the requested period */
    ret = tx_timer_change(&timer->tx_timer, initial_ticks, reschedule_ticks);
    if (ret != TX_SUCCESS) {
        TX_RESTORE;
        return osErrorResource;
    }
    /* Finally, activate the timer */
    ret = tx_timer_activate(&timer->tx_timer);

    TX_RESTORE;

    return (ret == TX_SUCCESS) ? osOK : osErrorResource;
}

osStatus_t osTimerStop(osTimerId_t timer_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    tx_cmsis_timer_t *timer = (tx_cmsis_timer_t *)timer_id;

    UINT ret;
    UINT timer_active;

    /* Check whether timer is active - if not we must return osErrorResource */
    ret = tx_timer_info_get(&timer->tx_timer,
                            NULL,          /* name */
                            &timer_active, /* active */
                            NULL,          /* remaining_ticks */
                            NULL,          /* reschedule_ticks */
                            NULL           /* next_timer */
    );

    if (ret != TX_SUCCESS) {
        return osErrorParameter;
    }
    if (timer_active == TX_FALSE) {
        return osErrorResource;
    }

    ret = tx_timer_deactivate(&timer->tx_timer);

    return (ret == TX_SUCCESS) ? osOK : osErrorParameter;
}

uint32_t osTimerIsRunning(osTimerId_t timer_id)
{
    if (tx_cmsis_in_isr()) {
        return 0;
    }

    tx_cmsis_timer_t *timer = (tx_cmsis_timer_t *)timer_id;

    UINT ret;
    UINT timer_active;

    ret = tx_timer_info_get(&timer->tx_timer,
                            NULL,          /* name */
                            &timer_active, /* active */
                            NULL,          /* remaining_ticks */
                            NULL,          /* reschedule_ticks */
                            NULL           /* next_timer */
    );

    if (ret != TX_SUCCESS) {
        return 0;
    }

    return timer_active == TX_TRUE;
}

const char *osTimerGetName(osTimerId_t timer_id)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }

    tx_cmsis_timer_t *timer = (tx_cmsis_timer_t *)timer_id;

    UINT ret;
    const char *name = NULL;

    ret = tx_timer_info_get(&timer->tx_timer,
                            (CHAR **)&name, /* name */
                            NULL,           /* active */
                            NULL,           /* remaining_ticks */
                            NULL,           /* reschedule_ticks */
                            NULL            /* next_timer */
    );

    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return name;
    }
}

osMemoryPoolId_t osMemoryPoolNew(uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }

    osMemoryPoolAttr_t default_attr = {
        .name = NULL, .attr_bits = 0, .cb_mem = NULL, .cb_size = 0, .mp_mem = NULL, .mp_size = 0};
    if (attr == NULL) {
        attr = &default_attr;
    }

    tx_cmsis_memory_pool_t *new_memory_pool = (tx_cmsis_memory_pool_t *)attr->cb_mem;
    UINT ret;
    bool alloced_new_memory_pool = false;

    if (new_memory_pool == NULL) {
        new_memory_pool = tx_cmsis_alloc(sizeof(tx_cmsis_memory_pool_t));
        if (new_memory_pool == NULL) {
            return NULL;
        }
        alloced_new_memory_pool = true;
    } else if (attr->cb_size < sizeof(tx_cmsis_memory_pool_t)) {
        return NULL;
    }

    /* Initialize the recorded backing region to NULL */
    new_memory_pool->backing_region = NULL;
    /* Set the block size and count members */
    new_memory_pool->block_size = block_size;
    new_memory_pool->block_count = block_count;

    /* We need enough memory to store the requested blocks plus one pointer's
     * worth of overhead per block that ThreadX uses. */
    ULONG pool_size = (ULONG)block_count * ((ULONG)block_size + sizeof(void *)) + sizeof(void *);

    VOID *pool_mem = (VOID *)attr->mp_mem;
    bool alloced_pool_mem = false;
    if (pool_mem == NULL) {
        pool_mem = tx_cmsis_alloc(pool_size);
        if (pool_mem == NULL) {
            if (alloced_new_memory_pool) {
                tx_cmsis_free((VOID *)new_memory_pool);
            }
            return NULL;
        }
        alloced_pool_mem = true;

        /* Add the new memory pool's backing memory bounds to the list of non-heap
         * regions to check in tx_cmsis_is_in_our_heap(). */
        tx_cmsis_region_t *pool_region = tx_cmsis_nonheap_region_add(pool_mem, pool_size);

        if (pool_region == NULL) {
            if (alloced_new_memory_pool) {
                tx_cmsis_free((VOID *)new_memory_pool);
            }
            if (alloced_pool_mem) {
                tx_cmsis_free((VOID *)pool_mem);
            }
            return NULL;
        }
        /* Store the tx_cmsis_region_t pointer, recording the memory region used. */
        new_memory_pool->backing_region = pool_region;

    } else if (attr->mp_size < pool_size) {
        if (alloced_new_memory_pool) {
            tx_cmsis_free((VOID *)new_memory_pool);
        }
        return NULL;
    }

    ret = tx_block_pool_create(
        &new_memory_pool->tx_block_pool, (CHAR *)attr->name, (ULONG)block_size, pool_mem, pool_size);

    if (ret != TX_SUCCESS) {
        if (new_memory_pool->backing_region != NULL) {
            tx_cmsis_nonheap_region_remove(new_memory_pool->backing_region);
        }
        if (alloced_new_memory_pool) {
            tx_cmsis_free((VOID *)new_memory_pool);
        }
        if (alloced_pool_mem) {
            tx_cmsis_free((VOID *)pool_mem);
        }
        return NULL;
    }

    return (osMemoryPoolId_t)new_memory_pool;
}

osStatus_t osMemoryPoolDelete(osMemoryPoolId_t mp_id)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    tx_cmsis_memory_pool_t *mem_pool = (tx_cmsis_memory_pool_t *)mp_id;
    UINT ret;

    ret = tx_block_pool_delete(&mem_pool->tx_block_pool);
    if (ret != TX_SUCCESS) {
        return osErrorParameter;
    }

    /* For backing memory, free it if it has a non-NULL backing_region pointer
     * as this means we earlier allocated it and registered it as non-heap. */
    if (mem_pool->backing_region != NULL) {
        /* Get the start of the backing memory from the region struct */
        VOID *region_start = (VOID *)mem_pool->backing_region->start;

        /* Remove and free the non-heap region struct */
        tx_cmsis_nonheap_region_remove(mem_pool->backing_region);
        mem_pool->backing_region = NULL;

        /* Free the actual backing memory */
        ret = tx_cmsis_free(region_start);
        if (ret != TX_SUCCESS) {
            return osErrorParameter;
        }
    }

    /* Free the memory pool struct if we allocated it. */
    if (tx_cmsis_is_in_our_heap(mem_pool)) {
        ret = tx_cmsis_free(mem_pool);
        if (ret != TX_SUCCESS) {
            return osErrorParameter;
        }
    }

    return osOK;
}

void *osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
    if ((timeout != 0) && tx_cmsis_in_isr()) {
        return NULL;
    }

    tx_cmsis_memory_pool_t *mem_pool = (tx_cmsis_memory_pool_t *)mp_id;

    void *block;
    UINT ret;

    ULONG wait_option;
    if (timeout == osWaitForever) {
        wait_option = TX_WAIT_FOREVER;
    } else if (timeout == 0) {
        wait_option = TX_NO_WAIT;
    } else {
        wait_option = (ULONG)timeout;
    }

    ret = tx_block_allocate(&mem_pool->tx_block_pool, (VOID **)&block, wait_option);

    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return block;
    }
}

osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void *block)
{
    /* ThreadX figures out which pool a pointer belongs to by itself, so we
     * don't actually need mp_id, but we need to return an error if it is NULL
     * to comply with the CMSIS-RTOSv2 specification. */
    if (mp_id == NULL) {
        return osErrorParameter;
    }

    UINT ret = tx_block_release((VOID *)block);

    if (ret != TX_SUCCESS) {
        return osErrorParameter;
    } else {
        return osOK;
    }
}

uint32_t osMemoryPoolGetCapacity(osMemoryPoolId_t mp_id)
{
    if (mp_id == NULL) {
        return 0;
    }
    tx_cmsis_memory_pool_t *mem_pool = (tx_cmsis_memory_pool_t *)mp_id;

    return mem_pool->block_count;
}

uint32_t osMemoryPoolGetBlockSize(osMemoryPoolId_t mp_id)
{
    if (mp_id == NULL) {
        return 0;
    }
    tx_cmsis_memory_pool_t *mem_pool = (tx_cmsis_memory_pool_t *)mp_id;

    return mem_pool->block_size;
}

uint32_t osMemoryPoolGetCount(osMemoryPoolId_t mp_id)
{
    if (mp_id == NULL) {
        return 0;
    }
    tx_cmsis_memory_pool_t *mem_pool = (tx_cmsis_memory_pool_t *)mp_id;

    ULONG spaces;
    ULONG total_blocks;
    UINT ret = tx_block_pool_info_get(&mem_pool->tx_block_pool,
                                      NULL,          /* name */
                                      &spaces,       /* available */
                                      &total_blocks, /* total_blocks */
                                      NULL,          /* first_suspended */
                                      NULL,          /* suspended_count */
                                      NULL           /* next_pool */
    );
    if (ret != TX_SUCCESS) {
        return 0;
    } else {
        return (total_blocks - spaces);
    }
}

uint32_t osMemoryPoolGetSpace(osMemoryPoolId_t mp_id)
{
    if (mp_id == NULL) {
        return 0;
    }
    tx_cmsis_memory_pool_t *mem_pool = (tx_cmsis_memory_pool_t *)mp_id;

    ULONG spaces;
    UINT ret = tx_block_pool_info_get(&mem_pool->tx_block_pool,
                                      NULL,    /* name */
                                      &spaces, /* available */
                                      NULL,    /* total_blocks */
                                      NULL,    /* first_suspended */
                                      NULL,    /* suspended_count */
                                      NULL     /* next_pool */
    );
    if (ret != TX_SUCCESS) {
        return 0;
    } else {
        return spaces;
    }
}

const char *osMemoryPoolGetName(osMemoryPoolId_t mp_id)
{

    if (tx_cmsis_in_isr()) {
        return NULL;
    }
    if (mp_id == NULL) {
        return NULL;
    }

    const char *name = NULL;
    UINT ret;
    tx_cmsis_memory_pool_t *mem_pool = (tx_cmsis_memory_pool_t *)mp_id;
    ret = tx_block_pool_info_get(&mem_pool->tx_block_pool,
                                 (CHAR **)&name, /* name */
                                 NULL,           /* available */
                                 NULL,           /* total_blocks */
                                 NULL,           /* first_suspended */
                                 NULL,           /* suspended_count */
                                 NULL            /* next_pool */
    );
    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return name;
    }
}

/* Create a new message queue. Note that messages can only be up to 16 words
 * (64 bytes) long in this implementation due to ThreadX limitations. */
osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }

    osMessageQueueAttr_t default_attr = {
        .name = NULL, .attr_bits = 0, .cb_mem = NULL, .cb_size = 0, .mq_mem = NULL, .mq_size = 0};
    if (attr == NULL) {
        attr = &default_attr;
    }

    tx_cmsis_msg_queue_t *new_msg_queue = (tx_cmsis_msg_queue_t *)attr->cb_mem;
    UINT ret;
    bool alloced_new_msg_queue = false;

    if (new_msg_queue == NULL) {
        new_msg_queue = tx_cmsis_alloc(sizeof(tx_cmsis_msg_queue_t));
        if (new_msg_queue == NULL) {
            return NULL;
        }
        alloced_new_msg_queue = true;
    } else if (attr->cb_size < sizeof(tx_cmsis_msg_queue_t)) {
        return NULL;
    }

    new_msg_queue->msg_size = msg_size;
    /* Message sizes must be 32-bit/4-byte aligned as ThreadX queues measure
     * sizes at 4-byte word granularity. */
    size_t padding_length = (4 - (msg_size % 4)) % 4;
    new_msg_queue->msg_storage_size = msg_size + padding_length;
    new_msg_queue->msg_count = msg_count;
    new_msg_queue->queue_mem = NULL;

    ULONG queue_mem_size = (ULONG)msg_count * (ULONG)new_msg_queue->msg_storage_size;

    VOID *queue_mem = (VOID *)attr->mq_mem;
    bool alloced_queue_mem = false;
    if (queue_mem == NULL) {
        queue_mem = tx_cmsis_alloc(queue_mem_size);
        if (queue_mem == NULL) {
            if (alloced_new_msg_queue) {
                tx_cmsis_free((VOID *)new_msg_queue);
            }
            return NULL;
        }
        alloced_queue_mem = true;
        /* Setup pointer to our allocated memory region */
        new_msg_queue->queue_mem = queue_mem;

    } else if ((attr->mq_size < queue_mem_size) || (((uintptr_t)attr->mq_mem % sizeof(ULONG)) != 0)) {
        if (alloced_new_msg_queue) {
            tx_cmsis_free((VOID *)new_msg_queue);
        }
        return NULL;
    }

    /* ThreadX measures message sizes in 32-bit words */
    UINT tx_msg_size = (UINT)new_msg_queue->msg_storage_size / 4;

    ret = tx_queue_create(
        &new_msg_queue->tx_queue, (CHAR *)attr->name, tx_msg_size, (VOID *)queue_mem, (ULONG)queue_mem_size);

    if (ret != TX_SUCCESS) {
        if (alloced_new_msg_queue) {
            tx_cmsis_free((VOID *)new_msg_queue);
        }
        if (alloced_queue_mem) {
            tx_cmsis_free((VOID *)queue_mem);
        }
        return NULL;
    }

    return (osMessageQueueId_t)new_msg_queue;
}

osStatus_t osMessageQueueDelete(osMessageQueueId_t mq_id)
{
    if (mq_id == NULL) {
        return osErrorParameter;
    }
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;
    UINT ret;

    ret = tx_queue_delete(&msg_queue->tx_queue);
    if (ret != TX_SUCCESS) {
        return osErrorParameter;
    }
    /* Free backing memory if we allocated it. */
    if (msg_queue->queue_mem != NULL) {
        ret = tx_cmsis_free(msg_queue->queue_mem);
        if (ret != TX_SUCCESS) {
            return osErrorParameter;
        }
    }
    /* Free the memory pool struct if we allocated it. */
    if (tx_cmsis_is_in_our_heap(msg_queue)) {
        ret = tx_cmsis_free(msg_queue);
        if (ret != TX_SUCCESS) {
            return osErrorParameter;
        }
    }

    return osOK;
}

/* Note that msg_prio is ignored in this implementation as ThreadX queues have
 * no priority sorting. */
osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
    uint8_t msg_copy[TX_MESSAGE_MAX_BYTE_SIZE];
    if ((timeout != 0) && tx_cmsis_in_isr()) {
        return osErrorParameter;
    }
    if (mq_id == NULL) {
        return osErrorParameter;
    }

    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;

    /* We do not use the priority so suppress unused warnings */
    (void)msg_prio;

    ULONG wait_option;
    if (timeout == osWaitForever) {
        wait_option = TX_WAIT_FOREVER;
    } else if (timeout == 0) {
        wait_option = TX_NO_WAIT;
    } else {
        wait_option = (ULONG)timeout;
    }

    if (msg_queue->msg_size != msg_queue->msg_storage_size) {
        memcpy(msg_copy, msg_ptr, msg_queue->msg_size);
        msg_ptr = msg_copy;
    }

    UINT ret;
    ret = tx_queue_send(&msg_queue->tx_queue, (VOID *)msg_ptr, wait_option);

    switch (ret) {
        case TX_SUCCESS:
            return osOK;

        case TX_QUEUE_FULL:
            if (timeout == 0) {
                return osErrorResource;
            } else {
                return osErrorTimeout;
            }

        case TX_DELETED:
            /* Fallthrough */
        case TX_WAIT_ABORTED:
            return osErrorResource;

        case TX_QUEUE_ERROR:
            /* Fallthrough */
        case TX_PTR_ERROR:
            /* Fallthrough */
        case TX_WAIT_ERROR:
            return osErrorParameter;

        default:
            /* Unexpected error */
            return osError;
    }
}

osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
    if ((timeout != 0) && tx_cmsis_in_isr()) {
        return osErrorParameter;
    }
    if (mq_id == NULL) {
        return osErrorParameter;
    }

    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;

    if (msg_prio != NULL) {
        /* Always return a priority of zero */
        *msg_prio = 0;
    }

    ULONG wait_option;
    if (timeout == osWaitForever) {
        wait_option = TX_WAIT_FOREVER;
    } else if (timeout == 0) {
        wait_option = TX_NO_WAIT;
    } else {
        wait_option = (ULONG)timeout;
    }

    UINT ret;
    if (msg_queue->msg_size != msg_queue->msg_storage_size) {
        uint8_t msg_receive[TX_MESSAGE_MAX_BYTE_SIZE];
        ret = tx_queue_receive(&msg_queue->tx_queue, (VOID *)msg_receive, wait_option);
        if (ret == TX_SUCCESS) {
            memcpy(msg_ptr, msg_receive, msg_queue->msg_size);
        }
    } else {
        ret = tx_queue_receive(&msg_queue->tx_queue, (VOID *)msg_ptr, wait_option);
    }

    switch (ret) {
        case TX_SUCCESS:
            return osOK;

        case TX_QUEUE_EMPTY:
            if (timeout == 0) {
                return osErrorResource;
            } else {
                return osErrorTimeout;
            }

        case TX_DELETED:
            /* Fallthrough */
        case TX_WAIT_ABORTED:
            return osErrorResource;

        case TX_QUEUE_ERROR:
            /* Fallthrough */
        case TX_PTR_ERROR:
            /* Fallthrough */
        case TX_WAIT_ERROR:
            return osErrorParameter;

        default:
            /* Unexpected error */
            return osError;
    }
}

osStatus_t osMessageQueueReset(osMessageQueueId_t mq_id)
{
    if (mq_id == NULL) {
        return osErrorParameter;
    }
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }

    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;
    UINT ret;

    ret = tx_queue_flush(&msg_queue->tx_queue);

    if (ret != TX_SUCCESS) {
        return osErrorParameter;
    } else {
        return osOK;
    }
}

uint32_t osMessageQueueGetCapacity(osMessageQueueId_t mq_id)
{
    if (mq_id == NULL) {
        return 0;
    }
    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;

    return msg_queue->msg_count;
}

uint32_t osMessageQueueGetMsgSize(osMessageQueueId_t mq_id)
{
    if (mq_id == NULL) {
        return 0;
    }
    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;

    return msg_queue->msg_size;
}

uint32_t osMessageQueueGetCount(osMessageQueueId_t mq_id)
{
    if (mq_id == NULL) {
        return 0;
    }
    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;

    ULONG count;
    UINT ret = tx_queue_info_get(&msg_queue->tx_queue,
                                 NULL,   /* name */
                                 &count, /* enqueued */
                                 NULL,   /* available_storage */
                                 NULL,   /* first_suspended */
                                 NULL,   /* suspended_count */
                                 NULL    /* next_pool */
    );
    if (ret != TX_SUCCESS) {
        return 0;
    } else {
        return count;
    }
}

uint32_t osMessageQueueGetSpace(osMessageQueueId_t mq_id)
{
    if (mq_id == NULL) {
        return 0;
    }
    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;

    ULONG space;
    UINT ret = tx_queue_info_get(&msg_queue->tx_queue,
                                 NULL,   /* name */
                                 NULL,   /* enqueued */
                                 &space, /* available_storage */
                                 NULL,   /* first_suspended */
                                 NULL,   /* suspended_count */
                                 NULL    /* next_pool */
    );
    if (ret != TX_SUCCESS) {
        return 0;
    } else {
        return space;
    }
}

const char *osMessageQueueGetName(osMessageQueueId_t mq_id)
{
    if (tx_cmsis_in_isr()) {
        return NULL;
    }
    if (mq_id == NULL) {
        return NULL;
    }

    const char *name = NULL;
    UINT ret;
    tx_cmsis_msg_queue_t *msg_queue = (tx_cmsis_msg_queue_t *)mq_id;
    ret = tx_queue_info_get(&msg_queue->tx_queue,
                            (CHAR **)&name, /* name */
                            NULL,           /* available */
                            NULL,           /* total_blocks */
                            NULL,           /* first_suspended */
                            NULL,           /* suspended_count */
                            NULL            /* next_pool */
    );
    if (ret != TX_SUCCESS) {
        return NULL;
    } else {
        return name;
    }
}

osStatus_t osDelay(uint32_t ticks)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    if (ticks == 0) {
        return osErrorParameter;
    }

    UINT ret = tx_thread_sleep(ticks);
    if (ret == TX_SUCCESS) {
        return osOK;
    } else {
        return osError;
    }
}

osStatus_t osDelayUntil(uint32_t ticks)
{
    if (tx_cmsis_in_isr()) {
        return osErrorISR;
    }
    /* The case where ticks is less than the current tick count is handled
     * automatically here by unsigned integer wrap-around because ThreadX's
     * tick counter happens to wrap at 0xffffffff (2^32 - 1). */
    uint32_t delay = ticks - osKernelGetTickCount();

    if (delay > 0x7fffffff) {
        return osErrorParameter;
    }
    return osDelay(delay);
}
