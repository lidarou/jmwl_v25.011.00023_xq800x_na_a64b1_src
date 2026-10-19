/*
 * adapter_thread.c
 *
 *  This file implements the starting of a new thread and stopping of threads.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : DDK-130_bsd                                              */
/*   Version       : 3.1                                                      */
/*   Configuration : DDK_EIP130_BSD                                           */
/*                                                                            */
/*   Date          : 2022-Feb-14                                              */
/*                                                                            */
/* Copyright (c) 2014-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or without         */
/* modification, are permitted provided that the following conditions are     */
/* met:                                                                       */
/*                                                                            */
/* 1. Redistributions of source code must retain the above copyright          */
/* notice, this list of conditions and the following disclaimer.              */
/*                                                                            */
/* 2. Redistributions in binary form must reproduce the above copyright       */
/* notice, this list of conditions and the following disclaimer in the        */
/* documentation and/or other materials provided with the distribution.       */
/*                                                                            */
/* 3. Neither the name of the copyright holder nor the names of its           */
/* contributors may be used to endorse or promote products derived from       */
/* this software without specific prior written permission.                   */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR      */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       */
/* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT           */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,      */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE      */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.       */
/* -------------------------------------------------------------------------- */

#include <pthread.h>                // pthread_create()
#include <signal.h>                 // pthread_kill()
#include <sched.h>                  // sched_yield()
#include "adapter_thread.h"         // API to implement
#include "basic_defs.h"             // IDENTIFIER_NOT_USED
#include "clib.h"
#include "adapter_lock.h"           // Adapter_Lock_t, Adapter_Lock*()
#include "adapter_sleep.h"          // Adapter_SleepMS()
#include "adapter_alloc.h"          // Adapter_Alloc() and Adapter_Free()


#define ADAPTER_THREAD_PREALLOC_SIZE    16

struct StartedThreadList
{
    struct StartedThreadList * pNext;
    struct StartedThreadList * pPrev;
    pthread_t Thread;
    bool Allocated;
};

static struct StartedThreadList * gl_ActiveRoot = NULL;
static struct StartedThreadList * gl_FreeRoot = NULL;
static struct StartedThreadList gl_Buffer[ADAPTER_THREAD_PREALLOC_SIZE];
static Adapter_Lock_t gl_AdminMutexlock = NULL;
static unsigned long gl_AdminMutexFlags = 0;


/*----------------------------------------------------------------------------
 * Adapter_ThreadStart
 */
void *
Adapter_ThreadStart(void * pThreadFunc,
                    void * pThreadData,
                    const char ThreadName[])
{
    struct StartedThreadList * pEntry;

    if ((gl_ActiveRoot == NULL) && (gl_FreeRoot == NULL))
    {
        int i;

        /* Initialize administration lock */
        gl_FreeRoot = &gl_Buffer[0];
        gl_AdminMutexlock = Adapter_Lock_Alloc();
        Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);

        /* Initialize pre-allocated free list */
        memset(gl_Buffer, 0, sizeof(gl_Buffer));
        for (i = 0; i < (ADAPTER_THREAD_PREALLOC_SIZE-1); i++)
        {
            gl_Buffer[i].pNext = &gl_Buffer[i+1];
        }

        IDENTIFIER_NOT_USED(ThreadName);
    }
    else
    {
        if (gl_AdminMutexlock == NULL)
        {
            Adapter_SleepMS(100);
        }
        Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
    }

    /* Get a free entry */
    pEntry = gl_FreeRoot;
    if (pEntry == NULL)
    {
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);

        pEntry = Adapter_Alloc(sizeof(struct StartedThreadList));
        if (pEntry != NULL)
        {
            memset(pEntry, 0, sizeof(struct StartedThreadList));
            pEntry->Allocated = true;
        }
    }
    else
    {
        gl_FreeRoot = pEntry->pNext;
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
    }

    if (pEntry != NULL)
    {
        int rc;

        pEntry->pPrev = NULL;

        /* Start the thread */
        rc = pthread_create(&pEntry->Thread, NULL, pThreadFunc, pThreadData);
        Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
        if (rc == 0)
        {
            /* Place entry in active list */
            if (gl_ActiveRoot != NULL)
            {
                pEntry->pNext = gl_ActiveRoot;
                gl_ActiveRoot->pPrev = pEntry;
            }
            else
            {
                pEntry->pNext = NULL;
            }
            gl_ActiveRoot = pEntry;
        }
        else
        {
            /* Place entry in free list again */
            pEntry->pNext = gl_FreeRoot;
            gl_FreeRoot = pEntry;
            pEntry = NULL;
        }
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
    }

    return (void *)pEntry;
}

