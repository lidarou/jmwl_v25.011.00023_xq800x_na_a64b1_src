/* adapter_lock.c
 *
 * Adapter concurrency (locking) management
 * MicroBlaze FreeRTOS implementation
 *
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include <aic_osa.h>

// Adapter locking API
#include "adapter_lock.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"     // IDENTIFIER_NOT_USED

// Adapter Lock Internal API
#include "adapter_lock_internal.h"

// Adapter Memory Allocation API
#include "adapter_alloc.h"

// Logging API
#undef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX    LOG_SEVERITY_WARN

// Logging API
#include "log.h"

#if 0
// FreeRTOS API
#include "FreeRTOS.h"
#include "portable.h"   // pvPortMalloc, vPortFree
#include "task.h"       // vTaskDelay
#include "semphr.h"     // *Semaphore*
#endif


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef struct
{
    aos_semaphore_ptr SemHandle;
} Adapter_Lock_Internal_t;


/*----------------------------------------------------------------------------
 * Adapter_Lock_Alloc
 */
Adapter_Lock_t
Adapter_Lock_Alloc(void)
{
    Adapter_Lock_Internal_t * Lock_p;

    Lock_p = Adapter_Alloc(sizeof(Adapter_Lock_Internal_t));
    if (Lock_p == NULL)
        return Adapter_Lock_NULL;

    Log_FormattedMessage("%s: Lock = FreeRTOS binary semaphore\n", __func__);

    aos_create_semaphore(Lock_p->SemHandle, 0);

    if (Lock_p->SemHandle)
        return Lock_p; // success
    else
    {
        Adapter_Free((void*)Lock_p);
        return Adapter_Lock_NULL; // error
    }
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Free
 */
void
Adapter_Lock_Free(
        Adapter_Lock_t Lock)
{
    Adapter_Lock_Internal_t * Lock_p = (Adapter_Lock_Internal_t*)Lock;

    aos_delete_semaphore(Lock_p->SemHandle);

    Adapter_Free((void*)Lock_p);
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Acquire
 */
void
Adapter_Lock_Acquire(
        Adapter_Lock_t Lock,
        unsigned long * Flags)
{
    Adapter_Lock_Internal_t * Lock_p = (Adapter_Lock_Internal_t*)Lock;

    IDENTIFIER_NOT_USED(Flags);

    aos_get_semaphore(Lock_p->SemHandle, AOS_WAIT_FOREVER);
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Release
 */
void
Adapter_Lock_Release(
        Adapter_Lock_t Lock,
        unsigned long * Flags)
{
    Adapter_Lock_Internal_t * Lock_p = (Adapter_Lock_Internal_t*)Lock;

    IDENTIFIER_NOT_USED(Flags);

    aos_put_semaphore(Lock_p->SemHandle);
}


/* end of file adapter_lock.c */
