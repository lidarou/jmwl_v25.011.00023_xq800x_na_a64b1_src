/*
 * Copyright (c) 2021-2031, STONEOIM and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef _RTOS_DEF_H_
#define _RTOS_DEF_H_

#include <aic_errno.h>
#include <aic_osa.h>

#include <porting.h>

/*============OS parameter setting===================*/
#define OS_TASK_QUEUE_NUM (120)
#define OS_MSG_Q_NAME_LEN (128)

typedef void (*OsTask)(void *);

typedef void*				    OsTaskHandle;
typedef void*			OsMutex;
typedef void*			OsSemaphore;
typedef void*				OsMsgQ;
typedef void*                OsTimer;
typedef void*           OsTimerHandler;

#define OS_TASK_ENTER_CRITICAL()        ssv_type_u32   os_flags; os_flags = OS_EnterCritical()
#define OS_TASK_EXIT_CRITICAL()         OS_ExitCritical(os_flags)

typedef struct {
    OsTask entry;
    void *entry_param;
    aos_thread_id thread_id;
}OsThread;

typedef struct {
    aos_timer_ptr timer;
    aos_expiration_func func;
    uint32 timer_expire;
    uint8 autoload;
}OsTimer_t;

typedef struct {
    aos_semaphore_ptr sem;
    uint32 sem_count; 
}OsSemaphore_t;


#define TICK_RATE_MS  (1)

#endif