/*----------------------------------------------------------------------------
 * StopThreadAndReleaseEntry
 */
static void
StopThreadAndReleaseEntry(struct StartedThreadList * pEntry)
{
    Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
    if (pEntry->pNext != NULL)
    {
        pEntry->pNext->pPrev = pEntry->pPrev;
    }
    if (pEntry->pPrev != NULL)
    {
        pEntry->pPrev->pNext = pEntry->pNext;
    }
    else
    {
        gl_ActiveRoot = pEntry->pNext;
    }
    Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);

    (void)pthread_detach(pEntry->Thread);
    (void)pthread_cancel(pEntry->Thread);
    memset(&pEntry->Thread, 0, sizeof(pEntry->Thread));
    pEntry->pPrev = NULL;

    if (pEntry->Allocated)
    {
        pEntry->pNext = NULL;
        pEntry->Allocated = false;
        Adapter_Free(pEntry);
    }
    else
    {
        Adapter_Lock_Acquire(gl_AdminMutexlock, &gl_AdminMutexFlags);
        pEntry->pNext = gl_FreeRoot;
        gl_FreeRoot = pEntry;
        Adapter_Lock_Release(gl_AdminMutexlock, &gl_AdminMutexFlags);
    }
}

/*----------------------------------------------------------------------------
 * Adapter_ThreadStop
 */
void
Adapter_ThreadStop(void * pThreadRef)
{
    if (pThreadRef == NULL)
    {
        /* Stop all threads */
        while (gl_ActiveRoot != NULL)
        {
            StopThreadAndReleaseEntry(gl_ActiveRoot);
        }
        gl_FreeRoot = NULL;

        if (gl_AdminMutexlock != NULL)
        {
            Adapter_Lock_Free(gl_AdminMutexlock);
            gl_AdminMutexlock = NULL;
        }
    }
    else
    {
        /* Stop a specified thread but first check the thread reference */
        struct StartedThreadList * pEntry = gl_ActiveRoot;
        while (pEntry != NULL)
        {
            if (pEntry == pThreadRef)
            {
                /* Valid thread reference thus stop the thread */
                StopThreadAndReleaseEntry(pEntry);
                break;
            }
            pEntry = pEntry->pNext;
        }
    }
}

/*----------------------------------------------------------------------------
 * Adapter_ThreadSetCancelState
 */
void
Adapter_ThreadSetCancelState(void)
{
    (void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    (void)pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
}

/*----------------------------------------------------------------------------
 * Adapter_ThreadShouldStop
 */
bool
Adapter_ThreadShouldStop(void)
{
    pthread_testcancel();
    return false;
}

/*----------------------------------------------------------------------------
 * Adapter_ThreadYield
 */
void
Adapter_ThreadYield(void)
{
    sched_yield();
}

/*----------------------------------------------------------------------------
 * Adapter_ThreadMemoryBarrier
 */
void
Adapter_ThreadMemoryBarrier(bool Enter)
{
    IDENTIFIER_NOT_USED(Enter);
}

/*----------------------------------------------------------------------------
 * Adapter_ThreadMemoryBarrierRead
 */
void
Adapter_ThreadMemoryBarrierRead(void)
{
}

/*----------------------------------------------------------------------------
 * Adapter_ThreadMemoryBarrierWrite
 */
void
Adapter_ThreadMemoryBarrierWrite(void)
{
}


/* end of file adapter_thread.c */
