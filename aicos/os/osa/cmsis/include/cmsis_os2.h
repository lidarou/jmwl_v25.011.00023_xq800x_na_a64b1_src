/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CMSIS_OS2_H_
#define CMSIS_OS2_H_

#include <stddef.h>
#include <stdint.h>

#ifndef __NO_RETURN
#define __NO_RETURN
#endif

typedef void *osMutexId_t;
typedef void *osSemaphoreId_t;
typedef void *osThreadId_t;
typedef void *osEventFlagsId_t;
typedef void *osTimerId_t;
typedef void *osMemoryPoolId_t;
typedef void *osMessageQueueId_t;

typedef void (*osThreadFunc_t)(void *);
typedef void (*osTimerFunc_t)(void *);

typedef uint32_t TZ_ModuleId_t;

typedef enum _osStatus_t {
    osOK = 0,
    osError = -1,
    osErrorTimeout = -2,
    osErrorResource = -3,
    osErrorParameter = -4,
    osErrorNoMemory = -5,
    osErrorISR = -6,
    osStatusReserved = 0x7fffffff
} osStatus_t;

typedef enum _osKernelState_t {
    osKernelInactive = 0,
    osKernelReady = 1,
    osKernelRunning = 2,
    osKernelLocked = 3,
    osKernelSuspended = 4,
    osKernelError = -1,
    osKernelReserved = 0x7fffffff
} osKernelState_t;

typedef enum _osThreadState_t {
    osThreadInactive = 0,
    osThreadReady = 1,
    osThreadRunning = 2,
    osThreadBlocked = 3,
    osThreadTerminated = 4,
    osThreadError = -1,
    osThreadReserved = 0x7fffffff
} osThreadState_t;

typedef enum _osPriority_t {
    osPriorityNone = 0,
    osPriorityIdle = 1,

    osPriorityLow = 8,
    osPriorityLow1 = 8 + 1,
    osPriorityLow2 = 8 + 2,
    osPriorityLow3 = 8 + 3,
    osPriorityLow4 = 8 + 4,
    osPriorityLow5 = 8 + 5,
    osPriorityLow6 = 8 + 6,
    osPriorityLow7 = 8 + 7,

    osPriorityBelowNormal = 16,
    osPriorityBelowNormal1 = 16 + 1,
    osPriorityBelowNormal2 = 16 + 2,
    osPriorityBelowNormal3 = 16 + 3,
    osPriorityBelowNormal4 = 16 + 4,
    osPriorityBelowNormal5 = 16 + 5,
    osPriorityBelowNormal6 = 16 + 6,
    osPriorityBelowNormal7 = 16 + 7,

    osPriorityNormal = 24,
    osPriorityNormal1 = 24 + 1,
    osPriorityNormal2 = 24 + 2,
    osPriorityNormal3 = 24 + 3,
    osPriorityNormal4 = 24 + 4,
    osPriorityNormal5 = 24 + 5,
    osPriorityNormal6 = 24 + 6,
    osPriorityNormal7 = 24 + 7,

    osPriorityAboveNormal = 32,
    osPriorityAboveNormal1 = 32 + 1,
    osPriorityAboveNormal2 = 32 + 2,
    osPriorityAboveNormal3 = 32 + 3,
    osPriorityAboveNormal4 = 32 + 4,
    osPriorityAboveNormal5 = 32 + 5,
    osPriorityAboveNormal6 = 32 + 6,
    osPriorityAboveNormal7 = 32 + 7,

    osPriorityHigh = 40,
    osPriorityHigh1 = 40 + 1,
    osPriorityHigh2 = 40 + 2,
    osPriorityHigh3 = 40 + 3,
    osPriorityHigh4 = 40 + 4,
    osPriorityHigh5 = 40 + 5,
    osPriorityHigh6 = 40 + 6,
    osPriorityHigh7 = 40 + 7,

    osPriorityRealtime = 48,
    osPriorityRealtime1 = 48 + 1,
    osPriorityRealtime2 = 48 + 2,
    osPriorityRealtime3 = 48 + 3,
    osPriorityRealtime4 = 48 + 4,
    osPriorityRealtime5 = 48 + 5,
    osPriorityRealtime6 = 48 + 6,
    osPriorityRealtime7 = 48 + 7,

    osPriorityISR = 56,
    osPriorityError = -1,
    osPriorityReserved = 0x7fffffff
} osPriority_t;

typedef enum _osTimerType_t { osTimerOnce = 0, osTimerPeriodic = 1 } osTimerType_t;

typedef struct _osVersion_t {
    uint32_t api;
    uint32_t kernel;
} osVersion_t;

typedef struct _osThreadAttr_t {
    const char *name;
    uint32_t attr_bits;
    void *cb_mem;
    uint32_t cb_size;
    void *stack_mem;
    uint32_t stack_size;
    osPriority_t priority;
    TZ_ModuleId_t tz_module;
    uint32_t reserved;
} osThreadAttr_t;

typedef struct _osMutexAttr_t {
    const char *name;
    uint32_t attr_bits;
    void *cb_mem;
    uint32_t cb_size;
} osMutexAttr_t;

typedef struct _osSemaphoreAttr_t {
    const char *name;
    uint32_t attr_bits;
    void *cb_mem;
    uint32_t cb_size;
} osSemaphoreAttr_t;

typedef struct _osEventFlagsAttr_t {
    const char *name;
    uint32_t attr_bits;
    void *cb_mem;
    uint32_t cb_size;
} osEventFlagsAttr_t;

typedef struct _osTimerAttr_t {
    const char *name;
    uint32_t attr_bits;
    void *cb_mem;
    uint32_t cb_size;
} osTimerAttr_t;

typedef struct _osMemoryPoolAttr_t {
    const char *name;
    uint32_t attr_bits;
    void *cb_mem;
    uint32_t cb_size;
    void *mp_mem;
    uint32_t mp_size;
} osMemoryPoolAttr_t;

typedef struct _osMessageQueueAttr_t {
    const char *name;
    uint32_t attr_bits;
    void *cb_mem;
    uint32_t cb_size;
    void *mq_mem;
    uint32_t mq_size;
} osMessageQueueAttr_t;

osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size);
osKernelState_t osKernelGetState(void);
osStatus_t osKernelInitialize(void);
osStatus_t osKernelStart(void);
uint32_t osKernelGetTickCount(void);
uint32_t osKernelGetTickFreq(void);
int32_t osKernelLock(void);
int32_t osKernelUnlock(void);
int32_t osKernelRestoreLock(int32_t lock);

#define osWaitForever 0xFFFFFFFFU

/* Mutex attribute flags. Note that all mutexes are robust in this
 * implementation so the value of the osMutexRobust flag is ignored. */
#define osMutexRecursive   0x00000001U
#define osMutexPrioInherit 0x00000002U
#define osMutexRobust      0x00000008U

osMutexId_t osMutexNew(const osMutexAttr_t *attr);
osStatus_t osMutexDelete(osMutexId_t mutex_id);
osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout);
osStatus_t osMutexRelease(osMutexId_t mutex_id);
const char *osMutexGetName(osMutexId_t mutex_id);
osThreadId_t osMutexGetOwner(osMutexId_t mutex_id);

osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr);
osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id);
osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout);
osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id);
const char *osSemaphoreGetName(osSemaphoreId_t semaphore_id);
uint32_t osSemaphoreGetCount(osSemaphoreId_t semaphore_id);

#define osThreadDetached 0x00000000U
#define osThreadJoinable 0x00000001U

osThreadId_t osThreadGetId(void);
osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);
osStatus_t osThreadYield(void);
osStatus_t osThreadSuspend(osThreadId_t thread_id);
osStatus_t osThreadResume(osThreadId_t thread_id);
osStatus_t osThreadDetach(osThreadId_t thread_id);
osStatus_t osThreadJoin(osThreadId_t thread_id);
osStatus_t osThreadTerminate(osThreadId_t thread_id);
__NO_RETURN void osThreadExit(void);
const char *osThreadGetName(osThreadId_t thread_id);
osStatus_t osThreadSetPriority(osThreadId_t thread_id, osPriority_t priority);
osPriority_t osThreadGetPriority(osThreadId_t thread_id);
osThreadState_t osThreadGetState(osThreadId_t thread_id);
uint32_t osThreadGetStackSize(osThreadId_t thread_id);
uint32_t osThreadGetStackSpace(osThreadId_t thread_id);

#define osFlagsErrorUnknown   0xFFFFFFFFU
#define osFlagsErrorTimeout   0xFFFFFFFEU
#define osFlagsErrorResource  0xFFFFFFFDU
#define osFlagsErrorParameter 0xFFFFFFFCU
#define osFlagsErrorISR       0xFFFFFFFAU

#define osFlagsWaitAny 0x00000000U
#define osFlagsWaitAll 0x00000001U
#define osFlagsNoClear 0x00000002U

osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t *attr);
osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id);
uint32_t osEventFlagsSet(osEventFlagsId_t ef_id, uint32_t flags);
uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags);
uint32_t osEventFlagsGet(osEventFlagsId_t ef_id);
uint32_t osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout);
const char *osEventFlagsGetName(osEventFlagsId_t ef_id);

uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags);
uint32_t osThreadFlagsClear(uint32_t flags);
uint32_t osThreadFlagsGet(void);
uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout);

osTimerId_t osTimerNew(osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr);
osStatus_t osTimerDelete(osTimerId_t timer_id);
osStatus_t osTimerStart(osTimerId_t timer_id, uint32_t ticks);
osStatus_t osTimerStop(osTimerId_t timer_id);
uint32_t osTimerIsRunning(osTimerId_t timer_id);
const char *osTimerGetName(osTimerId_t timer_id);

osMemoryPoolId_t osMemoryPoolNew(uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr);
osStatus_t osMemoryPoolDelete(osMemoryPoolId_t mp_id);
void *osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout);
osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void *block);
uint32_t osMemoryPoolGetCapacity(osMemoryPoolId_t mp_id);
uint32_t osMemoryPoolGetBlockSize(osMemoryPoolId_t mp_id);
uint32_t osMemoryPoolGetCount(osMemoryPoolId_t mp_id);
uint32_t osMemoryPoolGetSpace(osMemoryPoolId_t mp_id);
const char *osMemoryPoolGetName(osMemoryPoolId_t mp_id);

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr);
osStatus_t osMessageQueueDelete(osMessageQueueId_t mq_id);
osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);
osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout);
osStatus_t osMessageQueueReset(osMessageQueueId_t mq_id);
uint32_t osMessageQueueGetCapacity(osMessageQueueId_t mq_id);
uint32_t osMessageQueueGetMsgSize(osMessageQueueId_t mq_id);
uint32_t osMessageQueueGetCount(osMessageQueueId_t mq_id);
uint32_t osMessageQueueGetSpace(osMessageQueueId_t mq_id);
const char *osMessageQueueGetName(osMessageQueueId_t mq_id);

osStatus_t osDelay(uint32_t ticks);
osStatus_t osDelayUntil(uint32_t ticks);

#endif /* CMSIS_OS2_H_ */
